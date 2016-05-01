#pragma once

#include "Core/Collections/Array_type.h"
#include "Game/Player.h"

namespace Network {

class GameRoomData : public Core::RefCounted {
    DeclareClassInfo;
public:
    Core::Collections::Array<Game::Player::NetData> playersData;
    // ToDo: enemies data

    GameRoomData();
    GameRoomData(const GameRoomData &other);
    GameRoomData(GameRoomData &&other);
    virtual ~GameRoomData();

    GameRoomData& operator =(const GameRoomData &other);
    GameRoomData& operator =(GameRoomData &&other);
};

}; // namespace Network
