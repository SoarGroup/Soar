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
 * Desc: Log device proxy
 * Author: Brian gerkey
 * Date: June 2004
 * CVS: $Id: dev_log.c 4227 2007-10-24 22:32:04Z thjc $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Create a new log proxy
playerc_log_t *playerc_log_create(playerc_client_t *client, int index)
{
  playerc_log_t *device;

  device = malloc(sizeof(playerc_log_t));
  memset(device, 0, sizeof(playerc_log_t));
  playerc_device_init(&device->info, client, PLAYER_LOG_CODE, index,
                      (playerc_putmsg_fn_t) NULL);
    
  return device;
}


// Destroy a log proxy
void playerc_log_destroy(playerc_log_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}

// Subscribe to the log device
int playerc_log_subscribe(playerc_log_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the log device
int playerc_log_unsubscribe(playerc_log_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

// Get logging/playback state; the result is written into the proxy
int playerc_log_get_state(playerc_log_t* device)
{
  player_log_get_state_t *req;

  if(playerc_client_request(device->info.client, 
                            &device->info,
                            PLAYER_LOG_REQ_GET_STATE,
                            NULL, (void**)&req) < 0)
  {
    PLAYERC_ERR("failed to get logging/playback state");
    return(-1);
  }
  device->type = req->type;
  device->state = req->state;
  player_log_get_state_t_free(req);
  return(0);
}

// Start/stop logging
int playerc_log_set_write_state(playerc_log_t* device, int state)
{
  player_log_set_write_state_t req;

  req.state = (uint8_t)state;

  if(playerc_client_request(device->info.client, 
                            &device->info,PLAYER_LOG_REQ_SET_WRITE_STATE,
                            &req, NULL) < 0)
  {
    PLAYERC_ERR("failed to start/stop data logging");
    return(-1);
  }
  return(0);
}

// Start/stop playback
int playerc_log_set_read_state(playerc_log_t* device, int state)
{
  player_log_set_read_state_t req;

  req.state = (uint8_t)state;

  if(playerc_client_request(device->info.client, 
                            &device->info, PLAYER_LOG_REQ_SET_READ_STATE,
                            &req, NULL) < 0)
  {
    PLAYERC_ERR("failed to start/stop data playback");
    return(-1);
  }
  return(0);
}

// Rewind playback
int playerc_log_set_read_rewind(playerc_log_t* device)
{
  if(playerc_client_request(device->info.client, 
                            &device->info, PLAYER_LOG_REQ_SET_READ_REWIND,
                            NULL, NULL) < 0)
  {
    PLAYERC_ERR("failed to rewind data playback");
    return(-1);
  }
  return(0);
}

// Change filename 
int playerc_log_set_filename(playerc_log_t* device, const char* fname)
{
  player_log_set_filename_t req;
  memset(&req,0,sizeof(req));

  if(strlen(fname) > (sizeof(req.filename)-1))
  {
    PLAYERC_ERR("filename too long");
    return(-1);
  }
  strcpy(req.filename,fname);
  req.filename_count = strlen(req.filename);

  if(playerc_client_request(device->info.client, 
                            &device->info, PLAYER_LOG_REQ_SET_FILENAME,
                            &req, NULL) < 0)
  {
    PLAYERC_ERR("failed to set logfile name");
    return(-1);
  }
  return(0);
}
