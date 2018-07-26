module type Q = {
  let count: unit => Js.Promise.t(int);
  let push: array(int) => Js.Promise.t(int);
  let pop: unit => Js.Promise.t(option(int));
};

module type Db = {
  let setup: unit => Js.Promise.t(unit);
  let count: unit => Js.Promise.t(int);
  let save: Js.Json.t => Js.Promise.t(Js.Json.t);
  let largestBlockNum: unit => Js.Promise.t(int);
  let findMissing: unit => Js.Promise.t(unit);
};
