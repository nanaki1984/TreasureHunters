#pragma once

#include "Core/RefCounted.h"
#include "Core/Pool/Pool_type.h"

using Core::Pool::Pool;

namespace Game {

//class Entity;

class Level : public Core::RefCounted {
    DeclareClassInfo;
protected:
    //Pool<Entity> entities;
public:
    Level();
    Level(const Level &other) = delete;
    virtual ~Level();

    Level& operator =(const Level &other) = delete;
/*
    Handle<Entity> NewEntity();
    Handle<Entity> CloneEntity(const Handle<Entity> &source);*/
};

} // namespace Game
