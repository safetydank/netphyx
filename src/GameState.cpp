#include "GameState.h"
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

GameState::GameState(StateManager& manager, Shared& shared) 
: State(manager, shared), mPlayer(0)
{
}

GameState::~GameState()
{
}

void GameState::enter()
{
}

void GameState::leave()
{
    GG.gui.detachAll();
}

void GameState::update()
{
    Vec2f mousePos = GG.mouse.getPos();
    Vec3f planeHit = GG.hexRender.raycastHexPlane(mousePos.x, mousePos.y);
    HexCoord selectedHex = GG.hexGrid.WorldToHex(planeHit);

    //  XXX copy-pasted from EditorState

    //  Pan the camera
    if (GG.mouse.getRight() == PRESSED) {
        if (!mDragStart) {
            mDragOrigin = mousePos;
            mDragEyeOrigin = Vec2f(GG.hexRender.getCamera().getEyePoint());
            mDragStart = true;
        }

        Vec2f stroke = (mousePos - mDragOrigin) * -48.0f;
        stroke.x *= GG.mouse.getAspectRatio();
        Vec2f dragTo = stroke + mDragEyeOrigin;
        Camera& camera = GG.hexRender.getCamera();
        Vec3f cameraTo(dragTo.x, dragTo.y, camera.getEyePoint().z);

        GG.hexRender.setCameraTo(cameraTo);
    }
    else {
        mDragStart = false;
    }
    
    GG.hexRender.update();
}

void GameState::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    GG.hexRender.drawHexes();
}

void GameState::keyDown(app::KeyEvent event)
{
    int keycode = event.getCode();

    if (keycode == app::KeyEvent::KEY_ESCAPE) {
        mManager.setActiveState("title");
    }
    else if (keycode == app::KeyEvent::KEY_SPACE) {
        mManager.setActiveState(string("editor"));
    }
}

