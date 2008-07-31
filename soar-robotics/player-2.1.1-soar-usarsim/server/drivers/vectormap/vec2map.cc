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
 * A driver to convert a vector map to a regular grid map.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_Vec2Map Vec2Map
 * @brief A driver to convert a vector map to a regular grid map.

@par Provides

- @ref interface_map

@par Requires

- @ref interface_vectormap

@par Configuration requests

- None

@par Configuration file options

- cells_per_unit (float)
  - Default: 0.0 (Should be set to something greater than zero!)
  - How many cells are occupied for one vectormap unit (for example 50.0 cells for one meter)
- init_geos (integer)
  - Default: 1
  - If set to 1, GEOS will be initialized by this driver. If you use other driver that uses GEOS (e.g. postgis), you must set it to 0.
- full_extent (integer)
  - Default: 1
  - If set to 1, extent will be computed as if point (0, 0) were in the middle. Warning! It can produce large grid map!
- draw_border (integer)
  - Default: 1
  - If set to 1, border will be drawn around whole map. Border line will be marked as occupied place in the grid.
- skip_feature (string)
  - Default: ""
  - Do not draw feature with that name on final grid map

@par Example 

@verbatim
driver
(
  name "vec2map"
  requires ["vectormap:0"]
  provides ["map:0"]
  cells_per_unit 50.0
  init_geos 0
)
@endverbatim

@par TODO
Try to use BuildWKB() method as in sicknav200 driver instead of GEOS. Note that it should use Linestring instead of Multipoint.

@author Paul Osmialowski

*/

/** @} */

#include <cassert>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <vector>
#include <string>
#include <pthread.h>
#include <libplayercore/playercore.h>
#ifdef HAVE_GEOS
#ifndef GEOS_VERSION
#include <geos_c.h>
#endif
#endif

#define EPS 0.00001
#define MAXFABS(a, b) ((fabs(a) > fabs(b)) ? fabs(a) : fabs(b))

using namespace std;

class Vec2Map : public Driver
{
  public:    
    // Constructor; need that
    Vec2Map(ConfigFile * cf, int section);

    virtual ~Vec2Map();

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();

    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr,
                               void * data);

  private:
    // Main function for device thread.
    virtual void Main();

    // some helper functions
#ifdef HAVE_GEOS
    void dumpFeature(GEOSGeom geom, vector<player_segment_t> & segments);
#endif
    void line(int a, int b, int c, int d, int8_t * cells, int maxx, int maxy);
    int over(int x, int min, int max);

    // The address of the vectormap device to which we will
    // subscribe
    player_devaddr_t vectormap_addr;

    player_devaddr_t map_addr;

    // Handle for the device that have the address given above
    Device * vectormap_dev;

    double cells_per_unit;
    int init_geos;
    int full_extent;
    int draw_border;
    const char * skip_feature;
};

#ifdef HAVE_GEOS
/** Dummy function passed as a function pointer GEOS when it is initialised. GEOS uses this for logging. */
void vec2map_geosprint(const char* format, ...)
{
    va_list ap;
    va_start(ap,format);
    fprintf(stderr,"GEOSError: ");
    vfprintf(stderr,format, ap);
    fflush(stderr);
    va_end(ap);
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
//
// This driver will support the map interface, and it will subscribe
// to the vectormap interface.
//
Vec2Map::Vec2Map(ConfigFile * cf, int section)
//  : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_MAP_CODE)
    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  memset(&(this->vectormap_addr), 0, sizeof(player_devaddr_t));
  memset(&(this->map_addr), 0, sizeof(player_devaddr_t));
  this->skip_feature = NULL;
  this->cells_per_unit = cf->ReadFloat(section, "cells_per_unit", 0.0);
  if ((this->cells_per_unit) <= 0.0)
  {
    PLAYER_ERROR("Invalid cells_per_unit value");
    this->SetError(-1);
    return;
  }
  this->init_geos = cf->ReadInt(section, "init_geos", 1);
  if (this->init_geos)
  {
#ifdef HAVE_GEOS
    PLAYER_WARN("Initializing GEOS");
    initGEOS(vec2map_geosprint, vec2map_geosprint);
#endif
  }
  this->full_extent = cf->ReadInt(section, "full_extent", 1);
  this->draw_border = cf->ReadInt(section, "draw_border", 1);
  this->skip_feature = cf->ReadString(section, "skip_feature", "");
  if (cf->ReadDeviceAddr(&(this->map_addr), section, "provides",
                         PLAYER_MAP_CODE, -1, NULL))
  {
    this->SetError(-1);
    return;
  }
  if (this->AddInterface(this->map_addr))
  {
    this->SetError(-1);
    return;
  }
  if (cf->ReadDeviceAddr(&(this->vectormap_addr), section, "requires",
                         PLAYER_VECTORMAP_CODE, -1, NULL))
  {
    this->SetError(-1);
    return;
  }
}

Vec2Map::~Vec2Map()
{
  if (this->init_geos)
  {
#ifdef HAVE_GEOS
    PLAYER_WARN("Finishing GEOS");
    finishGEOS();
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Vec2Map::Setup()
{  
  // Retrieve the handle to the vectormap device.
  this->vectormap_dev = deviceTable->GetDevice(this->vectormap_addr);
  if (!(this->vectormap_dev))
  {
    PLAYER_ERROR("unable to locate suitable vectormap device");
    return -1;
  }
  // Subscribe my message queue the vectormap device.
  if (this->vectormap_dev->Subscribe(this->InQueue))
  {
    PLAYER_ERROR("unable to subscribe to vectormap device");
    return -1;
  }

  // Start the device thread; spawns a new thread and executes
  // Vec2Map::Main(), which contains the main loop for the driver.
  StartThread();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Vec2Map::Shutdown()
{
  // Stop and join the driver thread
  StopThread();
    
  // Unsubscribe from the vectormap
  this->vectormap_dev->Unsubscribe(this->InQueue);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Vec2Map::Main() 
{
  struct timespec tspec;

  // The main loop
  for(;;)
  {
    // Wait till we get new messages
    this->InQueue->Wait();

    pthread_testcancel();
    
    // Process incoming messages
    ProcessMessages();

    // sleep for a while
    tspec.tv_sec = 0;
    tspec.tv_nsec = 5000;
    nanosleep(&tspec, NULL);
  }
}

#ifdef HAVE_GEOS
void Vec2Map::dumpFeature(GEOSGeom geom, vector<player_segment_t> & segments)
{
    GEOSCoordSeq seq;
    double x0, y0, x1, y1;
    unsigned int numcoords;
    player_segment_t segment;
    int i;

    switch (GEOSGeomTypeId(geom))
    {
    case GEOS_POINT:
	seq = GEOSGeom_getCoordSeq(geom);
	GEOSCoordSeq_getX(seq, 0, &x0);
	GEOSCoordSeq_getY(seq, 0, &y0);
	memset(&segment, 0, sizeof segment);
	segment.x0 = x0;
	segment.y0 = y0;
	segment.x1 = x0;
	segment.y1 = y0;
	segments.push_back(segment);
	break;
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
	seq = GEOSGeom_getCoordSeq(geom);
	if (GEOSCoordSeq_getSize(seq, &numcoords))
	{
	    if (numcoords > 0)
	    {
		GEOSCoordSeq_getX(seq, 0, &x1);
		GEOSCoordSeq_getY(seq, 0, &y1);
		if (numcoords < 2)
		{
		    memset(&segment, 0, sizeof segment);
		    segment.x0 = x1;
		    segment.y0 = y1;
		    segment.x1 = x1;
		    segment.y1 = y1;
		    segments.push_back(segment);
		} else for (i = 0; i < (signed)numcoords; i++)
		{
		    x0 = x1;
		    y0 = y1;
		    GEOSCoordSeq_getX(seq, i, &x1);
		    GEOSCoordSeq_getY(seq, i, &y1);
		    memset(&segment, 0, sizeof segment);
		    segment.x0 = x0;
		    segment.y0 = y0;
		    segment.x1 = x1;
		    segment.y1 = y1;
		    segments.push_back(segment);
		}
	    }
	}
	break;
    case GEOS_POLYGON:
	this->dumpFeature(GEOSGetExteriorRing(geom), segments);
	numcoords = GEOSGetNumInteriorRings(geom);
	for (i = 0; i < (signed)numcoords; i++) this->dumpFeature(GEOSGetInteriorRingN(geom, i), segments);
	break;
    case GEOS_MULTIPOINT:
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION:
	numcoords = GEOSGetNumGeometries(geom);
	for (i = 0; i < (signed)numcoords; i++) this->dumpFeature(GEOSGetGeometryN(geom, i), segments);
	break;
    default:
	PLAYER_WARN("unknown feature type!");
    }
}
#endif

int Vec2Map::over(int x, int min, int max)
{
    if (x<min) return -1;
    if (x>=max) return 1;
    return 0;
}

void Vec2Map::line(int a, int b, int c, int d, int8_t * cells, int width, int height)
{
    double x, y;
    int distX = abs(a - c);
    int distY = abs(b - d);
    double wspX;
    double wspY;

    if (distX > distY)
    {
	if (!distX)
	{
	    wspX = 0.0;
	    wspY = 0.0;
	} else
	{
	    wspX = 1.0;
	    if (!distY) wspY=0.0; else wspY=1.0 / (((double)distX) / ((double)distY));
	}
    } else
    {
	if (!distY)
	{
	    wspX = 0.0;
	    wspY = 0.0;
	} else
	{
	    wspY = 1.0;
	    if (!distX) wspX = 0.0; else wspX = 1.0 / (((double)distY) / ((double)distX));
	}
    }
    if (c < a) wspX = -wspX;
    if (d < b) wspY = -wspY;

    x = (double)a; y = (double)b;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width) x = (width - 1);
    if (y >= height) y = (height - 1);
    cells[(static_cast<int>(y) * width) + (static_cast<int>(x))] = 1;
    if ((fabs(wspX) < EPS) && (fabs(wspY) < EPS)) return;

    while (!((fabs(x - (double)c) < EPS) && (fabs(y - (double)d) < EPS)))
    {
	x += wspX;
	y += wspY;
	if (over(static_cast<int>(x), 0, width)) break;
	if (over(static_cast<int>(y), 0, height)) break;
	cells[(static_cast<int>(y) * width) + (static_cast<int>(x))] = 1;
    } 
}

int Vec2Map::ProcessMessage(QueuePointer & resp_queue, 
                            player_msghdr * hdr,
                            void * data)
{
  player_map_info_t map_info;
  player_map_data_t map_data, map_data_request;
  player_map_data_vector_t map_data_vector;
  player_vectormap_layer_data_t layer, * layer_data;
  const char * layer_name;
  player_vectormap_info_t * vectormap_info;
  Message * msg;
  uint32_t width, height, data_count, layers_count, ii, jj;
  int8_t * cells;
  vector<player_segment_t> segments;
  vector<string> layer_names;
#ifdef HAVE_GEOS
  GEOSGeom geom = NULL;
#endif
  double basex, basey;

  // Process messages here.  Send a response if necessary, using Publish().
  // If you handle the message successfully, return 0.  Otherwise,
  // return -1, and a NACK will be sent for you, if a response is required.

  // Is it new vectormap data?
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                            -1, // -1 means 'all message subtypes'
                            this->vectormap_addr))
  {
    // we don't expect any data
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                            PLAYER_MAP_REQ_GET_INFO,
                            this->map_addr))
  {
    memset(&map_info, 0, sizeof map_info);
    msg = this->vectormap_dev->Request(this->InQueue,
                                       PLAYER_MSGTYPE_REQ,
                                       PLAYER_VECTORMAP_REQ_GET_MAP_INFO,
                                       NULL, 0, NULL, true);
    if (!msg)
    {
      PLAYER_WARN("failed to acquire vectormap info");
      return -1;
    }
    if ((msg->GetDataSize()) < (sizeof(player_vectormap_info_t)))
    {
      PLAYER_WARN2("invalid acqired data size %d vs %d", msg->GetDataSize(), sizeof(player_vectormap_info_t));
      delete msg;
      return -1;
    }
    vectormap_info = reinterpret_cast<player_vectormap_info_t *>(msg->GetPayload());
    if (!vectormap_info)
    {
      PLAYER_WARN("no data acquired");
      delete msg;
      return -1;
    }
    map_info.scale = 1.0 / this->cells_per_unit;
    if (this->full_extent)
    {
      map_info.width = static_cast<uint32_t>((MAXFABS(vectormap_info->extent.x0, vectormap_info->extent.x1) * 2.0) * this->cells_per_unit);
      map_info.height = static_cast<uint32_t>((MAXFABS(vectormap_info->extent.y0, vectormap_info->extent.y1) * 2.0) * this->cells_per_unit);
      map_info.origin.pa = 0.0;
      map_info.origin.px = -MAXFABS(vectormap_info->extent.x0, vectormap_info->extent.x1);
      map_info.origin.py = -MAXFABS(vectormap_info->extent.y0, vectormap_info->extent.y1);
    } else
    {
      map_info.width = static_cast<uint32_t>(fabs((vectormap_info->extent.x1) - (vectormap_info->extent.x0)) * this->cells_per_unit);
      map_info.height = static_cast<uint32_t>(fabs((vectormap_info->extent.y1) - (vectormap_info->extent.y0)) * this->cells_per_unit);
      map_info.origin.pa = 0.0;
      map_info.origin.px = vectormap_info->extent.x0;
      map_info.origin.py = vectormap_info->extent.y0;
    }
    delete msg;
    msg = NULL;
    this->Publish(this->map_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_INFO,
                  (void *)&map_info, sizeof map_info, NULL);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                            PLAYER_MAP_REQ_GET_DATA,
                            this->map_addr))
  {
#ifdef HAVE_GEOS
    memset(&map_data, 0, sizeof map_data);
    if (!data)
    {
      PLAYER_WARN("request incomplete");
      return -1;
    }
    memcpy(&map_data_request, data, sizeof map_data_request);
    msg = this->vectormap_dev->Request(this->InQueue,
                                       PLAYER_MSGTYPE_REQ,
                                       PLAYER_VECTORMAP_REQ_GET_MAP_INFO,
                                       NULL, 0, NULL, true);
    if (!msg)
    {
      PLAYER_WARN("failed to acquire vectormap info");
      return -1;
    }
    if ((msg->GetDataSize()) < (sizeof(player_vectormap_info_t)))
    {
      PLAYER_WARN2("invalid acqired data size %d vs %d", msg->GetDataSize(), sizeof(player_vectormap_info_t));
      delete msg;
      return -1;
    }
    vectormap_info = reinterpret_cast<player_vectormap_info_t *>(msg->GetPayload());
    if (!vectormap_info)
    {
      PLAYER_WARN("no data acquired");
      delete msg;
      return -1;
    }
    if (this->full_extent)
    {
      width = static_cast<uint32_t>((MAXFABS(vectormap_info->extent.x0, vectormap_info->extent.x1) * 2.0) * this->cells_per_unit);
      height = static_cast<uint32_t>((MAXFABS(vectormap_info->extent.y0, vectormap_info->extent.y1) * 2.0) * this->cells_per_unit);
      basex = -MAXFABS(vectormap_info->extent.x0, vectormap_info->extent.x1);
      basey = -MAXFABS(vectormap_info->extent.y0, vectormap_info->extent.y1);
    } else
    {
      width = static_cast<uint32_t>(fabs((vectormap_info->extent.x1) - (vectormap_info->extent.x0)) * this->cells_per_unit);
      height = static_cast<uint32_t>(fabs((vectormap_info->extent.y1) - (vectormap_info->extent.y0)) * this->cells_per_unit);
      basex = vectormap_info->extent.x0;
      basey = vectormap_info->extent.y0;
    }
    data_count = width * height;
    layers_count = vectormap_info->layers_count;
    if (!((data_count > 0) && (layers_count > 0)))
    {
      PLAYER_WARN("Invalid map");
      return -1;
    }
    for (ii = 0; ii < layers_count; ii++)
    {
      layer_names.push_back(string(vectormap_info->layers[ii].name));
    }
    delete msg;
    msg = NULL;
    for (ii = 0; ii < layers_count; ii++)
    {
      memset(&layer, 0, sizeof layer);
      layer_name = layer_names[ii].c_str();
      layer.name_count = strlen(layer_name) + 1;
      layer.name = new char[layer.name_count];
      assert(layer.name);
      strcpy(layer.name, layer_name);
      msg = this->vectormap_dev->Request(this->InQueue,
                                         PLAYER_MSGTYPE_REQ,
                                         PLAYER_VECTORMAP_REQ_GET_LAYER_DATA,
                                         (void *)(&layer), 0, NULL, true);
      delete []layer.name;
      if (!msg)
      {
        PLAYER_WARN("failed to acquire layer data");
        return -1;
      }
      if ((msg->GetDataSize()) < (sizeof(player_vectormap_layer_data_t)))
      {
        PLAYER_WARN2("invalid acqired data size %d vs %d", msg->GetDataSize(), sizeof(player_vectormap_layer_data_t));
        delete msg;
        return -1;
      }
      layer_data = reinterpret_cast<player_vectormap_layer_data_t *>(msg->GetPayload());
      if (!layer_data)
      {
        PLAYER_WARN("no data acquired");
        delete msg;
        return -1;
      }
      for (jj = 0; jj < (layer_data->features_count); jj++)
      {
        if (this->skip_feature)
          if ((strlen(this->skip_feature)) && (layer_data->features[jj].name_count > 0))
            if (!strcmp(this->skip_feature, layer_data->features[jj].name)) continue;
        geom = GEOSGeomFromWKB_buf(layer_data->features[jj].wkb, layer_data->features[jj].wkb_count);
        this->dumpFeature(geom, segments);
        GEOSGeom_destroy(geom);
        geom = NULL;
      }
      delete msg;
      msg = NULL;
    }
    map_data.data = NULL;
    cells = new int8_t[data_count];
    assert(cells);
    memset(cells, -1, data_count);
    if (this->draw_border)
    {
      line(0, 0, width - 1, 0, cells, width, height);
      line(width - 1, 0, width - 1, height - 1, cells, width, height);
      line(width - 1, height - 1, 0, height - 1, cells, width, height);
      line(0, height - 1, 0, 0, cells, width, height);
    }
    for (ii = 0; ii < segments.size(); ii++)
    {
      line(static_cast<int>((segments[ii].x0 - basex) * this->cells_per_unit), static_cast<int>((segments[ii].y0 - basey) * this->cells_per_unit), static_cast<int>((segments[ii].x1 - basex) * this->cells_per_unit), static_cast<int>((segments[ii].y1 - basey) * this->cells_per_unit) , cells, width, height);
    }
    if (map_data_request.col >= width) map_data_request.col = width - 1;
    if (map_data_request.row >= height) map_data_request.row = height - 1;
    if (map_data_request.col + map_data_request.width >= width) map_data_request.width = width - map_data_request.col;
    if (map_data_request.row + map_data_request.height >= height) map_data_request.height = height - map_data_request.row;
    if ((map_data_request.width > 0) && (map_data_request.height > 0))
    {
      map_data.data_count = map_data_request.width * map_data_request.height;
      map_data.data = new int8_t[map_data.data_count];
      for (ii = 0; ii < (map_data_request.height); ii++) memcpy(map_data.data + (ii * (map_data_request.width)), cells + (ii * width) + map_data_request.col, map_data_request.width);
    }

    this->Publish(this->map_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_DATA,
                  (void *)&map_data);
    if (cells) delete []cells;
    if (map_data.data) delete [](map_data.data);
    return 0;
#else
    PLAYER_WARN("GEOS not installed");
    return -1;
#endif
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                            PLAYER_MAP_REQ_GET_VECTOR,
                            this->map_addr))
  {
#ifdef HAVE_GEOS
    memset(&map_data_vector, 0, sizeof map_data_vector);
    msg = this->vectormap_dev->Request(this->InQueue,
                                       PLAYER_MSGTYPE_REQ,
                                       PLAYER_VECTORMAP_REQ_GET_MAP_INFO,
                                       NULL, 0, NULL, true);
    if (!msg)
    {
      PLAYER_WARN("failed to acquire vectormap info");
      return -1;
    }
    if ((msg->GetDataSize()) < (sizeof(player_vectormap_info_t)))
    {
      PLAYER_WARN2("invalid acqired data size %d vs %d", msg->GetDataSize(), sizeof(player_vectormap_info_t));
      delete msg;
      return -1;
    }
    vectormap_info = reinterpret_cast<player_vectormap_info_t *>(msg->GetPayload());
    if (!vectormap_info)
    {
      PLAYER_WARN("no data acquired");
      delete msg;
      return -1;
    }
    layers_count = vectormap_info->layers_count;
    for (ii = 0; ii < layers_count; ii++)
    {
      layer_names.push_back(string(vectormap_info->layers[ii].name));
    }
    map_data_vector.minx = vectormap_info->extent.x0;
    map_data_vector.miny = vectormap_info->extent.y0;
    map_data_vector.maxx = vectormap_info->extent.x1;
    map_data_vector.maxy = vectormap_info->extent.y1;
    delete msg;
    msg = NULL;
    for (ii = 0; ii < layers_count; ii++)
    {
      memset(&layer, 0, sizeof layer);
      layer_name = layer_names[ii].c_str();
      layer.name_count = strlen(layer_name) + 1;
      layer.name = new char[layer.name_count];
      assert(layer.name);
      strcpy(layer.name, layer_name);
      msg = this->vectormap_dev->Request(this->InQueue,
                                         PLAYER_MSGTYPE_REQ,
                                         PLAYER_VECTORMAP_REQ_GET_LAYER_DATA,
                                         (void *)(&layer), 0, NULL, true);
      delete []layer.name;
      if (!msg)
      {
        PLAYER_WARN("failed to acquire layer data");
        return -1;
      }
      if ((msg->GetDataSize()) < (sizeof(player_vectormap_layer_data_t)))
      {
        PLAYER_WARN2("invalid acqired data size %d vs %d", msg->GetDataSize(), sizeof(player_vectormap_layer_data_t));
        delete msg;
        return -1;
      }
      layer_data = reinterpret_cast<player_vectormap_layer_data_t *>(msg->GetPayload());
      if (!layer_data)
      {
        PLAYER_WARN("no data acquired");
        delete msg;
        return -1;
      }
      for (jj = 0; jj < (layer_data->features_count); jj++)
      {
        if (this->skip_feature)
          if ((strlen(this->skip_feature)) && (layer_data->features[jj].name_count > 0))
            if (!strcmp(this->skip_feature, layer_data->features[jj].name)) continue;
        geom = GEOSGeomFromWKB_buf(layer_data->features[jj].wkb, layer_data->features[jj].wkb_count);
        this->dumpFeature(geom, segments);
        GEOSGeom_destroy(geom);
        geom = NULL;
      }
      delete msg;
      msg = NULL;
    }
    map_data_vector.segments = NULL;
    map_data_vector.segments_count = segments.size();
    if ((map_data_vector.segments_count) > 0)
    {
      map_data_vector.segments = new player_segment_t[map_data_vector.segments_count];
      assert(map_data_vector.segments);
      for (ii = 0; ii < map_data_vector.segments_count; ii++)
      {
        map_data_vector.segments[ii] = segments[ii];
      }
    }

    this->Publish(this->map_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_MAP_REQ_GET_VECTOR,
                  (void *)&map_data_vector);
    if (map_data_vector.segments) delete [](map_data_vector.segments);
    return 0;
#else
    PLAYER_WARN("GEOS not installed");
    return -1;
#endif
  }
  return -1;
}

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver * Vec2Map_Init(ConfigFile * cf, int section)
{
  // Create and return a new instance of this driver
  return (Driver *)(new Vec2Map(cf, section));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void Vec2Map_Register(DriverTable * table)
{
  table->AddDriver("vec2map", Vec2Map_Init);
}
