#pragma once

#include "Network/Serializable.h"

namespace Network {
    namespace Messages {

class StartGame : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(roomId);
        stream.Serialize(playerId);
        stream.Serialize(flags);
        stream.SerializeTimestamp(goTime);
    }
public:
    static const uint8_t kUnknownId = 0xff;

    enum Flags
    {
        Ready = 0,
        Go,
        Fail
    };

    uint32_t roomId;
    uint8_t playerId;
    Flags flags;
    float goTime;

    StartGame();
    StartGame(const StartGame &other) = delete;
    virtual ~StartGame();

    StartGame& operator =(const StartGame &other) = delete;
};

    } // namespace Messages
} // namespace Network
