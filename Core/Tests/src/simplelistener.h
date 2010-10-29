#ifndef SIMPLE_LISTENER_H
#define SIMPLE_LISTENER_H

#include "sml_Client.h"

// Listen for remote commands and live
// for "life" 10'ths of a second (e.g. 10 = live for 1 second)
class SimpleListener 
{
public:
	SimpleListener( int life );

	int run(); // returns zero on success, nonzero failure

	static std::string MyClientMessageHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessageType, char const* pMessage);

private:
	int life;
	
	// Choose how to connect (usually use NewThread) but for
	// testing currentThread can be helpful.
	bool useCurrentThread;

	static bool shutdownMessageReceived;
};


#endif // SIMPLE_LISTENER_H
