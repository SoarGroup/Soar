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
 * $Id: festival.cc 4135 2007-08-23 19:58:48Z gerkey $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_festival festival
 * @brief Festival speech synthesis system

The festival driver provides access to the
Festival speech synthesis system.  Festival is <a
href=http://www.cstr.ed.ac.uk/projects/festival/>available separately</a>
(also under the GNU GPL).  Unlike most drivers, the festival driver queues
incoming commands, rather than overwriting them.  When the queue is full,
new commands are discarded.

You must install Festival, but you don't need to run it yourself; Player
will handle starting and stopping the Festival server.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_speech

@par Requires

- None

@par Configuration requests

- none

@par Configuration file options

- port (integer)
  - Default: 1314
  - The TCP port on which the festival driver should talk to Festival.
- libdir (string)
  - Default: "/usr/local/festival/lib"
  - The path to Festival's library of phonemes and such.
 
@par Example 

@verbatim
driver
(
  name "festival"
  provides ["speech:0"]
)
@endverbatim

@author Brian Gerkey

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
//#include <socket_util.h>

#include <libplayercore/playercore.h>

/* don't change this unless you change the Festival init scripts as well*/
#define DEFAULT_FESTIVAL_PORTNUM 1314
/* change this if Festival is installed somewhere else*/
/* HHAA 14-02-2007 */
//#define DEFAULT_FESTIVAL_LIBDIR "/usr/local/festival/lib"
#define DEFAULT_FESTIVAL_LIBDIR "/usr/share/festival/"
#define DEFAULT_QUEUE_LEN 4

/* HHAA 13-02-2007 */
#define DEFAULT_FESTIVAL_LANGUAGE "english"


#include <deque>
using namespace std;

class Festival:public Driver 
{
  private:
    int pid;      // Festival's pid so we can kill it later (if necessary)

    int portnum;  // port number where Festival will run (default 1314)
    char festival_libdir_value[MAX_FILENAME_SIZE]; // the libdir
    /* HHAA 14-02-2007 */
    char festival_language[10];

    /* a queue to hold incoming speech strings */
    deque<char *> queue;
//    PlayerQueue* queue;

    bool read_pending;

  public:
    int sock;               // socket to Festival
    void KillFestival();
    
    // constructor 
    Festival( ConfigFile* cf, int section);

    ~Festival();
    virtual void Main();

    int Setup();
    int Shutdown();

    virtual int Unsubscribe(player_devaddr_t id);

    // MessageHandler
    int ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data);
};


// a factory creation function
Driver* Festival_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new Festival( cf, section)));
}

// a driver registration function
void 
Festival_Register(DriverTable* table)
{
  table->AddDriver("festival", Festival_Init);
}

#define FESTIVAL_SAY_STRING_PREFIX "(SayText \""
#define FESTIVAL_SAY_STRING_SUFFIX "\")\n"

#define FESTIVAL_QUIT_STRING "(quit)"

#define FESTIVAL_CODE_OK "LP\n"
#define FESTIVAL_CODE_ERR "ER\n"
#define FESTIVAL_RETURN_LEN 39


/* the following setting mean that we first try to connect after 1 seconds,
 * then try every 100ms for 6 more seconds before giving up */
#define FESTIVAL_STARTUP_USEC 1000000 /* wait before first connection attempt */
#define FESTIVAL_STARTUP_INTERVAL_USEC 100000 /* wait between connection attempts */
#define FESTIVAL_STARTUP_CONN_LIMIT 60 /* number of attempts to make */

/* delay inside loop */
#define FESTIVAL_DELAY_USEC 20000

void QuitFestival(void* speechdevice);

Festival::Festival( ConfigFile* cf, int section) :
  Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_SPEECH_CODE)
{
//  int queuelen;
  sock = -1;
  read_pending = false;

  portnum = cf->ReadInt(section, "port", DEFAULT_FESTIVAL_PORTNUM);
  strncpy(festival_libdir_value,
          cf->ReadString(section, "libdir", DEFAULT_FESTIVAL_LIBDIR),
          sizeof(festival_libdir_value));
  strncpy(festival_language,
          cf->ReadString(section, "language", DEFAULT_FESTIVAL_LANGUAGE),
          sizeof(festival_language));	

/*  queuelen = cf->ReadInt(section, "queuelen", DEFAULT_QUEUE_LEN);

  queue = new PlayerQueue(queuelen);
  assert(queue);*/
}

Festival::~Festival()
{
  Shutdown();
  if(sock != -1)
    QuitFestival(this);
/*  if(queue)
  {
    delete queue;
    queue = NULL;
  }*/
}

int
Festival::Setup()
{
  char festival_bin_name[] = "festival";
  char festival_server_flag[] = "--server";
  char festival_libdir_flag[] = "--libdir";
  /* HHAA 12-02-2007 */
  char festival_language_flag[] = "--language";
  /* HHAA 12-02-2007 */ 
  //char festival_language[] = "spanish";
  //char festival_libdir_value[] = DEFAULT_FESTIVAL_LIBDIR;

  int j;

  char* festival_args[8];

  static struct sockaddr_in server;
  char host[] = "localhost";
  struct hostent* entp;

  // start out with a clean slate
  //queue->Flush();
  read_pending = false;

  printf("Festival speech synthesis server connection initializing (%s,%d)...",
         festival_libdir_value,portnum);
  fflush(stdout);

  int i=0;
  festival_args[i++] = festival_bin_name;
  festival_args[i++] = festival_server_flag;
  if(strcmp(DEFAULT_FESTIVAL_LIBDIR,festival_libdir_value))
  {
    festival_args[i++] = festival_libdir_flag;
    festival_args[i++] = festival_libdir_value;
  }

  /* HHAA 13-02-2007 */
  fprintf(stdout, "festival language %s\n", festival_language);
  if(strcmp(DEFAULT_FESTIVAL_LANGUAGE,festival_language))
  {
  festival_args[i++] = festival_language_flag;
  festival_args[i++] = festival_language;
  }
  festival_args[i] = (char*)NULL;

  if(!(pid = fork()))
  {
    // make sure we don't get Festival output on console
    int dummy_fd = open("/dev/null",O_RDWR);
    dup2(dummy_fd,0);
    dup2(dummy_fd,1);
    dup2(dummy_fd,2);

    /* detach from controlling tty, so we don't get pesky SIGINTs and such */
    if(setpgid(0,0) == -1)
    {
      perror("Festival:Setup(): error while setpgrp()");
      exit(1);
    }

    if(execvp(festival_bin_name,festival_args) == -1)
    {
      /* 
       * some error.  print it here.  it will really be detected
       * later when the parent tries to connect(2) to it
       */
      perror("Festival:Setup(): error while execlp()ing Festival");
      exit(1);
    }
  }
  else
  {
    /* in parent */
    /* fill in addr structure */
    server.sin_family = PF_INET;
    /* 
     * this is okay to do, because gethostbyname(3) does no lookup if the 
     * 'host' * arg is already an IP addr
     */
    if((entp = gethostbyname(host)) == NULL)
    {
      fprintf(stderr, "Festival::Setup(): \"%s\" is unknown host; "
                      "can't connect to Festival\n", host);
      /* try to kill Festival just in case it's running */
      KillFestival();
      return(1);
    }

    memcpy(&server.sin_addr, entp->h_addr_list[0], entp->h_length);


    server.sin_port = htons(portnum);



    /* ok, we'll make this a bit smarter.  first, we wait a baseline amount
     * of time, then try to connect periodically for some predefined number
     * of times
     */
    usleep(FESTIVAL_STARTUP_USEC);

    for(j = 0;j<FESTIVAL_STARTUP_CONN_LIMIT;j++)
    {
      // make a new socket, because connect() screws with the old one somehow
      if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
      {
        perror("Festival::Setup(): socket(2) failed");
        KillFestival();
        return(1);
      }

      /* 
      * hook it up
       */
      if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0)
        break;
      usleep(FESTIVAL_STARTUP_INTERVAL_USEC);
    }
    if(j == FESTIVAL_STARTUP_CONN_LIMIT)
    {
      perror("Festival::Setup(): connect(2) failed");
      KillFestival();
      return(1);
    }
    puts("Done.");

    /* make it nonblocking */
    if(fcntl(sock,F_SETFL,O_NONBLOCK) < 0)
    {
      perror("Festival::Setup(): fcntl(2) failed");
      KillFestival();
      return(1);
    }

    /* now spawn reading thread */
    StartThread();

    return(0);
  }

  // shut up compiler!
  return(0);
}

int
Festival::Shutdown()
{
  /* if Setup() was never called, don't do anything */
  if(sock == -1)
    return(0);

  StopThread();

  sock = -1;
  puts("Festival speech server has been shutdown");
  return(0);
}


int
Festival::Unsubscribe(player_devaddr_t device)
{
	int retval = Driver::Unsubscribe(device);
	if (subscriptions == 0)
		queue.clear();
	return retval;
}

int Festival::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data)
{
	assert(hdr);
	assert(data);

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_SPEECH_CMD_SAY, device_addr))
	{
		player_speech_cmd_t * cmd = (player_speech_cmd_t *) data;
		// make ABSOLUTELY sure we've got one NULL
		cmd->string[cmd->string_count] = '\0';
	  
		Lock();
		/* if there's space, put it in the queue */
		queue.push_back(strdup(cmd->string));
		Unlock();
		return 0;
	}

	return -1;
}

void 
Festival::KillFestival()
{
  if(kill(pid,SIGHUP) == -1)
    perror("Festival::KillFestival(): some error while killing Festival");
  sock = -1;
}

void
Festival::Main()
{
  player_speech_cmd_t cmd;
  char prefix[] = FESTIVAL_SAY_STRING_PREFIX;
  char suffix[] = FESTIVAL_SAY_STRING_SUFFIX;
  // use this to hold temp data
  char buf[256];
  int numread;
  int numthisread;


  /* make sure we kill Festival on exiting */
  pthread_cleanup_push(QuitFestival,this);

  /* loop and read */
  for(;;)
  {
    /* test if we are supposed to cancel */
    pthread_testcancel();

	ProcessMessages();

    memset(&cmd,0,sizeof(cmd));
    /* do we have a string to send and is there not one pending? */
    if(!(queue.empty()) && !(read_pending))
    {
      /* send prefix to Festival */
      if(write(sock,(const void*)prefix,strlen(prefix)) == -1)
      {
        perror("festival: write() failed sending prefix; exiting.");
        break;
      }

		char * tempstr = queue.front();
		queue.pop_front();
		assert(tempstr);

		/* send the first string from the queue to Festival */
		if(write(sock,tempstr,strlen(tempstr)) == -1)
		{
			delete tempstr;
			perror("festival: write() failed sending string; exiting.");
			break;
		}
		delete tempstr;
      /* send suffix to Festival */
      if(write(sock,(const void*)suffix,strlen(suffix)) == -1)
      {
        perror("festival: write() failed sending suffix; exiting.");
        break;
      }
      read_pending = true;
    }

    /* do we have a read pending? */
    if(read_pending)
    {
      /* read the resultant string back */
      /* try to get one byte first */
      if((numread = read(sock,buf,1)) == -1)
      {
        /* was there no data? */
        if(errno == EAGAIN)
          continue;
        else
        {
          perror("festival: read() failed for code: exiting");
          break;
        }
      }

      /* now get the rest of the code */
      while((size_t)numread < strlen(FESTIVAL_CODE_OK))
      {
        /* i should really try to intrepret this some day... */
        if((numthisread = 
            read(sock,buf+numread,strlen(FESTIVAL_CODE_OK)-numread)) == -1)
        {
          /* was there no data? */
          if(errno == EAGAIN)
            continue;
          else 
          {
            perror("festival: read() failed for code: exiting");
            break;
          }
        }
        numread += numthisread;
      }
      if((size_t)numread != strlen(FESTIVAL_CODE_OK))
      {
        PLAYER_WARN2("something went wrong\n"
                "              expected %d bytes of code, but got %d\n", 
                (int) strlen(FESTIVAL_CODE_OK),numread);
        break;
      }
      // NULLify the end
      buf[numread]='\0';


      if(!strcmp(buf,FESTIVAL_CODE_OK))
      {
        /* get the other stuff that comes back */
        numread = 0;
        while(numread < FESTIVAL_RETURN_LEN)
        {
          if((numthisread = read(sock,buf+numread,
                              FESTIVAL_RETURN_LEN-numread)) == -1)
          {
            if(errno == EAGAIN)
              continue;
            else
            {
              perror("festival: read() failed for code: exiting");
              break;
            }
          }
          numread += numthisread;
        }
        if(numread != FESTIVAL_RETURN_LEN)
        {
          PLAYER_WARN("something went wrong while reading");
          break;
        }
      }
      else
      {
        /* got wrong code back */
        PLAYER_WARN1("got strange code back: %s\n", buf);
      }

      read_pending = false;
    }
    
    // so we don't run too fast
    usleep(FESTIVAL_DELAY_USEC);
  }

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

void 
QuitFestival(void* speechdevice)
{
  char quit[] = FESTIVAL_QUIT_STRING;

  Festival* sd = (Festival*)speechdevice;
  /* send quit cmd to Festival */
  if(write(sd->sock,(const void*)quit,strlen(quit)) == -1)
  {
    perror("festival: write() failed sending quit.");
  }
  /* don't know how to tell the Festival server to exit yet, so Kill */
  sd->KillFestival();
}
