/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
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
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/*
 * $Id: device.cc 6543 2008-06-11 20:49:48Z thjc $
 *
 * A device entry describes an instantiated driver/interface
 * combination.  Drivers may support more than one interface,
 * and hence appear more than once in the devicetable.
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <netinet/in.h>

#include <libplayercore/driver.h>
#include <libplayercore/device.h>
#include <libplayercore/message.h>
#include <libplayercore/error.h>
#include <libplayercore/playertime.h>
#include <libplayercore/devicetable.h>
#include <libplayercore/globals.h>
#include <libplayercore/error.h>
#include <libplayercore/interface_util.h>

// Constructor
Device::Device(player_devaddr_t addr, Driver *device)
{
  this->next = NULL;

  this->addr = addr;
  this->driver = device;

  memset(this->drivername, 0, sizeof(this->drivername));

  if(this->driver)
  {
    this->driver->entries++;
    this->driver->device_addr = addr;
    this->InQueue = this->driver->InQueue;
  }
  else
  {
    this->InQueue = QueuePointer(false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN);
  }

  // Start with just a couple of entries; we'll double the size as
  // necessary in the future.
  this->len_queues = 2;
  this->queues = (QueuePointer*)calloc(this->len_queues,
                                        sizeof(QueuePointer));
  assert(this->queues);
}


Device::~Device()
{
  if(this->driver)
  {
    this->driver->entries--;
    if(this->driver->entries == 0)
      delete this->driver;
  }
  free(this->queues);
}

int
Device::Subscribe(QueuePointer &sub_queue)
{
  int retval;
  size_t i;

  if(this->driver)
    this->driver->Lock();

  // find an empty spot to put the new queue
  for(i=0;i<this->len_queues;i++)
  {
    if(this->queues[i] == NULL)
      break;
  }

  // do we need to make room?
  if(i == this->len_queues)
  {
    size_t tmp = this->len_queues;
    this->len_queues *= 2;
    this->queues = (QueuePointer*)realloc(this->queues,
                                           (this->len_queues *
                                            sizeof(QueuePointer)));
    memset(this->queues+tmp,0,(this->len_queues-tmp)*sizeof(QueuePointer));
    assert(this->queues);
  }

  // add the subscriber's queue to the list
  this->queues[i] = sub_queue;

  if(this->driver)
  {
    // first try the new version which passes the queue in
    retval = this->driver->Subscribe(sub_queue, this->addr);
    if (retval < 0)
    {
      // remove the subscriber's queue, since the subscription failed
      this->queues[i] = QueuePointer();
      this->driver->Unlock();
      return(retval);
    }
    else if(retval == 1 && (retval = this->driver->Subscribe(this->addr)))
    {
      // remove the subscriber's queue, since the subscription failed
      this->queues[i] = QueuePointer();
      this->driver->Unlock();
      return(retval);
    }

    this->driver->Unlock();
  }

  return(0);
}

int
Device::Unsubscribe(QueuePointer &sub_queue)
{
  int retval;

  if(this->driver)
  {
    this->driver->Lock();

    // first try the new version which passes the queue in
    retval = this->driver->Unsubscribe(sub_queue, this->addr);
    if (retval < 0)
    {
      // remove the subscriber's queue, since the subscription failed
      this->driver->Unlock();
      return(retval);
    }
    else if(retval == 1 && (retval = this->driver->Unsubscribe(this->addr)))
    {
      this->driver->Unlock();
      return(retval);
    }
  }
  // look for the given queue
  for(size_t i=0;i<this->len_queues;i++)
  {
    if(this->queues[i] == sub_queue)
    {
      this->queues[i] = QueuePointer();
      if(this->driver)
        this->driver->Unlock();
      return(0);
    }
  }
  //PLAYER_ERROR("tried to unsubscribed not-subscribed queue");
  if(this->driver)
    this->driver->Unlock();
  return(-1);
}

void
Device::PutMsg(QueuePointer &resp_queue,
               player_msghdr_t* hdr,
               void* src,
               bool copy)
{
  hdr->addr = this->addr;
  Message msg(*hdr,src,resp_queue,copy);
  // don't need to lock here, because the queue does its own locking in Push
  if(!this->InQueue->Push(msg))
  {
    PLAYER_ERROR4("tried to push %s/%d from/onto %s/%d\n",
                  msgtype_to_str(hdr->type), hdr->subtype,
                  interf_to_str(hdr->addr.interf), hdr->addr.index);
  }
}


void
Device::PutMsg(QueuePointer &resp_queue,
               uint8_t type,
               uint8_t subtype,
               void* src,
               size_t deprecated,
               double* timestamp)
{
  struct timeval ts;
  double t;
  player_msghdr_t hdr;


  // Fill in the current time if not supplied
  if(timestamp)
    t = *timestamp;
  else
  {
    GlobalTime->GetTime(&ts);
    t = ts.tv_sec + ts.tv_usec/1e6;
  }

  memset(&hdr,0,sizeof(player_msghdr_t));
  //hdr.stx = PLAYER_STXX;
  hdr.type = type;
  hdr.subtype = subtype;
  hdr.timestamp = t;
  //hdr.size = len;

  this->PutMsg(resp_queue, &hdr, src);
}

Message*
Device::Request(QueuePointer &resp_queue,
                uint8_t type,
                uint8_t subtype,
                void* src,
                size_t deprecated,
                double* timestamp,
                bool threaded)
{
  // Send the request message
  this->PutMsg(resp_queue,
               type, subtype,
               src, 0, timestamp);

  // Set the message filter to look for the response
  resp_queue->SetFilter(this->addr.host,
                        this->addr.robot,
                        this->addr.interf,
                        this->addr.index,
                        -1,
                        subtype);
  // Await the reply

  // Ideally, we'd block here, but that won't work if we're making a
  // request of a non-threaded driver, and we are: (a) also non-threaded
  // or (b) making this request from within our Setup.  In either case,
  // the server thread would end up waiting on itself, which won't work.
  //
  // So....instead we'll do the following hack: update all drivers
  // except the requester (if we were to update the requester, and he's
  // non-threaded, then his ProcessMessage() would get called
  // recursively).

  Message* msg;
  if(threaded)
  {
    resp_queue->Wait();
    // HACK: this loop should not be neccesary!
    // pthread_cond_wait does not garuntee no false wake up, so maybe it is.
    while(!(msg = resp_queue->Pop()))
    {
      PLAYER_WARN("empty queue after waiting!");
      resp_queue->Wait(); // this is a cancelation point
    }
  }
  else
  {
    for(;;)
    {
      // We don't lock here, on the assumption that the caller is also the only
      // thread that can make changes to the device table.
      for(Device* dev = deviceTable->GetFirstDevice();
          dev;
          dev = deviceTable->GetNextDevice(dev))
      {
        Driver* dri = dev->driver;
        // don't update the requester
        //if(dri->InQueue == resp_queue)
        if(!dri || dev->InQueue == resp_queue)
          continue;
        if(!dri->driverthread && ((dri->subscriptions > 0) || dri->alwayson))
          dri->Update();
      }

      if((msg = resp_queue->Pop()))
        break;
    }
  }

  assert(msg);

  player_msghdr_t* hdr;
  hdr = msg->GetHeader();
  if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_RESP_ACK,
                           subtype, this->addr))
  {
    // got an ACK
    resp_queue->ClearFilter();
    return(msg);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_RESP_NACK,
                                subtype, this->addr))
  {
    // got a NACK
    resp_queue->ClearFilter();
    delete msg;
    return(NULL);
  }
  else
  {
    // got something else
    resp_queue->ClearFilter();
    printf("%d:%d:%d:%d\n",
           hdr->addr.interf, hdr->addr.index,
           hdr->type, hdr->subtype);
    PLAYER_ERROR("got unexpected message");
    delete msg;
    return(NULL);
  }
}

