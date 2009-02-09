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

/////////////////////////////////////////////////////////////////////////////////////
// Desc: RS4-Leuze range finder laser driver 
// Author: Ernesto Homar Teniente Avilés
//	   (C) Institut de Robótica Industrial
//	   Universidad Politécnica de Catalunya
// Date: May, 2007
// Version: 1.5.001
// 
//
// Usage:
//   (empty)
//
// Theory of operation:
//   Based on the 
//
// Known Bugs:
//   (empty)
//
// Possible enhancements:
//   (empty)
//
/////////////////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_RS4Leuzelaser RS4LeuzeLaser
 * @brief RS4-E leuze laser range-finder

The RS4-LeuzeLaser driver acquire date from the Leuze RS4 scanning laser range-finder.
Communication with the laser is via RS232.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_laser

@par Requires

- none

@par Configuration requests

- PLAYER_LASER_REQ_GET_GEOM
- PLAYER_LASER_REQ_GET_CONFIG
- PLAYER_LASER_REQ_SET_CONFIG

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS1" 
  - Port to which the laser is connected.  If you are using a USB/232 converter,
    this will be "dev/ttyUSBx". 

- pose (float tuple m m rad)
  - Default: [0.0 0.0 0.0]
  - Pose (x,y,theta) of the laser, relative to its parent object (e.g.,
    the robot to which the laser is attached).

- min_angle, max_angle (angle float)
  - Default: [] (or [-5.04 185.04] in degrees)
  - Minimum and maximum scan angles to return


- baud (integer)
  - Default: 57600
  - Baud rate to use when communicating with the laser over RS232.  Valid  rates are: 4800, 9600,
    19200, 38400, 57600, and 115200.  It should be chosen accordingly with the RS4 laser settings.

- scan_points (integer)
  - Default: 132
  - Others : 176, 264, 528

- invert_data(bool)
  - Default: 1 (invert_data_on) Leuze data must be inverted in order to be used in player
  - 0 (invert_data_off)  - Is the laser physically inverted (i.e., upside-down)?  Is so, scan data 
    will not be reversed accordingly.

@par Example

@verbatim
driver
(
  name	"rs4leuzelaser"
  plugin "RS4LeuzeLaserDriver.la"
  provides ["laser:0"]
  port "/dev/ttyS1"
  scan_points "528"
  #invert_data "0"

)
@endverbatim

@author Ernesto Homar Teniente Avilés
	IRI-UPC
	CONACYT-Schoolarship

*/
/** @} */




#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>

#include <time.h>

#include "RS4Leuze_laser.h"


#include <libplayercore/playercore.h>

using namespace std;

//Header file. Player driver class for the RS4 Leuze Laser(1);
class RS4LeuzeLaserDriver : public Driver {
public:

	// Constructor, teh constructor takes a CongifgFile parameter 
	//and an integer section parameter(2);
	RS4LeuzeLaserDriver(ConfigFile* cf, int section);
	// Destructor
	~RS4LeuzeLaserDriver();

	//The driver  must implement the abstract Setup and Shutdown methods (3);
	// Implementations of virtual functions
	int Setup();
	int Shutdown();
	//The drivers re-implements the ProcessMessage method to provide support for 
	//handling request and commands(4)
	// This method will be invoked on each incoming message
	virtual int ProcessMessage(QueuePointer &resp_queue,
                               player_msghdr * hdr,
                               void * data);

private:
	//The drivers re-implements Main, wich will be called when the dirver thread 
	//is started(5)
	// Main function for device thread.
	virtual void Main();

	RS4Leuze_laser_readings_t Readings;
	Claser *myLaser;
	bool laser_ON;

	player_laser_data_t Data;
	player_laser_geom_t Geom;
	player_laser_config_t Conf;

	//bool UseSerial;
	int BaudRate;
	const char * Port;
	bool invert;
	int ScanPoints;
	struct timeval tv;/**<termios variable time interval*/
	timeval timeStamp; /**<Time in microseconds resolution*/

};


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
RS4LeuzeLaserDriver::RS4LeuzeLaserDriver(ConfigFile* cf, int section)
: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_LASER_CODE)
{


    	//init vars
	memset(&Data, 0, sizeof(Data));
	memset(&Geom, 0, sizeof(Geom));
	memset(&Conf, 0, sizeof(Conf));
	Geom.size.sw = (0.050);
	Geom.size.sl = (0.050);



    	// read options from config file
	Geom.pose.px = (cf->ReadTupleLength(section,"pose",0,0));
	Geom.pose.py = (cf->ReadTupleLength(section,"pose",1,0));
	Geom.pose.pyaw = (cf->ReadTupleAngle(section,"pose",2,0));

	//set up config structure
	Conf.min_angle = cf->ReadAngle(section,"min_angle",DTOR(-5.04));
	Conf.max_angle = cf->ReadAngle(section,"max_angle",DTOR(185.04));
	//Conf.resolution = depends on the laser resolution
        Conf.max_range = 50;
	Conf.range_res = 0.01;
	Conf.intensity = 0;

	
   	// serial configuration 
		
	PLAYER_MSG1(1, "%s", "myLaser RS4 Leuze:");

	int b = cf->ReadInt(section, "baud", 57600);
	switch(b)
	{
		case 115200:
			BaudRate = B115200;
			break;
		case 57600:
			BaudRate = B57600;
			break;
		case 38400:
			BaudRate = B38400;
			break;	
		case 19200:
			BaudRate = B19200;
			break;
		case 9600:
			BaudRate = B9600;
			break;
		case 4800:
			BaudRate = B4800;
			break;
		default:
			PLAYER_WARN1("ignoring invalid baud rate %d", b);
			BaudRate = B57600;
			b = 57600;
			break;
	}
	PLAYER_MSG1(1, "baud rate: %d", b);

        Port = cf->ReadString(section, "port", "/dev/ttyS1");
	PLAYER_MSG1(1, "port: %s", Port);

	// Scan points configuration
	int sc = cf->ReadInt(section, "scan_points", 132);
	switch(sc)
	{
		case 132:
			ScanPoints = 133;
		Conf.resolution = DTOR(4*0.36);
			break;
		case 176:
			ScanPoints = 177;
		Conf.resolution = DTOR(3*0.36);
			break;
		case 264:
			ScanPoints = 265;
		Conf.resolution = DTOR(2*0.36);
			break;
		case 528:
			ScanPoints = 529;
			Conf.resolution = DTOR(0.36);
			break;
		default:
			PLAYER_WARN1("ignoring invalid scan points %d", sc);
			ScanPoints = 133;
			Conf.resolution = DTOR(4*0.36);
			break;
	}
	PLAYER_MSG1(1, "scan points: %d", ScanPoints);

	//invert data from teh leuze. Check if the leuze is upside-down.Normally dat must be inverted
	//int sc = cf->ReadInt(section, "scan_points", 132);	
	invert = cf -> ReadInt(section, "invert_data", 1);

	laser_ON = 1;

	myLaser=new Claser(ScanPoints);
	Data.ranges = new float[ScanPoints];
    return;
}

RS4LeuzeLaserDriver::~RS4LeuzeLaserDriver()
{
	//Reading are erased
	delete myLaser;	
	delete [] Data.ranges;
	//delete Readings;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int RS4LeuzeLaserDriver::Setup() {
	//config data
	// Si el el puerto no fue abierto, entonces mandar error. if(*mylaser.
	//if(Laser.Open(Port,UseSerial,BaudRate) < 0)
	//{
	//	this->SetError(1);
	//	return -1;
	//}
	PLAYER_MSG1(1, "%s", "S4LeuzeLaserDriver::Setup");

    // Start the device thread; spawns a new thread and executes
    // ExampleDriver::Main(), which contains the main loop for the driver.
	StartThread();


    return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int RS4LeuzeLaserDriver::Shutdown() {
  // Stop and join the driver thread
  	StopThread();

  	myLaser->closeSerial();

  return(0);
}


int RS4LeuzeLaserDriver::ProcessMessage(QueuePointer &resp_queue,
                                  player_msghdr * hdr,
                                  void * data)
{
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_LASER_REQ_GET_GEOM,
                           this->device_addr))
	{
		Publish(device_addr,resp_queue, PLAYER_MSGTYPE_RESP_ACK,hdr->subtype,&Geom,sizeof(Geom),NULL);
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
		PLAYER_LASER_REQ_GET_CONFIG,
	   this->device_addr))
	{
		Publish(device_addr,resp_queue, PLAYER_MSGTYPE_RESP_ACK,hdr->subtype,&Conf,sizeof(Conf),NULL);
	}
	else
	{
		return -1;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void RS4LeuzeLaserDriver::Main()
{
	PLAYER_MSG1(1, "%s", "RS4LeuzeLaserDriver::Main");

	bool laser_ON=1;
	//int i;

	PLAYER_MSG1(1, "%s", "Laser Ok");

	// The main loop; interact with the device here
	for(int veces = 0;;veces++)	{
    	// test if we are supposed to cancel
    	pthread_testcancel();

		// Process any pending messages
		ProcessMessages();
		// update device data
		myLaser->openSerial(&laser_ON, BaudRate, Port);
		myLaser->runLaser();
		Data.min_angle = Conf.min_angle;
		Data.max_angle = Conf.max_angle;
                Data.max_range = Conf.max_range;
		Data.resolution = Conf.resolution;
		Data.ranges_count = ScanPoints;

		//cout << endl << "Data RS4leuze Player : ";

		int top_ii = Data.ranges_count;
		float tmp;
		PLAYER_MSG1(1, "%s", "Data: ");
		for (unsigned int i = 0; i < Data.ranges_count; ++i)
		{ 
			tmp = myLaser->scanData.Reading[i];
			if (invert)
			{
			//Inverting the data, Laser upside
				Data.ranges[top_ii] = tmp;
				--top_ii;
			}
			else
			{
			//Laser upside-down
				Data.ranges[i] = tmp;
			}
			PLAYER_MSG1(1, "%.4f ", Data.ranges[i]);
		}
		

		Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN,
			&Data, sizeof(player_laser_data_t), NULL);
		//cout << endl << "Data RS4leuze passed ";

		myLaser->closeSerial();
		getchar();
		//cout << endl <<  "end laser **************" <<endl;
	}

}



////////////////////////////////////////////////////////////////////////////////
// Things for building shared object, and functions for registering and creating
//  new instances of the driver
////////////////////////////////////////////////////////////////////////////////

//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* RS4LeuzeLaserDriver_Init(ConfigFile* cf, int section) {
    // Create and return a new instance of this driver
	return((Driver*)(new RS4LeuzeLaserDriver(cf, section)));
}

//Registers the driver in the driver table. Called from the
// player_driver_init function that the loader looks for
int RS4LeuzeLaserDriver_Register(DriverTable* table) {
	table->AddDriver("rs4leuze", RS4LeuzeLaserDriver_Init);
  return 0;
}
