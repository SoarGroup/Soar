#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
//#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */
#include <math.h>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

#ifndef CRTSCTS
#ifdef IHFLOW
#ifdef OHFLOW
#define CRTSCTS ((IHFLOW) | (OHFLOW))
#endif
#endif
#endif

#define CAM_ERROR_NONE 0x30
#define CAM_ERROR_BUSY 0x31
#define CAM_ERROR_PARAM 0x35
#define CAM_ERROR_MODE 0x39

#define PTZ_SLEEP_TIME_USEC 100000

#define MAX_PTZ_COMMAND_LENGTH 16
#define MAX_PTZ_REQUEST_LENGTH 14
#define COMMAND_RESPONSE_BYTES 6

#define PTZ_PAN_MAX 98.0   // 875 units 0x36B
#define PTZ_TILT_MAX 88.0  // 790 units 0x316
#define PTZ_TILT_MIN -30.0 // -267 units 0x10B
#define MAX_ZOOM 1960 //1900
#define ZOOM_CONV_FACTOR 17

#define PT_BUFFER_INC       512
#define PT_READ_TIMEOUT   10000
#define PT_READ_TRIALS        2

#define DEFAULT_PTZ_PORT "/dev/ttyS1"

#define kaska printf("KASKA\n");

static unsigned int error_code;


class canonvcc4:public Driver
{
 private:
  bool ptz_fd_blocking;

  int SendCommand(unsigned char* str, int len);
  //  int SendRequest(unsigned char* str, int len);
  int SendRequest(unsigned char* str, int len, unsigned char* reply, uint8_t camera = 1);
  int ReceiveCommandAnswer();
  int ReceiveRequestAnswer(unsigned char* reply);
  int setControlMode();
  int setPower(int);
  int setDefaultTiltRange();
  int configurePort();
  int read_ptz(unsigned char *data, int size);
  // this function will be run in a separate thread
  virtual void Main();

  virtual int SendAbsPanTilt(int pan, int tilt);
  virtual int SendAbsZoom(int zoom);
  virtual int GetAbsZoom(int* zoom);
  virtual int GetAbsPanTilt(int* pan, int* tilt);
  virtual void PrintPacket(const char* str, unsigned char* cmd, int len);


 public:
   int maxfov, minfov;
  //player_ptz_cmd_t* command;
  //player_ptz_data_t* data;

  int ptz_fd; // ptz device file descriptor
  /* device used to communicate with the ptz */
  const char* ptz_serial_port;

  // Min and max values for camera field of view (degrees).
  // These are used to compute appropriate zoom values.

  canonvcc4(ConfigFile* cf, int section);
  void ProcessCommand(player_ptz_cmd_t & command);
  int pandemand , tiltdemand , zoomdemand ;

  // MessageHandler
int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
  // int ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, int * resp_len);

  virtual int Setup();
  virtual int Shutdown();
};

/************************************************************************/
Driver*
canonvcc4_Init(ConfigFile* cf, int section)
{
  return((Driver*)(new canonvcc4(cf, section)));
}

/************************************************************************/

// a driver registration function
void
canonvcc4_Register(DriverTable* table)
{
  table->AddDriver("canonvcc4", canonvcc4_Init);
}

/************************************************************************/

//canonvcc4::canonvcc4(ConfigFile* cf, int section)
// : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE, PLAYER_OPEN_MODE)

canonvcc4::canonvcc4( ConfigFile* cf, int section)
: Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE)
{
  pandemand = 0; tiltdemand = 0; zoomdemand = 0;
  ptz_fd = -1;

  this->minfov = (int) RTOD(cf->ReadTupleAngle(section, "fov", 0, DTOR(3)));
  this->maxfov = (int) RTOD(cf->ReadTupleAngle(section, "fov", 1, DTOR(30)));
  ptz_serial_port = cf->ReadString(section, "port", DEFAULT_PTZ_PORT);
}

/************************************************************************/
int
canonvcc4::Setup()
{
  int pan,tilt;
  int flags;
  int err = 0;

  printf("PTZ connection initializing (%s)...", ptz_serial_port);
  fflush(stdout);

  // open it.  non-blocking at first, in case there's no ptz unit.
  if((ptz_fd =
      open(ptz_serial_port,
	   O_RDWR | O_SYNC | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
  {
    perror("canonvcc4::Setup():open():");
    return(-1);
  }

  if(tcflush(ptz_fd, TCIFLUSH ) < 0 )
  {
    perror("canonvcc4::Setup():tcflush():");
    close(ptz_fd);
    ptz_fd = -1;
    return(-1);
  }

  if (configurePort())
    {
      fprintf(stderr, "setup(); could not configure serial port.\n");
      close(ptz_fd);
      return -1;
    }

  //  ptz_fd_blocking = false;
  err = setPower(0);
  fprintf(stderr, "\nPowering off/on the camera!!!!!!!!!!!!!!\n");
  err = setPower(1);
  while (error_code == CAM_ERROR_BUSY)
    {
      fprintf(stdout, "power on busy: %x\n", error_code);
      err = setPower(1);
    }
  if ((err != 0) &&
      (error_code != CAM_ERROR_NONE) && (error_code != CAM_ERROR_MODE))
    {
      printf("Could not set power on: %x\n", error_code);
      setPower(0);
      close(ptz_fd);
      return -1;
    }
  err = setControlMode();
  while (error_code == CAM_ERROR_BUSY)
    {
      printf("control mode busy: %x\n", error_code);
      err = setControlMode();
    }
  if (err)
    {
      printf("Could not set control mode\n");
      setPower(0);
      close(ptz_fd);
      return -1;
    }
  /* try to get current state, just to make sure we actually have a camera */
  err = GetAbsPanTilt(&pan,&tilt);
  if (err)
    {
      printf("Couldn't connect to PTZ device most likely because the camera\n"
	     "is not connected or is connected not to %s; %x\n",
	     ptz_serial_port, error_code);
      setPower(0);
      close(ptz_fd);
      ptz_fd = -1;
      return(-1);
    }
  //fprintf(stdout, "getAbsPantilt: %d %d\n", pan, tilt);
  err = setDefaultTiltRange();
  while (error_code == CAM_ERROR_BUSY)
    {
      printf("control mode busy: %x\n", error_code);
      err = setDefaultTiltRange();
    }
  if (err)
    {
      printf("Could not set default tilt range\n");
      setPower(0);
      close(ptz_fd);
      return -1;
    }

  /* ok, we got data, so now set NONBLOCK, and continue */
  if ((flags = fcntl(ptz_fd, F_GETFL)) < 0)
    {
      perror("canonvcc4::Setup():fcntl()");
      close(ptz_fd);
      ptz_fd = -1;
      return(1);
    }

  if (fcntl(ptz_fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
    {
      perror("canonvcc4::Setup():fcntl()");
      close(ptz_fd);
      ptz_fd = -1;
      return(1);
    }
  //  ptz_fd_blocking = true;
  puts("Done.");

  // zero the command and data buffers
  player_ptz_data_t data;
  player_ptz_cmd_t cmd;

  data.pan = data.tilt = data.zoom = 0;
  cmd.pan = cmd.tilt = cmd.zoom = 0;

  StartThread();

  return(0);
}
/************************************************************************/
int
canonvcc4::configurePort()
{
  struct termios tio;

  if(tcgetattr(ptz_fd, &tio) < 0 )
    {
      perror("canonvcc4::configurePort():tcgetattr():");
      close(ptz_fd);
      return(-1);
    }

  cfmakeraw(&tio);
  cfsetospeed(&tio, B9600);
  cfsetispeed(&tio, B9600);
  if (tcsetattr(ptz_fd, TCSAFLUSH, &tio) < 0)
    {
      fprintf(stderr, "Could not config serial port.\n");
      return -1;
    }
  tcgetattr(ptz_fd, &tio);

  /* check for hardware flow control */

  //tio.c_cflag |= CRTSCTS;

  tio.c_cflag &= ~CRTSCTS;
  tcsetattr(ptz_fd, TCSAFLUSH, &tio);
  return 0;
}

/************************************************************************/
int
canonvcc4::Shutdown()
{
  if(ptz_fd == -1)
    return(0);

  StopThread();
  usleep(PTZ_SLEEP_TIME_USEC);
  SendAbsPanTilt(0,0);
  usleep(PTZ_SLEEP_TIME_USEC);
  SendAbsZoom(0);
  setPower(0);
  if(close(ptz_fd))
    perror("canonvcc4::Shutdown():close():");
  ptz_fd = -1;
  puts("PTZ camera has been shutdown");
  return(0);
}

/************************************************************************/
int
canonvcc4::SendCommand(unsigned char *str, int len)
{
  int err = 0;

  if(len > MAX_PTZ_COMMAND_LENGTH)
  {
    fprintf(stderr,
	    "CANNONvcc4::SendCommand(): message is too large (%d bytes)\n",
	    len);
    return(-1);
  }

  err = write(ptz_fd, str, len);

  if (err < 0)
  {
    perror("canonvcc4::Send():write():");
    return(-1);
  }
  return(0);
}
/************************************************************************/
//int
//canonvcc4::SendRequest(unsigned char* str, int len)

int canonvcc4::SendRequest(unsigned char* str, int len, unsigned char* reply, uint8_t camera)

{
  int err = 0;

  if(len > MAX_PTZ_REQUEST_LENGTH)
  {
    fprintf(stderr,
	    "canonvcc4::SendRequest(): message is too large (%d bytes)\n",
	    len);
    return(-1);
  }
  err = write(ptz_fd, str, len);

  if(err < 0)
    {
      perror("canonvcc4::Send():write():");
      return(-1);
    }
  return 0;
}


/************************************************************************/
void
canonvcc4::PrintPacket(const char* str, unsigned char* cmd, int len)
{
  for(int i=0;i<len;i++)
    printf(" %.2x", cmd[i]);
  puts("");
}

/************************************************************************/

int
canonvcc4::SendAbsPanTilt(int pan, int tilt)
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];
  int convpan, convtilt;
  unsigned char buf[5];
  int ppan, ttilt;

  ppan = pan; ttilt = tilt;
  if (abs(pan) > PTZ_PAN_MAX)
    {
      if(pan < -PTZ_PAN_MAX)
	ppan = (int)-PTZ_PAN_MAX;
      else
	if(pan > PTZ_PAN_MAX)
	  ppan = (int)PTZ_PAN_MAX;
      //puts("Camera pan angle thresholded");
    }
  if (tilt > PTZ_TILT_MAX)
    ttilt = (int)PTZ_TILT_MAX;
  else
    if(tilt < PTZ_TILT_MIN)
      ttilt = (int)PTZ_TILT_MIN;
  //puts("Camera pan angle thresholded");

  //puts("Camera tilt angle thresholded");

  convpan = (int)floor(ppan/.1125) + 0x8000;
  convtilt = (int)floor(ttilt/.1125) + 0x8000;
//   fprintf(stdout, "ppan: %d ttilt: %d conpan: %d contilt: %d\n",
// 	  ppan,ttilt,convpan,convtilt);
  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0x62;
  // pan position

  sprintf((char *)buf, "%X", convpan);

  command[5] = buf[0];
  command[6] = buf[1];
  command[7] = buf[2];
  command[8] = buf[3];
  // tilt position
  sprintf((char *)buf, "%X", convtilt);
  command[9]  = buf[0];
  command[10] = buf[1];
  command[11] = buf[2];
  command[12] = buf[3];
  command[13] = (unsigned char) 0xEF;
  SendCommand(command, 14);
  //  PrintPacket( "sendabspantilt: ", command, 14);
  return(ReceiveCommandAnswer());
}
/************************************************************************/
int
canonvcc4::setDefaultTiltRange()
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];
  unsigned char buf[8];
  int maxtilt, mintilt;

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0x64;
  command[5] = 0x31;

  mintilt = (int)(floor(PTZ_TILT_MIN/.1125) + 0x8000);
  sprintf((char *)buf, "%X", mintilt);

  command[6] = buf[0];
  command[7] = buf[1];
  command[8] = buf[2];
  command[9] = buf[3];
  // tilt position
  maxtilt = (int)(floor(PTZ_TILT_MAX/.1125) + 0x8000);
  sprintf((char *)buf, "%X", maxtilt);

  command[10] = buf[0];
  command[11] = buf[1];
  command[12] = buf[2];
  command[13] = buf[3];
  command[14] = (unsigned char) 0xEF;

  SendCommand(command, 15);
  //  PrintPacket( "setDefaultRange: ", command, 15);
  return(ReceiveCommandAnswer());

}

/************************************************************************/
int
canonvcc4::GetAbsPanTilt(int* pan, int* tilt)
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];
  unsigned char reply[MAX_PTZ_REQUEST_LENGTH];
  int reply_len;
  unsigned char buf[4];
  char byte;
  unsigned int u_val;
  int val;
  int i;

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0x63;
  command[5] = 0xEF;

  if (SendRequest(command, 6, reply))
    return(-1);
  //  PrintPacket("getabspantilt: ", command, 6);
  reply_len = ReceiveRequestAnswer(reply);
  // remove the ascii encoding, and put into 4-byte array
  for (i = 0; i < 4; i++)
    {
      byte = reply[i+5];
      if (byte < 0x40)
	byte = byte - 0x30;
      else
	byte = byte - 'A' + 10;
      buf[i] = byte;
    }

  // convert the 4-bytes into a number
  u_val = buf[0] * 0x1000 + buf[1] * 0x100 + buf[2] * 0x10 + buf[3];

  // convert the number to a value that's meaningful, based on camera specs
  val = (int)(((int)u_val - (int)0x8000) * 0.1125);

  // now set myPan to the response received for where the camera thinks it is
  *pan = val;

  // repeat the steps for the tilt value
  for (i = 0; i < 4; i++)
    {
      byte = reply[i+9];
      if (byte < 0x40)
	byte = byte - 0x30;
      else
	byte = byte - 'A' + 10;
      buf[i] = byte;
    }
  u_val = buf[0] * 0x1000 + buf[1] * 0x100 + buf[2] * 0x10 + buf[3];
  val =(int)(((int)u_val  - (int)0x8000) * 0.1125);
  *tilt = val;

  return(0);
}

/************************************************************************/
int
canonvcc4::GetAbsZoom(int* zoom)
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];
  unsigned char reply[MAX_PTZ_REQUEST_LENGTH];
  int reply_len;
  char byte;
  unsigned char buf[4];
  unsigned int u_zoom;
  int i;

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0xA4;
  command[5] = 0xEF;

  if (SendRequest(command, 6, reply))
    return(-1);
  //  PrintPacket( "getabszoom: ", command, 6);

  reply_len = ReceiveRequestAnswer(reply);

//   if (reply_len < 0)
//     return -1;

  // remove the ascii encoding, and put into 2 bytes
  for (i = 0; i < 4; i++)
  {
    byte = reply[i + 5];
    if (byte < 0x40)
      byte = byte - 0x30;
    else
      byte = byte - 'A' + 10;
    buf[i] = byte;
  }

  // convert the 2 bytes into a number
  u_zoom = 0;
  for (i = 0; i < 4; i++)
    u_zoom += buf[i] * (unsigned int) pow(16.0, (double)(3 - i));
  *zoom = u_zoom;
  return(0);
}

/************************************************************************/
int
canonvcc4::SendAbsZoom(int zoom)
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];
  unsigned char buf[5];
  int i;

  if(zoom < 0)
    zoom = 0;
  //puts("Camera zoom thresholded");

  else
    if(zoom > MAX_ZOOM)
      zoom = MAX_ZOOM;

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0xB3;

  sprintf((char *)buf, "%4X", zoom);

  for (i=0;i<3;i++)
    if (buf[i] == ' ')
      buf[i] = '0';

  // zoom position
  command[5] = buf[0];
  command[6] = buf[1];
  command[7] = buf[2];
  command[8] = buf[3];
  command[9] = 0xEF;

  if (SendCommand(command, 10))
    return -1;
  //  PrintPacket( "setabszoom: ", command, 10);
  return (ReceiveCommandAnswer());
}

/************************************************************************/
 int
 canonvcc4::read_ptz(unsigned char *reply, int size)
{
  struct pollfd poll_fd;
  int len = 0;

  poll_fd.fd = ptz_fd;
  poll_fd.events = POLLIN;

  if (poll(&poll_fd, 1, 1000) <= 0)
    return -1;
  len = read(ptz_fd, reply, size);
  if (len < 0)
    return -1;

  return len;
}


/************************************************************************/
int
canonvcc4::ReceiveCommandAnswer()
{
  int num;
  unsigned char reply[COMMAND_RESPONSE_BYTES];
  int len = 0;
  unsigned char byte;
  int err;
  // puts("Receivecommandanswer begin\n");
  memset(reply, 0, COMMAND_RESPONSE_BYTES);

  for (num = 0; num <= COMMAND_RESPONSE_BYTES + 1; num++)
    {
      // if we don't get any bytes, or if we've just exceeded the limit
      // then return null
      err = read_ptz(&byte, 1);
      if (byte == (unsigned char)0xFE)
	{
	  reply[0] = byte;
	  len ++;
	  break;
	}
    }
  if (len == 0)
    return -1;

  // we got the header character so keep reading bytes for MAX_RESPONSE_BYTES more
  for(num = 1; num <= MAX_PTZ_REQUEST_LENGTH; num++)
    {
      err = read_ptz(&byte, 1);
      if (err == 0)
	continue;
      if (err < 0)
	{
	  // there are no more bytes, so check the last byte for the footer
	  if (reply[len - 1] !=  (unsigned char)0xEF)
	    {
	      fputs("canonvcc4::receiveRequest: Discarding bad packet.",
		    stderr);
	      return -1;
	    }
	  else
	    break;
	}
      else
	{
	  // add the byte to the array
	  reply[len] = byte;
	  len ++;
	}
    }

  // Check the response
  if (len != 6)
    {
      fputs("canonvcc4::receiveCommand:Incorrect number of bytes in response packet.", stderr);
      return -1;
    }

  // check the header and footer
  if (reply[0] != (unsigned char)0xFE || reply[5] != (unsigned char)0xEF)
    {
      fputs("canonvcc4::receiveCommand: Bad header or footer character in response packet.", stderr);
      return -1;
    }
  // so far so good.  Set myError to the error byte
  error_code = reply[3];
  //PrintPacket("receivecommandasnwer: ", reply, 6);
  if (error_code == CAM_ERROR_NONE)
      return 0;

  return -1;

}

/************************************************************************/
int
canonvcc4::ReceiveRequestAnswer(unsigned char *data)
{
  int num;
  unsigned char reply[MAX_PTZ_REQUEST_LENGTH];
  int len = 0;
  unsigned char byte;
  int err = 0;

  memset(reply, 0, MAX_PTZ_REQUEST_LENGTH);

  for (num = 0; num <= COMMAND_RESPONSE_BYTES + 1; num++)
    {
      // if we don't get any bytes, or if we've just exceeded the limit
      // then return null
      err = read_ptz(&byte, 1);
      if (byte == (unsigned char)0xFE)
	{
	  reply[0] = byte;
	  len ++;
	  break;
	}
    }
  if (len == 0)
    return -1;
  // we got the header character so keep reading bytes for MAX_RESPONSE_BYTES more
  for(num = 1; num <= MAX_PTZ_REQUEST_LENGTH; num++)
    {
      err = read_ptz(&byte, 1);
      if (err == 0)
	continue;
      if (err < 0)
	{
	  // there are no more bytes, so check the last byte for the footer
	  if (reply[len - 1] !=  (unsigned char)0xEF)
	    {
	      fputs("canonvcc4::receiveRequest: Discarding bad packet.",
		    stderr);
	      return -1;
	    }
	  else
	    break;
	}
      else
	{
	  // add the byte to the array
	  reply[len] = byte;
	  len ++;
	}
    }
  // Check the response length: pt: 14; zoom: 10
  if (len != 6 && len != 8 && len != 14)
    {
      fputs("Arvcc4::packetHandler: Incorrect number of bytes in response packet.", stderr);
      return -1;
    }

  if (reply[0] !=  (unsigned char)0xFE ||
      reply[len - 1] != (unsigned char)0xEF)
    {
      fputs("canonvcc4::receiveRequestArvcc4: Bad header or footer character in response packet.", stderr);
      return -1;
    }

  // so far so good.  Set myError to the error byte
  error_code = reply[3];
  //  PrintPacket("receiverequestasnwer: ", reply, len);
  if (error_code == CAM_ERROR_NONE)
    {
      memcpy(data, reply, len);
      return 0;
    }
  return -1;
}

/************************************************************************/
int
canonvcc4::setControlMode()
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0x90;
  command[5] = 0x30;
  command[6] = 0xEF;

  if (SendCommand(command, 7))
    return -1;
  //  usleep(5000000);
  return (ReceiveCommandAnswer());
}
/************************************************************************/
int
canonvcc4::setPower(int on)
{
  unsigned char command[MAX_PTZ_COMMAND_LENGTH];

  command[0] = 0xFF;
  command[1] = 0x30;
  command[2] = 0x30;
  command[3] = 0x00;
  command[4] = 0xA0;
  if (on)
    command[5] = 0x31;
  else
    command[5] = 0x30;
  command[6] = 0xEF;

  if (SendCommand(command, 7))
    return -1;
  //  usleep(5000000);
  return (ReceiveCommandAnswer());
}

/************************************************************************/

// this function will be run in a separate thread
void
canonvcc4::Main()
{
  player_ptz_data_t data;
  int pan, tilt, zoom;

  while(1)
    {
      pthread_testcancel();
      ProcessMessages();
      /* get current state */
      if(GetAbsPanTilt(&pan,&tilt) < 0)
	{
	  fputs("canonvcc4:Main():GetAbsPanTilt() errored. bailing.\n",
		stderr);
	  pthread_exit(NULL);
	}
      /* get current state */


      if(GetAbsZoom(&zoom) < 0)
	{
	  fputs("canonvcc4:Main():GetAbsZoom() errored. bailing.\n", stderr);
	  pthread_exit(NULL);
	}

      // Do the necessary coordinate conversions.  Camera's natural pan
      // coordinates increase clockwise; we want them the other way, so
      // we negate pan here.  Zoom values are converted from arbitrary
      // units to a field of view (in degrees).
      pan = -pan;

      //zoom = this->maxfov + (zoom * (this->maxfov - this->minfov)) / 1960;

      data.pan = DTOR((unsigned short)pan);
      data.tilt = DTOR((unsigned short)tilt);
      data.zoom = (unsigned short) (zoom - 3056)/ZOOM_CONV_FACTOR;
      pthread_testcancel();

      Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE, &data,sizeof(player_ptz_data_t),NULL);

      usleep(PTZ_SLEEP_TIME_USEC);
    }
}

int canonvcc4::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);

  if (Message::MatchMessage(hdr,
			    PLAYER_MSGTYPE_REQ,
			    PLAYER_PTZ_REQ_GENERIC, device_addr))
  {
    assert(hdr->size == sizeof(player_ptz_req_generic_t));

    player_ptz_req_generic_t *cfg = (player_ptz_req_generic_t *)data;

    // check whether command or inquiry...
    if (cfg->config[0] == 0x01)
    {
      if (SendCommand((uint8_t *)cfg->config, cfg->config_count) < 0)
        Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, hdr->subtype);
      else
        Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
      return 0;
    }
    else
    {
      // this is an inquiry, so we have to send data back
      cfg->config_count = SendRequest((uint8_t*)cfg->config, cfg->config_count, (uint8_t*)cfg->config);
      Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    }
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
			    PLAYER_PTZ_CMD_STATE, device_addr))
    {

      short zoomdemand=0;
      bool newpantilt=true, newzoom=true;

      assert (hdr->size == sizeof (player_ptz_cmd_t));
      player_ptz_cmd_t command = *reinterpret_cast<player_ptz_cmd_t *> (data);

      if(pandemand != (int)rint(RTOD(command.pan)))
	{
	  pandemand = (int)rint(RTOD(command.pan));
	  newpantilt = true;
	}
      if(tiltdemand != (int)rint(RTOD(command.tilt)))
	{
	  tiltdemand = (int)rint(RTOD(command.tilt));
	  newpantilt = true;
	}

      if(zoomdemand != command.zoom)
	{
	  zoomdemand = (int)command.zoom; //(int)rint(RTOD(command.zoom));
	  newzoom = true;
	}
      ////////////////////////////////////////////////////////////
      //zoomdemand = (1960 * (this->maxfov - zoomdemand)) /
      //	(this->maxfov - this->minfov);
      ////////////////////////////////////////////////////////////

      if(newzoom)
	{
	  if(SendAbsZoom(zoomdemand))
	    {
	      fputs("canonvcc4:Main():SendAbsZoom() errored. bailing.\n", stderr);
	      pthread_exit(NULL);
	    }
	}

      if(newpantilt)
	{
	  pandemand = -pandemand;
	  if(SendAbsPanTilt(pandemand,tiltdemand))
	    {
	      fputs("canonvcc4:Main():SendAbsPanTilt() errored. bailing.\n", stderr);
	      pthread_exit(NULL);
	    }
	  //  }
	}
      return 0;
    }

  return -1;
}




/************************************************************************/


// extern "C" {
//   int player_driver_init(DriverTable* table)
//   {
//     puts("Canonvcc4 driver initializing");
//     canonvcc4_Register(table);
//     puts("Canonvcc4 driver done");
//     return(0);
//   }
// }
