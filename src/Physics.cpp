#include "Physics.h"
#include "GuiController.h"

#include <string>

using namespace netphy;
using std::endl;

Physics::Physics(Shared& shared) : GG(shared)
{
}

Physics::~Physics()
{
}

void Physics::setup()
{
    b2Vec2 gravity(0, -10.0f);
    mWorld = shared_ptr<b2World>(new b2World(gravity, true));

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);

    b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);

	// Define the ground box shape.
	b2PolygonShape groundBox;

	// The extents are the half-widths of the box.
	groundBox.SetAsBox(50.0f, 10.0f);

	// Add the ground fixture to the ground body.
	groundBody->CreateFixture(&groundBox, 0.0f);

	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(0.0f, 4.0f);
    mBody = mWorld->CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(1.0f, 1.0f);

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = 1.0f;

	// Override the default friction.
	fixtureDef.friction = 0.3f;

	// Add the shape to the body.
	mBody->CreateFixture(&fixtureDef);
}

void Physics::update(float dt)
{
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

    mWorld->Step(timeStep, velocityIterations, positionIterations);
    mWorld->ClearForces();
}

void Physics::draw()
{
    b2Vec2 position = mBody->GetPosition();
    float32 angle = mBody->GetAngle();
    GuiConsoleOutput cout = GG.console->output();
    cout << position.x << position.y << angle << endl;
}

