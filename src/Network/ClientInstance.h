#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/SmartPtr.h"
#include "Core/Log.h"
#include "Core/Collections/Array_type.h"
#include "Core/Time/TimeServer.h"
#include "Core/StringsTable.h"
#include "Core/IO/FileServer.h"
#include "Core/Pool/Handle_type.h"
#include "Managers/BaseManager.h"
#include "Network/Serializable.h"
#include "Game/Player.h"

namespace Core {

class ClientInstance {
protected:
	static ClientInstance *instance;

    ENetHost *host;
    Array<ENetPeer*> peers;

    bool isServer;
    bool active;

    SmartPtr<Log> log;
    SmartPtr<Time::TimeServer> timeServer;
    SmartPtr<StringsTable> stringsTable;
    SmartPtr<IO::FileServer> fileServer;

    Collections::Array<SmartPtr<Managers::BaseManager>> managers;

    SmartPtr<Game::Player> player;
public:
	ClientInstance();
	~ClientInstance();

    bool Initialize(const char *serverHost, int serverPort);
    void Tick();

    const SmartPtr<Managers::BaseManager>& GetManager(const ClassInfo *classInfo);

    void RequestPause();
    void RequestResume();
    void RequestQuit();

	static ClientInstance* Instance();

    void Send(Network::Serializable *object);
};

inline ClientInstance*
ClientInstance::Instance()
{
	return ClientInstance::instance;
}

}; // namespace Core
