module type Queue = {
  let count: Eos.ChainId.t => Js.Promise.t(int);
  let push: (Eos.ChainId.t, array(Eos.BlockNum.t)) => Js.Promise.t(int);
  let pop: Eos.ChainId.t => Js.Promise.t(option(Eos.BlockNum.t));
  let chainIds: unit => Js.Promise.t(array(Eos.ChainId.t));
};