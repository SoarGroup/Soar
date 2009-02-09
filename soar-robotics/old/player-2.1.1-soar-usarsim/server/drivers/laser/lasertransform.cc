/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
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


#include "lasertransform.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserTransform::LaserTransform( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_LASER_CODE)
{
  // Must have an input laser
  if (cf->ReadDeviceAddr(&this->laser_addr, section, "requires",
                         PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->laser_device = NULL;
  this->laser_timestamp.tv_sec = 0;
  this->laser_timestamp.tv_usec = 0;

  // Outgoing data
  this->time.tv_sec = 0;
  this->time.tv_usec = 0;
  memset(&this->data, 0, sizeof(this->data));

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int LaserTransform::Setup()
{
  // Subscribe to the laser.
  if(Device::MatchDeviceAddress(this->laser_addr, this->device_addr))
  {
    PLAYER_ERROR("attempt to subscribe to self");
    return(-1);
  }
  if(!(this->laser_device = deviceTable->GetDevice(this->laser_addr)))
  {
    PLAYER_ERROR("unable to locate suitable laser device");
    return(-1);
  }
  if(this->laser_device->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return(-1);
  }
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int LaserTransform::Shutdown()
{
  // Unsubscribe from devices.
  this->laser_device->Unsubscribe(this->InQueue);
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserTransform::ProcessMessage(QueuePointer & resp_queue, 
                                player_msghdr * hdr, 
                                void * data)
{
  // Handle new data from the laser
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, 
                           this->laser_addr))
  {
    assert(hdr->size != 0);
    player_laser_data_t * l_data = reinterpret_cast<player_laser_data_t * > (data);
    this->UpdateLaser(l_data);
    return(0);
  }
  // Forward any request to the laser
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, this->device_addr))
  {
    // Forward the message
    laser_device->PutMsg(this->InQueue, hdr, data);
    // Store the return address for later use
    this->ret_queue = resp_queue;
    // Set the message filter to look for the response
    this->InQueue->SetFilter(this->laser_addr.host,
                             this->laser_addr.robot,
                             this->laser_addr.interf,
                             this->laser_addr.index,
                             -1,
                             hdr->subtype);
    // No response now; it will come later after we hear back from the
    // laser
    return(0);
  }
  // Forward response (success or failure) from the laser
  else if((Message::MatchMessage(hdr, PLAYER_MSGTYPE_RESP_ACK, 
                            -1, this->laser_addr)) ||
     (Message::MatchMessage(hdr, PLAYER_MSGTYPE_RESP_NACK,
                            -1, this->laser_addr)))
  {
    // Copy in our address and forward the response
    hdr->addr = this->device_addr;
    this->Publish(this->ret_queue, hdr, data);
    // Clear the filter
    this->InQueue->ClearFilter();

    return(0);
  }

  return(-1);
}
