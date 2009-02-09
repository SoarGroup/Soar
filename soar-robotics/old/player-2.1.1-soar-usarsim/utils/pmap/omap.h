/*
  pmap: simple mapping utilities
  Copyright (C) 2004 Andrew Howard  ahoward@usc.edu

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/** @file omap.h

@brief The omap library creates occupancy grids from aligned laser data.


*/

#ifndef OMAP_H
#define OMAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "slap.h"


/// @brief omap object data
typedef struct
{
  /// Number of range points in each scan
  int num_ranges;
  
  /// Maximum accepted range
  double range_max;

  /// Start angle for scans
  double range_start;

  /// Angular resolution of scans
  double range_step;
  
  /// Grid dimensions
  double grid_res;
  int grid_sx, grid_sy, grid_size;

  /// Match grid (contains projected boundary points)
  signed char *grid;
  
} omap_t;


/// @brief Allocate object  
omap_t *omap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step,
                   double grid_width, double grid_height, double grid_res);

/// @brief Free object
void omap_free(omap_t *self);

/// @brief Clear the map.
void omap_clear(omap_t *self);

/// @brief Add a scan to the map
void omap_add(omap_t *self, pose2_t pose, int num_ranges, double *ranges);

/// @brief Save as PGM format
/// @returns Returns non-zero on error.
int omap_save_pgm(omap_t *self, const char *filename);

/// @brief Draw the current map.
/// @param scale Pixel scale (m/pixel).
void omap_draw_map(omap_t *self, double scale);

/// @brief Grid access macros
#define OMAP_GRIDX(self, x) ((int) floor((x) / self->grid_res) + self->grid_sx / 2)
#define OMAP_GRIDY(self, y) ((int) floor((y) / self->grid_res) + self->grid_sy / 2)
#define OMAP_GRID_VALID(self, x, y) ((x) >= 0 && (x) < self->grid_sx && \
                                     (y) >= 0 && (y) < self->grid_sy)
#define OMAP_GRID_INDEX(self, x, y) ((x) + (y) * self->grid_sx)

#ifdef __cplusplus
}
#endif

#endif
