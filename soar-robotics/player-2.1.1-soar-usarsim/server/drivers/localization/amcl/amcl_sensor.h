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
// Desc: Adaptive Monte-Carlo localization
// Author: Andrew Howard
// Date: 6 Feb 2003
// CVS: $Id: amcl_sensor.h 6496 2008-06-09 22:32:57Z thjc $
//
///////////////////////////////////////////////////////////////////////////

#ifndef AMCL_SENSOR_H
#define AMCL_SENSOR_H

#include "amcl.h"

#include <libplayercore/playercore.h>
#include "pf/pf.h"


// Forward declarations
class AMCLSensorData;


// Base class for all AMCL sensors
class AMCLSensor
{
  // Default constructor
  public: AMCLSensor(AdaptiveMCL & aAMCL);
         
  // Default destructor
  public: virtual ~AMCLSensor();

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
                                     void * data) = 0;  
//  public: virtual AMCLSensorData *GetData(void);
  
  // Update the filter based on the action model.  Returns true if the filter
  // has been updated.
  public: virtual bool UpdateAction(pf_t *pf, AMCLSensorData *data);

  // Initialize the filter based on the sensor model.  Returns true if the
  // filter has been initialized.
  public: virtual bool InitSensor(pf_t *pf, AMCLSensorData *data);

  // Update the filter based on the sensor model.  Returns true if the
  // filter has been updated.
  public: virtual bool UpdateSensor(pf_t *pf, AMCLSensorData *data);

  // Flag is true if this is the action sensor
  public: bool is_action;

  // Action pose (action sensors only)
  public: pf_vector_t pose;

  // AMCL Base
  protected: AdaptiveMCL & AMCL;

#ifdef INCLUDE_RTKGUI
  // Setup the GUI
  public: virtual void SetupGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);

  // Finalize the GUI
  public: virtual void ShutdownGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);

  // Draw sensor data
  public: virtual void UpdateGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig, AMCLSensorData *data);
#endif
};



// Base class for all AMCL sensor measurements
class AMCLSensorData
{
  // Pointer to sensor that generated the data
  public: AMCLSensor *sensor;
          virtual ~AMCLSensorData() {}

  // Data timestamp
  public: double time;
};



#endif
