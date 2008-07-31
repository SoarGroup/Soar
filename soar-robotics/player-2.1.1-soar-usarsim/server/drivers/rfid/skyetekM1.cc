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
 Desc: Driver for the SkyeTek M1/M1-mini RFID readers
 Author: Radu Bogdan Rusu
 Date: 27 Jan 2006
 CVS: $Id: skyetekM1.cc 4270 2007-12-02 09:11:13Z thjc $
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_skyetekM1 skyetekM1
 * @brief Skyetek M1 RFID reader

The skyetekM1 driver controls the SkyeTek M1/M1-mini RFID readers (13.56Mhz). 
Currently, only ISO 15693 tags are supported.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_rfid

@par Requires

- none

@par Configuration requests

- PLAYER_RFID_REQ_POWER
- PLAYER_RFID_REQ_READTAG
- PLAYER_RFID_REQ_WRITETAG
- PLAYER_RFID_REQ_LOCKTAG

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port to which the SkyeTek M1/M1-mini reader is attached.  If you are 
    using a USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- speed (integer)
  - Default: 9600
  - Baud rate. Valid values are 9600, 19200, 38400 and 57600.

@par Example 

@verbatim
driver
(
  name "skyetekM1"
  provides ["rfid:0"]
  port "/dev/ttyS0"
  speed "9600"
)
@endverbatim

@author Radu Bogdan Rusu

*/
/** @} */

#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Includes needed for player
#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

#define DEFAULT_RFID_PORT "/dev/ttyS0"
#define DEFAULT_RFID_RATE 9600

// The SkyetekM1 device class.
class SkyetekM1 : public Driver
{
	public:
		// Constructor
		SkyetekM1 (ConfigFile* cf, int section);

		// Destructor
		~SkyetekM1 ();

		// Implementations of virtual functions
		int Setup ();
		int Shutdown ();

		// This method will be invoked on each incoming message
		virtual int ProcessMessage (QueuePointer & resp_queue, 
					    player_msghdr * hdr,
					    void * data);

	private:

		// Main function for device thread.
		virtual void Main  ();
		void RefreshData ();
				
		// Port file descriptor
		int                fd;
		// Initial serial port attributes
		struct termios     initial_options;

		// RFID interface
		player_rfid_data_t Data;
		unsigned int allocated_tags;
		player_rfid_data_t  Cmd;
		
		const char*        portName;
		int                portSpeed;
		
		// 0=select just a single tag (no anticollision)
		// 1=select all tags in the RFID field (anticollision)
		int                selectTagMultiple;

		// Inside functions
		unsigned int  CRC16       (unsigned char *dataP, unsigned char n);
		unsigned char VerifyCRC   (unsigned char *resp);
		void WriteSerial (unsigned char *mdmData, unsigned char commandLen);
		int  ReadSerial  (unsigned char *mdmData, unsigned char commandLen);
		int  S_ReadWrite (unsigned char *mdmData, unsigned char length, 
						  unsigned char *response);
		void SelectTags  ();
		unsigned char Wake  ();
		unsigned char Sleep ();
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* SkyetekM1_Init (ConfigFile* cf, int section)
{
	// Create and return a new instance of this driver
	return ((Driver*)(new SkyetekM1 (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void SkyetekM1_Register (DriverTable* table)
{
	table->AddDriver ("skyetekM1", SkyetekM1_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
SkyetekM1::SkyetekM1 (ConfigFile* cf, int section)
	: Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
			  PLAYER_RFID_CODE), allocated_tags(0)
{
	this->portName  = cf->ReadString (section, "port", DEFAULT_RFID_PORT);
	this->portSpeed = cf->ReadInt (section, "speed", DEFAULT_RFID_RATE);
	
	// Enable the anticollision mode
	this->selectTagMultiple = 1;
	
	memset(&Data, 0, sizeof(Data));
	memset(&Cmd, 0, sizeof(Cmd));
	
	return;
}

SkyetekM1::~SkyetekM1()
{
	player_rfid_data_t_cleanup(&Data);
	player_rfid_data_t_cleanup(&Cmd);
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int SkyetekM1::Setup ()
{
	
	// Open serial port
	this->fd = open (this->portName, O_RDWR);
	if (this->fd < 0)
	{
		PLAYER_ERROR2 ("> Connecting to Skyetek M1 on [%s]; [%s]...[failed!]",
				   (char*) this->portName, strerror (errno));
		return (-1);
	}
	PLAYER_MSG0 (1, "> Connecting to Skyetek M1...[done]");

	// Change port settings
	struct termios options;
	memset (&options, 0, sizeof (options));// clear the struct for new port settings
	
	// Get the current port settings
	if (tcgetattr (this->fd, &options) != 0) {
		PLAYER_ERROR (">> Unable to get serial port attributes !");
		return (-1);
	}
	tcgetattr (this->fd, &this->initial_options);
	
	// turn off break sig, cr->nl, parity off, 8 bit strip, flow control
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	// turn off echo, canonical mode, extended processing, signals
	options.c_lflag &= ~(ECHO | ECHOE | ICANON | IEXTEN | ISIG);

	options.c_cflag &= ~(CSTOPB);   // use one stop bit
	options.c_cflag &= ~(PARENB);   // no parity
	options.c_cflag &= ~(CSIZE );   // clear size
	options.c_cflag |= (CS8);       // set bit size (default is 8)
	options.c_oflag &= ~(OPOST);    // turn output processing off

	// read satisfied if TIME is exceeded (t = TIME *0.1 s)
	options.c_cc[VTIME] = 10;
	options.c_cc[VMIN] = 0;

	switch (this->portSpeed) {
	    case 9600:{
		this->portSpeed = B9600;
		break;
	    }
	    case 19200:{
		this->portSpeed = B19200;
		break;
	    }
	    case 38400:{
		this->portSpeed = B38400;
		break;
	    }
	    case 57600:{
		this->portSpeed = B57600;
		break;
	    }
	    default:{
		this->portSpeed = B9600;
		break;
	    }
	}
	// Set the baudrate to the given portSpeed
	cfsetispeed (&options, this->portSpeed);
	cfsetospeed (&options, this->portSpeed);
	
	// Activate the settings for the port
	if (tcsetattr (this->fd, TCSAFLUSH, &options) < 0)
	{
		PLAYER_ERROR (">> Unable to set serial port attributes !");
		return (-1);
	}

	// Make sure queues are empty before we begin
	tcflush (this->fd, TCIOFLUSH);
		
	// Start the device thread
	StartThread ();

	return (0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int SkyetekM1::Shutdown ()
{
	// Stop the driver thread
	StopThread ();

	// Close the serial port
	tcsetattr (this->fd, TCSANOW, &this->initial_options);
	close (this->fd);
	
	PLAYER_MSG0 (1, "> SkyetekM1 driver shutting down... [done]");
	return (0);
}


int SkyetekM1::ProcessMessage (QueuePointer & resp_queue, 
			   player_msghdr * hdr,
			   void * data)
{	
	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_RFID_REQ_POWER,
	    device_addr))
	{
		// Power up/down the RFID reader (NACK for now)
 		Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
			 hdr->subtype);
	}
	
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
			 PLAYER_RFID_REQ_READTAG, device_addr))
	{
		// Read RFID tag data (NACK for now)
		Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
			 hdr->subtype);
	}
	
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
			 PLAYER_RFID_REQ_WRITETAG, device_addr))
	{
		// Write data to the RFID tag (NACK for now)
		Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
			 hdr->subtype);
	}
	
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, 
			 PLAYER_RFID_REQ_LOCKTAG, device_addr))
	{
		// Lock a RFID tag (NACK for now)
		Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
			 hdr->subtype);
	}
	
	else
	{
		return -1;
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void SkyetekM1::Main () 
{
	// Zero data
	memset (&this->Data, 0, sizeof (player_rfid_data_t));
	
	timespec sleepTime = {0, 0};
	
	// The main loop; interact with the device here
	while (true)
	{
		// test if we are supposed to cancel
		pthread_testcancel ();
	
		// Process any pending messages
		ProcessMessages();
		
		// Interact with the device, and push out the resulting data.
		this->RefreshData ();
		
		nanosleep (&sleepTime, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////
// RefreshData function
void SkyetekM1::RefreshData ()
{
	memset (&this->Data, 0, sizeof (player_rfid_data_t));
	
	// Get the time at which we started reading
	// This will be a pretty good estimate of when the phenomena occured
	struct timeval time;
	GlobalTime->GetTime(&time);
	
	// Anticollision mode
	SelectTags ();
	Wake ();

	// Write the RFID data
	Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RFID_DATA_TAGS, 
			 &Data, sizeof (player_rfid_data_t), NULL);
	
	return;
}

////////////////////////////////////////////////////////////////////////////////
// WriteSerial function
void SkyetekM1::WriteSerial (unsigned char *mdmData, unsigned char commandLen)
{
	int wresult;
	wresult = write (this->fd, mdmData, commandLen);
	if (wresult < 0)
		PLAYER_WARN1 (">> Error while writing %d bytes !", commandLen);
}	

////////////////////////////////////////////////////////////////////////////////
// ReadSerial function
int SkyetekM1::ReadSerial (unsigned char *mdmData, unsigned char commandLen)
{
	int bytes_read = read (this->fd, mdmData, commandLen);
	return bytes_read;
}

////////////////////////////////////////////////////////////////////////////////
// SelectTags function
void SkyetekM1::SelectTags ()
{
	unsigned char command_buf[80], response_buf[80];
	unsigned char i = 1;
	unsigned int crc_check;

	// REQUEST for SELECT_TAG: MSGLEN|FLAGS|0x14|rid|TAGTYPE|tid|AFI|CRC
	// ---[ Build the request ]---
	command_buf[i++] = 0x22;             // SEL_TAG_INV (CRC_F, INV_F enabled)
	command_buf[i++] = 0x14;             // SELECT_TAG (set command (0x14))
	// set reader ID - empty (no RID)
	command_buf[i++] = 0x00;             // AUTODETECT (0x00)

	
	command_buf[0] = i + 1;              // set message length
	
	crc_check = CRC16 (command_buf, i);
	command_buf[i++] = crc_check >> 8;   // set CRC
	command_buf[i++] = crc_check & 0x00FF;

	// copy the command buffer and prepare to write - OK
	int j;
	unsigned char temp[80];
	temp[0] = 0x02;                  // Start of Transmission (STX = 0x02)
	for (j = 0; j < i+1; j++)
		temp[j+1] = command_buf[j];	
	WriteSerial (temp, i + 1);
	
	// poll for reader response
	while (response_buf[2] != 0x94)
	{
		ReadSerial (response_buf, 3);
		int len = 0;
		if (response_buf[0] == 0x02)
			len = response_buf[1];
		
		unsigned char TID[len];
		memset (&TID, 0, sizeof (TID)); // clear the struct for new port settings
		usleep (10000); // sleep for 10ms
		ReadSerial (TID, len);
		if (response_buf[2] == 0x94)
			break;
		
		if (response_buf[0] == 0x02)
		{
			if (this->Data.tags_count >= this->allocated_tags)
			{
				this->allocated_tags = this->Data.tags_count+1;
				this->Data.tags = (player_rfid_tag_t*)realloc(this->Data.tags,sizeof(this->Data.tags[0])*this->allocated_tags);
			}
			this->Data.tags[this->Data.tags_count].type       = TID[0];
			if (this->Data.tags[this->Data.tags_count].guid_count != 8);
			{
				this->Data.tags[this->Data.tags_count].guid_count = 8;
				this->Data.tags[this->Data.tags_count].guid = (char*)malloc(8);
			}
			
			int j;
			for (j = 0; j < 9; j++)
				this->Data.tags[this->Data.tags_count].guid[j] = TID[j+1];
			this->Data.tags_count++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// S_ReadWrite function
int SkyetekM1::S_ReadWrite (unsigned char *mdmData, 
			unsigned char length, unsigned char *response)
{
	unsigned char temp[80], read_data[80];
	bool read_done = FALSE;
	int tmp_count = 0;
	int i;
	
	// copy the command buffer and prepare to write
	temp[0] = 0x02;                  // Start of Transmission (STX = 0x02)
	for (i = 0; i < length + 1; i++)
		temp[i+1] = mdmData[i];	
	
	WriteSerial (temp, length + 1);
	
	// sleep for 10ms
	usleep (10000);
	
	while (!read_done)
	{ 
		tmp_count++;
	
		ReadSerial (read_data, sizeof (read_data));
		
		int m = 0;

		while (m != sizeof (read_data))
		{
			response[m] = read_data[m];
			m++;
		}
				
		if (tmp_count > 10)
			read_done = TRUE;
		
		if ((read_data[0] = 0x02) && (read_data[1] <= 0x50))
			read_done = TRUE;
		
		usleep (10000);               // Sleep for 10ms
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// CRC16 function
unsigned int SkyetekM1::CRC16 (unsigned char *dataP, unsigned char n)
{
	unsigned char i, j;     // byte counter, bit counter
	unsigned int  crc_16;   // calculation

	crc_16 = 0x0000;        // PRESET value

	for (i = 0; i < n; i++) // check each byte in the array
	{
		crc_16 ^= *dataP++;

		for (j = 0; j < 8; j++) 	  // test each bit in the byte
			if (crc_16 & 0x0001)
			{
				crc_16 >>= 1;
				crc_16 ^= 0x8408; // POLYNOMIAL x^16+x^12+x^5+1
			}
			else
				crc_16 >>= 1;
	}
	return (crc_16);	// returns calculated crc (16 bits)
}


////////////////////////////////////////////////////////////////////////////////
// VerifyCRC function
unsigned char SkyetekM1::VerifyCRC (unsigned char *resp)
{
	unsigned char i = 0;
	unsigned char ret_crc[80];
	unsigned int  crc_check;

	int k = 0;
	
	for (k = 0; k < (int)((resp[1])-1); k++)
	{
		ret_crc[i] = resp[k+1];
		i++;
	}	

	crc_check = CRC16 (ret_crc, i);
	
	if ((resp[i+1] == (crc_check >> 8)) && (resp[i+2] == (crc_check & 0x00FF)))
		return 0;
	else
		return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Sleep function
unsigned char SkyetekM1::Sleep ()
{
	unsigned char command_buf[80], response_buf[80];
	unsigned char i = 0;
	unsigned int crc_check;
	int      s_rwStatus;
	
	// binary mode command format: msgLen|flags|command|startblock|numblocks|crc
	command_buf[i++] = 0x07;             // set msg length
	command_buf[i++] = 0x20;             // set flags (CRC_F)
	command_buf[i++] = 0x42;             // WRITE_SYS (set command)
	
	command_buf[i++] = 0x04;             // OP_MODE
	command_buf[i++] = 0x01;             // set number of blocks to write
	command_buf[i++] = 0x00;             // set data to write

	crc_check = CRC16 (command_buf, i);
	command_buf[i++] = crc_check >> 8;   // set CRC
	command_buf[i++] = crc_check & 0x00FF;
	
	// copy the command buffer and prepare to write
	s_rwStatus = S_ReadWrite (command_buf, i, response_buf);
	
	// verify return CRC
	if (VerifyCRC (response_buf))
		return 0x81;                     // BAD_CRC
	
	return response_buf[2];
}

////////////////////////////////////////////////////////////////////////////////
// Wake function
unsigned char SkyetekM1::Wake ()
{

	unsigned char command_buf[80], response_buf[80];
	unsigned char i = 0;
	unsigned int crc_check;
	int      s_rwStatus;
	
	// binary mode command format: msgLen|flags|command|startblock|numblocks|crc
	command_buf[i++] = 0x07;             // set msg length
	command_buf[i++] = 0x20;             // set flags (CRC_F)
	command_buf[i++] = 0x42;             // WRITE_SYS (set command)
	command_buf[i++] = 0x04;             // OP_MODE
	command_buf[i++] = 0x01;             // set number of blocks to write
	command_buf[i++] = 0xFF;             // set data to write
	command_buf[0]   = i + 1;            // set msg Length

	crc_check = CRC16 (command_buf, i);
	command_buf[i++] = crc_check >> 8;   // set CRC
	command_buf[i++] = crc_check & 0x00FF;
	
	// copy the command buffer and prepare to write
	s_rwStatus = S_ReadWrite (command_buf, i, response_buf);
	
	// verify return CRC
	if (VerifyCRC (response_buf))
		return 0x81;                     // BAD_CRC
	
	return response_buf[2];
}

