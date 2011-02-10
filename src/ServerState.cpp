#include <Box2D/Box2D.h>

#include "ServerState.h"
#include "WarGame.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "StringCompressor.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

#include <string>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace netphy;
using std::vector;
using std::string;
using std::endl;
using std::stringstream;

namespace io=boost::iostreams;

static Rand random;

//  Handle console input to server
struct ServerConsoleInput : public GuiCallbackGG
{
    ServerState& mState;
    ServerConsoleInput(Shared& shared, ServerState& state) : GuiCallbackGG(shared), mState(state) { }
    bool operator()(string signal) {
        string input = GG.console->getInput();
        if (input == ".start") {
            GuiConsoleOutput cout = GG.console->output();
            cout << "Received command " << input << std::endl;            

            // tell clients to start
            mState.sendStartGame();
        }
        else {
            stringstream ss;
            ss << "SERVER: " << GG.console->getInput() << std::endl; 
            mState.sendMessage(ss.str());
        }
        return false;
    }
};

ServerState::ServerState(StateManager& manager, Shared& shared) : State(manager, shared)
{
    mPhysics = shared_ptr<Physics>(new Physics(shared));
}

ServerState::~ServerState()
{
}

void ServerState::sendMessage(const string& msg)
{
    // mServer->Send(message, (const int) msg.length()+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);
    mServer->Send(msg.c_str(), (const int) msg.length()+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void ServerState::sendStartGame()
{
    RakNet::BitStream bs;
    bs.Write((MessageID)ID_USER_PACKET_ENUM);
    stringCompressor->EncodeString("START GAME", 256, &bs);
    //  XXX send to each connected player instead of broadcasting
    mServer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void ServerState::enter()
{
    // console setup
    GG.console->clear();
    GG.gui.attach(GG.console);

    // raknet
    mServer = RakNetworkFactory::GetRakPeerInterface();    
	mServer->SetIncomingPassword("Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
	mSocketDesc = SocketDescriptorPtr(new SocketDescriptor(SERVER_PORT, 0));
	bool b = mServer->Startup(4, 30, mSocketDesc.get(), 1);
	mServer->SetMaximumIncomingConnections(4);
    mServer->SetOccasionalPing(true);
    mServer->SetUnreliableTimeout(1000);

    GuiConsoleOutput cout = GG.console->output();
    cout << "netphyx server" << endl;
    cout << "local IP: " << mServer->GetLocalIP(0) << endl;
    cout << "GUID: " << mServer->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS).ToString() << endl;

    DataStructures::List<RakNetSmartPtr<RakNetSocket> > sockets;
	mServer->GetSockets(sockets);
	cout << "Ports used by RakNet:\n";
	for (unsigned int i=0; i < sockets.Size(); i++)	{
        cout << i+1 << " " << sockets[i]->boundAddress.port << endl;
	}

    cout.flush();

    // console callback invoked on text input
    GG.console->slot("textInput", GuiCallbackPtr(new ServerConsoleInput(GG, *this)));

    // network classes
    mGameClient = WargameClientPtr(new WargameClient());
    mGameServer = WargameServerPtr(new WargameServer());

    // physics world
    mPhysics->setup();
}

void ServerState::leave()
{
    if (mServer) {
        mServer->Shutdown(300);
        RakNetworkFactory::DestroyRakPeerInterface(mServer);
        mServer = 0;
        mSocketDesc = SocketDescriptorPtr();
    }

    GG.console->resetSlot("textInput");
    GG.gui.detachAll();

    //  Release network classes
    mGameClient = WargameClientPtr();
    mGameServer = WargameServerPtr();
}

static unsigned char GetPacketIdentifier(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p->data[0];
}

void ServerState::update()
{
    mPhysics->update(0.16666f);

    io::stream<GuiConsoleStream> cout = GG.console->output();

	Packet* p;
    unsigned char packetIdentifier;
	SystemAddress clientID=UNASSIGNED_SYSTEM_ADDRESS;
	char message[2048];
    memset(message, 0, sizeof(message));

    for (p=mServer->Receive(); p; mServer->DeallocatePacket(p), p=mServer->Receive()) {
        mAddress = p->systemAddress;
        packetIdentifier = GetPacketIdentifier(p);

        switch (packetIdentifier)
        {
        case ID_DISCONNECTION_NOTIFICATION:
            // Connection lost normally
            cout << "ID_DISCONNECTION_NOTIFICATION from " << p->systemAddress.ToString(true) << endl;
            break;

        case ID_NEW_INCOMING_CONNECTION:
            // Somebody connected.  We have their IP now
            cout << "ID_NEW_INCOMING_CONNECTION from " << p->systemAddress.ToString(true) << " with GUID " << p->guid.ToString() << endl;
            clientID=p->systemAddress; // Record the player ID of the client
            break;

        case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            cout << "ID_INCOMPATIBLE_PROTOCOL_VERSION\n";
            break;

        case ID_MODIFIED_PACKET:
            // Cheater!
            cout << "ID_MODIFIED_PACKET\n";
            break;

        case ID_CONNECTION_LOST:
            // Couldn't deliver a reliable packet - i.e. the other system was abnormally
            // terminated
            cout << "ID_CONNECTION_LOST from " << p->systemAddress.ToString(true) << endl;
            break;

        default:
            // The server knows the static data of all clients, so we can prefix the message
            // With the name data
            cout << p->data << std::endl;

            // Relay the message.  We prefix the name for other clients.  This demonstrates
            // That messages can be changed on the server before being broadcast
            // Sending is the same as before
            sprintf(message, "%s\n", p->data);
            mServer->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);
            break;
        }
    }

    cout.flush();
}

void ServerState::draw()
{
    gl::clear( Color( 0.25f, 0.4f, 0.25f ) );
    mPhysics->draw();
}

void ServerState::mouseWheel(MouseEvent event)
{
}

void ServerState::keyDown(ci::app::KeyEvent event)
{
    static int count = 0;
    ++count;

    // Vec3f& cameraTo = GG.hexRender.getCameraTo();
    io::stream<GuiConsoleStream> cout = GG.console->output();

    int keycode = event.getCode();
    if (keycode == app::KeyEvent::KEY_ESCAPE) {
        mManager.setActiveState("title");
    }
    else if (keycode == app::KeyEvent::KEY_BACKQUOTE) {
        // XXX should push console to the top of the gui widget list
        GG.gui.attach(GG.console);
    }
    else if (keycode == app::KeyEvent::KEY_SPACE) {
        cout << "That's what I said: " << count << std::endl;
    }
}


