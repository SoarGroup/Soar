/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al.
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
// Desc: LASER sensor model for AMCL
// Author: Andrew Howard
// Date: 17 Aug 2003
// CVS: $Id: amcl_laser.h 6496 2008-06-09 22:32:57Z thjc $
//
///////////////////////////////////////////////////////////////////////////

#ifndef AMCL_LASER_H
#define AMCL_LASER_H

#include "amcl_sensor.h"
#include "map/map.h"
#include "models/laser.h"

// Laser sensor data
class AMCLLaserData : public AMCLSensorData
{
  public:
    AMCLLaserData () {ranges=NULL;};
    virtual ~AMCLLaserData() {delete [] ranges;};
  // Laser range data (range, bearing tuples)
  public: int range_count;
  public: double range_max;
  public: double (*ranges)[2];
};


// Laseretric sensor model
class AMCLLaser : public AMCLSensor
{
  // Default constructor
  public: AMCLLaser(AdaptiveMCL & aAMCL, player_devaddr_t addr);
  
  // Load the model
  public: virtual int Load(ConfigFile* cf, int section);

  // Unload the model
  public: virtual int Unload(void);

  // Initialize the model
  public: virtual int Setup(void);

  // Finalize the model
  public: virtual int Shutdown(void);

  // Process message for this interface
  public: virtual int ProcessMessage(QueuePointer &resp_queue, 
                                     player_msghdr * hdr, 
                                     void * data);
  // Check for new sensor measurements
  //private: virtual AMCLSensorData *GetData(void);
  
  // Update the filter based on the sensor model.  Returns true if the
  // filter has been updated.
  public: virtual bool UpdateSensor(pf_t *pf, AMCLSensorData *data);

  // Determine the probability for the given pose
  private: static double SensorModel(AMCLLaserData *data, 
                                     pf_sample_set_t* set);

  // retrieve the map
  private: int SetupMap(void);

  // Device info
  private: player_devaddr_t laser_addr;
  private: player_devaddr_t map_addr;
  private: Device *laser_dev;

  // Current data timestamp
  private: double time;

  // The laser map
  private: map_t *map;

  // Laser offset relative to robot
  private: pf_vector_t laser_pose;
  
  // Max beams to consider
  private: int max_beams;

  // Laser range variance
  private: double range_var;

  // Probability of bad range readings
  private: double range_bad;

#ifdef INCLUDE_RTKGUI
  // Setup the GUI
  private: virtual void SetupGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);

  // Finalize the GUI
  private: virtual void ShutdownGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);

  // Draw sensor data
  public: virtual void UpdateGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig, AMCLSensorData *data);

  // Figures
  private: rtk_fig_t *fig, *map_fig;
#endif
};




#endif
