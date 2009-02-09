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
// Desc: Driver wrapper around the Gearbox sickacfr library.
// Author: Geoffrey Biggs
// Date: 25/02/2008
//
// Provides - Ranger device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_gbxsickacfr gbxsickacfr
 * @brief Gearbox sickacfr SICK LMS driver library

This driver provides a @ref interface_ranger interface to the sickacfr SICK LMS400
laser scanner driver provided by Gearbox.

@par Compile-time dependencies

- Gearbox library GbxSickAcfr

@par Provides

- @ref interface_ranger : Output ranger interface

@par Supported configuration requests

- PLAYER_RANGER_REQ_GET_GEOM
- PLAYER_RANGER_REQ_GET_CONFIG

@par Configuration file options

 - min_range (float, metres)
   - Default: 0.0m
 - max_range (float, metres)
   - Default: 80.0m
 - field_of_view (float, radians)
   - Default: 3.14 radians (180.0 degrees)
 - start_angle (float, radians)
   - Default: -1.57 radians (-90.0 degrees)
 - num_samples (integer)
   - Default: 181
   - Number of range samples to take. Divide field_of_view by this to get the resolution.
 - baudrate (integer)
   - Default: 38400
 - port (string)
   - Default: /dev/ttyS0
   - Serial port the laser is connected to.
 - debug (boolean)
   - Default: false
   - Turn on debugging mode of the underlying library to get verbose output.
 - pose (float 6-tuple: (m, m, m, rad, rad, rad))
   - Default: [0.0 0.0 0.0 0.0 0.0 0.0]
   - Pose (x, y, z, roll, pitch, yaw) of the laser relative to its parent object (e.g. the robot).
 - size (float 3-tuple: (m, m, m))
   - Default: [0.0 0.0 0.0]
   - Size of the laser in metres.

@par Example

@verbatim
driver
(
  name "gbxsickacfr"
  provides ["ranger:0"]
  port "/dev/ttyS0"
  baud 57600
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
using namespace std;

#include <gbxsickacfr/driver.h>
#include <gbxsickacfr/gbxutilacfr/trivialtracer.h>
#include <gbxsickacfr/gbxutilacfr/trivialstatus.h>
#include <gbxsickacfr/gbxutilacfr/mathdefs.h>

#include <libplayercore/playercore.h>

class GbxSickAcfr : public Driver
{
    public:
        GbxSickAcfr (ConfigFile* cf, int section);
        ~GbxSickAcfr (void);

        virtual int Setup (void);
        virtual int Shutdown (void);
        virtual int ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data);

    private:
        virtual void Main (void);
        bool ReadLaser (void);

        // Configuration parameters
        gbxsickacfr::Config config;
        // Geometry
        player_ranger_geom_t geom;
        player_pose3d_t sensorPose;
        player_bbox3d_t sensorSize;
        // Data storage
        float *rawRanges;
        double *ranges;
        unsigned char *rawIntensities;
        double *intensities;
        gbxsickacfr::Data data;
        // The hardware device itself
        gbxsickacfr::Driver *device;
        // Objects to handle messages from the driver
        bool debug;
        gbxsickacfr::gbxutilacfr::TrivialTracer *tracer;
        gbxsickacfr::gbxutilacfr::TrivialStatus *status;
};

Driver*
GbxSickAcfr_Init (ConfigFile* cf, int section)
{
    return reinterpret_cast <Driver*> (new GbxSickAcfr (cf, section));
}

void GbxSickAcfr_Register(DriverTable* table)
{
    table->AddDriver ("gbxsickacfr", GbxSickAcfr_Init);
}

GbxSickAcfr::GbxSickAcfr (ConfigFile* cf, int section)
    : Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_RANGER_CODE),
    ranges (NULL), intensities (NULL), device (NULL), tracer (NULL), status (NULL)
{
    // Setup config object
    config.minRange = cf->ReadFloat (section, "min_range", 0.0f);
    config.maxRange = cf->ReadFloat (section, "max_range", 80.0f);
    config.fieldOfView = cf->ReadFloat (section, "field_of_view", DTOR (180.0f));
    config.startAngle = cf->ReadFloat (section, "start_angle", DTOR (-90.0f));
    config.numberOfSamples = cf->ReadInt (section, "num_samples", 181);
    config.baudRate = cf->ReadInt (section, "baudrate", 38400);
    config.device = cf->ReadString (section, "port", "/dev/ttyS0");
    debug = cf->ReadBool (section, "debug", false);
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
}

GbxSickAcfr::~GbxSickAcfr (void)
{
    if (rawRanges != NULL)
        delete[] rawRanges;
    if (ranges != NULL)
        delete[] ranges;
    if (rawIntensities != NULL)
        delete[] rawIntensities;
    if (intensities != NULL)
        delete[] intensities;
    if (device != NULL)
        delete device;
    if (status != NULL)
        delete status;
    if (tracer != NULL)
        delete tracer;
}

int GbxSickAcfr::Setup (void)
{
    // Validate the configuration
    if (!config.isValid ())
    {
        PLAYER_ERROR ("GbxSickAcfr: Invalid laser configuration.\n");
        return -1;
    }

    // Create status trackers
    tracer = new gbxsickacfr::gbxutilacfr::TrivialTracer (debug);
    status = new gbxsickacfr::gbxutilacfr::TrivialStatus (*tracer);

    // Create the driver object
    try
    {
        device = new gbxsickacfr::Driver(config, *tracer, *status);
    }
    catch (const std::exception& e)
    {
        PLAYER_ERROR1 ("GbxSickAcfr: Failed to initialise laser device: %s\n", e.what ());
        return -1;
    }

    // Create space to store data
    rawRanges = new float[config.numberOfSamples];
    ranges = new double[config.numberOfSamples];
    rawIntensities = new unsigned char[config.numberOfSamples];
    intensities = new double[config.numberOfSamples];
    if (rawRanges == NULL || ranges == NULL || rawIntensities == NULL || intensities == NULL)
    {
        PLAYER_ERROR ("GbxSickAcfr: Failed to allocate data store.\n");
        return -1;
    }
    data.ranges = rawRanges;
    data.intensities = rawIntensities;

    StartThread();
    return 0;
}

int GbxSickAcfr::Shutdown (void)
{
    StopThread();

    if (device != NULL)
    {
        delete device;
        device = NULL;
    }
    if (rawRanges != NULL)
    {
        delete[] rawRanges;
        rawRanges = NULL;
    }
    if (ranges != NULL)
    {
        delete[] ranges;
        ranges = NULL;
    }
    if (rawIntensities != NULL)
    {
        delete[] rawIntensities;
        rawIntensities = NULL;
    }
    if (intensities != NULL)
    {
        delete[] intensities;
        intensities = NULL;
    }
    if (status != NULL)
    {
        delete status;
        status = NULL;
    }
    if (tracer != NULL)
    {
        delete tracer;
        tracer = NULL;
    }

    return 0;
}

int GbxSickAcfr::ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
    // Check for capability requests
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ);
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM);
    HANDLE_CAPABILITY_REQUEST (device_addr, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG);

    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM, device_addr))
    {
        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_GEOM, &geom, sizeof (geom), NULL);
        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG, device_addr))
    {
        player_ranger_config_t rangerConfig;
        rangerConfig.min_angle = config.startAngle;
        rangerConfig.max_angle = config.startAngle + config.fieldOfView;
        rangerConfig.resolution = config.fieldOfView / static_cast<double> (config.numberOfSamples - 1);
        rangerConfig.max_range = config.maxRange;
        rangerConfig.range_res = 0.0f;
        rangerConfig.frequency = 0.0f;
        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_RANGER_REQ_GET_CONFIG, &rangerConfig, sizeof (rangerConfig), NULL);
        return 0;
    }
    return -1;
}

void GbxSickAcfr::Main (void)
{
    while (true)
    {
        pthread_testcancel ();
        ProcessMessages ();

        if (!ReadLaser ())
            break;
    }
}

bool GbxSickAcfr::ReadLaser (void)
{
    player_ranger_data_range_t rangeData;
    player_ranger_data_intns_t intensityData;

    try
    {
        device->read (data);

        rangeData.ranges_count = config.numberOfSamples;
        rangeData.ranges = ranges;
        for (int ii = 0; ii < config.numberOfSamples; ii++)
            ranges[ii] = rawRanges[ii];
        Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_RANGE, reinterpret_cast<void*> (&rangeData), sizeof (rangeData), NULL);

        intensityData.intensities_count = config.numberOfSamples;
        intensityData.intensities = intensities;
        for (int ii = 0; ii < config.numberOfSamples; ii++)
            intensities[ii] = rawIntensities[ii];
        Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_INTNS, reinterpret_cast<void*> (&intensityData), sizeof (intensityData), NULL);

        if (data.haveWarnings)
            PLAYER_WARN1 ("GbxSickAcfr: Got warnings with scan: %s\n", data.warnings.c_str ());
    }
    catch (gbxsickacfr::gbxutilacfr::Exception &e)
    {
        // No data received by the timeout; warn but go on anyway
        PLAYER_WARN ("GbxSickAcfr: Timed out while reading laser scan.\n");
    }
    catch (const std::exception &e)
    {
        PLAYER_ERROR1 ("GbxSickAcfr: Failed to read scan: %s\n", e.what ());
    }
}
