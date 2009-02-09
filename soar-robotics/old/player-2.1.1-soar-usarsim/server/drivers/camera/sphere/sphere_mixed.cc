/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al.
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

/**
  * @ingroup drivers
  * @{ */

/**
  *@defgroup driver_sphere sphere
  @brief Logitech Sphere camera

  * This driver is a heavily modified version of the camerav4l one developed
  * by Andrew Howard.  The sphere driver captures images from a Logitech Sphere
  * cameras.  It differs from a normal Video4Linux device in that it has a
  * built in Pan/Tilt/Zoom unit, so this driver supports multiple interfaces.

  * @date 1.2.2005

@image html sphere.png

@par Compile-time dependencies

- &lt;linux/videodev.h&gt;

@par Provides

- @ref interface_camera
- @ref interface_ptz

@par Requires

- none

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/video0"
  - %Device to read video data from.

- size (integer tuple)
  - Default: 320 x 240
  - Desired image size.   This may not be honoured if the driver does
    not support the requested size).

- mode (string)
  - Default: "RGB888"
  - Desired capture mode.  Can be one of:
    - GREY (8-bit monochrome)
    - RGB24P (24-bit planar RGB)
    - RGB888 (24-bit RGB)
    - YUV420P (planar YUV data; not currently implemented)

- save (integer)
  - Default: 0
  - Debugging option: set this to write each frame as an image file on disk.

- framerate (integer)
  - Default: 10
  - The framerate (0..63Hz)

- shutter_speed (integer)
  - Default: 64000
  - The shutter speed (1..65535)

- compression_preference (integer)
  - Default: 2
  - Set compression preference (0..2)

- automatic_gain (integer)
  - Default: 10000
  - Set automatic gain control (0..65535)

- automatic_wb (string)
  - Default: "auto"
  - Set automatic wb mode (auto/manual/indoor/outdoor/fl)

- sharpness (integer)
  - Default: -1
  - The image sharpness (0...65535)

- backlight (integer)
  - Default: 0
  - Set backlight compensation (0=off, other=on)

- flicker (integer)
  - Default: 0
  - Set antiflicker mode (0=off, other=on)

- noise_reduction (integer)
  - Default: 0
  - Set noise reduction mode (0=none...3=high)

- dump_settings (integer)
  - Default: 0
  - Output settings to display when driver starts (0=off, other=on)

- debug (integer)
  - Default: 0
  - Display a copy of the image on localhost (0=off, other=on)

@par Example

@verbatim
driver
(
  name "spheredriver"
  provides ["camera:0"
            "ptz:0"]
  port "/dev/video0"
  size [320 240]
  mode "RGB24"
  automatic_wb "fl"
  framerate 30
  shutter_speed   64000
  automatic_gain  10000
  dump_settings 0
)
@endverbatim


@author Brad Kratochvil
*/
/** @} */

// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/
#include "sphere_mixed.h"

#define DEG2RAD(x) (((double)(x))*0.01745329251994)
#define RAD2DEG(x) (((double)(x))*57.29577951308232)

extern "C" {
#include "setpwc_api.h"
}

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <math.h>
using namespace std;

const timespec NSLEEP_TIME = {0, 10000000}; // (0s, 10 ms) => max 100 fps

////////////////////////////////////////////////////////////////////////////////
// Now the driver

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
SphereDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*)(new SphereDriver(cf, section)));

}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void SphereDriver_Register(DriverTable* table)
{
  table->AddDriver("sphere", SphereDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
SphereDriver::SphereDriver(ConfigFile* cf, int section)
    : Driver(cf, section),
    mPanMin(0),
    mPanMax(1),
    mTiltMin(0),
    mTiltMax(1),
    mDevice(NULL),
    mFg(NULL),
    mFrame(NULL),
    mRGBFrame(NULL),
    mFrameNumber(0),
    mWidth(0),
    mHeight(0),
    mDepth(0)
{
  // Create camera interface
  if (0 !=
    cf->ReadDeviceAddr(&(mCameraAddr),section,"provides",PLAYER_CAMERA_CODE,-1,NULL))
  {
    PLAYER_ERROR("Could not read camera ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mCameraAddr))
  {
    PLAYER_ERROR("Could not add camera interface ");
    SetError(-1);
    return;
  }

  // Create Ptz interface
  if (0 !=
    cf->ReadDeviceAddr(&(mPtzAddr),section,"provides",PLAYER_PTZ_CODE,-1,NULL))
  {
    PLAYER_ERROR("Could not read ptz ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mPtzAddr))
  {
    PLAYER_ERROR("Could not add ptz interface ID ");
    SetError(-1);
    return;
  }

  /// @todo is there a replacement clear command?
  //ClearCommand(mPtzAddr);

  // Read options from the configuration file
  mSleep           =
    static_cast<int32_t>(rint(1e6/cf->ReadInt(section, "frequency", 100)));

  mDevice    = cf->ReadString(section, "port", "/dev/video0");
  mWidth     = cf->ReadTupleInt(section, "size", 0, 320);
  mHeight    = cf->ReadTupleInt(section, "size", 1, 240);
  mPalette   = cf->ReadString(section, "mode", "RGB888");
  mSave      = cf->ReadInt(section, "save", 0);
  mFrameRate = cf->ReadInt(section, "framerate", 10);

  // Not sure what a good default value is here
  mShutterSpeed          = cf->ReadInt(section, "shutter_speed", 64000);
  mCompressionPreference = cf->ReadInt(section, "compression_preference", 2);
  mAutomaticGain         = cf->ReadInt(section, "automatic_gain", 10000);
  mAutomaticWb           = cf->ReadString(section, "automatic_wb", "auto");
  mSharpness             = cf->ReadInt(section, "sharpness", -1);
  mBacklight             = cf->ReadInt(section, "backlight", 0);
  mFlicker               = cf->ReadInt(section, "flicker", 0);
  mNoiseReduction        = cf->ReadInt(section, "noise_reduction", 0);
  mDumpSettings          = cf->ReadInt(section, "dump_settings", 0);
  mDebug                 = cf->ReadInt(section, "debug", 0);

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int SphereDriver::Setup()
{
  puts("Setting up sphere driver");
  mFg = fg_open(mDevice);
  if (mFg == NULL)
  {
    PLAYER_ERROR("Setup():  unable to open ");
    return -1;
  }

  fg_set_format(mFg, VIDEO_PALETTE_YUV420P);
  mFrame = frame_new(mWidth, mHeight, VIDEO_PALETTE_YUV420P );

  if (0 == strcasecmp(mPalette, "GREY"))
  {
    // We don't actually have a greyscale mode, but we can just return
    // the Y component of our YUV.
    mCameraData.format = PLAYER_CAMERA_FORMAT_MONO8;
    mDepth = 8;
  }
  else if (0 == strcasecmp(mPalette, "YUV420P"))
  {
  // What should we have here?
  //    mCameraData.format = PLAYER_CAMERA_FORMAT_YUV420P;
  //    mDepth = 24;
  }
  else if (0 == strcasecmp(mPalette, "RGB24P"))
  {
    mRGBFrame = frame_new(mWidth, mHeight, VIDEO_PALETTE_RGB24 );
    mCameraData.format = PLAYER_CAMERA_FORMAT_RGB;
    mDepth = 24;
  }
  else if (0 == strcasecmp(mPalette, "RGB888"))
  {
    mRGBFrame = frame_new(mWidth, mHeight, VIDEO_PALETTE_RGB24 );
    mCameraData.format = PLAYER_CAMERA_FORMAT_RGB888;
    mDepth = 24;
  }
  else
  {
    PLAYER_ERROR("Palette is not supported ");
    return 1;
  }

  fg_set_capture_window(mFg, 0, 0, mWidth, mHeight);

  // Setup the camera based on configuration options
  set_framerate(mFg->fd, mFrameRate);
  set_shutter_speed(mFg->fd, mShutterSpeed);
  set_compression_preference(mFg->fd, mCompressionPreference);
  set_automatic_gain_control(mFg->fd, mAutomaticGain);
  set_automatic_white_balance_mode(mFg->fd, const_cast<char*>(mAutomaticWb));
  set_sharpness(mFg->fd, mSharpness);
  set_backlight_compensation(mFg->fd, mBacklight);
  set_antiflicker_mode(mFg->fd, mFlicker);
  set_noise_reduction(mFg->fd, mNoiseReduction);

  int32_t min, max;
  get_pan_or_tilt_limits(mFg->fd, GET_PAN, &min, &max);
  mPanMin = DEG2RAD(min/100.0);
  mPanMax = DEG2RAD(max/100.0);
  get_pan_or_tilt_limits(mFg->fd, GET_TILT, &min, &max);
  mTiltMin = DEG2RAD(min/100.0);
  mTiltMax = DEG2RAD(max/100.0);

  if (mDebug)
  {
    printf("min, max:\n");
    printf("pan      %.3f %.3f\n", mPanMin, mPanMax);
    printf("tilt     %.3f %.3f\n", mTiltMin, mTiltMax);
  }

  reset_pan_tilt(mFg->fd, SET_PAN);
  reset_pan_tilt(mFg->fd, SET_TILT);

  if (FALSE!=mDumpSettings)
    dump_current_settings(mFg->fd);

  puts("Sphere driver ready");

  // Start the device thread; spawns a new thread and executes
  // SphereDriver::Main(), which contains the main loop for the driver.
  StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int SphereDriver::Shutdown()
{
  puts("Shutting Sphere driver down");

  // Stop and join the driver thread
  StopThread();

  // Set to 0,0
  set_pan_and_tilt(mFg->fd, 0, 0);

  // Free resources
  if (mFrame != NULL)
    frame_release(mFrame);
  if (mRGBFrame != NULL)
    frame_release(mRGBFrame);
  if (mFg != NULL)
    fg_close(mFg);

  // Here you would shut the device down by, for example, closing a
  // serial port.

  puts("Sphere driver has been shutdown");

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void SphereDriver::Main()
{

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Go to sleep for a while (this is a polling loop)
    nanosleep(&NSLEEP_TIME, NULL);

    // Process incoming messages
    ProcessMessages();

    // Write outgoing data
    RefreshData();

  }
  return;
}

// Process an incoming message
int SphereDriver::ProcessMessage(QueuePointer & resp_queue,
                                 player_msghdr* hdr,
                                 void* data)
{
  assert(hdr);
  assert(data);

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                           PLAYER_PTZ_CMD_STATE, mPtzAddr))
  {
    assert(hdr->size == sizeof(player_ptz_cmd_t));
    ProcessCommand(hdr, *reinterpret_cast<player_ptz_cmd_t *>(data));
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_PTZ_REQ_STATUS, mPtzAddr))
  {
    player_ptz_req_status_t ptz_status;
    int status;

    query_pan_tilt_status(mFg->fd, &status);
    ptz_status.status = (uint32_t) status;

    Publish(device_addr, resp_queue,
      PLAYER_MSGTYPE_RESP_ACK,
      PLAYER_PTZ_REQ_STATUS,
      (void*)&status
    );
    return(0);
  }
  else
  {
    PLAYER_ERROR1("SphereDriver received unknown message: %s", hdr->type);
  }

  return -1;
}

void SphereDriver::ProcessCommand(player_msghdr_t* hdr, player_ptz_cmd_t &data)
{
  int16_t pan,tilt;

  pan       = (int16_t)rint(RAD2DEG(LimitPan(data.pan))*100);
  tilt      = (int16_t)rint(RAD2DEG(LimitTilt(data.tilt))*100);

  mPtzCmd.pan        = DEG2RAD(pan/100.0);
  mPtzCmd.tilt       = DEG2RAD(tilt/100.0);

  // we currently just store the command after checking the limits
  mPtzData.pan       = mPtzCmd.pan;
  mPtzData.tilt      = mPtzCmd.tilt;

  // we don't support these
  mPtzData.zoom      = -1;
  mPtzData.panspeed  = -1;
  mPtzData.tiltspeed = -1;

  // pan is inverted
  set_pan_and_tilt(mFg->fd, -pan, tilt);
}

void SphereDriver::RefreshData()
{
  // Grab the next frame (blocking)
  fg_grab_frame(mFg, mFrame);
  size_t image_size;

  // Compute size of image
  image_size = mWidth * mHeight * mDepth / 8;
  // Set the image properties
  mCameraData.width  = mWidth;
  mCameraData.height = mHeight;
  mCameraData.bpp    = mDepth;
  mCameraData.compression = PLAYER_CAMERA_COMPRESS_RAW;
  mCameraData.image_count = image_size;
  mCameraData.image = new unsigned char [image_size];


  // Copy the image pixels
  if (0 == strcasecmp(mPalette, "GREY"))
  {
    memcpy(mCameraData.image, mFrame->data, image_size);
  }
  else if (0 == strcasecmp(mPalette, "YUV420P"))
  {
    // What should we do here?
    // memcpy(mCameraData.image, mRGBFrame->data, image_size);
  }
  else // RGB
  {
    // We have to do a conversion
    YUV422toRGB(static_cast<uint8_t*>(mFrame->data),
                static_cast<uint8_t*>(mRGBFrame->data));

    assert(image_size <= (size_t) mRGBFrame->size);
    memcpy(mCameraData.image, mRGBFrame->data, image_size);
  }

  Publish(mCameraAddr, 
          PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
          reinterpret_cast<void*>(&mCameraData));

  Publish(mPtzAddr, 
          PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE,
          reinterpret_cast<void*>(&mPtzData));

  // Save frames
  if (FALSE!=mSave)
  {
    char filename[256];
    snprintf(filename, sizeof(filename), "click-%04d.ppm", mFrameNumber++);
    frame_save(mFrame, filename);
  }
  delete [] mCameraData.image;

}

#define LIMIT(p) static_cast<uint8_t>((p < 0) ? 0 : ((p > 255) ? 255 : p))

// These are the lookup tables for the YUV -> RGB conversion
static const int32_t LUT_Y[256]   =
{-4768, -4470, -4172, -3874, -3576, -3278, -2980, -2682, -2384, -2086, -1788,
-1490, -1192, -894, -596, -298, 0, 298, 596, 894, 1192, 1490, 1788, 2086,
2384, 2682, 2980, 3278, 3576, 3874, 4172, 4470, 4768, 5066, 5364, 5662, 5960,
6258, 6556, 6854, 7152, 7450, 7748, 8046, 8344, 8642, 8940, 9238, 9535, 9833,
10131, 10429, 10727, 11025, 11323, 11621, 11919, 12217, 12515, 12813, 13111,
13409, 13707, 14005, 14303, 14601, 14899, 15197, 15495, 15793, 16091, 16389,
16687, 16985, 17283, 17581, 17879, 18177, 18475, 18773, 19071, 19369, 19667,
19965, 20263, 20561, 20859, 21157, 21455, 21753, 22051, 22349, 22647, 22945,
23243, 23541, 23839, 24137, 24435, 24733, 25031, 25329, 25627, 25925, 26223,
26521, 26819, 27117, 27415, 27713, 28010, 28308, 28606, 28904, 29202, 29500,
29798, 30096, 30394, 30692, 30990, 31288, 31586, 31884, 32182, 32480, 32778,
33076, 33374, 33672, 33970, 34268, 34566, 34864, 35162, 35460, 35758, 36056,
36354, 36652, 36950, 37248, 37546, 37844, 38142, 38440, 38738, 39036, 39334,
39632, 39930, 40228, 40526, 40824, 41122, 41420, 41718, 42016, 42314, 42612,
42910, 43208, 43506, 43804, 44102, 44400, 44698, 44996, 45294, 45592, 45890,
46188, 46486, 46783, 47081, 47379, 47677, 47975, 48273, 48571, 48869, 49167,
49465, 49763, 50061, 50359, 50657, 50955, 51253, 51551, 51849, 52147, 52445,
52743, 53041, 53339, 53637, 53935, 54233, 54531, 54829, 55127, 55425, 55723,
56021, 56319, 56617, 56915, 57213, 57511, 57809, 58107, 58405, 58703, 59001,
59299, 59597, 59895, 60193, 60491, 60789, 61087, 61385, 61683, 61981, 62279,
62577, 62875, 63173, 63471, 63769, 64067, 64365, 64663, 64961, 65258, 65556,
65854, 66152, 66450, 66748, 67046, 67344, 67642, 67940, 68238, 68536, 68834,
69132, 69430, 69728, 70026, 70324, 70622, 70920};

static const int32_t LUT_V_R[256] =
{-52298, -51889, -51481, -51072, -50663, -50255, -49846, -49438, -49029,
-48621, -48212, -47803, -47395, -46986, -46578, -46169, -45761, -45352,
-44943, -44535, -44126, -43718, -43309, -42900, -42492, -42083, -41675,
-41266, -40858, -40449, -40040, -39632, -39223, -38815, -38406, -37998,
-37589, -37180, -36772, -36363, -35955, -35546, -35138, -34729, -34320,
-33912, -33503, -33095, -32686, -32278, -31869, -31460, -31052, -30643,
-30235, -29826, -29417, -29009, -28600, -28192, -27783, -27375, -26966,
-26557, -26149, -25740, -25332, -24923, -24515, -24106, -23697, -23289,
-22880, -22472, -22063, -21655, -21246, -20837, -20429, -20020, -19612,
-19203, -18794, -18386, -17977, -17569, -17160, -16752, -16343, -15934,
-15526, -15117, -14709, -14300, -13892, -13483, -13074, -12666, -12257,
-11849, -11440, -11032, -10623, -10214, -9806, -9397, -8989, -8580, -8172,
-7763, -7354, -6946, -6537, -6129, -5720, -5311, -4903, -4494, -4086, -3677,
-3269, -2860, -2451, -2043, -1634, -1226, -817, -409, 0, 409, 817, 1226, 1634,
2043, 2451, 2860, 3269, 3677, 4086, 4494, 4903, 5311, 5720, 6129, 6537, 6946,
7354, 7763, 8172, 8580, 8989, 9397, 9806, 10214, 10623, 11032, 11440, 11849,
12257, 12666, 13074, 13483, 13892, 14300, 14709, 15117, 15526, 15934, 16343,
16752, 17160, 17569, 17977, 18386, 18794, 19203, 19612, 20020, 20429, 20837,
21246, 21655, 22063, 22472, 22880, 23289, 23697, 24106, 24515, 24923, 25332,
25740, 26149, 26557, 26966, 27375, 27783, 28192, 28600, 29009, 29417, 29826,
30235, 30643, 31052, 31460, 31869, 32278, 32686, 33095, 33503, 33912, 34320,
34729, 35138, 35546, 35955, 36363, 36772, 37180, 37589, 37998, 38406, 38815,
39223, 39632, 40040, 40449, 40858, 41266, 41675, 42083, 42492, 42900, 43309,
43718, 44126, 44535, 44943, 45352, 45761, 46169, 46578, 46986, 47395, 47803,
48212, 48621, 49029, 49438, 49846, 50255, 50663, 51072, 51481};

static const int32_t LUT_U_G[256] =
{-12812, -12712, -12612, -12512, -12412, -12312, -12212, -12112, -12012,
-11911, -11811, -11711, -11611, -11511, -11411, -11311, -11211, -11111,
-11011, -10910, -10810, -10710, -10610, -10510, -10410, -10310, -10210,
-10110, -10010, -9910, -9809, -9709, -9609, -9509, -9409, -9309, -9209, -9109,
-9009, -8909, -8808, -8708, -8608, -8508, -8408, -8308, -8208, -8108, -8008,
-7908, -7807, -7707, -7607, -7507, -7407, -7307, -7207, -7107, -7007, -6907,
-6807, -6706, -6606, -6506, -6406, -6306, -6206, -6106, -6006, -5906, -5806,
-5705, -5605, -5505, -5405, -5305, -5205, -5105, -5005, -4905, -4805, -4705,
-4604, -4504, -4404, -4304, -4204, -4104, -4004, -3904, -3804, -3704, -3603,
-3503, -3403, -3303, -3203, -3103, -3003, -2903, -2803, -2703, -2602, -2502,
-2402, -2302, -2202, -2102, -2002, -1902, -1802, -1702, -1602, -1501, -1401,
-1301, -1201, -1101, -1001, -901, -801, -701, -601, -500, -400, -300, -200,
-100, 0, 100, 200, 300, 400, 500, 601, 701, 801, 901, 1001, 1101, 1201, 1301,
1401, 1501, 1602, 1702, 1802, 1902, 2002, 2102, 2202, 2302, 2402, 2502, 2602,
2703, 2803, 2903, 3003, 3103, 3203, 3303, 3403, 3503, 3603, 3704, 3804, 3904,
4004, 4104, 4204, 4304, 4404, 4504, 4604, 4705, 4805, 4905, 5005, 5105, 5205,
5305, 5405, 5505, 5605, 5705, 5806, 5906, 6006, 6106, 6206, 6306, 6406, 6506,
6606, 6706, 6807, 6907, 7007, 7107, 7207, 7307, 7407, 7507, 7607, 7707, 7807,
7908, 8008, 8108, 8208, 8308, 8408, 8508, 8608, 8708, 8808, 8909, 9009, 9109,
9209, 9309, 9409, 9509, 9609, 9709, 9809, 9910, 10010, 10110, 10210, 10310,
10410, 10510, 10610, 10710, 10810, 10910, 11011, 11111, 11211, 11311, 11411,
11511, 11611, 11711, 11811, 11911, 12012, 12112, 12212, 12312, 12412, 12512,
12612};

static const int32_t LUT_V_G[256] =
{-26640, -26432, -26224, -26016, -25808, -25600, -25392, -25183, -24975,
-24767, -24559, -24351, -24143, -23935, -23727, -23518, -23310, -23102,
-22894, -22686, -22478, -22270, -22062, -21853, -21645, -21437, -21229,
-21021, -20813, -20605, -20397, -20188, -19980, -19772, -19564, -19356,
-19148, -18940, -18732, -18523, -18315, -18107, -17899, -17691, -17483,
-17275, -17066, -16858, -16650, -16442, -16234, -16026, -15818, -15610,
-15401, -15193, -14985, -14777, -14569, -14361, -14153, -13945, -13736,
-13528, -13320, -13112, -12904, -12696, -12488, -12280, -12071, -11863,
-11655, -11447, -11239, -11031, -10823, -10615, -10406, -10198, -9990, -9782,
-9574, -9366, -9158, -8950, -8741, -8533, -8325, -8117, -7909, -7701, -7493,
-7284, -7076, -6868, -6660, -6452, -6244, -6036, -5828, -5619, -5411, -5203,
-4995, -4787, -4579, -4371, -4163, -3954, -3746, -3538, -3330, -3122, -2914,
-2706, -2498, -2289, -2081, -1873, -1665, -1457, -1249, -1041, -833, -624,
-416, -208, 0, 208, 416, 624, 833, 1041, 1249, 1457, 1665, 1873, 2081, 2289,
2498, 2706, 2914, 3122, 3330, 3538, 3746, 3954, 4163, 4371, 4579, 4787, 4995,
5203, 5411, 5619, 5828, 6036, 6244, 6452, 6660, 6868, 7076, 7284, 7493, 7701,
7909, 8117, 8325, 8533, 8741, 8950, 9158, 9366, 9574, 9782, 9990, 10198,
10406, 10615, 10823, 11031, 11239, 11447, 11655, 11863, 12071, 12280, 12488,
12696, 12904, 13112, 13320, 13528, 13736, 13945, 14153, 14361, 14569, 14777,
14985, 15193, 15401, 15610, 15818, 16026, 16234, 16442, 16650, 16858, 17066,
17275, 17483, 17691, 17899, 18107, 18315, 18523, 18732, 18940, 19148, 19356,
19564, 19772, 19980, 20188, 20397, 20605, 20813, 21021, 21229, 21437, 21645,
21853, 22062, 22270, 22478, 22686, 22894, 23102, 23310, 23518, 23727, 23935,
24143, 24351, 24559, 24767, 24975, 25183, 25392, 25600, 25808, 26016, 26224};

static const int32_t LUT_U_B[256] =
{-66126, -65609, -65093, -64576, -64059, -63543, -63026, -62510, -61993,
-61476, -60960, -60443, -59927, -59410, -58893, -58377, -57860, -57343,
-56827, -56310, -55794, -55277, -54760, -54244, -53727, -53211, -52694,
-52177, -51661, -51144, -50628, -50111, -49594, -49078, -48561, -48045,
-47528, -47011, -46495, -45978, -45462, -44945, -44428, -43912, -43395,
-42878, -42362, -41845, -41329, -40812, -40295, -39779, -39262, -38746,
-38229, -37712, -37196, -36679, -36163, -35646, -35129, -34613, -34096,
-33580, -33063, -32546, -32030, -31513, -30996, -30480, -29963, -29447,
-28930, -28413, -27897, -27380, -26864, -26347, -25830, -25314, -24797,
-24281, -23764, -23247, -22731, -22214, -21698, -21181, -20664, -20148,
-19631, -19114, -18598, -18081, -17565, -17048, -16531, -16015, -15498,
-14982, -14465, -13948, -13432, -12915, -12399, -11882, -11365, -10849,
-10332, -9816, -9299, -8782, -8266, -7749, -7233, -6716, -6199, -5683, -5166,
-4649, -4133, -3616, -3100, -2583, -2066, -1550, -1033, -517, 0, 517, 1033,
1550, 2066, 2583, 3100, 3616, 4133, 4649, 5166, 5683, 6199, 6716, 7233, 7749,
8266, 8782, 9299, 9816, 10332, 10849, 11365, 11882, 12399, 12915, 13432,
13948, 14465, 14982, 15498, 16015, 16531, 17048, 17565, 18081, 18598, 19114,
19631, 20148, 20664, 21181, 21698, 22214, 22731, 23247, 23764, 24281, 24797,
25314, 25830, 26347, 26864, 27380, 27897, 28413, 28930, 29447, 29963, 30480,
30996, 31513, 32030, 32546, 33063, 33580, 34096, 34613, 35129, 35646, 36163,
36679, 37196, 37712, 38229, 38746, 39262, 39779, 40295, 40812, 41329, 41845,
42362, 42878, 43395, 43912, 44428, 44945, 45462, 45978, 46495, 47011, 47528,
48045, 48561, 49078, 49594, 50111, 50628, 51144, 51661, 52177, 52694, 53211,
53727, 54244, 54760, 55277, 55794, 56310, 56827, 57343, 57860, 58377, 58893,
59410, 59927, 60443, 60960, 61476, 61993, 62510, 63026, 63543, 64059, 64576,
65093};



/**
* Conversion of YUV422 pixel data into RGB ( CCIR 601 )
*
* R = (Y - 16) * 1.164                   + (V-128) * 1.596
* G = (Y - 16) * 1.164 - (U-128) * 0.391 - (V-128) * 0.813
* B = (Y - 16) * 1.164 + (U-128) * 2.018
*
* The formulas are approximated by
*
* R = Y * (291/250) + V * (399/250) - 223
* G = Y * (291/250) + U * (319/1000) - V * (813/1000) + 35
* B = Y * (291/250) + U * (1009/500) - 277
*/

void SphereDriver::YUV422toRGB(uint8_t* argInputData, uint8_t* argOutputData)
{

  uint8_t* Y = argInputData;
  uint8_t* U = Y  + mHeight*mWidth;
  uint8_t* V = U  + mHeight*mWidth/4;

  uint8_t mode = 0;
  uint8_t* R;
  uint8_t* G;
  uint8_t* B;

  if (0 == strcasecmp(mPalette, "RGB24P"))
  {
    R = argOutputData;
    G = argOutputData + mHeight*mWidth;
    B = argOutputData + 2*mHeight*mWidth;
  }
  else // RGB888
  {
    R = argOutputData;
    G = argOutputData + 1;
    B = argOutputData + 2;
    mode = 1;
  }

  for (int32_t y=0;y < mHeight; ++y)
  {
    for (int32_t x=0;x<mWidth; ++x)
    {
      int32_t r,g,b;
      int32_t k=0,l=0;

      k = mWidth*y+x;
      // The /2 are grouped intentionally that way so the decimal
      // parts of each are truncated in that order.
      l = (mWidth/2)*(y/2) + x/2;

      r = LUT_Y[Y[k]]                 + LUT_V_R[V[l]];
      g = LUT_Y[Y[k]] - LUT_U_G[U[l]] - LUT_V_G[V[l]];
      b = LUT_Y[Y[k]] + LUT_U_B[U[l]];

      if (0==mode)
      {
        *R++ = LIMIT(r>>8);
        *G++ = LIMIT(g>>8);
        *B++ = LIMIT(b>>8);
      }
      else
      {
        *R = LIMIT(r>>8);
        *G = LIMIT(g>>8);
        *B = LIMIT(b>>8);

        R += 3;
        G += 3;
        B += 3;
      }

/*
      // This all uses integer math.  It can be optimized using lookup tables
      // I'll leave that as an excercise for the user (or someone using
      // a slower machine).

      int32_t r,g,b;
      int32_t k=0,l=0;

      k = Y[mWidth*y+x] * 291 / 250;
      // The /2 are grouped intentionally that way so the decimal
      // parts of each are truncated in that order.
      l = (mWidth/2)*(y/2) + x/2;

      r = k +  V[l] * 399/250 - 223;
      g = k + (U[l] * 319 - V[l] * 813) / 1000 + 35;
      b = k +  U[l] * 1009/500 - 277;


      *R++ = LIMIT(r);
      *G++ = LIMIT(g);
      *B++ = LIMIT(b);

*/


/*    // This was all for debugging purposes and runs slower due to
      // using all the doubles.  It's good for sanity checking though.
      double r,g,b;

      r = rint((Y[mWidth*y+x]-16.0)*1.164 +
               (V[(mWidth/2)*(y/2) + x/2]-128.0)*1.596);
      g = rint((Y[mWidth*y+x]-16.0)*1.164 -
               (U[(mWidth/2)*(y/2) + x/2]-128.0)*0.391 -
               (V[(mWidth/2)*(y/2) + x/2]-128.0)*0.813);
      b = rint((Y[mWidth*y+x]-16.0)*1.164 +
               (U[(mWidth/2)*(y/2) + x/2]-128.0)*2.018);


      *R++ = LIMIT(r);
      *G++ = LIMIT(g);
      *B++ = LIMIT(b);
*/
    }
  }

#if 0
  if (0!=mDebug)
  {
    // Let's display the image locally to make sure it looks right.
    static CImg<uint8_t> cameraImage;
    cameraImage.width = mWidth;
    cameraImage.height = mHeight;
    cameraImage.depth = 1;//camera->depth;
    cameraImage.dim = mDepth/8;
    cameraImage.data = argOutputData;
    static CImgDisplay player_disp(cameraImage,"Player Image");
    player_disp.display(cameraImage);
  }
#endif

}

#undef LIMIT

#if 0
////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    puts("Sphere driver initializing");
    SphereDriver_Register(table);
    puts("Sphere driver done");
    return(0);
  }
}
#endif
