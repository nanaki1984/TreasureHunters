#include "Network/GameRoom.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"

using namespace Core::Memory;

namespace Network {

DefineClassInfo(Network::GameRoom, Core::Pool::BaseObject);

GameRoom::GameRoom()
: peers(GetAllocator<MallocAllocator>())
{ }

GameRoom::~GameRoom()
{ }

}; // namespace Network
