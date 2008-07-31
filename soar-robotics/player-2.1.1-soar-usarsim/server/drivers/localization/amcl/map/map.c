
/**************************************************************************
 * Desc: Global map (grid-based)
 * Author: Andrew Howard
 * Date: 6 Feb 2003
 * CVS: $Id: map.c 1713 2003-08-23 04:03:43Z inspectorg $
**************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"


// Create a new map
map_t *map_alloc(void)
{
  map_t *map;

  map = (map_t*) malloc(sizeof(map_t));

  // Assume we start at (0, 0)
  map->origin_x = 0;
  map->origin_y = 0;
  
  // Make the size odd
  map->size_x = 0;
  map->size_y = 0;
  map->scale = 0;
  map->max_occ_dist = 0;
  
  // Allocate storage for main map
  map->cells = (map_cell_t*) NULL;
  
  return map;
}


// Destroy a map
void map_free(map_t *map)
{
  free(map->cells);
  free(map);
  return;
}


// Get the cell at the given point
map_cell_t *map_get_cell(map_t *map, double ox, double oy, double oa)
{
  int i, j;
  map_cell_t *cell;

  i = MAP_GXWX(map, ox);
  j = MAP_GYWY(map, oy);
  
  if (!MAP_VALID(map, i, j))
    return NULL;

  cell = map->cells + MAP_INDEX(map, i, j);
  return cell;
}


// Update the cspace distance values
void map_update_cspace(map_t *map, double max_occ_dist)
{
  int i, j;
  int ni, nj;
  int s;
  double d;
  map_cell_t *cell, *ncell;

  map->max_occ_dist = max_occ_dist;
  s = (int) ceil(map->max_occ_dist / map->scale);

  // Reset the distance values
  for (j = 0; j < map->size_y; j++)
  {
    for (i = 0; i < map->size_x; i++)
    {
      cell = map->cells + MAP_INDEX(map, i, j);
      cell->occ_dist = map->max_occ_dist;
    }
  }

  // Find all the occupied cells and update their neighbours
  for (j = 0; j < map->size_y; j++)
  {
    for (i = 0; i < map->size_x; i++)
    {
      cell = map->cells + MAP_INDEX(map, i, j);
      if (cell->occ_state != +1)
        continue;
          
      cell->occ_dist = 0;

      // Update adjacent cells
      for (nj = -s; nj <= +s; nj++)
      {
        for (ni = -s; ni <= +s; ni++)
        {
          if (!MAP_VALID(map, i + ni, j + nj))
            continue;

          ncell = map->cells + MAP_INDEX(map, i + ni, j + nj);
          d = map->scale * sqrt(ni * ni + nj * nj);

          if (d < ncell->occ_dist)
            ncell->occ_dist = d;
        }
      }
    }
  }
  
  return;
}
