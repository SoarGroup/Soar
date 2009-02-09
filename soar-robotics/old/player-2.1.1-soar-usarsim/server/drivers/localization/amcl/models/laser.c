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
 * Desc: Sensor model the laser sensor.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: laser.c 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "laser.h"

#define LASER_MAX_RANGES 401

// Pre-compute the range sensor probabilities
void laser_precompute(laser_t *self);


// Create an sensor model
laser_t *laser_alloc(map_t *map)
{
  laser_t *self;

  self = calloc(1, sizeof(laser_t));

  self->map = map;
  self->laser_pose = pf_vector_zero();

  self->range_cov = 0.10 * 0.10;
  self->range_bad = 0.50;

  laser_precompute(self);
    
  self->range_count = 0;
  self->ranges = calloc(LASER_MAX_RANGES, sizeof(laser_range_t));
  
  return self;
}


// Free an sensor model
void laser_free(laser_t *self)
{
  free(self->lut_probs);
  free(self->ranges);
  free(self);
  return;
}


// Clear all existing range readings
void laser_clear_ranges(laser_t *self)
{
  self->range_count = 0;
  return;
}


// Set the laser range readings that will be used.
void laser_add_range(laser_t *self, double range, double bearing)
{
  laser_range_t *beam;
  
  assert(self->range_count < LASER_MAX_RANGES);
  beam = self->ranges + self->range_count++;
  beam->range = range;
  beam->bearing = bearing;

  return;
}


// Pre-compute the range sensor probabilities.
// We use a two-dimensional array over (model_range, obs_range).
// currently, only the difference (obs_range - model_range) is significant,
// so this is somewhat inefficient.
void laser_precompute(laser_t *self)
{
  double max;
  double c, z, p;
  double mrange, orange;
  int i, j;
  
  // Laser max range and resolution
  max = 8.00;
  self->lut_res = 0.01;
  
  self->lut_size = (int) ceil(max / self->lut_res);
  self->lut_probs = malloc(self->lut_size * self->lut_size * sizeof(self->lut_probs[0]));

  for (i = 0; i < self->lut_size; i++)
  {
    mrange = i * self->lut_res;
    
    for (j = 0; j < self->lut_size; j++)
    {
      orange = j * self->lut_res;

      // TODO: proper sensor model (using Kolmagorov?)
      // Simple gaussian model
      c = self->range_cov;
      z = orange - mrange;
      p = self->range_bad + (1 - self->range_bad) * exp(-(z * z) / (2 * c));

      //printf("%f %f %f\n", orange, mrange, p);
      //assert(p >= 0 && p <= 1.0);
      
      self->lut_probs[i + j * self->lut_size] = p;
    }
    //printf("\n");
  }

  // TODO
  // Put beyond-max-range probabilities at the boundary of the LUT

  return;
}


// Determine the probability for the given range reading
inline double laser_sensor_prob(laser_t *self, double obs_range, double map_range)
{
  int i, j;
  double p;

  i = (int) (map_range / self->lut_res + 0.5);
  j = (int) (obs_range / self->lut_res + 0.5);

  assert(i >= 0);
  if (i >= self->lut_size)
    i = self->lut_size - 1;

  assert(j >= 0);
  if (j >= self->lut_size)
    j = self->lut_size - 1;

  p = self->lut_probs[i + j * self->lut_size];

  //assert(p >= 0 && p <= 1.0);
  
  return p;
}


// Determine the probability for the given pose
double laser_sensor_model(laser_t *self, pf_vector_t pose)
{
  int i;
  double p;
  double map_range;
  laser_range_t *obs;

  // Take account of the laser pose relative to the robot
  pose = pf_vector_coord_add(self->laser_pose, pose);

  p = 1.0;
  
  for (i = 0; i < self->range_count; i++)
  {
    obs = self->ranges + i;

    map_range = map_calc_range(self->map,
                               pose.v[0], pose.v[1], pose.v[2] + obs->bearing, 8.0);

    if (obs->range >= 8.0 && map_range >= 8.0)
      p *= 1.0;
    else if (obs->range >= 8.0 && map_range < 8.0)
      p *= self->range_bad;
    else if (obs->range < 8.0 && map_range >= 8.0)
      p *= self->range_bad;
    else
      p *= laser_sensor_prob(self, obs->range, map_range);
  }

  //printf("%e\n", p);
  assert(p >= 0);

  return p;
}

