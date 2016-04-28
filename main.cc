#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Memory/ScratchAllocator.h"
#include "Core/GameInstance.h"

#include "Managers/GetManager.h"

using namespace Core::Memory;
using namespace Managers;

int main(int argc, char **argv) {
	InitializeMemory();

	InitAllocator<MallocAllocator>();
	InitAllocator<LinearAllocator>(&GetAllocator<MallocAllocator>(), 1 * 1024 * 1024, 16);
	InitAllocator<BlocksAllocator>(&GetAllocator<MallocAllocator>(), 8192);
    InitAllocator<ScratchAllocator>(&GetAllocator<MallocAllocator>(), 512 * 1024);

	{
		Core::GameInstance gameInstance;
        if (gameInstance.Initialize(nullptr, 1234))
        {
            while (true)
            {
                gameInstance.Tick();

                Sleep(33); // 30 HZ?
            }
        }
	}

	ShutdownMemory();
}
