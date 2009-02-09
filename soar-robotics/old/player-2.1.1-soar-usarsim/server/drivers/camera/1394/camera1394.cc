/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al.
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
///////////////////////////////////////////////////////////////////////////
//
// Desc: 1394 camera capture
// Author: Nate Koenig, Andrew Howard
// Date: 03 June 2004
// CVS: $Id: camera1394.cc 6560 2008-06-13 23:47:47Z thjc $
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_camera1394 camera1394
 * @brief Firewire camera capture

The camera1394 driver captures images from IEEE1394 (Firewire, iLink)
cameras.  

@par Compile-time dependencies

- libraw1394
- libdc1394_control

@par Provides

- @ref interface_camera

@par Requires

- none

@par Configuration requests

- none

@par Configuration file options

- port (integer)
  - Default: 0
  - The 1394 port the camera is attached to.

- node (integer)
  - Default: 0
  - The node within the port

- framerate (float)
  - Default: 15
  - Requested frame rate (frames/second)

- mode (string)
  - Default: "640x480_yuv422"
  - Capture mode (size and color layour).  Valid modes are:
    - "320x240_yuv422"
    - "640x480_mono"
    - "640x480_yuv422"
    - "640x480_rgb"
    - "800x600_mono"
    - "800x600_yuv422" - will be rescaled to 600x450
    - "1024x768_mono"
    - "1024x768_yuv422" - will be rescaled to 512x384
    - "1280x960_mono"
    - "1280x960_yuv422" - will be rescaled to 640x480
    - "FORMAT7_MODE0" - only available with libdc1394 >= 2.0
  - Currently, all mono modes will produce 8-bit monochrome images unless 
  a color decoding option is provided (see bayer).
  - All yuv422 modes are converted to RGB24
  
- force_raw (integer)
  - Default: 0
  - Force the driver to use (slow) memory capture instead of DMA transfer
  (for buggy 1394 drivers).
  
- save (integer)
  - Default: 0
  - Debugging option: set this to write each frame as an image file on disk.

- bayer (string)
  - Default: None.
  - Bayer color decoding options for cameras such as the Point Grey Dragonfly and Bummblebee. 
  Option activates color decoding and specifies the Bayer color pattern. Valid modes are:
    - "BGGR"
    - "GRBG"
    - "RGGB"
    - "GBRG"

- method (string)    
  - Default: None (or "DownSample" if bayer option is specified)
  - Determines the algorithm used for Bayer coloro decoding. Valid modes are:
    - "DownSample"
    - "Nearest"
    - "Edge"

- brightness (string or unsigned int)
  - Default: None
  - Sets the camera brightness setting. Valid modes are:
    - "auto"
    - any suitable unsigned integer

- exposure (string or unsigned int)
  - Default: None
  - Sets the camera exposure setting. Valid modes are:
    - "auto"
    - any suitable unsigned integer

- shutter (string or unsigned int)
  - Default: None
  - Sets the camera shutter setting. Valid modes are:
    - "auto"
    - any suitable unsigned integer

- gain (string or unsigned int)
  - Default: None
  - Sets the camera gain setting. Valid modes are:
    - "auto"
    - any suitable unsigned integer

- whitebalance (string)
  - Default: None
  - Sets the manual camera white balance setting. Only valid option:
    - a string containing two suitable blue and red value unsigned integers 

- dma_buffers
  - Default: 4
  - the number of DMA buffers to use

- iso_speed (unsigned int)
  - Default: 400
  - Sets the data rate of the 1394 bus. Valid rates are 100, 200, 400, 800, 1600, 3200.
@par Example 

@verbatim
driver
(
  name "camera1394"
  provides ["camera:0"]
)
@endverbatim

@author Nate Koenig, Andrew Howard; major code rewrite by Paul Osmialowski, newchief@king.net.pl
*/
/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <stddef.h>       // for NULL
#include <unistd.h>
#include <assert.h>

#ifdef HAVE_LIBRAW1394
    #include <libraw1394/raw1394.h>
#endif

#if DC1394_DMA_SETUP_CAPTURE_ARGS == 20
#include <dc1394/control.h>
#define LIBDC1394_VERSION  0200
#else
#include <libdc1394/dc1394_control.h>
#endif
#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

// for color and format conversion (located in cmvision)
#include "conversions.h"

#define NUM_DMA_BUFFERS 4


// lots of defines are renames in v2 API, mask this so we dont have to
// modify all our code
#if DC1394_DMA_SETUP_CAPTURE_ARGS == 20
// Frame rate enum
#define FRAMERATE_1_875 DC1394_FRAMERATE_1_875
#define FRAMERATE_3_75 DC1394_FRAMERATE_3_75
#define FRAMERATE_7_5 DC1394_FRAMERATE_7_5
#define FRAMERATE_15 DC1394_FRAMERATE_15
#define FRAMERATE_30 DC1394_FRAMERATE_30
#define FRAMERATE_60 DC1394_FRAMERATE_60
#define FRAMERATE_120 DC1394_FRAMERATE_120
#define FRAMERATE_240 DC1394_FRAMERATE_240

// Format - not used in new API
#define FORMAT_VGA_NONCOMPRESSED 0
#define FORMAT_SVGA_NONCOMPRESSED_1 0
#define FORMAT_SVGA_NONCOMPRESSED_2 0
#define FORMAT_6 0
#define FORMAT_7 0

// mode enumneration
#define 	  MODE_160x120_YUV444	  DC1394_VIDEO_MODE_160x120_YUV444
#define 	  MODE_320x240_YUV422	  DC1394_VIDEO_MODE_320x240_YUV422
#define 	  MODE_640x480_YUV411	  DC1394_VIDEO_MODE_640x480_YUV411
#define 	  MODE_640x480_YUV422	  DC1394_VIDEO_MODE_640x480_YUV422
#define 	  MODE_640x480_RGB	  DC1394_VIDEO_MODE_640x480_RGB8
#define 	  MODE_640x480_MONO	  DC1394_VIDEO_MODE_640x480_MONO8
#define 	  MODE_640x480_MONO16	  DC1394_VIDEO_MODE_640x480_MONO16
#define 	  MODE_800x600_YUV422	  DC1394_VIDEO_MODE_800x600_YUV422
#define 	  MODE_800x600_RGB	  DC1394_VIDEO_MODE_800x600_RGB8
#define 	  MODE_800x600_MONO	  DC1394_VIDEO_MODE_800x600_MONO8
#define 	  MODE_1024x768_YUV422	  DC1394_VIDEO_MODE_1024x768_YUV422
#define 	  MODE_1024x768_RGB	  DC1394_VIDEO_MODE_1024x768_RGB8
#define 	  MODE_1024x768_MONO	  DC1394_VIDEO_MODE_1024x768_MONO8
#define 	  MODE_800x600_MONO16	  DC1394_VIDEO_MODE_800x600_MONO16
#define 	  MODE_1024x768_MONO16	  DC1394_VIDEO_MODE_1024x768_MONO16
#define 	  MODE_1280x960_YUV422	  DC1394_VIDEO_MODE_1280x960_YUV422
#define 	  MODE_1280x960_RGB	  DC1394_VIDEO_MODE_1280x960_RGB8
#define 	  MODE_1280x960_MONO	  DC1394_VIDEO_MODE_1280x960_MONO8
#define 	  MODE_1600x1200_YUV422	  DC1394_VIDEO_MODE_1600x1200_YUV422
#define 	  MODE_1600x1200_RGB	  DC1394_VIDEO_MODE_1600x1200_RGB8
#define 	  MODE_1600x1200_MONO	  DC1394_VIDEO_MODE_1600x1200_MONO8
#define 	  MODE_1280x960_MONO16	  DC1394_VIDEO_MODE_1280x960_MONO16
#define 	  MODE_1600x1200_MONO16	  DC1394_VIDEO_MODE_1600x1200_MONO16
#define 	  MODE_EXIF	          DC1394_VIDEO_MODE_EXIF
#define 	  MODE_FORMAT7_0	  DC1394_VIDEO_MODE_FORMAT7_0
#define 	  MODE_FORMAT7_1	  DC1394_VIDEO_MODE_FORMAT7_1
#define 	  MODE_FORMAT7_2	  DC1394_VIDEO_MODE_FORMAT7_2
#define 	  MODE_FORMAT7_3	  DC1394_VIDEO_MODE_FORMAT7_3
#define 	  MODE_FORMAT7_4	  DC1394_VIDEO_MODE_FORMAT7_4
#define 	  MODE_FORMAT7_5	  DC1394_VIDEO_MODE_FORMAT7_5
#define 	  MODE_FORMAT7_6	  DC1394_VIDEO_MODE_FORMAT7_6
#define 	  MODE_FORMAT7_7	  DC1394_VIDEO_MODE_FORMAT7_7

// Feature enumeration
#define	  FEATURE_BRIGHTNESS	  DC1394_FEATURE_BRIGHTNESS
#define	  FEATURE_EXPOSURE	  DC1394_FEATURE_EXPOSURE
#define	  FEATURE_SHARPNESS	  DC1394_FEATURE_SHARPNESS
#define	  FEATURE_WHITE_BALANCE	  DC1394_FEATURE_WHITE_BALANCE
#define	  FEATURE_HUE	  DC1394_FEATURE_HUE
#define	  FEATURE_SATURATION	  DC1394_FEATURE_SATURATION
#define	  FEATURE_GAMMA	  DC1394_FEATURE_GAMMA
#define	  FEATURE_SHUTTER	  DC1394_FEATURE_SHUTTER
#define	  FEATURE_GAIN	  DC1394_FEATURE_GAIN
#define	  FEATURE_IRIS	  DC1394_FEATURE_IRIS
#define	  FEATURE_FOCUS	  DC1394_FEATURE_FOCUS
#define	  FEATURE_TEMPERATURE	  DC1394_FEATURE_TEMPERATURE
#define	  FEATURE_TRIGGER	  DC1394_FEATURE_TRIGGER
#define	  FEATURE_TRIGGER_DELAY	  DC1394_FEATURE_TRIGGER_DELAY
#define	  FEATURE_WHITE_SHADING	  DC1394_FEATURE_WHITE_SHADING
#define	  FEATURE_FRAME_RATE	  DC1394_FEATURE_FRAME_RATE
#define	  FEATURE_ZOOM	  DC1394_FEATURE_ZOOM
#define	  FEATURE_PAN	  DC1394_FEATURE_PAN
#define	  FEATURE_TILT	  DC1394_FEATURE_TILT
#define	  FEATURE_OPTICAL_FILTER	  DC1394_FEATURE_OPTICAL_FILTER
#define	  FEATURE_CAPTURE_SIZE	  DC1394_FEATURE_CAPTURE_SIZE
#define	  FEATURE_CAPTURE_QUALITY	  DC1394_FEATURE_CAPTURE_QUALITY

#else

// speed enumerations
#define DC1394_ISO_SPEED_100 SPEED_100
#define DC1394_ISO_SPEED_200 SPEED_200
#define DC1394_ISO_SPEED_400 SPEED_400
#ifdef SPEED_800
#define DC1394_ISO_SPEED_800 SPEED_800
#endif
#ifdef SPEED_1600
#define DC1394_ISO_SPEED_1600 SPEED_1600
#endif
#ifdef SPEED_3200
#define DC1394_ISO_SPEED_3200 SPEED_3200
#endif

#endif

// Driver for detecting laser retro-reflectors.
class Camera1394 : public Driver
{
  // Constructor
  public: Camera1394( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();
  private: void SafeCleanup();

  // Main function for device thread.
  private: virtual void Main();

  // This method will be invoked on each incoming message
  public: virtual int ProcessMessage(QueuePointer & resp_queue,
                                     player_msghdr * hdr,
                                     void * data);

  // Save a frame to memory
  private: int GrabFrame();

  // Save a frame to disk
  private: int SaveFrame( const char *filename );

  // Update the device data (the data going back to the client).
  private: void RefreshData();

  // Video device
  private: unsigned int port;
  private: unsigned int node;

#ifdef HAVE_LIBRAW1394
  private: raw1394handle_t handle;
#endif

#if LIBDC1394_VERSION == 0200
  private: dc1394camera_t * camera;
  // Camera features
  private: dc1394featureset_t features;
  private: dc1394format7modeset_t modeset;
#else  
  private: dc1394_cameracapture camera;
  // Camera features
  private: dc1394_feature_set features;
#endif


  // Capture method: RAW or VIDEO (DMA)
  private: enum {methodRaw, methodVideo, methodNone};
  private: int method;
  private: bool forceRaw;

#if LIBDC1394_VERSION == 0200
  private: dc1394framerate_t frameRate;
  private: unsigned int format;
  private: dc1394video_mode_t mode;
  private: dc1394speed_t iso_speed;
#else
  private: unsigned int frameRate;
  private: unsigned int format;
  private: unsigned int mode;
  private: unsigned int iso_speed;
#endif

  // number of DMA buffers to use
  private: unsigned int num_dma_buffers;

  // Write frames to disk?
  private: int save;

  // Capture timestamp
  private: double frameTime;
  
  // Data to send to server
  private: player_camera_data_t * data;

  // Bayer Colour Conversion
  private: bool DoBayerConversion;
  private: bayer_pattern_t BayerPattern;
  private: int BayerMethod;

  // Camera settings
  private: bool setBrightness, setExposure, setWhiteBalance, setShutter, setGain;
  private: bool autoBrightness, autoExposure, autoShutter, autoGain;
  private: unsigned int brightness, exposure, redBalance, blueBalance, shutter, gain;

};


// Initialization function
Driver* Camera1394_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new Camera1394( cf, section)));
}


// a driver registration function
void Camera1394_Register(DriverTable* table)
{
  table->AddDriver("camera1394", Camera1394_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
Camera1394::Camera1394(ConfigFile* cf, int section)
  : Driver(cf,
           section,
           true,
           PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
           PLAYER_CAMERA_CODE)
{
  float fps;

  this->data = NULL;

#ifdef HAVE_LIBRAW1394
  this->handle = NULL;
#endif
  this->method = methodNone;

  // The port the camera is attached to
  this->port = cf->ReadInt(section, "port", 0);

  // The node inside the port
  this->node = cf->ReadInt(section, "node", 0);

  // Video frame rate
  fps = cf->ReadFloat(section, "framerate", 15);
  if (fps < 3.75)
    this->frameRate = FRAMERATE_1_875;
  else if (fps < 7.5)
    this->frameRate = FRAMERATE_3_75;
  else if (fps < 15)
    this->frameRate = FRAMERATE_7_5;
  else if (fps < 30)
    this->frameRate = FRAMERATE_15;
  else if (fps < 60)
    this->frameRate = FRAMERATE_30;
  else
    this->frameRate = FRAMERATE_60;

  // Get uncompressed video
  this->format = FORMAT_VGA_NONCOMPRESSED;
    
  // Image size. This determines the capture resolution. There are a limited
  // number of options available. At 640x480, a camera can capture at
  // _RGB or _MONO or _MONO16.  
  const char* str;
  str =  cf->ReadString(section, "mode", "640x480_yuv422");
  /*
  if (0==strcmp(str,"160x120_yuv444"))
  {
    this->mode = MODE_160x120_YUV444;
  }
  */
  if (0==strcmp(str,"320x240_yuv422"))
  {
    this->mode = MODE_320x240_YUV422;
  }
  /*
  else if (0==strcmp(str,"640x480_mono16"))
  {
    this->mode = MODE_640x480_MONO16;
    assert(false);
  }
  else if (0==strcmp(str,"640x480_yuv411"))
  {
    this->mode = MODE_640x480_YUV411;
  }
  */
  else if (0==strcmp(str,"640x480_mono"))
  {
    this->mode = MODE_640x480_MONO;
  }
  else if (0==strcmp(str,"640x480_yuv422"))
  {
    this->mode = MODE_640x480_YUV422;
  }
  else if (0==strcmp(str,"640x480_rgb"))
  {
    this->mode = MODE_640x480_RGB;
  }
  else if (0==strcmp(str,"800x600_mono"))
  {
    this->mode = MODE_800x600_MONO;
    this->format = FORMAT_SVGA_NONCOMPRESSED_1;
  }
  else if (0==strcmp(str,"800x600_yuv422"))
  {
    this->mode = MODE_800x600_YUV422;
    this->format = FORMAT_SVGA_NONCOMPRESSED_1;
  }
  else if (0==strcmp(str,"1024x768_mono"))
  {
    this->mode = MODE_1024x768_MONO;
    this->format = FORMAT_SVGA_NONCOMPRESSED_1;
  }
  else if (0==strcmp(str,"1024x768_yuv422"))
  {
    this->mode = MODE_1024x768_YUV422;
    this->format = FORMAT_SVGA_NONCOMPRESSED_1;
  }
  else if (0==strcmp(str,"1280x960_mono"))
  {
    this->mode = MODE_1280x960_MONO;
    this->format = FORMAT_SVGA_NONCOMPRESSED_2;
  }
  else if (0==strcmp(str,"1280x960_yuv422"))
  {
    this->mode = MODE_1280x960_YUV422;
    this->format = FORMAT_SVGA_NONCOMPRESSED_2;
  }
#if LIBDC1394_VERSION == 0200
  else if (0==strcmp(str,"FORMAT7_MODE0"))
  {
    this->mode = MODE_FORMAT7_0;
    this->format = FORMAT_7;
  }
#endif
  else
  {
    PLAYER_ERROR1("unknown video mode [%s]", str);
    this->SetError(-1);
    return;
  }

  // Many cameras such as the Pt Grey Dragonfly and Bumblebee devices
  // don't do onchip colour conversion. Various Bayer colour encoding 
  // patterns and decoding methods exist. They are now presented to the 
  // user as config file options.
  // check Bayer colour decoding option
  str =  cf->ReadString(section, "bayer", "NONE");
  this->DoBayerConversion = false;
  if (strcmp(str,"NONE"))
       {
	    if (!strcmp(str,"BGGR"))
		 {
		      this->DoBayerConversion = true;
		      this->BayerPattern = BAYER_PATTERN_BGGR;
		 }
	    else if (!strcmp(str,"GRBG"))
		 {
		      this->DoBayerConversion = true;
		      this->BayerPattern = BAYER_PATTERN_GRBG;
		 }
	    else if (!strcmp(str,"RGGB"))
		 {
		      this->DoBayerConversion = true;
		      this->BayerPattern = BAYER_PATTERN_RGGB;
		 }
	    else if (!strcmp(str,"GBRG"))
		 {
		      this->DoBayerConversion = true;
		      this->BayerPattern = BAYER_PATTERN_GBRG;
		 }
	    else
		 {
		      PLAYER_ERROR1("unknown bayer pattern [%s]", str);
		      this->SetError(-1);
		      return;
		 }
       }
  // Set default Bayer decoding method
  if (this->DoBayerConversion)
       this->BayerMethod = BAYER_DECODING_DOWNSAMPLE;
  else
       this->BayerMethod = NO_BAYER_DECODING;
  // Check for user selected method
  str =  cf->ReadString(section, "method", "NONE");
  if (strcmp(str,"NONE"))
       {
	    if (!this->DoBayerConversion)
		 {
		      PLAYER_ERROR1("bayer method [%s] specified without enabling bayer conversion", str);
		      this->SetError(-1);
		      return;
		 }
	    if (!strcmp(str,"DownSample"))
		 {
		      this->BayerMethod = BAYER_DECODING_DOWNSAMPLE;
		 }
	    else if (!strcmp(str,"Nearest"))
		 {
		      this->BayerMethod = BAYER_DECODING_NEAREST;
		 }
	    else if (!strcmp(str,"Edge"))
		 {
		      this->BayerMethod = BAYER_DECODING_EDGE_SENSE;
		 }
	    else
		 {
		      PLAYER_ERROR1("unknown bayer method [%s]", str);
		      this->SetError(-1);
		      return;
		 }
       }

  // Allow the user to set useful camera setting options
  // brightness, exposure, redBalance, blueBalance, shutter and gain
  // Parse camera settings - default is to leave them alone. 
  str =  cf->ReadString(section, "brightness", "NONE");
  if (strcmp(str,"NONE"))
       if (!strcmp(str,"auto"))
	    {
		 this->setBrightness=true;
		 this->autoBrightness=true;
	    }
       else
	    {
		 this->setBrightness=true;
		 this->autoBrightness=false;
		 this->brightness = atoi(str);		 
	    }
  str =  cf->ReadString(section, "exposure", "NONE");
  if (strcmp(str,"NONE"))
       if (!strcmp(str,"auto"))
	    {
		 this->setExposure=true;
		 this->autoExposure=true;
	    }
       else
	    {
		 this->setExposure=true;
		 this->autoExposure=false;
		 this->exposure = atoi(str);		 
	    }
  str =  cf->ReadString(section, "shutter", "NONE");
  if (strcmp(str,"NONE"))
       if (!strcmp(str,"auto"))
	    {
		 this->setShutter=true;
		 this->autoShutter=true;
	    }
       else
	    {
		 this->setShutter=true;
		 this->autoShutter=false;
		 this->shutter = atoi(str);		 
	    }
  str =  cf->ReadString(section, "gain", "NONE");
  if (strcmp(str,"NONE"))
       if (!strcmp(str,"auto"))
	    {
		 this->setGain=true;
		 this->autoGain=true;
	    }
       else
	    {
		 this->setGain=true;
		 this->autoGain=false;
		 this->gain = atoi(str);		 
	    }
  str =  cf->ReadString(section, "whitebalance", "NONE");
  if (strcmp(str,"NONE"))
       if(sscanf(str,"%u %u",&this->blueBalance,&this->redBalance)==2)
	    this->setWhiteBalance=true;
       else
	    PLAYER_ERROR1("didn't understand white balance values [%s]", str);

  // Force into raw mode
  this->forceRaw = cf->ReadInt(section, "force_raw", 0);
 
  // Save frames?
  this->save = cf->ReadInt(section, "save", 0);

  // Number of DMA buffers?
  this->num_dma_buffers = cf->ReadInt(section, "dma_buffers", NUM_DMA_BUFFERS);

  // ISO Speed?
  switch(cf->ReadInt(section, "iso_speed", 400)) {
    case 100:
      this->iso_speed = DC1394_ISO_SPEED_100;
      break;
    case 200:
      this->iso_speed = DC1394_ISO_SPEED_200;
      break;
    case 400:
      this->iso_speed = DC1394_ISO_SPEED_400;
      break;
#ifdef DC1394_ISO_SPEED_800
    case 800:
      this->iso_speed = DC1394_ISO_SPEED_800;
      break;
#endif
#ifdef DC1394_ISO_SPEED_1600
    case 1600:
      this->iso_speed = DC1394_ISO_SPEED_1600;
      break;
#endif
#ifdef DC1394_ISO_SPEED_3200
    case 3200:
      this->iso_speed = DC1394_ISO_SPEED_3200;
      break;
#endif
    default:
      PLAYER_ERROR("Unsupported iso_speed");
      this->SetError(-1);
      return;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Safe Cleanup
void Camera1394::SafeCleanup()
{

#if LIBDC1394_VERSION == 0200
  if (this->camera)
  {
    //dc1394_cleanup_iso_channels_and_bandwidth(camera);
    /* The above function has been removed from libdc1394 without a clear
       replacement...
      http://sourceforge.net/mailarchive/message.php?msg_id=1196638611.10412.31.camel%40pisces.mit.edu
    */
    switch (this->method)
    {
    case methodRaw:
      //dc1394_release_camera(this->camera);
      break;
    case methodVideo:
      //dc1394_dma_unlisten(this->camera);
      //dc1394_dma_release_camera(this->camera);
      break;
    }
    dc1394_camera_free(this->camera);
  }
  this->camera = NULL;
#else
  if (this->handle)
  {
    switch (this->method)
    {
    case methodRaw:
      dc1394_release_camera(this->handle, &this->camera);
      break;
    case methodVideo:
      dc1394_dma_unlisten(this->handle, &this->camera);
      dc1394_dma_release_camera(this->handle, &this->camera);
      break;
    }
    dc1394_destroy_handle(this->handle);
  }
  this->handle = NULL;
#endif
  if (this->data)
  {
    if (this->data->image) delete[](this->data->image);
    this->data->image = NULL;
    delete (this->data);
  }
  this->data = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int Camera1394::Setup()
{
  

#if LIBDC1394_VERSION == 0200
  dc1394speed_t speed;
#else
  unsigned int channel, speed;
#endif

  // Create a handle for the given port (port will be zero on most
  // machines)
#if LIBDC1394_VERSION == 0200
  // First we try to find a camera
  int err;
  dc1394_t *d;
  dc1394camera_list_t *list;

  d = dc1394_new ();
  err = dc1394_camera_enumerate (d, &list);
  if (err != DC1394_SUCCESS)
  {
    PLAYER_ERROR1("Could not get Camera List: %d\n", err);	
    return -1;
  }

  if (list->num == 0)
  {
    PLAYER_ERROR("No cameras found");
    return -1;
  }

  // we just use the first one returned and then free the rest
  camera = dc1394_camera_new (d, list->ids[0].guid);
  
  if (!camera)
  {
    PLAYER_ERROR1("Failed to initialize camera with guid %llx",
                  list->ids[0].guid);
    this->SafeCleanup();
    return -1;
  }

  dc1394_camera_free_list (list);

  //dc1394_cleanup_iso_channels_and_bandwidth(camera);
  /* Above has been removed from API. */

#else
  this->handle = dc1394_create_handle(this->port);
  if (this->handle == NULL)
  {
    PLAYER_ERROR("Unable to acquire a dc1394 handle");
    this->SafeCleanup();
    return -1;
  }

  this->camera.node = this->node;
  this->camera.port = this->port;
#endif

  // apply user config file provided camera settings
  if (this->setBrightness)
  {
#if LIBDC1394_VERSION == 0200
    if (DC1394_SUCCESS != dc1394_feature_set_mode(this->camera, FEATURE_BRIGHTNESS,this->autoBrightness ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL))
#else
    if (DC1394_SUCCESS != dc1394_auto_on_off(this->handle, this->camera.node,FEATURE_BRIGHTNESS,this->autoBrightness))
#endif
    {
      PLAYER_ERROR("Unable to set Brightness mode");
      this->SafeCleanup();
      return -1;
    }
    if (!this->autoBrightness)
    {
#if LIBDC1394_VERSION == 0200
      if (DC1394_SUCCESS != dc1394_feature_set_value(this->camera, FEATURE_BRIGHTNESS,this->brightness))
#else
      if (DC1394_SUCCESS != dc1394_set_brightness(this->handle, this->camera.node,this->brightness))
#endif
      {
        PLAYER_ERROR("Unable to set Brightness value");
        this->SafeCleanup();
        return -1;
      }
    }
  }
  if (this->setExposure)
  {
#if LIBDC1394_VERSION == 0200
    if (DC1394_SUCCESS != dc1394_feature_set_mode(this->camera, FEATURE_EXPOSURE,this->autoExposure ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL))
#else
    if (DC1394_SUCCESS != dc1394_auto_on_off(this->handle, this->camera.node,FEATURE_EXPOSURE,this->autoExposure))
#endif
    {
      PLAYER_ERROR("Unable to set Exposure mode");
		  this->SafeCleanup();
		  return -1;
    }
    if (!this->autoExposure)
    {
#if LIBDC1394_VERSION == 0200
      if (DC1394_SUCCESS != dc1394_feature_set_value(this->camera, FEATURE_EXPOSURE,this->exposure))
#else
      if (DC1394_SUCCESS != dc1394_set_exposure(this->handle, this->camera.node,this->exposure))
#endif
      {
		    PLAYER_ERROR("Unable to set Exposure value");
		    this->SafeCleanup();
		    return -1;
		  }
    }
  }
  if (this->setShutter)
  {
#if LIBDC1394_VERSION == 0200
    if (DC1394_SUCCESS != dc1394_feature_set_mode(this->camera, FEATURE_SHUTTER,this->autoShutter ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL))
#else
    if (DC1394_SUCCESS != dc1394_auto_on_off(this->handle, this->camera.node,FEATURE_SHUTTER,this->autoShutter))
#endif
    {
		  PLAYER_ERROR("Unable to set Shutter mode");
		  this->SafeCleanup();
		  return -1;
		}
	  if (!this->autoShutter)
    {
#if LIBDC1394_VERSION == 0200
      if (DC1394_SUCCESS != dc1394_feature_set_value(this->camera, FEATURE_SHUTTER,this->shutter))
#else
      if (DC1394_SUCCESS != dc1394_set_shutter(this->handle, this->camera.node,this->shutter))
#endif
      {
		    PLAYER_ERROR("Unable to set Shutter value");
		    this->SafeCleanup();
		    return -1;
		  }
    }
  }
  if (this->setGain)
  {
#if LIBDC1394_VERSION == 0200
    if (DC1394_SUCCESS != dc1394_feature_set_mode(this->camera, FEATURE_GAIN,this->autoGain ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL))
#else
    if (DC1394_SUCCESS != dc1394_auto_on_off(this->handle, this->camera.node,FEATURE_GAIN,this->autoGain))
#endif
    {
		  PLAYER_ERROR("Unable to set Gain mode");
		  this->SafeCleanup();
		  return -1;
		}
	  if (!this->autoShutter)
    {
#if LIBDC1394_VERSION == 0200
      if (DC1394_SUCCESS != dc1394_feature_set_value(this->camera, FEATURE_GAIN,this->gain))
#else
		  if (DC1394_SUCCESS != dc1394_set_gain(this->handle, this->camera.node,this->gain))
#endif
      {
		    PLAYER_ERROR("Unable to Gain value");
		    this->SafeCleanup();
		    return -1;
		  }
    }
  }
  if (this->setWhiteBalance)
  {
#if LIBDC1394_VERSION == 0200
    if (DC1394_SUCCESS != dc1394_feature_whitebalance_set_value(this->camera,this->blueBalance,this->redBalance))
#else
    if (DC1394_SUCCESS != dc1394_set_white_balance(this->handle, this->camera.node,this->blueBalance,this->redBalance))
#endif
		{
		  PLAYER_ERROR("Unable to set White Balance");
		  this->SafeCleanup();
		  return -1;
		}
  }

  // Collects the available features for the camera described by node and
  // stores them in features.
#if LIBDC1394_VERSION == 0200
  if (DC1394_SUCCESS != dc1394_feature_get_all(camera, &this->features))
#else
  if (DC1394_SUCCESS != dc1394_get_camera_feature_set(this->handle,
                                                      this->camera.node, 
                                                      &this->features))
#endif
  {
    PLAYER_ERROR("Unable to get feature set");
    this->SafeCleanup();
    return -1;
  }

  // TODO: need to indicate what formats the camera supports somewhere
  // Remove; leave?

#if LIBDC1394_VERSION == 0200
  // if format 7 requested check that it is supported
  if (FORMAT_7==this->format && DC1394_SUCCESS!=dc1394_format7_get_modeset(camera, &modeset))
  {
    bool HasMode7 = false;
    for (unsigned int i=0;i<DC1394_VIDEO_MODE_FORMAT7_NUM;i++) 
    {
      if (modeset.mode[i].present!=0) 
      {
        HasMode7 = true;
        break;	
      }
    }
    if (!HasMode7)
    {
      PLAYER_ERROR("Could not set Format 7");
      this->SafeCleanup();
      return -1;
    }
  }
#endif

  // Get the ISO channel and speed of the video
#if LIBDC1394_VERSION == 0200
  if (DC1394_SUCCESS != dc1394_video_get_iso_speed(this->camera, &speed))
#else
  if (DC1394_SUCCESS != dc1394_get_iso_channel_and_speed(this->handle, this->camera.node, 
                                                         &channel, &speed))
#endif
  {
    PLAYER_ERROR("Unable to get iso data; is the camera plugged in?");
    this->SafeCleanup();
    return -1;
  }


#if DC1394_DMA_SETUP_CAPTURE_ARGS == 11
  // Set camera to use DMA, improves performance.
  if (!this->forceRaw &&
      dc1394_dma_setup_capture(this->handle, this->camera.node, channel,
                               this->format, this->mode, this->iso_speed,
                               this->frameRate, this->num_dma_buffers, 1, NULL,
                               &this->camera) == DC1394_SUCCESS)
#elif DC1394_DMA_SETUP_CAPTURE_ARGS == 12
  // Set camera to use DMA, improves performance.
  if (!this->forceRaw &&
      dc1394_dma_setup_capture(this->handle, this->camera.node, channel,
                               this->format, this->mode, this->iso_speed,
                               this->frameRate, this->num_dma_buffers, 1, 0, NULL,
                               &this->camera) == DC1394_SUCCESS)
#elif LIBDC1394_VERSION == 0200
  // Set camera to use DMA, improves performance.
  bool DMA_Success = true;
  if (!this->forceRaw)
  {
  	// first set parameters that are common between format 7 and other modes
  	if (DC1394_SUCCESS != dc1394_video_set_framerate(camera,frameRate))
	{
		PLAYER_WARN("1394 failed to set frameRate");
  		DMA_Success = false;
	}
  	if (DC1394_SUCCESS != dc1394_video_set_iso_speed(camera,this->iso_speed))
	{
		PLAYER_WARN("1394 failed to set iso speed");
		DMA_Success = false;
	}
	if (DC1394_SUCCESS != dc1394_video_set_mode(camera,mode))
	{
		PLAYER_WARN("1394 failed to set mode");
		DMA_Success = false;
	}
  	
  	// now start capture
	if (DC1394_SUCCESS != dc1394_capture_setup(camera, this->num_dma_buffers, DC1394_CAPTURE_FLAGS_DEFAULT))
  		DMA_Success = false;
  }
  if (DMA_Success)
#else
  if (0)
#endif
  {    
    this->method = methodVideo;
  }
  else
  {
    PLAYER_WARN("DMA capture failed; falling back on RAW method");
          
    // Set camera to use RAW method (fallback)
#if LIBDC1394_VERSION == 0200
    if (0)
#else
    if (dc1394_setup_capture(this->handle, this->camera.node, channel,
                             this->format, this->mode, SPEED_400, this->frameRate,
                             &this->camera) == DC1394_SUCCESS)
#endif
    {
      this->method = methodRaw;
    }
    else
    {
      PLAYER_ERROR("unable to open camera in VIDE0 or RAW modes");
      this->SafeCleanup();
      return -1;
    }
  }

  // Start transmitting camera data
#if LIBDC1394_VERSION == 0200
  if (DC1394_SUCCESS != dc1394_video_set_transmission(this->camera, DC1394_ON))
#else
  if (DC1394_SUCCESS != dc1394_start_iso_transmission(this->handle, this->camera.node))
#endif
  {
    PLAYER_ERROR("unable to start camera");
    this->SafeCleanup();
    return -1;
  }

  // Start the driver thread.
  this->StartThread();
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int Camera1394::Shutdown()
{
  // Stop the driver thread.
  StopThread();

  // Stop transmitting camera data
#if LIBDC1394_VERSION == 0200
  if (DC1394_SUCCESS != dc1394_video_set_transmission(this->camera, DC1394_OFF)
      || DC1394_SUCCESS != dc1394_capture_stop(this->camera))
#else
  if (dc1394_stop_iso_transmission(this->handle, this->camera.node) != DC1394_SUCCESS) 
#endif
    PLAYER_WARN("unable to stop camera");

  // Free resources
  this->SafeCleanup();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Camera1394::Main() 
{
  char filename[255];
  int frameno;

  frameno = 0;
  //struct timeval now,old;
  while (true)
  {
    
    // Go to sleep for a while (this is a polling loop).
    // We shouldn't need to sleep if GrabFrame is blocking.    
    //nanosleep(&NSLEEP_TIME, NULL);

    // Test if we are supposed to cancel this thread.
    pthread_testcancel();

    // Process any pending requests.
    ProcessMessages();

    // Grab the next frame (blocking)
    this->GrabFrame();

    // this should go or be replaced
    // Save frames; must be done after writedata (which will byteswap)
    if (this->save)
    {
      //printf("click %d\n", frameno);
      snprintf(filename, sizeof(filename), "click-%04d.ppm", frameno++);
      this->SaveFrame(filename);
    }

    // Write data to server
    this->RefreshData();

    /*
      gettimeofday(&now,NULL);
      printf("dt = %lf\n",now.tv_sec-old.tv_sec+(now.tv_usec-old.tv_usec)*1.0e-6);
      old=now;
    */
 }
  printf("Camera1394::main() exited\n");
  
}


////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int Camera1394::ProcessMessage(QueuePointer & resp_queue,
                              player_msghdr * hdr,
                              void * data)
{
  assert(hdr);
  assert(data);

  /* We currently don't support any messages, but if we do...
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_FIDUCIAL_REQ_GET_GEOM, this->device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_data_t));
    ProcessOdom(hdr, *reinterpret_cast<player_position2d_data_t *> (data));
    return(0);

  }
  */

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Store an image frame into the 'frame' buffer
int Camera1394::GrabFrame()
{
#if LIBDC1394_VERSION == 0200
  dc1394video_frame_t * frame = NULL;
#endif
  switch (this->method)
  {
  case methodRaw:
#if LIBDC1394_VERSION == 0200
    if (1)
#else
    if (dc1394_single_capture(this->handle, &this->camera) != DC1394_SUCCESS)
#endif
    {
      PLAYER_ERROR("Unable to capture frame");
      return -1;
    }
    break;
  case methodVideo:
#if LIBDC1394_VERSION == 0200
    dc1394_capture_dequeue (camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    if (!frame) 
#else
    if (dc1394_dma_single_capture(&this->camera) != DC1394_SUCCESS)
#endif
    {
      PLAYER_ERROR("Unable to capture frame");
      return -1;
    }
    break;
  default:
    PLAYER_ERROR("Unknown grab method");
    return -1;
  }

  unsigned int frame_width;
  unsigned int frame_height;
  uint8_t * capture_buffer;
#if LIBDC1394_VERSION == 0200
  frameTime = frame->timestamp*1.e-6;
  frame_width = frame->size[0];
  frame_height = frame->size[1];
  capture_buffer = reinterpret_cast<uint8_t *>(frame->image);
#else
  frame_width = this->camera.frame_width;
  frame_height = this->camera.frame_height;
  capture_buffer = reinterpret_cast<uint8_t *>(this->camera.capture_buffer);
#endif
  assert(capture_buffer);
  assert(!(this->data));
  this->data = reinterpret_cast<player_camera_data_t *>(malloc(sizeof(player_camera_data_t)));
  assert(this->data);
  // memset(this->data, 0, sizeof(player_camera_data_t)); I assume everything is set properly later!
  switch (this->mode)
  {
  case MODE_320x240_YUV422:
  case MODE_640x480_YUV422:
  case MODE_800x600_YUV422:
  case MODE_1024x768_YUV422:
  case MODE_1280x960_YUV422:
    this->data->bpp = 24;
    this->data->format = PLAYER_CAMERA_FORMAT_RGB888;
    this->data->image_count = frame_width * frame_height * 3;
    this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
    assert(this->data->image);
    this->data->width = frame_width;
    this->data->height = frame_height;
    uyvy2rgb(reinterpret_cast<unsigned char *>(capture_buffer), reinterpret_cast<unsigned char *>(this->data->image), frame_width * frame_height);
    break;
  case MODE_640x480_RGB:
    this->data->bpp = 24;
    this->data->format = PLAYER_CAMERA_FORMAT_RGB888;
    this->data->image_count = frame_width * frame_height * 3;
    this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
    assert(this->data->image);
    this->data->width = frame_width;
    this->data->height = frame_height;
    memcpy(this->data->image, capture_buffer, this->data->image_count);
    break;
  case MODE_640x480_MONO:
  case MODE_800x600_MONO:
  case MODE_1024x768_MONO:
  case MODE_1280x960_MONO:
  case MODE_FORMAT7_0:
    if (!DoBayerConversion)
    {
      this->data->bpp = 8;
      this->data->format = PLAYER_CAMERA_FORMAT_MONO8;
      this->data->image_count = frame_width * frame_height;
      this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
      assert(this->data->image);
      this->data->width = frame_width;
      this->data->height = frame_height;
      memcpy(this->data->image, capture_buffer, this->data->image_count);
    } 
    else
    {
      this->data->bpp = 24;
      this->data->format = PLAYER_CAMERA_FORMAT_RGB888;
      switch (this->BayerMethod)
      {
      
      case BAYER_DECODING_DOWNSAMPLE:
        // quarter of the image but 3 bytes per pixel
	this->data->image_count = (frame_width / 2) * (frame_height / 2) * 3;
        this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
	assert(this->data->image);
	BayerDownsample(reinterpret_cast<unsigned char *>(capture_buffer), 
	                reinterpret_cast<unsigned char *>(this->data->image),
			frame_width / 2, frame_height / 2,
			this->BayerPattern);
	break;
      case BAYER_DECODING_NEAREST:
        this->data->image_count = frame_width * frame_height * 3;
        this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
	assert(this->data->image);
        BayerNearestNeighbor(reinterpret_cast<unsigned char *>(capture_buffer),
			     reinterpret_cast<unsigned char *>(this->data->image),
			     frame_width, frame_height,
			     this->BayerPattern);
	break;
      case BAYER_DECODING_EDGE_SENSE:
        this->data->image_count = frame_width * frame_height * 3;
        this->data->image = reinterpret_cast<uint8_t *>(malloc(this->data->image_count));
        assert(this->data->image);
	BayerEdgeSense(reinterpret_cast<unsigned char *>(capture_buffer),
	               reinterpret_cast<unsigned char *>(this->data->image),
		       frame_width, frame_height,
		       this->BayerPattern);
	break;
      default:
        PLAYER_ERROR("camera1394: Unknown Bayer Method");
	return -1;
      }
      if (this->BayerMethod != BAYER_DECODING_DOWNSAMPLE)
      {
        this->data->width = frame_width;
        this->data->height = frame_height;
      } 
      else
      {    //image is half the size of grabbed frame
        this->data->width = frame_width / 2;
        this->data->height = frame_height / 2;
      }
    }
    break;
  default:
    PLAYER_ERROR("camera1394: Unknown mode");
    return -1;
  }
#if LIBDC1394_VERSION == 0200
  if (this->method == methodVideo) dc1394_capture_enqueue(camera, frame);
#else
  if (this->method == methodVideo) dc1394_dma_done_with_buffer(&this->camera);
#endif
  this->data->compression = PLAYER_CAMERA_COMPRESS_RAW;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void Camera1394::RefreshData()
{
  assert(this->data);
  if (!(this->data->image_count))
  {
    PLAYER_ERROR("No image data to publish");
    return;
  }
  assert(this->data->image);
  Publish(this->device_addr, 
          PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
          reinterpret_cast<void *>(this->data),
#if LIBDC1394_VERSION == 0200
          0, &this->frameTime, false);
#else
          0, NULL, false);
#endif
  // I assume that Publish() freed this data!
  this->data = NULL;
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Save a frame to disk
int Camera1394::SaveFrame(const char *filename)
{
  FILE * fp;
  
  assert(this->data);
  if (!(this->data->image_count))
  {
    PLAYER_ERROR("No image data to write");
    return -1;
  }
  assert(this->data->image);

  fp = fopen(filename, "wb");
  if (!fp)
  {
    PLAYER_ERROR("Couldn't create image file");
    return -1;
  }

  switch (this->data->format)
  {
    case PLAYER_CAMERA_FORMAT_MONO8:
      fprintf(fp,"P5\n%u %u\n255\n", this->data->width, this->data->height);
      fwrite((unsigned char*)this->data->image, 1, this->data->image_count, fp);
      break;
    case PLAYER_CAMERA_FORMAT_RGB888:
      fprintf(fp,"P6\n%u %u\n255\n", this->data->width, this->data->height);
      fwrite((unsigned char*)this->data->image, 1, this->data->image_count, fp);
      break;
    default:
      break;
  }

  fclose(fp);

  return 0;
}

