#include "WarGame.h"
#include "StateManager.h"

#include "cinder/app/AppBasic.h"

#include "boost/unordered_set.hpp"

#include <string>
#include <sstream>

using namespace ci;
using namespace ci::app;
using namespace netphy;

using std::string;
using std::vector;
using boost::shared_ptr;
using boost::unordered_set;

HexGrid::HexGrid(double xspacing, double yspacing) 
    : mXSpacing(xspacing), mYSpacing(yspacing) { }

void HexGrid::setSpacing(double xspacing, double yspacing) 
{
    mXSpacing = xspacing;
    mYSpacing = yspacing;
}

HexCoord HexGrid::WorldToHex(Vec3f worldPos)
{
    double x = worldPos.x / mXSpacing;
    double y = worldPos.y / mYSpacing;
    double z = -0.5*x - y;
           y = y - 0.5*x;

    int ix = static_cast<int>(floor(x+0.5));
    int iy = static_cast<int>(floor(y+0.5));
    int iz = static_cast<int>(floor(z+0.5));
    int s = ix+iy+iz;
    if (s)
    {
        double abs_dx = fabs(ix-x);
        double abs_dy = fabs(iy-y);
        double abs_dz = fabs(iz-z); 
        if (abs_dx >= abs_dy && abs_dx >= abs_dz)
            ix -= s;
        else if (abs_dy >= abs_dx && abs_dy >= abs_dz)
            iy -= s;
        else
            iz -= s;
    }
    iy = (((s = iy - iz) < 0) ? s - 1 + ((ix+1) & 1) : s + 1 - (ix & 1)) / 2;

    return HexCoord(ix, iy);
}

Vec3f HexGrid::HexToWorld(HexCoord hexPos, bool scale)
{
    float x = hexPos.x * float(scale ? mXSpacing : 1.0f);
    float yoffset = -0.5f * ((hexPos.x & 1) ? hexPos.x-1 : hexPos.x);
    float y = (hexPos.y + 0.5f * hexPos.x + yoffset) * float(scale ? mYSpacing : 1.0f);
    return Vec3f(x, y, 0);
}

//  Returns neighbours in order nw, n, ne, se, s, sw
HexAdjacent HexGrid::adjacent(HexCoord pos)
{
    HexAdjacent result;

    if (pos.x % 2) {
        // odd column
        result.nw = HexCoord(pos.x-1, pos.y+1);
        result.n  = HexCoord(pos.x, pos.y+1);
        result.ne = HexCoord(pos.x+1, pos.y+1);
        result.se = HexCoord(pos.x+1, pos.y);
        result.s  = HexCoord(pos.x, pos.y-1);
        result.sw = HexCoord(pos.x-1, pos.y);
    }
    else {
        // even column
        result.nw = HexCoord(pos.x-1, pos.y);
        result.n  = HexCoord(pos.x, pos.y+1);
        result.ne = HexCoord(pos.x+1, pos.y);
        result.se = HexCoord(pos.x+1, pos.y-1);
        result.s  = HexCoord(pos.x, pos.y-1);
        result.sw = HexCoord(pos.x-1, pos.y-1);
    }

    return result;
}

vector<HexCoord> HexAdjacent::toVector()
{
    vector<HexCoord> ret;
    ret.push_back(nw);
    ret.push_back(n);
    ret.push_back(ne);
    ret.push_back(se);
    ret.push_back(s);
    ret.push_back(sw);
    return ret;
}

string HexAdjacent::toString()
{
    std::stringstream ss;
    ss << "nw " << nw << " n " << n << " ne " << ne;
    ss << " sw " << sw << " s " << s << " se " << se;

    return ss.str();
}


HexCell::HexCell()
{
    mLand = 0;
}

HexCell::~HexCell()
{ 
}

void HexCell::setPos(HexCoord& pos) 
{
    mPos = pos;
}

void HexCell::setColor(Color& color) 
{
    mColor = color;
}

Color& HexCell::getColor()
{
    return mColor;
}

int HexCell::getLand()
{
    return mLand;
}

void HexCell::setLand(int land)
{
    mLand = land;
}

int HexCell::getOwner()
{
    return mOwner;
}

void HexCell::setOwner(int id)
{
    mOwner = id;
}

HexMap::HexMap(HexGrid& grid, int width, int height) : mHexGrid(grid)
{ 
    mSize.x = width;
    mSize.y = height;

    mCells = new HexCell*[width];
    for (int i=0; i < width; ++i ) {
        mCells[i] = new HexCell[height];
        for (int j=0; j < height; ++j) {
            HexCell& cell = at(HexCoord(i,j));
            cell.setPos(HexCoord(i,j));
            cell.setColor(Color(0.15f, 0.15f, 0.15f));
        }
    }
}

HexMap::~HexMap()
{
    for (int i=0; i < mSize.x; ++i) {
        delete [] mCells[i];
    }
}

HexCell& HexMap::at(HexCoord& pos)
{
    assert(pos.x >=0 && pos.x < mSize.x && pos.y >= 0 && pos.y < mSize.y);
    return mCells[pos.x][pos.y];
}

Vec2i HexMap::getSize()
{
    return mSize;
}

//  Check position lies on hex map
bool HexMap::isValid(HexCoord& pos)
{
    return (pos.x >= 0 && pos.y >= 0 && pos.x < mSize.x && pos.y < mSize.y);
}

vector<HexCoord> HexMap::connected(HexCoord pos)
{
    unordered_set<HexCoord> search;
    unordered_set<HexCoord> checked;
    std::vector<HexCoord> result;

    int owner = at(pos).getOwner();
    search.emplace(pos);
    while (search.begin() != search.end()) {
        HexCoord check = *(search.begin());
        if (at(check).getOwner() == owner) {
            result.push_back(check);            
            vector<HexCoord> adjacent = mHexGrid.adjacent(check).toVector();
            for (vector<HexCoord>::iterator it = adjacent.begin(); it != adjacent.end(); ++it) {
                if (isValid(*it) && checked.find(*it) == checked.end() && at(*it).getLand()) {
                    search.emplace(*it);
                }
            }
        }
        checked.emplace(check);
        search.erase(check);
    }

    return result;
}

void HexRegion::addHexes(vector<HexCoord>& hexes)
{
    for (vector<HexCoord>::iterator it=hexes.begin(); it != hexes.end(); ++it) {
        mHexes.push_back(*it);
    }
}

std::vector<HexRegion> HexMap::regions()
{
    unordered_set<HexCoord> search;
    unordered_set<HexCoord> checked;
    vector<HexRegion> regions;

    HexCoord pos;
    for (int ix=0; ix < mSize.x; ++ix) {
        for (int iy=0; iy < mSize.y; ++iy) {
            pos = HexCoord(ix, iy);
            if (at(pos).getLand()) {
                search.emplace(pos);
            }
        }
    }

    while (search.begin() != search.end()) {
        HexCoord check = *(search.begin());

        vector<HexCoord> conn = connected(check);
        for (vector<HexCoord>::iterator it = conn.begin(); it != conn.end(); ++it) {
            search.erase(*it);
        }

        HexRegion region;
        region.addHexes(conn);
        regions.push_back(region);        

        //  XXX not required, already erased above
        search.erase(check);
    }

    return regions;
}

//vector<int> HexMap::countHexes()
//{
//    vector<int> counts();
//
//    for (int ix=0; ix < mSize.x; ++ix) {
//        for (int iy=0; iy < mSize.y; ++iy) {
//            HexCell& cell = at(ix, iy);
//            if (cell.getLand()) {
//            }
//        }
//    }
//}

Player::Player(int id, const string& name, Color& color)
    : mObj(new Obj(id, name, color))
{
}

Player::~Player()
{
}

WarGame::WarGame()
{
}

WarGame::~WarGame()
{
}

void WarGame::addPlayer(const string& name)
{
    float colors[5][3] = { { 1.0f, 0, 0 },
        { 1.0f, 1.0f, 0 },
        { 0, 0.8f, 0 },
        { 1.0f, 0.6f, 0.2f },
        { 0.2f, 0.5f, 0.7f }
    };

    int playerCount = mPlayers.size();
    float* pcolor = colors[playerCount % 5];
    mPlayers.push_back(Player(playerCount, name, Color(pcolor[0], pcolor[1], pcolor[2])));
}

vector<Player>& WarGame::getPlayers()
{
    return mPlayers;
}

HexRender::HexRender(HexMap& map)
    : mHexMap(map), mHexGrid(map.hexGrid())
{
}

HexRender::~HexRender()
{
}

void HexRender::setup(Vec2i wsize)
{
    mWindowSize = wsize;
    mHexGrid.setSpacing(1.5, 1.732050807);

    // gl::enableDepthRead();
    gl::disableDepthRead();
    // gl::enableDepthWrite();
	gl::enableAlphaBlending();

    generateMeshes();

    mCamera.setAspectRatio((float) mWindowSize.x / mWindowSize.y);
	mCamera.lookAt( Vec3f( 0, 0, 30.0f ), Vec3f::zero() );
    mCameraTo = mCamera.getEyePoint();

    gl::Texture::Format format;
    format.setInternalFormat(GL_RGBA_FLOAT32_ATI);
}

void HexRender::generateMeshes()
{
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    // XXX switch to 4-triangle hexes
    mHexMesh = gl::VboMesh(7, 8, layout, GL_TRIANGLE_FAN);

    vector<uint32_t> indices;
    vector<Vec3f>    positions;

    for (int i=0; i < 7; ++i) {
        indices.push_back(i);
        positions.push_back(i == 0 ? Vec3f(0, 0, 0) : Vec3f(float(cos(i*M_PI/3)), float(sin(i*M_PI/3)), 0));
    }
    indices.push_back(1);

    mHexMesh.bufferIndices( indices );
    mHexMesh.bufferPositions( positions );

    mHexOutlineMesh = gl::VboMesh( 7, 6, mHexMesh.getLayout(), GL_LINE_LOOP, NULL, &mHexMesh.getStaticVbo(), NULL );
    indices.clear();
    assert(indices.empty());
    for (int i=1; i < 1+6; ++i) {
        indices.push_back(i);        
    }
    mHexOutlineMesh.bufferIndices( indices );
}

Vec3f HexRender::raycastHexPlane(float u, float v)
{
    // Calculate camera ray intersection with z=0 hexagon plane 
    Ray ray = mCamera.generateRay(u, v, mCamera.getAspectRatio());
    float t = -ray.getOrigin().z / ray.getDirection().z;
    Vec3f planeHit = ray.calcPosition(t);
    return planeHit;
}


void HexRender::update()
{
    //Vec3f planeHit = raycastHexPlane(mMouseLocNorm.x, mMouseLocNorm.y);
    //mSelectedHex = mHexGrid.WorldToHex(planeHit);

    gl::setMatrices(mCamera);
    mTopRight = mHexGrid.WorldToHex(raycastHexPlane(1.0f, 1.0f));
    mBottomLeft = mHexGrid.WorldToHex(raycastHexPlane(0, 0));

    Vec3f dir = (mCameraTo - mCamera.getEyePoint()) * 0.05f;
    Vec3f eyePoint = mCamera.getEyePoint();
    eyePoint += dir;
    mCamera.setEyePoint(eyePoint);
}

void HexRender::drawHexes()
{
    Vec2i mapSize = mHexMap.getSize();

    for (int ix=mBottomLeft.x-1; ix <= mTopRight.x+1; ++ix) {
        for (int iy=mBottomLeft.y-1; iy <= mTopRight.y+1; ++iy) {

            ColorA cellColor;
            HexCoord loc(ix, iy);
            if (!mHexMap.isValid(loc)) {
                // cellColor = ColorA(0.2, 0.2, 0.2);
                continue;
            }
            else {
                cellColor = mHexMap.at(loc).getColor();
            }

            gl::pushMatrices();
            gl::color(cellColor);
            gl::translate(mHexGrid.HexToWorld(HexCoord(ix, iy)));
            gl::draw(mHexMesh);

            if (mHexMap.isValid(loc)) {
                gl::color(ColorA(0, 0, 0, 0.5));
                gl::draw(mHexOutlineMesh);
            }

            gl::popMatrices();
        }
    }
}

void HexRender::setCameraTo(Vec3f& cameraTo)
{
    mCameraTo = cameraTo;
}

Camera& HexRender::getCamera()
{
    return mCamera;
}

void HexRender::drawSelection()
{
    //  Draw highlighted hex
    if (mHexMap.isValid(mSelectedHex)) {
        glLineWidth(3.0f);
        gl::pushMatrices();
        gl::color(ColorA(1.0f, 1.0f, 0, 0.5f + 0.5f * float(abs(sin(2.5*app::getElapsedSeconds())))));
        gl::translate(mHexGrid.HexToWorld(mSelectedHex));
        gl::draw(mHexOutlineMesh);
        gl::popMatrices();
        glLineWidth(1.0f);
    }
}

void Mouse::mouseMove(MouseEvent event)
{
	mScreenPos = Vec2f(float(event.getX()), float(event.getY()));
    mPos = Vec2f(mScreenPos.x / float(mWindowSize.x), 1.0f - (mScreenPos.y / float(mWindowSize.y)));
}

void Mouse::mouseDown(MouseEvent event)
{
    if (event.isLeftDown()) {
        mLeft = (mLeft == PRESSED ? DOWN : PRESSED);
        if (mLeft == PRESSED) {
            // XXX store drag origin
        }
    }
    else if (event.isRightDown()) {
        mRight = (mRight == PRESSED ? DOWN : PRESSED);
            // XXX store drag origin
    }
}

void Mouse::mouseUp(MouseEvent event)
{
    if (event.isLeft()) {
        mLeft = UP;
    }
    else if (event.isRight()) {
        mRight = UP;
    }
}

void Mouse::mouseDrag(ci::app::MouseEvent event)
{
	mScreenPos = Vec2f(float(event.getX()), float(event.getY()));
    mPos = Vec2f(mScreenPos.x / float(mWindowSize.x), 1.0f - (mScreenPos.y / float(mWindowSize.y)));
}

void Mouse::mouseWheel(ci::app::MouseEvent event)
{
}

Shared::Shared(HexMap& hexmap, HexGrid& hexgrid, HexRender& hexrender, GuiController& gui, GuiFactory& factory, Mouse& mouse, WarGame& wargame, GuiConsolePtr console)
    : hexMap(hexmap), hexGrid(hexgrid), hexRender(hexrender), gui(gui), guiFactory(factory), mouse(mouse), warGame(wargame), console(console)
{
}


WargameServer::WargameServer()
{
}

WargameServer::~WargameServer()
{
}

WargameClient::WargameClient()
{
}

WargameClient::~WargameClient()
{
}