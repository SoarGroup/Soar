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
// Desc: usarsim (simulator) ptz driver
// Author: Jijun Wang
// Date: 18 May 2004
//
// Modified: 
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed the definitions of PutCommand and PutConfig for Player1.6
// 18 Aug  2006    Stefan Stiene update to player-2.0.2
///////////////////////////////////////////////////////////////////////////
#ifndef US_PTZ_H
#define US_PTZ_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libplayercore/player.h"

#include "us_bot.h"


/// pan tilt zoom driver 
/**
 * you can set the pan tilt and zoom of a usarsim camera Todo the
 * parser doesn't implement the us_get_ptz funktion yet, because we
 * want to use a camera mounted on a MISPKG and we didn't find a way
 * yet to get the camera orientation.
 */
class UsPtz : public Driver
{
public:
  /**
   * Constructor
   * @param cf Configfile
   * @param section The section in the configfile
   */
  UsPtz(ConfigFile* cf, int section);
  /**
   * Destructor
   */
  ~UsPtz();
  /**
   * get the UsPtz::bot using the UsPtz::bot_id
   */
  int Setup();                
  int Shutdown();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * this method handles messages send to this device.
   */
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void *	data);
  /**
   * this method publish new ptz data (if available) to any
   * device that has subscribed to this device.
   */
  void PublishNewData();


private:
  player_devaddr_t bot_id;
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
  /// usarsim laser name 
  char cam_name[128];
  char ptz_name[128];
  uint32_t mode;
  int ptz_index;
};
#endif //US_PTZ_H
