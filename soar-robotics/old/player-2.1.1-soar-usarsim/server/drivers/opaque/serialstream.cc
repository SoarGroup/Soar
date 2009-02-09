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

/** @ingroup drivers Drivers */
/** @{ */
/*
 * 
The serialstream driver reads form a serial port continuously and publishes the data.
Currently this is usable with the SickS3000 driver and the Nav200 driver. This driver does
no interpretation of data output, merely reading it and publishing it, or, if it is sent a
data command it will write whatever it recieves onto the serial port 

@par Compile-time dependencies

- none

@par Provides

- @ref opaque

@par Requires

- none

@par Configuration requests

- PLAYER_LASER_REQ_GET_GEOM
- PLAYER_LASER_REQ_GET_CONFIG
  
@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port to which laser is attached.  If you are using a
    USB/232 or USB/422 converter, this will be "/dev/ttyUSBx".

- transfer_rate (integer)
  - Rate desired for data transfers, negotiated after connection
  - Default: 38400
  - Baud rate.  Valid values are 9600, 19200, 38400, 125k, 250k, 500k

- buffer_size (integer
  - The size of the buffer to be used when reading, this is the maximum that can be read in one read command
  - Default 4096
      
@par Example 

@verbatim
driver
(
  name "sicks3000"
  provides ["laser:0"]
  requires ["opaque:0"]
)

driver
(
  name "serialstream"
  provides ["opaque:0]
  port "/dev/ttyS0"
)

@endverbatim

@author Toby Collett

*/
/** @} */
  

// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/


#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <assert.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
//#include <arpa/inet.h> // for htons etc

#include <libplayercore/playercore.h>

#define DEFAULT_OPAQUE_BUFFER_SIZE 4096
#define DEFAULT_OPAQUE_PORT "/dev/ttyS0"
#define DEFAULT_OPAQUE_TRANSFER_RATE 38400
#define DEFAULT_OPAQUE_PARITY "none"

////////////////////////////////////////////////////////////////////////////////
// Device codes

#define STX     0x02
#define ACK     0xA0
#define NACK    0x92
#define CRC16_GEN_POL 0x8005

////////////////////////////////////////////////////////////////////////////////
// Error macros
#define RETURN_ERROR(erc, m) {PLAYER_ERROR(m); return erc;}

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class SerialStream : public Driver
{
  public:

    // Constructor; need that
    SerialStream(ConfigFile* cf, int section);
    virtual ~SerialStream();

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();
    
    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer &resp_queue,
                               player_msghdr * hdr,
                               void * data);

  private:

    // Main function for device thread.
    virtual void Main();

    // Update the data
    virtual void ReadData();

    // Open the terminal
    // Returns 0 on success
    virtual int OpenTerm();

    // Close the terminal
    // Returns 0 on success
    virtual int CloseTerm();
    
    // Set the io flags.
    // Just a little helper function that sets the parity and so on.
    void UpdateFlags();
    
    // Set the terminal speed
    // Valid values are 9600, 19200, 38400, 115200
    // Returns 0 on success
    virtual int ChangeTermSpeed(int speed);
    
  protected:
    //int transfer_rate; // Desired rate for operation
    int current_rate;
    
    // Name of device used to communicate with the laser
    //const char *device_name;
    
    uint8_t * rx_buffer;
    //unsigned int rx_buffer_size;
    
    struct termios oldtio;
    
    // opaque device file descriptor
    int opaque_fd; 
    
    // Properties
    IntProperty buffer_size, transfer_rate;
    StringProperty port, parity;
    
    // This is the data we store and send
    player_opaque_data_t mData;

};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
SerialStream_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return((Driver*)(new SerialStream(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void SerialStream_Register(DriverTable* table)
{
  table->AddDriver("serialstream", SerialStream_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
SerialStream::SerialStream(ConfigFile* cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
             PLAYER_OPAQUE_CODE),
             buffer_size ("buffer_size", DEFAULT_OPAQUE_BUFFER_SIZE, 0),
             transfer_rate ("transfer_rate", DEFAULT_OPAQUE_TRANSFER_RATE, 0),
             port ("port", DEFAULT_OPAQUE_PORT, 0),
             parity ("parity", DEFAULT_OPAQUE_PARITY, 0)
{
	  this->RegisterProperty ("buffer_size", &this->buffer_size, cf, section);
	  this->RegisterProperty ("port", &this->port, cf, section);
	  this->RegisterProperty ("transfer_rate", &this->transfer_rate, cf, section);
	  this->RegisterProperty ("parity", &this->parity, cf, section);
	
	  rx_buffer = new uint8_t[buffer_size];
	  assert(rx_buffer);

	  this->current_rate = 0;

	  return;
}

SerialStream::~SerialStream()
{
	delete [] rx_buffer;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int SerialStream::Setup()
{
	PLAYER_MSG1(2, "Opaque Driver initialising (%s)", port.GetValue());

	// Open the terminal
	if (OpenTerm())
	    return -1;

	PLAYER_MSG0(2, "Opaque Driver ready");

  // Start the device thread; spawns a new thread and executes
  // SerialStream::Main(), which contains the main loop for the driver.
  StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int SerialStream::Shutdown()
{
  // Stop and join the driver thread
  StopThread();
  
  CloseTerm();

  PLAYER_MSG0(2, "Opaque Driver Shutdown");

  return(0);
}

int SerialStream::ProcessMessage(QueuePointer & resp_queue,
                                 player_msghdr* hdr,
                                 void* data)
{  
	// Process messages here.  Send a response if necessary, using Publish().
	// If you handle the message successfully, return 0.  Otherwise,
	// return -1, and a NACK will be sent for you, if a response is required.
	int res;
	// Check for properties
	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_INTPROP_REQ, this->device_addr))
	{
	    player_intprop_req_t req = *reinterpret_cast<player_intprop_req_t*> (data);
	    PLAYER_MSG1(2, "%s", req.key);
	    if (strcmp("transfer_rate", req.key) == 0)
	    {
	    	res = ChangeTermSpeed(req.value);	    
	   
			// Check the error code
			if (res == 0)
			{
			  transfer_rate.SetValueFromMessage (reinterpret_cast<void*> (&req));
			  Publish(this->device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_SET_INTPROP_REQ, NULL, 0, NULL);
			}
			else
			{
			  Publish(this->device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_SET_INTPROP_REQ, NULL, 0, NULL);
			}
			return (0);
	    }
	}
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_SET_STRPROP_REQ, this->device_addr))
	{
	    player_strprop_req_t req = *reinterpret_cast<player_strprop_req_t*> (data);
	    PLAYER_MSG1(2, "%s", req.key);
		if (strcmp("parity", req.key) == 0)
	    {
	    	parity.SetValueFromMessage(reinterpret_cast<void*> (&req));
	    	UpdateFlags();
	    	return 0;
	    }
	}
	//else if it is a opaque data message then I want to flush the current serial port and write to whatever is connected to the serial port
	else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_OPAQUE_CMD_DATA, this->device_addr))
	{
		PLAYER_MSG0(2, "Command message received");
	    player_opaque_data_t * recv = reinterpret_cast<player_opaque_data_t * > (data);
	    // Make sure both input and output queues are empty
	    tcflush(opaque_fd, TCIOFLUSH);

	    // switch to blocking IO for the write
	    int flags = fcntl(opaque_fd, F_GETFL);
	    if (flags < 0 || fcntl(opaque_fd,F_SETFL,flags &~O_NONBLOCK) < 0)
	    {
	      fprintf(stderr,"Error changing to blocking write (%d - %s), disabling\n",errno,strerror(errno));
	      return -1;
	    }

	    if((recv->data_count && (write(opaque_fd, recv->data, recv->data_count)) < recv->data_count))
	    {
	      fprintf(stderr,"Error writing to FOB (%d - %s), disabling\n",errno,strerror(errno));
	      return -1;
	    }

	    // restore flags
	    if (fcntl(opaque_fd,F_SETFL,flags) < 0)
	    {
	      fprintf(stderr,"Error restoring file mode (%d - %s), disabling\n",errno,strerror(errno));
	    }
	    
	    return (0);
	}

	return(-1);
}



////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void SerialStream::Main()
{
  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages.  SerialStream::ProcessMessage() is
    // called on each message.
    ProcessMessages();

    // Reads the data from the serial port and then publishes it
    ReadData();

    // Sleep (you might, for example, block on a read() instead)
    usleep(100000);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Open the terminal
// Returns 0 on success
int SerialStream::OpenTerm()
{
  //this->opaque_fd = ::open(/*this->device_name*/ port, O_RDWR | O_SYNC , S_IRUSR | S_IWUSR );
  this->opaque_fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
  if (this->opaque_fd < 0)
  {
    PLAYER_ERROR2("unable to open serial port [%s]; [%s]",
                    port.GetValue(), strerror(errno));
    return -1;
  }
   
  // save the current io settings
  tcgetattr(opaque_fd, &oldtio);
  
  // set up new settings
  UpdateFlags();
  if (ChangeTermSpeed(transfer_rate))
	    return -1;
  
  // Make sure queue is empty
  tcflush(this->opaque_fd, TCIOFLUSH);
  usleep(1000);
  tcflush(opaque_fd, TCIFLUSH);
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Update io flags
// Parity is set to whatever the parity property contains.
//
void SerialStream::UpdateFlags()
{
	// set up new settings
	struct termios newtio;
	memset(&newtio, 0,sizeof(newtio));
	newtio.c_cflag = CS8 | CREAD;
	newtio.c_iflag = INPCK;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	if (strncmp(parity, "none", 4) == 0)
		; // Do nothing.
	else if (strncmp(parity, "even", 4) == 0)
		newtio.c_cflag |= PARENB;
	else if (strncmp(parity, "odd", 3) == 0)
		newtio.c_cflag |= PARENB | PARODD;
	else
		PLAYER_WARN("Invalid parity. Defaulting to none.");
	
	tcsetattr(opaque_fd, TCSANOW, &newtio);
	tcflush(opaque_fd, TCIOFLUSH);
}

////////////////////////////////////////////////////////////////////////////////
// Set the terminal speed
// Valid values are 9600, 19200, 38400, 115200
// Returns 0 on success
//
int SerialStream::ChangeTermSpeed(int speed)
{
  struct termios term;

  current_rate = speed;

  int term_speed;
  switch(speed)
  {
    case 9600:
		term_speed = B9600;
		break;
	case 19200:
		term_speed = B19200;
		break;
	case 38400:
		term_speed = B38400;
		break;
	case 115200:
		term_speed = B115200;
		break;
	default:
		term_speed = speed;
  }

  switch(term_speed)
  {
    case B9600:
    case B19200:
    case B38400:
    case B115200:
      if( tcgetattr( this->opaque_fd, &term ) < 0 )
        RETURN_ERROR(1, "unable to get device attributes");
        
      //cfmakeraw( &term );
	  if(cfsetispeed( &term, term_speed ) < 0 || cfsetospeed( &term, term_speed ) < 0)
	  {
		  RETURN_ERROR(1, "failed to set serial baud rate");
	  }
        
      if( tcsetattr( this->opaque_fd, TCSAFLUSH, &term ) < 0 )
        RETURN_ERROR(1, "unable to set device attributes");
      break;
      PLAYER_MSG0(2, "Communication rate changed");

    default:
      PLAYER_ERROR1("unknown speed %d", speed);
      return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Close the terminal
// Returns 0 on success
//
int SerialStream::CloseTerm()
{
  ::close(this->opaque_fd);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Read range data from laser
//
void SerialStream::ReadData()
{
  // Read a packet from the laser
  //
  int len = read(this->opaque_fd, rx_buffer, /*rx_buffer_size*/ buffer_size);
  if (len == 0)
  {
   // PLAYER_MSG0(2, "empty packet");
    return;
  }

  if (len < 0)
  {
    PLAYER_ERROR2("error reading form serial port: %d %s", errno, strerror(errno));
    return;
  }

  assert(len <  int(buffer_size));
  mData.data_count = len;
  mData.data = rx_buffer;
  Publish(this->device_addr, PLAYER_MSGTYPE_DATA, PLAYER_OPAQUE_DATA_STATE, reinterpret_cast<void*>(&mData));
}
