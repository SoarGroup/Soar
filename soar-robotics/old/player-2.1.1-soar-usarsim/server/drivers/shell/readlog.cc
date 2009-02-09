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
 * Desc: Driver for reading log files.
 * Author: Andrew Howard
 * Date: 17 May 2003
 * CVS: $Id: readlog.cc 6566 2008-06-14 01:00:19Z thjc $
 *
 * The writelog driver will write data from another device to a log file.
 * The readlog driver will replay the data as if it can from the real sensors.
 */
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_readlog readlog
 * @brief Playback of logged data

The readlog driver can be used to replay data stored in a log file.
This is particularly useful for debugging client programs, since users
may run their clients against the same data set over and over again.
Suitable log files can be generated using the @ref driver_writelog driver.
The format for the log file can be found in the
@ref tutorial_datalog "data logging tutorial".

See below for an example configuration file; note that the device
id's specified in the provides field must match those stored in the
log file (i.e., data logged as "position2d:0" must also be read back as
"position2d:0").

For help in controlling playback, try @ref util_playervcr.
Note that you must declare a @ref interface_log device to allow
playback control.

@par Compile-time dependencies

- none

@par Provides

The readlog driver can provide the following device interfaces.

- @ref interface_laser
- @ref interface_position2d
- @ref interface_sonar
- @ref interface_wifi
- @ref interface_wsn
- @ref interface_imu
- @ref interface_pointcloud3d
- @ref interface_opaque
- @ref interface_ptz
- @ref interface_actarray

The following interfaces are supported in principle but are currently
disabled because they need to be updated:

- @ref interface_blobfinder
- @ref interface_camera
- @ref interface_fiducial
- @ref interface_gps
- @ref interface_joystick
- @ref interface_position3d

The driver also provides an interface for controlling the playback:

- @ref interface_log

@par Requires

- none

@par Configuration requests

- PLAYER_LOG_SET_READ_STATE_REQ
- PLAYER_LOG_GET_STATE_REQ
- PLAYER_LOG_SET_READ_REWIND_REQ

@par Configuration file options

- filename (filename)
  - Default: NULL
  - The log file to play back.
- speed (float)
  - Default: 1.0
  - Playback speed; 1.0 is real-time
- autoplay (integer)
  - Default: 1
  - Begin playing back log data when first client subscribes
    (as opposed to waiting for the client to tell the @ref
    interface_log device to play).
- autorewind (integer)
  - Default: 0
  - Automatically rewind and play the log file again when the end is
    reached (as opposed to not producing any more data).

@par Example

@verbatim

# Play back odometry and laser data at twice real-time from "mydata.log"
driver
(
  name "readlog"
  filename "mydata.log"
  provides ["position2d:0" "laser:0" "log:0"]
  speed 2.0
)
@endverbatim

@author Andrew Howard, Radu Bogdan Rusu

*/
/** @} */

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>

#if HAVE_ZLIB_H
  #include <zlib.h>
#endif

#include "encode.h"
#include "readlog_time.h"

#if 0
// we use this pointer to reset timestamps in the client objects when the
// log gets rewound
#include "clientmanager.h"
extern ClientManager* clientmanager;
#endif

// The logfile driver
class ReadLog: public Driver
{
  // Constructor
  public: ReadLog(ConfigFile* cf, int section);

  // Destructor
  public: ~ReadLog();

  // Initialize the driver
  public: virtual int Setup();

  // Finalize the driver
  public: virtual int Shutdown();

  // Main loop
  public: virtual void Main();

  public: virtual int ProcessMessage(QueuePointer & resp_queue,
                                     player_msghdr_t * hdr,
                                     void * data);
  // Process log interface configuration requests
  private: int ProcessLogConfig(QueuePointer & resp_queue,
                                player_msghdr_t * hdr,
                                void * data);

  // Process position interface configuration requests
  private: int ProcessPositionConfig(QueuePointer & resp_queue,
                                     player_msghdr_t * hdr,
                                     void * data);

  // Process laser interface configuration requests
  private: int ProcessLaserConfig(QueuePointer & resp_queue,
                                  player_msghdr_t * hdr,
                                  void * data);

  // Process sonar interface configuration requests
  private: int ProcessSonarConfig(QueuePointer & resp_queue,
                                  player_msghdr_t * hdr,
                                  void * data);

  // Process WSN interface configuration requests
  private: int ProcessWSNConfig(QueuePointer & resp_queue,
                                player_msghdr_t * hdr,
                                void * data);

  // Process IMU interface configuration requests
  private: int ProcessIMUConfig(QueuePointer &resp_queue,
                                player_msghdr_t * hdr,
                                void * data);

  // Parse the header info
  private: int ParseHeader(int linenum, int token_count, char **tokens,
                           player_devaddr_t *id, double *dtime,
                           unsigned short* type, unsigned short* subtype);
  // Parse some data
  private: int ParseData(player_devaddr_t id,
                         unsigned short type, unsigned short subtype,
                         int linenum, int token_count, char **tokens,
                         double time);

#if 0
  // Parse blobfinder data
  private: int ParseBlobfinder(player_devaddr_t id,
                               unsigned short type, unsigned short subtype,
                               int linenum,
                               int token_count, char **tokens, double time);

  // Parse camera data
  private: int ParseCamera(player_devaddr_t id,
                           unsigned short type, unsigned short subtype,
                           int linenum,
                          int token_count, char **tokens, double time);

  // Parse fiducial data
  private: int ParseFiducial(player_devaddr_t id,
                             unsigned short type, unsigned short subtype,
                             int linenum,
                          int token_count, char **tokens, double time);

  // Parse gps data
  private: int ParseGps(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum,
                        int token_count, char **tokens, double time);

  // Parse joystick data
  private: int ParseJoystick(player_devaddr_t id,
                             unsigned short type, unsigned short subtype,
                             int linenum,
                        int token_count, char **tokens, double time);
#endif

  // Parse laser data
  private: int ParseLaser(player_devaddr_t id,
                          unsigned short type, unsigned short subtype,
                          int linenum,
                          int token_count, char **tokens, double time);

  // Parse localize data
  private: int ParseLocalize(player_devaddr_t id, unsigned short type,
			     unsigned short subtype,
			     int linenum, int token_count,
			     char **tokens, double time);

  // Parse sonar data
  private: int ParseSonar(player_devaddr_t id,
                          unsigned short type, unsigned short subtype,
                          int linenum,
                          int token_count, char **tokens, double time);
  // Parse position data
  private: int ParsePosition(player_devaddr_t id,
                             unsigned short type, unsigned short subtype,
                             int linenum,
                             int token_count, char **tokens, double time);

  // Parse opaque data
  private: int ParseOpaque(player_devaddr_t id,
                             unsigned short type, unsigned short subtype,
                             int linenum,
                             int token_count, char **tokens, double time);

  // Parse wifi data
  private: int ParseWifi(player_devaddr_t id,
                         unsigned short type, unsigned short subtype,
                         int linenum,
                         int token_count, char **tokens, double time);
  // Parse WSN data
  private: int ParseWSN(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum,
                        int token_count, char **tokens, double time);

  // Parse IMU data
  private: int ParseIMU (player_devaddr_t id,
                         unsigned short type, unsigned short subtype,
                         int linenum,
                         int token_count, char **tokens, double time);

  // Parse PointCloud3D data
  private: int ParsePointCloud3d (player_devaddr_t id,
                    		  unsigned short type, unsigned short subtype,
                    		  int linenum,
                    		  int token_count, char **tokens, double time);

  // Parse PTZ data
  private: int ParsePTZ (player_devaddr_t id,
                         unsigned short type, unsigned short subtype,
                         int linenum,
                         int token_count, char **tokens, double time);

  // Parse Actarray data
  private: int ParseActarray (player_devaddr_t id,
                              unsigned short type, unsigned short subtype,
                              int linenum,
                              int token_count, char **tokens, double time);

  // Parse AIO data
  private: int ParseAIO(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum, int token_count, char **tokens,
                        double time);

  // Parse DIO data
  private: int ParseDIO(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum, int token_count, char **tokens,
                        double time);

  // Parse RFID data
  private: int ParseRFID(player_devaddr_t id,
                         unsigned short type, unsigned short subtype,
                         int linenum, int token_count, char **tokens,
                         double time);
#if 0

  // Parse position3d data
  private: int ParsePosition3d(player_devaddr_t id,
                               unsigned short type, unsigned short subtype,
                               int linenum,
                               int token_count, char **tokens, double time);

#endif

  // List of provided devices
  private: int provide_count;
  private: player_devaddr_t provide_ids[1024];
  // spots to cache metadata for a device (e.g., sonar geometry)
  private: void* provide_metadata[1024];

  // The log interface (at most one of these)
  private: player_devaddr_t log_id;

  // File to read data from
  private: const char *filename;
  private: FILE *file;
#if HAVE_ZLIB_H
  private: gzFile gzfile;
#endif

  // localize particles
  private: player_localize_get_particles_t particles;
  private: bool particles_set;
  private: player_devaddr_t localize_addr;


  // Input buffer
  private: size_t line_size;
  private: char *line;

  // File format
  private: char *format;

  // Playback speed (1 = real time, 2 = twice real time)
  private: double speed;

  // Playback enabled?
  public: bool enable;

  // Has a client requested that we rewind?
  public: bool rewind_requested;

  // Should we auto-rewind?  This is set in the log devie in the .cfg
  // file, and defaults to false
  public: bool autorewind;
};


////////////////////////////////////////////////////////////////////////////
// Create a driver for reading log files
Driver* ReadReadLog_Init(ConfigFile* cf, int section)
{
  return ((Driver*) (new ReadLog(cf, section)));
}


////////////////////////////////////////////////////////////////////////////
// Device factory registration
void ReadLog_Register(DriverTable* table)
{
  table->AddDriver("readlog", ReadReadLog_Init);
  return;
}


////////////////////////////////////////////////////////////////////////////
// Constructor
ReadLog::ReadLog(ConfigFile* cf, int section)
    : Driver(cf, section)
{
  int i,j;
  player_devaddr_t id;

  this->filename = cf->ReadFilename(section, "filename", NULL);
  if(!this->filename)
  {
    PLAYER_ERROR("must specify a log file to read from");
    this->SetError(-1);
    return;
  }
  this->speed = cf->ReadFloat(section, "speed", 1.0);

  this->provide_count = 0;
  memset(&this->log_id, 0, sizeof(this->log_id));
  memset(this->provide_metadata,0,sizeof(this->provide_metadata));

  particles_set = false;

  // Get a list of devices to provide
  for (i = 0; i < 1024; i++)
  {
    // TODO: fix the indexing here
    if (cf->ReadDeviceAddr(&id, section, "provides", -1, i, NULL) != 0)
      break;
    if (id.interf == PLAYER_LOG_CODE)
      this->log_id = id;
    else
      this->provide_ids[this->provide_count++] = id;
  }

  // Register the log device
  if (this->log_id.interf == PLAYER_LOG_CODE)
  {
    if (this->AddInterface(this->log_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Register all the provides devices
  for (i = 0; i < this->provide_count; i++)
  {
    if (this->AddInterface(this->provide_ids[i]) != 0)
    {
      for(j=0;j<this->provide_count;j++)
      {
        // free any allocated metadata slots
        if(this->provide_metadata[j])
        {
          free(provide_metadata[j]);
          provide_metadata[j] = NULL;
        }
      }
      this->SetError(-1);
      return;
    }

    // if it's sonar, then make a spot to cache geometry info
    if(this->provide_ids[i].interf == PLAYER_SONAR_CODE)
      assert((this->provide_metadata[i] =
              calloc(sizeof(player_sonar_geom_t),1)));

    // if it's localize, remember address
    if(this->provide_ids[i].interf == PLAYER_LOCALIZE_CODE){
      this->localize_addr = this->provide_ids[i];
    }
  }

  // Get replay options
  this->enable = cf->ReadInt(section, "autoplay", 1);
  this->autorewind = cf->ReadInt(section, "autorewind", 0);

  // Initialize other stuff
  this->format = strdup("unknown");
  this->file = NULL;
#if HAVE_ZLIB_H
  this->gzfile = NULL;
#endif

  // Set up the global time object.  We're just shoving our own in over the
  // pre-existing WallclockTime object.  Not pretty but it works.
  if(GlobalTime)
    delete GlobalTime;
  GlobalTime = new ReadLogTime();

  return;
}


////////////////////////////////////////////////////////////////////////////
// Destructor
ReadLog::~ReadLog()
{
  // Free allocated metadata slots
  for(int i=0;i<this->provide_count;i++)
  {
    if(this->provide_metadata[i])
    {
      free(this->provide_metadata[i]);
      this->provide_metadata[i] = NULL;
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////
// Initialize driver
int ReadLog::Setup()
{
  // Reset the time
  ReadLogTime_time.tv_sec = 0;
  ReadLogTime_time.tv_usec = 0;
  ReadLogTime_timeDouble = 0.0;

  // Open the file (possibly compressed)
  if (strlen(this->filename) >= 3 && \
      strcasecmp(this->filename + strlen(this->filename) - 3, ".gz") == 0)
  {
#if HAVE_ZLIB_H
    this->gzfile = gzopen(this->filename, "r");
#else
    PLAYER_ERROR("no support for reading compressed log files");
    return -1;
#endif
  }
  else
    this->file = fopen(this->filename, "r");

  /** @todo Fix support for reading gzipped files */
  if (this->file == NULL)
  {
    PLAYER_ERROR2("unable to open [%s]: %s\n", this->filename, strerror(errno));
    return -1;
  }

  // Rewind not requested by default
  this->rewind_requested = false;

  // Make some space for parsing data from the file.  This size is not
  // an exact upper bound; it's just my best guess.
  this->line_size = PLAYER_MAX_MESSAGE_SIZE;
  this->line = (char*) malloc(this->line_size);
  assert(this->line);

  // Start device thread
  this->StartThread();

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Finalize the driver
int ReadLog::Shutdown()
{
  // Stop the device thread
  this->StopThread();

  // Free allocated mem
  free(this->line);

  // Close the file
#if HAVE_ZLIB_H
  if (this->gzfile)
  {
    gzclose(this->gzfile);
    this->gzfile = NULL;
  }
#endif
  if(this->file)
  {
    fclose(this->file);
    this->file = NULL;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Driver thread
void ReadLog::Main()
{
  int ret;
  int i, len, linenum;
  bool use_stored_tokens;
  int token_count=0;
  char *tokens[4096];
  player_devaddr_t header_id, provide_id;
  struct timeval tv;
  double last_wall_time, curr_wall_time;
  double curr_log_time, last_log_time;
  unsigned short type, subtype;
  bool reading_configs;

  linenum = 0;

  last_wall_time = -1.0;
  last_log_time = -1.0;

  // First thing, we'll read all the configs from the front of the file
  reading_configs = true;
  use_stored_tokens = false;

  while (true)
  {
    pthread_testcancel();

    // Process requests
    if(!reading_configs)
      ProcessMessages();

    // If we're not supposed to playback data, sleep and loop
    if(!this->enable && !reading_configs)
    {
      usleep(10000);
      continue;
    }

    // If a client has requested that we rewind, then do so
    if(!reading_configs && this->rewind_requested)
    {
      // back up to the beginning of the file
#if HAVE_ZLIB_H
      if (this->gzfile)
        ret = gzseek(this->file,0,SEEK_SET);
      else
        ret = fseek(this->file,0,SEEK_SET);
#else
      ret = fseek(this->file,0,SEEK_SET);
#endif

      if(ret < 0)
      {
        // oh well, warn the user and keep going
        PLAYER_WARN1("while rewinding logfile, gzseek()/fseek() failed: %s",
                     strerror(errno));
      }
      else
      {
        linenum = 0;

        // reset the time
        ReadLogTime_time.tv_sec = 0;
        ReadLogTime_time.tv_usec = 0;
        ReadLogTime_timeDouble = 0.0;

#if 0
        // reset time-of-last-write in all clients
        //
        // FIXME: It's not really thread-safe to call this here, because it
        //        writes to a bunch of fields that are also being read and/or
        //        written in the server thread.  But I'll be damned if I'm
        //        going to add a mutex just for this.
        clientmanager->ResetClientTimestamps();
#endif

        // reset the flag
        this->rewind_requested = false;

        PLAYER_MSG0(2, "logfile rewound");
        continue;
      }
    }

    if(!use_stored_tokens)
    {
      // Read a line from the file; note that gzgets is really slow
      // compared to fgets (on uncompressed files), so use the latter.
#if HAVE_ZLIB_H
      if (this->gzfile)
        ret = (gzgets(this->file, this->line, this->line_size) == NULL);
      else
        ret = (fgets(this->line, this->line_size, (FILE*) this->file) == NULL);
#else
      ret = (fgets(this->line, this->line_size, (FILE*) this->file) == NULL);
#endif

      if (ret != 0)
      {
        PLAYER_MSG1(1, "reached end of log file %s", this->filename);
        // File is done, so just loop forever, unless we're on auto-rewind,
        // or until a client requests rewind.
        reading_configs = false;

        // deactivate driver so clients subscribing to the log interface will notice
        if(!this->autorewind && !this->rewind_requested)
          this->enable=false;

        while(!this->autorewind && !this->rewind_requested)
        {
          usleep(100000);
          pthread_testcancel();

          // Process requests
          this->ProcessMessages();

          ReadLogTime_timeDouble += 0.1;
          ReadLogTime_time.tv_sec = (time_t)floor(ReadLogTime_timeDouble);
          ReadLogTime_time.tv_sec = (time_t)fmod(ReadLogTime_timeDouble,1.0);
        }

        // request a rewind and start again
        this->rewind_requested = true;
        continue;
      }

      // Possible buffer overflow, so bail
      assert(strlen(this->line) < this->line_size);

      linenum += 1;

      //printf("line %d\n", linenum);
      //continue;

      // Tokenize the line using whitespace separators
      token_count = 0;
      len = strlen(line);
      for (i = 0; i < len; i++)
      {
        if (isspace(line[i]))
          line[i] = 0;
        else if (i == 0)
        {
          assert(token_count < (int) (sizeof(tokens) / sizeof(tokens[i])));
          tokens[token_count++] = line + i;
        }
        else if (line[i - 1] == 0)
        {
          assert(token_count < (int) (sizeof(tokens) / sizeof(tokens[i])));
          tokens[token_count++] = line + i;
        }
      }

      if (token_count >= 1)
      {
        // Discard comments
        if (strcmp(tokens[0], "#") == 0)
          continue;

        // Parse meta-data
        if (strcmp(tokens[0], "##") == 0)
        {
          if (token_count == 4)
          {
            free(this->format);
            this->format = strdup(tokens[3]);
          }
          continue;
        }
      }
    }
    else
      use_stored_tokens = false;

    // Parse out the header info
    if (this->ParseHeader(linenum, token_count, tokens,
                          &header_id, &curr_log_time, &type, &subtype) != 0)
      continue;

    if(reading_configs)
    {
      if(type != PLAYER_MSGTYPE_RESP_ACK)
      {
        // not a config
        reading_configs = false;
        // we'll reuse this tokenized string next time through, instead of
        // reading a fresh line from the file
        use_stored_tokens = true;
        continue;
      }
    }

    // Set the global timestamp
    ::ReadLogTime_timeDouble = curr_log_time;
    ::ReadLogTime_time.tv_sec = (time_t)floor(curr_log_time);
    ::ReadLogTime_time.tv_usec = (time_t)fmod(curr_log_time,1.0);

    gettimeofday(&tv,NULL);
    curr_wall_time = tv.tv_sec + tv.tv_usec/1e6;
    if(!reading_configs)
    {
      // Have we published at least one message from this log?
      if(last_wall_time >= 0)
      {
        // Wait until it's time to publish this message
        while((curr_wall_time - last_wall_time) <
              ((curr_log_time - last_log_time) / this->speed))
        {
          gettimeofday(&tv,NULL);
          curr_wall_time = tv.tv_sec + tv.tv_usec/1e6;
          this->ProcessMessages();
          usleep(1000);
        }
      }

      last_wall_time = curr_wall_time;
      last_log_time = curr_log_time;
    }

    // Look for a matching read interface; data will be output on
    // the corresponding provides interface.
    for (i = 0; i < this->provide_count; i++)
    {
      provide_id = this->provide_ids[i];
      if(Device::MatchDeviceAddress(header_id, provide_id))
      {
        this->ParseData(provide_id, type, subtype,
                        linenum, token_count, tokens, curr_log_time);
        break;
      }
    }
    if(i >= this->provide_count)
    {
      PLAYER_MSG6(2, "unhandled message from %d:%d:%d:%d %d:%d\n",
                  header_id.host,
                  header_id.robot,
                  header_id.interf,
                  header_id.index,
                  type, subtype);

    }
  }

  return;
}



////////////////////////////////////////////////////////////////////////////
// Process configuration requests
int ReadLog::ProcessLogConfig(QueuePointer & resp_queue,
                              player_msghdr_t * hdr,
                              void * data)
{
  player_log_set_read_state_t* sreq;
  player_log_get_state_t greq;

  switch(hdr->subtype)
  {
    case PLAYER_LOG_REQ_SET_READ_STATE:
      if(hdr->size != sizeof(player_log_set_read_state_t))
      {
        PLAYER_WARN2("request wrong size (%d != %d)",
                     hdr->size, sizeof(player_log_set_read_state_t));
        return(-1);
      }
      sreq = (player_log_set_read_state_t*)data;
      if(sreq->state)
      {
        puts("ReadLog: start playback");
        this->enable = true;
      }
      else
      {
        puts("ReadLog: stop playback");
        this->enable = false;
      }
      this->Publish(this->log_id, resp_queue,
                    PLAYER_MSGTYPE_RESP_ACK,
                    PLAYER_LOG_REQ_SET_READ_STATE);
      return(0);

    case PLAYER_LOG_REQ_GET_STATE:
      greq.type = PLAYER_LOG_TYPE_READ;
      if(this->enable)
        greq.state = 1;
      else
        greq.state = 0;

      this->Publish(this->log_id, resp_queue,
                    PLAYER_MSGTYPE_RESP_ACK,
                    PLAYER_LOG_REQ_GET_STATE,
                    (void*)&greq, sizeof(greq), NULL);
      return(0);

    case PLAYER_LOG_REQ_SET_READ_REWIND:
      // set the appropriate flag in the manager
      this->rewind_requested = true;

      this->Publish(this->log_id, resp_queue,
                    PLAYER_MSGTYPE_RESP_ACK,
                    PLAYER_LOG_REQ_SET_READ_REWIND);
      return(0);

    default:
      return(-1);
  }
}

int
ReadLog::ProcessPositionConfig(QueuePointer & resp_queue,
                               player_msghdr_t * hdr,
                               void * data)
{
  switch(hdr->subtype)
  {
    case PLAYER_POSITION2D_REQ_GET_GEOM:
      {
        // Find the right place from which to retrieve it
        int j;
        for(j=0;j<this->provide_count;j++)
        {
          if(Device::MatchDeviceAddress(this->provide_ids[j], hdr->addr))
            break;
        }
        if(j>=this->provide_count)
          return(-1);

        if(!this->provide_metadata[j])
          return(-1);

        this->Publish(this->provide_ids[j], resp_queue,
                      PLAYER_MSGTYPE_RESP_ACK, hdr->subtype,
                      this->provide_metadata[j],
                      sizeof(player_position2d_geom_t),
                      NULL);
        return(0);
      }
    default:
      return(-1);
  }
}

int
ReadLog::ProcessLaserConfig(QueuePointer & resp_queue,
                            player_msghdr_t * hdr,
                            void * data)
{
  switch(hdr->subtype)
  {
    case PLAYER_LASER_REQ_GET_GEOM:
      {
        // Find the right place from which to retrieve it
        int j;
        for(j=0;j<this->provide_count;j++)
        {
          if(Device::MatchDeviceAddress(this->provide_ids[j], hdr->addr))
            break;
        }
        if(j>=this->provide_count)
        {
          puts("no matching device");
          return(-1);
        }

        if(!this->provide_metadata[j])
        {
          puts("no metadata");
          return(-1);
        }

        this->Publish(this->provide_ids[j], resp_queue,
                      PLAYER_MSGTYPE_RESP_ACK, hdr->subtype,
                      this->provide_metadata[j],
                      sizeof(player_laser_geom_t),
                      NULL);
        return(0);
      }
    default:
      return(-1);
  }
}

int
ReadLog::ProcessSonarConfig(QueuePointer & resp_queue,
                            player_msghdr_t * hdr,
                            void * data)
{
  switch(hdr->subtype)
  {
    case PLAYER_SONAR_REQ_GET_GEOM:
      {
        // Find the right place from which to retrieve it
        int j;
        for(j=0;j<this->provide_count;j++)
        {
          if(Device::MatchDeviceAddress(this->provide_ids[j], hdr->addr))
            break;
        }
        if(j>=this->provide_count)
          return(-1);

        if(!this->provide_metadata[j])
          return(-1);

        this->Publish(this->provide_ids[j], resp_queue,
                      PLAYER_MSGTYPE_RESP_ACK, hdr->subtype,
                      this->provide_metadata[j],
                      sizeof(player_sonar_geom_t),
                      NULL);
        return(0);
      }
    default:
      return(-1);
  }
}

int
ReadLog::ProcessWSNConfig(QueuePointer & resp_queue,
                          player_msghdr_t * hdr,
                          void * data)
{
    switch(hdr->subtype)
    {
        case PLAYER_WSN_REQ_DATATYPE:
        {
        // Find the right place from which to retrieve it
            int j;
            for(j=0;j<this->provide_count;j++)
            {
                if(Device::MatchDeviceAddress(this->provide_ids[j], hdr->addr))
                    break;
            }
            if(j>=this->provide_count)
                return(-1);

            if(!this->provide_metadata[j])
                return(-1);

            this->Publish(this->provide_ids[j], resp_queue,
                          PLAYER_MSGTYPE_RESP_ACK, hdr->subtype,
                          this->provide_metadata[j],
                          sizeof(player_wsn_datatype_config_t),
                          NULL);
            return(0);
        }
        default:
            return(-1);
    }
}

int
ReadLog::ProcessIMUConfig(QueuePointer &resp_queue,
                          player_msghdr_t * hdr,
                          void * data)
{
    switch(hdr->subtype)
    {
        case PLAYER_IMU_REQ_SET_DATATYPE:
        {
        // Find the right place from which to retrieve it
            int j;
            for(j=0;j<this->provide_count;j++)
            {
                if(Device::MatchDeviceAddress(this->provide_ids[j], hdr->addr))
                    break;
            }
            if(j>=this->provide_count)
                return(-1);

            if(!this->provide_metadata[j])
                return(-1);

            this->Publish(this->provide_ids[j], resp_queue,
                          PLAYER_MSGTYPE_RESP_ACK, hdr->subtype,
                          this->provide_metadata[j],
                          sizeof(player_imu_datatype_config_t),
                          NULL);
            return(0);
        }
        default:
            return(-1);
    }
}

int
ReadLog::ProcessMessage(QueuePointer & resp_queue,
                        player_msghdr_t * hdr,
                        void * data)
{
  // Handle log config requests
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1,
                           this->log_id))
  {
    return(this->ProcessLogConfig(resp_queue, hdr, data));
  }
  else if((hdr->type == PLAYER_MSGTYPE_REQ) &&
          (hdr->addr.interf == PLAYER_LASER_CODE))
  {
    return(this->ProcessLaserConfig(resp_queue, hdr, data));
  }
  else if((hdr->type == PLAYER_MSGTYPE_REQ) &&
          (hdr->addr.interf == PLAYER_SONAR_CODE))
  {
    return(this->ProcessSonarConfig(resp_queue, hdr, data));
  }
  else if((hdr->type == PLAYER_MSGTYPE_REQ) &&
           (hdr->addr.interf == PLAYER_WSN_CODE))
  {
      return(this->ProcessWSNConfig(resp_queue, hdr, data));
  }
  else if((hdr->type == PLAYER_MSGTYPE_REQ) &&
           (hdr->addr.interf == PLAYER_IMU_CODE))
  {
      return(this->ProcessIMUConfig(resp_queue, hdr, data));
  }
  else if((hdr->type == PLAYER_MSGTYPE_REQ) &&
          (hdr->addr.interf == PLAYER_POSITION2D_CODE))
  {
    return(this->ProcessPositionConfig(resp_queue, hdr, data));
  }
  else if(particles_set &&
          Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_LOCALIZE_REQ_GET_PARTICLES,
                                this->localize_addr))
  {
    this->Publish(this->localize_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LOCALIZE_REQ_GET_PARTICLES,
                  (void*)&particles, sizeof(particles), NULL);
    return(0);
  }
  else
    return -1;
}

////////////////////////////////////////////////////////////////////////////
// Signed int conversion macros
#define NINT16(x) (htons((int16_t)(x)))
#define NUINT16(x) (htons((uint16_t)(x)))
#define NINT32(x) (htonl((int32_t)(x)))
#define NUINT32(x) (htonl((uint32_t)(x)))


////////////////////////////////////////////////////////////////////////////
// Unit conversion macros
#define M_MM(x) ((x) * 1000.0)
#define CM_MM(x) ((x) * 100.0)
#define RAD_DEG(x) ((x) * 180.0 / M_PI)


////////////////////////////////////////////////////////////////////////////
// Parse the header info
int
ReadLog::ParseHeader(int linenum, int token_count, char **tokens,
                     player_devaddr_t *id, double *dtime,
                     unsigned short* type, unsigned short* subtype)
{
  char *name;
  player_interface_t interface;

  if (token_count < 7)
  {
    PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
    return -1;
  }

  name = tokens[3];

  if (lookup_interface(name, &interface) == 0)
  {
    *dtime = atof(tokens[0]);
    id->host = atoi(tokens[1]);
    id->robot = atoi(tokens[2]);
    id->interf = interface.interf;
    id->index = atoi(tokens[4]);
    *type = atoi(tokens[5]);
    *subtype = atoi(tokens[6]);
  }
  else
  {
    PLAYER_WARN1("unknown interface name [%s]", name);
    return -1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Parse data
int ReadLog::ParseData(player_devaddr_t id,
                       unsigned short type, unsigned short subtype,
                       int linenum, int token_count, char **tokens,
                       double time)
{
#if 0
  if (id.interf == PLAYER_BLOBFINDER_CODE)
    return this->ParseBlobfinder(id, type, subtype, linenum,
                                 token_count, tokens, time);
  else if (id.interf == PLAYER_CAMERA_CODE)
    return this->ParseCamera(id, type, subtype, linenum,
                             token_count, tokens, time);
  else if (id.interf == PLAYER_FIDUCIAL_CODE)
    return this->ParseFiducial(id, type, subtype, linenum,
                               token_count, tokens, time);
  else if (id.interf == PLAYER_GPS_CODE)
    return this->ParseGps(id, type, subtype, linenum,
                          token_count, tokens, time);
  else if (id.interf == PLAYER_JOYSTICK_CODE)
    return this->ParseJoystick(id, type, subtype, linenum,
                               token_count, tokens, time);
#endif
  if (id.interf == PLAYER_LASER_CODE)
    return this->ParseLaser(id, type, subtype, linenum,
                            token_count, tokens, time);
  else if (id.interf == PLAYER_LOCALIZE_CODE)
    return this->ParseLocalize(id, type, subtype, linenum,
                               token_count, tokens, time);
  else if (id.interf == PLAYER_SONAR_CODE)
    return this->ParseSonar(id, type, subtype, linenum,
                            token_count, tokens, time);
  else if (id.interf == PLAYER_POSITION2D_CODE)
    return this->ParsePosition(id, type, subtype, linenum,
                               token_count, tokens, time);
  else if (id.interf == PLAYER_OPAQUE_CODE)
    return this->ParseOpaque(id, type, subtype, linenum,
                               token_count, tokens, time);
  else if (id.interf == PLAYER_WIFI_CODE)
    return this->ParseWifi(id, type, subtype, linenum,
                           token_count, tokens, time);
  else if (id.interf == PLAYER_WSN_CODE)
      return this->ParseWSN(id, type, subtype, linenum,
                            token_count, tokens, time);
  else if (id.interf == PLAYER_IMU_CODE)
      return this->ParseIMU (id, type, subtype, linenum,
                            token_count, tokens, time);
  else if (id.interf == PLAYER_POINTCLOUD3D_CODE)
      return this->ParsePointCloud3d (id, type, subtype, linenum,
                                      token_count, tokens, time);
  else if (id.interf == PLAYER_PTZ_CODE)
      return this->ParsePTZ (id, type, subtype, linenum,
                            token_count, tokens, time);

  else if (id.interf == PLAYER_ACTARRAY_CODE)
      return this->ParseActarray (id, type, subtype, linenum,
                                  token_count, tokens, time);
  else if (id.interf == PLAYER_AIO_CODE)
      return this->ParseAIO(id, type, subtype, linenum, token_count, tokens,
                            time);
  else if (id.interf == PLAYER_DIO_CODE)
      return this->ParseDIO(id, type, subtype, linenum, token_count, tokens,
                            time);
  else if (id.interf == PLAYER_RFID_CODE)
      return this->ParseRFID(id, type, subtype, linenum, token_count, tokens,
                            time);
#if 0
  else if (id.interf == PLAYER_POSITION3D_CODE)
    return this->ParsePosition3d(id, type, subtype, linenum,
                                 token_count, tokens, time);
#endif

  PLAYER_WARN1("unknown interface code [%s]",
               ::lookup_interface_name(0, id.interf));
  return -1;
}

#if 0
////////////////////////////////////////////////////////////////////////////
// Parse blobfinder data
int ReadLog::ParseBlobfinder(player_devaddr_t id, int linenum,
                             int token_count, char **tokens, struct timeval time)
{
  player_blobfinder_data_t data;
  player_blobfinder_blob_t *blob;
  size_t size;
  int i, blob_count;

  if (token_count < 9)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  data.width = NUINT16(atoi(tokens[6]));
  data.height = NUINT16(atoi(tokens[7]));
  blob_count = atoi(tokens[8]);
  data.blob_count = NUINT16(blob_count);

  if (token_count < 9 + blob_count * 10)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  for (i = 0; i < blob_count; i++)
  {
    blob = data.blobs + i;
    blob->id =  NINT16(atoi(tokens[9 + i]));
    blob->color = NUINT32(atoi(tokens[10 + i]));
    blob->area = NUINT32(atoi(tokens[11 + i]));
    blob->x = NUINT16(atoi(tokens[12 + i]));
    blob->y = NUINT16(atoi(tokens[13 + i]));
    blob->left = NUINT16(atoi(tokens[14 + i]));
    blob->right = NUINT16(atoi(tokens[15 + i]));
    blob->top = NUINT16(atoi(tokens[16 + i]));
    blob->bottom = NUINT16(atoi(tokens[17 + i]));
    blob->range = NUINT16(M_MM(atof(tokens[18 + i])));
  }

  size = sizeof(data) - sizeof(data.blobs) + blob_count * sizeof(data.blobs[0]);
  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0, &data, size, &time);

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Parse camera data
int ReadLog::ParseCamera(player_devaddr_t id, int linenum,
                               int token_count, char **tokens, struct timeval time)
{
  player_camera_data_t *data;
  size_t src_size, dst_size;

  if (token_count < 13)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  data = (player_camera_data_t*) malloc(sizeof(player_camera_data_t));
  assert(data);

  data->width = NUINT16(atoi(tokens[6]));
  data->height = NUINT16(atoi(tokens[7]));
  data->bpp = atoi(tokens[8]);
  data->format = atoi(tokens[9]);
  data->compression = atoi(tokens[10]);
  data->image_size = NUINT32(atoi(tokens[11]));

  // Check sizes
  src_size = strlen(tokens[12]);
  dst_size = ::DecodeHexSize(src_size);
  assert(dst_size = NUINT32(data->image_size));
  assert(dst_size < sizeof(data->image));

  // Decode string
  ::DecodeHex(data->image, dst_size, tokens[12], src_size);

  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0, data, sizeof(*data) - sizeof(data->image) + dst_size, &time);

  free(data);

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Parse fiducial data
int ReadLog::ParseFiducial(player_devaddr_t id, int linenum,
                               int token_count, char **tokens, struct timeval time)
{
  player_fiducial_data_t data;
  int fiducial_count;

  if (token_count < 7)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  fiducial_count = atoi( tokens[6] );
  data.count = NUINT16( fiducial_count );

  for( int i = 0; i < fiducial_count; i++ )
  {
    data.fiducials[i].id = NINT16( atof(tokens[13*i + 7]) );
    data.fiducials[i].pos[0] = NINT32(M_MM(atof(tokens[13*i+ 8])));
    data.fiducials[i].pos[1] = NINT32(M_MM(atof(tokens[13*i+ 9])));
    data.fiducials[i].pos[2] = NINT32(M_MM(atof(tokens[13*i+10])));
    data.fiducials[i].rot[0] = NINT32(M_MM(atof(tokens[13*i+11])));
    data.fiducials[i].rot[1] = NINT32(M_MM(atof(tokens[13*i+12])));
    data.fiducials[i].rot[2] = NINT32(M_MM(atof(tokens[13*i+13])));
    data.fiducials[i].upos[0] = NINT32(M_MM(atof(tokens[13*i+14])));
    data.fiducials[i].upos[1] = NINT32(M_MM(atof(tokens[13*i+15])));
    data.fiducials[i].upos[2] = NINT32(M_MM(atof(tokens[13*i+16])));
    data.fiducials[i].urot[0] = NINT32(M_MM(atof(tokens[13*i+17])));
    data.fiducials[i].urot[1] = NINT32(M_MM(atof(tokens[13*i+18])));
    data.fiducials[i].urot[2] = NINT32(M_MM(atof(tokens[13*i+19])));
  }

  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0, &data, sizeof(data), &time);

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Parse GPS data
int ReadLog::ParseGps(player_devaddr_t id, int linenum,
                      int token_count, char **tokens, struct timeval time)
{
  player_gps_data_t data;

  if (token_count < 17)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  data.time_sec = NUINT32((int) atof(tokens[6]));
  data.time_usec = NUINT32((int) fmod(atof(tokens[6]), 1.0));

  data.latitude = NINT32((int) (60 * 60 * 60 * atof(tokens[7])));
  data.longitude = NINT32((int) (60 * 60 * 60 * atof(tokens[8])));
  data.altitude = NINT32(M_MM(atof(tokens[9])));

  data.utm_e = NINT32(CM_MM(atof(tokens[10])));
  data.utm_n = NINT32(CM_MM(atof(tokens[11])));

  data.hdop = NINT16((int) (10 * atof(tokens[12])));
  data.hdop = NINT16((int) (10 * atof(tokens[13])));
  data.err_horz = NUINT32(M_MM(atof(tokens[14])));
  data.err_vert = NUINT32(M_MM(atof(tokens[15])));

  data.quality = atoi(tokens[16]);
  data.num_sats = atoi(tokens[17]);

  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0,&data, sizeof(data), &time);

  return 0;
}


////////////////////////////////////////////////////////////////////////////
// Parse joystick data
int ReadLog::ParseJoystick(player_devaddr_t id, int linenum,
                      int token_count, char **tokens, struct timeval time)
{
  player_joystick_data_t data;

  if (token_count < 11)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  data.xpos = NINT16((short) atoi(tokens[6]));
  data.ypos = NINT16((short) atoi(tokens[7]));
  data.xscale = NINT16((short) atoi(tokens[8]));
  data.yscale = NINT16((short) atoi(tokens[9]));
  data.buttons = NUINT16((unsigned short) (unsigned int) atoi(tokens[10]));

  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0, &data, sizeof(data), &time);

  return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////
// Parse laser data
int ReadLog::ParseLaser(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum,
                        int token_count, char **tokens, double time)
{
  int i, count, ret;
  ret = 0;
  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_LASER_DATA_SCAN:
          {
            player_laser_data_t data;

            if (token_count < 13)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }

            data.id = atoi(tokens[7]);
            data.min_angle = atof(tokens[8]);
            data.max_angle = atof(tokens[9]);
            data.resolution = atof(tokens[10]);
            data.max_range = atof(tokens[11]);
            data.ranges_count = atoi(tokens[12]);
            data.intensity_count = data.ranges_count;

            data.ranges = new float[ data.ranges_count ];
            data.intensity = new uint8_t[ data.ranges_count ];
            
            count = 0;
            for (i = 13; i < token_count; i += 2)
            {
              data.ranges[count] = atof(tokens[i + 0]);
              data.intensity[count] = atoi(tokens[i + 1]);
              count += 1;
            }

            if (count != (int)data.ranges_count)
            {
              PLAYER_ERROR2("range count mismatch at %s:%d",
                            this->filename, linenum);
              ret = -1;
            }
            else
            {
              this->Publish(id, type, subtype,
                          (void*)&data, sizeof(data), &time);
            }
            delete [] data.ranges;
            delete [] data.intensity;
            
            return ret;
          }

        case PLAYER_LASER_DATA_SCANPOSE:
          {
            player_laser_data_scanpose_t data;

            if (token_count < 16)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }

            data.scan.id = atoi(tokens[7]);
            data.pose.px = atof(tokens[8]);
            data.pose.py = atof(tokens[9]);
            data.pose.pa = atof(tokens[10]);
            data.scan.min_angle = atof(tokens[11]);
            data.scan.max_angle = atof(tokens[12]);
            data.scan.resolution = atof(tokens[13]);
            data.scan.max_range = atof(tokens[14]);
            data.scan.ranges_count = atoi(tokens[15]);
            data.scan.intensity_count = data.scan.ranges_count;

            data.scan.ranges = new float[ data.scan.ranges_count ];
            data.scan.intensity = new uint8_t[ data.scan.ranges_count ];
            
            count = 0;
            for (i = 16; i < token_count; i += 2)
            {
              data.scan.ranges[count] = atof(tokens[i + 0]);
              data.scan.intensity[count] = atoi(tokens[i + 1]);
              count += 1;
            }

            if (count != (int)data.scan.ranges_count)
            {
              PLAYER_ERROR2("range count mismatch at %s:%d",
                            this->filename, linenum);
              ret = -1;
            }
            else
            {
              this->Publish(id, type, subtype,
                          (void*)&data, sizeof(data), &time);
              delete [] data.scan.ranges;
              delete [] data.scan.intensity;
            }
            return ret;
          }

        default:
          PLAYER_ERROR1("unknown laser data subtype %d\n", subtype);
          return(-1);
      }
      break;

    case PLAYER_MSGTYPE_RESP_ACK:
      switch(subtype)
      {
        case PLAYER_LASER_REQ_GET_GEOM:
          {
            if(token_count < 12)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }

            // cache it
            player_laser_geom_t* geom =
                    (player_laser_geom_t*)calloc(1,sizeof(player_laser_geom_t));
            assert(geom);

            geom->pose.px = atof(tokens[7]);
            geom->pose.py = atof(tokens[8]);
            geom->pose.pyaw = atof(tokens[9]);
            geom->size.sl = atof(tokens[10]);
            geom->size.sw = atof(tokens[11]);

            // Find the right place to put it
            int j;
            for(j=0;j<this->provide_count;j++)
            {
              if(Device::MatchDeviceAddress(this->provide_ids[j], id))
                break;
            }
            assert(j<this->provide_count);

            if(this->provide_metadata[j])
              free(this->provide_metadata[j]);

            this->provide_metadata[j] = (void*)geom;

            // nothing to publish
            return(0);
          }

        default:
          PLAYER_ERROR1("unknown laser reply subtype %d\n", subtype);
          return(-1);
      }
      break;

    default:
      PLAYER_ERROR1("unknown laser msg type %d\n", type);
      return(-1);
  }
}



////////////////////////////////////////////////////////////////////////////
// Parse localize data
int ReadLog::ParseLocalize(player_devaddr_t id,
                           unsigned short type, unsigned short subtype,
                           int linenum,
                           int token_count, char **tokens, double time)
{
  int i, count;

  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_LOCALIZE_DATA_HYPOTHS:
          {
	    player_localize_data_t hypoths;

            if (token_count < 10)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }


	    hypoths.pending_count = atoi(tokens[7]);
	    hypoths.pending_time = atof(tokens[8]);
	    hypoths.hypoths_count = atoi(tokens[9]);

            count = 0;
            for (i = 10; i < token_count; i += 7)
            {
	      hypoths.hypoths[count].mean.px = atof(tokens[i + 0]);
	      hypoths.hypoths[count].mean.py = atof(tokens[i + 1]);
	      hypoths.hypoths[count].mean.pa = atof(tokens[i + 2]);
	      hypoths.hypoths[count].cov[0] = atof(tokens[i + 3]);
	      hypoths.hypoths[count].cov[1] = atof(tokens[i + 4]);
	      hypoths.hypoths[count].cov[2] = atof(tokens[i + 5]);
	      hypoths.hypoths[count].alpha = atof(tokens[i + 6]);
              count += 1;
            }

            if (count != (int)hypoths.hypoths_count)
            {
              PLAYER_ERROR2("hypoths count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;

            }

            this->Publish(id,  type, subtype,
                          (void*)&hypoths, sizeof(hypoths), &time);
            return(0);
          }


        default:
          PLAYER_ERROR1("unknown localize data subtype %d\n", subtype);
          return(-1);
      }
      break;

    case PLAYER_MSGTYPE_RESP_ACK:
      switch(subtype)
      {
        case PLAYER_LOCALIZE_REQ_GET_PARTICLES:
          {
            if(token_count < 12)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }


	    particles.mean.px = atof(tokens[7]);
	    particles.mean.py = atof(tokens[8]);
	    particles.mean.pa = atof(tokens[9]);
	    particles.variance = atof(tokens[10]);
	    particles.particles_count = atoi(tokens[11]);

            count = 0;
            for (i = 12; i < token_count; i += 4)
            {
              particles.particles[count].pose.px = atof(tokens[i + 0]);
	      particles.particles[count].pose.py = atof(tokens[i + 1]);
	      particles.particles[count].pose.pa = atof(tokens[i + 2]);
	      particles.particles[count].alpha = atof(tokens[i + 3]);
              count += 1;
            }

            if (count != (int)particles.particles_count)
            {
              PLAYER_ERROR2("particles count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;
            }
	    particles_set = true;

            return(0);
          }

        default:
          PLAYER_ERROR1("unknown localize reply subtype %d\n", subtype);
          return(-1);
      }
      break;

    default:
      PLAYER_ERROR1("unknown localize msg type %d\n", type);
      return(-1);
  }
}


////////////////////////////////////////////////////////////////////////////
// Parse sonar data
int ReadLog::ParseSonar(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum,
                        int token_count, char **tokens, double time)
{
  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_SONAR_DATA_RANGES:
          {
            player_sonar_data_t data;
            if(token_count < 8)
            {
              PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
              return -1;
            }
            data.ranges_count = atoi(tokens[7]);
            int count = 0;
            for(int i=8;i<token_count;i++)
            {
              data.ranges[count++] = atof(tokens[i]);
            }
            if(count != (int)data.ranges_count)
            {
              PLAYER_ERROR2("range count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;
            }
            this->Publish(id, type, subtype,
                          (void*)&data, sizeof(data), &time);
            return(0);
          }
        case PLAYER_SONAR_DATA_GEOM:
          {
            player_sonar_geom_t geom;
            if(token_count < 8)
            {
              PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
              return -1;
            }
            geom.poses_count = atoi(tokens[7]);
            int count = 0;
            for(int i=8;i<token_count;i+=3)
            {
              geom.poses[count].px = atof(tokens[i]);
              geom.poses[count].py = atof(tokens[i+1]);
              geom.poses[count].pyaw = atof(tokens[i+2]);
              count++;
            }
            if(count != (int)geom.poses_count)
            {
              PLAYER_ERROR2("range count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;
            }
            this->Publish(id, type, subtype,
                          (void*)&geom, sizeof(geom), &time);
            return(0);
          }
        default:
          PLAYER_ERROR1("unknown sonar data subtype %d\n", subtype);
          return(-1);
      }
    case PLAYER_MSGTYPE_RESP_ACK:
      switch(subtype)
      {
        case PLAYER_SONAR_REQ_GET_GEOM:
          {
            if(token_count < 8)
            {
              PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
              return -1;
            }

            // cache it
            player_sonar_geom_t* geom =
                    (player_sonar_geom_t*)calloc(1,sizeof(player_sonar_geom_t));
            assert(geom);

            geom->poses_count = atoi(tokens[7]);
            int count = 0;
            for(int i=8;i<token_count;i+=3)
            {
              geom->poses[count].px = atof(tokens[i]);
              geom->poses[count].py = atof(tokens[i+1]);
              geom->poses[count].pyaw = atof(tokens[i+2]);
              count++;
            }
            if(count != (int)geom->poses_count)
            {
              PLAYER_ERROR2("range count mismatch at %s:%d",
                            this->filename, linenum);
              free(geom);
              return -1;
            }

            // Find the right place to put it
            int j;
            for(j=0;j<this->provide_count;j++)
            {
              if(Device::MatchDeviceAddress(this->provide_ids[j], id))
                break;
            }
            assert(j<this->provide_count);

            if(this->provide_metadata[j])
              free(this->provide_metadata[j]);

            this->provide_metadata[j] = (void*)geom;

            // nothing to publish
            return(0);
          }
        default:
          PLAYER_ERROR1("unknown sonar reply subtype %d\n", subtype);
          return(-1);
      }
    default:
      PLAYER_ERROR1("unknown sonar message type %d\n", type);
      return(-1);
  }
}


////////////////////////////////////////////////////////////////////////////
// Parse position data
int
ReadLog::ParsePosition(player_devaddr_t id,
                       unsigned short type, unsigned short subtype,
                       int linenum,
                       int token_count, char **tokens, double time)
{
  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_POSITION2D_DATA_STATE:
          {
            player_position2d_data_t data;
            if(token_count < 14)
            {
              PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
              return -1;
            }
            data.pos.px = atof(tokens[7]);
            data.pos.py = atof(tokens[8]);
            data.pos.pa = atof(tokens[9]);
            data.vel.px = atof(tokens[10]);
            data.vel.py = atof(tokens[11]);
            data.vel.pa = atof(tokens[12]);
            data.stall = atoi(tokens[13]);

            this->Publish(id, type, subtype,
                          (void*)&data, sizeof(data), &time);
            return(0);
          }
        default:
          PLAYER_ERROR1("unknown position data subtype %d\n", subtype);
          return(-1);
      }
    case PLAYER_MSGTYPE_RESP_ACK:
      switch(subtype)
      {
        case PLAYER_POSITION2D_REQ_GET_GEOM:
          {
            if(token_count < 12)
            {
              PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
              return -1;
            }

            // cache it
            player_position2d_geom_t* geom =
                    (player_position2d_geom_t*)calloc(1,sizeof(player_position2d_geom_t));
            assert(geom);

            geom->pose.px = atof(tokens[7]);
            geom->pose.py = atof(tokens[8]);
            geom->pose.pyaw = atof(tokens[9]);
            geom->size.sl = atof(tokens[10]);
            geom->size.sw = atof(tokens[11]);

            // Find the right place to put it
            int j;
            for(j=0;j<this->provide_count;j++)
            {
              if(Device::MatchDeviceAddress(this->provide_ids[j], id))
                break;
            }
            assert(j<this->provide_count);

            if(this->provide_metadata[j])
              free(this->provide_metadata[j]);

            this->provide_metadata[j] = (void*)geom;

            // nothing to publish
            return(0);
          }
        default:
          PLAYER_ERROR1("unknown position reply subtype %d\n", subtype);
          return(-1);
      }
    default:
      PLAYER_ERROR1("unknown position message type %d\n", type);
      return(-1);
  }
}

////////////////////////////////////////////////////////////////////////////
// Parse opaque data
int ReadLog::ParseOpaque(player_devaddr_t id,
                        unsigned short type, unsigned short subtype,
                        int linenum,
                        int token_count, char **tokens, double time)
{
  int i, count;

  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_OPAQUE_DATA_STATE:
          {
            player_opaque_data_t data;

            if (token_count < 8)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }

            data.data_count = atoi(tokens[7]);

            count = 0;
            for (i = 8; i < token_count; i++)
            {
              data.data[count] = atoi(tokens[i]);
              count++;
            }

            if (count != (int)data.data_count)
            {
              PLAYER_ERROR2("data count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;
           }
            this->Publish(id,  type, subtype,
                          (void*)&data, sizeof(data), &time);
            return(0);
          }

        default:
          PLAYER_ERROR1("unknown opaque data subtype %d\n", subtype);
          return(-1);
      }
      break;

    case PLAYER_MSGTYPE_CMD:
      switch(subtype)
      {
        case PLAYER_OPAQUE_CMD:
          {
            player_opaque_data_t data;

            if (token_count < 8)
            {
              PLAYER_ERROR2("incomplete line at %s:%d",
                            this->filename, linenum);
              return -1;
            }

            data.data_count = atoi(tokens[7]);

            count = 0;
            for (i = 8; i < token_count; i++)
            {
              data.data[count] = atoi(tokens[i]);
              count++;
            }

            if (count != (int)data.data_count)
            {
              PLAYER_ERROR2("data count mismatch at %s:%d",
                            this->filename, linenum);
              return -1;
           }
            this->Publish(id,  type, subtype,
                          (void*)&data, sizeof(data), &time);
            return(0);
          }

        default:
          PLAYER_ERROR1("unknown opaque data subtype %d\n", subtype);
          return(-1);
      }
      break;

    default:
      PLAYER_ERROR1("unknown opaque msg type %d\n", type);
      return(-1);
  }
}

////////////////////////////////////////////////////////////////////////////
// Parse wifi data
int ReadLog::ParseWifi(player_devaddr_t id,
                       unsigned short type, unsigned short subtype,
                       int linenum,
                       int token_count, char **tokens, double time)
{
  player_wifi_data_t data;
  player_wifi_link_t *link;
  int i;
  unsigned int reported_count;

  switch(type)
  {
    case PLAYER_MSGTYPE_DATA:
      switch(subtype)
      {
        case PLAYER_WIFI_DATA_STATE:
          {
            if (token_count < 8)
            {
              PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
              return -1;
            }

            reported_count = atoi(tokens[7]);
            data.links_count = 0;
            for(i = 8; (i+8) < token_count; i += 9)
            {
              link = data.links + data.links_count;

              memcpy(link->mac, tokens[i + 0]+1, strlen(tokens[i+0])-2);
              link->mac_count = strlen(tokens[i+0])-2;
              memcpy(link->ip, tokens[i + 1]+1, strlen(tokens[i+1])-2);
              link->ip_count = strlen(tokens[i+1])-2;
              memcpy(link->essid, tokens[i + 2]+1, strlen(tokens[i+2])-2);
              link->essid_count = strlen(tokens[i+2])-2;
              link->mode = atoi(tokens[i + 3]);
              link->freq = atoi(tokens[i + 4]);
              link->encrypt = atoi(tokens[i + 5]);
              link->qual = atoi(tokens[i + 6]);
              link->level = atoi(tokens[i + 7]);
              link->noise = atoi(tokens[i + 8]);

              data.links_count++;
            }
            if(data.links_count != reported_count)
              PLAYER_WARN("read fewer wifi link entries than expected");

            this->Publish(id, type, subtype,
                          (void*)&data, sizeof(data), &time);
            return(0);
          }
        default:
          PLAYER_ERROR1("unknown wifi data subtype %d\n", subtype);
          return(-1);
      }
    default:
      PLAYER_ERROR1("unknown wifi message type %d\n", type);
      return(-1);
  }
}

////////////////////////////////////////////////////////////////////////////
// Parse WSN data
int ReadLog::ParseWSN(player_devaddr_t id,
                      unsigned short type, unsigned short subtype,
                      int linenum,
                      int token_count, char **tokens, double time)
{
    player_wsn_data_t data;

    switch(type)
    {
        case PLAYER_MSGTYPE_DATA:
            switch(subtype)
            {
                case PLAYER_WSN_DATA_STATE:
                {
                    if(token_count < 20)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
                    data.node_type      = atoi(tokens[7]);
                    data.node_id        = atoi(tokens[8]);
                    data.node_parent_id = atoi(tokens[9]);

                    data.data_packet.light       = atof(tokens[10]);
                    data.data_packet.mic         = atof(tokens[11]);
                    data.data_packet.accel_x     = atof(tokens[12]);
                    data.data_packet.accel_y     = atof(tokens[13]);
                    data.data_packet.accel_z     = atof(tokens[14]);
                    data.data_packet.magn_x      = atof(tokens[15]);
                    data.data_packet.magn_y      = atof(tokens[16]);
                    data.data_packet.magn_z      = atof(tokens[17]);
                    data.data_packet.temperature = atof(tokens[18]);
                    data.data_packet.battery     = atof(tokens[19]);

                    this->Publish(id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return(0);
                }
                default:
                    PLAYER_ERROR1("unknown WSN data subtype %d\n", subtype);
                    return(-1);
            }
        default:
            PLAYER_ERROR1("unknown WSN message type %d\n", type);
            return(-1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Parse IMU data
int ReadLog::ParseIMU (player_devaddr_t id,
                      unsigned short type, unsigned short subtype,
                      int linenum,
                      int token_count, char **tokens, double time)
{
    switch(type)
    {
        case PLAYER_MSGTYPE_DATA:
            switch(subtype)
            {
                case PLAYER_IMU_DATA_STATE:
                {
                    if (token_count < 13)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    player_imu_data_state_t data;

		    data.pose.px = atof (tokens[7]);
		    data.pose.py = atof (tokens[8]);
		    data.pose.pz = atof (tokens[9]);
		    data.pose.proll  = atof (tokens[10]);
		    data.pose.ppitch = atof (tokens[11]);
		    data.pose.pyaw   = atof (tokens[12]);

                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
                }

		case PLAYER_IMU_DATA_CALIB:
		{
                    if (token_count < 16)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    player_imu_data_calib_t data;

		    data.accel_x = atof (tokens[7]);
		    data.accel_y = atof (tokens[8]);
		    data.accel_z = atof (tokens[9]);
		    data.gyro_x  = atof (tokens[10]);
		    data.gyro_y  = atof (tokens[11]);
		    data.gyro_z  = atof (tokens[12]);
		    data.magn_x  = atof (tokens[13]);
		    data.magn_y  = atof (tokens[14]);
		    data.magn_z  = atof (tokens[15]);

                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
		}

		case PLAYER_IMU_DATA_QUAT:
		{
                    if (token_count < 20)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    player_imu_data_quat_t data;

		    data.calib_data.accel_x = atof (tokens[7]);
		    data.calib_data.accel_y = atof (tokens[8]);
		    data.calib_data.accel_z = atof (tokens[9]);
		    data.calib_data.gyro_x  = atof (tokens[10]);
		    data.calib_data.gyro_y  = atof (tokens[11]);
		    data.calib_data.gyro_z  = atof (tokens[12]);
		    data.calib_data.magn_x  = atof (tokens[13]);
		    data.calib_data.magn_y  = atof (tokens[14]);
		    data.calib_data.magn_z  = atof (tokens[15]);
		    data.q0      = atof (tokens[16]);
		    data.q1      = atof (tokens[17]);
		    data.q2      = atof (tokens[18]);
		    data.q3      = atof (tokens[19]);

                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
		}

		case PLAYER_IMU_DATA_EULER:
		{
                    if (token_count < 19)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    player_imu_data_euler_t data;

		    data.calib_data.accel_x = atof (tokens[7]);
		    data.calib_data.accel_y = atof (tokens[8]);
		    data.calib_data.accel_z = atof (tokens[9]);
		    data.calib_data.gyro_x  = atof (tokens[10]);
		    data.calib_data.gyro_y  = atof (tokens[11]);
		    data.calib_data.gyro_z  = atof (tokens[12]);
		    data.calib_data.magn_x  = atof (tokens[13]);
		    data.calib_data.magn_y  = atof (tokens[14]);
		    data.calib_data.magn_z  = atof (tokens[15]);
		    data.orientation.proll  = atof (tokens[16]);
		    data.orientation.ppitch = atof (tokens[17]);
		    data.orientation.pyaw   = atof (tokens[18]);

                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
		}
                default:
                    PLAYER_ERROR1 ("unknown IMU data subtype %d\n", subtype);
                    return (-1);
            }
        default:
            PLAYER_ERROR1 ("unknown IMU message type %d\n", type);
            return (-1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Parse PointCloud3d data
int ReadLog::ParsePointCloud3d (player_devaddr_t id,
                                unsigned short type, unsigned short subtype,
                                int linenum,
                                int token_count, char **tokens, double time)
{
    unsigned int i;
    switch(type)
    {
        case PLAYER_MSGTYPE_DATA:
            switch(subtype)
            {
                case PLAYER_POINTCLOUD3D_DATA_STATE:
                {
		    player_pointcloud3d_data_t data;
		    data.points_count = atoi (tokens[7]);
                    if (token_count < (int)(7+data.points_count))
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    for (i = 0; i < data.points_count; i++)
		    {
			player_pointcloud3d_element_t element;
                        memset(&element,0,sizeof(player_pointcloud3d_element_t));
			player_point_3d_t point;
			point.px = atof (tokens[8+i*3]);
			point.py = atof (tokens[9+i*3]);
			point.pz = atof (tokens[10+i*3]);
			element.point = point;
			data.points[i] = element;
		    }
                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
                }

                default:
                    PLAYER_ERROR1 ("unknown PointCloud3d data subtype %d\n", subtype);
                    return (-1);
            }
        default:
            PLAYER_ERROR1 ("unknown PointCloud3d message type %d\n", type);
            return (-1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Parse PTZ data
int ReadLog::ParsePTZ (player_devaddr_t id,
                      unsigned short type, unsigned short subtype,
                      int linenum,
                      int token_count, char **tokens, double time)
{
    switch(type)
    {
        case PLAYER_MSGTYPE_DATA:
            switch(subtype)
            {
                case PLAYER_PTZ_DATA_STATE:
                {
                    if (token_count < 12)
                    {
                        PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                        return -1;
                    }
		    player_ptz_data_t data;

		    data.pan  = atof (tokens[7]);
		    data.tilt = atof (tokens[8]);
		    data.zoom = atof (tokens[9]);
		    data.panspeed  = atof (tokens[10]);
		    data.tiltspeed = atof (tokens[11]);
                    this->Publish (id, type, subtype,
                                  (void*)&data, sizeof(data), &time);
                    return (0);
                }

                default:
                    PLAYER_ERROR1 ("unknown PTZ data subtype %d\n", subtype);
                    return (-1);
            }
        default:
            PLAYER_ERROR1 ("unknown PTZ message type %d\n", type);
            return (-1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Parse Actarray data
int ReadLog::ParseActarray (player_devaddr_t id,
                    	    unsigned short type, unsigned short subtype,
                            int linenum,
                            int token_count, char **tokens, double time)
{
    unsigned int i;
    switch(type)
    {
      case PLAYER_MSGTYPE_DATA:
        switch(subtype)
        {
          case PLAYER_ACTARRAY_DATA_STATE:
            player_actarray_data_t data;
            data.actuators_count = atoi (tokens[7]);
            data.actuators = new player_actarray_actuator[data.actuators_count];
            if (token_count < (int)(7+data.actuators_count))
            {
                PLAYER_ERROR2("invalid line at %s:%d", this->filename, linenum);
                return -1;
            }
            for (i = 0; i < data.actuators_count; i++)
            {
              player_actarray_actuator actuator;
              actuator.position     = atof (tokens[5*i+8]);
              actuator.speed        = atof (tokens[5*i+9]);
              actuator.acceleration = atof (tokens[5*i+10]);
              actuator.current      = atof (tokens[5*i+11]);
              actuator.state        = atoi (tokens[5*i+12]);
              data.actuators[i] = actuator;
            }
            data.motor_state = atoi (tokens[data.actuators_count*5 + 8]);

            this->Publish (id, type, subtype, (void*)&data, sizeof(data), &time);
            delete[] data.actuators;
            return (0);
          default:
            PLAYER_ERROR1 ("unknown Actarray data subtype %d\n", subtype);
            return (-1);
        }
        break;
      default:
        PLAYER_ERROR1 ("unknown Actarray message type %d\n", type);
        return (-1);
    }
}

////////////////////////////////////////////////////////////////////////////
// Parse AIO data
int ReadLog::ParseAIO(player_devaddr_t id, unsigned short type,
                      unsigned short subtype, int linenum, int token_count,
                      char **tokens, double time)
{
  switch (type) {
    case PLAYER_MSGTYPE_DATA:
      switch (subtype) {
        case PLAYER_AIO_DATA_STATE: {
            player_aio_data_t inputs;

            if (token_count < 8) {
              PLAYER_ERROR2("invalid line at %s:%d: count missing", filename,
                            linenum);
              return -1;
            }

            inputs.voltages_count = atoi(tokens[7]);

            if (token_count - 8 != (int)inputs.voltages_count) {
              PLAYER_ERROR2("invalid line at %s:%d: number of tokens does not "
                            "match count", filename, linenum);
              return -1;
            }

            char **t(tokens + 8);
            inputs.voltages = new float[inputs.voltages_count];
            for (float *v(inputs.voltages);
                 v != inputs.voltages + inputs.voltages_count; ++v, ++t)
              *v = atof(*t);

            Publish(id, type, subtype, (void *)&inputs, sizeof(inputs),
                    &time);
            delete [] inputs.voltages;
            return 0;
          }
        default:
          PLAYER_WARN3("cannot parse log of unknown aio data subtype '%d' at "
                       "%s:%d", subtype, filename, linenum);
          return -1;
      }
      default:
        PLAYER_WARN3("cannot parse log unknown of aio message type '%d' at "
                     "%s:%d", type, filename, linenum);
        return -1;
  }
}

////////////////////////////////////////////////////////////////////////////
// Parse DIO data
int ReadLog::ParseDIO(player_devaddr_t id, unsigned short type,
                      unsigned short subtype, int linenum, int token_count,
                      char **tokens, double time)
{
  switch (type) {
    case PLAYER_MSGTYPE_DATA:
      switch (subtype) {
        case PLAYER_DIO_DATA_VALUES: {
            player_dio_data_t inputs;

            if (token_count < 8) {
              PLAYER_ERROR2("invalid line at %s:%d: count missing", filename,
                            linenum);
              return -1;
            }

            inputs.count = atoi(tokens[7]);
            inputs.bits = 0;

            if (token_count - 8 != static_cast<int>(inputs.count)) {
              PLAYER_ERROR2("invalid line at %s:%d: number of tokens does not "
                            "match count", filename, linenum);
              return -1;
            }

            if (inputs.count > 32 /* MAX_INPUTS */) {
              PLAYER_ERROR2("invalid line at %s:%d: too much data for buffer",
                            filename, linenum);
              return -1;
            }

            char **t(tokens + 8);
            for (uint32_t mask(1); mask != (1ul << inputs.count);
                 mask <<=1, ++t) {
              if (strcmp(*t, "1") == 0) inputs.bits |= mask;
            }

            Publish(id, type, subtype, (void *)&inputs, sizeof(inputs),
                    &time);
            return 0;
          }
        default:
          PLAYER_WARN3("cannot parse log of unknown dio data subtype '%d' at "
                       "%s:%d", subtype, filename, linenum);
          return -1;
      }
      default:
        PLAYER_WARN3("cannot parse log of unknown dio message type '%d' at "
                     "%s:%d", type, filename, linenum);
        return -1;
  }
}

////////////////////////////////////////////////////////////////////////////
// Parse RFID data
/*
  The format changed so the rfid "type" will be saved.
  To convert the old log files to the new format use this awk filter:

  awk '/rfid/ {
           split($0, a);
           for (i=1; i<=length(a); i++)
               printf(i < 9 ? "%s " : "0001 %s ", a[i]);
           printf("\n")
       }
       !/rfid/ {
           print $0
       }'
*/
int ReadLog::ParseRFID(player_devaddr_t id, unsigned short type,
                      unsigned short subtype, int linenum, int token_count,
                      char **tokens, double time)
{
  switch (type) {
    case PLAYER_MSGTYPE_DATA:
      switch (subtype) {
        case PLAYER_RFID_DATA_TAGS: {
            player_rfid_data_t rdata;

            if (token_count < 8) {
              PLAYER_ERROR2("invalid line at %s:%d: count missing",
                            this->filename, linenum);
              return -1;
            }

            rdata.tags_count = strtoul(tokens[7], NULL, 10);

            if (token_count - 8 != 2 * static_cast<int>(rdata.tags_count)) {
              PLAYER_ERROR2("invalid line at %s:%d: number of tokens does not "
                            "match count", this->filename, linenum);
              return -1;
            }


            char **t(tokens + 8);
            rdata.tags = new player_rfid_tag_t[ rdata.tags_count];
            for (player_rfid_tag_t *r(rdata.tags);
                 r != rdata.tags + rdata.tags_count; ++r, ++t) {
              r->type = strtoul(*t, NULL, 10);
              ++t;
              r->guid_count = strlen(*t) / 2;
              r->guid = new char [r->guid_count];
              DecodeHex(r->guid, r->guid_count, *t, strlen(*t));
            }

            Publish(id, type, subtype, (void *)&rdata, sizeof(rdata),
                    &time);
            player_rfid_data_t_cleanup(&rdata);
            return 0;
          }
        default:
          PLAYER_WARN3("cannot parse log of unknown rfid data subtype '%d' at "
                       "%s:%d", subtype, filename, linenum);
          return -1;
      }
      default:
        PLAYER_WARN3("cannot parse log of unknown rfid message type '%d' at "
                     "%s:%d", type, filename, linenum);
        return -1;
  }
}

#if 0
////////////////////////////////////////////////////////////////////////////
// Parse position3d data
int ReadLog::ParsePosition3d(player_devaddr_t id, int linenum,
                             int token_count, char **tokens, struct timeval time)
{
 player_position3d_data_t data;

  if (token_count < 19)
  {
    PLAYER_ERROR2("incomplete line at %s:%d", this->filename, linenum);
    return -1;
  }

  data.xpos = NINT32(M_MM(atof(tokens[6])));
  data.ypos = NINT32(M_MM(atof(tokens[7])));
  data.zpos = NINT32(M_MM(atof(tokens[8])));

  data.roll = NINT32(1000 * atof(tokens[9]));
  data.pitch = NINT32(1000 * atof(tokens[10]));
  data.yaw = NINT32(1000 * atof(tokens[11]));

  data.xspeed = NINT32(M_MM(atof(tokens[12])));
  data.yspeed = NINT32(M_MM(atof(tokens[13])));
  data.zspeed = NINT32(M_MM(atof(tokens[14])));

  data.rollspeed = NINT32(1000 * atof(tokens[15]));
  data.pitchspeed = NINT32(1000 * atof(tokens[16]));
  data.yawspeed = NINT32(1000 * atof(tokens[17]));

  data.stall = atoi(tokens[18]);

  this->PutMsg(id,NULL,PLAYER_MSGTYPE_DATA,0, &data, sizeof(data), &time);

  return 0;
}

#endif

