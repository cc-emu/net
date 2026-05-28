const fs = require("node:fs");
const _path = require("node:path");

const platform = process.argv[2];
const version = (process.argv[3] || "").replace(/^v/, "");

const os = platform === "win32-x64" ? "win32" : platform.startsWith("darwin") ? "darwin" : "linux";
const cpu = platform === "darwin-arm64" ? "arm64" : "x64";

const pkg = {
  name: `@cc-emu/net-${platform}`,
  version,
  os: [os],
  cpu: [cpu],
  files: ["ccemu-net.node"],
  publishConfig: { registry: "https://npm.pkg.github.com" },
};

fs.mkdirSync("pkg", { recursive: true });
fs.writeFileSync("pkg/package.json", JSON.stringify(pkg, null, 2));
console.log(`Created pkg/package.json for ${pkg.name}@${version}`);
