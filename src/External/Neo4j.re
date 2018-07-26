/* https://neo4j.com/docs/api/javascript-driver/1.6/ */
/* const neo4j = require('neo4j-driver').v1;

   const driver = neo4j.driver(uri, neo4j.auth.basic(user, password));
   const session = driver.session();

   const personName = 'Alice';
   const resultPromise = session.run(
     'CREATE (a:Person {name: $name}) RETURN a',
     {name: personName}
   );

   resultPromise.then(result => {
     session.close();

     const singleRecord = result.records[0];
     const node = singleRecord.get(0);

     console.log(node.properties.name);

     // on application exit:
     driver.close();
   }); */

module Auth = {
  type t;

  [@bs.module "neo4j-driver"] [@bs.scope ("v1", "auth")]
  external makeBasic : (string, string) => t = "basic";
};

module Driver = {
  type t;

  [@bs.module "neo4j-driver"] [@bs.scope "v1"]
  external make : (string, Auth.t) => t = "driver";

  [@bs.send] external close : t => unit = "";
};

module Session = {
  type t;

  [@bs.send] external make : Driver.t => t = "session";
  [@bs.send]
  external run : (t, string, 'params) => Js.Promise.t(Js.Json.t) = "";
  [@bs.send] external close : t => unit = "";
};
