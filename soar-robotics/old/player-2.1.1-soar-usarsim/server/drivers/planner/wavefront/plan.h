
/**************************************************************************
 * Desc: Path planning
 * Author: Andrew Howard
 * Date: 10 Oct 2002
 * CVS: $Id: plan.h 6290 2008-04-09 01:37:38Z gerkey $
**************************************************************************/

#ifndef PLAN_H
#define PLAN_H

#include "heap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLAN_DEFAULT_HEAP_SIZE 1000
#define PLAN_MAX_COST 1e9

// Description for a grid single cell
typedef struct _plan_cell_t
{
  // Cell index in grid map
  unsigned short ci, cj;
  
  // Occupancy state (-1 = free, 0 = unknown, +1 = occ)
  char occ_state;
  char occ_state_dyn;

  // Distance to the nearest occupied cell
  float occ_dist;
  float occ_dist_dyn;

  // Distance (cost) to the goal
  float plan_cost;

  // Mark used in dynamic programming
  char mark;

  // The next cell in the plan
  struct _plan_cell_t *plan_next;
  
} plan_cell_t;


// Planner info
typedef struct
{
  // Grid dimensions (number of cells)
  int size_x, size_y;

  // Grid bounds (for limiting the search).
  int min_x, min_y, max_x, max_y;

  // Grid origin (real-world coords, in meters, of the lower-left grid
  // cell)
  double origin_x, origin_y;

  // Grid scale (m/cell)
  double scale;

  // Effective robot radius
  double des_min_radius, abs_min_radius;

  // Max radius we will consider
  double max_radius;

  // Penalty factor for cells inside the max radius
  double dist_penalty;

  // The grid data
  plan_cell_t *cells;

  // Dynamic obstacle hitpoints
  unsigned short* obs_pts;
  size_t obs_pts_size;
  size_t obs_pts_num;

  // Distance penalty kernel, pre-computed in plan_compute_dist_kernel();
  float* dist_kernel;
  int dist_kernel_width;
  float dist_kernel_3x3[9];
  
  // Priority queue of cells to update
  heap_t* heap;

  // The global path
  int path_count, path_size;
  plan_cell_t **path;
  
  // The local path (mainly for debugging)
  int lpath_count, lpath_size;
  plan_cell_t **lpath;

  // Waypoints extracted from global path
  int waypoint_count, waypoint_size;
  plan_cell_t **waypoints;
} plan_t;


// Create a planner
plan_t *plan_alloc(double abs_min_radius, double des_min_radius,
                   double max_radius, double dist_penalty);

void plan_compute_dist_kernel(plan_t* plan);

// Destroy a planner
void plan_free(plan_t *plan);

// Initialize the plan
void plan_init(plan_t *plan);

// Reset the plan
void plan_reset(plan_t *plan);

#if 0
// Load the occupancy values from an image file
int plan_load_occ(plan_t *plan, const char *filename, double scale);
#endif

void plan_set_bounds(plan_t* plan, int min_x, int min_y, int max_x, int max_y);

void plan_set_bbox(plan_t* plan, double padding, double min_size,
                   double x0, double y0, double x1, double y1);

int plan_check_inbounds(plan_t* plan, double x, double y);

// Construct the configuration space from the occupancy grid.
//void plan_update_cspace(plan_t *plan, const char* cachefile);
void plan_compute_cspace(plan_t *plan);

int plan_do_global(plan_t *plan, double lx, double ly, double gx, double gy);

int plan_do_local(plan_t *plan, double lx, double ly, double plan_halfwidth);

// Generate a path to the goal
void plan_update_waypoints(plan_t *plan, double px, double py);

// Get the ith waypoint; returns zero if there are no more waypoints
int plan_get_waypoint(plan_t *plan, int i, double *px, double *py);

// Convert given waypoint cell to global x,y
void plan_convert_waypoint(plan_t* plan, plan_cell_t *waypoint, 
                           double *px, double *py);

double plan_get_carrot(plan_t* plan, double* px, double* py, 
                       double lx, double ly, 
                       double maxdist, double distweight);

#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
// Write the cspace occupancy distance values to a file, one per line.
// Read them back in with plan_read_cspace().
// Returns non-zero on error.
int plan_write_cspace(plan_t *plan, const char* fname, unsigned int* hash);

// Read the cspace occupancy distance values from a file, one per line.
// Write them in first with plan_read_cspace().
// Returns non-zero on error.
int plan_read_cspace(plan_t *plan, const char* fname, unsigned int* hash);

// Compute and return the 16-bit MD5 hash of the map data in the given plan
// object.
void plan_md5(unsigned int* digest, plan_t* plan);
#endif // HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO

/**************************************************************************
 * Plan manipulation macros
 **************************************************************************/

// Convert from plan index to world coords
//#define PLAN_WXGX(plan, i) (((i) - plan->size_x / 2) * plan->scale)
//#define PLAN_WYGY(plan, j) (((j) - plan->size_y / 2) * plan->scale)
#define PLAN_WXGX(plan, i) ((plan)->origin_x + (i) * (plan)->scale)
#define PLAN_WYGY(plan, j) ((plan)->origin_y + (j) * (plan)->scale)

// Convert from world coords to plan coords
//#define PLAN_GXWX(plan, x) (floor((x) / plan->scale + 0.5) + plan->size_x / 2)
//#define PLAN_GYWY(plan, y) (floor((y) / plan->scale + 0.5) + plan->size_y / 2)
#define PLAN_GXWX(plan, x) ((int)(((x) - (plan)->origin_x) / (plan)->scale + 0.5))
#define PLAN_GYWY(plan, y) ((int)(((y) - (plan)->origin_y) / (plan)->scale + 0.5))

// Test to see if the given plan coords lie within the absolute plan bounds.
#define PLAN_VALID(plan, i, j) ((i >= 0) && (i < plan->size_x) && (j >= 0) && (j < plan->size_y))
// Test to see if the given plan coords lie within the user-specified plan bounds
#define PLAN_VALID_BOUNDS(plan, i, j) ((i >= plan->min_x) && (i <= plan->max_x) && (j >= plan->min_y) && (j <= plan->max_y))

// Compute the cell index for the given plan coords.
#define PLAN_INDEX(plan, i, j) ((i) + (j) * plan->size_x)

#ifdef __cplusplus
}
#endif

#endif
