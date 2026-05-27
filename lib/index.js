"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Server = exports.Connection = void 0;
const node_events_1 = require("node:events");
// Platform detection
function getRuntimePlatform() {
    const platform = `${process.platform}-${process.arch}`;
    if (platform.startsWith("linux")) {
        try {
            const { isNonGlibcLinuxSync } = require("detect-libc");
            if (isNonGlibcLinuxSync()) {
                return platform.replace("linux", "linuxmusl");
            }
        }
        catch { /* ignore */ }
    }
    return platform;
}
// Load native addon
const runtimePlatform = getRuntimePlatform();
const paths = [
    `../build/Release/ccemu-net.node`,
    `@cc-emu/net-${runtimePlatform}/ccemu-net.node`,
];
let binding;
const errors = [];
for (const p of paths) {
    try {
        binding = require(p);
        break;
    }
    catch (err) {
        if (err.code !== "MODULE_NOT_FOUND") {
            errors.push(err);
        }
    }
}
if (!binding) {
    const supported = [
        "darwin-x64", "darwin-arm64",
        "linux-x64", "linuxmusl-x64",
        "win32-x64",
    ];
    throw new Error(`@cc-emu/net: No prebuilt binary found for ${runtimePlatform}.\n` +
        `Supported platforms: ${supported.join(", ")}.\n` +
        `Errors:\n${errors.map(e => `  - ${e.message}`).join("\n")}`);
}
class Connection extends node_events_1.EventEmitter {
    native_;
    constructor(native) {
        super();
        this.native_ = native;
        // Bridge events
        this.native_.on("receive", (packet) => {
            this.emit("receive", packet);
        });
    }
    get id() {
        return this.native_.id;
    }
    get address() {
        return this.native_.address;
    }
    get port() {
        return this.native_.port;
    }
    send(event, data) {
        this.native_.send(event, data);
    }
    disconnect(sendNotification) {
        this.native_.disconnect(sendNotification);
    }
    on(event, handler) {
        super.on(event, handler);
        return this;
    }
    once(event, handler) {
        super.once(event, handler);
        return this;
    }
    off(event, handler) {
        if (handler) {
            super.off(event, handler);
        }
        else {
            super.removeAllListeners(event);
        }
        return this;
    }
}
exports.Connection = Connection;
class Server extends node_events_1.EventEmitter {
    native_;
    connections_ = new Set();
    ports_;
    constructor(options) {
        super();
        this.native_ = new binding.Server(options);
        this.ports_ = Array.isArray(options.port) ? options.port : [options.port];
        // Bridge native events
        this.native_.on("connect", (nativeClient) => {
            const client = new Connection(nativeClient);
            this.connections_.add(client);
            this.emit("connect", client);
        });
        this.native_.on("disconnect", (nativeClient, reason) => {
            const client = this.findClient(nativeClient);
            if (client) {
                this.connections_.delete(client);
                this.emit("disconnect", client, reason);
            }
        });
        this.native_.on("message", (event) => {
            this.emit("message", event);
        });
        this.native_.on("error", (error) => {
            this.emit("error", error);
        });
    }
    get connections() {
        return this.connections_;
    }
    get ports() {
        return this.ports_;
    }
    shutdown() {
        this.native_.shutdown();
        this.connections_.clear();
    }
    on(event, handler) {
        super.on(event, handler);
        return this;
    }
    findClient(native) {
        for (const client of this.connections_) {
            if (client.native_ === native) {
                return client;
            }
        }
        return undefined;
    }
}
exports.Server = Server;
