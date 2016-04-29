#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class JoinRoom : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(roomId);
        stream.Serialize(flags);
    }
public:
    enum Flags
    {
        Request = 0,
        Success,
        Fail
    };

    uint32_t roomId;
    Flags flags;

    // ToDo: room data

    JoinRoom();
    JoinRoom(const JoinRoom &other) = delete;
    virtual ~JoinRoom();

    JoinRoom& operator =(const JoinRoom &other) = delete;
};

    } // namespace Messages
} // namespace Network
