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

///////////////////////////////////////////////////////////////////////////
//
// Desc: Driver wrapper around the Gearbox urg_nz library.
// Author: Geoffrey Biggs
// Date: 25/02/2008
//
// Provides - Ranger device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_urg_nz urg_nz
 * @brief Gearbox urg_nz Hokuyo URG laser scanner driver library

This driver provides a @ref interface_ranger interface to the urg_nz Hokuyo URG laser scanner driver
provided by Gearbox. Communication with the laser can be either via USB or RS232. The driver
supports SCIP procol versions 1 and 2.

@par Compile-time dependencies

- Gearbox library urg_nz

@par Provides

- @ref interface_ranger : Output ranger interface

@par Configuration requests

- PLAYER_RANGER_REQ_GET_GEOM
- PLAYER_RANGER_REQ_GET_CONFIG
- PLAYER_RANGER_REQ_SET_CONFIG
   - Note: Only the min_angle and max_angle values can be configured using this request.

@par Configuration file options

 - port (string)
   - Default: "/dev/ttyACM0"
   - Port to which the laser is connected. Can be a serial port or the port associated with a USB ACM
     device.
 - baudrate (integer)
   - Default: 115200
   - Initial baud rate to connect at. Can be changed with the "baudrate" property. Valid rates are
     19200, 57600 and 115200. Only applies when use_serial is true.
 - use_serial (boolean)
   - Default: false
   - Connect over an RS232 serial connection instead of the default USB connection.
 - pose (float 6-tuple: (m, m, m, rad, rad, rad))
   - Default: [0.0 0.0 0.0 0.0 0.0 0.0]
   - Pose (x, y, z, roll, pitch, yaw) of the laser relative to its parent object (e.g. the robot).
 - size (float 3-tuple: (m, m, m))
   - Default: [0.0 0.0 0.0]
   - Size of the laser in metres.
 - min_angle (float, radians)
   - Default: -2.094 rad (-120.0 degrees)
   - Minimum scan angle to return.
 - max_angle (float, radians)
   - Default: 2.094 rad (120.0 degrees)
   - Maximum scan angle to return.
 - verbose (boolean)
   - Default: false
   - Enable verbose debugging information in the underlying library.

@par Properties

 - baudrate (integer)
   - Change the baud rate of the connection to the laser. Valid rates are 19200, 57600 and 115200.
     Only applies when use_serial is true. Not currently supported if SCIP v2 is in use.

@par Example

@verbatim
driver
(
  name "urg_nz"
  provides ["ranger:0"]
  port "/dev/ttyACM0"
)
@endverbatim

@author Geoffrey Biggs

*/
/** @} */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <iostream>
using namespace std;

#include <urg_nz/urg_nz.h>

#include <libplayercore/playercore.h>

const int DEFAULT_BAUDRATE = 115200;

class UrgDriver : public Driver
{
    public:
        UrgDriver (ConfigFile* cf, int section);
        ~UrgDriver (void);

        virtual int Setup (void);
        virtual int Shutdown (void);
        virtual int ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data);

    private:
        virtual void Main (void);
        bool ReadLaser (void);
        bool CalculateMinMaxIndices (void);

        // Configuration parameters
        bool useSerial, verbose;
        double minAngle, maxAngle;
        IntProperty baudRate;
        string port;
        int numSamples;
        // Config received from the laser
        urg_nz::urg_nz_laser_config_t config;
        // Geometry
        player_ranger_geom_t geom;
        player_pose3d_t sensorPose;
        player_bbox3d_t sensorSize;
        // Data storage, etc
        double *ranges;
        urg_nz::urg_nz_laser_readings_t *readings;
        unsigned int minIndex, maxIndex;
        // The hardware device itself
        urg_nz::urg_laser device;
};

Driver*
UrgDriver_Init (ConfigFile* cf, int section)
{
    return reinterpret_cast <Driver*> (new UrgDriver (cf, section));
}

void UrgDriver_Register(DriverTable* table)
{
    table->AddDriver ("urg_nz", UrgDriver_Init);
}

UrgDriver::UrgDriver (ConfigFile* cf, int section)
    : Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_RANGER_CODE),
    baudRate ("baudrate", DEFAULT_BAUDRATE, false), ranges (NULL), readings (NULL),
    minIndex (0), maxIndex (urg_nz::MAX_READINGS)
{
    // Get and sanity-check the baudrate
    RegisterProperty ("baudrate", &baudRate, cf, section);
    if (baudRate.GetValue () != 19200 && baudRate.GetValue () != 57600 && baudRate.GetValue () != 115200)
    {
        PLAYER_WARN2 ("urg_nz: Ignored bad baud rate: %d, using default of %d", baudRate.GetValue (), DEFAULT_BAUDRATE);
        baudRate.SetValue (DEFAULT_BAUDRATE);
    }

    // Get config
    minAngle = cf->ReadFloat (section, "min_angle", DTOR (-120.0f));
    maxAngle = cf->ReadFloat (section, "max_angle", DTOR (120.0f));
    useSerial = cf->ReadBool (section, "use_serial", false);
    port = cf->ReadString (section, "port", "/dev/ttyACM0");
    verbose = cf->ReadBool (section, "verbose", false);

    // Set up geometry information
    geom.pose.px = cf->ReadTupleLength (section, "pose", 0, 0.0f);
    geom.pose.py = cf->ReadTupleLength (section, "pose", 1, 0.0f);
    geom.pose.pz = cf->ReadTupleLength (section, "pose", 2, 0.0f);
    geom.pose.proll = cf->ReadTupleAngle (section, "pose", 3, 0.0f);
    geom.pose.ppitch = cf->ReadTupleAngle (section, "pose", 4, 0.0f);
    geom.pose.pyaw = cf->ReadTupleAngle (section, "pose", 5, 0.0f);
    geom.size.sw = cf->ReadTupleLength (section, "size", 0, 0.0f);
    geom.size.sl = cf->ReadTupleLength (section, "size", 1, 0.0f);
    geom.size.sh = cf->ReadTupleLength (section, "size", 2, 0.0f);
    geom.sensor_poses_count = 1;
    geom.sensor_poses = &sensorPose;
    memcpy (geom.sensor_poses, &geom.pose, sizeof (geom.pose));
    geom.sensor_sizes_count = 1;
    geom.sensor_sizes = &sensorSize;
    memcpy (geom.sensor_sizes, &geom.size, sizeof (geom.size));

    // Turn on/off verbose mode
    device.SetVerbose (verbose);
}

UrgDriver::~UrgDriver (void)
{
    if (ranges != NULL)
        delete[] ranges;
    if (readings != NULL)
        delete[] readings;
}

int UrgDriver::Setup (void)
{
    try
    {
        // Open the laser
        device.Open (port.c_str (), useSerial, baudRate.GetValue ());
        // Get the current config
        device.GetSensorConfig (&config);
        if (!CalculateMinMaxIndices ())
            return -1;
    }
    catch (urg_nz::urg_nz_exception &e)
    {
        PLAYER_ERROR2 ("urg_nz: Failed to setup laser driver: (%d) %s", e.error_code, e.error_desc.c_str ());
        SetError (e.error_code);
        return -1;
    }

    // Create space to store data
    if ((ranges = new double[maxIndex - minIndex + 1]) == NULL)
    {
        PLAYER_ERROR ("urg_nz: Failed to allocate data store.");
        return -1;
    }
    if ((readings = new urg_nz::urg_nz_laser_readings_t) == NULL)
    {
        PLAYER_ERROR ("urg_nz: Failed to allocate intermediate data store.");
        return -1;
    }

    StartThread();
    return 0;
}

int UrgDriver::Shutdown (void)
{
    StopThread();

    device.Close ();

    if (ranges != NULL)
    {
        delete[] ranges;
        ranges = NULL;
    }
    if (readings != NULL)
    {
        delete readings;
        readings = NULL;
    }

    return 0;
}

int UrgDriver::ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
    // Check for capability requests
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ);
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM);
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG);

    // Check for a change in the baud rate property; we need to handle this manually rather than letting the driver
    // class handle it because we need to change the baud rate in the library
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_INTPROP_REQ, this->device_addr))
    {
        player_intprop_req_t *req = reinterpret_cast<player_intprop_req_t*> (data);
        if (strcmp(req->key, "baudrate") == 0)
        {
            try
            {
                // Change the baud rate
                if (device.ChangeBaud (baudRate, req->value) == 0)
                {
                    baudRate.SetValueFromMessage (data);
                    Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_INTPROP_REQ, NULL, 0, NULL);
                }
                else
                {
                    PLAYER_WARN ("urg_nz: Unable to change baud rate.");
                    Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SET_INTPROP_REQ, NULL, 0, NULL);
                }
            }
            catch (urg_nz::urg_nz_exception &e)
            {
                PLAYER_ERROR2 ("urg_nz: Fatal error while changing baud rate: (%d) %s", e.error_code, e.error_desc.c_str ());
                SetError (e.error_code);
                return -1;
            }
            return 0;
        }
    }
    // Standard ranger messages
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM, device_addr))
    {
        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_GEOM, &geom, sizeof (geom), NULL);
        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG, device_addr))
    {
        player_ranger_config_t rangerConfig;
        rangerConfig.min_angle = minAngle;
        rangerConfig.max_angle = maxAngle;
        rangerConfig.resolution = config.resolution;
        rangerConfig.max_range = config.max_range / 1000.0f;
        rangerConfig.range_res = 0.0f;
        rangerConfig.frequency = 0.0f;
        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_CONFIG, &rangerConfig, sizeof (rangerConfig), NULL);
        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_SET_CONFIG, device_addr))
    {
        player_ranger_config_t *newParams = reinterpret_cast<player_ranger_config_t*> (data);
        minAngle = newParams->min_angle;
        maxAngle = newParams->max_angle;
        if (!CalculateMinMaxIndices ())
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_RANGER_REQ_GET_CONFIG, NULL, 0, NULL);
        else
        {
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_CONFIG, newParams, sizeof (*newParams), NULL);
            // Reallocate ranges
            delete[] ranges;
            if ((ranges = new double[maxIndex - minIndex + 1]) == NULL)
            {
                PLAYER_ERROR ("urg_nz: Failed to allocate data store.");
                Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_RANGER_REQ_GET_CONFIG, NULL, 0, NULL);
            }
            else
                Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_CONFIG, newParams, sizeof (*newParams), NULL);
        }
        return 0;
    }

    return -1;
}

void UrgDriver::Main (void)
{
    while (true)
    {
        pthread_testcancel ();
        ProcessMessages ();

        if (!ReadLaser ())
            break;
    }
}

bool UrgDriver::ReadLaser (void)
{
    player_ranger_data_range_t rangeData;

    try
    {
        unsigned int numRead = device.GetReadings (readings, minIndex, maxIndex);
        if (numRead != (maxIndex - minIndex + 1))
        {
            PLAYER_WARN2 ("urg_nz: Warning: Got an unexpected number of range readings (%d != %d)", numRead, maxIndex - minIndex + 1);
            return true;    // Maybe we'll get more next time
        }

        for (unsigned int ii; ii < numRead; ii++)
            ranges[ii] = readings->Readings[ii] / 1000.0f;
        rangeData.ranges = ranges;
        rangeData.ranges_count = numRead;
        Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_RANGE, reinterpret_cast<void*> (&rangeData), sizeof (rangeData), NULL);
    }
    catch (urg_nz::urg_nz_exception &e)
    {
        PLAYER_ERROR2 ("urg_nz: Failed to read scan: (%d) %s", e.error_code, e.error_desc.c_str ());
        SetError (e.error_code);
        return false;
    }

    return true;
}

bool UrgDriver::CalculateMinMaxIndices (void)
{
    unsigned int minPossibleIndex, maxPossibleIndex;

    // Calculate min and max scan indices
    minIndex = static_cast<unsigned int> (round ((urg_nz::MAX_READINGS / 2) + minAngle / config.resolution));
    maxIndex = static_cast<unsigned int> (round ((urg_nz::MAX_READINGS / 2) + maxAngle / config.resolution));
    // Sanity check
    if (minIndex > maxIndex)
        minIndex = maxIndex;
    // Clip the min and max scan indices
    minPossibleIndex = static_cast<unsigned int> (round ((urg_nz::MAX_READINGS / 2) + config.min_angle / config.resolution));
    maxPossibleIndex = static_cast<unsigned int> (round ((urg_nz::MAX_READINGS / 2) + config.max_angle / config.resolution));
    if (minIndex < minPossibleIndex)
    {
        minIndex = minPossibleIndex;
        minAngle = config.min_angle;
        PLAYER_WARN1 ("urg_nz: Warning: min_angle clipped to %f", config.min_angle);
    }
    if (maxIndex > maxPossibleIndex)
    {
        maxIndex = maxPossibleIndex;
        maxAngle = config.max_angle;
        PLAYER_WARN1 ("urg_nz: Warning: max_angle clipped to %f", config.max_angle);
    }

    return true;
}
