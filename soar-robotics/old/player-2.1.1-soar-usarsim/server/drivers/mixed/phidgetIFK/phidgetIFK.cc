/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007 Alexis Maldonado and Federico Ruiz
 *                     maldonad and ruizf \/at/\ cs.tum.edu
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
/** @defgroup driver_phidgetIFK phidgetIFK
 * @brief Phidget InterfaceKit driver

The phidgetIFK driver communicates with the all the Phidget Interface Kits: IFK 8/8/8 , IFK 0/16/16 , Circular and Linear Touch IFKs, and LCD IFK.
The ammount of digital and analog inputs is adjusted automatically for each device (queried after the initial connection).

@par Compile-time dependencies

- none

@par Provides

- @ref interface_aio
- @ref interface_dio
- @ref interface_speech

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
  - Default: 20
  - How often (in mS) should the phidget produce data. Reading at 17mS produces data at a rate of ~ 59Hz.

- alarmtime (integer)
  - Default: 25
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
  provides ["aio:0" "dio:0" "speech:0"]
  #provides ["aio:0" "dio:0"]
  serial -1
  alwayson 1
  samplingrate 17
  alarmtime 21
)
@endverbatim

@author Alexis Maldonado

 */
/** @} */



#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>

#include "phidget21.h"

#include <libplayercore/playercore.h>


//For nanosleep:
#include <time.h>
#include <sys/time.h>
//To catch the errors of nanosleep
#include <errno.h>


//This function returns the difference in mS between two timeval structures
inline float timediffms(struct timeval start, struct timeval end) {
    return(end.tv_sec*1000.0 + end.tv_usec/1000.0 - (start.tv_sec*1000.0 + start.tv_usec/1000.0));
}



class PhidgetIFK : public Driver {
public:

    // Constructor; need that
    PhidgetIFK(ConfigFile* cf, int section);

    // Destructor
    ~PhidgetIFK();

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

    //! Pointer to the Phidget Handle
    CPhidgetInterfaceKitHandle ifk;
    CPhidgetTextLCDHandle lcd;

    //Sampling rate and alarm time
    float samplingrate;
    float alarmtime;

    //! Player Interfaces
    player_devaddr_t aio_id;
    player_devaddr_t dio_id;
    player_devaddr_t speech_id;

    bool lcd_present;
    int phidget_serial;
};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
PhidgetIFK_Init(ConfigFile* cf, int section) {
    // Create and return a new instance of this driver
    return((Driver*)(new PhidgetIFK(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void PhidgetIFK_Register(DriverTable* table) {
    table->AddDriver("phidgetIFK", PhidgetIFK_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
PhidgetIFK::PhidgetIFK(ConfigFile* cf, int section)
        : Driver(cf, section) {

    memset(&aio_id, 0, sizeof(player_devaddr_t));
    memset(&dio_id, 0, sizeof(player_devaddr_t));
    memset(&speech_id, 0, sizeof(player_devaddr_t));

    lcd_present=false;


    // Do we create an aio interface?
    if (cf->ReadDeviceAddr(&(aio_id), section, "provides",
                           PLAYER_AIO_CODE, -1, NULL) == 0) {
        if (AddInterface(aio_id) != 0) {
            SetError(-1);
            return;
        }
    } else {
        PLAYER_WARN("aio interface not created for phidgetifk driver");
    }

    // Do we create a dio interface?
    if (cf->ReadDeviceAddr(&(dio_id), section, "provides",
                           PLAYER_DIO_CODE, -1, NULL) == 0) {
        if (AddInterface(dio_id) != 0) {
            SetError(-1);
            return;
        }
    } else {
        PLAYER_WARN("dio interface not created for phidgetifk driver");
    }

    // Do we create a speech interface for the LCD?
    if (cf->ReadDeviceAddr(&(speech_id), section, "provides",
                           PLAYER_SPEECH_CODE, -1, NULL) == 0) {
        if (AddInterface(speech_id) != 0) {
            SetError(-1);
            return;
        }
        lcd_present=true;
    } else {
        PLAYER_WARN("speech (LCD) interface not created for phidgetifk driver");
    }



// Read an option from the configuration file
    phidget_serial = cf->ReadInt(section, "serial", -1);

    //Sampling rate and alarm time in mS
    samplingrate = cf->ReadFloat(section, "samplingrate", 20.0);
    alarmtime = cf->ReadFloat(section, "alarmtime", 25.0);


    //set the pointer to NULL
    ifk=0;

    return;
}

//Destructor
PhidgetIFK::~PhidgetIFK() {}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int PhidgetIFK::Setup() {
    PLAYER_MSG0(1,"PhidgetIFK driver initialising");

    // Here you do whatever is necessary to setup the device, like open and
    // configure a serial port.

    CPhidgetInterfaceKit_create(&ifk);
    CPhidget_open((CPhidgetHandle)ifk, phidget_serial);

    if (lcd_present) {
        CPhidgetTextLCD_create(&lcd);
        CPhidget_open((CPhidgetHandle)lcd,phidget_serial);
    }

    using namespace std;

    PLAYER_MSG0 (1,"Wait for Attachment.\n");

    int status(-1);
    status=CPhidget_waitForAttachment((CPhidgetHandle)ifk, 1000); //Wait for 1s
    if (lcd_present) {
        status+=CPhidget_waitForAttachment((CPhidgetHandle)lcd, 1000); //Wait for 1s
    }

    if (status != 0) {
        PLAYER_ERROR("Could not connect to the Phidgets. Aborting.\n");
        return(1);

    } else {
        PLAYER_MSG0(1,"Connected correctly to the Phidget Interface Kit.\n");
    }


    //Make sure that the LCD has the backlight on, and the cursor is invisible
    if (lcd_present) {
        CPhidgetTextLCD_setBacklight(lcd,1);
        CPhidgetTextLCD_setCursorOn(lcd,0);

    }

    PLAYER_MSG0(1,"PhidgetIFK driver ready");

    // Start the device thread; spawns a new thread and executes
    // PhidgetIFK::Main(), which contains the main loop for the driver.
    StartThread();

    return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int PhidgetIFK::Shutdown() {
    PLAYER_MSG0(1,"Shutting PhidgetIFK driver down");

    // Stop and join the driver thread
    StopThread();

    usleep(100000);
    CPhidget_close((CPhidgetHandle)ifk);
    CPhidget_delete((CPhidgetHandle)ifk);
    ifk=0;

    if (lcd_present) {
        //std::cout << "About to close the LCD handle.\n";
        //CPhidget_close((CPhidgetHandle)&lcd);   //It hangs here.  //for now, we don't clean up (Bug in libphidget21)
        //CPhidget_delete((CPhidgetHandle)&lcd);
        lcd=0;
    }


    PLAYER_MSG0(1,"PhidgetIFK driver has been shutdown");

    return(0);
}

int PhidgetIFK::ProcessMessage(QueuePointer &resp_queue,
                                     player_msghdr * hdr,
                                     void * data) {
    // Process messages here.  Send a response if necessary, using Publish().
    // If you handle the message successfully, return 0.  Otherwise,
    // return -1, and a NACK will be sent for you, if a response is required.

    if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_DIO_CMD_VALUES, dio_id)) {
        //All we get is a void pointer. Cast it to the right structure
        player_dio_cmd_t * cmd = reinterpret_cast<player_dio_cmd_t*>(data);

        int count=cmd->count;
        int do_values=cmd->digout;

        const unsigned int max_do=32; // For now, player stores this values in a uint32.
        static bool old_do_values[max_do];
        static bool init_done(false);

        //the actual ammount of digital outputs of the widget
        int phidget_num_outputs(0);
        CPhidgetInterfaceKit_getNumOutputs(ifk, &phidget_num_outputs);

        if (count > static_cast<int>(max_do)) {
            PLAYER_WARN("PhidgetIFK: Received a command with a huge ammount of digital outputs. Check the value of count.\n");
            PLAYER_WARN1("PhidgetIFK: Limiting to the maximum possible value: %d\n",max_do);
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

                //send order to the phidget
                CPhidgetInterfaceKit_setOutputState (ifk, i, static_cast<int>(new_do_values[i]));
                //std::cout << "DO" << i << " -> " << new_do_values[i] << "\n";
            }
        }
        return(0);  //send the ACK


    }

    if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_SPEECH_CMD_SAY, speech_id)) {
        //All we get is a void pointer. Cast it to the right structure
        player_speech_cmd_t * cmd = reinterpret_cast<player_speech_cmd_t*>(data);

        using namespace std;

        //Get the size of the LCD screen
        int numcolumns(0);
        CPhidgetTextLCD_getNumColumns(lcd,&numcolumns);
        int numrows(0);
        CPhidgetTextLCD_getNumRows(lcd,&numrows);

        //Copy the text to a string for easier manipulation
        string completemessage;
        completemessage = const_cast<const char *>(cmd->string);

        //vector to hold each line on the LCD (with the correct length)
        vector<string> lines_to_print;

        //check the size of the message: Fits on one line, or on several?
        if (cmd->string_count > static_cast<unsigned int>(numcolumns) ) {
            //Need to cut the string into several rows
            for (unsigned int i=0 ; completemessage.size() > i*numcolumns; ++i) {
                string line=completemessage.substr(i*numcolumns,numcolumns);  // Get a piece of the string with the length of numcolumns
                lines_to_print.push_back(line);
            }

        } else {
            //it all fits in one row, the rest should be empty
            string line=completemessage;
            lines_to_print.push_back(line);
        }

        //Rest of empty lines, if they are necessary
        for (unsigned int i=lines_to_print.size() ; i < static_cast<unsigned int>(numrows) ; ++i) {
            string line("");
            lines_to_print.push_back(line);
        }


        if (lcd_present) {
            //std::cout << "SAY: " << txtdata << "\n";
            for (unsigned int i=0; i != lines_to_print.size() ; ++i) {
                //cout << "Printing: " << lines_to_print[i] << "\n";
                CPhidgetTextLCD_setDisplayString(lcd,i,const_cast<char*>(lines_to_print[i].c_str()) );
            }

        }
        return(0); // send the ACK

    }

    return(-1); //return that the message wasn't handled
}



////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void PhidgetIFK::Main() {

    //Need two timers: one for calculating the sleep time to keep a desired framerate.
    // The other for measuring the real elapsed time. (And maybe give an alarm)
    struct timeval tv_framerate_start;
    struct timeval tv_framerate_end;
    struct timeval tv_realtime_start;
    struct timeval tv_realtime_end;

    gettimeofday( &tv_framerate_start, NULL );  // NULL -> don't want timezone information
    tv_realtime_start = tv_framerate_start;

    // The main loop; interact with the device here
    while (true) {

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

        // Process incoming messages.  PhidgetIFK::ProcessMessage() is
        // called on each message.
        ProcessMessages();

        // Interact with the device, and push out the resulting data, using
        // Driver::Publish()

        //Read from the device.
        int numsensors(0);
        CPhidgetInterfaceKit_getNumSensors(ifk, &numsensors);

        std::vector<float> values;
        for (int i = 0 ; i != numsensors ; ++i) {
            values.push_back(0.0);
        }

        for (int i=0; i!=numsensors; ++i) {
            int tmpval=0;
            //getSensorValue returns an int from 0 to 1000 (0 to 5V)
            CPhidgetInterfaceKit_getSensorValue(ifk,i,&tmpval);
            values[i]=static_cast<float>(tmpval)/1000.0*5.0;  //change the scale to: 0-5V
        }


        //Generate the player data packet
        player_aio_data_t data_ai;

        data_ai.voltages_count=numsensors;
        data_ai.voltages = new float[numsensors];

        for (int i=0; i!=numsensors; ++i) {
            data_ai.voltages[i]=values[i];
            //std::cout << "Value[" << i << "] = " << values[i] << "\n";
        }


        player_dio_data_t data_di;
        //need the digital inputs as a bitfield
        int num_di(0);
        CPhidgetInterfaceKit_getNumInputs(ifk, &num_di);

        data_di.count=num_di;

        std::vector<bool> divalues;
        for (int i = 0 ; i != num_di ; ++i) {
            divalues.push_back(0.0);
        }

        for (int i=0; i!=num_di; ++i) {
            int tmpval=0;
            CPhidgetInterfaceKit_getInputState(ifk,i,&tmpval);
            divalues[i]=static_cast<bool>(tmpval);
        }


        //transfer the digital values to the required format for the player packet
        uint32_t bitfield(0);
        for (int i=0; i!=num_di; ++i) {
            bitfield+=(divalues[i] << i); //shift the 1 of the bool i bits to the left
            //std::cout << "DI-Value[" << i << "] = " << divalues[i] << "\n";

        }
        //std::cout << "Bitfield: " << bitfield << "\n";

        data_di.bits=bitfield;


        //Time to publish the packets! (only for the interfaces defined in the config file)
        if (aio_id.interf !=0) {
            Publish(aio_id, PLAYER_MSGTYPE_DATA, PLAYER_AIO_DATA_STATE, (unsigned char*)&data_ai, sizeof(player_aio_data_t), NULL);
        }
        delete [] data_ai.voltages;
        
        if (dio_id.interf != 0) {
            Publish(dio_id, PLAYER_MSGTYPE_DATA, PLAYER_DIO_DATA_VALUES, (unsigned char*)&data_di, sizeof(player_dio_data_t), NULL);
        }

        //point to calculate how much to sleep, call nanosleep, after sleep restart the timer
        //Get the ammount of time passed:
        gettimeofday( &tv_framerate_end, NULL );
        // figure out how much to sleep
        long usecs    = tv_framerate_end.tv_usec - tv_framerate_start.tv_usec;
        long secs     = tv_framerate_end.tv_sec  - tv_framerate_start.tv_sec;
        long elapsed_usecs = 1000000*secs  + usecs;

        long us_tosleep = static_cast<long>(samplingrate*1000) - elapsed_usecs;
        //std::cout << "usec to sleep: " << us_tosleep << std::endl;

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
