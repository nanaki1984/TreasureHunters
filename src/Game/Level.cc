#include "Game/Level.h"
#include "Core/SmartPtr.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"

using namespace Core::Memory;

namespace Game {

DefineClassInfo(Game::Level, Core::RefCounted);

Level::Level()
: players(GetAllocator<MallocAllocator>())
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
}

void
Level::DeletePlayer(uint8_t playerId)
{
    players.RemoveAt(playerId);
}

void
Level::Update(float simTime)
{
    auto it = players.Begin(), end = players.End();
    for (; it != end; ++it)
        (*it)->Update(simTime);
}

} // namespace Game
