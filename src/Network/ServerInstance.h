#pragma once

#include "Network/HostInstance.h"
#include "Network/Serializable.h"
#include "Core/Collections/Queue_type.h"

namespace Network {

class ServerInstance : public HostInstance {
protected:
    Array<ENetPeer*> peers;
public:
    ServerInstance();
    virtual ~ServerInstance();

    bool Initialize(int port);
    void Tick();

    void RequestStop();

    void Broadcast(Network::Serializable *object);
};

}; // namespace Network
