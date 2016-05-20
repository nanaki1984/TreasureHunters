#include "Game/Level.h"
#include "Core/SmartPtr.h"
#include "Core/Memory/MallocAllocator.h"
#include "Core/Memory/BlocksAllocator.h"
#include "Core/Collections/Array.h"
#include "Math/Math.h"
#include "Math/Vector2.h"

using namespace Core::Memory;
using namespace Math;

namespace Game {

DefineClassInfo(Game::Level, Core::RefCounted);

Level::Level()
: players(GetAllocator<MallocAllocator>()),
  enemies(GetAllocator<MallocAllocator>())
{ }

Level::~Level()
{ }

void
Level::Init(const SmartPtr<Network::GameRoomData> &roomData)
{
    uint8_t id = 0, count = roomData->playersData.Count();
    players.Reserve(count);
    for (; id < count; ++id)
        players.PushBack(SmartPtr<Player>::MakeNew<BlocksAllocator>(
            Player::SimulatedOnServer,
            roomData->playersData[id]));

    id = 0;
    count = roomData->enemiesData.Count();
    enemies.Reserve(count);
    for (; id < count; ++id)
        enemies.PushBack(SmartPtr<Enemy>::MakeNew<BlocksAllocator>(
            Enemy::SimulatedOnServer,
            roomData->enemiesData[id]));
}

void
Level::Init(const SmartPtr<Network::GameRoomData> &roomData, uint8_t clientPlayerId)
{
    uint8_t id = 0, count = roomData->playersData.Count();
    players.Reserve(count);
    for (; id < count; ++id)
        players.PushBack(SmartPtr<Player>::MakeNew<BlocksAllocator>(
            id == clientPlayerId ? Player::SimulatedLagless : Player::Cloned,
            roomData->playersData[id]));

    id = 0;
    count = roomData->enemiesData.Count();
    enemies.Reserve(count);
    for (; id < count; ++id)
        enemies.PushBack(SmartPtr<Enemy>::MakeNew<BlocksAllocator>(
            Enemy::Cloned,
            roomData->enemiesData[id]));
}

void
Level::DeletePlayer(uint8_t playerId)
{
    players.RemoveAt(playerId);
}

void
Level::Update(uint32_t simStep)
{
    auto plyIt = players.Begin(), plyEnd = players.End();
    for (; plyIt != plyEnd; ++plyIt)
        (*plyIt)->Update(simStep);

    auto enmIt = enemies.Begin(), enmEnd = enemies.End();
    for (; enmIt != enmEnd; ++enmIt)
        (*enmIt)->Update(simStep);
}

void
Level::GetEnemiesInRange(float t, float x, float y, float angle, float radius, float coneAngle, Array<SmartPtr<Enemy>> &list) const
{
    Vector2 p(x, y);
    auto enmIt = enemies.Begin(), enmEnd = enemies.End();
    for (; enmIt != enmEnd; ++enmIt)
    {
        Vector2 enmPos;
        (*enmIt)->GetPositionAtTime(t, &p.x, &p.y);

        Vector2 toEnemy = enmPos - p;
        float dist = toEnemy.Normalize();

        if (dist <= radius)
        {
            float aDiff = atan2f(toEnemy.y, toEnemy.x) - angle;
            if (std::abs(aDiff) <= coneAngle)
                list.PushBack((*enmIt));
        }
    }
}

} // namespace Game
