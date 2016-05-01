#include "Network/GameRoomData.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/ScratchAllocator.h"

using namespace Core::Memory;
using namespace Core::Collections;

namespace Network {

DefineClassInfo(Network::GameRoomData, Core::RefCounted);

GameRoomData::GameRoomData()
: playersData(GetAllocator<MallocAllocator>())
{ }

GameRoomData::GameRoomData(const GameRoomData &other)
: playersData(other.playersData)
{ }

GameRoomData::GameRoomData(GameRoomData &&other)
: playersData(std::forward<Array<Game::Player::NetData>>(other.playersData))
{ }

GameRoomData::~GameRoomData()
{ }

GameRoomData&
GameRoomData::operator =(const GameRoomData &other)
{
    playersData = other.playersData;
    return (*this);
}

GameRoomData&
GameRoomData::operator =(GameRoomData &&other)
{
    playersData = std::forward<Array<Game::Player::NetData>>(other.playersData);
    return (*this);
}

}; // namespace Network
