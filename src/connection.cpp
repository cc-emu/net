#include "connection.h"
#include "packet_registry.h"
#include <BitStream.h>

std::atomic<uint64_t> Connection::connectionCounter_{0};

Connection::Connection(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<Connection>(info), peer_(nullptr) {
}

void Connection::Initialize(const std::string& id, const std::string& address, uint16_t port,
                              RakNetGUID guid, RakPeerInterface* peer) {
    connectionId_ = id;
    address_ = address;
    port_ = port;
    guid_ = guid;
    peer_ = peer;
}

void Connection::Send(const std::string& event, const std::map<std::string, Napi::Value>& data, 
                        Napi::Env env) {
    if (!peer_) return;
    
    RakNet::BitStream bs;
    PacketRegistry::Serialize(bs, 0, event, data, env);
    
    peer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, guid_, false);
}

void Connection::Disconnect(bool sendNotification) {
    if (!peer_) return;
    
    SystemAddress addr = peer_->GetSystemAddressFromGuid(guid_);
    peer_->CloseConnection(addr, sendNotification);
}

Napi::Function Connection::GetClass(Napi::Env env) {
    Napi::Function func = DefineClass(env, "Connection", {
        InstanceMethod("send", &Connection::SendWrapper),
        InstanceMethod("disconnect", &Connection::DisconnectWrapper),
        InstanceMethod("on", &Connection::OnWrapper),
        InstanceMethod("once", &Connection::OnceWrapper),
        InstanceMethod("off", &Connection::OffWrapper),
        InstanceAccessor("id", &Connection::GetIdWrapper, nullptr),
        InstanceAccessor("address", &Connection::GetAddressWrapper, nullptr),
        InstanceAccessor("port", &Connection::GetPortWrapper, nullptr),
    });
    return func;
}

Napi::Value Connection::SendWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsObject()) {
        Napi::TypeError::New(env, "Expected (event, data)").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string event = info[0].As<Napi::String>().Utf8Value();
    Napi::Object dataObj = info[1].As<Napi::Object>();
    
    std::map<std::string, Napi::Value> data;
    Napi::Array keys = dataObj.GetPropertyNames();
    for (uint32_t i = 0; i < keys.Length(); i++) {
        std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
        data[key] = dataObj.Get(key);
    }
    
    Send(event, data, env);
    return env.Undefined();
}

Napi::Value Connection::DisconnectWrapper(const Napi::CallbackInfo& info) {
    bool sendNotification = true;
    if (info.Length() > 0 && info[0].IsBoolean()) {
        sendNotification = info[0].As<Napi::Boolean>().Value();
    }
    
    Disconnect(sendNotification);
    return info.Env().Undefined();
}

Napi::Value Connection::OnWrapper(const Napi::CallbackInfo& info) {
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

Napi::Value Connection::OnceWrapper(const Napi::CallbackInfo& info) {
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

Napi::Value Connection::OffWrapper(const Napi::CallbackInfo& info) {
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

Napi::Value Connection::GetIdWrapper(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), connectionId_);
}

Napi::Value Connection::GetAddressWrapper(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), address_);
}

Napi::Value Connection::GetPortWrapper(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), port_);
}
