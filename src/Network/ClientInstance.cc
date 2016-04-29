#include "Network/ClientInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Queue.h"
#include "Core/IO/BitStream.h"

using namespace Core;
using namespace Core::IO;
using namespace Core::Memory;
using namespace Managers;

namespace Network {

ClientInstance::ClientInstance()
: HostInstance(),
  server(nullptr),
  active(false),
  sendQueue(GetAllocator<MallocAllocator>())
{ }

ClientInstance::~ClientInstance()
{ }

bool
ClientInstance::Initialize(const char *serverHost, int serverPort)
{
    return this->Connect(serverHost, serverPort);
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
            log->Write(Log::Info, "Connected with %x:%u.",
                event.peer->address.host,
                event.peer->address.port);
            assert(nullptr == server);
            server = event.peer;
            while (sendQueue.Count() > 0)
            {
                auto obj = sendQueue.Front();
                sendQueue.PopFront();
                this->Send(obj);
            }
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            log->Write(Log::Info, "Disconnected from %x:%u.",
                event.peer->address.host,
                event.peer->address.port);
            assert(event.peer == server);
            server = nullptr;
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
                ptr->Deserialize(data);/*
                if (ptr->IsInstanceOf<Game::PlayerState>())
                {
                    auto plaState = SmartPtr<Game::PlayerState>::CastFrom(ptr);
                    log->Write(Log::Info, "%d, %f, %f - %d", plaState->t, plaState->x, plaState->y, event.peer->roundTripTime);
                    //player->SetState(plaState->t, plaState->x, plaState->y);
                }*/
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

    active = false;

    this->Stop();
}

void
ClientInstance::Send(const SmartPtr<Serializable> &object)
{
    if (server != nullptr)
    {
        BitStream data(GetAllocator<ScratchAllocator>());
        object->Serialize(data);
        ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(server, 0, packet);
    }
    else
    {
        sendQueue.PushBack(object);
    }
}

}; // namespace Network
