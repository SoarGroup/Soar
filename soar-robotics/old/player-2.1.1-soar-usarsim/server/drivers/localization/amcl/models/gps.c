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
 * Desc: Sensor model for GPS
 * Author: Andrew Howard
 * Date: 8 Aug 2003
 * CVS: $Id: gps.c 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "gps.h"


// Create an sensor model
gps_model_t *gps_alloc()
{
  gps_model_t *self;

  self = calloc(1, sizeof(gps_model_t));
  
  return self;
}


// Free an sensor model
void gps_free(gps_model_t *self)
{
  free(self);
  return;
}


// Set the observed gps coordinates (UTM grid coordinates)
void gps_set_utm(gps_model_t *self, double utm_e, double utm_n, double err_horz)
{
  self->utm_e = utm_e;
  self->utm_n = utm_n;
  self->err_horz = err_horz;
  return;
}


// Prepare to initialize the distribution
void gps_init_init(gps_model_t *self)
{
  pf_vector_t x;
  pf_matrix_t xc;
  
  x = pf_vector_zero();
  x.v[0] = self->utm_e - self->utm_base_e;
  x.v[1] = self->utm_n - self->utm_base_n;
  x.v[2] = 0;

  xc = pf_matrix_zero();
  xc.m[0][0] = self->err_horz * self->err_horz;
  xc.m[1][1] = self->err_horz * self->err_horz;
  xc.m[2][2] = M_PI;
  
  self->pdf = pf_pdf_gaussian_alloc(x, xc);

  return;
}


// Finish initializing the distribution
void gps_init_term(gps_model_t *self)
{
  pf_pdf_gaussian_free(self->pdf);
  return;
}


// The sensor initialization function
pf_vector_t gps_init_model(gps_model_t *self)
{
  return pf_pdf_gaussian_sample(self->pdf);
}


// The sensor model function.  This applies a simple circular
// Gaussian.
double gps_sensor_model(gps_model_t *self, pf_vector_t pose)
{  
  double dx, dy, sigma;
  double z, p;
  
  dx = (self->utm_e - self->utm_base_e) - pose.v[0];
  dy = (self->utm_n - self->utm_base_n) - pose.v[1];
  sigma = self->err_horz;
  
  z = (dx * dx + dy * dy) / (2 * sigma * sigma);
  p = exp(-z);
  
  return p;
}

