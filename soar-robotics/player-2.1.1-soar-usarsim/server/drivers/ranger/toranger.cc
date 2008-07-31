/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

///////////////////////////////////////////////////////////////////////////
//
// Desc: Base class for ->ranger interface converter drivers.
// Author: Geoffrey Biggs
// Date: 06/05/2007
//
///////////////////////////////////////////////////////////////////////////

#include "toranger.h"

////////////////////////////////////////////////////////////////////////////////
//	Driver management
////////////////////////////////////////////////////////////////////////////////

// Constructor
// Nothing to do
ToRanger::ToRanger (ConfigFile* cf, int section)
	: Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_RANGER_CODE)
{
	inputDevice = NULL;
}

// Destructor
ToRanger::~ToRanger (void)
{
	if (deviceGeom.sensor_poses != NULL)
		delete deviceGeom.sensor_poses;
	if (deviceGeom.sensor_sizes != NULL)
		delete deviceGeom.sensor_sizes;
}

// Setup function
// Cleans up the output data for use
int ToRanger::Setup (void)
{
	// Clean output
	memset (&deviceGeom, 0, sizeof (deviceGeom));

	return 0;
}

// Shutdown function
// Ensures all the ranger data memory is freed
int ToRanger::Shutdown (void)
{
	if (deviceGeom.sensor_poses != NULL)
	{
		delete deviceGeom.sensor_poses;
		deviceGeom.sensor_poses = NULL;
	}
	if (deviceGeom.sensor_sizes != NULL)
	{
		delete deviceGeom.sensor_sizes;
		deviceGeom.sensor_sizes = NULL;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Message handling
////////////////////////////////////////////////////////////////////////////////

// Message processing
int ToRanger::ProcessMessage (QueuePointer &respQueue, player_msghdr *hdr, void *data)
{
	// Check for capabilities requests first
	HANDLE_CAPABILITY_REQUEST (device_addr, respQueue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ);

	// Pass property get/set messages through to the input device
	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_INTPROP_REQ, device_addr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_INTPROP_REQ, device_addr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_DBLPROP_REQ, device_addr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_DBLPROP_REQ, device_addr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_STRPROP_REQ, device_addr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_STRPROP_REQ, device_addr))
	{
		inputDevice->PutMsg (InQueue, hdr, data);
		ret_queue = respQueue;
		return 0;
	}
	// Pass responses to them back to the client
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_INTPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_INTPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_DBLPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_DBLPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_STRPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_STRPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_GET_INTPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SET_INTPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_GET_DBLPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SET_DBLPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_GET_STRPROP_REQ, inputDeviceAddr) ||
			Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SET_STRPROP_REQ, inputDeviceAddr))
	{
		hdr->addr = device_addr;
		Publish (ret_queue, hdr, data);
		return 0;
	}

	return -1;
}
