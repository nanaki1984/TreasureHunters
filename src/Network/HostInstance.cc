#include "Network/HostInstance.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"

using namespace Core;
using namespace Core::Memory;
using namespace Managers;

namespace Network {

HostInstance *HostInstance::instance = nullptr;

HostInstance::HostInstance()
: host(nullptr),
  managers(GetAllocator<MallocAllocator>())
{
    assert(nullptr == instance);
    instance = this;

    log = SmartPtr<Core::Log>::MakeNew<LinearAllocator>();
    stringsTable = SmartPtr<StringsTable>::MakeNew<LinearAllocator>(&GetAllocator<MallocAllocator>());

    timeServer = SmartPtr<Time::TimeServer>::MakeNew<LinearAllocator>();
    timeServer->Pause();
}

HostInstance::~HostInstance()
{
    assert(this == instance);
    instance = nullptr;
}

bool
HostInstance::StartServer(int port)
{
    ClassInfoUtils::Instance()->Initialize();

    ENetCallbacks callbacks;
    Memory::Zero(&callbacks);
    callbacks.malloc = [](size_t sz) { return Memory::GetAllocator<MallocAllocator>().Allocate(sz, 4); };
    callbacks.free = [](void *ptr) { return Memory::GetAllocator<MallocAllocator>().Free(ptr); };

    if (0 == enet_initialize_with_callbacks(ENET_VERSION, &callbacks))
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        host = enet_host_create(&address, 8, 2, 0, 0);
        if (nullptr == host)
            return false;

        if (enet_host_compress_with_range_coder(host) != 0)
            return false;
    }

    return true;
}

bool
HostInstance::Connect(const char *serverHost, int serverPort)
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
HostInstance::Stop()
{
    managers = Collections::Array<SmartPtr<BaseManager>>(GetAllocator<MallocAllocator>());

    RefCounted::GC.Collect();

    timeServer.Reset();
    stringsTable.Reset();
    log.Reset();

    RefCounted::GC.Collect();

    if (host != nullptr)
        enet_host_destroy(host);
    enet_deinitialize();

    ClassInfoUtils::Destroy();
}

const SmartPtr<Managers::BaseManager>&
HostInstance::GetManager(const ClassInfo *classInfo)
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

}; // namespace Network
