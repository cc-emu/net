#include "packet_registry.h"
#include <cstring>

bool PacketRegistry::Serialize(RakNet::BitStream& bs, uint8_t packetId, const std::string& event,
                               const std::map<std::string, Napi::Value>& fields, Napi::Env env) {
    bs.Write(packetId);
    bs.Write(RakNet::RakString(event.c_str()));
    
    uint8_t kvCount = static_cast<uint8_t>(fields.size());
    bs.Write(kvCount);
    
    for (const auto& [key, value] : fields) {
        bs.Write(RakNet::RakString(key.c_str()));
        
        ValueType type = DetectType(value);
        bs.Write(static_cast<uint8_t>(type));
        
        if (!WriteValue(bs, type, value)) {
            return false;
        }
    }
    
    return true;
}

bool PacketRegistry::Deserialize(RakNet::BitStream& bs, PacketData& out, Napi::Env env) {
    if (!bs.Read(out.packetId)) return false;
    
    RakNet::RakString eventStr;
    if (!bs.Read(eventStr)) return false;
    out.event = eventStr.C_String();
    
    uint8_t kvCount;
    if (!bs.Read(kvCount)) return false;
    
    for (uint8_t i = 0; i < kvCount; i++) {
        RakNet::RakString keyStr;
        if (!bs.Read(keyStr)) return false;
        
        uint8_t typeId;
        if (!bs.Read(typeId)) return false;
        
        Napi::Value value;
        if (!ReadValue(bs, static_cast<ValueType>(typeId), value, env)) {
            return false;
        }
        
        out.fields[std::string(keyStr.C_String())] = value;
    }
    
    return true;
}

ValueType PacketRegistry::DetectType(const Napi::Value& value) {
    if (value.IsBoolean()) return ValueType::Bool;
    if (value.IsBigInt()) {
        auto bi = value.As<Napi::BigInt>();
        bool lossless;
        uint64_t u64 = bi.Uint64Value(&lossless);
        if (lossless && u64 <= UINT32_MAX) return ValueType::UInt32;
        return ValueType::UInt64;
    }
    if (value.IsNumber()) {
        double d = value.As<Napi::Number>().DoubleValue();
        if (d == static_cast<int64_t>(d)) {
            if (d >= 0) {
                if (d <= UINT8_MAX) return ValueType::UInt8;
                if (d <= UINT16_MAX) return ValueType::UInt16;
                if (d <= UINT32_MAX) return ValueType::UInt32;
                return ValueType::UInt64;
            } else {
                if (d >= INT8_MIN) return ValueType::Int8;
                if (d >= INT16_MIN) return ValueType::Int16;
                if (d >= INT32_MIN) return ValueType::Int32;
                return ValueType::Int64;
            }
        }
        return ValueType::Double;
    }
    if (value.IsString()) return ValueType::String;
    if (value.IsArrayBuffer()) return ValueType::Binary;
    return ValueType::String; // fallback
}

bool PacketRegistry::WriteValue(RakNet::BitStream& bs, ValueType type, const Napi::Value& value) {
    switch (type) {
        case ValueType::Bool:
            bs.Write(value.As<Napi::Boolean>().Value());
            break;
        case ValueType::UInt8:
            bs.Write(value.As<Napi::Number>().Uint32Value() & 0xFF);
            break;
        case ValueType::UInt16:
            bs.Write(static_cast<uint16_t>(value.As<Napi::Number>().Uint32Value()));
            break;
        case ValueType::UInt32:
            bs.Write(value.As<Napi::Number>().Uint32Value());
            break;
        case ValueType::UInt64: {
            auto bi = value.As<Napi::BigInt>();
            bool lossless;
            uint64_t v = bi.Uint64Value(&lossless);
            bs.Write(v);
            break;
        }
        case ValueType::Int8:
            bs.Write(static_cast<int8_t>(value.As<Napi::Number>().Int32Value()));
            break;
        case ValueType::Int16:
            bs.Write(static_cast<int16_t>(value.As<Napi::Number>().Int32Value()));
            break;
        case ValueType::Int32:
            bs.Write(value.As<Napi::Number>().Int32Value());
            break;
        case ValueType::Int64: {
            auto bi = value.As<Napi::BigInt>();
            bool lossless;
            int64_t v = bi.Int64Value(&lossless);
            bs.Write(v);
            break;
        }
        case ValueType::Float:
            bs.Write(value.As<Napi::Number>().FloatValue());
            break;
        case ValueType::Double:
            bs.Write(value.As<Napi::Number>().DoubleValue());
            break;
        case ValueType::String: {
            std::string str = value.As<Napi::String>().Utf8Value();
            bs.Write(RakNet::RakString(str.c_str()));
            break;
        }
        case ValueType::Binary: {
            auto ab = value.As<Napi::ArrayBuffer>();
            uint32_t len = static_cast<uint32_t>(ab.ByteLength());
            bs.Write(len);
            bs.Write(static_cast<const char*>(ab.Data()), len);
            break;
        }
    }
    return true;
}

bool PacketRegistry::ReadValue(RakNet::BitStream& bs, ValueType type, Napi::Value& out, Napi::Env env) {
    switch (type) {
        case ValueType::Bool: {
            bool v;
            if (!bs.Read(v)) return false;
            out = Napi::Boolean::New(env, v);
            break;
        }
        case ValueType::UInt8: {
            uint8_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::UInt16: {
            uint16_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::UInt32: {
            uint32_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::UInt64: {
            uint64_t v;
            if (!bs.Read(v)) return false;
            out = Napi::BigInt::New(env, v);
            break;
        }
        case ValueType::Int8: {
            int8_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::Int16: {
            int16_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::Int32: {
            int32_t v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::Int64: {
            int64_t v;
            if (!bs.Read(v)) return false;
            out = Napi::BigInt::New(env, v);
            break;
        }
        case ValueType::Float: {
            float v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::Double: {
            double v;
            if (!bs.Read(v)) return false;
            out = Napi::Number::New(env, v);
            break;
        }
        case ValueType::String: {
            RakNet::RakString str;
            if (!bs.Read(str)) return false;
            out = Napi::String::New(env, str.C_String());
            break;
        }
        case ValueType::Binary: {
            uint32_t len;
            if (!bs.Read(len)) return false;
            
            if (len == 0) {
                out = Napi::ArrayBuffer::New(env, 0);
                break;
            }
            
            // For zero-copy on large buffers
            if (len > 64) {
                char* data = new char[len];
                bs.Read(data, len);
                out = Napi::ArrayBuffer::New(env, data, len, 
                    [](Napi::Env, void* finalizeData) {
                        delete[] static_cast<char*>(finalizeData);
                    });
            } else {
                Napi::ArrayBuffer ab = Napi::ArrayBuffer::New(env, len);
                bs.Read(static_cast<char*>(ab.Data()), len);
                out = ab;
            }
            break;
        }
    }
    return true;
}
