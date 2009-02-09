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
 * Desc: Sensor model for GPS.
 * Author: Andrew Howard
 * Date: 8 Aug 2003
 * CVS: $Id: gps.h 1685 2003-08-17 18:51:44Z inspectorg $
 *************************************************************************/

#ifndef GPS_H
#define GPS_H

#include "../pf/pf.h"
#include "../pf/pf_pdf.h"

#ifdef __cplusplus
extern "C" {
#endif

  
// Model information
typedef struct
{
  // UTM origin: UTM coord that maps to (0, 0) in global coords
  double utm_base_e, utm_base_n;
  
  // UTM grid coordinates
  double utm_e, utm_n;

  // Circular horizontal error
  double err_horz;

  // PDF used for initialization
  pf_pdf_gaussian_t *pdf;
  
} gps_model_t;


// Create an sensor model
gps_model_t *gps_alloc();

// Free an sensor model
void gps_free(gps_model_t *sensor);

// Set the observed gps coordinates (UTM grid coordinates)
void gps_set_utm(gps_model_t *self, double utm_e, double utm_n, double err_horz);

// Prepare to initialize the distribution
void gps_init_init(gps_model_t *self);

// Finish initializing the distribution
void gps_init_term(gps_model_t *self);

// The sensor initialization function
pf_vector_t gps_init_model(gps_model_t *self);

// The sensor model function
double gps_sensor_model(gps_model_t *self, pf_vector_t pose);

#ifdef __cplusplus
}
#endif

#endif

