#pragma once

#include "Core/SmartPtr.h"
#include "Network/Serializable.h"
#include "Network/GameRoomData.h"
#include "Core/Memory/MallocAllocator.h"

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
        if (Success == flags)
        {
            if (Stream::IsWriting)
                stream.SerializeArray(roomData->playersData);
            else
            {
                roomData = SmartPtr<GameRoomData>::MakeNew<Core::Memory::MallocAllocator>();
                stream.SerializeArray(roomData->playersData);
            }
        }
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
    SmartPtr<GameRoomData> roomData;

    JoinRoom();
    JoinRoom(const JoinRoom &other) = delete;
    virtual ~JoinRoom();

    JoinRoom& operator =(const JoinRoom &other) = delete;
};

    } // namespace Messages
} // namespace Network
