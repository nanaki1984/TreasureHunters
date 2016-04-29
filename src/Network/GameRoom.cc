#include "Network/GameRoom.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/SmartPtr.h"
#include "Core/Collections/Array.h"
#include "Core/Time/TimeServer.h"
#include "Network/ServerInstance.h"

using namespace Core::Memory;

namespace Network {

DefineClassInfo(Network::GameRoom, Core::Pool::BaseObject);

GameRoom::GameRoom(uint8_t playersCount)
: state(WaitingJoin),
  peers(GetAllocator<MallocAllocator>(), playersCount),
  startGameMsgs(GetAllocator<MallocAllocator>(), playersCount)
{ }

GameRoom::~GameRoom()
{ }

void
GameRoom::AddPlayer(ENetPeer *peer)
{
    assert(WaitingJoin == state && peers.Count() < peers.Capacity());
    peers.PushBack(peer);
    if (peers.Count() == peers.Capacity())
        state = WaitingPlayers;
}

bool
GameRoom::PlayerReady(ENetPeer *peer, const SmartPtr<Messages::StartGame> &startGame)
{
    assert(state != Playing);
    auto it = peers.Begin(), end = peers.End();
    for (; it != end; ++it)
    {
        if (*it == peer)
        {
            startGame->playerId = it - peers.Begin();

            auto it2 = startGameMsgs.Begin(), end2 = startGameMsgs.End();
            for (; it2 != end2; ++it2)
            {
                if ((*it2)->playerId == startGame->playerId)
                    return false;
            }

            startGameMsgs.PushBack(startGame);
            if (startGameMsgs.Count() == startGameMsgs.Capacity())
            {
                float goTime = Core::Time::TimeServer::Instance()->GetRealTime();
                enet_uint32 maxRTT = 0;
                for (it = peers.Begin(); it != end; ++it)
                    maxRTT = std::max(maxRTT, (*it)->roundTripTime);
                goTime += (float)(maxRTT >> 1);

                it2 = startGameMsgs.Begin();
                for (; it2 != end2; ++it2)
                {
                    (*it2)->goTime = goTime;
                    ServerInstance::Instance()->Send(peers[(*it2)->playerId], SmartPtr<Network::Serializable>::CastFrom(*it2), ServerInstance::ReliableSequenced);
                }

                startGameMsgs.Clear();
                startGameMsgs.Trim();

                lastTimestamp = goTime;

                state = Playing;
            }

            return true;
        }
    }
    return false;
}

void
GameRoom::PlayerLeft(ENetPeer *peer)
{
    // ToDo
}

}; // namespace Network
