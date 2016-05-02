#include "Network/GameRoomData.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/ScratchAllocator.h"

using namespace Core::Memory;
using namespace Core::Collections;

namespace Network {

DefineClassInfo(Network::GameRoomData, Core::RefCounted);

GameRoomData::GameRoomData()
: playersData(GetAllocator<MallocAllocator>()),
  enemiesData(GetAllocator<MallocAllocator>())
{ }

GameRoomData::~GameRoomData()
{ }

}; // namespace Network
