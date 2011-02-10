#ifndef SERVERSTATE_H
#define SERVERSTATE_H

#include "WarGame.h"
#include "Physics.h"
#include "StateManager.h"
#include "GuiController.h"

#include "RakPeerInterface.h"

//  forward declarations
class RakPeerInterface;
struct SocketDescriptor;
typedef boost::shared_ptr<SocketDescriptor> SocketDescriptorPtr;

namespace netphy
{

class ServerState : public State
{
public:
    ServerState(StateManager& manager, Shared& shared);
    ~ServerState();

    void enter();
    void leave();
    void update();
    void draw();

    virtual void keyDown(ci::app::KeyEvent event);
    virtual void mouseWheel(ci::app::MouseEvent event);

    void sendMessage(const std::string& msg);
    void sendStartGame();

private:
    RakPeerInterface*   mServer;
    SocketDescriptorPtr mSocketDesc;

    // Physics game
    boost::shared_ptr<Physics> mPhysics;

    // Gui
    GuiLabelWidgetPtr mLabel;
    AddressOrGUID mAddress;
    // GuiConsolePtr mConsole;

    // Network comms
    WargameServerPtr mGameServer;
    WargameClientPtr mGameClient;
};

}

#endif
