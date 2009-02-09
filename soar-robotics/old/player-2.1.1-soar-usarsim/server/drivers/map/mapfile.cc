/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004  Brian Gerkey gerkey@stanford.edu    
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * $Id: mapfile.cc 4232 2007-11-01 22:16:23Z gerkey $
 *
 * A driver to read an occupancy grid map from an image file.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_mapfile mapfile
 * @brief Read grid maps from image files

The mapfile driver reads a occupancy grid map from a bitmap image file and
provides the map to others via the @ref interface_map interface.
Since gdk-pixbuf is used to load the file, pretty much all bitmap formats
are supported.

Each cell in an occupancy grid map takes 1 of 3 states: occupied (1),
unknown (0), and free (-1).  The mapfile driver converts each pixel of an
image to a cell with one of these states in the following way: average
the color values; divide this average by max value to get a ratio; if
this ratio is greater than .95, the cell is occupied; if ratio is less
than 0.1, the cell is free; otherwise it is unknown.  In other words,
"blacker" pixels are occupied, "whiter" pixels are free, and those in
between are unknown.

Note that @ref interface_map devices produce no data; the map is
delivered via a sequence of configuration requests.

@par Compile-time dependencies

- gdk-pixbuf-2.0 (usually installed as part of GTK)

@par Provides

- @ref interface_map

@par Requires

- None

@par Configuration requests

- PLAYER_MAP_REQ_GET_INFO
- PLAYER_MAP_REQ_GET_DATA

@par Configuration file options

- filename (string)
  - Default: NULL
  - The image file to read.
- resolution (length)
  - Default: -1.0
  - Resolution (length per pixel) of the image.
- negate (integer)
  - Default: 0
  - Should we negate (i.e., invert) the colors in the image before
    reading it?  Useful if you're using the same image file as the
    world bitmap for Stage 1.3.x, which has the opposite semantics for
    free/occupied pixels.
- origin ([length length angle] tuple)
  - Default: [-width*resolution/2 -height*resolution/2 0]
  - The real-world coordinates of the lower-left pixel in the image.  
    The default puts (0,0,0) in the middle of the image.  The angle is 
    currently ignored.
 
@par Example 

@verbatim
driver
(
  name "mapfile"
  provides ["map:0"]
  filename "mymap.pgm"
  resolution 0.1  # 10cm per pixel
)
@endverbatim

@author Brian Gerkey

*/
/** @} */

#include <sys/types.h> // required by Darwin
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

// use gdk-pixbuf for image loading
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

// compute linear index for given map coords
#define MAP_IDX(mf, i, j) ((mf->size_x) * (j) + (i))

// check that given coords are valid (i.e., on the map)
#define MAP_VALID(mf, i, j) ((i >= 0) && (i < mf->size_x) && (j >= 0) && (j < mf->size_y))


extern int global_playerport;

class MapFile : public Driver
{
  private:
    const char* filename;
    double resolution;
    int negate;
    int size_x, size_y;
    player_pose2d_t origin;
    char* mapdata;
    
    // Handle map info request
    void HandleGetMapInfo(void *client, void *request, int len);
    // Handle map data request
    void HandleGetMapData(void *client, void *request, int len);

  public:
    MapFile(ConfigFile* cf, int section, const char* file, 
            double res, int neg, player_pose2d_t o);
    ~MapFile();
    int Setup();
    int Shutdown();

    // MessageHandler
    int ProcessMessage(QueuePointer & resp_queue, 
		       player_msghdr * hdr, 
		       void * data);

};

Driver*
MapFile_Init(ConfigFile* cf, int section)
{
  const char* filename;
  double resolution;
  int negate;
  player_pose2d_t origin;

  if(!(filename = cf->ReadFilename(section,"filename", NULL)))
  {
    PLAYER_ERROR("must specify map filename");
    return(NULL);
  }
  if((resolution = cf->ReadLength(section,"resolution",-1.0)) < 0)
  {
    PLAYER_ERROR("must specify positive map resolution");
    return(NULL);
  }
  negate = cf->ReadInt(section,"negate",0);
  origin.px = cf->ReadTupleLength(section,"origin",0,FLT_MAX);
  origin.py = cf->ReadTupleLength(section,"origin",1,FLT_MAX);
  //origin.pa = cf->ReadTupleAngle(section,"origin",2,FLT_MAX);
  origin.pa = 0.0;

  return((Driver*)(new MapFile(cf, section, filename, 
                               resolution, negate, origin)));
}

// a driver registration function
void 
MapFile_Register(DriverTable* table)
{
  table->AddDriver("mapfile", MapFile_Init);
}


// this one has no data or commands, just configs
MapFile::MapFile(ConfigFile* cf, int section, const char* file, 
                 double res, int neg, player_pose2d_t o) 
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_MAP_CODE)
{
  this->mapdata = NULL;
  this->size_x = this->size_y = 0;
  this->filename = file;
  this->resolution = res;
  this->negate = neg;
  this->origin = o;
}

MapFile::~MapFile()
{
}

int
MapFile::Setup()
{
  GdkPixbuf* pixbuf;
  guchar* pixels;
  guchar* p;
  int rowstride, n_channels, bps;
  GError* error = NULL;
  int i,j,k;
  double occ;
  int color_sum;
  double color_avg;

  // Initialize glib
  g_type_init();

  printf("MapFile loading image file: %s...", this->filename);
  fflush(stdout);

  // Read the image
  if(!(pixbuf = gdk_pixbuf_new_from_file(this->filename, &error)))
  {
    PLAYER_ERROR1("failed to open image file %s", this->filename);
    return(-1);
  }

  this->size_x = gdk_pixbuf_get_width(pixbuf);
  this->size_y = gdk_pixbuf_get_height(pixbuf);

  assert(this->mapdata = (char*)malloc(sizeof(char) *
                                       this->size_x * this->size_y));

  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  bps = gdk_pixbuf_get_bits_per_sample(pixbuf)/8;
  n_channels = gdk_pixbuf_get_n_channels(pixbuf);
  if(gdk_pixbuf_get_has_alpha(pixbuf))
    n_channels++;

  // Read data
  pixels = gdk_pixbuf_get_pixels(pixbuf);
  for(j = 0; j < this->size_y; j++)
  {
    for (i = 0; i < this->size_x; i++)
    {
      p = pixels + j*rowstride + i*n_channels*bps;
      color_sum = 0;
      for(k=0;k<n_channels;k++)
        color_sum += *(p + (k * bps));
      color_avg = color_sum / (double)n_channels;

      if(this->negate)
        occ = color_avg / 255.0;
      else
        occ = (255 - color_avg) / 255.0;
      if(occ > 0.95)
        this->mapdata[MAP_IDX(this,i,this->size_y - j - 1)] = +1;
      else if(occ < 0.1)
        this->mapdata[MAP_IDX(this,i,this->size_y - j - 1)] = -1;
      else
        this->mapdata[MAP_IDX(this,i,this->size_y - j - 1)] = 0;
    }
  }

  gdk_pixbuf_unref(pixbuf);

  puts("Done.");
  printf("MapFile read a %d X %d map, at %.3f m/pix\n",
         this->size_x, this->size_y, this->resolution);
  return(0);
}

int
MapFile::Shutdown()
{
  free(this->mapdata);
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int MapFile::ProcessMessage(QueuePointer & resp_queue, 
                            player_msghdr * hdr, 
                            void * data)
{
  // Is it a request for map meta-data?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_MAP_REQ_GET_INFO, 
                           this->device_addr))
  {
    player_map_info_t info;
    info.scale = this->resolution;
    info.width = this->size_x;
    info.height = this->size_y;
    // Did the user specify an origin?
    if(this->origin.px == FLT_MAX)
    {
      info.origin.px = -(this->size_x / 2.0) * this->resolution;
      info.origin.py = -(this->size_y / 2.0) * this->resolution;
      info.origin.pa = 0.0;
    }
    else
      info.origin = this->origin;

    this->Publish(this->device_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_INFO,
                  (void*)&info, sizeof(info), NULL);
    return(0);
  }
  // Is it a request for a map tile?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_MAP_REQ_GET_DATA,
                           this->device_addr))
  {
    player_map_data_t* mapreq = (player_map_data_t*)data;

    // Can't declare a map tile on the stack (it's too big)
    /*
    size_t mapsize = (sizeof(player_map_data_t) - PLAYER_MAP_MAX_TILE_SIZE + 
                      (mapreq->width * mapreq->height));
                      */
    size_t mapsize = sizeof(player_map_data_t);
    player_map_data_t* mapresp = (player_map_data_t*)calloc(1,mapsize);
    assert(mapresp);
    
    int i, j;
    int oi, oj, si, sj;

    // Construct reply
    oi = mapresp->col = mapreq->col;
    oj = mapresp->row = mapreq->row;
    si = mapresp->width = mapreq->width;
    sj = mapresp->height = mapreq->height;
    mapresp->data_count = mapresp->width * mapresp->height;
    mapresp->data = new int8_t [mapresp->data_count];
    // Grab the pixels from the map
    for(j = 0; j < sj; j++)
    {
      for(i = 0; i < si; i++)
      {
        if(MAP_VALID(this, i + oi, j + oj))
          mapresp->data[i + j * si] = this->mapdata[MAP_IDX(this, i+oi, j+oj)];
        else
        {
          PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
          mapresp->data[i + j * si] = 0;
        }
      }
    }

    this->Publish(this->device_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_DATA,
                  (void*)mapresp);
    delete [] mapresp->data;
    free(mapresp);
    return(0);
  }
  return(-1);
}

