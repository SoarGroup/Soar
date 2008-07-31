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
 Desc: Driver for nIMU IMU from Memsense
 Author: Toby Collett
 Date: 20 Oct 2006
  */
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_nimu nimu
 * @brief nIMU Inertial Measurement Unit driver

The nimu driver interfaces to the nIMU from MemSense. Currently supports the 
I2C version communicating via the provided USB dongle.

Currently only provides the raw outputs.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_imu

@par Requires

- none

@par Configuration requests

- PLAYER_IMU_REQ_SET_DATATYPE

@par Configuration file options

- accel_range (float, G)
  - Default: 5

- gyro_range (float, Degrees/sec)
  - Default: 300

- mag_range (float, Gauss)
  - Default: 1.9

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
  name "nimu"
  provides ["imu:0"]
  data_packet_type 2
)
@endverbatim

@author Radu Bogdan Rusu

 */
/** @} */

#include <unistd.h>
#include <string.h>
#include <libplayercore/playercore.h>
#include "nimu.h"

#define DEFAULT_ACCEL_RANGE 2
#define DEFAULT_GYRO_RANGE 300
#define DEFAULT_MAG_RANGE 1.9

#define GET_VALUE_IN_UNITS(VALUE,RANGE) (VALUE*((RANGE)*1.5/32768))

////////////////////////////////////////////////////////////////////////////////
// The XSensMT device class.
class PlayerNIMU : public Driver
{
	public:
        // Constructor
		PlayerNIMU (ConfigFile* cf, int section);

        // Destructor
		~PlayerNIMU ();

        // Implementations of virtual functions
		virtual int Setup ();
		virtual int Shutdown ();

        // This method will be invoked on each incoming message
		virtual int ProcessMessage (QueuePointer &resp_queue,
									player_msghdr * hdr,
									void * data);
	private:

        // IMU object
		nimu imu;

		float AccelRange;
		float GyroRange;
		float MagRange;
		
        // Data
		player_imu_data_calib_t  imu_data_calib;

        // Desired data packet type
		int dataType;

        // Main function for device thread.
		virtual void Main ();
};

////////////////////////////////////////////////////////////////////////////////
// Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* NIMU_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
	return ((Driver*)(new PlayerNIMU (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void NIMU_Register (DriverTable* table)
{
	table->AddDriver ("nimu", NIMU_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
PlayerNIMU::PlayerNIMU (ConfigFile* cf, int section)
	: Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
			  PLAYER_IMU_CODE)
{
    // raw values
	dataType = cf->ReadInt    (section, "data_packet_type", 2);

	AccelRange = 9.81*cf->ReadFloat(section, "accel_range", DEFAULT_ACCEL_RANGE);
	GyroRange = DTOR(cf->ReadFloat(section, "gyro_range", DEFAULT_GYRO_RANGE));
	MagRange = cf->ReadFloat(section, "mag_range", DEFAULT_MAG_RANGE);

	
	return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
PlayerNIMU::~PlayerNIMU()
{
	return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int PlayerNIMU::Setup ()
{
    // Open the device
	if (imu.Open() < 0)
		return -1;
	
    // Start the device thread
	StartThread ();

	return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int PlayerNIMU::Shutdown ()
{
    // Stop the driver thread
	StopThread ();

	// close the device
	imu.Close();
	
	return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void PlayerNIMU::Main () 
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
		nimu_data data = imu.GetData();
		
		imu_data_calib.accel_x = GET_VALUE_IN_UNITS(data.AccelX,AccelRange);
		imu_data_calib.accel_y = GET_VALUE_IN_UNITS(data.AccelY,AccelRange);
		imu_data_calib.accel_z = GET_VALUE_IN_UNITS(data.AccelZ,AccelRange);

		imu_data_calib.gyro_x = GET_VALUE_IN_UNITS(data.GyroX,GyroRange);
		imu_data_calib.gyro_y = GET_VALUE_IN_UNITS(data.GyroY,GyroRange);
		imu_data_calib.gyro_z = GET_VALUE_IN_UNITS(data.GyroZ,GyroRange);
		
		imu_data_calib.magn_x = GET_VALUE_IN_UNITS(data.MagX,MagRange);
		imu_data_calib.magn_y = GET_VALUE_IN_UNITS(data.MagY,MagRange);
		imu_data_calib.magn_z = GET_VALUE_IN_UNITS(data.MagZ,MagRange);
		
		Publish( device_addr, PLAYER_MSGTYPE_DATA, PLAYER_IMU_DATA_CALIB, &imu_data_calib, sizeof(imu_data_calib));
		
		nanosleep (&sleepTime, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage function
int PlayerNIMU::ProcessMessage (QueuePointer &resp_queue,
							 player_msghdr * hdr,
							 void * data)
{
	assert (hdr);
	assert (data);

	return -1;
}


