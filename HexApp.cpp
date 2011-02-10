#include "cinder/app/AppBasic.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Text.h"
#include "cinder/Display.h"
#include "cinder/Rand.h"

#include "helper.h"
#include "GuiController.h"
#include "Resources.h"

#include "WarGame.h"
#include "StateManager.h"

#include <cassert>

using namespace ci;
using namespace ci::app;
using std::string;

using std::vector;
using namespace netphy;

#define BUFLEN 2
class DebugConsole
{
private:    
    ci::gl::Texture mTexture;
    std::string mBuffer[BUFLEN];
    int mLine;
    bool mDirty;

public:
    DebugConsole() : mLine(0), mDirty(true) {}
    
    void update() 
    {
        if (mDirty) {
            TextLayout layout;
            layout.setFont(Font("Droid Sans", 20));
            layout.setColor(Color(1.0f, 1.0f, 0));
            for (int i=0; i < BUFLEN; ++i) {
                if (!mBuffer[i].empty()) {
                    layout.addLine(mBuffer[i]);
                }
            }
            mTexture = gl::Texture(layout.render(true));
            mTexture.unbind();
        }
    }

    void print(const std::string& input) {
        mBuffer[mLine++] = input;
        mLine %= BUFLEN;
        mDirty = true;
    }

    void draw() 
    {
        float x = 0;
        float y = 0;
        glColor4f(1, 1, 1, 1);
        mTexture.bind();
        gl::draw(mTexture, Vec2f(x, y));
        mTexture.unbind();
    }
};

class HexApp : public AppBasic
{
public:
    void prepareSettings(Settings *settings);
    void setup();
    void update();
    void draw();

    void keyDown(KeyEvent event);
    void mouseMove(MouseEvent event);
    void mouseDown(MouseEvent event);
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseWheel(MouseEvent event);

    GuiController mGui;
    GuiFactoryPtr mFactory;
    GuiConsolePtr mConsole;

    MousePtr      mMouse;

    //  Game stuff
    HexGrid       mHexGrid;
    HexMapPtr     mHexMap;
    HexRenderPtr  mHexRender;

    WarGame          mWarGame;
    StateManagerPtr  mStateManager;
};

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

void HexApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    settings->setFrameRate(60.0f);
}

void HexApp::setup()
{
    Vec2i wsize(getWindowSize());

    mHexMap    = HexMapPtr(new HexMap(mHexGrid, 64, 32));
    mHexRender = HexRenderPtr(new HexRender(*mHexMap));
    mHexRender->setup(wsize);
    mMouse = MousePtr(new Mouse(wsize));
    mFactory = GuiFactoryPtr(new GuiFactory(mGui));
    mConsole = mFactory->createConsole(20, 15.0f);

    //  Set console to full window width
    mConsole->setWidth(mMouse->getWindowSize().x);

    SharedPtr shared(new Shared(*mHexMap, mHexMap->hexGrid(), *mHexRender, mGui, *mFactory, *mMouse, mWarGame, mConsole));

    //  XXX hack -- required to pass the Shared object to Gui callbacks
    mGui.setShared(shared);

    mStateManager = StateManagerPtr(new StateManager(shared));
    mStateManager->setActiveState("title");

    mWarGame.addPlayer("Dan");
    mWarGame.addPlayer("Abe");
    mWarGame.addPlayer("Tim");
    mWarGame.addPlayer("Mickey");
    mWarGame.addPlayer("Zen");
}

void HexApp::update()
{
    mStateManager->update();
    mGui.update();
}

void HexApp::draw()
{	
    mStateManager->draw();

    //  Draw gui
    glDisable(GL_LIGHTING);

    gl::enableAlphaBlending();
    gl::disableDepthRead();

    gl::pushMatrices();
    gl::setMatricesWindow(getWindowSize());
    mGui.draw();
    gl::popMatrices();
}

void HexApp::keyDown( KeyEvent event )
{
    // XXX GUI should intercept key events too
    if (!mGui.keyDown(event)) {
        mStateManager->getActiveState()->keyDown(event);
    }
}

void HexApp::mouseMove(MouseEvent event)
{
    mGui.mouseMove(event);
    mMouse->mouseMove(event);
    mStateManager->getActiveState()->mouseMove(event);
}

void HexApp::mouseDown(MouseEvent event)
{
    if (!mGui.mouseDown(event)) {
        mMouse->mouseDown(event);
        mStateManager->getActiveState()->mouseDown(event);
    }
}

void HexApp::mouseUp(MouseEvent event)
{
    if (!mGui.mouseUp(event)) {
        mMouse->mouseUp(event);
        mStateManager->getActiveState()->mouseUp(event);
    }
}

void HexApp::mouseDrag(MouseEvent event)
{
    mMouse->mouseDrag(event);
    mGui.mouseDrag(event);
    mStateManager->getActiveState()->mouseDrag(event);
}

void HexApp::mouseWheel(MouseEvent event)
{
    mMouse->mouseWheel(event);
    mGui.mouseWheel(event);
    mStateManager->getActiveState()->mouseWheel(event);
}

CINDER_APP_BASIC( HexApp, RendererGl )

