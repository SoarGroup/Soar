/*=========================================================================
    CMVision.cc
  -------------------------------------------------------------------------
    Implementation of the CMVision real time Color Machine Vision library
  -------------------------------------------------------------------------
    Copyright 1999, 2000         #### ### ### ## ## ## #### ##  ###  ##  ##
    James R. Bruce              ##    ####### ## ## ## ##   ## ## ## ######
    School of Computer Science  ##    ## # ## ## ## ##  ### ## ## ## ## ###
    Carnegie Mellon University   #### ##   ##  ###  ## #### ##  ###  ##  ##
  -------------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  -------------------------------------------------------------------------
    Revision History:
      1999-11-18:  Initial release version (JRB)
      2000-05-20:  Added Bugfixes from Peter,
                   fixed bounding box bug (JRB)
      2000-06-04:  Some other minor fixes (JRB)
      2000-07-02:  Added average color and density merging (JRB)
      2000-07-20:  Added dual threshold capability (JRB)
  =========================================================================*/

#include "cmvision.h"
#include <string.h>
#include <strings.h>

//==== Utility Functions ===========================================//
// These could be coded as macros, but the inline versions seem to
// optimize better, and tend to have cleaner definitions

// sum of integers over range [x,x+w)
inline int range_sum(int x,int w)
{
  return(w*(2*x + w-1) / 2);
}

// returns maximal value of two parameters
template <class num>
inline num max(num a,num b)
{
  return((a > b)? a : b);
}

// returns minimal value of two parameters
template <class num>
inline num min(num a,num b)
{
  return((a < b)? a : b);
}

// returns index of least significant set bit
int log2modp[37] = {
  0, 1, 2,27, 3,24,28, 0, 4,17,25,31,29,12, 0,14, 5, 8,18,
  0,26,23,32,16,30,11,13, 7, 0,22,15,10, 6,21, 9,20,19
};

template <class num>
inline int bottom_bit(num n)
{
  return(log2modp[(n & -n) % 37]);
}

/* Marginally slower naive version of above function
template <class num>
inline num bottom_bit(num n)
{
  int i = 0;
  if(!n) return(0);
  while(!(n&(1<<i))) i++;
  return(i + 1);
}
*/

// returns index of most significant set bit
template <class num>
inline num top_bit(num n)
{
  int i = 1;
  if(!n) return(0);
  while(n>>i) i++;
  return(i);
}


//==== Class Implementation ========================================//

void CMVision::classifyFrame(image_pixel * restrict img,unsigned * restrict map)
// Classifies an image passed in as img, saving bits in the entries
// of map representing which thresholds that pixel satisfies.
{
  int i,m,s;
  int m1,m2;
  image_pixel p;

  unsigned *uclas = u_class; // Ahh, the joys of a compiler that
  unsigned *vclas = v_class; //   has to consider pointer aliasing
  unsigned *yclas = y_class;

  s = width * height;

  if(options & CMV_DUAL_THRESHOLD){
    for(i=0; i<s; i+=2){
      p = img[i/2];
      m = uclas[p.u] & vclas[p.v];
      m1 = m & yclas[p.y1];
      m2 = m & yclas[p.y2];
      map[i + 0] = m1 | (m1 >> 16);
      map[i + 1] = m2 | (m2 >> 16);
    }
  }else{
    for(i=0; i<s; i+=2){
      p = img[i/2];
      m = uclas[p.u] & vclas[p.v];
      map[i + 0] = m & yclas[p.y1];
      map[i + 1] = m & yclas[p.y2];
    }
  }
}

int CMVision::encodeRuns(rle * restrict out,unsigned * restrict map)
// Changes the flat array version of the threshold satisfaction map
// into a run length encoded version, which speeds up later processing
// since we only have to look at the points where values change.
{
  int x,y,j,l;
  unsigned m,save;
  int size;
  unsigned *row;
  rle r;

  size = width * height;

  // initialize terminator restore
  save = map[0];

  j = 0;
  for(y=0; y<height; y++){
    row = &map[y * width];

    // restore previous terminator and store next
    // one in the first pixel on the next row
    row[0] = save;
    save = row[width];
    row[width] = CMV_NONE;

    x = 0;
    while(x < width){
      m = row[x];
      // m = m & (~m + 1); // get last bit
      l = x;
      while(row[x] == m) x++;
      // x += (row[x] == CMV_NONE); //  && (last & m);

      r.color  = m;
      r.length = x - l;
      r.parent = j;
      out[j++] = r;
      if(j >= CMV_MAX_RUNS) return(0);
    }
  }

  return(j);
}

void CMVision::connectComponents(rle * restrict map,int num)
// Connect components using four-connecteness so that the runs each
// identify the global parent of the connected region they are a part
// of.  It does this by scanning adjacent rows and merging where similar
// colors overlap.  Used to be union by rank w/ path compression, but now
// is just uses path compression as the global parent index seems to be
// a simpler fast approximation of rank in practice.
// WARNING: This code is *extremely* complicated and twitchy.  It appears
//   to be a correct implementation, but minor changes can easily cause
//   big problems.  Read the papers on this library and have a good
//   understanding of tree-based union find before you touch it
{
  int x1,x2;
  int l1,l2;
  rle r1,r2;
  int i,p,s,n;

  l1 = l2 = 0;
  x1 = x2 = 0;

  // Lower scan begins on second line, so skip over first
  while(x1 < width){
    x1 += map[l1++].length;
  }
  x1 = 0;

  // Do rest in lock step
  r1 = map[l1];
  r2 = map[l2];
  s = l1;
  while(l1 < num){
    if(r1.color==r2.color && r1.color){
      if((x1>=x2 && x1<x2+r2.length) || (x2>=x1 && x2<x1+r1.length)){
        if(s != l1){
          map[l1].parent = r1.parent = r2.parent;
          s = l1;
        }else{
          // find terminal roots of each path
          n = r1.parent;
          while(n != map[n].parent) n = map[n].parent;
          p = r2.parent;
          while(p != map[p].parent) p = map[p].parent;

          // must use smaller of two to preserve DAGness!
          if(n < p){
            map[p].parent = n;
          }else{
            map[n].parent = p;
          }
        }
      }
    }

    // Move to next point where values may change
    if(x1+r1.length < x2+r2.length){
      x1 += r1.length;
      r1 = map[++l1];
    }else{
      x2 += r2.length;
      r2 = map[++l2];
    }
  }

  // Now we need to compress all parent paths
  for(i=0; i<num; i++){
    p = map[i].parent;
    if(p > i){
      while(p != map[p].parent) p = map[p].parent;
      map[i].parent = p;
    }else{
      map[i].parent = map[p].parent;
    }
  }

  // Ouch, my brain hurts.
}

int CMVision::extractRegions(region * restrict reg,rle * restrict rmap,int num)
// Takes the list of runs and formats them into a region table,
// gathering the various statistics we want along the way.
// num is the number of runs in the rmap array, and the number of
// unique regions in reg[] (< CMV_MAX_REGIONS) is returned.
// Implemented as a single pass over the array of runs.
{
  int x,y,i;
  int b,n,a;
  rle r;
  yuv black = {0,0,0};

  x = y = n = 0;
  for(i=0; i<num; i++){
    r = rmap[i];

    if(r.color){
      if(r.parent == i){
        // Add new region if this run is a root (i.e. self parented)
        rmap[i].parent = b = n;  // renumber to point to region id
        reg[b].color = bottom_bit(r.color) - 1;
        reg[b].area = r.length;
        reg[b].x1 = x;
        reg[b].y1 = y;
        reg[b].x2 = x + r.length;
        reg[b].y2 = y;
        reg[b].sum_x = range_sum(x,r.length);
        reg[b].sum_y = y * r.length;
        reg[b].average = black;
        // reg[b].area_check = 0; // DEBUG ONLY
        n++;
        if(n >= CMV_MAX_REGIONS) return(CMV_MAX_REGIONS);
      }else{
        // Otherwise update region stats incrementally
        b = rmap[r.parent].parent;
        rmap[i].parent = b; // update to point to region id
        reg[b].area += r.length;
        reg[b].x2 = max(x + r.length,reg[b].x2);
        reg[b].x1 = min(x,reg[b].x1);
        reg[b].y2 = y; // last set by lowest run
        reg[b].sum_x += range_sum(x,r.length);
        reg[b].sum_y += y * r.length;
      }
      /* DEBUG
      if(r.color == 1){
        printf("{%d,%d,%d} ",i,rmap[i].parent,b);
      }
      */
    }

    // step to next location
    x = (x + r.length) % width;
    y += (x == 0);
  }

  // printf("\n");

  // calculate centroids from stored temporaries
  for(i=0; i<n; i++){
    a = reg[i].area;
    reg[i].cen_x = (float)reg[i].sum_x / a;
    reg[i].cen_y = (float)reg[i].sum_y / a;
  }

  return(n);
}

void CMVision::calcAverageColors(region * restrict reg,int num_reg,
                                 image_pixel * restrict img,
                                 rle * restrict rmap,int num_runs)
// calculates the average color for each region.
// num is the number of runs in the rmap array, and the number of
// unique regions in reg[] (< CMV_MAX_REGIONS) is returned.
// Implemented as a single pass over the image, and a second pass over
// the regions.
{
  int i,j,x,l;
  image_pixel p;
  rle r;
  int sum_y,sum_u,sum_v;
  int b,xs;

  yuv avg;
  int area;

  // clear out temporaries
  for(i=0; i<num_reg; i++){
    reg[i].sum_x = 0;
    reg[i].sum_y = 0;
    reg[i].sum_z = 0;
  }

  x = 0;

  // printf("FRAME_START\n");

  // sum up color components for each region, by traversing image and runs
  for(i=0; i<num_runs; i++){
    r = rmap[i];
    l = r.length;

    if(!r.color){
      x += l;
    }else{
      xs = x;
      p = img[x / 2];

      if(x & 1){
        sum_y = p.y2;
        sum_u = p.u;
        sum_v = p.v;
        // area = 1;
        x++;
        l--;
      }else{
        sum_y = sum_u = sum_v = 0;
        area = 0;
      }

      for(j=0; j<l/2; j++){
        p = img[x / 2];
        sum_y += p.y1 + p.y2;
        sum_u += 2 * p.u;
        sum_v += 2 * p.v;
        x+=2;
        // area += 2;
      }

      if(l & 1){
        x++;
        p = img[x / 2];
        sum_y += p.y1;
        sum_u += p.u;
        sum_v += p.v;
        // area++;
      }

      // add sums to region
      b = r.parent;
      reg[b].sum_x += sum_y;
      reg[b].sum_y += sum_u;
      reg[b].sum_z += sum_v;
      // reg[b].area_check += area;

      /*
      if((r.color & (1 << reg[b].color)) != (1 << reg[b].color)){
        printf("(%d,%d)",r.color,reg[b].color);
      }

      if(x != xs + r.length){
	printf("Length mismatch %d:%d\n",x,xs + r.length);
      }
      */

      x = xs + r.length;
    }
  }

  // Divide sums by area to calculate average colors
  for(i=0; i<num_reg; i++){
    area = reg[i].area;
    avg.y = reg[i].sum_x / area;
    avg.u = reg[i].sum_y / area;
    avg.v = reg[i].sum_z / area;

    /*
    if(reg[i].area != reg[i].area_check){
      printf("Area Mismatch: %d %d\n",reg[i].area,reg[i].area_check);
    }

    x = (y_class[avg.y] & u_class[avg.u] & v_class[avg.v]);
    j = reg[i].color;
    l = (1 << j);
    if((x & l) != l){
      printf("Error: c=%d a=%d (%d,%d) (%d,%d,%d)\n",
	     reg[i].color,area,
	     (int)reg[i].cen_x,(int)reg[i].cen_y,
             avg.y,avg.u,avg.v);
    }
    */

    reg[i].average = avg;
  }
}

int CMVision::separateRegions(region * restrict reg,int num)
// Splits the various regions in the region table a separate list
// for each color.  The lists are threaded through the table using
// the region's 'next' field.  Returns the maximal area of the
// regions, which we use below to speed up sorting.
{
  region *p;
  int i,l;
  int area,max_area;

  // clear out the region table
  for(i=0; i<CMV_MAX_COLORS; i++){
    region_count[i] = 0;
    region_list[i] = NULL;
  }

  // step over the table, adding successive
  // regions to the front of each list
  max_area = 0;
  for(i=0; i<num; i++){
    p = &reg[i];
    area = p->area;
    if(area >= CMV_MIN_AREA){
      if(area > max_area) max_area = area;
      l = p->color;
      region_count[l]++;
      p->next = region_list[l];
      region_list[l] = p;
    }
  }

  return(max_area);
}

// These are the tweaking values for the radix sort given below
// Feel free to change them, though these values seemed to work well
// in testing.  Don't worry about extra passes to get all 32 bits of
// the area; the implementation only does as many passes as needed to
// touch the most significant set bit (MSB of biggest region's area)
#define CMV_RBITS 6
#define CMV_RADIX (1 << CMV_RBITS)
#define CMV_RMASK (CMV_RADIX-1)

CMVision::region *CMVision::sortRegionListByArea(region * restrict list,int passes)
// Sorts a list of regions by their area field.
// Uses a linked list based radix sort to process the list.
{
  region *tbl[CMV_RADIX],*p,*pn;
  int slot,shift;
  int i,j;

  // Handle trivial cases
  if(!list || !list->next) return(list);

  // Initialize table
  for(j=0; j<CMV_RADIX; j++) tbl[j] = NULL;

  for(i=0; i<passes; i++){
    // split list into buckets
    shift = CMV_RBITS * i;
    p = list;
    while(p){
      pn = p->next;
      slot = ((p->area) >> shift) & CMV_RMASK;
      p->next = tbl[slot];
      tbl[slot] = p;
      p = pn;
    }

    // integrate back into partially ordered list
    list = NULL;
    for(j=0; j<CMV_RADIX; j++){
      p = tbl[j];
      tbl[j] = NULL;  // clear out table for next pass
      while(p){
        pn = p->next;
        p->next = list;
        list = p;
        p = pn;
      }
    }
  }

  return(list);
}

void CMVision::sortRegions(int max_area)
// Sorts entire region table by area, using the above
// function to sort each threaded region list.
{
  int i,p;

  // do minimal number of passes sufficient to touch all set bits
  p = top_bit((max_area + CMV_RBITS-1) / CMV_RBITS);

  // sort each list
  for(i=0; i<CMV_MAX_COLORS; i++){
    region_list[i] = sortRegionListByArea(region_list[i],p);
  }
}

int CMVision::mergeRegions(region *p,int num,double density_thresh)
// Looks through regions and merges pairs of the same color that would
// have a high density after combining them (where density is the area
// in pixels of the region divided by the bounding box area).  This
// implementation sucks, and I promise real spatial data structures in
// the future so n^2 ugliness like this is not necessary.
{
  region *q,*s;
  int l,r,t,b;
  int a;
  int merged;

  //double tmp;

  merged = 0;

  while(p && merged<num){
    q = p->next;
    s = p;

    while(q){
      // find union box and get its total area
      l = min(p->x1,q->x1);
      r = max(p->x2,q->x2);
      t = min(p->y1,q->y1);
      b = max(p->y2,q->y2);
      a = (r-l) * (b-t);

      // if density of merged region is still above threshold
      if((double)(p->area + q->area) / a > density_thresh){
	// merge them to create a new region
	a = p->area + q->area;
	p->x1 = l;
	p->x2 = r;
	p->y1 = t;
	p->y2 = b;
	p->cen_x = ((p->cen_x * p->area) + (q->cen_x * q->area)) / a;
	p->cen_y = ((p->cen_y * p->area) + (q->cen_y * q->area)) / a;
	p->area = a;

	// remove q from list (old smaller region)
	q = q->next;
	s->next = q;
	merged++;
      }else{
	s = q;
	q = q->next;
      }
    }
    p = p->next;
  }

  return(merged);
}

int CMVision::mergeRegions()
// Apply merge operation to all regions using the above function.
{
  int i,m;
  int num;

  num = 0;

  for(i=0; i<CMV_MAX_COLORS; i++){
    m = mergeRegions(region_list[i],colors[i].expected_num,colors[i].merge);
    region_count[i] -= m;
    num += m;
  }

  return(num);
}

//==== Interface/Public Functions ==================================//

#define ZERO(x) memset(x,0,sizeof(x))

void CMVision::clear()
{
  ZERO(y_class);
  ZERO(u_class);
  ZERO(v_class);

  ZERO(region_list);
  ZERO(region_count);

  ZERO(colors);

  map = NULL;
}

bool CMVision::initialize(int nwidth,int nheight)
// Initializes library to work with images of specified size
{
  width = nwidth;
  height = nheight;

  if(map) delete(map);

  map = new unsigned[width * height + 1];
  // Need 1 extra element to store terminator value in encodeRuns()

  options = CMV_THRESHOLD;

  return(map != NULL);
}

// sets bits in k in array arr[l..r]
template <class num>
void set_bits(num *arr,int len,int l,int r,num k)
{
  int i;

  l = max(l,0);
  r = min(r+1,len);

  for(i=l; i<r; i++) arr[i] |= k;
}

template <class num>
void clear_bits(num *arr,int len,int l,int r,num k)
{
  int i;

  l = max(l,0);
  r = min(r+1,len);

  k = ~k;
  for(i=l; i<r; i++) arr[i] &= k;
}

#define CMV_STATE_SCAN   0
#define CMV_STATE_COLORS 1
#define CMV_STATE_THRESH 2
#define CMV_MAX_BUF 256

bool CMVision::loadOptions(char *filename)
// Loads in options file specifying color names and representative
// rgb triplets.  Also loads in color class threshold values.
{
  char buf[CMV_MAX_BUF],str[CMV_MAX_BUF];
  FILE *in;
  int state,i,n;

  int r,g,b;
  int exp_num;
  double merge;
  color_info *c;

  int y1,y2,u1,u2,v1,v2;
  unsigned k;

  // Open options file
  in = fopen(filename,"rt");
  if(!in) return(false);

  // Clear out previously set options
  for(i=0; i<CMV_COLOR_LEVELS; i++){
    y_class[i] = u_class[i] = v_class[i] = 0;
  }
  for(i=0; i<CMV_MAX_COLORS; i++){
    if(colors[i].name){
      delete(colors[i].name);
      colors[i].name = NULL;
    }
  }

  // Loop ever lines, processing via a simple parser
  state = 0;
  while(fgets(buf,CMV_MAX_BUF,in)){
    switch(state){
      case CMV_STATE_SCAN:
        n = sscanf(buf,"[%s",str);
        if(n == 1){
          if(!strncasecmp(str,"colors]",CMV_MAX_BUF)){
            state = CMV_STATE_COLORS;
            i = 0;
	  }else if(!strncasecmp(str,"thresholds]",CMV_MAX_BUF)){
	    state = CMV_STATE_THRESH;
            i = 0;
	  }else{
            printf("CMVision: Ignoring unknown option header '%s'.\n",str);
          }
	}
        break;
      case CMV_STATE_COLORS:
        n = sscanf(buf,"(%d,%d,%d) %lf %d %s",&r,&g,&b,&merge,&exp_num,str);
        if(n == 6){
          // printf("(%d,%d,%d) %lf %d '%s'\n",
	  //        r,g,b,merge,exp_num,str); fflush(stdout);
          if(i < CMV_MAX_COLORS){
            c = &colors[i];
            c->color.red   = r;
            c->color.green = g;
            c->color.blue  = b;
            c->name  = strdup(str);
            c->merge = merge;
	    c->expected_num = exp_num;
            i++;
	  }else{
	    printf("CMVision: Too many colors, ignoring '%s'.\n",str);
	  }
	}else if(n == 0){
          state = CMV_STATE_SCAN;
        }
        break;
      case CMV_STATE_THRESH:
        n = sscanf(buf,"(%d:%d,%d:%d,%d:%d)",&y1,&y2,&u1,&u2,&v1,&v2);
        if(n == 6){
          // printf("(%d:%d,%d:%d,%d:%d)\n",y1,y2,u1,u2,v1,v2);
          if(i < CMV_MAX_COLORS){
            c = &colors[i];
            c->y_low = y1;  c->y_high = y2;
            c->u_low = u1;  c->u_high = u2;
            c->v_low = v1;  c->v_high = v2;

            k = (1 << i);
            set_bits(y_class,CMV_COLOR_LEVELS,y1,y2,k);
            set_bits(u_class,CMV_COLOR_LEVELS,u1,u2,k);
            set_bits(v_class,CMV_COLOR_LEVELS,v1,v2,k);
            i++;
	  }else{
	    printf("CMVision: Too many thresholds.\n");
	  }
	}else if(n == 0){
          state = CMV_STATE_SCAN;
        }
        break;
    }
  }

  /*
  for(i=0; i<CMV_COLOR_LEVELS; i++){
    printf("%08X %08X %08X\n",y_class[i],u_class[i],v_class[i]);
  }
  */

  fclose(in);

  return(true);
}

bool CMVision::saveOptions(char *filename)
{
  color_info *c;
  FILE *out;
  int i;

  out = fopen(filename,"wt");
  if(!out) return(false);

  fprintf(out,"[Colors]\n");
  i = 0;
  while(colors[i].name){
    c = &colors[i];
    fprintf(out,"(%3d,%3d,%3d) %6.4f %d %s\n",
      c->color.red,c->color.green,c->color.blue,
      c->merge,c->expected_num,c->name);
    i++;
  }

  fprintf(out,"\n[Thresholds]\n");
  i = 0;
  while(colors[i].name){
    c = &colors[i];
    fprintf(out,"(%3d:%3d,%3d:%3d,%3d:%3d)\n",
      c->y_low,c->y_high,
      c->u_low,c->u_high,
      c->v_low,c->v_high);
    i++;
  }

  fclose(out);

  return(true);
}

bool CMVision::enable(unsigned opt)
{
  unsigned int valid;

  valid = opt & CMV_VALID_OPTIONS;
  options |= valid;

  return(opt == valid);
}

bool CMVision::disable(unsigned opt)
{
  unsigned int valid;

  valid = opt & CMV_VALID_OPTIONS;
  options &= ~valid;

  return(opt == valid);
}

void CMVision::close()
{
  if(map) delete(map);
  map = NULL;
}


//==== Vision Testing Functions ====================================//

bool CMVision::testClassify(rgb * restrict out,image_pixel * restrict image)
{
  int i,s;
  rgb black = {0,0,0};

  if(!image || !out) return(false);

  classifyFrame(image,map);

  s = width * height;

  i = 0;
  while(i < s){
    while(i<s && !map[i]){
      out[i] = black;
      i++;
    }
    while(i<s && map[i]){
      out[i] = colors[bottom_bit(map[i])-1].color;
      i++;
    }
  }

  return(true);
}

bool CMVision::getThreshold(int color,
       int &y_low,int &y_high,
       int &u_low,int &u_high,
       int &v_low,int &v_high)
{
  color_info *c;

  if(color<0 || color>=CMV_MAX_COLORS) return(false);

  c = &colors[color];
  y_low = c->y_low;  y_high = c->y_high;
  u_low = c->u_low;  u_high = c->u_high;
  v_low = c->v_low;  v_high = c->v_high;

  return(true);
}

bool CMVision::setThreshold(int color,
       int y_low,int y_high,
       int u_low,int u_high,
       int v_low,int v_high)
{
  color_info *c;
  unsigned k;

  if(color<0 || color>=CMV_MAX_COLORS) return(false);

  c = &colors[color];
  k = 1 << color;

  clear_bits(y_class,CMV_COLOR_LEVELS,c->y_low,c->y_high,k);
  clear_bits(u_class,CMV_COLOR_LEVELS,c->u_low,c->u_high,k);
  clear_bits(v_class,CMV_COLOR_LEVELS,c->v_low,c->v_high,k);

  c->y_low = y_low;  c->y_high = y_high;
  c->u_low = u_low;  c->u_high = u_high;
  c->v_low = v_low;  c->v_high = v_high;

  set_bits(y_class,CMV_COLOR_LEVELS,y_low,y_high,k);
  set_bits(u_class,CMV_COLOR_LEVELS,u_low,u_high,k);
  set_bits(v_class,CMV_COLOR_LEVELS,v_low,v_high,k);

  return(true);
}

//==== Main Vision Functions =======================================//

bool CMVision::processFrame(image_pixel *image)
{
  int runs;
  int regions;
  int max_area;

  if(!image) return(false);

  if(options & CMV_THRESHOLD){

    classifyFrame(image,map);
    runs = encodeRuns(rmap,map);
    connectComponents(rmap,runs);

    regions = extractRegions(region_table,rmap,runs);

    if(options & CMV_COLOR_AVERAGES){
      calcAverageColors(region_table,regions,image,rmap,runs);
    }

    max_area = separateRegions(region_table,regions);
    sortRegions(max_area);

    if(options & CMV_DENSITY_MERGE){
      mergeRegions();
    }
  }

  return(true);
}

bool CMVision::processFrame(unsigned *map)
{
  int runs;
  int regions;
  int max_area;

  if(!map) return(false);

  runs = encodeRuns(rmap,map);
  connectComponents(rmap,runs);

  regions = extractRegions(region_table,rmap,runs);

  // if(options & CMV_COLOR_AVERAGES){
  //   calcAverageColors(region_table,regions,image,rmap,runs);
  // }

  max_area = separateRegions(region_table,regions);
  sortRegions(max_area);

  if(options & CMV_DENSITY_MERGE){
    mergeRegions();
  }

  return(true);
}

int CMVision::numRegions(int color_id)
{
  if(color_id<0 || color_id>=CMV_MAX_COLORS) return(CMV_NONE);
  return(region_count[color_id]);
}

CMVision::region *CMVision::getRegions(int color_id)
{
  if(color_id<0 || color_id>=CMV_MAX_COLORS) return(NULL);
  return(region_list[color_id]);
}
