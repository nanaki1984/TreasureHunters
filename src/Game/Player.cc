#include "Game/Player.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Math/Vector2.h"
#include "Math/Math.h"
#include "Core/Log.h"
#include "Network/ClientInstance.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

DefineClassInfo(Game::Player, Core::RefCounted);

Player::Player(Type _type, const NetData &data)
: type(_type),
  inputs(GetAllocator<BlocksAllocator>(), 32),
  states(GetAllocator<BlocksAllocator>(), 32),
  offsetX(0.0f),
  offsetY(0.0f),
  hasChanged(false)
{
    states.PushBack(State(0, data.startX, data.startY));
}

Player::~Player()
{ }

void
Player::RemoveOlderInputs(uint32_t step)
{
    while (inputs.Count() > 0)
    {
        if (inputs.Back().step >= step)
            break;

        inputs.PopBack();
    }
}

void
Player::Step(State &state, const Input &input)
{
    assert(state.step <= input.step);

    Vector2 v(input.x, input.y);
    float vMag = std::min(1.0f, v.Normalize());
    if (vMag > 0.02f)
        v *= vMag * vMag;
    else
        v = Vector2::Zero;

    state.px += v.x * 10.0f * Network::HostInstance::kFixedTimeStep;
    state.py += v.y * 10.0f * Network::HostInstance::kFixedTimeStep;
    //Core::Log::Instance()->Write(Core::Log::Info, "Player step %u -> %u", state.step, input.step + 1);
    state.step = input.step + 1;
}

void
Player::SendPlayerInput(uint32_t step, float x, float y)
{
    assert(type != Cloned);

    int i = 0, c = inputs.Count();
    for (; i < c; ++i)
    {
        if (inputs[i].step < step)
            break;
    }

    if (c == inputs.Capacity())
    {
        if (i == c)
            return;
        else
            inputs.PopBack();
    }

    inputs.Insert(i, Input(step, x, y));
}

void
Player::SendPlayerState(uint32_t step, float px, float py)
{
    assert(type != SimulatedOnServer);

    int i = 0, c = states.Count();
    for (; i < c; ++i)
    {
        if (states[i].step <= step)
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

        states.Insert(i, State(step, px, py));
    }
    else // simulated on client, lagless
    {
        State s = states[i];
        states.RemoveRange(i, c - i); // remove older states

        if (0 == i)
        { // newer state
            //Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f@%u (client: %f,%f@%u)", px, py, step, s.px, s.py, s.step);

            states.PushBack(State(step, px, py));

            // no lerp, just snap
            offsetX = offsetY = 0.0f;
        }
        else
        { // check old states for errors
            Vector2 serverPos = Vector2(px, py);
            Vector2 clientPos = Vector2(s.px, s.py);

            float sqDist = (serverPos - clientPos).GetSqrMagnitude();

            //Core::Log::Instance()->Write(Core::Log::Info, "Recv player state %f,%f@%u (client: %f,%f@%u) - sqDist: %f", px, py, step, s.px, s.py, s.step, sqDist);

            if (sqDist > 0.01f)//0.1089f)//0.25f)//0.0625f)//0.04f)//0.0025f)
            {
                // take current position
                s = states[0];
                float x = s.px + offsetX;
                float y = s.py + offsetY;

                // resimulate with inputs
                uint32_t prevStateStep = step,
                         newStateStep  = s.step;

                states.Clear();
                states.PushBack(State(prevStateStep, px, py));

                type = SimulatedOnServer;
                this->Update(newStateStep);
                type = SimulatedLagless;

                // refresh lerp offsets
                s = states[0];
                offsetX = x - s.px;
                offsetY = y - s.py;
            }
        }
    }
}

void
Player::Update(uint32_t step)
{
    if (Cloned == type)
        return;

    hasChanged = false;
    State newState = states[0];
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
                this->Step(newState, inputs[i--]);

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
        offsetX *= (1.0f - (Network::HostInstance::kFixedTimeStep * 12.0f));
        offsetY *= (1.0f - (Network::HostInstance::kFixedTimeStep * 12.0f));
    }
}

uint32_t
Player::GetCurrentStep() const
{
    return states.Front().step;
}

void
Player::GetCurrentPosition(float *x, float *y) const
{
    auto &s = states.Front();
    if (SimulatedLagless == type)
    {
        *x = s.px + offsetX;
        *y = s.py + offsetY;
    }
    else
    {
        *x = s.px;
        *y = s.py;
    }
}

void
Player::GetPositionAtTime(float t, float *x, float *y) const
{
    int last = states.Count() - 1, i = last;

    uint32_t s = floorf(t / Network::HostInstance::kFixedTimeStep);

    while (i >= 0 && states[i].step < s)
        --i;

    if (-1 == i)
    { // too new
        *x = states[0].px;
        *y = states[0].py;
    }
    else
    {
        if (i == last)
        { // too old
            *x = states[i].px;
            *y = states[i].py;
        }
        else
        {
            auto &s0 = states[i + 1],
                 &s1 = states[i];

            float t0 = s0.step * Network::HostInstance::kFixedTimeStep,
                  t1 = s1.step * Network::HostInstance::kFixedTimeStep,
                  u  = (t - t0) / (t1 - t0);

            *x = Math::Lerp(s0.px, s1.px, u);
            *y = Math::Lerp(s0.py, s1.py, u);
        }
    }

    if (SimulatedLagless == type)
    {
        *x += offsetX;
        *y += offsetY;
    }
}

} // namespace Game
