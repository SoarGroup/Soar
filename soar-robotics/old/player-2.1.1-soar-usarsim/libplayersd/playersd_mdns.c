/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007
 *     Brian Gerkey
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
/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007
 *     Brian Gerkey
 *                      
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
 */

/*
 * libplayersd implementation based on Apple's mDNSResponder (aka Bonjour).
 * This library works with either mDNSResponder or the Avahi mDNSResponder
 * compatibility layer.
 *
 * $Id: playersd_mdns.c 4196 2007-10-09 21:15:04Z gerkey $
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

#include <dns_sd.h>

#include <libplayercore/error.h>
#include <libplayercore/interface_util.h>
#include <libplayercore/addr_util.h>

#include <replace/replace.h> // for poll(2)

#include "playersd.h"

#define PLAYER_SD_MDNS_DEVS_LEN_INITIAL 4
#define PLAYER_SD_MDNS_DEVS_LEN_MULTIPLIER 2

// Info for one registered device
typedef struct
{
  // Is this entry valid?
  uint8_t valid;
  // Did registration fail?
  uint8_t fail;
  // Identifying information, as provided by user when registering
  player_sd_dev_t sdDev;
  // Index appended to sdDev.name to make it unique
  int nameIdx;
  // Session reference used for registration.  We deallocate to terminate
  DNSServiceRef regRef;
  // TXT reference
  TXTRecordRef txtRecord;
  // Local storage for TXT record
  uint8_t txtBuf[PLAYER_SD_TXT_MAXLEN];
} player_sd_mdns_dev_t;

// We'll use this as the opaque reference that the user is given
typedef struct
{
  // Session reference for browsing
  DNSServiceRef browseRef;
  // Is browse reference valid?
  uint8_t browseRef_valid;
  // List of registered devices
  player_sd_mdns_dev_t* mdnsDevs;
  // Length of list
  size_t mdnsDevs_len;
  // User's browse callback
  player_sd_browse_callback_fn_t callb;
  // Last set of flags that we got in the browse callback
  DNSServiceFlags flags;
  // Mutex to protect access to this structure
  pthread_mutex_t mutex;
} player_sd_mdns_t;

static void registerCB(DNSServiceRef sdRef, 
                       DNSServiceFlags flags, 
                       DNSServiceErrorType errorCode, 
                       const char *name, 
                       const char *regtype, 
                       const char *domain, 
                       void *context);  

static void browseCB(DNSServiceRef sdRef, 
                     DNSServiceFlags flags, 
                     uint32_t interfaceIndex, 
                     DNSServiceErrorType errorCode, 
                     const char *serviceName, 
                     const char *regtype, 
                     const char *replyDomain, 
                     void *context);

static void resolveCB(DNSServiceRef sdRef, 
                      DNSServiceFlags flags, 
                      uint32_t interfaceIndex, 
                      DNSServiceErrorType errorCode, 
                      const char *fullname, 
                      const char *hosttarget, 
                      uint16_t port, 
                      uint16_t txtLen, 
                      const char *txtRecord, 
                      void *context);  

player_sd_t* 
player_sd_init(void)
{
  player_sd_t* sd;
  player_sd_mdns_t* mdns;

  sd = (player_sd_t*)malloc(sizeof(player_sd_t));
  assert(sd);
  sd->devs = NULL;
  sd->devs_len = 0;

  mdns = (player_sd_mdns_t*)malloc(sizeof(player_sd_mdns_t));
  assert(mdns);
  mdns->browseRef_valid = 0;
  mdns->mdnsDevs = NULL;
  mdns->mdnsDevs_len = 0;
  mdns->callb = NULL;
  mdns->flags = 0;
  sd->sdRef = mdns;

  pthread_mutex_init(&(mdns->mutex),NULL);

  // Initialize the interface table, so that we can decode the interface
  // strings names that are used in TXT records.
  itable_init();

  // Pick a default debug level
  ErrorInit(1);

  return(sd);
}

void
player_sd_fini(player_sd_t* sd)
{
  int i;
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);

  if(mdns->browseRef_valid)
  {
    DNSServiceRefDeallocate(mdns->browseRef);
    mdns->browseRef_valid = 0;
  }

  for(i=0;i<mdns->mdnsDevs_len;i++)
  {
    if(mdns->mdnsDevs[i].valid)
    {
      DNSServiceRefDeallocate(mdns->mdnsDevs[i].regRef);
      mdns->mdnsDevs[i].valid = 0;
    }
  }
  
  pthread_mutex_destroy(&(mdns->mutex));

  if(mdns->mdnsDevs)
    free(mdns->mdnsDevs);
  free(mdns);
  free(sd);
}

void 
player_sd_lock(player_sd_t* sd)
{
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);
  pthread_mutex_lock(&(mdns->mutex));
}

void 
player_sd_unlock(player_sd_t* sd)
{
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);
  pthread_mutex_unlock(&(mdns->mutex));
}

int 
player_sd_register(player_sd_t* sd, 
                   const char* name, 
                   player_devaddr_t addr)
{
  DNSServiceErrorType sdErr;
  char recordval[PLAYER_SD_TXT_MAXLEN];
  int i,j;
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);
  player_sd_mdns_dev_t* dev;
  char nameBuf[PLAYER_SD_NAME_MAXLEN];

  // Find a spot for this device
  for(i=0;i<mdns->mdnsDevs_len;i++)
  {
    if(!mdns->mdnsDevs[i].valid)
      break;
  }
  if(i==mdns->mdnsDevs_len)
  {
    // Make the list bigger
    if(!mdns->mdnsDevs_len)
      mdns->mdnsDevs_len = PLAYER_SD_MDNS_DEVS_LEN_INITIAL;
    else
      mdns->mdnsDevs_len *= PLAYER_SD_MDNS_DEVS_LEN_MULTIPLIER;
    mdns->mdnsDevs = 
            (player_sd_mdns_dev_t*)realloc(mdns->mdnsDevs,
                                           (mdns->mdnsDevs_len * 
                                            sizeof(player_sd_mdns_dev_t)));
    assert(mdns->mdnsDevs);
    for(j=i;j<mdns->mdnsDevs_len;j++)
      mdns->mdnsDevs[j].valid = 0;
  }

  dev = mdns->mdnsDevs + i;
  dev->fail = 0;
  memset(dev->sdDev.name,0,sizeof(dev->sdDev.name));
  strncpy(dev->sdDev.name,name,sizeof(dev->sdDev.name)-1);
  memset(dev->sdDev.hostname,0,sizeof(dev->sdDev.hostname));
  packedaddr_to_dottedip(dev->sdDev.hostname,sizeof(dev->sdDev.hostname),
                         addr.host);
  dev->sdDev.robot = addr.robot;
  dev->sdDev.interf = addr.interf;
  dev->sdDev.index = addr.index;
  dev->nameIdx = 1;

  TXTRecordCreate(&(dev->txtRecord),sizeof(dev->txtBuf),dev->txtBuf);
  memset(recordval,0,sizeof(recordval));
  snprintf(recordval, sizeof(recordval), "%s:%u",
           interf_to_str(addr.interf), addr.index);
  if((sdErr = TXTRecordSetValue(&(dev->txtRecord),
                                "device",
                                strlen(recordval),
                                recordval)))
  {
    PLAYER_ERROR1("TXTRecordSetValue returned error: %d", sdErr);
    return(-1);
  }

  memset(nameBuf,0,sizeof(nameBuf));
  strncpy(nameBuf,name,sizeof(nameBuf)-1);
  sdErr = kDNSServiceErr_NameConflict;

  // Avahi can return the kDNSServiceErr_NameConflict immediately.
  while(sdErr == kDNSServiceErr_NameConflict)
  {
    sdErr = DNSServiceRegister(&(dev->regRef), 
                               0,
                               0,
                               nameBuf,
                               PLAYER_SD_SERVICENAME,
                               NULL,
                               NULL,
                               addr.robot,
                               TXTRecordGetLength(&(dev->txtRecord)),
                               TXTRecordGetBytesPtr(&(dev->txtRecord)),
                               registerCB,
                               (void*)dev);

    if(sdErr == kDNSServiceErr_NameConflict)
    {
      // Pick a new name
      memset(nameBuf,0,sizeof(nameBuf));
      snprintf(nameBuf,sizeof(nameBuf),"%s (%d)",
               name,dev->nameIdx++);
    }
  }

  if(sdErr != kDNSServiceErr_NoError)
  {
    PLAYER_ERROR1("DNSServiceRegister returned error: %d", sdErr);
    return(-1);
  }
  else
  {
    dev->valid = 1;
    if(strcmp(nameBuf,name))
      PLAYER_WARN2("Changing service name of %s to %s\n",
                   name,nameBuf);
    PLAYER_MSG1(2,"Registration of %s successful", name);
    return(0);
  }
}

int 
player_sd_unregister(player_sd_t* sd, 
                     const char* name)
{
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);
  player_sd_mdns_dev_t* dev = NULL;
  int i;

  for(i=0;i<mdns->mdnsDevs_len;i++)
  {
    if(mdns->mdnsDevs[i].valid && !strcmp(name,mdns->mdnsDevs[i].sdDev.name))
    {
      dev = mdns->mdnsDevs + i;
      break;
    }
  }

  if(dev)
  {
    DNSServiceRefDeallocate(dev->regRef);
    dev->valid = 0;
    PLAYER_MSG1(2,"Unregistration of %s successful", name);
    return(0);
  }
  else
  {
    PLAYER_ERROR1("Failed to find and unregister device %s", name);
    return(-1);
  }
}

int 
player_sd_browse(player_sd_t* sd,
                 double timeout, 
                 int keepalive,
                 player_sd_browse_callback_fn_t cb)
{
  DNSServiceErrorType sdErr;
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)(sd->sdRef);
  int retval=0;
  struct timeval curr;
  double currtime, starttime;
  double polltime;
  
  // Close any previously open session
  if(mdns->browseRef_valid)
  {
    DNSServiceRefDeallocate(mdns->browseRef);
    mdns->browseRef_valid = 0;
  }

  mdns->flags = 0;
  // Initiate the browse session
  if((sdErr = DNSServiceBrowse(&(mdns->browseRef),
                               0,
                               0,
                               PLAYER_SD_SERVICENAME,
                               NULL,
                               browseCB,
                               (void*)sd)) != kDNSServiceErr_NoError)
  {
    PLAYER_ERROR1("DNSServiceBrowse returned error: %d", sdErr);
    return(-1);
  }

  mdns->browseRef_valid = 1;
  mdns->callb = cb;

  // Should we wait here for responses?
  if(timeout != 0.0)
  {
    // Record the current time
    gettimeofday(&curr,NULL);
    starttime = currtime = curr.tv_sec + curr.tv_usec / 1e6;

    // Update until the requested time has elapsed
    while((timeout < 0.0) || ((currtime - starttime) < timeout))
    {
      // Set up to poll on the DNSSD socket
      if(timeout > 0.0)
        polltime = timeout - (currtime - starttime);
      else
        polltime = -1.0;

      if((retval = player_sd_update(sd,polltime)) != 0)
        break;

      gettimeofday(&curr,NULL);
      currtime = curr.tv_sec + curr.tv_usec / 1e6;
    }
  }

  // Should we leave the session open?
  if(!keepalive)
  {
    DNSServiceRefDeallocate(mdns->browseRef);
    mdns->browseRef_valid = 0;
  }
  return(retval);
}

int
player_sd_browse_stop(player_sd_t* sd)

{
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)sd->sdRef;

  if(mdns->browseRef_valid)
  {
    DNSServiceRefDeallocate(mdns->browseRef);
    mdns->browseRef_valid = 0;
  }
  return(0);
}

int 
player_sd_update(player_sd_t* sd, double timeout)
{
  struct pollfd ufds[1];
  int numready;
  int polltime;
  DNSServiceErrorType sdErr;
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)sd->sdRef;

  if(!mdns->browseRef_valid)
  {
    PLAYER_ERROR("Can't update without a valid browsing session");
    return(-1);
  }

  ufds[0].fd = DNSServiceRefSockFD(mdns->browseRef);
  ufds[0].events = POLLIN;

  if(timeout >= 0.0)
    polltime = (int)rint(timeout * 1e3);
  else
    polltime = -1;


  for(;;)
  {
    if((numready = poll(ufds,1,polltime)) < 0)
    {
      if(errno == EAGAIN)
      {
        // TODO: strictly speaking, we should decrement polltime here by
        // the time that has elapsed so far
        continue;
      }
      else
      {
        PLAYER_ERROR1("poll returned error: %s", strerror(errno));
        return(-1);
      }
    }
    else if(numready > 0)
    {
      // Read all queued up responses
      if((sdErr = DNSServiceProcessResult(mdns->browseRef)) != 
         kDNSServiceErr_NoError)
      {
        PLAYER_ERROR1("DNSServiceProcessResult returned error: %d", sdErr);
        return(-1);
      }
      while(mdns->flags & kDNSServiceFlagsMoreComing)
      {
        if((sdErr = DNSServiceProcessResult(mdns->browseRef)) != 
           kDNSServiceErr_NoError)
        {
          PLAYER_ERROR1("DNSServiceProcessResult returned error: %d", sdErr);
          return(-1);
        }
      }
      break;
    }
    else
    {
      // timeout
      break;
    }
  }
  return(0);
}

void 
registerCB(DNSServiceRef sdRef, 
           DNSServiceFlags flags, 
           DNSServiceErrorType errorCode, 
           const char *name, 
           const char *regtype, 
           const char *domain, 
           void *context)
{
  // Don't need to do anything here
}

void 
browseCB(DNSServiceRef sdRef, 
         DNSServiceFlags flags, 
         uint32_t interfaceIndex, 
         DNSServiceErrorType errorCode, 
         const char *serviceName, 
         const char *regtype, 
         const char *replyDomain, 
         void *context)
{
  player_sd_t* sd = (player_sd_t*)context;
  player_sd_mdns_t* mdns = (player_sd_mdns_t*)sd->sdRef;
  player_sd_dev_t* sddev;
  DNSServiceRef resolveRef;
  DNSServiceErrorType sdErr;
  struct pollfd ufds[1];
  int numready;

  mdns->flags = flags;

  // Got a browse event.  
  if(flags & kDNSServiceFlagsAdd)
  {
    // A new service was added.  First check whether we already have an
    // entry for a device with this name.
    if(!(sddev = player_sd_get_device(sd, serviceName)))
    {
      // No existing entry.  Add one
      sddev = _player_sd_add_device(sd, serviceName);
    }

    // Record the name
    sddev->valid = 1;
    sddev->addr_valid = 0;
    sddev->addr_fail = 0;
    memset(sddev->name,0,sizeof(sddev->name));
    strncpy(sddev->name,serviceName,sizeof(sddev->name)-1);

    // Resolve its address
    if((sdErr = DNSServiceResolve(&resolveRef,
                                  0,
                                  interfaceIndex, 
                                  serviceName, 
                                  regtype, 
                                  replyDomain,
                                  resolveCB,
                                  (void*)sddev)) != kDNSServiceErr_NoError)
    {
      PLAYER_ERROR1("DNSServiceResolve returned error: %d\n", sdErr);
      return;
    }

    ufds[0].fd = DNSServiceRefSockFD(resolveRef);
    ufds[0].events = POLLIN;
    while(!sddev->addr_valid && !sddev->addr_fail)
    {
      if((numready = poll(ufds,1,-1)) < 0)
      {
        if(errno == EAGAIN)
          continue;
        else
        {
          PLAYER_ERROR1("poll returned error: %s", strerror(errno));
          sddev->addr_fail = 1;
        }
      }
      else if(numready > 0)
      {
        if((sdErr = DNSServiceProcessResult(resolveRef)) != 
           kDNSServiceErr_NoError)
        {
          PLAYER_ERROR1("DNSServiceProcessResult returned error: %d\n", sdErr);
          sddev->addr_fail = 1;
        }
      }
    }
    DNSServiceRefDeallocate(resolveRef);
    if(sddev->addr_valid)
    {
      // Invoke the users' callback
      if(mdns->callb)
        (mdns->callb)(sd,sddev);
    }
  }
  else
  {
    // An existing service was removed.  Delete our records for it.
    if((sddev = player_sd_get_device(sd, serviceName)))
    {
      sddev->valid = 0;
      sddev->addr_valid = 0;
      // Invoke the users' callback
      if(mdns->callb)
        (mdns->callb)(sd,sddev);
    }
  }

  //player_sd_printcache(sd);
}

void 
resolveCB(DNSServiceRef sdRef, 
          DNSServiceFlags flags, 
          uint32_t interfaceIndex, 
          DNSServiceErrorType errorCode, 
          const char *fullname, 
          const char *hosttarget, 
          uint16_t port, 
          uint16_t txtLen, 
          const char *txtRecord, 
          void *context)
{
  player_sd_dev_t* sddev = (player_sd_dev_t*)context;
  const char* value;
  uint8_t value_len;
  char* colon;
  char buf[PLAYER_SD_TXT_MAXLEN];

  // Handle resolution result
  if(errorCode == kDNSServiceErr_NoError)
  {
    // Fill in the address info
    memset(sddev->hostname,0,sizeof(sddev->hostname));
    strncpy(sddev->hostname,hosttarget,sizeof(sddev->hostname)-1);
    sddev->robot = port;
    if(!(value = (const char*)TXTRecordGetValuePtr(txtLen,
                                                   txtRecord,
                                                   PLAYER_SD_DEVICE_TXTNAME,
                                                   &value_len)))
    {
      PLAYER_ERROR1("Failed to find TXT info for service %s\n", sddev->name);
      sddev->addr_fail = 1;
      return;
    }
    if(!(colon = strchr(value,':')) ||
       ((colon-value) <= 0) ||
       ((value_len - (colon-value+1)) <= 0))
    {
      PLAYER_ERROR2("Failed to parse TXT info \"%s\" for service %s\n",
                    value, sddev->name);
      sddev->addr_fail = 1;
      return;
    }
    memset(buf,0,sizeof(buf));
    strncpy(buf,value,(colon-value));
    sddev->interf = str_to_interf(buf);

    memset(buf,0,sizeof(buf));
    strncpy(buf,colon+1,(value_len-(colon-value+1)));
    sddev->index = atoi(buf);

    sddev->addr_valid = 1;
  }
  else
  {
    // Something went wrong.
    sddev->addr_fail = 1;
  }
}
