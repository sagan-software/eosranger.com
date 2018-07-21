const glob = require("glob");
const path = require("path");

const isProd = process.env.NODE_ENV === "production";

console.warn("Production build?", (isProd + "").toUpperCase());

function camelCaseToDash(str) {
	return str
		.replace(/[^a-zA-Z0-9]+/g, "-")
		.replace(/([A-Z]+)([A-Z][a-z])/g, "$1-$2")
		.replace(/([a-z])([A-Z])/g, "$1-$2")
		.replace(/([0-9])([^0-9])/g, "$1-$2")
		.replace(/([^0-9])([0-9])/g, "$1-$2")
		.replace(/-+/g, "-")
		.toLowerCase();
}

const entry = glob.sync("./src/Scripts/*.js").reduce((result, entryPath) => {
	let basename = path.basename(entryPath, ".js");
	let filename = camelCaseToDash(basename);
	result[filename] = entryPath;
	return result;
}, {});

console.log("Entry chunks:", entry);

module.exports = {
	target: "node",
	mode: isProd ? "production" : "development",
	entry: entry,
	output: {
		path: path.resolve(__dirname, "scripts"),
		filename: "[name].js",
	},
};
