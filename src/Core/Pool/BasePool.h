#pragma once

#include "Core/ClassInfo.h"
#include "Core/Collections/Array_type.h"

using namespace Core::Collections;

namespace Core {
    namespace Pool {

class BaseObject;

class BasePool {
protected:
    static const uint32_t kEndOfList = 0xffffffff;

    struct Entry {
        uint32_t id;
        uint32_t index;
        uint32_t prev;
        uint32_t next;
    };

    uint32_t nextId;

    ClassInfo *classInfo;

    Array<uint32_t> map;
    Array<Entry> entries;

    void *data;
    uint32_t size;
    uint32_t capacity;

    virtual void CopyObjects(void *dest, void *src, uint32_t objectsCount) = 0;
    virtual void MoveObjects(void *dest, void *src, uint32_t objectsCount) = 0;

    void Grow();
    bool IsHashFull() const;
    void Rehash(uint32_t newMapSize);

    void Insert(BaseObject *pointer);
    void Remove(BaseObject *pointer);

    BaseObject* Allocate();
    void Free(BaseObject *pointer);
public:
    BasePool(Memory::Allocator &allocator, ClassInfo *_classInfo);
    BasePool(const BasePool &other);
    BasePool(BasePool &&other);
    virtual ~BasePool();

    BasePool& operator =(const BasePool &other);
    BasePool& operator =(BasePool &&other);

    const ClassInfo* GetBaseObjectClassInfo() const;
    Memory::Allocator& GetAllocator() const;
    uint32_t Count() const;
    uint32_t Capacity() const;
    bool IsEmpty() const;
    void Clear();

    const BaseObject* Get(uint32_t id) const;
    BaseObject* Get(uint32_t id);

    BaseObject* Clone(uint32_t id);

    friend class BaseObject;
};

    } // namespace Pool
} // namespace Core
