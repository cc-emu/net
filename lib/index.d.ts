import { EventEmitter } from "node:events";
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
export declare class Connection extends EventEmitter {
    private native_;
    constructor(native: any);
    get id(): string;
    get address(): string;
    get port(): number;
    send(event: string, data: Record<string, any>): void;
    disconnect(sendNotification?: boolean): void;
    on(event: "receive", handler: (packet: Packet) => void): this;
    once(event: "receive", handler: (packet: Packet) => void): this;
    off(event: "receive", handler?: (packet: Packet) => void): this;
}
export declare class Server extends EventEmitter {
    private native_;
    private connections_;
    private ports_;
    constructor(options: ServerOptions);
    get connections(): Set<Connection>;
    get ports(): number[];
    shutdown(): void;
    on(event: "connect", handler: (client: Connection) => void): this;
    on(event: "disconnect", handler: (client: Connection, reason: DisconnectReason) => void): this;
    on(event: "message", handler: (event: MessageEvent) => void): this;
    on(event: "error", handler: (error: Error) => void): this;
    private findClient;
}
