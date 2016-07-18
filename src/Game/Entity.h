#pragma once

#include <type_traits>
#include "Core/RefCounted.h"
#include "Core/SmartPtr.h"
#include "Core/Collections/Array.h"
#include "Game/EntityState.h"
#include "Math/Math.h"
#include "Math/Vector2.h"

namespace Game {

using Core::Collections::Array;
using Math::Vector2;

template <typename Input, typename Actions>
class Entity : public Core::RefCounted {
    DeclareClassInfo;
public:
    static_assert(std::is_enum<Actions>::value);

    enum Type
    {
        SimulatedOnServer = 0,  // server
        SimulatedLagless,       // client - user player
        Cloned                  // client - other players
    };
protected:
    Type type;
    Array<Input> inputs;
    Array<EntityState<Actions>> states;

    float offsetX, offsetY;
    bool hasChanged;

    void Step(const Input &input, State &state);

    void RemoveOlderInputs(uint32_t step);

    void InsertInput(const Input &entityInputs);
    void InsertState(const EntityState<Actions> &entityState);
public:
    Entity(Type _type, const NetData &data);
    Entity(const Entity<Input, Actions> &other) = delete;
    virtual ~Entity();

    Entity& operator =(const Entity<Input, Actions> &other) = delete;

    void Update(uint32_t step);

    Type GetType() const;
    bool HasChanged() const;

    Vector2 GetCurrentPosition() const;
    Vector2 GetCurrentDirection() const;
    Actions GetCurrentAction(float *time) const;

    void GetStateAtTime(float t, Vector2 &p, Vector2 &d, Actions &action, float *time) const;

    void FillEntityState(const SmartPtr<NetEntityState> &entityState);
};

template <typename Input, typename Actions>
inline
Entity<Input, Actions>::Entity(Type _type)
: type(_type),
  states(GetAllocator<BlocksAllocator>(), 32),
  offsetX(0.0f),
  offsetY(0.0f),
  hasChanged(false)
{ }

template <typename Input, typename Actions>
inline
Entity<Input, Actions>::~Entity()
{ }

template <typename Input, typename Actions>
inline void
Entity<Input, Actions>::RemoveOlderInputs(uint32_t step)
{
    while (inputs.Count() > 0)
    {
        if (inputs.Back().step >= step)
            break;

        inputs.PopBack();
    }
}

template <typename Input, typename Actions>
inline void
Entity<Input, Actions>::InsertInput(const Input &entityInputs)
{
    assert(type != Cloned);

    int i = 0, c = inputs.Count();
    for (; i < c; ++i)
    {
        if (inputs[i].step < entityInputs->step)
            break;
    }

    if (c == inputs.Capacity())
    {
        if (i == c)
            return;
        else
            inputs.PopBack();
    }

    inputs.Insert(i, entityInputs);
}

template <typename Input, typename Actions>
inline void
Entity<Input, Actions>::InsertState(const EntityState<Actions> &entityState)
{
    assert(type != SimulatedOnServer);

    int i = 0, c = states.Count();
    for (; i < c; ++i)
    {
        if (states[i].step <= entityState.step)
            break;
    }

    if (Cloned == type)
    {
        if (c == states.Capacity())
        {
            if (i == c)
                return;
            else
                states.PopBack();
        }

        states.Insert(i, entityState);
    }
    else // simulated on client, lagless
    {
        auto s = states[i];
        states.RemoveRange(i, c - i); // remove older states

        if (0 == i)
        { // newer state
            //Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f@%u (client: %f,%f@%u)", playerState->x, playerState->y, playerState->step, s.position.x, s.position.y, s.step);

            states.PushBack(entityState);

            // no lerp, just snap
            offsetX = offsetY = 0.0f;
        }
        else
        { // check old states for errors
            Vector2 serverPos = Vector2(playerState->x, playerState->y);
            Vector2 clientPos = s.position;

            float sqDist = (serverPos - clientPos).GetSqrMagnitude();

            //Core::Log::Instance()->Write(Core::Log::Info, "Recv player state %f,%f@%u (client: %f,%f@%u) - sqDist: %f", serverPos.x, serverPos.y, playerState->step, clientPos.x, clientPos.y, s.step, sqDist);

            if (sqDist > 0.0025f)//0.01f)
            {
                // take current position
                s = states[0];
                float x = s.position.x + offsetX;
                float y = s.position.y + offsetY;

                // resimulate with inputs
                uint32_t prevStateStep = playerState->step,
                         newStateStep  = s.step;

                states.Clear();
                states.PushBack(entityState);

                type = SimulatedOnServer;
                this->Update(newStateStep);
                type = SimulatedLagless;

                // refresh lerp offsets
                s = states[0];
                offsetX = x - s.position.x;
                offsetY = y - s.position.y;
            }
        }
    }
}

template <typename Input, typename Actions>
inline void
Entity<Input, Actions>::Update(uint32_t step)
{
    if (Cloned == type)
        return;

    hasChanged = false;
    auto newState = states[0];
    uint32_t prevStateStep = newState.step;

    // process new inputs
    if (SimulatedLagless == type)
    {
        int i = 0, c = inputs.Count();
        if (c > 0)
        {
            while (i < c && inputs[i].step > prevStateStep)
                ++i;

            while (i >= 0 && newState.step < step)
                this->Step(inputs[i--], newState);

            if (newState.step > prevStateStep)
            {
                if (states.Capacity() == states.Count())
                    states.PopBack();

                states.Insert(0, newState);

                hasChanged = true;
            }
        }
    }
    else // SimulatedOnServer
    {
        this->RemoveOlderInputs(prevStateStep);

        int i = inputs.Count() - 1;
        while (i >= 0 && inputs[i].step <= step)
        {
            this->Step(newState, inputs[i--]);

            if (newState.step > prevStateStep)
            {
                if (states.Capacity() == states.Count())
                    states.PopBack();

                states.Insert(0, newState);

                hasChanged = true;
                prevStateStep = newState.step;
/*
                if (SimulatedOnServer == type)
                Core::Log::Instance()->Write(Core::Log::Info, "Sending new player state %f,%f@%u", newState.px, newState.py, newState.step);*/
            }
        }
    }

    if (SimulatedLagless == type)
    {
        offsetX *= (1.0f - (Network::HostInstance::kFixedTimeStep * 16.0f));
        offsetY *= (1.0f - (Network::HostInstance::kFixedTimeStep * 16.0f));
    }
}

template <typename Input, typename Actions>
inline Entity<Input, Actions>::Type
Entity<Input, Actions>::GetType() const
{
    return type;
}

template <typename Input, typename Actions>
inline bool
Entity<Input, Actions>::HasChanged() const
{
    return hasChanged;
}

template <typename Input, typename Actions>
inline Vector2
Entity<Input, Actions>::GetCurrentPosition() const
{
    auto &s = states.Front();
    if (SimulatedLagless == type)
    {
        return Vector2(
            s.position.x + offsetX,
            s.position.y + offsetY);
    }
    else
    {
        return s.position;
    }
}

template <typename Input, typename Actions>
inline Vector2
Entity<Input, Actions>::GetCurrentDirection() const
{
    auto &s = states.Front();
    return s.direction;
}

template <typename Input, typename Actions>
inline Actions
Entity<Input, Actions>::GetCurrentAction(float *time) const
{
    auto &s = states.Front();
    if (time != nullptr)
        *time = (s.step - s.actionStep) * Network::HostInstance::kFixedTimeStep;
    return s.actionState;
}

template <typename Input, typename Actions>
inline void
Entity<Input, Actions>::GetStateAtTime(float t, Vector2 &p, Vector2 &d, Actions &action, float *time) const
{
    int last = states.Count() - 1, i = last;

    uint32_t s = floorf(t / Network::HostInstance::kFixedTimeStep);

    while (i >= 0 && states[i].step < s)
        --i;

    if (-1 == i) // too new
    {
        auto &s0 = states[0];

        p.x = s0.position.x;
        p.y = s0.position.y;
        d.x = s0.direction.x;
        d.y = s0.direction.y;
        action = s0.actionState;
        if (time != nullptr)
            *time = (s0.step - s0.actionStep) * Network::HostInstance::kFixedTimeStep; // ToDo: fix?
    }
    else if (last == i) // too old
    {
        auto &s1 = states[last];

        p.x = s1.position.x;
        p.y = s1.position.y;
        d.x = s1.direction.x;
        d.y = s1.direction.y;
        action = s1.actionState;
        if (time != nullptr)
            *time = (s1.step - s1.actionStep) * Network::HostInstance::kFixedTimeStep; // ToDo: fix?
    }
    else
    {
        auto &s0 = states[i + 1],
             &s1 = states[i];

        float t0 = s0.step * Network::HostInstance::kFixedTimeStep,
              t1 = s1.step * Network::HostInstance::kFixedTimeStep,
              u  = (t - t0) / (t1 - t0);

        float a0 = atan2f(s0.direction.y, s0.direction.x),
              a1 = atan2f(s1.direction.y, s1.direction.x),
              a  = Math::AngleLerp(a0, a1, u);

        p.x = Math::Lerp(s0.position.x, s1.position.x, u);
        p.y = Math::Lerp(s0.position.y, s1.position.y, u);
        d.x = cosf(a);
        d.y = sinf(a);
        action = s0.actionState;
        if (time != nullptr)
            *time = (s0.step - s1.actionStep) * Network::HostInstance::kFixedTimeStep + (t - t0);
    }
}

} // namespace Game
