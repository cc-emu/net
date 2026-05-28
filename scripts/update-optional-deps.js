const fs = require("node:fs");
const path = require("node:path");

const version = process.argv[2];
if (!version) {
  console.error("Usage: node update-optional-deps.js <version>");
  process.exit(1);
}

const packageJsonPath = path.join(__dirname, "..", "package.json");
const pkg = JSON.parse(fs.readFileSync(packageJsonPath, "utf8"));

const platforms = ["darwin-x64", "darwin-arm64", "linux-x64", "linuxmusl-x64", "win32-x64"];

pkg.optionalDependencies = {};
for (const platform of platforms) {
  pkg.optionalDependencies[`@cc-emu/net-${platform}`] = version;
}

fs.writeFileSync(packageJsonPath, `${JSON.stringify(pkg, null, 2)}\n`);
console.log(`Updated optionalDependencies to version ${version}`);
