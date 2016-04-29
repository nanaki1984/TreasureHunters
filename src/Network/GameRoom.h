#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/Pool/BaseObject.h"
#include "Core/Collections/Array_type.h"
#include "Network/Messages/StartGame.h"

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
    State state;

    Core::Collections::Array<ENetPeer*> peers;
    Core::Collections::Array<SmartPtr<Messages::StartGame>> startGameMsgs;

    float lastTimestamp;
public:
    GameRoom(uint8_t playersCount);
    virtual ~GameRoom();

    State GetState() const;

    void AddPlayer(ENetPeer *peer);
    bool PlayerReady(ENetPeer *peer, const SmartPtr<Messages::StartGame> &startGame);
    void PlayerLeft(ENetPeer *peer);

    void Update(float time);
};

inline GameRoom::State
GameRoom::GetState() const
{
    return state;
}

}; // namespace Network
