#pragma once

#include <cstdint>
#include "Core/Debug.h"

namespace Core {
    namespace Pool {

class BasePool;

    } // namespace Pool
} // namespace Core

template <typename T>
class Handle {
private:
    Core::Pool::BasePool *pool;
    uint32_t instanceId;
public:
    Handle();
    Handle(T *p);

    void Set(T *p);
    T* Get() const;
    bool IsValid() const;
    bool EqualsTo(const T *p) const;

    Handle<T>& operator =(T *p);

    bool operator ==(const Handle<T> &p) const;
    bool operator !=(const Handle<T> &p) const;

    operator T*() const;
    T* operator ->() const;
    T& operator *() const;

    template <class U> U* Cast() const;

    static const Handle<T> Null;
};
