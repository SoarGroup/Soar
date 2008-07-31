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
 Desc: Driver for XSens MTx/MTi IMU. CMTComm class borrowed from XSens under GPL.
 Author: Radu Bogdan Rusu
 Date: 1 Aug 2006
  */
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_xsensmt xsensmt
 * @brief XSens MTx/MTi Inertial Measurement Unit driver

The xsensmt driver controls the XSens MTx/MTi Inertial Measurement Unit. It 
provides Kalman filtered orientation information (pitch, roll, yaw) via its 
internal 3-axis accelerometer, 3-axis gyroscope and 3-axis magnetometer.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_imu

@par Requires

- none

@par Configuration requests

- PLAYER_IMU_REQ_SET_DATATYPE
- PLAYER_IMU_REQ_RESET_ORIENTATION

@par Configuration file options

- port (string)
  - Default: "/dev/ttyUSB0"
  - Serial port to which the XSens MTx/MTi sensor is attached.  If you are
    using a USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- data_packet_type (integer)
  - Default: 4. Possible values: 1, 2, 3, 4.
   (1 = 3D pose as X, Y, Z and + orientation/Euler angles as Roll, Pitch, Yaw; 
    2 = calibrated IMU data: accel, gyro, magnetometer;
    3 = quaternions + calibrated IMU data;
    4 = Euler angles + calibrated IMU data.)
  - Specify the type of data packet to send (can be set using 
    PLAYER_IMU_REQ_SET_DATATYPE as well).

@par Example 

@verbatim
driver
(
  name "xsensmt"
  provides ["imu:0"]
  port "/dev/ttyUSB0"
  # We need quaternions + calibrated accel/gyro/magnetometer data
  data_packet_type 3
)
@endverbatim

@author Radu Bogdan Rusu

 */
/** @} */

#include <unistd.h>
#include <string.h>
#include <libplayercore/playercore.h>
#include "MTComm.h"

#define DEFAULT_PORT "/dev/ttyUSB0"

////////////////////////////////////////////////////////////////////////////////
// The XSensMT device class.
class XSensMT : public Driver
{
    public:
        // Constructor
        XSensMT (ConfigFile* cf, int section);

        // Destructor
        ~XSensMT ();

        // Implementations of virtual functions
        virtual int Setup ();
        virtual int Shutdown ();

        // This method will be invoked on each incoming message
        virtual int ProcessMessage (QueuePointer &resp_queue,
                                    player_msghdr * hdr,
                                    void * data);
    private:

        // MTComm object
        CMTComm mtcomm;

        const char* portName;

        // Data
        player_imu_data_state_t  imu_data_state;
        player_imu_data_calib_t  imu_data_calib;
        player_imu_data_quat_t   imu_data_quat;
        player_imu_data_euler_t  imu_data_euler;

        unsigned char data[MAXMSGLEN];
        short datalen;
        int received;

        // Desired data packet type
        int dataType;

        // Main function for device thread.
        virtual void Main ();
        virtual void RefreshData  ();

        player_imu_data_calib_t GetCalibValues (const unsigned char data[]);
};

////////////////////////////////////////////////////////////////////////////////
// Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* XSensMT_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
    return ((Driver*)(new XSensMT (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void XSensMT_Register (DriverTable* table)
{
    table->AddDriver ("xsensmt", XSensMT_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
XSensMT::XSensMT (ConfigFile* cf, int section)
    : Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
              PLAYER_IMU_CODE)
{
    portName = cf->ReadString (section, "port", DEFAULT_PORT);

    // Euler + Calibrated values
    dataType = cf->ReadInt    (section, "data_packet_type", 4);

    return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
XSensMT::~XSensMT()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int XSensMT::Setup ()
{
    // Open the device
    if (mtcomm.openPort (portName) != MTRV_OK)
        return (-1);

    int outputSettings = OUTPUTSETTINGS_ORIENTMODE_EULER;

    unsigned long tmpOutputMode, tmpOutputSettings;
    unsigned short tmpDataLength;

    // Put MTi/MTx in Config State
    if (mtcomm.writeMessage (MID_GOTOCONFIG) != MTRV_OK){
        PLAYER_ERROR ("No device connected!");
        return false;
    }

    unsigned short numDevices;
    // Get current settings and check if Xbus Master is connected
    if (mtcomm.getDeviceMode (&numDevices) != MTRV_OK) {
        if (numDevices == 1)
            PLAYER_ERROR ("MTi / MTx has not been detected\nCould not get device mode!");
        else
            PLAYER_ERROR ("Not just MTi / MTx connected to Xbus\nCould not get all device modes!");
        return false;
    }

    // Check if Xbus Master is connected
    mtcomm.getMode (tmpOutputMode, tmpOutputSettings, tmpDataLength, BID_MASTER);
    if (tmpOutputMode == OUTPUTMODE_XM)
    {
        // If Xbus Master is connected, attached Motion Trackers should not send sample counter
        PLAYER_ERROR ("Sorry, this driver only talks to one MTx/MTi device.");
        return false;
    }

    int outputMode = OUTPUTMODE_CALIB + OUTPUTMODE_ORIENT;
    // Set output mode and output settings for the MTi/MTx
    if (mtcomm.setDeviceMode (outputMode, outputSettings, BID_MASTER) != MTRV_OK) {
        PLAYER_ERROR ("Could not set device mode(s)!");
        return false;
    }

    // Put MTi/MTx in Measurement State
    mtcomm.writeMessage (MID_GOTOMEASUREMENT);

    PLAYER_MSG0 (1, "> XSensMT starting up... [done]");

    // Start the device thread
    StartThread ();

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int XSensMT::Shutdown ()
{
    // Stop the driver thread
    StopThread ();

    // Close the MTx device
    if (mtcomm.close () != MTRV_OK)
        PLAYER_ERROR ("Could not close device!");

    PLAYER_MSG0 (1, "> XSensMT driver shutting down... [done]");
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void XSensMT::Main () 
{
    timespec sleepTime = {0, 0};

    // modify the scheduling priority
//    nice (10);

    // The main loop; interact with the device here
    while (true)
    {
        // test if we are supposed to cancel
        pthread_testcancel ();

        // Process any pending messages
        ProcessMessages ();

        // Refresh data
        this->RefreshData ();

        nanosleep (&sleepTime, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage function
int XSensMT::ProcessMessage (QueuePointer &resp_queue,
                             player_msghdr * hdr,
                             void * data)
{
    assert (hdr);
    assert (data);
    
    // this holds possible error messages returned by mtcomm.writeMessage
    int err;
    
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
        PLAYER_IMU_REQ_SET_DATATYPE, device_addr))
    {
        // Change the data type according to the user's preferences
        player_imu_datatype_config *datatype =
                (player_imu_datatype_config*)data;

        if ((datatype->value > 0) && (datatype->value < 5))
        {
            dataType = datatype->value;
	    
	    int outputSettings = OUTPUTSETTINGS_ORIENTMODE_EULER;
	    switch (dataType)
	    {
		case 1:
		{
		    outputSettings = OUTPUTSETTINGS_ORIENTMODE_EULER;
		    break;
		}
		case 2:
		{
		    break;
		}
		case 3:
		{
		    outputSettings = OUTPUTSETTINGS_ORIENTMODE_QUATERNION;
		    break;
		}
		case 4:
		{
		    outputSettings = OUTPUTSETTINGS_ORIENTMODE_EULER;
		    break;
		}
		default:
		{
		    outputSettings = OUTPUTSETTINGS_ORIENTMODE_EULER;
		}
		
	    }
	    // Put MTi/MTx in Config State
	    if (mtcomm.writeMessage (MID_GOTOCONFIG) != MTRV_OK)
	    {
    		PLAYER_ERROR ("No device connected!");
        	Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK,
                     hdr->subtype);
		return (-1);
	    }
	    
	    int outputMode = OUTPUTMODE_CALIB + OUTPUTMODE_ORIENT;
	    // Set output mode and output settings for the MTi/MTx
	    if (mtcomm.setDeviceMode (outputMode, outputSettings, BID_MASTER) != MTRV_OK) {
    		PLAYER_ERROR ("Could not set device mode(s)!");
    		return false;
	    }

	    // Put MTi/MTx in Measurement State
	    mtcomm.writeMessage (MID_GOTOMEASUREMENT);
            
	    Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                     hdr->subtype);
        }
        else
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK,
                     hdr->subtype);

        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
        PLAYER_IMU_REQ_RESET_ORIENTATION, device_addr))
    {
        // Change the data type according to the user's preferences
        player_imu_reset_orientation_config *rconfig =
                (player_imu_reset_orientation_config*)data;

	// 0 = store current settings
	// 1 = heading reset
	// 2 = global reset
	// 3 = object reset
	// 4 = align reset
        if ((rconfig->value >= 0) && (rconfig->value <= 4))
        {
	    // Force <global reset> until further tests.
            rconfig->value = 2;
	    
	    if ((err = mtcomm.writeMessage (MID_RESETORIENTATION, 
		RESETORIENTATION_GLOBAL, LEN_RESETORIENTATION, BID_MASTER)) != MTRV_OK)
	    {
    		PLAYER_ERROR1 ("Could not put reset orientation on device! Error 0x%x\n", err);
        	Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK,
                     hdr->subtype);
		return (-1);
	    }
	    
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                     hdr->subtype);
        }
        else
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK,
                     hdr->subtype);

        return 0;
    }
    else
    {
        return -1;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// RefreshData function
void XSensMT::RefreshData ()
{
    // Get data from the MTx device
    received = mtcomm.readDataMessage (data, datalen);

    // Need to test this!
    if (received != MTRV_OK)
        return;

    // Check which type of data does the user want to receive
    switch (dataType)
    {
        case PLAYER_IMU_DATA_STATE:
        {
            float euler_data[3] = {0};

            // Parse and get value (EULER orientation)
            mtcomm.getValue (VALUE_ORIENT_EULER, euler_data, data, BID_MASTER);
            imu_data_state.pose.px = -1;
            imu_data_state.pose.py = -1;
            imu_data_state.pose.pz = -1;

            imu_data_state.pose.proll  = euler_data[0];
            imu_data_state.pose.ppitch = euler_data[1];
            imu_data_state.pose.pyaw   = euler_data[2];

            Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_IMU_DATA_STATE,
                     &imu_data_state, sizeof (player_imu_data_state_t), NULL);
            break;
        }
        case PLAYER_IMU_DATA_CALIB:
        {
            imu_data_calib = GetCalibValues (data);

            Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_IMU_DATA_CALIB,
                     &imu_data_calib, sizeof (player_imu_data_calib_t), NULL);
            break;
        }
        case PLAYER_IMU_DATA_QUAT:
        {
            float quaternion_data[4] = {0};
            // Parse and get value (quaternion orientation)
            imu_data_quat.calib_data = GetCalibValues (data);
	    
            mtcomm.getValue (VALUE_ORIENT_QUAT, quaternion_data, data, BID_MASTER);
            imu_data_quat.q0 = quaternion_data[0];
            imu_data_quat.q1 = quaternion_data[1];
            imu_data_quat.q2 = quaternion_data[2];
            imu_data_quat.q3 = quaternion_data[3];

            Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_IMU_DATA_QUAT,
                     &imu_data_quat, sizeof (player_imu_data_quat_t), NULL);
            break;
        }
        case PLAYER_IMU_DATA_EULER:
        {
            float euler_data[3] = {0};
            // Parse and get value (Euler orientation)
            mtcomm.getValue (VALUE_ORIENT_EULER, euler_data, data, BID_MASTER);

            imu_data_euler.calib_data = GetCalibValues (data);
            imu_data_euler.orientation.proll  = euler_data[0];
            imu_data_euler.orientation.ppitch = euler_data[1];
            imu_data_euler.orientation.pyaw   = euler_data[2];

            Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_IMU_DATA_EULER,
                     &imu_data_euler, sizeof (player_imu_data_euler_t), NULL);
            break;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// GetCalibValues function
player_imu_data_calib_t XSensMT::GetCalibValues (const unsigned char data[]) {
    player_imu_data_calib_t calib_data;
    float accel_data[3] = {0};
    float gyro_data [3] = {0};
    float magn_data [3] = {0};

    // Parse and get calibrated acceleration values
    mtcomm.getValue (VALUE_CALIB_ACC, accel_data, data, BID_MASTER);
    // Parse and get calibrated gyro values
    mtcomm.getValue (VALUE_CALIB_GYR, gyro_data, data, BID_MASTER);
    // Parse and get calibrated magnetometer values
    mtcomm.getValue (VALUE_CALIB_MAG, magn_data, data, BID_MASTER);

    calib_data.accel_x = accel_data [0];
    calib_data.accel_y = accel_data [1];
    calib_data.accel_z = accel_data [2];
    calib_data.gyro_x  = gyro_data  [0];
    calib_data.gyro_y  = gyro_data  [1];
    calib_data.gyro_z  = gyro_data  [2];
    calib_data.magn_x  = magn_data  [0];
    calib_data.magn_y  = magn_data  [1];
    calib_data.magn_z  = magn_data  [2];

    return calib_data;
}
