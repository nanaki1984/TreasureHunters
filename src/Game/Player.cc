#include "Game/Player.h"
#include "Core/Collections/Queue.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Math/Vector2.h"
#include "Math/Math.h"
#include "Core/Log.h"
#include "Network/ClientInstance.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

DefineClassInfo(Game::Player, Core::RefCounted);

Player::Player(Type _type, float px, float py)
: type(_type),
  inputs(GetAllocator<MallocAllocator>(), 8),
  states(GetAllocator<MallocAllocator>(), 8),
  lastPx(px),
  lastPy(py),
  lerp(1.0f)
{
    states.PushBack(State(.0f, px, py));
}

Player::~Player()
{ }

float
Player::GetX() const
{
    return Math::Lerp(lastPx, states.Back().px, lerp);
}

float
Player::GetY() const
{
    return Math::Lerp(lastPy, states.Back().py, lerp);
}

float
Player::GetLastTimestamp() const
{
    return states.Back().t;
}

void
Player::RemoveOlderInputs(float t)
{
    while (inputs.Count() > 0)
    {
        Input input = inputs.Front();
        if (input.t < t)
        {
            inputs.PopFront();
        }
        else
            break;
    }
}

void
Player::RemoveOlderStates(float t)
{
    while (states.Count() > 0)
    {
        State state = states.Front();
        if (state.t < t)
        {
            states.PopFront();
        }
        else
            break;
    }
}

void
Player::SendInput(float t, float x, float y)
{
    assert(type != Cloned);

    if (inputs.Capacity() == inputs.Count())
        inputs.PopFront();

    inputs.PushBack(Input(t, x, y));
}

void
Player::SendPlayerState(float t, float px, float py)
{
    assert(type != SimulatedOnServer);

    if (Cloned == type)
    {
        if (states.Capacity() == states.Count())
            states.PopFront();

        states.PushBack(State(t, px, py));
    }
    else // simulated on client, lagless
    {
        float tDiff = states.Back().t - t;

        this->RemoveOlderInputs(t);
        this->RemoveOlderStates(t);

        if (0 == states.Count())
        {
            Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f @ %f - tDiff: %f", px, py, t, tDiff);

            states.PushBack(State(t, px, py));

            // no lerp, just snap
        }
        else
        { // check old states for errors
            State state = states.Front();
            Vector2 serverPos = Vector2(px, py);
            Vector2 clientPos = Vector2(state.px, state.py);
            float sqDist = (serverPos - clientPos).GetSqrMagnitude();
            tDiff = state.t - t;

            //Core::Log::Instance()->Write(Core::Log::Info, "Recv player state %f,%f @ %f - sqDist: %f, tDiff: %f", px, py, t, sqDist, tDiff);

            if (sqDist > 0.04f)
            {
                float newStateTime = states.Back().t;

                lastPx = this->GetX();
                lastPy = this->GetY();
                lerp   = .0f;

                // resimulate with inputs
                states.Clear();
                states.PushBack(State(t, px, py));
                this->Update(newStateTime);
            }
        }
    }
}

void
Player::Update(float t)
{
    assert(type != Cloned);

    State newState = states.Back();

    float t0 = newState.t;
    if (SimulatedOnServer == type)
    {
        float oldest = inputs.Front().t;
        if (oldest < t0)
            t0 = oldest;
    }
    newState.t = t;

    // process new inputs
    while (inputs.Count() > 0)
    {
        Input input = inputs.Front();
        if (input.t <= t)
        {
            // process input
            Vector2 v(input.x, input.y);
            float vv = v.GetSqrMagnitude();
            if (vv > 0.02f)
                v /= sqrtf(vv);
            else
                v = Vector2::Zero;

            newState.px += v.x * 10.0f * (input.t - t0);
            newState.py += v.y * 10.0f * (input.t - t0);
            t0 = input.t;

            inputs.PopFront();
        }
        else
            break;
    }

    if (states.Capacity() == states.Count())
        states.PopFront();

    states.PushBack(newState);

    if (SimulatedLagless == type)
    {
        lerp += Network::ClientInstance::kFixedStepTime * 4.0f;
        if (lerp >= 1.0f)
        {
            lastPx = states.Back().px;
            lastPy = states.Back().py;
            lerp   = 1.0f;
        }
    }
}

} // namespace Game
