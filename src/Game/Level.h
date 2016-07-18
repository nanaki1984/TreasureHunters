#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Array_type.h"
#include "Network/GameRoomData.h"

namespace Game {

using Core::Collections::Array;

class Level : public Core::RefCounted {
    DeclareClassInfo;
protected:
    uint8_t userPlayerId;
    Array<SmartPtr<Player>> players;
    Array<SmartPtr<Enemy>> enemies;
public:
    Level();
    Level(const Level &other) = delete;
    virtual ~Level();

    Level& operator =(const Level &other) = delete;

    void Init(const SmartPtr<Network::GameRoomData> &roomData);
    void Init(const SmartPtr<Network::GameRoomData> &roomData, uint8_t clientPlayerId);
    void DeletePlayer(uint8_t playerId);
    void Update(uint32_t simStep);

    const SmartPtr<Player>& GetPlayer(uint8_t playerId) const;
    const SmartPtr<Player>* PlayersBegin() const;
    const SmartPtr<Player>* PlayersEnd() const;

    const SmartPtr<Enemy>& GetEnemy(uint8_t enemyId) const;
    const SmartPtr<Enemy>* EnemiesBegin() const;
    const SmartPtr<Enemy>* EnemiesEnd() const;

    void GetEnemiesInRange(float t, float x, float y, float angle, float radius, float coneAngle, Array<SmartPtr<Enemy>> &list) const;

    void EnqueueAttack(const SmartPtr<Player> &attacker, uint32_t simStep, const Player::AttackHitData &hitData);
};

inline const SmartPtr<Player>&
Level::GetPlayer(uint8_t playerId) const
{
    return players[playerId];
}

inline const SmartPtr<Player>*
Level::PlayersBegin() const
{
    return players.Begin();
}

inline const SmartPtr<Player>*
Level::PlayersEnd() const
{
    return players.End();
}

inline const SmartPtr<Enemy>&
Level::GetEnemy(uint8_t enemyId) const
{
    return enemies[enemyId];
}

inline const SmartPtr<Enemy>*
Level::EnemiesBegin() const
{
    return enemies.Begin();
}

inline const SmartPtr<Enemy>*
Level::EnemiesEnd() const
{
    return enemies.End();
}

} // namespace Game
