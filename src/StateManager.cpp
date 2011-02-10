#include "StateManager.h"
#include "GameState.h"
#include "EditorState.h"
#include "TitleState.h"
#include "ServerState.h"
#include "ClientState.h"

#include "WarGame.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/ImageIO.h"
#include "cinder/Text.h"
#include "cinder/Display.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace netphy;
using std::string;

StateManager::StateManager(SharedPtr share) : GG(share)
{
    setupStates();
}

StateManager::~StateManager()
{
}

void StateManager::setupStates()
{
    mStates["editor"] = StatePtr(new EditorState(*this, *GG));
    mStates["game"]   = StatePtr(new GameState(*this, *GG));
    mStates["title"]  = StatePtr(new TitleState(*this, *GG));
    mStates["netserver"] = StatePtr(new ServerState(*this, *GG));
    mStates["netclient"] = StatePtr(new ClientState(*this, *GG));
}

void StateManager::setActiveState(std::string stateName)
{
    if (mActiveState) {
        mActiveState->leave();
    }
    mActiveState = mStates[stateName];
    mActiveState->enter();
}

void StateManager::update()
{
    mActiveState->update();
}

void StateManager::draw()
{
    mActiveState->draw();
}

