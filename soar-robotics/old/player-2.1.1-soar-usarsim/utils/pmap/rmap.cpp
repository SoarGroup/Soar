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
  Desc: Relaxation engine
  Author: Andrew Howard
  Date: 19 Nov 2004
  CVS: $Id: rmap.cpp 4062 2007-05-05 11:53:08Z thjc $
 */

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gsl/gsl_multimin.h>

#include <config.h>

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif

#include "rmap.h"


/// @brief Grid access macros
#define RMAP_GRIDX(self, x) ((int) floor((x) / self->grid_res) + self->grid_sx / 2)
#define RMAP_GRIDY(self, y) ((int) floor((y) / self->grid_res) + self->grid_sy / 2)
#define RMAP_GRID_VALID(self, x, y) ((x) >= 0 && (x) < self->grid_sx && \
                                     (y) >= 0 && (y) < self->grid_sy)
#define RMAP_GRID_INDEX(self, x, y) ((x) + (y) * self->grid_sx)

// Error function for fitting
void rmap_fit_fdf(const gsl_vector *x, rmap_t *self, double *f, gsl_vector *g);
void rmap_fit_df(const gsl_vector *x, rmap_t *self, gsl_vector *g);
double rmap_fit_f(const gsl_vector *x, rmap_t *self);



// Allocate object  
rmap_t *rmap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step,
                   double grid_width, double grid_height)
{
  rmap_t *self;

  self = new rmap_t;

  // Settings
  self->max_dist = 0.20;
  self->range_interval = 10;
  self->key_interval = 20;

  // Record range settings
  self->num_ranges = num_ranges;
  self->range_max = range_max;
  self->range_start = range_start;
  self->range_step = range_step;
  
  // Allocate space for scans
  self->num_scans = 0;
  self->num_key_scans = 0;
  self->max_scans = 2000;
  self->scans = new rmap_scan_t[self->max_scans];
  
  // Allocate space for grid
  self->grid_res = self->max_dist;
  self->grid_sx = (int) ceil(grid_width / self->grid_res);
  self->grid_sy = (int) ceil(grid_height / self->grid_res);
  self->grid_size = self->grid_sx * self->grid_sy * sizeof(self->grid[0]);
  self->grid = new rmap_cell_t[self->grid_sx * self->grid_sy];
  assert(self->grid);

  // Allocate space for grid items
  self->num_items = 0;
  self->max_items = self->max_scans * RMAP_MAX_RANGES;
  self->items_size = self->max_items * sizeof(self->items[0]);
  self->items = new rmap_item_t[self->max_items];

  // Allocate space for constraints
  self->num_cons = 0;
  self->max_cons = self->max_scans * self->num_ranges;
  self->cons = new rmap_constraint_t[self->max_cons];

  self->match_count = 0;
  
  return self;
}


// Free object
void rmap_free(rmap_t *self)
{
  delete [] self->cons;
  delete [] self->items;
  delete [] self->grid;
  delete [] self->scans;
  delete self;
  return;
}


// Add a scan to the map
void rmap_add(rmap_t *self, pose2_t pose, int num_ranges, double *ranges)
{
  int i;
  double r;
  rmap_scan_t *scan;
  vector2_t *p;
  
  assert(num_ranges == self->num_ranges);
  
  assert(self->num_scans < self->max_scans);
  scan = self->scans + self->num_scans++;
  scan->init = pose;
  scan->pose = pose;
  scan->num_hits = 0;

  // Add key-scans at periodic intervals; the first scan must be a
  // key-scan.
  if ((self->num_scans - 1) % self->key_interval == 0)
    scan->index = self->num_key_scans++;
  else
    scan->index = -1;

  // Add hit points
  for (i = 0; i < self->num_ranges; i++)
  {
    scan->ranges[i] = ranges[i];
    
    r = ranges[i];
    if (r > self->range_max)
      continue;

    assert(scan->num_hits < RMAP_MAX_RANGES);
    p = scan->hits + scan->num_hits++;
    p->x = r * cos(self->range_start + i * self->range_step);
    p->y = r * sin(self->range_start + i * self->range_step);
  }
  
  return;
}


// Match points across scans
void rmap_match(rmap_t *self)
{
  int i;
  rmap_scan_t *scan;

  // Reset constraint list
  self->num_cons = 0;

  // Reset the grid
  self->num_items = 0;
  memset(self->grid, 0, self->grid_size);

  // Project scans into grid for fast indexing
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    if (scan->index >= 0)
      rmap_match_prepare(self, scan);
  }
  
  // Match each key scan
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    if (scan->index >= 0)
      rmap_match_scan(self, scan);
  }

  self->match_count++;
      
  return;
}


// Project bounds into the grid
void rmap_match_prepare(rmap_t *self, rmap_scan_t *scan)
{
  int i;
  int ni, nj;
  vector2_t p, q;
  rmap_cell_t *cell;
  rmap_item_t *item;
  
  // Update the global pose of each boundary point and project into
  // grid
  for (i = 0; i < scan->num_hits; i++)
  {
    p = scan->hits[i];
    q = pose2_add_pos(p, scan->pose);      

    ni = RMAP_GRIDX(self, q.x);
    nj = RMAP_GRIDY(self, q.y);

    if (RMAP_GRID_VALID(self, ni, nj))
    {
      cell = self->grid + RMAP_GRID_INDEX(self, ni, nj);
      assert(self->num_items < self->max_items);
      item = self->items + self->num_items++;

      item->scan = scan;
      item->local = p;
      item->global = q;
      item->next = NULL;

      if (cell->last != NULL)
        cell->last->next = item;
      if (cell->first == NULL)
        cell->first = item;
      cell->last = item;
    }
  }
  return;
}


// Match points for a single scan
void rmap_match_scan(rmap_t *self, rmap_scan_t *scan_a)
{
  int i;
  int ni, nj, di, dj, mi, mj;
  double d;
  rmap_constraint_t *con;
  rmap_constraint_t **cons;
  rmap_cell_t *cell;
  vector2_t pa, pb;
  vector2_t qa, qb;
  rmap_item_t *item;
  rmap_scan_t *scan_b;

  // Workspace for constraint map
  printf("num-scans = %d\n",self->num_scans);
  cons = new rmap_constraint_t *[self->num_scans];
  
  // Match hits to boundaries
  for (i = 0; i < scan_a->num_hits; i += self->range_interval)
  {
    pa = scan_a->hits[i];
    qa = pose2_add_pos(pa, scan_a->pose);      

    ni = RMAP_GRIDX(self, qa.x);
    nj = RMAP_GRIDY(self, qa.y);

    // Reset the map from scan index to constraint pointer
    memset(cons, 0, self->num_scans * sizeof(cons[0]));

    // Look in the grid for the nearest boundary point; we have to
    // check all the cells in the vicinity of the hit point.
    for (dj = -1; dj <= +1; dj++)
    {
      for (di = -1; di <= +1; di++)
      {
        mi = ni + di;
        mj = nj + dj;
        if (RMAP_GRID_VALID(self, mi, mj))
        {
          cell = self->grid + RMAP_GRID_INDEX(self, mi, mj);

          for (item = cell->first; item != NULL; item = item->next)
          {
            scan_b = item->scan;
            if (scan_b == scan_a)
              continue;

            pb = item->local;
            qb = item->global;
            
            d = vector2_mag(vector2_sub(qa, qb));

            con = cons[scan_b->index];
            if (con == NULL)
            {
              assert(self->num_cons < self->max_cons);
              con = self->cons + self->num_cons++;
              cons[scan_b->index] = con;              
              con->scan_a = scan_a;
              con->scan_b = scan_b;
              con->local_a = pa;
              con->dist = DBL_MAX;
            }
    
            if (d < con->dist)
            {
              assert(con->scan_b == scan_b);
              con->local_b = pb;
              con->dist = d;
            }
          }
        }
      }
    }
  }

  delete [] cons;
    
  return;
}


// Relax the map
void rmap_relax(rmap_t *self, int num_cycles)
{
  int i, n;
  gsl_vector *x, *y;
  gsl_multimin_function_fdf fdf;
  gsl_multimin_fdfminimizer *s;
  double step_size, tol;
  rmap_scan_t *scan;

  // HACK
  step_size = 0.01;
  tol = 0.001;
  
  // Compute number of free variables
  n = 3 * self->num_key_scans;

  // Set the initial vector
  x = gsl_vector_alloc(n);
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    if (scan->index >= 0)
    {
      gsl_vector_set(x, 3 * scan->index + 0, scan->pose.pos.x);
      gsl_vector_set(x, 3 * scan->index + 1, scan->pose.pos.y);
      gsl_vector_set(x, 3 * scan->index + 2, scan->pose.rot);
    }
  }
  
  // Allocate minimizer
  fdf.f = (double (*) (const gsl_vector*, void*)) rmap_fit_f;
  fdf.df = (void (*) (const gsl_vector*, void*, gsl_vector*)) rmap_fit_df;
  fdf.fdf = (void (*) (const gsl_vector*, void*, double*, gsl_vector*)) rmap_fit_fdf;
  fdf.n = n;
  fdf.params = self;  
  s = gsl_multimin_fdfminimizer_alloc(gsl_multimin_fdfminimizer_vector_bfgs, n);

  // Initialize minimizer
  gsl_multimin_fdfminimizer_set(s, &fdf, x, step_size, tol);

  // Optimize
  for (i = 0; i < num_cycles; i++)
  {
    if (gsl_multimin_fdfminimizer_iterate(s) != GSL_SUCCESS)
      break;
  }
  
  self->relax_err = gsl_multimin_fdfminimizer_minimum(s);
  
  // Copy corrections back to data structures
  y = gsl_multimin_fdfminimizer_x(s);
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    if (scan->index >= 0)
    {
      scan->delta.pos.x = gsl_vector_get(y, 3 * scan->index + 0) - scan->pose.pos.x;
      scan->delta.pos.y = gsl_vector_get(y, 3 * scan->index + 1) - scan->pose.pos.y;
      scan->delta.rot = gsl_vector_get(y, 3 * scan->index + 2) - scan->pose.rot;
    }
  }

  // Clean up
  gsl_multimin_fdfminimizer_free(s);
  gsl_vector_free(x);

  return;
}


// Error function for fitting
void rmap_fit_fdf(const gsl_vector *x, rmap_t *self, double *f, gsl_vector *g)
{
  int i, n;
  rmap_constraint_t *con;
  pose2_t sa, sb;
  vector2_t pa, pb, qa, qb;
  vector2_t diff;
  vector2_t qa_sax, qa_say, qa_sar;
  vector2_t qb_sbx, qb_sby, qb_sbr;
  double u;
  double u_sax, u_say, u_sar;
  double u_sbx, u_sby, u_sbr;

  if (f)
    *f = 0;
  if (g)
    gsl_vector_set_zero(g);
  
  for (i = 0; i < self->num_cons; i++)
  {
    con = self->cons + i;

    // Suck the scan pose from the current vector
    sa.pos.x = gsl_vector_get(x, con->scan_a->index * 3 + 0);
    sa.pos.y = gsl_vector_get(x, con->scan_a->index * 3 + 1);
    sa.rot = gsl_vector_get(x, con->scan_a->index * 3 + 2);

    sb.pos.x = gsl_vector_get(x, con->scan_b->index * 3 + 0);
    sb.pos.y = gsl_vector_get(x, con->scan_b->index * 3 + 1);
    sb.rot = gsl_vector_get(x, con->scan_b->index * 3 + 2);

    // Local points
    pa = con->local_a;
    pb = con->local_b;

    // Global points
    qa = pose2_add_pos(pa, sa);
    qb = pose2_add_pos(pb, sb);

    // Compute difference term
    diff = vector2_sub(qa, qb);

    // Compute error term
    u = 0.5 * (vector2_dot(diff, diff) - self->max_dist * self->max_dist);
    if (u > 0.0)
    {
      u = 0.0;
      diff.x = 0;
      diff.y = 0;
    }

    // Derivatives
    qa_sax = vector2_set(+1, 0);
    qa_say = vector2_set(0, +1);
    qa_sar = vector2_set(-pa.x * sin(sa.rot) - pa.y * cos(sa.rot),
                         +pa.x * cos(sa.rot) - pa.y * sin(sa.rot));

    qb_sbx = vector2_set(-1, 0);
    qb_sby = vector2_set(0, -1);
    qb_sbr = vector2_set(+pb.x * sin(sb.rot) + pb.y * cos(sb.rot),
                         -pb.x * cos(sb.rot) + pb.y * sin(sb.rot));

    u_sax = vector2_dot(diff, qa_sax);
    u_say = vector2_dot(diff, qa_say);
    u_sar = vector2_dot(diff, qa_sar);    

    u_sbx = vector2_dot(diff, qb_sbx);
    u_sby = vector2_dot(diff, qb_sby);
    u_sbr = vector2_dot(diff, qb_sbr);    

    if (f)
    {
      *f += u;
    }

    if (g)
    {
      n = con->scan_a->index * 3 + 0;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_sax);
      n = con->scan_a->index * 3 + 1;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_say);
      n = con->scan_a->index * 3 + 2;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_sar);

      n = con->scan_b->index * 3 + 0;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_sbx);
      n = con->scan_b->index * 3 + 1;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_sby);
      n = con->scan_b->index * 3 + 2;
      gsl_vector_set(g, n, gsl_vector_get(g, n) + u_sbr);
    }
  }
  
  return;
}


// Error function for fitting
void rmap_fit_df(const gsl_vector *x, rmap_t *self, gsl_vector *g)
{
  rmap_fit_fdf(x, self, NULL, g);
  return;
}


// Error function for fitting
double rmap_fit_f(const gsl_vector *x, rmap_t *self)
{
  double f;
  rmap_fit_fdf(x, self, &f, NULL);
  return f;
}


// Interpolate between key-scans 
void rmap_interpolate(rmap_t *self)
{
  int i, a, b;
  rmap_scan_t *scan, *scan_a, *scan_b;
  double t;

  a = 0;
  b = 0;
  
  for (a = b; b < self->num_scans; a = b)
  {
    // Find the bracketing key scans
    scan_a = self->scans + a;        
    for (b = a + 1; b < self->num_scans; b++)
    {
      scan_b = self->scans + b;
      if (scan_b->index >= 0)
        break;
    }

    if (b < self->num_scans)
    {
      assert(scan_a->index >= 0);
      assert(scan_b->index >= 0);

      // Interpolate correction between key scans
      for (i = a + 1; i < b; i++)
      {
        scan = self->scans + i;
        t = (double) (i - a) / (b - a);
        scan->delta.pos.x = (1 - t) * scan_a->delta.pos.x + t * scan_b->delta.pos.x;
        scan->delta.pos.y = (1 - t) * scan_a->delta.pos.y + t * scan_b->delta.pos.y;
        scan->delta.rot = (1 - t) * scan_a->delta.rot + t * scan_b->delta.rot;
      }
    }
    else
    {
      assert(scan_a->index >= 0);

      // Interpolate correction using last known key-scan
      for (i = a + 1; i < b; i++)
      {
        scan = self->scans + i;
        scan->delta.pos.x = scan_a->delta.pos.x;
        scan->delta.pos.y = scan_a->delta.pos.y;
        scan->delta.rot = scan_a->delta.rot;
      }
    }
  }

  // Second pass to correct the scans
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    scan->pose.pos.x += scan->delta.pos.x;
    scan->pose.pos.y += scan->delta.pos.y;
    scan->pose.rot += scan->delta.rot;
  }
  
  return;
}



// Draw the current map
void rmap_draw_map(rmap_t *self)
{
#ifdef HAVE_LIBGLUT
  int i, j;
  rmap_scan_t *scan;
  vector2_t p;
  
  for (i = 0; i < self->num_scans; i++)
  {
    scan = self->scans + i;
    if (scan->index < 0)
      continue;

    glPushMatrix();
    glTranslatef(scan->pose.pos.x, scan->pose.pos.y, 0.0);
    glRotatef(scan->pose.rot * 180 / M_PI, 0, 0, 1);

    glColor3f(0, 0, 0.9);
    glutSolidSphere(0.20, 16, 3);

    glColor3f(0, 0, 0);
    glBegin(GL_POINTS);
    for (j = 0; j < scan->num_hits; j++)
    {
      p = scan->hits[j];     
      glVertex2f(p.x, p.y);
    }
    glEnd();

    glPopMatrix();
  }
#endif
  return;
}


// Draw constraints
void rmap_draw_cons(rmap_t *self)
{
#ifdef HAVE_LIBGLUT
  int i;
  rmap_constraint_t *con;
  vector2_t pa, pb;

  // Draw inter-scan constraints
  glColor3f(0.7, 0, 0.7);
  glBegin(GL_LINES);
  for (i = 0; i < self->num_cons; i++)
  {
    con = self->cons + i;
    pa = con->scan_a->pose.pos;
    pb = con->scan_b->pose.pos;
    glVertex2f(pa.x, pa.y);
    glVertex2f(pb.x, pb.y);
  }
  glEnd();

  /*
  /// Draw individual point matches
  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  for (i = 0; i < self->num_cons; i++)
  {
    con = self->cons + i;
    pa = pose2_add_pos(con->local_a, con->scan_a->pose);
    pb = pose2_add_pos(con->local_b, con->scan_b->pose);
    glVertex2f(pa.x, pa.y);
    glVertex2f(pb.x, pb.y);
  }
  glEnd();
  */
#endif
  return;
}


