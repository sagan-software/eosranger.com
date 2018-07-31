module type T = {
  let startTime: Js.Date.t;
  let nodes: Js.Dict.t(Eosr_Node.t);
  let chains: Js.Dict.t(Eos.BlockNum.t);
  let addNode: Eosr_Node.t => unit;
  let addEndpoint: string => unit;
  let run: unit => Js.Promise.t(array(Eosr_Node.t));
};

module Make = (Queue: Eosr_Queue.Queue, Store: Eosr_Store.Store) : T => {
  let startTime = Js.Date.make();
  let nodes = Js.Dict.empty();
  let chains = Js.Dict.empty();

  let addNode = node => {
    let key = node |. Eosr_Node.key;
    nodes |. Js.Dict.set(key, node);
  };

  let addEndpoint = endpoint => endpoint |. Eosr_Node.make |. addNode;

  let getInfo = (node: Eosr_Node.t) => node |. Eosr_Node.getInfo;

  let getBlock = (node: Eosr_Node.t, blockNum: Eos.BlockNum.t) => ();

  let useNode = (node: Eosr_Node.t) =>
    switch (node.info) {
    | Some(info) => node |. getInfo
    | None => node |. getInfo
    };

  let run = () =>
    nodes |. Js.Dict.values |. Belt.Array.map(useNode) |. Js.Promise.all;
};