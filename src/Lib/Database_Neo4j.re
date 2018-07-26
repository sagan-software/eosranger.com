module type Info = {let uri: string; let user: string; let pass: string;};

module Make = (I: Info) : Types.Db => {
  let auth = Neo4j.Auth.makeBasic(I.user, I.pass);
  let driver = Neo4j.Driver.make(I.uri, auth);
  let session = Neo4j.Session.make(driver);
  let setup = () => Js.Promise.resolve();
  let count = () => Js.Promise.resolve(0);
  /* session |. Neo4j.Session.run("MATCH (b:Block) RETURN COUNT(b)", ""); */
  let save = json =>
    session
    |. Neo4j.Session.run("CREATE (b:Block $block) RETURN b", {"block": json});
  let largestBlockNum = () => Js.Promise.resolve(0);
  let findMissing = () => Js.Promise.resolve();
};
