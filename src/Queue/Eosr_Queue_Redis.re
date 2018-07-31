module Redis = {
  /* Add Async functions to redis */
  [@bs.module "bluebird"] external promisifyAll : 'm => unit = "";

  type t;

  [@bs.module "redis"] external make_ : unit => t = "createClient";

  let make = () => {
    let client = make_();
    promisifyAll(client);
    client;
  };

  [@bs.send]
  external sadd : (t, string, array(string)) => Js.Promise.t(int) =
    "saddAsync";

  [@bs.send]
  external spop : (t, string) => Js.Promise.t(Js.Nullable.t(string)) =
    "spopAsync";

  [@bs.send]
  external smembers : (t, string) => Js.Promise.t(array(string)) =
    "smembersAsync";

  [@bs.send] external scard : (t, string) => Js.Promise.t(int) = "scardAsync";
};

let count = (client, chainId) =>
  client |. Redis.scard(chainId |. Eos.ChainId.toString);

let push = (client, chainId, blockNums) => {
  let key = chainId |. Eos.ChainId.toString;
  let values =
    blockNums
    |. Belt.Array.map(bn => bn |. Eos.BlockNum.toInt |. string_of_int);
  client |. Redis.sadd(key, values);
};

let pop = (client, chainId) =>
  client
  |. Redis.spop(chainId |. Eos.ChainId.toString)
  |> Js.Promise.then_(blockNum =>
       blockNum
       |. Js.Nullable.toOption
       |. Belt.Option.map(str => str |. int_of_string |. Eos.BlockNum.fromInt)
       |. Js.Promise.resolve
     );

let chainIds = (_client, ()) => Js.Promise.resolve([||]);

module Make = (()) : Eosr_Queue.Queue => {
  let client = Redis.make();
  let count = client |. count;
  let push = client |. push;
  let pop = client |. pop;
  let chainIds = client |. chainIds;
};