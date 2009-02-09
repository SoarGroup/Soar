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
 * $Id: vmapfile.cc 4232 2007-11-01 22:16:23Z gerkey $
 *
 * A driver to read a vector map from a text file
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_vmapfile vmapfile
 * @brief Read vector maps from text files

The vmapfile driver reads a vector map from a text file and
provides the map to others via the @ref interface_map interface.

The format of the text file is...

@par Compile-time dependencies

- None

@par Provides

- @ref interface_map

@par Requires

- None

@par Configuration requests

- PLAYER_MAP_REQ_GET_VECTOR

@par Configuration file options

- filename (string)
  - Default: NULL
  - The file to read.
 
@par Example 

@verbatim
driver
(
  name "vmapfile"
  provides ["map:0"]
  filename "mymap.wld"
)
@endverbatim

@author Brian Gerkey

*/
/** @} */

#include <sys/types.h> // required by Darwin
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libplayercore/playercore.h>

class VMapFile : public Driver
{
  private:
    const char* filename;
    player_map_data_vector_t* vmap;
    size_t vmapsize;
    
    // Handle map data request
    void HandleGetMapVector(void *client, void *request, int len);

  public:
    VMapFile(ConfigFile* cf, int section, const char* file);
    ~VMapFile();
    int Setup();
    int Shutdown();

    // MessageHandler
    int ProcessMessage(QueuePointer & resp_queue, 
		       player_msghdr * hdr, 
		       void * data);

};

Driver*
VMapFile_Init(ConfigFile* cf, int section)
{
  const char* filename;

  if(!(filename = cf->ReadFilename(section,"filename", NULL)))
  {
    PLAYER_ERROR("must specify map filename");
    return(NULL);
  }
  return((Driver*)(new VMapFile(cf, section, filename)));
}

// a driver registration function
void 
VMapFile_Register(DriverTable* table)
{
  table->AddDriver("vmapfile", VMapFile_Init);
}


// this one has no data or commands, just configs
VMapFile::VMapFile(ConfigFile* cf, int section, const char* file)
  : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_MAP_CODE)
{
  this->vmap = NULL;
  this->filename = file;
}

VMapFile::~VMapFile()
{
}

int
VMapFile::Setup()
{
  FILE* fp;
  int ox, oy, w, h;
  int x0,y0,x1,y1;
  char linebuf[512];
  char keyword [512];
  int got_origin, got_width, got_height;

  printf("VMapFile loading file: %s...", this->filename);
  fflush(stdout);

  if(!(fp = fopen(this->filename, "r")))
  {
    PLAYER_ERROR1("failed to open file %s", this->filename);
    return(-1);
  }

  // Allocate space for the biggest possible vector map; we'll realloc
  // later
  this->vmap = 
          (player_map_data_vector_t*)malloc(sizeof(player_map_data_vector_t));
  assert(this->vmap);

  this->vmap->segments_count = 0;
  this->vmap->segments = NULL;
  got_origin = got_width = got_height = 0;
  while(!feof(fp))
  {
    if(!fgets(linebuf, sizeof(linebuf), fp))
      break;
    if(!strlen(linebuf) || (linebuf[0] == '#'))
      continue;

    if(sscanf(linebuf,"%s",keyword) == 1)
    {
      if(!strcmp(keyword, "origin"))
      {
        if(sscanf(linebuf,"%s %d %d", keyword, &ox, &oy) == 3)
          got_origin = 1;
        else
          PLAYER_WARN1("invalid line:%s:",linebuf);
        continue;
      }
      else if(!strcmp(keyword, "width"))
      {
        if(sscanf(linebuf,"%s %d", keyword, &w) == 2)
          got_width = 1;
        else
          PLAYER_WARN1("invalid line:%s:",linebuf);
        continue;
      }
      else if(!strcmp(keyword, "height"))
      {
        if(sscanf(linebuf,"%s %d", keyword, &h) == 2)
          got_height = 1;
        else
          PLAYER_WARN1("invalid line:%s:",linebuf);
        continue;
      }
    }

    if(sscanf(linebuf, "%d %d %d %d", &x0, &y0, &x1, &y1) == 4)
    {
      this->vmap->segments = (player_segment_t*) realloc(
        this->vmap->segments, 
        (this->vmap->segments_count+1)*sizeof(this->vmap->segments[0])
      );
      this->vmap->segments[this->vmap->segments_count].x0 = x0/1e3;
      this->vmap->segments[this->vmap->segments_count].y0 = y0/1e3;
      this->vmap->segments[this->vmap->segments_count].x1 = x1/1e3;
      this->vmap->segments[this->vmap->segments_count].y1 = y1/1e3;
      this->vmap->segments_count++;
    }
    else
      PLAYER_WARN1("ignoring line:%s:", linebuf);
  }

  if(!got_origin || !got_width || !got_height)
  {
    PLAYER_ERROR("file is missing meta-data");
    return(-1);
  }

  this->vmap->minx = ox/1e3;
  this->vmap->miny = oy/1e3;
  this->vmap->maxx = (w + ox)/1e3;
  this->vmap->maxy = (h + oy)/1e3;

  assert(this->vmap);

  puts("Done.");
  printf("VMapFile read a %d-segment map\n", this->vmap->segments_count);
  return(0);
}

int
VMapFile::Shutdown()
{
  free(this->vmap->segments);
  free(this->vmap);
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int VMapFile::ProcessMessage(QueuePointer & resp_queue, 
                             player_msghdr * hdr, 
                             void * data)
{
  // Is it a request for the map?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_MAP_REQ_GET_VECTOR,
                           this->device_addr))
  {
    // Give it the map.
    this->Publish(this->device_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_VECTOR,
                  (void*)this->vmap);
    return(0);
  }
  else
    return(-1);
}

