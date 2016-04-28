#pragma once

#include "Core/RefCounted.h"

template <class T> class WeakPtr;

namespace Core {

class Trackable : public RefCounted {
    DeclareClassInfo;
protected:
    unsigned long weakRefCount;

    WeakPtr<Trackable> *head;
    WeakPtr<Trackable> *tail;
public:
    Trackable();
    virtual ~Trackable();

    void AddWeakRef(WeakPtr<Trackable> *ptr);
    void RemoveWeakRef(WeakPtr<Trackable> *ptr);
    void InvalidateAllWeakRefs();

    unsigned long GetWeakRefCount() const;
};

inline unsigned long
Trackable::GetWeakRefCount() const
{
    return weakRefCount;
}

}; // namespace Core
