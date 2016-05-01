#pragma once

#include "Network/HostInstance.h"
#include "Network/Serializable.h"
#include "Network/GameRoom.h"
#include "Core/Pool/Pool_type.h"

namespace Network {

class ServerInstance : public HostInstance {
protected:
    Core::Pool::Pool<GameRoom> rooms;
public:
    ServerInstance();
    virtual ~ServerInstance();

    bool Initialize(int port);
    void Tick();

    void RequestStop();

    void Send(ENetPeer *peer, const SmartPtr<Serializable> &object, MessageType messageType, uint8_t channel);
    void Broadcast(const Array<ENetPeer*> &peers, const SmartPtr<Serializable> &object, MessageType messageType, uint8_t channel);

    static ServerInstance* Instance();
};

inline ServerInstance*
ServerInstance::Instance()
{
    return static_cast<ServerInstance*>(HostInstance::instance);
}

}; // namespace Network
