/*  One Hell of a Robot Server
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
// Desc: usarsim (simulator) sonar driver
// Author: Jijun Wang
// Date: 17 May 2004
//
// Modified: 
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed the definitions of PutCommand and PutConfig for Player1.6
// 18 Aug 2006     Stefan Stiene updates this driver to player 2.0.2
///////////////////////////////////////////////////////////////////////////
#ifndef US_SONAR_H
#define US_SONAR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libplayercore/player.h"
#include "us_bot.h"


/// player sonar driver
/**
 * This class implements the player driver for the USARSim Sonar.\n
 * Like all us_... sensor player drivers it gets the corresponding bot from\n
 * the player device table (UsSonar::Setup()). Handles all incomming Messages\n
 * in the UsSonar::ProcessMessage() function and publish the data to each player\n
 * driver that has subscribed for this device using the UsSonar::PublishNewData()\n
 * function.\n
 * \n
 * The player configfile entry looks like this:\n
 *driver\n
 *(\n
 *  name "us_sonar"\n
 *  provides ["sonar:0"]\n
 *  requires ["simulation:0"]\n
 *  sonar_name "F"\n
 *)\n
 * provides == this driver provides the player sonar interface\n
 * requires == the UsBot this sonar array is mounted on.\n
 * sonar_name == the name of this sonar array. If you have Sonars F1 up to F8 the name must F\n
 */
class UsSonar : public Driver
{
  /// Constructor
  public: UsSonar(ConfigFile* cf, int section);
  
  /// Destructor
  public: virtual ~UsSonar();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * this method connects to the UsBot and alocates the data fields like UsBot::sonar 
   */
  int Setup(); 
  int Shutdown();
  /**
   * this method handles messages send to this device. Hence this device
   * works directly on the us_bot member variables, this method only needs to
   * catch a player request for the laser's geometry
   */
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void* data);
  /**
   * this method publish new sonar data (if available) to any
   * device that has subscribed for this laser.
   */
  void PublishNewData();
  /**
   * Copys the laser data from src to dest
   */
  void CopySonarData(player_sonar_data_t *src, player_sonar_data_t *dest); 
  private:
  /// bot player device addr
  player_devaddr_t bot_id; 
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
  /// usarsim sonar name 
  char name[128];
  /// number of sonars for this sonar array
  int number;
  /// a bot could have multiple sonars arrays sonar_index is the index in the us_bot sonar
  /// array vector for this sonar array.
  int sonar_index;
  /// buffer sonar data to prevent sigseg faults (necessary?)
  player_sonar_data_t mySonarData;
};
#endif //US_SONAR_H
