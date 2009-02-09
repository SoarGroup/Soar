/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
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
 * $Id: ptu46.cc 6566 2008-06-14 01:00:19Z thjc $
 *
 * methods for initializing, commanding, and getting data out of
 * the directed perceptions ptu-46 pan tilt unit camera
 *
 * Author: Toby Collett (University of Auckland)
 * Date: 2003-02-10
 *
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_ptu46 ptu46
 * @brief Directed Perceptions PTU-46-17.5 pan-tilt unit

The ptu46 driver provides control of the PTU-46-17.5 pan-tilt unit from
directed perceptions through its text interface (This unit is standard on
the RWI b21r robot). This driver will probably work with other directed
perceptions pan tilt units, please let me know if you have tested it.

The ptu46 driver supports both position and velocity control, via the
PLAYER_PTZ_REQ_CONTROL_MODE request.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_ptz

@par Requires

- none

@par Configuration requests

- PLAYER_PTZ_REQ_CONTROL_MODE

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - The serial port to which the unit is attached.

@author Toby Collett, Radu Bogdan Rusu

*/

/** @} */

/* This file is divided into two classes, the first simply deals with
 * the control of the pan-tilt unit, providing simple interfaces such as
 * set pos and get pos.
 * The second class provides the player interface, hopefully this makes
 * the code easier to understand and a good base for a pretty much minimal
 * set up of a player driver
 */

// Includes needed for player
#include <libplayercore/playercore.h>

// serial includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

//serial defines
#define PTU46_DEFAULT_BAUD B9600
#define PTU46_BUFFER_LEN 255

// command defines
#define PTU46_PAN 'p'
#define PTU46_TILT 't'
#define PTU46_MIN 'n'
#define PTU46_MAX 'x'
#define PTU46_MIN_SPEED 'l'
#define PTU46_MAX_SPEED 'u'
#define PTU46_VELOCITY 'v'
#define PTU46_POSITION 'i'

#define DEFAULT_PTZ_PORT "/dev/ttyR1"
#define PTZ_SLEEP_TIME_USEC 100000

//
// Pan-Tilt Control Class
//

class PTU46
{
    public:
	PTU46(char * port, int rate);
	~PTU46();

	// get count/degree resolution
	float GetRes (char type);
	// get position limit
	int GetLimit (char type, char LimType);

	// get/set position in degrees
	float GetPos (char type);
	bool SetPos  (char type, float pos, bool Block = false);

	// get/set speed in degrees/sec
	bool SetSpeed  (char type, float speed);
	float GetSpeed (char type);

	// get/set move mode
	bool SetMode (char type);
	char GetMode ();

	bool Open () {return fd >0;};

	// Position Limits
	int TMin, TMax, PMin, PMax;
	// Speed Limits
	int TSMin, TSMax, PSMin, PSMax;

    protected:
	// pan and tilt resolution
	float tr,pr;

	// serial port descriptor
	int fd;
	struct termios oldtio;

	// read buffer
	char buffer[PTU46_BUFFER_LEN+1];

	int Write(const char * data, int length = 0);
};


// Constructor opens the serial port, and read the config info from it
PTU46::PTU46(char * port, int rate)
{
	tr = pr = 1;
	fd = -1;

	// open the serial port

	fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if ( fd<0 )
	{
		fprintf(stderr, "Could not open serial device %s\n",port);
		return;
	}
	fcntl(fd,F_SETFL, 0);

	// save the current io settings
	tcgetattr(fd, &oldtio);

	// rtv - CBAUD is pre-POSIX and doesn't exist on OS X
	// should replace this with ispeed and ospeed instead.

	// set up new settings
	struct termios newtio;
	memset(&newtio, 0,sizeof(newtio));
	newtio.c_cflag = /*(rate & CBAUD) |*/ CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = ICANON;
	if (cfsetispeed(&newtio, rate) < 0 || 	cfsetospeed(&newtio, rate) < 0)
	{
		fprintf(stderr,"Failed to set serial baud rate: %d\n", rate);
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);
		fd = -1;
		return;
	}
	// activate new settings
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	// now set up the pan tilt camera
	Write(" "); // terse feedback
	usleep(100000);
	tcflush(fd, TCIFLUSH);

	Write("ft "); // terse feedback
	Write("ed "); // disable echo
	Write("ci "); // position mode

	// delay here so data has arrived at serial port so we can flush it
	usleep(200000);
	tcflush(fd, TCIFLUSH);

	// get pan tilt encoder res
	tr = GetRes(PTU46_TILT);
	pr = GetRes(PTU46_PAN);

	PMin = GetLimit(PTU46_PAN, PTU46_MIN);
	PMax = GetLimit(PTU46_PAN, PTU46_MAX);
	TMin = GetLimit(PTU46_TILT, PTU46_MIN);
	TMax = GetLimit(PTU46_TILT, PTU46_MAX);
	PSMin = GetLimit(PTU46_PAN, PTU46_MIN_SPEED);
	PSMax = GetLimit(PTU46_PAN, PTU46_MAX_SPEED);
	TSMin = GetLimit(PTU46_TILT, PTU46_MIN_SPEED);
	TSMax = GetLimit(PTU46_TILT, PTU46_MAX_SPEED);

	if (tr <= 0 || pr <= 0 || PMin == 0 || PMax == 0 || TMin == 0 || TMax == 0)
	{
		// if limit request failed try resetting the unit and then getting limits..
		Write(" r "); // reset pan-tilt unit (also clears any bad input on serial port)

		// wait for reset to complete
		int len = 0;
		char temp;
		char response[10] = "!T!T!P!P*";

		for (int i = 0; i < 9; ++i)
		{
			while((len = read(fd, &temp, 1 )) == 0);
			if ((len != 1) || (temp != response[i]))
			{
				fprintf(stderr,"Error Resetting Pan Tilt unit\n");
				fprintf(stderr,"Stopping access to pan-tilt unit\n");
				tcsetattr(fd, TCSANOW, &oldtio);
				fd = -1;
			}
		}

		// delay here so data has arrived at serial port so we can flush it
		usleep(100000);
		tcflush(fd, TCIFLUSH);


		// get pan tilt encoder res
		tr = GetRes(PTU46_TILT);
		pr = GetRes(PTU46_PAN);

		PMin = GetLimit(PTU46_PAN, PTU46_MIN);
		PMax = GetLimit(PTU46_PAN, PTU46_MAX);
		TMin = GetLimit(PTU46_TILT, PTU46_MIN);
		TMax = GetLimit(PTU46_TILT, PTU46_MAX);
		PSMin = GetLimit(PTU46_PAN, PTU46_MIN_SPEED);
		PSMax = GetLimit(PTU46_PAN, PTU46_MAX_SPEED);
		TSMin = GetLimit(PTU46_TILT, PTU46_MIN_SPEED);
		TSMax = GetLimit(PTU46_TILT, PTU46_MAX_SPEED);

		if (tr <= 0 || pr <= 0 || PMin == 0 || PMax == 0 || TMin == 0 || TMax == 0)
		{
			// if it really failed give up and disable the driver
			fprintf(stderr,"Error getting pan-tilt resolution...is the serial port correct?\n");
			fprintf(stderr,"Stopping access to pan-tilt unit\n");
			tcsetattr(fd, TCSANOW, &oldtio);
			fd = -1;
		}
	}
}


PTU46::~PTU46()
{
	// restore old port settings
	if (fd > 0)
		tcsetattr(fd, TCSANOW, &oldtio);
}


int PTU46::Write(const char * data, int length)
{

	if (fd < 0)
		return -1;

	// autocalculate if using short form
	if (length == 0)
		length = strlen(data);

	// ugly error handling, if write fails then shut down unit
	if(write(fd, data, length) < length)
	{
		fprintf(stderr,"Error writing to Pan Tilt Unit, disabling\n");
		tcsetattr(fd, TCSANOW, &oldtio);
		fd = -1;
		return -1;
	}
	return 0;
}


// get count/degree resolution
float PTU46::GetRes(char type)
{
	if (fd < 0)
		return -1;
	char cmd[4] = " r ";
	cmd[0] = type;

	// get pan res
	int len = 0;
	Write(cmd);
	len = read(fd, buffer, PTU46_BUFFER_LEN );

	if (len < 3 || buffer[0] != '*')
	{
		fprintf(stderr,"Error getting pan-tilt res\n");
		return -1;
	}

	buffer[len] = '\0';
	return strtod(&buffer[2],NULL)/3600;
}

// get position limit
int PTU46::GetLimit(char type, char LimType)
{
	if (fd < 0)
		return -1;
	char cmd[4] = "   ";
	cmd[0] = type;
	cmd[1] = LimType;

	// get limit
	int len = 0;
	Write(cmd);
	len = read(fd, buffer, PTU46_BUFFER_LEN );

	if (len < 3 || buffer[0] != '*')
	{
		fprintf(stderr,"Error getting pan-tilt limit\n");
		return -1;
	}

	buffer[len] = '\0';
	return strtol(&buffer[2],NULL,0);
}


// get position in degrees
float PTU46::GetPos (char type)
{
    if (fd < 0)
	return -1;

    char cmd[4] = " p ";
    cmd[0] = type;

    // get pan pos
    int len = 0;
    Write (cmd);
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len < 3 || buffer[0] != '*')
    {
	fprintf(stderr,"Error getting pan-tilt pos\n");
	return -1;
    }

    buffer[len] = '\0';

    return strtod (&buffer[2],NULL) * (type == PTU46_TILT ? tr : pr);
}


// set position in degrees
bool PTU46::SetPos (char type, float pos, bool Block)
{
    if (fd < 0)
	return false;

    // get raw encoder count to move
    int Count = static_cast<int> (pos/(type == PTU46_TILT ? tr : pr));

    // Check limits
    if (Count < (type == PTU46_TILT ? TMin : PMin) || Count > (type == PTU46_TILT ? TMax : PMax))
    {
	fprintf (stderr,"Pan Tilt Value out of Range: %c %f(%d) (%d-%d)\n", type, pos, Count, (type == PTU46_TILT ? TMin : PMin),(type == PTU46_TILT ? TMax : PMax));
	return false;
    }

    char cmd[16];
    snprintf (cmd,16,"%cp%d ",type,Count);

    // set pos
    int len = 0;
    Write (cmd);
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len <= 0 || buffer[0] != '*')
    {
	fprintf(stderr,"Error setting pan-tilt pos\n");
	return false;
    }

    if (Block)
        while (GetPos (type) != pos);

    return true;
}

// get speed in degrees/sec
float PTU46::GetSpeed (char type)
{
    if (fd < 0)
    	return -1;

    char cmd[4] = " s ";
    cmd[0] = type;

    // get speed
    int len = 0;
    Write (cmd);
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len < 3 || buffer[0] != '*')
    {
	fprintf (stderr,"Error getting pan-tilt speed\n");
	return -1;
    }

    buffer[len] = '\0';

    return strtod(&buffer[2],NULL) * (type == PTU46_TILT ? tr : pr);
}



// set speed in degrees/sec
bool PTU46::SetSpeed (char type, float pos)
{
    if (fd < 0)
	return false;

    // get raw encoder speed to move
    int Count = static_cast<int> (pos/(type == PTU46_TILT ? tr : pr));
    // Check limits
    if (abs(Count) < (type == PTU46_TILT ? TSMin : PSMin) || abs(Count) > (type == PTU46_TILT ? TSMax : PSMax))
    {
	fprintf (stderr,"Pan Tilt Speed Value out of Range: %c %f(%d) (%d-%d)\n", type, pos, Count, (type == PTU46_TILT ? TSMin : PSMin),(type == PTU46_TILT ? TSMax : PSMax));
	return false;
    }

    char cmd[16];
    snprintf (cmd,16,"%cs%d ",type,Count);

    // set speed
    int len = 0;
    Write (cmd);
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len <= 0 || buffer[0] != '*')
    {
    	fprintf (stderr,"Error setting pan-tilt speed\n");
	return false;
    }
    return true;
}


// set movement mode (position/velocity)
bool PTU46::SetMode (char type)
{
    if (fd < 0)
    	return false;

    char cmd[4] = "c  ";
    cmd[1] = type;

    // set mode
    int len = 0;
    Write (cmd);
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len <= 0 || buffer[0] != '*')
    {
	fprintf (stderr,"Error setting pan-tilt move mode\n");
	return false;
    }
    return true;
}

// get position in degrees
char PTU46::GetMode ()
{
    if (fd < 0)
    	return -1;

    // get pan tilt mode
    int len = 0;
    Write ("c ");
    len = read (fd, buffer, PTU46_BUFFER_LEN );

    if (len < 3 || buffer[0] != '*')
    {
    	fprintf (stderr,"Error getting pan-tilt pos\n");
	return -1;
    }

    if (buffer[2] == 'p')
    	return PTU46_VELOCITY;
    else if (buffer[2] == 'i')
    	return PTU46_POSITION;
    else
	return -1;
}


///////////////////////////////////////////////////////////////
// Player Driver Class                                       //
///////////////////////////////////////////////////////////////
class PTU46_Device:public Driver
{
    protected:
	// this function will be run in a separate thread
	virtual void Main ();

	player_ptz_data_t data;
	player_ptz_cmd_t cmd;

    public:
	PTU46 * pantilt;

	// config params
	char ptz_serial_port[MAX_FILENAME_SIZE];
	int Rate;
	unsigned char MoveMode;

	PTU46_Device (ConfigFile* cf, int section);

	virtual int Setup    ();
	virtual int Shutdown ();

	// MessageHandler
	int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);
};

////////////////////////////////////////////////////////////////////////////////
// initialization function
Driver* PTU46_Init (ConfigFile* cf, int section)
{
    return static_cast<Driver*> (new PTU46_Device (cf, section));
}


////////////////////////////////////////////////////////////////////////////////
// a driver registration function
void
PTU46_Register (DriverTable* table)
{
    table->AddDriver ("ptu46",  PTU46_Init);
}

////////////////////////////////////////////////////////////////////////////////
PTU46_Device::PTU46_Device (ConfigFile* cf, int section) :
    Driver (cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE)
{

    data.pan = data.tilt = data.zoom = data.panspeed = data.tiltspeed = 0;
    cmd.pan  = cmd.tilt  = cmd.zoom  = cmd.panspeed  = data.tiltspeed = 0;

    MoveMode = PLAYER_PTZ_POSITION_CONTROL;

    strncpy (ptz_serial_port,
	cf->ReadString (section, "port", DEFAULT_PTZ_PORT),
        sizeof (ptz_serial_port));
    Rate = cf->ReadInt (section, "baudrate", PTU46_DEFAULT_BAUD);
}

////////////////////////////////////////////////////////////////////////////////
int
PTU46_Device::Setup ()
{
    printf ("> PTU46 connection initializing (%s)...", ptz_serial_port);
    fflush (stdout);

    pantilt = new PTU46 (ptz_serial_port, Rate);
    if (pantilt != NULL && pantilt->Open ())
	printf ("[success]\n");
    else
    {
	printf ("[failed]\n");
	return -1;
    }

    player_ptz_cmd_t cmd;
    cmd.pan = cmd.tilt = 0;
    cmd.zoom      = 0;
    cmd.panspeed  = 0;
    cmd.tiltspeed = 0;

    // start the thread to talk with the pan-tilt-zoom unit
    StartThread ();
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
int
PTU46_Device::Shutdown ()
{
    StopThread ();

    delete pantilt;

    PLAYER_MSG0 (1, "> PTU46 driver shutting down... [done]");
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
int
PTU46_Device::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
    assert (hdr);
    assert (data);

    // No REQ_GENERIC
    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_PTZ_REQ_GENERIC, device_addr))
        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, hdr->subtype);
    else
	// REQ_CONTROL_MODE: position or velocity?
	if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ, PLAYER_PTZ_REQ_CONTROL_MODE, device_addr))
	{
	    player_ptz_req_control_mode * control_mode = reinterpret_cast<player_ptz_req_control_mode *> (data);
	    if (control_mode->mode != MoveMode)
	    {
		uint8_t NewMode;
		if (control_mode->mode == PLAYER_PTZ_VELOCITY_CONTROL)
			NewMode = PTU46_VELOCITY;
		else
		    if (control_mode->mode == PLAYER_PTZ_POSITION_CONTROL)
			NewMode = PTU46_POSITION;
		    else
		    {
		        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, hdr->subtype);
		        return 0;
		    }

		    if (pantilt->SetMode (NewMode))
		    	MoveMode = NewMode;
		    else
		    {
		        Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, hdr->subtype);
		        return 0;
		    }
	    }
	    Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
	}

	else
	    // CMD mode
	    if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, device_addr))
	    {
		player_ptz_cmd_t * new_command = reinterpret_cast<player_ptz_cmd_t *> (data);

		bool success = true;
		// Different processing depending on movement mode
		if (MoveMode == PTU46_VELOCITY)
		{
		    // ignore pan and tilt, just use velocity
		    if (cmd.panspeed != new_command->panspeed)
		    {
			if (pantilt->SetSpeed (PTU46_PAN, RTOD(new_command->panspeed)))
				cmd.panspeed = new_command->panspeed;
			else
				success = false;
		    }
		    if (cmd.tiltspeed != new_command->tiltspeed)
		    {
			if (pantilt->SetSpeed (PTU46_TILT, RTOD(new_command->tiltspeed)))
				cmd.tiltspeed = new_command->tiltspeed;
			else
				success = false;
		    }
		}
		else
		{
		    // position move mode, ignore zero velocities, set pan tilt pos
		    if (cmd.pan != new_command->pan)
		    {
			if (pantilt->SetPos (PTU46_PAN, RTOD (new_command->pan)))
				cmd.pan = new_command->pan;
			else
				success = false;
		    }
		    if (cmd.tilt != new_command->tilt)
		    {
			if (pantilt->SetPos (PTU46_TILT, RTOD (new_command->tilt)))
				cmd.tilt = new_command->tilt;
			else
				success = false;
		    }
		    if (cmd.panspeed != new_command->panspeed && new_command->panspeed != 0)
		    {
			if (pantilt->SetSpeed (PTU46_PAN, RTOD(new_command->panspeed)))
				cmd.panspeed = new_command->panspeed;
			else
				success = false;
		    }
		    if (cmd.tiltspeed != new_command->tiltspeed && new_command->tiltspeed != 0)
		    {
			if (pantilt->SetSpeed (PTU46_TILT, RTOD (new_command->tiltspeed)))
				cmd.tiltspeed = new_command->tiltspeed;
			else
				success = false;
		    }
		}
	    }
	    else
		return -1;
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// this function will be run in a separate thread
void
PTU46_Device::Main ()
{
    for (;;)
    {
        // test if we are supposed to cancel
        pthread_testcancel();

  	ProcessMessages ();

	// Copy the data.
    	data.pan  = DTOR (pantilt->GetPos (PTU46_PAN));
	data.tilt = DTOR (pantilt->GetPos (PTU46_TILT));
	data.zoom = 0;
	data.panspeed  = DTOR (pantilt->GetSpeed (PTU46_PAN));
	data.tiltspeed = DTOR (pantilt->GetSpeed (PTU46_TILT));

	Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_PTZ_DATA_STATE,
	    &data, sizeof (player_ptz_data_t), NULL);

	// repeat frequency (default to 10 Hz)
	usleep (PTZ_SLEEP_TIME_USEC);
    }
}
