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
/**************************************************************************
 * Desc: Sensor model for IMU.
 * Author: Andrew Howard
 * Date: 8 Aug 2003
 * CVS: $Id: imu.h 1685 2003-08-17 18:51:44Z inspectorg $
 *************************************************************************/

#ifndef IMU_H
#define IMU_H

#include "../pf/pf.h"
#include "../pf/pf_pdf.h"

#ifdef __cplusplus
extern "C" {
#endif

  
// Model information
typedef struct
{
  // UTM heading
  double utm_head;
  
  // Expected bearing error
  double err_head;
  
} imu_model_t;


// Create an sensor model
imu_model_t *imu_alloc();

// Free an sensor model
void imu_free(imu_model_t *sensor);

// Set the observed heading (utm)
void imu_set_utm(imu_model_t *self, double utm_head);

// The sensor model function
double imu_sensor_model(imu_model_t *self, pf_vector_t pose);

#ifdef __cplusplus
}
#endif

#endif

