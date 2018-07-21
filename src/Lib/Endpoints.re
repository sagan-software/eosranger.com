type state = {
  endpoint: string,
  info: option(Eos.Info.t),
  averageResponseTime: float,
  numErrors: int,
  numSuccess: int,
};

let initialState = endpoint => {
  endpoint,
  info: None,
  averageResponseTime: 1.,
  numErrors: 0,
  numSuccess: 0,
};

let initialStates = endpoints => endpoints |. Belt.Array.map(initialState);

let updateInfoForState = state =>
  Request.make(
    ~url=
      state.endpoint |> URL.makeWithBase("/v1/chain/get_info") |. URL.toString,
    ~timeout=5000,
    (),
  )
  |> Js.Promise.then_(response =>
       response
       |. Request.body
       |> Util.parseAndDecodeAsPromise(Eos.Info.decode)
     )
  |. Util.promiseToResult
  |> Js.Promise.then_(result =>
       switch (result) {
       | Belt.Result.Ok(info) =>
         /* Log.info(__MODULE__, "OK get_info:", state.endpoint); */
         {...state, info: Some(info), numSuccess: state.numSuccess + 1}
         |. Js.Promise.resolve
       | Belt.Result.Error(_error) =>
         Log.error(__MODULE__, "BAD get_info:", state.endpoint);
         {...state, info: None, numErrors: state.numErrors + 1}
         |. Js.Promise.resolve;
       }
     );

let updateInfoForStates = states =>
  states |. Belt.Array.map(updateInfoForState) |. Js.Promise.all;

type blockNums = {
  head: int,
  irreversible: int,
};

let getLargestBlockNums = states =>
  states
  |. Belt.Array.map(state => state.info)
  |. Util.onlySome
  |. Belt.Array.reduce({head: 0, irreversible: 0}, (largestBlockNums, info) =>
       info.headBlockNum > largestBlockNums.head ?
         {
           head: info.headBlockNum,
           irreversible: info.lastIrreversibleBlockNum,
         } :
         largestBlockNums
     );

let onlyWithInfo = states =>
  states
  |. Belt.Array.reduce(
       [||],
       (result, state) => {
         switch (state.info) {
         | Some(info) => result |> Js.Array.push((state, info)) |> ignore
         | None => ()
         };
         result;
       },
     );

type blockResult = {
  state,
  num: int,
  block: option(Js.Json.t),
};

let getBlock = (state, num) =>
  Request.make(
    ~url=
      state.endpoint
      |> URL.makeWithBase("/v1/chain/get_block")
      |. URL.toString,
    ~timeout=5000,
    ~method_="POST",
    ~body=
      [("block_num_or_id", num |. Json.Encode.int)]
      |. Json.Encode.object_
      |. Json.stringify,
    (),
  )
  |> Js.Promise.then_(response =>
       response |. Request.body |. Util.parseJsonAsPromise
     )
  |. Util.promiseToResult
  |> Js.Promise.then_(result =>
       switch (result) {
       | Belt.Result.Ok(block) =>
         /* Log.info(__MODULE__, "OK get_block:", state.endpoint); */
         {
           state: {
             ...state,
             numSuccess: state.numSuccess + 1,
           },
           num,
           block: Some(block),
         }
         |. Js.Promise.resolve
       | Belt.Result.Error(_error) =>
         Log.error(__MODULE__, "BAD get_block:", state.endpoint);
         {
           state: {
             ...state,
             numErrors: state.numErrors + 1,
           },
           num,
           block: None,
         }
         |. Js.Promise.resolve;
       }
     );
