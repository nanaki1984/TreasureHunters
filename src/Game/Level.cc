#include "Game/Level.h"
#include "Core/SmartPtr.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"

using namespace Core::Memory;

namespace Game {

DefineClassInfo(Game::Level, Core::RefCounted);

Level::Level(const SmartPtr<Network::GameRoomData> &roomData)
: players(GetAllocator<MallocAllocator>(), roomData->playersData.Count())
{
    for (uint8_t id = 0, count = roomData->playersData.Count(); id < count; ++id)
        players.PushBack(SmartPtr<Player>::MakeNew<BlocksAllocator>(Player::Cloned, roomData->playersData[id]));
}

Level::~Level()
{ }

} // namespace Game
