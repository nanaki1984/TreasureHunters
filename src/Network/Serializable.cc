#include "Network/Serializable.h"

namespace Network {

DefineClassInfoWithFactoryAndFCC(Network::Serializable, 'SERI', Core::RefCounted);
DefineSerializable(Network::Serializable);

Serializable::Serializable()
{ }

Serializable::~Serializable()
{ }

} // namespace Network
