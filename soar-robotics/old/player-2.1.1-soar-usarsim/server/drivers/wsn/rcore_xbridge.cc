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
 Desc: Driver for the TeCo Particle Router Core (using the XBridge) nodes.
 Author: Radu Bogdan Rusu
 Date: 01 May 2006
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_rcore_xbridge rcore_xbridge
 * @brief TeCo Particle Router Core (XBridge) sensor node

The rcore_xbridge driver controls the TeCo Particle Router Core sensor node, 
using the XBridge. The SSimp Full board is supported.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_wsn

@par Requires

- none

@par Configuration requests

- PLAYER_WSN_REQ_POWER (to be implemented)
- PLAYER_WSN_REQ_DATATYPE
- PLAYER_WSN_REQ_DATAFREQ (to be implemented)

@par Configuration file options

- port (integer)
  - Default: 5555
  - TCP port to which the XBridge data gets broadcasted.

- node (integer tupple)
  - These are the calibration values for -1G/+1G for the ADXL210 accelerometer
    sensor (see the appropriate data sheet on how to obtain it). Each sepparate
    board *MUST* be calibrated! Only usable when requesting data in converted
    metric units mode.
  - The tuple means: [node_id
                      group_id
                      calibration_negative_1g_x_axis
                      calibration_positive_1g_x_axis
                      calibration_negative_1g_y_axis
                      calibration_positive_1g_y_axis
                      calibration_negative_1g_z_axis
                      calibration_positive_1g_z_axis
                     ]

- converted (integer)
  - Default: 1. Possible values: 0, 1, 2. (0=RAW, 1=m/s^2, 2=G)
  - Fill the data buffer with converted engineering units (1,2) or RAW (0) values.

- readppacket (integer)
  - Default: 8.
  - How many readings does the Particle send in one packet? (using multiple readings 
    per packet, will increase the sample rate). Note: Use 0 as a special value for 
    enabling the standard TeCo SSIMP mode (multiple tuples).

@par Example 

@verbatim
driver
(
  name "rcore_xbridge"
  provides ["wsn:0"]
  port 5555
  # Calibrate node 0 from group 125 (default) with X={419,532} and Y={440,552}
  node [0 125 419 532 440 552 0 0]
  # Calibrate node 2 from group 125 (default) with X={447,557} and Y={410,520}
  node [2 125 447 557 410 520 0 0]
  # Use converted engineering units (G)
  converted 2
)
@endverbatim

@author Radu Bogdan Rusu

*/
/** @} */

#ifdef __cplusplus
extern "C" {
#endif

#include <libparticle.h>

#ifdef __cplusplus
}
#endif

#include "rcore_xbridge.h"

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

// The RCore_XBridge device class.
class RCore_XBridge : public Driver
{
    public:
        // Constructor
        RCore_XBridge (ConfigFile* cf, int section);

        // Destructor
        ~RCore_XBridge ();

        // Implementations of virtual functions
        int Setup ();
        int Shutdown ();

        // This method will be invoked on each incoming message
        virtual int ProcessMessage (QueuePointer &resp_queue, 
                                    player_msghdr * hdr,
                                    void * data);
    private:

        // Main function for device thread.
        virtual void Main ();
        void RefreshData  ();

        // Port file descriptor
        int               fd;

        // Does the user want RAW or converted values?
        int               raw_or_converted;
	
	// How many readings per packet does the sensor node sends ?
	int               readppacket;

        // WSN interface
        player_wsn_data_t data;
        player_wsn_cmd_t  cmd;

        int               port_number;
        int               filter;
        int               sockd;

        // Calibration values
        int               nodes_count;
        NCV               ncv;

        // Calibration values
        int               calibration_values[6];
        // Calibration node ID
        int               calibration_node_id;

        NodeCalibrationValues FindNodeValues (unsigned int nodeID);
        short ParseTuple (unsigned char b1, unsigned char b2);
        player_wsn_data_t DecodePacket (struct p_packet *pkt);
        float ConvertAccel (unsigned short raw_accel, int neg_1g, int pos_1g,
                            int converted);
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* RCore_XBridge_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
    return ((Driver*)(new RCore_XBridge (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void RCore_XBridge_Register (DriverTable* table)
{
    table->AddDriver ("rcore_xbridge", RCore_XBridge_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
RCore_XBridge::RCore_XBridge (ConfigFile* cf, int section)
	: Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
              PLAYER_WSN_CODE)
{
    int i = 0;
    int j = 0;
	
    port_number = cf->ReadInt (section, "port", DEFAULT_XBRIDGE_PORT);
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
    raw_or_converted = cf->ReadInt (section, "converted", 1);

    // How many readings per packet does the sensor node sends ?
    readppacket = cf->ReadInt (section, "readppacket", 8);
    
    return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
RCore_XBridge::~RCore_XBridge()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int RCore_XBridge::Setup ()
{
    // Create a filter and open the socket
    filter = p_filter_create ("filter");
    sockd  = p_socket_open   (0, 0, port_number);

    // Set socket options
    p_socket_set_option (sockd, SOCKET_RECV_ALL, 1);
    p_socket_set_option (sockd, SOCKET_AUTOACK,  0);

    PLAYER_MSG0 (1, "> RCore_XBridge driver initializing... [done]");

    // Start the device thread
    StartThread ();

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int RCore_XBridge::Shutdown ()
{
    // Stop the driver thread
    StopThread ();

    // Close the Particle socket
    p_socket_close (sockd);
	
    PLAYER_MSG0 (1, "> RCore_XBridge driver shutting down... [done]");
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void RCore_XBridge::Main () 
{
    // Zero data
    memset (&data, 0, sizeof (player_wsn_data_t));
	
    timespec sleepTime = {0, 0};
	
    // The main loop; interact with the device here
    while (true)
    {
        // test if we are supposed to cancel
        pthread_testcancel ();

        // Process any pending messages
        ProcessMessages();

        RefreshData ();

        nanosleep (&sleepTime, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage function
int RCore_XBridge::ProcessMessage (QueuePointer &resp_queue, 
                           player_msghdr * hdr,
                           void * data)
{	
    assert (hdr);
    assert (data);
	
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
        PLAYER_WSN_REQ_POWER, device_addr))
    {
        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
             PLAYER_WSN_REQ_DATATYPE, device_addr))
    {
	// Change the data type to RAW or converted metric units
        player_wsn_datatype_config *datatype = 
                (player_wsn_datatype_config*)data;
	unsigned int val = datatype->value;
	
        if ((val >= 0) && (val < 3))
        {
            raw_or_converted = val;
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
                     hdr->subtype);
        }
        else
            Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
                     hdr->subtype);

        return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
             PLAYER_WSN_REQ_DATAFREQ, device_addr))
    {
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
void RCore_XBridge::RefreshData ()
{
    memset (&data, 0, sizeof (player_wsn_data_t));
	
    // Get the time at which we started reading
    // This will be a pretty good estimate of when the phenomena occured
    struct timeval time;
    GlobalTime->GetTime(&time);
	
    // Get the TCP packet
    packet = p_socket_recv (sockd, sockd);
    if (packet == NULL)
        return;

    // Decode and publish the data
    data = DecodePacket (packet);

    return;
}

////////////////////////////////////////////////////////////////////////////////
// ParseTuple function - return the value of a tuple
short RCore_XBridge::ParseTuple (unsigned char b1, unsigned char b2)
{
    long value;
    value  = b1 << 8;
    value |= b2;

    return (short)value;
}

////////////////////////////////////////////////////////////////////////////////
// DecodePacket function - decode a TeCo Particle packet
player_wsn_data_t RCore_XBridge::DecodePacket (struct p_packet *pkt)
{
    NodeCalibrationValues node_values;
    player_wsn_data_t temp_data;
    short accelX[MAXREADPPACKET], accelY[MAXREADPPACKET], accelZ[MAXREADPPACKET];
    int i;
    const uint8_t *srcid = p_pkt_get_srcid(pkt);
    struct p_acl_tuple *tuple;
    uint8_t *acl_data;
    const uint8_t *acl_type;
    uint16_t acl_len;
    
    char gid[12];
    char nid[12];
    sprintf (gid, "%d%d%d%d", srcid[0], srcid[1], srcid[2], srcid[3] );
    sprintf (nid, "%d%d%d%d", srcid[4], srcid[5], srcid[6], srcid[7] );

    temp_data.node_type      = 1;
    temp_data.node_id        = atol (nid);
    temp_data.node_parent_id = atol (gid);

    tuple = p_acl_first(pkt);

    // Standard TeCO SSIMP software includes the following tuples:
    // sgx, sgy, sgz = acceleration values for X, Y and Z axis
    // sli = sensor light
    // ste = sensor temperatures
    // svc = sensor voltage
    temp_data.data_packet.light       = -1;
    temp_data.data_packet.mic         = -1;
    temp_data.data_packet.magn_x      = -1;
    temp_data.data_packet.magn_y      = -1;
    temp_data.data_packet.magn_z      = -1;
    temp_data.data_packet.temperature = -1;
    temp_data.data_packet.battery     = -1;
    
    // Parse all the tuples
    for (tuple = p_acl_first (pkt); tuple != NULL; tuple = p_acl_next (pkt, tuple))
    {
      acl_type = p_acl_get_type (tuple);
      acl_len  = p_acl_get_data (tuple, &acl_data);
      
      if (readppacket == 0)	// Assume normal, standard SSIMP mode
      {
        if ((acl_type[0] == 234) && (acl_type[1] == 128))	// SGX
	    temp_data.data_packet.accel_x = ParseTuple (acl_data[0], acl_data[1]);
        if ((acl_type[0] == 240) && (acl_type[1] == 192))	// SGY
    	    temp_data.data_packet.accel_y = ParseTuple (acl_data[0], acl_data[1]);
        if ((acl_type[0] == 247) && (acl_type[1] == 0))		// SGZ
            temp_data.data_packet.accel_z = ParseTuple (acl_data[0], acl_data[1]);
        if ((acl_type[0] == 141) && (acl_type[1] == 136))	// SLI
    	    temp_data.data_packet.light = acl_data[1];
	if ((acl_type[0] == 117) && (acl_type[1] == 200))	// STE
	    temp_data.data_packet.temperature = acl_data[0];
	if ((acl_type[0] == 105) && (acl_type[1] == 152))	// SVC
	    temp_data.data_packet.battery = ParseTuple (acl_data[0], acl_data[1]);
	if ((acl_type[0] == 214) && (acl_type[1] == 208))	// SAU
	    temp_data.data_packet.mic = acl_data[1];
      }
      else			// Using a single tuple, defaulting to SGX
        if ((acl_type[0] == 234) && (acl_type[1] == 128)) // SGX
    	    for (i = 0; i < readppacket; i++)
    	    {
        	accelX[i] = ParseTuple (acl_data[0+(i*6)], acl_data[1+(i*6)]);
        	accelY[i] = ParseTuple (acl_data[2+(i*6)], acl_data[3+(i*6)]);
        	accelZ[i] = ParseTuple (acl_data[4+(i*6)], acl_data[5+(i*6)]);
    	    }
    }

    // If multiple tuples/single packet mode enabled...
    if (readppacket == 0)
	// Publish the WSN data
	Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_WSN_DATA_STATE,
	         &temp_data, sizeof (player_wsn_data_t), NULL);
    else
	// If single tuple/multiple packets mode enabled...
	for (i = 0; i < readppacket; i++)
	{
	    if (raw_or_converted != 0)
    	    {
		node_values = FindNodeValues (temp_data.node_id);
    		temp_data.data_packet.accel_x = ConvertAccel (accelX[i], 
        	    node_values.c_values[0], node_values.c_values[1], 
            	    raw_or_converted);
        	temp_data.data_packet.accel_y = ConvertAccel (accelY[i], 
	            node_values.c_values[2], node_values.c_values[3],
    	            raw_or_converted);
		temp_data.data_packet.accel_z = ConvertAccel (accelZ[i], 
        	    node_values.c_values[4], node_values.c_values[5],
            	    raw_or_converted);
	    }
    	    else
	    {
		temp_data.data_packet.accel_x = accelX[i];
    		temp_data.data_packet.accel_y = accelY[i];
        	temp_data.data_packet.accel_z = accelZ[i];
    	    }
    
	    // Publish the WSN data (each packet goes separately)
	    Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_WSN_DATA_STATE,
	         &temp_data, sizeof (player_wsn_data_t), NULL);
	}

    p_pkt_free (packet);
    return temp_data;
}
////////////////////////////////////////////////////////////////////////////////
// FindNodeValues function - find the appropriate calibration values for nodeID
NodeCalibrationValues RCore_XBridge::FindNodeValues (unsigned int nodeID)
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

////////////////////////////////////////////////////////////////////////////////
// ConvertAccel function - convert RAW accel. data to metric units (m/s^2)
float RCore_XBridge::ConvertAccel (unsigned short raw_accel, 
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
//------------------------------------------------------------------------------
