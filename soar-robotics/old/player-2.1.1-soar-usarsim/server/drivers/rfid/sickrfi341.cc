/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007
 *     Radu Bogdan Rusu (rusu@cs.tum.edu)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 Desc: Driver for the SICK RFI341 (Radio Frequency Interrogator) unit
 Author: Nico Blodow and Radu Bogdan Rusu
 Date: 9 Mar 2007
 CVS: $Id: sickrfi341.cc 4232 2007-11-01 22:16:23Z gerkey $
*/
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_sickrfi341 sickrfi341
 * @brief SICK RFI341 RFID reader

The sickrfi341 driver controls the SICK RFI341 RFID reader (13.56Mhz). 

@par Compile-time dependencies

- none

@par Provides

- @ref interface_rfid

@par Requires

- none

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port to which the SICK RFI341 reader is attached.  If you are 
    using a USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- connect_rate (integer)
  - Default: 9600.
  - Rate at which we connect to the RFID unit. Valid values are 1200, 2400,
    4800, 9600, 19200, 38400, 57600 and 115200.


- transfer_rate (integer)
  - Default: 9600.
  - Rate at which we want to transfer data from the RFID unit. Valid values
    are 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200.

- debug (integer)
  - Default: 0
  - Print additional debug informations on screen. Valid values are 0 and 1.
  
@par Example 

@verbatim
driver
(
  name "skyetekM1"
  provides ["rfid:0"]
  port "/dev/ttyS0"
  connect_rate 9600
  transfer_rate 38400
)
@endverbatim

@author Radu Bogdan Rusu

*/
/** @} */

#include "rfi341_protocol.h"
#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

#define DEFAULT_RFI341_PORT "/dev/ttyS0"
#define DEFAULT_RFI341_RATE 9600

// The SICK RFI 341 device class.
class SickRFI341 : public Driver
{
  public:
    
    // Constructor/Destructor
    SickRFI341  (ConfigFile* cf, int section);
    ~SickRFI341 ();

    int Setup    ();
    int Shutdown ();

    // MessageHandler
    int ProcessMessage (QueuePointer &resp_queue, 
		        player_msghdr* hdr, 
		        void* data);
  private:
    // Main function for device thread.
    virtual void Main ();
    
    // Reference to rfi341_protocol
    rfi341_protocol* rfi341;
    
    // connection parameters
    const char* portName;
    int         connect_rate;
    int         transfer_rate;
    int         current_rate;
    
    int debug;
};

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
SickRFI341::SickRFI341 (ConfigFile* cf, int section)
    : Driver (cf, section, true, 
              PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_RFID_CODE)
{
  // Read connection settings
  portName = cf->ReadString (section, "port", DEFAULT_RFI341_PORT);
  connect_rate  = cf->ReadInt (section, "connect_rate", DEFAULT_RFI341_RATE);
  transfer_rate = cf->ReadInt (section, "transfer_rate", DEFAULT_RFI341_RATE);
  current_rate  = 0;
  
  debug = cf->ReadInt (section, "debug", 0);
}

////////////////////////////////////////////////////////////////////////////////
// Destructor.
SickRFI341::~SickRFI341 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int
  SickRFI341::Setup ()
{
  // Create a rfi341_protocol object
  rfi341 = new rfi341_protocol (portName, debug);

  // Attempt to connect to the rfid unit
  if (rfi341->Connect (connect_rate) != 0)
    return (-1);

  current_rate = connect_rate;
  
  if (connect_rate != transfer_rate)
  {
    // Attempt to connect to the rfid unit
    if (rfi341->SetupSensor (transfer_rate) != 0)
      return (-1);
    else
      current_rate = transfer_rate;
  }

  // Start the device thread
  StartThread ();

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int
  SickRFI341::Shutdown ()
{
  // shutdown rfid device
  StopThread ();
 
  // Change back to the original speed
  if (current_rate != connect_rate)
    rfi341->SetupSensor (connect_rate);
  
  // Disconnect from the rfid unit
  rfi341->Disconnect ();
  
  PLAYER_MSG0 (1, "> SICK RFI341 driver shutting down... [done]");

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage
int 
  SickRFI341::ProcessMessage (QueuePointer &resp_queue, 
                              player_msghdr* hdr,
                              void* data)
{
  assert (hdr);
  assert (data);
  
  return (-1);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void
  SickRFI341::Main () 
{
  timespec sleepTime = {0, 0};
  
  while (true)
  {
    // test if we are supposed to cancel
    pthread_testcancel ();
    
    // Request/replies handler
    ProcessMessages ();
  
    player_rfid_data_t data = rfi341->ReadTags ();
    
    // Make data available
    Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RFID_DATA_TAGS,
            &data, sizeof (data), NULL);
            
    player_rfid_data_t_cleanup(&data);
    nanosleep (&sleepTime, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Factory creation function. This functions is given as an argument when the 
// driver is added to the driver table
Driver*
  SickRFI341_Init (ConfigFile* cf, int section)
{
  return ((Driver*)(new SickRFI341 (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// Registers the driver in the driver table. Called from the player_driver_init 
// function that the loader looks for
void
  SickRFI341_Register (DriverTable* table)
{
  table->AddDriver ("sickrfi341", SickRFI341_Init);
}
