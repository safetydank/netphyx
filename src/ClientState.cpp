#include "ClientState.h"
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

namespace io=boost::iostreams;

static Rand random;

//  Handle console input to server
struct ClientConsoleInput : public GuiCallbackGG
{
    RakPeerInterface* mClient;
    ClientConsoleInput(Shared& shared, RakPeerInterface* client) 
        : GuiCallbackGG(shared), mClient(client) { }

    bool operator()(string signal) {
        GuiConsoleOutput cout = GG.console->output();
        string input = GG.console->getInput();
        // cout << "Received command " << input << std::endl;
        // send message
        mClient->Send(input.c_str(), (int) input.length()+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
        return false;
    }
};

ClientState::ClientState(StateManager& manager, Shared& shared) : State(manager, shared)
{
}

ClientState::~ClientState()
{
}

void ClientState::enter()
{
    GG.console->clear();
    GG.gui.attach(GG.console);

    mClient = RakNetworkFactory::GetRakPeerInterface();
    int clientPort = 0;
	mSocketDesc = SocketDescriptorPtr(new SocketDescriptor(clientPort, 0));
    mClient->Startup(8, 30, mSocketDesc.get(), 1);
    mClient->SetOccasionalPing(true);
	bool b = mClient->Connect("127.0.0.1", SERVER_PORT, "Rumpelstiltskin", (int) strlen("Rumpelstiltskin"));

    GuiConsoleOutput cout = GG.console->output();
    cout << "Wargame client" << endl;
    cout << "local IP: " << mClient->GetLocalIP(0) << endl;
    cout << "GUID: " << mClient->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS).ToString() << endl;

    if (b) {
		cout << "Attempting connection" << endl;
    }
	else {
		cout << "Bad connection attempt.  Terminating." << endl;
	}

    cout.flush();

    //  callbacks
    if (mClient) {
        GG.console->slot("textInput", GuiCallbackPtr(new ClientConsoleInput(GG, mClient)));
    }
}

void ClientState::leave()
{
    if (mClient) {
        mClient->Shutdown(300);
        RakNetworkFactory::DestroyRakPeerInterface(mClient);
        mClient = 0;
        mSocketDesc = SocketDescriptorPtr();
    }

    GG.console->resetSlot("textInput");
    GG.gui.detachAll();
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

void ClientState::update()
{
    GuiConsoleOutput cout = GG.console->output();

	Packet* p;
    unsigned char packetIdentifier;
	SystemAddress clientID=UNASSIGNED_SYSTEM_ADDRESS;
	char message[2048];

    for (p=mClient->Receive(); p; mClient->DeallocatePacket(p), p=mClient->Receive()) {
        RakNet::RakString incoming;
        RakNet::BitStream bs;

        packetIdentifier = GetPacketIdentifier(p);

        // Check if this is a network message packet
        switch (packetIdentifier)
        {
        case ID_DISCONNECTION_NOTIFICATION:
            // Connection lost normally
            cout << "ID_DISCONNECTION_NOTIFICATION\n";
            break;
        case ID_ALREADY_CONNECTED:
            // Connection lost normally
            cout << "ID_ALREADY_CONNECTED\n";
            break;
        case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            cout << "ID_INCOMPATIBLE_PROTOCOL_VERSION\n";
            break;
        case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
            cout << "ID_REMOTE_DISCONNECTION_NOTIFICATION\n"; 
            break;
        case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
            cout << "ID_REMOTE_CONNECTION_LOST\n";
            break;
        case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
            cout << "ID_REMOTE_NEW_INCOMING_CONNECTION\n";
            break;
        case ID_CONNECTION_BANNED: // Banned from this server
            cout << "We are banned from this server.\n";
            break;			
        case ID_CONNECTION_ATTEMPT_FAILED:
            cout << "Connection attempt failed\n";
            break;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
            // Sorry, the server is full.  I don't do anything here but
            // A real app should tell the user
            cout << "ID_NO_FREE_INCOMING_CONNECTIONS\n";
            break;
        case ID_MODIFIED_PACKET:
            // Cheater!
            cout << "ID_MODIFIED_PACKET\n";
            break;

        case ID_INVALID_PASSWORD:
            cout << "ID_INVALID_PASSWORD\n";
            break;

        case ID_CONNECTION_LOST:
            // Couldn't deliver a reliable packet - i.e. the other system was abnormally
            // terminated
            cout << "ID_CONNECTION_LOST\n";
            break;

        case ID_CONNECTION_REQUEST_ACCEPTED:
            // This tells the client they have connected
            cout << "ID_CONNECTION_REQUEST_ACCEPTED to " << p->systemAddress.ToString(true) << " with GUID " << p->guid.ToString() << endl;
            cout << "My external address is " << mClient->GetExternalID(p->systemAddress).ToString(true) << endl;
            break;

        case ID_START_GAME:
            bs = RakNet::BitStream(p->data, p->length, false);
            char packetTypeID;
            cout << "Start game packet received" << endl;
            bs.Read(packetTypeID);
            stringCompressor->DecodeString(&incoming, 256, &bs);
            cout << "String payload: " << incoming << endl;
            break;

        default:
            // It's a client, so just show the message
            cout << p->data;
            break;
        }
    }

}

void ClientState::draw()
{
    gl::clear( Color( 0.25f, 0.25f, 0.4f ) );
}

void ClientState::mouseWheel(MouseEvent event)
{
}

void ClientState::keyDown(ci::app::KeyEvent event)
{
    // Vec3f& cameraTo = GG.hexRender.getCameraTo();

    int keycode = event.getCode();
    if (keycode == app::KeyEvent::KEY_ESCAPE) {
        mManager.setActiveState("title");
    }
    else if (keycode == app::KeyEvent::KEY_BACKQUOTE) {
        // XXX should push console to the top of the gui widget list
        GG.gui.attach(GG.console);
    }
}


