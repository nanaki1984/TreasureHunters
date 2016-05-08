#include "Network/GameRoom.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Memory/ScratchAllocator.h"
#include "Core/SmartPtr.h"
#include "Core/Time/TimeServer.h"
#include "Network/ServerInstance.h"
#include "Network/Messages/PlayerState.h"
#include "Network/Messages/EnemyState.h"

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
  simStep(0),
  data(SmartPtr<GameRoomData>::MakeNew<MallocAllocator>())
{
    data->playersData.Resize(playersCount);
    data->enemiesData.Resize(1);
    auto &enemyData = data->enemiesData[0];
    enemyData.p0x = -10.0f;
    enemyData.p0y = 10.0f;
    enemyData.p1x = 10.0f;
    enemyData.p1y = 10.0f;
    enemyData.p2x = 0.0f;
    enemyData.p2y = -10.0f;
}

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

    level.Reset();
    data.Reset();
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
                }

                startGameMsgs.Clear();
                startGameMsgs.Trim();

                accumulator = simTime = .0f;
                simStep = 0;
                lastTimestamp = goTime;

                level = SmartPtr<Game::Level>::MakeNew<BlocksAllocator>();
                level->Init(data);

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
            level->DeletePlayer(playerId);
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
        level->GetPlayer(playerId)->SendPlayerInput(playerInputs);
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
    simTime += dt;
    while (accumulator >= kServerFixedTime)
    {
        //simTime += kServerFixedTime;

        simStep += kStepsCount;
        level->Update(simStep);

        auto it = level->PlayersBegin(), end = level->PlayersEnd();
        uint8_t playerId = 0;
        for (; it != end; ++it, ++playerId)
        {
            if (!(*it)->HasChanged())
                continue;

            auto playerState = SmartPtr<Messages::PlayerState>::MakeNew<ScratchAllocator>();
            playerState->id = playerId;

            (*it)->FillPlayerState(playerState);

            ServerInstance::Instance()->Broadcast(peers, SmartPtr<Serializable>::CastFrom(playerState), HostInstance::Unsequenced, 0);
        }

        auto it2 = level->EnemiesBegin(), end2 = level->EnemiesEnd();
        uint8_t enemyId = 0;
        for (; it2 != end2; ++it2, ++enemyId)
        {
            auto enemyState = SmartPtr<Messages::EnemyState>::MakeNew<ScratchAllocator>();
            enemyState->id = enemyId;
            enemyState->step = simStep;
            (*it2)->GetCurrentPosition(&enemyState->x, &enemyState->y);

            ServerInstance::Instance()->Broadcast(peers, SmartPtr<Serializable>::CastFrom(enemyState), HostInstance::Unsequenced, 0);
        }

        accumulator -= kServerFixedTime;
    }

    return false;
}

}; // namespace Network
