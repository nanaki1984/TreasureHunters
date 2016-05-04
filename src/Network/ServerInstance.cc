#include "Network/ServerInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Pool/Pool.h"
#include "Core/Collections/Array.h"
#include "Core/IO/BitStream.h"
#include "Network/Messages/CreateRoom.h"
#include "Network/Messages/JoinRoom.h"
#include "Network/Messages/StartGame.h"
#include "Network/Messages/PlayerInputs.h"

using namespace Core;
using namespace Core::IO;
using namespace Core::Memory;
using namespace Managers;

namespace Network {

ServerInstance::ServerInstance()
: HostInstance(),
  rooms(GetAllocator<MallocAllocator>())
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
    timeServer->Tick();

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            log->Write(Log::Info, "Connected with %x:%u.",
                event.peer->address.host,
                event.peer->address.port);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            log->Write(Log::Info, "Disconnected from %x:%u.",
                event.peer->address.host,
                event.peer->address.port);
            if (event.peer->data != nullptr)
            {
                GameRoom *peerRoom = static_cast<GameRoom*>(event.peer->data);
                if (peerRoom->PlayerLeft(event.peer))
                    peerRoom->Destroy();

                event.peer->data = nullptr;
            }
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            BitStream data(GetAllocator<MallocAllocator>(), event.packet->data, event.packet->dataLength, false);
            uint32_t fcc;
            data >> fcc;
            auto classInfo = ClassInfoUtils::Instance()->FindClassFCC(fcc);
            if (classInfo != nullptr)
            {
                auto ptr = SmartPtr<Network::Serializable>::CastFrom(classInfo->Create(&GetAllocator<BlocksAllocator>()));
                ptr->Deserialize(event.peer, data);

                if (ptr->IsInstanceOf<Messages::CreateRoom>())
                {
                    auto createRoom = SmartPtr<Messages::CreateRoom>::CastFrom(ptr);
                    assert(Messages::CreateRoom::kUnknownId == createRoom->roomId);
                    createRoom->roomId = rooms.NewInstance(createRoom->playersCount)->GetInstanceID();

                    this->Send(event.peer, ptr, ReliableSequenced, 1);
                }
                else if (ptr->IsInstanceOf<Messages::JoinRoom>())
                {
                    auto joinRoom = SmartPtr<Messages::JoinRoom>::CastFrom(ptr);
                    assert(Messages::JoinRoom::Request == joinRoom->flags);
                    joinRoom->flags = Messages::JoinRoom::Fail;

                    auto room = rooms.GetInstance(joinRoom->roomId);
                    if (room.IsValid() && GameRoom::WaitingJoin == room->GetState())
                    {
                        room->AddPlayer(event.peer);

                        joinRoom->flags = Messages::JoinRoom::Success;
                        joinRoom->roomData = room->GetData();

                        event.peer->data = room.Get();
                    }

                    this->Send(event.peer, ptr, ReliableSequenced, 1);
                }
                else if (ptr->IsInstanceOf<Messages::StartGame>())
                {
                    auto startGame = SmartPtr<Messages::StartGame>::CastFrom(ptr);
                    assert(Messages::StartGame::kUnknownId == startGame->playerId && Messages::StartGame::Ready == startGame->flags);
                    startGame->flags = Messages::StartGame::Fail;

                    auto room = rooms.GetInstance(startGame->roomId);
                    if (!room.IsValid() ||
                        GameRoom::Playing == room->GetState() ||
                        !room->PlayerReady(event.peer, startGame))
                        this->Send(event.peer, ptr, ReliableSequenced, 1);
                }
                else if (ptr->IsInstanceOf<Messages::PlayerInputs>())
                {
                    if (event.peer->data != nullptr)
                    {
                        GameRoom *peerRoom = static_cast<GameRoom*>(event.peer->data);
                        peerRoom->RecvPlayerInputs(event.peer, SmartPtr<Messages::PlayerInputs>::CastFrom(ptr));
                    }
                }
            }
            enet_packet_destroy(event.packet);
            break;
        }
    }

    // update rooms
    Array<Handle<GameRoom>> roomsToDelete(GetAllocator<ScratchAllocator>());
    for (auto roomIt = rooms.Begin(), roomsEnd = rooms.End(); roomIt < roomsEnd; ++roomIt)
    {
        if (roomIt->Update())
            roomsToDelete.PushBack(roomIt);
    }
    for (auto delIt = roomsToDelete.Begin(), delEnd = roomsToDelete.End(); delIt < delEnd; ++delIt)
        rooms.DeleteInstance(*delIt);

    enet_host_flush(host);

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
    rooms.Clear();

    auto it = managers.Begin(), end = managers.End();
    for (; it != end; ++it)
        (*it)->OnQuit();

    this->Stop();
}

void
ServerInstance::Send(ENetPeer *peer, const SmartPtr<Serializable> &object, MessageType messageType, uint8_t channel)
{
    BitStream data(GetAllocator<ScratchAllocator>());

    object->Serialize(peer, data);
    ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), this->MessageTypeToFlags(messageType));

    enet_peer_send(peer, channel, packet);
}

void
ServerInstance::Broadcast(const Array<ENetPeer*> &peers, const SmartPtr<Serializable> &object, MessageType messageType, uint8_t channel)
{
    if (peers.Count() > 0)
    {
        BitStream data(GetAllocator<ScratchAllocator>());

        auto it = peers.Begin(), end = peers.End();
        for (; it < end; ++it)
        {
            object->Serialize(*it, data);
            ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), this->MessageTypeToFlags(messageType));

            enet_peer_send(*it, channel, packet);
        }
    }
}

}; // namespace Network
