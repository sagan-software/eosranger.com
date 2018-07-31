module Arango = {
  module Cursor = {
    type t;

    [@bs.send]
    external next : t => Js.Promise.t(Js.Nullable.t(Js.Json.t)) = "";

    [@bs.send] external all : t => Js.Promise.t(array(Js.Json.t)) = "";

    [@bs.send]
    external reduce :
      (t, ('accu, Js.Json.t) => 'accu, 'accu) => Js.Promise.t('accu) =
      "";
  };

  module Collection = {
    type t;

    [@bs.send] external create : t => Js.Promise.t(Js.Json.t) = "";
    [@bs.send]
    external document : (t, string) => Js.Promise.t(Js.Json.t) = "";
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
    external saveWithOpts :
      (t, Js.Json.t, saveOpts) => Js.Promise.t(Js.Json.t) =
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
    external queryWithVars : (t, string, Js.Json.t) => Js.Promise.t(Cursor.t) =
      "query";
  };
};

let setupDatabase = (client, dbName) =>
  client
  |. Arango.Db.useDatabase(dbName)
  |. Arango.Db.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Js.Promise.resolve();
       } else {
         client
         |. Arango.Db.createDatabase(dbName)
         |> Js.Promise.then_(_ => Js.Promise.resolve());
       }
     );

let setupCollection = (client, collectionName) => {
  let collection = client |. Arango.Db.collection(collectionName);
  collection
  |. Arango.Collection.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Js.Promise.resolve();
       } else {
         collection
         |. Arango.Collection.create
         |> Js.Promise.then_(_ => Js.Promise.resolve());
       }
     );
};

let setup = (client, dbName, ()) =>
  client
  |. setupDatabase(dbName)
  |> Js.Promise.then_(_ => client |. setupCollection("blocks"));

let blockKey = (b: Eosr_Store.block) => {
  let chainId = b.chainId |. Eos.ChainId.toString;
  let blockNum = b.blockNum |. Eos.BlockNum.toInt |. string_of_int;
  chainId ++ "_" ++ blockNum;
};

let encodeBlock = (b: Eosr_Store.block) =>
  Json.Encode.(
    [
      ("_key", b |. blockKey |. string),
      ("chain_id", b.chainId |. Eos.ChainId.encode),
      ("block_id", b.blockId |. Eos.BlockId.encode),
      ("block_num", b.blockNum |. Eos.BlockNum.encode),
      ("irreversible", b.irreversible |. bool),
      ("data", b.data),
    ]
    |. object_
  );

let save = (client, b: Eosr_Store.block) => {
  let query = {j|
    UPSERT { _key: @key }
    INSERT @block
    UPDATE @block
    IN blocks|j};
  let vars =
    [
      ("key", b |. blockKey |. Json.Encode.string),
      ("block", b |. encodeBlock),
    ]
    |. Json.Encode.object_;
  client
  |. Arango.Db.queryWithVars(query, vars)
  |> Js.Promise.then_(_ => Js.Promise.resolve());
};

let count = (client, _chainId) =>
  "RETURN LENGTH(blocks)"
  |> Arango.Db.query(client)
  |> Js.Promise.then_(Arango.Cursor.next)
  |> Js.Promise.then_(json =>
       json
       |. Js.Nullable.toOption
       |. Belt.Option.map(Json.Decode.int)
       |. Belt.Option.getWithDefault(0)
       |. Js.Promise.resolve
     );

let scan = (client, chainId, ~start, ~limit) => {
  let query = {j|
    FOR b IN blocks
    FILTER b.chain_id == @chain_id
    FILTER b.block_num >= @start
    SORT b.block_num ASC
    LIMIT @limit
    RETURN b.block_num|j};
  let vars =
    [
      ("chain_id", chainId |. Eos.ChainId.encode),
      ("start", start |. Eos.BlockNum.encode),
      ("limit", limit |. Json.Encode.int),
    ]
    |. Json.Encode.object_;
  client
  |. Arango.Db.queryWithVars(query, vars)
  |> Js.Promise.then_(Arango.Cursor.all)
  |> Js.Promise.then_(json =>
       json |. Belt.Array.map(Eos.BlockNum.decode) |. Js.Promise.resolve
     );
};

module type Args = {let db: string; let user: string; let pass: string;};

module Make = (Args: Args) : Eosr_Store.Store => {
  let client =
    Arango.Db.make() |. Arango.Db.useBasicAuth(Args.user, Args.pass);
  let setup = client |. setup(Args.db);
  let count = client |. count;
  let save = client |. save;
  let scan = client |. scan;
};