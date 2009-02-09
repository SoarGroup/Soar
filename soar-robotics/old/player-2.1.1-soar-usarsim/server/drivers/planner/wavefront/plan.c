
/**************************************************************************
 * Desc: Path planning
 * Author: Andrew Howard
 * Date: 10 Oct 2002
 * CVS: $Id: plan.c 6295 2008-04-10 01:30:37Z gerkey $
**************************************************************************/

#if HAVE_CONFIG_H
  #include <config.h>
#endif

// This header MUST come before <openssl/md5.h>
#include <sys/types.h>

#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
  #include <openssl/md5.h>
#endif

#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libplayercore/playercommon.h>
#include <libplayercore/error.h>

#include "plan.h"
//#include "heap.h"

#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
// length of the hash, in unsigned ints
#define HASH_LEN (MD5_DIGEST_LENGTH / sizeof(unsigned int))
#endif

#if 0
void draw_cspace(plan_t* plan, const char* fname);
#endif

// Create a planner
plan_t *plan_alloc(double abs_min_radius, double des_min_radius,
                   double max_radius, double dist_penalty)
{
  plan_t *plan;

  plan = calloc(1, sizeof(plan_t));

  plan->abs_min_radius = abs_min_radius;
  plan->des_min_radius = des_min_radius;

  plan->max_radius = max_radius;
  plan->dist_penalty = dist_penalty;
  
  plan->heap = heap_alloc(PLAN_DEFAULT_HEAP_SIZE, (heap_free_elt_fn_t)NULL);
  assert(plan->heap);

  plan->path_size = 1000;
  plan->path = calloc(plan->path_size, sizeof(plan->path[0]));

  plan->lpath_size = 100;
  plan->lpath = calloc(plan->lpath_size, sizeof(plan->lpath[0]));

  plan->waypoint_size = 100;
  plan->waypoints = calloc(plan->waypoint_size, sizeof(plan->waypoints[0]));
  
  return plan;
}

void
plan_set_obstacles(plan_t* plan, double* obs, size_t num)
{
  size_t i;
  int di,dj;
  float* p;
  plan_cell_t* cell, *ncell;

  // Remove any previous obstacles
  for(i=0;i<plan->obs_pts_num;i++)
  {
    cell = plan->cells + 
            PLAN_INDEX(plan,plan->obs_pts[2*i],plan->obs_pts[2*i+1]);
    cell->occ_state_dyn = cell->occ_state;
    cell->occ_dist_dyn = cell->occ_dist;
  }

  // Do we need more room?
  if(num > plan->obs_pts_size)
  {
    plan->obs_pts_size = num;
    plan->obs_pts = (unsigned short*)realloc(plan->obs_pts, 
                                             sizeof(unsigned short) * 2 *
                                             plan->obs_pts_size);
    assert(plan->obs_pts);
  }

  // Copy and expand costs around them
  plan->obs_pts_num = num;
  for(i=0;i<plan->obs_pts_num;i++)
  {
    // Convert to grid coords
    int gx,gy;
    gx = PLAN_GXWX(plan, obs[2*i]);
    gy = PLAN_GYWY(plan, obs[2*i+1]);
    plan->obs_pts[2*i] = gx;
    plan->obs_pts[2*i+1] = gy;

    cell = plan->cells + PLAN_INDEX(plan,gx,gy);

    cell->occ_state_dyn = 1;
    cell->occ_dist_dyn = 0.0;

    p = plan->dist_kernel;
    for (dj = -plan->dist_kernel_width/2; 
         dj <= plan->dist_kernel_width/2; 
         dj++)
    {
      ncell = cell + -plan->dist_kernel_width/2 + dj*plan->size_x;
      for (di = -plan->dist_kernel_width/2;
           di <= plan->dist_kernel_width/2; 
           di++, p++, ncell++)
      {
        if(!PLAN_VALID_BOUNDS(plan,cell->ci+di,cell->cj+dj))            
          continue;

        if(*p < ncell->occ_dist_dyn)
          ncell->occ_dist_dyn = *p;
      }
    }
  }
}

void
plan_compute_dist_kernel(plan_t* plan)
{
  int i,j;
  float* p;

  // Compute variable sized kernel, for use in propagating distance from
  // obstacles
  plan->dist_kernel_width = 1 + 2 * (int)ceil(plan->max_radius / plan->scale);
  plan->dist_kernel = (float*)realloc(plan->dist_kernel,
                                      sizeof(float) * 
                                      plan->dist_kernel_width *
                                      plan->dist_kernel_width);
  assert(plan->dist_kernel);

  p = plan->dist_kernel;
  for(j=-plan->dist_kernel_width/2;j<=plan->dist_kernel_width/2;j++)
  {
    for(i=-plan->dist_kernel_width/2;i<=plan->dist_kernel_width/2;i++,p++)
    {
      *p = sqrt(i*i+j*j) * plan->scale;
    }
  }
  // also compute a 3x3 kernel, used when propagating distance from goal
  p = plan->dist_kernel_3x3;
  for(j=-1;j<=1;j++)
  {
    for(i=-1;i<=1;i++,p++)
    {
      *p = sqrt(i*i+j*j) * plan->scale;
    }
  }
}


// Destroy a planner
void plan_free(plan_t *plan)
{
  if (plan->cells)
    free(plan->cells);
  heap_free(plan->heap);
  free(plan->waypoints);
  if(plan->dist_kernel)
    free(plan->dist_kernel);
  free(plan);

  return;
}

// Initialize the plan
void plan_init(plan_t *plan)
{
  int i, j;
  plan_cell_t *cell;

  cell = plan->cells;
  for (j = 0; j < plan->size_y; j++)
  {
    for (i = 0; i < plan->size_x; i++, cell++)
    {
      cell->ci = i;
      cell->cj = j;
      cell->occ_state_dyn = cell->occ_state;
      if(cell->occ_state >= 0)
        cell->occ_dist_dyn = cell->occ_dist = 0.0;
      else
        cell->occ_dist_dyn = cell->occ_dist = plan->max_radius;
      cell->plan_cost = PLAN_MAX_COST;
      cell->plan_next = NULL;
    }
  }
  plan->waypoint_count = 0;

  plan_compute_dist_kernel(plan);

  plan_set_bounds(plan, 0, 0, plan->size_x - 1, plan->size_y - 1);
}


// Reset the plan
void plan_reset(plan_t *plan)
{
  int i, j;
  plan_cell_t *cell;

  for (j = plan->min_y; j <= plan->max_y; j++)
  {
    for (i = plan->min_x; i <= plan->max_x; i++)
    {
      cell = plan->cells + PLAN_INDEX(plan,i,j);
      cell->plan_cost = PLAN_MAX_COST;
      cell->plan_next = NULL;
      cell->mark = 0;
    }
  }
  plan->waypoint_count = 0;
}

void
plan_set_bounds(plan_t* plan, int min_x, int min_y, int max_x, int max_y)
{
  min_x = MAX(0,min_x);
  min_x = MIN(plan->size_x-1, min_x);
  min_y = MAX(0,min_y);
  min_y = MIN(plan->size_y-1, min_y);
  max_x = MAX(0,max_x);
  max_x = MIN(plan->size_x-1, max_x);
  max_y = MAX(0,max_y);
  max_y = MIN(plan->size_y-1, max_y);

  assert(min_x <= max_x);
  assert(min_y <= max_y);

  plan->min_x = min_x;
  plan->min_y = min_y;
  plan->max_x = max_x;
  plan->max_y = max_y;

  //printf("new bounds: (%d,%d) -> (%d,%d)\n",
         //plan->min_x, plan->min_y,
         //plan->max_x, plan->max_y);
}

int
plan_check_inbounds(plan_t* plan, double x, double y)
{
  int gx, gy;

  gx = PLAN_GXWX(plan, x);
  gy = PLAN_GYWY(plan, y);

  if((gx >= plan->min_x) && (gx <= plan->max_x) &&
     (gy >= plan->min_y) && (gy <= plan->max_y))
    return(1);
  else
    return(0);
}

void
plan_set_bbox(plan_t* plan, double padding, double min_size,
              double x0, double y0, double x1, double y1)
{
  int gx0, gy0, gx1, gy1;
  int min_x, min_y, max_x, max_y;
  int sx, sy;
  int dx, dy;
  int gmin_size;
  int gpadding;

  gx0 = PLAN_GXWX(plan, x0);
  gy0 = PLAN_GYWY(plan, y0);
  gx1 = PLAN_GXWX(plan, x1);
  gy1 = PLAN_GYWY(plan, y1);

  // Make a bounding box to include both points.
  min_x = MIN(gx0, gx1);
  min_y = MIN(gy0, gy1);
  max_x = MAX(gx0, gx1);
  max_y = MAX(gy0, gy1);

  // Make sure the min_size is achievable
  gmin_size = (int)ceil(min_size / plan->scale);
  gmin_size = MIN(gmin_size, MIN(plan->size_x-1, plan->size_y-1));

  // Add padding
  gpadding = (int)ceil(padding / plan->scale);
  min_x -= gpadding / 2;
  min_x = MAX(min_x, 0);
  max_x += gpadding / 2;
  max_x = MIN(max_x, plan->size_x - 1);
  min_y -= gpadding / 2;
  min_y = MAX(min_y, 0);
  max_y += gpadding / 2;
  max_y = MIN(max_y, plan->size_y - 1);

  // Grow the box if necessary to achieve the min_size
  sx = max_x - min_x;
  while(sx < gmin_size)
  {
    dx = gmin_size - sx;
    min_x -= (int)ceil(dx / 2.0);
    max_x += (int)ceil(dx / 2.0);

    min_x = MAX(min_x, 0);
    max_x = MIN(max_x, plan->size_x-1);

    sx = max_x - min_x;
  }
  sy = max_y - min_y;
  while(sy < gmin_size)
  {
    dy = gmin_size - sy;
    min_y -= (int)ceil(dy / 2.0);
    max_y += (int)ceil(dy / 2.0);

    min_y = MAX(min_y, 0);
    max_y = MIN(max_y, plan->size_y-1);

    sy = max_y - min_y;
  }

  plan_set_bounds(plan, min_x, min_y, max_x, max_y);
}

void
plan_compute_cspace(plan_t* plan)
{
  int i, j;
  int di, dj;
  float* p;
  plan_cell_t *cell, *ncell;

  PLAYER_MSG0(2,"Generating C-space....");

  for (j = plan->min_y; j <= plan->max_y; j++)
  {
    cell = plan->cells + PLAN_INDEX(plan, 0, j);
    for (i = plan->min_x; i <= plan->max_x; i++, cell++)
    {
      if (cell->occ_state < 0)
        continue;

      p = plan->dist_kernel;
      for (dj = -plan->dist_kernel_width/2; 
           dj <= plan->dist_kernel_width/2; 
           dj++)
      {
        ncell = cell + -plan->dist_kernel_width/2 + dj*plan->size_x;
        for (di = -plan->dist_kernel_width/2;
             di <= plan->dist_kernel_width/2; 
             di++, p++, ncell++)
        {
          if(!PLAN_VALID_BOUNDS(plan,i+di,j+dj))            
            continue;

          if(*p < ncell->occ_dist)
            ncell->occ_dist_dyn = ncell->occ_dist = *p;
        }
      }
    }
  }
}

#if 0
#include <gdk-pixbuf/gdk-pixbuf.h>

        void
draw_cspace(plan_t* plan, const char* fname)
{
  GdkPixbuf* pixbuf;
  GError* error = NULL;
  guchar* pixels;
  int p;
  int paddr;
  int i, j;

  pixels = (guchar*)malloc(sizeof(guchar)*plan->size_x*plan->size_y*3);

  p=0;
  for(j=plan->size_y-1;j>=0;j--)
  {
    for(i=0;i<plan->size_x;i++,p++)
    {
      paddr = p * 3;
      if(plan->cells[PLAN_INDEX(plan,i,j)].occ_state == 1)
      {
        pixels[paddr] = 255;
        pixels[paddr+1] = 0;
        pixels[paddr+2] = 0;
      }
      else if(plan->cells[PLAN_INDEX(plan,i,j)].occ_dist < plan->max_radius)
      {
        pixels[paddr] = 0;
        pixels[paddr+1] = 0;
        pixels[paddr+2] = 255;
      }
      else
      {
        pixels[paddr] = 255;
        pixels[paddr+1] = 255;
        pixels[paddr+2] = 255;
      }
    }
  }

  pixbuf = gdk_pixbuf_new_from_data(pixels, 
                                    GDK_COLORSPACE_RGB,
                                    0,8,
                                    plan->size_x,
                                    plan->size_y,
                                    plan->size_x * 3,
                                    NULL, NULL);

  gdk_pixbuf_save(pixbuf,fname,"png",&error,NULL);
  gdk_pixbuf_unref(pixbuf);
  free(pixels);
}

        void
draw_path(plan_t* plan, double lx, double ly, const char* fname)
{
  GdkPixbuf* pixbuf;
  GError* error = NULL;
  guchar* pixels;
  int p;
  int paddr;
  int i, j;
  plan_cell_t* cell;

  pixels = (guchar*)malloc(sizeof(guchar)*plan->size_x*plan->size_y*3);

  p=0;
  for(j=plan->size_y-1;j>=0;j--)
  {
    for(i=0;i<plan->size_x;i++,p++)
    {
      paddr = p * 3;
      if(plan->cells[PLAN_INDEX(plan,i,j)].occ_state == 1)
      {
        pixels[paddr] = 255;
        pixels[paddr+1] = 0;
        pixels[paddr+2] = 0;
      }
      else if(plan->cells[PLAN_INDEX(plan,i,j)].occ_dist < plan->max_radius)
      {
        pixels[paddr] = 0;
        pixels[paddr+1] = 0;
        pixels[paddr+2] = 255;
      }
      else
      {
        pixels[paddr] = 255;
        pixels[paddr+1] = 255;
        pixels[paddr+2] = 255;
      }
      /*
         if((7*plan->cells[PLAN_INDEX(plan,i,j)].plan_cost) > 255)
         {
         pixels[paddr] = 0;
         pixels[paddr+1] = 0;
         pixels[paddr+2] = 255;
         }
         else
         {
         pixels[paddr] = 255 - 7*plan->cells[PLAN_INDEX(plan,i,j)].plan_cost;
         pixels[paddr+1] = 0;
         pixels[paddr+2] = 0;
         }
       */
    }
  }

  for(i=0;i<plan->path_count;i++)
  {
    cell = plan->path[i];
    
    paddr = 3*PLAN_INDEX(plan,cell->ci,plan->size_y - cell->cj - 1);
    pixels[paddr] = 0;
    pixels[paddr+1] = 255;
    pixels[paddr+2] = 0;
  }

  for(i=0;i<plan->lpath_count;i++)
  {
    cell = plan->lpath[i];
    
    paddr = 3*PLAN_INDEX(plan,cell->ci,plan->size_y - cell->cj - 1);
    pixels[paddr] = 255;
    pixels[paddr+1] = 0;
    pixels[paddr+2] = 255;
  }

  /*
  for(p=0;p<plan->waypoint_count;p++)
  {
    cell = plan->waypoints[p];
    for(j=-3;j<=3;j++)
    {
      cj = cell->cj + j;
      for(i=-3;i<=3;i++)
      {
        ci = cell->ci + i;
        paddr = 3*PLAN_INDEX(plan,ci,plan->size_y - cj - 1);
        pixels[paddr] = 255;
        pixels[paddr+1] = 0;
        pixels[paddr+2] = 255;
      }
    }
  }
  */

  pixbuf = gdk_pixbuf_new_from_data(pixels, 
                                    GDK_COLORSPACE_RGB,
                                    0,8,
                                    plan->size_x,
                                    plan->size_y,
                                    plan->size_x * 3,
                                    NULL, NULL);
  
  gdk_pixbuf_save(pixbuf,fname,"png",&error,NULL);
  gdk_pixbuf_unref(pixbuf);
  free(pixels);
}
#endif

// Construct the configuration space from the occupancy grid.
// This treats both occupied and unknown cells as bad.
// 
// If cachefile is non-NULL, then we try to read the c-space from that
// file.  If that fails, then we construct the c-space as per normal and
// then write it out to cachefile.
#if 0
void 
plan_update_cspace(plan_t *plan, const char* cachefile)
{
#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
  unsigned int hash[HASH_LEN];
  plan_md5(hash, plan);
  if(cachefile && strlen(cachefile))
  {
    PLAYER_MSG1(2,"Trying to read c-space from file %s", cachefile);
    if(plan_read_cspace(plan,cachefile,hash) == 0)
    {
      // Reading from the cache file worked; we're done here.
      PLAYER_MSG1(2,"Successfully read c-space from file %s", cachefile);
#if 0
      draw_cspace(plan,"plan_cspace.png");
#endif
      return;
    }
    PLAYER_MSG1(2, "Failed to read c-space from file %s", cachefile);
  }
#endif

  //plan_update_cspace_dp(plan);
  plan_update_cspace_naive(plan);

#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
  if(cachefile)
    plan_write_cspace(plan,cachefile, (unsigned int*)hash);
#endif

  PLAYER_MSG0(2,"Done.");

#if 0
  draw_cspace(plan,"plan_cspace.png");
#endif
}
#endif

#if HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
// Write the cspace occupancy distance values to a file, one per line.
// Read them back in with plan_read_cspace().
// Returns non-zero on error.
int 
plan_write_cspace(plan_t *plan, const char* fname, unsigned int* hash)
{
  plan_cell_t* cell;
  int i,j;
  FILE* fp;

  if(!(fp = fopen(fname,"w+")))
  {
    PLAYER_MSG2(2,"Failed to open file %s to write c-space: %s",
                fname,strerror(errno));
    return(-1);
  }

  fprintf(fp,"%d\n%d\n", plan->size_x, plan->size_y);
  fprintf(fp,"%.3lf\n%.3lf\n", plan->origin_x, plan->origin_y);
  fprintf(fp,"%.3lf\n%.3lf\n", plan->scale,plan->max_radius);
  for(i=0;i<HASH_LEN;i++)
    fprintf(fp,"%08X", hash[i]);
  fprintf(fp,"\n");

  for(j = 0; j < plan->size_y; j++)
  {
    for(i = 0; i < plan->size_x; i++)
    {
      cell = plan->cells + PLAN_INDEX(plan, i, j);
      fprintf(fp,"%.3f\n", cell->occ_dist);
    }
  }

  fclose(fp);
  return(0);
}

// Read the cspace occupancy distance values from a file, one per line.
// Write them in first with plan_read_cspace().
// Returns non-zero on error.
int 
plan_read_cspace(plan_t *plan, const char* fname, unsigned int* hash)
{
  plan_cell_t* cell;
  int i,j;
  FILE* fp;
  int size_x, size_y;
  double origin_x, origin_y;
  double scale, max_radius;
  unsigned int cached_hash[HASH_LEN];

  if(!(fp = fopen(fname,"r")))
  {
    PLAYER_MSG1(2,"Failed to open file %s", fname);
    return(-1);
  }
  
  /* Read out the metadata */
  if((fscanf(fp,"%d", &size_x) < 1) ||
     (fscanf(fp,"%d", &size_y) < 1) ||
     (fscanf(fp,"%lf", &origin_x) < 1) ||
     (fscanf(fp,"%lf", &origin_y) < 1) ||
     (fscanf(fp,"%lf", &scale) < 1) ||
     (fscanf(fp,"%lf", &max_radius) < 1))
  {
    PLAYER_MSG1(2,"Failed to read c-space metadata from file %s", fname);
    fclose(fp);
    return(-1);
  }

  for(i=0;i<HASH_LEN;i++)
  {
    if(fscanf(fp,"%08X", cached_hash+i) < 1)
    {
      PLAYER_MSG1(2,"Failed to read c-space metadata from file %s", fname);
      fclose(fp);
      return(-1);
    }
  }

  /* Verify that metadata matches */
  if((size_x != plan->size_x) ||
     (size_y != plan->size_y) ||
     (fabs(origin_x - plan->origin_x) > 1e-3) ||
     (fabs(origin_y - plan->origin_y) > 1e-3) ||
     (fabs(scale - plan->scale) > 1e-3) ||
     (fabs(max_radius - plan->max_radius) > 1e-3) ||
     memcmp(cached_hash, hash, sizeof(unsigned int) * HASH_LEN))
  {
    PLAYER_MSG1(2,"Mismatch in c-space metadata read from file %s", fname);
    fclose(fp);
    return(-1);
  }

  for(j = 0; j < plan->size_y; j++)
  {
    for(i = 0; i < plan->size_x; i++)
    {
      cell = plan->cells + PLAN_INDEX(plan, i, j);
      if(fscanf(fp,"%f", &(cell->occ_dist)) < 1)
      {
        PLAYER_MSG3(2,"Failed to read c-space data for cell (%d,%d) from file %s",
                     i,j,fname);
        fclose(fp);
        return(-1);
      }
    }
  }

  fclose(fp);
  return(0);
}

// Compute the 16-byte MD5 hash of the map data in the given plan
// object.
void
plan_md5(unsigned int* digest, plan_t* plan)
{
  MD5_CTX c;

  MD5_Init(&c);

  MD5_Update(&c,(const unsigned char*)plan->cells,
             (plan->size_x*plan->size_y)*sizeof(plan_cell_t));

  MD5_Final((unsigned char*)digest,&c);
}
#endif // HAVE_OPENSSL_MD5_H && HAVE_LIBCRYPTO
