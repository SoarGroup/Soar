/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al
 *                      gerkey@usc.edu    
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
// Desc: Driver for detecting artoolkitplus markers
// Author: Toby Collett
// Date: 15 Feb 2004
// CVS: $Id: artoolkitplus.cc 4232 2007-11-01 22:16:23Z gerkey $
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_artoolkitplus artoolkitplus
 * @brief coded marker detection from the artoolkitplus project


@par Compile-time dependencies

- ARToolkitPlus (http://studierstube.icg.tu-graz.ac.at/handheld_ar/artoolkitplus.php)
- A pkgconfig definition for artoolkitplus is included in the player source 'patch' directory

@par Requires

- This driver acquires image data from a @ref interface_camera
  interface. The image must be in uncompressed gray8 format

@par Provides

- This driver provides detected shapes through a @ref
  interface_blobfinder interface.

@par Configuration requests

- none

@par Configuration file options

- marker_count (int)
  - Default: 1
  - Number of marker id's to look for

- marker_ids (int tuple)
  - Default: [0]
  - Id's to match

@par Example

@verbatim
driver
(
  name "artoolkitplus"
  requires ["camera:0"]
  provides ["blobfinder:0"]
)
@endverbatim

@author Toby Collett
*/
/** @} */

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include <ARToolKitPlus/TrackerSingleMarkerImpl.h>
#include <ARToolKitPlus/Camera.h>
#include <ARToolKitPlus/ar.h>
#include <ARToolKitPlus/Logger.h>

#include "../../base/imagebase.h"

using namespace ARToolKitPlus;


class MyLogger : public ARToolKitPlus::Logger
{
    void artLog(const char* nStr)
    {
        printf(nStr);
    }
};

// Driver for detecting laser retro-reflectors.
class ARToolkitPlusDriver : public ImageBase
{
	public: 
		// Constructor
		ARToolkitPlusDriver( ConfigFile* cf, int section);

	protected: 
		int ProcessFrame();

		unsigned int LastFrameWidth;
		unsigned int LastFrameHeight;

		// ar toolkit plus bits
    	int minfocnt;				//< number of minfo structures returned last time
    	ARToolKitPlus::ARMarkerInfo *tmp_markers;		//< the information about the found markers
		ARToolKitPlus::Camera * DummyCam;			//< the camera

	    bool    	useBCH;
    	MyLogger      logger;
		ARToolKitPlus::TrackerSingleMarker *tracker;

};

// Initialization function
Driver* ARToolkitPlusDriver_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new ARToolkitPlusDriver( cf, section)));
}


// a driver registration function
void ARToolkitPlusDriver_Register(DriverTable* table)
{
  table->AddDriver("artoolkitplus", ARToolkitPlusDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
ARToolkitPlusDriver::ARToolkitPlusDriver( ConfigFile* cf, int section)
	: ImageBase(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_BLOBFINDER_CODE)

{
  // Other camera settings
  DummyCam = NULL;
  LastFrameWidth = 640;
  LastFrameHeight = 480;

	minfocnt = 0;
	tmp_markers = NULL;
	
	useBCH = true;
	
	// One off Initialisation of the artoolkitplus
    // create a tracker that does:
    //  - 6x6 sized marker images
    //  - samples at a maximum of 6x6
    //  - can load a maximum of 1 patterns
    //  - can detect a maximum of 8 patterns in one image
    //  - with an arbitrary default image size
    tracker = new ARToolKitPlus::TrackerSingleMarkerImpl<6,6,6, 1, 8>(LastFrameWidth,LastFrameHeight);
    
	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);

	
    // set a logger so we can output error messages
    //
    tracker->setLogger(&logger);
    
	if(!tracker->init(NULL, 1.0f, 1000.0f))            // load NULL camera
	{
		SetError(-1);
		return;
	}

	// disable undistortion as we dont have a proper calibration file to suppert it anyway
    tracker->setUndistortionMode(ARToolKitPlus::UNDIST_NONE);
    
    // setup a fake camera so we can set the width and height
	DummyCam = new CameraImpl;
    memset(DummyCam->dist_factor,0,sizeof (DummyCam->dist_factor));
    memset(DummyCam->mat,0,sizeof (DummyCam->mat));
    DummyCam->changeFrameSize(LastFrameWidth,LastFrameHeight);
    tracker->setCamera(DummyCam);	
    
	// the marker in the BCH test image has a thin border...
    tracker->setBorderWidth(useBCH ? 0.125f : 0.250f);

	tracker->activateAutoThreshold(true);

    // switch to simple ID based markers
    // use the tool in tools/IdPatGen to generate markers
    tracker->setMarkerMode(useBCH ? ARToolKitPlus::MARKER_ID_BCH : ARToolKitPlus::MARKER_ID_SIMPLE);
}

int ARToolkitPlusDriver::ProcessFrame()
{
	// at the moment we require the image to already be in grayscale uncompressed data
	if (stored_data.compression != PLAYER_CAMERA_COMPRESS_RAW)
		return -1;
	
	if (stored_data.format == PLAYER_CAMERA_FORMAT_RGB888)
	{
		// convert to grayscale	
		for (unsigned int i = 0; i < stored_data.width * stored_data.height; ++i)
		{
			stored_data.image[i] = (stored_data.image[i*3] + stored_data.image[i*3+1] + stored_data.image[i*3+2]) / 3;
		}
		stored_data.format = PLAYER_CAMERA_FORMAT_MONO8;
	} 

	if (stored_data.format != PLAYER_CAMERA_FORMAT_MONO8)
	{
		return -1;
	}
	
	// check we havent got a different sized image
	//check width and height
	if(stored_data.width != LastFrameWidth || stored_data.height != LastFrameHeight) 
	{
		LastFrameWidth = stored_data.width;
		LastFrameHeight = stored_data.height;
		tracker->changeCameraSize(LastFrameWidth,LastFrameHeight);
	}
		
	int ret;
	//now do the processing
	if((ret =tracker->arDetectMarker(const_cast<unsigned char*>(stored_data.image), 0, &tmp_markers, &minfocnt)) < 0)
	{
		PLAYER_WARN1("Error (%d) when detecting markers\n",ret);
		return -1;
	}
	
	// now generate our data packet
	player_blobfinder_data_t blobs;
	memset(&blobs, 0, sizeof(blobs));
	blobs.width = stored_data.width;
	blobs.height = stored_data.height;
	blobs.blobs_count = 0;
	blobs.blobs = (player_blobfinder_blob_t*)calloc(minfocnt,sizeof(blobs.blobs[0]));
	for(int i =0 ;i < minfocnt; i++) 
	{
		if (tmp_markers[i].id < 0)
			continue;

		blobs.blobs[blobs.blobs_count].id = tmp_markers[i].id;
		blobs.blobs[blobs.blobs_count].color = 0x000000FF;
		blobs.blobs[blobs.blobs_count].area = tmp_markers[i].area;
		blobs.blobs[blobs.blobs_count].x = static_cast<unsigned int> ((tmp_markers[i].vertex[0][0] + tmp_markers[i].vertex[2][0])/2);
		blobs.blobs[blobs.blobs_count].y = static_cast<unsigned int> ((tmp_markers[i].vertex[0][1] + tmp_markers[i].vertex[2][1])/2);
		blobs.blobs[blobs.blobs_count].left = static_cast<unsigned int> (tmp_markers[i].vertex[0][0]);
		blobs.blobs[blobs.blobs_count].right = static_cast<unsigned int> (tmp_markers[i].vertex[0][0]);
		blobs.blobs[blobs.blobs_count].top = static_cast<unsigned int> (tmp_markers[i].vertex[0][1]);
		blobs.blobs[blobs.blobs_count].bottom = static_cast<unsigned int> (tmp_markers[i].vertex[0][1]);
		for (int j = 1; j < 4; ++j)
		{
			if (tmp_markers[i].vertex[j][0] < blobs.blobs[blobs.blobs_count].left)
				blobs.blobs[blobs.blobs_count].left = static_cast<unsigned int> (tmp_markers[i].vertex[j][0]);
			if (tmp_markers[i].vertex[j][0] > blobs.blobs[blobs.blobs_count].right)
				blobs.blobs[blobs.blobs_count].right = static_cast<unsigned int> (tmp_markers[i].vertex[j][0]);
			if (tmp_markers[i].vertex[j][1] < blobs.blobs[blobs.blobs_count].top)
				blobs.blobs[blobs.blobs_count].top = static_cast<unsigned int> (tmp_markers[i].vertex[j][1]);
			if (tmp_markers[i].vertex[j][1] > blobs.blobs[blobs.blobs_count].bottom)
				blobs.blobs[blobs.blobs_count].bottom = static_cast<unsigned int> (tmp_markers[i].vertex[j][1]);
		}
		
		++blobs.blobs_count;
	}
	
	Publish(device_addr,PLAYER_MSGTYPE_DATA,PLAYER_BLOBFINDER_DATA_BLOBS,&blobs);
	free(blobs.blobs);
	return 0;	
}



