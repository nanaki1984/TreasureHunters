#include "Game/Level.h"
#include "Core/SmartPtr.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"

using namespace Core::Memory;

namespace Game {

DefineClassInfo(Game::Level, Core::RefCounted);

Level::Level()
: players(GetAllocator<MallocAllocator>()),
  enemies(GetAllocator<MallocAllocator>())
{ }

Level::~Level()
{ }

void
Level::Init(const SmartPtr<Network::GameRoomData> &roomData)
{
    uint8_t id = 0, count = roomData->playersData.Count();
    players.Reserve(count);
    for (; id < count; ++id)
        players.PushBack(SmartPtr<Player>::MakeNew<BlocksAllocator>(
            Player::SimulatedOnServer,
            roomData->playersData[id]));

    id = 0;
    count = roomData->enemiesData.Count();
    enemies.Reserve(count);
    for (; id < count; ++id)
        enemies.PushBack(SmartPtr<Enemy>::MakeNew<BlocksAllocator>(
            Enemy::SimulatedOnServer,
            roomData->enemiesData[id]));
}

void
Level::Init(const SmartPtr<Network::GameRoomData> &roomData, uint8_t clientPlayerId)
{
    uint8_t id = 0, count = roomData->playersData.Count();
    players.Reserve(count);
    for (; id < count; ++id)
        players.PushBack(SmartPtr<Player>::MakeNew<BlocksAllocator>(
            id == clientPlayerId ? Player::SimulatedLagless : Player::Cloned,
            roomData->playersData[id]));

    id = 0;
    count = roomData->enemiesData.Count();
    enemies.Reserve(count);
    for (; id < count; ++id)
        enemies.PushBack(SmartPtr<Enemy>::MakeNew<BlocksAllocator>(
            Enemy::Cloned,
            roomData->enemiesData[id]));
}

void
Level::DeletePlayer(uint8_t playerId)
{
    players.RemoveAt(playerId);
}

void
Level::Update(uint32_t simStep)
{
    auto plyIt = players.Begin(), plyEnd = players.End();
    for (; plyIt != plyEnd; ++plyIt)
        (*plyIt)->Update(simStep);

    auto enmIt = enemies.Begin(), enmEnd = enemies.End();
    for (; enmIt != enmEnd; ++enmIt)
        (*enmIt)->Update(simStep);
}

} // namespace Game
