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
 * Desc: Sensor model the sonar sensor.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: sonar.c 1359 2003-05-08 17:22:19Z inspectorg $
 * Notes:
 *   This is just a sketch of the sensor model; much work needs to be done
 * to make this model genuinely useful.
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "sonar.h"


// Pre-compute the range sensor probabilities
void sonar_precompute(sonar_t *self);


// Create an sensor model
sonar_t *sonar_alloc(map_t *map, int pose_count, pf_vector_t *poses)
{
  int i;
  sonar_t *self;

  self = calloc(1, sizeof(sonar_t));

  self->map = map;

  self->pose_count = pose_count;
  assert(self->pose_count < sizeof(self->poses) / sizeof(self->poses[0]));
  for (i = 0; i < self->pose_count; i++)
    self->poses[i] = poses[i];

  self->range_cov = 0.20 * 0.20;
  self->range_bad = 0.20;
  self->range_max = 2.0;

  self->range_count = 0;

  // Precompute the sensor model
  sonar_precompute(self);
  
  return self;
}


// Free an sensor model
void sonar_free(sonar_t *self)
{
  free(self->lut_probs);
  free(self);
  return;
}


// Clear all existing range readings
void sonar_clear_ranges(sonar_t *self)
{
  self->range_count = 0;
  return;
}


// Set the sonar range readings that will be used.
void sonar_add_range(sonar_t *self, double range)
{
  assert(self->range_count < SONAR_MAX_RANGES);
  self->ranges[self->range_count++] = range;
  return;
}


// Pre-compute the range sensor probabilities.
// We use a two-dimensional array over (model_range, obs_range).
// currently, only the difference (obs_range - model_range) is significant,
// so this is somewhat inefficient.
void sonar_precompute(sonar_t *self)
{
  double max;
  double c, z, p;
  double mrange, orange;
  int i, j;
  
  // Sonar max range and resolution
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
      p = self->range_bad + (1 - self->range_bad) *
        (1 / (sqrt(2 * M_PI * c)) * exp(-(z * z) / (2 * c)));

      self->lut_probs[i + j * self->lut_size] = p;
    }
  }

  // TODO
  // Put beyond-max-range probabilities at the boundary of the LUT

  return;
}


// Determine the probability for the given range reading
inline double sonar_sensor_prob(sonar_t *self, double obs_range, double map_range)
{
  int i, j;

  i = (int) (map_range / self->lut_res + 0.5);
  j = (int) (obs_range / self->lut_res + 0.5);

  assert(i >= 0);
  if (i >= self->lut_size)
    i = self->lut_size - 1;

  assert(j >= 0);
  if (j >= self->lut_size)
    j = self->lut_size - 1;

  return self->lut_probs[i + j * self->lut_size];
}


// Determine the probability for the given pose
double sonar_sensor_model(sonar_t *self, pf_vector_t pose)
{
  int i;
  double p;
  double map_range, obs_range;
  pf_vector_t spose;

  p = 1.0;
  
  for (i = 0; i < self->range_count; i++)
  {
    // Get the observed range
    obs_range = self->ranges[i];

    // Ignore long range readings
    if (obs_range > self->range_max)
      continue;

    // Compute the sonar pose in absolue coordinates
    spose = pf_vector_coord_add(self->poses[i], pose);

    map_range = map_calc_range(self->map,
                               spose.v[0], spose.v[1], spose.v[2], 8.0);

    p *= sonar_sensor_prob(self, obs_range, map_range);
  }

  //printf("%e\n", p);
  assert(p >= 0);

  return p;
}

