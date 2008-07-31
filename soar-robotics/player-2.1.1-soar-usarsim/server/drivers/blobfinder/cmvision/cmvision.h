/*=========================================================================
    CMVision.h
  -------------------------------------------------------------------------
    API definition for the CMVision real time Color Machine Vision library
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
  =========================================================================*/

#ifndef __CMVISION_H__
#define __CMVISION_H__

// uncomment if your compiler supports the "restrict" keyword
// #define restrict __restrict__
#define restrict

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
Ultra-fast intro to processing steps:
 - Color segmentation
   - load / save
   - set new values
   - process frame
 - Connected Regions
   - RLE
   - merge
   - extract blobs
   - separate blobs
   - sort blobs
   - merge blobs
 - Blob merging (not currently in release)
   - merge by area occupied

Options File Format: (RGB merge name)
[Colors]
(00,00,00) 0.95 'Orange'
(00,00,00) 0.00 'Pink'
(00,00,00) 0.00 'Red'
(00,00,00) 0.00 'DarkBlue'
(00,00,00) 0.00 'Blue'

[Thresholds]
(<lo>:<hi>,<lo>:<hi>,<lo>:<hi>)
(<lo>:<hi>,<lo>:<hi>,<lo>:<hi>)
(<lo>:<hi>,<lo>:<hi>,<lo>:<hi>)
*/

#define CMV_COLOR_LEVELS  256
#define CMV_MAX_COLORS     32

// sets tweaked optimal values for image size
#define CMV_DEFAULT_WIDTH  320
#define CMV_DEFAULT_HEIGHT 240

// values may need tweaked, although these seem to work usually
#define CMV_MAX_RUNS     (CMV_DEFAULT_WIDTH * CMV_DEFAULT_HEIGHT) / 4
#define CMV_MAX_REGIONS  CMV_MAX_RUNS / 4
#define CMV_MIN_AREA     20

#define CMV_NONE ((unsigned)(-1))

#ifndef NULL
#define NULL (0)
#endif

struct yuv{
  unsigned char y,u,v;
};

struct yuv422{
  unsigned char y1,u,y2,v;
};

struct uyvy{
  unsigned char u,y1,v,y2;
};

#ifdef USE_METEOR
  typedef struct uyvy image_pixel;
#else
  typedef struct yuv422 image_pixel;
#endif

#ifndef RGB_STRUCT
#define RGB_STRUCT
struct rgb{
  unsigned char red,green,blue;
};
#endif

// Options for level of processing
//   use enable()/disable() to change 
#define CMV_THRESHOLD      0x01
#define CMV_COLOR_AVERAGES 0x02
#define CMV_DUAL_THRESHOLD 0x04
#define CMV_DENSITY_MERGE  0x08

#define CMV_VALID_OPTIONS  0x0F


class CMVision{
public:
  struct region{
    int color;          // id of the color
    int area;           // occupied area in pixels
    int x1,y1,x2,y2;    // bounding box (x1,y1) - (x2,y2)
    float cen_x,cen_y;  // centroid
    yuv average;        // average color (if CMV_COLOR_AVERAGES enabled)

    int sum_x,sum_y,sum_z; // temporaries for centroid and avg color
    // int area_check; // DEBUG ONLY

    region *next;       // next region in list

    // int number; // DEBUG ONLY
  };

  struct rle{
    unsigned color;     // which color(s) this run represents
    int length;         // the length of the run (in pixels)
    int parent;         // run's parent in the connected components tree
  };

  struct color_info{
    rgb color;          // example color (such as used in test output)
    char *name;         // color's meaninful name (e.g. ball, goal)
    double merge;       // merge density threshold
    int expected_num;   // expected number of regions (used for merge)
    int y_low,y_high;   // Y,U,V component thresholds
    int u_low,u_high;
    int v_low,v_high;
  };

  struct point{
    double x,y;
  };

  struct line{
    point a,b;
  };

  struct rectangle{
    int x,y,w,h;
  };

protected:
  unsigned y_class[CMV_COLOR_LEVELS];
  unsigned u_class[CMV_COLOR_LEVELS];
  unsigned v_class[CMV_COLOR_LEVELS];

  region region_table[CMV_MAX_REGIONS];
  region *region_list[CMV_MAX_COLORS];
  int region_count[CMV_MAX_COLORS];

  rle rmap[CMV_MAX_RUNS];

  color_info colors[CMV_MAX_COLORS];
  int width,height;
  unsigned *map;

  unsigned options;

protected:
// Private functions
  void classifyFrame(image_pixel * restrict img,unsigned * restrict map);
  int  encodeRuns(rle * restrict out,unsigned * restrict map);
  void connectComponents(rle * restrict map,int num);
  int  extractRegions(region * restrict reg,rle * restrict rmap,int num);
  void calcAverageColors(region * restrict reg,int num_reg,
                         image_pixel * restrict img,
                         rle * restrict rmap,int num_runs);
  int  separateRegions(region * restrict reg,int num);
  region *sortRegionListByArea(region * restrict list,int passes);
  void sortRegions(int max_area);

  // density based merging support
  int mergeRegions(region *p,int num,double density_thresh);
  int mergeRegions();

  void clear();

public:
  CMVision()  {clear();}
  ~CMVision() {close();}

  bool initialize(int nwidth,int nheight);
  bool loadOptions(char *filename);
  bool saveOptions(char *filename);
  bool enable(unsigned opt);
  bool disable(unsigned opt);
  void close();

  bool testClassify(rgb * restrict out,image_pixel * restrict image);
  bool getThreshold(int color,
         int &y_low,int &y_high,
	 int &u_low,int &u_high,
         int &v_low,int &v_high);
  bool setThreshold(int color,
         int y_low,int y_high,
	 int u_low,int u_high,
         int v_low,int v_high);

  unsigned *getMap()
    {return(map);}

  char *getColorName(int color)
    {return(colors[color].name);}
  rgb getColorVisual(int color)
    {return(colors[color].color);}

  color_info *getColorInfo(int color)
    {return(&colors[color]);}
  void getColorInfo(int color,color_info &info)
    {info = colors[color];}
  void setColorInfo(int color,color_info &info)
    {colors[color] = info;}

  bool processFrame(image_pixel *image);
  bool processFrame(unsigned *map);
  int numRegions(int color_id);
  region *getRegions(int color_id);
};

#endif
