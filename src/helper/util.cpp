#include "helper.h"

using namespace ci;
using namespace std;

Capture getAvailableCapture(int width, int height)
{
    vector<Capture::Device> devices( Capture::getDevices() );

    Capture cap;
    for ( vector<Capture::Device>::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
        try {
            if( deviceIt->checkAvailable() ) {
                cap = Capture( width, height, *deviceIt);
            }
        }
        catch( CaptureExc & ) {
            //app::console() << "Unable to initialize device: " << deviceIt->getName() << endl;
            //cap.reset();
        }
    }

    return cap;
}
