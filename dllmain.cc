#include "Network/ClientInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Memory/ScratchAllocator.h"

#if _MSC_VER // this is defined when compiling with Visual Studio
#   define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this
#else
#   define EXPORT_API // XCode does not need annotating exported functions, so define is empty
#endif

/*
Client stays back in time of 100ms, regarding other objects state, usually interpolates between last 2 states received from server,
its state is in server time predicting position in advance (no input lag) but if a state from server differs much from the old
(in client time) state, the client rolls back and replay simulation with inputs registered.
Server rollsback when a player attacks to do checks, because players see state of other (players & bots) in the past (100ms),
anyway there's some mitigation system, e.g. the receiver of the attack has done something to defend itself.
Server shouldn't worry about rollback for inputs, just process them as they come (except for old inputs, meaning inputs with
timestamp < last_input_received_timestamp)
Server runs at 20Hz, game at 60Hz
*/

extern "C"
{
    Network::ClientInstance *clientInstance = nullptr;
    char clientInstanceBuffer[sizeof(Network::ClientInstance)];

    void EXPORT_API GameLoadLibrary()
    {
        Core::Memory::InitializeMemory();

        Core::Memory::InitAllocator<MallocAllocator>();
        Core::Memory::InitAllocator<LinearAllocator>(&GetAllocator<MallocAllocator>(), 1 * 1024 * 1024, 16);
        Core::Memory::InitAllocator<BlocksAllocator>(&GetAllocator<MallocAllocator>(), 8192);
        Core::Memory::InitAllocator<ScratchAllocator>(&GetAllocator<MallocAllocator>(), 512 * 1024);

        Core::ClassInfoUtils::Instance()->Initialize();
    }

    void EXPORT_API GameUnloadLibrary()
    {
        Core::ClassInfoUtils::Destroy();

        Core::Memory::ShutdownMemory();
    }

    bool EXPORT_API GameInit(const char *serverHost, int serverPort, Core::Log::LogFunction debugLog)
    {
        clientInstance = new(clientInstanceBuffer) Network::ClientInstance();
        Core::Log::Instance()->SetCallback(debugLog);

        return clientInstance->Initialize(serverHost, serverPort);
    }

    int EXPORT_API GameGetState()
    {
        return clientInstance->GetState();
    }

    float EXPORT_API GameGetRTT()
    {
        return clientInstance->GetRTT();
    }

    void EXPORT_API GameTick()
    {
        clientInstance->Tick();
    }

    void EXPORT_API GameCreateRoom(uint8_t playersCount, Network::ClientInstance::RoomCreationCallback callback)
    {
        clientInstance->CreateRoom(playersCount, callback);
    }

    void EXPORT_API GameJoinRoom(uint32_t roomId, Network::ClientInstance::JoinRoomCallback callback)
    {
        clientInstance->JoinRoom(roomId, callback);
    }

    void EXPORT_API GameStart(Network::ClientInstance::StartGameCallback callback)
    {
        clientInstance->StartGame(callback);
    }

    void EXPORT_API GameSendInput(float x, float y)
    {
        clientInstance->SendPlayerInputs(x, y);
    }

    void EXPORT_API GameReceivePosition(float *x, float *y)
    {
        clientInstance->GetPlayerPosition(x, y);
    }

    void EXPORT_API GamePause()
    {
        clientInstance->RequestPause();
    }

    void EXPORT_API GameResume()
    {
        clientInstance->RequestResume();
    }

    void EXPORT_API GameQuit()
    {
        clientInstance->RequestQuit();
        clientInstance->~ClientInstance();
        clientInstance = nullptr;
    }
}
/*
#ifdef _WIN32
#	define NOMINMAX
#	define WIN32_LEAN_AND_MEAN
#	define VC_EXTRALEAN
#   include <windows.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
#endif
*/