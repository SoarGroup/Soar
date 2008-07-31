/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 - Radu Bogdan Rusu (rusu@cs.tum.edu)
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
/** @defgroup driver_laserptzcloud laserptzcloud
 * @brief Build a 3D point cloud from laser and ptz data

The laserptztcloud driver reads laser scans from a laser device and PTZ poses 
from a ptz device, linearly interpolates to estimate the actual pan/tilt pose 
from which the scan was taken, then outputs messages containing the cartesian 
3D coordinates (X,Y,Z in [m]) via a pointcloud3d interface. No additional 
thread is started. Based on Brian's laserposerinterpolator.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_pointcloud3d

@par Requires

- @ref interface_laser
- @ref interface_ptz

@par Configuration requests

- None (yet)

@par Configuration file options

@par Example 

@verbatim
driver
(
  name "laserptzcloud"
  provides ["pointcloud3d:0"]
  requires ["laser:0" "ptz:0"]
)
@endverbatim

@author Radu Bogdan Rusu

 */
/** @} */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <assert.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#define DEFAULT_MAXSCANS    100
#define DEFAULT_MAXPOINTS   1024
#define DEFAULT_MAXDISTANCE 10

// PTZ defaults for tilt
#define PTZ_PAN  0
#define PTZ_TILT 1
#define DEFAULT_PTZ_PAN_OR_TILT PTZ_TILT

// The laser device class.
class LaserPTZCloud : public Driver
{
    public:
        // Constructor
        LaserPTZCloud (ConfigFile* cf, int section);
        ~LaserPTZCloud ();

        int Setup();
        int Shutdown();

        // MessageHandler
        int ProcessMessage (QueuePointer &resp_queue, 
                            player_msghdr * hdr, 
                            void * data);
    private:

        // device bookkeeping
        player_devaddr_t laser_addr;
        player_devaddr_t ptz_addr;
        Device*          laser_device;
        Device*          ptz_device;

        // Laser scans
        player_laser_data_t* scans;
        // Laser timestamps
        double* scantimes;
        // Maximum number of laser scans to buffer
        int maxnumscans;
        // Total number of laser scans
        int numscans;

        // 3D points buffer
        player_point_3d_t* points;
        // Maximum number of poins in a graphics3d data packet
        int maxpoints;

        // Maximum distance that we should consider from the laser
        float maxdistance;

        // PTZ tilt parameters
        float ptz_pan_or_tilt;

        // Timeouts, delays
        float delay;

        // First PTZ pose
        player_ptz_data_t lastpose;
        double            lastposetime;
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* LaserPTZCloud_Init (ConfigFile* cf, int section)
{
    return ((Driver*)(new LaserPTZCloud (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void LaserPTZCloud_Register (DriverTable* table)
{
    table->AddDriver ("laserptzcloud", LaserPTZCloud_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
LaserPTZCloud::LaserPTZCloud (ConfigFile* cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
             PLAYER_POINTCLOUD3D_CODE)
{
    // Must have an input laser
    if (cf->ReadDeviceAddr (&this->laser_addr, section, "requires",
        PLAYER_LASER_CODE, -1, NULL) != 0)
    {
        PLAYER_ERROR ("must have an input laser");
        this->SetError (-1);
        return;
    }
    this->laser_device = NULL;

    // Must have an input ptz
    if (cf->ReadDeviceAddr (&this->ptz_addr, section, "requires",
        PLAYER_PTZ_CODE, -1, NULL) != 0)
    {
        PLAYER_ERROR ("must have an input ptz");
        this->SetError (-1);
        return;
    }
    this->ptz_device = NULL;

    // ---[ PTZ parameters ]---
    this->ptz_pan_or_tilt = cf->ReadFloat 
            (section, "ptz_pan_or_tilt", DEFAULT_PTZ_PAN_OR_TILT);

    // Maximum number of laser scans to buffer
    this->maxnumscans = cf->ReadInt (section, "max_scans", DEFAULT_MAXSCANS);

    // Maximum allowed distance
    this->maxdistance = cf->ReadFloat (section, "max_distance", DEFAULT_MAXDISTANCE);

    // Maximum number of points that can be sent at once
    this->maxpoints   = cf->ReadInt (section, "max_points", DEFAULT_MAXPOINTS);
    if (this->maxpoints > DEFAULT_MAXPOINTS)
    {
        maxpoints = MIN (maxpoints, DEFAULT_MAXPOINTS);
        PLAYER_WARN1 ("number of points cannot exceeded MAXPOINTS (%d)", 
                      DEFAULT_MAXPOINTS);
    }

    // Allocate memory for the buffer
    this->scans = (player_laser_data_t*)calloc 
	(this->maxnumscans, sizeof (player_laser_data_t));
    assert (this->scans);
    // Allocate memory for the laser timestamps
    this->scantimes = (double*)calloc (this->maxnumscans, sizeof (double));
    assert (this->scantimes);

    // Allocate memory for the points buffer
    this->points = (player_point_3d_t*)calloc 
        (this->maxnumscans, sizeof (player_point_3d_t));
    return;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor.
LaserPTZCloud::~LaserPTZCloud()
{
    free (this->scans);
    free (this->scantimes);
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int LaserPTZCloud::Setup()
{
    // Subscribe to the laser
    if (!(this->laser_device = deviceTable->GetDevice (this->laser_addr)))
    {
        PLAYER_ERROR ("unable to locate suitable laser device");
        return (-1);
    }
    if (this->laser_device->Subscribe (this->InQueue) != 0)
    {
        PLAYER_ERROR ("unable to subscribe to laser device");
        return (-1);
    }

    // Subscribe to the ptz.
    if (!(this->ptz_device = deviceTable->GetDevice (this->ptz_addr)))
    {
        PLAYER_ERROR ("unable to locate suitable ptz device");
        return (-1);
    }
    if (this->ptz_device->Subscribe (this->InQueue) != 0)
    {
        PLAYER_ERROR ("unable to subscribe to ptz device");
        return (-1);
    }

    this->numscans     = 0;
    this->lastposetime = -1;
    return (0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int LaserPTZCloud::Shutdown()
{
    this->laser_device->Unsubscribe (this->InQueue);
    this->ptz_device->Unsubscribe   (this->InQueue);
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage
int LaserPTZCloud::ProcessMessage (QueuePointer &resp_queue, 
                                          player_msghdr * hdr,
                                          void * data)
{
    // Is it a laser scan?
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA,
	PLAYER_LASER_DATA_SCAN, 
        this->laser_addr))
    {
        // Buffer the scan
        // is there room?
        if (this->numscans >= this->maxnumscans)
        {
            PLAYER_WARN1 ("exceeded maximum number of scans to buffer (%d)",
                          this->maxnumscans);
            return (0);
        }
        // store the scan and timestamp
        this->scans[this->numscans]     = *((player_laser_data_t*)data);
        this->scantimes[this->numscans] = hdr->timestamp;
        this->numscans++;
        return (0);
    }
    // Is it a ptz pose?
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, 
             PLAYER_PTZ_DATA_STATE, this->ptz_addr))
    {
        player_ptz_data_t newpose = *((player_ptz_data_t*)data);
        // Is it the first pose?
        if (this->lastposetime < 0)
        {
            // Just store it.
            this->lastpose     = newpose;
            this->lastposetime = hdr->timestamp;
        }
        else
        {
            // Interpolate pose for all buffered scans and send them out
            double t1 = hdr->timestamp - this->lastposetime;

            if (newpose.tilt != lastpose.tilt)
                for (int i = 0; i < this->numscans; i++)
        	{
            	    double t0 = this->scantimes[i] - this->lastposetime;

            	    float corrected_tilt = this->lastpose.tilt + t0 * 
                        (newpose.tilt - this->lastpose.tilt) / t1;

            	    // Convert the vertical angle to radians
            	    //float angle_y = corrected_tilt * M_PI/180.0;
		    // No need to: already converted from PTZ
            	    float angle_y = corrected_tilt;

            	    // Calculate the horizontal angles and the cartesian coordinates
            	    float angle_x    = this->scans[i].min_angle;
            	    float resolution = this->scans[i].resolution;

            	    int ranges_count = (int)(this->scans[i].ranges_count);
		
            	    // The 3D point array
            	    player_pointcloud3d_data_t cloud_data;
            	    player_pointcloud3d_element_t all_elements[ranges_count];

            	    int counter = 0;
            	    for (int j = 0; j < ranges_count; j++)
            	    {
                	float distance = this->scans[i].ranges[j];
                	if (distance < maxdistance)
                	{
                    	    float X = distance * cos (angle_x) * sin (angle_y);
                    	    float Y = distance * cos (angle_x) * cos (angle_y);
                    	    float Z = distance * sin (angle_x);

                    	    player_point_3d_t p3d;
                    	    p3d.px = X;
                    	    p3d.py = Y;
                    	    p3d.pz = Z;
                    	    all_elements[counter].point = p3d;
                    	    counter++;
                	}
                	angle_x += resolution;
            	    }
		
            	    cloud_data.points_count = counter;
            	    cloud_data.points = (player_pointcloud3d_element_t*)calloc(sizeof(cloud_data.points[0]),cloud_data.points_count);
            	    for (int j=0; j < counter; j++)
                	cloud_data.points[j] = all_elements[j];
		
            	    Publish (this->device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_POINTCLOUD3D_DATA_STATE, &cloud_data, 
                         sizeof (player_pointcloud3d_data_t), NULL);
            	    free(cloud_data.points);
        	}
            this->numscans     = 0;
            this->lastpose     = newpose;
            this->lastposetime = hdr->timestamp;
        }
        return(0);
    }
    // Don't know how to handle this message.
    return (-1);
}

