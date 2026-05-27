#pragma once

#include <napi.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>

class EventEmitter {
public:
    EventEmitter();
    
    void on(const std::string& event, Napi::Function handler);
    void once(const std::string& event, Napi::Function handler);
    void off(const std::string& event, Napi::Function handler);
    void emit(const std::string& event, const std::vector<Napi::Value>& args);
    void RemoveAllListeners(const std::string& event);
    
private:
    struct HandlerEntry {
        Napi::FunctionReference ref;
        bool once;
    };
    
    std::map<std::string, std::vector<HandlerEntry>> handlers_;
    std::mutex handlersMutex_;
};
