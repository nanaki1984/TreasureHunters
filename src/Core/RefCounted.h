#pragma once

#include "Core/ClassInfo.h"
#include "Core/Collections/List_type.h"

namespace Core {
    namespace Memory {
        class Allocator;
    } // namespace Memory

class GarbageCollector;

class RefCounted {
    DeclareRootClassInfo;
public:
	static GarbageCollector GC;
private:
	Collections::ListNode<RefCounted> node;
	
	unsigned long refCount;
public:
	Memory::Allocator *allocator;

    RefCounted();

    void AddRef();
    void Release();

    unsigned long GetRefCount() const;
protected:
    virtual ~RefCounted();

	friend class GarbageCollector;
};

class GarbageCollector {
private:
	Collections::List<RefCounted, &RefCounted::node> garbage;
public:
	GarbageCollector();
	~GarbageCollector();

	void Collect();

	friend class RefCounted;
};

}; // namespace Core
