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

/** @file pmap.h

@brief The pmap library performs simple particle-filter-based map
building (SLAM) for a single robot.  Generally speaking, the output
maps are topologically correct, but a little rough around the edges (some
minor scan mis-alignments).  For very precise results, the output maps
should be refined using the rmap library.

@par How it works

The library maintains a PF over possible maps (i.e., each particle, or
sample, represents a complete map).  Laser and odometry data is used
to incrementally update the filter; resampling is used to concentrate
particles around likely maps.


@par Caveats

- The space of possible maps is vast, and the particle set necessarily
represents a very sparse sampling of this space.  In practice, it
therefore necessary to pre-process raw odometry data (using the lodo
library, for example) to minimize the odometric drift rate.

- Maintaining a PF over maps is very memory intensive (the pmap
library stores a complete map for each particle).  For example, a 2500
sq. m map with 10cm resolution requires 0.25 Mb of storage for each
map, or 250 Mb of storage for 1000 particles.  When using this
library, take care not to exceed the physical memory of the machine.

- The algorithm has constant update time for each new sensor reading,
but this value scales linearly with the number of particles in the
filter.  With more than 1000 particles, the algorithm can be very
slow.

*/

#ifndef PMAP_H
#define PMAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gsl/gsl_rng.h"
#include "slap.h"


/// Limits
#define PMAP_MAX_RANGES 1024
  
  
/// @brief Structure for neighborhood lookup table
typedef struct
{
  /// Cell offset
  int dx, dy;

  /// Cell distance
  double dist;
  
} pmap_nbor_t;


/// @brief Structure storing a single range scan
typedef struct
{
  /// Range readings
  double ranges[PMAP_MAX_RANGES];
  
} pmap_scan_t;


/// @brief Structure describing a single sample
typedef struct
{
  /// Current sample error
  double w;
  
  /// Cummulative error for this sample
  double err;

  /// Current sample pose
  pose2_t pose;

  /// Working space for temporary results
  vector2_t *global_points;

  /// Sample trajectory
  pose2_t *poses;
  
  /// Grid map
  signed char *cells;

} pmap_sample_t;


  
/// @brief Structure describing the map
typedef struct
{
  /// Number of points in each scan
  int num_ranges;

  /// Maximum accepted range
  double range_max;

  /// Start angle for scans
  double range_start;

  /// Angular resolution of scans
  double range_step;

  /// Grid dimensions
  double grid_res;
  int grid_sx, grid_sy;

  /// Grid and trajectory dimensions (for copying samples)
  int traj_size, grid_size;

  /// Action model (coefficients for action distribution).
  matrix44_t action_model;

  /// Maximum error value
  double max_err;

  /// Interval (map update steps) between resampling
  int resample_interval;
  
  /// Standard deviation (sigma) value for resampling
  double resample_s;

  /// Neighborhood LUT
  int num_nbors;
  pmap_nbor_t *nbors;

  /// Number of scans processed
  int scan_count;

  /// Number of scans added to the sample map/trajectories
  int step_count;

  /// Maximum number of scans that can be added to the map/trajectories
  int step_max_count;

  /// List of stored range scans.
  pmap_scan_t *scans;

  /// Number of samples and current working sample set
  int samples_len, sample_set;

  /// List of samples (multiple sets, indexed by sample_set)
  pmap_sample_t *samples;

  /// Index of "best" sample
  int best_sample;
  
  /// Initial robot pose
  pose2_t initial_pose;

  /// Current odometric settings
  pose2_t odom_pose;
  double odom_dist, odom_turn;

  /// Odometer setting of last update
  double update_dist, update_turn;

  /// Pose count at last resample
  int resample_count;
  
  /// Random number generator
  gsl_rng *rng;

} pmap_t;


/// @brief Allocate object
/// @param num_ranges Number of range readings in each scan (should be 181).
/// @param range_max Maximum useable range value (e.g., 8.00 or 16.00).
/// @param range_start Starting angle for range readings (should be -M_PI / 2).
/// @param range_step Angular step size for each successive range reading.
/// @param samples_len Number of samples in filter.
/// @param grid_width Width of occupancy grid (cells).
/// @param grid_height Heigt of occupancy grid (cells).
/// @param grid_scale Size of each grid cell (m).
///                   (should be M_PI / 180).
pmap_t *pmap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step, int samples_len,
                   double grid_width, double grid_height, double grid_scale);

/// @brief Free object
void pmap_free(pmap_t *self);

/// @brief Create neighborhood LUT
/// @internal
void pmap_init_nbors(pmap_t *self);

/// @brief Set the initial robot pose.
/// This must be called before the first call to pmap_update().
void pmap_set_pose(pmap_t *self, pose2_t pose);

/// @brief Update map
void pmap_update(pmap_t *self, pose2_t odom_pose, int num_ranges, double *ranges);

/// @brief Apply the action model to the current sample set.
/// @internal
void pmap_apply_action(pmap_t *self, pose2_t delta);

/// @brief Add a scan to list
/// @internal
void pmap_add_scan(pmap_t *self, double *ranges);

/// Add a scan to a particular sample
/// @internal
void pmap_add_scan_sample(pmap_t *self, int sample_index, double *ranges);

/// @brief Apply sensor model to the current sample set.
/// @internal
void pmap_apply_sensor(pmap_t *self, double *ranges);

/// @brief Apply sensor model to a particular sample.
/// @internal
void pmap_apply_sensor_sample(pmap_t *self, int sample_index, double *ranges);

/// @brief Compute entropy
/// @internal
double pmap_entropy(pmap_t *self, int scan_count);

/// @brief Resample
/// @internal
void pmap_resample(pmap_t *self, int scan_count);

/// @brief Draw the current range scan
void pmap_draw_scan(pmap_t *self, double *ranges);

/// @brief Draw all samples
void pmap_draw_samples(pmap_t *self);

/// @brief Draw a particular sample
void pmap_draw_sample(pmap_t *self, int sample_index);

/// @brief Draw the current best map.
/// @param scale Pixel scale (m/pixel).
void pmap_draw_map(pmap_t *self, double scale);

/// @brief Draw a particular sample map
/// @param scale Pixel scale (m/pixel).
void pmap_draw_sample_map(pmap_t *self, double scale, int sample_index);

/// @brief Sample access macros
#define PMAP_GET_SAMPLE(self, i) (self->samples + self->sample_set * self->samples_len + (i))

/// @brief Grid access macros
#define PMAP_GRIDX(self, x) ((int) floor((x) / self->grid_res) + self->grid_sx / 2)
#define PMAP_GRIDY(self, y) ((int) floor((y) / self->grid_res) + self->grid_sy / 2)
#define PMAP_GRID_VALID(self, x, y) ((x) >= 0 && (x) < self->grid_sx && (y) >= 0 && (y) < self->grid_sy)
#define PMAP_GRID_INDEX(self, x, y) ((x) + (y) * self->grid_sx)

  
#ifdef __cplusplus
}
#endif

#endif
