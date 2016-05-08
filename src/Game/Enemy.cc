#include "Game/Enemy.h"
#include "Core/Collections/Array.h"
#include "Core/Collections/Queue.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Math/Math.h"
#include "Core/Log.h"
#include "Network/ClientInstance.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

DefineClassInfo(Game::Enemy, Core::RefCounted);

Enemy::Enemy(Type _type, const NetData &data)
: type(_type),
  states(GetAllocator<BlocksAllocator>(), _type == SimulatedOnServer ? 32 : 10),
  waypoints(GetAllocator<MallocAllocator>(), 3),
  waypointIndex(0)
{
    states.PushBack(State(.0f, data.p0x, data.p0y));

    waypoints.PushBack(Vector2(data.p0x, data.p0y));
    waypoints.PushBack(Vector2(data.p1x, data.p1y));
    waypoints.PushBack(Vector2(data.p2x, data.p2y));
}

Enemy::~Enemy()
{ }

void
Enemy::Step(State &state)
{
    Vector2 newPos(state.px, state.py);
    Vector2 currWaypoint = waypoints[waypointIndex];
    Vector2 toWaypoint = currWaypoint - newPos;

    float sqDist = toWaypoint.GetSqrMagnitude();
    if (sqDist <= 1.0f)
    {
        waypointIndex = (waypointIndex + 1) % waypoints.Count();

        currWaypoint = waypoints[waypointIndex];
        toWaypoint = currWaypoint - newPos;
        sqDist = toWaypoint.GetSqrMagnitude();
    }

    toWaypoint /= sqrtf(sqDist);
    newPos += (toWaypoint * 2.5f * Network::HostInstance::kFixedTimeStep);

    state.px = newPos.x;
    state.py = newPos.y;

    ++state.step;
}

void
Enemy::SendEnemyState(const SmartPtr<EnemyState> &enemyState)
{
    assert(type != SimulatedOnServer);

    int i = 0, c = states.Count();
    for (; i < c; ++i)
    {
        if (states[i].step < enemyState->step)
            break;
    }

    if (c == states.Capacity())
    {
        if (i == c)
            return;
        else
            states.PopBack();
    }

    states.Insert(i, State(enemyState->step, enemyState->x, enemyState->y));
}

void
Enemy::Update(uint32_t step)
{
    if (Cloned == type)
        return;

    State newState = states[0];

    while (newState.step < step)
    {
        this->Step(newState);

        if (states.Capacity() == states.Count())
            states.PopBack();

        states.Insert(0, newState);
    }
}

void
Enemy::GetCurrentPosition(float *x, float *y)
{
    auto &s = states.Front();
    *x = s.px;
    *y = s.py;
}

void
Enemy::GetPositionAtTime(float t, float *x, float *y)
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
}

} // namespace Game
