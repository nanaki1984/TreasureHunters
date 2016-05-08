#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class PlayerInputs : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(step);
        stream.Serialize(x);
        stream.Serialize(y);
        stream.Serialize(attack);
    }
public:
    uint32_t step;
    float x, y;
    bool attack;

    PlayerInputs();
    PlayerInputs(const PlayerInputs &other) = delete;
    virtual ~PlayerInputs();

    PlayerInputs& operator =(const PlayerInputs &other) = delete;
};

    } // namespace Messages
} // namespace Network
