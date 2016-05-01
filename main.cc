#include <iostream>

#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Memory/ScratchAllocator.h"
#include "Network/ServerInstance.h"
#include "Managers/GetManager.h"

using namespace Core::Memory;
using namespace Managers;

Network::ServerInstance *serverInstance = nullptr;
char serverInstanceBuffer[sizeof(Network::ServerInstance)];

void shutdown()
{
    serverInstance->RequestStop();
    serverInstance->~ServerInstance();
    serverInstance = nullptr;

    Core::ClassInfoUtils::Destroy();

    ShutdownMemory();
}

int main(int argc, char **argv) {
	InitializeMemory();

	InitAllocator<MallocAllocator>();
	InitAllocator<LinearAllocator>(&GetAllocator<MallocAllocator>(), 1 * 1024 * 1024, 16);
	InitAllocator<BlocksAllocator>(&GetAllocator<MallocAllocator>(), 8192);
    InitAllocator<ScratchAllocator>(&GetAllocator<MallocAllocator>(), 512 * 1024);

    Core::ClassInfoUtils::Instance()->Initialize();

    serverInstance = new(serverInstanceBuffer) Network::ServerInstance();
    Core::Log::Instance()->SetCallback([](int msgType, const char *msg)
    {
        std::cout << msg << std::endl;
    });

    atexit(shutdown);

    if (serverInstance->Initialize(1234))
    {
        std::cout << "server started" << std::endl;

        while (true)
        {
            serverInstance->Tick();

            Sleep(0);
        }
    }
}
