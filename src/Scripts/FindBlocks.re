let redis = Redis.make();

open Js.Promise;

let rec findNewBlocks = () =>
  all2((
    Env.Db.largestBlockNum(),
    Env.endpoints |. Endpoints.initialStates |. Endpoints.updateInfoForStates,
  ))
  |> then_(((dbBlockNum, endpoints)) => {
       let (_, liveBlockNums) = Endpoints.getLargestBlockNums(endpoints);
       let head = liveBlockNums.head;
       let irreversible = liveBlockNums.irreversible;
       let diff = head - dbBlockNum;
       Log.info(
         __MODULE__,
         {j|Block numbers: db=$dbBlockNum irreversible=$irreversible head=$head diff=$diff|j},
         "",
       );
       let startBlockNum = min(dbBlockNum + 1, liveBlockNums.irreversible);
       let endBlockNum = liveBlockNums.head;
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
                      if (numAdded > 0) {
                        Log.info(
                          __MODULE__,
                          {j|Added $numAdded items to the blockNums queue|j},
                          "",
                        );
                      };
                      numAdded + totalAdded |. resolve;
                    })
               )
          );
     })
  |> then_(_ => findNewBlocks());

Env.Db.setup()
|> then_(_ => findNewBlocks())
|> then_(_ => Env.Db.findMissing())
|> then_(_ =>
     Js.Global.setInterval(
       () => Env.Db.findMissing() |. ignore,
       15 * 60 * 1000,
     )
     |. Js.Promise.resolve
   );
