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
  inputs(GetAllocator<MallocAllocator>(), 16),
  states(GetAllocator<MallocAllocator>(), 16),
  offsetX(0.0f),
  offsetY(0.0f)
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
    while (inputs.Count() > 1)
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
        float cPx, cPy;
        this->GetPositionAtTime(t, &cPx, &cPy);
        this->RemoveOlderStates(t);

        if (0 == states.Count())
        {
            //Core::Log::Instance()->Write(Core::Log::Info, "Recv newer player state %f,%f @ %f", px, py, t);

            states.PushBack(State(t, px, py));

            // no lerp, just snap
            offsetX = offsetY = 0.0f;
        }
        else
        { // check old states for errors
            Vector2 serverPos = Vector2(px, py);
            Vector2 clientPos = Vector2(cPx, cPy);
            float sqDist = (serverPos - clientPos).GetSqrMagnitude();

            //Core::Log::Instance()->Write(Core::Log::Info, "Recv player state %f,%f @ %f (now: %f, interp: %f,%f) - sqDist: %f", px, py, t, states.Back().t, cPx, cPy, sqDist);

            if (sqDist > 0.01f)//0.1089f)//0.25f)//0.0625f)//0.04f)//0.0025f)
            {
                auto &s = states.Back();
                cPx = s.px + offsetX;
                cPy = s.py + offsetY;

                // resimulate with inputs
                float prevStateTime = t,//states.Front().t,
                      newStateTime  = s.t;

                states.Clear();
                states.PushBack(State(prevStateTime, px, py));

                type = SimulatedOnServer;
                this->RemoveOlderInputs(prevStateTime);
                if (inputs.Count() > 0)
                    inputs.Front().t = prevStateTime;
                this->Update(newStateTime);
                type = SimulatedLagless;

                s = states.Back();
                offsetX = cPx - s.px;
                offsetY = cPy - s.py;
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

    float prevStateT = newState.t;
    if (SimulatedOnServer == type)
    {
        //this->RemoveOlderInputs(prevStateT);
        float oldestInputT = inputs.Front().t;
        if (oldestInputT < prevStateT)
        {
            prevStateT = oldestInputT;
            this->GetPositionAtTime(prevStateT, &newState.px, &newState.py);
            newState.t = prevStateT;
        }
    }

    // process new inputs
    Input input;
    if (SimulatedLagless == type)
    {
        uint32_t i = 1, c = inputs.Count();
        if (c > 0)
        {
            for (; i < c; ++i)
            {
                if (inputs[i].t > t)
                    break;
            }
            input = inputs[i - 1];

            Vector2 v(input.x, input.y);
            float vMag = std::min(1.0f, v.Normalize());
            if (vMag > 0.02f)
                v *= vMag * vMag;
            else
                v = Vector2::Zero;

            float dt = (t - std::max(prevStateT, input.t));
            if (dt > .0f)
            {
                Core::Log::Instance()->Write(Core::Log::Info, "input dt: %f", dt);
                newState.px += v.x * 10.0f * dt;
                newState.py += v.y * 10.0f * dt;
            }
        }
        newState.t = t;
    }
    else
    {
        float t0, t1 = .0f;
        bool stop = false;
        //Core::Log::Instance()->Write(Core::Log::Info, "inputs count: %d", inputs.Count());
        do
        {
            input = inputs.Front();

            t0 = std::max(t1, input.t);
            t1 = t;

            stop = true;
            if (inputs.Count() > 1)
            {
                inputs.PopFront();
                if (inputs.Front().t < t1)
                {
                    t1 = inputs.Front().t;
                    stop = false;
                }
            }

            // process input
            Vector2 v(input.x, input.y);
            float vMag = std::min(1.0f, v.Normalize());
            if (vMag > 0.02f)
                v *= vMag * vMag;
            else
                v = Vector2::Zero;

            float dt = t1 - t0;
            Core::Log::Instance()->Write(Core::Log::Info, "server input dt: %f, stop: %d", dt, stop ? 1 : 0);

            dt = std::min(Network::ClientInstance::kFixedStepTime * 2.0f, dt); // keep input for 2frames MAX

            newState.px += v.x * 10.0f * dt;
            newState.py += v.y * 10.0f * dt;
            newState.t += dt;
        } while (!stop);
    }

    if (states.Capacity() == states.Count())
        states.PopFront();

    states.PushBack(newState);

    if (SimulatedLagless == type)
    {
        offsetX *= (1.0f - (Network::ClientInstance::kFixedStepTime * 2.0f));// 4.0f);// 12.0f));
        offsetY *= (1.0f - (Network::ClientInstance::kFixedStepTime * 2.0f));// 4.0f);// 12.0f));
    }
}

void
Player::GetCurrentPosition(float *x, float *y)
{
    auto &s = states.Back();
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
