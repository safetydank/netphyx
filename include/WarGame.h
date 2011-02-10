#pragma once

#include <string>
#include <vector>

#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Color.h"
#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"

#include "GuiController.h"

// XXX network packet types need a home
#include "MessageIdentifiers.h"
enum {
    ID_START_GAME = ID_USER_PACKET_ENUM,
};

namespace netphy {

typedef ci::Vec2i HexCoord;

//  Adjacency check results
struct HexAdjacent
{
    HexCoord nw;
    HexCoord n;
    HexCoord ne;
    HexCoord se;
    HexCoord s;
    HexCoord sw;

    std::vector<HexCoord> toVector();
    std::string toString();
};

/** 
  * A regular hexagon grid
  *
  * Models a hexagon grid as an isometric projection of cubes on the x+y+z=0 plane.
  * Based on http://www-cs-students.stanford.edu/~amitp/Articles/Hexagon2.html
  *
*/
class HexGrid 
{

private:
    double mXSpacing;
    double mYSpacing;

public:
    HexGrid(double xspacing=1.0, double yspacing=1.0);
    ~HexGrid() { };

    void setSpacing(double xspacing, double yspacing);

    HexCoord  WorldToHex(ci::Vec3f worldPos);
    ci::Vec3f HexToWorld(HexCoord hexPos, bool scale=true);

    HexAdjacent adjacent(HexCoord pos);
};

class HexCell
{
public:
    HexCell();
    ~HexCell();

    void setPos(HexCoord& pos);
    void setColor(ci::Color& color);
    ci::Color& getColor();
    int getLand();
    void setLand(int land);
    int getOwner();
    void setOwner(int id);

private:
    HexCoord  mPos;
    ci::Color mColor;
    int       mLand;  //  0 for sea, 1 for land
    int       mOwner; //  player owner ID
};

class HexRegion
{
private:
    std::vector<HexCoord> mHexes;

public:
    HexRegion() { }
    ~HexRegion() { }

    void addHexes(std::vector<HexCoord>& hexes);
    void clear();
};

class HexMap
{
private:
    HexGrid& mHexGrid;
    ci::Vec2i mSize;
    HexCell** mCells;

public:
    HexMap(HexGrid& grid, int width, int height);
    ~HexMap();

    HexCell& at(HexCoord& pos);
    ci::Vec2i getSize();

    //  Find all connected cells belonging to a player
    std::vector<HexCoord> connected(HexCoord pos);

    //  XXX should move to WarGame
    //std::vector<int> countHexes();

    //  Check position lies on hex map
    bool isValid(HexCoord& pos);
    HexGrid& hexGrid() { return mHexGrid; }
    std::vector<HexRegion> regions();

};
typedef boost::shared_ptr<HexMap> HexMapPtr;

class HexRender
{
private:
    ci::Vec2i         mWindowSize;
    ci::gl::VboMesh   mHexMesh;
    ci::gl::VboMesh   mHexOutlineMesh;
    ci::CameraPersp   mCamera;
    ci::Vec3f         mCameraTo;

    //  Frustum culling
    HexCoord         mBottomLeft;
    HexCoord         mTopRight;

    ci::gl::GlslProg  mShader;

    HexMap&  mHexMap;
    HexGrid& mHexGrid;

    HexCoord mSelectedHex;

    void generateMeshes();

public:
    HexRender(HexMap& map);
    ~HexRender(); 

    void setup(ci::Vec2i wsize);
    void update();
    
    void drawHexes();
    void drawSelection();

    ///  Cast a ray from camera projection plane (u,v) onto hex grid's plane
    ci::Vec3f raycastHexPlane(float u, float v);

    void setSelectedHex(HexCoord loc) { mSelectedHex = loc; }
    HexCoord getSelectedHex() { return mSelectedHex; }

    ci::Vec3f& getCameraTo() { return mCameraTo; }
    void setCameraTo(ci::Vec3f& cameraTo);

    ci::Camera& getCamera();
};
typedef boost::shared_ptr<HexRender> HexRenderPtr;

class Soldier
{
public:
    Soldier();
    ~Soldier();

private:
    int mLevel;
};

class Player
{
public:
    Player(int id, const std::string& name, ci::Color& color);
    ~Player();

    ci::Color& getColor() { return mObj->mColor; }

private:
    struct Obj {
        Obj(int playerID, const std::string& name, ci::Color color) :
            mPlayerID(playerID), mName(name), mColor(color) { };
        int    mPlayerID;
        std::string mName;
        ci::Color mColor;
    };

    boost::shared_ptr<Obj> mObj;

public:
 	//@{
	//! Emulates shared_ptr-like behavior
	Player( const Player &other ) { mObj = other.mObj; }	
	Player& operator=( const Player &other ) { mObj = other.mObj; return *this; }
	bool operator==( const Player &other ) { return mObj == other.mObj; }
    typedef boost::shared_ptr<Obj> Player::*unspecified_bool_type;
	operator unspecified_bool_type() { return ( mObj.get() == 0 ) ? 0 : &Player::mObj; }
	void reset() { mObj.reset(); }
	//@}	
};

//  New instance created when a game is started
class WarGame
{
public:
    WarGame();
    ~WarGame();

    void addPlayer(const std::string& name);
    std::vector<Player>& getPlayers();

    void update();
    void draw();

private:
    std::string mName;
    std::vector<Player> mPlayers;
    int mTurnPlayer;
};

typedef enum
{
    UP      = 0,
    PRESSED = 1,
    DOWN    = 2
} ButtonState;

///  Tracks mouse state from incoming events
///  XXX window size member must be updated for window resize to work
class Mouse
{
private:
    ci::Vec2f mWindowSize;
    ci::Vec2f mPos;
    ci::Vec2f mScreenPos;
    
    ButtonState mLeft;
    ButtonState mRight;

public:
    Mouse(ci::Vec2f wsize) : mWindowSize(wsize), mLeft(UP), mRight(UP) { }
    ci::Vec2f getPos() { return mPos; }
    ci::Vec2f getScreenPos() { return mScreenPos; }
    ButtonState getLeft() { return mLeft; }
    ButtonState getRight() { return mRight; }

    float getAspectRatio() { return (mWindowSize.x / mWindowSize.y); }
    ci::Vec2f getWindowSize() { return mWindowSize; }

    void mouseMove(ci::app::MouseEvent event);
    void mouseDown(ci::app::MouseEvent event);
    void mouseUp(ci::app::MouseEvent event);
    void mouseDrag(ci::app::MouseEvent event);
    void mouseWheel(ci::app::MouseEvent event);
};
typedef boost::shared_ptr<Mouse> MousePtr;

class StateManager;

//  Manages network input commands and updates game state (WarGame)
class WargameServer
{
public:
    WargameServer();
    ~WargameServer();

    void update();
    void draw();

private:
    std::string mServerName;
    std::vector<Player> mPlayers;
    int mTurnPlayer;
};
typedef boost::shared_ptr<WargameServer> WargameServerPtr;

//  Manage player input and sends network commands to server
class WargameClient
{
public:
    WargameClient();
    ~WargameClient();

    void update();
    void draw();

private:
    std::string mPlayerName;
    std::vector<Player> mPlayers;
};
typedef boost::shared_ptr<WargameClient> WargameClientPtr;

// shared data between states
struct Shared
{
    HexMap&        hexMap;
    HexGrid&       hexGrid;
    HexRender&     hexRender;
    GuiController& gui;
    GuiFactory&    guiFactory;
    Mouse&         mouse;
    WarGame&       warGame;

    GuiConsolePtr  console;   // XXX have to use a smart ptr here to attach/detach, 
                              // but is there a cyclic dependency between Shared & console?
                              // fix by not using pointers to attach/detach, generate uid's for widgets instead.
    Shared(HexMap& hexmap, HexGrid& hexgrid, HexRender& hexrender, GuiController& gui, GuiFactory& factory, Mouse& mouse, WarGame& wargame, GuiConsolePtr console);
};

}
