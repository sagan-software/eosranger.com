module type Info = {let uri: string; let db: string;};

module Make = (Info: Info, Q: Types.Q) : Types.Db => {
  let collection = ref(None);
  let setup = () =>
    Info.uri
    |. Mongo.Client.make
    |> Js.Promise.then_(client => {
         collection :=
           client
           |. Mongo.Client.db(Info.db)
           |. Mongo.Database.collection("blocks")
           |. Some;
         Js.Promise.resolve();
       });
  let count = () =>
    switch (collection^) {
    | Some(collection) =>
      collection |. Mongo.Collection.findAll |. Mongo.Cursor.count
    | None => Js.Promise.resolve(0)
    };
  let save = block =>
    switch (collection^) {
    | Some(collection) =>
      [%bs.raw {|block._id = block.block_num|}] |. ignore;
      collection
      |. Mongo.Collection.save(block)
      |> Js.Promise.then_(_ => Js.Promise.resolve(Js.Json.boolean(true)));
    | None => Js.Promise.resolve(Js.Json.boolean(false))
    };

  let largestBlockNum = () =>
    switch (collection^) {
    | Some(collection) =>
      collection
      |. Mongo.Collection.findAll
      |. Mongo.Cursor.sort("block_num", -1)
      |. Mongo.Cursor.next
      |> Js.Promise.then_(block =>
           switch (block |. Js.Nullable.toOption) {
           | Some(block) =>
             block
             |> Json.Decode.field("block_num", Json.Decode.int)
             |. Js.Promise.resolve
           | None => Js.Promise.resolve(0)
           }
         )
    | None => Js.Promise.resolve(0)
    };

  let findMissing = () => Js.Promise.resolve();
  /* let largestBlockNum = () => largestBlockNum(client);
     let findMissing = () =>
       findMissing(client, Q.push, ~lastBlockNum=0)
       |> Js.Promise.then_(_ => {
            Log.info(__MODULE__, "Done finding missing blocks.", "");
            Js.Promise.resolve();

       }); */
};
