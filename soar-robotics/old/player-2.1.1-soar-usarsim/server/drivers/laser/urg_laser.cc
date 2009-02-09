#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>

#include <replace/replace.h>

#include "urg_laser.h"

///////////////////////////////////////////////////////////////////////////////
// Reads characters (and throws them away) until the nth occurence of char c.
int
  urg_laser::ReadUntil_nthOccurence (int file, int n, char c)
{
  int retval = 0;
  unsigned char Buffer[2];
  Buffer[0] = 0;
  Buffer[1] = 0;
  for (int i = 0; i < n; i++)
  {
    do 
    {
      retval = ReadUntil (file, &Buffer[0], 1, -1);
    } while (Buffer[0] != c && retval > 0);
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// This is intended to find out which protocol the Hokuyo supports. Old
//  Firmware Revisions support protocol SCIP1.0, and a max range of 4
//  meters Since Firmware Revision 3.0.00, it's called SCIP2.0, and the max
//  range is 5.6 meters (hey!)
int
  urg_laser::QuerySCIPVersion ()
{
  unsigned char Buffer [18];
  memset (Buffer, 0, 18);
  int file = fileno (laser_port);
  /////////////////
  // try SCIP1 first:
  /////////////////
  tcflush (fileno (laser_port), TCIFLUSH);
  fprintf (laser_port, "V\n");
  
  // check the returned command
  memset (Buffer, 0, 18);
  ReadUntil (file, Buffer, 4, -1);
  
  if (strncmp ((const char *) Buffer, "V\n0\n", 4) != 0)
  { 
    // SCIP1.0 failed, so we test it with SCIP2.0:
    tcflush (fileno (laser_port), TCIFLUSH);
    fprintf (laser_port, "VV\n");
    
    int file = fileno (laser_port);
    
    // check the returned command
    memset (Buffer, 0, 18);
    ReadUntil (file, Buffer, 7, -1);
    tcflush (fileno (laser_port), TCIFLUSH);
    
    if (strncmp ((const char *) Buffer, "VV\n00P\n", 7) != 0)
    {
      printf ("> E: QuerySCIPVersion: Error reading after VV command. Answer: %s\n", Buffer);
      return (-1);
    }
    
    // Set SCIP version 2 and return
    SCIP_Version = 2;
    return (0);
  }
  
  // we are currently in SCIP 1.0
  else
  {
    Buffer[0] = 0;
    // Read the rest of the values, up till right before firmware version
    ReadUntil_nthOccurence (file, 2, (char)0xa);
    // Read "FIRM:" 
    memset (Buffer, 0, 18);
    ReadUntil (file, Buffer, 5, -1);
      
    if (strncmp ((const char *) Buffer, "FIRM:", 5) != 0)
    {
      //printf ("> W: QuerySCIPVersion: Warning, 'FIRM:' is not where it is supposed to be!\n");
      // HACK: assume that we're talking to a TOP-URG
      tcflush (fileno (laser_port), TCIFLUSH);
      this->SCIP_Version = 3;
      this->num_ranges = 1128;
      return(0);
    }
      
    // Read the firmware version major value 
    ReadUntil (file, Buffer, 1, -1);
    Buffer[1] = 0;
    int firmware = atol ((const char*)Buffer);

    ReadUntil_nthOccurence (file, 4, (char)0xa);
    if (firmware < 3)
    {
      // Set SCIP version 1 and return
      SCIP_Version = 1;
      return 0;
    }
    else
    {
      // try to switch to SCIP2.0
      tcflush (fileno (laser_port), TCIFLUSH);
      fprintf (laser_port, "SCIP2.0\n");
  
      // check the returned command
      memset (Buffer, 0, 18);
      ReadUntil (file, Buffer, 2, -1);
      if (strncmp ((const char *) Buffer, "SC", 2) != 0)
      {
        // Set SCIP version 1 and return
        SCIP_Version = 1;
	return 0;
      }
      else
      {
        memset (&Buffer[2], 0, 16);
        ReadUntil (file, &Buffer[2], 8, -1);
        if (strncmp ((const char *) Buffer, "SCIP2.0\n0\n", 11) != 0)
        {
          // Set SCIP version 1 and return
          SCIP_Version = 1; 
	  return (0);
        }
        // Set SCIP version 2, turn laser on and return
	SCIP_Version = 2;
	fprintf (laser_port, "BM\n");
        ReadUntil_nthOccurence (file, 3, (char)0xa);
        tcflush (fileno (laser_port), TCIFLUSH);
	return 0;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
int
  urg_laser::GetSensorConfig (player_laser_config_t *cfg)
{
  // TODO: look into getting intensity data
  cfg->intensity = 0;

  if (SCIP_Version == 1)
  {
    unsigned char Buffer[10];
    memset (Buffer, 0, 10);
    tcflush (fileno (laser_port), TCIFLUSH);
    // send the command
    fprintf (laser_port, "V\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 4, -1);
  
    if (strncmp ((const char *) Buffer, "V\n0\n", 4) != 0)
    {
      printf ("> E: GetSensorConfig: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
    
    // The following might not work on all versions of the hokuyos
    // since it reads out the Product description returned by 'V'

    ReadUntil_nthOccurence (file, 2, (char)0xa);
    
    // Read FIRM:
    ReadUntil (file, Buffer, 5, -1);
  
    if (strncmp ((const char *) Buffer, "FIRM:", 5) == 0)
    {
      // Read the firmware version major value 
      ReadUntil (file, Buffer, 1, -1);
      Buffer[1] = 0;
      int firmware = atol ((const char*)Buffer);

      if (firmware < 3)
      {
        ReadUntil_nthOccurence (file, 4, (char)0xa);
        tcflush (fileno (laser_port), TCIFLUSH);
        return (-1);
      }
    }

    ReadUntil_nthOccurence(file, 1, (char)'(');
    ReadUntil_nthOccurence(file, 1, (char)'-');
            
    int i = 0;
    do
    {
      ReadUntil (file, &Buffer[i], 1, -1);
    } while (Buffer[i++] != '[');
    
    Buffer[i-1] = 0;
    int max_range = atol((const char*)Buffer);
     
    ReadUntil_nthOccurence (file, 2, (char)',');
    i = 0;
    do
    {
      ReadUntil(file, &Buffer[i], 1, -1);
    } while (Buffer[i++] != '-');
    
    Buffer[i-1] = 0;
    int min_i = atol ((const char*)Buffer);
    i = 0;
    do
    {
      ReadUntil (file, &Buffer[i], 1, -1);
    } while (Buffer[i++] != '[');
    Buffer[i-1] = 0;
    
    int max_i = atol ((const char*)Buffer);
    
    ReadUntil (file, Buffer, 4, -1);
    if (strncmp ((const char *) Buffer, "step", 4) != 0)
    {
      printf ("> E: GetSensorConfig: Error reading angle_min_idx and angle_max_idx. Using an older firmware?\n");
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
    cfg->max_range  = max_range / 1000.0;
    cfg->min_angle  = (min_i-384)*cfg->resolution;
    cfg->max_angle  = (max_i-384)*cfg->resolution;
    printf ("> I: URG-04 specifications: [min_angle, max_angle, resolution, max_range] = [%f, %f, %f, %f]\n",
             RTOD (cfg->min_angle), RTOD (cfg->max_angle), RTOD (cfg->resolution), cfg->max_range);
    tcflush (fileno(laser_port), TCIFLUSH);
  }
  else if(SCIP_Version == 2)
  {
    // ask hokuyo: PP
    unsigned char Buffer[10];
    memset (Buffer, 0, 10);
    tcflush (fileno (laser_port), TCIFLUSH);
    
    // send the command
    fprintf (laser_port, "PP\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer,7, -1);
  
    if (strncmp ((const char *) Buffer, "PP\n00P\n", 7) != 0)
    {
      printf ("> E: GetSensorConfig: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
    int i = 0;
    ReadUntil_nthOccurence (file, 2, (char)0xa);
    // read DMAX
    ReadUntil_nthOccurence (file, 1, ':');
    do 
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      i++;
    } while (Buffer[i-1] != ';');
    Buffer[i-1] = 0;
    cfg->max_range = atol ((const char*)Buffer);
    cfg->max_range /= 1000;
    
    // read angular resolution
    ReadUntil_nthOccurence (file, 1, ':');
    i = 0;
    do 
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      i++;
    } while (Buffer[i-1] != ';');
    Buffer[i-1] = 0;
    cfg->resolution = DTOR (360.0 / atol ((const char*)Buffer));
    
    // read AMIN
    ReadUntil_nthOccurence (file, 1, ':');
    i = 0;
    do 
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      i++;
    } while (Buffer[i-1] != ';');
    Buffer[i-1] = 0;
    cfg->min_angle = atol ((const char*)Buffer);
    cfg->min_angle -= 384.0;
    cfg->min_angle *= cfg->resolution;
    
    // read AMAX
    ReadUntil_nthOccurence (file, 1, ':');
    i=0;
    do 
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      i++;
    } while (Buffer[i-1] != ';');
    Buffer[i-1] = 0;
    cfg->max_angle = atol ((const char*)Buffer);
    cfg->max_angle -= 384.0;
    cfg->max_angle *= cfg->resolution;

    ReadUntil_nthOccurence (file, 4, (char)0xa);

    printf ("> I: URG-04 specifications: [min_angle, max_angle, resolution, max_range] = [%f, %f, %f, %f]\n",
             RTOD (cfg->min_angle), RTOD (cfg->max_angle), RTOD (cfg->resolution), cfg->max_range);
  }
  else 			// SCIP_Version = 3 (TOP-URG)
  {
    // HACK: should ask the device, but for now just hardcode it:
    cfg->min_angle = DTOR(-141.0);
    cfg->max_angle = DTOR(141.0);
    cfg->resolution = DTOR(282.0/1128.0);
    cfg->max_range = 30.0;

    printf ("> I: TOP-URG specifications: [min_angle, max_angle, resolution, max_range] = [%f, %f, %f, %f]\n",
             RTOD (cfg->min_angle), RTOD (cfg->max_angle), RTOD (cfg->resolution), cfg->max_range);
  }
  return (0);
}

///////////////////////////////////////////////////////////////////////////////
int 
  urg_laser::ReadUntil (int fd, unsigned char *buf, int len, int timeout)
{
  int ret;
  int current=0;
  struct pollfd ufd[1];
  int retval;

  ufd[0].fd = fd;
  ufd[0].events = POLLIN;

  do
  {
    if(timeout >= 0)
    {
      if ((retval = poll (ufd, 1, timeout)) < 0)
      {
        perror ("poll():");
        return (-1);
      }
      else if (retval == 0)
      {
        puts ("Timed out on read");
        return (-1);
      }
    }

    ret = read (fd, &buf[current], len-current);
    if (ret < 0)
      return ret;
    
    current += ret;
    if (current > 2 && current < len && buf[current-2] == '\n' && buf[current-1] == '\n')
    {
      puts ("> E: ReadUntil: Got an end of command while waiting for more data, this is bad.\n");
      return (-1);
    }
  } while (current < len);
  return len;
}

///////////////////////////////////////////////////////////////////////////////
urg_laser::urg_laser ()
{
  // Defaults to SCIP version 1
  SCIP_Version = 1;
  num_ranges = 769;
  laser_port   = NULL;
}

///////////////////////////////////////////////////////////////////////////////
urg_laser::~urg_laser ()
{
  if (PortOpen ())
    fclose (laser_port);
}

///////////////////////////////////////////////////////////////////////////////
int 
  urg_laser::ChangeBaud (int curr_baud, int new_baud, int timeout)
{
  struct termios newtio;
  int fd;
  fd = fileno (laser_port);

  if (tcgetattr (fd, &newtio) < 0)
  {
    perror ("urg_laser::ChangeBaud:tcgetattr():");
    close (fd);
    return (-1);
  }
  cfmakeraw (&newtio);
  cfsetispeed (&newtio, curr_baud);
  cfsetospeed (&newtio, curr_baud);

  if (tcsetattr (fd, TCSAFLUSH, &newtio) < 0 )
  {
    perror ("urg_laser::ChangeBaud:tcsetattr():");
    close (fd);
    return (-1);
  }

  unsigned char buf[17];
  memset (buf,0,sizeof (buf));
  
  // TODO: Check if this works with SCIP2.0
  if (SCIP_Version == 1)
  {
    buf[0] = 'S';
    switch (new_baud)
    {
      case B19200:
        buf[1] = '0';
        buf[2] = '1';
        buf[3] = '9';
        buf[4] = '2';
        buf[5] = '0';
        buf[6] = '0';
        break;
      case B57600:
        buf[1] = '0';
        buf[2] = '5';
        buf[3] = '7';
        buf[4] = '6';
        buf[5] = '0';
        buf[6] = '0';
        break;
      case B115200:
        buf[1] = '1';
        buf[2] = '1';
        buf[3] = '5';
        buf[4] = '2';
        buf[5] = '0';
        buf[6] = '0';
        break;
      default:
        printf ("unknown baud rate %d\n", new_baud);
        return (-1);
    }
    buf[7] = '0';
    buf[8] = '0';
    buf[9] = '0';
    buf[10] = '0';
    buf[11] = '0';
    buf[12] = '0';
    buf[13] = '0';
    buf[14] = '\n';
  }
  else				// SCIP 2
  {
    buf[0] = 'S';
    buf[1] = 'S';
    switch (new_baud)
    {
      case B19200:
        buf[2] = '0';
        buf[3] = '1';
        buf[4] = '9';
        buf[5] = '2';
        buf[6] = '0';
        buf[7] = '0';
        break;
      case B57600:
        buf[2] = '0';
        buf[3] = '5';
        buf[4] = '7';
        buf[5] = '6';
        buf[6] = '0';
        buf[7] = '0';
        break;
      case B115200:
        buf[2] = '1';
        buf[3] = '1';
        buf[4] = '5';
        buf[5] = '2';
        buf[6] = '0';
        buf[7] = '0';
        break;
      default:
        printf ("unknown baud rate %d\n", new_baud);
        return (-1);
    }
    buf[8] = '\n';
  }

  fprintf (laser_port, "%s", buf);
  memset (buf, 0, sizeof (buf));
  int len;
  // The docs say that the response ends in 'status LF LF', where
  // status is '0' if everything went alright.  But it seems that
  // the response actually ends in 'LF status LF'.
  if (((len = ReadUntil (fd, buf, sizeof (buf), timeout)) < 0) ||
     (buf[15] != '0'))
  {
    puts ("failed to change baud rate");
    return (-1);
  }
  else
  {
    if (tcgetattr (fd, &newtio) < 0)
    {
      perror ("urg_laser::ChangeBaud:tcgetattr():");
      close (fd);
      return (-1);
    }
    cfmakeraw (&newtio);
    cfsetispeed (&newtio, new_baud);
    cfsetospeed (&newtio, new_baud);
    if (tcsetattr (fd, TCSAFLUSH, &newtio) < 0 )
    {
      perror ("urg_laser::ChangeBaud:tcsetattr():");
      close (fd);
      return (-1);
    }
    else
    {
      usleep (200000);
      return (0);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
int 
  urg_laser::Open (const char * PortName, int use_serial, int baud)
{
  if (PortOpen ())
    this->Close ();

  laser_port = fopen (PortName, "r+");
  if (laser_port == NULL)
  {
    printf ("> E: Open: Failed to open Port: %s error = %d:%s\n",
            PortName, errno, strerror (errno));
    return (-1);
  }

  int fd = fileno (laser_port);
  if (use_serial)
  {
    puts ("Trying to connect at 19200");
    if (this->ChangeBaud (B19200, baud, 100) != 0)
    {
      puts ("Trying to connect at 57600");
      if (this->ChangeBaud (B57600, baud, 100) != 0)
      {
        puts ("Trying to connect at 115200");
        if (this->ChangeBaud (B115200, baud, 100) != 0)
        {
          puts ("failed to connect at any baud");
          close (fd);
          return (-1);
        }
      }
    }
    puts ("Successfully changed baud rate");
  }
  else
  {
    // set up new settings
    struct termios newtio;
    memset (&newtio, 0, sizeof (newtio));
    newtio.c_cflag = /*(rate & CBAUD) |*/ CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    // activate new settings
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &newtio);
    usleep (200000);
    QuerySCIPVersion ();
    tcflush (fd, TCIOFLUSH);
  }

  return (0);
}

///////////////////////////////////////////////////////////////////////////////
int
  urg_laser::Close ()
{
  int retval;

  assert (this->laser_port);

  tcflush (fileno (this->laser_port), TCIOFLUSH);
  retval = fclose (this->laser_port);
  this->laser_port = NULL;
  return (retval);
}

///////////////////////////////////////////////////////////////////////////////
bool
  urg_laser::PortOpen ()
{
  return laser_port != NULL;
}

///////////////////////////////////////////////////////////////////////////////
int 
  urg_laser::GetReadings (urg_laser_readings_t * readings, int min_i, int max_i)
{
  unsigned char Buffer[16];
  //memset (Buffer, 0, 11);
  assert (readings);

  if (!PortOpen ())
    return (-3);

  if (SCIP_Version == 1)
  {
    tcflush (fileno (laser_port), TCIFLUSH);
    // send the command
    fprintf (laser_port, "G00076801\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 10, -1);
  
    if (strncmp ((const char *) Buffer, "G00076801", 9) != 0)
    {
      printf ("> E: GetReadings: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
  
    // check the returned status
    ReadUntil (file, Buffer, 2, -1);
    
    if (Buffer[0] != '0')
      return Buffer[0] - '0';
  
    for (int i=0; ; ++i)
    {
      ReadUntil (file, Buffer, 2, -1);
  
      if (Buffer[0] == '\n' && Buffer[1] == '\n')
        break;
      
      else if (Buffer[0] == '\n')
      {
        Buffer[0] = Buffer[1];
        if (ReadUntil (file, &Buffer[1], 1, -1) < 0)
          return -1;
      }
  
      if (i < MAX_READINGS)
        readings->Readings[i] = ((Buffer[0]-0x30) << 6) | (Buffer[1]-0x30);
      else
        printf ("Got too many readings! %d\n",i);
    }
  }
  else if(SCIP_Version == 2)
  {
    tcflush (fileno (laser_port), TCIFLUSH);
    // send the command
    fprintf (laser_port, "GD0000076801\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 13, -1);
  
    if (strncmp ((const char *) Buffer, "GD0000076801", 12) != 0)
    {
      printf ("> E: GetReadings: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
    
    // check the returned status
    ReadUntil (file, Buffer, 3, -1);
    Buffer[2] = 0;
    if (Buffer[0] != '0' || Buffer[1] != '0')
      return (Buffer[0] - '0')*10 + (Buffer[1] - '0');

    ReadUntil_nthOccurence (file, 2, (char)0xa);
    
    // NOTE: This only works for 769 requested samples.. (64 data bytes
    // blocks are not the best choice for 3-byte values...)
    
    for (int i = 0; ; ++i)
    {
      ReadUntil (file, Buffer, 3, -1);
    
      //printf ("[%d of %d] 0x%x 0x%x 0x%x\n", i, MAX_READINGS, Buffer[0], Buffer[1], Buffer [2]);
        
      if ((Buffer[1] == '\n') && (Buffer[2] == '\n'))
        break;
      else if (Buffer[2] == '\n')
      {
        if (ReadUntil(file, &Buffer[1], 2, -1) < 0)
          return (-1);
      }
      else if (Buffer[0] == '\n')
      {
	if (i <= MAX_READINGS)
	{
		readings->Readings[i - 1] = ((readings->Readings[i - 1] & 0xFFC0) | (Buffer[1]-0x30));
		Buffer [0] = Buffer [2];
		if (ReadUntil (file, &Buffer[1], 2, -1) < 0)
		return (-1);
	}
	else
	        printf ("> E: Got too many readings! %d\n",i);
      }
      else if (Buffer[1] == '\n')
      {
        Buffer[0] = Buffer[2];
        if (ReadUntil (file, &Buffer[1], 2, -1) < 0)
          return (-1);
      }
  
      if (i < MAX_READINGS)
      {
        readings->Readings[i] = ((Buffer[0]-0x30) << 12) | ((Buffer[1]-0x30) << 6) | (Buffer[2]-0x30);
        if ((readings->Readings[i] > 5600) && (i >= min_i) && (i <= max_i))
	  printf ("> W: [%d] read error: %i is bigger than 5.6 meters\n", i, readings->Readings[i]);
      }
      else
        printf ("> E: Got too many readings! %d\n",i);
    }
  }
  else // SCIP_Version == 3 (TOP-URG)
  {
    tcflush (fileno (laser_port), TCIFLUSH);
    // send the command
    fprintf (laser_port, "GD0000112700\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 13, -1);
  
    if (strncmp ((const char *) Buffer, "GD0000112700", 12) != 0)
    {
      printf ("> E: GetReadings: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
    
    // check the returned status
    ReadUntil (file, Buffer, 3, -1);
    Buffer[2] = 0;
    if (Buffer[0] != '0' || Buffer[1] != '0')
      return (Buffer[0] - '0')*10 + (Buffer[1] - '0');

    ReadUntil_nthOccurence (file, 2, (char)0xa);
    
    // NOTE: This only works for 769 requested samples.. (64 data bytes
    // blocks are not the best choice for 3-byte values...)
    
    for (int i = 0; ; ++i)
    {
      ReadUntil (file, Buffer, 3, -1);
    
      //printf ("[%d of %d] 0x%x 0x%x 0x%x\n", i, MAX_READINGS, Buffer[0], Buffer[1], Buffer [2]);
        
      if ((Buffer[1] == '\n') && (Buffer[2] == '\n'))
        break;
      else if (Buffer[2] == '\n')
      {
        if (ReadUntil(file, &Buffer[1], 2, -1) < 0)
          return (-1);
      }
      else if (Buffer[0] == '\n')
      {
	if (i <= MAX_READINGS)
	{
		readings->Readings[i - 1] = ((readings->Readings[i - 1] & 0xFFC0) | (Buffer[1]-0x30));
		Buffer [0] = Buffer [2];
		if (ReadUntil (file, &Buffer[1], 2, -1) < 0)
		return (-1);
	}
	else
	        printf ("> E: Got too many readings! %d\n",i);
      }
      else if (Buffer[1] == '\n')
      {
        Buffer[0] = Buffer[2];
        if (ReadUntil (file, &Buffer[1], 2, -1) < 0)
          return (-1);
      }
  
      if (i < MAX_READINGS)
      {
        readings->Readings[i] = ((Buffer[0]-0x30) << 12) | ((Buffer[1]-0x30) << 6) | (Buffer[2]-0x30);
        // > 50000 seems to be an error code for when an object is too close
        //if(readings->Readings[i] >= 50000)
          //readings->Readings[i] = 0;
        if(readings->Readings[i] >= 30000)
          readings->Readings[i] = 30000;
        if ((readings->Readings[i] > 30000) && (i >= min_i) && (i <= max_i))
	  printf ("> W: [%d] read error: %i is bigger than 30.0 meters\n", i, readings->Readings[i]);
      }
      else
        printf ("> E: Got too many readings! %d\n",i);
    }
  }

  return (0);
}

//////////////////////////////////////////////////////////////////////////////
int 
  urg_laser::GetIDInfo ()
{
  unsigned char Buffer [18];
  memset (Buffer, 0, 18);
  int i;
  int id; 
  if (!PortOpen ())
    return -3;

  tcflush (fileno (laser_port), TCIFLUSH);

  if (SCIP_Version == 1)
  {
    // send the command
    fprintf (laser_port, "V\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 2, -1);
  
    if (strncmp ((const char *) Buffer, "V", 1) != 0)
    {
      printf ("> E: GetIDInfo: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
  
    // check the returned status
    ReadUntil (file, Buffer, 2, -1);
    
    if (Buffer[0] != '0')
      return Buffer[0] - '0';
  
    Buffer[0] = 0;
    // Read the rest of the values
    for (i = 0; i < 4; i++)
    {
      do 
      {
        ReadUntil (file, &Buffer[0], 1, -1);
      } while (Buffer[0] != 0xa);
    }
    
    // Read "SERI:H" 
    ReadUntil (file, Buffer, 6, -1);
    // Read the serial number value
    for (i = 0; ; i++)
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      if (Buffer[i] == 0xa)
        break;
    }
    
    id = atol ((const char*)Buffer);
    // Read the last LF
    ReadUntil (file, Buffer, 1, -1);
  }
  else // SCIP_Version == 2
  {
    // send the command
    fprintf (laser_port, "VV\n");
  
    int file = fileno (laser_port);
  
    // check the returned command
    ReadUntil (file, Buffer, 7, -1);
  
    if (strncmp ((const char *) Buffer, "VV\n00P\n", 7) != 0)
    {
      printf (">E: GetIDInfo: Error reading command result: %s\n", Buffer);
      tcflush (fileno (laser_port), TCIFLUSH);
      return (-1);
    }
  
    Buffer[0] = 0;
    // Read the rest of the values
    for (i = 0; i < 4; i++)
    {
      do 
      {
        ReadUntil (file, &Buffer[0], 1, -1);
      } while (Buffer[0] != 0xa);
    }
    
    // Read "SERI:H" 
    ReadUntil (file, Buffer, 6, -1);
    // Read the serial number value
    for (i = 0; ; i++)
    {
      ReadUntil (file, &Buffer[i], 1, -1);
      if (Buffer[i] == ';')
      {
        Buffer[i] = 0;
        break;
      }
    }
    
    id = atol ((const char*)Buffer);

    ReadUntil (file, Buffer, 3, -1);
  }
  return id;
}
