#pragma once

#include "Network/Serializable.h"
#include "Math/Vector2.h"

namespace Game {

class NetEntityState;

template <typename S>
struct EntityState
{
    uint32_t step;
    Math::Vector2 position;
    Math::Vector2 direction;
    S actionState;
    uint32_t actionStep;

    EntityState()
    { }

    EntityState(uint32_t _step, float _px, float _py, S _actionState)
    : step(_step),
      position(_px, _py),
      direction(0.0f, 1.0f),
      actionState(_actionState),
      actionStep(_step)
    { }

    explicit EntityState(const SmartPtr<NetEntityState> &entityState);
};

class NetEntityState : public Network::Serializable {
    DeclareClassInfo;
    DeclareSerializable;
protected:
    template <typename Stream> void SerializeImpl(Stream &stream)
    {
        stream.Serialize(id);
        stream.Serialize(step);
        stream.Serialize(x);
        stream.Serialize(y);
        stream.SerializeHalfFloat(dx);
        stream.SerializeHalfFloat(dy);
        stream.Serialize(actionState);
        stream.Serialize(actionStep);
    }
public:
    uint8_t id;
    uint32_t step;
    float x, y;
    float dx, dy;
    uint8_t actionState;
    uint32_t actionStep;

    NetEntityState();
    NetEntityState(const NetEntityState &other) = delete;
    virtual ~NetEntityState();

    NetEntityState& operator =(const NetEntityState &other) = delete;
};

template <typename S>
EntityState<S>::EntityState(const SmartPtr<NetEntityState> &entityState)
: step(entityState->step),
  position(entityState->x, entityState->y),
  direction(entityState->dx, entityState->dy),
  actionState(entityState->actionState),
  actionStep(entityState->actionStep)
{ }

} // namespace Game
