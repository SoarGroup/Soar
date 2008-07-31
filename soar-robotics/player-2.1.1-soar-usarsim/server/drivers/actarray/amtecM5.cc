/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 Alexis Maldonado (maldonad \/at/\ cs.tum.edu)
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
 * A driver for the Powercube Modules from Amtec Robotics, Berlin.
 * This driver uses the official C++ driver from Amtec (Called Powercube M5api).
 * You can download it from here after obtaining a password from the company:
 * http://www.powercube.de/files/PCube_M5api_Linux/M5DLL_SUSE_10.0.tar
 *
 * For precise timing, use a recent Linux kernel (2.6.19 should have the hrtimers code inside)
 */

// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/

//#include <unistd.h>
//#include <string.h>

#include <libplayercore/playercore.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <set>

// These flags are needed by Device.h
// They should probably be moved to -D flags on the compile of this program
// As they are, they assume a Linux system
#define __LINUX__
#define UNIX
#define LINUX

#include "Device.h"

const float initSpeed=0.2; //Rads/s
const float initAccel=0.2; //rads/s/s


//For nanosleep:
#include <time.h>
#include <sys/time.h>
//To catch the errors of nanosleep
#include <errno.h>

//For sched_setscheduler
#include <sched.h>



using namespace std;


//This function returns the difference in mS between two timeval structures
inline float timediffms(struct timeval start, struct timeval end) {
	return(end.tv_sec*1000.0 + end.tv_usec/1000.0 - (start.tv_sec*1000.0 + start.tv_usec/1000.0));
}


////////////////////////////////////////////
//This is the class for the Player Driver
////////////////////////////////////////////
class AmtecM5 : public Driver {
	public:

		// Constructor; need that
		AmtecM5(ConfigFile* cf, int section);
		~AmtecM5(void);

		// Must implement the following methods.
		virtual int Setup();
		virtual int Shutdown();

		// This method will be invoked on each incoming message
		virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);


	private:
		int HandleRequest(QueuePointer &resp_queue, player_msghdr * hdr, void* data);
		int HandleCommand(player_msghdr * hdr, void * data);
                int ModuleSyncMotion(int state);
                float normalize_angle(float angle);

		//This class talks to the Bus with Powercubes (CAN or RS232 bus)
                //Designed with the Factory Design Pattern:
                //CDevice is an abstract class. There is a concrete class that inherits from it for each serial protocol
		CDevice * pclDevice;
		bool initok;


		//Sampling rate and alarm time
		float samplingrate;
		float alarmtime;

                //Normally this should be 1. Set to -1 if you want to invert the direction of rotation/movement of a specific joint
                int directions[PLAYER_ACTARRAY_NUM_ACTUATORS];

                //Normally, this should be set to 0. Set to a value to add an offset to the position sent to the modules
                float offsets[PLAYER_ACTARRAY_NUM_ACTUATORS];

                //If 1, it limits the values of the commands sent to -pi rads and pi rads. (-180-180deg) (Safer for most rotary joints).
                int normalize_angles[PLAYER_ACTARRAY_NUM_ACTUATORS];

                //Should we show some debug messages?
                int debug_level;

                //if true, the modules have their synchro mode activated.
                bool modsynchro;

                //0 if they are off (powercubes are all halted). 1 if they are on.
                int motor_state;

		//Controls if we tell the scheduler to give us high priority
		int highpriority;

		//The connstring needed by the CDevice
		const char * connstring;

		//can==true if we are using CAN-Bus
		bool can;

		//Contain the speed and acceleration that the modules use with the command movePos
		std::vector<float> rampSpeed;
		std::vector<float> rampAccel;
		float initAccel; //Rads/s
		float initSpeed;

		// Number of modules to control
		unsigned int module_count;
		// IDs of those modules
		std::vector<int> idModuleList;

		// Main function for device thread.
		virtual void Main();

		player_actarray_data_t actArray;
		player_actarray_actuator_t *actuators;
		player_actarray_actuatorgeom_t *actuatorsGeom;
		player_actarray_position_cmd_t lastActArrayPosCmd;
		player_actarray_home_cmd_t lastActArrayHomeCmd;

		player_point_3d_t aaBasePos;
		player_orientation_3d_t aaBaseOrient;
		void ToggleActArrayPower (unsigned char val, bool lock = true);   // Toggle actarray power on/off
		void SetActArrayJointSpeed (int joint, double speed);             // Set a joint speed
		void HandleActArrayPosCmd (player_actarray_position_cmd_t cmd);
		void HandleActArrayHomeCmd (player_actarray_home_cmd_t cmd);
		int HandleActArrayCommand (player_msghdr * hdr, void * data);

};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
AmtecM5_Init(ConfigFile* cf, int section) {
	// Create and return a new instance of this driver
	return((Driver*)(new AmtecM5(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void AmtecM5_Register(DriverTable* table) {
	table->AddDriver("amtecM5", AmtecM5_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
AmtecM5::AmtecM5(ConfigFile* cf, int section)
		: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
         PLAYER_ACTARRAY_CODE) {


	using namespace std;

	// Read options from the configuration file

	//Connstring (Used by the Pcube driver from Amtec)
	connstring = cf->ReadString(section, "connstring", 0);

	//Sampling rate and alarm time in mS
	samplingrate = cf->ReadFloat(section, "samplingrate", 20.0);
	alarmtime = cf->ReadFloat(section, "alarmtime", 25.0);

	//Number of modules that we expect on the bus
	module_count = cf->ReadInt(section, "module_count", 0);

	//Should we display some debugging messages?
	debug_level = cf->ReadInt(section, "debug_level", 0);


	//Initial Speed and Acceleration
	initSpeed = cf->ReadFloat(section, "initial_ramp_speed", 0.1);
	initAccel = cf->ReadFloat(section, "initial_ramp_accel", 0.1);

	//Vector for the direction of the modules
	const char * directionString= cf->ReadString(section, "directions", 0);
	//Initialize with 1 (normal direction of rotation)
	fill(directions,directions+PLAYER_ACTARRAY_NUM_ACTUATORS,1);

       	//Initialize with false (modules move as soon as they get a command)
	modsynchro=false;

        //After power-on, the modules are ready to move.
        motor_state=1;

        if (PLAYER_ACTARRAY_NUM_ACTUATORS != 16) {
                cout << "NUM_ACTUATORS is no longer 16. Remember to fix server/drivers/actarray/amtecM5.cc accordingly.\n";
        }

	//This needs to be changed if PLAYER_ACTARRAY_NUM_ACTUATORS changes (now it's 16)
	int numbermatched_directions=sscanf(directionString,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&directions[0],&directions[1],&directions[2],&directions[3],&directions[4],&directions[5],&directions[6],&directions[7],&directions[8],&directions[9],&directions[10],&directions[11],&directions[12],&directions[13],&directions[14],&directions[15]);
	cout << "Found " << numbermatched_directions << " values for the direction vector." << endl;

        //Checking if the values for directions are correct. Only accept 1 or -1.
        for (int i = 0 ; i != numbermatched_directions ; ++i) {
                if ((directions[i]==-1) || (directions[i]==1)) {
                        //it's ok.
                } else {
                        cout << "Invalid value for direction[" << i << "]. Valid values are -1 and 1. Setting to 1.\n";
                        directions[i]=1;
                }
        }


	//Vector for the direction of the modules
	const char * offsetString= cf->ReadString(section, "offsets", 0);
	//Initialize with 1 (normal direction of rotation)
	fill(offsets,offsets+PLAYER_ACTARRAY_NUM_ACTUATORS,0.0);

	//This needs to be changed if PLAYER_ACTARRAY_NUM_ACTUATORS changes (now it's 16)
	int numbermatched_offsets=sscanf(offsetString,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",&offsets[0],&offsets[1],&offsets[2],&offsets[3],&offsets[4],&offsets[5],&offsets[6],&offsets[7],&offsets[8],&offsets[9],&offsets[10],&offsets[11],&offsets[12],&offsets[13],&offsets[14],&offsets[15]);
	cout << "Found " << numbermatched_offsets << " values for the offset vector." << endl;


	//Vector for the normalization of the angles of the modules
	const char * normalizeString= cf->ReadString(section, "normalize_angles", 0);
	//Initialize with 1 (normalize to -180deg and 180deg)
	fill(normalize_angles,normalize_angles+PLAYER_ACTARRAY_NUM_ACTUATORS,1);

	//This needs to be changed if PLAYER_ACTARRAY_NUM_ACTUATORS changes (now it's 16)
	int numbermatched_normalize=sscanf(normalizeString,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&normalize_angles[0],&normalize_angles[1],&normalize_angles[2],&normalize_angles[3],&normalize_angles[4],&normalize_angles[5],&normalize_angles[6],&normalize_angles[7],&normalize_angles[8],&normalize_angles[9],&normalize_angles[10],&normalize_angles[11],&normalize_angles[12],&normalize_angles[13],&normalize_angles[14],&normalize_angles[15]);
	cout << "Found " << numbermatched_normalize << " values for the angle normalization vector." << endl;


	//Ids of those modules. A mapping is done using this sequence to actuators[0], actuators[1], ...
	const char * idString= cf->ReadString(section, "module_ids", 0);
	int idlist[PLAYER_ACTARRAY_NUM_ACTUATORS];
	//Initialize with -1
	fill(idlist,idlist+PLAYER_ACTARRAY_NUM_ACTUATORS,-1);

	//This needs to be changed if PLAYER_ACTARRAY_NUM_ACTUATORS changes (now it's 16)
	int numbermatched_ids=sscanf(idString,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&idlist[0],&idlist[1],&idlist[2],&idlist[3],&idlist[4],&idlist[5],&idlist[6],&idlist[7],&idlist[8],&idlist[9],&idlist[10],&idlist[11],&idlist[12],&idlist[13],&idlist[14],&idlist[15]);

	cout << "Found " << numbermatched_ids << " IDs. The assignment follows:" << endl;
	for (int i=0 ; i<numbermatched_ids ; i++ ) {
		cout << "actuators[" << i << "] -> Module ID:" << idlist[i] << " Direction: " << directions[i] << " Offset: " << offsets[i] << " Normalize angle: " << normalize_angles[i] << endl;
	}

	if (numbermatched_ids != static_cast<int>(module_count)) {
		cout << "WARNING: The ammount of detected IDs (" << numbermatched_ids << ") is not equal to the expected module count (" << module_count << ").\n";
		cout << "Fix the config file.\n";
	}

	//Store the mapping in the idModuleList
	for (int i=0 ; i<numbermatched_ids ; i++ ) {
		idModuleList.push_back(idlist[i]);

		//also initialize the rampSpeed and rampAccel vectors
		rampAccel.push_back(initAccel);
		rampSpeed.push_back(initSpeed);
	}



	//Do we tell the scheduler that we need a very high priority?
	highpriority = cf->ReadInt(section, "highpriority", 0);

	actuators = NULL;
	actuatorsGeom = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor.
AmtecM5::~AmtecM5(void)
{
	if (actuators != NULL) {
		delete[] actuators;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
// Usually runs the first time someone connects to the interface
int AmtecM5::Setup() {
	using namespace std;

	puts("Amtec M5 Powercube driver initialising");

	// Here you do whatever is necessary to setup the device.

	if (highpriority != 0) {
		//The highest priority!
		struct sched_param par;
		par.sched_priority = sched_get_priority_max(SCHED_FIFO);
		if( sched_setscheduler(0,SCHED_FIFO,&par) == -1) {
			cout << "Error: Unable to set realtime priority" << endl;
			cout << "Disable this with \"highpriority 0\" in the configuration file." << endl;
			return(-1);
		}
	}




	if (module_count != idModuleList.size()) {
		cout << "The module count and the number of module IDs don't match.\n";
		cout << "Check your configuration file. Aborting.\n";
		return(-1);
	}

	if (module_count >= PLAYER_ACTARRAY_NUM_ACTUATORS) {
		cout << "The number of modules on this bus is bigger than the supported by this version of player.\n";
		cout << "The maximum is defined by: PLAYER_ACTARRAY_NUM_ACTUATORS=" << PLAYER_ACTARRAY_NUM_ACTUATORS << endl;
		cout << "Aborting!\n";
		return(-1);
	}

	//check if we can use the advanced features (only available when using a CAN bus).
	string connectionstr(connstring);
	if ( connectionstr.find(string("ESD")) == string::npos) {
		//Didn't find ESD in the connection string. Probably using RS232
		can=false;
	} else {
		can=true;
	}

	printf("Connection String: %s\n", connstring);

	//If connection string begins with ESD, an ESD-CAN device is returned.
	//If it begins with RS232, then a serial port device is returned
        //newDevice is the factory function that returns the appropriate specialized class.
	pclDevice = newDevice(connstring);

	//Set some flags to make the driver quieter
	const int g_iM5DebugLevel = 0;
	const int g_iM5DebugFile = 0;
	const int g_iM5Debug = 0;

	pclDevice->setDebug(g_iM5Debug);
	pclDevice->setDebugLevel(g_iM5DebugLevel);
	pclDevice->setDebugFile(g_iM5DebugFile);

	puts("Initializing the CAN/RS232 Bus:\n");
	//Actually connect to the BUS with init(connection_string)
        //pclDevice is already an instance of the appropriate class (CAN/RS232), init() opens the connection,
        // and reads the port number and speed parameter from the connstring.
	int iRetVal = pclDevice->init(connstring);

	if(iRetVal != 0) {
		delete pclDevice;
		pclDevice=0;
		printf("Failed\n");
		return(-1);
	} else {
		printf("Success opening port: %s\n",connstring);
	}

	//int arm_module_count=module_count;
	unsigned int arm_module_count=0;

	//Sometimes the modules don't get detected on the first pass
	iRetVal = pclDevice->updateModuleIdMap();
	usleep(100000);
	iRetVal = pclDevice->updateModuleIdMap();

	//Transfer the module IDs to a vector for easier manipulation
	std::vector<int> foundModuleList;
	iRetVal = pclDevice->getModuleIdMap(foundModuleList);

	cout << "Found the following modules: \n";
	for (unsigned int i=0; i != foundModuleList.size() ; ++i) {
		short unsigned int version(0);
		long unsigned int serial(0);
		pclDevice->getModuleVersion(foundModuleList[i],&version);
		pclDevice->getModuleSerialNo(foundModuleList[i],&serial);

		cout << "Bus ID: " << foundModuleList[i] << "  Version: 0x" << hex << version << dec << "  Serial No.: " << serial << ")\n";

	}
	cout << endl;


	if (foundModuleList.size() < arm_module_count) {
		printf("The system didn't detect all the expected modules (%d).\n", arm_module_count);
		printf("Only detected %d modules. Getting out.\n", static_cast<int>(foundModuleList.size()));
		return(-1);
	}


	//We need to check that the Module-IDs assigned in the configuration file are present
	//The easiest way is to create a set, fill it with the foundModuleList, measure the size of it,
	//  then add the IDs of the assignment from the config-file, and the size should stay the same
	//  (Only repeated values).
	set<int> checkset;
	for (unsigned int i=0; i<foundModuleList.size() ; i++) {
		checkset.insert(foundModuleList[i]);
	}

	for (unsigned int i=0; i<idModuleList.size() ; i++ ) {
		checkset.insert(idModuleList[i]);
	}

	if (checkset.size() != foundModuleList.size()) {
		cout << "At least one of the module IDs given in the config file was not found in the arm.\n";
		cout << "Please check the configuration file. Aborting.\n";
		return(-1);
	}


	for (unsigned int indx = 0; indx < idModuleList.size(); ++indx) {
		std::cout << "Resetting Module: " << foundModuleList[indx] << " " << std::endl;
		pclDevice->resetModule(foundModuleList[indx]);

		//pclDevice->setHomeVel(foundModuleList[indx],0.4); //Set speed to home-position
		//pclDevice->homeModule(foundModuleList[indx]);

		if (can) {
			//Activating the watchdog
			unsigned long config(0);
			pclDevice->getConfig(idModuleList[indx],&config);
			config |= CONFIGID_MOD_WATCHDOG_ENABLE;
			pclDevice->setConfig(idModuleList[indx],config);

			pclDevice->getConfig(idModuleList[indx],&config);
			if (config & CONFIGID_MOD_WATCHDOG_ENABLE) {
				printf("Watchdog activated for module: %d\n",idModuleList[indx]);
			}
		}

	}

	for (unsigned int indx = 0; indx < idModuleList.size(); ++indx) {
		unsigned long state(0);
		pclDevice->getModuleState(idModuleList[indx],&state);
		if ( (state & STATEID_MOD_HALT) || (state & STATEID_MOD_ERROR) ) {
			std::cout << "Error in module: " <<  idModuleList[indx] << " Resetting again." << std::endl;
			pclDevice->resetModule(idModuleList[indx]);

		}
	}

	//Allocate some space to send actuator data and geometry
	actuators = new player_actarray_actuator_t[module_count];
	if (actuators == NULL) {
		PLAYER_ERROR("amtecM5: Failed to allocate memory to store actuator data");
		return(-1);
	}
	actuatorsGeom = new player_actarray_actuatorgeom_t[module_count];
	if (actuatorsGeom == NULL) {
		PLAYER_ERROR("amtecM5: Failed to allocate memory to store actuator geometry");
		return(-1);
	}

	puts("Amtec M5 powercube driver ready.");

	// Start the device thread; spawns a new thread and executes
	// AmtecM5::Main(), which contains the main loop for the driver.
	StartThread();

	return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int AmtecM5::Shutdown() {
	puts("Shutting Amtec M5 Powercube driver down");

        //Wait a little to let everything settle
        sleep(1);  //We've been getting some segfaults at disconnect from the last client without this

	// Stop and join the driver thread
	StopThread();

        if (debug_level) {
                cout << "About to delete the pclDevice.\n";
        }
	delete pclDevice;  //The destructor closes the can/rs232 port
	pclDevice=0;

	if (actuators != NULL) {
		delete[] actuators;
		actuators = NULL;
	}

	if (actuatorsGeom != NULL) {
		delete[] actuatorsGeom;
		actuatorsGeom = NULL;
	}

	puts("Amtec M5 Powercube driver has been shutdown");

	return(0);
}

int AmtecM5::ProcessMessage(QueuePointer &resp_queue,
                                player_msghdr * hdr,
                                void * data) {
	// Process messages here.  Send a response if necessary, using Publish().
	// If you handle the message successfully, return 0.  Otherwise,
	// return -1, and a NACK will be sent for you, if a response is required

	//puts("ProcessMessage.");
	// Handle Requests and Commands
	if(hdr->type == PLAYER_MSGTYPE_REQ)
		return(HandleRequest(resp_queue,hdr,data));
	else if(hdr->type == PLAYER_MSGTYPE_CMD)
		return(HandleCommand(hdr,data));
	else
		return(-1);
}
int AmtecM5::HandleRequest(QueuePointer &resp_queue, player_msghdr * hdr, void* data) {
	//puts("Asked to handle a Request.");

        bool handled(false);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_ACTARRAY_REQ_POWER, device_addr))	{
		//Got a Power-Request
		int value =  ((reinterpret_cast<player_actarray_power_config_t*>(data))->value);
		//Do something with the powercubes to turn them on and off

                if (debug_level) {
                        printf("Power-Request. Value: %d\n",value);
                }

                //Since we cannot physically turn the modules off, this is what we do:
		//If power==off  -> Halt all the modules
		//If power==on   -> Reset all the modules

		if (value==1) {
			//reset all the modules
			printf("Resetting all the modules.\n");

			if (can) {
				pclDevice->resetAll();
			} else {
				for (unsigned int i=0; i != idModuleList.size() ; ++i) {
					pclDevice->resetModule(idModuleList[i]);
				}
			}
                        motor_state=1;

		} else {
			//halt all the modules
			printf("Halting all the modules.\n");

			if (can) {
				pclDevice->haltAll();
			} else {
				for (unsigned int i=0; i != idModuleList.size() ; ++i) {
					pclDevice->haltModule(idModuleList[i]);
				}
			}
                        motor_state=0;

		}

		//Respond with an acknowledgment
		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_ACTARRAY_REQ_POWER);
                handled=true;

	}  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,PLAYER_ACTARRAY_REQ_GET_GEOM,device_addr))  {

		//printf("Get-Geom Request.\n");

		player_actarray_geom_t aaGeom;

		aaGeom.actuators_count=module_count;
		aaGeom.actuators=actuatorsGeom;

		for (unsigned int i = 0; i < module_count; i++) {

			//Read Configuration from the module:
			unsigned long configword=0;
			pclDevice->getConfig(idModuleList[i],&configword);

			uint8_t hasbrakes=0;
			if (configword && CONFIGID_MOD_BRAKE_PRESENT) {
				hasbrakes=1;
			} else {
				hasbrakes=0;
			}

			uint8_t type=0;
			if (configword && CONFIGID_MOD_LINEAR) {
				type=PLAYER_ACTARRAY_TYPE_LINEAR;
			} else {
				type=PLAYER_ACTARRAY_TYPE_ROTARY;
			}

			float homeOffset=0.0;
			pclDevice->getHomeOffset(idModuleList[i],&homeOffset);
			float maxPos=0.0;
			pclDevice->getMaxPos(idModuleList[i],&maxPos);
			float minPos=0.0;
			pclDevice->getMinPos(idModuleList[i],&minPos);

			aaGeom.actuators[i].type = type;
			aaGeom.actuators[i].min = minPos;
			aaGeom.actuators[i].centre = 0.0;  //The powercubes have their zero position always at 0.0
			aaGeom.actuators[i].max = maxPos;
			aaGeom.actuators[i].home = homeOffset;
			aaGeom.actuators[i].config_speed = 0.0; //This speed is kept in the driver and not in the module. Should set a variable for it, and read it here.
			aaGeom.actuators[i].hasbrakes = hasbrakes;

			//The following parameters don't make sense for the Powercubes,
			// as the user can re-position them, and Amtec sells mostly custom configurations.
			//This information should come from a configuration file
			aaGeom.actuators[i].length = 0.0;
			aaGeom.actuators[i].orientation.proll = 0;
			aaGeom.actuators[i].orientation.ppitch = 0;
			aaGeom.actuators[i].orientation.pyaw = 0;
			aaGeom.actuators[i].axis.px = 0;
			aaGeom.actuators[i].axis.py = 0;
			aaGeom.actuators[i].axis.pz = 0;

		}
		//This should be read from the configuration file
		aaGeom.base_pos.px = 0;
		aaGeom.base_pos.py = 0;
		aaGeom.base_pos.pz = 0;
		aaGeom.base_orientation.proll = 0;
		aaGeom.base_orientation.ppitch = 0;
		aaGeom.base_orientation.pyaw = 0;

		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_ACTARRAY_REQ_GET_GEOM, &aaGeom, sizeof (aaGeom), NULL);
                handled=true;

	} else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,PLAYER_ACTARRAY_REQ_SPEED,device_addr)) {
		//printf("Speed Request. (Configuration of the speed for the next movements)\n");

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_speed_config_t * req = reinterpret_cast<player_actarray_speed_config_t*>(data);

		int joint=req->joint;
		float speed=req->speed;

		if (debug_level)
                        cout << "Speed_config. joint: " << joint << "  speed: " << speed << endl;

		if (joint == -1) {
			//set the speed to all the joints
			for (unsigned int i = 0 ; i != idModuleList.size() ; ++i) {
				rampSpeed[i]=speed;
			}

		} else {
			//Set the value in the speed vector
			rampSpeed[joint]=speed;
		}



		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_ACTARRAY_REQ_SPEED);
                handled=true;

	} else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,PLAYER_ACTARRAY_REQ_ACCEL,device_addr)) {

                //printf("Accel Request. (Configuration of the acceleration for the next movements)\n");

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_accel_config_t * req = reinterpret_cast<player_actarray_accel_config_t*>(data);

		int joint=req->joint;
		float accel=req->accel;

                if (debug_level) {
                        cout << "Accel_config. joint: " << joint << "  accel: " << accel << endl;
                }

		if (joint == -1) {
			//set the accel to all the joints
			for (unsigned int i = 0 ; i != idModuleList.size() ; ++i) {
				rampAccel[i]=accel;
			}

		} else {
			//Set the value in the acceleration vector
			rampAccel[joint]=accel;

		}

		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_ACTARRAY_REQ_ACCEL);
                handled=true;

	} else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,PLAYER_ACTARRAY_REQ_BRAKES,device_addr)) {
                if (debug_level) {
                        printf("Brakes Request!\n");
                        printf("  Brakes are handled automatically by the powercubes. Nothing to do.\n");
                }

		Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_ACTARRAY_REQ_BRAKES);
                handled=true;
	}


        if (handled) {
                return(0);
        } else {
                return(-1);
        }

}


int AmtecM5::HandleCommand(player_msghdr * hdr, void * data) {
	//puts("Asked to handle a command.");
	//printf(".\n");

        bool handled(false);

	//If the command matches any of the following, an action is taken. If not, -1 is returned
	if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_POS,device_addr)) {
		//All we get is a void pointer. Cast it to the right structure
		player_actarray_position_cmd_t * cmd = reinterpret_cast<player_actarray_position_cmd_t*>(data);


		int jointnumber=cmd->joint;
                //The command that we send to the module is: angleplayer/direction - offset
                //Converting the angle taking into account the offset and direction vectors:
                //angleplayer = (anglepowercube + offset) * direction
                //anglepowercube = angleplayer/direction - offset

		float newposition(0.0);

                //Do we have to limit the command to -180 and 180deg?
                if (normalize_angles[jointnumber]) {
                        newposition=normalize_angle(cmd->position/directions[jointnumber]-offsets[jointnumber]);
                } else {
                        newposition=cmd->position/directions[jointnumber]-offsets[jointnumber];
                }

                if (debug_level) {
                        cout << "pos_cmd. Joint: " << cmd->joint << " pos: " << cmd->position << "Sent to module: " << newposition << endl;
                }

                if (can && modsynchro) {
                        ModuleSyncMotion(0); //Deactivate the syncmotion bit
                }


		int error=pclDevice->moveRamp(idModuleList[jointnumber],newposition,rampSpeed[jointnumber],rampAccel[jointnumber]); // module, pos, speed, accel

		if (error!=0) {
			std::cout<< "Error in position_cmd command. Number: " << error << std::endl;
		}
                handled=true;

	} else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_SPEED,device_addr)) {
		//We received a speed-cmd

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_speed_cmd_t * cmd = reinterpret_cast<player_actarray_speed_cmd_t*>(data);

		int jointnumber=cmd->joint;
                //directions[i] is 1 or -1. Fixes the positive direction of rotation (or translation)
		float newspeed=cmd->speed/directions[jointnumber];

                if (debug_level) {
                        cout << "speed_cmd. Joint: " << cmd->joint << " pos: " << cmd->speed << "Sent to module: " << newspeed << endl;
                }

                if (can && modsynchro) {
                        ModuleSyncMotion(0); //Deactivate the syncmotion bit
                }

		int error=pclDevice->moveVel(idModuleList[jointnumber],newspeed);
		if (error!=0) {
			std::cout<< "Error in speed_cmd command. Number: " << error << std::endl;
		}

                handled=true;

	} else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_HOME,device_addr)) {
		//We received a home-cmd

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_home_cmd_t * cmd = reinterpret_cast<player_actarray_home_cmd_t*>(data);
		int joint=cmd->joint;

                if (debug_level) {
                        cout << "home_cmd. Joint: " << joint << " modID: " << idModuleList[joint] << endl;
                }

                if (can && modsynchro) {
                        ModuleSyncMotion(0); //Deactivate the syncmotion bit
                }

		if (joint == -1) {
			for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {
				int error=pclDevice->homeModule(idModuleList[i]);
                                if (error!=0) {
                                        std::cout<< "Error in Home command. Number: " << error << std::endl;
                                }

			}

		} else {
			int error=pclDevice->homeModule(idModuleList[joint]);
              		if (error!=0) {
                                std::cout<< "Error in Home command. Number: " << error << std::endl;
                        }

		}
                handled=true;

	} else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_CURRENT,device_addr)) {
		//We received a current-cmd

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_current_cmd_t * cmd = reinterpret_cast<player_actarray_current_cmd_t*>(data);
		int jointnumber=cmd->joint;
                //switch the direction of the current too.
		float current=cmd->current/directions[jointnumber];

                if (can && modsynchro) {
                        ModuleSyncMotion(0); //Deactivate the syncmotion bit
                }

		//printf("CMD: Joint: %d  ModID: %d  Current: %3f.\n",jointnumber,idModuleList[jointnumber],current);
                if (debug_level) {
                        cout << "current_cmd. Joint: " << jointnumber << " modID: " << idModuleList[jointnumber] << " current: " << current << endl;
                }

		int error=pclDevice->moveCur(idModuleList[jointnumber],current);
      		if (error!=0) {
			std::cout<< "Error in current command. Number: " << error << std::endl;
		}

                handled=true;

	}  else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_MULTI_CURRENT,device_addr)) {
		//We received a current-cmd for several joints

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_multi_current_cmd_t * cmd = reinterpret_cast<player_actarray_multi_current_cmd_t*>(data);


                if (debug_level) {
                        cout << "multi_current_cmd" << endl;
                }

                if ((cmd->currents_count == idModuleList.size()) || (cmd->currents_count == PLAYER_ACTARRAY_NUM_ACTUATORS)) {


                        if (can && !modsynchro) {
                                ModuleSyncMotion(1); //activate the syncmotion bit
                        }

                        for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {
                                pclDevice->moveCur(idModuleList[i],cmd->currents[i]/directions[i]);
                        }


                       if (can) {
                                pclDevice->startMotionAll(); //Tells the modules to start moving now (All together)
                        }

                        handled=true;

                } else {
                        cout << "WARNING: multi_current_cmd: The # of current commands must equal the # of modules or to the maximum # of modules (PLAYER_ACTARRAY_NUM_ACTUATORS). Ignoring command.\n";
                }

	}  else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_MULTI_POS,device_addr)) {
		//We received a position-cmd for several joints

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_multi_position_cmd_t * cmd = reinterpret_cast<player_actarray_multi_position_cmd_t*>(data);

                if (debug_level) {
                        cout << "multi_pos_cmd" << endl;
                }

                if ( (cmd->positions_count == idModuleList.size()) || (cmd->positions_count == PLAYER_ACTARRAY_NUM_ACTUATORS)) {

                        if (can && !modsynchro) {
                                ModuleSyncMotion(1); //activate the syncmotion bit
                        }


                        //transmit the commands to the modules. if modsynchro=1, they wait until they get StartMotionAll to move
                        for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {


                                float newposition(0.0);


                                if (normalize_angles[i]) {
                                        newposition=normalize_angle(cmd->positions[i]/directions[i]-offsets[i]);
                                } else {
                                        newposition=cmd->positions[i]/directions[i]-offsets[i];
                                }


                                int error=pclDevice->moveRamp(idModuleList[i],newposition,rampSpeed[i],rampAccel[i]); // module, pos, speed, accel
                                if (error != 0) {
                                        cout << "Error in Multi-position cmd. Module:" << idModuleList[i] << ".\n";
                                }
                        }

                        if (can) {
                                pclDevice->startMotionAll(); //Tells the modules to start moving now (All together)
                        }


                        handled=true;

                } else {
                        cout << "WARNING: multi_pos_cmd: The # of position commands must equal the # of modules, or to the maximum # of modules (PLAYER_ACTARRAY_NUM_ACTUATORS. Ignoring command.\n";

                }


	}   else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_ACTARRAY_CMD_MULTI_SPEED,device_addr)) {
		//We received a position-cmd for several joints

		//All we get is a void pointer. Cast it to the right structure
		player_actarray_multi_speed_cmd_t * cmd = reinterpret_cast<player_actarray_multi_speed_cmd_t*>(data);


                if (debug_level) {
                        cout << "multi_speed_cmd" << endl;
                }

                if ((cmd->speeds_count == idModuleList.size()) || (cmd->speeds_count == PLAYER_ACTARRAY_NUM_ACTUATORS)) {

                        if (can && !modsynchro) {
                                ModuleSyncMotion(1); //activate the syncmotion bit
                        }

                        //transmit the commands to the modules. if modsynchro=1, they wait until they get StartMotionAll to move
                        for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {
                                float newspeed=cmd->speeds[i]/directions[i];
                                int error=pclDevice->moveVel(idModuleList[i],newspeed);
                                if (error != 0) {
                                        cout << "Error in Multi-speed cmd. Module:" << idModuleList[i] << ".\n";
                                }

                        }

                        if (can) {
                                pclDevice->startMotionAll(); //Tells the modules to start moving now (All together)
                        }

                        handled=true;

                } else {
                        cout << "WARNING: multi_speed_cmd: The # of speed commands must equal the # of modules, or to the maximum number of modules (PLAYER_ACTARRAY_NUM_ACTUATORS). Ignoring command.\n";

                }


	}


        if (handled) {
                return(0);
        } else {
                return(-1);
        }

}



// This function limits the angles to -180deg and 180deg. For example, if the player command is
// 350deg, this function returns -10deg. This is the logical and safe thing to do for
// the majority of the joints. Only joints that can rotate indefinitely in a safe way
// could want to have the behaviour without limits (set 0 in the normalize_angles vector in the config file)
float AmtecM5::normalize_angle(float angle) {

        return (atan2( sin(angle), cos(angle) ) );
}

int AmtecM5::ModuleSyncMotion(int state) {

        if (can) {

                if (state==1) {
                        //Activate the synchro bits in all modules
                        for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {
                                unsigned long config(0);
                                pclDevice->getConfig(idModuleList[i],&config);
                                config |= CONFIGID_MOD_SYNC_MOTION;
                                pclDevice->setConfig(idModuleList[i],config);

                                pclDevice->getConfig(idModuleList[i],&config);
                                if (config & CONFIGID_MOD_SYNC_MOTION) {
                                        printf("Sync-motion activated for module: %d\n",idModuleList[i]);
                                }
                        }
                        modsynchro=true;

                } else {
                        //deactivate the synchro bits on all modules
                        for (unsigned int i=0 ; i != idModuleList.size() ; ++i ) {
                                unsigned long config(0);
                                pclDevice->getConfig(idModuleList[i],&config);
                                config &= ~(CONFIGID_MOD_SYNC_MOTION); // ~ is the binary NOT
                                pclDevice->setConfig(idModuleList[i],config);

                                pclDevice->getConfig(idModuleList[i],&config);
                                if (!(config & CONFIGID_MOD_SYNC_MOTION)) {
                                        printf("Sync-motion de-activated for module: %d\n",idModuleList[i]);
                                }
                        }
                        modsynchro=false;

                }
        }
        return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void AmtecM5::Main() {
	// The main loop; interact with the device here


	// Prepare everything for soft-realtime

	//Need two timers: one for calculating the sleep time to keep a desired framerate.
	// The other for measuring the real elapsed time. (And maybe give an alarm)
	struct timeval tv_framerate_start;
	struct timeval tv_framerate_end;
	struct timeval tv_realtime_start;
	struct timeval tv_realtime_end;

	gettimeofday( &tv_framerate_start, NULL );  // NULL -> don't want timezone information
	tv_realtime_start = tv_framerate_start;



	// The main loop of the driver, where we read from the Powercubes and publish the information to the clients
	while(true) {


		//find out the real elapsed time.
		gettimeofday( &tv_realtime_end, NULL );
		//calculate the time in mS
		float real_elapsed=timediffms(tv_realtime_start,tv_realtime_end);
		//restart the timer
		gettimeofday( &tv_realtime_start, NULL );

		//check if the time was too long
		static bool gavewarning(false);
		if ((!gavewarning) && (real_elapsed > alarmtime)) {
			cout << "WARNING:\n";
			cout << "Cycle took " << real_elapsed <<"mS instead of the desired " << samplingrate << "mS (Alarm at " <<alarmtime << "mS)" <<endl;
			gavewarning=true;
		}

		//cout << real_elapsed << "mS\n";

		// Do the real work of the cycle:
		// test if we are supposed to cancel
		pthread_testcancel();



		// Interact with the device, and push out the resulting data, using
		// Driver::Publish()
		// Copy the data.
		player_actarray_data_t data;
		data.actuators = actuators;

		// The number of actuators in the array.
		data.actuators_count=module_count;

		//Report if the motors are all halted, or ready
		data.motor_state=motor_state; // 0 or 1


                //Memory to record the old positions (for velocity and accel calculation)
                static float oldpos[PLAYER_ACTARRAY_NUM_ACTUATORS];
                static float oldspeed[PLAYER_ACTARRAY_NUM_ACTUATORS];
                static bool init_oldposspeed(false);
                static bool valid_deltapos(false); // true after the second run. (Valid velocity calculation)
                static bool valid_deltaspeed(false); // true after the third run. (Valid accel calculation)

                if (!init_oldposspeed) {
                        //Initialize with zeros the first time.
                        memset(&oldpos,0,sizeof(oldpos));
                        memset(&oldspeed,0,sizeof(oldspeed));
                        init_oldposspeed=true;
                }


                //Temporary variables to hold the data that we read/calculate
                float pos[module_count];
                float speed[module_count];
                float accel[module_count];
                float current[module_count];
                unsigned long state[module_count];
                uint8_t report_state[module_count];

                //Zero those arrays
                memset(&pos,0,sizeof(pos));
                memset(&speed,0,sizeof(speed));
                memset(&accel,0,sizeof(accel));
                memset(&current,0,sizeof(current));
                memset(&report_state,0,sizeof(report_state));



                //First read position (The most time critical)
                if (can) {
                        pclDevice->savePosAll(); //Broadcast to save positions at the same time
                        //Now read the currents (we get a snapshot of the currents as close as we can to the positions)
                        for (unsigned int i=0; i<module_count ; ++i ) {
               			pclDevice->getCur(idModuleList[i],&(current[i]));
                        }

                        //Now read the saved positions
                        for (unsigned int i=0; i<module_count ; ++i ) {
                                pclDevice->getSavePos(idModuleList[i],&(pos[i]));  //read synchronised position from the powercube
                        }

                } else {
                        //Iterate and read the positions
                        for (unsigned int i=0; i<module_count ; ++i ) {
                                pclDevice->getPos(idModuleList[i],&(pos[i]));  //read position from the powercube
                        }
                        //Now read the currents
                        for (unsigned int i=0; i<module_count ; ++i ) {
               			pclDevice->getCur(idModuleList[i],&(current[i]));
                                //pclDevice->getVel(idModuleList[i],&(speed[i])); //read speed from the powercube

                        }


                }

                //This is not time critical anymore. Read the state
                //Also calculate velocity and acceleration
                //Note that deltapos is only valid after the first cycle, and deltavel after the second
                for (unsigned int i=0; i != module_count ; ++i ) {
                        pclDevice->getModuleState(idModuleList[i],&(state[i]));

                        float deltapos=pos[i]-oldpos[i];
                        if (valid_deltapos) {
                                speed[i]=deltapos/(real_elapsed/1000.0); //real_elapsed is in mS

                        } else {
                                speed[i]=0.0;
                        }

                        float deltaspeed=speed[i]-oldspeed[i];
                        if (valid_deltaspeed) {
                                accel[i]=deltaspeed/(real_elapsed/1000.0);
                        } else {
                                accel[i]=0.0;
                        }

                        //After the first go, deltapos will be valid. After the second, delta_speed also.
                        //Mark them as valid only after one complete cycle through all the degrees of freedom ( i+1 == module_count)
                        if ((!valid_deltapos) && (i+1 == module_count)) {
                                //comes here the first time
                                valid_deltapos=true;
                        } else if ((!valid_deltaspeed) && (i+1 == module_count)) {
                                //comes here the second time
                                valid_deltaspeed=true;
                        }

                        //save for the next cycle
                        oldpos[i]=pos[i];
                        oldspeed[i]=speed[i];

                }


                //Prepare the data for publication
		for (unsigned int i=0; i<module_count ; ++i ) {

			// The status bits of the Powercubes are very rich. This encodes very badly in just one
			// Status value, as defined by the current actarray interface.

			//   ACTARRAY										POWERCUBE bits in status_long_int
			///** Idle state code */
			//#define PLAYER_ACTARRAY_ACTSTATE_IDLE     1		!STATEID_MOD_ERROR
			///** Moving state code */
			//#define PLAYER_ACTARRAY_ACTSTATE_MOVING   2 		STATEID_MOD_MOTION
			///** Braked state code */
			//#define PLAYER_ACTARRAY_ACTSTATE_BRAKED   3		STATEID_MOD_BRAKEACTIVE
			///** Stalled state code */
			//#define PLAYER_ACTARRAY_ACTSTATE_STALLED  4		STATEID_MOD_HALT


			if ( (state[i] & STATEID_MOD_HALT) || (state[i] & STATEID_MOD_ERROR) ) {
				report_state[i]=PLAYER_ACTARRAY_ACTSTATE_STALLED;
				printf("ModuleId: %d  BusID: %d  Error word: 0x%0lx\n",i,idModuleList[i],state[i]);
			} else if (state[i] & STATEID_MOD_BRAKEACTIVE) {
				report_state[i]=PLAYER_ACTARRAY_ACTSTATE_BRAKED;
			} else if (state[i] & STATEID_MOD_MOTION) {
				report_state[i]=PLAYER_ACTARRAY_ACTSTATE_MOVING;
			} else {
				report_state[i]=PLAYER_ACTARRAY_ACTSTATE_IDLE;
			}

                        //Converting the angle taking into account the offset and direction vectors:
                        //angleplayer = (anglepowercube + offset) * direction
                        //anglepowercube = angleplayer/direction - offset

			data.actuators[i].position=(pos[i]+offsets[i])*directions[i];
			data.actuators[i].speed=speed[i]*directions[i];
			data.actuators[i].acceleration=accel[i]*directions[i];
			data.actuators[i].current=current[i]*directions[i];
			data.actuators[i].state=report_state[i];
		}
                //Data is now ready for publication.

		// Process incoming messages.  AmtecM5::ProcessMessage() is
		// called on each message. (We do this after reading data, to avoid jitter on the acquisition rate)
		ProcessMessages();


		//Tell the watchdog we are still alive:
		if (can) {
			pclDevice->serveWatchdogAll();
		}

		//Broadcast our data
		Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_ACTARRAY_DATA_STATE, (unsigned char*)&data, sizeof(player_actarray_data_t),NULL);


		//point to calculate how much to sleep, call nanosleep, after sleep restart the timer
		//Get the ammount of time passed:
		gettimeofday( &tv_framerate_end, NULL );
		// figure out how much to sleep
		long usecs    = tv_framerate_end.tv_usec - tv_framerate_start.tv_usec;
		long secs     = tv_framerate_end.tv_sec  - tv_framerate_start.tv_sec;
		long elapsed_usecs = 1000000*secs  + usecs;

		long us_tosleep = static_cast<long>(samplingrate*1000) - elapsed_usecs;
		//cout << "usec to sleep: " << us_tosleep << endl;

		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = us_tosleep*1000;
		int done=nanosleep(&ts, NULL);

		//restart the counter
		gettimeofday( &tv_framerate_start, NULL );

		if (done != 0) {
			cout << "Error in nanosleep! ERRNO: " << errno << " ";
			if (errno == EINTR) {
				cout << "EINTR" ;
			} else if (errno == EINVAL) {
				cout << "EINVAL" ;
			}
			cout << endl;

		}


	}
}

