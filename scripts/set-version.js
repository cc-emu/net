const fs = require("node:fs");
const path = require("node:path");

const version = (process.argv[2] || "").replace(/^v/, "");
if (!version) {
  console.error("Usage: node set-version.js <version>");
  process.exit(1);
}

const packageJsonPath = path.join(__dirname, "..", "package.json");
const pkg = JSON.parse(fs.readFileSync(packageJsonPath, "utf8"));

pkg.version = version;

fs.writeFileSync(packageJsonPath, `${JSON.stringify(pkg, null, 2)}\n`);
console.log(`Set version to ${version}`);
