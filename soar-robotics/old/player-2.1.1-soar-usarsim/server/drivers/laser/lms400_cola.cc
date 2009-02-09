/*
 Desc: Driver for the SICK LMS400 unit
 Author: Nico Blodow and Radu Bogdan Rusu
 Date: 7 Feb 2007
 CVS: $Id: lms400_cola.cc 6499 2008-06-10 01:13:51Z thjc $
*/
#include <sys/socket.h>
#include <netdb.h>
#include <libplayercore/playercore.h>

#include "lms400_cola.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor.
lms400_cola::lms400_cola (const char* host, int port, int debug_mode)
{
  portno   = port;
  hostname = host;
  verbose  = debug_mode;
  bzero (command, BUF_SIZE);
  MeasurementQueue = new std::vector<MeasurementQueueElement_t >;
}

////////////////////////////////////////////////////////////////////////////////
// Connect to the LMS400 unit using hostname:portno
// Returns 0 if connection was successful, -1 otherwise
int
  lms400_cola::Connect ()
{
  // Create a socket
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    return (-1);

  // Get the network host entry
  server = gethostbyname ((const char *)hostname);
  if (server == NULL)
    return (-1);

  // Fill in the sockaddr_in structure values
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port   = htons (portno);
  bcopy ((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

  // Attempt to connect
  if (connect (sockfd, (const sockaddr*)&serv_addr, sizeof (serv_addr)) < 0)
    return (-1);

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Disconnect from the LMS400 unit
// Returns 0 if connection was successful, -1 otherwise
int
  lms400_cola::Disconnect ()
{
  return (close (sockfd));
}

////////////////////////////////////////////////////////////////////////////////
// Enable/Disable extended RIS (Remission Information System) detectivity
int
  lms400_cola::EnableRIS (int onoff)
{
  char cmd[40];
  sprintf (cmd, "sWN MDblex %i", onoff);
  SendCommand (cmd);

  if (ReadAnswer () != 0)
    return (-1);
  ExtendedRIS = onoff;
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Get the current laser unit configuration and return it into Player format
player_laser_config
  lms400_cola::GetConfiguration ()
{
  player_laser_config_t cfg;
  cfg = Configuration;
  return cfg;
}

////////////////////////////////////////////////////////////////////////////////
// Set the mean filter parameters
int
  lms400_cola::SetMeanFilterParameters (int num_scans)
{
  char cmd[40];
  sprintf (cmd, "sWN FLmean 0 %i", num_scans);
  SendCommand (cmd);

  if (ReadAnswer () != 0)
    return (-1);
  MeanFilterNumScans = num_scans;
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Set the range filter parameters
int
  lms400_cola::SetRangeFilterParameters (float *ranges)
{
  char cmd[40];
  sprintf (cmd, "sWN FLrang %+f %+f", ranges[0], ranges[1]);
  SendCommand (cmd);

  if (ReadAnswer () != 0)
    return (-1);
  RangeFilterBottomLimit = ranges[0];
  RangeFilterTopLimit    = ranges[1];
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Enable filters using a filter mask
int
  lms400_cola::EnableFilters (int filter_mask)
{
  char cmd[40];
  sprintf (cmd, "sWN FLsel %+i", filter_mask);
  SendCommand (cmd);

  if (ReadAnswer () != 0)
    return (-1);
  FilterMask = filter_mask;
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Takes a string containing an ip adress and returns an array of 4 u_chars
unsigned char*
  lms400_cola::ParseIP (char* ip)
{
  char* tmp = (char*) malloc (strlen (ip) + 1);
  unsigned char* _ip = (unsigned char*) malloc (4);

  strcpy (tmp, ip);
  _ip[0] = atoi (strtok (tmp, "."));
  for (int i = 1; i < 4; i++)
    _ip[i] = atoi (strtok (NULL, "."));

  free (tmp);
  return _ip;
}

////////////////////////////////////////////////////////////////////////////////
// Set the desired userlevel by logging in with the appropriate password
int
  lms400_cola::SetUserLevel (int8_t userlevel, const char* password)
{
  char cmd[255];
  sprintf (cmd, "sMN SetAccessMode %d %s", userlevel, password);
  SendCommand (cmd);
  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Fills string pointed to by macadress with the MAC adress read from the sensor
int
  lms400_cola::GetMACAddress (char** macaddress)
{
  char *mac = (char*) malloc (20);
  int index = 0;
  char* tmp;

  SendCommand ("sRN EImac ");
  if (ReadAnswer () != 0)
    return (-1);

  strtok ((char*) buffer, " ");
  strtok (NULL, " ");

  for (int i = 0; i < 6; i++)
  {
    tmp = strtok (NULL, "-");
    strncpy (&mac[index], tmp, 2);
    index += 2;
    mac[index++] = ':';
  }

  mac[--index] = 0;
  *macaddress = mac;
  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Set the IP address of the LMS400
int
  lms400_cola::SetIP (char* ip)
{
  unsigned char* ip_str;
  ip_str = ParseIP (ip);
  char cmd[80];

  sprintf (cmd, "sWN EIip %X %X %X %X", ip_str[0], ip_str[1], ip_str[2], ip_str[3]);
  free (ip_str);
  SendCommand (cmd);

  return (ReadAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the gateway address for the Ethernet interface
int
  lms400_cola::SetGateway (char* gw)
{
  unsigned char* gw_str;
  gw_str = ParseIP (gw);
  char cmd[80];

  sprintf (cmd, "sWN EIgate %X %X %X %X", gw_str[0], gw_str[1], gw_str[2], gw_str[3]);
  free (gw_str);
  SendCommand (cmd);

  return (ReadAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the subnet mask for the Ethernet interface
int
  lms400_cola::SetNetmask (char* mask)
{
  unsigned char* mask_str;
  mask_str = ParseIP (mask);
  char cmd[80];

  sprintf (cmd, "sWN EImask %X %X %X %X", mask_str[0], mask_str[1], mask_str[2], mask_str[3]);
  free (mask_str);
  SendCommand (cmd);

  return (ReadAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set port for TCP/IP communication
int
  lms400_cola::SetPort (uint16_t port)
{
  char cmd[80];

  sprintf (cmd, "sWN EIport %04X", port);
  SendCommand (cmd);

  return (ReadAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Reset the LMS400 unit
int
  lms400_cola::ResetDevice ()
{
  const char* cmd = "sMN mDCreset ";
  SendCommand (cmd);

  return (ReadAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Terminate configuration and change back to userlevel 0
int
  lms400_cola::TerminateConfiguration ()
{
  const char* cmd = "sMN Run";
  SendCommand (cmd);

  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the laser angular resolution. Requires userlevel 2. Unused for now.
int
  lms400_cola::SetAngularResolution (const char* password, float ang_res,
                                    float angle_start, float angle_range)
{
  char cmd[80];
  sprintf (cmd, "sMN mSCconfigbyang 04 %s %+f 01 %+f %+f",
           password, ang_res, angle_start, angle_range);
  SendCommand (cmd);

  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the laser scanning frequency. Requires userlevel 2. Unused for now.
int
  lms400_cola::SetScanningFrequency (const char* password, float freq,
                                    float angle_start, float angle_range)
{
  char cmd[80];
  sprintf (cmd, "sMN mSCconfigbyfreq 04 %s %+f 01 %+f %+f",
           password, freq, angle_start, angle_range);
  SendCommand (cmd);

  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Set both resolution and frequency without going to a higher user level (?)
int
  lms400_cola::SetResolutionAndFrequency (float freq, float ang_res,
                                          float angle_start, float angle_range)
{
  char cmd[80];
  sprintf (cmd, "sMN mSCsetscanconfig %+.2f %+.2f %+.2f %+.2f",
    freq, ang_res, angle_start, angle_range);
  SendCommand (cmd);

  int error = ReadConfirmationAndAnswer ();

  // If no error, parse the results
  if (error == 0)
  {
    strtok ((char*)buffer, " "); strtok (NULL, " ");
    int ErrorCode = strtol (strtok (NULL, " "), NULL, 16);
    long int sf = strtol (strtok (NULL, " "), NULL, 16);
    long int re = strtol (strtok (NULL, " "), NULL, 16);

    if ((ErrorCode != 0) && (verbose))
      printf (">> Warning: got an error code %d\n", ErrorCode);

    memcpy (&Configuration.scanning_frequency, &sf, sizeof (uint32_t));
    memcpy (&Configuration.resolution, &re, sizeof (uint32_t));

    if (verbose)
      printf (">> Measured value quality is: %ld [5-10]\n",
        strtol (strtok (NULL, " "), NULL, 16));
  }

  return (error);
}

////////////////////////////////////////////////////////////////////////////////
// Start a measurement for both distance and intensity or just distance.
int
  lms400_cola::StartMeasurement (bool intensity)
{
  char cmd[40];
  if (intensity)
    sprintf (cmd, "sMN mLRreqdata %x", 0x20);
  else
    sprintf (cmd, "sMN mLRreqdata %x", 0x21);

  SendCommand (cmd);

  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Read a measurement
player_laser_data_t
  lms400_cola::ReadMeasurement ()
{
  player_laser_data_t player_data;
  player_data.ranges_count = -1;

  char cs_read = 0, cs_calc = 0;
  int length  = 0;
  int current = 0;

  bzero (buffer, 256);
  if (!MeasurementQueue->empty ())
  {
    if (verbose) printf (">>> Reading from queue...\n");
    memcpy (buffer, (char*) MeasurementQueue->front ().string, MeasurementQueue->front ().length + 1);
    free (MeasurementQueue->front ().string);
    MeasurementQueue->erase (MeasurementQueue->begin ());
  }
  else
  {
    if (verbose == 2) printf (">>> Queue empty. Reading from socket...\n");
    n = read (sockfd, buffer, 8);
    if (n < 0)
    {
      if (verbose) printf (">>> E: error reading from socket!\n");
      return (player_data);
    }
    if (buffer[0] != 0x02 || buffer[1] != 0x02 || buffer[2] != 0x02 || buffer[3] != 0x02)
    {
      if (verbose) printf (">>> E: error expected 4 bytes STX's!\n");
      n = read (sockfd, buffer, 255);
      return (player_data);
    }

    // find message length
    length = ( (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] <<  8) | (buffer[7]) );
    do
    {
      n = read (sockfd, &buffer[current], length-current);
      current += n;
    } while (current < length);

    // read checksum:
    read (sockfd, &cs_read, 1);

    for (int i = 0; i < length; i++)
      cs_calc ^= buffer[i];

    if (cs_calc != cs_read)
    {
      if (verbose) printf (">>> E: checksums do not match!\n");
      return (player_data);
    }
  }

  // parse measurements header and fill in the configuration parameters
  MeasurementHeader_t meas_header;
  memcpy (&meas_header, (void *)buffer, sizeof (MeasurementHeader_t));

  Configuration.min_angle  = meas_header.StartingAngle / 10000.0;
  Configuration.resolution = meas_header.AngularStepWidth / 10000.0;
  Configuration.max_angle =
    ((float) meas_header.NumberMeasuredValues) * Configuration.resolution + Configuration.min_angle;
  Configuration.scanning_frequency = meas_header.ScanningFrequency;

  if (verbose == 2) printf (">>> Reading %d values from %f to %f\n",
    meas_header.NumberMeasuredValues,
    meas_header.StartingAngle / 10000.0,
    ((float) meas_header.NumberMeasuredValues) * Configuration.resolution + Configuration.min_angle);

  uint16_t distance;
  uint8_t remission;
  int index = sizeof (MeasurementHeader_t);

  // Fill in the appropriate values
  player_data.min_angle       = DTOR (Configuration.min_angle);
  player_data.max_angle       = DTOR (Configuration.max_angle);
  player_data.resolution      = DTOR (Configuration.resolution);
  player_data.max_range       = 3;
  player_data.ranges_count    = meas_header.NumberMeasuredValues;
  player_data.intensity_count = meas_header.NumberMeasuredValues;
  player_data.id              = 0;
  player_data.ranges = new float[  player_data.ranges_count];
  player_data.intensity = new uint8_t[  player_data.intensity_count];

  memcpy (&player_data.id, &buffer[sizeof(MeasurementHeader_t) +
                                 meas_header.NumberMeasuredValues * 3 +
                                 14], 2);

  // Parse the read buffer and copy values into our distance/intensity buffer
  for (int i = 0; i < meas_header.NumberMeasuredValues ; i++)
  {
    if (meas_header.Format == 0x20 || meas_header.Format == 0x21)
    {
      memcpy (&distance, (void *)&buffer[index], sizeof (uint16_t) );
      index += sizeof (uint16_t);
    }
    if (meas_header.Format == 0x20 || meas_header.Format == 0x22)
    {
      memcpy (&remission, (void *)&buffer[index], sizeof (uint8_t) );
      index += sizeof (uint8_t);
    }
    player_data.ranges[i]    = distance * meas_header.DistanceScaling / 1000.0;
    player_data.intensity[i] = remission * meas_header.RemissionScaling;

    if (verbose == 2)
      printf (" >>> [%i] dist: %i\t remission: %i\n", i,
        distance * meas_header.DistanceScaling ,
        remission * meas_header.RemissionScaling);
  }

  return (player_data);
}

////////////////////////////////////////////////////////////////////////////////
// Stop a measurement
int
  lms400_cola::StopMeasurement ()
{
  char cmd[40];
  sprintf (cmd, "sMN mLRstopdata");
  SendCommand (cmd);

  return (ReadConfirmationAndAnswer ());
}

////////////////////////////////////////////////////////////////////////////////
// Send a command to the laser unit. Returns -1 on error.
int
  lms400_cola::SendCommand (const char* cmd)
{
  if (verbose)
    printf (">> Sent: \"%s\"\n", cmd);
  assemblecommand ((unsigned char *) cmd, strlen (cmd));

  n = write (sockfd, command, commandlength);
  if (n < 0)
    return (-1);

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Read a result from the laser unit.
int
  lms400_cola::ReadResult ()
{
  bzero (buffer, 256);
  n = read (sockfd, buffer, 8);
  if (n < 0)
    return (-1);

  if (buffer[0] != 0x02 || buffer[1] != 0x02 || buffer[2] != 0x02 || buffer[3] != 0x02)
  {
    if (verbose) printf ("> E: expected 4 bytes STX's!\n");
    n = read (sockfd, buffer, 255);
    return (-1);
  }

  // Find message length
  int length = ( (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] <<  8) | (buffer[7]) );
  int current = 0;
  do
  {
    n = read (sockfd, &buffer[current], length-current);
    current += n;
  } while (current < length);

  bufferlength = length;
  if ((verbose) && (buffer[0] != 0x20))
    printf (">> Received: \"%s\"\n", buffer);

  // Check for error
  if (strncmp ((const char*)buffer, "sFA", 3) == 0)
  {
    strtok ((char*)buffer, " ");
    printf (">> E: Got an error message with code 0x%s\n", strtok (NULL, " "));
  }

  // Read checksum:
  char cs_read = 0;
  read (sockfd, &cs_read, 1);

//  printf ("%d %d 0x%x\n", bufferlength, sizeof(MeasurementHeader_t), buffer[0]);
  if (buffer[0] == 's')
    return (0);
  else if (buffer[0] == 0x20)
    return (ReadResult ());
  else if (bufferlength > sizeof (MeasurementHeader_t))
  {
    if (verbose) printf (">>>> ReadResult: probably found a data packet!\n>>>>             %s\n", buffer);
    // Don't throw away our precious measurement, queue it for later use :)
    unsigned char* tmp = (unsigned char*) malloc (bufferlength + 1);
    memcpy (tmp, buffer, bufferlength + 1);
    MeasurementQueueElement_t q;
    q.string = tmp;
    q.length = bufferlength;
    MeasurementQueue->push_back (q);
    // and then, try to read what we actually wanted to read...
    return (ReadResult ());
  }

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Read an answer from the laser unit
int
  lms400_cola::ReadAnswer ()
{
  return ReadResult ();
}

////////////////////////////////////////////////////////////////////////////////
// Read a confirmation and an answer from the laser unit
int
  lms400_cola::ReadConfirmationAndAnswer ()
{
  ReadResult ();
  if (buffer[0] == 's' && buffer[1] == 'F' && buffer[2] == 'A')
    return (-1);
  else
    return ReadResult ();
}

////////////////////////////////////////////////////////////////////////////////
// adds a header and the checksum to the command to be sent
int
  lms400_cola::assemblecommand (unsigned char* cmd, int len)
{
  unsigned char checksum = 0;
  int index = 0;

  command[0]  = 0x02;  // Messages start with 4 STX's
  command[1]  = 0x02;
  command[2]  = 0x02;
  command[3]  = 0x02;
  command[4]  = (len >> 24) & 0xff; // then message length
  command[5]  = (len >> 16) & 0xff;
  command[6]  = (len >>  8) & 0xff;
  command[7]  = (len      ) & 0xff;

  for (index = 0; index < len; index++)
  {
    command[index + 8]  = cmd[index];
    checksum ^= cmd[index];
  }
  command[8 + len] = checksum;
  command[9 + len] = 0x00;

  commandlength = 9 + len;
  return (0);
}
