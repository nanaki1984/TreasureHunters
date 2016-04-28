#include "Game/Player.h"
#include "Core/Collections/Queue.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/MallocAllocator.h"

using namespace Core::Memory;

namespace Game {

DefineClassInfo(Game::Player, Core::RefCounted);

Player::Player()
: inputs(GetAllocator<MallocAllocator>(), 4),
  states(GetAllocator<MallocAllocator>(), 4)
{
    Zero(&currentState);
    Zero(&currentInput);
}

Player::~Player()
{ }

void
Player::SetInput(float x, float y)
{
    currentInput.t = currentState.t;
    currentInput.x = x;
    currentInput.y = y;
}

void
Player::SetState(int t, float x, float y)
{
    currentState.t = t;
    currentState.px = x;
    currentState.py = y;
}

void
Player::Update()
{/*
    if (inputs.Count() == inputs.Capacity())
        inputs.PopFront();
    inputs.PushBack(currentInput);

    if (states.Count() == states.Capacity())
        states.PopFront();
    states.PushBack(currentState);
*/
    currentState.px += currentInput.x * 0.033f;
    currentState.py += currentInput.y * 0.033f;
    currentState.t++;

    Zero(&currentInput);
}

} // namespace Game
