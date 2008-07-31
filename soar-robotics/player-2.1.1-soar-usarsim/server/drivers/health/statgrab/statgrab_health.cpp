/*
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

/** Player driver for the health of a robot.
  *
  * @author Raphael Sznitman
  * @date 23.06.2005*/

/** @ingroup drivers */
/** @{ */
/**
  * @defgroup driver_statgrab statgrab

The @p health driver allows for a user to get general systems data concerning a specific robot.
Allows a user to look at cpu and memory usage of the robot.

@par Provides
- @ref player_interface_health : Interface to the health


@par Configuration file options:
- frequency (int)
  - Default: 100

@par Notes

@par libstatgrab:
The driver uses the library libstatgrab in order to fetch data concerning the cpu and the
memory usage. Documentation on this library can be found at: http://www.i-scream.org/libstatgrab/.

@par Example: The following configuration file should demonstrate the use of this
driver.

@include health.cfg

@todo
- Test this code more!
- Allow for the driver to return the temperature of the system in multiple location of the robot.
- Also allow for the battery voltage to be monitored.
*/

/** @} */

#include "statgrab_health.h"

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <stdint.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Now the driver

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
StatGrabDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return((Driver*)(new StatGrabDriver(cf, section)));

}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void StatGrabDriver_Register(DriverTable* table)
{
  table->AddDriver("statgrabdriver", StatGrabDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
StatGrabDriver::StatGrabDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{

  // For Health Interface
  if(cf->ReadDeviceAddr(&mHealthId, section, "provides",
                        PLAYER_HEALTH_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(mHealthId))
    {
      this->SetError(-1);
      return;
    }
  }


  // Allow to just have to change the config file if you want to adjust the sleep
  // duration.
  mSleep = static_cast<int32_t>((1e6/cf->ReadInt(section, "frequency", 100)));

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int StatGrabDriver::Setup()
{
  // Initialise statgrab
  sg_init();
  /* Drop setuid/setgid privileges. */
  if (sg_drop_privileges() != 0)
  {
    perror("Error. Failed to drop privileges");
   return 1;
  }

  puts("Health driver ready");

  StartThread();

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int StatGrabDriver::Shutdown()
{

  puts("Shutting health driver down");

  // Stop and join the driver thread
  StopThread();

  puts("Health driver has been shutdown");

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void StatGrabDriver::Main()
{

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    usleep(mSleep);

    ProcessMessages();

    // Write outgoing data
    RefreshData();


  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
void StatGrabDriver::RefreshData()
{
  //double receivedCpu;
  float cpuIdle, cpuServer, cpuUser ;
	//CPU
	cpu_percent = sg_get_cpu_percents();

	cpuIdle =  cpu_percent->idle;
	mHealth.cpu_usage.idle = cpuIdle;
	cpuServer = cpu_percent->kernel + cpu_percent->iowait + cpu_percent->swap;
	mHealth.cpu_usage.system = cpuServer;
	cpuUser = cpu_percent->nice+ cpu_percent->user;
	mHealth.cpu_usage.user = cpuUser;



	//Virtual Memory
	mem_data     = sg_get_mem_stats();
	swap_stats   = sg_get_swap_stats();

	mHealth.mem.total = mem_data->total;
	mHealth.mem.used =  mem_data->used;
	mHealth.mem.free = mem_data->free;

	mHealth.swap.total = swap_stats->total;
	mHealth.swap.used = swap_stats->used;
	mHealth.swap.free = swap_stats->free;

   // Other data which should be retrieved here!

    Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_HEALTH_DATA_STATE  ,
            reinterpret_cast<void*>(&mHealth));


}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

// leftover from when this was a standalone plugin

/* need the extern to avoid C++ name-mangling  */
//extern "C" {
//  int player_driver_init(DriverTable* table)
//  {
//    puts("StatGrab driver initializing");
//    StatGrabDriver_Register(table);
//    puts("StatGrab driver done");
//    return(0);
//  }
//}
