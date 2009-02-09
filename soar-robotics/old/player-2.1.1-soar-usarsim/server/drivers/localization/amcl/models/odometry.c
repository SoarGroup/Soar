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
/**************************************************************************
 * Desc: Sensor/action model odometry.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: odometry.c 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "odometry.h"


// Create sensor model
odometry_t *odometry_alloc()
{
  odometry_t *self;

  self = calloc(1, sizeof(odometry_t));
  
  return self;
}


// Free an sensor model
void odometry_free(odometry_t *self)
{
  free(self);
  return;
}


// Prepare to initialize the distribution
void odometry_init_init(odometry_t *self, pf_vector_t mean, pf_matrix_t cov)
{
  self->init_pdf = pf_pdf_gaussian_alloc(mean, cov);
  return;
}


// Finish initializing the distribution
void odometry_init_term(odometry_t *self)
{
  pf_pdf_gaussian_free(self->init_pdf);
  self->init_pdf = NULL;
  return;
}


// The sensor initialization function
pf_vector_t odometry_init_model(odometry_t *self)
{
  return pf_pdf_gaussian_sample(self->init_pdf);
}


// Prepare to update the distribution using the action model.
void odometry_action_init(odometry_t *self, pf_vector_t old_pose, pf_vector_t new_pose)
{
  pf_vector_t x;
  pf_matrix_t cx;
  double ux, uy, ua;
  
  x = pf_vector_coord_sub(new_pose, old_pose);

  // HACK - FIX
  ux = 0.2 * x.v[0];
  uy = 0.2 * x.v[1];
  ua = fabs(0.2 * x.v[2]) + fabs(0.2 * x.v[0]);

  cx = pf_matrix_zero();
  cx.m[0][0] = ux * ux;
  cx.m[1][1] = uy * uy;
  cx.m[2][2] = ua * ua;

  //printf("x = %f %f %f\n", x.v[0], x.v[1], x.v[2]);
  
  // Create a pdf with suitable characterisitics
  self->action_pdf = pf_pdf_gaussian_alloc(x, cx); 

  return;
}


// Finish updating the distrubiotn using the action model
void odometry_action_term(odometry_t *self)
{
  pf_pdf_gaussian_free(self->action_pdf);
  self->action_pdf = NULL;
  return;
}


// The action model function
pf_vector_t odometry_action_model(odometry_t *self, pf_vector_t pose)
{
  pf_vector_t z, npose;
  
  z = pf_pdf_gaussian_sample(self->action_pdf);
  npose = pf_vector_coord_add(z, pose);
    
  return npose; 
}

