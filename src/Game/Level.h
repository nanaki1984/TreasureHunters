#pragma once

#include "Core/RefCounted.h"
#include "Core/Collections/Array_type.h"
#include "Game/Player.h"
#include "Network/GameRoomData.h"

namespace Game {

using Core::Collections::Array;

class Level : public Core::RefCounted {
    DeclareClassInfo;
protected:
    uint8_t userPlayerId;
    Array<SmartPtr<Player>> players;
public:
    Level();
    Level(const Level &other) = delete;
    virtual ~Level();

    Level& operator =(const Level &other) = delete;

    void Init(const SmartPtr<Network::GameRoomData> &roomData);
    void Init(const SmartPtr<Network::GameRoomData> &roomData, uint8_t clientPlayerId);
    void DeletePlayer(uint8_t playerId);
    void Update(float simTime);

    const SmartPtr<Player>& GetPlayer(uint8_t playerId) const;
    const SmartPtr<Player>* PlayersBegin() const;
    const SmartPtr<Player>* PlayersEnd() const;
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

} // namespace Game
