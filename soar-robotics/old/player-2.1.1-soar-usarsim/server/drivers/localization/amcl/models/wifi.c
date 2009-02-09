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
 * Desc: Sensor model for WiFi
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: wifi.c 1512 2003-06-02 15:33:45Z inspectorg $
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "wifi.h"


// Create an sensor model
wifi_t *wifi_alloc(map_t *map)
{
  wifi_t *self;

  self = calloc(1, sizeof(wifi_t));

  self->map = map;
  self->level_count = 0;
  
  return self;
}


// Free an sensor model
void wifi_free(wifi_t *self)
{
  free(self);
  return;
}


// Set the observed wifi levels
void wifi_set_levels(wifi_t *self, int level_count, int levels[])
{
  int i;
  
  self->level_count = level_count;
  for (i = 0; i < level_count; i++)
    self->levels[i] = levels[i];
  
  return;
}


// The sensor model function
double wifi_sensor_model(wifi_t *self, pf_vector_t pose)
{  
  double p, z, a, c;
  int i;
  int mlevel, olevel;
  map_cell_t *cell;  

  cell = map_get_cell(self->map, pose.v[0], pose.v[1], pose.v[2]);
  if (!cell)
    return 0;

  // ** HACK
  a = 0.10;
  c = 10; 
  
  p = 1.0;

  for (i = 0; i < self->level_count; i++)
  {
    mlevel = cell->wifi_levels[i];
    olevel = self->levels[i];

    z = (olevel - mlevel);

    if (olevel == 0)
      p *= 1.0;
    else if (mlevel == 0)
      p *= 0.0;
    else
      p *= a + (1 - a) * exp(-(z * z) / (2 * c));
  }
  
  return p;
}

