/*
  LODO library: laser-stabilized odometry
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

/** @file lodo.h

@brief The lodo library provides laser-stabilizied odometric pose
estimates: the inputs are raw odometry and laser scans, the output is
a corrected pose estimate.  The drift in this corrected estimate is
much lower than that seen with odometry alone.  See, for example, the
figure below: this plot shows the trajectory of a robot that has
travelled 125m and executed 19 complete rotations before returning to
the starting location.  The final cumulative orientation error is less
than 5 degrees (versus 110 degrees for pure odometry).

@image html lodo_comparison.gif "Odometry versus laser-stabilized odometry"


@par How it works

The lodo library uses an incremental SLAM algorithm to correct drift
in the robot's orientation.  The algorithm has three key data structures:

- The current laser scan (a list of range values).
- The local map (a ring buffer containing recent laser scans).
- The <i>polar error function</i> (a grid generated from the local map).

The algorithm applied to each new laser scan is as follows:

-# Generate the polar error function:
  - For each scan in the local map:
    - For each point on the polygonal boundary of the map scan:
      - Project point into the local coordinate frame of the new scan.
      - Convert to polar coordinates and mark the corresponding cell
        in the polar error function as "occupied".
  - For each cell in the polar error function, compute the distance
    to the nearest occupied cell with the same radial value.
-# Find orientation correction:
  - For each possible correction:
    - Apply correction to each point in the new scan and convert into
      polar coordinates.
    - Look up error value in the polar error function.
    - Add to the todal error for this correction value.
  - Select correction value with the lowest total error.
-# Build map:
  - If the new scan has a signficant number of "outlier" points
    (large error values), add this scan to the current map.  The map
    is a ring buffer, so adding a new scan will necessarily lead to
    an old one being discarded.

The intuition behind this algorithm is best explained in diagrams (below).

Figure (a) shows a map scan in global carestian coordinates; the map
scan is projected into the coordinate frame of the new scan and
converted to polar coordinates (b); using dynamic programming, we
compute the error function (c).  This polar error function can be used
to test possible orientation corrections very efficiently: changes in
the orientation of the new scan correspond the translations of the
polar error function.

<table align="center">
<tr>
<td> @image html lodo_map.gif "(a) Original map scan (cartesian coordinates)."
<td> @image html lodo_map_polar.gif "(b) Map scan in local polar coordinates."
<tr><td colspan=2> @image html lodo_map_pef.gif "(c) Calculated polar error function."
</table>


@par lodo_caveats

The current version of the library takes about 15ms to process each
scan on a 2.8 GHz P4, so expect it to use lots of cycles on your
robot.  There is still lots of optimization to do, however, so expect
future releases to clock in around the 5ms mark.

@todo Accuracy, reliability and performance 

*/

#ifndef LODO_H
#define LODO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <gsl/gsl_min.h>
#include "slap.h"


/// Limits
#define LODO_MAX_RANGES 1024


/// @brief Data pertaining to an individual scan
typedef struct
{
  /// Odometric pose (x, y, theta)
  pose2_t opose;

  /// Corrected pose (x, y, theta)
  pose2_t cpose;
  
  /// Range values
  double ranges[LODO_MAX_RANGES];
  
} lodo_scan_t;


/// @brief Working data for a map point
typedef struct
{
  /// Local cartesian position
  vector3_t local;

  /// Local polar position
  vector3_t polar;
  
} lodo_map_point_t;


/// @brief Working data for scan point
typedef struct
{
  /// Range bin in PEF
  int ni;
  
} lodo_scan_point_t;


/// @brief Laser odometry module data
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

  /// Laser pose relative to robot
  pose2_t laser_pose;

  /// Maximum error value on a single point (m)
  double max_error;

  /// Number of bins in polar error function
  int pef_num_ranges, pef_num_bearings;

  /// Range bin starting value and resolution in polar error function.
  double pef_range_start, pef_range_step;

  /// Bearing bin starting value and resolution in polar error
  /// function.  Note that the pef_bearing_step measures a distance,
  /// not an angle.
  double pef_bearing_start, pef_bearing_step;

  /// Polar error function (map)
  int pef_size;
  int *pef;

  /// Search interval for fitting (fitting will check interval
  /// [-fit_interval, +fit_interval] radians).
  double fit_interval;
  
  /// Error threshold for good fits (error must be less than this
  /// value for a good fit).
  double fit_err_thresh;

  /// Error value for outlier points (points with error values larger
  /// than this are labeled as outliers).
  double fit_outlier_dist;

  /// Outlier fraction threshold (used to decide when scans should be
  /// added to the map).
  double fit_outlier_frac;

  /// Cumulative odometric distance and rotation
  double odom_dist, odom_turn;

  /// Odometer interval for adding scans.
  double map_dist_interval, map_turn_interval;

  /// Odometer value of last scan in map
  double map_last_dist, map_last_turn;

  /// Map (ring buffer of scans)
  int map_scan_count, max_map_scans;
  lodo_scan_t *map_scans;

  /// Current scan
  int scan_count;
  lodo_scan_t scan;

  /// Working space for current scan (useful pre-computed values).
  lodo_scan_point_t scan_points[LODO_MAX_RANGES];

  /// Working space for projected map points
  int num_map_points, max_map_points;
  lodo_map_point_t *map_points;

  /// Minimizer
  gsl_min_fminimizer *mini;

  /// Fit correction
  double fit_correct;

  /// True if last scan fitted
  int fit_valid;

  /// True if the last scan should be added to the map
  int fit_add;
  
} lodo_t;


/// @brief Allocate object
/// @param num_ranges Number of range readings in each scan (should be 181).
/// @param range_max Maximum useable range value (e.g., 8.00 or 16.00).
/// @param range_res Resolution for comparing range values (i.e., range bin width).
/// @param range_start Starting angle for range readings (should be -M_PI / 2).
/// @param range_step Angular step size for each successive range reading
///                   (should be M_PI / 180).
/// @returns Object handle.
lodo_t *lodo_alloc(int num_ranges, double range_max, double range_res,
                   double range_start, double range_step);

/// @brief Free object
void lodo_free(lodo_t *self);

/// @brief Add a scan, compute correction, update map
/// @param self Object handle.
/// @param odom_pose Raw odometric pose of the robot.
/// @param num_ranges Number of range readings.
/// @param ranges Array of laser range readings.
/// @returns Returns the corrected robot pose.
pose2_t lodo_add_scan(lodo_t *self, pose2_t odom_pose, int num_ranges, double *ranges);

/// @brief Project map points into scan polar coordinates
/// @internal
void lodo_project_map(lodo_t *self);

/// @brief Project map free space into the scan polar frame
/// @internal
void lodo_project_map_free(lodo_t *self, lodo_scan_t *scan_m,
                           matrix33_t Pd, matrix33_t Pm);

/// @brief Compute correction for a scan
/// @internal
double lodo_correct(lodo_t *self);

/// @brief Test a scan offset
/// @internal
double lodo_test_offset(lodo_t *self, double offset, double *outliers);

/// @brief Print projected map points
void lodo_print_map(lodo_t *self);

/// @brief Print a polar error functions for scan and map
void lodo_print_pef(lodo_t *self);

/// @brief Print error histograms
void lodo_print_err(lodo_t *self);

/// @brief Draw a scan
void lodo_draw_scan(lodo_t *self, lodo_scan_t *scan);

/// @brief Draw the current map (hits)
void lodo_draw_map(lodo_t *self);

#ifdef __cplusplus
}
#endif

#endif
