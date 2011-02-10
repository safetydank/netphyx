#ifndef CLIENTSTATE_H
#define CLIENTSTATE_H

#include "WarGame.h"
#include "StateManager.h"
#include "GuiController.h"

//  forward declarations
class RakPeerInterface;
struct SocketDescriptor;
typedef boost::shared_ptr<SocketDescriptor> SocketDescriptorPtr;

namespace netphy
{

class ClientState : public State
{
public:
    ClientState(StateManager& manager, Shared& shared);
    ~ClientState();

    void enter();
    void leave();
    void update();
    void draw();

    virtual void keyDown(ci::app::KeyEvent event);
    virtual void mouseWheel(ci::app::MouseEvent event);

private:
    RakPeerInterface*   mClient;
    SocketDescriptorPtr mSocketDesc;

    // Gui
    GuiLabelWidgetPtr mLabel;
    WargameClientPtr mGameClient;
};

}

#endif
