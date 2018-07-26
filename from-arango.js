const fs = require("fs");
const JSONStream = require("JSONStream");
const es = require("event-stream");
// const mongodb = require("mongodb");

const readStream = fs.createReadStream(
	"/home/sagan/eos-ranger-backup/blocks_e734964953f880e5164e32827950ff92.data.json",
);

// mongodb.MongoClient.connect("mongodb://localhost:27017").then(client => {
// 	const db = client.db("EOSMainNet");
// 	const blocks = db.collection("blocks");
// 	const parser = JSONStream.parse("data");
// 	const logger = es.mapSync(function(data) {
// 		delete data._id;
// 		delete data._key;
// 		delete data._rev;
// 		data._id = data.block_num;
// 		// console.log(data.block_num);
// 		blocks.save(data).then(() => console.log("Saved block", data.block_num));
// 		return data;
// 	});

// 	readStream.pipe(parser);
// 	parser.pipe(logger);
// });

const neo4j = require("neo4j-driver").v1;
const driver = neo4j.driver(
	"bolt://localhost",
	neo4j.auth.basic("neo4j", "changeme"),
);
const session = driver.session();

session
	.run(
		`
create constraint on (b:Block) assert b.block_num is unique;
create constraint on (b:Block) assert b.id is unique;
`,
	)
	.then(result => console.log("constraint created", result));

const parser = JSONStream.parse("data");
const logger = es.mapSync(function(data) {
	delete data._id;
	delete data._key;
	delete data._rev;
	// data._id = data.block_num;
	data.irreversible = true;
	// console.log(data.block_num);
	session
		.run("CREATE (b:Block $block) RETURN b", { block: data })
		.then(result => console.log("block created", data.block_num));
	return data;
});

readStream.pipe(parser);
parser.pipe(logger);
