#include "Network/Messages/EnemyState.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::EnemyState, 'ENST', Network::Serializable);
DefineSerializable(Network::Messages::EnemyState);

EnemyState::EnemyState()
{ }

EnemyState::~EnemyState()
{ }

    } // namespace Messages
} // namespace Network
