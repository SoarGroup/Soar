/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  
 *     Brian Gerkey, Andrew Howard
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

/*
 * Desc: A simple example of how to write a driver that supports multiple interface.
 * Also demonstrates use of a driver as a loadable object.
 * Author: Andrew Howard
 * Date: 25 July 2004
 * CVS: $Id: multidriver.cc 4314 2007-12-14 00:19:55Z thjc $
 */


// ONLY if you need something that was #define'd as a result of configure 
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#include <libplayercore/playercore.h>


////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class MultiDriver : public Driver
{
  public:
    
    // Constructor; need that
    MultiDriver(ConfigFile* cf, int section);

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();
    virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr, 
                               void * data);

  private:
    // Main function for device thread.
    virtual void Main();

    // My position interface
    player_devaddr_t m_position_addr;
    // My laser interface
    player_devaddr_t m_laser_addr;

    // Address of and pointer to the laser device to which I'll subscribe
    player_devaddr_t laser_addr;
    Device* laser_dev;
};


// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver* MultiDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*) (new MultiDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void MultiDriver_Register(DriverTable* table)
{
  table->AddDriver("multidriver", MultiDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C"
{
  int player_driver_init(DriverTable* table)
  {
    MultiDriver_Register(table);
    return(0);
  }
}


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
MultiDriver::MultiDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{
  // Create my position interface
  if (cf->ReadDeviceAddr(&(this->m_position_addr), section, 
                         "provides", PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }  
  if (this->AddInterface(this->m_position_addr))
  {
    this->SetError(-1);    
    return;
  }

  // Create my laser interface
  if (cf->ReadDeviceAddr(&(this->m_laser_addr), section, 
                         "provides", PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }    
  if (this->AddInterface(this->m_laser_addr))
  {
    this->SetError(-1);        
    return;
  }

  // Find out which laser I'll subscribe to
  if (cf->ReadDeviceAddr(&(this->laser_addr), section, 
                         "requires", PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int MultiDriver::Setup()
{   
  puts("Example driver initialising");

  // Subscribe to the laser device
  if(!(this->laser_dev = deviceTable->GetDevice(this->laser_addr)))
  {
    PLAYER_ERROR("unable to locate suitable laser device");
    return(-1);
  }
  if(this->laser_dev->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return(-1);
  }

  // Here you do whatever else is necessary to setup the device, like open and
  // configure a serial port.
    
  puts("Example driver ready");

  // Start the device thread; spawns a new thread and executes
  // MultiDriver::Main(), which contains the main loop for the driver.
  this->StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int MultiDriver::Shutdown()
{
  puts("Shutting example driver down");

  // Stop and join the driver thread
  this->StopThread();

  // Unsubscribe from the laser
  this->laser_dev->Unsubscribe(this->InQueue);

  // Here you would shut the device down by, for example, closing a
  // serial port.

  puts("Example driver has been shutdown");

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void MultiDriver::Main() 
{
  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages.  Calls ProcessMessage() on each pending
    // message.
    this->ProcessMessages();

    // Do work here.  
    //
    // Send out new messages with Driver::Publish()
    //
    // For example, to send a new position pose message:
    player_position2d_data_t posdata;
    posdata.pos.px = 43.2;
    posdata.pos.py = -12.2;
    posdata.pos.pa = M_PI/3.0;
    posdata.vel.px = 0.25;
    posdata.vel.py = 0.0;
    posdata.vel.pa = -M_PI/6.0;
    posdata.stall = 0;

    this->Publish(this->m_position_addr, 
                  PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
                  (void*)&posdata, sizeof(posdata), NULL);

    
    // Sleep (or you might, for example, block on a read() instead)
    usleep(100000);
  }
  return;
}


int MultiDriver::ProcessMessage(QueuePointer & resp_queue, 
                                player_msghdr * hdr, 
                                void * data)
{
  // Handle new data from the laser
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, 
                           this->laser_addr))
  {
    // Do someting with it
    return(0);
  }
  
  // Tell the caller that you don't know how to handle this message
  return(-1);
}

