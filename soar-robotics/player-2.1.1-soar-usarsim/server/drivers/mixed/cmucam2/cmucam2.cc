/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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
 * $Id: cmucam2.cc 4289 2007-12-07 01:50:15Z gerkey $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_cmucam2 cmucam2
 * @brief CMUCam2 pan-tilt-zoom blob-tracking camera

The cmucam2 driver connects over a serial port to a CMUCam2. Presents a
@ref interface_blobfinder interface and a @ref interface_ptz
interface and can track multiple color blobs (plus an additional 
@ref interface_camera for getting image data). Color tracking parameters are 
defined in Player's config file (see below for an example).


@par Compile-time dependencies

- none

@par Provides

- @ref interface_blobfinder : the blobs detected by the CMUCam2
- @ref interface_ptz : control of the servos that pan and tilt
  the CMUCam2
- @ref interface_camera : snapshot images taken by the CMUCam2

@par Requires

- none

@par Supported configuration requests

- The @ref interface_ptz interface supports:
  - PLAYER_PTZ_REQ_AUTOSERVO
- The @ref interface_blobfinder interface supports:
  - PLAYER_BLOBFINDER_REQ_SET_COLOR
  - PLAYER_BLOBFINDER_REQ_SET_IMAGER_PARAMS

@par Configuration file options

- devicepath (string)
  - Default: NULL
  - Serial port where the CMUCam2 is connected
- num_blobs (integer)
  - Default: 1
  - Number of colors to track; you must also include this many color%d options
- color%d (float tuple)
  - Default: none
  - Each color%d is a tuple [rmin rmax gmin gmax bmin bmax] of min/max
    values for red, green, and blue, which defines a region in RGB space
    that the CMUCam2 will track.
- bloborcamera (integer)
  - Default: 1
  - Set bloborcamera to 1 if you want the blobfinder/ptz active, or set it
    to 2 if you want camera/ptz active. (this will be changed in the future)
  
@par Example 

@verbatim
driver
(
  name "cmucam2"
  provides ["blobfinder:0" "ptz:0" "camera:0"]
  devicepath "/dev/ttyS1"
  bloborcamera 1
  num_blobs 2
# values must be between 40 and 240 (!)
  color0 [  red_min red_max blue_min blue_max green_min green_max] )
# values must be between 40 and 240 (!)
  color1 [  red_min red_max blue_min blue_max green_min green_max] )  
)
@endverbatim

@author Pouya Bastani, Richard Vaughan, Radu Bogdan Rusu
 */
/** @} */


#include <assert.h>
#include <stdio.h>
#include <unistd.h> /* close(2),fcntl(2),getpid(2),usleep(3),execvp(3),fork(2)*/
#include <netdb.h> /* for gethostbyname(3) */
#include <sys/types.h>  /* for socket(2) */
#include <sys/socket.h>  /* for socket(2) */
#include <signal.h>  /* for kill(2) */
#include <fcntl.h>  /* for fcntl(2) */
#include <string.h>  /* for strncpy(3),memcpy(3) */
#include <stdlib.h>  /* for atexit(3),atoi(3) */
#include <pthread.h>  /* for pthread stuff */
#include <arpa/inet.h>

#include <libplayercore/playercore.h>

#include "camera.h"

#define MAX_CHANNELS 32

class Cmucam2:public Driver 
{
	private:

		// Descriptive colors for each channel.
		uint32_t colors[MAX_CHANNELS];
		void get_blob (packet_t cam_packet, player_blobfinder_blob_t *blob,
					   color_config range);
		void get_image (packet_f* cam_packet, player_camera_data_t *image);
		int fd;
		int num_of_blobs;
		const char* devicepath;
		color_config *color;

		// Blobfinder interface (provides)
		player_devaddr_t         blobfinder_id;
		player_blobfinder_data_t blobfinder_data;

		// PTZ interface (provides)
		player_devaddr_t         ptz_id;
		player_ptz_data_t        ptz_data;
	
		// Camera interface (provides)
		player_devaddr_t         cam_id;
		player_camera_data_t     cam_data;

		int pan_position;
		int tilt_position;

		void CheckConfigPtz        ();
		void CheckConfigBlobfinder ();

		void RefreshDataBlobfinder ();
		void RefreshDataPtz        ();
		void RefreshDataCamera     ();

		int BlobORCamera;

	public:

		// constructor 
		//
		Cmucam2( ConfigFile* cf, int section);
		~Cmucam2();

		// Process incoming messages from clients 
		virtual int ProcessMessage(QueuePointer & resp_queue, 
								   player_msghdr * hdr, 
								   void * data);

		virtual void Main();
  
		int Setup();
		int Shutdown();
};

////////////////////////////////////////////////////////////////////////////////
// a factory creation function
Driver* Cmucam2_Init( ConfigFile* cf, int section)
{
	return((Driver*)(new Cmucam2( cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// a driver registration function
void 
Cmucam2_Register(DriverTable* table)
{
	table->AddDriver("cmucam2", Cmucam2_Init);
}

Cmucam2::Cmucam2( ConfigFile* cf, int section)
	: Driver(cf, section)
{
	memset (&this->blobfinder_id, 0, sizeof (player_devaddr_t));
	memset (&this->ptz_id,        0, sizeof (player_devaddr_t));
	memset (&this->cam_id,        0, sizeof (player_devaddr_t));
			
	BlobORCamera = cf->ReadInt (section, "bloborcamera", 1);
	if ((BlobORCamera != 1) && (BlobORCamera != 2))
		BlobORCamera = 1;
	
	// Outgoing blobfinder interface
	if(cf->ReadDeviceAddr(&(this->blobfinder_id), section, "provides",
	   PLAYER_BLOBFINDER_CODE, -1, NULL) == 0)
	{
		if(this->AddInterface(this->blobfinder_id) != 0)
		{
			this->SetError(-1);
			return;
		}
	}
  
	// Outgoing camera interface
	if(cf->ReadDeviceAddr(&(this->cam_id), section, "provides",
	   PLAYER_CAMERA_CODE, -1, NULL) == 0)
	{
		if(this->AddInterface(this->cam_id) != 0)
		{
			this->SetError(-1);
			return;
		}
	}
	
	// Outgoing ptz interface
	if(cf->ReadDeviceAddr(&(this->ptz_id), section, "provides",
	   PLAYER_PTZ_CODE, -1, NULL) == 0)
	{
		if(this->AddInterface(this->ptz_id) != 0)
		{
			this->SetError(-1);
			return;
		}
	}

	if (BlobORCamera == 1)
	{
		num_of_blobs = cf->ReadInt (section, "num_blobs", 1);
		char variable[20];

		color = new color_config[num_of_blobs];
		for (int i = 0; i < num_of_blobs; i++)
		{
			sprintf (variable, "color%d", i);   
			color[i].rmin = (int)cf->ReadTupleFloat (section, variable, 0, 16);
			color[i].rmax = (int)cf->ReadTupleFloat (section, variable, 1, 16);
			color[i].gmin = (int)cf->ReadTupleFloat (section, variable, 2, 16);
			color[i].gmax = (int)cf->ReadTupleFloat (section, variable, 3, 16);
			color[i].bmin = (int)cf->ReadTupleFloat (section, variable, 4, 16);
			color[i].bmax = (int)cf->ReadTupleFloat (section, variable, 5, 16);
		}
	}
	else
	{
		color = new color_config[1];
		num_of_blobs = 0;
		color[0].rmin = 0; color[0].rmax = 0;
		color[0].gmin = 0; color[0].gmax = 0;
		color[0].bmin = 0; color[0].bmax = 0;
	}

	pan_position = 0;
	tilt_position = 0;
 
	if(!(this->devicepath = (char*)cf->ReadString(section, "devicepath", NULL)))
	{
		PLAYER_ERROR("must specify devicepath");
		this->SetError(-1);
		return;
	}
}

Cmucam2::~Cmucam2()
{
	delete [] color;
}

////////////////////////////////////////////////////////////////////////////////
int Cmucam2::Setup()
{
	fflush(stdout);

	fd = open_port((char*)devicepath);        // opening the serial port
	if(fd<0)                           // if not successful, stop
	{
		printf("Camera connection failed!\n");
		return -1; 
	}
	auto_servoing(fd, 0);

	/* now spawn reading thread */
	StartThread();

	return(0);
}


////////////////////////////////////////////////////////////////////////////////
int Cmucam2::Shutdown()
{
	StopThread();
	stop_tracking(fd);
	close_port(fd);                 // close the serial port
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
int Cmucam2::ProcessMessage (QueuePointer & resp_queue, 
								player_msghdr * hdr,
								void * data)
{
	assert(hdr);
	assert(data);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 0, ptz_id))
	{
		player_ptz_cmd_t & command = 
				*reinterpret_cast<player_ptz_cmd_t * > (data);

		if(pan_position != (short)ntohs((unsigned short)(command.pan)))
		{
			pan_position = (short)ntohs((unsigned short)(command.pan));
			if( abs(pan_position) <= 90 )
				// Pan value must be negated.
				set_servo_position(fd, 0, -1*pan_position);
		}
		if(tilt_position != (short)ntohs((unsigned short)(command.tilt)))
		{
			tilt_position = (short)ntohs((unsigned short)(command.tilt));
			if( abs(tilt_position) <= 90 )
				// Tilt value must be negated.
				set_servo_position(fd, 1, -1*tilt_position);
		}

		Publish (this->ptz_id, resp_queue,
				 PLAYER_MSGTYPE_RESP_ACK,
				 hdr->subtype);
		return 0;
	}	

// 	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
// 		PLAYER_PTZ_REQ_AUTOSERVO, ptz_id))
// 	{
// 		player_ptz_req_control_mode *servo = (player_ptz_req_control_mode*)data;
// 		auto_servoing (fd, servo->mode);
// 		if(servo->mode)
// 			PLAYER_MSG0 (1, "Auto servoing is enabled.");
// 		else
// 			PLAYER_MSG0 (1, "Auto servoing is disabled.");
// 
// 		Publish (this->ptz_id, resp_queue,
// 				 PLAYER_MSGTYPE_RESP_ACK,
// 				 PLAYER_PTZ_REQ_AUTOSERVO);
// 		return PLAYER_MSGTYPE_RESP_ACK;
// 	}

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
		PLAYER_BLOBFINDER_REQ_SET_COLOR, blobfinder_id))
	{
		player_blobfinder_color_config_t color_config;
		color_config = *((player_blobfinder_color_config_t*)data);

		color[0].rmin = color_config.rmin;
		color[0].rmax = color_config.rmax;
		color[0].gmin = color_config.gmin;
		color[0].gmax = color_config.gmax;
		color[0].bmin = color_config.bmin;
		color[0].bmax = color_config.bmax;
		
		PLAYER_MSG6 (1, 
		"Cmucam2_blobfinder received new tracking color: [%d,%d,%d,%d,%d,%d]",
					 color[0].rmin, color[0].rmax,
					 color[0].gmin, color[0].gmax,
					 color[0].bmin, color[0].bmax);

		Publish (this->blobfinder_id, resp_queue,
				 PLAYER_MSGTYPE_RESP_ACK,
				 PLAYER_BLOBFINDER_REQ_SET_COLOR);
		return PLAYER_MSGTYPE_RESP_ACK;
	}
	
	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
		PLAYER_BLOBFINDER_REQ_SET_IMAGER_PARAMS, blobfinder_id))
	{
		player_blobfinder_imager_config player_ic;
		player_ic = *((player_blobfinder_imager_config*)data);
		
		stop_tracking (fd);

		imager_config ic;
                memset(&ic,0,sizeof(imager_config));

		ic.brightness = (int16_t)player_ic.brightness;
		ic.contrast   = (int16_t)player_ic.contrast;
		ic.autogain   = (int16_t)player_ic.autogain;
		ic.colormode  = (int8_t)player_ic.colormode;

		if (set_imager_config (fd, ic) != 1)
			PLAYER_WARN ("Cmucam2_blobfinder set_imager_params failed!");

		color_config cc;
		track_blob (fd, cc);
		
		Publish (this->blobfinder_id, resp_queue,
				 PLAYER_MSGTYPE_RESP_ACK,
				 PLAYER_BLOBFINDER_REQ_SET_IMAGER_PARAMS);
		return PLAYER_MSGTYPE_RESP_ACK;
	}
	
	return -1;
}



////////////////////////////////////////////////////////////////////////////////
void Cmucam2::Main()
{
	memset (&blobfinder_data, 0, sizeof (blobfinder_data));
	memset (&ptz_data,        0, sizeof (ptz_data       ));
	memset (&cam_data,        0, sizeof (cam_data       ));

	for(;;)
	{
    // handle commands and requests/replies -----------------------------------
		pthread_testcancel();
		ProcessMessages();
		pthread_testcancel();

    // get data ---------------------------------------------------------------
		this->RefreshDataPtz ();

		if (BlobORCamera == 1)
			this->RefreshDataBlobfinder ();
		if (BlobORCamera == 2)
			this->RefreshDataCamera     ();
	}

	pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////////////////
void Cmucam2::RefreshDataBlobfinder ()
{
	packet_t                  blob_info;
	player_blobfinder_blob_t  blob;
	int                       blobs_observed;

	memset (&this->blobfinder_data, 0, sizeof (this->blobfinder_data));

	blobfinder_data.width       = IMAGE_WIDTH;
	blobfinder_data.height      = IMAGE_HEIGHT;
	blobfinder_data.blobs_count = num_of_blobs;
	blobfinder_data.blobs       = new player_blobfinder_blob_t [blobfinder_data.blobs_count];

	blobs_observed = 0;

	for (int i = 0; i < num_of_blobs; i++)
	{
		track_blob (fd, color[0]);
		if (!get_t_packet (fd, &blob_info))
			pthread_exit (NULL);

		stop_tracking (fd);

		get_blob (blob_info, &blob, color[i]);
		if (blob.area > 0)
			blobs_observed++;

		blob.id     = 0;
		memcpy (&blobfinder_data.blobs[i], &blob, sizeof (blob));
	}

	blobfinder_data.blobs_count = blobs_observed;
  
	/* got the data. now fill it in */
	Publish (this->blobfinder_id, PLAYER_MSGTYPE_DATA, 
			 PLAYER_BLOBFINDER_DATA_BLOBS, &blobfinder_data, 
			 sizeof (player_blobfinder_data), NULL);
	delete [] blobfinder_data.blobs;
	return;
}

////////////////////////////////////////////////////////////////////////////////
void Cmucam2::RefreshDataPtz        ()
{
	memset (&this->ptz_data, 0, sizeof (this->ptz_data));

	ptz_data.zoom      = 45;                  // cmucam does not have these 
	ptz_data.panspeed  = 0;
	ptz_data.tiltspeed = 0;
	ptz_data.zoom      = ptz_data.zoom;
	ptz_data.panspeed  = ptz_data.panspeed;
	ptz_data.tiltspeed = ptz_data.tiltspeed;
 
	ptz_data.pan   = -1*get_servo_position (fd, 0);
	ptz_data.tilt  = -1*get_servo_position (fd, 1);
	ptz_data.pan   = ptz_data.pan;
	ptz_data.tilt  = ptz_data.tilt; 

	Publish (this->ptz_id, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE, 
			 &ptz_data, sizeof (player_ptz_data_t), NULL);
	
	return;
}

////////////////////////////////////////////////////////////////////////////////
void Cmucam2::RefreshDataCamera     ()
{
	packet_f*              camera_packet;

        camera_packet = (packet_f*)malloc(sizeof(packet_f));
        assert(camera_packet);
  
	memset (&this->cam_data, 0, sizeof (this->cam_data));
	memset (camera_packet,  0, sizeof (packet_f));

	if (read_image (fd, -1, camera_packet) != 0)
        {
          free(camera_packet);
          pthread_exit (NULL);
        }

	get_image (camera_packet, &cam_data);
        free(camera_packet);

	Publish (this->cam_id, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE, 
			 &cam_data, sizeof (player_camera_data_t), NULL);
	return;
}

/**************************************************************************	
                           *** GET BLOB ***
**************************************************************************/
/* Description: This function uses CMUcam's T packet for tracking to get
   the blob information as described by Player
   Parameters:  cam_packet: camera's T packet generated during tracking
                color: the color range used in tracking
   Returns:     The Player format for blob information
*/

void Cmucam2::get_blob (packet_t cam_packet, 
						player_blobfinder_blob_t *blob,
						color_config range)
{
	// a descriptive color for the blob
	unsigned char red   = (range.rmin + range.rmax)/2; 
	unsigned char green = (range.gmin + range.gmax)/2;
	unsigned char blue  = (range.bmin + range.bmax)/2;
  
	(*blob).color  = red << 16 + green << 8 + blue;
	// the number of pixels in the blob
	(*blob).area   = cam_packet.blob_area;
	// setting the bounding box for the blob
	(*blob).x      = 2*cam_packet.middle_x;
	(*blob).y      = cam_packet.middle_y;
	(*blob).left   = 2*cam_packet.left_x;
	// highest and lowest y-value for top and bottom
	(*blob).right  = 2*cam_packet.right_x;
	(*blob).top    = (cam_packet.left_y > cam_packet.right_y) ? 
			cam_packet.left_y : cam_packet.right_y;
	(*blob).bottom = (cam_packet.left_y <= cam_packet.right_y) ? 
			cam_packet.left_y : cam_packet.right_y;
}

/**************************************************************************	
                           *** GET IMAGE ***
**************************************************************************/
/* Description: This function uses CMUcam's F packet to get an image
   Parameters:  packet: camera's F packet in raw format
   Returns:     The Player format for image data
*/

void Cmucam2::get_image (packet_f* cam_packet, 
						 player_camera_data_t *cam_data)
{
	int x = 0;
	int y = 0;
  
	(*cam_data).width       = cam_packet->xsize;
	(*cam_data).height      = cam_packet->ysize;
	(*cam_data).bpp         = 24;
	(*cam_data).format      = 5;    // PLAYER_CAMERA_FORMAT_RGB888
	(*cam_data).fdiv        = 1;
	(*cam_data).compression = 0;    // PLAYER_CAMERA_COMPRESS_RAW
	(*cam_data).image_count = (*cam_data).width * 2 * (*cam_data).height;

	for (y = 0; y < (int32_t)(*cam_data).height; y++)
	{
		for (x = 0; x < (int32_t)(*cam_data).width; x++)
		{
			int red   = cam_packet->rows[y].rgb[x].r << 16;
			int green = cam_packet->rows[y].rgb[x].g << 8;
			int blue  = cam_packet->rows[y].rgb[x].b;
			(*cam_data).image[y * (*cam_data).width * 2 + x * 2]
					= (red + green + blue);
			(*cam_data).image[y * (*cam_data).width * 2 + x * 2 + 1] 
					= (red + green + blue);
		}
	}

	return;
}
