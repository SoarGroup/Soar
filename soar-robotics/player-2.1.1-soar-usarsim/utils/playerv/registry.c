/*
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
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
/**************************************************************************
 * Desc: Device registry.
 * Author: Andrew Howard
 * Date: 16 Aug 2002
 * CVS: $Id: registry.c 4325 2008-01-09 11:45:37Z thjc $
 *************************************************************************/

#include "playerv.h"

// Create the appropriate GUI proxy for a given set of device info.
void create_proxy(device_t *device, opt_t *opt, mainwnd_t *mainwnd, playerc_client_t *client)
{
  switch (device->addr.interf)
  {
    case PLAYER_ACTARRAY_CODE:
      device->proxy = actarray_create(mainwnd, opt, client,
                                    device->addr.index,
                                    device->drivername,
                                    device->subscribe);
      device->fndestroy = (fndestroy_t) actarray_destroy;
      device->fnupdate = (fnupdate_t) actarray_update;
      break;

    case PLAYER_AIO_CODE:
      device->proxy = aio_create(mainwnd, opt, client,
                                    device->addr.index,
                                    device->drivername,
                                    device->subscribe);
      device->fndestroy = (fndestroy_t) aio_destroy;
      device->fnupdate = (fnupdate_t) aio_update;
      break;

     case PLAYER_BUMPER_CODE:
       device->proxy = bumper_create(mainwnd, opt, client,
                                   device->addr.index,
                                   device->drivername,
                                   device->subscribe);
       device->fndestroy = (fndestroy_t) bumper_destroy;
       device->fnupdate = (fnupdate_t) bumper_update;
       break;

     case PLAYER_DIO_CODE:
       device->proxy = dio_create(mainwnd, opt, client,
                                  device->addr.index,
                                  device->drivername,
                                  device->subscribe);
       device->fndestroy = (fndestroy_t) dio_destroy;
       device->fnupdate = (fnupdate_t) dio_update;
       break;

    case PLAYER_IR_CODE:
      device->proxy = ir_create(mainwnd, opt, client,
                                   device->addr.index, device->drivername, device->subscribe);
      device->fndestroy = (fndestroy_t) ir_destroy;
      device->fnupdate = (fnupdate_t) ir_update;
      break;

    case PLAYER_LASER_CODE:
      device->proxy = laser_create(mainwnd, opt, client,
                                   device->addr.index,
                                   device->drivername,
                                   device->subscribe);
      device->fndestroy = (fndestroy_t) laser_destroy;
      device->fnupdate = (fnupdate_t) laser_update;
      break;

    case PLAYER_POWER_CODE:
      device->proxy = power_create(mainwnd, opt, client,
                                   device->addr.index,
                                   device->drivername, device->subscribe);
      device->fndestroy = (fndestroy_t) power_destroy;
      device->fnupdate = (fnupdate_t) power_update;
      break;

    case PLAYER_POSITION2D_CODE:
      device->proxy = position2d_create(mainwnd, opt, client,
                                        device->addr.index,
                                        device->drivername,
                                        device->subscribe);
      device->fndestroy = (fndestroy_t) position2d_destroy;
      device->fnupdate = (fnupdate_t) position2d_update;
      break;

    case PLAYER_PTZ_CODE:
      device->proxy = ptz_create(mainwnd, opt, client,
                                        device->addr.index,
                                        device->drivername,
                                        device->subscribe);
      device->fndestroy = (fndestroy_t) ptz_destroy;
      device->fnupdate = (fnupdate_t) ptz_update;
      break;

    case PLAYER_RANGER_CODE:
      device->proxy = ranger_create(mainwnd, opt, client,
                                    device->addr.index,
                                    device->drivername,
                                    device->subscribe);
      device->fndestroy = (fndestroy_t) ranger_destroy;
      device->fnupdate = (fnupdate_t) ranger_update;
      break;

    case PLAYER_SONAR_CODE:
      device->proxy = sonar_create(mainwnd, opt, client,
                                   device->addr.index,
                                   device->drivername,
                                   device->subscribe);
      device->fndestroy = (fndestroy_t) sonar_destroy;
      device->fnupdate = (fnupdate_t) sonar_update;
      break;

    case PLAYER_WIFI_CODE:
      device->proxy = wifi_create(mainwnd, opt, client,
                                        device->addr.index,
                                        device->drivername,
                                        device->subscribe);
      device->fndestroy = (fndestroy_t) wifi_destroy;
      device->fnupdate = (fnupdate_t) wifi_update;
      break;

    case PLAYER_BLOBFINDER_CODE:
      device->proxy = blobfinder_create(mainwnd, opt, client,
					device->addr.index,
                                        device->drivername,
					device->subscribe);
      device->fndestroy = (fndestroy_t) blobfinder_destroy;
      device->fnupdate = (fnupdate_t) blobfinder_update;
      break;

    case PLAYER_CAMERA_CODE:
      device->proxy = camera_create(mainwnd, opt, client,
					device->addr.index,
                                        device->drivername,
					device->subscribe);
      device->fndestroy = (fndestroy_t) camera_destroy;
      device->fnupdate = (fnupdate_t) camera_update;
      break;



    case PLAYER_FIDUCIAL_CODE:
      device->proxy = fiducial_create(mainwnd, opt, client,
                                      device->addr.index,
				      device->drivername,
				      device->subscribe);
      device->fndestroy = (fndestroy_t) fiducial_destroy;
      device->fnupdate = (fnupdate_t) fiducial_update;
      break;

    case PLAYER_GRIPPER_CODE:
      device->proxy = gripper_create(mainwnd, opt, client,
				     device->addr.index,
				     device->drivername,
				     device->subscribe);
      device->fndestroy = (fndestroy_t) gripper_destroy;
      device->fnupdate = (fnupdate_t) gripper_update;
      break;

    case PLAYER_MAP_CODE:
      device->proxy = map_create(mainwnd, opt, client,
				 device->addr.index,
				 device->drivername,
				 device->subscribe);
      device->fndestroy = (fndestroy_t) map_destroy;
      device->fnupdate = (fnupdate_t) map_update;
      break;

    case PLAYER_VECTORMAP_CODE:
      device->proxy = vectormap_create(mainwnd, opt, client,
         device->addr.index,
         device->drivername,
         device->subscribe);
      device->fndestroy = (fndestroy_t) vectormap_destroy;
      device->fnupdate = (fnupdate_t) vectormap_update;
      break;


#if 0
    case PLAYER_LOCALIZE_CODE:
      device->proxy = localize_create(mainwnd, opt, client,
                                 device->index, device->drivername, device->subscribe);
      device->fndestroy = (fndestroy_t) localize_destroy;
      device->fnupdate = (fnupdate_t) localize_update;
      break;



#endif

    default:
      device->proxy = NULL;
      device->fndestroy = NULL;
      device->fnupdate = NULL;
      break;
  }
  return;
}
