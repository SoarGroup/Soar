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
/*
  Desc: Fast mapper
  Author: Andrew Howard
  Date: 19 Nov 2004
  CVS: $Id: pmap.cpp 4062 2007-05-05 11:53:08Z thjc $
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
#include <gsl/gsl_randist.h>

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif

#include "pmap.h"


// Create object
pmap_t *pmap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step, int samples_len,
                   double grid_width, double grid_height, double grid_scale)
{
  pmap_t *self;
  int i, sample_size, scans_size, total_size;
  pmap_sample_t *sample;
  
  self = new pmap_t;

  total_size = 0;
  
  self->num_ranges = num_ranges;
  self->range_max = range_max; 
  self->range_start = range_start;
  self->range_step = range_step;

  self->samples_len = samples_len;

  self->step_count = 0;
  self->step_max_count = 8192;
  
  self->grid_sx = (int) ceil(grid_width / grid_scale);
  self->grid_sy = (int) ceil(grid_height / grid_scale);
  self->grid_res = grid_scale;

  self->traj_size = self->step_max_count * sizeof(pose2_t);
  self->grid_size = self->grid_sx * self->grid_sy * sizeof(char);

  sample_size = self->grid_size + self->traj_size;
  total_size += self->samples_len * 2 * sample_size;
  
  self->samples = new pmap_sample_t[2 * self->samples_len];
  for (i = 0; i < 2 * self->samples_len; i++)
  {
    sample = self->samples + i;
    sample->global_points = new vector2_t[self->num_ranges];
    sample->poses = new pose2_t[self->traj_size];
    sample->cells = new signed char[self->grid_size];
  }

  // Allocate space for stored range scans
  scans_size = self->step_max_count;
  total_size += scans_size * sizeof (pmap_scan_t);
  printf("allocating %d bytes for scans\n",scans_size);
  self->scans = new pmap_scan_t[scans_size];

  // Print an lower-bound estimate of allocated memory
  fprintf(stderr, "allocating %d Mb of map space (estimated lower bound)\n",
          total_size / 1024 / 1024);
          
  self->max_err = 0.50;
  self->resample_interval = 20;
  self->resample_s = 0.30; // 0.20;  

  self->action_model.m[0][0] = 0.01;
  self->action_model.m[2][0] = 0.00;
  self->action_model.m[2][2] = 0.05;

  self->sample_set = 0;
  self->best_sample = 0;
  self->scan_count = 0;
  self->odom_dist = 0;
  self->odom_turn = 0;

  self->update_dist = -1000;
  self->update_turn = 0;
  self->resample_count = 0;

  pmap_init_nbors(self);
  
  self->rng = gsl_rng_alloc(gsl_rng_taus);
  assert(self->rng);
  
  return self;
}


// Free object
void pmap_free(pmap_t *self)
{
  int i;
  pmap_sample_t *sample;
  
  gsl_rng_free(self->rng);
  self->rng = NULL;
  
  for (i = 0; i < 2 * self->samples_len; i++)
  {
    sample = self->samples + i;
    delete [] sample->cells;
    delete [] sample->poses;
    delete [] sample->global_points;
  }
  delete [] self->samples;
  delete self;
  
  return;
}


// Sorting function for neighborhood cells
int pmap_sort_nbors(pmap_nbor_t *a, pmap_nbor_t *b)
{
  if (a->dist < b->dist)
    return -1;
  else if (a->dist > b->dist)
    return +1;
  else
    return 0;
}


// Create neighborhood LUT
void pmap_init_nbors(pmap_t *self)
{
  int s, dx, dy, count;
  pmap_nbor_t *nbor;

  s = (int) ceil(self->max_err / self->grid_res);
  
  self->num_nbors = (s * 2 + 1) * (s * 2 + 1);
  self->nbors = new pmap_nbor_t[self->num_nbors];

  // Fill list
  count = 0;
  for (dy = -s; dy <= s; dy++)
  {
    for (dx = -s; dx <= s; dx++)
    {
      nbor = self->nbors + count;
      nbor->dx = dx;
      nbor->dy = dy;
      nbor->dist = sqrt(dx * dx + dy * dy);
      if (nbor->dist * self->grid_res < self->max_err)
        count++;
    }
  }

  // Sort list from nearest to farthest
  qsort(self->nbors, count, sizeof(self->nbors[0]),
        (int(*) (const void*, const void*)) pmap_sort_nbors);

  return;
}


// Set the initial robot pose
void pmap_set_pose(pmap_t *self, pose2_t pose)
{
  int i;
  pmap_sample_t *sample;

  self->initial_pose = pose;
  
  for (i = 0; i < 2 * self->samples_len; i++)
  {
    sample = self->samples + i;
    sample->pose = pose;
  }
  return;
}


// Update the map
void pmap_update(pmap_t *self, pose2_t odom_pose, int num_ranges, double *ranges)
{
  int i;
  double best_err;
  pose2_t delta;
  pmap_sample_t *sample;

  // Sanity check
  assert(num_ranges == self->num_ranges);
  
  if (self->scan_count == 0)
  {
    self->odom_dist = 0.0;
    self->odom_turn = 0.0;
    self->odom_pose = odom_pose;

    // Add current scan to list
    pmap_add_scan(self, ranges);

    // Resample
    pmap_resample(self, 1);
  }
  else
  {
    // Update the odometer distance
    delta = pose2_sub(odom_pose, self->odom_pose);
    self->odom_dist += sqrt(delta.pos.x * delta.pos.x + delta.pos.y * delta.pos.y);
    self->odom_turn += fabs(delta.rot);
    self->odom_pose = odom_pose;

    // Update the pose of each sample using the action model
    pmap_apply_action(self, delta);
    
    // Update each sample maps and re-compute the sensor model
    if (self->odom_dist - self->update_dist > 0.10 ||
        self->odom_turn - self->update_turn > 15 * M_PI / 180) // HACK
    {
      self->update_dist = self->odom_dist;
      self->update_turn = self->odom_turn;
      
      // Apply the sensor model to each sample
      pmap_apply_sensor(self, ranges);
    
      // Add current range data to the map
      pmap_add_scan(self, ranges);

      self->step_count++;
    }

    // Resample
    if (self->step_count - self->resample_count > self->resample_interval)
    {
      pmap_resample(self, self->step_count - self->resample_count);
      self->resample_count = self->step_count;
    }

    // Find the best current sample
    best_err = DBL_MAX;
    for (i = 0; i < self->samples_len; i++)
    {
      sample = PMAP_GET_SAMPLE(self, i);
      if (sample->err < best_err)
      {
        best_err = sample->err;
        self->best_sample = i;
      }
    }
  }

  self->scan_count++;
  return;
}


// Apply the action model to the current sample set.
void pmap_apply_action(pmap_t *self, pose2_t delta)
{
  int i;
  pmap_sample_t *sample;
  vector4_t sigma;
  pose2_t offset;

  sigma.x = fabs(delta.pos.x);
  sigma.y = fabs(delta.pos.y);
  sigma.z = fabs(delta.rot);
  sigma.w = 1.0;
  sigma = matrix44_mul_4(self->action_model, sigma);

  for (i = 0; i < self->samples_len; i++)
  { 
    sample = PMAP_GET_SAMPLE(self, i);
    
    offset = delta;
    offset.pos.x += gsl_ran_gaussian(self->rng, sigma.x);
    offset.pos.y += gsl_ran_gaussian(self->rng, sigma.y);
    offset.rot += gsl_ran_gaussian(self->rng, sigma.z);

    sample->pose = pose2_add(offset, sample->pose);
  }
  
  return;
}


// Apply sensor model to the current sample set.
void pmap_apply_sensor(pmap_t *self, double *ranges)
{
  int i;
    
  // Compute error value for each sample
  for (i = 0; i < self->samples_len; i++)
    pmap_apply_sensor_sample(self, i, ranges);

  return;
}


// Apply sensor model to a particular sample.
void pmap_apply_sensor_sample(pmap_t *self, int sample_index, double *ranges)
{
  int i, j;
  int nx, ny, mx, my;
  int min_d;
  int occ;
  double r, b;
  double e, err;
  matrix33_t P;
  vector3_t p;
  pmap_sample_t *sample;
  pmap_nbor_t *nbor;
  
  // Set some pointers
  sample = PMAP_GET_SAMPLE(self, sample_index);  
  
  // Homogeneous from map scan local to global
  P = matrix33_set(cos(sample->pose.rot), -sin(sample->pose.rot), sample->pose.pos.x,
                   sin(sample->pose.rot), +cos(sample->pose.rot), sample->pose.pos.y,
                   0, 0, 1);

  err = 0.0;

  // Test current range set
  for (i = 0; i < self->num_ranges; i++)
  {
    r = ranges[i];
    if (r > self->range_max)
      continue;
    
    b = self->range_start + i * self->range_step;
    p.x = r * cos(b);
    p.y = r * sin(b);
    p.z = 1.0;

    // Convert to global coordinates
    p = matrix33_mul_3(P, p);

    // Store for later use
    sample->global_points[i].x = p.x;
    sample->global_points[i].y = p.y;
    
    // Convert to grid coordinates
    nx = PMAP_GRIDX(self, p.x);
    ny = PMAP_GRIDY(self, p.y);
    
    min_d = 1000;

    // Look for nearest neighbor
    for (j = 0; j < self->num_nbors; j++)
    {
      nbor = self->nbors + j;
      
      mx = nx + nbor->dx;
      my = ny + nbor->dy;        
      if (PMAP_GRID_VALID(self, mx, my))
        occ = (int) sample->cells[PMAP_GRID_INDEX(self, mx, my)];
      else
        occ = 0;
        
      if (occ > 8) // HACK; threshold
      {
        min_d = static_cast<int> (nbor->dist);
        break;
      }
    }

    // Update total error
    e = min_d * self->grid_res;
    if (e > self->max_err)
      e = self->max_err;
    err += e;
  }

  //err /= self->num_ranges;
  sample->w += err * err;
  sample->err += err * err;

  return;
}


// REMOVE?
// Compute entropy
double pmap_entropy(pmap_t *self, int scan_count)
{
  int i;
  pmap_sample_t *sample;
  double e, p, n, en;

  // Compute normalization const
  n = 0.0;
  for (i = 0; i < self->samples_len; i++)
  {
    sample = PMAP_GET_SAMPLE(self, i);
    e = sample->w / self->num_ranges / scan_count;
    p = exp(-e / (self->resample_s * self->resample_s));
    n += p;
  }

  // Compute entropy 
  en = 0.0;
  for (i = 0; i < self->samples_len; i++)
  {
    sample = PMAP_GET_SAMPLE(self, i);
    e = sample->w / self->num_ranges / scan_count;
    p = exp(-e / (self->resample_s * self->resample_s)) / n;
    // printf("%d %f %e\n", i, e, p);
    en += p * log(p);
  }

  en = -en;

  printf("entropy %f\n", en);

  return en;
}


// Resample
void pmap_resample(pmap_t *self, int scan_count)
{
  int i, n;
  double e, p, norm=0.0;
  gsl_ran_discrete_t *dist=NULL;
  pmap_sample_t *oldset=NULL, *newset=NULL;
  pmap_sample_t *old=NULL, *newsample=NULL;
  double *sample_probs;

  sample_probs = new double[self->samples_len];
    
  // Work out old and new sample sets
  oldset = self->samples + self->sample_set * self->samples_len;
  newset = self->samples + ((self->sample_set + 1) % 2) * self->samples_len;
  
  // Convert to probability 
  for (i = 0; i < self->samples_len; i++)
  {
    old = oldset + i;
    e = old->w / self->num_ranges / scan_count;
    p = exp(-e / (self->resample_s * self->resample_s));
    norm += p;
    sample_probs[i] = p;
  }

  // Normalize probabiities
  for (i = 0; i < self->samples_len; i++)
  {
    sample_probs[i] /= norm;
    assert(finite(sample_probs[i]));
  }

  dist = gsl_ran_discrete_preproc(self->samples_len, sample_probs);
  assert(dist);
    
  // Create discrete distribution
  for (i = 0; i < self->samples_len; i++)
  {
    assert(self);
    assert(self->rng);
    n = gsl_ran_discrete(self->rng, dist);

    old = oldset + n;
    newsample = newset + i;

    newsample->w = 0.0;
    newsample->err = old->err;
    newsample->pose = old->pose;
    memcpy(newsample->cells, old->cells, self->grid_size);
    memcpy(newsample->poses, old->poses, self->traj_size);
  }  

  gsl_ran_discrete_free(dist);
  delete [] sample_probs;

  self->sample_set = (self->sample_set + 1) % 2;
  
  return;
}



// Add a scan to the map
void pmap_add_scan(pmap_t *self, double *ranges)
{
  int i;
  pmap_scan_t *scan;
  pmap_sample_t *sample;

  assert(self->step_count < self->step_max_count);
      
  // Add to scan list
  scan = self->scans + self->step_count;
  memcpy(scan->ranges, ranges, self->num_ranges * sizeof(ranges[0]));

  // Add to samples
  for (i = 0; i < self->samples_len; i++)
  {
    sample = PMAP_GET_SAMPLE(self, i);

    // Add to trajectory
    sample->poses[self->step_count] = sample->pose;

    // Add to map
    pmap_add_scan_sample(self, i, ranges);
  }
  return;
}


// Add a scan to a particular sample
void pmap_add_scan_sample(pmap_t *self, int sample_index, double *ranges)
{
  int i;
  double r;
  vector2_t p;
  pmap_sample_t *sample;
  int nx, ny, nindex, occ;

  // Set some pointers
  sample = PMAP_GET_SAMPLE(self, sample_index);
      
  // Update the map
  for (i = 0; i < self->num_ranges; i++)
  {
    r = ranges[i];
    if (r > self->range_max)
      continue;

    p = sample->global_points[i];
      
    nx = PMAP_GRIDX(self, p.x);
    ny = PMAP_GRIDY(self, p.y);
              
    if (PMAP_GRID_VALID(self, nx, ny))
    {
      nindex = PMAP_GRID_INDEX(self, nx, ny);
      occ = (int) sample->cells[nindex] + 1;
      if (occ > 127)
        occ = 127;
      sample->cells[nindex] = occ;
    }
  }

  return;
}


// Draw the current range scan
void pmap_draw_scan(pmap_t *self, double *ranges)
{
#ifdef HAVE_LIBGLUT
  int i, best_i;
  double max_w;
  vector2_t p;
  pmap_sample_t *sample;

  // Find best sample
  best_i = -1;
  max_w = DBL_MAX;
  for (i = 0; i < self->samples_len; i++)
  {
    sample = PMAP_GET_SAMPLE(self, i);  
    if (sample->err < max_w)
    {
      max_w = sample->err;
      best_i = i;
    }
  }
  assert(best_i >= 0);

  sample = PMAP_GET_SAMPLE(self, best_i);  

  glPushMatrix();
  glTranslatef(sample->pose.pos.x, sample->pose.pos.y, 0.0);
  glRotatef(sample->pose.rot * 180 / M_PI, 0, 0, 1);

  glColor3f(0, 0, 0.5);

  glutSolidSphere(0.20, 10, 16);
  
  glBegin(GL_LINE_LOOP);
  for (i = 0; i < self->num_ranges; i++)
  {
    p.x = ranges[i] * cos(i * self->range_step + self->range_start);
    p.y = ranges[i] * sin(i * self->range_step + self->range_start);
    glVertex3f(p.x, p.y, 0.0);
  }
  glEnd();

  // Draw the max range ellipse
  glColor3f(0.5, 0.5, 1.0);
  glBegin(GL_LINE_LOOP);
  for (i = 0; i < self->num_ranges; i++)
  {
    p.x = self->range_max * cos(i * self->range_step + self->range_start);
    p.y = self->range_max * sin(i * self->range_step + self->range_start);
    glVertex3f(p.x, p.y, 0.0);
  }
  glEnd();

  glPopMatrix();
  
  return;
#endif  
}


// Draw all samples
void pmap_draw_samples(pmap_t *self)
{
#ifdef HAVE_LIBGLUT
  int i;
  pmap_sample_t *sample;

  // Draw all samples 
  for (i = 0; i < self->samples_len; i++)
  {
    sample = PMAP_GET_SAMPLE(self, i);  
    glColor3f(0, 0, 0.5);
    pmap_draw_sample(self, i);
  }

  // Re-draw the best sample
  glColor3f(1, 0, 0);
  pmap_draw_sample(self, self->best_sample);
  
  return;
#endif
}


// Draw a particular sample
void pmap_draw_sample(pmap_t *self, int sample_index)
{
#ifdef HAVE_LIBGLUT
  int i;
  pmap_sample_t *sample;
  pose2_t pose;

  sample = PMAP_GET_SAMPLE(self, sample_index);
    
  glBegin(GL_LINE_STRIP);

  for (i = 0; i < self->step_count; i++)
  {
    pose = sample->poses[i];
    glVertex3f(pose.pos.x, pose.pos.y, 0);
  }

  glEnd();
  
  return;
#endif
}


// Draw a candidate map
void pmap_draw_map(pmap_t *self, double scale)
{
#ifdef HAVE_LIBGLUT
  pmap_draw_sample_map(self, scale, self->best_sample);  
  return;
#endif
}


// Draw a particular sample map
void pmap_draw_sample_map(pmap_t *self, double scale, int sample_index)
{
#ifdef HAVE_LIBGLUT
  int i, j;
  pmap_sample_t *sample;

  sample = PMAP_GET_SAMPLE(self, sample_index);

  glPixelZoom(self->grid_res / scale / 2, self->grid_res / scale / 2);

  // Invert colors
  glPixelTransferf(GL_RED_SCALE, -1);
  glPixelTransferf(GL_GREEN_SCALE, -1);
  glPixelTransferf(GL_BLUE_SCALE, -1);
  glPixelTransferf(GL_RED_BIAS, 0.5);
  glPixelTransferf(GL_GREEN_BIAS, 0.5);
  glPixelTransferf(GL_BLUE_BIAS, 0.5);

  // Draw the image in tiles to prevent the whole thing from being
  // clipped
  glPixelStorei(GL_UNPACK_ROW_LENGTH, self->grid_sx);
  for (j = 0; j < self->grid_sy / 16; j++)
  {
    for (i = 0; i < self->grid_sx / 16; i++)
    {
      glRasterPos2f(-self->grid_sx / 2 * self->grid_res + i * 16 * self->grid_res,
                    -self->grid_sy / 2 * self->grid_res + j * 16 * self->grid_res);
      glDrawPixels(16, 16,
                   GL_LUMINANCE, GL_BYTE,
                   sample->cells + i * 16 + j * 16 * self->grid_sx);
    }
  }

  return;
#endif
}
