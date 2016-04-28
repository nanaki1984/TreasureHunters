#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Queue_type.h"

namespace Game {

class Player : public Core::RefCounted {
    DeclareClassInfo;
public:
    struct Input
    {
        int t;
        float x, y;
    };

    struct State
    {
        int t;
        float px, py;
    };
protected:
    Core::Collections::Queue<Input> inputs;
    Core::Collections::Queue<State> states;

    State currentState;
    Input currentInput;
public:
    Player();
    Player(const Player &other) = delete;
    virtual ~Player();

    Player& operator =(const Player &other) = delete;

    void SetInput(float x, float y);
    void SetState(int t, float x, float y);
    void Update();

    int GetT() const { return currentState.t; }
    float GetX() const { return currentState.px; }
    float GetY() const { return currentState.py; }
};

} // namespace Game
