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

/*
  Desc: Laser-odometry (incremental SLAM) package
  Author: Andrew Howard
  Date: 19 Nov 2004
  CVS: $Id: lodo.cpp 4424 2008-03-19 01:40:15Z gerkey $
*/

#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gsl/gsl_errno.h>

#include "lodo.h"

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif


// GSL-styled callback function for minimization
static double lodo_correct_func(double offset, lodo_t *self);


// Convert from local coords to PEF range index
#define LODO_LTOR(x) \
  ((int) floor(((x) - self->pef_range_start) / self->pef_range_step))

// Convert from local coords to PEF bearing index
#define LODO_LTOB(nx, y) \
   ((int) floor(((y) - self->pef_bearing_start) / (self->pef_bearing_step / ((nx) * self->pef_range_step + self->pef_range_start))))


// Create object
lodo_t *lodo_alloc(int num_ranges, double range_max, double range_res,
                   double range_start, double range_step)
{
  lodo_t *self;

  self = new lodo_t;
  memset(self,0,sizeof(lodo_t));

  self->num_ranges = num_ranges;
  self->range_max = range_max;
  self->range_start = range_start;
  self->range_step = range_step;

  self->laser_pose.pos.x = 0;
  self->laser_pose.pos.y = 0;
  self->laser_pose.rot = 0;

  self->max_error = 0.50;

  self->pef_range_step = range_res;
  self->pef_range_start = 0.50;
  self->pef_num_ranges = (int) ceil(self->range_max / self->pef_range_step);

  self->pef_bearing_step = 0.05;
  self->pef_bearing_start = -M_PI;
  self->pef_num_bearings = (int) ceil(2 * M_PI * (self->pef_num_ranges * self->pef_range_step + self->pef_range_start) / self->pef_bearing_step);

  self->pef_size = self->pef_num_ranges * self->pef_num_bearings * sizeof(self->pef[0]);
  self->pef = new int[self->pef_num_ranges * self->pef_num_bearings];
  memset(self->pef,0,self->pef_size);
  assert(self->pef);
//  printf("allocated pef of %d bytes\n",self->pef_size);

  gsl_set_error_handler_off();
  self->mini = gsl_min_fminimizer_alloc(gsl_min_fminimizer_brent);
  assert(self->mini);

  self->fit_interval = 30 * M_PI / 180; // 45
  self->fit_err_thresh = 0.20;
  self->fit_outlier_dist = 0.10;
  self->fit_outlier_frac = 0.10;

  self->odom_dist = 0.0;
  self->odom_turn = 0.0;

  self->scan_count = 0;

  self->map_scan_count = 0;
  self->max_map_scans = 10;
  self->map_scans = new lodo_scan_t[self->max_map_scans];

  // The array length here is somewhat abitrary, and lead to an
  // assertion failure at runtime
  self->num_map_points = 0;
  self->max_map_points = 10 * self->num_ranges * self->max_map_scans;
  self->map_points = new lodo_map_point_t[self->max_map_points];
  memset(self->map_points,0,self->max_map_points*sizeof(self->map_points[0]));

  self->map_dist_interval = 1.00;
  self->map_turn_interval = 45 * M_PI / 180;

  self->map_last_dist = -1000;
  self->map_last_turn = -1000;

  return self;
}


// Free object
void lodo_free(lodo_t *self)
{
  delete [] self->map_points;
  delete [] self->pef;
  delete [] self->map_scans;
  delete self;
  return;
}


// Add a scan to the ring buffer
pose2_t lodo_add_scan(lodo_t *self, pose2_t odom_pose, int num_ranges, double *ranges)
{
  lodo_scan_t scan;
  pose2_t delta;
  double delta_dist, delta_turn;

  // Check that scan is consistent
  assert(num_ranges == self->num_ranges);

  // Optionally add the old scan to the map
  if (self->scan_count > 0)
  {
    if (self->fit_valid || self->map_scan_count == 0)
    {
      if (self->fit_add ||
          self->odom_dist - self->map_last_dist > self->map_dist_interval ||
          self->odom_turn - self->map_last_turn > self->map_turn_interval)
      {
        self->map_last_dist = self->odom_dist;
        self->map_last_turn = self->odom_turn;
        self->map_scans[(self->map_scan_count++) % self->max_map_scans] = self->scan;
      }
    }
  }

  // Initialize the new scan
  scan.opose = odom_pose;
  assert(num_ranges == self->num_ranges);
  assert(num_ranges <= LODO_MAX_RANGES);
  memcpy(scan.ranges, ranges, num_ranges * sizeof(ranges[0]));

  if (self->scan_count == 0)
  {
    // Initialize odometer
    self->odom_dist = 0.0;
    self->odom_turn = 0.0;

    // Initialize pose
    scan.cpose = scan.opose;

    // Make this the current scan
    self->scan = scan;
    self->scan_count++;

    // Project map points into current polar frame
    lodo_project_map(self);

    // Fit is good by definition
    self->fit_correct = 0.0;
    self->fit_valid = 1;
    self->fit_add = 1;
  }
  else
  {
    // Compute change in pose based on this new scan
    delta = pose2_sub(odom_pose, self->scan.opose);

    // Update the odometer distance
    delta_dist = sqrt(delta.pos.x * delta.pos.x + delta.pos.y * delta.pos.y);
    delta_turn = fabs(delta.rot);

    // TESTING
    //if (delta_dist < 0.001 && delta_turn < 0.001)
    //  return;

    self->odom_dist += delta_dist;
    self->odom_turn += delta_turn;

    // Compute new (uncorrected) pose based on this new scan
    scan.cpose = pose2_add(delta, self->scan.cpose);

    // Make this the current scan
    self->scan = scan;
    self->scan_count++;

    // Project map points into current polar frame
    lodo_project_map(self);

    // Correct the scan
    lodo_correct(self);
  }

  return self->scan.cpose;
}


// Project map points into scan polar coordinates
void lodo_project_map(lodo_t *self)
{
  int m, count, max_count;
  int ni, nr;
  pose2_t lpose;
  matrix33_t Pm, Pd;
  lodo_scan_t *scan_m, *scan_d;
  int *pef, *dj, *dk;

  scan_d = &self->scan;

  // Initialize the PEF
  memset(self->pef, 0x7F, self->pef_size);

  // Initialize working space
  self->num_map_points = 0;

  // TODO: integrate with SLAP pose somehow
  // Homogeneous from global cartesian to data scan cartesian
  Pd = matrix33_set(cos(scan_d->cpose.rot), -sin(scan_d->cpose.rot), scan_d->cpose.pos.x,
                    sin(scan_d->cpose.rot), +cos(scan_d->cpose.rot), scan_d->cpose.pos.y,
                    0, 0, 1);
  Pd = matrix33_inv(Pd);

  // Project points into PEF
  for (m = 0; m < self->max_map_scans && m < self->map_scan_count; m++)
  {
    scan_m = self->map_scans + m;

    // Compute pose of laser in global cartesian
    lpose = pose2_add(self->laser_pose, scan_m->cpose);

    // Homogeneous from model cartesian to global cartesian
    Pm = matrix33_set(cos(lpose.rot), -sin(lpose.rot), lpose.pos.x,
                      sin(lpose.rot), +cos(lpose.rot), lpose.pos.y,
                      0, 0, 1);

    // Project free-space boundary for this scan
    lodo_project_map_free(self, scan_m, Pd, Pm);
  }

  // Use DP to compute PEF on each range value
  for (ni = 0; ni < self->pef_num_ranges; ni++)
  {
    // Starting pointer for this range bin
    pef = self->pef + ni * self->pef_num_bearings;

    // Max number of cells at this range
    max_count = (int) ceil(2 * M_PI *
                           (ni * self->pef_range_step + self->pef_range_start) /
                           self->pef_bearing_step);
    assert(max_count <= self->pef_num_bearings);

    // Step-wise distance along arc
    nr = static_cast <int> (1000 * fabs(self->pef_bearing_step));

    // Use DP to compute error values on arc (+ve direction).
    // This code is optimized.
    dj = pef + 1;
    dk = pef + 0;
    count = max_count - 1;
    while (count)
    {
      if (*dj > nr + *dk)
        *dj = nr + *dk;
      dj++;
      dk++;
      count--;
    }

    // Use DP to compute error values on arc (-ve direction).
    // This code is optimized.
    dj = pef + max_count - 2;
    dk = pef + max_count - 1;
    count = max_count - 1;
    while (count)
    {
      if (*dj > nr + *dk)
        *dj = nr + *dk;
      dj--;
      dk--;
      count--;
    }
  }

  return;
}


// Project map free space into the scan polar frame
void lodo_project_map_free(lodo_t *self, lodo_scan_t *scan_m,
                           matrix33_t Pd, matrix33_t Pm)
{
  int i, j;
  int ni, nj;
  double r;
  vector3_t a, b, p, q;
  double dx, dy, dr;
  lodo_map_point_t *free;
  double step_size;
  int step_count;

  // Make sure we hit each range bin
  step_size = self->pef_range_step * 0.9;

  // Project map free-space boundary into data polar frame
  for (i = 0; i <= self->num_ranges; i++)
  {
    // Homogeneous coordinates of line end-points in local cartesian
    // frame
    if (i == 0)
    {
      a.x = 0;
      a.y = 0;
      a.z = 1.0;
    }
    else
    {
      r = scan_m->ranges[i - 1];
      if (r > self->range_max)
        r = self->range_max;
      a.x = r * cos((i - 1) * self->range_step + self->range_start);
      a.y = r * sin((i - 1) * self->range_step + self->range_start);
      a.z = 1.0;
    }
    if (i < self->num_ranges)
    {
      r = scan_m->ranges[i];
      if (r > self->range_max)
        r = self->range_max;
      b.x = r * cos(i * self->range_step + self->range_start);
      b.y = r * sin(i * self->range_step + self->range_start);
      b.z = 1.0;
    }
    else
    {
      b.x = 0;
      b.y = 0;
      b.z = 1.0;
    }

    // Transform into global cartesian frame
    a = matrix33_mul_3(Pm, a);
    b = matrix33_mul_3(Pm, b);

    // Transform into data cartesian frame
    a = matrix33_mul_3(Pd, a);
    b = matrix33_mul_3(Pd, b);

    // Compute line parameters
    dx = b.x - a.x;
    dy = b.y - a.y;
    dr = sqrt(dx * dx + dy * dy);
    step_count = (int) floor(dr / step_size) + 2;
    dx /= (step_count - 1);
    dy /= (step_count - 1);

    // Walk the line
    for (j = 0; j < step_count; j++)
    {
      assert(self->num_map_points < self->max_map_points);
      free = self->map_points + self->num_map_points++;

      p.x = a.x + dx * j;
      p.y = a.y + dy * j;
      p.z = 1.0;
      free->local = p;

      // Transform into data polar frame
      q.x = sqrt(p.x * p.x + p.y * p.y);
      q.y = atan2(p.y, p.x);
      q.z = 0;
      free->polar = q;

      // Transform into polar grid coordinates
      ni = LODO_LTOR(q.x);
      nj = LODO_LTOB(ni, q.y);

      // Update the polar error function
      if (ni >= 0 && ni < self->pef_num_ranges &&
          nj >= 0 && nj < self->pef_num_bearings)
      {
        self->pef[ni * self->pef_num_bearings + nj] = 0;
      }
    }
  }

  return;
}


// Find correction
double lodo_correct(lodo_t *self)
{
  int i, ni, erc;
  vector2_t q;
  double r;
  double interval;
  double offset, err;
  double best_offset, best_err, best_outliers;
  gsl_function func;

  // Pre-compute some values used in the test function
  for (i = 0; i < self->num_ranges; i++)
  {
    // Get range relative to laser
    r = self->scan.ranges[i];
    if (r > self->range_max)
    {
      self->scan_points[i].ni = -1;
      continue;
    }

    // Compute cartesian points relative to robot
    q.x = r * cos(i * self->range_step + self->range_start);
    q.y = r * sin(i * self->range_step + self->range_start);    //printf("ni=%d\n",ni);

    q = pose2_add_pos(q, self->laser_pose);

    // Compute range relative to robot
    r = vector2_mag(q);
    ni = LODO_LTOR(r);
	//printf ("ni = %d\tself->pef_num_ranges = %d\n", ni, self->pef_num_ranges);
    if (ni < 0 || ni > self->pef_num_ranges)
      ni = -1;
    self->scan_points[i].ni = ni;
  }

  best_offset = 0.0;
  best_err = DBL_MAX;

  // Initialize the minimizer
  func.function = (double (*) (double, void*)) lodo_correct_func;
  func.params = self;
  erc = gsl_min_fminimizer_set(self->mini, &func,
                               0.0, -self->fit_interval, +self->fit_interval);

  // If the minimizer failes, revert to exhaustive search
  if (erc != GSL_SUCCESS)
  {
    //printf("brute force\n\n");
    best_err = DBL_MAX;
    for (i = -100; i <= 100; i++)
    {
      offset = i * self->fit_interval / 100.0;
      err = lodo_test_offset(self, offset, NULL);
      //printf("%d %f %f\n", i, offset, err);
      if (err < best_err)
      {
        best_err = err;
        best_offset = offset;
      }
    }
    //printf("\n\n");
  }
  else
  {
    for (i = 0; i < 10; i++) // HACK
    {
      erc = gsl_min_fminimizer_iterate(self->mini);
      if (erc == GSL_EBADFUNC)
        assert(0);
      if (erc == GSL_FAILURE)
        break;

      // Test for convergence
      interval = gsl_min_fminimizer_x_upper(self->mini);
      interval -= gsl_min_fminimizer_x_lower(self->mini);
      if (interval < 0.01 * M_PI / 180) // HACK
        break;
    }

    best_offset = gsl_min_fminimizer_x_minimum(self->mini);
    best_err = gsl_min_fminimizer_f_minimum(self->mini);
  }

  // Re-do the test to get the outlier count
  lodo_test_offset(self, best_offset, &best_outliers);

  // Check it his is a good fit.
  if (best_err > self->fit_err_thresh)
  {
    self->fit_valid = 0;
    self->fit_add = 0;
    self->fit_correct = 0;
    return 0;
  }

  self->fit_valid = 1;

  // See if should add this scan to the map.  The logic is this: if we
  // have a good fit, but there are lots of points that are "outliers"
  // (i.e., large-ish error value), then these points should be added
  // to the map.  Works for both turning in place and "bursting"
  // through doorways.
  if (fabs(best_offset) < 5 * M_PI / 180) // HACK
  {
    if (best_outliers > self->fit_outlier_frac)
      self->fit_add = 1;
    else
      self->fit_add = 0;
  }

  // Correct the scan pose
  self->fit_correct = best_offset;
  self->scan.cpose.rot = pose2_add_rot(self->fit_correct, self->scan.cpose);

  return 0;
}


// GSL-styled callback function for minimization
double lodo_correct_func(double offset, lodo_t *self)
{
  return lodo_test_offset(self, offset, NULL);
}


// Test a scan offset
double lodo_test_offset(lodo_t *self, double offset, double *outliers)
{
  int i, ni, nj;
  int outlier_dist, max_err;
  int err, m0, m1, ocount;

  outlier_dist = (int) (1000 * self->fit_outlier_dist);
  max_err = (int) (1000 * self->max_error);

  m0 = 0;
  m1 = 0;
  ocount = 0;

  offset += self->range_start;

  // Test all scan hits against the map pef
  for (i = 0; i < self->num_ranges; i++)
  {
    ni = self->scan_points[i].ni;
    if (ni < 0)
      continue;

    // Convert to polar grid coordinates
    nj = LODO_LTOB(ni, i * self->range_step + offset);

/*    printf("ni=%d\n",ni);
    printf("nj=%d\n",nj);
    printf("i=%d\n",i);
    printf("self->range_step=%f\n",self->range_step);
    printf("offset=%f\n",offset);
    printf("self->pef_num_bearings=%d\n",self->pef_num_bearings);
*/
    assert(nj >= 0 && nj < self->pef_num_bearings);
    assert(self->pef);

    //printf("ni * self->pef_num_bearings + nj=%d\n",ni * self->pef_num_bearings + nj);
    //printf("%d\n",self->pef[0]);
    //printf("%d\n",self->pef[ni * self->pef_num_bearings + nj]);


    err = self->pef[ni * self->pef_num_bearings + nj];
    if (err > max_err)
      err = max_err;

    // Count outliers
    if (err > outlier_dist)
      ocount += 1;

    m0 += 1;
    m1 += err;
  }

  if (m0 == 0)
    return 0;

  if (outliers)
    *outliers = (double) ocount / m0;

  return (double) m1 / m0 / 1000.0;
}


// Print projected map points
void lodo_print_map(lodo_t *self)
{
  int i;
  lodo_map_point_t *point;

  for (i = 0; i < self->num_map_points; i++)
  {
    point = self->map_points + i;
    printf("%f %f %f %f\n",
           point->local.x, point->local.y,
           point->polar.x, point->polar.y);
  }
  printf("\n\n");

  return;
}


// Print polar error function
void lodo_print_pef(lodo_t *self)
{
  int i, j;
  double r, b, err;

  for (i = 0; i < self->pef_num_ranges; i++)
  {
    for (j = 0; j < self->pef_num_bearings; j++)
    {
      err = self->pef[i * self->pef_num_bearings + j] / 1000.0;
      if (err > self->max_error)
        err = self->max_error;

      r = i * self->pef_range_step + self->pef_range_start;
      b = j * self->pef_bearing_step / r + self->pef_bearing_start;

      if (fabs(b) > M_PI)
        continue;

      printf("%d %d %f %f %f\n", i, j, r, b, err);
    }
    printf("\n");
  }
  printf("\n\n");

  fflush(stdout);

  return;
}


// Print error histogram
void lodo_print_err(lodo_t *self)
{
  /* TODO: retain somehow (for debugging)
  int i;

  for (i = 0; i < self->err_count; i++)
    printf("%f %f %f\n", i * self->err_step + self->err_start,
           self->err_m[i], self->err_o[i]);
  printf("\n\n");

  fflush(stdout);
  */

  return;
}


// Draw a scan
void lodo_draw_scan(lodo_t *self, lodo_scan_t *scan)
{
#ifdef HAVE_LIBGLUT
  int i;
  vector2_t p;

  /*
  glColor3f(0, 0, 0.5);
  glBegin(GL_LINE_LOOP);
  for (i = 0; i < self->num_ranges; i++)
  {
    p.x = scan->ranges[i] * cos(i * self->range_step + self->range_start);
    p.y = scan->ranges[i] * sin(i * self->range_step + self->range_start);
    glVertex3f(p.x, p.y, 0.0);
  }
  glEnd();
  */

  glColor3f(0, 0, 0.5);
  glBegin(GL_POINTS);
  for (i = 0; i < self->num_ranges; i++)
  {
    if (scan->ranges[i] > self->range_max)
      continue;
    p.x = scan->ranges[i] * cos(i * self->range_step + self->range_start);
    p.y = scan->ranges[i] * sin(i * self->range_step + self->range_start);
    p = pose2_add_pos(p, self->laser_pose);
    glVertex3f(p.x, p.y, 0.0);
  }
  glEnd();

  return;
#endif
}


// Draw current map (hits)
void lodo_draw_map(lodo_t *self)
{
#ifdef HAVE_LIBGLUT
  int i;
  vector3_t p;

  glBegin(GL_POINTS);
  for (i = 0; i < self->num_map_points; i++)
  {
    p = self->map_points[i].local;
    glVertex3f(p.x, p.y, 0.0);
  }
  glEnd();

  return;
#endif
}
