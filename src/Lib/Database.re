let db =
  Arango.Db.make()
  |. Arango.Db.useBasicAuth(Env.dbUser, Env.dbPass)
  |. Arango.Db.useDatabase(Env.dbName);

let createDatabase = () =>
  db
  |. Arango.Db.createDatabase(Env.dbName)
  |> Js.Promise.then_(result =>
       Log.info(__MODULE__, "Created database:", result) |. Js.Promise.resolve
     )
  |> Js.Promise.catch(error => {
       Log.error(
         __MODULE__,
         "Exiting due to error creating database:",
         error,
       );
       Node.Process.exit(1) |. Js.Promise.resolve;
     });

let ensureDatabase = () =>
  db
  |. Arango.Db.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Log.info(__MODULE__, "Database exists:", Env.dbName);
         Js.Promise.resolve();
       } else {
         Log.info(__MODULE__, "Attempting to create database:", Env.dbName);
         createDatabase();
       }
     );

let createCollection = collection =>
  collection
  |. Arango.Collection.create
  |> Js.Promise.then_(result => {
       Log.info(__MODULE__, "Created collection:", result);
       Js.Promise.resolve(collection);
     })
  |> Js.Promise.catch(error => {
       Log.error(
         __MODULE__,
         "Exiting due to error creating collection:",
         error,
       );
       Node.Process.exit(1);
       Js.Promise.resolve(collection);
     });

let ensureCollection = collectionName => {
  let collection = db |. Arango.Db.collection(collectionName);
  collection
  |. Arango.Collection.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Log.info(__MODULE__, "Collection exists:", collectionName);
         Js.Promise.resolve(db);
       } else {
         Log.info(
           __MODULE__,
           "Attempting to create collection:",
           collectionName,
         );
         createCollection(collection)
         |> Js.Promise.then_(_ => Js.Promise.resolve(db));
       }
     );
};

let setup = () =>
  ensureDatabase() |> Js.Promise.then_(_ => ensureCollection("blocks"));

let getLargestBlockNum = () =>
  {j|
FOR block IN blocks
SORT block.block_num DESC
LIMIT 1
RETURN block.block_num
|j}
  |> Arango.Db.query(db)
  |> Js.Promise.then_(Arango.Cursor.next)
  |> Js.Promise.then_(json =>
       json
       |. Js.Nullable.toOption
       |. Belt.Option.map(Json.Decode.optional(Json.Decode.int))
       |. Belt.Option.getWithDefault(None)
       |. Belt.Option.getWithDefault(0)
       |. Js.Promise.resolve
     );

let saveBlock = (isIrreversible, json) => {
  Util.doNothing(isIrreversible);
  %bs.raw
  {|json._key = json.block_num + ""|};
  %bs.raw
  {|json.irreversible = isIrreversible|};
  let saveOpts = Arango.Collection.saveOpts(~overwrite=true, ());
  db
  |. Arango.Db.collection("blocks")
  |. Arango.Collection.saveWithOpts(json, saveOpts);
};
