
/**************************************************************************
 * Desc: Path planner: plan generation
 * Author: Andrew Howard
 * Date: 10 Oct 2002
 * CVS: $Id: plan_plan.c 6566 2008-06-14 01:00:19Z thjc $
**************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

#include <sys/time.h>
static double get_time(void);

#include "plan.h"

// Plan queue stuff
void plan_push(plan_t *plan, plan_cell_t *cell);
plan_cell_t *plan_pop(plan_t *plan);
int _plan_update_plan(plan_t *plan, double lx, double ly, double gx, double gy);
int _plan_find_local_goal(plan_t *plan, double* gx, double* gy, double lx, double ly);


int
plan_do_global(plan_t *plan, double lx, double ly, double gx, double gy)
{
  plan_cell_t* cell;
  int li, lj;
  double t0,t1;

  t0 = get_time();

  // Set bounds to look over the entire grid
  plan_set_bounds(plan, 0, 0, plan->size_x - 1, plan->size_y - 1);

  // Reset plan costs
  plan_reset(plan);

  plan->path_count = 0;
  if(_plan_update_plan(plan, lx, ly, gx, gy) < 0)
  {
    // no path
    return(-1);
  }

  li = PLAN_GXWX(plan, lx);
  lj = PLAN_GYWY(plan, ly);

  // Cache the path
  for(cell = plan->cells + PLAN_INDEX(plan,li,lj);
      cell;
      cell = cell->plan_next)
  {
    if(plan->path_count >= plan->path_size)
    {
      plan->path_size *= 2;
      plan->path = (plan_cell_t**)realloc(plan->path,
                                          plan->path_size * sizeof(plan_cell_t*));
      assert(plan->path);
    }
    plan->path[plan->path_count++] = cell;
  }

  t1 = get_time();

  printf("computed global path: %.6lf\n", t1-t0);

  return(0);
}

int
plan_do_local(plan_t *plan, double lx, double ly, double plan_halfwidth)
{
  double gx, gy;
  int li, lj;
  int xmin,ymin,xmax,ymax;
  plan_cell_t* cell;
  double t0,t1;

  t0 = get_time();

  // Set bounds as directed
  xmin = PLAN_GXWX(plan, lx - plan_halfwidth);
  ymin = PLAN_GXWX(plan, ly - plan_halfwidth);
  xmax = PLAN_GXWX(plan, lx + plan_halfwidth);
  ymax = PLAN_GXWX(plan, ly + plan_halfwidth);
  plan_set_bounds(plan, xmin, ymin, xmax, ymax);

  // Reset plan costs (within the local patch)
  plan_reset(plan);

  // Find a local goal to pursue
  if(_plan_find_local_goal(plan, &gx, &gy, lx, ly) != 0)
  {
    puts("no local goal");
    return(-1);
  }

  //printf("local goal: %.3lf, %.3lf\n", gx, gy);

  plan->lpath_count = 0;
  if(_plan_update_plan(plan, lx, ly, gx, gy) != 0)
  {
    puts("local plan update failed");
    return(-1);
  }

  li = PLAN_GXWX(plan, lx);
  lj = PLAN_GYWY(plan, ly);

  // Cache the path
  for(cell = plan->cells + PLAN_INDEX(plan,li,lj);
      cell;
      cell = cell->plan_next)
  {
    if(plan->lpath_count >= plan->lpath_size)
    {
      plan->lpath_size *= 2;
      plan->lpath = (plan_cell_t**)realloc(plan->lpath,
                                           plan->lpath_size * sizeof(plan_cell_t*));
      assert(plan->lpath);
    }
    plan->lpath[plan->lpath_count++] = cell;
  }

  t1 = get_time();

  printf("computed local path: %.6lf\n", t1-t0);
  return(0);
}


// Generate the plan
int 
_plan_update_plan(plan_t *plan, double lx, double ly, double gx, double gy)
{
  int oi, oj, di, dj, ni, nj;
  int gi, gj, li,lj;
  float cost;
  plan_cell_t *cell, *ncell;

  // Reset the queue
  heap_reset(plan->heap);

  // Initialize the goal cell
  gi = PLAN_GXWX(plan, gx);
  gj = PLAN_GYWY(plan, gy);

  // Initialize the start cell
  li = PLAN_GXWX(plan, lx);
  lj = PLAN_GYWY(plan, ly);

  //printf("planning from %d,%d to %d,%d\n", li,lj,gi,gj);

  if(!PLAN_VALID_BOUNDS(plan, gi, gj))
  {
    puts("goal out of bounds");
    return(-1);
  }
  
  if(!PLAN_VALID_BOUNDS(plan, li, lj))
  {
    puts("start out of bounds");
    return(-1);
  }

  cell = plan->cells + PLAN_INDEX(plan, gi, gj);
  cell->plan_cost = 0;
  plan_push(plan, cell);

  while (1)
  {
    float * p;
    cell = plan_pop(plan);
    if (cell == NULL)
      break;

    oi = cell->ci;
    oj = cell->cj;

    //printf("pop %d %d %f\n", cell->ci, cell->cj, cell->plan_cost);

    p = plan->dist_kernel_3x3;
    for (dj = -1; dj <= +1; dj++)
    {
      ncell = plan->cells + PLAN_INDEX(plan,oi-1,oj+dj);
      for (di = -1; di <= +1; di++, p++, ncell++)
      {
        if (!di && !dj)
          continue;
        //if (di && dj)
          //continue;
        
        ni = oi + di;
        nj = oj + dj;

        if (!PLAN_VALID_BOUNDS(plan, ni, nj))
          continue;

        if(ncell->mark)
          continue;

        if (ncell->occ_dist_dyn < plan->abs_min_radius)
          continue;

        cost = cell->plan_cost + *p;

        if(ncell->occ_dist_dyn < plan->max_radius)
          cost += plan->dist_penalty * (plan->max_radius - ncell->occ_dist_dyn);

        if(cost < ncell->plan_cost)
        {
          ncell->plan_cost = cost;
          ncell->plan_next = cell;

          plan_push(plan, ncell);
        }
      }
    }
  }

  cell = plan->cells + PLAN_INDEX(plan, li, lj);
  if(!cell->plan_next)
  {
    puts("never found start");
    return(-1);
  }
  else
    return(0);
}

int 
_plan_find_local_goal(plan_t *plan, double* gx, double* gy, 
                      double lx, double ly)
{
  int c;
  int c_min;
  double squared_d;
  double squared_d_min;
  int li,lj;
  plan_cell_t* cell;

  // Must already have computed a global goal
  if(plan->path_count == 0)
  {
    puts("no global path");
    return(-1);
  }

  li = PLAN_GXWX(plan, lx);
  lj = PLAN_GYWY(plan, ly);

  // Find the closest place to jump on the global path
  squared_d_min = DBL_MAX;
  c_min = -1;
  for(c=0;c<plan->path_count;c++)
  {
    cell = plan->path[c];
    squared_d = ((cell->ci - li) * (cell->ci - li) + 
                 (cell->cj - lj) * (cell->cj - lj));
    if(squared_d < squared_d_min)
    {
      squared_d_min = squared_d;
      c_min = c;
    }
  }
  assert(c_min > -1);

  // Follow the path to find the last cell that's inside the local planning
  // area
  for(c=c_min; c<plan->path_count; c++)
  {
    cell = plan->path[c];
    
    //printf("step %d: (%d,%d)\n", c, cell->ci, cell->cj);

    if((cell->ci < plan->min_x) || (cell->ci > plan->max_x) ||
       (cell->cj < plan->min_y) || (cell->cj > plan->max_y))
    {
      // Did we move at least one cell along the path?
      if(c == c_min)
      {
        // nope; the entire global path is outside the local region; can't
        // fix that here
        puts("global path not in local region");
        return(-1);
      }
      else
        break;
    }
  }

  assert(c > c_min);

  cell = plan->path[c-1];

  //printf("ci: %d cj: %d\n", cell->ci, cell->cj);
  *gx = PLAN_WXGX(plan, cell->ci);
  *gy = PLAN_WYGY(plan, cell->cj);
  
  return(0);
}

// Push a plan location onto the queue
void plan_push(plan_t *plan, plan_cell_t *cell)
{
  // Substract from max cost because the heap is set up to return the max
  // element.  This could of course be changed.
  assert(PLAN_MAX_COST-cell->plan_cost > 0);
  cell->mark = 1;
  heap_insert(plan->heap, PLAN_MAX_COST - cell->plan_cost, cell);

  return;
}


// Pop a plan location from the queue
plan_cell_t *plan_pop(plan_t *plan)
{

  if(heap_empty(plan->heap))
    return(NULL);
  else
    return(heap_extract_max(plan->heap));
}

double 
static get_time(void)
{
  struct timeval curr;
  gettimeofday(&curr,NULL);
  return(curr.tv_sec + curr.tv_usec / 1e6);
}
