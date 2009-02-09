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
// Desc: Odometry sensor model for AMCL
// Author: Andrew Howard
// Date: 17 Aug 2003
// CVS: $Id: amcl_odom.h 4135 2007-08-23 19:58:48Z gerkey $
//
///////////////////////////////////////////////////////////////////////////

#ifndef AMCL_ODOM_H
#define AMCL_ODOM_H

#include "amcl_sensor.h"
#include "pf/pf_pdf.h"


// Odometric sensor data
class AMCLOdomData : public AMCLSensorData
{
  // Odometric pose
  public: pf_vector_t pose;

  // Change in odometric pose
  public: pf_vector_t delta;
};


// Odometric sensor model
class AMCLOdom : public AMCLSensor
{
  // Default constructor
  public: AMCLOdom(AdaptiveMCL & aAMCL, player_devaddr_t addr);

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

  // Update the filter based on the action model.  Returns true if the filter
  // has been updated.
  public: virtual bool UpdateAction(pf_t *pf, AMCLSensorData *data);

  // The action model callback (static method)
  public: static void ActionModel(AMCLOdom *self, pf_sample_set_t* set);
  
  // Device info
  private: player_devaddr_t odom_addr;
  private: Device *odom_dev;

  // Current data timestamp
  private: double time;
  
  // Drift model
  private: pf_matrix_t drift;
  
  // PDF used to generate action samples
  private: pf_pdf_gaussian_t *action_pdf;

#ifdef INCLUDE_RTKGUI
  // Setup the GUI
  private: virtual void SetupGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);

  // Finalize the GUI
  private: virtual void ShutdownGUI(rtk_canvas_t *canvas, rtk_fig_t *robot_fig);
#endif
};




#endif
