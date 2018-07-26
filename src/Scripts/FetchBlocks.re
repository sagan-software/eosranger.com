open Js.Promise;

type state = array(Endpoints.state);

let initialState = endpoints =>
  endpoints |. Belt.Array.map(Endpoints.initialState);

let emptyState = [||];

let fetchBlock = (endpoint, num) =>
  Request.make(
    ~url=endpoint |> URL.makeWithBase("/v1/chain/get_block") |. URL.toString,
    ~timeout=5000,
    ~method_="POST",
    ~body=
      [("block_num_or_id", num |. Json.Encode.int)]
      |. Json.Encode.object_
      |. Json.stringify,
    ~time=true,
    ~simple=true,
    (),
  )
  |> then_(response =>
       response
       |. Request.body
       |. Util.parseJsonAsPromise
       |> then_(json =>
            resolve((
              json,
              response
              |. Request.timings
              |. Js.Nullable.toOption
              |. Belt.Option.map(Request.Timings.end_Get)
              |. Belt.Option.getWithDefault(Env.throttleTime |. float_of_int),
            ))
          )
     )
  |. Util.promiseToOption;

let rec fetchNextBlock = endpoint =>
  Env.Q.pop()
  |> then_(blockNum =>
       switch (blockNum) {
       | Some(blockNum) =>
         endpoint
         |. fetchBlock(blockNum)
         |> then_(blockOpt =>
              switch (blockOpt) {
              | Some((block, responseTime)) =>
                switch (
                  block |> Json.Decode.field("block_num", Json.Decode.int)
                ) {
                | actualBlockNum =>
                  if (blockNum == actualBlockNum) {
                    [%bs.raw {|block.irreversible = true|}] |. ignore;
                    Env.Db.save(block)
                    |> then_(_ => {
                         let throttleTime =
                           responseTime *. Env.responseTimeMultiplier;
                         resolve(throttleTime |. int_of_float);
                       })
                    |> catch(error => {
                         Log.error(
                           __MODULE__,
                           {j|Error saving block from $endpoint to DB, added block $blockNum back to the queue.|j},
                           error,
                         );
                         resolve(Env.throttleTime);
                       });
                  } else {
                    Env.Q.push([|blockNum|])
                    |> then_(_ => {
                         Log.error(
                           __MODULE__,
                           {j|Received unexpected block number $actualBlockNum when fetching block $blockNum from $endpoint|j},
                           "",
                         );
                         resolve(Env.throttleTime);
                       });
                  }
                | exception _ =>
                  Env.Q.push([|blockNum|])
                  |> then_(_ => {
                       Log.error(
                         __MODULE__,
                         {j|Couldn't decode block number from response from $endpoint for block $blockNum|j},
                         block,
                       );
                       resolve(Env.throttleTime);
                     })
                }
              | None =>
                Env.Q.push([|blockNum|])
                |> then_(_ => {
                     Log.error(
                       __MODULE__,
                       {j|Error fetching block from $endpoint, added block $blockNum back to the queue.|j},
                       "",
                     );
                     resolve(Env.throttleTime);
                   })
              }
            )
         |> catch(error =>
              Env.Q.push([|blockNum|])
              |> then_(_ => {
                   Log.error(
                     __MODULE__,
                     {j|Error fetching block from $endpoint, added block $blockNum back to the queue.|j},
                     error,
                   );
                   resolve(Env.throttleTime);
                 })
            )
       | None => resolve(Env.throttleTime)
       }
     )
  |> then_(Util.timeoutPromise)
  |> then_(_ => fetchNextBlock(endpoint));

let estimateTimeToComplete = (numBlocksInQueue, blocksPerSecond) => {
  let now = Js.Date.now();
  let secFromNow = numBlocksInQueue / blocksPerSecond |. float_of_int;
  let endTime = now +. secFromNow *. 1000.;
  let endDate = Js.Date.fromFloat(endTime);
  endDate |. Moment.fromDate |. Moment.fromNow;
};

let rec reportStats = () =>
  Env.Db.count()
  |> then_(numBlocks =>
       Util.timeoutPromise(Env.reportStatsTimeout)
       |> then_(_ => all2((Env.Db.count(), Env.Q.count())))
       |> then_(((newBlockCount, numInQueue)) => {
            let numNewBlocks = newBlockCount - numBlocks;
            let blocksPerSecond =
              numNewBlocks / (Env.reportStatsTimeout / 1000);
            let timeLeft =
              estimateTimeToComplete(numInQueue, blocksPerSecond);
            let now = Js.Date.make() |. Js.Date.toISOString;
            let message = {j|------------------------
        $blocksPerSecond blocks per second
            $newBlockCount blocks in ArangoDB
            $numInQueue in Redis queue
Should finish $timeLeft
|j};
            Log.info(__MODULE__, message, "");
            reportStats();
          })
     );

Env.Db.setup()
|> Js.Promise.then_(_ => {
     Log.info(__MODULE__, "Database setup", "");
     Env.endpoints
     |. Belt.Array.map(fetchNextBlock)
     |. all
     |> then_(_ => Log.info(__MODULE__, "Done", "") |. resolve);
   });

reportStats();
