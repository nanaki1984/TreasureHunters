#include "Core/Memory/MallocAllocator.h"

namespace Core {
    namespace Memory {

struct Header {
    uint32_t size;
};

DefineClassInfo(Core::Memory::MallocAllocator, Core::Memory::Allocator);
DefineAllocator(Core::Memory::MallocAllocator);

MallocAllocator::MallocAllocator()
: totalAllocated(0)
{ }

MallocAllocator::~MallocAllocator()
{
    assert(0 == totalAllocated);
}

void*
MallocAllocator::Allocate(size_t size, size_t align)
{
    void *d = nullptr;

    size_t ts = Allocator::GetAlignedSize<Header>(size, align);
    Header *p = (Header*)malloc(ts);
    d = Allocator::GetDataFromPointer<Header>(p, align);
    p->size = ts;
    totalAllocated += ts;

    Allocator::FillPadding(p, d);

    return d;
}

void
MallocAllocator::Free(void *pointer)
{
    if (nullptr == pointer)
        return;

    Header *h = Allocator::GetPointerFromData<Header>(pointer);
    totalAllocated -= h->size;
    free(h);
}

size_t
MallocAllocator::GetAllocatedSize(void *pointer)
{
    return Allocator::GetPointerFromData<Header>(pointer)->size;
}

size_t
MallocAllocator::GetTotalAllocated()
{
    return totalAllocated;
}

    } // namespace Memory
} // namespace Core
