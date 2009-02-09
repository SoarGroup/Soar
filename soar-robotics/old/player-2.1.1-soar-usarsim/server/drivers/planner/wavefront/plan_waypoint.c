
/**************************************************************************
 * Desc: Path planner: waypoint generation
 * Author: Andrew Howard
 * Date: 10 Oct 2002
 * CVS: $Id: plan_waypoint.c 3333 2006-01-20 21:26:31Z gerkey $
**************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <libplayercore/error.h>

#include "plan.h"

// Test to see if once cell is reachable from another
int plan_test_reachable(plan_t *plan, plan_cell_t *cell_a, plan_cell_t *cell_b);


// Generate a path to the goal
void plan_update_waypoints(plan_t *plan, double px, double py)
{
  double dist;
  int ni, nj;
  plan_cell_t *cell, *ncell;

  plan->waypoint_count = 0;

  ni = PLAN_GXWX(plan, px);
  nj = PLAN_GYWY(plan, py);

  // Can't plan a path if we're off the map
  if(!PLAN_VALID(plan,ni,nj))
    return;

  cell = plan->cells + PLAN_INDEX(plan, ni, nj);

  while (cell != NULL)
  {
    if (plan->waypoint_count >= plan->waypoint_size)
    {
      plan->waypoint_size *= 2;
      plan->waypoints = realloc(plan->waypoints,
                                plan->waypoint_size * sizeof(plan->waypoints[0]));
    }
    
    plan->waypoints[plan->waypoint_count++] = cell;

    if (cell->plan_next == NULL)
    {
      // done
      break;
    }

    // Find the farthest cell in the path that is reachable from the
    // currrent cell.
    dist = 0;
    for(ncell = cell; ncell->plan_next != NULL; ncell = ncell->plan_next)
    {
      if(dist > 0.50)
      {
        if(!plan_test_reachable(plan, cell, ncell->plan_next))
          break;
      }
      dist += plan->scale;
    }
    if(ncell == cell)
    {
      break;
    }
    
    cell = ncell;
  }

  if(cell && (cell->plan_cost > 0))
  {
    // no path
    plan->waypoint_count = 0;
  }
  
  return;
}


// Get the ith waypoint; returns non-zero of there are no more waypoints
int plan_get_waypoint(plan_t *plan, int i, double *px, double *py)
{
  if (i < 0 || i >= plan->waypoint_count)
    return 0;

  *px = PLAN_WXGX(plan, plan->waypoints[i]->ci);
  *py = PLAN_WYGY(plan, plan->waypoints[i]->cj);

  return 1;
}

// Convert given waypoint cell to global x,y
void plan_convert_waypoint(plan_t* plan, 
                           plan_cell_t *waypoint, double *px, double *py)
{
  *px = PLAN_WXGX(plan, waypoint->ci);
  *py = PLAN_WYGY(plan, waypoint->cj);
}

// Test to see if once cell is reachable from another.
int plan_test_reachable(plan_t *plan, plan_cell_t *cell_a, plan_cell_t *cell_b)
{
  double theta;
  double sinth, costh;
  double i,j;
  int lasti, lastj;

  theta = atan2((double)(cell_b->cj - cell_a->cj), 
                (double)(cell_b->ci - cell_a->ci));
  sinth = sin(theta);
  costh = cos(theta);

  lasti = lastj = -1;
  i = (double)cell_a->ci;
  j = (double)cell_a->cj;

  while((lasti != cell_b->ci) || (lastj != cell_b->cj))
  {
    if((lasti != (int)floor(i)) || (lastj != (int)floor(j)))
    {
      lasti = (int)floor(i);
      lastj = (int)floor(j);
      if(!PLAN_VALID(plan,lasti,lastj))
      {
        PLAYER_WARN("stepped off the map!");
        return(0);
      }
      if(plan->cells[PLAN_INDEX(plan,lasti,lastj)].occ_dist <
         plan->abs_min_radius)
        return(0);
    }
    
    if(lasti != cell_b->ci)
      i += costh;
    if(lastj != cell_b->cj)
      j += sinth;
  }
  return(1);
}

#if 0
// Test to see if once cell is reachable from another.
// This could be improved.
int plan_test_reachable(plan_t *plan, plan_cell_t *cell_a, plan_cell_t *cell_b)
{
  int i, j;
  int ai, aj, bi, bj;
  double ox, oy, oa;
  double dx, dy;
  plan_cell_t *cell;

  ai = cell_a->ci;
  aj = cell_a->cj;
  bi = cell_b->ci;
  bj = cell_b->cj;

  ox = PLAN_WXGX(plan, ai);
  oy = PLAN_WYGY(plan, aj);
  oa = atan2(bj - aj, bi - ai);
  
  if (fabs(cos(oa)) > fabs(sin(oa)))
  {
    dy = tan(oa) * plan->scale;

    if (ai < bi)
    {
      for (i = ai; i < bi; i++)
      {
        j = PLAN_GYWY(plan, oy + (i - ai) * dy);
        if (PLAN_VALID(plan, i, j))
        {
          cell = plan->cells + PLAN_INDEX(plan, i, j);
          if (cell->occ_dist < plan->abs_min_radius)
            return 0;
        }
      }
    }
    else
    {
      for (i = ai; i > bi; i--)
      {
        j = PLAN_GYWY(plan, oy + (i - ai) * dy);
        if (PLAN_VALID(plan, i, j))
        {
          cell = plan->cells + PLAN_INDEX(plan, i, j);
          if (cell->occ_dist < plan->abs_min_radius)
            return 0;
        }
      }
    }
  }
  else
  {
    dx = tan(M_PI/2 - oa) * plan->scale;

    if (aj < bj)
    {
      for (j = aj; j < bj; j++)
      {
        i = PLAN_GXWX(plan, ox + (j - aj) * dx);
        if (PLAN_VALID(plan, i, j))
        {
          cell = plan->cells + PLAN_INDEX(plan, i, j);
          if (cell->occ_dist < plan->abs_min_radius)
            return 0;
        }
      }
    }
    else
    {
      for (j = aj; j > bj; j--)
      {
        i = PLAN_GXWX(plan, ox + (j - aj) * dx);
        if (PLAN_VALID(plan, i, j))
        {
          cell = plan->cells + PLAN_INDEX(plan, i, j);
          if (cell->occ_dist < plan->abs_min_radius)
            return 0;
        }
      }
    }
  }
  return 1;
}
#endif

