#pragma once

#include "boost/smart_ptr.hpp"
#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"

#include <map>
#include <string>

namespace netphy {

struct Shared;
typedef boost::shared_ptr<Shared> SharedPtr;

class StateManager;

class State
{
public:
    State(StateManager& manager, Shared& shared) : mManager(manager), GG(shared) { }

    virtual void enter() = 0;
    virtual void leave() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;

    //  Optional event handlers
    virtual void keyDown(ci::app::KeyEvent event) {}
    virtual void mouseMove(ci::app::MouseEvent event) {}
    virtual void mouseDown(ci::app::MouseEvent event) {}
    virtual void mouseUp(ci::app::MouseEvent event) {}
    virtual void mouseDrag(ci::app::MouseEvent event) {}
    virtual void mouseWheel(ci::app::MouseEvent event) {}

protected:
    StateManager& mManager;
    Shared& GG;
};
typedef boost::shared_ptr<State> StatePtr;

class StateManager
{
public:
    StateManager(SharedPtr share);
    ~StateManager();

    void setupStates();
    void setActiveState(std::string stateName);
    StatePtr getActiveState() { return mActiveState; }

    void update();
    void draw();

private:
    std::map<std::string, StatePtr> mStates;
    StatePtr mActiveState;
    SharedPtr GG;
};
typedef boost::shared_ptr<StateManager> StateManagerPtr;

}
