#include "Game/PlayerState.h"

namespace Game {

DefineClassInfoWithFactoryAndFCC(Game::PlayerState, 'PLST', Network::Serializable);
DefineSerializable(Game::PlayerState);

PlayerState::PlayerState()
{ }

PlayerState::~PlayerState()
{ }

} // namespace Game
