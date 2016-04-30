#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Queue_type.h"

namespace Game {

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
        float t, x, y;

        Input()
        { }

        Input(float _t, float _x, float _y)
        : t(_t), x(_x), y(_y)
        { }
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
protected:
    Type type;
    Core::Collections::Queue<Input> inputs;
    Core::Collections::Queue<State> states;

    void RemoveOlderInputs(float t);
    void RemoveOlderStates(float t);

    float lastPx, lastPy, lerp;
public:
    Player(Type _type, float px, float py);
    Player(const Player &other) = delete;
    virtual ~Player();

    Player& operator =(const Player &other) = delete;

    void SendInput(float t, float x, float y);
    void SendPlayerState(float t, float px, float py);

    void Update(float t);

    float GetX() const;
    float GetY() const;
    float GetLastTimestamp() const;
};

} // namespace Game
