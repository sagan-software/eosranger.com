type block = {
  chainId: Eos.ChainId.t,
  blockId: Eos.BlockId.t,
  blockNum: Eos.BlockNum.t,
  irreversible: bool,
  data: Js.Json.t,
};

module type Store = {
  let setup: unit => Js.Promise.t(unit);
  let count: Eos.ChainId.t => Js.Promise.t(int);
  let save: block => Js.Promise.t(unit);
  let scan:
    (Eos.ChainId.t, ~start: Eos.BlockNum.t, ~limit: int) =>
    Js.Promise.t(array(Eos.BlockNum.t));
};