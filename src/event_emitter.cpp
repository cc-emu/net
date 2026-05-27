#include "event_emitter.h"

EventEmitter::EventEmitter() {
}

void EventEmitter::on(const std::string& event, Napi::Function handler) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    HandlerEntry entry;
    entry.ref = Napi::Persistent(handler);
    entry.once = false;
    handlers_[event].push_back(std::move(entry));
}

void EventEmitter::once(const std::string& event, Napi::Function handler) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    HandlerEntry entry;
    entry.ref = Napi::Persistent(handler);
    entry.once = true;
    handlers_[event].push_back(std::move(entry));
}

void EventEmitter::off(const std::string& event, Napi::Function handler) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    auto it = handlers_.find(event);
    if (it == handlers_.end()) return;
    
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [&handler](const HandlerEntry& entry) {
            return entry.ref.Value() == handler;
        }), vec.end());
    
    if (vec.empty()) {
        handlers_.erase(it);
    }
}

void EventEmitter::emit(const std::string& event, const std::vector<Napi::Value>& args) {
    std::vector<Napi::FunctionReference> toCall;
    std::vector<size_t> toRemove;
    
    {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        auto it = handlers_.find(event);
        if (it == handlers_.end()) return;
        
        for (size_t i = 0; i < it->second.size(); i++) {
            toCall.push_back(Napi::Persistent(it->second[i].ref.Value()));
            if (it->second[i].once) {
                toRemove.push_back(i);
            }
        }
        
        // Remove once handlers in reverse order
        for (auto itRem = toRemove.rbegin(); itRem != toRemove.rend(); ++itRem) {
            it->second.erase(it->second.begin() + *itRem);
        }
        
        if (it->second.empty()) {
            handlers_.erase(it);
        }
    }
    
    // Convert args to napi_value array
    std::vector<napi_value> napiArgs;
    for (const auto& arg : args) {
        napiArgs.push_back(arg);
    }
    
    // Call handlers
    for (auto& ref : toCall) {
        ref.Call(napiArgs);
    }
}

void EventEmitter::RemoveAllListeners(const std::string& event) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    handlers_.erase(event);
}
