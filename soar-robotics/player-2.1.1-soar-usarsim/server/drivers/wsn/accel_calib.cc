/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 Radu Bogdan Rusu (rusu@cs.tum.edu)
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
 Desc: Acceleration calibration driver
 Author: Radu Bogdan Rusu
 Date: 30 Jun 2006
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_accel_calib accel_calib
 * @brief Acceleration calibration driver

The accel_calib driver receives acceleration data from a WSN interface, then 
calculates the calibrated values and returns them via another WSN interface.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_wsn

@par Requires

- @ref interface_wsn

@par Configuration requests

- none

@par Configuration file options

- node (integer tupple)
  - These are the calibration values for -1G/+1G for the accelerometer
    sensor (see the appropriate data sheet on how to obtain it). Each sepparate
    board *MUST* be calibrated! 
  - The tuple means: [node_id
                      group_id
                      calibration_negative_1g_x_axis
                      calibration_positive_1g_x_axis
                      calibration_negative_1g_y_axis
                      calibration_positive_1g_y_axis
                      calibration_negative_1g_z_axis
                      calibration_positive_1g_z_axis
                     ]

- units (integer)
  - Default: 1.
  - Fill the data buffer with converted engineering units (e.g. m/s^2 - 1) or
    G (2) values.

@par Example 

@verbatim
driver
(
  name "accel_calib"
  requires ["wsn:0"]
  provides ["wsn:1"]
# Calibrate node 0 from group 125 (default) with X={419,532} and Y={440,552}
  node [0 125 419 532 440 552 0 0]
# Calibrate node 2 from group 125 (default) with X={447,557} and Y={410,520}
  node [2 125 447 557 410 520 0 0]
# Use m/s^2 values
  units 1
)
@endverbatim

@author Radu Bogdan Rusu

 */
/** @} */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <vector>

// Includes needed for player
#include <libplayercore/playercore.h>

// ---[ Node calibration values ]---
class NodeCalibrationValues
{
    public:
        unsigned int node_id;           // node identifier
        unsigned int group_id;          // group identifier
        int          c_values[6];       // calibration values
};
typedef std::vector<NodeCalibrationValues> NCV;

// The Accel_Calib device class.
class Accel_Calib : public Driver
{
    public:
        // Constructor
        Accel_Calib (ConfigFile* cf, int section);

        // Destructor
        ~Accel_Calib ();

        // Implementations of virtual functions
        virtual int Setup       ();
        virtual int Shutdown    ();
        // This method will be invoked on each incoming message
        virtual int ProcessMessage (QueuePointer &resp_queue, 
                                    player_msghdr * hdr,
                                    void * data);

    private:

        // Main function for device thread.
        virtual void Main ();
        void RefreshData  ();

        // Port file descriptor
        int                fd;

        // What kind of conversion do we need?
        int                converted_units;

        // Interfaces that we might be using
        // WSN interface
        player_devaddr_t   wsn_addr;
        Device*            wsn_device;

        // Calibration values
        int                nodes_count;
        NCV                ncv;

        // Calibration values
        int                calibration_values[6];
        // Calibration node ID
        int                calibration_node_id;

        NodeCalibrationValues FindNodeValues (unsigned int nodeID);

        float ConvertAccel (float raw_accel, int neg_1g, int pos_1g,
                            int converted);
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* Accel_Calib_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
    return ((Driver*)(new Accel_Calib (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void Accel_Calib_Register (DriverTable* table)
{
    table->AddDriver ("accel_calib", Accel_Calib_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Accel_Calib::Accel_Calib (ConfigFile* cf, int section)
    : Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
              PLAYER_WSN_CODE)
{
    int i = 0;
    int j = 0;

    memset(&this->wsn_addr,  0, sizeof (player_devaddr_t));

    nodes_count = cf->ReadInt (section, "nodes", 0);
	
    for (i = 0; i < nodes_count; i++)
    {
        char node_nr[7];
        sprintf (node_nr, "node%d", (i+1));
        NodeCalibrationValues n;
        n.node_id  = cf->ReadTupleInt (section, node_nr, 0, 0);
        n.group_id = cf->ReadTupleInt (section, node_nr, 1, 0);
        for (j = 0; j < 6; j++)
            n.c_values[j] = cf->ReadTupleInt (section, node_nr, j+2, 0);
        ncv.push_back (n);
    }

    // Defaults to converted values
    converted_units = cf->ReadInt (section, "converted", 1);

    // Do we create a WSN interface?
    if (cf->ReadDeviceAddr (&wsn_addr, section, "requires",
        PLAYER_WSN_CODE, -1, NULL) != 0)
    {
        PLAYER_ERROR ("> Must provide one output WSN interface!");
        this->SetError(-1);
        return;
    }
    return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
Accel_Calib::~Accel_Calib()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Accel_Calib::Setup ()
{
    // Subscribe to the WSN device
    if (!(wsn_device = deviceTable->GetDevice (wsn_addr)))
    {
        PLAYER_ERROR ("> Unable to locate the suitable WSN device!");
        return (-1);
    }
    if (wsn_device->Subscribe (this->InQueue) != 0)
    {
        PLAYER_ERROR ("> Unable to subscribe to the WSN device!");
        return (-1);
    }

    // Start the device thread
    StartThread ();

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Accel_Calib::Shutdown ()
{
    // Stop the driver thread
    StopThread ();

    // Unsubscribe from the WSN device
    wsn_device->Unsubscribe (this->InQueue);

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Accel_Calib::Main () 
{
//    timespec sleepTime = {0, 0};

    // The main loop; interact with the device here
    while (true)
    {
        // wait to receive a new message (blocking)
        Wait();

        // test if we are supposed to cancel
        pthread_testcancel();

        // Process incoming messages, and update outgoing data
        ProcessMessages();

        //nanosleep (&sleepTime, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage function
int Accel_Calib::ProcessMessage (QueuePointer &resp_queue, 
                                 player_msghdr * hdr,
                                 void * data)
{
    NodeCalibrationValues node_values;
    player_wsn_data_t  new_wsn_data;
    player_wsn_data_t* original_wsn_data;

    assert (hdr);
    assert (data);

    // Handle new data from the WSN device
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_WSN_DATA_STATE, 
       wsn_addr))
    {
        original_wsn_data = reinterpret_cast<player_wsn_data_t *>(data);
        if (converted_units != 0)
        {
            node_values = FindNodeValues (original_wsn_data->node_id);

            if (original_wsn_data->data_packet.accel_x != -1)
                new_wsn_data.data_packet.accel_x = ConvertAccel 
                        (original_wsn_data->data_packet.accel_x, 
                         node_values.c_values[0], 
                         node_values.c_values[1], converted_units);

            if (original_wsn_data->data_packet.accel_y != -1)
                new_wsn_data.data_packet.accel_y = ConvertAccel 
                        (original_wsn_data->data_packet.accel_y,
                         node_values.c_values[2],
                        node_values.c_values[3], converted_units);

            if (original_wsn_data->data_packet.accel_z != -1)
                new_wsn_data.data_packet.accel_z = ConvertAccel 
                        (original_wsn_data->data_packet.accel_z,
                         node_values.c_values[4],
                        node_values.c_values[5], converted_units);
        } 
        else
        {
            new_wsn_data.data_packet.accel_x = 
                    original_wsn_data->data_packet.accel_x;
            new_wsn_data.data_packet.accel_y = 
                    original_wsn_data->data_packet.accel_y;
            new_wsn_data.data_packet.accel_z = 
                    original_wsn_data->data_packet.accel_z;
        }

        new_wsn_data.data_packet.light       = -1;
        new_wsn_data.data_packet.mic         = -1;
        new_wsn_data.data_packet.magn_x      = -1;
        new_wsn_data.data_packet.magn_y      = -1;
        new_wsn_data.data_packet.magn_z      = -1;
        new_wsn_data.data_packet.temperature = -1;
        new_wsn_data.data_packet.battery     = -1;

        // Write the WSN data
        Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_WSN_DATA_STATE, 
                 &new_wsn_data, sizeof (player_wsn_data_t), NULL);

        return 0;
    }
    else
    {
        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ConvertAccel function - convert RAW accel. data to metric units (m/s^2)
float Accel_Calib::ConvertAccel (float raw_accel, 
                           int neg_1g, int pos_1g, int converted)
{
    if (neg_1g == 0)
        neg_1g = 450;
    if (pos_1g == 0)
        pos_1g = 550;
	
    float sensitivity  = (pos_1g - neg_1g) / 2.0f;
    float offset       = (pos_1g + neg_1g) / 2.0f;
    float acceleration = (raw_accel - offset) / sensitivity;
    if (converted == 1)
        return acceleration * 9.81;
    else
        return acceleration;
}

////////////////////////////////////////////////////////////////////////////////
// FindNodeValues function - find the appropriate calibration values for nodeID
NodeCalibrationValues Accel_Calib::FindNodeValues (unsigned int nodeID)
{
    NodeCalibrationValues n;
	
    unsigned int i = 0;
	
    for (i = 0; i < ncv.size (); i++)
    {
        n = ncv.at (i);
	
        if (n.node_id == nodeID)
            break;
    }
	
    return n;
}
//------------------------------------------------------------------------------
