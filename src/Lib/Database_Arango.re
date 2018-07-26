let makeClient = (~db, ~user, ~pass) =>
  Arango.Db.make()
  |. Arango.Db.useBasicAuth(user, pass)
  |. Arango.Db.useDatabase(db);

let createDatabase = (client, db) =>
  client
  |. Arango.Db.createDatabase(db)
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

let ensureDatabase = (client, name) =>
  client
  |. Arango.Db.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Log.info(__MODULE__, "Database exists:", name);
         Js.Promise.resolve();
       } else {
         Log.info(__MODULE__, "Attempting to create database:", name);
         createDatabase(client, name);
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

let ensureCollection = (client, collectionName) => {
  let collection = client |. Arango.Db.collection(collectionName);
  collection
  |. Arango.Collection.exists
  |> Js.Promise.then_(exists =>
       if (exists) {
         Log.info(__MODULE__, "Collection exists:", collectionName);
         Js.Promise.resolve(client);
       } else {
         Log.info(
           __MODULE__,
           "Attempting to create collection:",
           collectionName,
         );
         createCollection(collection)
         |> Js.Promise.then_(_ => Js.Promise.resolve(client));
       }
     );
};

let setup = (client, dbName) =>
  ensureDatabase(client, dbName)
  |> Js.Promise.then_(_ =>
       ensureCollection(client, "blocks") |. ignore |. Js.Promise.resolve
     );

let largestBlockNum = client =>
  {j|
FOR block IN blocks
SORT block.block_num DESC
LIMIT 1
RETURN block.block_num
|j}
  |> Arango.Db.query(client)
  |> Js.Promise.then_(Arango.Cursor.next)
  |> Js.Promise.then_(json =>
       json
       |. Js.Nullable.toOption
       |. Belt.Option.map(Json.Decode.optional(Json.Decode.int))
       |. Belt.Option.getWithDefault(None)
       |. Belt.Option.getWithDefault(0)
       |. Js.Promise.resolve
     );

let saveBlock = (client, json) => {
  [%bs.raw {|json._key = json.block_num + ""|}] |. ignore;
  /* Util.doNothing(isIrreversible);
     [%bs.raw {|json._key = json.block_num + ""|}] |. ignore;
     [%bs.raw {|json.irreversible = isIrreversible|}] |. ignore; */
  let saveOpts = Arango.Collection.saveOpts(~overwrite=true, ());
  client
  |. Arango.Db.collection("blocks")
  |. Arango.Collection.saveWithOpts(json, saveOpts);
};

let count = client =>
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

let rec findMissing = (client, push, ~lastBlockNum, ~limit=1000, ()) => {
  let query = {j|FOR b IN blocks
  FILTER @lastBlockNum < b.block_num
  SORT b.block_num ASC
  LIMIT @limit
  RETURN b.block_num|j};
  let vars =
    [
      ("lastBlockNum", Json.Encode.int(lastBlockNum)),
      ("limit", Json.Encode.int(limit)),
    ]
    |. Json.Encode.object_;
  client
  |. Arango.Db.queryWithVars(query, vars)
  |> Js.Promise.then_(Arango.Cursor.all)
  |> Js.Promise.then_(blockNums =>
       if (blockNums |. Belt.Array.length > 0) {
         blockNums
         |. Belt.Array.reduce(
              Js.Promise.resolve(lastBlockNum), (promise, json) =>
              promise
              |> Js.Promise.then_(lastBlockNum => {
                   let blockNum = json |. Json.Decode.int;
                   let diff = blockNum - lastBlockNum;
                   if (diff > 1) {
                     let i = ref(1);
                     let missing = [||];
                     while (i^ < diff) {
                       missing |> Js.Array.push(lastBlockNum + i^) |. ignore;
                       i := i^ + 1;
                     };
                     let numMissing = missing |. Belt.Array.length;
                     Log.info(
                       __MODULE__,
                       {j|Found $numMissing missing block(s). First three:|j},
                       missing |. Belt.Array.slice(~offset=0, ~len=3),
                     );
                     push(missing)
                     |> Js.Promise.then_(_ => Js.Promise.resolve(blockNum));
                   } else {
                     Js.Promise.resolve(blockNum);
                   };
                 })
            )
         |> Js.Promise.then_(lastBlockNum =>
              findMissing(client, push, ~lastBlockNum, ())
            );
       } else {
         Js.Promise.resolve();
       }
     );
};

module type Info = {let db: string; let user: string; let pass: string;};

module Make = (Info: Info, Q: Types.Q) : Types.Db => {
  let client = makeClient(~db=Info.db, ~user=Info.user, ~pass=Info.pass);
  let setup = () => setup(client, Info.db);
  let count = () => count(client);
  let save = saveBlock(client);
  let largestBlockNum = () => largestBlockNum(client);
  let findMissing = () =>
    findMissing(client, Q.push, ~lastBlockNum=0, ())
    |> Js.Promise.then_(_ => {
         Log.info(__MODULE__, "Done finding missing blocks.", "");
         Js.Promise.resolve();
       });
};
