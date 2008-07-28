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
// Desc: usarsim (simulator) position driver
// Author: Jijun Wang
// Date: 14 May 2004
//
// Modified: 
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed the definitions of PutCommand and PutConfig for Player1.6
// 21 Juli 2006    Stefan Stiene changed the whole thing to player-2.0.2
///////////////////////////////////////////////////////////////////////////
#ifndef US_POSITION_H
#define US_POSITION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <iostream>
using std::cout;
using std::endl;

#include "libplayercore/player.h"
#include "libplayercore/playercore.h"  //for PLAYER_MSG

#include "us_bot.h"

#define SKIDSTEERED 1
#define ACKERMANNSTEERED 2
 

///Incremental navigation driver
/**
 *  This class implements the player driver for the USARSim Odometry sensor.\n
 * Like all us_... sensor player drivers it gets the corresponding bot from\n
 * the player device table (UsPosition::Setup()). Handles all incomming Messages\n
 * in the UsPosition::ProcessMessage() function and publish the data to each player\n
 * driver that has subscribed for this device using the UsPosition::PublishNewData()\n
 * function.\n
 * \n
 * The player configfile entry looks like this:\n
 *driver\n
 *(\n
 *  name "us_position"\n
 *  provides ["odometry:::position2d:0"]\n
 *  requires ["simulation:0"]\n
 *  odo_name "Odometry"\n
 *)\n
 * provides == this driver provides the player position2d interface\n
 * requires == the UsBot this odometry informations are for.\n
 * odo_name == the USARSim name of this odometry sensor.\n
 * use_encoder == use USARSim build in Odometry (0) or compute via encoder sensors(1).\n 
 * steer_type == (1 = skid steered 2 = ackermann steered).\n
 */
class UsPosition : public Driver
{ 
public:
  /**
   * Constructor
   */
  UsPosition(ConfigFile* cf, int section);
  /**
   * Destructor
   */
  ~UsPosition();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * get the UsPosition::bot using the UsPosition::bot_id
   */
  int Setup();
  /**
   *
   */
  int Shutdown();
  /**
   * handles reguests for this device:\n
   * Requests:\n
   * PLAYER_POSITION2D_REQ_GET_GEOM\n
   * PLAYER_POSITION2D_REQ_MOTOR_POWER\n
   * PLAYER_POSITION2D_REQ_RESET_ODOM\n
   * Commands:\n
   * PLAYER_POSITION2D_CMD_VEL\n
   * PLAYER_POSITION2D_CMD_CAR
   */
  int ProcessMessage(QueuePointer & resp_queue, player_msghdr* hdr,void* data);
  /**
   * this method publish new position data (if available) to any
   * device that has subscribed.
   */
  void PublishNewData();
  /**
   * set UsPosition::steer_type to the parsed value
   */
  bool setSteerType();
  /**
   * Set observed wheel turn ticks
   * Values are gathered through a USARSim encoder
   * @param ticks_l Number of ticks of left wheel
   * @param ticks_r Number of ticks of right wheel
   */
  void setTicks(int ticks_l, int ticks_r);
  /**
   * Set position data as observed by the internal USARSim odometry device
   * @param pos Position object
   */
  void setPosition(player_position2d_data_t* pos);
private:
  /// bot player device addr
  player_devaddr_t  bot_id;
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
  /// ? used for odometry command ?
  player_devaddr_t odometry_addr, command_addr;
  /// ? used for odometry command ?
  float set_trans, set_rot;
  /// name of the odometry sensor needed for reset the odometry
  char odo_name[MAX_FILENAME_SIZE];
  /// skid or ackermann steered robot
  int steer_type;
  /// ticks of left wheel
  int ticks_l;
  /// ticks of right wheel
  int ticks_r;
  /// position
  player_position2d_data_t *pos;
  /// mutex for position data
  pthread_mutex_t posMutex;
  /// avoid sending of drive speed 0 cmds
  float old_trans;
};
#endif //US_POSITION3D_H
