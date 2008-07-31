/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003
 *     Brian Gerkey
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
 * A simple example of how to write a driver that will be built as a
 * shared object.
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
#include <math.h>

#include <libplayercore/playercore.h>

#include "sharedstruct.h"

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class OpaqueDriver : public Driver
{
  public:

    // Constructor; need that
    OpaqueDriver(ConfigFile* cf, int section);

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();

    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer &resp_queue,
                               player_msghdr * hdr,
                               void * data);

  private:

    // Main function for device thread.
    virtual void Main();

    // Update the data
    virtual void RefreshData();

    // This is the structure we want to send
    test_t mTestStruct;

    // This is the data we store and send
    player_opaque_data_t mData;
};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
OpaqueDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return((Driver*)(new OpaqueDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void OpaqueDriver_Register(DriverTable* table)
{
  table->AddDriver("opaquedriver", OpaqueDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
OpaqueDriver::OpaqueDriver(ConfigFile* cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
             PLAYER_OPAQUE_CODE)
{
  mData.data_count = sizeof(test_t);
  mData.data = reinterpret_cast<uint8_t*>(&mTestStruct);

  mTestStruct.uint8 = 0;
  mTestStruct.int8 = 0;
  mTestStruct.uint16 = 0;
  mTestStruct.int16 = 0;
  mTestStruct.uint32 = 0;
  mTestStruct.int32 = 0;
  mTestStruct.doub = 0;

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int OpaqueDriver::Setup()
{
  puts("Example driver initialising");

  // Here you do whatever is necessary to setup the device, like open and
  // configure a serial port.


  puts("Opaque driver ready");

  // Start the device thread; spawns a new thread and executes
  // OpaqueDriver::Main(), which contains the main loop for the driver.
  StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int OpaqueDriver::Shutdown()
{
  puts("Shutting opaque driver down");

  // Stop and join the driver thread
  StopThread();

  // Here you would shut the device down by, for example, closing a
  // serial port.

  puts("Opaque driver has been shutdown");

  return(0);
}

int OpaqueDriver::ProcessMessage(QueuePointer & resp_queue,
                                 player_msghdr* hdr,
                                 void* data)
{
  // Process messages here.  Send a response if necessary, using Publish().
  // If you handle the message successfully, return 0.  Otherwise,
  // return -1, and a NACK will be sent for you, if a response is required.
  return(0);
}



////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void OpaqueDriver::Main()
{
  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages.  OpaqueDriver::ProcessMessage() is
    // called on each message.
    ProcessMessages();

    // Interact with the device, and push out the resulting data, using
    RefreshData();

    // Sleep (you might, for example, block on a read() instead)
    usleep(100000);
  }
}

void OpaqueDriver::RefreshData()
{
   mTestStruct.uint8 += 1;
   mTestStruct.int8 += 1;
   mTestStruct.uint16 += 5;
   mTestStruct.int16 += 5;
   mTestStruct.uint32 += 10;
   mTestStruct.int32 += 10;
   mTestStruct.doub = sin(mTestStruct.uint8/10.0);

  // only send the data we need to
  uint32_t size = sizeof(mData) - sizeof(mData.data) + mData.data_count;
  Publish(device_addr, 
          PLAYER_MSGTYPE_DATA, PLAYER_OPAQUE_DATA_STATE,
          reinterpret_cast<void*>(&mData), size, NULL);
}
////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    puts("Opaque driver initializing");
    OpaqueDriver_Register(table);
    puts("Opaque driver done");
    return(0);
  }
}

