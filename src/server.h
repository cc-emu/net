#pragma once

#include <napi.h>
#include "event_emitter.h"
#include "connection.h"
#include "thread_pool.h"
#include <RakPeerInterface.h>
#include <RakNetworkFactory.h>
#include <MessageIdentifiers.h>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

struct ServerOptions {
    std::vector<uint16_t> ports;
    uint16_t maxConnections;
    uint8_t workerThreads;
    std::string password;
};

class Server : public Napi::ObjectWrap<Server> {
public:
    Server(const Napi::CallbackInfo& info);
    ~Server();
    
    void Shutdown();
    
    static Napi::Function GetClass(Napi::Env env);
    
private:
    Napi::Value ShutdownWrapper(const Napi::CallbackInfo& info);
    Napi::Value OnWrapper(const Napi::CallbackInfo& info);
    Napi::Value OnceWrapper(const Napi::CallbackInfo& info);
    Napi::Value OffWrapper(const Napi::CallbackInfo& info);
    Napi::Value GetConnectionsWrapper(const Napi::CallbackInfo& info);
    Napi::Value GetPortsWrapper(const Napi::CallbackInfo& info);
    
    void Start();
    void PollThreadFunc();
    void ProcessPacket(Packet* packet);
    void HandleNewConnection(Packet* packet);
    void HandleDisconnection(Packet* packet);
    void HandleDataPacket(Packet* packet);
    
    RakPeerInterface* peer_;
    ServerOptions options_;
    
    std::thread pollThread_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::atomic<bool> running_;
    
    std::unordered_map<uint64_t, Connection*> guidToClient_;
    std::unordered_set<Connection*> connections_;
    std::mutex connectionsMutex_;
    
    EventEmitter eventEmitter_;
    Napi::Reference<Napi::Array> connectionsSet_;
    
    static std::atomic<uint64_t> connectionCounter_;
};
