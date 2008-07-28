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
// Desc: usarsim (simulator) laser3d driver
// Author: Florian Halbritter and Stefan Stiene
//         (adopting code of the respective authors of the original UsLaser driver)
// Date: 22 Nov 2006
//
///////////////////////////////////////////////////////////////////////////
#ifndef US_LASER3D_H
#define US_LASER3D_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libplayercore/playercore.h>

#include "us_bot.h"
///player laser3d driver
/**
 * This class implements the player driver for the USARSim RangeScanner3d.\n
 * Like all us_... sensor player drivers it gets the corresponding bot from\n
 * the player device table (UsLaser3d::Setup()). Handles all incomming Messages\n
 * in the UsLaser3d::ProcessMessage() function and publish the data to each player\n
 * driver that has subscribed for this device using the UsLaser3d::PublishNewData()\n
 * function.\n
 * \n
 * The player configfile entry looks like this:\n
 *driver\n
 *(\n
 *  name "us_laser3d"\n
 *  provides ["pointcloud3d:0"]\n
 *  requires ["simulation:0"]\n
 *  laser_name "Scanner1"\n
 *)\n
 * provides == this driver provides the player pointcloud3d interface (not yet included in interface_util.cc!)\n
 * requires == the UsBot this laser3d is mounted on.\n
 * laser3d_name == the name of this laserd3.\n
 */
class UsLaser3d : public Driver
{
public:
  /**
   * Constructor
   * @param cf Configfile
   * @param section The section in the configfile
   */
  UsLaser3d(ConfigFile* cf, int section); 
  /**
   * Destructor
   */
  ~UsLaser3d();
  /**
   * Main method containing the ProcessMessage PublishNewData Loop
   */
  void Main();
  /**
   * this method alocates the data fields like UsBot::laser and
   * gets the laser configuration from us_bot
   */
  int Setup(); 
  /**
   * this method publish new laser data (if available) to any
   * device that has subscribed for this laser.
   */
  void PublishNewData();
  /**
   *
   */
  int Shutdown();
  /**
   * this method handles messages send to this device. Hence this device
   * works directly on the UsBot member variables, this method only needs to
   * catch a player request for the laser's geometry
   */
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void* data);
  /**
   * New laser data will be set by UsBot
   * The bot informs the laser of new data if available and provides
   * a reference to the data object
   * @param data Pointer to new laser data
   */
  void SetData(player_pointcloud3d_data_t* data);
  /**
   * Sets geomtry data for current laser
   * @param geom Geometry data
   *
   */
  void SetGeom(player_laser3d_geom_t* geom);
  /**
   * Sets config data for current laser
   * @param conf Config data
   */
  void SetConf(player_laser3d_config_t* conf);
private:
  /// bot player device addr
  player_devaddr_t bot_id; 
  /// bot player driver using this driver we can access the us_bot fields directly
  UsBot* bot;
  /// usarsim laser name 
  char name[128];
  /// pointer to new laser data sent by bot !!do not access directly!!
  player_pointcloud3d_data_t* data;
  /// pointer to geometry data  !!do not access directly!!
  player_laser3d_geom_t* geom;
  /// pointer to config data  !!do not access directly!!
  player_laser3d_config_t* conf;

  /**
   * Returns pointer to up to date laser data
   * May return NULL if no new data is available since last method call.
   * @return Laser data object, NULL if no new data available
   */
  player_pointcloud3d_data_t* GetData();
  /**
   * Returns pointer to geometry data
   * May return NULL if Bot didn't provide data in time. Should not happen.
   * @return Laser geometry data, NULL if bot answer took too much time
   */
  player_laser3d_geom_t* GetGeom();
  /**
   * Returns pointer to config data
   * May return NULL if Bot didn't provide data in time. Should not happen.
   * @return Laser config data, NULL if bot answer took too much time
   */
  player_laser3d_config_t* GetConf();
  /// mutex for config data
  pthread_mutex_t confMutex;
  /// mutex for laser data
  pthread_mutex_t dataMutex;
  /// mutex for geometry data
  pthread_mutex_t geomMutex;

};
#endif //US_LASER3D_H
