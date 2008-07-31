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
 Desc: Driver for the INSIDE M-300/R-300 2G RFID reader
 Author: Radu Bogdan Rusu
 Date: 25 Jan 2006
 CVS: $Id: insideM300.cc 6566 2008-06-14 01:00:19Z thjc $
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_insideM300 insideM300
 * @brief Inside M300 RFID reader

The insideM300 driver controls the Inside Contactless M300/R300 2G RFID reader 
(13.56Mhz). Currently, only ISO 15693 tags are supported.

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
  - Serial port to which the Inside M300/R300 2G reader is attached.  If you are
    using a USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- rate (integer)
  - Default: 9600
  - Baud rate. Valid values are 9600, 19200, 38400, 57600 and 115200.

@par Example 

@verbatim
driver
(
  name "insideM300"
  provides ["rfid:0"]
  port "/dev/ttyS0"
  rate "57600"
)
@endverbatim

@author Radu Bogdan Rusu

*/
/** @} */

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
#include <stddef.h>
#include <stdlib.h>

// Includes needed for player
#include <libplayercore/playercore.h>

#define DEFAULT_RFID_PORT "/dev/ttyS0"
#define DEFAULT_RFID_RATE B9600

// ISO 7816-4 command set format
#define CMD_CLASS 0
#define CMD_INSTR 1
#define CMD_P1_P  2
#define CMD_P2_P  3
#define CMD_P3_P  4

// T=0 protocol direction
#define ISO_NONE   0x00
#define ISO_IN     0x01
#define ISO_OUT    0x02
#define ISO_IN_OUT 0x03

// STATUS DEFINITION
#define STATUS_OK                  0x9000
#define RESET_EEPROM_SUCCESFUL     0x3B00
#define P3_INCORRECT               0x6700
#define P1_P2_INCORRECT            0x6B00
#define CLASS_NOT_RECOGNIZED       0x6E00
#define INSTRUCTION_NOT_RECOGNIZED 0x6D00
#define CARD_NOT_IDENTIFIED        0x6982
#define COMMAND_FLOW_INCORRECT     0x9835
#define CARD_NOT_FOUND             0x6A82
#define EEPROM_RERROR              0x6200

// The InsideM300 device class.
class InsideM300 : public Driver
{
	public:
		// Constructor
		InsideM300 (ConfigFile* cf, int section);

		// Destructor
		~InsideM300 ();

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
		int           CouplerSynchronize ();
		void          WriteByte          (unsigned char byte);
		unsigned char ReadByte           ();
		unsigned int  ReadStatusWord     ();
		int           SelectTags ();
		unsigned int  SendISOCommand     (unsigned char typeISO, int dataOutLen,
						  unsigned char commandType, 
						  unsigned char p1,
						  unsigned char p2, unsigned char p3,
						  unsigned char *dataIn, 
						  unsigned char *dataOut);
		unsigned int  IsoExchange (unsigned char typeISO, 
	    				   unsigned char *ISOCommand,
					   int dataInLen, unsigned char *dataIn,
					   int dataOutLen, unsigned char *dataOut);
		void InsideInventory (int maskLength, int chipMask, 
				  unsigned char *globalChipAnswer);
		unsigned int ResetField ();
};

////////////////////////////////////////////////////////////////////////////////
//Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver* InsideM300_Init (ConfigFile* cf, int section)
{
	// Create and return a new instance of this driver
	return ((Driver*)(new InsideM300 (cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
//Registers the driver in the driver table. Called from the 
// player_driver_init function that the loader looks for
void InsideM300_Register (DriverTable* table)
{
	table->AddDriver ("insideM300", InsideM300_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
InsideM300::InsideM300 (ConfigFile* cf, int section)
	: Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, 
			  PLAYER_RFID_CODE), allocated_tags(0)
{
	this->portName  = cf->ReadString (section, "port", DEFAULT_RFID_PORT);
	this->portSpeed = cf->ReadInt (section, "speed", DEFAULT_RFID_RATE);
	
	// Enable the anticollision mode
	this->selectTagMultiple = 1;
	
	memset(&Data,0, sizeof(Data));
	
	return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor.
InsideM300::~InsideM300()
{
	free(Data.tags);
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int InsideM300::Setup ()
{
	unsigned int status;
	unsigned char chipAnswer[24];
	unsigned char ISOCommand[4];
	
	// Open serial port
	this->fd = open (this->portName, O_RDWR | O_NOCTTY);
	if (this->fd < 0)
	{
		PLAYER_ERROR2 ("> Connecting to Inside M/R300 on [%s]; [%s]...[failed!]"
				,(char*) this->portName, strerror (errno));
		return (-1);
	}
	PLAYER_MSG0 (1, "> Connecting to Inside M/R300... [done]");

	// Change port settings
	struct termios options;
	memset (&options, 0,sizeof (options));// clear the struct for new port settings
	
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

	options.c_cflag |= (CSTOPB);    // use two stop bits
	options.c_cflag |= (PARENB);    // generate parity bits (even parity)
	options.c_cflag &= ~(CSIZE );   // clear size
	options.c_cflag |= (CS8);       // set bit size (default is 8)
	options.c_oflag &= ~(OPOST);    // turn output processing off

	// read satisfied if TIME is exceeded (t = TIME *0.1 s)
	options.c_cc[VTIME] = 1;
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
	    case 115200:{
		this->portSpeed = B115200;
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

	// Synchronizing buffer
	int result = CouplerSynchronize ();
	if (result != 0) {
		PLAYER_ERROR (">> Unable to synchronize with the reader !");
		return (-1);
	}
	
	// Get the current port settings
	if (tcgetattr (this->fd, &options) != 0) {
		PLAYER_ERROR (">> Unable to get serial port attributes !");
		return (-1);
	}
	// read satisfied if one char is read, or TIME is exceeded (t = TIME *0.1 s)
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 1;
	// Activate the settings for the port
	if (tcsetattr (this->fd, TCSANOW, &options) !=0 )
	{
		PLAYER_ERROR (">> Unable to set serial port attributes !");
		return (-1);
	}

	ISOCommand[0] = 0x21;
	// 0xF4 = SET_STATUS
	status = SendISOCommand (ISO_IN, 0, 0xF4, 0x03, 0x5E, 0x01, ISOCommand, 
				 chipAnswer);
	if (status != STATUS_OK)
		PLAYER_WARN1 (">> Error 0x%x while sending ISO command (F4035E0121) !", 
				  status);
	
	ISOCommand[0] = 0x31;
	// 0xF4 = SET_STATUS
	status = SendISOCommand (ISO_IN, 0, 0xF4, 0x03, 0x5F, 0x01, ISOCommand, 
				 chipAnswer);
	if (status != STATUS_OK)
		PLAYER_WARN1 (">> Error 0x%x while sending ISO command (F4035F0131) !", 
				  status);
	
	ISOCommand[0] = 0x3B;
	// 0xF4 = SET_STATUS
	status = SendISOCommand (ISO_IN, 0, 0xF4, 0x03, 0x6B, 0x01, ISOCommand, 
				 chipAnswer);
	if (status != STATUS_OK)
		PLAYER_WARN1 (">> Error 0x%x while sending ISO command (F4036B013B) !", 
				  status);

	status = ResetField ();
	if (status != STATUS_OK)
		PLAYER_WARN1 (">> Error 0x%x while resetting field !", status);
	
	// Start the device thread
	StartThread ();

	return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int InsideM300::Shutdown ()
{
	// Stop the driver thread
	StopThread ();

	// Close the serial port
	tcsetattr (this->fd, TCSANOW, &this->initial_options);
	close (this->fd);
	
	PLAYER_MSG0 (1, "> InsideM300 driver shutting down... [done]");
	return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void InsideM300::Main () 
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
// ProcessMessage function
int InsideM300::ProcessMessage (QueuePointer & resp_queue, 
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
// RefreshData function
void InsideM300::RefreshData ()
{
	memset (&this->Data, 0, sizeof (player_rfid_data_t));
	
	// Get the time at which we started reading
	// This will be a pretty good estimate of when the phenomena occured
	struct timeval time;
	GlobalTime->GetTime(&time);
	
	// Anticollision mode
	SelectTags ();

	// Write the RFID data
	Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_RFID_DATA_TAGS, 
			&Data, sizeof (player_rfid_data_t), NULL);
	
	return;
}

////////////////////////////////////////////////////////////////////////////////
// WriteByte function
void InsideM300::WriteByte (unsigned char buf)
{
	int wresult;
	wresult = write (this->fd, &buf, 1);
	if (wresult < 0)
		PLAYER_WARN (">> Error while writing 1 byte !");
}	

////////////////////////////////////////////////////////////////////////////////
// ReadByte function
unsigned char InsideM300::ReadByte ()
{
	unsigned char buf;
	int bresult = read (this->fd, &buf, 1);
	if (bresult == 0)
		return (0);
	return buf;
}

////////////////////////////////////////////////////////////////////////////////
// ReadStatusWord function
unsigned int InsideM300::ReadStatusWord ()
{
	unsigned int status;

	status  = (ReadByte () << 8);
	status |=  ReadByte ();

	return status;
}
////////////////////////////////////////////////////////////////////////////////
// CouplerSynchronize function
// Return 0 if synchronization succeeded, -1 otherwise.
int InsideM300::CouplerSynchronize ()
{
	unsigned int status = 0;
	int          i;

	// try a couple of times (12 attempts) to synchronize with the coupler
	for (i = 0; i < 7; i++)
	{
		WriteByte (0x00);              // send 0x00 to the coupler
		status = ReadStatusWord ();    // read the status word

		// check to see if the status word matches one of the known error codes
		if ((status == P1_P2_INCORRECT)            ||
			(status == INSTRUCTION_NOT_RECOGNIZED) ||
			(status == CLASS_NOT_RECOGNIZED)       ||
			(status == COMMAND_FLOW_INCORRECT)     ||
			(status == CARD_NOT_FOUND))
			return (0);                // synchronization succeeded
	} 

	return (-1);
}

////////////////////////////////////////////////////////////////////////////////
// SendISOCommand
unsigned int InsideM300::SendISOCommand (unsigned char typeISO, int dataOutLen,
					 unsigned char commandType, 
					 unsigned char p1,
					 unsigned char p2, unsigned char p3,
					 unsigned char *dataIn, 
					 unsigned char *dataOut)
{
	unsigned char ISOCommand[5];
	int           dataInLen = 0;

	if ((typeISO == ISO_IN) || (typeISO == ISO_IN_OUT)) 
		dataInLen = p3;

	// Prepare the command
	ISOCommand[CMD_CLASS] = 0x80;           // command class
	ISOCommand[CMD_INSTR] = commandType;    // command type
	ISOCommand[CMD_P1_P]  = p1;
	ISOCommand[CMD_P2_P]  = p2;
	ISOCommand[CMD_P3_P]  = p3;

	// Send ISO Command
	return (IsoExchange (typeISO, ISOCommand, dataInLen, dataIn, dataOutLen, 
			dataOut));
}

////////////////////////////////////////////////////////////////////////////////
// IsoExchange
unsigned int InsideM300::IsoExchange (unsigned char typeISO, 
				    unsigned char *ISOCommand,
				    int dataInLen, unsigned char *dataIn,
				    int dataOutLen, unsigned char *dataOut)
{
	// Local variable declaration
	unsigned char ACK;
	unsigned int  status;

	int i; //for counter

	// Send command
	WriteByte (ISOCommand[CMD_CLASS]);
	//printf ("Writing .%x", p_abCommand[CMD_CLASS]);
	WriteByte (ISOCommand[CMD_INSTR]);
	//printf (".%x", p_abCommand[CMD_INSTR]);
	WriteByte (ISOCommand[CMD_P1_P]);
	//printf (".%x", p_abCommand[CMD_P1_P]);
	WriteByte (ISOCommand[CMD_P2_P]);
	//printf (".%x", p_abCommand[CMD_P2_P]);
	WriteByte (ISOCommand[CMD_P3_P]);
	//printf (".%x [5 bytes]\n", p_abCommand[CMD_P3_P]);

	switch (typeISO)
	{
		case ISO_NONE :                     // Neither data sent nor received
			return ReadStatusWord ();

		case ISO_IN :
		// Verify ACK
			ACK = ReadByte ();
			if (ACK != ISOCommand[CMD_INSTR])
			{
				status = (ACK<<8);
				status = status | ReadByte ();
				return status;
			}
			for (i = 0; i < dataInLen ; i++)
			{
				WriteByte (dataIn[i]);      // Send buffer data
			}
			return ReadStatusWord ();

		case ISO_OUT :
		// Verify ACK
			ACK = ReadByte ();
			if (ACK != ISOCommand[CMD_INSTR])
			{
				status  = (ACK << 8);
				status |= ReadByte ();
				return status;
			}
			for (i = 0; i < dataOutLen; i++)
				dataOut[i] = ReadByte ();   // Grab buffer data
			return ReadStatusWord ();

		case ISO_IN_OUT :
		// Verify ACK
			ACK = ReadByte ();
			if (ACK != ISOCommand[CMD_INSTR])
			{
				status = (ACK<<8);
				status = status | ReadByte ();
				return status;
			}
			for (i = 0; i < dataInLen ; i++)
				WriteByte (dataIn[i]);      // Send buffer data in
		// Verify ACK
			ACK = ReadByte ();
			if (ACK != ISOCommand[CMD_INSTR])
			{
				status = (ACK<<8);
				status = status | ReadByte ();
				return status;
			}
			for (i = 0; i < dataOutLen; i++)
				dataOut[i] = ReadByte ();   // Grab buffer data out
			return ReadStatusWord();
		default: 
			return (0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// ReadMultipleTags function
int InsideM300::SelectTags ()
{
	unsigned int  status, iStatus;
	unsigned char chipAnswer[24], globalChipAnswer[24];
	unsigned char ISOCommand[10];
	int           chipMask;
	
	status = ResetField ();
	if (status != STATUS_OK)
		PLAYER_WARN1 (">> Error 0x%x while resetting field !", status);
	
	ISOCommand[0] = 0x36; ISOCommand[1] = 0x01;
	ISOCommand[2] = 0x00; ISOCommand[3] = 0x00;
	// 0xC2 = TRANSMIT - Basic inventory command
	iStatus = SendISOCommand (ISO_IN, 0, 0xC2, 0xF3, 0x0A, 0x04, ISOCommand, 
				  chipAnswer);
	
	int tagsFound = 1;
	while (tagsFound != 0) {
		
		ISOCommand[0] = 0x00;
		// 0xC0 = GET_RESPONSE - get chip UID
		status = SendISOCommand (ISO_OUT, 10, 0xC0, 0x00, 0x00, 0x0A, 
					 ISOCommand, chipAnswer);
		chipMask = chipAnswer[2];
		
		switch (iStatus) {
			case STATUS_OK:
			{
				if (this->Data.tags_count >= this->allocated_tags)
				{
					this->allocated_tags = this->Data.tags_count+1;
					this->Data.tags = (player_rfid_tag_t*)realloc(this->Data.tags,sizeof(this->Data.tags[0])*this->allocated_tags);
				}
				this->Data.tags[this->Data.tags_count].type = chipAnswer[1];
				this->Data.tags[this->Data.tags_count].guid_count = 8;
				int j;
				for (j = 0; j < 8; j++)
					this->Data.tags[this->Data.tags_count].guid[j] = 
							chipAnswer [9-j];
				this->Data.tags_count++;
			
				tagsFound = 0;
				break;
			}
			case CARD_NOT_IDENTIFIED:
			{
				InsideInventory (1, chipMask, globalChipAnswer);
			
				memset (&ISOCommand, 0,sizeof (ISOCommand));
				int k;
				for (k = 2; k < 10; k++)
					ISOCommand[k] = globalChipAnswer[k-2];
			
				ISOCommand[0] = 0x20; ISOCommand[1] = 0x02;
				// 0xC2 = TRANSMIT (Note: get Chip Answer)
				status = SendISOCommand (ISO_IN, 0, 0xC2, 0xB3, 0x00, 0x0A, 
							 ISOCommand, chipAnswer);
				memset (&ISOCommand, 0,sizeof (ISOCommand));
				ISOCommand[0] = 0x36; ISOCommand[1] = 0x01;
				ISOCommand[2] = 0x00; ISOCommand[3] = 0x00;
				// 0xC2 = TRANSMIT (Note: get Chip Answer)
				iStatus = SendISOCommand (ISO_IN, 0, 0xC2, 0xF3, 0x0A, 0x04, 
							  ISOCommand, chipAnswer);
				break;
			}
			default:
			{
				tagsFound = 0;
				break;
			}
		}
	}
	return (0);
}

////////////////////////////////////////////////////////////////////////////////
// InsideInventory function
void InsideM300::InsideInventory (int maskLength, int chipMask, 
				  unsigned char *globalChipAnswer)
{
	unsigned int  status, iStatus;
	unsigned char ISOCommand[5];
	unsigned char chipAnswer[24];
	int i;
	
	if (maskLength > 15)
		return;
	
	ISOCommand[0] = 0x36; ISOCommand[1] = 0x01;
	ISOCommand[2] = 0x00; ISOCommand[3] = 0x00 + maskLength;
	ISOCommand[4] = 0x00 + chipMask;
	
	// Masked Inventory command
	iStatus = SendISOCommand (ISO_IN, 0, 0xC2, 0xF3, 0x0A, 0x05, ISOCommand, 
				  chipAnswer);
	// Get UID
	status  = SendISOCommand (ISO_OUT, 10, 0xC0, 0x00, 0x00, 0x0A, NULL, 
				  chipAnswer);

	chipMask = chipAnswer[2];

	switch (iStatus)
	{
		case STATUS_OK:
		{
			for (i = 0; i < 8; i++)
				globalChipAnswer[i] = chipAnswer[i+2];
			
			if (this->Data.tags_count >= this->allocated_tags)
			{
				this->allocated_tags = this->Data.tags_count+1;
				this->Data.tags = (player_rfid_tag_t*)realloc(this->Data.tags,sizeof(this->Data.tags[0])*this->allocated_tags);
			}
			this->Data.tags[this->Data.tags_count].type = chipAnswer[1];
			this->Data.tags[this->Data.tags_count].guid_count = 8;
			int j;
			for (j = 0; j < 8; j++)
				this->Data.tags[this->Data.tags_count].guid[j] = chipAnswer [9-j];
			this->Data.tags_count++;
			
			break;
		}
		case CARD_NOT_IDENTIFIED:
		{
			InsideInventory (maskLength + 1, chipMask, globalChipAnswer);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// ResetField function
unsigned int InsideM300::ResetField ()
{
	unsigned char ISOCommand[5];
	unsigned char dataIn[2] = {0x00, 0x00}; // Data in initialisation

	// Prepare the command
	ISOCommand[CMD_CLASS] = 0x80;
	ISOCommand[CMD_INSTR] = 0xF4;
	ISOCommand[CMD_P1_P]  = 0x40;
	ISOCommand[CMD_P2_P]  = 0x00;
	ISOCommand[CMD_P3_P]  = 0x01;

	// Send ISO Command
	return (IsoExchange (ISO_IN, ISOCommand, 1, dataIn, 0, NULL));
}
