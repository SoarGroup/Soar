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
// Desc: Video for Linux capture driver
// Author: Andrew Howard
// Date: 9 Jan 2004
// CVS: $Id: camerav4l.cc 4232 2007-11-01 22:16:23Z gerkey $
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_camerav4l camerav4l
 * @brief Video4Linux camera capture

The camerav4l driver captures images from V4l-compatible cameras.  See
below for notes on specific camera/frame grabber combinations.

@par Compile-time dependencies

- &lt;linux/videodev.h&gt;

@par Provides

- @ref interface_camera

@par Requires

- none

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/video0"
  - Device to read video data from.

- source (integer)
  - Default: 3
  - Some capture cards have multiple input sources; use this field to
    select which one is used.

- norm (string)
  - Default: "ntsc"
  - Capture format; "ntsc" or "pal"

- size (integer tuple)
  - Default: varies with norm
  - Desired image size.   This may not be honoured if the driver does
    not support the requested size).

- mode (string)
  - Default: "RGB888"
  - Desired capture mode.  Can be one of:
    - GREY (8-bit monochrome)
    - RGB565 (16-bit RGB)
    - RGB888 (24-bit RGB)
    - RGB32 (32-bit RGB; will produce 24-bit color images)
    - YUV420P (planar YUV data converted to 24-bit color images)
    - YUV420P_GREY (planar YUV data; will produce 8-bit monochrome images)
    - JPEG (for some webcams)
  - Note that not all capture modes are supported by Player's internal image
  format; in these modes, images will be translated to the closest matching
  internal format (e.g., RGB32 -> RGB888).

- save (integer)
  - Default: 0
  - Debugging option: set this to write each frame as an image file on disk.

- have_ov519 (integer)
  - Default: 0
  - Needed for ovc519 based cameras that send data jpeg compressed on the usb bus.

- max_buffer (integer)
  - Default: -1 (do not set)
  - Limit the maximum number of buffers to use for grabbing. This reduces latency, but also 
  	potentially reduces throughput. Use this if you are reading slowly from the player
  	driver and dont want to get stale frames

- brightness (integer)
  - Default: -1 (do not set)

- hue (integer)
  - Default: -1 (do not set)

- colour (integer)
  - Default: -1 (do not set)

- contrast (integer)
  - Default: -1 (do not set)

- read_mode (integer)
  - Default: 0
  - Set to 1 if read should be used instead of grab (in most cases it isn't a good idea!)

- publish_interval (integer)
  - Default: 0
  - how many second between publishing real images (may break your client app!)

- sleep_nsec (integer)
  - Default: 10000000 (=10ms which gives max 100 fps)
  - timespec value for nanosleep()

Note that some of these options may not be honoured by the underlying
V4L kernel driver (it may not support a given image size, for
example).

@par Example

@verbatim
driver
(
  name "camerav4l"
  provides ["camera:0"]
)
@endverbatim


@par Logitech QuickCam Pro 4000

For the Logitech QuickCam Pro 4000, use:
@verbatim
driver
(
  name "camerav4l"
  provides ["camera:0"]
  port "/dev/video0"
  source 0
  size [160 120]
  mode "YUV420P"
)
@endverbatim

Kernel notes: with a little bit of tweaking, this camera will work with the pwc
(Phillips Web-Cam) driver in the Linux 2.4.20 kernel.  The stock driver recognizes
the QC Pro 3000, but not the QC Pro 4000; to support the latter, you must modify
the kernel source (add a product id in a couple of places in pwc-if.c).  Milage may
vary for other kernel versions.  Also, the binary-only pwcx.o module is needed to
access frame sizes larger than 160x120; good luck finding this and/or getting
it to work (the developer spat the dummy and took down his website).

Update for pwc:
The original source has been taken up by another developer and is now available
from http://www.saillard.org/linux/pwc/. This new version also doesnt need the 
binary driver which is a bonus


@author Andrew Howard

*/
/** @} */

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <unistd.h>
#include <time.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include "v4lcapture.h"  // For Gavin's libfg; should integrate this
#include "ccvt.h"        // For YUV420P-->RGB conversion

// Time for timestamps
extern PlayerTime *GlobalTime;

// Driver for detecting laser retro-reflectors.
class CameraV4L : public Driver
{
  // Constructor
  public: CameraV4L( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

  // This method will be invoked on each incoming message
  public: virtual int ProcessMessage(QueuePointer & resp_queue,
                                     player_msghdr * hdr,
                                     void * data);

  // Main function for device thread.
  private: virtual void Main();

  // Update the device data (the data going back to the client).
  private: void RefreshData();

  // Video device
  private: const char *device;

  // Input source
  private: int source;

  // The signal norm: VIDEO_NORM_PAL or VIDEO_NORM_NTSC.
  private: int norm;

  // Pixel depth
  private: int depth;

  // Camera palette
  private: const char *palette;

  // Image dimensions
  private: int width, height;

  // Frame grabber interface
  private: FRAMEGRABBER* fg;

  // Max buffers to use
  private: int max_buffer;

  // The current image (local copy)
  private: FRAME* frame;

  // a place to store rgb image for the yuv grabbers
  private: FRAME* rgb_converted_frame;

  // Write frames to disk?
  private: int save;

  // unpack jpeg data from ov519 based cameras
  private: int have_ov519;

  private: int brightness;

  private: int hue;

  private: int colour;

  private: int contrast;

  private: int read_mode;

  private: int publish_interval;

  private: int sleep_nsec;

  // Capture timestamp
  private: uint32_t tsec, tusec;

  private: time_t publish_time;

  // Data to send to server
  private: player_camera_data_t data;

};


// Initialization function
Driver* CameraV4L_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new CameraV4L( cf, section)));
}


// Driver registration function
void CameraV4L_Register(DriverTable* table)
{
  table->AddDriver("camerav4l", CameraV4L_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
CameraV4L::CameraV4L(ConfigFile* cf, int section)
  : Driver(cf,
           section,
           true,
           1, // 1 instead of PLAYER_MSGQUEUE_DEFAULT_MAXLEN
           PLAYER_CAMERA_CODE)
{
  const char *snorm;

  // Camera defaults to /dev/video0 and NTSC
  this->device = cf->ReadString(section, "port", "/dev/video0");

  // Input source
  this->source = cf->ReadInt(section, "source", 3);

  // NTSC or PAL
  snorm = cf->ReadString(section, "norm", "ntsc");
  if (strcasecmp(snorm, "ntsc") == 0)
  {
    this->norm = VIDEO_MODE_NTSC;
    this->width = 640;
    this->height = 480;
  }
  else if (strcasecmp(snorm, "pal") == 0)
  {
    this->norm = VIDEO_MODE_PAL;
    this->width = 768;
    this->height = 576;
  }
  else
  {
    this->norm = VIDEO_MODE_AUTO;
    this->width = 320;
    this->height = 240;
  }

  // Size
  this->width = cf->ReadTupleInt(section, "size", 0, this->width);
  this->height = cf->ReadTupleInt(section, "size", 1, this->height);

  // Palette type
  this->palette = cf->ReadString(section, "mode", "RGB888");

  this->max_buffer = cf->ReadInt(section, "max_buffer", -1);

  // Save frames?
  this->save = cf->ReadInt(section, "save", 0);

  // unpack ov519
  this->have_ov519 = cf->ReadInt(section, "have_ov519", 0);

  this->brightness = cf->ReadInt(section, "brightness", -1);

  this->hue = cf->ReadInt(section, "hue", -1);

  this->colour = cf->ReadInt(section, "colour", -1);

  this->contrast = cf->ReadInt(section, "contrast", -1);

  this->read_mode = cf->ReadInt(section, "read_mode", 0);

  this->publish_interval = cf->ReadInt(section, "publish_interval", 0);

  this->sleep_nsec = cf->ReadInt(section, "sleep_nsec", 10000000);
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int CameraV4L::Setup()
{
  this->fg = fg_open(this->device);
  if (this->fg == NULL)
  {
    PLAYER_ERROR1("unable to open %s", this->device);
    return -1;
  }
  
  if (max_buffer > 0)
  	fg->max_buffer = max_buffer;

  fg_set_source(this->fg, this->source);
  fg_set_source_norm(this->fg, this->norm);

  if (strcasecmp(this->palette, "GREY") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_GREY);
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_GREY );
    this->data.format = PLAYER_CAMERA_FORMAT_MONO8;
    this->depth = 8;
  }
  else if (strcasecmp(this->palette, "RGB565") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_RGB565 );
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_RGB565 );
    this->data.format = PLAYER_CAMERA_FORMAT_RGB565;
    this->depth = 16;
  }
  else if (strcasecmp(this->palette, "RGB888") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_RGB24 );
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_RGB24 );
    this->data.format = PLAYER_CAMERA_FORMAT_RGB888;
    this->depth = 24;
  }
  else if (strcasecmp(this->palette, "RGB32") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_RGB32 );
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_RGB32 );
    this->data.format = PLAYER_CAMERA_FORMAT_RGB888;
    this->depth = 32;
  }
  else if (strcasecmp(this->palette, "YUV420P") == 0)
  {
    // YUV420P is now converted to RGB rather than GREY as in player
    // 1.6.2 and earlier
    fg_set_format(this->fg, VIDEO_PALETTE_YUV420P);
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_YUV420P );
    this->data.format = PLAYER_CAMERA_FORMAT_RGB888;
    this->depth = 24;
    this->rgb_converted_frame = frame_new(this->width,
            this->height, VIDEO_PALETTE_RGB24 );
    //    this->data.format = PLAYER_CAMERA_FORMAT_MONO8;
    //    this->depth = 8;
  }
  else if (strcasecmp(this->palette, "YUV420P_GREY") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_YUV420P);
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_YUV420P );
    this->data.format = PLAYER_CAMERA_FORMAT_MONO8;
    this->depth = 8;
   }
  else if (strcasecmp(this->palette, "JPEG") == 0)
  {
    fg_set_format(this->fg, VIDEO_PALETTE_JPEG );
    this->frame = frame_new(this->width, this->height, VIDEO_PALETTE_JPEG );
    this->data.format = PLAYER_CAMERA_FORMAT_RGB888;
    this->depth = 24;  
  }
  else
  {
    PLAYER_ERROR2("image depth %d is not supported (add it yourself in %s)",
                  this->depth, __FILE__);
    return 1;
  }

  fg_set_capture_window(this->fg, 0, 0, this->width, this->height);

  // Start the driver thread.
  this->StartThread();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int CameraV4L::Shutdown()
{
  // Stop the driver thread.
  StopThread();

  // Free resources
  frame_release(this->frame);
  if ((this->frame->format == VIDEO_PALETTE_YUV420P)&&
      (this->data.format == PLAYER_CAMERA_FORMAT_RGB888))
       frame_release(this->rgb_converted_frame);
  fg_close(this->fg);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void CameraV4L::Main()
{
  struct timeval time;
  struct timespec tspec;
  int frameno;
  char filename[256];

  if (this->brightness > 0) fg_set_brightness(this->fg, this->brightness);
  if (this->hue > 0) fg_set_hue(this->fg, this->hue);
  if (this->colour > 0) fg_set_colour(this->fg, this->colour);
  if (this->contrast > 0) fg_set_contrast(this->fg, this->contrast);

  this->publish_time = 0;
  frameno = 0;

  while (true)
  {
    // Go to sleep for a while (this is a polling loop).
    tspec.tv_sec = 0;
    tspec.tv_nsec = this->sleep_nsec;
    nanosleep(&tspec, NULL);

    // Test if we are supposed to cancel this thread.
    pthread_testcancel();

    // Process any pending requests.
    ProcessMessages();

    // Get the time
    GlobalTime->GetTime(&time);
    this->tsec = time.tv_sec;
    this->tusec = time.tv_usec;

    // Grab the next frame (blocking)
    if (this->read_mode)
    {
      fg_read(this->fg, this->frame);
    } else
    {
      fg_grab_frame(this->fg, this->frame);
    }

    // Write data to server
    this->RefreshData();

    // Save frames
    if (this->save)
    {
      //printf("click %d\n", frameno);
      snprintf(filename, sizeof(filename), "click-%04d.ppm", frameno++);
      if ((this->frame->format == VIDEO_PALETTE_YUV420P)&&
	  (this->data.format == PLAYER_CAMERA_FORMAT_RGB888))
	   {
		frame_save(this->rgb_converted_frame, filename);
		//printf("saved converted frame\n");
	   }
      else
	   frame_save(this->frame, filename);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int CameraV4L::ProcessMessage(QueuePointer & resp_queue,
                              player_msghdr * hdr,
                              void * data)
{
  assert(hdr);
  assert(data);

  /* We currently don't support any messages, but if we do...
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_CAMERA_REQ_GET_GEOM, this->device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_data_t));
    ProcessGetGeom(hdr, *reinterpret_cast<player_camera_data_t *> (data));
    return(0);

  }
  */

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void CameraV4L::RefreshData()
{
  int i;
  size_t image_count;
  unsigned char * ptr1, * ptr2;

  // Compute size of image
  image_count = this->width * this->height * this->depth / 8;

  // Set the image properties
  this->data.width       = this->width;
  this->data.height      = this->height;
  this->data.bpp         = this->depth;
  this->data.image_count = image_count;
  this->data.image       = new unsigned char [image_count];
  this->data.compression = PLAYER_CAMERA_COMPRESS_RAW;

  if (have_ov519)
  {
    this->data.image_count = (*(unsigned short *)(frame->data))*8;
    assert(data.image_count > 0);
    assert(data.image_count <= ((size_t)(this->frame->size)));
    this->data.compression = PLAYER_CAMERA_COMPRESS_JPEG;
    memcpy(data.image, &(((char*)frame->data)[2]), data.image_count);
  }
  else if (this->fg->picture.palette == VIDEO_PALETTE_JPEG)
  {
    this->data.compression = PLAYER_CAMERA_COMPRESS_JPEG;
    memcpy(&i, this->frame->data, sizeof(int));
    data.image_count = i;
    assert(data.image_count > 0);
    assert(data.image_count <= ((size_t)(this->frame->size)));
    memcpy(data.image, ((unsigned char *)(this->frame->data)) + sizeof(int), data.image_count);
  }
  else
  {
    // Copy the image pixels
    if ((this->frame->format == VIDEO_PALETTE_YUV420P) &&
	(this->data.format == PLAYER_CAMERA_FORMAT_RGB888))
	 {// do conversion to RGB (which is bgr at the moment for some reason?)
	      assert(data.image_count <= (size_t) this->rgb_converted_frame->size);
	      ccvt_420p_bgr24(this->width, this->height,
		   (unsigned char*) this->frame->data,
		   (unsigned char*) this->rgb_converted_frame->data);
	      ptr1 = (unsigned char *)this->rgb_converted_frame->data;
	 }
    else
	 {
	      assert(data.image_count <= (size_t) this->frame->size);
	      ptr1 = (unsigned char *)this->frame->data;
	 }
    ptr2 = this->data.image;
    switch (this->depth)
    {
    case 24:
      for (i = 0; i < ((this->width) * (this->height)); i++)
      {
        ptr2[0] = ptr1[2];
	ptr2[1] = ptr1[1];
        ptr2[2] = ptr1[0];
        ptr1 += 3;
        ptr2 += 3;
      }
      break;
    case 32:
      for (i = 0; i < ((this->width) * (this->height)); i++)
      {
        ptr2[0] = ptr1[2];
        ptr2[1] = ptr1[1];
        ptr2[2] = ptr1[0];
        ptr2[3] = ptr1[3];
        ptr1 += 4;
        ptr2 += 4;
      }
      break;
    default:
      memcpy(ptr2, ptr1, data.image_count);
    }
  }

  if (this->publish_interval)
  {
    if ((time(NULL) - (this->publish_time)) < (this->publish_interval))
    {
      this->data.width       = 0;
      this->data.height      = 0;
      this->data.bpp         = 0;
      this->data.image_count = 0;
    } else this->publish_time = time(NULL);
  }

  Publish(this->device_addr, 
          PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
          reinterpret_cast<void*>(&this->data));
  delete [] this->data.image;

  return;
}
