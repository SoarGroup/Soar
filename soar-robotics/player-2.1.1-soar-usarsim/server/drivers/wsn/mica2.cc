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
 Desc: Driver for the Crossbow Mica2/Mica2Dot mote
 Author: Radu Bogdan Rusu
 Date: 09 Mar 2006
 Portions borrowed from the TinyOS project (http://www.tinyos.net), distributed
 according to the Intel Open Source License.
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_mica2 mica2
 * @brief Crossbow Mica2/Mica2Dot mote sensor node

The mica2 driver controls the Crossbow Mica2/Mica2DOT mote sensor node. The 
MTS310 and MTS510 boards are supported, as well as the M1-mini RFID reader 
board.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_wsn
- @ref interface_rfid

@par Requires

- none

@par Configuration requests

- PLAYER_WSN_REQ_POWER
- PLAYER_WSN_REQ_DATATYPE
- PLAYER_WSN_REQ_DATAFREQ
- PLAYER_RFID_REQ_READTAG
- PLAYER_RFID_REQ_WRITETAG

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port to which the Crossbow MIB510 is attached.  If you are
    using a USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- speed (integer)
  - Default: 57600
  - Baud rate. Valid values are 19200 for MICA2DOT and 57600 for MICA2.

- node (integer tupple)
  - These are the calibration values for -1G/+1G for the ADXL202JE accelerometer
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
  - Default: 1.
  - Fill the data buffer with converted engineering units (1) or RAW (0) values.

- filterbasenode (integer)
  - Default: 0.
  - Filter the base node (ID 0) in case there is another application != TOSBase
    installed on it.
  
@par Example 

@verbatim
driver
(
  name "mica2"
  provides ["wsn:0" "rfid:0"]
  port "/dev/ttyS0"
  speed "57600"
  # Calibrate node 0 from group 125 (default) with X={419,532} and Y={440,552}
  node [0 125 419 532 440 552 0 0]
  # Calibrate node 2 from group 125 (default) with X={447,557} and Y={410,520}
  node [2 125 447 557 410 520 0 0]
  # Use RAW values
  converted 0
  # Filter the base node (in case TOSBase is not installed on it)
  filterbasenode 1
)
@endverbatim

@author Radu Bogdan Rusu

*/
/** @} */

#include "mica2.h"

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

// The Mica2 device class.
class Mica2 : public Driver
{
    public:
    	// Constructor
	Mica2 (ConfigFile* cf, int section);

	// Destructor
	~Mica2 ();

	// Implementations of virtual functions
	virtual int Setup       ();
	virtual int Shutdown    ();
	virtual int Subscribe   (player_devaddr_t id);
	virtual int Unsubscribe (player_devaddr_t id);

	// This method will be invoked on each incoming message
	virtual int ProcessMessage (QueuePointer & resp_queue, 
				    player_msghdr * hdr,
				    void * data);
    private:

	// Main function for device thread.
	virtual void Main ();
	void RefreshData  ();

	// Port file descriptor
	int                fd;

	// Does the user want RAW or converted values?
	int                raw_or_converted;

	// Is the base node awake or sleeping?
	int                base_node_status;

	// Interfaces that we might be using
	// WSN interface
	player_devaddr_t   wsn_addr;
	player_wsn_cmd_t   wsn_cmd;
	bool               provideWSN;
	int                wsn_subscriptions;

	// RFID interface
	player_devaddr_t   rfid_addr;
	player_rfid_data_t  rfid_cmd;
	bool               provideRFID;
	int                rfid_subscriptions;

	const char*        port_name;
	int                port_speed;

	// Filter base node from readings ?
	int                filterbasenode;

	// Calibration values
	int                nodes_count;
	NCV                ncv;
		
	// Calibration values
	int                calibration_values[6];
	// Calibration node ID
	int                calibration_node_id;
		
	int ReadSerial   (unsigned char *buffer);
	int WriteSerial  (unsigned char *buffer, int length, unsigned char ack);
	NodeCalibrationValues FindNodeValues (unsigned int nodeID);
	int DecodeSerial (unsigned char *buffer, int length);
	int BuildXCommandHeader (unsigned char* buffer, int command, 
				 int node_id, int group_id, 
				 int device, int state, 
				 int rate);
	int BuildRFIDHeader (unsigned char command, unsigned char* buf, 
			     unsigned short len, int node_id, int group_id);
	int  calcByte (int crc, int b);
	char getDigit (char c);
	void calcCRC  (unsigned char *packet, int length);
		
	void ChangeNodeState (int node_id, int group_id, unsigned char state, 
			      int device, int enable, double rate);
	float ConvertAccel (unsigned short raw_accel, int neg_1g, int pos_1g,
	            	    int converted);
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* Mica2_Init (ConfigFile* cf, int section)
{
    // Create and return a new instance of this driver
    return ((Driver*)(new Mica2 (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void Mica2_Register (DriverTable* table)
{
    table->AddDriver ("mica2", Mica2_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Mica2::Mica2 (ConfigFile* cf, int section)
	: Driver (cf, section)
{
    int i = 0;
    int j = 0;
    this->wsn_subscriptions = this->rfid_subscriptions = 0;
    provideRFID = FALSE;
    provideWSN  = FALSE;
    base_node_status = 1;

    memset(&this->wsn_addr,  0, sizeof (player_devaddr_t));
    memset(&this->rfid_addr, 0, sizeof (player_devaddr_t));

    port_name   = cf->ReadString (section, "port", DEFAULT_MICA2_PORT);
    port_speed  = cf->ReadInt (section, "speed", DEFAULT_MICA2_RATE);
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

    // Filter base node from readings ?
    filterbasenode   = cf->ReadInt (section, "filterbasenode", 0);

    // Do we create a WSN interface?
    if (cf->ReadDeviceAddr (&wsn_addr, section, "provides",
       PLAYER_WSN_CODE, -1, NULL) == 0)
    {
        if (this->AddInterface (this->wsn_addr))
        {
            this->SetError(-1);
            return;
        }
	provideWSN = TRUE;
    }

    // Do we create a RFID interface?
    if (cf->ReadDeviceAddr (&rfid_addr, section, "provides",
       PLAYER_RFID_CODE, -1, NULL) == 0)
    {
        if (this->AddInterface (this->rfid_addr))
        {
            this->SetError(-1);
            return;
        }
        provideRFID = TRUE;
    }

    return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
Mica2::~Mica2()
{
}

////////////////////////////////////////////////////////////////////////////////
// Subscribe
int Mica2::Subscribe (player_devaddr_t id)
{
    int setupResult;
	
    // do the subscription
    if ((setupResult = Driver::Subscribe (id)) == 0)
    {
	// also increment the appropriate subscription counter
	if (Device::MatchDeviceAddress (id, this->wsn_addr))
	    this->wsn_subscriptions++;
	else if (Device::MatchDeviceAddress (id, this->rfid_addr))
	    this->rfid_subscriptions++;
    }
    return setupResult;
}

////////////////////////////////////////////////////////////////////////////////
// Unsubscribe
int Mica2::Unsubscribe (player_devaddr_t id)
{
    int (shutdownResult);
	
    // do the unsubscription
    if ((shutdownResult = Driver::Unsubscribe (id)) == 0)
    {
	// also increment the appropriate subscription counter
	if (Device::MatchDeviceAddress (id, this->wsn_addr))
	{
	    this->wsn_subscriptions--;
	    assert(this->wsn_subscriptions >= 0);
	}
	else if (Device::MatchDeviceAddress (id, this->rfid_addr))
	{
	    this->rfid_subscriptions--;
	    assert(this->rfid_subscriptions >= 0);
	}
    }
    return (shutdownResult);
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Mica2::Setup ()
{
    // Open serial port
    fd = open (port_name, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
	PLAYER_ERROR2 ("> Connecting to MIB510 on [%s]; [%s]...[failed!]",
	      (char*) port_name, strerror (errno));
	return (-1);
    }
    PLAYER_MSG0 (1, "> Connecting to MIB510... [done]");

    // Change port settings
    struct termios options;
    memset (&options, 0, sizeof (options));  // clear the struct for new settings
    // read satisfied if one char received
    options.c_cc[VMIN] = 1;
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNBRK | IGNPAR;

    switch (port_speed)
    {
	case 19200:
	{
	    port_speed = B19200;           // mica2dot
	    break;
	}
	case 57600:
	{
	    port_speed = B57600;           // mica2
	    break;
	}
	default:
	{
	    port_speed = B57600;
	    break;
	}
    }
    // Set the baudrate to the given portSpeed
    cfsetispeed (&options, port_speed);
    cfsetospeed (&options, port_speed);

    // Activate the settings for the port
    if (tcsetattr (fd, TCSANOW, &options) < 0)
    {
	PLAYER_ERROR (">> Unable to set serial port attributes !");
	return (-1);
    }

    // Make sure queues are empty before we begin
    tcflush (fd, TCIFLUSH);

    // Start the device thread
    StartThread ();

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Mica2::Shutdown ()
{
    // Stop the driver thread
    StopThread ();
	
    // Close the serial port
    close (fd);
	
    PLAYER_MSG0 (1, "> Mica2 driver shutting down... [done]");
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Mica2::Main () 
{
    timespec sleepTime = {0, 0};

    // The main loop; interact with the device here
    while (true)
    {
	// test if we are supposed to cancel
	pthread_testcancel ();

	// Process any pending messages
	ProcessMessages ();

	// Interact with the device, and push out the resulting data.
	if (base_node_status != 0) // if the base node is asleep, no serial
        	            	   // data can be read
    	    RefreshData ();

	nanosleep (&sleepTime, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessage function
int Mica2::ProcessMessage (QueuePointer & resp_queue, 
			   player_msghdr * hdr,
			   void * data)
{	
    assert (hdr);
    assert (data);
	
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, 
	PLAYER_WSN_CMD_DEVSTATE, wsn_addr))
    {
    	// Actuate various devices on the node
	player_wsn_cmd_t *command = (player_wsn_cmd_t*)data;

	ChangeNodeState (command->node_id, command->group_id, 
			 2, command->device, command->enable, -1);

	return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
	 PLAYER_WSN_REQ_POWER, wsn_addr))
    {
	// Put the node in sleep mode (0) or wake it up (1)
	player_wsn_power_config *powerconfig = 
			(player_wsn_power_config*)data;
		
	// Only allow 0/1 values here
	if ((powerconfig->value != 0) && (powerconfig->value != 1))
	{
	    Publish (wsn_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
		     hdr->subtype);
	    return -1;
	}
			
	ChangeNodeState (powerconfig->node_id, powerconfig->group_id, 
			 powerconfig->value, -1, -1, -1);
		
	Publish (wsn_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
	 PLAYER_WSN_REQ_DATATYPE, wsn_addr))
    {
	// Change the data type to RAW or converted metric units
	player_wsn_datatype_config *datatype = 
			(player_wsn_datatype_config*)data;

	unsigned int val = datatype->value;
	if ((val >= 0) && (val < 3))
	{
	    raw_or_converted = val;
	    Publish (wsn_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	}
	else
	    Publish (wsn_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, hdr->subtype);
	return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
	 PLAYER_WSN_REQ_DATAFREQ, wsn_addr))
    {
	// Change the data frequency rate
	player_wsn_datafreq_config *datafreq = 
			(player_wsn_datafreq_config*)data;
	ChangeNodeState (datafreq->node_id, datafreq->group_id, 
			 3, -1, -1, datafreq->frequency);

	Publish (wsn_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
	 PLAYER_RFID_REQ_READTAG, rfid_addr))
    {
	// to be implemented
	Publish (rfid_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	return 0;
    }
    else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
	 PLAYER_RFID_REQ_WRITETAG, rfid_addr))
    {
	// to be implemented
	Publish (rfid_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	return 0;
    }
    else
    {
	return -1;
    }
	
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// calcByte function
int Mica2::calcByte (int crc, int b)
{
    crc = crc ^ (int)b << 8;

    for (int i = 0; i < 8; i++)
    {
	if ((crc & 0x8000) == 0x8000)
	    crc = crc << 1 ^ 0x1021;
	else
	    crc = crc << 1;
    }

    return (crc & 0xFFFF);
}

////////////////////////////////////////////////////////////////////////////////
// calcCRC function
void Mica2::calcCRC (unsigned char *packet, int length)
{
    int crc   = 0;
    int index = 1;
    int count = length - 3;
	
    while (count > 0) {
    	crc = calcByte (crc, packet[index++]);
	count--;
    }
    packet[length-2] = (char)(crc & 0xFF);
    packet[length-1] = (char)((crc >> 8) & 0xFF);
}

////////////////////////////////////////////////////////////////////////////////
// GetDigit function
char Mica2::getDigit (char c)
{
   if (( c >= '0' ) && (c <= '9'))
      return (c - '0');
   if (( c >= 'a') && (c <= 'f'))
      return (c - 'a' + 10 );
   if (( c >= 'A' ) && (c <= 'F'))
      return (c - 'A' + 10);
   return (-1);
}
////////////////////////////////////////////////////////////////////////////////
// BuildXCommandHeader function
int Mica2::BuildXCommandHeader (unsigned char* buffer, int command, 
				int node_id, int group_id, 
				int device, int state, 
				int rate)
{
    // TinyOS header
    XCommandMsg *msg = (XCommandMsg *)buffer;
    msg->tos.addr    = (unsigned short)node_id;// Broadcast address: 0xFFFF
    msg->tos.type    = 0x30;                   // AMTYPE_XCOMMAND
    msg->tos.group   = (unsigned char)group_id;// Broadcast: 0xFF
    msg->tos.length  = sizeof (XCommandMsg) - sizeof (TOSMsgHeader);

    // Data payload
    msg->seq_no      = 0xFF; // Broadcast sequence number
    msg->destination_id = (unsigned short)node_id; // Broadcast address: 0xFFFF
    msg->inst[0].cmd    = (unsigned short)command;
    if (device != -1)
    {
	msg->inst[0].param.actuate.device = (unsigned short)device;
	msg->inst[0].param.actuate.state  = (unsigned short)state;
	return sizeof (XCommandMsg);
    }
    if (rate != -1)
    {
	msg->inst[0].param.new_rate = (unsigned int)rate;
	return sizeof (XCommandMsg);
    }
    msg->inst[0].param.new_rate = 0xCCCCCCCC;  // Fill unused in known way
    return sizeof (XCommandMsg);
}

////////////////////////////////////////////////////////////////////////////////
// BuildRFIDHeader function - ASCII mode
int Mica2::BuildRFIDHeader (unsigned char command, unsigned char* buffer,
						 	unsigned short len, int node_id, int group_id)
{
    M1miniCommand  cmd;
    RFIDMsg        msg;
    int            msg_len = 23;
    int            cmd_len = 0;
    int            cmd_i   = 0;
    int            ptotal  = 1;
    int            pi      = 0;
    int            i       = 0;
    int            datalen;
    unsigned short crc;
    unsigned char *ptr;

	// If read/write tag command selected, buffer must hold the GUID values
    if ((buffer == NULL) && (command != 0))
	return -1;

    if (len > sizeof (M1miniCommand))
        return -1;

    // clear cmd buffer
    memset ((char *) &cmd, '0', sizeof (M1miniCommand));

    switch (command)
    {
	case 0:			// SELECT_TAG (0x14)
	{
	    cmd.flags[0]   = '0';
	    cmd.flags[1]   = '0';
	    cmd.request[0] = '1';
	    cmd.request[1] = '4';
	    cmd_len = 6;
	    break;
	}
	case 1:			// READ_TAG (0x24)
	{
	    cmd.flags[0]   = '4';
	    cmd.flags[1]   = '0';
	    cmd.request[0] = '2';
	    cmd.request[1] = '4';
	    cmd_len = 26;
	    memcpy (cmd.type, &buffer[0], len);
	    break;
	}
	case 2:			// WRITE_TAG (0x44)
	{
	    cmd.flags[0]   = '4';
	    cmd.flags[1]   = '0';
	    cmd.request[0] = '4';
	    cmd.request[1] = '4';
	    cmd_len = 34;
	    memcpy (cmd.type, &buffer[0], len);
	    break;
	}
	// Only allow already implemented commands
	default:
	    return -1;
    }

	// Split the packet into several chunks for Read/Write tag commands
    if (cmd_len > msg_len)
    {
        ptotal = cmd_len / msg_len;
        if (cmd_len % msg_len)
            ptotal++; 
    }

    // TOS packet struct
    msg.tos.addr  = (unsigned short)node_id;// Broadcast address: 0xFFFF
    msg.tos.type  = 0x51;				     // AMTYPE_RFID
    msg.tos.group = (unsigned char)group_id;// Broadcast: 0xFF
    msg.ptotal = ptotal;
    msg.RID    = 0x1234;
    msg.SG     = 0x0022;

	// Data payload
    for (pi = 0; pi < ptotal; pi++)
    {
        if (pi == ptotal - 1)
            datalen = cmd_len - cmd_i;
        else
            datalen = msg_len;

        msg.tos.length = datalen + 6;		// PAYLOAD_DATA_INDEX
        msg.pi = pi;
		
	ptr = ((unsigned char*)&cmd) + cmd_i;

        for (i = 0; i < datalen; i++)
            msg.data[i] = ptr[i];

	ptr = (unsigned char*)&msg;

	// Calculate CRC for the entire packet
        crc = 0;
        crc = calcByte (crc, 0x42);		// PACKET_NOACK
        for (i = 0; i < (5 + msg.tos.length); i++)
            crc = calcByte (crc, ptr[i]);

        // Append CRC
        msg.data[datalen]   = 0;
        msg.data[datalen]  |= (unsigned char) crc;
        msg.data[datalen+1] = (unsigned char) (crc >> 8);

        WriteSerial ((unsigned char*)&msg, 7 + msg.tos.length, 0x42);	// NOACK

        cmd_i += datalen;

	// sleep a while so that TOSBase can forward all UART to RADIO
        usleep (50000);			// 50ms
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ChangeNodeState function
void Mica2::ChangeNodeState (int node_id, int group_id, unsigned char state, 
			     int device, int enable, double rate)
{
    unsigned char buffer[255];
    int index = 0;
	
    // Assign defaults to {node,group}_id
    if ((group_id == -1) || (group_id == 0))
    	group_id = 0xFF;
    if (node_id == -1)
	node_id = 0xFFFF;
		
    int node_sleep = 0;
    if (rate != -1)
	node_sleep =  static_cast<int> (1000 / rate);

    // Start constructing the TinyOS packet
	// (serial start byte and ACK are handled in the WriteSerial method) 
    buffer[index++] = 0xFF;                // Broadcast sequence number
	
    switch (state)
    {
    	case 0:                            // sleep (XCOMMAND_SLEEP)
	{
	    index += BuildXCommandHeader 
		(buffer+index, 0x11, node_id, group_id, -1, -1, -1);
	    if (node_id == 0)             // base node?
		base_node_status = 0;
		
	    break;
	}
	case 1:                            // wake up (XCOMMAND_WAKEUP)
	{
	    index += BuildXCommandHeader 
		(buffer+index, 0x12, node_id, group_id, -1, -1, -1);
	    if (node_id == 0)             // base node?
		base_node_status = 1;
	    break;
	}
	case 2:                            // actuate (XCOMMAND_ACTUATE)
	{
	    index += BuildXCommandHeader 
		(buffer+index, 0x40, node_id, group_id, device, enable, -1);
	    break;
	}
	case 3:                            // set_rate (XCOMMAND_SET_RATE)
	{
	    index += BuildXCommandHeader 
		(buffer+index, 0x20, node_id, group_id, -1, -1, node_sleep);
	    break;
	}
    }
	
    index += 2;
    calcCRC (buffer, index);               // calculate and add CRC
	
    WriteSerial (buffer, index, 0x41);		// Write with ACK
}

////////////////////////////////////////////////////////////////////////////////
// RefreshData function
void Mica2::RefreshData ()
{
    int length;
    unsigned char buffer[255];
    // Get the time at which we started reading
    // This will be a pretty good estimate of when the phenomena occured
    struct timeval time;
    GlobalTime->GetTime (&time);
	
    // In case the RFID interface is enabled, send a "select_tag" command first
    if ((provideRFID) && (this->rfid_subscriptions > 0))
        BuildRFIDHeader (0, NULL, 0, 0xFFFF, 1);

	// Reading from UART
    length = ReadSerial (buffer);
    if (length < 16)        // minimum valid packet size
	return;             // ignore partial packets

    // Decoding and publishing the UART data
    if (DecodeSerial (buffer, length) == -1)
	return;

    return;
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// ReadSerial function - reads one XSensorPacket from the serial port
int Mica2::ReadSerial (unsigned char *buffer)
{
    unsigned char c;
    int err, i = 0;

    buffer[i] = 0x7e;          // serial start byte
    while (1) {
	err = read (fd, &c, 1);
	if (err < 0)
	{
	    PLAYER_ERROR (">> Error reading from serial port !");
	    return (-1);
	}
	if (err == 1)
	{
	    if (++i > 255) return i;
		buffer[i] = c;
	    if (c == 0x7e) return i;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
// WriteSerial function - write one XSensorPacket to the serial port
int Mica2::WriteSerial (unsigned char *buffer, int length, unsigned char ack)
{
    unsigned char c;
    int err, i = 0;

    c = 0x7e;					// serial start byte
    write (fd, &c , 1);
    c = ack;					// P_PACKET_ACK or P_PACKET_NOACK
    write (fd, &c , 1); 

    c = buffer[0];
    while (1)
    {
	if (i>= length)
	    return length;
	c = buffer[i++];
	err = write (fd, &c, 1);

	if (err < 0)
	{
	    PLAYER_ERROR (">> Error writing to serial port !");
	    return (-1);
	}
    }
    c = 0x7e;					// serial SYNC_BYTE
    write (fd, &c , 1);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// FindNodeValues function - find the appropriate calibration values for nodeID
NodeCalibrationValues Mica2::FindNodeValues (unsigned int nodeID)
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
// DecodeSerial function - decode a XSensorPacket or a M1MiniPacket
int Mica2::DecodeSerial (unsigned char *buffer, int length)
{
    NodeCalibrationValues node_values;
    player_wsn_data_t  wsn_data;
    player_rfid_data_t rfid_data;
    bool rfidPacket = FALSE;
    bool wsnPacket  = FALSE;
    int  i = 0, o = 2;    // index and offset

    if ((this->wsn_subscriptions < 1) && (this->rfid_subscriptions < 1))
	return -1;

    // Zero data
    memset (&wsn_data,  0, sizeof (player_wsn_data_t));
    memset (&rfid_data, 0, sizeof (player_rfid_data_t));

    while (i < length)
    {
	if (buffer[o] == 0x7d)           // handle escape characters
	{
	    buffer[i++] = buffer[++o] ^ 0x20;
	    ++o;
	}
	else
	{
	    buffer[i++] = buffer[o++];
	}
    }
	
    switch (buffer[2])
    {
	case 0x03:
	{   // a HEALTH packet
	    // Health offset to data payload
	    //SensorPacket *packet = (SensorPacket *)(buffer + 5);
	    //HealthData *data = (HealthData *)packet;
	    break;
	}
	case 0x33:
	{   // a MULTIHOP packet
	    if (wsn_subscriptions < 1)
		break;
	    // Multihop offset to data payload
	    SensorPacket *packet = (SensorPacket *)(buffer + 12);
	    switch (packet->board_id)
	    {
		case 0x02:
		{                       // MTS510
		    if (packet->packet_id == 1)
		    {
			wsnPacket = TRUE;
			MTS510Data *data = (MTS510Data *)packet->data;
			wsn_data.node_type      = packet->board_id;
			wsn_data.node_id        = packet->node_id;
			
			wsn_data.node_parent_id = packet->parent;
				
			wsn_data.data_packet.light       = data->light;
			int sound = (data->sound[0] + data->sound[1] + 
				data->sound[2] + data->sound[3] + 
				data->sound[4]) / 5;
			wsn_data.data_packet.mic         = sound;
						
			if (raw_or_converted != 0)
			{
			    node_values = FindNodeValues (packet->node_id);
						
			    wsn_data.data_packet.accel_x = ConvertAccel 
					(data->accelX, 
					node_values.c_values[0], 
					node_values.c_values[1], raw_or_converted);
			    wsn_data.data_packet.accel_y = ConvertAccel 
					(data->accelY,
					node_values.c_values[2],
					node_values.c_values[3], raw_or_converted);
			} 
			else
			{
			    wsn_data.data_packet.accel_x     = data->accelX;
			    wsn_data.data_packet.accel_y     = data->accelY;
			}
			wsn_data.data_packet.accel_z     = -1;
			wsn_data.data_packet.magn_x      = -1;
			wsn_data.data_packet.magn_y      = -1;
			wsn_data.data_packet.magn_z      = -1;
			wsn_data.data_packet.temperature = -1;
			wsn_data.data_packet.battery     = -1;
		    }
		    break;
		}
		case 0x84:
		{                       // MTS310
		    if (packet->packet_id == 1)
		    {
			wsnPacket = TRUE;
			MTS310Data *data = (MTS310Data *)packet->data;
			wsn_data.node_type      = packet->board_id;
			wsn_data.node_id        = packet->node_id;
			wsn_data.node_parent_id = packet->parent;
	
			wsn_data.data_packet.mic         = data->mic;
	
			if (raw_or_converted != 0)
			{
			    node_values = FindNodeValues (packet->node_id);
				
			    wsn_data.data_packet.accel_x = ConvertAccel 
					(data->accelX,
					node_values.c_values[0],
					node_values.c_values[1], raw_or_converted);
			    wsn_data.data_packet.accel_y = ConvertAccel 
					(data->accelY,
					node_values.c_values[2],
					node_values.c_values[3], raw_or_converted);
			    // Convert battery to Volts
			    wsn_data.data_packet.battery     =
					(1252352 / (float)data->vref) / 1000;
						
			    // Convert temperature to degrees Celsius
			    float thermistor = (float)data->thermistor;
			    unsigned short rthr = (unsigned short)
					(10000 * (1023 - thermistor) / thermistor);
							
			    wsn_data.data_packet.temperature = 
					(1 / (0.001307050f + 0.000214381f *
					log (rthr) + 0.000000093f *
					pow (log (rthr),3))) - 273.15;
							
			    // Convert the magnetometer data to Gauss
			    wsn_data.data_packet.magn_x      = 
					(data->magX / (1.023*2.262*3.2)) / 1000;
			    wsn_data.data_packet.magn_y      = 
					(data->magY / (1.023*2.262*3.2)) / 1000;
							
			    // Convert the light to mV
			    wsn_data.data_packet.light       = (data->light *
					wsn_data.data_packet.battery / 1023);
			} 
			else
			{
			    wsn_data.data_packet.accel_x     = data->accelX;
			    wsn_data.data_packet.accel_y     = data->accelY;
			    wsn_data.data_packet.battery     = data->vref;
			    wsn_data.data_packet.temperature = data->thermistor;
			    wsn_data.data_packet.magn_x      = data->magX;
			    wsn_data.data_packet.magn_y      = data->magY;
			    wsn_data.data_packet.light       = data->light;
			}
			wsn_data.data_packet.accel_z     = -1;
			wsn_data.data_packet.magn_z      = -1;
		    }
		    break;
		}
	    }
	    break;
	}
	case 0x51:
	{   // a RFID packet
	    if (rfid_subscriptions < 1)
	        break;
		
	    rfidPacket = TRUE;
			
	    player_rfid_tag_t RFIDtag;
	    memset (&RFIDtag, 0, sizeof (RFIDtag));
			
	    RFIDMsg *rmsg = (RFIDMsg *)buffer;
	    int dataoffset;
			
	    // Get tag information if first packet
	    if ((rmsg->ptotal == 1) && (rmsg->pi == 0))
	    {
	        unsigned char response_code = getDigit (rmsg->data[0]);
	        response_code <<= 4;
	        response_code &= 0xF0;
	        response_code |= getDigit (rmsg->data[1]);
				
	        if (response_code == 0x14)		// SELECT TAG pass
	        {
		    unsigned char tag_type = getDigit (rmsg->data[2]);
		    tag_type <<= 4;
		    tag_type &= 0xF0;
		    tag_type |= getDigit (rmsg->data[3]);
		    RFIDtag.type = tag_type;
		    dataoffset = 4;
					
		    RFIDtag.guid_count = 8;
		
		    int x = 0, cc = 0;
		    int xlength = 23 - (29 - buffer[4]);
		    for (x = dataoffset; x < xlength; x += 2)
		    {
		        if ((((rmsg->data[x]   > 0x2F) && (rmsg->data[x]   < 0x3A)) ||     // if p[i] is a digit
			    ((rmsg->data[x]    > 0x40) && (rmsg->data[x]   < 0x47)) ) &&   // if p[i] is a capital lette
			    (((rmsg->data[x+1] > 0x2F) && (rmsg->data[x+1] < 0x3A)) ||     // if p[i+1] is a digit
			    ((rmsg->data[x+1]  > 0x40) && (rmsg->data[x+1] < 0x47)) ))     // if p[i+1] is a capital let
			{
			    char str[3];
			    sprintf (str, "%c%c", rmsg->data[x], rmsg->data[x+1]);
			    sscanf (str, "%x", (unsigned int*)&RFIDtag.guid[cc]);
			    cc++;
			}
		    }
		
		    rfid_data.tags_count = 1;
		    rfid_data.tags[0] = RFIDtag;
		}
	    }
    	    break;
	}
	default:
	{
	    // we only handle RFID, HEALTH and MULTIHOP package types for now
	    break;
	}
    }
	
    if ((wsn_data.node_id == 0) && (filterbasenode == 1) && (!rfidPacket))
        return -1;

    if ((provideRFID) && (rfidPacket))
	// Write the RFID data
	Publish (rfid_addr, PLAYER_MSGTYPE_DATA, PLAYER_RFID_DATA_TAGS,
		&rfid_data, sizeof (player_rfid_data_t), NULL);

    if ((provideWSN) && (wsnPacket))
	// Write the WSN data
	Publish (wsn_addr, PLAYER_MSGTYPE_DATA, PLAYER_WSN_DATA_STATE,
		&wsn_data, sizeof (player_wsn_data_t), NULL);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ConvertAccel function - convert RAW accel. data to metric units (m/s^2)
float Mica2::ConvertAccel (unsigned short raw_accel, 
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
