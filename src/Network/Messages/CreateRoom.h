#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class CreateRoom : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(roomId);
        stream.Serialize(playersCount);
    }
public:
    uint32_t roomId;
    uint8_t playersCount;

    CreateRoom();
    CreateRoom(const CreateRoom &other) = delete;
    virtual ~CreateRoom();

    CreateRoom& operator =(const CreateRoom &other) = delete;
};

    } // namespace Messages
} // namespace Network
