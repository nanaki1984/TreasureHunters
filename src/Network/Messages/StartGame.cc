#include "Network/Messages/StartGame.h"

namespace Network {
    namespace Messages {

DefineClassInfoWithFactoryAndFCC(Network::Messages::StartGame, 'STRM', Network::Serializable);
DefineSerializable(Network::Messages::StartGame);

StartGame::StartGame()
{ }

StartGame::~StartGame()
{ }

    } // namespace Messages
} // namespace Network
