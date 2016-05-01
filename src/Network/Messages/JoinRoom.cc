#include "Network/Messages/JoinRoom.h"
#include "Core/Collections/Array.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::JoinRoom, 'JORM', Network::Serializable);
DefineSerializable(Network::Messages::JoinRoom);

JoinRoom::JoinRoom()
{ }

JoinRoom::~JoinRoom()
{ }

    } // namespace Messages
} // namespace Network
