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
// Desc: usarsim (simulator) position3d driver
// Author: Jijun Wang
// Date: 14 May 2004
//
// Modified: 
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed the definitions of PutCommand and PutConfig for Player1.6 
///////////////////////////////////////////////////////////////////////////
#ifndef US_POSITION3D_H
#define US_POSITION3D_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libplayercore/player.h"

#include "us_bot.h"

#include "debug.h"

/// Incremental navigation driver
/**
 * same information as UsPosition but stores information in an 3d interface.\n
 * See UsPosition for more details.
 */
class UsPosition3d : public Driver
{
  public:
  /**
   * Constructor
   * @param cf Configfile
   * @param section The section in the configfile
   */
  UsPosition3d(ConfigFile* cf, int section);

  /// Destructor
  ~UsPosition3d();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * get the UsPosition::bot using the UsPosition::bot_id
   */
  int Setup();
  int Shutdown();
  /**
   * handles reguests for this device:\n
   * Requests:\n
   * PLAYER_POSITION2D_REQ_GET_GEOM\n
   * PLAYER_POSITION2D_REQ_MOTOR_POWER\n
   * Commands:\n
   * PLAYER_POSITION2D_CMD_VEL
   */
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void* data);
  /**
   * this method publish new position data (if available) to any
   * device that has subscribed.
   */
  void PublishNewData();
  private:
  /// bot player device addr
  player_devaddr_t  bot_id;
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
};
#endif //US_POSITION3D_H
