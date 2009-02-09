/*
 Desc: Driver for the SICK RFI341 unit
 Author: Nico Blodow and Radu Bogdan Rusu
 Date: 9 Mar 2007
 CVS: $Id: rfi341_protocol.cc 6566 2008-06-14 01:00:19Z thjc $
*/
#include <termios.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <libplayercore/playercore.h>
#include <libplayercore/playercommon.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "rfi341_protocol.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor.
rfi341_protocol::rfi341_protocol (const char* port_name, int debug_mode)
{
  port    = port_name;
  verbose = debug_mode;
  tags    = (char**)NULL;
  number_of_tags = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Connect to the RFI341 unit using hostname:portno
// Returns 0 if connection was successful, -1 otherwise
int
  rfi341_protocol::Connect (int port_speed)
{
  // Open serial port
  fd = open (port, O_RDWR);
  if (fd < 0)
  {
    PLAYER_ERROR2 ("> Connecting to SICK RFI341 on [%s]; [%s]...[failed!]",
                   (char*) port, strerror (errno));
    return (-1);
  }

  // Change port settings
  struct termios options;
  memset (&options, 0, sizeof (options));// clear the struct for new port settings
  // Get the current port settings
  if (tcgetattr (fd, &options) != 0) {
    PLAYER_ERROR (">> Unable to get serial port attributes !");
    return (-1);
  }
  tcgetattr (fd, &initial_options);

  // turn off break sig, cr->nl, parity off, 8 bit strip, flow control
  options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  // turn off echo, canonical mode, extended processing, signals
  options.c_lflag &= ~(ECHO | ECHOE | ICANON | IEXTEN | ISIG);

  options.c_cflag &= ~(CSTOPB);   // use one stop bit
  options.c_cflag &= ~(PARENB);   // no parity
  options.c_cflag &= ~(CSIZE );   // clear size
  options.c_cflag |= (CS8);     // set bit size (default is 8)
  options.c_oflag &= ~(OPOST);  // turn output processing off

  // read satisfied if TIME is exceeded (t = TIME *0.1 s)
//  options.c_cc[VTIME] = 1;
//  options.c_cc[VMIN] = 0;

  // Change the baud rate
  switch (port_speed) {
    case 1200: {
      portspeed = B1200;
      break;
    }
    case 2400: {
      portspeed = B2400;
      break;
    }
    case 4800: {
      portspeed = B4800;
      break;
    }
    case 9600: {
      portspeed = B9600;
      break;
    }
    case 19200: {
      portspeed = B19200;
      break;
    }
    case 38400: {
      portspeed = B38400;
      break;
    }
    case 57600: {
      portspeed = B57600;
      break;
    }
    case 115200: {
      portspeed = B115200;
      break;
    }
    default: {
      PLAYER_ERROR1 (">> Unsupported speed [%d] given!", port_speed);
      return (-1);
    }
  }
  // Set the baudrate to the given port_speed
  cfsetispeed (&options, portspeed);
  cfsetospeed (&options, portspeed);

  // Activate the settings for the port
  if (tcsetattr (fd, TCSAFLUSH, &options) < 0)
  {
    PLAYER_ERROR (">> Unable to set serial port attributes !");
    return (-1);
  }

  PLAYER_MSG1 (1, "> Connecting to SICK RFI341 at %dbps...[done]", port_speed);
  // Make sure queues are empty before we begin
  tcflush (fd, TCIOFLUSH);

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Disconnect from the RFI341 unit
// Returns 0 if connection was successful, -1 otherwise
int
  rfi341_protocol::Disconnect ()
{
  // Close the serial port
  tcsetattr (fd, TCSANOW, &initial_options);
  return (close (fd));
}

////////////////////////////////////////////////////////////////////////////////
// Changes the transfer speed (or other parameters) of the RFI341 unit.
int
  rfi341_protocol::SetupSensor (int transfer_speed)
{
  char transferspeed_string[5];     // to be sent to the sensor
  int transferspeed_const;          // to be passed to cfsetispeed ()

  switch (transfer_speed)
  {
    case 1200:
    {
      sprintf (transferspeed_string , "04B0");
      transferspeed_const  = B1200;
      break;
    }
    case 2400:
    {
      sprintf (transferspeed_string , "0960");
      transferspeed_const  = B2400;
      break;
    }
    case 4800:
    {
      sprintf (transferspeed_string , "12C0");
      transferspeed_const  = B4800;
      break;
    }
    case 9600:
    {
      sprintf (transferspeed_string , "2580");
      transferspeed_const  = B9600;
      break;
    }
    case 19200:
    {
      sprintf (transferspeed_string , "4B00");
      transferspeed_const  = B19200;
      break;
    }
    case 38400:
    {
      sprintf (transferspeed_string , "9600");
      transferspeed_const  = B38400;
      break;
    }
    case 57600:
    {
      sprintf (transferspeed_string , "E100");
      transferspeed_const  = B57600;
      break;
    }
    case 115200:
    {
      sprintf (transferspeed_string , "FFFF");
      transferspeed_const  = B115200;
      break;
    }
    default:
    {
      PLAYER_ERROR1 (">> Unsupported speed [%d] given!", transfer_speed);
      return (-1);
    }
  }

  // Tell sensor to change the baud rate
  char *c = (char*) malloc (10);
  while (strncmp ((const char*)buffer, "1003", 4) != 0)
  {
    sprintf (c, "1003%s", transferspeed_string);
    SendCommand (c);
    ReadResult ();
  }

  // OK, we told the sensor to change baud rate, so let's do it also
  struct termios options;
  // clear the struct for new port settings
  memset (&options, 0, sizeof (options));

  // Get the current port settings
  if (tcgetattr (fd, &options) != 0) {
    PLAYER_ERROR (">> Unable to get serial port attributes !");
    return (-1);
  }
  // Set the baudrate to the given transfer_speed
  cfsetispeed (&options, transferspeed_const);
  cfsetospeed (&options, transferspeed_const);

  // Activate the settings for the port
  if (tcsetattr (fd, TCSAFLUSH, &options) < 0)
  {
    PLAYER_ERROR (">> Unable to set serial port attributes !");
    return (-1);
  }

  // issue Interface Test request so the sensor knows the baud rate change went fine
  if (checksum == 0x05) // if checksum == 5, then it's only STX "1003" ETX
  {
    SendCommand ("1002");
    ReadResult  ();
    PLAYER_MSG1 (1, "> Changing transfer speed to %dbps...[done]", transfer_speed);
    return (0);
  }
  else
    PLAYER_WARN1 ("> Checksum error [0x%x]!", checksum);
  return (-1);
}

////////////////////////////////////////////////////////////////////////////////
// Send a command to the rfid unit. Returns -1 on error.
int
  rfi341_protocol::SendCommand (const char* cmd)
{
  assemblecommand ((unsigned char *) cmd, strlen (cmd));

  if (verbose)
  {
    printf ("--> STX ");
    printf ("%s ", cmd);
    printf ("ETX 0x%x\n", command[commandlength-1]);
  }

  int n = write (fd, command, commandlength);
  if (n < 0)
    return (-1);

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Read a result from the rfid unit.
int
  rfi341_protocol::ReadResult ()
{
  memset(buffer, 0, 256);
  // Read ACK
  int n = read (fd, buffer, 1);
  if (verbose && ((n < 0) || (buffer[0] != ACK)))
    printf (">> Error reading ACK [0x%x]!\n", buffer[0]);

  // Read STX
  n = read (fd, buffer, 1);
  if (verbose && ((n < 0) || (buffer[0] != STX)))
    printf (">> Error reading STX [0x%x]!\n", buffer[0]);

  int read_count = 0;
  do	// read until we find ETX
  {
    n = read (fd, &buffer[read_count], 1);
    read_count += n;
  } while (buffer[read_count-1] != ETX);

  // don't forget to read checksum
  n = read (fd, &buffer[read_count], 1);
  checksum = buffer[read_count];
  read_count += n;

  // TODO: check the checksum (that's what it's for!)
  buffer[read_count-2] = 0x00;
  bufferlength = read_count-2;

  if (verbose)
  {
    printf ("<-- STX ");
    printf ("%s ", buffer);
    printf ("ETX 0x%X\n", checksum);
  }
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Adds a header and the checksum to the command to be sent
int
  rfi341_protocol::assemblecommand (unsigned char* cmd, int len)
{
  unsigned char checksum = 0;
  int index = 0;

  command[0] = STX;		// Messages start with STX

  for (index = 0; index < len; index++)
    command[index + 1]  = cmd[index];

  command[1 + len] = ETX;	// Messages end with ETX

  for (int i = 0; i < len+2; i++)
    checksum ^= command[i];

  command[2 + len] = checksum;

  commandlength = 3 + len;

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Create an inventory of tags from the reader, fill in the Player data packet
// and return it
player_rfid_data_t
  rfi341_protocol::ReadTags ()
{
  char buf[17];
  unsigned int hexnumber;

  // create inventory, single mode
  SendCommand ("6C20s");
  ReadResult  ();

  // get inventory
  SendCommand ("6C21");
  ReadResult  ();

  if (tags != NULL)
  {
    for (int i = 0; i < number_of_tags; i++)
      free (tags[i]);
    free (tags);
  }

  // read number of tags:
  memcpy (buf, &buffer[4], 4);
  buf[4] = 0;
  number_of_tags = atol(buf);

  // allocate memory for the tags
  tags = (char**)malloc (number_of_tags*sizeof(char*));
  for (int i=0; i < number_of_tags; i++)
    tags[i] = (char*) malloc (17*sizeof(char));

  // read the tag UID's
  for (int i = 0; i < number_of_tags; i++)
  {
    memcpy (tags[i], &buffer[8+i*16], 16);
    tags[i][16] = 0;
  }

  // fill in player structure and return it
  player_rfid_data_t player_data;
  player_data.tags_count = number_of_tags;
  player_data.tags = (player_rfid_tag_t*)calloc(player_data.tags_count, sizeof(player_data.tags[0]));

  player_rfid_tag_t tag;
  for (int i=0; i < number_of_tags; i++)
  {
    tag.type       = 1;
    tag.guid_count = 8;
    tag.guid = (char*)calloc(tag.guid_count, sizeof(tag.guid[0]));
    for (int j = 0; j < 8; j++)
    {
      // transfer ASCII 0x30 into a char '0', f. ex.
      // this is done in steps of two digits (1 char)
      sscanf (tags[i]+j*2, "%2X", &hexnumber);
      tag.guid[7-j] = (char) hexnumber;
    }
    player_data.tags[i] = tag;
  }
  return player_data;
}
