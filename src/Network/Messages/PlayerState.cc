#include "Network/Messages/PlayerState.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::PlayerState, 'PLST', Network::Serializable);
DefineSerializable(Network::Messages::PlayerState);

PlayerState::PlayerState()
{ }

PlayerState::~PlayerState()
{ }

    } // namespace Messages
} // namespace Network
