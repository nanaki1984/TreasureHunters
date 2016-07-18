#include "Game/EntityState.h"

namespace Game {

DefineClassInfoWithFactoryAndFCC(Game::NetEntityState, 'ENST', Network::Serializable);
DefineSerializable(Game::NetEntityState);

NetEntityState::NetEntityState()
{ }

NetEntityState::~NetEntityState()
{ }

} // namespace Game
