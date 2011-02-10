#include "TitleState.h"
#include "WarGame.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

#include <string>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace netphy;
using std::vector;
using std::string;

static Rand random;

TitleState::TitleState(StateManager& manager, Shared& shared) : State(manager, shared)
{
}

TitleState::~TitleState()
{
}

struct StateActivate : public GuiCallback {
    StateManager& mStateManager;
    StateActivate(StateManager& mgr) : mStateManager(mgr) { }
};

struct EditorActivate : public StateActivate {
    EditorActivate(StateManager& manager) : StateActivate(manager) { }
    virtual bool operator()(string signal) {
        mStateManager.setActiveState("editor");
        return false;
    };
};

struct PlayActivate : public StateActivate {
    PlayActivate(StateManager& manager) : StateActivate(manager) { }
    virtual bool operator()(string signal) {
        mStateManager.setActiveState("game");
        return false;
    };
};

struct ServerActivate : public StateActivate {
    ServerActivate(StateManager& manager) : StateActivate(manager) { }
    virtual bool operator()(string signal) {
        mStateManager.setActiveState("netserver");
        return false;
    };
};

struct ClientActivate : public StateActivate {
    ClientActivate(StateManager& manager) : StateActivate(manager) { }
    virtual bool operator()(string signal) {
        mStateManager.setActiveState("netclient");
        return false;
    };
};

void TitleState::enter()
{
    GuiLabelData label;
    label.Text = "Wargame";
    label.FontSize = 32.0f;
    mTitle = GG.gui.createLabel(label);
    mTitle->setPos(Vec2f(400, 20));

    float fontSize = 18.0f;
    mPlayButton = GG.guiFactory.createTextButton("Play", fontSize, true);
    mPlayButton->setPos(Vec2f(400, 100));
    mPlayButton->slot("mouseClick", GuiCallbackPtr(new PlayActivate(mManager)));

    mEditorButton = GG.guiFactory.createTextButton("Editor", fontSize, true);
    mEditorButton->setPos(Vec2f(400, 150));
    mEditorButton->slot("mouseClick", GuiCallbackPtr(new EditorActivate(mManager)));

    mServerButton = GG.guiFactory.createTextButton("Server", fontSize, true);
    mServerButton->setPos(Vec2f(400, 200));
    mServerButton->slot("mouseClick", GuiCallbackPtr(new ServerActivate(mManager)));

    mClientButton = GG.guiFactory.createTextButton("Client", fontSize, true);
    mClientButton->setPos(Vec2f(400, 250));
    mClientButton->slot("mouseClick", GuiCallbackPtr(new ClientActivate(mManager)));
}

void TitleState::leave()
{
    GG.gui.detachAll();
}

void TitleState::update()
{
}

void TitleState::draw()
{
    gl::clear( Color( 0.16f, 0.4f, 0.16f ) );
}

void TitleState::mouseWheel(MouseEvent event)
{
}

void TitleState::keyDown(ci::app::KeyEvent event)
{
}


