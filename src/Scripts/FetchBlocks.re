open Js.Promise;

let saveBlock = (isIrreversible, json) => {
  Util.doNothing(isIrreversible);
  [%bs.raw {|json._key = json.block_num + ""|}] |. ignore;
  [%bs.raw {|json.irreversible = isIrreversible|}] |. ignore;
  let saveOpts = Arango.Collection.saveOpts(~overwrite=true, ());
  Database.db
  |. Arango.Db.collection("blocks")
  |. Arango.Collection.saveWithOpts(json, saveOpts);
};

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

let redis = Redis.make();

let rec fetchNextBlock = endpoint =>
  redis
  |. Redis.spop("blockNums")
  |> then_(blockNum =>
       switch (blockNum |. Js.Nullable.toOption) {
       | Some(blockNum) =>
         endpoint
         |. fetchBlock(blockNum |. int_of_string)
         |> then_(blockOpt =>
              switch (blockOpt) {
              | Some((block, responseTime)) =>
                saveBlock(true, block)
                |> then_(_ => {
                     let throttleTime =
                       responseTime *. Env.responseTimeMultiplier;
                     resolve(throttleTime |. int_of_float);
                   })
              | None =>
                redis
                |. Redis.sadd("blockNums", [|blockNum|])
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
       | None =>
         Log.info(__MODULE__, "Block num queue is empty", "");
         resolve(Env.throttleTime);
       }
     )
  |> then_(Util.timeoutPromise)
  |> then_(_ => fetchNextBlock(endpoint));

Env.endpoints
|. Belt.Array.map(fetchNextBlock)
|. all
|> then_(_ => Log.info(__MODULE__, "Done", "") |. resolve);

let getBlockCount = () =>
  "RETURN LENGTH(blocks)"
  |> Arango.Db.query(Database.db)
  |> Js.Promise.then_(Arango.Cursor.next)
  |> Js.Promise.then_(json =>
       json
       |. Js.Nullable.toOption
       |. Belt.Option.map(Json.Decode.int)
       |. Belt.Option.getWithDefault(0)
       |. Js.Promise.resolve
     );

let estimateTimeToComplete = (numBlocksInQueue, blocksPerSecond) => {
  let now = Js.Date.now();
  let secFromNow = numBlocksInQueue / blocksPerSecond |. float_of_int;
  let endTime = now +. secFromNow *. 1000.;
  let endDate = Js.Date.fromFloat(endTime);
  endDate |. Moment.fromDate |. Moment.fromNow;
};

let rec reportStats = () =>
  getBlockCount()
  |> then_(numBlocks =>
       Util.timeoutPromise(Env.reportStatsTimeout)
       |> then_(_ =>
            all2((getBlockCount(), redis |. Redis.scard("blockNums")))
          )
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

reportStats();
