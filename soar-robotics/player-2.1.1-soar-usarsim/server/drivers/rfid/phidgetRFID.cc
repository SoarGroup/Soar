/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007 Federico Ruiz   ruizf /at/ cs.tum.edu
 *  Small changes by Alexis Maldonado  maldonad /at/ cs.tum.edu
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

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_phidgetRFID phidgetRFID
 * @brief Phidget RFID reader

The phidgetRFID driver communicates with the PhidgetRFID (Part# 1023) reader. (125 kHz Read-only). It does not support anti-collision, but it is inexpensive, small and USB Powered. 

@par Compile-time dependencies

- none

@par Provides

- @ref interface_rfid

@par Requires

- libphidget from www.phidgets.com should be installed.

@par Configuration requests

- none

@par Configuration file options

- serial (integer)
  - Default: -1
  - This defines which phidget will be controlled if there is more than one connected to the USB bus. 
    You can obtain the number with lsusb, like this:  "lsusb -v |grep iSerial".
    The default is -1 , and it will connect to the first phidget available.

- sampling_rate (integer)
  - Default: 40
  - How often (in mS) should the phidget produce data. 40mS produces RFID data at a rate of 25Hz.

- alarmtime (integer)
  - Default: 45
  - If the data acquisition cycle takes longer than this time (in mS), a warning will be printed.

- provides
  - The driver supports the "speech" interface for printing data to the LCD of the Interface kits that have it.
  - An "aio" interface gives information about the analog sensors connected to the Interface Kit.
  - The "dio" interface controls the digital inputs and outputs present.
  
@par Example 

@verbatim
driver
(
  name "phidgetIFK"
  provides ["rfid:0" "dio:0"]
  serial -1
  alwayson 1
  samplingrate 40
  alarmtime 45
)
@endverbatim

@author Federico Ruiz & Alexis Maldonado

 */
/** @} */



#include "phidget21.h"
#include <libplayercore/playercore.h>

#include <unistd.h>
#include <string.h>
#include <iostream>

//For nanosleep:
#include <time.h>
#include <sys/time.h>
//To catch the errors of nanosleep
#include <errno.h>

struct tagControlS {
  int tagPresent;               //This variable changes to 1 if a tag is in front of the reader.
  CPhidgetRFIDHandle rfid_present;
};

struct tagControlS tagControl;

//This function returns the difference in mS between two timeval structures
inline float timediffms(struct timeval start, struct timeval end) {
    return(end.tv_sec*1000.0 + end.tv_usec/1000.0 - (start.tv_sec*1000.0 + start.tv_usec/1000.0));
}

int TagLost(CPhidgetRFIDHandle rfid,void *hola, unsigned char *usrchar);
int TagFound(CPhidgetRFIDHandle rfid,void *hola, unsigned char *usrchar);




class Phidgetrfid : public Driver {
	public:

    // Constructor;
		Phidgetrfid(ConfigFile* cf, int section);

    //Destructor
		~Phidgetrfid();

		virtual int Setup();
		virtual int Shutdown();

		virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);

	private:

    // Main function for device thread.
		virtual void Main();

		//!Time between samples (in mS)
		float samplingrate;
		//!Alarm time (mS)
		float alarmtime;


		//! Pointer to the RFID Phidget Handle
		CPhidgetRFIDHandle rfid;

	    //! Player Interfaces
		player_devaddr_t rfid_id;
		player_devaddr_t dio_id;

		//!Serial number of the phidget
		int serial;
};


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Phidgetrfid::Phidgetrfid(ConfigFile* cf, int section)
        : Driver(cf, section) {
    //! Start with a clean device
    memset(&rfid_id,0,sizeof(player_devaddr_t));
    memset(&dio_id,0,sizeof(player_devaddr_t));

    // Creating the rfid interface
    if (cf->ReadDeviceAddr(&(rfid_id), section, "provides", PLAYER_RFID_CODE, -1, NULL) == 0) {
        if (AddInterface(rfid_id) != 0) {
            SetError(-1);
            return;
        }
    } else {
        PLAYER_WARN("rfid interface not created for phidgetRFID driver");
    }
    if (cf->ReadDeviceAddr(&(dio_id), section, "provides", PLAYER_DIO_CODE, -1, NULL) == 0) {
        if (AddInterface(dio_id) != 0) {
            SetError(-1);
            return;
        }
    } else {
        PLAYER_WARN("dio interface not created for phidgetrfid driver");
    }


    // Set the phidgetrfid pointer to NULL
    rfid=0;
    tagControl.tagPresent=0;
    // Add more code here.

    // Read an option from the configuration file
    serial = cf->ReadInt(section, "serial", -1);

    //Sampling rate and alarm time in mS
    samplingrate = cf->ReadFloat(section, "samplingrate", 40.0);
    alarmtime = cf->ReadFloat(section, "alarmtime", 45.0);

    return;
}

Phidgetrfid::~Phidgetrfid() {}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Phidgetrfid::Setup() {
    PLAYER_MSG0(1,"PhidgetRFID driver initialising");

    //Use the Phidgets library to communicate with the devices
    CPhidgetRFID_create(&rfid);
    CPhidget_open((CPhidgetHandle)rfid,serial);

    PLAYER_MSG0(1,"Waiting for Attachment.");

    int status(-1);

    //Wait for attachment 1s or aborts.
    status=CPhidget_waitForAttachment((CPhidgetHandle)rfid, 1000);

    if (status != 0) {
        PLAYER_ERROR("There was a problem connecting to the PhidgetRFID.");
        return(1);
    } else {
        PLAYER_MSG0(1,"Connection granted to the PhidgetRFID Reader.");
    }

    CPhidgetRFID_set_OnTagLost_Handler(rfid,TagLost,NULL);
    CPhidgetRFID_set_OnTag_Handler(rfid,TagFound,NULL);
    //Turning on the Antena.
    CPhidgetRFID_setAntennaOn(rfid,1);
    CPhidgetRFID_setLEDOn(rfid,1);

    PLAYER_MSG0(1,"PhidgetRFID driver ready");

    // Start the device thread; spawns a new thread and executes
    // Phidgetrfid::Main(), which contains the main loop for the driver.
    StartThread();

    return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Phidgetrfid::Shutdown() {
	PLAYER_MSG0(1,"Shutting PhidgetRFID driver down");

    // Stop and join the driver thread
    StopThread();

    // Turn of the device and delete the Phidget objects
    CPhidgetRFID_setAntennaOn(rfid,0);
    CPhidget_close((CPhidgetHandle)rfid);
    CPhidget_delete((CPhidgetHandle)rfid);
    rfid=0;

    PLAYER_MSG0(1,"PhidgetRFID driver has been shutdown");

    return(0);
}

int Phidgetrfid::ProcessMessage(QueuePointer &resp_queue,
                                      player_msghdr * hdr,
                                      void * data) {

    if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_DIO_CMD_VALUES,dio_id)) {
        player_dio_cmd_t * cmd = reinterpret_cast<player_dio_cmd_t*>(data);
        int count=cmd->count;
        int do_values=cmd->digout;

        const unsigned int max_do=32;
        static bool old_do_values[max_do];
        static bool init_done(false);

        //the actual ammount of digital outputs of the widget
        int phidget_num_outputs(0);
        CPhidgetRFID_getNumOutputs(rfid, &phidget_num_outputs);
        printf("Num of outputs: %d\n", phidget_num_outputs);

        if (count > static_cast<int>(max_do)) {
            PLAYER_WARN("PhidgetRFID: Received a command with a huge ammount of digital outputs. Check the value of count.\n");
            PLAYER_WARN1("PhidgetRFID: Limiting to the maximum possible value: %d\n",max_do);
            count = max_do;
        }

        if (!init_done) {
            for (unsigned int i=0; i!=max_do; ++i) {
                old_do_values[i]=false;
            }
            init_done=true;
        }

        bool new_do_values[max_do];
        for (unsigned int i=0 ; i != max_do ; ++i) {
            new_do_values[i]=false;
        }

        //separate the bitfield into bools
        for (int i=0 ; i != count ; i++) {
            //Extract the values of the single bits
            bool dix=(do_values & ( 1<<i ) )? true : false ;
            new_do_values[i]=dix;

        }


        //see if the status changed
        for (int i=0 ; i != phidget_num_outputs ; ++i) {
            if (new_do_values[i] != old_do_values[i]) {
                //store value
                old_do_values[i]=new_do_values[i];

                //send command to the phidget
                CPhidgetRFID_setOutputState (rfid, i, static_cast<int>(new_do_values[i]));
                //std::cout << "DO" << i << " -> " << new_do_values[i] << "\n";
            }
        }
        return(0);  //send the ACK





    }

    return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Phidgetrfid::Main() {

    //Need two timers: one for calculating the sleep time to keep a desired framerate.
    // The other for measuring the real elapsed time. (And maybe give an alarm)
    struct timeval tv_framerate_start;
    struct timeval tv_framerate_end;
    struct timeval tv_realtime_start;
    struct timeval tv_realtime_end;

    gettimeofday( &tv_framerate_start, NULL );  // NULL -> don't want timezone information
    tv_realtime_start = tv_framerate_start;

    int tagPresent=0;
    // The main loop; interact with the device here
    while (true)  {

        //find out the real elapsed time
        gettimeofday( &tv_realtime_end, NULL );
        //calculate the time in mS
        float real_elapsed=timediffms(tv_realtime_start,tv_realtime_end);
        //restart the timer
        gettimeofday( &tv_realtime_start, NULL );

        //check if the time was too long
        static bool gavewarning(false);
        if ((!gavewarning) && (real_elapsed > alarmtime)) {
		PLAYER_WARN2("Cycle took %d mS instead of the desired %d mS. (Only warning once)\n",real_elapsed , samplingrate);
     	gavewarning=true;
        }
        //std::cout << real_elapsed << "mS\n";


        // test if we are supposed to cancel
        pthread_testcancel();

        // Process incoming messages.  Phidgetrfid::ProcessMessage() is
        // called on each message.
        ProcessMessages();



        unsigned char tag[20];
        CPhidgetRFID_getLastTag(rfid,tag);
        int ledstate;
        player_rfid_data_t data_rfid;
        data_rfid.tags = new player_rfid_tag_t[1];
        data_rfid.tags[0].guid = new char[8];
        if (tagControl.rfid_present==rfid) {
              tagPresent=tagControl.tagPresent;
        }
        if (tagPresent!=0) {
            //printf("Tag: %x %x %x %x %x %x\n",tag[0], tag[1], tag[2], tag[3], tag[4], tag[5]);
            data_rfid.tags_count=1;
            data_rfid.tags[0].type=1;
            data_rfid.tags[0].guid_count=8;
            for (unsigned int i=0;i<5;i++) {
                data_rfid.tags[0].guid[i]=tag[i];
            }
            data_rfid.tags[0].guid[5]=0;
            data_rfid.tags[0].guid[6]=0;
            data_rfid.tags[0].guid[7]=0;
            //toggle the LED
            CPhidgetRFID_getLEDOn(rfid,&ledstate);
            if (ledstate==0) {
                CPhidgetRFID_setLEDOn(rfid,1);
            } else {
                CPhidgetRFID_setLEDOn(rfid,0);
            }
        } else {
            data_rfid.tags_count=0;
            data_rfid.tags[0].type=0;
            data_rfid.tags[0].guid_count=0;
            for (unsigned int i=0;i<8;i++) {
                data_rfid.tags[0].guid[i]=0;
            }
            CPhidgetRFID_setLEDOn(rfid,0);
        }


        //Publishing data.
        if (rfid_id.interf !=0) {
            Publish(rfid_id, PLAYER_MSGTYPE_DATA, PLAYER_RFID_DATA_TAGS, (unsigned char*)&data_rfid, sizeof(player_rfid_data_t), NULL);
        }
        delete [] data_rfid.tags[0].guid;
        delete [] data_rfid.tags;

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
            std::cout << "Error in nanosleep! ERRNO: " << errno << " ";
            if (errno == EINTR) {
                std::cout << "EINTR" ;
            } else if (errno == EINVAL) {
                std::cout << "EINVAL" ;
            }
            std::cout << std::endl;
        }

    }
}

//Handler functions to check if there is a new tag there. They are handlers as seen on the Phidget library.
int TagLost(CPhidgetRFIDHandle rfid,void *dummy, unsigned char *usrchar) {
    tagControl.rfid_present=rfid;
    tagControl.tagPresent=0;
    return (0);
}

int TagFound(CPhidgetRFIDHandle rfid,void *dummy, unsigned char *usrchar) {
    tagControl.rfid_present=rfid;
    tagControl.tagPresent=1;
    return (0);
}

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
PhidgetRFID_Init(ConfigFile* cf, int section) {
    // Create and return a new instance of this driver
	return((Driver*)(new Phidgetrfid(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void PhidgetRFID_Register(DriverTable* table) {
	table->AddDriver("phidgetRFID", PhidgetRFID_Init);
}

