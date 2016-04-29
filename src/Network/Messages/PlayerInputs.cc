#include "Game/PlayerInputs.h"

namespace Game {

DefineClassInfoWithFactoryAndFCC(Game::PlayerInputs, 'PLIN', Network::Serializable);
DefineSerializable(Game::PlayerInputs);

PlayerInputs::PlayerInputs()
{ }

PlayerInputs::~PlayerInputs()
{ }

} // namespace Game
