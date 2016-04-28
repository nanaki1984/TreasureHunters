#pragma once

#include "Core/GameInstance.h"

namespace Managers {

template <typename T>
T* GetManager()
{
    return static_cast<T*>(Core::Application::Instance()->GetManager(&T::RTTI).Get());
}

} // namespace Managers
