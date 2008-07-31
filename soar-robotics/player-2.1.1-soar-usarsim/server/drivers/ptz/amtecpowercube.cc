/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  Brian Gerkey gerkey@robotics.stanford.edu
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
 * $Id: amtecpowercube.cc 4135 2007-08-23 19:58:48Z gerkey $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_amtecpowercube amtecpowercube
 * @brief Amtec PowerCube pan-tilt unit

The amtecpowercube driver controls the Amtec PowerCube Wrist,
a powerful pan-tilt unit that can, for example, carry a SICK laser
(@ref driver_sicklms200).

This driver communicates with the PowerCube via RS232, and does NOT handle
the newer CAN-based units.  Please submit a patch to support the CAN
protocol.

The amtecpowercube driver supports both position and velocity control,
via the PLAYER_PTZ_REQ_CONTROL_MODE request.  For constant swiveling,
the PowerCube works better under velocity control.

Note that this driver is relatively new and not thoroughly tested.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_ptz

@par Requires

- none

@par Configuration requests

- PLAYER_PTZ_REQ_CONTROL_MODE

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port where the unit is attached.
- home (integer)
  - Default: 0
  - Whether to home (i.e., reset to the zero position) the unit before
    commanding it
- speed (angle)
  - Default: 40 deg/sec
  - Maximum pan/tilt speed 

@par Example

@verbatim
driver
(
  name "amtecpowercube"
  port "/dev/ttyS0"
  home 1
)
@endverbatim

@author Brian Gerkey

*/

/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */
#include <math.h>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

#define AMTEC_DEFAULT_PORT "/dev/ttyS0"

#define AMTEC_SLEEP_TIME_USEC 20000

/* angular velocity used when in position control mode */
#define AMTEC_DEFAULT_SPEED_DEG_PER_SEC DTOR(40)

// start, end, and escape chars
#define AMTEC_STX       0x02
#define AMTEC_ETX       0x03
#define AMTEC_DLE       0x10

// sizes
#define AMTEC_MAX_CMDSIZE     48

// module IDs
#define AMTEC_MODULE_TILT       11
#define AMTEC_MODULE_PAN        12

// command IDs
#define AMTEC_CMD_RESET        0x00
#define AMTEC_CMD_HOME         0x01
#define AMTEC_CMD_HALT         0x02
#define AMTEC_CMD_SET_EXT      0x08
#define AMTEC_CMD_GET_EXT      0x0a
#define AMTEC_CMD_SET_MOTION   0x0b
#define AMTEC_CMD_SET_ISTEP    0x0d

// parameter IDs
#define AMTEC_PARAM_ACT_POS   0x3c
#define AMTEC_PARAM_MIN_POS   0x45
#define AMTEC_PARAM_MAX_POS   0x46
#define AMTEC_PARAM_CUBESTATE 0x27
#define AMTEC_PARAM_MAXCURR   0x4c
#define AMTEC_PARAM_ACT_VEL   0x41

// motion IDs
#define AMTEC_MOTION_FRAMP       4
#define AMTEC_MOTION_FRAMP_ACK  14
#define AMTEC_MOTION_FSTEP_ACK  16
#define AMTEC_MOTION_FVEL_ACK   17

// module state bitmasks
#define AMTEC_STATE_ERROR     0x01
#define AMTEC_STATE_HOME_OK   0x02
#define AMTEC_STATE_HALTED    0x04

class AmtecPowerCube:public Driver 
{
  private:
    // this function will be run in a separate thread
    virtual void Main();

    // bookkeeping
    bool fd_blocking;
    int return_to_home;
    int minpan, maxpan;
    int mintilt, maxtilt;
    int speed;
    uint8_t controlmode;

    // low-level methods to interact with the device
    int SendCommand(int id, unsigned char* cmd, size_t len);
    int WriteData(unsigned char *buf, size_t len);
    int AwaitAnswer(unsigned char* buf, size_t len);
    int AwaitETX(unsigned char* buf, size_t len);
    int ReadAnswer(unsigned char* buf, size_t len);
    size_t ConvertBuffer(unsigned char* buf, size_t len);
    int GetFloatParam(int id, int param, float* val);
    int GetUint32Param(int id, int param, unsigned int* val);
    int SetFloatParam(int id, int param, float val);

    // data (de)marshalling helpers
    // NOTE: these currently assume little-endianness, which is NOT
    //       portable (but works fine on x86).
    float BytesToFloat(unsigned char* bytes);
    unsigned int BytesToUint32(unsigned char* bytes);
    void FloatToBytes(unsigned char *bytes, float f);
    void Uint16ToBytes(unsigned char *bytes, unsigned short s);

    // higher-level methods for common use
    int GetPanTiltPos(short* pan, short* tilt);
    int GetPanTiltVel(short* panspeed, short* tiltspeed);
    int SetPanPos(short oldpan, short pan);
    int SetTiltPos(short oldtilt, short tilt);
    int SetPanVel(short panspeed);
    int SetTiltVel(short tiltspeed);
    int Home();
    int Halt();
    int Reset();
    int SetLimits();

    // helper for dealing with config requests.
    //void HandleConfig(void *client, unsigned char *buf, size_t len);
      short lastpan, lasttilt;

      short lastpanspeed, lasttiltspeed;

  public:
    int fd; // amtec device file descriptor
    /* device used to communicate with the ptz */
    const char* serial_port;

    AmtecPowerCube( ConfigFile* cf, int section);

    // MessageHandler
    int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);

    virtual int Setup();
    virtual int Shutdown();
};

// initialization function
Driver* AmtecPowerCube_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new AmtecPowerCube( cf, section)));
}

// a driver registration function
void 
AmtecPowerCube_Register(DriverTable* table)
{
  table->AddDriver("amtecpowercube",  AmtecPowerCube_Init);
}

AmtecPowerCube::AmtecPowerCube( ConfigFile* cf, int section) 
        : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE)
{
  fd = -1;
/*  player_ptz_data_t data;
  player_ptz_cmd_t cmd;

  data.pan = data.tilt = data.zoom = 0;
  cmd.pan = cmd.tilt = cmd.zoom = 0;

  PutData((unsigned char*)&data,sizeof(data),NULL);
  PutCommand(this->device_id,(unsigned char*)&cmd,sizeof(cmd),NULL);*/

  this->serial_port = cf->ReadString(section, "port", AMTEC_DEFAULT_PORT);
  this->return_to_home = cf->ReadInt(section, "home", 0);
  this->speed = (int)rint(RTOD(cf->ReadAngle(section, "speed", 
                                             AMTEC_DEFAULT_SPEED_DEG_PER_SEC)));
}

int 
AmtecPowerCube::Reset()
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[1];

  cmd[0] = AMTEC_CMD_RESET;
  if(SendCommand(AMTEC_MODULE_PAN,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  if(SendCommand(AMTEC_MODULE_TILT,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  return(0);
}

int 
AmtecPowerCube::Home()
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[1];
  unsigned int state;

  cmd[0] = AMTEC_CMD_HOME;
  if(SendCommand(AMTEC_MODULE_PAN,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  // poll the device state, wait for homing to finish
  for(;;)
  {
    usleep(AMTEC_SLEEP_TIME_USEC);
    if(GetUint32Param(AMTEC_MODULE_PAN, AMTEC_PARAM_CUBESTATE, &state) < 0)
    {
      PLAYER_ERROR("GetUint32Param() failed");
      return(-1);
    }
    if(state & AMTEC_STATE_HOME_OK)
      break;
  }
  if(SendCommand(AMTEC_MODULE_TILT,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  // poll the device state, wait for homing to finish
  for(;;)
  {
    usleep(AMTEC_SLEEP_TIME_USEC);
    if(GetUint32Param(AMTEC_MODULE_TILT, AMTEC_PARAM_CUBESTATE, &state) < 0)
    {
      PLAYER_ERROR("GetUint32Param() failed");
      return(-1);
    }
    if(state & AMTEC_STATE_HOME_OK)
      break;
  }
  return(0);
}

int 
AmtecPowerCube::Halt()
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[1];

  cmd[0] = AMTEC_CMD_HALT;
  if(SendCommand(AMTEC_MODULE_PAN,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  if(SendCommand(AMTEC_MODULE_TILT,cmd,1) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }
  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }
  return(0);
}

int 
AmtecPowerCube::Setup()
{
  struct termios term;
  short pan,tilt;
  int flags;

  // default to position control
  this->controlmode = PLAYER_PTZ_POSITION_CONTROL;

  player_ptz_cmd_t cmd;
  cmd.pan = cmd.tilt = cmd.zoom = 0;

  printf("Amtec PowerCube connection initializing (%s)...", serial_port);
  fflush(stdout);

  // open it.  non-blocking at first, in case there's no ptz unit.
  if((fd = open(serial_port, O_RDWR | O_SYNC | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
  {
    PLAYER_ERROR1("open() failed: %s", strerror(errno));
    return(-1);
  }  
 
  if(tcflush(fd, TCIFLUSH ) < 0 )
  {
    PLAYER_ERROR1("tcflush() failed: %s", strerror(errno));
    close(fd);
    fd = -1;
    return(-1);
  }
  if(tcgetattr(fd, &term) < 0 )
  {
    PLAYER_ERROR1("tcgetattr() failed: %s", strerror(errno));
    close(fd);
    fd = -1;
    return(-1);
  }
  
  cfmakeraw(&term);
  cfsetispeed(&term, B38400);
  cfsetospeed(&term, B38400);
  
  if(tcsetattr(fd, TCSAFLUSH, &term) < 0 )
  {
    PLAYER_ERROR1("tcsetattr() failed: %s", strerror(errno));
    close(fd);
    fd = -1;
    return(-1);
  }

  fd_blocking = false;
  /* try to get current state, just to make sure we actually have a camera */
  if(GetPanTiltPos(&pan,&tilt))
  {
    printf("Couldn't connect to Amtec PowerCube most likely because the unit\n"
                    "is not connected or is connected not to %s\n", 
                    serial_port);
    close(fd);
    fd = -1;
    return(-1);
  }

  /* ok, we got data, so now set NONBLOCK, and continue */
  if((flags = fcntl(fd, F_GETFL)) < 0)
  {
    PLAYER_ERROR1("fcntl() failed: %s", strerror(errno));
    close(fd);
    fd = -1;
    return(1);
  }
  if(fcntl(fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    PLAYER_ERROR1("fcntl() failed: %s", strerror(errno));
    close(fd);
    fd = -1;
    return(1);
  }
  fd_blocking = true;
  puts("Done.");

  // zero the command buffer
  //PutCommand(this->device_id,(unsigned char*)&cmd,sizeof(cmd),NULL);

  // reset and home the unit.
  if(Reset() < 0)
  {
    PLAYER_ERROR("Reset() failed; bailing.");
    close(fd);
    fd = -1;
    return(-1);
  }
  if(Home() < 0)
  {
    PLAYER_ERROR("Home() failed; bailing.");
    close(fd);
    fd = -1;
    return(-1);
  }

  // start the thread to talk with the camera
  StartThread();

  return(0);
}

int
AmtecPowerCube::Shutdown()
{
  if(fd == -1)
    return(0);

  StopThread();

  // stop the unit
  if(Halt())
    PLAYER_WARN("Halt() failed.");

  // maybe return it to home
  if(return_to_home && Home())
    PLAYER_WARN("Home() failed.");

  if(close(fd))
    PLAYER_ERROR1("close() failed:%s",strerror(errno));
  fd = -1;
  puts("Amtec PowerCube has been shutdown");
  return(0);
}

////////////////////////////////////////////////////////////////////////////
// The following methods are based on some found in CARMEN.  Thanks to the
// authors.

// NOTE: these conversion methods only work on little-endian machines
// (the Amtec protocol also uses little-endian).
float
AmtecPowerCube::BytesToFloat(unsigned char *bytes)
{
  float f;
  memcpy((void*)&f, bytes, 4);
  return(f);
}
unsigned int
AmtecPowerCube::BytesToUint32(unsigned char* bytes)
{
  unsigned int i;
  memcpy((void*)&i, bytes, 4);
  return(i);
}
void
AmtecPowerCube::FloatToBytes(unsigned char *bytes, float f)
{
  memcpy(bytes, (void*)&f, 4);
}
void
AmtecPowerCube::Uint16ToBytes(unsigned char *bytes, unsigned short s)
{
  memcpy(bytes, (void*)&s, 2);
}

int 
AmtecPowerCube::SendCommand(int id, unsigned char* cmd, size_t len)
{
  size_t i;
  int ctr, add;
  unsigned char rcmd[AMTEC_MAX_CMDSIZE];
  unsigned char bcc;
  unsigned char umnr;
  unsigned char lmnr;

  add  = 0;
  lmnr = id & 7;
  lmnr = lmnr << 5;
  umnr = id >> 3;
  umnr = umnr | 4;
  for (i=0;i<len;i++) {
    if ( (cmd[i]==0x02) ||
	 (cmd[i]==0x03) ||
	 (cmd[i]==0x10) ) {
      add++;
    }
  }
  lmnr = lmnr + len;
  rcmd[0] = AMTEC_STX;
  rcmd[1] = umnr;
  rcmd[2] = lmnr;
  ctr = 3;
  for (i=0;i<len;i++) {
    switch(cmd[i]) {
    case 0x02:
      rcmd[ctr] = 0x10;
      rcmd[++ctr] = 0x82;
      break;
    case 0x03:
      rcmd[ctr] = 0x10;
      rcmd[++ctr] = 0x83;
      break;
    case 0x10:
      rcmd[ctr] = 0x10;
      rcmd[++ctr] = 0x90;
      break;
    default:
      rcmd[ctr] = cmd[i];
    }
    ctr++;
  }
  bcc = id;
  for (i=0;i<len;i++) {
    bcc += cmd[i];
  }
  bcc = bcc + (bcc>>8);
  switch(bcc) {
  case 0x02:
    rcmd[ctr++] = 0x10;
    rcmd[ctr++] = 0x82;
    break;
  case 0x03:
    rcmd[ctr++] = 0x10;
    rcmd[ctr++] = 0x83;
    break;
  case 0x10:
    rcmd[ctr++] = 0x10;
    rcmd[ctr++] = 0x90;
    break;
  default:
    rcmd[ctr++] = bcc;
  }
  rcmd[ctr++] = AMTEC_ETX;

  if(WriteData(rcmd, ctr) == ctr)
    return(0);
  else
  {
    PLAYER_ERROR("short write");
    return(-1);
  }
}

int
AmtecPowerCube::WriteData(unsigned char *buf, size_t len)
{
  size_t written = 0;
  int tmp = 0;

  while(written < len)
  {
    if((tmp = write(fd, buf, len)) < 0)
    {
      PLAYER_ERROR1("write() failed: %s", strerror(errno));
      return(-1);
    }

    written += tmp;
  }
  return(written);
}

int
AmtecPowerCube::AwaitETX(unsigned char* buf, size_t len)
{
  int pos, loop, numread, totalnumread;
  pos = 0; loop = 0;
  while(loop<10)
  {
    if((numread = read(fd,buf+pos,len-pos)) < 0)
    {
      PLAYER_ERROR1("read() failed:%s", strerror(errno));
      return(-1);
    }
    else if(!numread)
    {
      if(!fd_blocking)
        usleep(10000);
      loop++;
    }
    else
    {
      if(buf[pos+numread-1]==AMTEC_ETX)
      {
	totalnumread = pos+numread-1;
	return(totalnumread);
      }
      pos += numread;
    }
  }
  PLAYER_ERROR("never found ETX");
  return(-1);
}

int
AmtecPowerCube::AwaitAnswer(unsigned char* buf, size_t len)
{
  int numread;

  // if we're not blocking, give the unit some time to respond
  if(!fd_blocking)
    usleep(AMTEC_SLEEP_TIME_USEC);

  for(;;)
  {
    if((numread = read(fd, buf, 1)) < 0)
    {
      PLAYER_ERROR1("read() failed:%s", strerror(errno));
      return(-1);
    }
    else if(!numread)
    {
      // hmm...we were expecting something, yet we read
      // zero bytes. some glitch.  drain input, and return
      // zero.  we'll get a message next time through.
      PLAYER_WARN("read 0 bytes");
      if(tcflush(fd, TCIFLUSH ) < 0 )
      {
        PLAYER_ERROR1("tcflush() failed:%s",strerror(errno));
        return(-1);
      }
      return(0);
    }
    else
    {
      if(buf[0]==AMTEC_STX) 
        return(AwaitETX(buf,len));
      else
        continue;
    }
  }
}

size_t
AmtecPowerCube::ConvertBuffer(unsigned char* buf, size_t len)
{
  size_t i, j, actual_len;

  actual_len = len;

  for (i=0;i<len;i++) 
  {
    if(buf[i]==AMTEC_DLE) 
    {
      switch(buf[i+1]) 
      {
        case 0x82:
          buf[i] = 0x02;
          for(j=i+2;j<len;j++) 
            buf[j-1] = buf[j];
          actual_len--;
          break;
        case 0x83:
          buf[i] = 0x03;
          for(j=i+2;j<len;j++) 
            buf[j-1] = buf[j];
          actual_len--;
          break;
        case 0x90:
          buf[i] = 0x10;
          for(j=i+2;j<len;j++) 
            buf[j-1] = buf[j];
          actual_len--;
          break;
      }
    }
  }
  return(actual_len);
}

int
AmtecPowerCube::ReadAnswer(unsigned char* buf, size_t len)
{
  int actual_len;

  if((actual_len = AwaitAnswer(buf, len)) <= 0)
    return(actual_len);
  else
    return((int)ConvertBuffer(buf, (size_t)actual_len));
}
// The preceding methods are based some found in CARMEN.  Thanks to the
// authors.
////////////////////////////////////////////////////////////////////////////

int
AmtecPowerCube::GetFloatParam(int id, int param, float* val)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[2];

  cmd[0] = AMTEC_CMD_GET_EXT;
  cmd[1] = param;

  if(SendCommand(id, cmd, 2) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }

  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }

  *val = BytesToFloat(buf+4);
  return(0);
}

int
AmtecPowerCube::GetUint32Param(int id, int param, unsigned int* val)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[2];

  cmd[0] = AMTEC_CMD_GET_EXT;
  cmd[1] = param;

  if(SendCommand(id, cmd, 2) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }

  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }

  *val = BytesToUint32(buf+4);
  return(0);
}

int
AmtecPowerCube::SetFloatParam(int id, int param, float val)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[6];

  cmd[0] = AMTEC_CMD_SET_EXT;
  cmd[1] = param;
  FloatToBytes(cmd+2, val);

  if(SendCommand(id, cmd, 6) < 0)
  {
    PLAYER_ERROR("SendCommand() failed");
    return(-1);
  }

  if(ReadAnswer(buf,sizeof(buf)) < 0)
  {
    PLAYER_ERROR("ReadAnswer() failed");
    return(-1);
  }

  return(0);
}

int
AmtecPowerCube::GetPanTiltPos(short* pan, short* tilt)
{
  float tmp;

  // get the pan
  if(GetFloatParam(AMTEC_MODULE_PAN, AMTEC_PARAM_ACT_POS, &tmp) < 0)
  {
    PLAYER_ERROR("GetFloatParam() failed");
    return(-1);
  }
  // reverse pan angle, to increase ccw, then normalize
  *pan = -(short)RTOD(NORMALIZE(tmp));
  
  // get the tilt
  if(GetFloatParam(AMTEC_MODULE_TILT, AMTEC_PARAM_ACT_POS, &tmp) < 0)
  {
    PLAYER_ERROR("GetFloatParam() failed");
    return(-1);
  }
  *tilt = (short)(RTOD(tmp));

  return(0);
}

int
AmtecPowerCube::GetPanTiltVel(short* panspeed, short* tiltspeed)
{
  float tmp;

  // get the pan
  if(GetFloatParam(AMTEC_MODULE_PAN, AMTEC_PARAM_ACT_VEL, &tmp) < 0)
  {
    PLAYER_ERROR("GetFloatParam() failed");
    return(-1);
  }
  // reverse pan angle, to increase ccw, then normalize
  *panspeed = -(short)RTOD(NORMALIZE(tmp));
  
  // get the tilt
  if(GetFloatParam(AMTEC_MODULE_TILT, AMTEC_PARAM_ACT_VEL, &tmp) < 0)
  {
    PLAYER_ERROR("GetFloatParam() failed");
    return(-1);
  }
  *tiltspeed = (short)(RTOD(tmp));

  return(0);
}

int
AmtecPowerCube::SetLimits()
{
  // have to reverse the signs for the pan limits, since the Amtec unit
  // counts up clockwise, rather than ccw.
  if(this->minpan != INT_MAX)
  {
    if(SetFloatParam(AMTEC_MODULE_PAN, AMTEC_PARAM_MAX_POS, 
                     NORMALIZE(DTOR(-this->minpan))) < 0)
    {
      PLAYER_ERROR("SetFloatParam() failed");
      return(-1);
    }
  }
  if(this->maxpan != INT_MAX)
  {
    if(SetFloatParam(AMTEC_MODULE_PAN, AMTEC_PARAM_MIN_POS, 
                     NORMALIZE(DTOR(-this->maxpan))) < 0)
    {
      PLAYER_ERROR("SetFloatParam() failed");
      return(-1);
    }
  }
  if(this->mintilt != INT_MAX)
  {
    if(SetFloatParam(AMTEC_MODULE_TILT, AMTEC_PARAM_MIN_POS, 
                     NORMALIZE(DTOR(this->mintilt))) < 0)
    {
      PLAYER_ERROR("SetFloatParam() failed");
      return(-1);
    }
  }
  if(this->maxtilt != INT_MAX)
  {
    if(SetFloatParam(AMTEC_MODULE_TILT, AMTEC_PARAM_MAX_POS, 
                     NORMALIZE(DTOR(this->maxtilt))) < 0)
    {
      PLAYER_ERROR("SetFloatParam() failed");
      return(-1);
    }
  }
  return(0);
}

int
AmtecPowerCube::SetPanPos(short oldpan, short pan)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[8];
  float newpan;
  unsigned short time;

  newpan = DTOR(pan);
  time = (unsigned short)rint(((double)abs(pan - oldpan) / 
                               (double)this->speed) * 1e3);

  cmd[0] = AMTEC_CMD_SET_MOTION;
  cmd[1] = AMTEC_MOTION_FSTEP_ACK;
  FloatToBytes(cmd+2,newpan);
  Uint16ToBytes(cmd+6,time);
  if(SendCommand(AMTEC_MODULE_PAN,cmd,8) < 0)
    return(-1);
  if(ReadAnswer(buf,sizeof(buf)) < 0)
    return(-1);
  return(0);
}

int
AmtecPowerCube::SetTiltPos(short oldtilt, short tilt)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[8];
  float newtilt;
  unsigned short time;

  newtilt = DTOR(tilt);
  time = (unsigned short)rint(((double)abs(tilt - oldtilt) / 
                               (double)this->speed) * 1e3);

  cmd[0] = AMTEC_CMD_SET_MOTION;
  cmd[1] = AMTEC_MOTION_FSTEP_ACK;
  FloatToBytes(cmd+2,newtilt);
  Uint16ToBytes(cmd+6,time);
  if(SendCommand(AMTEC_MODULE_TILT,cmd,8) < 0)
    return(-1);
  if(ReadAnswer(buf,sizeof(buf)) < 0)
    return(-1);
  return(0);
}

int
AmtecPowerCube::SetPanVel(short panspeed)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[6];
  float newpanspeed;

  newpanspeed = DTOR(panspeed);

  cmd[0] = AMTEC_CMD_SET_MOTION;
  cmd[1] = AMTEC_MOTION_FVEL_ACK;
  FloatToBytes(cmd+2,newpanspeed);
  printf("sending pan command: %d (%f)\n", panspeed, newpanspeed);
  if(SendCommand(AMTEC_MODULE_PAN,cmd,6) < 0)
    return(-1);
  if(ReadAnswer(buf,sizeof(buf)) < 0)
    return(-1);
  return(0);
}

int
AmtecPowerCube::SetTiltVel(short tiltspeed)
{
  unsigned char buf[AMTEC_MAX_CMDSIZE];
  unsigned char cmd[6];
  float newtiltspeed;

  newtiltspeed = DTOR(tiltspeed);

  cmd[0] = AMTEC_CMD_SET_MOTION;
  cmd[1] = AMTEC_MOTION_FVEL_ACK;
  FloatToBytes(cmd+2,newtiltspeed);
  if(SendCommand(AMTEC_MODULE_TILT,cmd,6) < 0)
    return(-1);
  if(ReadAnswer(buf,sizeof(buf)) < 0)
    return(-1);
  return(0);
}


int AmtecPowerCube::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, device_addr))
	{
      short newpan, newtilt;

      short newpanspeed, newtiltspeed;
	  assert(hdr->size == sizeof(player_ptz_cmd_t));
	  player_ptz_cmd_t & command = *reinterpret_cast<player_ptz_cmd_t *> (data);
  
      if(this->controlmode == PLAYER_PTZ_POSITION_CONTROL)
      {
        // reverse pan angle, to increase ccw
        newpan = -(short)(command.pan);
        newtilt = (short)(command.tilt);

        if(newpan != lastpan)
        {
          // send new pan position
          if(SetPanPos(lastpan,newpan))
          {
            PLAYER_ERROR("SetPan() failed(); bailing.");
            pthread_exit(NULL);
          }

          lastpan = newpan;
        }

        if(newtilt != lasttilt)
        {
          // send new tilt position
          if(SetTiltPos(lasttilt,newtilt))
          {
            PLAYER_ERROR("SetTilt() failed(); bailing.");
            pthread_exit(NULL);
          }

          lasttilt = newtilt;
        }
      }
      else if(this->controlmode == PLAYER_PTZ_VELOCITY_CONTROL)
      {
        // reverse pan angle, to increase ccw
        newpanspeed = -(short)(command.panspeed);
        newtiltspeed = (short)(command.tiltspeed);

        if(newpanspeed != lastpanspeed)
        {
          // send new pan speed
          if(SetPanVel(newpanspeed))
          {
            PLAYER_ERROR("SetPanVel() failed(); bailing.");
            pthread_exit(NULL);
          }

          lastpanspeed = newpanspeed;
         }

        if(newtiltspeed != lasttiltspeed)
        {
          // send new tilt position
          if(SetTiltVel(newtiltspeed))
          {
            PLAYER_ERROR("SetTiltVel() failed(); bailing.");
            pthread_exit(NULL);
          }

          lasttiltspeed = newtiltspeed;
        }
      }
      else
      {
        PLAYER_ERROR1("unkown control mode: %d; bailing",this->controlmode);
         pthread_exit(NULL);
      }

  	  return 0;
	}

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_PTZ_REQ_CONTROL_MODE, device_addr))
	{
	  player_ptz_req_control_mode_t * cfg = reinterpret_cast<player_ptz_req_control_mode_t *> (data);

      if((cfg->mode != PLAYER_PTZ_VELOCITY_CONTROL) &&
          (cfg->mode != PLAYER_PTZ_POSITION_CONTROL))
      {
        PLAYER_WARN1("unknown control mode requested: %d", cfg->mode);
        Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_PTZ_REQ_CONTROL_MODE);

        return 0;
      }
      else
      {
        controlmode = cfg->mode;
        Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_PTZ_REQ_CONTROL_MODE);
        return 0;
      }
	}
    
    return -1;
}
 
void 
AmtecPowerCube::Main()
{
  player_ptz_data_t data;

//  short lastpan, lasttilt;
//  short newpan, newtilt;
  short currpan, currtilt;

//  short lastpanspeed, lasttiltspeed;
//  short newpanspeed, newtiltspeed;
  short currpanspeed, currtiltspeed;

  // read the current state
/*  if(GetPanTiltPos(&lastpan,&lasttilt) < 0)
  {
    PLAYER_ERROR("GetPanTiltPos() failed; bailing.");
    pthread_exit(NULL);
  }
  if(GetPanTiltVel(&lastpanspeed,&lasttiltspeed) < 0)
  {
    PLAYER_ERROR("GetPanTiltVel() failed; bailing.");
    pthread_exit(NULL);
  }*/

  for(;;)
  {
    pthread_testcancel();

    ProcessMessages();

    if(GetPanTiltPos(&currpan,&currtilt))
    {
      PLAYER_ERROR("GetPanTiltPos() failed(); bailing.");
      pthread_exit(NULL);
    }
    if(GetPanTiltVel(&currpanspeed,&currtiltspeed))
    {
      PLAYER_ERROR("GetPanTiltVel() failed(); bailing.");
      pthread_exit(NULL);
    }

    data.pan = (currpan);
    data.tilt = (currtilt);
    data.zoom = 0;
    data.panspeed = (currpanspeed);
    data.tiltspeed = (currtiltspeed);

    Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE, (unsigned char*)&data, sizeof(player_ptz_data_t),NULL);

    // get the module state (for debugging and warning)
    unsigned int state;
    if(GetUint32Param(AMTEC_MODULE_PAN, AMTEC_PARAM_CUBESTATE, &state) < 0)
    {
      PLAYER_ERROR("GetUint32Param() failed; bailing");
      pthread_exit(NULL);
    }
    //printf("state: 0x%x\n", state);
    if(state & AMTEC_STATE_ERROR)
    {
      PLAYER_ERROR1("the Amtec unit has encountered an error and will need\n"
                    "    to be reset; bailing.   Current module state: 0x%x", 
                    state);
      pthread_exit(NULL);
    }
    
    usleep(AMTEC_SLEEP_TIME_USEC);
  }
}
