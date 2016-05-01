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
    Array<SmartPtr<Player>> players;
public:
    Level(const SmartPtr<Network::GameRoomData> &roomData);
    Level(const Level &other) = delete;
    virtual ~Level();

    Level& operator =(const Level &other) = delete;
};

} // namespace Game
