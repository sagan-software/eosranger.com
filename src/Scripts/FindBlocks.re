let redis = Redis.make();

open Js.Promise;

let findNewBlocks = () =>
  all2((
    Database.getLargestBlockNum(),
    Env.endpoints |. Endpoints.initialStates |. Endpoints.updateInfoForStates,
  ))
  |> then_(((largestDbBlockNum, endpoints)) => {
       Log.info(
         __MODULE__,
         "Largest block number from database:",
         largestDbBlockNum,
       );
       let largestEndpointBlockNums =
         Endpoints.getLargestBlockNums(endpoints);
       Log.info(
         __MODULE__,
         "Largest block number from endpoints:",
         largestEndpointBlockNums.head,
       );
       let startBlockNum =
         min(largestDbBlockNum + 1, largestEndpointBlockNums.irreversible);
       let endBlockNum = largestEndpointBlockNums.head;
       let blockNumChunks =
         Belt.Array.range(startBlockNum, endBlockNum)
         |. Belt.Array.map(string_of_int)
         |. Util.chunkArray(1000);
       blockNumChunks
       |. Belt.Array.reduce(Js.Promise.resolve(0), (promise, blockNums) =>
            promise
            |> then_(totalAdded =>
                 redis
                 |. Redis.sadd("blockNums", blockNums)
                 |> then_(numAdded => {
                      Log.info(
                        __MODULE__,
                        {j|Added $numAdded items to the blockNums queue|j},
                        "",
                      );
                      numAdded + totalAdded |. resolve;
                    })
               )
          );
     });

Database.setup() |> then_(_ => findNewBlocks());
