module Request = {
  type t;

  [@bs.get] external statusCode : t => int = "";
  [@bs.get] external statusText : t => string = "statusMessage";
  [@bs.get] external body : t => string = "";
  [@bs.get] external headers : t => Js.Dict.t(string) = "";
  let header = (t, key) => t |> headers |. Js.Dict.get(key);
  [@bs.get] external httpVersionMajor : t => int = "";
  [@bs.get] external httpVersionMinor : t => int = "";
  [@bs.get] [@bs.scope "request"] external url : t => URL.t = "uri";

  module Options = {
    [@bs.deriving abstract]
    type t = {
      url: string,
      [@bs.as "method"]
      method_: string,
      [@bs.optional]
      json: bool,
      [@bs.optional]
      resolveWithFullResponse: bool,
      [@bs.optional]
      body: string,
      [@bs.optional]
      simple: bool,
      [@bs.optional]
      time: bool,
      [@bs.optional]
      timeout: int,
      [@bs.optional]
      headers: Js.Dict.t(string),
      [@bs.optional]
      encoding: Js.Null_undefined.t(string),
    };
  };

  [@bs.module]
  external make_ : Options.t => Js.Promise.t(t) = "request-promise";

  exception TimedOut(string);
  exception UnknownError(Js.Promise.error);

  module Error = {
    type t;
    [@bs.get] external options : t => Options.t = "";
    [@bs.get] external name : t => string = "";
    [@bs.get] external message : t => string = "";
    [@bs.get] external error : t => Js.Exn.t = "";
    [@bs.get] [@bs.scope "error"] external code : t => string = "";
    external fromPromiseError : Js.Promise.error => t = "%identity";
  };

  let make =
      (
        ~url,
        ~method_="GET",
        ~json=false,
        ~body=?,
        ~timeout=0,
        ~headers=?,
        ~encoding=Js.Null_undefined.undefined,
        ~simple=?,
        ~time=?,
        (),
      ) =>
    Options.t(
      ~url,
      ~method_,
      ~json,
      ~resolveWithFullResponse=true,
      ~body?,
      ~simple?,
      ~time?,
      ~timeout,
      ~headers=headers |> Js.Option.getWithDefault([||]) |> Js.Dict.fromArray,
      ~encoding,
      (),
    )
    |> make_
    |> Js.Promise.catch(e =>
         (
           switch (e |> Error.fromPromiseError |> Error.code) {
           | "ETIMEDOUT" => TimedOut(url)
           | _ => UnknownError(e)
           }
         )
         |> Js.Promise.reject
       );

  module TimingPhases = {
    [@bs.deriving abstract]
    type t = {
      wait: float,
      dns: float,
      tcp: float,
      firstByte: float,
      download: float,
      total: float,
    };
  };

  [@bs.get] external timingPhases : t => Js.Nullable.t(TimingPhases.t) = "";

  module Timings = {
    [@bs.deriving abstract]
    type t = {
      socket: float,
      lookup: float,
      connect: float,
      response: float,
      [@bs.as "end"]
      end_: float,
    };
  };

  [@bs.get] external timings : t => Js.Nullable.t(Timings.t) = "";
};

type t = {
  endpoint: string,
  info: option(Eos.Info.t),
  errorTimes: array(float),
  responseTimes: array(float),
  numErrors: int,
  numSuccess: int,
  numBlocks: int,
};

let make = endpoint => {
  endpoint,
  info: None,
  errorTimes: [||],
  responseTimes: [||],
  numErrors: 0,
  numSuccess: 0,
  numBlocks: 0,
};

let key = t => t.endpoint |. URL.parse |. URL.hostname;

let empty = make("");

let pruneErrorTimes = t => {
  let now = Js.Date.now();
  let cutoff = now -. 10. *. 60. *. 1000.; /* 10 minutes */
  {...t, errorTimes: t.errorTimes |> Js.Array.filter(e => e > cutoff)};
};

let addErrorTime = t => {
  t.errorTimes |> Js.Array.push(Js.Date.now()) |. ignore;
  t |. pruneErrorTimes;
};

let addError = t => {...t, numErrors: t.numErrors + 1} |. addErrorTime;

let addSuccess = t => {...t, numSuccess: t.numSuccess + 1};

let pruneResponseTimes = t => {
  ...t,
  responseTimes: t.responseTimes |. Belt.Array.slice(~offset=-30, ~len=30),
};

let addResponseTime = (t, r) => {
  let responseTime =
    r
    |. Request.timings
    |. Js.Nullable.toOption
    |. Belt.Option.map(Request.Timings.end_Get)
    |. Belt.Option.getWithDefault(Eosr_Env.throttleTime |. float_of_int);
  t.responseTimes |> Js.Array.push(responseTime) |. ignore;
  t |. pruneResponseTimes;
};

let avgResponseTime = t => {
  let total = t.responseTimes |. Belt.Array.reduce(0., (a, b) => a +. b);
  let count = t.responseTimes |. Belt.Array.length |. max(1) |. float_of_int;
  total /. count;
};

let getThrottleTime = t => {
  let responseTime = t |. avgResponseTime;
  let errorCount =
    t.errorTimes |. Belt.Array.length |. max(1) |. float_of_int;
  let errorMultiplier = Js.Math.pow_float(~base=2., ~exp=errorCount);
  let baseMultiplier = 1.5;
  responseTime *. baseMultiplier *. errorMultiplier;
};

let getInfo = t =>
  Request.make(
    ~url=t.endpoint |> URL.makeWithBase("/v1/chain/get_info") |. URL.toString,
    ~timeout=5000,
    (),
  )
  |> Js.Promise.then_(res => {
       let body = res |. Request.body;
       let info = Eosr_Util.decodeText(Eos.Info.decode, body);
       let t =
         switch (info) {
         | Some(_) => {...t, info} |. addSuccess
         | None => t |. addError
         };
       t |. addResponseTime(res) |. Js.Promise.resolve;
     })
  |> Js.Promise.catch(_ => t |. addError |. Js.Promise.resolve);

let getBlock = (t, blockNum) =>
  Request.make(
    ~url=
      t.endpoint |> URL.makeWithBase("/v1/chain/get_block") |. URL.toString,
    ~timeout=5000,
    ~method_="POST",
    ~body=
      [("block_num_or_id", blockNum |. Eos.BlockNum.encode)]
      |. Json.Encode.object_
      |. Json.stringify,
    (),
  )
  |> Js.Promise.then_(res => {
       let body = res |. Request.body;
       let block = body |. Json.parse;
       let t =
         switch (block) {
         | Some(_) => t |. addSuccess
         | None => t |. addError
         };
       (t |. addResponseTime(res), block) |. Js.Promise.resolve;
     })
  |> Js.Promise.catch(_ => (t |. addError, None) |. Js.Promise.resolve);