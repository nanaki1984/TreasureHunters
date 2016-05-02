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

Player::Player(Type _type, const NetData &data)
: type(_type),
  inputs(GetAllocator<MallocAllocator>(), 8),
  states(GetAllocator<MallocAllocator>(), 8),
  lastPx(data.startX),
  lastPy(data.startY),
  lerp(1.0f),
  lastInputT(0.0f)
{
    states.PushBack(State(.0f, data.startX, data.startY));
}

Player::~Player()
{ }

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
        float lastTimestamp = states.Back().t;

        this->RemoveOlderInputs(t);

        float cPx, cPy;
        this->GetPositionAtTime(t, &cPx, &cPy);
        this->RemoveOlderStates(t);

        if (0 == states.Count())
        {
            Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f @ %f (now: %f)", px, py, t, lastTimestamp);

            states.PushBack(State(t, px, py));

            // no lerp, just snap
        }
        else
        { // check old states for errors
            Vector2 serverPos = Vector2(px, py);
            Vector2 clientPos = Vector2(cPx, cPy);
            float sqDist = (serverPos - clientPos).GetSqrMagnitude();

            //Core::Log::Instance()->Write(Core::Log::Info, "Recv player state %f,%f @ %f - sqDist: %f", px, py, t, sqDist);

            if (sqDist > 0.25f)//0.0625f)//0.04f)
            {
                float newStateTime = states.Back().t;

                this->GetCurrentPosition(&lastPx, &lastPy);
                lerp = .0f;

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
    if (Cloned == type)
        return;

    State newState = states.Back();

    float t0 = lastInputT;// newState.t;
    if (SimulatedOnServer == type)
    {
        if (inputs.Count() > 0)
        {
            float oldest = inputs.Front().t;
            if (oldest < t0)
                t0 = oldest;
        }
        //this->RemoveOlderInputs(t0);
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

            //Core::Log::Instance()->Write(Core::Log::Info, "input dt: %f", (input.t - t0));

            newState.px += v.x * 10.0f * (input.t - t0);
            newState.py += v.y * 10.0f * (input.t - t0);

            t0 = input.t;

            inputs.PopFront();
        }
        else
            break;
    }

    lastInputT = t0;

    if (states.Capacity() == states.Count())
        states.PopFront();

    states.PushBack(newState);

    if (SimulatedLagless == type)
    {
        lerp += Network::ClientInstance::kFixedStepTime * 12.0f;
        if (lerp >= 1.0f)
        {
            lastPx = states.Back().px;
            lastPy = states.Back().py;
            lerp   = 1.0f;
        }
    }
}

void
Player::GetCurrentPosition(float *x, float *y)
{
    auto &s = states.Back();
    *x = Math::Lerp(lastPx, s.px, lerp);
    *y = Math::Lerp(lastPy, s.py, lerp);
}

void
Player::GetPositionAtTime(float t, float *x, float *y)
{
    int i = 0, count = states.Count() - 1;
    for (; i < count; ++i)
    {
        auto &s0 = states[i],
             &s1 = states[i + 1];
        if (s0.t <= t && s1.t > t)
        {
            float s = (t - s0.t) / (s1.t - s0.t);
            *x = Math::Lerp(s0.px, s1.px, s);
            *y = Math::Lerp(s0.py, s1.py, s);
            return;
        }
    }
    *x = states.Back().px;
    *y = states.Back().py;
}

} // namespace Game
