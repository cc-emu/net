import { EventEmitter } from "node:events";

// Platform detection
function getRuntimePlatform(): string {
  const platform = `${process.platform}-${process.arch}`;
  if (platform.startsWith("linux")) {
    try {
      const { isNonGlibcLinuxSync } = require("detect-libc");
      if (isNonGlibcLinuxSync()) {
        return platform.replace("linux", "linuxmusl");
      }
    } catch {
      /* ignore */
    }
  }
  return platform;
}

// Load native addon
const runtimePlatform = getRuntimePlatform();
const paths = [`../build/Release/ccemu-net.node`, `@cc-emu/net-${runtimePlatform}/ccemu-net.node`];

let binding: any;
const errors: Error[] = [];
for (const p of paths) {
  try {
    binding = require(p);
    break;
  } catch (err: any) {
    if (err.code !== "MODULE_NOT_FOUND") {
      errors.push(err);
    }
  }
}

if (!binding) {
  const supported = ["darwin-x64", "darwin-arm64", "linux-x64", "linuxmusl-x64", "win32-x64"];
  throw new Error(
    `@cc-emu/net: No prebuilt binary found for ${runtimePlatform}.\n` +
      `Supported platforms: ${supported.join(", ")}.\n` +
      `Errors:\n${errors.map((e) => `  - ${e.message}`).join("\n")}`,
  );
}

export interface Packet {
  packetId: number;
  event: string;
  [key: string]: any;
}

export interface DisconnectReason {
  reason: "notification" | "lost" | "kicked";
}

export interface MessageEvent {
  client: Connection;
  packet: Packet;
}

export interface ServerOptions {
  port: number | number[];
  maxConnections: number;
  workerThreads?: number;
  password?: string;
}

export class Connection extends EventEmitter {
  private native_: any;

  constructor(native: any) {
    super();
    this.native_ = native;

    // Bridge events
    this.native_.on("receive", (packet: Packet) => {
      this.emit("receive", packet);
    });
  }

  get id(): string {
    return this.native_.id;
  }
  get address(): string {
    return this.native_.address;
  }
  get port(): number {
    return this.native_.port;
  }

  send(event: string, data: Record<string, any>): void {
    this.native_.send(event, data);
  }

  disconnect(sendNotification?: boolean): void {
    this.native_.disconnect(sendNotification);
  }

  on(event: "receive", handler: (packet: Packet) => void): this {
    super.on(event, handler);
    return this;
  }

  once(event: "receive", handler: (packet: Packet) => void): this {
    super.once(event, handler);
    return this;
  }

  off(event: "receive", handler?: (packet: Packet) => void): this {
    if (handler) {
      super.off(event, handler);
    } else {
      super.removeAllListeners(event);
    }
    return this;
  }
}

export class Server extends EventEmitter {
  private native_: any;
  private connections_: Set<Connection> = new Set();
  private ports_: number[];

  constructor(options: ServerOptions) {
    super();
    this.native_ = new binding.Server(options);
    this.ports_ = Array.isArray(options.port) ? options.port : [options.port];

    // Bridge native events
    this.native_.on("connect", (nativeClient: any) => {
      const client = new Connection(nativeClient);
      this.connections_.add(client);
      this.emit("connect", client);
    });

    this.native_.on("disconnect", (nativeClient: any, reason: DisconnectReason) => {
      const client = this.findClient(nativeClient);
      if (client) {
        this.connections_.delete(client);
        this.emit("disconnect", client, reason);
      }
    });

    this.native_.on("message", (event: MessageEvent) => {
      this.emit("message", event);
    });

    this.native_.on("error", (error: Error) => {
      this.emit("error", error);
    });
  }

  get connections(): Set<Connection> {
    return this.connections_;
  }

  get ports(): number[] {
    return this.ports_;
  }

  shutdown(): void {
    this.native_.shutdown();
    this.connections_.clear();
  }

  on(event: "connect", handler: (client: Connection) => void): this;
  on(event: "disconnect", handler: (client: Connection, reason: DisconnectReason) => void): this;
  on(event: "message", handler: (event: MessageEvent) => void): this;
  on(event: "error", handler: (error: Error) => void): this;
  on(event: string, handler: (...args: any[]) => void): this {
    super.on(event, handler);
    return this;
  }

  private findClient(native: any): Connection | undefined {
    for (const client of this.connections_) {
      if ((client as any).native_ === native) {
        return client;
      }
    }
    return undefined;
  }
}
