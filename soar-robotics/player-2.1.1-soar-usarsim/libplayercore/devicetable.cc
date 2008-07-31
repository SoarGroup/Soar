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
 * $Id: devicetable.cc 6566 2008-06-14 01:00:19Z thjc $
 *
 *   class to keep track of available devices.
 */
#include <string.h> // for strncpy(3)
#include <stdlib.h> // for atoi(3)

#include <libplayercore/error.h>
#include <libplayercore/devicetable.h>
#include <libplayercore/interface_util.h>
#include <libplayercore/addr_util.h>

// initialize the table
DeviceTable::DeviceTable()
{
  this->numdevices = 0;
  this->head = NULL;
  pthread_mutex_init(&this->mutex,NULL);
  this->remote_driver_fn = NULL;
  this->remote_driver_arg = NULL;
}

// tear down the table
DeviceTable::~DeviceTable()
{
  Device* thisentry;
  Device* tmpentry;
  // First, shutdown each active driver
  thisentry=head;
  while(thisentry)
  {
    if(thisentry->driver->subscriptions)
    {
      thisentry->driver->Shutdown();
      thisentry->driver->subscriptions = 0;
      thisentry->driver->alwayson = 0;
    }
    thisentry = thisentry->next;
  }
  pthread_mutex_lock(&mutex);
  // Second, delete each device
  thisentry=head;
  while(thisentry)
  {
    tmpentry = thisentry->next;
    delete thisentry;
    numdevices--;
    thisentry = tmpentry;
  }
  pthread_mutex_unlock(&mutex);

  // destroy the mutex.
  pthread_mutex_destroy(&mutex);
}

// this is the 'base' AddDevice method, which sets all the fields
Device*
DeviceTable::AddDevice(player_devaddr_t addr,
                       Driver* driver, bool havelock)
{
  Device* thisentry;
  Device* preventry;

  if(!havelock)
    pthread_mutex_lock(&mutex);

  // Check for duplicate entries (not allowed)
  for(thisentry = head,preventry=NULL; thisentry;
      preventry=thisentry, thisentry=thisentry->next)
  {
    if(Device::MatchDeviceAddress(thisentry->addr, addr))
      break;
  }
  if(thisentry)
  {
    PLAYER_ERROR4("duplicate device addr %X:%d:%s:%d",
                  addr.host, addr.robot,
                  interf_to_str(addr.interf),
                  addr.index);
    if(!havelock)
      pthread_mutex_unlock(&mutex);
    return(NULL);
  }

  // Create a new device entry
  thisentry = new Device(addr, driver);
  thisentry->next = NULL;
  if(preventry)
    preventry->next = thisentry;
  else
    head = thisentry;
  numdevices++;

  if(!havelock)
    pthread_mutex_unlock(&mutex);
  return(thisentry);
}

// find a device entry, based on addr, and return the pointer (or NULL
// on failure)
Device*
DeviceTable::GetDevice(player_devaddr_t addr, bool lookup_remote)
{
  Device* thisentry;
  pthread_mutex_lock(&mutex);
  for(thisentry=head;thisentry;thisentry=thisentry->next)
  {
    if(Device::MatchDeviceAddress(thisentry->addr, addr))
      break;
  }

  // If we didn't find the device, give the application's remote device
  // handler a try
  if((thisentry == NULL) && lookup_remote && (this->remote_driver_fn != NULL))
  {
    Driver* rdriver = (*this->remote_driver_fn)(addr,this->remote_driver_arg);
    if(rdriver != NULL)
    {
      if(this->AddDevice(addr, rdriver, true) == NULL)
      {
        PLAYER_ERROR("failed to add remote device");
        delete rdriver;
      }
      else
      {
        for(thisentry=head;thisentry;thisentry=thisentry->next)
        {
          if(Device::MatchDeviceAddress(thisentry->addr, addr))
            break;
        }
        assert(thisentry);
        strncpy(thisentry->drivername, "remote",
                sizeof(thisentry->drivername));
      }
    }
  }

  pthread_mutex_unlock(&mutex);
  return(thisentry);
}

// find a device, based on id, and return the pointer (or NULL on
// failure)
Device* 
DeviceTable::GetDevice(const char* str_addr,
                       bool lookup_remote)
{
  player_devaddr_t addr;
  memset(&addr,0,sizeof(player_devaddr_t));

  char* str_addr_copy = strdup(str_addr);
  char* colon;

  // Must have an index
  if(!((colon = strrchr(str_addr_copy,':'))))
    return(NULL);
  addr.index = atoi(colon+1);
  *colon = '\0';

  // Must have an interface (but it might not have a preceding colon)
  if(!((colon = strrchr(str_addr_copy,':'))))
  {
    if(!strlen(str_addr_copy))
      return(NULL);
    addr.interf = str_to_interf(str_addr_copy);
    colon = str_addr_copy;
  }
  else
  {
    addr.interf = str_to_interf(colon+1);
  }
  *colon = '\0';
  
  // Might have a robot
  if(!((colon = strrchr(str_addr_copy,':'))))
  {
    colon = str_addr_copy;
    if(strlen(str_addr_copy))
      addr.robot = atoi(str_addr_copy);
  }
  else
  {
    addr.robot = atoi(colon+1);
  }
  *colon = '\0';

  // Might have a host
  if(!((colon = strrchr(str_addr_copy,':'))))
  {
    if(strlen(str_addr_copy))
      hostname_to_packedaddr(&addr.host,str_addr_copy);
  }
  else
    hostname_to_packedaddr(&addr.host,colon+1);


  return(this->GetDevice(addr,lookup_remote));
}

// Call Update() on each driver with non-zero subscriptions
//
// NOTE: this will call Update() once for each subscribed interface to a
// multi-interface driver.
void
DeviceTable::UpdateDevices()
{
  Device* thisentry;
  Driver* dri;

  // We don't lock here, on the assumption that the caller is also the only
  // thread that can make changes to the device table.
  for(thisentry=head;thisentry;thisentry=thisentry->next)
  {
    dri = thisentry->driver;
    if((dri->subscriptions > 0) || dri->alwayson)
      dri->Update();
  }
}

int
DeviceTable::StartAlwaysonDrivers()
{
  Device* thisentry;

  // We don't lock here, on the assumption that the caller is also the only
  // thread that can make changes to the device table.
  for(thisentry=head;thisentry;thisentry=thisentry->next)
  {
    if(thisentry->driver->alwayson)
    {
    	  QueuePointer Temp = QueuePointer();
      if(thisentry->Subscribe(Temp) != 0)
      {
        PLAYER_ERROR2("initial subscription failed for device %s:%d",
                      interf_to_str(thisentry->addr.interf), thisentry->addr.index);
        return(-1);
      }
    }
  }
  return(0);
}

// Register a factory creation function.  It will be called when
// GetDevice fails to find a device in the deviceTable.  This function
// might, for example, locate the device on a remote host (in a
// transport-dependent manner).
void
DeviceTable::AddRemoteDriverFn(remote_driver_fn_t rdf, void* arg)
{
  this->remote_driver_fn = rdf;
  this->remote_driver_arg = arg;
}
