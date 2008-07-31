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
 * $Id: driver.cc 6501 2008-06-10 05:16:08Z thjc $
 *
 *  the base class from which all device classes inherit.  here
 *  we implement some generic methods that most devices will not need
 *  to override
 */
#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdlib.h>
#include <device.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <assert.h>

#include <libplayercore/playertime.h>
#include <libplayercore/driver.h>
#include <libplayercore/error.h>
#include <libplayercore/devicetable.h>
#include <libplayercore/configfile.h>
#include <libplayercore/globals.h>
#include <libplayercore/filewatcher.h>
#include <libplayercore/property.h>
#include <libplayercore/interface_util.h>

// Default constructor for single-interface drivers.  Specify the
// interface code and buffer sizes.
Driver::Driver(ConfigFile *cf, int section,
               bool overwrite_cmds, size_t queue_maxlen,
               int interf) : InQueue(overwrite_cmds, queue_maxlen)
{
  this->error = 0;
  this->driverthread = 0;

  // Look for our default device id
  if(cf->ReadDeviceAddr(&this->device_addr, section, "provides",
                        interf, -1, NULL) != 0)
  {
    PLAYER_ERROR1("Failed to find provides block for interface %d",interf);
    this->SetError(-1);
    return;
  }

  this->subscriptions = 0;
  this->entries = 0;
  this->alwayson = false;

  // Create an interface
  if(this->AddInterface(this->device_addr) != 0)
  {
    this->SetError(-1);
    return;
  }

  pthread_mutex_init(&this->accessMutex,NULL);
}

// this is the other constructor, used by multi-interface drivers.
Driver::Driver(ConfigFile *cf, int section,
               bool overwrite_cmds, size_t queue_maxlen) : InQueue(overwrite_cmds, queue_maxlen)
{
  this->error = 0;
  this->driverthread = 0;

  this->device_addr.interf = 0xFFFF;

  this->subscriptions = 0;
  this->alwayson = false;
  this->entries = 0;

  pthread_mutex_init(&this->accessMutex,NULL);
}

// destructor, to free up allocated queue.
Driver::~Driver()
{
}

// Add an interface
int
Driver::AddInterface(player_devaddr_t addr)
{
  // Add ourself to the device table
  if(deviceTable->AddDevice(addr, this) == NULL)
  {
    PLAYER_ERROR("failed to add interface");
    return -1;
  }
  return 0;
}

int
Driver::AddInterface(player_devaddr_t *addr, ConfigFile * cf, int section, int code, char * key)
{
  assert(addr);
  // Create position interface
  if (cf->ReadDeviceAddr(addr, section,"provides", code, -1, key) != 0)
  {
    if (key)
      PLAYER_ERROR2("Could not load interface address. %s:*:*:%s:*",key,interf_to_str(code));
    else
      PLAYER_ERROR1("Could not load interface address. %s:*",interf_to_str(code));
    return -1;
  }
  return this->AddInterface(*addr);
}

void
Driver::Publish(QueuePointer &queue,
                player_msghdr_t* hdr,
                void* src, bool copy)
{
  Message msg(*hdr,src,copy);
  // push onto the given queue, which provides its own locking
  if(!queue->Push(msg))
  {
    PLAYER_ERROR4("tried to push %d/%d from %d:%d",
                  hdr->type, hdr->subtype,
                  hdr->addr.interf, hdr->addr.index);
  }
}

void
Driver::Publish(player_msghdr_t* hdr,
                void* src, bool copy)
{
  Device* dev;

  // lock here, because we're accessing our device's queue list
  this->Lock();
  // push onto each queue subscribed to the given device
  if(!(dev = deviceTable->GetDevice(hdr->addr,false)))
  {
    // This is generally ok, because a driver might call Publish on all
    // of its possible interfaces, even though some have not been
    // requested.
    //
    //PLAYER_ERROR2("tried to publish message via non-existent device %d:%d", hdr->addr.interf, hdr->addr.index);
    this->Unlock();
    return;
  }
  Message msg(*hdr,src,copy);
  for(size_t i=0;i<dev->len_queues;i++)
  {
    if(dev->queues[i] != NULL)
    {
      if(!dev->queues[i]->Push(msg))
      {
        PLAYER_ERROR4("tried to push %d/%d from %d:%d",
                      hdr->type, hdr->subtype,
                      hdr->addr.interf, hdr->addr.index);
      }
    }
  }
  this->Unlock();
}

void
Driver::Publish(player_devaddr_t addr,
                QueuePointer &queue,
                uint8_t type,
                uint8_t subtype,
                void* src,
                size_t deprecated,
                double* timestamp,
                bool copy)
{
  double t;

  // Fill in the time structure if not supplied
  if(timestamp)
    t = *timestamp;
  else
    GlobalTime->GetTimeDouble(&t);

  player_msghdr_t hdr;
  memset(&hdr,0,sizeof(player_msghdr_t));
  hdr.addr = addr;
  hdr.type = type;
  hdr.subtype = subtype;
  hdr.timestamp = t;
  hdr.size = 0;

  this->Publish(queue, &hdr, src, copy);
}

void
Driver::Publish(player_devaddr_t addr,
                uint8_t type,
                uint8_t subtype,
                void* src,
                size_t deprecated,
                double* timestamp,
                bool copy)
{
  double t;

  // Fill in the time structure if not supplied
  if(timestamp)
    t = *timestamp;
  else
    GlobalTime->GetTimeDouble(&t);

  player_msghdr_t hdr;
  memset(&hdr,0,sizeof(player_msghdr_t));
  hdr.addr = addr;
  hdr.type = type;
  hdr.subtype = subtype;
  hdr.timestamp = t;
  hdr.size = 0;

  this->Publish(&hdr, src, copy);
}

void Driver::Lock()
{
  pthread_mutex_lock(&accessMutex);
}

void Driver::Unlock()
{
  pthread_mutex_unlock(&accessMutex);
}

int Driver::Subscribe(player_devaddr_t addr)
{
  int setupResult;

  if(subscriptions == 0)
  {
    setupResult = Setup();
    if (setupResult == 0 )
      subscriptions++;
  }
  else
  {
    subscriptions++;
    setupResult = 0;
  }

  return(setupResult);
}

int Driver::Unsubscribe(player_devaddr_t addr)
{
  int shutdownResult;

  if(subscriptions == 0)
    shutdownResult = -1;
  else if ( subscriptions == 1)
  {
    shutdownResult = Shutdown();
    subscriptions--;
  }
  else
  {
    subscriptions--;
    shutdownResult = 0;
  }

  return( shutdownResult );
}

/** @brief Wake up the driver if the specified event occurs on the file descriptor */
int Driver::AddFileWatch(int fd, bool ReadWatch , bool WriteWatch , bool ExceptWatch )
{
  return fileWatcher->AddFileWatch(fd,InQueue,ReadWatch,WriteWatch,ExceptWatch);
}

/** @brief Remove a previously added watch, call with the same arguments as when adding the watch */
int Driver::RemoveFileWatch(int fd, bool ReadWatch , bool WriteWatch , bool ExceptWatch )
{
  return fileWatcher->RemoveFileWatch(fd,InQueue,ReadWatch,WriteWatch,ExceptWatch);
}


/* start a thread that will invoke Main() */
void
Driver::StartThread(void)
{
  pthread_create(&driverthread, NULL, &DummyMain, this);
}

/* cancel (and wait for termination) of the thread */
void
Driver::StopThread(void)
{
  void* dummy;
  pthread_cancel(driverthread);
  // Release the driver thread, in case it's waiting on the message queue
  // or the driver mutex.
  this->InQueue->DataAvailable();
  this->Unlock();
  if(pthread_join(driverthread,&dummy))
    perror("Driver::StopThread:pthread_join()");
}

/* Dummy main (just calls real main) */
void*
Driver::DummyMain(void *devicep)
{
  // Install a cleanup function
  pthread_cleanup_push(&DummyMainQuit, devicep);

  // Run the overloaded Main() in the subclassed device.
  ((Driver*)devicep)->Main();

  // Run the uninstall cleanup function
  pthread_cleanup_pop(1);

  return NULL;
}

/* Dummy main cleanup (just calls real main cleanup) */
void
Driver::DummyMainQuit(void *devicep)
{
  // Run the overloaded MainCleanup() in the subclassed device.
  ((Driver*)devicep)->MainQuit();
}

void
Driver::Main()
{
  PLAYER_ERROR("You have called StartThread(), "
               "but didn't provide your own Main()!");
}

void
Driver::MainQuit()
{
}

// Default message handler
int Driver::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr,
                           void * data)
{
  return -1;
}

void Driver::ProcessMessages(void)
{
  this->ProcessMessages(0);
}

/// Call this to automatically process messages using registered handler
/// Processes messages until no messages remaining in the queue or
/// a message with no handler is reached
void Driver::ProcessMessages(int maxmsgs)
{
  // See if we have any pending messages and process them
  if(maxmsgs == 0)
    maxmsgs = this->InQueue->GetLength();
  int currmsg = 0;
  Message* msg;
  while(((maxmsgs < 0) || (currmsg < maxmsgs)) && (msg = this->InQueue->Pop()))
  {
    player_msghdr * hdr = msg->GetHeader();
    void * data = msg->GetPayload();

    // Try the driver's process function first
    // Drivers can override internal message handlers this way
    int ret = this->ProcessMessage(msg->Queue, hdr, data);
    if(ret < 0)
    {
      // Check if it's an internal message, if that doesn't handle it, give a warning
      if (ProcessInternalMessages(msg->Queue, hdr, data) != 0)
      {
        PLAYER_WARN7("Unhandled message for driver "
                   "device=%d:%d:%s:%d type=%s subtype=%d len=%d\n",
                   hdr->addr.host, hdr->addr.robot,
                   interf_to_str(hdr->addr.interf), hdr->addr.index,
                   msgtype_to_str(hdr->type), hdr->subtype, hdr->size);

        // If it was a request, reply with an empty NACK
        if(hdr->type == PLAYER_MSGTYPE_REQ)
          this->Publish(hdr->addr, msg->Queue, PLAYER_MSGTYPE_RESP_NACK,
                      hdr->subtype, NULL, 0, NULL);
      }
    }
    delete msg;
    pthread_testcancel();
    currmsg++;
  }
}

int Driver::ProcessInternalMessages(QueuePointer &resp_queue,
                                    player_msghdr * hdr, void * data)
{
  Property *property = NULL;

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_INTPROP_REQ))
  {
    player_intprop_req_t req = *reinterpret_cast<player_intprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->GetValueToMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_INTPROP_REQ, reinterpret_cast<void*> (&req), sizeof(player_intprop_req_t), NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_INTPROP_REQ))
  {
    player_intprop_req_t req = *reinterpret_cast<player_intprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->SetValueFromMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_INTPROP_REQ, NULL, 0, NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_DBLPROP_REQ))
  {
    player_dblprop_req_t req = *reinterpret_cast<player_dblprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->GetValueToMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_DBLPROP_REQ, reinterpret_cast<void*> (&req), sizeof(player_dblprop_req_t), NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_DBLPROP_REQ))
  {
    player_dblprop_req_t req = *reinterpret_cast<player_dblprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->SetValueFromMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_DBLPROP_REQ, NULL, 0, NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_GET_STRPROP_REQ))
  {
    player_strprop_req_t req = *reinterpret_cast<player_strprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->GetValueToMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_GET_STRPROP_REQ, reinterpret_cast<void*> (&req), sizeof(player_strprop_req_t), NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_STRPROP_REQ))
  {
    player_strprop_req_t req = *reinterpret_cast<player_strprop_req_t*> (data);
    if ((property = propertyBag.GetProperty (req.key)) == NULL)
      return -1;
    property->SetValueFromMessage (reinterpret_cast<void*> (&req));
    Publish(hdr->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_STRPROP_REQ, NULL, 0, NULL);
    return 0;
  }

  return -1;
}

bool Driver::RegisterProperty(const char *key, Property *prop, ConfigFile* cf, int section)
{
  if (!propertyBag.AddProperty (key, prop))
    return false;

  if (cf != NULL)
    prop->ReadConfig (cf, section);

  return true;
}

bool Driver::RegisterProperty(Property *prop, ConfigFile* cf, int section)
{
  return RegisterProperty(prop->GetKey(), prop, cf, section);
}

