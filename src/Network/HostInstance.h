#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/SmartPtr.h"
#include "Core/Log.h"
#include "Core/Collections/Array_type.h"
#include "Core/Time/TimeServer.h"
#include "Core/StringsTable.h"
#include "Managers/BaseManager.h"

namespace Network {

class HostInstance {
protected:
    static HostInstance *instance;

    ENetHost *host;

    SmartPtr<Core::Log> log;
    SmartPtr<Core::Time::TimeServer> timeServer;
    SmartPtr<Core::StringsTable> stringsTable;

    Array<SmartPtr<Managers::BaseManager>> managers;

    bool StartServer(int port);
    bool Connect(const char *serverHost, int serverPort);
    void Stop();
public:
    HostInstance();
    virtual ~HostInstance();

    const SmartPtr<Managers::BaseManager>& GetManager(const Core::ClassInfo *classInfo);

    static HostInstance* Instance();
};

inline HostInstance*
HostInstance::Instance()
{
    return HostInstance::instance;
}

}; // namespace Network
