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
 Desc: Driver for connecting to a YARP server and getting an Image.
 Author: Radu Bogdan Rusu
 Date: 21 Jul 2006
*/
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_yarp_image yarpimage
 * @brief Driver for connecting to a YARP server and getting an Image.

The yarpimage driver connects to a YARP server using a specified port 
name, gets image data, and puts it on a Player camera interface.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_camera

@par Requires

- none

@par Configuration requests

- none yet

@par Configuration file options

- carrier (string)
  - Default: tcp
  - Type of carrier. Possible values: tcp, udp, mcast, shmem.

- port (string)
  - Default: NULL
  - Name of the internally created YARP port for our driver.

- image_port (string)
  - Default: NULL
  - Name of the YARP port that we want to connect to, to get images.

- image_format (integer)
  - Default: 5 (PLAYER_CAMERA_FORMAT_RGB888)
  - Possible values: 1 (PLAYER_CAMERA_FORMAT_MONO8 - 8-bit monochrome) or 
                     5 (PLAYER_CAMERA_FORMAT_RGB888 - 24-bit color).
                     Anything else will be ignored and defaulted.
  - Format of the image to provide.

@par Example 

@verbatim
driver
(
  name "yarpimage"
  provides ["camera:0"]
  # Set the port name
  carrier "tcp"
  image_port "/images"
  port "/player"
  image_format 1
)
@endverbatim

@author Radu Bogdan Rusu

 */
/** @} */

#include <unistd.h>
#include <string.h>
#include <vector>
#include <libplayercore/playercore.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>

using namespace yarp::os;
using namespace yarp::sig;

////////////////////////////////////////////////////////////////////////////////
// The Yarp_Image device class.
class Yarp_Image : public Driver
{
    public:
        // Constructor
        Yarp_Image (ConfigFile* cf, int section);

        // Destructor
        ~Yarp_Image ();

        // Implementations of virtual functions
        virtual int Setup ();
        virtual int Shutdown ();

        // Camera interface (provides)
        player_devaddr_t         cam_id;
        player_camera_data_t     cam_data;
    private:

        // Main function for device thread.
        virtual void Main ();
        virtual void RefreshData  ();

        const char* portName;
        const char* imagePortName;
        const char* carrier;
        int         imageFormat;

        BufferedPort<ImageOf<PixelRgb> > portIn;
};

////////////////////////////////////////////////////////////////////////////////
// Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* Yarp_Image_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
    return ((Driver*)(new Yarp_Image (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void Yarp_Image_Register (DriverTable* table)
{
    table->AddDriver ("yarpimage", Yarp_Image_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Yarp_Image::Yarp_Image (ConfigFile* cf, int section)
    : Driver (cf, section)
{
    memset (&this->cam_id, 0, sizeof (player_devaddr_t));

    // Outgoing camera interface
    if(cf->ReadDeviceAddr(&(this->cam_id), section, "provides",
       PLAYER_CAMERA_CODE, -1, NULL) == 0)
    {
        if(this->AddInterface(this->cam_id) != 0)
        {
            this->SetError(-1);
            return;
        }
    }

    // Carrier type (udp, tcp, mcast, etc)
    this->carrier = cf->ReadString (section, "carrier", "tcp");

    // Name of the internal port created.
    this->portName = cf->ReadString (section, "port", NULL);

    // Name of the port on which we have to connect to, to get images.
    this->imagePortName = cf->ReadString (section, "image_port", NULL);

    // Name of the port on which we have to connect to, to get images.
    this->imageFormat = cf->ReadInt
            (section, "image_format", PLAYER_CAMERA_FORMAT_RGB888);

    // Initialize all YARP network-related stuff
    Network::init ();

    return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
Yarp_Image::~Yarp_Image()
{
    // Deinitialize all YARP network-related stuff
    Network::fini ();

    return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Yarp_Image::Setup ()
{
    // Open a local port
    portIn.open (portName);

    // Connect the two ports
    Network::connect (imagePortName, portName, carrier);
    printf ("carrier is %s\n", carrier);

    PLAYER_MSG0 (1, "> Yarp_Image starting up... [done]");

    // Start the device thread
    StartThread ();

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Yarp_Image::Shutdown ()
{
    // Stop the driver thread
    StopThread ();

    // Disconnect the two ports
    Network::disconnect (imagePortName, portName);

    // Close the local port
    portIn.close ();

    PLAYER_MSG0 (1, "> Yarp_Image driver shutting down... [done]");
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Yarp_Image::Main () 
{
    memset (&cam_data, 0, sizeof (cam_data));

    timespec sleepTime = {0, 0};

    // modify the scheduling priority
    //nice (10);

    // The main loop; interact with the device here
    while (true)
    {
        nanosleep (&sleepTime, NULL);

        // test if we are supposed to cancel
        pthread_testcancel ();

        // Refresh data
        this->RefreshData ();
    }
}

void Yarp_Image::RefreshData ()
{
    ImageOf<PixelRgb> *imgIn = portIn.read (true);
    if (imgIn!=0)
    {
        cam_data.width  = imgIn->width ();
        cam_data.height = imgIn->height ();
        switch (imageFormat) {
            case PLAYER_CAMERA_FORMAT_MONO8:
            {
                cam_data.format = PLAYER_CAMERA_FORMAT_MONO8;
                cam_data.image_count = imgIn->getRawImageSize () / 3;
                cam_data.image = new unsigned char [cam_data.image_count];

                for (int i=0; i <= (imgIn->getRawImageSize ()/3) ; i++) {
                    unsigned char red   = *(unsigned char *)
                            (imgIn->getRawImage()+i*3);
                    unsigned char green = *(unsigned char *)
                            (imgIn->getRawImage()+i*3+1);
                    unsigned char blue  = *(unsigned char *)
                            (imgIn->getRawImage()+i*3+2);
                    cam_data.image[i] = static_cast<uint8_t>
                            (red*0.299 + green*0.587 + blue*0.114);
                }
                break;
            }
            case PLAYER_CAMERA_FORMAT_RGB888:
            {
                cam_data.format = PLAYER_CAMERA_FORMAT_RGB888;
                cam_data.image_count = imgIn->getRawImageSize ();
                cam_data.image = new unsigned char [cam_data.image_count];
                for (int i=0; i <= (imgIn->getRawImageSize ()) ; i++) {
                    unsigned char value = *(unsigned char *)
                            (imgIn->getRawImage ()+i);
                    cam_data.image[i] = static_cast<uint8_t> (value);
                }

                break;
            }
        }
        Publish (this->cam_id, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
                 &cam_data);
        delete [] cam_data.image;

    }
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
#if 0
extern "C" {
    int player_driver_init (DriverTable* table)
    {
        Yarp_Image_Register (table);
        return(0);
    }
}
#endif
