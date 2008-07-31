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
// Desc: Base class for laser transformations (i.e. cspace etc)
// Author: Andrew Howard
// Date: 1 Sep 2002
// CVS: $Id: lasertransform.h 4135 2007-08-23 19:58:48Z gerkey $
//
// Theory of operation - 
//
// Requires - Laser device.
//
///////////////////////////////////////////////////////////////////////////



#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <unistd.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

// Driver for computing the free c-space from a laser scan.
class LaserTransform : public Driver
{
  // Constructor
  public: LaserTransform( ConfigFile* cf, int section);

    // MessageHandler
  public: virtual int ProcessMessage(QueuePointer & resp_queue, 
                              player_msghdr * hdr, 
                              void * data);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

  protected:
  // Process laser data.  Returns non-zero if the laser data has been
  // updated.
  virtual int UpdateLaser(player_laser_data_t * data) = 0;

  // Process requests.  Returns 1 if the configuration has changed.
  int HandleRequests();

  // Handle geometry requests.
  void HandleGetGeom(void *client, void *req, int reqlen);

  // Laser stuff.
  Device *laser_device;
  player_devaddr_t laser_addr;
  struct timeval laser_timestamp;

  // Fiducila stuff (the data we generate).
  player_laser_data_t data;
  struct timeval time;
};

