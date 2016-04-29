#include "Network/ClientInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Queue.h"
#include "Core/IO/BitStream.h"
#include "Network/Messages/CreateRoom.h"
#include "Network/Messages/JoinRoom.h"
#include "Network/Messages/StartGame.h"

using namespace Core;
using namespace Core::IO;
using namespace Core::Memory;
using namespace Managers;

namespace Network {

ClientInstance::ClientInstance()
: HostInstance(),
  server(nullptr),
  active(false),
  state(Disconnected),
  roomCreationCallback(nullptr),
  sendQueue(GetAllocator<MallocAllocator>()),
  sendQueueMsgType(GetAllocator<MallocAllocator>())
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

            assert(nullptr == server && Disconnected == state);
            server = event.peer;
            state = Connected;

            while (sendQueue.Count() > 0)
            {
                this->Send(sendQueue.Front(), sendQueueMsgType.Front());
                sendQueue.PopFront();
                sendQueueMsgType.PopFront();
            }
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            log->Write(Log::Info, "Disconnected from %x:%u.",
                event.peer->address.host,
                event.peer->address.port);

            assert(event.peer == server && state != Disconnected);
            server = nullptr;
            event.peer->data = nullptr;

            // ToDo: callback, try to reconnect?

            state = Disconnected;
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

                    assert(roomCreationCallback != nullptr);
                    roomCreationCallback(createRoom->roomId);
                    roomCreationCallback = nullptr;
                }
                else if (ptr->IsInstanceOf<Messages::JoinRoom>())
                {
                    bool success = false;

                    if (state < JoinedRoom)
                    {
                        auto joinRoom = SmartPtr<Messages::JoinRoom>::CastFrom(ptr);
                        success = Messages::JoinRoom::Success == joinRoom->flags;
                        if (success)
                        {
                            roomId = joinRoom->roomId;
                            state = JoinedRoom;
                        }
                    }

                    assert(joinRoomCallback != nullptr);
                    joinRoomCallback(success);
                    joinRoomCallback = nullptr;
                }
                else if (ptr->IsInstanceOf<Messages::StartGame>())
                {
                    bool success = false;

                    if (Waiting == state)
                    {
                        auto startGame = SmartPtr<Messages::StartGame>::CastFrom(ptr);
                        success = Messages::StartGame::Go == startGame->flags;
                        if (success)
                        {
                            playerId = startGame->playerId;
                            lastTimestamp = startGame->goTime;
                            state = Playing;
                        }
                    }

                    assert(startGameCallback != nullptr);
                    startGameCallback(success);
                    startGameCallback = nullptr;
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
ClientInstance::CreateRoom(uint8_t playersCount, RoomCreationCallback callback)
{
    if (nullptr == roomCreationCallback)
    {
        roomCreationCallback = callback;

        auto createRoom = SmartPtr<Messages::CreateRoom>::MakeNew<MallocAllocator>();
        createRoom->roomId = Messages::CreateRoom::kUnknownId;
        createRoom->playersCount = playersCount;

        this->Send(SmartPtr<Network::Serializable>::CastFrom(createRoom), ReliableSequenced);
    }
    else
        callback(Messages::CreateRoom::kUnknownId);
}

void
ClientInstance::JoinRoom(uint32_t roomId, JoinRoomCallback callback)
{
    if (Connected == state && nullptr == joinRoomCallback)
    {
        joinRoomCallback = callback;

        auto joinRoom = SmartPtr<Messages::JoinRoom>::MakeNew<MallocAllocator>();
        joinRoom->roomId = roomId;
        joinRoom->flags = Messages::JoinRoom::Request;

        this->Send(SmartPtr<Network::Serializable>::CastFrom(joinRoom), ReliableSequenced);
    }
    else
        callback(false);
}

void
ClientInstance::StartGame(StartGameCallback callback)
{
    if (JoinedRoom == state && nullptr == startGameCallback)
    {
        startGameCallback = callback;

        auto startGame = SmartPtr<Messages::StartGame>::MakeNew<MallocAllocator>();
        startGame->roomId = roomId;
        startGame->playerId = Messages::StartGame::kUnknownId;
        startGame->flags = Messages::StartGame::Ready;
        startGame->goTime = .0f;

        this->Send(SmartPtr<Network::Serializable>::CastFrom(startGame), ReliableSequenced);

        state = Waiting;
    }
    else
        callback(false);
}

void
ClientInstance::Send(const SmartPtr<Serializable> &object, MessageType messageType)
{
    if (server != nullptr)
    {
        BitStream data(GetAllocator<ScratchAllocator>());

        object->Serialize(server, data);
        ENetPacket *packet = enet_packet_create(data.GetData(), data.GetSize(), this->MessageTypeToFlags(messageType));

        enet_peer_send(server, 0, packet);
    }
    else
    {
        sendQueue.PushBack(object);
        sendQueueMsgType.PushBack(messageType);
    }
}

}; // namespace Network