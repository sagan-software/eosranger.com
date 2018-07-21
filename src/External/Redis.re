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
external rpush : (t, string, array(string)) => Js.Promise.t(int) =
  "rpushAsync";

[@bs.send]
external lpop : (t, string) => Js.Promise.t(Js.Nullable.t(string)) =
  "lpopAsync";

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
