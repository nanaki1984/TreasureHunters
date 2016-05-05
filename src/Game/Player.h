#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Array_type.h"

namespace Game {

using Core::Collections::Array;

class Player : public Core::RefCounted {
    DeclareClassInfo;
public:
    enum Type
    {
        SimulatedOnServer = 0,  // server
        SimulatedLagless,       // client - user player
        Cloned                  // client - other players
    };

    struct Input
    {
        uint32_t step;
        float x, y;

        Input()
        { }

        Input(uint32_t _step, float _x, float _y)
        : step(_step), x(_x), y(_y)
        { }
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
        float startX, startY;
        // ToDo: other data
    };
protected:
    Type type;
    Array<Input> inputs;
    Array<State> states;

    void RemoveOlderInputs(uint32_t step);
    void Step(State &state, const Input &input);

    float offsetX, offsetY;
    bool hasChanged;
public:
    Player(Type _type, const NetData &data);
    Player(const Player &other) = delete;
    virtual ~Player();

    Player& operator =(const Player &other) = delete;

    void SendPlayerInput(uint32_t step, float x, float y);
    void SendPlayerState(uint32_t step, float px, float py);

    void Update(uint32_t step);

    Type GetType() const;
    bool HasChanged() const;

    uint32_t GetCurrentStep() const;
    void GetCurrentPosition(float *x, float *y) const;
    void GetPositionAtTime(float t, float *x, float *y) const;
};

inline Player::Type
Player::GetType() const
{
    return type;
}

inline bool
Player::HasChanged() const
{
    return hasChanged;
}

} // namespace Game
