#pragma once

#include "Core/Collections/Array_type.h"
#include "Game/Player.h"
#include "Game/Enemy.h"

namespace Network {

class GameRoomData : public Core::RefCounted {
    DeclareClassInfo;
public:
    Core::Collections::Array<Game::Player::NetData> playersData;
    Core::Collections::Array<Game::Enemy::NetData> enemiesData;

    GameRoomData();
    GameRoomData(const GameRoomData &other) = delete;
    virtual ~GameRoomData();

    GameRoomData& operator =(const GameRoomData &other) = delete;
};

}; // namespace Network
