module Cursor = {
  type t;

  [@bs.send]
  external next : t => Js.Promise.t(Js.Nullable.t(Js.Json.t)) = "";
};

module Collection = {
  type t;

  [@bs.send] external create : t => Js.Promise.t(Js.Json.t) = "";
  [@bs.send] external document : (t, string) => Js.Promise.t(Js.Json.t) = "";
  [@bs.send] external save : (t, Js.Json.t) => Js.Promise.t(Js.Json.t) = "";

  [@bs.deriving abstract]
  type saveOpts = {
    [@bs.optional]
    waitForSync: bool,
    [@bs.optional]
    returnNew: bool,
    [@bs.optional]
    returnOld: bool,
    [@bs.optional]
    silent: bool,
    [@bs.optional]
    overwrite: bool,
  };

  [@bs.send]
  external saveWithOpts : (t, Js.Json.t, saveOpts) => Js.Promise.t(Js.Json.t) =
    "save";

  [@bs.send] external exists : t => Js.Promise.t(bool) = "";
};

module Db = {
  type t;

  [@bs.deriving abstract]
  type config = {
    [@bs.optional]
    url: string,
    [@bs.optional]
    urlArray: array(string),
    [@bs.optional]
    isAbsolute: bool,
    [@bs.optional]
    arangoVersion: int,
  };

  [@bs.module "arangojs"] [@bs.new] external make : unit => t = "Database";

  [@bs.module "arangojs"] [@bs.new]
  external makeWithConfig : config => t = "Database";

  [@bs.send] external useDatabase : (t, string) => t = "";

  [@bs.send] external useBasicAuth : (t, string, string) => t = "";
  [@bs.send] external useBearerAuth : (t, string) => t = "";

  [@bs.send]
  external createDatabase : (t, string) => Js.Promise.t(Js.Json.t) = "";

  [@bs.send] external exists : t => Js.Promise.t(bool) = "";

  [@bs.send] external close : t => unit = "";

  [@bs.send] external collection : (t, string) => Collection.t = "";
  [@bs.send] external edgeCollection : (t, string) => Collection.t = "";

  [@bs.send] external query : (t, string) => Js.Promise.t(Cursor.t) = "";

  [@bs.send]
  external queryVars : (t, string, Js.Json.t) => Js.Promise.t(Cursor.t) =
    "query";
};
