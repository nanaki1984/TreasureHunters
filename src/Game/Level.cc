#include "Game/Level.h"
#include "Core/Memory/MallocAllocator.h"
//#include "Game/Entity.h"
#include "Core/Pool/Pool.h"
//#include "Components/Transform.h"

using namespace Core::Memory;
//using namespace Components;

namespace Game {

Level::Level()
//: entities(GetAllocator<MallocAllocator>())
{ }

Level::~Level()
{ }
/*
Handle<Entity>
Level::NewEntity()
{
	Handle<Entity> newEntity = entities.NewInstance(this);
	newEntity->AddComponent<Transform>();
	return newEntity;
}

Handle<Entity>
Level::CloneEntity(const Handle<Entity> &source)
{
	return entities.CloneInstance(source);
}
*/
} // namespace Game
