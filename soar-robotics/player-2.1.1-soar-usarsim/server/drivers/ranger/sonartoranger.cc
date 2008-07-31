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
// Desc: Driver for converting sonar-interface devices to ranger-interface
//       devices
// Author: Geoffrey Biggs
// Date: 06/05/2007
//
// Requires - sonar device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_sonartoranger sonartoranger
 * @brief Sonar-to-Ranger converter

This driver translates data provided via the @ref interface_sonar interface into
the @ref interface_ranger interface.

@par Compile-time dependencies

- None

@par Provides

- @ref interface_ranger : Output ranger interface

@par Requires

- @ref interface_sonar : Sonar interface to translate

@par Configuration requests

- PLAYER_RANGER_REQ_GET_GEOM
- PLAYER_RANGER_REQ_POWER

@par Configuration file options

 - None

@par Example

@verbatim
driver
(
  name "p2os"
  provides ["sonar:0"]
)
driver
(
  name "sonartoranger"
  requires ["sonar:0"] # read from sonar:0
  provides ["ranger:0"] # output results on ranger:0
)
@endverbatim

@author Geoffrey Biggs

*/
/** @} */

#include <errno.h>
#include <string.h>

#include <libplayercore/playercore.h>

#include "toranger.h"

// Driver for computing the free c-space from a sonar scan.
class SonarToRanger : public ToRanger
{
	public:
		SonarToRanger (ConfigFile *cf, int section);

		int Setup (void);
		int Shutdown (void);

	protected:
		// Child message handler, for handling messages from the input device
		int ProcessMessage (QueuePointer &respQueue, player_msghdr *hdr, void *data);
		// Function called when a property has been changed so it can be passed on to the input driver
		bool PropertyChanged (void);
		// Set power state
		int SetPower (QueuePointer &respQueue, player_msghdr *hdr, uint8_t state);
		// Convert sonar geometry to ranger geometry
		int ConvertGeom (player_sonar_geom_t *geom);
		// Convert sonar data to ranger data
		int ConvertData (player_sonar_data_t *data);
};

// Initialisation function
Driver* SonarToRanger_Init (ConfigFile* cf, int section)
{
	return reinterpret_cast<Driver*> (new SonarToRanger (cf, section));
}

// Register function
void SonarToRanger_Register (DriverTable* table)
{
	table->AddDriver ("sonartoranger", SonarToRanger_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Driver management
////////////////////////////////////////////////////////////////////////////////

// Constructor
// Sets up the input sonar interface
SonarToRanger::SonarToRanger( ConfigFile* cf, int section)
	: ToRanger (cf, section)
{
	// Need a sonar device as input
	if (cf->ReadDeviceAddr(&inputDeviceAddr, section, "requires", PLAYER_SONAR_CODE, -1, NULL) != 0)
	{
		SetError (-1);
		return;
	}
}

// Setup function
int SonarToRanger::Setup (void)
{
	// First call the base setup
	if (ToRanger::Setup () != 0)
		return -1;

	// Subscribe to the sonar.
	if ((inputDevice = deviceTable->GetDevice (inputDeviceAddr)) == NULL)
	{
		PLAYER_ERROR ("Could not find input sonar device");
		return -1;
	}

	if (inputDevice->Subscribe (InQueue) != 0)
	{
		PLAYER_ERROR ("Could not subscribe to input sonar device");
		return -1;
	}

	// Request the sonar geometry
	inputDevice->PutMsg (InQueue, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_GET_GEOM, NULL, 0, NULL);

	return 0;
}

// Shutdown function
int SonarToRanger::Shutdown (void)
{
	// Unsubscribe from the sonar device
	inputDevice->Unsubscribe (InQueue);

	// Call the base shutdown function
	return ToRanger::Shutdown ();
}

////////////////////////////////////////////////////////////////////////////////
//  Message handling
////////////////////////////////////////////////////////////////////////////////

bool SonarToRanger::PropertyChanged (void)
{
	// No properties for sonar
	return true;
}

int SonarToRanger::ConvertGeom (player_sonar_geom_t *geom)
{
	double minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f, minZ = 0.0f, maxZ = 0.0f;

	// Prepare some space for storing geometry data - the parent class will clean this up when necessary
	if (deviceGeom.sensor_poses != NULL)
		delete deviceGeom.sensor_poses;
	if (deviceGeom.sensor_sizes != NULL)
		delete deviceGeom.sensor_sizes;
	memset (&deviceGeom, 0, sizeof (player_ranger_geom_t));
	if ((deviceGeom.sensor_poses = new player_pose3d_t[geom->poses_count]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for sensor poses");
		deviceGeom.sensor_poses = NULL;
		return 0;
	}
	memset (deviceGeom.sensor_poses, 0, sizeof (player_pose3d_t) * geom->poses_count);
	if ((deviceGeom.sensor_sizes = new player_bbox3d_t[geom->poses_count]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for sensor sizes");
		delete deviceGeom.sensor_sizes;
		deviceGeom.sensor_sizes = NULL;
		return 0;
	}
	memset (deviceGeom.sensor_sizes, 0, sizeof (player_bbox3d_t) * geom->poses_count);

	// Copy across the poses, making a note of the bounding box for all poses
	deviceGeom.sensor_poses_count = geom->poses_count;
	for (uint32_t ii = 0; ii < geom->poses_count; ii++)
	{
		minX = (geom->poses[ii].px < minX) ? geom->poses[ii].px : minX;
		maxX = (geom->poses[ii].px > maxX) ? geom->poses[ii].px : maxX;
		deviceGeom.sensor_poses[ii].px = geom->poses[ii].px;
		minY = (geom->poses[ii].py < minY) ? geom->poses[ii].py : minY;
		maxY = (geom->poses[ii].py > maxY) ? geom->poses[ii].py : maxY;
		deviceGeom.sensor_poses[ii].py = geom->poses[ii].py;
    minZ = (geom->poses[ii].pz < minZ) ? geom->poses[ii].pz : minZ;
    maxZ = (geom->poses[ii].pz > maxZ) ? geom->poses[ii].pz : maxZ;
    deviceGeom.sensor_poses[ii].pz = geom->poses[ii].pz;
    deviceGeom.sensor_poses[ii].proll = geom->poses[ii].proll;
    deviceGeom.sensor_poses[ii].ppitch = geom->poses[ii].ppitch;
		deviceGeom.sensor_poses[ii].pyaw = geom->poses[ii].pyaw;
	}
	// Even though the sensor sizes are all zero, they're still there
	deviceGeom.sensor_sizes_count = geom->poses_count;
	// Set the device size
	deviceGeom.size.sw = maxX - minX;
	deviceGeom.size.sl = maxY - minY;
  deviceGeom.size.sh = maxZ - minZ;

	return 0;
}

int SonarToRanger::ConvertData (player_sonar_data_t *data)
{
	player_ranger_data_range_t rangeData;

	memset (&rangeData, 0, sizeof (rangeData));
	if ((rangeData.ranges = new double[data->ranges_count]) == NULL)
	{
		PLAYER_ERROR ("Failed to allocate memory for range data");
		return 0;
	}

	// Copy the data out
	rangeData.ranges_count = data->ranges_count;
	for (uint32_t ii = 0; ii < data->ranges_count; ii++)
		rangeData.ranges[ii] = data->ranges[ii];

	// Publish the data
	Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_RANGE, reinterpret_cast<void*> (&rangeData), sizeof (rangeData), NULL);

	// Clean up
	delete[] rangeData.ranges;

	return 0;
}

int SonarToRanger::ProcessMessage (QueuePointer &respQueue, player_msghdr *hdr, void *data)
{
	// Check the parent message handler
	if (ToRanger::ProcessMessage (respQueue, hdr, data) == 0)
		return 0;

	// Check capabilities requests
	HANDLE_CAPABILITY_REQUEST (device_addr, respQueue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_POWER);
	HANDLE_CAPABILITY_REQUEST (device_addr, respQueue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM);

	// Messages from the ranger interface
	// Power config request
	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_POWER, device_addr))
	{
		// Tell the sonar device to switch the power state
		player_sonar_power_config_t req;
		req.state = reinterpret_cast<player_ranger_power_config_t*> (data)->state;
		inputDevice->PutMsg (InQueue, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_POWER, &req, sizeof (req), 0);
		// Store the return queue
		ret_queue = respQueue;
		return 0;
	}
	// Geometry request
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM, device_addr))
	{
		// Get geometry from the sonar device
		inputDevice->PutMsg (InQueue, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_GET_GEOM, NULL, 0, NULL);
		ret_queue = respQueue;
		return 0;
	}
	// Config get request
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG, device_addr))
	{
		// No config for this device, so send back a pile of zeroes
		player_ranger_config_t resp;
		memset (&resp, 0, sizeof (resp));
		Publish (device_addr, ret_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_CONFIG, &resp, sizeof (resp), NULL);
		return 0;
	}


	// Messages from the sonar interface
	// Reqest ACKs
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SONAR_REQ_POWER, inputDeviceAddr))
	{
		// Power request
		Publish (device_addr, ret_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_POWER, NULL, 0, NULL);
		return 0;
	}
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SONAR_REQ_GET_GEOM, inputDeviceAddr))
	{
		// Geometry request - need to manage the info we just got
		if (ConvertGeom (reinterpret_cast<player_sonar_geom_t*> (data)) == 0)
		{
			Publish (device_addr, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_GEOM, reinterpret_cast<void*> (&deviceGeom), sizeof (deviceGeom), NULL);
			return 0;
		}
		else
		{
			Publish (device_addr, ret_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_RANGER_REQ_GET_GEOM, NULL, 0, NULL);
			return 0;
		}
	}

	// Request NACKs
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SONAR_REQ_POWER, inputDeviceAddr))
	{
		// Power request
		Publish (device_addr, ret_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_RANGER_REQ_POWER, NULL, 0, NULL);
		return 0;
	}
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SONAR_REQ_GET_GEOM, inputDeviceAddr))
	{
		// Geometry request
		Publish (device_addr, ret_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_RANGER_REQ_GET_GEOM, NULL, 0, NULL);
		return 0;
	}

	// Data
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_SONAR_DATA_RANGES, inputDeviceAddr))
	{
		return ConvertData (reinterpret_cast<player_sonar_data_t*> (data));
	}
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_SONAR_DATA_GEOM, inputDeviceAddr))
	{
		if (ConvertGeom (reinterpret_cast<player_sonar_geom_t*> (data)) == 0)
		{
			Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_GEOM, reinterpret_cast<void*> (&deviceGeom), sizeof (deviceGeom), NULL);
			return 0;
		}
	}

	return -1;
	}
