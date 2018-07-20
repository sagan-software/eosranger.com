type state = array(Endpoints.state);

let initialState = Env.endpoints |. Endpoints.initialStates;

let estimateTimeToComplete = (numBlocksInQueue, numValidEndpoints) => {
  let now = Js.Date.now();
  let numBlocksPerEndpoint =
    (numBlocksInQueue |. float_of_int) /. (numValidEndpoints |. float_of_int);
  let msFromNow = numBlocksPerEndpoint *. (Env.throttleTime |. float_of_int);
  let endTime = now +. msFromNow;
  let endDate = Js.Date.fromFloat(endTime);
  Log.info(
    __MODULE__,
    {j|Expecting to fetch $numBlocksInQueue blocks from $numValidEndpoints endpoints|j},
    endDate |. Moment.fromDate |. Moment.fromNow,
  );
};

let rec start = endpoints =>
  Js.Promise.all2((
    Database.getLargestBlockNum(),
    endpoints |. Endpoints.updateInfoForStates,
  ))
  |> Js.Promise.then_(((largestDbBlockNum, endpoints)) => {
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

       let numBlocksDifference = largestEndpointBlockNums.head - startBlockNum;
       let numBlocksToFetch = min(Env.maxBlocksPerCycle, numBlocksDifference);
       Log.info(
         __MODULE__,
         "Number of blocks to fetch in this cycle:",
         numBlocksToFetch,
       );
       let okEndpoints = endpoints |. Endpoints.onlyWithInfo;
       let numValidEndpoints = okEndpoints |. Belt.Array.length;
       estimateTimeToComplete(numBlocksToFetch, numValidEndpoints);
       let numBlocksPerEndpoint = numBlocksToFetch / numValidEndpoints;
       okEndpoints
       |. Belt.Array.mapWithIndex((index, (endpoint, info)) => {
            let begin_ = index * numBlocksPerEndpoint + startBlockNum;
            let end_ = begin_ + numBlocksPerEndpoint;
            Belt.Array.range(begin_, end_)
            |. Belt.Array.reduce(
                 Js.Promise.resolve(endpoint), (result, blockNum) =>
                 result
                 |> Js.Promise.then_((state: Endpoints.state) =>
                      Endpoints.getBlock(state, blockNum)
                    )
                 |> Js.Promise.then_((blockResult: Endpoints.blockResult) =>
                      switch (blockResult.block) {
                      | Some(block) =>
                        let isIrreversible =
                          blockNum <= largestEndpointBlockNums.irreversible;
                        Database.saveBlock(isIrreversible, block)
                        |> Js.Promise.then_(_result
                             /* Log.info(__MODULE__, "Saved block", blockNum); */
                             => Util.timeoutPromise(Env.throttleTime))
                        |> Js.Promise.then_(_ =>
                             Js.Promise.resolve(blockResult.state)
                           );
                      | None =>
                        Log.error(
                          __MODULE__,
                          {j|Didn't fetch block $blockNum from endpoint:|j},
                          endpoint.endpoint,
                        );
                        Util.timeoutPromise(Env.throttleTime)
                        |> Js.Promise.then_(_ =>
                             Js.Promise.resolve(blockResult.state)
                           );
                      }
                    )
               );
          })
       |. Js.Promise.all;
     })
  |> Js.Promise.then_(_ => {
       Log.info(__MODULE__, "Restarting...", "");
       start(endpoints);
     });
