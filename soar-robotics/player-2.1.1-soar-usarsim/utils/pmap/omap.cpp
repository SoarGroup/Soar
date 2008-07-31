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
  Desc: Occupancy grid map
  Author: Andrew Howard
  Date: 19 Nov 2004
  CVS: $Id: omap.cpp 4064 2007-05-16 21:43:00Z thjc $
 */


#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif

#include "omap.h"


// Allocate object  
omap_t *omap_alloc(int num_ranges, double range_max,
                   double range_start, double range_step,
                   double grid_width, double grid_height, double grid_res)
{
  omap_t *self;

  self = new omap_t;

  // Record range settings
  self->num_ranges = num_ranges;
  self->range_max = range_max;
  self->range_start = range_start;
  self->range_step = range_step;
    
  // Allocate space for grid
  self->grid_res = grid_res;
  self->grid_sx = static_cast<int> (ceil(grid_width / grid_res));
  self->grid_sy = static_cast<int> (ceil(grid_height / self->grid_res));
  self->grid_size = self->grid_sx * self->grid_sy * sizeof(self->grid[0]);
  self->grid = new signed char[self->grid_size];
  assert(self->grid);
  
  return self;
}


// Free object
void omap_free(omap_t *self)
{
  delete [] self->grid;
  delete self;
  
  return;
}


// Clear the map.
void omap_clear(omap_t *self)
{
  memset(self->grid, 0, self->grid_size);
  return;
}


// Add a scan to the map
void omap_add(omap_t *self, pose2_t pose, int num_ranges, double *ranges)
{
  int i, j;
  double r, dx, dy, dr;
  vector2_t a, b, p;
  int step_count;
  double step_size;
  int nx, ny, nindex, occ;
  int maxed;

  // Make sure we hit every grid cell
  step_size = self->grid_res;

  a = pose.pos;
      
  // Ray-trace the grid
  for (i = 0; i < self->num_ranges; i++)
  {
    r = ranges[i];
    if (r > self->range_max)
    {
      r = self->range_max;
      maxed = 1;
    }
    else
      maxed = 0;

    // Compute rate end-point
    b.x = r * cos(self->range_start + i * self->range_step);
    b.y = r * sin(self->range_start + i * self->range_step);
    b = pose2_add_pos(b, pose);

    // Compute line parameters
    dx = b.x - a.x;
    dy = b.y - a.y;
    dr = sqrt(dx * dx + dy * dy);

    step_count = (int) floor(dr / step_size) + 2;
    dx /= (step_count - 1);
    dy /= (step_count - 1);

    // Walk the line and update the grid
    for (j = 0; j < step_count; j++)
    {      
      p.x = a.x + dx * j;
      p.y = a.y + dy * j;
      
      nx = OMAP_GRIDX(self, p.x);
      ny = OMAP_GRIDY(self, p.y);
              
      if (OMAP_GRID_VALID(self, nx, ny))
      {
        nindex = OMAP_GRID_INDEX(self, nx, ny);
        occ = (int) self->grid[nindex] - 1;
        if (occ < -127)
          occ = -127;
        self->grid[nindex] = occ;
      }
    }
        
    if (!maxed)
    {
      p.x = a.x + dx * step_count;
      p.y = a.y + dy * step_count;
      
      nx = OMAP_GRIDX(self, p.x);
      ny = OMAP_GRIDY(self, p.y);
              
      if (OMAP_GRID_VALID(self, nx, ny))
      {
        nindex = OMAP_GRID_INDEX(self, nx, ny);
        occ = (int) self->grid[nindex] + 1;
        if (occ > 127)
          occ = 127;
        self->grid[nindex] = occ;
      }
    }
  }

  return;
}


// Save as PGM format
int omap_save_pgm(omap_t *self, const char *filename)
{
  int i, j;
  signed char c;
  unsigned char d;
  FILE *file;

  file = fopen(filename, "w+");
  if (file == NULL)
  {
    fprintf(stderr, "error writing %s : %s", filename, strerror(errno));
    return -1;
  }

  fprintf(file, "P5 %d %d 255\n", self->grid_sx, self->grid_sy);

  for (j = 0; j < self->grid_sy; j++)
  {
    for (i = 0; i < self->grid_sx; i++)
    {
      c = self->grid[OMAP_GRID_INDEX(self, i, self->grid_sy - 1 - j)];
      d = (unsigned char) (127 - c);
      fwrite(&d, 1, 1,  file);
    }
  }

  fclose(file);

  return 0;
}


// Draw the current map
void omap_draw_map(omap_t *self, double scale)
{
#ifdef HAVE_LIBGLUT
  int i, j;

  // Set pixel zoom factor so DrawPixel ops align
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
                   self->grid + i * 16 + j * 16 * self->grid_sx);
    }
  }

  return;
#endif
}


