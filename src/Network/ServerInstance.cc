#include "Network/ServerInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"
#include "Core/IO/BitStream.h"

using namespace Core;
using namespace Core::IO;
using namespace Core::Memory;
using namespace Managers;

namespace Network {

ServerInstance::ServerInstance()
: HostInstance(),
  peers(GetAllocator<MallocAllocator>())
{ }

ServerInstance::~ServerInstance()
{ }

bool
ServerInstance::Initialize(int port)
{
    return this->StartServer(port);
}

void
ServerInstance::Tick()
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
                ptr->Deserialize(data);/*
                if (ptr->IsInstanceOf<Game::PlayerInputs>())
                {
                    auto plaInputs = SmartPtr<Game::PlayerInputs>::CastFrom(ptr);
                    player->SetInput(plaInputs->x, plaInputs->y);
                }*/
            }
            enet_packet_destroy(event.packet);
            break;
        }
    }

    // physics
    /*
    player->Update();
    {
        SmartPtr<Game::PlayerState> plaState = SmartPtr<Game::PlayerState>::MakeNew<BlocksAllocator>();
        plaState->t = player->GetT();
        plaState->x = player->GetX();
        plaState->y = player->GetY();
        this->Broadcast(plaState);
    }
    */

    auto it = managers.Begin(), end = managers.End();
    for (; it != end; ++it)
        (*it)->OnUpdate();

    // update scenegraph, animations, ecc...

    for (it = managers.Begin(); it != end; ++it)
        (*it)->OnLateUpdate();

    RefCounted::GC.Collect();
}

void
ServerInstance::RequestStop()
{
    auto it = managers.Begin(), end = managers.End();
    for (; it != end; ++it)
        (*it)->OnQuit();

    this->Stop();
}

void
ServerInstance::Broadcast(Network::Serializable *object)
{
    if (peers.Count() > 0) // ToDo: isConnected
    {
        BitStream data(GetAllocator<ScratchAllocator>());
        object->Serialize(data);
        ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), ENET_PACKET_FLAG_RELIABLE);

        auto it = peers.Begin(), end = peers.End();
        for (; it < end; ++it)
            enet_peer_send(*it, 0, packet);

        //enet_host_flush(host);
    }
    else
    {
        // ToDo: put in queue SmartPtr
    }
}

}; // namespace Network
