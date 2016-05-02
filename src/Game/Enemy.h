#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Array_type.h"
#include "Core/Collections/Queue_type.h"
#include "Math/Vector2.h"

namespace Game {

using Core::Collections::Array;
using Core::Collections::Queue;

class Enemy : public Core::RefCounted {
    DeclareClassInfo;
public:
    enum Type
    {
        SimulatedOnServer = 0,  // server
        Cloned                  // client - other players
    };

    struct State
    {
        float t, px, py;

        State()
        { }

        State(float _t, float _px, float _py)
        : t(_t), px(_px), py(_py)
        { }
    };

    struct NetData
    {
        float p0x, p0y;
        float p1x, p1y;
        float p2x, p2y;
    };
protected:
    Type type;
    Queue<State> states;
    Array<Math::Vector2> waypoints;
    uint8_t waypointIndex;
public:
    Enemy(Type _type, const NetData &data);
    Enemy(const Enemy &other) = delete;
    virtual ~Enemy();

    Enemy& operator =(const Enemy &other) = delete;

    void SendEnemyState(float t, float px, float py);

    void Update(float t);

    Type GetType() const;
    float GetLastTimestamp() const;

    void GetCurrentPosition(float *x, float *y);
    void GetPositionAtTime(float t, float *x, float *y);
};

inline Enemy::Type
Enemy::GetType() const
{
    return type;
}

} // namespace Game
