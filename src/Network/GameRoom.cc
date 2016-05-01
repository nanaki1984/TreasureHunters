#include "Network/GameRoom.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Memory/ScratchAllocator.h"
#include "Core/SmartPtr.h"
#include "Core/Collections/Array.h"
#include "Core/Time/TimeServer.h"
#include "Network/ServerInstance.h"
#include "Network/Messages/PlayerState.h"

using namespace Core::Memory;

namespace Network {

DefineClassInfo(Network::GameRoom, Core::Pool::BaseObject);

GameRoom::GameRoom(uint8_t playersCount)
: lifeTime(.0f),
  state(WaitingJoin),
  peers(GetAllocator<MallocAllocator>(), playersCount),
  startGameMsgs(GetAllocator<MallocAllocator>(), playersCount),
  lastTimestamp(.0f),
  accumulator(.0f), simTime(.0f),
  players(GetAllocator<MallocAllocator>(), playersCount)
{ }

GameRoom::~GameRoom()
{
    if (WaitingPlayers == state)
    {
        auto it = startGameMsgs.Begin(), end = startGameMsgs.End();
        for (; it != end; ++it)
        {
            (*it)->flags = Messages::StartGame::Fail;
            ServerInstance::Instance()->Send(peers[(*it)->playerId], SmartPtr<Network::Serializable>::CastFrom(*it), ServerInstance::ReliableSequenced, 1);
        }
    }
}

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
    if (Playing == state)
        return false;
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
                float goTime = Core::Time::TimeServer::Instance()->GetSeconds();
                enet_uint32 maxRTT = 0;
                for (it = peers.Begin(); it != end; ++it)
                    maxRTT = std::max(maxRTT, (*it)->roundTripTime);
                goTime += ((float)(maxRTT >> 1) * 0.0015f);

                for (it2 = startGameMsgs.Begin(), end2 = startGameMsgs.End(); it2 != end2; ++it2)
                {
                    (*it2)->flags = Messages::StartGame::Go;
                    (*it2)->goTime = goTime;

                    Core::Log::Instance()->Write(Core::Log::Info, "Starting game for player %d at time %f.", (*it2)->playerId, (*it2)->goTime);

                    ServerInstance::Instance()->Send(peers[(*it2)->playerId], SmartPtr<Network::Serializable>::CastFrom(*it2), ServerInstance::ReliableSequenced, 1);

                    players.PushBack(SmartPtr<Game::Player>::MakeNew<BlocksAllocator>(Game::Player::SimulatedOnServer, .0f, .0f));
                }

                startGameMsgs.Clear();
                startGameMsgs.Trim();

                accumulator = simTime = .0f;
                lastTimestamp = goTime;

                state = Playing;
            }

            return true;
        }
    }
    return false;
}

bool
GameRoom::PlayerLeft(ENetPeer *peer)
{
    int32_t playerId = peers.IndexOf(peer);
    if (playerId > -1)
    {
        peers.RemoveAt(playerId);

        switch (state)
        {
        case Network::GameRoom::WaitingJoin:
            return false;
        case Network::GameRoom::WaitingPlayers:
            {
                auto it = startGameMsgs.Begin(), end = startGameMsgs.End();
                for (; it != end; ++it)
                {
                    if ((*it)->playerId == playerId)
                    {
                        startGameMsgs.RemoveAt(it - startGameMsgs.Begin());
                        break;
                    }
                }
            }
            return true;
        case Network::GameRoom::Playing:
            players.RemoveAt(playerId);
            return 0 == peers.Count();
        }
    }
    return false;
}

void
GameRoom::RecvPlayerInputs(ENetPeer *peer, const SmartPtr<Messages::PlayerInputs> &playerInputs)
{
    int32_t playerId = peers.IndexOf(peer);
    if (playerId > -1)
        players[playerId]->SendInput(playerInputs->t, playerInputs->x, playerInputs->y);
}

bool
GameRoom::Update()
{
    lifeTime += Core::Time::TimeServer::Instance()->GetDeltaTime();
    if (WaitingJoin == state && 0 == peers.Count() && lifeTime > 5.0f)
        return true;

    if (state != Playing)
        return false;

    float newTimestamp = Core::Time::TimeServer::Instance()->GetSeconds();
    float dt = newTimestamp - lastTimestamp;
    if (dt <= .0f)
        return false;

    lastTimestamp = newTimestamp;

    accumulator += dt;
    while (accumulator >= kFixedStepTime)
    {
        simTime += kFixedStepTime;

        auto it = players.Begin(), end = players.End();
        for (; it != end; ++it)
        {
            (*it)->Update(simTime);

            auto playerState = SmartPtr<Messages::PlayerState>::MakeNew<ScratchAllocator>();
            playerState->t = simTime;
            playerState->x = (*it)->GetX();
            playerState->y = (*it)->GetY();

            ServerInstance::Instance()->Broadcast(peers, SmartPtr<Serializable>::CastFrom(playerState), HostInstance::Sequenced, 0);
        }

        accumulator -= kFixedStepTime;
    }

    return false;
}

}; // namespace Network
