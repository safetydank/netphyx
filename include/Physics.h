#pragma once

#include <Box2D/Box2D.h>
#include "boost/smart_ptr.hpp"

//  XXX included just for Shared
#include "WarGame.h"

using boost::shared_ptr;

namespace netphy
{

//  The physics simulation
class Physics
{
public:
    Physics(Shared& shared);
    ~Physics();

    void setup();
    void update(float dt);
    void draw();

private:
    shared_ptr<b2World> mWorld;
    b2Body* mBody;
    Shared& GG;
};


}
