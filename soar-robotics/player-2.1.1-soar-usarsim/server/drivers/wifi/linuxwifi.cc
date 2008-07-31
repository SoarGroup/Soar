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

/* Copyright (C) 2002
 *   John Sweeney, UMASS, Amherst, Laboratory for Perceptual Robotics
 *
 * $Id: linuxwifi.cc 4366 2008-02-18 19:32:04Z thjc $
 *
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_linuxwifi linuxwifi
 * @brief Linux WiFi devices

The linuxwifi driver provides access to information about wireless
Ethernet cards in Linux.  It reads the wireless info found in
/proc/net/wireless.  Sort of ad hoc right now, as I've only tested on
our own orinoco cards.

@par Compile-time dependencies

- &lt;linux/wireless.h&gt;

@par Provides

- @ref interface_wifi

@par Requires

- None

@par Configuration requests

- PLAYER_WIFI_REQ_MAC

@par Configuration file options

- interval (integer)
  - Default: 1000
  - Update interval; i.e., time between reading /proc/net/wireless
    (milliseconds)
 
@par Example 

@verbatim
driver
(
  name "linuxwifi"
  provides ["wifi:0"]
)
@endverbatim

@author John Sweeney

*/
/** @} */

#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/if_ether.h>

#include <time.h>

#include <libplayercore/playercore.h>

extern PlayerTime *GlobalTime;

#define WIFI_INFO_FILE "/proc/net/wireless"

#define WIFI_UPDATE_INTERVAL 1000 // in milliseconds
class LinuxWiFi : public Driver
{
public:
  LinuxWiFi( ConfigFile *cf, int section);

  ~LinuxWiFi();

  void Update(void);

  int Setup();
  int Shutdown();

  // MessageHandler
  int ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data);


protected:
  char * PrintEther(char *buf, unsigned char *addr);
  char * GetMACAddress(char *buf, int len);
protected:
  FILE *info_fp; // pointer to wifi info file
  fpos_t start_pos; // position of relevant info in info file

  int sfd; //socket to kernel
  char interface_name[32]; // name of wireless device

  struct iwreq *req; // used for getting wireless info in ioctls
  struct iw_range *range; 
  struct iw_statistics *stats;

  bool has_range;

  struct timeval last_update;
  int update_interval;
};

Driver * LinuxWiFi_Init( ConfigFile *cf, int section);
void LinuxWiFi_Register(DriverTable *table);

/* check for supported interfaces.
 *
 * returns: pointer to new LinuxWiFi driver if supported, NULL else
 */
Driver *
LinuxWiFi_Init( ConfigFile *cf, int section)
{ 
  return ((Driver*)(new LinuxWiFi( cf, section)));
}

/* register with drivertable
 *
 * returns: 
 */
void
LinuxWiFi_Register(DriverTable *table)
{
  table->AddDriver("linuxwifi", LinuxWiFi_Init);
}

LinuxWiFi::LinuxWiFi( ConfigFile *cf, int section) :
  Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_WIFI_CODE) 
{
  info_fp = NULL;
  
  sfd = -1;
  req = new struct iwreq;
  range = new struct iw_range;
  stats = new struct iw_statistics;

  has_range = false;

  last_update.tv_sec =0;
  last_update.tv_usec = 0;
  
  update_interval = cf->ReadInt(section, "interval", WIFI_UPDATE_INTERVAL);
}

LinuxWiFi::~LinuxWiFi() 
{
  delete req;
  delete range;
  delete stats;
}

int
LinuxWiFi::Setup()
{
  //  printf("LinuxWiFi: Wireless extensions %d\n", WIRELESS_EXT);
  
  // get the wireless device from /proc/net/wireless
  if ((this->info_fp = fopen(WIFI_INFO_FILE, "r")) == NULL) {
    fprintf(stderr, "LinuxWiFi: couldn't open info file \"%s\"\n",
	    WIFI_INFO_FILE);
    return -1;
  } 
  GlobalTime->GetTime(&last_update);

  // lets read it to the point we are interested in
  char buf[128];
  // read 3 lines
  for (int i =0; i < 3; i++) {
    if (i == 2) {
      fgetpos(info_fp, &start_pos);
    }

    if (fgets(buf, sizeof(buf),this->info_fp) == NULL) {
      fprintf(stderr, "LinuxWiFi: couldn't read line from info file\n");
      fclose(this->info_fp);
      return -1;
    }
  }

  // now we are at line of interest
  int eth, status; 
  double link, level, noise;
  sscanf(buf, "  eth%d: %d %lf %lf %lf", &eth, &status,
	 &link, &level, &noise);
  
  // buf has info about wireless interface
  char *name = strchr(buf, ':');
  if (name) {
    *name = '\0';
  } else {
    // no wireless interface
    fprintf(stderr, "LinuxWiFi: no wireless interface\n");
    return -1;
  }

  char *p = buf;
  while (isspace(*p)) {
    p++;
  }
  strncpy(interface_name, p, sizeof(interface_name));

  // copy this name into the iwreq struct for use in ioctls
  strncpy(req->ifr_name, interface_name, IFNAMSIZ);

  // this is the socket to use for wireless info
  sfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sfd < 0) {
    // didn't get a socket
    return -1;
  }
  
  struct iw_range reqbuf;
  
  // set the data part of the request
  req->u.data.pointer = (caddr_t)&reqbuf;
  req->u.data.length = sizeof(struct iw_range);
  req->u.data.flags = 0;
  
  // get range info... get it here because we set a flag on 
  // how it returns, so we know how to update
  if(ioctl(sfd, SIOCGIWRANGE, req) >= 0) 
  {
    has_range = true;
    memcpy((char *) range, &reqbuf, sizeof(struct iw_range));
  }

  return 0;
}  

int
LinuxWiFi::Shutdown()
{
  fclose(this->info_fp);
  
  close(sfd);
  return 0;
}

int LinuxWiFi::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data)
{
	assert(hdr);
	assert(data);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_WIFI_REQ_MAC, device_addr))
	{
		player_wifi_mac_req_t req;
  		GetMACAddress((char *)req.mac,32);
		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 0,(void*)&req, sizeof(player_wifi_mac_req_t),NULL);
	}
	return -1;
}

/* equivalent of main loop.  just read info from file, and PutData.
 */
void
LinuxWiFi::Update(void)
{
  int eth, status;
  int link, level, noise;
  uint16_t wqual=0, wlevel=0, wnoise=0;
  uint16_t  wmaxqual=0, wmaxlevel=0, wmaxnoise=0;
  uint8_t qual_type=0;
  uint32_t throughput=0;
  int32_t bitrate =0; 
  uint8_t mode = 0;
  player_wifi_data_t wifi_data;
  player_wifi_link_t wifi_link;
  
  struct timeval curr;

  GlobalTime->GetTime(&curr);

	ProcessMessages();

  // check whether we should update...
  if(((curr.tv_sec - last_update.tv_sec)*1000 +
       (curr.tv_usec - last_update.tv_usec)/1000) < update_interval)
  {
    // nope
    return;
  }

  last_update.tv_sec = curr.tv_sec;
  last_update.tv_usec = curr.tv_usec;

  // get new data

  // get the stats using an ioctl

  req->u.data.pointer = (caddr_t) stats;
  req->u.data.length = 0;
  req->u.data.flags = 1;  // clear updated flag
#ifdef SIOCGIWSTATS
  if (ioctl(sfd, SIOCGIWSTATS, req) < 0) {
    printf("LINUXWIFI: couldn't ioctl stats!\n");
#endif
    // failed to get stats, try from /proc/net/wireless
    // Dummy rewind; this is a hack to force the kernel/stdlib to
    // re-read the file.
    rewind(this->info_fp);
    
    // get the wifi info
    if (fsetpos(this->info_fp, &this->start_pos)) {
      fprintf(stderr, "LinuxWiFi: fsetpos returned error\n");
    }
    
    fscanf(this->info_fp, "  eth%d: %d %d. %d. %d.", &eth, &status,
	   &link, &level, &noise);
    
    wqual = (uint16_t) link;
    wlevel = (uint16_t) level;
    wnoise = (uint16_t) noise;

    qual_type = PLAYER_WIFI_QUAL_UNKNOWN;
#ifdef SIOCGIWSTATS
  } else {
    struct iw_quality *qual = &stats->qual;
    if (has_range) {
      throughput = range->throughput;
      if (qual->level != 0) {
	
	if (qual->level > range->max_qual.level) {
	  // we have dbm info
	  qual_type = PLAYER_WIFI_QUAL_DBM;
	} else {
	  qual_type = PLAYER_WIFI_QUAL_REL;
	}
      } 
    }else {
      qual_type = PLAYER_WIFI_QUAL_UNKNOWN;
    }

    wqual = qual->qual;
    wmaxqual = range->max_qual.qual;
    wlevel = qual->level;
    wmaxlevel = range->max_qual.level;
    wnoise = qual->noise;
    wmaxnoise = range->max_qual.noise;
  }
#endif

  // get operation mode
  mode = PLAYER_WIFI_MODE_UNKNOWN;
  if (ioctl(sfd, SIOCGIWMODE, req) >= 0) {
    // change it into a LINUX WIFI independent value
    switch(req->u.mode) {
    case IW_MODE_AUTO:
      mode = PLAYER_WIFI_MODE_AUTO;
      break;
    case IW_MODE_ADHOC:
      mode = PLAYER_WIFI_MODE_ADHOC;
      break;
    case IW_MODE_INFRA:
      mode = PLAYER_WIFI_MODE_INFRA;
      break;
    case IW_MODE_MASTER:
      mode = PLAYER_WIFI_MODE_MASTER;
      break;
    case IW_MODE_REPEAT:
      mode = PLAYER_WIFI_MODE_REPEAT;
      break;
    case IW_MODE_SECOND:
      mode = PLAYER_WIFI_MODE_SECOND;
      break;
    default:
      mode = PLAYER_WIFI_MODE_UNKNOWN;
    }
  }

  // set interface data
  wifi_data.throughput = htonl(throughput);
  wifi_data.mode = mode;
  
  // get AP address
  if (ioctl(sfd, SIOCGIWAP, req) >= 0) {
    // got it...
    struct sockaddr sa;
    memcpy(&sa, &(req->u.ap_addr), sizeof(sa));
    
    PrintEther((char *)wifi_data.ap, (unsigned char *)sa.sa_data);
  } else {
    strncpy(wifi_data.ap, "00:00:00:00:00:00", sizeof(wifi_data.ap));
  }
  
  // get bitrate
  bitrate = 0;
  if (ioctl(sfd, SIOCGIWRATE, req) >= 0) {
    bitrate = req->u.bitrate.value;
  }

  wifi_data.bitrate = bitrate;
    
  wifi_data.links_count = 1;
  wifi_data.links = &wifi_link;
  strncpy((char*)wifi_data.links[0].ip, "0.0.0.0", sizeof(wifi_data.links[0].ip));
  wifi_data.links[0].ip_count = sizeof(wifi_data.links[0].ip);
  wifi_data.links[0].qual = wqual;
  wifi_data.links[0].level = wlevel;
  wifi_data.links[0].noise = wnoise;

  wifi_data.links[0].mac_count = 0;
  wifi_data.links[0].essid_count = 0;
  
  wifi_data.maxqual = wmaxqual;
  wifi_data.maxlevel = wmaxlevel;
  wifi_data.maxnoise = wmaxnoise;
 
  wifi_data.qual_type = qual_type;

  Publish(device_addr, PLAYER_MSGTYPE_DATA,PLAYER_WIFI_DATA_STATE,(void*)&wifi_data);
}


/* Taken from iwlib.c in wireless tools which in turn claims to be a
 * cut & paste from net-tools-1.2.0 */
char *
LinuxWiFi::GetMACAddress(char *buf, int len)
{
  struct ifreq ifr;
  
  /* Get the type of hardware address */
  strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
  if ((ioctl(sfd, SIOCGIFHWADDR, &ifr) < 0) ||
      (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)) {
    /* Deep trouble... */
    fprintf(stderr, "LinuxWiFi: Interface %s doesn't support MAC addresses\n", 
	    interface_name);
    *buf='\0';
    return buf;
  }
  PrintEther(buf, (unsigned char *)ifr.ifr_hwaddr.sa_data);

  return buf;
}

/* write the ethernet address in standard hex format
 * writes it into given buffer.
 * taken from iwlib.c (wireless_tools)
 *
 * returns: pointer to buffer given as arg
 */
char *
LinuxWiFi::PrintEther(char *buf, unsigned char *data)
{
  struct ether_addr * p = (struct ether_addr *)data;
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
	  p->ether_addr_octet[0], p->ether_addr_octet[1],
	  p->ether_addr_octet[2], p->ether_addr_octet[3],
	  p->ether_addr_octet[4], p->ether_addr_octet[5]);

  return buf;
}

  
