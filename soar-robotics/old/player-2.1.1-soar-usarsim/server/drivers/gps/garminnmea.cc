/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003  Brian Gerkey gerkey@usc.edu
 *                           Andrew Howard ahoward@usc.edu
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
 * $Id: garminnmea.cc 6566 2008-06-14 01:00:19Z thjc $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_garminnmea garminnmea
 * @brief Garmin GPS unit
 
%Device driver for the Garmin geko 201 handheld GPS unit.  Interacts with
the unit by speaking NMEA over a serial line.  As such, this driver may
work with other Garmin units, and (likely with some modification) other
NMEA-compliant GPS units.
 
The driver may also attempt to read DGPS RTCM corrections from a
multi-cast network address, and forward these corrections to the
GPS unit.  The @ref util_dgps_server utility may be used to gather
and broadcast the DGPS RTCM corrections.
 
NMEA and proprietary Garmin codes can be found at
http://home.mira.net/~gnb/gps/nmea.html

@par Compile-time dependencies

- none

@par Requires

- none

@par Provides

- @ref interface_gps

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port where the GPS unit is connected
- baud (integer)
  - Default: 4800
  - Speed of serial connection to the GPS unit
- dgps_enable (integer)
  - Default: 1
  - Enable/disable listening for DGPS corrections via UDP multicast
    (use @ref util_dgps_server to send the corrections)
- dgps_group (string)
  - Default: "225.0.0.43"
  - Multicast group on which to listen for DGPS corrections</td></tr>
- dgps_port (integer)
  - Default: 7778
  - UDP port on which to listen for DGPS corrections

@par Example

@verbatim
driver
(
  name "garminnmea"
  provides ["gps:0"]
  port "/dev/ttyS1"
)
@endverbatim

@authors Brian Gerkey, Andrew Howard

*/
/** @} */
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif


#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>  // for htons(3)
#include <arpa/inet.h> // inet_addr
#include <errno.h>


#include <libplayercore/playercore.h>
#include <replace/replace.h>

#define DEFAULT_GPS_PORT "/dev/ttyS0"
#define DEFAULT_DGPS_GROUP "225.0.0.43"
#define DEFAULT_DGPS_PORT 7778


// time that we'll wait to get the first round of data from the unit.
// currently 1s
#define GPS_STARTUP_CYCLE_USEC 100000
#define GPS_STARTUP_CYCLES 10

// these are the standard NMEA sentences that come 
// out of the geko 201
#define NMEA_GPRMB "GPRMB"
#define NMEA_GPRMC "GPRMC"
#define NMEA_GPGGA "GPGGA"
#define NMEA_GPGSA "GPGSA"
#define NMEA_GPGSV "GPGSV"
#define NMEA_GPGLL "GPGLL"
#define NMEA_GPBOD "GPBOD"
#define NMEA_GPRTE "GPRTE"

// these are standard NMEA sentences that come
// out of other units (NovaTel)
#define NMEA_GPGST "GPGST"

// these are the proprietary NMEA sentences that 
// come out of the geko 201
#define NMEA_PGRME "PGRME"
#define NMEA_PGRMZ "PGRMZ"
#define NMEA_PSLIB "PSLIB"

// spec says 82, but that done mean squat. Make it big
#define NMEA_MAX_SENTENCE_LEN 128

#define NMEA_START_CHAR '$'
#define NMEA_END_CHAR '\n'
#define NMEA_CHKSUM_CHAR '*'

// WGS84 Parameters
#define WGS84_A		6378137.0		// major axis
#define WGS84_B		6356752.31424518	// minor axis
#define WGS84_F		0.0033528107		// ellipsoid flattening
#define WGS84_E		0.0818191908		// first eccentricity
#define WGS84_EP	0.0820944379		// second eccentricity

// UTM Parameters
#define UTM_K0		0.9996			// scale factor
#define UTM_FE		500000.0		// false easting
#define UTM_FN_N	0.0			// false northing on north hemisphere
#define UTM_FN_S	10000000.0		// false northing on south hemisphere
#define UTM_E2		(WGS84_E*WGS84_E)	// e^2
#define UTM_E4		(UTM_E2*UTM_E2)		// e^4
#define UTM_E6		(UTM_E4*UTM_E2)		// e^6
#define UTM_EP2		(UTM_E2/(1-UTM_E2))	// e'^2


class GarminNMEA:public Driver 
{
  private:

    // string name of serial port to use
    const char* gps_serial_port;

    // GPS baud rate
    int gps_baud;

    // file descriptor for the gps unit
    int gps_fd;  

    // Enable DGPS corrections?
    int dgps_enable;
  
    // Port number for DGPS RTCM corrections
    const char *dgps_group;
    int dgps_port;

    // file descriptor for the DGPS UDP socket
    int dgps_fd;
    bool gps_fd_blocking;
  
    char nmea_buf[NMEA_MAX_SENTENCE_LEN+1];
    size_t nmea_buf_len;

    // Status
    int read_count;

    // Filtered GPS geodetic coords; for outlier rejection
    double filter_a, filter_thresh;
    double filter_lat, filter_lon;
    bool filter_good;
  
    // Current GPS data packet
    player_gps_data_t data;

    int SetupSerial();
    void ShutdownSerial();
  
    int SetupSocket();
    void ShutdownSocket();

    int ReadSentence(char* buf, size_t len);
    int WriteSentence(const char *buf, size_t len);
    int ReadSocket(char *buf, size_t len);

    int FillBuffer();
    int ParseSentence(const char* buf);
    int ParseGPGGA(const char *buf);
    int ParseGPRMC(const char *buf);
    int ParsePGRME(const char *buf);
    int ParseGPGST(const char *buf);
    const char* GetNextField(char* field, size_t len, const char* ptr);

    // utility functions to convert geodetic to UTM position
    void UTM(double lat, double lon, double *x, double *y);


  public:
    GarminNMEA( ConfigFile* cf, int section);

    virtual int Setup();
    virtual int Shutdown();
    virtual void Main();
};


///////////////////////////////////////////////////////////////////////////
// initialization function
Driver* GarminNMEA_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new GarminNMEA( cf, section)));
}


///////////////////////////////////////////////////////////////////////////
// a driver registration function
void 
GarminNMEA_Register(DriverTable* table)
{
  table->AddDriver("garminnmea",  GarminNMEA_Init);
}

///////////////////////////////////////////////////////////////////////////
GarminNMEA::GarminNMEA( ConfigFile* cf, int section) 
: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_GPS_CODE)
{
  memset(&data,0,sizeof(data));

  gps_fd = -1;

  gps_serial_port = cf->ReadString(section, "port", DEFAULT_GPS_PORT);
  gps_baud = cf->ReadInt(section, "baud", 4800);

  read_count = 0;
  
  filter_a = 0.80;
  filter_thresh = 1.0;
  filter_lat = 0;
  filter_lon = 0;

  dgps_enable = cf->ReadInt(section, "dgps_enable", 1);
  dgps_group = cf->ReadString(section, "dgps_group", DEFAULT_DGPS_GROUP);
  dgps_port = cf->ReadInt(section, "dgps_port", DEFAULT_DGPS_PORT);

  return;
}


///////////////////////////////////////////////////////////////////////////
int
GarminNMEA::Setup()
{
  // Set up the serial port to talk to the GPS unit
  if (SetupSerial() != 0)
    return -1;

  // Set up the UDP port to get DGPS corrections
  if (SetupSocket() != 0)
    return -1;
  
  puts("Done.");
  
  // start the thread to talk with the GPS unit
  StartThread();

  return(0);
}


///////////////////////////////////////////////////////////////////////////
int
GarminNMEA::Shutdown()
{
  StopThread();

  ShutdownSocket();
  ShutdownSerial();

  return(0);
}


///////////////////////////////////////////////////////////////////////////
// Initialize the serial port
int
GarminNMEA::SetupSerial()
{
  struct termios term;
  int flags;
  int attempt;
  int maxattempts=10;

  printf("GPS connection initializing (%s)...", gps_serial_port);
  fflush(stdout);

  // open it.  non-blocking at first, in case there's no gps unit.
  if((gps_fd = open(gps_serial_port, 
                    O_RDWR | O_SYNC | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0)
  {
    PLAYER_ERROR1("open(): %s\n", strerror(errno));
    return(-1);
  }  

  gps_fd_blocking = false;
 
  if(tcflush(gps_fd, TCIFLUSH ) < 0 )
  {
    PLAYER_ERROR1("tcflush(): %s\n", strerror(errno));
    close(gps_fd);
    gps_fd = -1;
    return(-1);
  }
  if(tcgetattr(gps_fd, &term) < 0 )
  {
    PLAYER_ERROR1("tcgetattr(): %s\n", strerror(errno));
    close(gps_fd);
    gps_fd = -1;
    return(-1);
  }
  
  cfmakeraw(&term);

  if (gps_baud == 9600)
  {
    cfsetispeed(&term, B9600);
    cfsetospeed(&term, B9600);
  }
  else if (gps_baud == 19200)
  {
    cfsetispeed(&term, B19200);
    cfsetospeed(&term, B19200);
  }
  else if (gps_baud == 38400)
  {
    cfsetispeed(&term, B38400);
    cfsetospeed(&term, B38400);
  }
  else if (gps_baud == 57600)
  {
    cfsetispeed(&term, B57600);
	 cfsetospeed(&term, B57600);
  }
  else if (gps_baud == 115200)
  {
    cfsetispeed(&term, B115200);
	 cfsetospeed(&term, B115200);
  }
#ifdef B230400
  else if (gps_baud == 230400)
  {
    cfsetispeed(&term, B230400);
	 cfsetospeed(&term, B230400);
  }
#endif
  else
  {
    cfsetispeed(&term, B4800);
    cfsetospeed(&term, B4800);
  }
  
  if(tcsetattr(gps_fd, TCSAFLUSH, &term) < 0 )
  {
    PLAYER_ERROR1("tcsetattr(): %s\n", strerror(errno));
    close(gps_fd);
    gps_fd = -1;
    return(-1);
  }

  printf("filling buffer\n");
  fflush(stdout);
  
  memset(nmea_buf,0,sizeof(nmea_buf));
  nmea_buf_len=0;
  /* try to read some data, just to make sure we actually have a gps unit */
  for(attempt=0;attempt<maxattempts;attempt++)
  {
    if(!FillBuffer())
      break;
  }
  if(attempt==maxattempts)
  {
    PLAYER_ERROR1("Couldn't connect to GPS unit, most likely because the \n"
                  "unit is not connected or is connected not to %s\n", 
                    gps_serial_port);
    close(gps_fd);
    gps_fd = -1;
    return(-1);
  }

  printf("done filling buffer\n");
  fflush(stdout);

  /* ok, we got data, so now set NONBLOCK, and continue */
  if((flags = fcntl(gps_fd, F_GETFL)) < 0)
  {
    PLAYER_ERROR1("fcntl(): %s\n", strerror(errno));
    close(gps_fd);
    gps_fd = -1;
    return(1);
  }
  if(fcntl(gps_fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    PLAYER_ERROR1("fcntl(): %s\n", strerror(errno));
    close(gps_fd);
    gps_fd = -1;
    return(1);
  }

  gps_fd_blocking = true;

  return 0;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the serial port
void
GarminNMEA::ShutdownSerial()
{
  close(gps_fd);
  gps_fd=-1;
  return;
}


///////////////////////////////////////////////////////////////////////////
// Initialise the UDP socket for recieving DGPS corrections
int
GarminNMEA::SetupSocket()
{
  sockaddr_in addr;
  struct ip_mreq mreq;

  if (!this->dgps_enable)
    return 0;
  
  // Set up the read socket
  this->dgps_fd = socket(PF_INET, SOCK_DGRAM, 0);
  if (this->dgps_fd == -1)
  {
    PLAYER_ERROR1("error initializing socket : %s", strerror(errno));
    return 1;
  }

  // Set socket options to allow sharing of port
  u_int share = 1;
  if (setsockopt(this->dgps_fd, SOL_SOCKET, SO_REUSEADDR,
                 (const char*)&share, sizeof(share)) < 0)
  {
    PLAYER_ERROR1("error initializing socket : %s", strerror(errno));
    return 1;
  }
    
  // Bind socket to port
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(this->dgps_port);
  if (bind(this->dgps_fd, (sockaddr*) &addr, sizeof(addr)) < 0)
  {
    PLAYER_ERROR1("error initializing socket : %s", strerror(errno));
    return 1;
  }

  // Join the multi-cast group
  mreq.imr_multiaddr.s_addr = inet_addr(this->dgps_group);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(this->dgps_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0)
  {
    PLAYER_ERROR1("error joining multicast group : %s", strerror(errno));
    return 1;
  }

  return 0;
}


/*
 * Shutdown the DGPS socket
 */
void
GarminNMEA::ShutdownSocket()
{
  if (!this->dgps_enable)
    return;

  close(this->dgps_fd);

  return;
}


/*
 * Driver thread runs here.  We have to poll on both the serial port
 * and the UDP socket.
 */
void
GarminNMEA::Main()
{
  int len;
  char buf[NMEA_MAX_SENTENCE_LEN];
  char rtcm_buf[1024];
  struct pollfd fds[2];
  int fd_count;

  fd_count = 0;
  
  fds[fd_count].fd = this->gps_fd;
  fds[fd_count].events = POLLIN | POLLPRI | POLLERR | POLLHUP;
  fd_count++;

  if (this->dgps_enable)
  {
    fds[fd_count].fd = this->dgps_fd;
    fds[fd_count].events = POLLIN | POLLPRI | POLLERR | POLLHUP;
    fd_count++;
  }

  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  for(;;)
  {
    pthread_testcancel();

    if (poll(fds, fd_count, 100) < 0)
    {
      PLAYER_ERROR1("poll returned [%s]", strerror(errno));
      continue;
    }

    // Read incoming data from the GPS
    if (fds[0].revents)
    {
      if(ReadSentence(buf,sizeof(buf)))
      {
        PLAYER_ERROR("while reading from GPS unit; bailing");
        pthread_exit(NULL);
      }
      ParseSentence((const char*)buf);
    }

    // Read incoming DGPS corrections from the socket
    if (this->dgps_enable)
    {
      if (fds[1].revents)
      {
        len = ReadSocket(rtcm_buf, sizeof(rtcm_buf));
        if (len < 0)
          pthread_exit(NULL);

        printf("got udp packet of length %d\n", len);
        fflush(stdout);
      
        // Write the DGPS sentence to the GPS unit
        if (WriteSentence(rtcm_buf, len) != 0)
          pthread_exit(NULL);
      }
    }
  }

  return;
}


/*
 * Find a complete NMEA sentence and copy it into 'buf', which should be of
 * length 'len'.  This function will call FillBuffer() as necessary to get
 * enough data into 'nmea_buf' to form a complete sentence. 'buf' will be
 * NULL-terminated.
 *
 */
int
GarminNMEA::ReadSentence(char* buf, size_t len)
{
  char* ptr;
  char* ptr2;
  size_t sentlen;
  char tmp[8];
  int chksum;
  int oursum; 

  //printf("reading sentence\n");
  //fflush(stdout);
  
  while(!(ptr = strchr(nmea_buf, NMEA_START_CHAR)))
  {
    nmea_buf_len=0;
    memset(nmea_buf,0,sizeof(nmea_buf));
    if(FillBuffer())
      return(-1);
  }

  nmea_buf_len = strlen(ptr);
  memmove(nmea_buf,ptr,strlen(ptr)+1);

  //printf("found start char:[%s]:[%d]\n", nmea_buf,nmea_buf_len);
  //fflush(stdout);
  
  while(!(ptr = strchr(nmea_buf, NMEA_END_CHAR)))
  {
    if(nmea_buf_len >= sizeof(nmea_buf) - 1)
    {
      // couldn't get an end char and the buffer is full.
      PLAYER_WARN("couldn't find an end character; discarding data");
      memset(nmea_buf,0,sizeof(nmea_buf));
      nmea_buf_len=0;
      buf = NULL;
      return(0);
    }
    if(FillBuffer())
      return(-1);
  }
  
  //printf("found end char:[%s]\n", nmea_buf);
  //fflush(stdout);
  
  sentlen = nmea_buf_len - strlen(ptr) + 1;
  if(sentlen > len - 1)
  {
    PLAYER_WARN1("NMEA sentence too long (%d bytes); truncating", sentlen);
    sentlen = len - 1;
  }
  
  //printf("reading checksum\n");
  //fflush(stdout);
  
  // copy in all but the leading $ and trailing carriage return and line feed
  if (sentlen > 3)
  {
    strncpy(buf,nmea_buf+1,sentlen-3);
    buf[sentlen-3] = '\0';
  }
  else
  {
    PLAYER_WARN("NMEA sentence is too short; ignoring");
    buf[0] = '\0';
  }

  //printf("got: [%s]\n", buf);
  //fflush(stdout);

  // verify the checksum, if present.  two hex digits are the XOR of all the 
  // characters between the $ and *.
  if((ptr2 = strchr(buf,NMEA_CHKSUM_CHAR)) && (strlen(ptr2) == 3))
  {
    ////printf("ptr2 %s\n", ptr2);
    ////fflush(stdout);

    strncpy(tmp,ptr2+1,2);
    tmp[2]='\0';
    chksum = strtol(tmp,NULL,16);
    
    oursum=0;
    for(int i=0;i<(int)(strlen(buf)-strlen(ptr2));i++)
      oursum ^= buf[i];

    // HACK
    //chksum = oursum;

    if(oursum != chksum)
    {
      PLAYER_WARN2("checksum mismatch (0x%2x != 0x%2x); discarding sentence",
                   oursum, chksum);
      buf=NULL;
    }
    else
    {
      // strip off checksum
      *ptr2='\0';
    }
  }
  else
  {
    PLAYER_WARN1("no checksum: [%s]", buf);
    buf = NULL;
  }

  memmove(nmea_buf,ptr+1,strlen(ptr));
  nmea_buf_len -= sentlen;
  nmea_buf[nmea_buf_len]='\0';

  //printf("done reading sentence\n");
  //fflush(stdout);

  return(0);
}


/*
 * Read more data into the buffer 'nmea_buf', starting 'nmea_buf_len' chars
 * in, and not overrunning the total length.  'nmea_buf' will be
 * NULL-terminated.
 */
int
GarminNMEA::FillBuffer()
{
  int numread=0;
  int readcnt=0;

  while(numread<=0)
  {
    if((numread = read(gps_fd,nmea_buf+nmea_buf_len,
                       sizeof(nmea_buf)-nmea_buf_len-1)) < 0)
    {      
      if(!gps_fd_blocking && (errno == EAGAIN))
      {
        if(readcnt >= GPS_STARTUP_CYCLES)
          return(-1);
        else
        {
          readcnt++;
          usleep(GPS_STARTUP_CYCLE_USEC);
        }
      }
      else
      {
        PLAYER_ERROR1("read(): %s", strerror(errno));
        return(-1);
      }
    }
  }
  nmea_buf_len += numread;
  nmea_buf[nmea_buf_len] = '\0';

  /*
  for (int i = 0; i < nmea_buf_len; i++)
    printf("%02X ", (int) (unsigned char) nmea_buf[i]);
  printf("\n");
  */
  
  return(0);
}


/*
 * Write a sentence to the GPS unit
 */
int
GarminNMEA::WriteSentence(const char *buf, size_t len)
{
  int s;
  size_t sent, remaining;
  
	sent = 0;
  remaining = len;

  while (sent < len)
  {
    s = ::write(this->gps_fd, buf + sent, remaining);
    if (s < 0)
    {
      PLAYER_ERROR1("error writing to GPS [%s]", strerror(errno));
      return -1;
    }
    sent += s;
    remaining -= s;
	}

  return 0;
}


/*
 * Get the next field from an NMEA sentence.
 */
const char*
GarminNMEA::GetNextField(char* field, size_t len, const char* ptr)
{
  const char* start;
  const char* end;
  size_t fieldlen;

  if(strlen(ptr) < 2 || !(start = strchr(ptr, ',')))
  {
    field[0]='\0';
    return(NULL);
  }

  if(!(end = strchr(start+1, ',')))
    fieldlen = strlen(ptr) - (start - ptr);
  else
    fieldlen = end - start - 1;

  if(fieldlen > (len - 1))
  {
    PLAYER_WARN("NMEA field too big; truncating");
    fieldlen = len - 1;
  }

  strncpy(field,start+1,fieldlen);
  field[fieldlen] = '\0';

  return(end);
}


/*
 * Parse an NMEA sentence, doing something appropriate with each message in
 * which we're interested. 'buf' should be NULL-terminated.
 */
int
GarminNMEA::ParseSentence(const char* buf)
{
  const char* ptr = buf;
  char tmp[8];
  
  if(!buf)
    return(0);
  if (strlen(buf) < 5)
    return 0;

  // copy in the sentence header, for checking
  strncpy(tmp,buf,5);
  tmp[5]='\0';

  //printf("sentence [%s]\n", tmp);
  //fflush(stdout);
      
  // the GGA msg has the position data that we want
  if(!strcmp(tmp,NMEA_GPGGA))
    ParseGPGGA(ptr);

  // the RMC msg has the date and time
  if(!strcmp(tmp,NMEA_GPRMC))
    ParseGPRMC(ptr);

  // the PGRME msg has the error (Garmin)
  if(!strcmp(tmp,NMEA_PGRME))
    ParsePGRME(ptr);

  // the GPGST msg has the error (NMEA)
  if(!strcmp(tmp,NMEA_GPGST))
    ParseGPGST(ptr);

  return(0);
}


/*
 * Parse the GPGGA sentence, which has lat/lon.
 */
int GarminNMEA::ParseGPGGA(const char *buf)
{
  const char *ptr = buf;
  char field[32];
  char tmp[8];
  double degrees, minutes, arcseconds;
  double lat, lon;
  double utm_e, utm_n;
  
  //printf("got GGA (%s)\n", buf);
  //fflush(stdout);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // first field is time of day. Skip
  
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // 2nd field is latitude.  first two chars are degrees.
  strncpy(tmp,field,2);
  tmp[2]='\0';
  degrees = atoi(tmp);
  // next is minutes
  minutes = atof(field+2);

  arcseconds = ((degrees * 60.0) + minutes) * 60.0;

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 3rd field is N or S for north or south. adjust sign accordingly.
  if(field[0] == 'S')
    arcseconds *= -1;

  lat = arcseconds / 3600.0;
  data.latitude = (int32_t)rint(lat * 1e7);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 4th field is longitude.  first 3 chars are degrees.
  strncpy(tmp,field,3);
  tmp[3]='\0';
  degrees = atoi(tmp);
  // next is minutes
  minutes = atof(field+3);

  arcseconds = ((degrees * 60.0) + minutes) * 60.0;
    
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 5th field is E or W for east or west. adjust sign accordingly.
  if(field[0] == 'W')
    arcseconds *= -1;

  lon = arcseconds / 3600.0;
  data.longitude = (int32_t)rint(lon * 1e7);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 6th field is fix quality
  data.quality = atoi(field);
  
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 7th field is number of sats in view
  data.num_sats = atoi(field);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 8th field is HDOP.  we'll multiply by ten to make it an integer.
  data.hdop = (uint16_t)rint(atof(field) * 10);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 9th field is altitude, in meters.  we'll convert to mm.
  data.altitude = (int32_t)rint(atof(field) * 1000.0);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 10th field tells us the reference for measuring altitude. e.g., 'M' is
  // mean sea level.  ignore it.

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 11th field is "height of geoid above WGS84 ellipsoid" ignore it.

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  // 12th field tells us the reference for the above-mentioned geode.  e.g.,
  // 'M' is mean sea level.  ignore it.

  // fields 13 and 14 are for DGPS. ignore them.


  ////printf("%f %f %d\n", lat, lon, (int) data.quality);

  // Update the filtered lat/lon, and see if the new values are any
  // good
  filter_lat = filter_a * lat + (1 - filter_a) * filter_lat;
  filter_lon = filter_a * lon + (1 - filter_a) * filter_lon;

  // Reject outliers
  filter_good = true;
  if (fabs(lat - filter_lat) > filter_thresh)
    filter_good = false;
  if (fabs(lon - filter_lon) > filter_thresh)
    filter_good = false;

  if (!filter_good)
  {
    PLAYER_WARN4("rejected: %f %f (should be %f %f)\n", lat, lon, filter_lat, filter_lon);
    return -1;
  }
  
  // Compute the UTM coordindates
  UTM(lat, lon, &utm_e, &utm_n);
  //printf("utm: %.3f %.3f\n", utm_e, utm_n);
  
  data.utm_e = (int32_t) rint(utm_e * 100);
  data.utm_n = (int32_t) rint(utm_n * 100);

  Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_GPS_DATA_STATE, &data,sizeof(player_gps_data_t),NULL);

  return 0;
}


/*
 * Parse the GPRMC sentence, which has date/time
 */
int GarminNMEA::ParseGPRMC(const char *buf)
{
  const char *ptr = buf;
  char field[32];
  char tmp[8];
  struct tm tms;
  time_t utc;
  
  //printf("got RMC (%s)\n", buf);
  //fflush(stdout);
  
  memset(&tms, 0, sizeof(tms));
      
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  if (strlen(field) < 6)
  {
    PLAYER_WARN("short time field; ignoring");
    return -1;
  }

  // first field is time of day. HHMMSS
  strncpy(tmp,field,2);
  tmp[2]='\0';
  tms.tm_hour = atoi(tmp);

  strncpy(tmp,field + 2,2);
  tmp[2]='\0';
  tms.tm_min = atoi(tmp);

  strncpy(tmp,field + 4,2);
  tmp[2]='\0';
  tms.tm_sec = atoi(tmp);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  if (strlen(field) < 6)
  {
    PLAYER_WARN("short date field; ignoring");
    return -1;
  }
  
  // fifth field has the date DDMMYY
  strncpy(tmp,field,2);
  tmp[2]='\0';
  tms.tm_mday = atoi(tmp);
  
  strncpy(tmp,field + 2,2);
  tmp[2]='\0';
  tms.tm_mon = atoi(tmp) - 1;
  
  strncpy(tmp,field + 4,2);
  tmp[2]='\0';
  tms.tm_year = 100 + atoi(tmp);
  
  //printf("%02d %02d %02d : %02d %02d %02d \n",
  //       tms.tm_year, tms.tm_mon, tms.tm_mday,
  //       tms.tm_hour, tms.tm_min, tms.tm_sec);
  
  // Compute to time since the epoch.  We only get it to the nearest
  // second, unfortunately.
  utc = mktime(&tms);
  
  data.time_sec = (uint32_t) utc;
  data.time_usec = (uint32_t) 0;

  /* Dont write here
  // Need to parse to sentences before write data
  read_count += 1;
  if (filter_good && filter_good >= 2)
  {
    PutData(&data,sizeof(player_gps_data_t),0,0);
    read_count = 0;
  }
  */
  
  return 0;
}


/*
 * Parse the PGRME sentence, which has esimated position error.
 * This is a proprietry Garmin message
 */
int GarminNMEA::ParsePGRME(const char *buf)
{
  const char *ptr = buf;
  char field[32];
  double err;
  
  //printf("got PGRME (%s)\n", buf);
  //fflush(stdout);
  
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // First field is horizontal error
  err = atof(field);
  data.err_horz = (uint32_t) (err * 1000);
  
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // Should be "M"
  if (strcmp(field, "M") != 0)
  {
    PLAYER_WARN1("invalid unit code [%s]", field);
    return -1;
  }

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // Third field is vertical error
  err = atof(field);
  data.err_vert = (uint32_t) (err * 1000);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);

  // Should be "M"
  if (strcmp(field, "M") != 0)
  {
    PLAYER_WARN1("invalid unit code [%s]", field);
    return -1;
  }

  /* Dont write here
  PutData(&data,sizeof(player_gps_data_t),0,0);
  */

  return 0;
}

/*
 * Parse the GPGST sentence, which has esimated position error.
 */
int GarminNMEA::ParseGPGST(const char *buf)
{
  const char *ptr = buf;
  char field[32];
  double err;
  double utc;
  
//  printf("got GPGST (%s)\n", buf);
//  fflush(stdout);
  
  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
    return(-1);
  
  // First field is UTC time
  utc = atof(field);
  /* Maybe do a check to make sure current packet is the right packet */

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
	 return(-1);
  
  // Second field is RMS deviation
  err = atof(field);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
	 return(-1);
  
  // Third field is Semi-minor deviation
  err = atof(field);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
	 return(-1);
  
  // Fourth field is Semi-major deviation
  err = atof(field);

  if(!(ptr = GetNextField(field, sizeof(field), ptr)))
	 return(-1);
  
  // Fifth field is Latitude error deviation
  err = atof(field);
  data.err_horz = (uint32_t) ( err * 1000);


  // Sixth field is Longitude error deviation
  err = atof(field);
  data.err_vert = (uint32_t) ( err * 1000);

  // Seventh field is Altitude error deviation
  err = atof(field);

  /* Dont write here
  PutData(&data,sizeof(player_gps_data_t),0,0);
  */

  return 0;
}



/*
 * Read DGPS sentence from the UDP socket
 */
int GarminNMEA::ReadSocket(char *buf, size_t len)
{
  int plen;
  
  plen = recv(this->dgps_fd, buf, len, 0);
  if (plen < 0)
  {
    PLAYER_ERROR1("error reading from udp socket [%s]", strerror(errno));
    return -1;
  }
  return plen;
}


/*
 * Utility functions to convert geodetic to UTM position
 */
void GarminNMEA::UTM(double lat, double lon, double *x, double *y)
{
	// constants
	const static double m0 = (1 - UTM_E2/4 - 3*UTM_E4/64 - 5*UTM_E6/256);
	const static double m1 = -(3*UTM_E2/8 + 3*UTM_E4/32 + 45*UTM_E6/1024);
	const static double m2 = (15*UTM_E4/256 + 45*UTM_E6/1024);
	const static double m3 = -(35*UTM_E6/3072);

	// compute the central meridian
	int cm = (lon >= 0.0) ? ((int)lon - ((int)lon)%6 + 3) : ((int)lon - ((int)lon)%6 - 3);

	// convert degrees into radians
	double rlat = lat * M_PI/180;
	double rlon = lon * M_PI/180;
	double rlon0 = cm * M_PI/180;

	// compute trigonometric functions
	double slat = sin(rlat);
	double clat = cos(rlat);
	double tlat = tan(rlat);

	// decide the flase northing at origin
	double fn = (lat > 0) ? UTM_FN_N : UTM_FN_S;

	double T = tlat * tlat;
	double C = UTM_EP2 * clat * clat;
	double A = (rlon - rlon0) * clat;
	double M = WGS84_A * (m0*rlat + m1*sin(2*rlat) + m2*sin(4*rlat) + m3*sin(6*rlat));
	double V = WGS84_A / sqrt(1 - UTM_E2*slat*slat);

	// compute the easting-northing coordinates
	*x = UTM_FE + UTM_K0 * V * (A + (1-T+C)*pow(A,3)/6 + (5-18*T+T*T+72*C-58*UTM_EP2)*pow(A,5)/120);
	*y = fn + UTM_K0 * (M + V * tlat * (A*A/2 + (5-T+9*C+4*C*C)*pow(A,4)/24 + (61-58*T+T*T+600*C-330*UTM_EP2)*pow(A,6)/720));

  return;
}
