#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class EnemyState : public Network::Serializable {
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

    EnemyState();
    EnemyState(const EnemyState &other) = delete;
    virtual ~EnemyState();

    EnemyState& operator =(const EnemyState &other) = delete;
};

    } // namespace Messages
} // namespace Network
