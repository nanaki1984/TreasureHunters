#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class PlayerState : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(id);
        stream.Serialize(t);
        stream.Serialize(x);
        stream.Serialize(y);
    }
public:
    uint8_t id;
    float t, x, y;

    PlayerState();
    PlayerState(const PlayerState &other) = delete;
    virtual ~PlayerState();

    PlayerState& operator =(const PlayerState &other) = delete;
};

    } // namespace Messages
} // namespace Network
