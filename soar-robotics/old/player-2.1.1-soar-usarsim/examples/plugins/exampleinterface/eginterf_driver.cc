/*
 * This file implements a driver using the pluggable interface. See the plugin
 * driver example for details of how it works.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <libplayercore/playercore.h>

#include "example_interface.h"

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class EgInterfDriver : public Driver
{
	public:

		// Constructor; need that
		EgInterfDriver(ConfigFile* cf, int section);

		// Must implement the following methods.
		virtual int Setup();
		virtual int Shutdown();

		// This method will be invoked on each incoming message
		virtual int ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data);
};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver* EgInterfDriver_Init(ConfigFile* cf, int section)
{
	// Create and return a new instance of this driver
	return((Driver*)(new EgInterfDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void EgInterfDriver_Register(DriverTable* table)
{
	table->AddDriver("eginterfdriver", EgInterfDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
EgInterfDriver::EgInterfDriver(ConfigFile* cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_EXAMPLE_CODE)
{
	return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int EgInterfDriver::Setup()
{
	puts("EgInterfDriver initialising");

	srandom (time (NULL));

	puts("EgInterfDriver ready");

	return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int EgInterfDriver::Shutdown()
{
	puts("Shutting EgInterfDriver down");

	puts("EgInterfDriver has been shutdown");

	return(0);
}

int EgInterfDriver::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	player_eginterf_data resp;
	player_eginterf_req reqResp;

	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_EXAMPLE_CMD_EXAMPLE, device_addr))
	{
		printf ("EgInterfDriver: Received command: %d\n", reinterpret_cast<player_eginterf_cmd*> (data)->doStuff);
		if (reinterpret_cast<player_eginterf_cmd*> (data)->doStuff)
		{
			resp.stuff_count = reinterpret_cast<player_eginterf_cmd*> (data)->doStuff;
			resp.stuff = new double[reinterpret_cast<player_eginterf_cmd*> (data)->doStuff];
			printf ("EgInterfDriver: Sending data:\n");
			for (char ii = 0; ii < reinterpret_cast<player_eginterf_cmd*> (data)->doStuff; ii++)
			{
				double temp = (((double) random ()) / RAND_MAX) * 10.0f;
				resp.stuff[ii] = temp;
				printf ("\t%f\n", resp.stuff[ii]);
			}
			Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_EXAMPLE_DATA_EXAMPLE, &resp, sizeof (resp), NULL);
		}
		delete[] resp.stuff;
		return 0;
	}
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_EXAMPLE_REQ_EXAMPLE, device_addr))
	{
		printf ("EgInterfDriver: Got request: %d\n", reinterpret_cast<player_eginterf_req*> (data)->value);
		reqResp.value = RAND_MAX;
		printf ("EgInterfDriver: Sending response: %d\n", reqResp.value);
		Publish (device_addr,  PLAYER_MSGTYPE_RESP_ACK, PLAYER_EXAMPLE_REQ_EXAMPLE, &reqResp, sizeof (reqResp), NULL);
		return 0;
	}

	return(-1);
}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

extern "C" {
	int player_driver_init(DriverTable* table)
	{
		puts("EgInterfDriver initializing");
		EgInterfDriver_Register(table);
		puts("EgInterfDriver done");
		return(0);
	}
}

