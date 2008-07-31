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
/********************************************************************
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
 *
 ********************************************************************/

#ifndef _GLOBALS_H
#define _GLOBALS_H

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <libplayercore/devicetable.h>
#include <libplayercore/drivertable.h>
#include <libplayercore/filewatcher.h>
#include <libplayercore/playertime.h>
#include <libplayercore/wallclocktime.h>

#if HAVE_PLAYERSD
  #include <libplayersd/playersd.h>
#endif

// this table holds all the currently *instantiated* devices
DeviceTable* deviceTable;

// this table holds all the currently *available* drivers
DriverTable* driverTable;

// the global PlayerTime object has a method
//   int GetTime(struct timeval*)
// which everyone must use to get the current time
PlayerTime* GlobalTime;

// global class for watching for changes in files and sockets
FileWatcher* fileWatcher;

char playerversion[32];

bool player_quit;
bool player_quiet_startup;

// global access to the cmdlihe arguments
int player_argc;
char** player_argv;

#if HAVE_PLAYERSD
struct player_sd* globalSD;
#endif

void
player_globals_init()
{
  deviceTable = new DeviceTable();
  driverTable = new DriverTable();
  GlobalTime = new WallclockTime();
  fileWatcher = new FileWatcher();
  strncpy(playerversion, VERSION, sizeof(playerversion));
  player_quit = false;
  player_quiet_startup = false;
#if HAVE_PLAYERSD
  globalSD = player_sd_init();
#endif
}

void
player_globals_fini()
{
  delete deviceTable;
  delete driverTable;
  delete GlobalTime;
  delete fileWatcher;
#if HAVE_PLAYERSD
  if(globalSD)
    player_sd_fini(globalSD);
#endif
}

#endif
