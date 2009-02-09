/*
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002-2003
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/***************************************************************************
 * Desc: IMU proxy
 * Author: Radu Bogdan Rusu
 * Date: 8th of September 2006
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"

// Process incoming data
void playerc_imu_putmsg (playerc_imu_t *device,
                         player_msghdr_t *header,
			 void *data);

// Create a new imu proxy
playerc_imu_t *playerc_imu_create(playerc_client_t *client, int index)
{
    playerc_imu_t *device;
    device = malloc(sizeof(playerc_imu_t));
    memset(device, 0, sizeof(playerc_imu_t));
    playerc_device_init(&device->info, client, PLAYER_IMU_CODE, index,
       (playerc_putmsg_fn_t) playerc_imu_putmsg);

    return device;
}


// Destroy a imu proxy
void playerc_imu_destroy(playerc_imu_t *device)
{
    playerc_device_term(&device->info);
    free(device);
}


// Subscribe to the imu device
int playerc_imu_subscribe(playerc_imu_t *device, int access)
{
    return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the imu device
int playerc_imu_unsubscribe(playerc_imu_t *device)
{
    return playerc_device_unsubscribe(&device->info);
}

// Process incoming data
void playerc_imu_putmsg (playerc_imu_t *device,
			 player_msghdr_t *header,
			 void *data)
{
//    int i, j;
    if (header->type == PLAYER_MSGTYPE_DATA)
	switch (header->subtype)
	{
	    case PLAYER_IMU_DATA_STATE:
	    {
		player_imu_data_state_t* imu_data = (player_imu_data_state_t*)data;
		device->pose = imu_data->pose;
		break;
	    }
	    case PLAYER_IMU_DATA_CALIB:
	    {
		player_imu_data_calib_t* imu_data = (player_imu_data_calib_t*)data;
		device->calib_data.accel_x = imu_data->accel_x;
		device->calib_data.accel_y = imu_data->accel_y;
		device->calib_data.accel_z = imu_data->accel_z;
		device->calib_data.gyro_x = imu_data->gyro_x;
		device->calib_data.gyro_y = imu_data->gyro_y;
		device->calib_data.gyro_z = imu_data->gyro_z;
		device->calib_data.magn_x = imu_data->magn_x;
		device->calib_data.magn_y = imu_data->magn_y;
		device->calib_data.magn_z = imu_data->magn_z;
		break;
	    }
	    case PLAYER_IMU_DATA_QUAT:
	    {
		player_imu_data_quat_t* imu_data = (player_imu_data_quat_t*)data;
		device->calib_data = imu_data->calib_data;
		device->q0 = imu_data->q0;
		device->q1 = imu_data->q1;
		device->q2 = imu_data->q2;
		device->q3 = imu_data->q3;
		break;
	    }
	    case PLAYER_IMU_DATA_EULER:
	    {
		player_imu_data_euler_t* imu_data = (player_imu_data_euler_t*)data;
		device->calib_data  = imu_data->calib_data;
		device->pose.proll  = imu_data->orientation.proll;
		device->pose.ppitch = imu_data->orientation.ppitch;
		device->pose.pyaw   = imu_data->orientation.pyaw;
		break;
	    }
	    default:
	    {
		PLAYERC_WARN1 ("skipping imu message with unknown data subtype: %d\n",
		    header->subtype);
		break;
	    }
	}
    else
	PLAYERC_WARN2 ("skipping imu message with unknown type/subtype: %s/%d\n",
	    msgtype_to_str(header->type), header->subtype);
}

// Change the data type to one of the predefined data structures.
int
playerc_imu_datatype (playerc_imu_t *device, int value)
{
  player_imu_datatype_config_t config;

  config.value = value;

  return (playerc_client_request(device->info.client,
                                 &device->info,
                                 PLAYER_IMU_REQ_SET_DATATYPE,
                                 &config, NULL));
}

// Reset orientation
int
playerc_imu_reset_orientation (playerc_imu_t *device, int value)
{
  player_imu_reset_orientation_config_t config;

  config.value = value;

  return (playerc_client_request(device->info.client,
                                 &device->info,
                                 PLAYER_IMU_REQ_RESET_ORIENTATION,
                                 &config, NULL));
}
