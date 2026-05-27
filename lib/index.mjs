import { createRequire } from "node:module";
const require = createRequire(import.meta.url);

const net = require("./index.js");

export const Server = net.Server;
export const Connection = net.Connection;
