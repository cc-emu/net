#include "server.h"
#include "packet_registry.h"
#include <BitStream.h>
#include <RakNetTypes.h>
#include <RakNetworkFactory.h>

std::atomic<uint64_t> Server::connectionCounter_{0};

Server::Server(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<Server>(info), peer_(nullptr), running_(false) {
    
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected options object").ThrowAsJavaScriptException();
        return;
    }
    
    Napi::Object options = info[0].As<Napi::Object>();
    
    // Parse ports
    if (options.Has("port")) {
        Napi::Value portVal = options.Get("port");
        if (portVal.IsNumber()) {
            options_.ports.push_back(portVal.As<Napi::Number>().Uint32Value());
        } else if (portVal.IsArray()) {
            Napi::Array ports = portVal.As<Napi::Array>();
            for (uint32_t i = 0; i < ports.Length(); i++) {
                options_.ports.push_back(ports.Get(i).As<Napi::Number>().Uint32Value());
            }
        }
    } else {
        options_.ports.push_back(7777); // default
    }
    
    // Parse maxConnections
    options_.maxConnections = options.Has("maxConnections") 
        ? options.Get("maxConnections").As<Napi::Number>().Uint32Value() 
        : 100;
    
    // Parse workerThreads
    options_.workerThreads = options.Has("workerThreads")
        ? options.Get("workerThreads").As<Napi::Number>().Uint32Value()
        : 4;
    
    // Parse password
    if (options.Has("password")) {
        options_.password = options.Get("password").As<Napi::String>().Utf8Value();
    }
    
    // Initialize connections Array
    connectionsSet_ = Napi::Persistent(Napi::Array::New(env));
    
    Start();
}

Server::~Server() {
    Shutdown();
}

void Server::Start() {
    peer_ = RakNetworkFactory::GetRakPeerInterface();
    
    std::vector<SocketDescriptor> sockets;
    for (auto port : options_.ports) {
        sockets.emplace_back(port, static_cast<const char*>(nullptr));
    }
    
    bool result = peer_->Startup(
        options_.maxConnections, 
        10,
        &sockets[0], 
        static_cast<unsigned int>(sockets.size())
    );
    
    if (!result) {
        throw std::runtime_error("Failed to start RakNet peer");
    }
    
    if (!options_.password.empty()) {
        peer_->SetIncomingPassword(options_.password.c_str(), 
                                   static_cast<int>(options_.password.length()));
    }
    
    peer_->SetMaximumIncomingConnections(options_.maxConnections);
    
    running_ = true;
    threadPool_ = std::make_unique<ThreadPool>(options_.workerThreads);
    pollThread_ = std::thread(&Server::PollThreadFunc, this);
}

void Server::PollThreadFunc() {
    while (running_) {
        Packet* packet = peer_->Receive();
        if (packet) {
            // Process directly in poll thread for now
            // TODO: Move deserialization to worker threads using ThreadSafeFunction
            ProcessPacket(packet);
            peer_->DeallocatePacket(packet);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Server::ProcessPacket(Packet* packet) {
    switch (packet->data[0]) {
        case ID_NEW_INCOMING_CONNECTION:
            HandleNewConnection(packet);
            break;
        case ID_DISCONNECTION_NOTIFICATION:
        case ID_CONNECTION_LOST:
            HandleDisconnection(packet);
            break;
        default:
            HandleDataPacket(packet);
            break;
    }
}

void Server::HandleNewConnection(Packet* packet) {
    uint64_t guid = packet->guid.g;
    std::string addr = peer_->GetSystemAddressFromGuid(packet->guid).ToString(false);
    uint16_t port = packet->systemAddress.port;
    
    std::string connId = "conn_" + std::to_string(connectionCounter_.fetch_add(1));
    
    // Create Connection - this would need to be created in JS context
    // For now, we'll emit an event that JS can use to track connections
    
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        // Connection* client = new Connection(...);
        // guidToClient_[guid] = client;
        // connections_.insert(client);
    }
    
    // Emit connect event
    // eventEmitter_.emit("connect", {client});
}

void Server::HandleDisconnection(Packet* packet) {
    uint64_t guid = packet->guid.g;
    
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        auto it = guidToClient_.find(guid);
        if (it != guidToClient_.end()) {
            Connection* client = it->second;
            connections_.erase(client);
            guidToClient_.erase(it);
            
            // Emit disconnect event
            // eventEmitter_.emit("disconnect", {client, reason});
            
            delete client;
        }
    }
}

void Server::HandleDataPacket(Packet* packet) {
    // TODO: Implement using ThreadSafeFunction for cross-thread JS event emission
    // For now, this is a placeholder that will be completed once ThreadSafeFunction
    // is properly integrated
    
    uint64_t guid = packet->guid.g;
    
    Connection* client = nullptr;
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        auto it = guidToClient_.find(guid);
        if (it != guidToClient_.end()) {
            client = it->second;
        }
    }
    
    if (!client) return;
    
    // Packet data will be processed via ThreadSafeFunction in a future update
    // For now, packets are received but not deserialized/emitted to JS
}

void Server::Shutdown() {
    running_ = false;
    
    if (pollThread_.joinable()) {
        pollThread_.join();
    }
    
    if (threadPool_) {
        threadPool_->Shutdown();
    }
    
    if (peer_) {
        // Disconnect all clients
        peer_->Shutdown(300);
        RakNetworkFactory::DestroyRakPeerInterface(peer_);
        peer_ = nullptr;
    }
    
    // Cleanup remaining connections
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (auto client : connections_) {
            delete client;
        }
        connections_.clear();
        guidToClient_.clear();
    }
}

Napi::Function Server::GetClass(Napi::Env env) {
    Napi::Function func = DefineClass(env, "Server", {
        InstanceMethod("shutdown", &Server::ShutdownWrapper),
        InstanceMethod("on", &Server::OnWrapper),
        InstanceMethod("once", &Server::OnceWrapper),
        InstanceMethod("off", &Server::OffWrapper),
        InstanceAccessor("connections", &Server::GetConnectionsWrapper, nullptr),
        InstanceAccessor("ports", &Server::GetPortsWrapper, nullptr),
    });
    return func;
}

Napi::Value Server::ShutdownWrapper(const Napi::CallbackInfo& info) {
    Shutdown();
    return info.Env().Undefined();
}

Napi::Value Server::OnWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
        Napi::TypeError::New(env, "Expected (event, handler)").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string event = info[0].As<Napi::String>().Utf8Value();
    Napi::Function handler = info[1].As<Napi::Function>();
    
    eventEmitter_.on(event, handler);
    return env.Undefined();
}

Napi::Value Server::OnceWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
        Napi::TypeError::New(env, "Expected (event, handler)").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string event = info[0].As<Napi::String>().Utf8Value();
    Napi::Function handler = info[1].As<Napi::Function>();
    
    eventEmitter_.once(event, handler);
    return env.Undefined();
}

Napi::Value Server::OffWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected (event, [handler])").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string event = info[0].As<Napi::String>().Utf8Value();
    
    if (info.Length() >= 2 && info[1].IsFunction()) {
        Napi::Function handler = info[1].As<Napi::Function>();
        eventEmitter_.off(event, handler);
    } else {
        eventEmitter_.RemoveAllListeners(event);
    }
    
    return env.Undefined();
}

Napi::Value Server::GetConnectionsWrapper(const Napi::CallbackInfo& info) {
    return connectionsSet_.Value();
}

Napi::Value Server::GetPortsWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Array arr = Napi::Array::New(env, options_.ports.size());
    for (size_t i = 0; i < options_.ports.size(); i++) {
        arr.Set(i, Napi::Number::New(env, options_.ports[i]));
    }
    return arr;
}
