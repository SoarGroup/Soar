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

/** @file rmap.h

@brief The rmap library performs relaxation over a set of roughly
aligned laser scans to find the optimal map.

@par Caveats

This library has not yet been optimized, so it please be patient when
using it.

*/

#ifndef RMAP_H
#define RMAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "slap.h"

/// Limits
#define RMAP_MAX_RANGES 1024


/// @brief Data for a single scan
typedef struct
{
  /// Index of scan (use for indexing vectors during fitting)
  int index;

  /// Initial (uncorrected) pose.
  /// @todo Remove (not very useful).
  pose2_t init;
  
  /// Scan pose
  pose2_t pose;

  /// Correction to pose
  pose2_t delta;

  /// Raw range list
  double ranges[RMAP_MAX_RANGES];
  
  /// List of hit points
  int num_hits;
  vector2_t hits[RMAP_MAX_RANGES];
  
} rmap_scan_t;


/// @brief Data for grid cell items; each cell is a linked list of
/// boundary points.
typedef struct rmap_item
{
  /// Scan pointer
  rmap_scan_t *scan;

  /// Position relative to scan
  vector2_t local;

  /// Position in global frame
  vector2_t global;
  
  /// Index of next item
  struct rmap_item *next;
  
} rmap_item_t;


/// @brief Data for a grid cell
typedef struct
{
  /// Index of first and last items
  rmap_item_t *first, *last;
  
} rmap_cell_t;


/// @brief Data for a constraint
typedef struct
{
  /// Related scans
  rmap_scan_t *scan_a, *scan_b;

  /// Local points in each scan
  vector2_t local_a, local_b;

  /// Distance between points
  double dist;
  
} rmap_constraint_t;


/// @brief rmap object data
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
  
  /// Maximum distance for matching points
  double max_dist;

  /// Interval between key-ranges
  int range_interval;

  /// Interval between key-scans
  int key_interval;
  
  /// Scan list
  int num_scans, max_scans;
  rmap_scan_t *scans;

  /// Number of key scans in list
  int num_key_scans;

  /// Grid dimensions
  double grid_res;
  int grid_sx, grid_sy, grid_size;

  /// Match grid (contains projected boundary points)
  rmap_cell_t *grid;

  /// Flat list of items from the grid
  int num_items, max_items, items_size;
  rmap_item_t *items;

  /// Constraint list
  int num_cons, max_cons;
  rmap_constraint_t *cons;

  /// Useful counters for keeping stats
  int match_count;
  double relax_err;
  
} rmap_t;


/// @brief Allocate object  
rmap_t *rmap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step,
                   double grid_width, double grid_height);

/// @brief Free object
void rmap_free(rmap_t *self);

/// @brief Add a scan to the map
void rmap_add(rmap_t *self, pose2_t pose, int num_ranges, double *ranges);

/// @brief Match points across scans
void rmap_match(rmap_t *self);

/// @brief Project bounds into the grid
/// @internal
void rmap_match_prepare(rmap_t *self, rmap_scan_t *scan);

/// @brief Match points for a single scan.
/// @internal
void rmap_match_scan(rmap_t *self, rmap_scan_t *scan_a);

/// @brief Relax key-scans
/// @param num_cycles Number of optimization cycles.
void rmap_relax(rmap_t *self, int num_cycles);

/// @brief Interpolate between key-scans 
void rmap_interpolate(rmap_t *self);

/// @brief Draw the current map
void rmap_draw_map(rmap_t *self);

/// @brief Draw constraints
void rmap_draw_cons(rmap_t *self);


#ifdef __cplusplus
}
#endif

#endif
