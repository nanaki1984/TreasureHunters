#pragma once

#include "Network/HostInstance.h"
#include "Network/Serializable.h"
#include "Core/Collections/Queue_type.h"

namespace Network {

class ClientInstance : public HostInstance {
protected:
    ENetPeer *server;
    Queue<SmartPtr<Serializable>> sendQueue;

    bool active;
public:
    ClientInstance();
    virtual ~ClientInstance();

    bool Initialize(const char *serverHost, int serverPort);
    void Tick();

    void RequestPause();
    void RequestResume();
    void RequestQuit();

    void Send(const SmartPtr<Serializable> &object);
    
    static ClientInstance* Instance();
};

inline ClientInstance*
ClientInstance::Instance()
{
	return static_cast<ClientInstance*>(HostInstance::instance);
}

}; // namespace Network
