/****************************************************************************\
 *  CameraUVC version 0.1a                                                  *
 *  A Camera Plugin Driver for the Player/Stage robot server                *
 *                                                                          *
 *  Copyright (C) 2006 Raymond Sheh                                         *
 *  rsheh at cse dot unsw dot edu dot au     http://rsheh.cse.unsw.edu.au/  *
 *                                                                          *
 *  A Player/Stage plugin driver for cameras compatible with the Linux      *
 *  UVC camera driver (see http://linux-uvc.berlios.de/), such as the       *
 *  Logitech QuickCam Pro 5000.                                             *
 *                                                                          *
 *                                                                          *
 *  This program is free software; you can redistribute it and/or modify    *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation; either version 2 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program; if not, write to the Free Software             *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,                   *
 *  MA  02111-1307  USA                                                     *
 *                                                                          *
 *                                                                          *
 *  Portions based on the Player/Stage Sample Plugin Driver                 *
 *  Portions based on luvcview by Laurent Pinchart and Michel Xhaard        *
 *                                                                          *
 * Updated in September 2006 by Luke Gumbley to enhance stability
 *                                                                          *
 *                                                                          *
\****************************************************************************/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_camerauvc camerauvc
 * @brief USB video (UVC) camera interface
 
%Device driver for webcams supporting the USB video standard. Works with
recent Logitech cams (QuickCam Fusion, QuickCam Orbit, QuickCam Pro for 
Notebooks and QuickCam Pro 5000) and hopefully many USB webcams released 
2007 onward will support the standard.
 
@par Compile-time dependencies

- none

@par Requires

- none

@par Provides

- @ref interface_camera

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/video0"
  - Linux video device entry
- size (integer tuple)
  - Default: [320 240]
  - size of the image to grab, most cameras should at least support [640 480] as well

@par Example

@verbatim
driver
(
  name "camerauvc"
  provides ["camera:0"]
  port "/dev/video0"
  size [640 480]
)
@endverbatim

@authors Raymond Sheh, Modified by Luke Gumbley

 */
/** @} */

#include "cameraUVC.h"

Driver* CameraUvc_Init(ConfigFile* cf, int section)
{
	return((Driver*)(new CameraUvc(cf, section)));
}

void CameraUVC_Register(DriverTable* table)
{
	table->AddDriver("camerauvc", CameraUvc_Init);
}

/*
extern "C"
{
	int player_driver_init(DriverTable* table)
	{
		CameraUVC_Register(table);
		return(0);
	} 
}
*/

CameraUvc::CameraUvc(ConfigFile* cf, int section)
    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_CAMERA_CODE)
{
	ui=new UvcInterface(cf->ReadString(section, "port", "/dev/video0"),
						cf->ReadTupleInt( section, "size", 0, 320),
						cf->ReadTupleInt( section, "size", 1, 240));
}

CameraUvc::~CameraUvc()
{
  delete ui;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
// Also starts the mainloop. 
////////////////////////////////////////////////////////////////////////////////
int CameraUvc::Setup()
{   
	printf("CameraUvc: Driver initialising\n");

	if(ui->Open()==-1)
	{
		PLAYER_ERROR("CameraUvc: Error setting up video capture!");
		SetError(-1);
		return -1; 
	}

	StartThread();
	printf("CameraUvc: Driver initialisation done\n");
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device. 
// Also stops the mainloop. 
////////////////////////////////////////////////////////////////////////////////
int CameraUvc::Shutdown()
{
	printf("CameraUvc: Driver shutting down\n");

	StopThread();
	ui->Close();

	printf("CameraUvc: Driver shutdown complete\n");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main function (mainloop)
////////////////////////////////////////////////////////////////////////////////
void CameraUvc::Main() 
{
	while (true)
	{
		// Test if we are supposed to cancel this thread.
		pthread_testcancel();

		// Process any pending requests.
		ProcessMessages();

		ui->Read();

		// Store image details
		data.width=ui->GetWidth(); 
		data.height=ui->GetHeight(); 
		data.bpp=24; 
		data.format=PLAYER_CAMERA_FORMAT_RGB888;
		data.fdiv=1; 
		data.compression=PLAYER_CAMERA_COMPRESS_JPEG;
		data.image_count=ui->GetFrameSize();
		data.image = new unsigned char[data.image_count];
		ui->CopyFrame(data.image);

		// Write data to the client (through the server)
		Publish (device_addr,PLAYER_MSGTYPE_DATA,PLAYER_CAMERA_DATA_STATE,&data);
		delete [] data.image;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Process requests.  Returns 1 if the configuration has changed.
// Ignore all requests for now. 
////////////////////////////////////////////////////////////////////////////////
int CameraUvc::ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
  return -1;
}
