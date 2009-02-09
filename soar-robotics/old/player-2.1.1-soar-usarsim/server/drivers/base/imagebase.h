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
// Desc: base driver for image processing and transform drivers
// Author: Toby Collett
// Date: 15 Feb 2004
// CVS: $Id: imagebase.h 4367 2008-02-18 19:42:49Z thjc $
//
///////////////////////////////////////////////////////////////////////////


/** @ingroup drivers */
/** @{ */
/** @defgroup driver_imagebase imagebase
 * @brief driver base for image processing drivers, inherit from this driver and implement the process frame method


@par Requires

- This driver acquires image data from a @ref interface_camera
  interface. 

@par Provides

- Depends on inheriting class

@par Configuration requests

- none

@par Configuration file options

@par Example

@author Toby Collett
*/
/** @} */

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

// Driver for detecting laser retro-reflectors.
class ImageBase : public Driver
{
	public: 
		// Constructor
		ImageBase(ConfigFile *cf, int section, bool overwrite_cmds, size_t queue_maxlen, int interf);
		ImageBase(ConfigFile *cf, int section, bool overwrite_cmds = true, size_t queue_maxlen = PLAYER_MSGQUEUE_DEFAULT_MAXLEN);
		virtual ~ImageBase()
		{
		  if (stored_data.image) delete [](stored_data.image);
		  PLAYER_WARN("image deleted from the memory");
		}

		// Setup/shutdown routines.
		virtual int Setup();
		virtual int Shutdown();

		// Process incoming messages from clients 
		int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

	private:
	        ImageBase(); // no default constructor
	        ImageBase(const ImageBase &); // no copy constructor

	protected: 
		virtual int ProcessFrame() = 0;
		// Main function for device thread.
		virtual void Main();	
	
		// Input camera stuff
  		Device *camera_driver;
		player_devaddr_t camera_addr;
		player_camera_data_t stored_data;
		bool HaveData;
};
