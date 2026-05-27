#include <napi.h>
#include "server.h"
#include "connection.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Export classes
    exports.Set("Server", Server::GetClass(env));
    exports.Set("Connection", Connection::GetClass(env));
    
    return exports;
}

NODE_API_MODULE(raknetcc, InitAll)
