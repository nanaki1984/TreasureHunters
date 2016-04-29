#pragma once

#include "Core/RefCounted.h"
#include "Network/NetStream.h"

#define DeclareSerializable \
public: \
    virtual void Serialize(ENetPeer *peer, Core::IO::BitStream &stream); \
    virtual void Deserialize(ENetPeer *peer, Core::IO::BitStream &stream);

#define DefineSerializable(type) \
void type::Serialize(ENetPeer *peer, Core::IO::BitStream &stream) \
{ \
    Network::NetWriteStream writeStream(peer, stream); \
    stream << type::RTTI.GetFCC(); \
    this->SerializeImpl(writeStream); \
} \
 \
void type::Deserialize(ENetPeer *peer, Core::IO::BitStream &stream) \
{ \
    Network::NetReadStream readStream(peer, stream); \
    uint32_t fcc; \
    stream >> fcc; \
    assert(fcc == type::RTTI.GetFCC()); \
    this->SerializeImpl(readStream); \
}

namespace Network {

class Serializable : public Core::RefCounted {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    { }
public:
    Serializable();
    Serializable(const Serializable &other) = delete;
    virtual ~Serializable();

    Serializable& operator =(const Serializable &other) = delete;
};

} // namespace Network
