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
 * $Id: maptransform.cc 4340 2008-02-04 19:40:27Z thjc $
 *
 * Base class for map transform drivers, simply reimplement the transform method
 * with your trasformation function. See MapScale for example
 */

#include "maptransform.h"

// this one has no data or commands, just configs
MapTransform::MapTransform(ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_MAP_CODE)
{
  PLAYER_MSG0(9,"Initialising the MapTransform Driver");
  memset(&source_map,0,sizeof(source_map));
  memset(&new_map,0,sizeof(new_map));

  // Find our source map
  if(cf->ReadDeviceAddr(&source_map_addr, section, "requires",
                        PLAYER_MAP_CODE, -1,NULL) != 0)
  {
    PLAYER_ERROR("must specify source map");
    return;  	
  }
	
  this->source_data = this->new_data = NULL;
}

MapTransform::~MapTransform()
{
}

int
MapTransform::Setup()
{
  if(this->GetMap() < 0)
    return(-1);
  if(this->Transform() < 0)
    return(-1);

  delete [] source_data;
  source_data = NULL;

  return(0);
}

// get the map from the underlying map device
// TODO: should Unsubscribe from the map on error returns in the function
int
MapTransform::GetMap()
{
  Device* mapdev;

  // Subscribe to the map device
  if(!(mapdev = deviceTable->GetDevice(this->source_map_addr)))
  {
    PLAYER_ERROR("unable to locate suitable map device");
    return -1;
  }
  if(mapdev->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to map device");
    return -1;
  }

  // first, get the map info
  Message* msg;
  if(!(msg = mapdev->Request(this->InQueue,
                             PLAYER_MSGTYPE_REQ,
                             PLAYER_MAP_REQ_GET_INFO,
                             NULL, 0, NULL, false)))
  {
    PLAYER_ERROR("failed to get map info");
    return(-1);
  }

  // copy in the map info
  source_map = *(player_map_info_t*)msg->GetPayload();
  
  delete msg;

  // allocate space for map cells
  this->source_data = new char[this->source_map.width * this->source_map.height];
  assert(this->source_data);

  if(!(msg = mapdev->Request(this->InQueue,
                             PLAYER_MSGTYPE_REQ,
                             PLAYER_MAP_REQ_GET_INFO,
                             NULL, 0, NULL, false)))
  {
    PLAYER_ERROR("failed to get map info");
    return(-1);
  }

  // now, get the map data
  player_map_data_t* data_req;
  unsigned int i,j;
  unsigned int oi,oj;
  unsigned int sx,sy;
  unsigned int si,sj;

  data_req = (player_map_data_t*)malloc(sizeof(player_map_data_t));
  assert(data_req);

  // Tile size, limit to sensible default of about 640x640
  sy = sx = 640;
  oi=oj=0;
  while((oi < this->source_map.width) && (oj < this->source_map.height))
  {
    si = MIN(sx, this->source_map.width - oi);
    sj = MIN(sy, this->source_map.height - oj);

    data_req->col = oi;
    data_req->row = oj;
    data_req->width = si;
    data_req->height = sj;
    data_req->data_count = 0;

    if(!(msg = mapdev->Request(this->InQueue,
                               PLAYER_MSGTYPE_REQ,
                               PLAYER_MAP_REQ_GET_DATA,
                               (void*)data_req,0,NULL,false)))
    {
      PLAYER_ERROR("failed to get map info");
      free(data_req);
      delete [] source_data;
      source_data=NULL;
      return(-1);
    }

    player_map_data_t* mapcells = (player_map_data_t*)msg->GetPayload();

    // copy the map data
    for(j=0;j<sj;j++)
    {
      for(i=0;i<si;i++)
      {
        source_data[MAP_IDX(source_map,oi+i,oj+j)] = 
                mapcells->data[j*si + i];
//        mapdata[MAP_IDX(source_map,oi+i,oj+j)].occ_dist = 0;
      }
    }

    delete msg;

    oi += si;
    if(oi >= this->source_map.width)
    {
      oi = 0;
      oj += sj;
    }
  }

  free(data_req);

  // we're done with the map device now
  if(mapdev->Unsubscribe(this->InQueue) != 0)
    PLAYER_WARN("unable to unsubscribe from map device");


  puts("Done.");
  PLAYER_MSG3(4,"MapScale read a %d X %d map, at %.3f m/pix\n",
         this->source_map.width, this->source_map.height, this->source_map.scale);
  return(0);
}

int
MapTransform::Shutdown()
{
  delete [] this->new_data;
  new_data = NULL;
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int MapTransform::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  PLAYER_MSG0(9,"ProcessMessage called for MapTransform Driver");

  assert(hdr);
  assert(data);
 
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_MAP_REQ_GET_INFO, device_addr))
  {
    PLAYER_MSG0(9,"ProcessMessage called for MapTransform Driver: PLAYER_MAP_REQ_GET_INFO");
  	Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_MAP_REQ_GET_INFO, &new_map, sizeof(new_map), NULL);
  	return 0;
  }
  
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_MAP_REQ_GET_DATA, device_addr))
  {
    PLAYER_MSG0(9,"ProcessMessage called for MapTransform Driver: PLAYER_MAP_REQ_GET_DATA");
    assert(new_data);
    player_map_data_t & map_data = *reinterpret_cast<player_map_data_t *> (data);
    player_map_data_t resp_data;
    memcpy(&resp_data, &map_data, sizeof(map_data));

    unsigned int i, j;
    unsigned int oi, oj, si, sj;

    // Construct reply
    oi = map_data.col;
    oj = map_data.row;
    si = map_data.width;
    sj = map_data.height;
    PLAYER_MSG4(9,"Block Requested is: %d,%d + %d,%d",oi,oj,si,sj);
    resp_data.data_count = map_data.width * map_data.height;
    resp_data.data = new int8_t [resp_data.data_count];

    // Grab the pixels from the map
    for(j = 0; j < sj; j++)
    {
      for(i = 0; i < si; i++)
      {
        if(MAP_VALID(new_map, i + oi, j + oj))
          resp_data.data[i + j * si] = this->new_data[MAP_IDX(new_map, i+oi, j+oj)];
        else
        {
          PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
          resp_data.data[i + j * si] = 0;
        }
      }
    }
    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_MAP_REQ_GET_DATA, &resp_data);
    delete [] resp_data.data;
    return 0;
  }

  return -1;
}
