#include "Network/Messages/CreateRoom.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::CreateRoom, 'CRRM', Network::Serializable);
DefineSerializable(Network::Messages::CreateRoom);

CreateRoom::CreateRoom()
{ }

CreateRoom::~CreateRoom()
{ }

    } // namespace Messages
} // namespace Network
