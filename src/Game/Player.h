#pragma once

#include "Core/RefCounted.h"
#include "Core/SmartPtr.h"
#include "Core/Collections/Array_type.h"
#include "Math/Math.h"
#include "Math/Vector2.h"

namespace Network {
    namespace Messages {
        class PlayerInputs;
        class PlayerState;
    }
}

namespace Game {

using Core::Collections::Array;
using Network::Messages::PlayerInputs;
using Network::Messages::PlayerState;

class Player : public Core::RefCounted {
    DeclareClassInfo;
public:
    enum Type
    {
        SimulatedOnServer = 0,  // server
        SimulatedLagless,       // client - user player
        Cloned                  // client - other players
    };

    enum ActionState
    {
        Idle = 0,
        Moving,
        Attacking
    };

    struct Input
    {
        uint32_t step;
        float x, y;
        bool attack;

        Input()
        { }

        explicit Input(const SmartPtr<PlayerInputs> &playerInputs);
    };

    struct State
    {
        uint32_t step;
        Math::Vector2 position;
        Math::Vector2 direction;
        ActionState actionState;
        uint32_t actionStep;

        State()
        { }

        State(uint32_t _step, float _px, float _py)
        : step(_step),
          position(_px, _py),
          direction(0.0f, 1.0f),
          actionState(Idle),
          actionStep(_step)
        { }

        explicit State(const SmartPtr<PlayerState> &playerState);
    };

    struct AttackFrameData
    {
        uint32_t step;
        float velocity;
    };

    struct AttackHitData
    {
        uint32_t step;
        uint32_t stepsCount;
        Vector2 offset;
        float angle;
        float radius;
        float coneAngle;
    };

    struct AttackData
    {
        float startVel;
        Array<AttackFrameData> keyFrames;
        Array<AttackHitData> hitFrames;
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

    void SendPlayerInput(const SmartPtr<PlayerInputs> &playerInputs);
    void SendPlayerState(const SmartPtr<PlayerState> &playerState);

    void Update(uint32_t step);

    Type GetType() const;
    bool HasChanged() const;

    void GetCurrentPosition(float *x, float *y) const;
    void GetCurrentDirection(float *dx, float *dy) const;
    void GetCurrentState(ActionState *state, float *time) const;

    void GetStateAtTime(float t, float *x, float *y, float *dx, float *dy, ActionState *state, float *time) const;

    void FillPlayerState(const SmartPtr<PlayerState> &playerState);
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
