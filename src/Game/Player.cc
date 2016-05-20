#include "Game/Player.h"
#include "Core/Collections/Array.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Log.h"
#include "Network/ClientInstance.h"
#include "Network/Messages/PlayerInputs.h"
#include "Network/Messages/PlayerState.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

Player::Input::Input(const SmartPtr<PlayerInputs> &playerInputs)
: step(playerInputs->step),
  x(playerInputs->x),
  y(playerInputs->y),
  attack(playerInputs->attack)
{ }

Player::State::State(const SmartPtr<PlayerState> &playerState)
: step(playerState->step),
  position(playerState->x, playerState->y),
  direction(playerState->dx, playerState->dy),
  actionState(playerState->actionState),
  actionStep(playerState->actionStep)
{ }

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
    bool isMoving = vMag > 0.02f;

    switch (state.actionState)
    {
    case Idle:
        if (input.attack)
        {
            state.actionState = Attacking;
            state.actionStep = input.step + 1;

            if (isMoving)
                state.direction = v;

            state.position += state.direction * 10.0f * Network::HostInstance::kFixedTimeStep;
        }
        else if (isMoving)
        {
            state.actionState = Moving;
            state.actionStep = input.step + 1;

            state.position += v * vMag * vMag * 10.0f * Network::HostInstance::kFixedTimeStep;
            state.direction = v;
        }
        break;
    case Moving:
        if (input.attack)
        {
            state.actionState = Attacking;
            state.actionStep = input.step + 1;

            if (isMoving)
                state.direction = v;

            state.position += state.direction * 10.0f * Network::HostInstance::kFixedTimeStep;
        }
        else if (!isMoving)
        {
            state.actionState = Idle;
            state.actionStep = input.step + 1;
        }
        else
        {
            state.position += v * vMag * vMag * 10.0f * Network::HostInstance::kFixedTimeStep;
            state.direction = v;
        }
        break;
    case Attacking:
        uint32_t attackStep = (input.step + 1 - state.actionStep);
        float speed = Math::Lerp(10.0f, 0.0f, attackStep / 30.0f);
        state.position += state.direction * speed * Network::HostInstance::kFixedTimeStep;

        if (15 == attackStep)
        {

        }

        if (attackStep >= 30)
        {
            state.actionState = Idle;
            state.actionStep = input.step + 1;
        }
        break;
    }

    //Core::Log::Instance()->Write(Core::Log::Info, "Player step %u -> %u", state.step, input.step + 1);

    state.step = input.step + 1;
}

void
Player::SendPlayerInput(const SmartPtr<PlayerInputs> &playerInputs)
{
    assert(type != Cloned);

    int i = 0, c = inputs.Count();
    for (; i < c; ++i)
    {
        if (inputs[i].step < playerInputs->step)
            break;
    }

    if (c == inputs.Capacity())
    {
        if (i == c)
            return;
        else
            inputs.PopBack();
    }

    inputs.Insert(i, Input(playerInputs));
}

void
Player::SendPlayerState(const SmartPtr<PlayerState> &playerState)
{
    assert(type != SimulatedOnServer);

    int i = 0, c = states.Count();
    for (; i < c; ++i)
    {
        if (states[i].step <= playerState->step)
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

        states.Insert(i, State(playerState));
    }
    else // simulated on client, lagless
    {
        State s = states[i];
        states.RemoveRange(i, c - i); // remove older states

        if (0 == i)
        { // newer state
            //Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f@%u (client: %f,%f@%u)", playerState->x, playerState->y, playerState->step, s.position.x, s.position.y, s.step);

            states.PushBack(State(playerState));

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
                states.PushBack(State(playerState));

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
        offsetX *= (1.0f - (Network::HostInstance::kFixedTimeStep * 16.0f));
        offsetY *= (1.0f - (Network::HostInstance::kFixedTimeStep * 16.0f));
    }
}

void
Player::GetCurrentPosition(float *x, float *y) const
{
    auto &s = states.Front();
    if (SimulatedLagless == type)
    {
        *x = s.position.x + offsetX;
        *y = s.position.y + offsetY;
    }
    else
    {
        *x = s.position.x;
        *y = s.position.y;
    }
}

void
Player::GetCurrentDirection(float *dx, float *dy) const
{
    auto &s = states.Front();
    *dx = s.direction.x;
    *dy = s.direction.y;
}

void
Player::GetCurrentState(ActionState *state, float *time) const
{
    auto &s = states.Front();
    *state = s.actionState;
    *time = (s.step - s.actionStep) * Network::HostInstance::kFixedTimeStep;
}

void
Player::GetStateAtTime(float t, float *x, float *y, float *dx, float *dy, ActionState *state, float *time) const
{
    int last = states.Count() - 1, i = last;

    uint32_t s = floorf(t / Network::HostInstance::kFixedTimeStep);

    while (i >= 0 && states[i].step < s)
        --i;

    if (-1 == i) // too new
    {
        auto &s0 = states[0];

        *x     = s0.position.x;
        *y     = s0.position.y;
        *dx    = s0.direction.x;
        *dy    = s0.direction.y;
        *state = s0.actionState;
        *time  = (s0.step - s0.actionStep) * Network::HostInstance::kFixedTimeStep; // ToDo: fix?
    }
    else if (last == i) // too old
    {
        auto &s1 = states[last];

        *x     = s1.position.x;
        *y     = s1.position.y;
        *dx    = s1.direction.x;
        *dy    = s1.direction.y;
        *state = s1.actionState;
        *time  = (s1.step - s1.actionStep) * Network::HostInstance::kFixedTimeStep; // ToDo: fix?
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

        *x     = Math::Lerp(s0.position.x, s1.position.x, u);
        *y     = Math::Lerp(s0.position.y, s1.position.y, u);
        *dx    = cosf(a);
        *dy    = sinf(a);
        *state = s0.actionState;
        *time  = (s0.step - s1.actionStep) * Network::HostInstance::kFixedTimeStep + (t - t0);
    }
}

void
Player::FillPlayerState(const SmartPtr<PlayerState> &playerState)
{
    auto &s = states.Front();
    playerState->step = s.step;
    playerState->x = s.position.x;
    playerState->y = s.position.y;
    playerState->dx = s.direction.x;
    playerState->dy = s.direction.y;
    playerState->actionState = s.actionState;
    playerState->actionStep = s.actionStep;
}

} // namespace Game
