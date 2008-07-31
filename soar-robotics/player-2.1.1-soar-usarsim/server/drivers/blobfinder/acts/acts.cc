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

/*
 * $Id: acts.cc 6499 2008-06-10 01:13:51Z thjc $
 *
 *   the P2 vision device.  it takes pan tilt zoom commands for the
 *   sony PTZ camera (if equipped), and returns color blob data gathered
 *   from ACTS, which this device spawns and then talks to.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_acts acts
 * @brief ActivMedia color tracking system

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

ACTS is a fast color segmentation system written by Paul Rybski and sold
by <a href=http://www.activrobots.com>ActivMedia</a>.  After training,
ACTS finds colored blobs in a single camera image.  Player's acts driver
starts and controls ACTS, which must already be installed and trained.

The acts driver supports ACTS versions 1.0, 1.2, and 2.0.

PXC200 framegrabbers, when accessed through the bttv module, may cause
the machine to hang.  A workaround is to first read a frame from a
framegrabber channel on which there is no video signal, and then start
reading from the right channel.  (This problem is unrelated to Player's
acts driver).

@par Compile-time dependencies

- none

@par Provides

- @ref interface_blobfinder

@par Requires

- none

@par Configuration requests

- none

@par Configuration file options

In the list below, a default value of (none) indicates that the associated
option will not be passed to ACTS.  As a result, ACTS's own internal
default for that parameter will be used.  Consult the ACTS manual to
determine what those defaults are.

- path (string)
  - Default: ""
  - Path to the ACTS executable (leave empty to search the user's PATH
    for acts).
- configfile (string)
  - Default: "/usr/local/acts/actsconfig"
  - Path to the ACTS configuration file to be used.
- version (string)
  - Default: "2.0"
  - The version of ACTS in use (should be "1.0", "1.2", or "2.0")
- width (integer)
  - Default: 160
  - Width of the camera image (in pixels)
- height (integer)
  - Default: 120
  - Height of the camera image (in pixels)
- pixels (integer)
  - Default: (none)
  - Minimum area required to call a blob a blob (in pixels)
- port (integer)
  - Default: 5001
  - TCP port by which Player should connect to ACTS.
- fps (integer)
  - Default: (none)
  - Frame per second of the camera.
- drivertype (string)
  - Default: (none)
  - Type of framegrabber driver in use (e.g., "bttv", "bt848", "matrox").
- invert (integer)
  - Default: (none)
  - Is the camera inverted?
- devicepath (string)
  - Default: (none)
  - Path to the device file for the framegrabber (e.g., "/dev/fg0")
- channel (integer)
  - Default: (none)
  - Which channel to select on the framegrabber.
- norm (string)
  - Default: (none)
  - Something to do with normalization
- pxc200 (integer)
  - Default: (none)
  - Is the framegrabber a PXC200?
- brightness (float)
  - Default: (none)
  - Brightness level
- contrast (float)
   - Default: (none)
   - Contrast level

@par Example

@verbatim
driver
(
  name "acts"
  provides ["blobfinder:0"]
  configfile "/tmp/myactsconfig"
)
@endverbatim

@author Brian Gerkey

*/

/** @} */

#include <assert.h>
#include <stdio.h>
#include <unistd.h> /* close(2),fcntl(2),getpid(2),usleep(3),execvp(3),fork(2)*/
#include <netdb.h> /* for gethostbyname(3) */
#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */
#include <sys/types.h>  /* for socket(2) */
#include <sys/socket.h>  /* for socket(2) */
#include <signal.h>  /* for kill(2) */
#include <fcntl.h>  /* for fcntl(2) */
#include <string.h>  /* for strncpy(3),memcpy(3) */
#include <stdlib.h>  /* for atexit(3),atoi(3) */
#include <pthread.h>  /* for pthread stuff */
//#include <socket_util.h>

#include <libplayercore/playercore.h>

#define ACTS_NUM_CHANNELS 32
#define ACTS_HEADER_SIZE_1_0 2*ACTS_NUM_CHANNELS
#define ACTS_HEADER_SIZE_1_2 4*ACTS_NUM_CHANNELS
#define ACTS_BLOB_SIZE_1_0 10
#define ACTS_BLOB_SIZE_1_2 16
#define ACTS_MAX_BLOBS_PER_CHANNEL 10

#define ACTS_VERSION_1_0_STRING "1.0"
#define ACTS_VERSION_1_2_STRING "1.2"
#define ACTS_VERSION_2_0_STRING "2.0"

#define DEFAULT_ACTS_PORT 5001

/* a variable of this type tells the vision device how to interact with ACTS */
typedef enum
{
  ACTS_VERSION_UNKNOWN = 0,
  ACTS_VERSION_1_0 = 1,
  ACTS_VERSION_1_2 = 2,
  ACTS_VERSION_2_0 = 3
} acts_version_t;
/* default is to use older ACTS (until we change our robots) */
#define DEFAULT_ACTS_VERSION ACTS_VERSION_1_0
#define DEFAULT_ACTS_CONFIGFILE "/usr/local/acts/actsconfig"
/* default is to give no path for the binary; in this case, use execvp()
 * and user's PATH */
#define DEFAULT_ACTS_PATH ""
#define DEFAULT_ACTS_WIDTH 160
#define DEFAULT_ACTS_HEIGHT 120
/********************************************************************/


/** The maximum number of unique color classes. */
#define ACTS_MAX_CHANNELS 32

/** The maximum number of blobs for each color class. */
#define ACTS_MAX_BLOBS_PER_CHANNEL 10

/** The maximum number of blobs in total. */
#define ACTS_MAX_BLOBS ACTS_MAX_CHANNELS * ACTS_MAX_BLOBS_PER_CHANNEL

/** Blob index entry. */
typedef struct acts_header_elt
{
  /** Offset of the first blob for this channel. */
  uint16_t index;

  /** Number of blobs for this channel. */
  uint16_t num;

} acts_header_elt_t;


/** Structure describing a single blob. */
typedef struct acts_blob_elt
{
  /** A descriptive color for the blob (useful for gui's).  The color
      is stored as packed 32-bit RGB, i.e., 0x00RRGGBB. */
  uint32_t color;

  /** The blob area (pixels). */
  uint32_t area;

  /** The blob centroid (image coords). */
  uint16_t x, y;

  /** Bounding box for the blob (image coords). */
  uint16_t left, right, top, bottom;

} acts_blob_elt_t;


/** The list of detected blobs. */
typedef struct acts_data
{
  /** The image dimensions. */
  uint16_t width, height;

  /** An index into the list of blobs (blobs are indexed by channel). */
  acts_header_elt_t header[ACTS_MAX_CHANNELS];

  /** The list of blobs. */
  acts_blob_elt_t blobs[ACTS_MAX_BLOBS];

} acts_data_t;



class Acts:public Driver
{
  private:
    int debuglevel;             // debuglevel 0=none, 1=basic, 2=everything
    int pid;      // ACTS's pid so we can kill it later


    // returns the enum representation of the given version string, or
    // ACTS_VERSION_UNKNOWN on failure to match.
    acts_version_t version_string_to_enum(const char* versionstr);

    // writes the string representation of the given version number into
    // versionstr, up to len.
    // returns  0 on success
    //         -1 on failure to match.
    int version_enum_to_string(acts_version_t versionnum, char* versionstr,
                               int len);

    // stuff that will be used on the cmdline to start ACTS
    acts_version_t acts_version;  // the ACTS version, as an enum
    char binarypath[MAX_FILENAME_SIZE];  // path to executable
    char configfilepath[MAX_FILENAME_SIZE];  // path to configfile (-t)
    char minarea[8];  // min num of pixels for tracking (-w)
    int portnum;  // port number where we'll connect to ACTS (-p)
    char fps[8]; // capture rate (-R)
    char drivertype[8]; // e.g., bttv, bt848 (-G)
    char invertp; // invert the image? (-i)
    char devicepath[MAX_FILENAME_SIZE]; // e.g., /dev/video0 (-d)
    char channel[8]; // channel to use (-n)
    char norm[8]; // PAL or NTSC (-V)
    char pxc200p; // using PXC200? (-x)
    char brightness[8]; // brightness of image (-B)
    char contrast[8]; // contrast of image (-C)
    int width, height;  // the image dimensions. (-W, -H)

    int header_len; // length of incoming packet header (varies by version)
    int header_elt_len; // length of each header element (varies by version)
    int blob_size;  // size of each incoming blob (varies by version)
    char portnumstring[128]; // string version of portnum
    char widthstring[128];
    char heightstring[128];

    // Descriptive colors for each channel.
    uint32_t colors[ACTS_MAX_CHANNELS];

  public:
    int sock;               // socket to ACTS

    // constructor
    //
    Acts( ConfigFile* cf, int section);

    virtual void Main();

    void KillACTS();

    int Setup();
    int Shutdown();
};

// a factory creation function
Driver* Acts_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new Acts( cf, section)));
}

// a driver registration function
void
Acts_Register(DriverTable* table)
{
  table->AddDriver("acts", Acts_Init);
}

#define ACTS_REQUEST_QUIT '1'
#define ACTS_REQUEST_PACKET '0'

/* the following setting mean that we first try to connect after 1 seconds,
 * then try every 100ms for 6 more seconds before giving up */
#define ACTS_STARTUP_USEC 1000000 /* wait before first connection attempt */
#define ACTS_STARTUP_INTERVAL_USEC 100000 /* wait between connection attempts */
#define ACTS_STARTUP_CONN_LIMIT 60 /* number of attempts to make */


void QuitACTS(void* visiondevice);


Acts::Acts( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_BLOBFINDER_CODE)
{
  char tmpstr[MAX_FILENAME_SIZE];
  int tmpint;
  double tmpfloat;
  int ch;
  uint32_t color;

  sock = -1;

  // first, get the necessary args
  strncpy(binarypath,
          cf->ReadFilename(section, "path", DEFAULT_ACTS_PATH),
          sizeof(binarypath));
  strncpy(configfilepath,
          cf->ReadFilename(section, "configfile", DEFAULT_ACTS_CONFIGFILE),
          sizeof(configfilepath));
  strncpy(tmpstr,
          cf->ReadString(section, "version", ACTS_VERSION_2_0_STRING),
          sizeof(tmpstr));
  if((acts_version = version_string_to_enum(tmpstr)) == ACTS_VERSION_UNKNOWN)
  {
    PLAYER_WARN2("unknown version \"%s\"; using default \"%s\"",
                 tmpstr, ACTS_VERSION_1_0_STRING);
    acts_version = version_string_to_enum(ACTS_VERSION_2_0_STRING);
  }
  width = cf->ReadInt(section, "width", DEFAULT_ACTS_WIDTH);
  height = cf->ReadInt(section, "height", DEFAULT_ACTS_HEIGHT);

  // now, get the optionals
  memset(minarea,0,sizeof(minarea));
  if((tmpint = cf->ReadInt(section, "pixels", -1)) >= 0)
    snprintf(minarea,sizeof(minarea),"%d",tmpint);
  portnum = cf->ReadInt(section, "port", DEFAULT_ACTS_PORT);
  memset(fps,0,sizeof(fps));
  if((tmpint = cf->ReadInt(section, "fps", -1)) >= 0)
    snprintf(fps,sizeof(fps),"%d",tmpint);
  memset(drivertype,0,sizeof(drivertype));
  if(cf->ReadString(section, "drivertype", NULL))
    strncpy(drivertype, cf->ReadString(section, "drivertype", NULL),
            sizeof(drivertype)-1);
  invertp = cf->ReadInt(section, "invert", -1);
  memset(devicepath,0,sizeof(devicepath));
  if(cf->ReadString(section, "devicepath", NULL))
    strncpy(devicepath, cf->ReadString(section, "devicepath", NULL),
            sizeof(devicepath)-1);
  memset(channel,0,sizeof(channel));
  if((tmpint = cf->ReadInt(section, "channel", -1)) >= 0)
    snprintf(channel,sizeof(channel),"%d",tmpint);
  memset(norm,0,sizeof(norm));
  if(cf->ReadString(section, "norm", NULL))
    strncpy(norm, cf->ReadString(section, "norm", NULL),
            sizeof(norm)-1);
  pxc200p = cf->ReadInt(section, "pxc200", -1);
  memset(brightness,0,sizeof(brightness));
  if((tmpfloat = cf->ReadFloat(section, "brightness", -1)) >= 0)
    snprintf(brightness,sizeof(brightness),"%.3f",tmpfloat);
  memset(contrast,0,sizeof(contrast));
  if((tmpfloat = cf->ReadFloat(section, "contrast", -1)) >= 0)
    snprintf(contrast,sizeof(brightness),"%.3f",tmpfloat);

  // set up some version-specific parameters
  switch(acts_version)
  {
    case ACTS_VERSION_1_0:
      header_len = ACTS_HEADER_SIZE_1_0;
      blob_size = ACTS_BLOB_SIZE_1_0;
      // extra byte-swap cause ACTS 1.0 got it wrong
      portnum = htons(portnum);
      break;
    case ACTS_VERSION_1_2:
    case ACTS_VERSION_2_0:
    default:
      header_len = ACTS_HEADER_SIZE_1_2;
      blob_size = ACTS_BLOB_SIZE_1_2;
      break;
  }
  header_elt_len = header_len / ACTS_MAX_CHANNELS;
  memset(portnumstring, 0, sizeof(portnumstring));
  snprintf(portnumstring,sizeof(portnumstring),"%d",portnum);

  memset(widthstring, 0, sizeof(widthstring));
  snprintf(widthstring,sizeof(widthstring),"%d",width);

  memset(heightstring, 0, sizeof(heightstring));
  snprintf(heightstring,sizeof(heightstring),"%d",height);


  // Get the descriptive colors.
  for (ch = 0; ch < ACTS_MAX_CHANNELS; ch++)
  {
    color = cf->ReadTupleColor(section, "colors", ch, 0xFFFFFFFF);
    if (color == 0xFFFFFFFF)
      break;
    this->colors[ch] = color;
  }
}

// returns the enum representation of the given version string, or
// -1 on failure to match.
acts_version_t Acts::version_string_to_enum(const char* versionstr)
{
  if(!strcmp(versionstr,ACTS_VERSION_1_0_STRING))
    return(ACTS_VERSION_1_0);
  else if(!strcmp(versionstr,ACTS_VERSION_1_2_STRING))
    return(ACTS_VERSION_1_2);
  else if(!strcmp(versionstr,ACTS_VERSION_2_0_STRING))
    return(ACTS_VERSION_2_0);
  else
    return(ACTS_VERSION_UNKNOWN);
}

// writes the string representation of the given version number into
// versionstr, up to len.
// returns  0 on success
//         -1 on failure to match.
int Acts::version_enum_to_string(acts_version_t versionnum,
                                          char* versionstr, int len)
{
  switch(versionnum)
  {
    case ACTS_VERSION_1_0:
      strncpy(versionstr, ACTS_VERSION_1_0_STRING, len);
      return(0);
    case ACTS_VERSION_1_2:
      strncpy(versionstr, ACTS_VERSION_1_2_STRING, len);
      return(0);
    case ACTS_VERSION_2_0:
      strncpy(versionstr, ACTS_VERSION_2_0_STRING, len);
      return(0);
    default:
      return(-1);
  }
}

int
Acts::Setup()
{
  int i;
  int j;

  char acts_bin_name[] = "acts";
  //char acts_configfile_flag[] = "-t";
  //char acts_port_flag[] = "-s";

  char* acts_args[32];

  static struct sockaddr_in server;
  char host[] = "localhost";
  struct hostent* entp;

  printf("ACTS vision server connection initializing...");
  fflush(stdout);

  /* REMOVE
  player_blobfinder_data_t dummy;
  memset(&dummy,0,sizeof(dummy));
  // zero the data buffer
  PutData((unsigned char*)&dummy,
          sizeof(dummy.width)+sizeof(dummy.height)+sizeof(dummy.header),NULL);
  */

  i = 0;
  acts_args[i++] = acts_bin_name;
  // build the argument list, based on version
  switch(acts_version)
  {
    // these are needed as execv expects a const array of char *'s not an array of const char *'s
    static char dash_d[3] = "-d";
    static char dash_i[3] = "-i";
    static char dash_n[3] = "-n";
    static char dash_p[3] = "-p";
    static char dash_s[3] = "-s";
    static char dash_t[3] = "-t";
    static char dash_w[3] = "-w";
    static char dash_x[3] = "-x";

    static char dash_B[3] = "-B";
    static char dash_C[3] = "-C";
    static char dash_G[3] = "-G";
    static char dash_H[3] = "-H";
    static char dash_R[3] = "-R";
    static char dash_V[3] = "-V";
    static char dash_W[3] = "-W";

  case ACTS_VERSION_1_0:
      acts_args[i++] = dash_t;
      acts_args[i++] = configfilepath;
      if(strlen(portnumstring))
      {
        acts_args[i++] = dash_s;
        acts_args[i++] = portnumstring;
      }
      if(strlen(devicepath))
      {
        acts_args[i++] = dash_d;
        acts_args[i++] = devicepath;
      }
      break;
    case ACTS_VERSION_1_2:
      acts_args[i++] = dash_t;
      acts_args[i++] = configfilepath;
      if(strlen(portnumstring))
      {
        acts_args[i++] = dash_p;
        acts_args[i++] = portnumstring;
      }
      if(strlen(devicepath))
      {
        acts_args[i++] = dash_d;
        acts_args[i++] = devicepath;
      }
      if(strlen(contrast))
      {
        acts_args[i++] = dash_C;
        acts_args[i++] = contrast;
      }
      if(strlen(brightness))
      {
        acts_args[i++] = dash_B;
        acts_args[i++] = brightness;
      }
      acts_args[i++] = dash_W;
      acts_args[i++] = widthstring;
      acts_args[i++] = dash_H;
      acts_args[i++] = heightstring;
      break;
    case ACTS_VERSION_2_0:
      acts_args[i++] = dash_t;
      acts_args[i++] = configfilepath;
      if(strlen(minarea))
      {
        acts_args[i++] = dash_w;
        acts_args[i++] = minarea;
      }
      if(strlen(portnumstring))
      {
        acts_args[i++] = dash_p;
        acts_args[i++] = portnumstring;
      }
      if(strlen(fps))
      {
        acts_args[i++] = dash_R;
        acts_args[i++] = fps;
      }
      if(strlen(drivertype))
      {
        acts_args[i++] = dash_G;
        acts_args[i++] = drivertype;
      }
      if(invertp > 0)
        acts_args[i++] = dash_i;
      if(strlen(devicepath))
      {
        acts_args[i++] = dash_d;
        acts_args[i++] = devicepath;
      }
      if(strlen(channel))
      {
        acts_args[i++] = dash_n;
        acts_args[i++] = channel;
      }
      if(strlen(norm))
      {
        acts_args[i++] = dash_V;
        acts_args[i++] = norm;
      }
      if(pxc200p > 0)
        acts_args[i++] = dash_x;
      if(strlen(brightness))
      {
        acts_args[i++] = dash_B;
        acts_args[i++] = brightness;
      }
      if(strlen(contrast))
      {
        acts_args[i++] = dash_C;
        acts_args[i++] = contrast;
      }
      acts_args[i++] = dash_W;
      acts_args[i++] = widthstring;
      acts_args[i++] = dash_H;
      acts_args[i++] = heightstring;
      break;
    case ACTS_VERSION_UNKNOWN:
    default:
      PLAYER_ERROR("unknown ACTS version!");
      return(-1);
      break;
  }
  acts_args[i] = (char*)NULL;

  assert((unsigned int)i <= sizeof(acts_args) / sizeof(acts_args[0]));

  printf("\ninvoking ACTS with:\n\n    ");
  for(int j=0;acts_args[j];j++)
    printf("%s ", acts_args[j]);
  puts("\n");

  if(!(pid = fork()))
  {
    // make sure we don't get that "ACTS: Packet" bullshit on the console
    //int dummy_fd = open("/dev/null",O_RDWR);
    //dup2(dummy_fd,0);
    //dup2(dummy_fd,1);
    //dup2(dummy_fd,2);

    /* detach from controlling tty, so we don't get pesky SIGINTs and such */
    if(setpgid(0,0) == -1)
    {
      perror("Acts:Setup(): error while setpgrp()");
      exit(1);
    }

    // if no path to the binary was given, search the user's PATH
    if(!strlen(binarypath))
    {
      if(execvp(acts_bin_name,acts_args) == -1)
      {
        /*
        * some error.  print it here.  it will really be detected
        * later when the parent tries to connect(2) to it
         */
        perror("Acts:Setup(): error while execvp()ing ACTS");
        exit(1);
      }
    }
    else
    {
      if(execv(binarypath,acts_args) == -1)
      {
        /*
        * some error.  print it here.  it will really be detected
        * later when the parent tries to connect(2) to it
         */
        perror("Acts:Setup(): error while execv()ing ACTS");
        exit(1);
      }
    }
  }
  else
  {
    /* in parent */
    /* fill in addr structure */
    server.sin_family = PF_INET;
    /*
     * this is okay to do, because gethostbyname(3) does no lookup if the
     * 'host' * arg is already an IP addr
     */
    if((entp = gethostbyname(host)) == NULL)
    {
      fprintf(stderr, "Acts::Setup(): \"%s\" is unknown host; "
                      "can't connect to ACTS\n", host);
      /* try to kill ACTS just in case it's running */
      KillACTS();
      return(1);
    }

    memcpy(&server.sin_addr, entp->h_addr_list[0], entp->h_length);

    server.sin_port = htons(portnum);

    /* ok, we'll make this a bit smarter.  first, we wait a baseline amount
     * of time, then try to connect periodically for some predefined number
     * of times
     */
    usleep(ACTS_STARTUP_USEC);

    for(j = 0;j<ACTS_STARTUP_CONN_LIMIT;j++)
    {
      /*
       * hook it up
       */

      // make a new socket, because connect() screws with the old one somehow
      if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
      {
        perror("Acts::Setup(): socket(2) failed");
        KillACTS();
        return(1);
      }
      if(connect(sock,(struct sockaddr*)&server, sizeof(server)) == 0)
        break;
      usleep(ACTS_STARTUP_INTERVAL_USEC);
    }
    if(j == ACTS_STARTUP_CONN_LIMIT)
    {
      perror("Acts::Setup(): connect(2) failed");
      KillACTS();
      return(1);
    }
    puts("Done.");

    /* now spawn reading thread */
    StartThread();

    return(0);
  }

  // shut up compiler!
  return(0);
}

int
Acts::Shutdown()
{
  /* if Setup() was never called, don't do anything */
  if(sock == -1)
    return(0);

  StopThread();

  sock = -1;
  puts("ACTS vision server has been shutdown");
  return(0);
}

void
Acts::KillACTS()
{
  if(kill(pid,SIGKILL) == -1)
    perror("Acts::KillACTS(): some error while killing ACTS");
}

void
Acts::Main()
{
  int numread;
  int num_blobs;
  int i;

  // we'll convert the data into this structured buffer
  acts_data_t acts_data;

  // we'll write the data from this buffer
  player_blobfinder_data_t local_data;

  acts_blob_elt_t *src;
  player_blobfinder_blob_t *dst;

  // AH: this can't be very safe (buffer sizes)
  // first, we'll read into these two temporary buffers
  uint8_t acts_hdr_buf[sizeof(acts_data.header)];
  uint8_t acts_blob_buf[sizeof(acts_data.blobs)];

  char acts_request_packet = ACTS_REQUEST_PACKET;

  /* make sure we kill ACTS on exiting */
  pthread_cleanup_push(QuitACTS,this);

  /* loop and read */
  for(;;)
  {
    // clean our buffers
    memset(&acts_data,0,sizeof(acts_data));
    memset(&local_data,0,sizeof(local_data));

    // put in some stuff that doesnt change
    acts_data.width = htons(this->width);
    acts_data.height = htons(this->height);

    /* test if we are supposed to cancel */
    pthread_testcancel();

    /* request a packet from ACTS */
    if(write(sock,&acts_request_packet,sizeof(acts_request_packet)) == -1)
    {
      perror("RunVisionThread: write() failed sending ACTS_REQUEST_PACKET;"
             "exiting.");
      break;
    }

    /* get the header first */
    if((numread = read(sock,acts_hdr_buf,header_len)) == -1)
    {
      perror("RunVisionThread: read() failed for header: exiting");
      break;
    }
    else if(numread != header_len)
    {
      fprintf(stderr,"RunVisionThread: something went wrong\n"
              "              expected %d bytes of header, but only got %d\n",
              header_len,numread);
      break;
    }

    /* convert the header, if necessary */
    if(acts_version == ACTS_VERSION_1_0)
    {
      for(i=0;i<ACTS_MAX_CHANNELS;i++)
      {
        // convert 2-byte ACTS 1.0 encoded entries to byte-swapped integers
        // in a structured array
        acts_data.header[i].index =
          htons(acts_hdr_buf[header_elt_len*i]-1);
        acts_data.header[i].num =
          htons(acts_hdr_buf[header_elt_len*i+1]-1);
      }
    }
    else
    {
      for(i=0;i<ACTS_MAX_CHANNELS;i++)
      {
        // convert 4-byte ACTS 1.2/2.0 encoded entries to byte-swapped integers
        // in a structured array
        acts_data.header[i].index = acts_hdr_buf[header_elt_len*i]-1;
        acts_data.header[i].index =
          acts_data.header[i].index << 6;
        acts_data.header[i].index |=
          acts_hdr_buf[header_elt_len*i+1]-1;
        acts_data.header[i].index =
          htons(acts_data.header[i].index);

        acts_data.header[i].num = acts_hdr_buf[header_elt_len*i+2]-1;
        acts_data.header[i].num =
          acts_data.header[i].num << 6;
        acts_data.header[i].num |=
          acts_hdr_buf[header_elt_len*i+3]-1;
        acts_data.header[i].num =
          htons(acts_data.header[i].num);
      }
    }

    /* sum up the data we expect */
    num_blobs=0;
    for(i=0;i<ACTS_MAX_CHANNELS;i++)
      num_blobs += ntohs(acts_data.header[i].num);

    /* read in the blob data */
    if((numread = read(sock,acts_blob_buf,num_blobs*blob_size)) == -1)
    {
      perror("RunVisionThread: read() failed on blob data; exiting.");
      break;
    }
    else if(numread != num_blobs*blob_size)
    {
      fprintf(stderr,"RunVisionThread: something went wrong\n"
              "              expected %d bytes of blob data, but only got %d\n",
              num_blobs*blob_size,numread);
      break;
    }


    if(acts_version == ACTS_VERSION_1_0)
    {
      // convert 10-byte ACTS 1.0 blobs to new byte-swapped structured array
      for(i=0;i<num_blobs;i++)
      {
        int tmpptr = blob_size*i;

        // TODO: put a descriptive color in here (I'm not sure where
        // to get it from).
        acts_data.blobs[i].color = 0xFF0000;

        // get the 4-byte area first
        acts_data.blobs[i].area = 0;
        for(int j=0;j<4;j++)
        {
          acts_data.blobs[i].area = acts_data.blobs[i].area << 6;
          acts_data.blobs[i].area |= acts_blob_buf[tmpptr++] - 1;
        }

        // store the 1 byte values
        acts_data.blobs[i].x = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].y = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].left = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].right = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].top = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].bottom = acts_blob_buf[tmpptr++] - 1;
      }
    }
    else
    {
      // convert 16-byte ACTS 1.2/2.0 blobs to new byte-swapped structured array
      for(i=0;i<num_blobs;i++)
      {
        int tmpptr = blob_size*i;

        // Figure out the blob channel number
        int ch = 0;
        for (int j = 0; j < ACTS_MAX_CHANNELS; j++)
        {
          if (i >= acts_data.header[j].index &&
              i < acts_data.header[j].index + acts_data.header[j].num)
          {
            ch = j;
            break;
          }
        }

        // Put in a descriptive color.
        if (ch < (int) (sizeof(colors) / sizeof(colors[0])))
          acts_data.blobs[i].color = colors[ch];
        else
          acts_data.blobs[i].color = 0xFF0000;

        // get the 4-byte area first
        acts_data.blobs[i].area = 0;
        for(int j=0;j<4;j++)
        {
          acts_data.blobs[i].area = acts_data.blobs[i].area << 6;
          acts_data.blobs[i].area |= acts_blob_buf[tmpptr++] - 1;
        }

        // Get the other 2 byte values
        acts_data.blobs[i].x = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].x = acts_data.blobs[i].x << 6;
        acts_data.blobs[i].x |= acts_blob_buf[tmpptr++] - 1;

        acts_data.blobs[i].y = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].y = acts_data.blobs[i].y << 6;
        acts_data.blobs[i].y |= acts_blob_buf[tmpptr++] - 1;

        acts_data.blobs[i].left = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].left = acts_data.blobs[i].left << 6;
        acts_data.blobs[i].left |= acts_blob_buf[tmpptr++] - 1;

        acts_data.blobs[i].right = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].right = acts_data.blobs[i].right << 6;
        acts_data.blobs[i].right |= acts_blob_buf[tmpptr++] - 1;

        acts_data.blobs[i].top = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].top = acts_data.blobs[i].top << 6;
        acts_data.blobs[i].top |= acts_blob_buf[tmpptr++] - 1;

        acts_data.blobs[i].bottom = acts_blob_buf[tmpptr++] - 1;
        acts_data.blobs[i].bottom = acts_data.blobs[i].bottom << 6;
        acts_data.blobs[i].bottom |= acts_blob_buf[tmpptr++] - 1;
      }
    }

    // Convert data to interface format
    local_data.width = acts_data.width;
    local_data.height = acts_data.height;
    local_data.blobs_count = num_blobs;
    local_data.blobs = (player_blobfinder_blob_t *)calloc(num_blobs, sizeof(local_data.blobs[0]));

    for (i = 0; i < num_blobs; i++)
    {
      src = acts_data.blobs + i;
      dst = local_data.blobs + i;
      dst->id = 0;
      dst->color = src->color;
      dst->x = src->x;
      dst->y = src->y;
      dst->left = src->left;
      dst->right = src->right;
      dst->top = src->top;
      dst->bottom = src->bottom;
      dst->range = 0;
    }

    /* got the data. now fill it in */
    Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_BLOBFINDER_DATA_BLOBS, &local_data);
    free(local_data.blobs);
  }

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

void
QuitACTS(void* visiondevice)
{
  char acts_request_quit = ACTS_REQUEST_QUIT;
  Acts* vd = (Acts*)visiondevice;


  if((fcntl(vd->sock, F_SETFL, O_NONBLOCK) == -1) ||
     (write(vd->sock,&acts_request_quit,sizeof(acts_request_quit)) == -1))
  {
    perror("Acts::QuitACTS(): WARNING: either fcntl() or write() "
            "failed while sending QUIT command; killing ACTS by hand");
    vd->KillACTS();
  }
  vd->sock = -1;
}
