#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Array_type.h"
#include "Math/Vector2.h"

namespace Game {

using Core::Collections::Array;

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
        uint32_t step;
        float px, py;

        State()
        { }

        State(uint32_t _step, float _px, float _py)
        : step(_step), px(_px), py(_py)
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
    Array<State> states;
    Array<Math::Vector2> waypoints;
    uint8_t waypointIndex;

    void Step(State &state);
public:
    Enemy(Type _type, const NetData &data);
    Enemy(const Enemy &other) = delete;
    virtual ~Enemy();

    Enemy& operator =(const Enemy &other) = delete;

    void SendEnemyState(uint32_t step, float px, float py);

    void Update(uint32_t step);

    Type GetType() const;

    void GetCurrentPosition(float *x, float *y);
    void GetPositionAtTime(float t, float *x, float *y);
};

inline Enemy::Type
Enemy::GetType() const
{
    return type;
}

} // namespace Game
