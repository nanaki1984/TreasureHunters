#include "Network/ClientInstance.h"
#include "Core/Time/TimeServer.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"
#include "Core/IO/BitStream.h"
#include "Managers/GetManager.h"
#include "Game/PlayerInputs.h"
#include "Game/PlayerState.h"

using namespace Core::Memory;
using namespace Managers;
using namespace Core::IO;

namespace Core {

ClientInstance *ClientInstance::instance = nullptr;

ClientInstance::ClientInstance()
: host(nullptr),
  peers(GetAllocator<MallocAllocator>()),
  isServer(false),
  active(false),
  managers(GetAllocator<MallocAllocator>())
{
    assert(nullptr == instance);
    instance = this;

    log = SmartPtr<Core::Log>::MakeNew<LinearAllocator>();
    stringsTable = SmartPtr<StringsTable>::MakeNew<LinearAllocator>(&GetAllocator<MallocAllocator>());
    fileServer = SmartPtr<IO::FileServer>::MakeNew<LinearAllocator>();

    timeServer = SmartPtr<Time::TimeServer>::MakeNew<LinearAllocator>();
    timeServer->Pause();
}

ClientInstance::~ClientInstance()
{
	assert(this == instance);
    instance = nullptr;
}

bool
ClientInstance::Initialize(const char *serverHost, int serverPort)
{
    ClassInfoUtils::Instance()->Initialize();

    ENetCallbacks callbacks;
    Memory::Zero(&callbacks);
    callbacks.malloc = [](size_t sz) { return Memory::GetAllocator<MallocAllocator>().Allocate(sz, 4); };
    callbacks.free = [](void *ptr) { return Memory::GetAllocator<MallocAllocator>().Free(ptr); };

    if (0 == enet_initialize_with_callbacks(ENET_VERSION, &callbacks))
    {
        ENetAddress address;

        host = enet_host_create(nullptr, 1, 2, 0, 0);
        if (nullptr == host)
            return false;

        enet_address_set_host(&address, serverHost);
        address.port = serverPort;

        enet_host_connect(host, &address, 2, 0);

        timeServer->Resume();

        return true;
    }

    return false;
}

void
ClientInstance::Tick()
{
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            log->Write(Log::Info, "Connected with %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);
            peers.PushBack(event.peer);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            log->Write(Log::Info, "Disconnected from %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);
            peers.Remove(event.peer);
            event.peer->data = nullptr;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            BitStream data(GetAllocator<MallocAllocator>(), event.packet->data, event.packet->dataLength, false);
            uint32_t fcc;
            data >> fcc;
            auto classInfo = ClassInfoUtils::Instance()->FindClassFCC(fcc);
            if (classInfo != nullptr)
            {
                auto ptr = SmartPtr<Network::Serializable>::CastFrom(classInfo->Create(&GetAllocator<BlocksAllocator>()));
                ptr->Deserialize(data);
                if (ptr->IsInstanceOf<Game::PlayerInputs>())
                {
                    auto plaInputs = SmartPtr<Game::PlayerInputs>::CastFrom(ptr);
                    player->SetInput(plaInputs->x, plaInputs->y);
                }
                else if (ptr->IsInstanceOf<Game::PlayerState>())
                {
                    auto plaState = SmartPtr<Game::PlayerState>::CastFrom(ptr);
                    log->Write(Log::Info, "%d, %f, %f - %d", plaState->t, plaState->x, plaState->y, event.peer->roundTripTime);
                    player->SetState(plaState->t, plaState->x, plaState->y);
                }
            }
            enet_packet_destroy(event.packet);
            break;
        }
    }

    // physics

    auto it = managers.Begin(), end = managers.End();
    for (; it != end; ++it)
        (*it)->OnUpdate();

    // update scenegraph, animations, ecc...

    for (it = managers.Begin(); it != end; ++it)
        (*it)->OnLateUpdate();

    RefCounted::GC.Collect();
}

const SmartPtr<Managers::BaseManager>&
ClientInstance::GetManager(const ClassInfo *classInfo)
{
    for (auto it = managers.Begin(), end = managers.End(); it < end; ++it)
    {
        if ((*it)->GetRTTI() == classInfo)
            return *const_cast<const SmartPtr<Managers::BaseManager>*>(it);
    }

    auto newMng = SmartPtr<Managers::BaseManager>::CastFrom(classInfo->Create(&GetAllocator<LinearAllocator>()));

    int index = BinarySearch<SmartPtr<Managers::BaseManager>, SmartPtr<Managers::BaseManager>>(managers, 0, managers.Count(), newMng,
    [] (const SmartPtr<Managers::BaseManager> &a,
        const SmartPtr<Managers::BaseManager> &b)
    {
        int execOrderA = a->GetExecutionOrder(),
            execOrderB = b->GetExecutionOrder();
        if (execOrderA > execOrderB)
            return 1;
        else if (execOrderA < execOrderB)
            return -1;
        else
            return 0;
    });

    if (index < 0)
        index = ~index;

    managers.Insert(index, newMng);

    if (timeServer->IsPaused())
        newMng->OnPause();

    return managers[index];
}

void
ClientInstance::RequestPause()
{
    if (!timeServer->IsPaused()) {
        timeServer->Pause();

        auto it = managers.Begin(), end = managers.End();
        for (; it != end; ++it)
            (*it)->OnPause();
    }
}

void
ClientInstance::RequestResume()
{
    if (timeServer->IsPaused()) {
    	timeServer->Resume();

        auto it = managers.Begin(), end = managers.End();
        for (; it != end; ++it)
            (*it)->OnResume();
    }
}

void
ClientInstance::RequestQuit()
{
    auto it = managers.Begin(), end = managers.End();
    for (; it != end; ++it)
        (*it)->OnQuit();

    managers = Collections::Array<SmartPtr<BaseManager>>(GetAllocator<MallocAllocator>());
    player.Reset();

    RefCounted::GC.Collect();

    timeServer.Reset();
    fileServer.Reset();
    stringsTable.Reset();
    log.Reset();

	RefCounted::GC.Collect();

    if (host != nullptr)
        enet_host_destroy(host);
    enet_deinitialize();

    ClassInfoUtils::Destroy();

    active = false;
}

void
ClientInstance::Send(Network::Serializable *object)
{
    if (peers.Count() > 0) // ToDo: isConnected
    {
        BitStream data(GetAllocator<ScratchAllocator>());
        object->Serialize(data);
        ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peers[0], 0, packet);
        //enet_host_flush(host);
        //log->Write(Log::Info, "RTT: %d", peers[0]->roundTripTime);
    }
    else
    {
        // ToDo: put in queue SmartPtr
    }
}

}; // namespace Core
