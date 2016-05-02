#include "Game/Enemy.h"
#include "Core/Collections/Array.h"
#include "Core/Collections/Queue.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"
#include "Math/Math.h"
#include "Core/Log.h"
#include "Network/ClientInstance.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

DefineClassInfo(Game::Enemy, Core::RefCounted);

Enemy::Enemy(Type _type, const NetData &data)
: type(_type),
  states(GetAllocator<MallocAllocator>(), 8),
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

float
Enemy::GetLastTimestamp() const
{
    return states.Back().t;
}

void
Enemy::SendEnemyState(float t, float px, float py)
{
    assert(type != SimulatedOnServer);

    if (states.Capacity() == states.Count())
        states.PopFront();

    states.PushBack(State(t, px, py));
}

void
Enemy::Update(float t)
{
    if (Cloned == type)
        return;

    State newState = states.Back();
    float t0 = newState.t;
    newState.t = t;

    Vector2 newPos(newState.px, newState.py);
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
    newPos += (toWaypoint * 7.5f * (t - t0));

    newState.px = newPos.x;
    newState.py = newPos.y;

    if (states.Capacity() == states.Count())
        states.PopFront();

    states.PushBack(newState);
}

void
Enemy::GetCurrentPosition(float *x, float *y)
{
    auto &s = states.Back();
    *x = s.px;
    *y = s.py;
}

void
Enemy::GetPositionAtTime(float t, float *x, float *y)
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
