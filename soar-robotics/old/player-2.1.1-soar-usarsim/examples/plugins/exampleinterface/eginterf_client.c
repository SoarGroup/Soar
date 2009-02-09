/*
 * This file implements a C client proxy for the interface defined in
 * eginterf_client.h. The following functions are essential, others depend on
 * the design of your interface:
 *
 * eginterf_create      Creates a proxy for the interface
 * eginterf_destroy     Destroys a proxy for the interface
 * eginterf_subscribe   Subscribes to a device that provides the interface
 * eginterf_unsubscribe Unsubscribes from a subscribed device
 * eginterf_putmsg      Called by the client library whenever there a data
 *                      message is received for this proxy
 */

#include <string.h>
#include <stdlib.h>
#include <libplayerc/playerc.h>
#include <libplayercore/error.h>

#include "example_interface.h"
#include "example_xdr.h"
#include "eginterf_client.h"

void eginterf_putmsg (eginterf_t *device, player_msghdr_t *header, uint8_t *data, size_t len);

eginterf_t *eginterf_create (playerc_client_t *client, int index)
{
	eginterf_t *device;

	device = (eginterf_t*) malloc (sizeof (eginterf_t));
	memset (device, 0, sizeof (eginterf_t));
	playerc_device_init (&device->info, client, PLAYER_EXAMPLE_CODE, index, (playerc_putmsg_fn_t) eginterf_putmsg);

	device->stuff_count = 0;
	device->stuff = NULL;
	device->value = 0;
	return device;
}

void eginterf_destroy (eginterf_t *device)
{
	playerc_device_term (&device->info);
	free (device);
}

int eginterf_subscribe (eginterf_t *device, int access)
{
	return playerc_device_subscribe (&device->info, access);
}

int eginterf_unsubscribe (eginterf_t *device)
{
	return playerc_device_unsubscribe (&device->info);
}

void eginterf_putmsg (eginterf_t *device, player_msghdr_t *header, uint8_t *data, size_t len)
{
	if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_EXAMPLE_DATA_EXAMPLE))
	{
		player_eginterf_data_t *stuffData = (player_eginterf_data_t *) data;
		assert(header->size > 0);
		if (device->stuff != NULL)
			free (device->stuff);
		if ((device->stuff = (double*) malloc (stuffData->stuff_count)) == NULL)
			printf ("Failed to allocate space to store stuff locally");
		else
			memcpy (device->stuff, stuffData->stuff, stuffData->stuff_count * sizeof(double));
		device->stuff_count = stuffData->stuff_count;
	}
	else
		printf ("skipping eginterf message with unknown type/subtype: %s/%d\n", msgtype_to_str(header->type), header->subtype);
}

int eginterf_cmd (eginterf_t *device, char value)
{
	player_eginterf_cmd_t cmd;
	memset (&cmd, 0, sizeof (player_eginterf_cmd_t));
	cmd.doStuff = value;

	return playerc_client_write (device->info.client, &device->info, PLAYER_EXAMPLE_CMD_EXAMPLE, &cmd, NULL);
}

int eginterf_req (eginterf_t *device, int blah)
{
	int result = 0;
	player_eginterf_req_t req;
	player_eginterf_req_t *rep;
	memset (&rep, 0, sizeof (player_eginterf_req_t));
	req.value = blah;
	if ((result = playerc_client_request (device->info.client, &device->info, PLAYER_EXAMPLE_REQ_EXAMPLE, &req, &rep)) < 0)
		return result;

	device->value = rep->value;
	player_eginterf_req_t_free(rep);
	return 0;
}
