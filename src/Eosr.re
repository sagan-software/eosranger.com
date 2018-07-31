module Queue =
  Eosr_Queue_Redis.Make({});

module Store =
  Eosr_Store_Arango.Make({
    let db = "eos";
    let user = "root";
    let pass = "openSesame";
  });

module Bot = Eosr_Bot.Make(Queue, Store);

Eosr_Env.endpoints |. Belt.Array.map(Bot.addEndpoint);

Bot.run()
|> Js.Promise.then_(nodes => Js.log2("nodes", nodes) |. Js.Promise.resolve);