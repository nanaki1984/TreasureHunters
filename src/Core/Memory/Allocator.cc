#include "Core/Memory/Allocator.h"
#include "Core/Collections/List.h"

namespace Core {
    namespace Memory {

DefineRootAbstractClassInfo(Core::Memory::Allocator);

Allocator::Allocator()
{ }

Allocator::~Allocator()
{ }

Allocator::Allocator(const Allocator &other)
{ }

Allocator&
Allocator::operator =(const Allocator &other)
{
    return (*this);
}

    } // namespace Memory
} // namespace Core
