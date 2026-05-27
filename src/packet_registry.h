#pragma once

#include <BitStream.h>
#include <RakString.h>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <napi.h>

enum class ValueType : uint8_t {
    Bool = 0,
    UInt8 = 1,
    UInt16 = 2,
    UInt32 = 3,
    UInt64 = 4,
    Int8 = 5,
    Int16 = 6,
    Int32 = 7,
    Int64 = 8,
    Float = 9,
    Double = 10,
    String = 11,
    Binary = 12
};

struct PacketData {
    uint8_t packetId;
    std::string event;
    std::map<std::string, Napi::Value> fields;
};

class PacketRegistry {
public:
    static bool Serialize(RakNet::BitStream& bs, uint8_t packetId, const std::string& event, 
                          const std::map<std::string, Napi::Value>& fields, Napi::Env env);
    static bool Deserialize(RakNet::BitStream& bs, PacketData& out, Napi::Env env);
    
private:
    static bool WriteValue(RakNet::BitStream& bs, ValueType type, const Napi::Value& value);
    static bool ReadValue(RakNet::BitStream& bs, ValueType type, Napi::Value& out, Napi::Env env);
    static ValueType DetectType(const Napi::Value& value);
};
