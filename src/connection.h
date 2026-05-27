#pragma once

#include <napi.h>
#include "event_emitter.h"
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <string>
#include <atomic>

class Connection : public Napi::ObjectWrap<Connection> {
public:
    Connection(const Napi::CallbackInfo& info);
    
    void Initialize(const std::string& id, const std::string& address, uint16_t port,
                    RakNetGUID guid, RakPeerInterface* peer);
    
    void Send(const std::string& event, const std::map<std::string, Napi::Value>& data, Napi::Env env);
    void Disconnect(bool sendNotification = true);
    
    std::string GetId() const { return connectionId_; }
    std::string GetAddress() const { return address_; }
    uint16_t GetPort() const { return port_; }
    RakNetGUID GetGuid() const { return guid_; }
    EventEmitter& GetEventEmitter() { return eventEmitter_; }
    
    static Napi::Function GetClass(Napi::Env env);
    
private:
    Napi::Value SendWrapper(const Napi::CallbackInfo& info);
    Napi::Value DisconnectWrapper(const Napi::CallbackInfo& info);
    Napi::Value OnWrapper(const Napi::CallbackInfo& info);
    Napi::Value OnceWrapper(const Napi::CallbackInfo& info);
    Napi::Value OffWrapper(const Napi::CallbackInfo& info);
    Napi::Value GetIdWrapper(const Napi::CallbackInfo& info);
    Napi::Value GetAddressWrapper(const Napi::CallbackInfo& info);
    Napi::Value GetPortWrapper(const Napi::CallbackInfo& info);
    
    std::string connectionId_;
    std::string address_;
    uint16_t port_;
    RakNetGUID guid_;
    RakPeerInterface* peer_;
    EventEmitter eventEmitter_;
    
    static std::atomic<uint64_t> connectionCounter_;
};
