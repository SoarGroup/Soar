/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Andrew Howard
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
 * Desc: Driver generating dummy data
 * Authors: Andrew Howard, Radu Bogdan Rusu
 * Date: 15 Sep 2004
 * CVS: $Id: dummy.cc 6565 2008-06-14 00:03:05Z thjc $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_dummy dummy
 * @brief Dummy driver

The dummy driver generates dummy data and consumes dummy commands for
any interface; useful for debugging client libraries and benchmarking
server performance.

@par Compile-time dependencies

- none

@par Provides

- This driver can support any interface (currently supported are: laser, camera,
  position2d, ptz, and wsn).

@par Requires

- none

@par Configuration requests

- This driver will consume any configuration requests.

@par Configuration file options

  - rate (float)
  - Default: 10
  - Data rate (Hz); e.g., rate 20 will generate data at 20Hz.

@par Example 

@verbatim
driver
(
  name "dummy"
  provides ["laser:0"]  # Generate dummy laser data
  rate 75               # Generate data at 75Hz
)
@endverbatim

@author Andrew Howard, Radu Bogdan Rusu
*/
/** @} */

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // required for in.h on OS X
#include <netinet/in.h>

// Includes needed for player
#include <libplayercore/playercore.h>

// The Dummy driver
class Dummy: public Driver 
{
    public:
        // Constructor
        Dummy (ConfigFile* cf, int section);

        // Destructor
        ~Dummy ();

        // Implementations of virtual functions
        virtual int Setup ();
        virtual int Shutdown ();

    private:

        // Main function for device thread.
        virtual void Main ();

        // Data rate
        double rate;
};



////////////////////////////////////////////////////////////////////////////////
// Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* Dummy_Init(ConfigFile* cf, int section)
{
    return ((Driver*) (new Dummy(cf, section)));
}


////////////////////////////////////////////////////////////////////////////
// Device factory registration
void Dummy_Register(DriverTable* table)
{
    table->AddDriver("dummy", Dummy_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Dummy::Dummy(ConfigFile* cf, int section)
	: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
    // Look for our default device id
    if (cf->ReadDeviceAddr(&this->device_addr, section, "provides", 
                        0, -1, NULL) != 0)
    {
        this->SetError(-1);
        return;
    }

    // Add our interface
    if (this->AddInterface(this->device_addr) != 0)
    {
        this->SetError(-1);
        return;
    }

    // Data rate
    this->rate = cf->ReadFloat(section, "rate", 10);

    return;
}


////////////////////////////////////////////////////////////////////////////
// Destructor
Dummy::~Dummy()
{
    return;
}


////////////////////////////////////////////////////////////////////////////
// Initialize driver
int Dummy::Setup()
{
    // Start device thread
    this->StartThread();

    return 0;
}


////////////////////////////////////////////////////////////////////////////
// Finalize the driver
int Dummy::Shutdown()
{
    // Stop the device thread
    this->StopThread();

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Dummy::Main(void)
{
    unsigned int i = 0;
    struct timespec req;
    //  void *client;

    req.tv_sec = (time_t) (1.0 / this->rate);
    req.tv_nsec = (long) (fmod(1e9 / this->rate, 1e9));

    while (1)
    {
        pthread_testcancel();
        if (nanosleep(&req, NULL) == -1)
        continue;

//    ProcessMessages();
    // Process pending configuration requests
/*    while (this->GetConfig(this->local_id, &client, this->req_buffer,
    PLAYER_MAX_REQREP_SIZE, NULL))
    {
    if (this->PutReply(this->local_id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
}*/

        // Write data
        switch (this->device_addr.interf)
        {
            case PLAYER_CAMERA_CODE:
            {
                player_camera_data_t data;
                int w = 320;
                int h = 240;

                data.width = w;
                data.height = h;
                data.bpp = 24;
                data.format = PLAYER_CAMERA_FORMAT_RGB888;
                data.compression = PLAYER_CAMERA_COMPRESS_RAW;
                data.image_count = w * h * 3;

                for (int j = 0; j < h; j++)
                {
                    for (int i = 0; i < w; i++)
                    {
                        data.image[(i + j * w) * 3 + 0] = ((i + j) % 2) * 255;
                        data.image[(i + j * w) * 3 + 1] = ((i + j) % 2) * 255;
                        data.image[(i + j * w) * 3 + 2] = ((i + j) % 2) * 255;
                    }
                }

                int data_len = sizeof(data) - sizeof(data.image) + w * h * 3;

                Publish (device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_CAMERA_DATA_STATE, (void*)&data, data_len, 
                         NULL);
                break;
            }
            case PLAYER_LASER_CODE:
            {
                // Bogus data borrowed from Stage
                player_laser_data_t data;
                data.min_angle  = -1.5707964;
                data.max_angle  = 1.5707964;
                data.resolution = .5 * M_PI/180;
                data.max_range  = 8.0;
                data.ranges_count    = 361;
                data.intensity_count = 361;
                data.ranges = (float *) 
                              malloc( data.ranges_count * sizeof(float) );

                data.intensity = (uint8_t *) 
                              malloc( data.ranges_count * sizeof(uint8_t) );
                for (i = 0; i < data.ranges_count; i++)
                {
                    data.ranges[i]    = data.max_range;
                    data.intensity[i] = 1;
                }
                data.id = 1;

                Publish (device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_LASER_DATA_SCAN, (void*)&data, sizeof(data), 
                         NULL);
                free(data.ranges);
                free(data.intensity);
                break;
            }
            case PLAYER_POSITION2D_CODE:
            {
                player_position2d_data_t data;
                data.pos.px = 1.0;
                data.pos.py = 1.0;
                data.pos.pa = 1.0;
                data.vel.px = 1.0;
                data.vel.py = 1.0;
                data.vel.pa = 1.0;
                data.stall  = 0;
                Publish (device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_POSITION2D_DATA_STATE, (void*)&data, 
                         sizeof (data), NULL);
                break;
            }
            case PLAYER_PTZ_CODE:
            {
                player_ptz_data_t data;
                data.pan  = 1.0;
                data.tilt = 1.0;
                data.zoom = 1.0;
                data.panspeed  = 1.0;
                data.tiltspeed = 1.0;
                Publish (device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_PTZ_DATA_STATE, (void*)&data, 
                         sizeof (data), NULL);
                break;
            }
            case PLAYER_WSN_CODE:
            {
                player_wsn_data_t data;
                data.node_type      = 132;
                data.node_id        = 1;
                data.node_parent_id = 125;
                // Fill in the data packet with bogus values
                data.data_packet.light   = 779;
                data.data_packet.mic     = 495;
                data.data_packet.accel_x = 500;
                data.data_packet.accel_y = 500;
                data.data_packet.accel_z = 500;
                data.data_packet.magn_x  = 224;
                data.data_packet.magn_y  = 224;
                data.data_packet.magn_z  = 224;
                data.data_packet.temperature = 500;
                data.data_packet.battery = 489;

                Publish (device_addr, PLAYER_MSGTYPE_DATA, 
                         PLAYER_WSN_DATA_STATE, (void*)&data, 
                         sizeof (data), NULL);
                break;
            }
        }
    }
    return;
}
