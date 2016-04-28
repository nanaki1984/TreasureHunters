#pragma once

#include "Network/HostInstance.h"

namespace Managers {

template <typename T>
T* GetManager()
{
    return static_cast<T*>(Network::HostInstance::Instance()->GetManager(&T::RTTI).Get());
}

} // namespace Managers
