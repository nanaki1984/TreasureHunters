#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/Pool/BaseObject.h"
#include "Core/Collections/Array_type.h"
#include "Network/Messages/StartGame.h"
#include "Network/Messages/PlayerInputs.h"
#include "Game/Player.h"

namespace Network {

class GameRoom : public Core::Pool::BaseObject {
    DeclareClassInfo;
public:
    enum State
    {
        WaitingJoin,
        WaitingPlayers,
        Playing
    };
protected:
    float lifeTime;
    State state;

    Core::Collections::Array<ENetPeer*> peers;
    Core::Collections::Array<SmartPtr<Messages::StartGame>> startGameMsgs;

    float lastTimestamp;
    float accumulator, simTime;
    Core::Collections::Array<SmartPtr<Game::Player>> players;
public:
    const float kFixedStepTime = 0.05f;

    GameRoom(uint8_t playersCount);
    virtual ~GameRoom();

    State GetState() const;

    void AddPlayer(ENetPeer *peer);
    bool PlayerReady(ENetPeer *peer, const SmartPtr<Messages::StartGame> &startGame);
    bool PlayerLeft(ENetPeer *peer);

    void RecvPlayerInputs(ENetPeer *peer, const SmartPtr<Messages::PlayerInputs> &playerInputs);

    bool Update();
};

inline GameRoom::State
GameRoom::GetState() const
{
    return state;
}

}; // namespace Network
