#include "Network/Messages/PlayerInputs.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::PlayerInputs, 'PLIN', Network::Serializable);
DefineSerializable(Network::Messages::PlayerInputs);

PlayerInputs::PlayerInputs()
{ }

PlayerInputs::~PlayerInputs()
{ }

    } // namespace Messages
} // namespace Network
