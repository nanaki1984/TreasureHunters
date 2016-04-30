#pragma once

#include "Network/HostInstance.h"
#include "Network/Serializable.h"
#include "Core/Collections/Queue_type.h"
#include "Game/Player.h"

namespace Network {

class ClientInstance : public HostInstance {
public:
    typedef void (*RoomCreationCallback)(uint32_t);
    typedef void (*JoinRoomCallback)(bool); // ToDo: room data
    typedef void (*StartGameCallback)(bool);

    enum State {
        Disconnected = 0,

        Connected,
        JoinedRoom,
        Waiting,
        Playing
    };
protected:
    ENetPeer *server;
    Queue<SmartPtr<Serializable>> sendQueue;
    Queue<MessageType> sendQueueMsgType;

    bool active;
    State state;

    RoomCreationCallback roomCreationCallback;
    JoinRoomCallback joinRoomCallback;
    StartGameCallback startGameCallback;

    uint32_t roomId;
    uint8_t playerId;
    float lastTimestamp;
    float accumulator, simTime;
    SmartPtr<Game::Player> player;
public:
    static const float kFixedStepTime;

    ClientInstance();
    virtual ~ClientInstance();

    bool Initialize(const char *serverHost, int serverPort);
    void Tick();

    void RequestPause();
    void RequestResume();
    void RequestQuit();

    void CreateRoom(uint8_t playersCount, RoomCreationCallback callback);
    void JoinRoom(uint32_t roomId, JoinRoomCallback callback);
    void StartGame(StartGameCallback callback);
    void Send(const SmartPtr<Serializable> &object, MessageType messageType);

    State GetState() const;
    uint8_t GetRoomId() const;
    uint8_t GetPlayerId() const;
    float GetRTT() const;

    void SendPlayerInputs(float x, float y);
    void GetPlayerPosition(float *x, float *y);

    static ClientInstance* Instance();
};

inline ClientInstance::State
ClientInstance::GetState() const
{
    return state;
}

inline uint8_t
ClientInstance::GetRoomId() const
{
    assert(state >= JoinedRoom);
    return roomId;
}

inline uint8_t
ClientInstance::GetPlayerId() const
{
    assert(Playing == state);
    return playerId;
}

inline float
ClientInstance::GetRTT() const
{
    assert(state > Disconnected);
    return server->roundTripTime * 0.001f;
}

inline ClientInstance*
ClientInstance::Instance()
{
	return static_cast<ClientInstance*>(HostInstance::instance);
}

}; // namespace Network
