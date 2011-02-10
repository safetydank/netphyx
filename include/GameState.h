#include "StateManager.h"
#include "GuiController.h"

namespace netphy
{

class GameState : public State
{
public:
    GameState(StateManager& manager, Shared& shared);
    ~GameState();

    void enter();
    void leave();
    void update();
    void draw();

    virtual void keyDown(ci::app::KeyEvent event);    

private:
    //  Camera drag panning
    bool mDragStart;
    ci::Vec2f mDragOrigin;
    ci::Vec2f mDragEyeOrigin;

    int mPlayer;
};

}
