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

/*
 * $Id: sonyevid30.cc 6499 2008-06-10 01:13:51Z thjc $
 *
 * methods for initializing, commanding, and getting data out of
 * the Sony EVI-D30 PTZ camera
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_sonyevid30 sonyevid30
 * @brief Sony EVI-D30 and EVI-D100 pan-tilt-zoom cameras

The sonyevid30 driver provides control of a Sony EVI-D30, Sony EVI-D70 or Sony
EVI-D100 pan-tilt-zoom camera unit.

The sonyevid30 driver operates over a direct serial link, not
through the P2OS microcontroller's AUX port, as is the normal
configuration for ActivMedia robots.  You may have to make or buy
a cable to connect your camera to a normal serial port.  Look <a
href="http://playerstage.sourceforge.net/faq.html#evid30_wiring">here</a>
for more information and wiring instructions.

The sonyevid30 driver supports position and velocity control.  Position
estimates from the camera can be considered accurate, velocity estimates can
not.

@par Known Issues and Bugs

- Speed and position can not be set simultaneously.
- All angles (and angular speeds) are communicated in radians (rad/s) except
the FOV setting in the configuration file.
- No data is returned from a generic request.
- Zoom values are approximate only.
- The D70 model cameras may not function correctly when inverted.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_ptz

@par Requires

- None

@par Configuration requests

- PLAYER_PTZ_REQ_GENERIC

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS2"
  - The serial port to be used.

- fov (float tuple)
  - Default depends on detected camera model
    - D30: [4.4 48.8]
	- D70: [2.7 48.0]
	- D100: [6.6 65.0]
  - The minimum and maximum fields of view (in degrees), which will depend on
   the lens(es) you are using.

- movement (integer)
  - Default: 0
  - Movement mode (?)

@par Example

@verbatim
driver
(
  name "sonyevid30"
  provides ["ptz:0"]
  port "/dev/ttyS2"
  fov [3 30]
)
@endverbatim

@author Brian Gerkey, Brad Tonkes (D70, D100 mode)

*/
/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

#define MODEL_D3X 0x0402
#define MODEL_D100 0x040D

#define PTZ_SLEEP_TIME_USEC 100000

#define MAX_PTZ_PACKET_LENGTH 16
#define MAX_PTZ_MESSAGE_LENGTH 14
#define MAX_PTZ_REPLY_LENGTH 11

#define MAX_VER_MESSAGE_LENGTH 4
#define MAX_VER_REPLY_LENGTH 14

#define PTZ_PAN_MAX 100.0
#define PTZ_TILT_MAX 25.0

#define PTZ_MAX_PAN_SPEED	0x18
#define PTZ_MAX_TILT_SPEED	0x14

#define DEFAULT_PTZ_PORT "/dev/ttyS2"

#define VISCA_COMMAND_CODE	0x01
#define VISCA_INQUIRY_CODE	0x09

/*
 *
 *
 * Camera
 *
 *
 */

/*
 * Struct to describe the parameters of a DXX camera.
 */
typedef struct {
	char type[10];				// Text name of camera.
	int model_id;				// Internal camera model ID.

	// Pan parameters.
	float pan_max_rad;			// Maximum pan angle (radians) of ptz.
	uint16_t pan_max_cu;		// As above, in camera units.
	unsigned char pan_speed_max_cu;	// Maximum pan speed in camera units. */
	// Pan speeds (rad/sec) for each control value. This array is
	// pan_speed_max_cmd+1 long, giving a speed for each legal command value.
	const float *pan_speeds;

	// Tilt parameters. */
	float tilt_min_rad;		// Minimum tilt angle (radians)
	float tilt_max_rad;		// Maximum tilt angle (radians) of ptz.
	uint16_t tilt_min_cu;		// Minumum tilt angle in camera units.
	uint16_t tilt_max_cu;		// Maximum tilt angle in camera units
	unsigned char tilt_speed_max_cu;	// Maximum tilt speed in camera units. */
	// Tilt speeds (rad/sec) for each control value. This array is
	// tilt_speed_max_cmd+1 long, giving a speed for each legal command value.
	const float *tilt_speeds;

	// Zoom parameters.
	float fov_min_rad;			// Minimum half field-of-view - from config file.
	uint16_t fov_min_cu;		// As above, in camera units.
	float fov_max_rad;			// Maximum half field-of-view - from config file.
	uint16_t fov_max_cu;		// As above, in camera units.
} evid_cam_t;

inline float clip_float(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}


/*
 * Some handy conversions to/from camera units / radians.
 */


float evid_cam_pan_radians(evid_cam_t cam, short pan_cu)
{
	float conv_factor = -cam.pan_max_rad / cam.pan_max_cu;
	return pan_cu * conv_factor;
}

float evid_cam_tilt_radians(evid_cam_t cam, short tilt_cu)
{
	float conv_factor = cam.tilt_max_rad / cam.tilt_max_cu;
	return tilt_cu * conv_factor;
}

float evid_cam_panspeed_radians(evid_cam_t cam, short panspeed_cu)
{
	int dir = panspeed_cu < 0 ? 1 : -1;
	panspeed_cu = abs(panspeed_cu);
	if (panspeed_cu < cam.pan_speed_max_cu) {
		return cam.pan_speeds[panspeed_cu] * dir;
	} else {
		return cam.pan_speeds[cam.pan_speed_max_cu] * dir;
	}
}

float evid_cam_tiltspeed_radians(evid_cam_t cam, short tiltspeed_cu)
{
	int dir = tiltspeed_cu < 0 ? -1 : 1;
	tiltspeed_cu = abs(tiltspeed_cu);
	if (tiltspeed_cu < cam.tilt_speed_max_cu) {
		return cam.tilt_speeds[tiltspeed_cu] * dir;
	} else {
		return cam.tilt_speeds[cam.tilt_speed_max_cu] * dir;
	}
}

float evid_cam_zoom_radians(evid_cam_t cam, short zoom_cu)
{
	float conv_factor = (cam.fov_max_rad - cam.fov_min_rad)
							/ (cam.fov_max_cu - cam.fov_min_cu);

	return (zoom_cu - cam.fov_min_cu) * conv_factor + cam.fov_min_rad;
}

short evid_cam_pan_cu(evid_cam_t cam, float pan_radians)
{
	float conv_factor = -cam.pan_max_cu / cam.pan_max_rad;
	pan_radians = clip_float(pan_radians, -cam.pan_max_rad, cam.pan_max_rad);
	return (short) rint(pan_radians * conv_factor);
}

short evid_cam_tilt_cu(evid_cam_t cam, float tilt_radians)
{
	float conv_factor = cam.tilt_max_cu / cam.tilt_max_rad;
	tilt_radians = clip_float(tilt_radians, cam.tilt_min_rad, cam.tilt_max_rad);
	return (short) rint(tilt_radians * conv_factor);
}

/*
 * Translate given pan speed (radians per second) into camera units.  Where
 * the speed can not be translated exactly, use the closest match.
 */
short evid_cam_panspeed_cu(evid_cam_t cam, float panspeed_radians)
{
	int dir = panspeed_radians < 0 ? 1 : -1;
	panspeed_radians = fabsf(panspeed_radians);

	int i = 0;
	while (i < cam.pan_speed_max_cu && cam.pan_speeds[i] < panspeed_radians) {
		i++;
	}

	if (i == 0) {
		return 0;
	} else {
		float err1 = fabs(cam.pan_speeds[i] - panspeed_radians);
		float err2 = fabs(cam.pan_speeds[i - 1] - panspeed_radians);

		if (err1 < err2) {
			return (short) (i * dir);
		} else {
			return (short) ((i - 1) * dir);
		}
	}
}

/*
 * Translate given tilt speed (radians per second) into camera units.  Where
 * the speed can not be translated exactly, use the closest match.
 */
short evid_cam_tiltspeed_cu(evid_cam_t cam, float tiltspeed_radians)
{
	int dir = tiltspeed_radians < 0 ? -1 : 1;
	tiltspeed_radians = fabsf(tiltspeed_radians);

	int i = 0;
	while (i < cam.tilt_speed_max_cu && cam.tilt_speeds[i] < tiltspeed_radians) {
		i++;
	}

	if (i == 0) {
		return 0;
	} else {
		float err1 = fabs(cam.tilt_speeds[i] - tiltspeed_radians);
		float err2 = fabs(cam.tilt_speeds[i - 1] - tiltspeed_radians);

		if (err1 < err2) {
			return (short) (i * dir);
		} else {
			return (short) ((i - 1) * dir);
		}
	}
}

/*
 * Translate camera zoom (viewing angle) from radians to camera units.  This
 * function assumes that the relationship is linear (it isn't).
 */
short evid_cam_zoom_cu(evid_cam_t cam, float zoom_radians)
{
	float conv_factor = (cam.fov_max_cu - cam.fov_min_cu)
							/ (cam.fov_max_rad - cam.fov_min_rad);
	if (zoom_radians == 0) {
		return cam.fov_max_cu;
	} else {
		zoom_radians = clip_float(zoom_radians, cam.fov_min_rad, cam.fov_max_rad);
		return (short) ((zoom_radians - cam.fov_min_rad) * conv_factor
					+ cam.fov_min_cu);
	}
}

/*
 * Pan speed (rad/sec) for available pan commands.  Pan commands range from 0
 * through to evid_cam_t.pan_speed_max_cmd.  Thus, there should be
 * pan_speed_max_cmd + 1 entries in this list.
 */

/*
 * Control on the D30 is linear.
 */
const static float CAM_D30_PAN_SPEEDS[] =
	{ DTOR(0.0), DTOR(3.3), DTOR(6.7), DTOR(10.0), DTOR(13.3), DTOR(16.7),
		DTOR(20.0), DTOR(23.3), DTOR(26.7), DTOR(30.0), DTOR(33.3), DTOR(36.7),
		DTOR(40.0), DTOR(43.3), DTOR(46.7), DTOR(50.0), DTOR(53.3), DTOR(56.7),
		DTOR(60.0), DTOR(63.3), DTOR(66.7), DTOR(70.0), DTOR(73.3), DTOR(76.7),
		DTOR(80.0)
};

/*
 * Control on the D70 is a bit non-linear.
 */
const static float CAM_D70_PAN_SPEEDS[] =
	{ DTOR(0.0), DTOR(1.7), DTOR(4.3), DTOR(7.4), DTOR(9.8), DTOR(13.2),
		DTOR(18.0), DTOR(21.8), DTOR(31.5), DTOR(34.5), DTOR(39.3), DTOR(47.0),
		DTOR(49.0), DTOR(54.1), DTOR(56.6), DTOR(61.8), DTOR(64.7), DTOR(69.3),
		DTOR(72.2), DTOR(79.5), DTOR(84.0), DTOR(90.9), DTOR(100.0)
};

/*
 * Control on the D100 is non-linear.
 */
const static float CAM_D100_PAN_SPEEDS[] =
	{ DTOR(0.0), DTOR(2.0), DTOR(2.4), DTOR(3.0), DTOR(3.7), DTOR(4.7),
		DTOR(6.1), DTOR(7.4), DTOR(9.1), DTOR(11), DTOR(14), DTOR(18),
		DTOR(22), DTOR(27), DTOR(34), DTOR(42), DTOR(52), DTOR(65), DTOR(81),
		DTOR(100), DTOR(125), DTOR(155), DTOR(190), DTOR(240), DTOR(300),
};

/*
 * Tilt speed (rad/sec) for available tilt commands.  Tilt commands range from
 * 0 through to evid_cam_t.tilt_speed_max_cmd.  Thus, there should be
 * tilt_speed_max_cmd + 1 entries in this list.
 */

/*
 * Control on the D30 is linear.
 */
const static float CAM_D30_TILT_SPEEDS[] =
	{ DTOR(0.0), DTOR(2.5), DTOR(5.0), DTOR(7.5), DTOR(10.0), DTOR(12.5),
		DTOR(15.0), DTOR(17.5), DTOR(20.0), DTOR(22.5), DTOR(25.0), DTOR(27.5),
		DTOR(30.0), DTOR(32.5), DTOR(35.0), DTOR(37.5), DTOR(40.0), DTOR(42.5),
		DTOR(45.0), DTOR(47.5), DTOR(50.0),
};

/*
 * Control on the D70 is only approximately linear.
 */
const static float CAM_D70_TILT_SPEEDS[] =
	{ DTOR(0.0), DTOR(1.7), DTOR(4.3), DTOR(7.4), DTOR(9.8), DTOR(13.2),
		DTOR(18.0), DTOR(21.8), DTOR(25.0), DTOR(28.8), DTOR(31.5), DTOR(34.5),
		DTOR(39.3), DTOR(47.0), DTOR(49.0), DTOR(54.1), DTOR(56.6), DTOR(61.8),
		DTOR(64.7), DTOR(69.3), DTOR(72.2)
};

/*
 * Control on the D100 is non-linear.
 */
const static float CAM_D100_TILT_SPEEDS[] =
	{ DTOR(0.0), DTOR(2.0), DTOR(2.4), DTOR(3.0), DTOR(3.7), DTOR(4.7),
		DTOR(6.1), DTOR(7.4), DTOR(9.1), DTOR(11), DTOR(14), DTOR(18),
		DTOR(22), DTOR(27), DTOR(34), DTOR(42), DTOR(52), DTOR(65), DTOR(81),
		DTOR(100), DTOR(125),
};


/*
 *
 *           The actual cameras: D30, D70 and D100
 *
 */


const static evid_cam_t CAM_D30 = { "D30", 0x0402,	// ID
	DTOR(100), 0x370, 0x18, CAM_D30_PAN_SPEEDS,	// Pan data
	DTOR(-25), DTOR(25), 0xFED4, 0x12C, 0x14, CAM_D30_TILT_SPEEDS,	// Tilt data
	DTOR(4.4), 0x0000, DTOR(48.8), 0x03FF, // FOV data
};

// Tilt on the D70 ranges from -30 to +90.
const static evid_cam_t CAM_D70 = { "D70", 0x040E,	// ID
	DTOR(170), 0x08DB, 0x18, CAM_D70_PAN_SPEEDS,	// pan data
	DTOR(-30), DTOR(90), 0xFE98, 0x190, 0x14, CAM_D70_TILT_SPEEDS,	// tilt data
	DTOR(2.7), 0x0000, DTOR(48.0), 0x4000, // FOV data
};

const static evid_cam_t CAM_D100 = { "D100", 0x040D,	// ID
	DTOR(100), 0x5A0, 0x18, CAM_D100_PAN_SPEEDS,	// pan data
	DTOR(-25), DTOR(25), 0xFE98, 0x168, 0x14, CAM_D100_TILT_SPEEDS,	// tilt data
	DTOR(6.6), 0x4000, DTOR(65.0), 0x0000, // FOV data
};

const static int NUM_SONY_EVI_CAMERAS = 3;
const static evid_cam_t SONY_EVI_CAMERAS[NUM_SONY_EVI_CAMERAS]
				= { CAM_D30, CAM_D70, CAM_D100 };

/*
 *
 *
 *
 *
 */

class SonyEVID30:public Driver
{
 protected:
  bool command_pending1;  // keep track of how many commands are pending;
  bool command_pending2;  // that way, we can cancel them if necessary
  bool ptz_fd_blocking;

  evid_cam_t cam_config_;

  int control_mode_; // VELOCITY or POSITION?

  // Position mode only ...
  float pan_demand_rad; // Last requested pan (radians)
  float tilt_demand_rad; // Last requested tilt (radians)
  float zoom_demand_rad; // Last requested zoom (radians)

  // Velocity mode only ...
  float panspeed_demand_rad; // Last requested pan speed (radians/s)
  float tiltspeed_demand_rad; // Last requested tilt speed (radians/s)
  float panspeed_estimate_rad; // Last executed pan speed (radians/s)
  float tiltspeed_estimate_rad; // Last executed pan speed (radians/s)

  // Internal methods for handling VISCA commands
  int Send(unsigned char* str, int len, unsigned char* reply, uint8_t camera = 1);
  int Receive(unsigned char* reply);
  int SendCommand(unsigned char* str, int len, uint8_t camera = 1);
  int CancelCommand(char socket);
  int SendRequest(unsigned char* str, int len, unsigned char* reply, uint8_t camera = 1);
//  int HandleConfig(void *client, unsigned char *buf, size_t len);

  // MessageHandler
  int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
  int ProcessGenericRequest(QueuePointer &resp_queue,
									player_msghdr *hdr,
									player_ptz_req_generic_t *req);
  int ProcessPtzRequest(player_ptz_cmd_t *cmd);

  // this function will be run in a separate thread
  virtual void Main();

  // High level camera commands
  int PowerOn();
  virtual int GetCameraType();
  virtual int SendAbsPanTilt(short pan, short tilt);
  virtual int SendPanTiltSpeed(short panspeed_cu, short tiltspeed_cu);
  virtual int SendStepPan(int);
  virtual int SendStepTilt(int);
  virtual int SendAbsZoom(short zoom);
  virtual int GetAbsZoom(short* zoom);
  virtual int GetAbsPanTilt(short* pan, short* tilt);
  virtual void PrintPacket(const char* str, unsigned char* cmd, int len);

  /*
   * Get the current Pan/Tilt/Zoom state of the camera (and take a guess at
   * pan/tilt speed in velocity mode.
   */
  int UpdateState(player_ptz_data_t &data);

  double ptz_pan_conv_factor;
  double ptz_tilt_conv_factor;

 public:
  int ptz_fd; // ptz device file descriptor
  /* device used to communicate with the ptz */
  char ptz_serial_port[MAX_FILENAME_SIZE];

  // Min and max values for camera field of view (radians).
  // These are used to compute appropriate zoom values.
  float maxfov, minfov;

protected:
  struct pollfd read_pfd;

  int movement_mode;
  int pandemand;
  int tiltdemand;
  short zoomdemand;
public:

  SonyEVID30( ConfigFile* cf, int section);

  virtual int Setup();
  virtual int Shutdown();
};

// initialization function
Driver* SonyEVID30_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new SonyEVID30( cf, section)));
}

/* how to make this work for multiple cameras...
   want to make a player device for each camera, ie ptz:0 ptz:1, so can read/write commands
   on the client side independently of how they are controlled.

   but for the sonys, sets of cameras are paritioned by serial port.  so
   we add a parameter "camera" to the config for ptz, and then here we have a table
   which keeps track of instantiations of devices according to serial port.

   will need to redo the class so that cameras on the same serial port share the port
   instead of each trying to open it.  they also have a port-id.

   so _Init will read the config file and based on the serial port and camera parameter
   it will either create an instance of a serial-owning device, or instantiate a camera
   that shares an existing port.

   so SonyEVIController is the one that controls the port
   and create SonyEVIPeripheral that are the cameras.  each peripheral has an id that
   is given to create packets for that peripheral.

   problem is this makes broadcasting commands more difficult/less efficient.
   use the new Wait and GetAvailable to share the port...
*/

// a driver registration function
void SonyEVID30_Register(DriverTable * table)
{
  table->AddDriver("sonyevid30",  SonyEVID30_Init);
}

SonyEVID30::SonyEVID30( ConfigFile* cf, int section)
: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE)
{
  ptz_fd = -1;
  command_pending1 = false;
  command_pending2 = false;

  control_mode_ = PLAYER_PTZ_POSITION_CONTROL;
  movement_mode = 0;
  pan_demand_rad = 0.0;
  tilt_demand_rad = 0.0;
  zoom_demand_rad = 0.0;
  panspeed_demand_rad = 0.0;
  tiltspeed_demand_rad = 0.0;
  read_pfd.events = POLLIN;

	/*
	 * If user doesn't fill these in (default 0), we'll do it later for the
	 * specific camera type in GetCameraType.  According to the docs, these
	 * functions will translate from degrees to radians (as desired).
	 */
	this->minfov = cf->ReadTupleAngle(section, "fov", 0, 0.0);
	this->maxfov = cf->ReadTupleAngle(section, "fov", 1, 0.0);

  // Assume we've got a D3X
  this->ptz_pan_conv_factor = 0x0370 / (double) PTZ_PAN_MAX;
  this->ptz_tilt_conv_factor = 0x012C / (double) PTZ_TILT_MAX;

  movement_mode = (int) cf->ReadInt(section, "movement", 0);
  control_mode_ = (int)
		cf->ReadInt(section, "movement", PLAYER_PTZ_POSITION_CONTROL);

  strncpy(ptz_serial_port,
          cf->ReadString(section, "port", DEFAULT_PTZ_PORT),
          sizeof(ptz_serial_port));
	InQueue->SetReplace(true);
}

int SonyEVID30::Setup()
{
  struct termios term;
  short pan,tilt;
  int flags;

  printf("PTZ connection initializing (%s)...", ptz_serial_port);
  fflush(stdout);

  // open it.  non-blocking at first, in case there's no ptz unit.
  if((ptz_fd = open(ptz_serial_port, O_RDWR | O_SYNC | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
  {
    perror("SonyEVID30::Setup():open():");
    return(-1);
  }

  read_pfd.fd = ptz_fd;

  if(tcflush(ptz_fd, TCIFLUSH ) < 0 )
  {
    perror("SonyEVID30::Setup():tcflush():");
    close(ptz_fd);
    ptz_fd = -1;
    return(-1);
  }
  if(tcgetattr(ptz_fd, &term) < 0 )
  {
    perror("SonyEVID30::Setup():tcgetattr():");
    close(ptz_fd);
    ptz_fd = -1;
    return(-1);
  }

  cfmakeraw(&term);
  cfsetispeed(&term, B9600);
  cfsetospeed(&term, B9600);

  if(tcsetattr(ptz_fd, TCSAFLUSH, &term) < 0 )
  {
    perror("SonyEVID30::Setup():tcsetattr():");
    close(ptz_fd);
    ptz_fd = -1;
    return(-1);
  }

  /* Work out what version of camera we are: d3x, d70 or d100.  The
   * parameters for each are slightly different.
   */
	if (GetCameraType()) {
		printf("Couldn't connect to PTZ device most likely because "
				"the camera is not connected or is connected not to %s\n",
				ptz_serial_port);
		close(ptz_fd);
		ptz_fd = -1;
		return -1;
	}

  ptz_fd_blocking = false;
  /* try to get current state, just to make sure we actually have a camera */
  if(GetAbsPanTilt(&pan,&tilt))
  {
    printf("Couldn't connect to PTZ device most likely because the camera\n"
                    "is not connected or is connected not to %s\n",
                    ptz_serial_port);
    close(ptz_fd);
    ptz_fd = -1;
    return(-1);
  }

  /* ok, we got data, so now set NONBLOCK, and continue */
  if((flags = fcntl(ptz_fd, F_GETFL)) < 0)
  {
    perror("SonyEVID30::Setup():fcntl()");
    close(ptz_fd);
    ptz_fd = -1;
    return(1);
  }
  if(fcntl(ptz_fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    perror("SonyEVID30::Setup():fcntl()");
    close(ptz_fd);
    ptz_fd = -1;
    return(1);
  }
  ptz_fd_blocking = true;
	/*
	 * Power up the camera, just in case it's been powered down.
	 */
	if (PowerOn() < 0) {
		PLAYER_WARN1("Failed to communicate with camera on %s.\n",
					ptz_serial_port);
	}

  puts("Done.");

  // start the thread to talk with the camera
  StartThread();

  return(0);
}

int SonyEVID30::Shutdown()
{
  puts("SonyEVID30::Shutdown");

  if(ptz_fd == -1)
    return(0);

  StopThread();

  // put the camera back to center
  usleep(PTZ_SLEEP_TIME_USEC);
  SendAbsPanTilt(0,0);
  usleep(PTZ_SLEEP_TIME_USEC);
	SendAbsZoom(cam_config_.fov_max_cu);

  if(close(ptz_fd))
    perror("SonyEVID30::Shutdown():close():");
  ptz_fd = -1;
  puts("PTZ camera has been shutdown");
  return(0);
}

int SonyEVID30::Send(unsigned char *str, int len, unsigned char *reply,
					 uint8_t camera)
{
  unsigned char command[MAX_PTZ_PACKET_LENGTH];
  int i;

  if(len > MAX_PTZ_MESSAGE_LENGTH)
  {
    fprintf(stderr, "SonyEVID30::Send(): message is too large (%d bytes)\n",
                    len);
    return(-1);
  }

  assert(camera < 8);

  command[0] = 0x80 | camera; // controller address is 0, camera address 1
	for (i = 0; i < len; i++) {
    command[i+1] = str[i];
	}

  command[i+1] = 0xFF;  // packet terminator

  // send the command
  if(write(ptz_fd, command, i+2) < 0)
  {
    perror("SonyEVID30::Send():write():");
    return(-1);
  }

  //puts("Send(): calling Receive()");
  return(Receive(reply));
}

int SonyEVID30::Receive(unsigned char *reply)
{
  static unsigned char buffer[MAX_PTZ_PACKET_LENGTH];
  static int numread = 0;

  unsigned char temp_reply[MAX_PTZ_PACKET_LENGTH];
  int newnumread = 0;
  int bufptr = -1;
  int i;
  int temp;
  int pret;

  memset(temp_reply,0,MAX_PTZ_PACKET_LENGTH);
  memset(reply,0,MAX_PTZ_PACKET_LENGTH);
  if(numread > 0)
  {
    //printf("copying %d old bytes\n", numread);
    memcpy(temp_reply,buffer,numread);
    // look for the terminator
    for(i=0;i<numread;i++)
    {
      if(temp_reply[i] == 0xFF)
      {
        bufptr = i;
        break;
      }
    }
  }

  while(bufptr < 0)
    {
      pret = poll(&read_pfd, 1, 1000);
      if (pret == 0) {
	printf("SONY: poll timedout !\n");
      } else if (pret < 0) {
	printf("SONY: poll returned error!\n");
      }
    newnumread = read(ptz_fd, temp_reply+numread, MAX_PTZ_REPLY_LENGTH-numread);
    if((numread += newnumread) < 0)
    {
      perror("SonyEVID30::Send():read():");
      return(-1);
    }
    else if(!newnumread)
    {
      // hmm...we were expecting something, yet we read
      // zero bytes. some glitch.  drain input, and return
      // zero.  we'll get a message next time through
      //puts("Receive(): read() returned 0");
      if(tcflush(ptz_fd, TCIFLUSH ) < 0 )
      {
        perror("SonyEVID30::Send():tcflush():");
        return(-1);
      }
      numread = 0;
      return(0);
    }
    // look for the terminator
    for(i=0;i<numread;i++)
    {
      if(temp_reply[i] == 0xFF)
      {
        bufptr = i;
        break;
      }
    }
  }

  temp = numread;
  // if we read extra bytes, keep them around
  if(bufptr == numread-1)
    numread = 0;
  else
  {
    //printf("storing %d bytes\n", numread-(bufptr+1));
    memcpy(buffer,temp_reply+bufptr+1,numread-(bufptr+1));
    numread = numread-(bufptr+1);
  }

  //PrintPacket("Really Received", temp_reply, temp);
  //PrintPacket("Received", temp_reply, bufptr+1);

  // strip off leading trash, up to start character 0x90
  for(i = 0;i< bufptr;i++)
  {
    if(temp_reply[i] == 0x90 && temp_reply[i+1] != 0x90)
      break;
  }
  //if(i)
    //printf("SonyEVID30::Receive(): strip off zeros up to: %d\n", i);
  if(i == bufptr)
    return(0);
  memcpy(reply,temp_reply+i,bufptr+1-i);

  // if it's a command completion, record it, then go again
  if((reply[0] == 0x90) && ((reply[1] >> 4) == 0x05) && (reply[2] == 0xFF))
  {
    //puts("got command completion");
    if((reply[1] & 0x0F) == 0x01)
      command_pending1 = false;
    else if((reply[1] & 0x0F) == 0x02)
      command_pending2 = false;
  }

  return(bufptr+1-i);
}

int SonyEVID30::CancelCommand(char socket)
{
  unsigned char command[MAX_PTZ_MESSAGE_LENGTH];
  unsigned char reply[MAX_PTZ_MESSAGE_LENGTH];
  int reply_len;

  //printf("Canceling socket %d\n", socket);

  command[0] = socket;
  command[0] |= 0x20;

  if((reply_len = Send(command, 1, reply)) <= 0)
    return(reply_len);

  // wait for the response
  while((reply[0] != 0x90) || ((reply[1] >> 4) != 0x06) ||
        !((reply[2] == 0x04) || (reply[2] == 0x05)) || (reply_len != 4))
  {
    if((reply[0] != 0x90) || ((reply[1] >> 4) != 0x05) || (reply[2] != 0xFF))
      PrintPacket("SonyEVID30::CancelCommand(): unexpected response",reply,
                      reply_len);
    //puts("CancelCommand(): calling Receive()");
    if((reply_len = Receive(reply)) <= 0)
      return(reply_len);
  }
  if(socket == 1)
    command_pending1 = false;
  else if(socket == 2)
    command_pending2 = false;
  return(0);
}

int SonyEVID30::SendCommand(unsigned char *str, int len, uint8_t camera)
{
  unsigned char reply[MAX_PTZ_PACKET_LENGTH];
  int reply_len;

  if(command_pending1 && command_pending2)
  {
    if((command_pending1 && CancelCommand(1)) ||
       (command_pending2 && CancelCommand(2)))
      return(-1);
  }

  if(command_pending1 && command_pending2)
  {
    puts("2 commands still pending. wait");
    return(-1);
  }



  if((reply_len = Send(str, len, reply)) <= 0)
    return(reply_len);

  // wait for the ACK
  while((reply[0] != 0x90) || ((reply[1] >> 4) != 0x04) || (reply_len != 3))
  {
    if((reply[0] != 0x90) || ((reply[1] >> 4) != 0x05) || (reply_len != 3))
    {
      PrintPacket("SonyEVID30::SendCommand(): expected ACK, but got",
                      reply,reply_len);
    }
    //puts("SendCommand(): calling Receive()");
    if((reply_len = Receive(reply)) <= 0)
      return(reply_len);
  }

  if((reply[1] & 0x0F) == 0x01)
    command_pending1 = true;
  else if((reply[1] & 0x0F) == 0x02)
    command_pending2 = true;
  else
    fprintf(stderr,"SonyEVID30::SendCommand():got ACK for socket %d\n",
                    reply[1] & 0x0F);

  return(0);
}


int
SonyEVID30::SendRequest(unsigned char* str, int len, unsigned char* reply, uint8_t camera)
{
  int reply_len;

  if((reply_len = Send(str, len, reply, camera)) <= 0)
    return(reply_len);

  // check that it's an information return
  while((reply[0] != 0x90) || (reply[1] != 0x50))
  {
    if((reply[0] != 0x90) || ((reply[1] >> 4) != 0x05) || (reply_len != 3))
    {
      PrintPacket("SonyEVID30::SendCommand(): expected information return, but got",
                    reply,reply_len);
    }
    //puts("SendRequest(): calling Receive()");
    if((reply_len = Receive(reply)) <= 0)
      return(reply_len);
  }

  return(reply_len);
}

int SonyEVID30::SendAbsPanTilt(short pan_cu, short tilt_cu)
{
  unsigned char command[MAX_PTZ_MESSAGE_LENGTH];
  short convpan,convtilt;

  //printf("Send abs pan/tilt: %d, %d\n", pan_cu, tilt_cu);

  if (abs(pan_cu)>(short)PTZ_PAN_MAX)
  {
    if (pan_cu < (short) -PTZ_PAN_MAX)
      pan_cu = (short)-PTZ_PAN_MAX;
    else if (pan_cu > (short) PTZ_PAN_MAX)
      pan_cu = (short)PTZ_PAN_MAX;
    puts("Camera pan angle thresholded");
  }

  if(abs(tilt_cu)>(short)PTZ_TILT_MAX)
  {
    if(tilt_cu<(short)-PTZ_TILT_MAX)
      tilt_cu=(short)-PTZ_TILT_MAX;
    else if(tilt_cu>(short)PTZ_TILT_MAX)
      tilt_cu=(short)PTZ_TILT_MAX;
    puts("Camera tilt angle thresholded");
  }

  convpan = (short)(pan_cu*ptz_pan_conv_factor);
  convtilt = (short)(tilt_cu*ptz_tilt_conv_factor);

  //printf("[Conv] Send abs pan/tilt: %d, %d\n", convpan, convtilt);
  //printf("[Conv] Send abs pan/tilt: %04x, %04x\n", convpan, convtilt);

  command[0] = 0x01;  // absolute position command
  command[1] = 0x06;  // absolute position command
  command[2] = 0x02;  // absolute position command
  command[3] = cam_config_.pan_speed_max_cu;	// max pan speed
  command[4] = cam_config_.tilt_speed_max_cu;	// max tilt speed
  // pan position
  command[5] = (unsigned char)((convpan & 0xF000) >> 12);
  command[6] = (unsigned char)((convpan & 0x0F00) >> 8);
  command[7] = (unsigned char)((convpan & 0x00F0) >> 4);
  command[8] = (unsigned char)(convpan & 0x000F);
  // tilt position
  command[9] = (unsigned char)((convtilt & 0xF000) >> 12);
  command[10] = (unsigned char)((convtilt & 0x0F00) >> 8);
  command[11] = (unsigned char)((convtilt & 0x00F0) >> 4);
  command[12] = (unsigned char)(convtilt & 0x000F);

  return(SendCommand(command, 13));
}

int
SonyEVID30::SendStepPan(int dir)
{
  unsigned char cmd[MAX_PTZ_MESSAGE_LENGTH];

  cmd[0] = 0x01;
  cmd[1] = 0x06;
  cmd[2] = 0x01;
  cmd[3] = PTZ_MAX_PAN_SPEED;
  cmd[4] = PTZ_MAX_TILT_SPEED;
  // if dir >= 0 then pan left, else right
  cmd[5] = dir >= 0 ? 0x01 : 0x02;
  cmd[6] = 0x03;

  printf("step pan\n");

  return (SendCommand(cmd, 7));
}

int
SonyEVID30::SendStepTilt(int dir)
{
  unsigned char cmd[MAX_PTZ_MESSAGE_LENGTH];

  cmd[0] = 0x01;
  cmd[1] = 0x06;
  cmd[2] = 0x01;
  cmd[3] = PTZ_MAX_PAN_SPEED;
  cmd[4] = PTZ_MAX_TILT_SPEED;
  cmd[5] = 0x03;
  // if dir >= 0 then tilt up, else down
  cmd[6] = dir >= 0 ? 0x01 : 0x02;

  return (SendCommand(cmd, 7));
}

int SonyEVID30::SendPanTiltSpeed(short panspeed_cu, short tiltspeed_cu)
{
	unsigned char command[MAX_PTZ_MESSAGE_LENGTH];

	unsigned char pan_dir;
	unsigned char tilt_dir;

	if (panspeed_cu == 0) {
		pan_dir = 0x03;
	} else if (panspeed_cu < 0) {
		pan_dir = 0x01;
	} else {
		pan_dir = 0x02;
	}

	if (tiltspeed_cu == 0) {
		tilt_dir = 0x03;
	} else if (tiltspeed_cu < 0) {
		tilt_dir = 0x02;
	} else {
		tilt_dir = 0x01;
	}

	command[0] = 0x01;
	command[1] = 0x06;
	command[2] = 0x01;
	command[3] = (unsigned char) abs(panspeed_cu);
	command[4] = (unsigned char) abs(tiltspeed_cu);
	command[5] = pan_dir;
	command[6] = tilt_dir;

	/*
	 * Issuing a speed command while a position command is current doesn't
	 * work: it doesn't set the speed and generates an error.  This solution is
	 * probably not ideal, but what the hey, let's just cancel everyone else's
	 * commands since we don't know which of them is a position command (if
	 * any).
	 */
	if (command_pending1) {
		CancelCommand(1);
	}
	if (command_pending2) {
		CancelCommand(2);
	}
	return SendCommand(command, 7);
}

int SonyEVID30::GetAbsPanTilt(short *pan_cu, short *tilt_cu)
{
  unsigned char command[MAX_PTZ_MESSAGE_LENGTH];
  unsigned char reply[MAX_PTZ_PACKET_LENGTH];
  int reply_len;
  short convpan, convtilt;

  command[0] = 0x09;
  command[1] = 0x06;
  command[2] = 0x12;

  if((reply_len = SendRequest(command,3,reply)) <= 0)
    return(reply_len);

  // first two bytes are header (0x90 0x50)

  // next 4 are pan
  convpan = reply[5];
  convpan |= (reply[4] << 4);
  convpan |= (reply[3] << 8);
  convpan |= (reply[2] << 12);

  *pan_cu = (short)(convpan / ptz_pan_conv_factor);

  // next 4 are tilt
  convtilt = reply[9];
  convtilt |= (reply[8] << 4);
  convtilt |= (reply[7] << 8);
  convtilt |= (reply[6] << 12);

  *tilt_cu = (short)(convtilt / ptz_tilt_conv_factor);

  return(0);
}

int SonyEVID30::GetAbsZoom(short *zoom_cu)
{
  unsigned char command[MAX_PTZ_MESSAGE_LENGTH];
  unsigned char reply[MAX_PTZ_PACKET_LENGTH];
  int reply_len;

  command[0] = 0x09;
  command[1] = 0x04;
  command[2] = 0x47;

  if((reply_len = SendRequest(command,3,reply)) <= 0)
    return(reply_len);

  // first two bytes are header (0x90 0x50)
  // next 4 are zoom
  *zoom_cu = reply[5];
  *zoom_cu |= (reply[4] << 4);
  *zoom_cu |= (reply[3] << 8);
  *zoom_cu |= (reply[2] << 12);

  return(0);
}

/*
* Get the device type and version information from the camera.  Query packet is
* in the format: 8x 09 00 02 FF, return packet in the format: y0 50 gg gg hh hh
* jj jj kk FF where
* 	gggg: Vendor ID
*   hhhh: Model ID
*   jjjj: ROM Version
*   kk: socket number (2)
*/
int SonyEVID30::GetCameraType()
{
	unsigned char command[MAX_VER_MESSAGE_LENGTH];
	unsigned char reply[MAX_VER_REPLY_LENGTH];
	int reply_len;

	command[0] = 0x09;
	command[1] = 0x00;
	command[2] = 0x02;

	if ((reply_len = SendRequest(command, 3, reply)) <= 0) {
		return reply_len;
	} else {
      int i;
      int model_id = (reply[4] << 8) + reply[5];
      switch (model_id) {
		case MODEL_D3X:
			this->ptz_pan_conv_factor = 0x0370 / (double) PTZ_PAN_MAX;
			this->ptz_tilt_conv_factor = 0x012C / (double) PTZ_TILT_MAX;
			break;
		case MODEL_D100:
			this->ptz_pan_conv_factor = 0x05A0 / (double) PTZ_PAN_MAX;
			this->ptz_tilt_conv_factor = 0x0168 / (double) PTZ_TILT_MAX;
			break;
		default:
			printf("Unknown camera type: %d\n", model_id);
			break;
		}
		for (i = 0; i < NUM_SONY_EVI_CAMERAS; i++) {
			if (SONY_EVI_CAMERAS[i].model_id == model_id) {
				cam_config_ = SONY_EVI_CAMERAS[i];
				printf(" [Found %s Camera] ", cam_config_.type);
				fflush(stdout);
				break;
			}
		}

		if (i >= NUM_SONY_EVI_CAMERAS) {
			fprintf(stderr, "Unable to identify camera type.  Assuming default.\n");
			cam_config_ = SONY_EVI_CAMERAS[0];
		}

		if (maxfov != 0) {
			cam_config_.fov_max_rad = maxfov;
		}
		if (minfov != 0) {
			cam_config_.fov_min_rad = minfov;
		}
	}
	return 0;
}

/*
 * Power on the device.
 */
int SonyEVID30::PowerOn()
{
	unsigned char power_inq_command[] =  {0x09, 0x04, 0x00};
	unsigned char power_inq_reply[MAX_PTZ_MESSAGE_LENGTH];

	int power_inq_rep_len = SendRequest(power_inq_command, 3, power_inq_reply);
	if (power_inq_rep_len < 4) {
		return -1;
	} else if (power_inq_reply[2] == 0x02) {
		// Power already on
		return 0;
	} else if (power_inq_reply[2] == 0x03) {
		// Power up ...
		unsigned char power_on_command[] = { 0x01, 0x04, 0x00, 0x02 };
		return SendCommand(power_on_command, 4, 7);
	} else {
		return -1;
	}
}

int SonyEVID30::SendAbsZoom(short zoom)
{
  unsigned char command[MAX_PTZ_MESSAGE_LENGTH];

  if(zoom<0) {
    zoom=0;
    //puts("Camera zoom thresholded");
  }
  else if(zoom>1023){
    zoom=1023;
    //puts("Camera zoom thresholded");
  }

  //printf( "Send zoom: %d\n", zoom );

  command[0] = 0x01;  // absolute position command
  command[1] = 0x04;  // absolute position command
  command[2] = 0x47;  // absolute position command

  // zoom position
  command[3] =  (unsigned char)((zoom & 0xF000) >> 12);
  command[4] = (unsigned char)((zoom & 0x0F00) >> 8);
  command[5] = (unsigned char)((zoom & 0x00F0) >> 4);
  command[6] = (unsigned char)(zoom & 0x000F);

  return(SendCommand(command, 7));
}

int SonyEVID30::ProcessGenericRequest(QueuePointer &resp_queue,
									player_msghdr *hdr,
									player_ptz_req_generic_t *req)
{
	/*
	 * VISCA commands come in two major flavours - Commands (for which there is
	 * a completion acknowledgement) and Requests (which return immidiately)
	 * with the data that was requested.
	 */

	if (req->config[0] == VISCA_COMMAND_CODE) {
		if (SendCommand((uint8_t *) req->config, req->config_count) < 0) {
			return -1;
		} else {
			return 0;
		}
	} else {
		// Reply gets stuffed back into cfg->config.
		req->config_count = SendRequest((uint8_t *) req->config,
										req->config_count,
										(uint8_t *) req->config);
		// FIXME: need to send back the data, not just a NACK
		/*
		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
				hdr->subtype);
		*/
		return -1;
	}
}

int SonyEVID30::ProcessPtzRequest(player_ptz_cmd_t *cmd)
{
	switch (control_mode_) {
	case PLAYER_PTZ_VELOCITY_CONTROL:
		// Change of velocity requested?
		if (cmd->panspeed != panspeed_demand_rad
		|| cmd->tiltspeed != tiltspeed_demand_rad) {
			panspeed_demand_rad = cmd->panspeed;
			tiltspeed_demand_rad = cmd->tiltspeed;

			// Translate to camera units
			short panspeed_demand_cu
				= evid_cam_panspeed_cu(cam_config_, panspeed_demand_rad);
			short tiltspeed_demand_cu
				= evid_cam_tiltspeed_cu(cam_config_, tiltspeed_demand_rad);

			// We can't poll the camera for the current speed, so let's just
			// assume the camera is moving at the speed we asked for (which
			// might be a little different from the speed the client
			// requested).  If it hits the pan/tilt limit, we'll reset to 0 in
			// UpdateState.
			panspeed_estimate_rad = evid_cam_panspeed_radians(
										cam_config_, panspeed_demand_cu);
			tiltspeed_estimate_rad = evid_cam_tiltspeed_radians(
										cam_config_, tiltspeed_demand_cu);

			if (SendPanTiltSpeed(panspeed_demand_cu, tiltspeed_demand_cu)) {
				fputs("SonyEVID30:Main():SendPanTiltSpeed() errored.\n", stderr);
				return -1;
			}
			return 0;
		}
		break;
	case PLAYER_PTZ_POSITION_CONTROL:
		// Change of pan/tilt position requested?
		if (cmd->pan != pan_demand_rad || cmd->tilt != tilt_demand_rad) {
			pan_demand_rad = cmd->pan;
			tilt_demand_rad = cmd->tilt;

			// Translate to camera units
			short pan_demand_cu = evid_cam_pan_cu(cam_config_, pan_demand_rad);
			short tilt_demand_cu = evid_cam_tilt_cu(cam_config_, tilt_demand_rad);

			if (SendAbsPanTilt(pan_demand_cu, tilt_demand_cu)) {
				fputs("SonyEVID30:Main():SendAbsPanTilt() errored.\n", stderr);
				return -1;
			}
		}

		// Change of zoom position requested?
		if (cmd->zoom != 0 && cmd->zoom != zoom_demand_rad) {
			zoom_demand_rad = cmd->zoom;

			// Translate to camera units
			short zoom_demand_cu = evid_cam_zoom_cu(cam_config_, zoom_demand_rad);

			if (SendAbsZoom(zoom_demand_cu)) {
				fputs("SonyEVID30:Main():SendAbsZoom() errored.\n", stderr);
				return -1;
			}
		}

		// In theory, the pan/tilt speeds should be (about) max, so here we
		// could set them to their known max values.  The problem is knowing
		// when to set them back to 0 - we'd have to check to see if the target
		// had been reached in UpdateState(), but that's a bit of a PITA to get
		// exactly right, so let's just assume that in POSITION mode, the
		// clients aren't actually interested in the speed.
		panspeed_estimate_rad = 0;
		tiltspeed_estimate_rad = 0;
		break;
	default:
		fputs("Unknown control mode.\n", stderr);
		return -1;
	}

	return 0;
}

int SonyEVID30::ProcessMessage(QueuePointer &resp_queue,
							   player_msghdr * hdr, void *data)
{
  assert(hdr);
  assert(data);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
							PLAYER_PTZ_REQ_GENERIC, device_addr)) {

    assert(hdr->size == sizeof(player_ptz_req_generic_t));

		assert(hdr->size == sizeof(player_ptz_req_generic_t));
		return ProcessGenericRequest(resp_queue, hdr,
									(player_ptz_req_generic_t *) data);

	} else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
							PLAYER_PTZ_CMD_STATE, device_addr)) {
		assert(hdr->size == sizeof(player_ptz_cmd_t));
		player_ptz_cmd_t *cmd = reinterpret_cast<player_ptz_cmd_t*> (data);
		if (ProcessPtzRequest(cmd)) {
			return -1;
		} else {
			return 0;
		}
	} else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
							PLAYER_PTZ_REQ_CONTROL_MODE, device_addr)) {
		// Switch between VELOCITY and POSITION modes.
		player_ptz_req_control_mode_t *cfg =
			reinterpret_cast<player_ptz_req_control_mode_t *> (data);
		if (cfg->mode != PLAYER_PTZ_VELOCITY_CONTROL
		&& cfg->mode != PLAYER_PTZ_POSITION_CONTROL) {
			PLAYER_WARN1("unkown control mode requested: %d", cfg->mode);
			return -1;
		} else {
			control_mode_ = cfg->mode;
        Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
      return 0;
    }
	} else {
  return -1;
	}
}


void SonyEVID30::PrintPacket(const char *str, unsigned char *cmd, int len)
{
  printf("%s: ", str);
  for(int i=0;i<len;i++)
    printf(" %.2x", cmd[i]);
  puts("");
}

int SonyEVID30::UpdateState(player_ptz_data_t &data)
{
	int err;
	short pan_cu, tilt_cu, zoom_cu;

	if ((err = GetAbsPanTilt(&pan_cu, &tilt_cu)) != 0) {
		fprintf(stderr, "SonyEVID30:UpdateState():GetAbsPanTilt() errored.  "
						"Aborting.\n");
		return err;
	}
	if ((err = GetAbsZoom(&zoom_cu)) != 0) {
		fprintf(stderr, "SonyEVID30:UpdateState():GetAbsZoom() errored.  "
						"Aborting.\n");
		return err;
	}

	data.pan = evid_cam_pan_radians(cam_config_, pan_cu);
	data.tilt = evid_cam_tilt_radians(cam_config_, tilt_cu);
	data.zoom = evid_cam_zoom_radians(cam_config_, zoom_cu);

	/*
	 * The camera can't tell us the current pan/tilt speed.  Assume that, in
	 * POSITION mode the client isn't interested.  In VELOCITY mode we assume
	 * that the camera moves at (as close as it can get to) the requested speed
	 * until it hits the pan/tilt limit at which point its speed becomes 0.
	 */

	// Check for pan limit.  Remember that negative pan in camera units is
	// positive pan in radians.
	if ((pan_cu <= -cam_config_.pan_max_cu && panspeed_estimate_rad > 0)
	|| (pan_cu >= cam_config_.pan_max_cu && panspeed_estimate_rad < 0)) {
		panspeed_estimate_rad = 0;
	}
	data.panspeed = panspeed_estimate_rad;

	// Check for tilt limit.
	if ((tilt_cu <= -cam_config_.tilt_max_cu && tiltspeed_estimate_rad < 0)
	|| (tilt_cu >= cam_config_.tilt_max_cu && tiltspeed_estimate_rad > 0)) {
		tiltspeed_estimate_rad = 0;
	}
	data.tiltspeed = tiltspeed_estimate_rad;

/*    // Do the necessary coordinate conversions.  Camera's natural pan
    // coordinates increase clockwise; we want them the other way, so
    // we negate pan here.  Zoom values are converted from arbitrary
    // units to a field of view (in degrees).
    pan = -pan;
    zoom = this->maxfov + (zoom * (this->minfov - this->maxfov)) / 1024;

    if (movement_mode)
	{
	  if (pandemand-pan)
	  {
	    SendStepPan(pandemand-pan);
	  }

	  if (tiltdemand - tilt)
	  {
	    SendStepTilt(tiltdemand - tilt);
	  }
	}

    // Copy the data.
    data.pan = DTOR(pan);
    data.tilt = DTOR(tilt);
    data.zoom = DTOR(zoom);
*/
	return 0;
}

// this function will be run in a separate thread
void SonyEVID30::Main()
{
	while (1) {
    player_ptz_data_t data;

    // Process incoming requests
    pthread_testcancel();
    ProcessMessages();
    pthread_testcancel();


    // Update the ptz state
    if (UpdateState(data))
    {
	  pthread_exit(NULL);
    }

    /* test if we are supposed to cancel */
    pthread_testcancel();
    Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE, &data,sizeof(player_ptz_data_t),NULL);

    usleep(PTZ_SLEEP_TIME_USEC);
    }
}

