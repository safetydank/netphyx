#include "EditorState.h"
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

EditorState::EditorState(StateManager& manager, Shared& shared) : State(manager, shared)
{
}

EditorState::~EditorState()
{
}

void EditorState::enter()
{
    GuiLabelData labelData;
    labelData.FontSize = 15.0f;
    mLabel = GG.gui.createLabel(labelData, false);

    GuiQuadData boxQuad;
    boxQuad.Color = ColorA(0, 0, 0, 0.8f);
    GuiBoxData debugLabelBox;
    debugLabelBox.Padding = 2.0f;

    mLabelBox = GG.gui.createBox(boxQuad, debugLabelBox, true);
    mLabelBox->addChild(mLabel);
}

void EditorState::leave()
{
    // mLabelBox->detach();
    GG.gui.detachAll();
}

void EditorState::update()
{
    Vec2f mousePos = GG.mouse.getPos();
    Vec3f planeHit = GG.hexRender.raycastHexPlane(mousePos.x, mousePos.y);

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

    //  Set land
    if (GG.mouse.getLeft() == PRESSED) {
        HexCoord selectedHex = GG.hexGrid.WorldToHex(planeHit);
        if (GG.hexMap.isValid(selectedHex)) {
            GG.hexMap.at(selectedHex).setColor(Color(1.0f, 1.0f, 0.8f));
            GG.hexMap.at(selectedHex).setLand(1);
        }
    }

    std::stringstream ss;
    HexCoord selectedHex = GG.hexGrid.WorldToHex(planeHit);
    GG.hexRender.setSelectedHex(selectedHex);
    ss << "Hex:" << selectedHex; // << " World: " << planeHit;

    GuiLabelData& labelData = mLabel->getData();
    labelData.Text = ss.str();

    GG.hexRender.update();
}

void EditorState::draw()
{
    gl::clear( Color( 0.3f, 0.3f, 0.3f ) );
    GG.hexRender.drawHexes();
    GG.hexRender.drawSelection();
}

void EditorState::mouseWheel(MouseEvent event)
{
    Vec3f cameraTo = GG.hexRender.getCameraTo();
    cameraTo.z = cameraTo.z - event.getWheelIncrement() * 5.0f;
    GG.hexRender.setCameraTo(cameraTo);
}

void EditorState::keyDown(ci::app::KeyEvent event)
{
    // XXX duplicate raycast
    Vec2f pos = GG.mouse.getPos();
    Vec3f planeHit = GG.hexRender.raycastHexPlane(pos.x, pos.y);
    HexCoord selectedHex = GG.hexGrid.WorldToHex(planeHit);

    int keycode = event.getCode();

    Vec3f& cameraTo = GG.hexRender.getCameraTo();
    if (keycode == app::KeyEvent::KEY_ESCAPE) {
        mManager.setActiveState("title");
    }
    else if (keycode == app::KeyEvent::KEY_UP) {
        cameraTo += Vec3f(0, 2.0f, 0);
    }
    else if (keycode == app::KeyEvent::KEY_DOWN) {
        cameraTo += Vec3f(0, -2.0f, 0);
    }   
    else if (keycode == app::KeyEvent::KEY_LEFT) {
        cameraTo += Vec3f(-2.0f, 0, 0);
    }
    else if (keycode == app::KeyEvent::KEY_RIGHT) {
        cameraTo += Vec3f(2.0f, 0, 0);
    }
    else if (keycode == app::KeyEvent::KEY_g) {
        //  Generate hex colors
        Vec2i mapSize = GG.hexMap.getSize();
        for (int iy=0; iy < mapSize.y; ++iy) {
            for (int ix=0; ix < mapSize.x; ++ix) {
                HexCell& cell = GG.hexMap.at(HexCoord(ix, iy));
                if (cell.getLand()) {
                    int playerID = random.nextInt(0, 5);
                    cell.setOwner(playerID);
                    cell.setColor(GG.warGame.getPlayers()[playerID].getColor());
                }
            }
        }
    }
    else if (keycode == app::KeyEvent::KEY_DELETE) {
        GG.hexMap.at(selectedHex).setLand(0);
    }
    else if (keycode == app::KeyEvent::KEY_SPACE) {
        mManager.setActiveState(string("game"));
    }
    else if (keycode == app::KeyEvent::KEY_c) {
        vector<HexCoord> connected = GG.hexMap.connected(selectedHex);
        for (vector<HexCoord>::iterator it = connected.begin(); it != connected.end(); ++it) {
            HexCell& cell = GG.hexMap.at(*it);
            Color cellColor = cell.getColor();
            cellColor.r = 0.5f * cellColor.r;
            cellColor.g = 0.5f * cellColor.g;
            cellColor.b = 0.5f * cellColor.b;
            cell.setColor(cellColor);
        }
    }
}


