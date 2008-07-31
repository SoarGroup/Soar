/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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

/*
 * $Id: sphinx2.cc 4232 2007-11-01 22:16:23Z gerkey $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_sphinx2 sphinx2
 * @brief Sphinx2 speech recognition system

@todo Add documentation about sphinx2

@par Compile-time dependencies

- none

@par Provides

- @ref interface_speech_recognition

@par Requires

- None

@par Configuration requests

- none

@par Configuration file options

- hmm_dir (string)
  - Default: "/usr/local/share/sphinx2/model/hmm/6k"
  - HMM file

- map_file (string)
  - Default: "/usr/local/share/sphinx2/model/hmm/6k/map"
  - Map file

- phone_file (string)
  - Default: "/usr/local/share/sphinx2/model/hmm/6k/phone"
  - Phone file

- noise_file (string)
  - Default: "/usr/local/share/sphinx2/model/hmm/6k/noisedict"
  - Noise dictionary file

- sendump_file (string)
  - Default: "/usr/local/share/sphinx2/model/hmm/6k/sendump"
  - Send UMP file

- task_dir (string)
  - Default: "/usr/local/share/sphinx2/model/lm/turtle"
  - Task dictionary file

- lm_file (string)
  - Default: "/usr/local/share/sphinx2/model/lm/turtle/turtle.lm"
  - LM file

- dict_file (string)
  - Default: "/usr/local/share/sphinx2/model/lm/turtle/turtle.dict"
  - Dictionary file


@par Example 

@verbatim
driver
(
  name "sphinx2"
  provides ["speech_recognition:0"]
)
@endverbatim

@author Nate Koenig

*/
/** @} */

#include <stdio.h>
#include <errno.h>
#include <unistd.h> /* close(2),fcntl(2),getpid(2),usleep(3),execlp(3),fork(2)*/
#include <netdb.h> /* for gethostbyname(3) */
#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */
#include <sys/types.h>  /* for socket(2) */
#include <sys/socket.h>  /* for socket(2) */
#include <signal.h>  /* for kill(2) */
#include <fcntl.h>  /* for fcntl(2) */
#include <string.h>  /* for strncpy(3),memcpy(3) */
#include <stdlib.h>  /* for atexit(3),atoi(3) */
#include <pthread.h>  /* for pthread stuff */
#include <libplayertcp/socket_util.h>

#include <libplayercore/playercore.h>

#ifdef __cplusplus
extern "C" {
#endif

//#include <sphinx2/s2types.h>
#include "s2types.h"
#include <sphinx2/CM_macros.h>
#include <sphinx2/ad.h>
#include <sphinx2/cont_ad.h>
#include <sphinx2/lm_3g.h>
#include <sphinx2/dict.h>
#include <sphinx2/fbs.h>
#include <sphinx2/list.h>

#ifdef __cplusplus
}
#endif


#define SAMPLE_RATE 16000

class Sphinx2 : public Driver 
{
  public:
    // constructor 
    Sphinx2( ConfigFile* cf, int section);

    virtual ~Sphinx2();

    virtual void Main();

    int Setup();
    int Shutdown();

	// This method will be invoked on each incoming message
	virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr,
                               void * data);

  private: ad_rec_t *audioDev;
  private: cont_ad_t *continuousModule;
  private: int32 startwid;
  private: const char *hmmDir;
  private: const char *mapFile;
  private: const char *phoneFile;
  private: const char *noiseFile;
  private: const char *sendumpFile;
  private: const char *taskDir;
  private: const char *lmFile;
  private: const char *dictFile;
};



// a factory creation function
Driver* Sphinx2_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new Sphinx2( cf, section)));
}

// a driver registration function
void Sphinx2_Register(DriverTable* table)
{
  table->AddDriver("sphinx2", Sphinx2_Init);
}

Sphinx2::Sphinx2( ConfigFile *cf, int section )
: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_SPEECH_RECOGNITION_CODE)
{
  this->hmmDir = cf->ReadFilename( section, "hmm_dir",
      "/usr/local/share/sphinx2/model/hmm/6k");
  this->mapFile = cf->ReadFilename( section, "map_file",
      "/usr/local/share/sphinx2/model/hmm/6k/map");
  this->phoneFile = cf->ReadFilename( section, "phone_file",
      "/usr/local/share/sphinx2/model/hmm/6k/phone");
  this->noiseFile = cf->ReadFilename( section, "noise_file",
      "/usr/local/share/sphinx2/model/hmm/6k/noisedict");
  this->sendumpFile = cf->ReadFilename( section, "sendump_file",
      "/usr/local/share/sphinx2/model/hmm/6k/sendump");

  this->taskDir = cf->ReadFilename( section, "task_dir",
      "/usr/local/share/sphinx2/model/lm/turtle");
  this->lmFile = cf->ReadFilename( section, "lm_file",
      "/usr/local/share/sphinx2/model/lm/turtle/turtle.lm");
  this->dictFile = cf->ReadFilename( section, "dict_file",
      "/usr/local/share/sphinx2/model/lm/turtle/turtle.dic");
}

Sphinx2::~Sphinx2()
{
}

int Sphinx2::Setup()
{
  int argc = 70;

  // Here are all the available options. Maybe at some point this will
  // become config file options...
  const char *argv[70] = {
                  "-hmmdir", this->hmmDir,
                  "-hmmdirlist", this->hmmDir,
                  "-cbdir", this->hmmDir,
                  "-kbdumpdir", this->taskDir,
                  "-noisedict", this->noiseFile,
                  "-phnfn", this->phoneFile,
                  "-mapfn", this->mapFile,
                  "-sendumpfn", this->sendumpFile,
                  "-lmfn", this->lmFile,
                  "-dictfn", this->dictFile,
                  "-live", "TRUE",
                  "-ctloffset", "0",
                  "-ctlcount", "100000000",
                  "-agcemax", "TRUE",
                  "-langwt", "6.5",
                  "-fwdflatlw", "8.5",
                  "-rescorelw", "9.5",
                  "-ugwt", "0.5",
                  "-fillpen", "1e-10",
                  "-silpen", "0.005",
                  "-inspen", "0.65",
                  "-top", "1",
                  "-topsenfrm", "3",
                  "-topsenthresh", "-70000",
                  "-beam", "2e-06",
                  "-npbeam", "2e-06",
                  "-lpbeam", "2e-05",
                  "-lponlybeam", "0.0005",
                  "-nwbeam", "0.0005",
                  "-fwdflat", "FALSE",
                  "-fwdflatbeam", "1e-08",
                  "-fwdflatnwbeam", "0.0003",
                  "-bestpath", "TRUE",
                  "-8bsen", "TRUE",
                  "-maxwpf", "1" // < max word repeat by sentence.
                 };

  /* Initialize recognition engine */
  fbs_init (argc, (char**)(argv));

  /* Open audio device and calibrate for background noise level */
  this->audioDev = ad_open_sps(SAMPLE_RATE);
  if (this->audioDev  == NULL)
    PLAYER_ERROR("ad_open() failed\n");

  /* now spawn reading thread */
  StartThread();

  // shut up compiler!
  return(0);
}

int Sphinx2::Shutdown()
{
  ad_stop_rec( this->audioDev );

  StopThread();

  fbs_end();
  ad_close(this->audioDev);

  return(0);
}


int Sphinx2::ProcessMessage(QueuePointer & resp_queue, 
                                  player_msghdr * hdr,
                                  void * data)
{	
	return -1;
}


void Sphinx2::Main()
{
  int sampleCount;
  int timestamp;
  struct timeval time;
  int remainingFrames;
  int frames;
  char *hypothesis;
  short audioBuff[4096];
  player_speech_recognition_data_t data;

  this->continuousModule = cont_ad_init (this->audioDev, ad_read); 

  if (this->continuousModule == NULL)
    PLAYER_ERROR("cont_ad_init failed\n");

  if (ad_start_rec( this->audioDev ) <0)
    PLAYER_ERROR("ad_start_rec failed\n");

  if (cont_ad_calib( this->continuousModule ) < 0)
    PLAYER_ERROR("cont_ad_calid failed\n");

  uttproc_init();

  /* loop and read */
  while (true) 
  {
    /* test if we are supposed to cancel */
    pthread_testcancel();

    sampleCount = 0;

    // Await data for next utterance
    if ((sampleCount = cont_ad_read(this->continuousModule, audioBuff, 4096)) == 0)
    {
      usleep(100000);
      continue;
    }

    if (sampleCount < 0)
    {
      PLAYER_ERROR("cont_ad_read failed\n");
//      continue;
    }

    //Non-zero amount of data received; start recognition of new utterance.
    //NULL argument to uttproc_begin_utt => automatic generation of utterance-id
    if (uttproc_begin_utt (NULL) < 0)
    {
      PLAYER_ERROR("uttproc_begin_utt failed\n");
 //     continue;
    }

    uttproc_rawdata (audioBuff, sampleCount, 0);
    //printf ("Listening...\n"); fflush (stdout);

    /* Note timestamp for this first block of data */
    timestamp = this->continuousModule->read_ts;
    GlobalTime->GetTime(&time);

    data.text[0] = '\0';

    /* Decode utterance until end (marked by a "long" silence, >1sec) */
    for (;;) 
    {
      if ((this->continuousModule->read_ts - timestamp) > 
          DEFAULT_SAMPLES_PER_SEC)
        break;

      // Read non-silence audio data, if any, from continuous listening module
      if ((sampleCount = cont_ad_read (this->continuousModule, audioBuff, 4096)) < 0)
      {
        PLAYER_ERROR("cont_ad_read failed\n");
  //      break;
      }

      if (sampleCount == 0) {
         // No speech data available; check current timestamp with most recent
         //speech to see if more than 1 sec elapsed.  If so, end of utterance.
        if ((this->continuousModule->read_ts - timestamp) > 
            DEFAULT_SAMPLES_PER_SEC)
          break;
      } else {
        /* New speech data received; note current timestamp */
        //timestamp = this->continuousModule->read_ts;
      }

      // Decode whatever data was read above.  NOTE: Non-blocking mode!!
      // rem = #frames remaining to be decoded upon return from the function.
      remainingFrames = uttproc_rawdata (audioBuff, sampleCount, 0);

      // If no work to be done, sleep a bit
      if ((remainingFrames == 0) && (sampleCount == 0))
        usleep(20000);
    }

     // Utterance ended; flush any accumulated, unprocessed A/D data and stop
     // listening until current utterance completely decoded
    ad_stop_rec (this->audioDev);
    while (ad_read (this->audioDev, audioBuff, 4096) >= 0);
    cont_ad_reset (this->continuousModule);

    //printf ("Stopped listening, please wait...\n"); fflush (stdout);

    /* Finish decoding, obtain and print result */
    uttproc_end_utt ();

    if (uttproc_result (&frames, &hypothesis, 1) < 0)
    {
      PLAYER_ERROR("uttproc_result failed\n");
      //continue;
    }
    
    data.text = hypothesis;
    data.text_count = strlen(data.text)+1;

    printf("data.text[%d] = %s\n",data.text_count,data.text);

    Publish(device_addr,
        PLAYER_MSGTYPE_DATA,PLAYER_SPEECH_RECOGNITION_DATA_STRING,
        (uint8_t*)&data, sizeof(data), NULL);

    // Resume A/D recording for next utterance
    if (ad_start_rec (this->audioDev) < 0)
      PLAYER_ERROR("ad_start_rec failed\n");
  }

}
