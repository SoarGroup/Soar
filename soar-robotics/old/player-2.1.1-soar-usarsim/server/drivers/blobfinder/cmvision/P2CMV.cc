/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *     Andrew Martignoni III
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
 * $Id: P2CMV.cc 4335 2008-01-28 02:26:32Z thjc $
 *
 * Uses CMVision to retrieve the blob data
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_cmvision cmvision
@brief CMVision

CMVision (Color Machine Vision) is a fast
color-segmentation (aka blob-finding) software library.
CMVision was written by Jim Bruce at CMU and is Freely <a
href=http://www-2.cs.cmu.edu/~jbruce/cmvision/>available</a> under
the GNU GPL.  But you don't have to download CMVision yourself, because
Player's cmvision driver includes the CMVision code.  The cmvision driver
provides a stream of camera images to the CMVision code and assembles
the resulting blob information into Player's data format.

Consult the CMVision documentation for details on writing a CMVision
configuration file.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_blobfinder

@par Requires

- @ref interface_camera : camera device to get image data from

@par Configuration requests

- none

@par Configuration file options

- debuglevel (int)
  - Default: 0
  - If set to 1, the blobfinder will output a testpattern of three blobs.


- colorfile (string)
  - Default: ""
  - CMVision configuration file.  In the colors section, the tuple is the RGB
  value of the intended color.  In the thresholds section, the values are the
  min:max of the respective YUV channels.

@verbatim
[Colors]
(255,  0,  0) 0.000000 10 Red
(  0,255,  0) 0.000000 10 Green
(  0,  0,255) 0.000000 10 Blue

[Thresholds]
( 25:164, 80:120,150:240)
( 20:220, 50:120, 40:115)
( 15:190,145:255, 40:120)
@endverbatim

@par Example

@verbatim
driver
(
  name "cmvision"
  provides ["blobfinder:0"]
  requires ["camera:0"]
)
@endverbatim

@author Andy Martignoni III, Brian Gerkey, Brendan Burns,
Ben Grocholsky, Brad Kratochvil

*/

/** @} */

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <unistd.h>  /* close(2),fcntl(2),getpid(2),usleep(3),execvp(3),fork(2)*/
#include <signal.h>  /* for kill(2) */
#include <fcntl.h>   /* for fcntl(2) */
#include <string.h>  /* for strncpy(3),memcpy(3) */
#include <stdlib.h>  /* for atexit(3),atoi(3) */
#include <pthread.h> /* for pthread stuff */
#include <math.h>    /* for rint */

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#if HAVE_JPEGLIB_H 
  #include <libplayerjpeg/playerjpeg.h>
#endif

#include "conversions.h"
#include "cmvision.h"
#include "capture.h"

#define CMV_NUM_CHANNELS CMV_MAX_COLORS
#define CMV_HEADER_SIZE 4*CMV_NUM_CHANNELS
#define CMV_BLOB_SIZE 16
#define CMV_MAX_BLOBS_PER_CHANNEL 10

#define DEFAULT_CMV_WIDTH CMV_DEFAULT_WIDTH
#define DEFAULT_CMV_HEIGHT CMV_DEFAULT_HEIGHT

/********************************************************************/

class CMVisionBF: public Driver
{
  private:
    int              mDebugLevel; // debuglevel 0=none,
                                  //            1=basic,
                                  //            2=everything
    uint16_t         mWidth;
    uint16_t         mHeight;     // the image dimensions
    uint8_t*         mImg;
    uint8_t*         mTmp;
    const char*      mColorFile;

    player_blobfinder_data_t   mData;
    unsigned int     allocated_blobs;

    player_devaddr_t mCameraAddr;
    Device*          mCameraDev;
    CMVision*        mVision;

    // this will output a testpattern for debugging
    void TestPattern();
    // print the blobs to the console
    void Print();

  public:
    int Setup();
    int Shutdown();
    // constructor
    CMVisionBF(ConfigFile* cf, int section);
    virtual ~CMVisionBF();
    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);
    virtual void Main();
    void ProcessImageData();
};

// a factory creation function
Driver*
CMVision_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new CMVisionBF( cf, section)));
}

// a driver registration function
void
CMVision_Register(DriverTable* table)
{
  table->AddDriver("cmvision", CMVision_Init);
}

CMVisionBF::CMVisionBF( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
           PLAYER_BLOBFINDER_CODE),
           mWidth(0),
           mHeight(0),
           mImg(NULL),
           mTmp(NULL),
           mColorFile(NULL),
           mCameraDev(NULL),
           mVision(NULL)
{
  mColorFile  = cf->ReadString(section, "colorfile", "");
  mDebugLevel = cf->ReadInt(section, "debuglevel", 0);
  // Must have an input camera
  if (cf->ReadDeviceAddr(&mCameraAddr, section, "requires",
                         PLAYER_CAMERA_CODE, -1, NULL) != 0)
  {
    PLAYER_ERROR("this driver requires a camera in the .cfg file");
    return;
  }
}

CMVisionBF::~CMVisionBF()
{
  if (mVision) delete mVision;
  if (mImg) delete []mImg;
  if (mTmp) delete []mTmp;
}

int
CMVisionBF::Setup()
{
  if (mVision)
  {
    PLAYER_ERROR("CMVision server already initialized");
    return -1;
  }
  printf("CMVision server initializing...");
  fflush(stdout);
  // Subscribe to the camera device
  if (!(this->mCameraDev = deviceTable->GetDevice(this->mCameraAddr)))
  {
    PLAYER_ERROR("unable to locate suitable camera device");
    return(-1);
  }
  if(0 != this->mCameraDev->Subscribe(this->InQueue))
  {
    PLAYER_ERROR("unable to subscribe to camera device");
    return(-1);
  }

  mVision = new CMVision();
  // clean our data
  memset(&mData,0,sizeof(mData));
  allocated_blobs = 0;
  puts("done.");

  StartThread();
  return(0);
}

int
CMVisionBF::Shutdown()
{
  /* if Setup() was never called, don't do anything */
  if (mVision==NULL)
    return 0;

  StopThread();

  // Unsubscribe from the camera
  this->mCameraDev->Unsubscribe(this->InQueue);

  delete mVision; mVision = NULL;

  puts("CMVision server has been shutdown");
  return(0);
}

void
CMVisionBF::Main()
{
  // The main loop; interact with the device here
  for(;;)
  {
    // wait to receive a new message (blocking)
    Wait();

    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages, and update outgoing data
    ProcessMessages();
  }
  return;
}

void
CMVisionBF::ProcessImageData()
{
    assert(mVision);
    // this shouldn't change often
    if ((mData.width != mWidth) || (mData.height != mHeight))
    {
      if(!(mVision->initialize(mWidth, mHeight)))
      {
        PLAYER_ERROR("Vision init failed.");
        exit(-1);
      }

      if(mColorFile[0])
      {
        if (!mVision->loadOptions(const_cast<char*>(mColorFile)))
        {
          PLAYER_ERROR("Error loading color file");
          exit(-1);
        }
      }
      else
      {
        PLAYER_ERROR("No color file given.  Use the \"mColorFile\" "
                     "option in the configuration file.");
        exit(-1);
      }
      mData.width      = mWidth;
      mData.height     = mHeight;
      printf("cmvision using camera: [w %d h %d]\n", mWidth, mHeight);
    }

    if (!mVision->processFrame(reinterpret_cast<image_pixel*>(mImg)))
    {
      PLAYER_ERROR("Frame error.");
    }

    
    mData.blobs_count = 0;
    for (int ch = 0; ch < CMV_MAX_COLORS; ++ch)
    {
      // Get the descriptive color
      rgb c = mVision->getColorVisual(ch);;

      // Grab the regions for this color
      CMVision::region* r = NULL;

      for (r = mVision->getRegions(ch); r != NULL; r = r->next)
      {
        if (mData.blobs_count >= allocated_blobs)
        {
          allocated_blobs = mData.blobs_count+1;
          mData.blobs = (player_blobfinder_blob_t*)realloc(mData.blobs,sizeof(mData.blobs[0])*allocated_blobs);
        }

        player_blobfinder_blob_t *blob;
        blob = mData.blobs + mData.blobs_count;
        mData.blobs_count++;

        blob->color = uint32_t(c.red)   << 16 |
                      uint32_t(c.green) <<  8 |
                      uint32_t(c.blue);

        // stage puts the range in here to simulate stereo mVision. we
        // can't do that (yet?) so set the range to zero - rtv
        blob->range = 0;

        // get the area first
        blob->area   = static_cast<uint32_t>(r->area);

        blob->x      = static_cast<uint32_t>(rint(r->cen_x+.5));
        blob->y      = static_cast<uint32_t>(rint(r->cen_y+.5));
        blob->left   = static_cast<uint32_t>(r->x1);
        blob->right  = static_cast<uint32_t>(r->x2);
        blob->top    = static_cast<uint32_t>(r->y1);
        blob->bottom = static_cast<uint32_t>(r->y2);
      }
    }

    // if we're debugging, output the test blobs
    if (0 != mDebugLevel)
      TestPattern();

    Publish(device_addr, 
          PLAYER_MSGTYPE_DATA, PLAYER_BLOBFINDER_DATA_BLOBS,
          reinterpret_cast<void*>(&mData));
}

void
CMVisionBF::TestPattern()
{
  mData.blobs_count = 3;

  for (uint32_t i=0; i<mData.blobs_count; ++i)
  {
    uint32_t x = mWidth/5*i + mWidth/5;
    uint32_t y = mHeight/2;

    mData.blobs[i].x = x;
    mData.blobs[i].y = y;

    mData.blobs[i].top    = y+10;
    mData.blobs[i].bottom = y-10;
    mData.blobs[i].left   = x-10;
    mData.blobs[i].right  = x+10;

    mData.blobs[i].color  = 0xff << i*8;
  }
}

void
CMVisionBF::Print()
{
  // this is mainly for debugging purposes
  for (uint32_t i=0; i<mData.blobs_count; ++i)
  {
    printf("%i: ", i);
    printf("%i, ", mData.blobs[i].x);
    printf("%i, ", mData.blobs[i].y);
    printf("%i, ", mData.blobs[i].top);
    printf("%i, ", mData.blobs[i].left);
    printf("%i, ", mData.blobs[i].bottom);
    printf("%i\n", mData.blobs[i].right);
  }
  printf("-----\n");
}

int
CMVisionBF::ProcessMessage(QueuePointer & resp_queue,
                           player_msghdr* hdr,
                           void* data)
{
  // Handle new data from the camera
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
                           this->mCameraAddr))
  {
    // Lock();
    // we can't quite do this so easily with camera data
    // because the images are different than the max size
    //assert(hdr->size == sizeof(player_camera_data_t));
    player_camera_data_t* camera_data = reinterpret_cast<player_camera_data_t *>(data);
    uint8_t* ptr;

    assert(camera_data);

#if !HAVE_JPEGLIB_H 
    if (camera_data->compression == PLAYER_CAMERA_COMPRESS_JPEG)
    {
      PLAYER_ERROR("No support for jpeg decompression");
      return(-1);
    }
#endif

    assert(camera_data->bpp == 24);
    if ((camera_data->width) && (camera_data->height))
    {
      if ((mWidth != camera_data->width) || (mHeight != camera_data->height) || (!mImg) || (!mTmp))
      {
        mWidth  = camera_data->width;
        mHeight = camera_data->height;
        if (mImg) delete []mImg; mImg = NULL;
        if (mTmp) delete []mTmp; mTmp = NULL;
        // we need to allocate some memory
        if (!mImg) mImg = new uint8_t[mWidth*mHeight*2];
        if (!mTmp) mTmp = new uint8_t[mWidth*mHeight*3];
      }
      ptr = camera_data->image;
      if (camera_data->compression == PLAYER_CAMERA_COMPRESS_JPEG)
      {
#if HAVE_JPEGLIB_H 
	jpeg_decompress((unsigned char*)mTmp, 
			mWidth*mHeight*3,
                        camera_data->image,
                        camera_data->image_count
                       );
#endif
        ptr = mTmp;
      }
      // now deal with the data
      rgb2uyvy(ptr, mImg, mWidth*mHeight);

      // we have a new image,
      ProcessImageData();
    }
    // Unlock();
    return(0);
  }

  // Tell the caller that you don't know how to handle this message
  return(-1);
}
