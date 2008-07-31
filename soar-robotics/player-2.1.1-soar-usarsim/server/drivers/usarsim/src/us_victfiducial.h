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
// Desc: usarsim (simulator) fiducial driver
// Author: Stefan Markov
// Date: 23 Feb 2006
//
// Modified: 
///////////////////////////////////////////////////////////////////////////
#ifndef US_VICTFIDUCIAL_H
#define US_VICTFIDUCIAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libplayercore/player.h"

#include "us_bot.h"

///player fiducial driver !!!not tested yet!!!
/**
 * This class implements the USARSim VictRFID sensor.\n
 * Like all us_... sensor player drivers it gets the corresponding bot from\n
 * the player device table (UsFiducial::Setup()). Handles all incomming Messages\n
 * in the Usfiducial::ProcessMessage() function and publish the data to each player\n
 * driver that has subscribed for this device using the UsFiducial::PublishNewData()\n
 * function.\n
 * So far only support for one RFID sensor per robot is added.\n
 * \n
 * The player configfile entry looks like this:\n
 *driver\n
 *(\n
 *  name "us_victfiducial"\n
 *  provides ["fiducial:0"]\n
 *  requires ["simulation:0"]\n
 *  fiducial_name "RFID1"\n
 *)\n
 * provides == this driver provides the player laser interface\n
 * requires == the UsBot this RFID is mounted on.\n
 * fiducial_name == the name of RFID sensor.(not used yet)\n
 */
class UsVictFiducial : public Driver
{
public:
  /**
   * Constructor
   * @param cf Configfile
   * @param section The section in the configfile
   */
  UsVictFiducial(ConfigFile* cf, int section);
  /**
   * Destructor
   */
  ~UsVictFiducial();
  /**
   * get the UsVictFiducial::bot using the UsVictFiducial::bot_id
   */
  int Setup(); 
  int Shutdown();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * this method handles messages send to this device. Hence this device
   * works directly on the Usbot member variables, this method only needs to
   * catch a player request for the victfiducials's geometry.
   */
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void* data);
  /**
   * this method publish new fiducial data (if available) to any
   * device that has subscribed for this laser.
   */
  void PublishNewData();
private:
  /// bot player device addr
  player_devaddr_t bot_id; 
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
  /// usarsim laser name 
  char fiducial_name[128];
  /// a bot could have multiple scanners thats the index in the us_bot laser
  /// vectors this laser is stored in.
  int fiducial_index;
};
#endif //US_VICTFIDUCIAL_H
