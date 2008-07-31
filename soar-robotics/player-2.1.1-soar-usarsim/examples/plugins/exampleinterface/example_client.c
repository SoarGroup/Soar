/*
 * This simple client illustrates the use of a plugin interface. It subscribes
 * to a device providing the interface, then tests each message.
 *
 * The important point to note for the purposes of this example is the call to
 * playerc_add_xdr_ftable(). This function adds the XDR serialisation functions
 * to Player's internal table so that messages sent and received by this client
 * can be serialised correctly.
 */

#include <stdio.h>
#include <libplayerc/playerc.h>

#include "eginterf_client.h"

playerxdr_function_t* player_plugininterf_gettable (void);

int main(int argc, const char **argv)
{
	int i;
	playerc_client_t *client;
	eginterf_t *device;

	// Create a client and connect it to the server.
	client = playerc_client_create(NULL, "localhost", 6665);
	if (0 != playerc_client_connect(client))
	{
		printf ("Could not connect\n");
		return -1;
	}

	if (playerc_add_xdr_ftable (player_plugininterf_gettable (), 0) < 0)
		printf ("Could not add xdr functions\n");

	// Create and subscribe to a position2d device.
	device = eginterf_create(client, 0);
	if (eginterf_subscribe(device, PLAYER_OPEN_MODE) != 0)
	{
		printf ("Could not subscribe\n");
		return -1;
	}

	// Send a command, get back a data message
	printf("sending a command\n");
	eginterf_cmd (device, 5);
	playerc_client_read (client);
	for (i = 0; i < device->stuff_count; i++)
		printf ("Stuff[%d] = %f\n", i, device->stuff[i]);

	// Send a request
	if (eginterf_req (device, 3) < 0)
		printf ("Request failed\n");
	else
		printf ("Request succeeded, got value: %d\n", device->value);

	// Shutdown
	eginterf_unsubscribe(device);
	eginterf_destroy(device);
	playerc_client_disconnect(client);
	playerc_client_destroy(client);

	return 0;
}
