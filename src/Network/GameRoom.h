#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/Pool/BaseObject.h"
#include "Core/Collections/Array_type.h"

namespace Network {

class GameRoom : public Core::Pool::BaseObject {
    DeclareClassInfo;
protected:
    Core::Collections::Array<ENetPeer*> peers;
public:
    GameRoom();
    virtual ~GameRoom();
};

}; // namespace Network
