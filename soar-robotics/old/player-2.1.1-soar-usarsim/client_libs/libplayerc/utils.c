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
/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) Andrew Howard 2003
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/***************************************************************************
 * Desc: Utility functions.
 * Author: Andrew Howard
 * Date: 16 Aug 2002
 * CVS: $Id: utils.c 4127 2007-08-20 19:42:49Z thjc $
 **************************************************************************/

#include <string.h>
#include <libplayerxdr/functiontable.h>

#include "playerc.h"
#include "error.h"

int playerc_add_xdr_ftable(playerxdr_function_t *flist, int replace)
{
  return playerxdr_ftable_add_multi(flist, replace);
}

/*
// Get the name for a given device code.
const char *playerc_lookup_name(int code)
{
  switch (code)
  {
    case PLAYER_ACTARRAY_CODE:
      return PLAYER_ACTARRAY_STRING;
    case PLAYER_AIO_CODE:
      return PLAYER_AIO_STRING;
    case PLAYER_BLOBFINDER_CODE:
      return PLAYER_BLOBFINDER_STRING;
    case PLAYER_BUMPER_CODE:
      return PLAYER_BUMPER_STRING;
    case PLAYER_CAMERA_CODE:
      return PLAYER_CAMERA_STRING;
    case PLAYER_DIO_CODE:
      return PLAYER_DIO_STRING;
    case PLAYER_FIDUCIAL_CODE:
      return PLAYER_FIDUCIAL_STRING;
    case PLAYER_JOYSTICK_CODE:
      return PLAYER_JOYSTICK_STRING;
    case PLAYER_GPS_CODE:
      return PLAYER_GPS_STRING;
    case PLAYER_LASER_CODE:
      return PLAYER_LASER_STRING;
    case PLAYER_LIMB_CODE:
      return PLAYER_LIMB_STRING;
    case PLAYER_LOCALIZE_CODE:
      return PLAYER_LOCALIZE_STRING;
    case PLAYER_GRAPHICS2D_CODE:
      return PLAYER_GRAPHICS2D_STRING;
    case PLAYER_GRIPPER_CODE:
      return PLAYER_GRIPPER_STRING;
    case PLAYER_IR_CODE:
      return PLAYER_IR_STRING;
    case PLAYER_POSITION1D_CODE:
      return PLAYER_POSITION1D_STRING;
    case PLAYER_POSITION2D_CODE:
      return PLAYER_POSITION2D_STRING;
    case PLAYER_POSITION3D_CODE:
      return PLAYER_POSITION3D_STRING;
    case PLAYER_POWER_CODE:
      return PLAYER_POWER_STRING;
    case PLAYER_PTZ_CODE:
      return PLAYER_PTZ_STRING;
    case PLAYER_RFID_CODE:
      return PLAYER_RFID_STRING;
    case PLAYER_SONAR_CODE:
      return PLAYER_SONAR_STRING;
    case PLAYER_SPEECH_CODE:
      return PLAYER_SPEECH_STRING;
    case PLAYER_WIFI_CODE:
      return PLAYER_WIFI_STRING;
    case PLAYER_WSN_CODE:
      return PLAYER_WSN_STRING;
    case PLAYER_MAP_CODE:
      return PLAYER_MAP_STRING;
    case PLAYER_LOG_CODE:
      return PLAYER_LOG_STRING;
    case PLAYER_SIMULATION_CODE:
      return PLAYER_SIMULATION_STRING;
    case PLAYER_IMU_CODE:
      return PLAYER_IMU_STRING;
    case PLAYER_POINTCLOUD3D_CODE:
      return PLAYER_POINTCLOUD3D_STRING;

    default:
      break;
  }
  return "unknown";
}


// Get the device code for a give name.
int playerc_lookup_code(const char *name)
{
  if (strcmp(name, PLAYER_ACTARRAY_STRING) == 0)
    return PLAYER_ACTARRAY_CODE;
  if (strcmp(name, PLAYER_AIO_STRING) == 0)
    return PLAYER_AIO_CODE;
  if (strcmp(name, PLAYER_BLOBFINDER_STRING) == 0)
    return PLAYER_BLOBFINDER_CODE;
  if (strcmp(name, PLAYER_BUMPER_STRING) == 0)
    return PLAYER_BUMPER_CODE;
  if (strcmp(name, PLAYER_CAMERA_STRING) == 0)
    return PLAYER_CAMERA_CODE;
  if (strcmp(name, PLAYER_DIO_STRING) == 0)
    return PLAYER_DIO_CODE;
  if (strcmp(name, PLAYER_FIDUCIAL_STRING) == 0)
    return PLAYER_FIDUCIAL_CODE;
  if (strcmp(name, PLAYER_JOYSTICK_STRING) == 0)
    return PLAYER_JOYSTICK_CODE;
  if (strcmp(name, PLAYER_GRAPHICS2D_STRING) == 0)
    return PLAYER_GRAPHICS2D_CODE;
  if (strcmp(name, PLAYER_GPS_STRING) == 0)
    return PLAYER_GPS_CODE;
  if (strcmp(name, PLAYER_LASER_STRING) == 0)
    return PLAYER_LASER_CODE;
  if (strcmp(name, PLAYER_LIMB_STRING) == 0)
    return PLAYER_LIMB_CODE;
  if (strcmp(name, PLAYER_LOCALIZE_STRING) == 0)
    return PLAYER_LOCALIZE_CODE;
  if (strcmp(name, PLAYER_GRIPPER_STRING) == 0)
    return PLAYER_GRIPPER_CODE;
  if (strcmp(name, PLAYER_IR_STRING) == 0)
    return PLAYER_IR_CODE;
  if (strcmp(name, PLAYER_POSITION1D_STRING) == 0)
    return PLAYER_POSITION1D_CODE;
  if (strcmp(name, PLAYER_POSITION2D_STRING) == 0)
    return PLAYER_POSITION2D_CODE;
  if (strcmp(name, PLAYER_POSITION3D_STRING) == 0)
    return PLAYER_POSITION3D_CODE;
  if (strcmp(name, PLAYER_POWER_STRING) == 0)
    return PLAYER_POWER_CODE;
  if (strcmp(name, PLAYER_PTZ_STRING) == 0)
    return PLAYER_PTZ_CODE;
  if (strcmp(name, PLAYER_RFID_STRING) == 0)
    return PLAYER_RFID_CODE;
  if (strcmp(name, PLAYER_SONAR_STRING) == 0)
    return PLAYER_SONAR_CODE;
  if (strcmp(name, PLAYER_SPEECH_STRING) == 0)
    return PLAYER_SPEECH_CODE;
  if (strcmp(name, PLAYER_WIFI_STRING) == 0)
    return PLAYER_WIFI_CODE;
  if (strcmp(name, PLAYER_WSN_STRING) == 0)
    return PLAYER_WSN_CODE;
  if (strcmp(name, PLAYER_MAP_STRING) == 0)
    return PLAYER_MAP_CODE;
  if (strcmp(name, PLAYER_LOG_STRING) == 0)
    return PLAYER_LOG_CODE;
  if (strcmp(name, PLAYER_SIMULATION_STRING) == 0)
    return PLAYER_SIMULATION_CODE;
  if (strcmp(name, PLAYER_IMU_STRING) == 0)
    return PLAYER_IMU_CODE;
  if (strcmp(name, PLAYER_POINTCLOUD3D_STRING) == 0)
    return PLAYER_POINTCLOUD3D_CODE;
  return -1;
}
*/

