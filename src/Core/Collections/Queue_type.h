#pragma once

#include "Core/Collections/Array_type.h"

namespace Core {
    namespace Collections {
        
template <typename T>
class Queue
{
private:
    uint32_t size;
    uint32_t offset;
    Array<T> data;

    void Grow(uint32_t minCapacity = 0);
public:
    Queue(Memory::Allocator &allocator);
    Queue(Memory::Allocator &allocator, uint32_t initialCapacity);
    Queue(const Queue<T> &other);
    Queue(Queue<T> &&other);
    ~Queue();

    Queue<T>& operator =(const Queue<T> &other);
    Queue<T>& operator =(Queue<T> &&other);

    const T& operator [](uint32_t index) const;
    T& operator [](uint32_t index);

    Memory::Allocator& GetAllocator() const;
    uint32_t Count() const;
    uint32_t Capacity() const;
    bool IsEmpty() const;

    const T& Front() const;
    T& Front();
    const T& Back() const;
    T& Back();

    void Clear();
    void Reserve(uint32_t newCapacity);

    void PushBack(const T &item);
    void PopBack();
    void PushFront(const T& item);
    void PopFront();
};
        
    } // namespace Collections
} // namespace Core
