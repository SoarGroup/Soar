/* This file was obtained from the nomadics respository on
   Sourceforge. I understand that it was released under the GPL by the
   copyright holders. Anyone with more information about the licensing
   or authorship of this code, please contact Richard Vaughan
   (vaughan@sfu.ca).

   $Header$
*/

/*
 * Nclient.c
 *
 * Implementation file for connection to robot and/or simulator via Nserver.
 *
 * Copyright 1991-98, Nomadic Technologies, Inc.
 *
 */

/* -- fixed a bug where initializing with an invalid robot id hung because
 *    Nclient tried to initialize sensors anyway -- rak, 16jun99
 */

/* defines */

/* generic message types */
#define MAX_MESSAGE             100

#ifndef FALSE
#define FALSE			0
#endif
#ifndef TRUE
#define TRUE			1
#endif
#define NEW_SOCKET       	2

#define MAX_VERTICES            10    /* maximum number of verices in a poly */
#define MAX_HOST_NAME_LENGTH	100
#define IS_SERVER_ALIVE_MSG	0     /* message types */
#define IS_SERVER_ALIVE_ACK	1

/* request message types */
#define AC_MSG                  10
#define SP_MSG                  11
#define VM_MSG                  12
#define PR_MSG                  13
#define MV_MSG                  43
#define PA_MSG                  14
#define GS_MSG                  15
#define CT_MSG                  16
#define ST_MSG                  17
#define LP_MSG                  18
#define TK_MSG                  19
#define ZR_MSG                  20
#define DP_MSG                  21
#define CONF_IR_MSG             22
#define CONF_SN_MSG             23
#define CONF_CP_MSG             24
#define CONF_LS_MSG             25
#define CONF_TM_MSG             26
#define GET_IR_MSG              27
#define GET_SN_MSG              28
#define GET_RC_MSG              29
#define GET_RV_MSG              30
#define GET_RA_MSG              31
#define GET_CP_MSG              32
#define GET_LS_MSG              33
#define SETUP_LS_MSG            34
#define GET_BP_MSG              35
#define CONF_SG_MSG             36
#define GET_SG_MSG              37
#define GET_RPX_MSG             75
#define RPX_MSG                 76
#define DA_MSG                  38
#define WS_MSG                  39

#define RMOVE_MSG               40
#define RPLACE_MSG              41
#define PREDICTSENSOR_MSG       42

#define ADDOBS_MSG  	        50
#define DELETEOBS_MSG           51
#define MOVEOBS_MSG             52
#define NEWWORLD_MSG            53

#define DRAWROBOT_MSG           60
#define DRAWLINE_MSG            61
#define DRAWARC_MSG             62

#define QUIT_MSG                70
#define REALROBOT_MSG           71
#define SEND_MESSAGE_MSG        72
#define RECEIVE_MESSAGE_MSG     73
#define SIMULATEDROBOT_MSG      74

#define REFRESHALL_MSG         150
#define REFRESHALLTRACES_MSG   151
#define REFRESHACTTRACE_MSG    152
#define REFRESHENCTRACE_MSG    153
#define REFRESHALLSENSORS_MSG  154
#define REFRESHBPSENSOR_MSG    155
#define REFRESHIRSENSOR_MSG    156
#define REFRESHSNSENSOR_MSG    157
#define REFRESHLSSENSOR_MSG    158
#define REFRESHGRAPHICS_MSG    159

/* reply message types */

#define STATE_MSG               80
#define MOVED_MSG               81
#define REPLY_MSG               82 
#define INFRARED_MSG            83
#define SONAR_MSG               84
#define LASER_MSG               85
#define COMPASS_MSG             86
#define BUMPER_MSG              87
#define CONFIGURATION_MSG       88
#define VELOCITY_MSG            89
#define ACCELERATION_MSG        90
#define ERROR_MSG               91

#define CREATE_ROBOT_MSG        101
#define CONNECT_ROBOT_MSG       102
#define DISCONNECT_MSG          103

#define GET_CONF_MSG            200

#define SPECIAL_MSG             300

#define MCHECK_MSG              400
#define INTERSECT_MSG           401

#define MAX_VERT     10
#define NUM_STATE    45
#define NUM_MASK     44 
#define NUM_LASER    482 
#define MAX_MESSAGE  100
#define MAX_USER_BUF 0xFFFF

#define SERIAL_ERROR 11
#define IPC_ERROR    111

/* constants */
#define MAX_VERTICES     10
#define NUM_STATE        45
#define NUM_MASK         44 
#define NUM_LASER        482 
#define MAX_MESSAGE      100


/* includes */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>          /* for "bind" system calls */
#include <sys/time.h>           /* for "select" system call */
#include <sys/socket.h>         /* for socket calls */
#ifdef _AIX32
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include "Nclient.h"

#define DEBUG
#undef DEBUG

/*
 * The voltages have different ranges to account for the fact that the
 * CPU measurement is taken after lossage on the slip-ring.
 */

#define RANGE_CPU_VOLTAGE        12.0
#define RANGE_MOTOR_VOLTAGE      12.85

/********************
 *                  *
 * Type definitions *
 *                  *
 ********************/

/*
 * PosDataAll is a struct that contains the Pos information for
 * all sensors. It is used to pass/store the Pos info within the 
 * server. It contains the laser, although the laser is not
 * used in the dual ported ram.
 */

typedef struct _PosDataAll
{
  PosData infrared [INFRAREDS];
  PosData sonar    [SONARS   ];
  PosData bumper;
  PosData laser;
  PosData compass;

} PosDataAll;


/********************
 *                  *
 * Global Variables *
 *                  *
 ********************/

long   State[NUM_STATE];
int    Smask[NUM_MASK];
int    Laser[2*NUM_LASER+1];
char   SERVER_MACHINE_NAME[80] = "";
int    SERV_TCP_PORT           = 7019;

/* dummy variables to stay compatible with Ndirect.c */

char   ROBOT_MACHINE_NAME[80] = "";
int    CONN_TYPE       = -1;
char   NOMAD_SERIAL_PORT[256] = "";
int    NOMAD_SERIAL_BAUD     = -1;
int    ROBOT_TCP_PORT  = -1;

double LASER_CALIBRATION[8];
double LASER_OFFSET[2];

/*******************
 *                 *
 * Local Variables *
 *                 *
 *******************/

static int   own_tcp_port = 0;
static int   dest_socket = 0;
static int   usedSmask[NUM_MASK];       /* mask vector */
static char  Host_name[255] = ""; /* name of the host computer where the 
				     client is running */
static int   laser_mode = 51;
static int   connectedp = 0;

static struct request_struct the_request;
static struct reply_struct the_reply;

/* this is where all the incoming posData is stored */
static PosDataAll posDataAll;
static unsigned long posDataTime;

/* for the voltages of motor/CPU as raw data */
static unsigned char voltageCPU;
static unsigned char voltageMotor;

/* function declaration */

//int gethostname(char *name, int len);
static int posDataProcess  (long *buffer, int current, PosData *posData);
static int timeDataProcess (long *buffer, int current, TimeData *theTime );
static int voltDataProcess (long *buffer, int current, 
			    unsigned char *voltCPU, unsigned char *voltMotor);
static float voltConvert   ( unsigned char reading , float range );
char *convertAddr ( char *name, char *addr );

/*************************
 *                       *
 * Some helper functions *
 *                       *
 *************************/

/*
 * open_socket_to_send_data - this function opens the socket and initializes
 *                            it properly for sending data through.  This
 *                            function returns "-1" if it cannot establish
 *                            a connection.
 */
char *convertAddr ( char *name, char *addr )
{
  int addrInt[10];

  sscanf(name, "%d.%d.%d.%d", 
	 &(addrInt[0]), &(addrInt[1]), &(addrInt[2]), &(addrInt[3]));
  addr[0] = addrInt[0];
  addr[1] = addrInt[1];
  addr[2] = addrInt[2];
  addr[3] = addrInt[3];
  return ( addr );
}


static int open_socket_to_send_data(int tcp_port_num)
{
  int s;
  struct hostent *hp;
  struct sockaddr_in serv_addr;
  char addr[10];
  
  if (!(strcmp(Host_name, "")))
  {
    if (!(strcmp(SERVER_MACHINE_NAME,"")))
      gethostname(Host_name, 100);
    else 
      strcpy(Host_name, SERVER_MACHINE_NAME);
  }
  if ( ((hp = gethostbyname(Host_name)) == NULL) &&
       ((hp = gethostbyaddr(convertAddr(Host_name,addr),4,AF_INET)) == NULL ) )
  {
    printf("host %s not valid\n", Host_name); 
#ifdef DEBUG
    perror("ERROR: in open_socket_to_send_data, gethostbyname failed");
#endif
    strcpy(Host_name, "");
    return(0);
  }
  /*
  bzero((char *) &serv_addr, sizeof(serv_addr));
  bcopy(hp->h_addr, (char *) &(serv_addr.sin_addr), hp->h_length);
  */
  memset ( (char *) &serv_addr, 0, sizeof(serv_addr) );
  memcpy ( (char *) &(serv_addr.sin_addr), hp->h_addr, hp->h_length );

  serv_addr.sin_family = AF_INET;            /* address family */
  serv_addr.sin_port = htons(tcp_port_num);  /* internet TCP port number */
  
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
#ifdef DEBUG
    printf("ERROR: in open_socket_to_send_data, socket failed.\n");
#endif
    return(0);
  }
  
  if (connect(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
#ifdef DEBUG
    printf("ERROR: in open_socket_to_send_data, connect failed.\n");
#endif
    return(-1);
  }
  
  return(s);
}

/*
 * readn - this function reads "n" bytes from a socket before returning.
 */
static int readn(register int fd, register char *ptr, register int nbytes)
{
  int nleft, nread;
  
  for (nleft = nbytes; nleft > 0; )
  {
    nread = read(fd, ptr, nleft);
    if (nread <= 0)
    {
      if (nread < 0 && (errno == EINTR || errno == EAGAIN))
      {
	nread = 0;
      } else {
	return(nread);
      }
    }
    
    nleft -= nread;
    ptr += nread;
  }
  return(nbytes - nleft);
}

/*
 * read_reply_from_socket - read reply from server
 */
static int read_reply_from_socket(int sock, struct reply_struct *this_reply)
{
  unsigned short i;
  short rep_size, rep_type;
  long n, mesg_size;
  /* 
     mesg size has to be a long, because bytes are carried as longs, thus the
     maximum size is 0xFFFF*4, which needs a long to be represented;
     this waists lots of bandwidth. I do not change it now, because we will
     move soon to a decent socket communications protocol 
     */
  n = readn(sock, (char *) &rep_type, sizeof(short));
  n = readn(sock, (char *) &rep_size, sizeof(short));
  if (n <= 0)
  {
    if (n == 0)
    {
#ifdef DEBUG
      printf("ERROR: read 0 byte\n");
#endif
      return(-1);
    }
    else
    {
#ifdef DEBUG
      printf("ERROR: in read_reply_from_socket.\n");
#endif
      return(-1);
    }
  }
  
  this_reply->type = ntohs(rep_type);
  this_reply->size = ntohs(rep_size);
  mesg_size = this_reply->size*sizeof(long);
  n = readn(sock, (char *) this_reply->mesg, mesg_size);
  
  if (n < mesg_size)
  {
#ifdef DEBUG
    printf("ERROR: in read_request_from_socket, readn 2 didn't.\n");
#endif
    return(-1);
  }
  else
  {
    for (i=0; i<this_reply->size; i++) {
      this_reply->mesg[i] = ntohl(this_reply->mesg[i]);
    }
  }
  return(this_reply->type);
}

/*
 * writen - this function writes "n" bytes down a socket before returning.
 */
static int writen(register int fd, register char *ptr, register int nbytes)
{
  int nleft, nwritten;
  
  for (nleft = nbytes; nleft > 0; )
  {
    nwritten = write(fd, ptr, nleft);
    if (nwritten <= 0)
    {
      if (nwritten < 0 && (errno == EINTR || errno == EAGAIN))
      {
	nwritten = 0;
      } else {
	return(nwritten);
      }
    }
    
    nleft -= nwritten;
    ptr += nwritten;
  }
  
  return(nbytes - nleft);
}

/* 
 * write_request_to_socket - sends the reply to the client who requested
 *                           service
 */
static int write_request_to_socket(int sock, struct request_struct *this_request)
{
  int i, request_size;
  
  /* changing to network data format before sending */
  this_request->type = htons(this_request->type);
  for (i=0; i<this_request->size; i++)
  {
    this_request->mesg[i] = htonl(this_request->mesg[i]);
  }
  request_size = 2*sizeof(short) + this_request->size*sizeof(long);
  this_request->size = htons(this_request->size);
  
  return(writen(sock, (char *)this_request, request_size));
}

/* 
 * ipc_comm - sends a request to the server and gets a reply from it
 */
static int ipc_comm(struct request_struct *this_request,
		    struct reply_struct *this_reply)
{
  int read_result;
  
  if (!connectedp)
  {
    printf("Not connected to any robot\n");
    return(FALSE);
  }
  
  if (!dest_socket)
  {
    if (own_tcp_port)
      dest_socket = open_socket_to_send_data(own_tcp_port);
    else
      dest_socket = open_socket_to_send_data(SERV_TCP_PORT);
    
    if (dest_socket <= -1)
      return(FALSE);		
  }
  write_request_to_socket(dest_socket, this_request);
  
  if (this_reply)
  {
    read_result = read_reply_from_socket(dest_socket, this_reply);
    return(read_result != -1);
  }
  else
  {
    close(dest_socket); /* this_reply is set to NULL to indicate disconnect; close the socket */
    return(TRUE);
  }
  
}

/* 
 * process_???_reply - processes reply from the server to extract the
 *                     appropriate information 
 */
static int process_state_reply(struct reply_struct *this_reply)
{
  int i,j, num_points;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    /* the state vector */

    for (i=0; i<=44; i++)
      State[i] = this_reply->mesg[i];
    num_points = this_reply->mesg[45];

    /* the laser data */

    if ((laser_mode == 1) || (laser_mode == 33))
    {
      for (i=45; i<=45+4*num_points; i++)
	Laser[i-45] = this_reply->mesg[i];
    }
    else
    {
      if ((laser_mode == 51) || (laser_mode == 50) || (laser_mode == 19))
	for (i=45; i<=45+2*num_points; i++)
	  Laser[i-45] = this_reply->mesg[i];
      else
	for (i=45; i<=45+num_points; i++)
	  Laser[i-45] = this_reply->mesg[i];
    }
    State[ STATE_ERROR ] = 0;

    /* 
     * depending on what PosData attachments were required,
     * the data have to be extracted from the buffer.
     * check for each of the sensors consecutively.
     */

    /* infrared */
    if ( POS_INFRARED_P )
      for ( j = 0; j < INFRAREDS; j++ )
	i = posDataProcess ( this_reply->mesg, i, &(posDataAll.infrared[j]) );
    
    /* sonar */
    if ( POS_SONAR_P )
      for ( j = 0; j < SONARS; j++ )
	i = posDataProcess ( this_reply->mesg, i, &(posDataAll.sonar[j]) );
    
    /* bumper */
    if ( POS_BUMPER_P )
      i = posDataProcess ( this_reply->mesg, i, &(posDataAll.bumper) );
    
    /* laser */
    if ( POS_LASER_P )
      i = posDataProcess ( this_reply->mesg, i, &(posDataAll.laser) );
    
    /* compass */
    if ( POS_COMPASS_P )
      i = posDataProcess ( this_reply->mesg, i, &(posDataAll.compass) );

    /* the Intellisys 100 time */
    i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

    /* the voltages for CPU and motors */
    i = voltDataProcess ( this_reply->mesg, i, &voltageCPU, &voltageMotor );

  }
  return(TRUE);
}

static int process_infrared_reply(struct reply_struct *this_reply)
{
  int i,j;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i= STATE_IR_0 ; i <= STATE_IR_15 ; i++)
      State[i] = this_reply->mesg[i-STATE_IR_0];

    /* 
     * if position attachment was requested for infrared....
     */
    
    i -= STATE_IR_0;
    if ( POS_INFRARED_P )
      for ( j = 0; j < INFRAREDS; j++ )
	i = posDataProcess ( this_reply->mesg, i, &(posDataAll.infrared[j]) );
  }

  /* the Intellisys 100 time */
  i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

  return(TRUE);
}

static int process_sonar_reply(struct reply_struct *this_reply)
{
  int i,j;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i = STATE_SONAR_0 ; i <= STATE_SONAR_15; i++)
      State[i] = this_reply->mesg[i - STATE_SONAR_0];

    /* 
     * if position attachment was requested for sonar....
     */
    
    i -= STATE_SONAR_0;
    if ( POS_SONAR_P )
      for ( j = 0; j < SONARS; j++ )
	i = posDataProcess ( this_reply->mesg, i, &(posDataAll.sonar[j]) );
  }

  /* the Intellisys 100 time */
  i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

  return(TRUE);
}

static int process_configuration_reply(struct reply_struct *this_reply)
{
  int i;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i = STATE_CONF_X; i <= STATE_CONF_TURRET; i++)
      State[i] = this_reply->mesg[i - STATE_CONF_X];
  }
  return(TRUE);
}

static int process_conf_reply(struct reply_struct *this_reply, long *conf)
{
  int i;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i= 0; i<4; i++)
      conf[i] = this_reply->mesg[i];
  }
  return(TRUE);
}

static int process_velocity_reply(struct reply_struct *this_reply)
{
  int i;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i = STATE_VEL_TRANS ; i <= STATE_VEL_TURRET; i++)
      State[i] = this_reply->mesg[i - STATE_VEL_TRANS];
  }
  return(TRUE);
}


static int process_compass_reply(struct reply_struct *this_reply)
{
  int i=0;

  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    State[ STATE_COMPASS ] = this_reply->mesg[0];

    /* 
     * if position attachment was requested for compass....
     */
    
    if ( POS_COMPASS_P )
      i = posDataProcess ( this_reply->mesg, 1, &(posDataAll.compass) );
  }

  /* the Intellisys 100 time */
  i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

  return(TRUE);
}

static int process_bumper_reply(struct reply_struct *this_reply)
{
  int i;

  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    State[ STATE_BUMPER ] = this_reply->mesg[0];

    /* 
     * if position attachment was requested for bumper....
     */
    
    if ( POS_BUMPER_P )
      i = posDataProcess ( this_reply->mesg, 1, &(posDataAll.bumper) );
  }

  /* the Intellisys 100 time */
  i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

  return(TRUE);
}

static int process_laser_reply(struct reply_struct *this_reply)
{
  int i, num_points;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    num_points = this_reply->mesg[0];
    
    if ((laser_mode == 1) || (laser_mode == 33))
    {
      for (i=0; i<=4*num_points; i++)
	Laser[i] = this_reply->mesg[i];
    }
    else
    {
      if ((laser_mode == 51) || (laser_mode == 50) || (laser_mode == 19))
	for (i=0; i<=2*num_points; i++)
	  Laser[i] = this_reply->mesg[i];
      else
	for (i=0; i<=num_points; i++)
	  Laser[i] = this_reply->mesg[i];
    }

    /* 
     * if position attachment was requested for laser....
     */
    
    if ( POS_LASER_P )
      i = posDataProcess ( this_reply->mesg, i, &(posDataAll.laser) );
  }

  /* the Intellisys 100 time */
  i = timeDataProcess ( this_reply->mesg, i, &posDataTime );

  return(TRUE);
}

static int process_predict_reply(struct reply_struct *this_reply, long *state,
				 int *laser)
{
  int i, num_points;
  
  for (i=1; i<33; i++)
    state[i] = this_reply->mesg[i];
  for (i=33; i<=44; i++)
    state[i] = 0;
  num_points = this_reply->mesg[44];
  if ((laser_mode == 1) || (laser_mode == 33))
  {
    for (i=44; i<=44+4*num_points; i++)
      laser[i-44] = this_reply->mesg[i];
  }
  else
  {
    if ((laser_mode == 51) || (laser_mode == 50) || (laser_mode == 19))
      for (i=44; i<=44+2*num_points; i++)
	laser[i-44] = this_reply->mesg[i];
    else
      for (i=44; i<=44+num_points; i++)
	laser[i-44] = this_reply->mesg[i];
  }
  return(TRUE);
}

static int process_simple_reply(struct reply_struct *this_reply)
{
  return(TRUE);
}

static int process_obstacle_reply(struct reply_struct *this_reply, long *obs)
{
  int i;
  
  for (i=0; i<this_reply->size; i++)
  {
    obs[i] = this_reply->mesg[i];
  }
  return(TRUE);
}

static int process_rpx_reply(struct reply_struct *this_reply, long *robot_pos)
{
  int i, num_robots;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    num_robots = this_reply->mesg[0];
    robot_pos[0] = num_robots;

    for (i=1; i<=3*num_robots; i++)
    {
      robot_pos[i] = this_reply->mesg[i];
    }
  }
  return(TRUE);
}


static int process_socket_reply(struct reply_struct *this_reply)
{
  own_tcp_port = this_reply->mesg[0];
  
  return(own_tcp_port);
}

/* this is something special for Greg at Yale */
static int process_mcheck_reply (struct reply_struct *this_reply,
				 double collide[3])
{
  if (this_reply->type == ERROR_MSG) {
    State[ STATE_ERROR ] = this_reply->mesg[0];
  }
  else {
    if (this_reply->mesg[0]) {
      collide[0] = (double)this_reply->mesg[1]/100.0;
      collide[1] = (double)this_reply->mesg[2]/100.0;
      collide[2] = (double)this_reply->mesg[3]/1000000.0;
      return(1);
    }
    else {
      return(0);
    }
  }
  return(TRUE);
}
static int process_special_reply(struct reply_struct *this_reply, unsigned char *data)
{
  int i;
  
  if (this_reply->type == ERROR_MSG)
  {
    State[ STATE_ERROR ] = this_reply->mesg[0];
    return(FALSE);
  }
  else
  {
    for (i=0; i<this_reply->size; i++)
    {
      data[i] = (unsigned char)this_reply->mesg[i];
    }
  }
  return(TRUE);
}

/*
 * bits_to_byte - converts 8 bits into one byte. Helper function for ct()
 */
static unsigned char bits_to_byte(char bt0, char bt1, char bt2, char bt3,
				  char bt4, char bt5, char bt6, char bt7)
{
  unsigned char  rbyte;
  
  if (bt0 > 0) bt0 = 1; else bt0 = 0;
  if (bt1 > 0) bt1 = 1; else bt1 = 0;
  if (bt2 > 0) bt2 = 1; else bt2 = 0;
  if (bt3 > 0) bt3 = 1; else bt3 = 0;
  if (bt4 > 0) bt4 = 1; else bt4 = 0;
  if (bt5 > 0) bt5 = 1; else bt5 = 0;
  if (bt6 > 0) bt6 = 1; else bt6 = 0;
  if (bt7 > 0) bt7 = 1; else bt7 = 0;
  
  rbyte = (unsigned char)(bt0 + (2*bt1) + (4*bt2) + (8*bt3) + (16*bt4) +
			  (32*bt5) + (64*bt6) + (128*bt7));
  return (rbyte);
}

/*****************************
 *                           *
 * Robot Interface Functions *
 *                           *
 *****************************/

/*
 * create_robot - requests the server to create a robot with
 *                id = robot_id and establishes a connection with
 *                the robot. This function is disabled in this
 *                version of the software.
 * 
 * parameters:
 *    long robot_id -- id of the robot to be created. The robot
 *                     will be referred to by this id. If a process
 *                     wants to talk (connect) to a robot, it must
 *                     know its id.
 */
int create_robot(long robot_id)
{
  pid_t process_id;
  
  process_id = getpid();
  the_request.type = CREATE_ROBOT_MSG;
  the_request.size = 2;
  the_request.mesg[0] = robot_id;
  the_request.mesg[1] = (long)process_id;
  
  if (dest_socket == -1) 
    dest_socket = 0; 
  own_tcp_port = 0; /* make sure the ipc_comm uses SERV_TCP_PORT */
  
  connectedp = 1;
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_socket_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    connectedp = 0;
    return(FALSE);
  }
}


/*
 * connect_robot - requests the server to connect to the robot
 *                 with id = robot_id. In order to talk to the server,
 *                 the SERVER_MACHINE_NAME and SERV_TCP_PORT must be
 *                 set properly. If a robot with robot_id exists,
 *                 a connection is established with that robot. If
 *                 no robot exists with robot_id, no connection is
 *                 established.
 *
 * parameters:
 *    long robot_id -- robot's id. In this multiple robot version, in order
 *                     to connect to a robot, you must know it's id.
 */
int connect_robot(long robot_id, ...)
{
  static char first = 1;
  pid_t process_id;
  int error;

  if (first)
  {
    fprintf(stderr, "Nclient version 2.7\n");
    fprintf(stderr, "Copyright 1991-1998, Nomadic Technologies, Inc.\n");
    first = 0;
  }
  
  process_id = getpid();
  the_request.type = CONNECT_ROBOT_MSG;
  the_request.size = 2;
  the_request.mesg[0] = robot_id;
  the_request.mesg[1] = process_id;
  
  if (dest_socket == -1) 
    dest_socket = 0; 
  own_tcp_port = 0; /* make sure the ipc_comm uses SERV_TCP_PORT */
  
  connectedp = 1;
  if (ipc_comm(&the_request, &the_reply))
  {
    error = process_socket_reply(&the_reply);

    /* if there was an error, we must not initialize the sensors,
     * but return an error immediately instead */
    if (error == 0)
      return 0;

    /* initialize clients Smask to match the one on the server side */
    init_sensors();

    return ( error );
  }
  else
  {
    printf("failed to connect to server on machine %s via tcp port #%d \n", 
	   Host_name, SERV_TCP_PORT);
    connectedp = 0;
    strcpy(Host_name, "");
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }

}

/*
 * disconnect_robot - requests the server to close the connect with robot
 *                    with id = robot_id. 
 *
 * parameters:
 *    long robot_id -- robot's id. In order to disconnect a robot, you
 *                     must know it's id.
 */
int disconnect_robot(long robot_id)
{
  pid_t process_id;
  
  process_id = getpid();
  the_request.type = DISCONNECT_MSG;
  the_request.size = 2;
  the_request.mesg[0] = robot_id;
  the_request.mesg[1] = process_id;
  
  
  if (ipc_comm(&the_request, 0))
  {
    dest_socket = 0;
    own_tcp_port = 0;
    connectedp = 0;
    return(TRUE);
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    connectedp = 0;
    return(FALSE);
  }
}

/* 
 * ac - sets accelerations of the robot. Currently it has no effect in 
 *      simulation mode.
 *
 * parameters:
 *    int t_ac, s_ac, r_ac -- the translation, steering, and turret
 *                            accelerations. t_ac is in 1/10 inch/sec^2
 *                            s_ac and r_ac are in 1/10 degree/sec^2.
 */
int ac(int t_ac, int s_ac, int r_ac)
{
  the_request.type = AC_MSG;
  the_request.size = 3;
  the_request.mesg[0] = t_ac;
  the_request.mesg[1] = s_ac;
  the_request.mesg[2] = r_ac;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * sp - sets speeds of the robot, this function will not cause the robot to
 *      move. However, the set speed will be used when executing a pr()
 *      or a pa().
 *
 * parameters:
 *    int t_sp, s_sp, r_sp -- the translation, steering, and turret
 *                            speeds. t_sp is in 1/10 inch/sec and
 *                            s_sp and r_sp are in 1/10 degree/sec.
 */
int sp(int t_sp, int s_sp, int r_sp)
{
  the_request.type = SP_MSG;
  the_request.size = 3;
  the_request.mesg[0] = t_sp;
  the_request.mesg[1] = s_sp;
  the_request.mesg[2] = r_sp;

  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * pr - moves the motors of the robot by a relative distance, using the speeds
 *      set by sp(). The three parameters specify the relative distances for
 *      the three motors: translation, steering, and turret. All the three
 *      motors move concurrently if the speeds are not set to zero and the 
 *      distances to be traveled are non-zero. Depending on the timeout 
 *      period set (by function conf_tm(timeout)), the motion may 
 *      terminate before the robot has moved the specified distances
 *
 * parameters:
 *    int t_pr, s_pr, r_pr -- the specified relative distances of the
 *                            translation, steering, and turret motors.
 *                            t_pr is in 1/10 inch and s_pr and r_pr are
 *                            in 1/10 degrees.
 */
int pr(int t_pr, int s_pr, int r_pr)
{
  the_request.type = PR_MSG;
  the_request.size = 3;
  the_request.mesg[0] = t_pr;
  the_request.mesg[1] = s_pr;
  the_request.mesg[2] = r_pr;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * pa - moves the motors of the robot to the specified absolute positions 
 *      using the speeds set by sp().  Depending on the timeout period set 
 *      (by conf_tm()), the motion may terminate before the robot has 
 *      moved to the specified positions.
 *
 * parameters:
 *    int t_pa, s_pa, r_pa -- the specified absolute positions of the
 *                            translation, steering, and turret motors.
 *                            t_pa is in 1/10 inch and s_pa and r_pa are
 *                            in 1/10 degrees.
 */
int pa(int t_pa, int s_pa, int r_pa);
int pa(int t_pa, int s_pa, int r_pa)
{
  return(FALSE);

  the_request.type = PA_MSG;
  the_request.size = 3;
  the_request.mesg[0] = t_pa;
  the_request.mesg[1] = s_pa;
  the_request.mesg[2] = r_pa;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * vm - velocity mode, command the robot to move at translational
 *      velocity = tv, steering velocity = sv, and rotational velocity =
 *      rv. The robot will continue to move at these velocities until
 *      either it receives another command or this command has been
 *      timeout (in which case it will stop its motion).
 *
 * parameters: 
 *    int t_vm, s_vm, r_vm -- the desired translation, steering, and turret
 *                            velocities. tv is in 1/10 inch/sec and
 *                            sv and rv are in 1/10 degree/sec.
 */
int vm(int t_vm, int s_vm, int r_vm)
{
  the_request.type = VM_MSG;
  the_request.size = 3;
  the_request.mesg[0] = t_vm;
  the_request.mesg[1] = s_vm;
  the_request.mesg[2] = r_vm;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}


/*
 * mv - move, send a generalized motion command to the robot.
 *      For each of the three axis (translation, steering, and
 *      turret) a motion mode (t_mode, s_mode, r_mode) can be 
 *      specified (using the values MV_IGNORE, MV_AC, MV_SP,
 *      MV_LP, MV_VM, and MV_PR defined above):
 *
 *         MV_IGNORE : the argument for this axis is ignored
 *                     and the axis's motion will remain 
 *                     unchanged.
 *         MV_AC :     the argument for this axis specifies
 *                     an acceleration value that will be used
 *                     during motion commands.
 *         MV_SP :     the argument for this axis specifies
 *                     a speed value that will be used during
 *                     position relative (PR) commands.
 *         MV_LP :     the arguemnt for this axis is ignored
 *                     but the motor is turned off.
 *         MV_VM :     the argument for this axis specifies
 *                     a velocity and the axis will be moved
 *                     with this velocity until a new motion
 *                     command is issued (vm,pr,mv) or 
 *                     recieves a timeout.
 *         MV_PR :     the argument for this axis specifies
 *                     a position and the axis will be moved
 *                     to this position, unless this command
 *                     is overwritten by another (vm,pr,mv).
 *
 * parameters: 
 *    int t_mode - the desired mode for the tranlation axis
 *    int t_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 *    int s_mode - the desired mode for the steering axis
 *    int s_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 *    int r_mode - the desired mode for the turret axis
 *    int r_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 */
int mv(int t_mode, int t_mv, int s_mode, int s_mv, int r_mode, int r_mv)
{
  /* check if the modes are correct */
  if (((t_mode != MV_IGNORE) && (t_mode != MV_AC) && (t_mode != MV_SP) &&
       (t_mode != MV_LP) && (t_mode != MV_VM) && (t_mode != MV_PR)) ||
      ((s_mode != MV_IGNORE) && (s_mode != MV_AC) && (s_mode != MV_SP) &&
       (s_mode != MV_LP) && (s_mode != MV_VM) && (s_mode != MV_PR)) ||
      ((r_mode != MV_IGNORE) && (r_mode != MV_AC) && (r_mode != MV_SP) &&
       (r_mode != MV_LP) && (r_mode != MV_VM) && (r_mode != MV_PR)))
    return ( FALSE );

  /* build the request packet */
  the_request.type = MV_MSG;
  the_request.size = 6;
  the_request.mesg[0] = t_mode;
  the_request.mesg[1] = t_mv;
  the_request.mesg[2] = s_mode;
  the_request.mesg[3] = s_mv;
  the_request.mesg[4] = r_mode;
  the_request.mesg[5] = r_mv;
 
  /* communicate with robot */
  if (ipc_comm(&the_request, &the_reply))
  {
    /* process the reply packet that contains the state info */
    return(process_state_reply(&the_reply));
  }
  else
  {
    /* indicate IPC_ERROR */
    State[ STATE_ERROR ] = IPC_ERROR;  
    return(FALSE);
  }
}

/*
 * ct - send the sensor mask, Smask, to the robot. You must first change
 *      the global variable Smask to the desired communication mask before
 *      calling this function. 
 *
 *      to avoid inconsistencies usedSmask is used in all other function.
 *      once ct is called the user accessible mask Smask is used to 
 *      redefine usedSmask. This avoids that a returning package is encoded
 *      with a different mask than the one it was sent with, in case
 *      the mask has been changed on the client side, but has not been 
 *      updated on the server side.
 */
int ct(void)
{
  int i;
  unsigned char b0, b1, b2, b3, b4, b5, b6;
  
  for ( i = 0; i < NUM_MASK; i++ )
    usedSmask[i] = Smask[i];

  /* first encode Smask */
  b0 = bits_to_byte (Smask[1], Smask[2], Smask[3], Smask[4],
		     Smask[5], Smask[6], Smask[7], Smask[8]);
  b1 = bits_to_byte (Smask[9], Smask[10], Smask[11], Smask[12],
		     Smask[13], Smask[14], Smask[15], Smask[16]);
  b2 = bits_to_byte (Smask[17], Smask[18], Smask[19], Smask[20],
		     Smask[21], Smask[22], Smask[23], Smask[24]);
  b3 = bits_to_byte (Smask[25], Smask[26], Smask[27], Smask[28],
		     Smask[29], Smask[30], Smask[31], Smask[32]);
  b4 = bits_to_byte (Smask[33], Smask[34], Smask[35], Smask[36],
		     Smask[37], Smask[38], Smask[39], Smask[40]);
  b5 = bits_to_byte (Smask[0], Smask[41], Smask[42], Smask[43], 
		     0,0,0,0);
  b6 = (unsigned char) Smask[0];
  
  the_request.type = CT_MSG;
  the_request.size = 7;
  the_request.mesg[0] = b0;
  the_request.mesg[1] = b1;
  the_request.mesg[2] = b2;
  the_request.mesg[3] = b3;
  the_request.mesg[4] = b4;
  the_request.mesg[5] = b5;
  the_request.mesg[6] = b6;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * gs - get the current state of the robot according to the mask (of 
 *      the communication channel)
 */
int gs(void)
{
  the_request.type = GS_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * st - stops the robot (the robot holds its current position)
 */
int st(void)
{
  the_request.type = ST_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * lp - set motor limp (the robot may not hold its position).
 */
int lp(void)
{
  the_request.type = LP_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * tk - sends the character stream, talk_string, to the voice synthesizer
 *      to make the robot talk.
 *
 * parameters:
 *    char *talk_string -- the string to be sent to the synthesizer.
 */
int tk(char *talk_string)
{
  the_request.type = TK_MSG;
  the_request.size = (strlen(talk_string)+4)/4;
  strcpy((char *)the_request.mesg, talk_string);
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * dp - define the current position of the robot as (x,y)
 * 
 * parameters:
 *    int x, y -- the position to set the robot to.
 */
int dp(int x, int y)
{
  the_request.type = DP_MSG;
  the_request.size = 2;
  the_request.mesg[0] = x;
  the_request.mesg[1] = y;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * zr - zeroing the robot, align steering and turret with bumper zero.
 *      The position, steering and turret angles are all set to zero.
 *      This function returns when the zeroing process has completed.
 */
int zr(void)
{
  the_request.type = ZR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_ir - configure infrared sensor system.
 *
 * parameters: 
 *    int history -- specifies the percentage dependency of the current 
 *                   returned reading on the previous returned reading.
 *                   It should be set between 0 and 10: 0 = no dependency 
 *                   10 = full dependency, i.e. the reading will not change
 *    int order[16] --  specifies the firing sequence of the infrared 
 *                      (#0 .. #15). You can terminate the order list by a 
 *                      "255". For example, if you want to use only the 
 *                      front three infrared sensors then set order[0]=0,
 *                      order[1]=1, order[2]=15, order[3]=255 (terminator).
 */
int conf_ir(int history, int order[16])
{
  int i;
  
  the_request.type = CONF_IR_MSG;
  the_request.size = 17;
  the_request.mesg[0] = history;
  for (i=0; i<16; i++)
  {
    the_request.mesg[i+1] = order[i];
  }
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_sn - configure sonar sensor system.
 *
 * parameters:
 *    int rate -- specifies the firing rate of the sonar in 4 milli-seconds 
 *                interval; 
 *    int order[16] -- specifies the firing sequence of the sonar (#0 .. #15).
 *                     You can terminate the order list by a "255". For 
 *                     example, if you want to use only the front three 
 *                     sensors, then set order[0]=0, order[1]=1, order[2]=15, 
 *                     order[3]=255 (terminator).
 */
int conf_sn(int rate, int order[16])
{
  int i;
  
  the_request.type = CONF_SN_MSG;
  the_request.size = 17;
  the_request.mesg[0] = rate;
  for (i=0; i<16; i++)
  {
    the_request.mesg[i+1] = order[i];
  }
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_cp - configure compass system.
 * 
 * parameters:
 *    int mode -- specifies compass on/off: 0 = off ; 1 = on; 2 = calibrate.
 *                When you call conf_cp (2), the robot will rotate slowly 360
 *                degrees. You must wake till the robot stop rotating before
 *                issuing another command to the robot (takes ~3 minutes).
 */
int conf_cp(int mode)
{
  the_request.type = CONF_CP_MSG;
  the_request.mesg[0] = mode;
  the_request.size = 1;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_ls - configure laser sensor system:
 *
 * parameters:
 *    unsigned int mode -- specifies the on-board processing mode of the laser 
 *                         sensor data which determines the mode of the data 
 *                         coming back: 
 *                           the first bit specifies the on/off;
 *                           the second bit specifies point/line mode;
 *                           the third to fifth bits specify the 
 *                           returned data types: 
 *                             000 = peak pixel, 
 *                             001 = rise pixel, 
 *                             010 = fall pixel, 
 *                             011 = magnitude,
 *                             100 = distance;
 *                           the sixth bit specifies data integrity checking.
 *
 *   unsigned int threshold -- specifies the inverted acceptable brightness
 *                             of the laser line. 
 *
 *   unsigned int width -- specifies the acceptable width in terms
 *                         of number of pixels that are brighter than the 
 *                         set threshold.
 *  
 *   unsigned int num_data -- specifies the number of sampling points. 
 *   unsigned int processing --  specifies the number of neighboring 
 *                               pixels for averaging
 *
 * If you don't understand the above, try this one:
 *   conf_ls(51, 20, 20, 20, 4)
 */
int conf_ls(unsigned int mode, unsigned int threshold, unsigned int width,
	    unsigned int num_data, unsigned int processing)
{
  the_request.type = CONF_LS_MSG;
  the_request.size = 5;
  the_request.mesg[0] = mode;
  the_request.mesg[1] = threshold;
  the_request.mesg[2] = width;
  the_request.mesg[3] = num_data;
  the_request.mesg[4] = processing;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    laser_mode = mode;
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_tm - sets the timeout period of the robot in seconds. If the
 *           robot has not received a command from the host computer
 *           for more than the timeout period, it will abort its 
 *           current motion
 * 
 * parameters:
 *    unsigned int timeout -- timeout period in seconds. If it is 0, there
 *                            will be no timeout on the robot.
 */
int conf_tm(unsigned char timeout)
{
  the_request.type = CONF_TM_MSG;
  the_request.size = 1;
  the_request.mesg[0] = timeout;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_ir - get infrared data, independent of mask. However, only 
 *          the active infrared sensor readings are valid. It updates
 *          the State vector.
 */
int get_ir(void)
{
  the_request.type = GET_IR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_infrared_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_sn - get sonar data, independent of mask. However, only 
 *          the active sonar sensor readings are valid. It updates
 *          the State vector.
 */
int get_sn(void)
{
  the_request.type = GET_SN_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_sonar_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_rc - get robot configuration data (x, y, th, tu), independent of 
 *          mask. It updates the State vector.
 */
int get_rc(void)
{
  the_request.type = GET_RC_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_configuration_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_rv - get robot velocities (translation, steering, and turret) data,
 *          independent of mask. It updates the State vector.
 */
int get_rv(void)
{
  the_request.type = GET_RV_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_velocity_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_ra - get robot acceleration (translation, steering, and turret) data,
 *          independent of mask. It updates the State vector.
 */
int get_ra(void)
{
  the_request.type = GET_RA_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_velocity_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_cp - get compass data, independent of mask. However, the
 *          data is valid only if the compass is on. It updates the
 *          State vector.
 */
int get_cp(void)
{
  the_request.type = GET_CP_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_compass_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_ls - get laser data point mode, independent of mask. However the
 *          data is valid only of the laser is on. It updates the Laser 
 *          vector.
 */
int get_ls(void)
{
  int temp_laser_mode;
  
  the_request.type = GET_LS_MSG;
  the_request.size = 0;
  
  temp_laser_mode = laser_mode;
  if ((laser_mode == 33) || (laser_mode == 1))
    laser_mode = 51;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_laser_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
  
  laser_mode = temp_laser_mode;
}

/* ##### setup_ls() is obsolete */

/*
 * get_bp - get bumper data, independent of mask. It updates the State
 *          vector.
 */
int get_bp(void)
{
  the_request.type = GET_BP_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_bumper_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * conf_sg - configure laser sensor system line segment processing mode:
 *
 * parameters:
 *    unsigned int threshold -- specifies the threshold value for least-square
 *                             fitting. When the error term grows above the 
 *                             threshold, the line segment will be broken
 *    unsigned int min_points -- specifies the acceptable number of points
 *                              to form a line segment.
 *    unsigned int gap -- specifies the acceptable "gap" between two segments
 *                        while they can still be treated as one (in 1/10 inch)
 *
 * If you don't understand the above, try this one:
 *    conf_sg(50, 4, 30)
 */
int conf_sg(unsigned int threshold, unsigned int min_points, unsigned int gap)
{
  the_request.type = CONF_SG_MSG;
  the_request.size = 3;
  the_request.mesg[0] = threshold;
  the_request.mesg[1] = min_points;
  the_request.mesg[2] = gap;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_sg - get laser data line mode, independent of mask. It updates
 *          the laser vector.
 */
int get_sg(void)
{
  int temp_laser_mode;
  
  the_request.type = GET_SG_MSG;
  the_request.size = 0;
  
  temp_laser_mode = laser_mode;
  laser_mode = 33;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_laser_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
  
  laser_mode = temp_laser_mode;
}

/*
 * da - define the current steering angle of the robot to be th
 *      and the current turret angle of the robot to be tu.
 * 
 * parameters:
 *    int th, tu -- the steering and turret orientations to set the
 *                  robot to.
 */
int da(int th, int tu)
{
  the_request.type = DA_MSG;
  the_request.size = 2;
  the_request.mesg[0] = th;
  the_request.mesg[1] = tu;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * ws - waits for stop of motors of the robot. This function is intended  
 *      to be used in conjunction with pr() and pa() to detect the desired
 *      motion has finished
 *
 * parameters:
 *    unsigned char t_ws, s_ws, r_ws -- These three parameters specify 
 *                                      which axis or combination of axis 
 *                                      (translation, steering, and turret) 
 *                                      to wait. 
 *    unsigned char timeout -- specifies how long to wait before timing out 
 *                             (return without stopping the robot).
 */
int ws(unsigned char t_ws, unsigned char s_ws,
       unsigned char r_ws, unsigned char timeout)
{
  the_request.type = WS_MSG;
  the_request.size = 4;
  the_request.mesg[0] = t_ws;
  the_request.mesg[1] = s_ws;
  the_request.mesg[2] = r_ws;
  the_request.mesg[3] = timeout;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_state_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_rpx - get the position of all nearby robots
 */
int get_rpx(long *robot_pos)
{
  the_request.type = GET_RPX_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_rpx_reply(&the_reply, robot_pos));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*****************************
 *                           *
 * World Interface Functions *
 *                           *
 *****************************/

/*
 * add_obstacle - creates an obstacle and adds it to the obstacle list
 *                of the robot environment. 
 * 
 * parameters:
 *    long obs[2*MAX_VERTICES+1] -- 
 *                The first element of obs specifies the number of 
 *                vertices of the polygonal obstacle (must be no greater 
 *                than MAX_VERTICES). The subsequent elements of obs 
 *                specifies the x and y coordinates of the vertices, 
 *                in counter-clockwise direction.
 */
int add_obstacle(long obs[2*MAX_VERTICES+1])
{
  int i;
  
  the_request.type = ADDOBS_MSG;
  the_request.size = obs[0]*2+1;
  for (i=0; i<obs[0]*2+1; i++)
  {
    the_request.mesg[i] = obs[i];
  }
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * add_Obs - is the same as add_obstacle, for backward compatibility
 */
int add_Obs(long obs[2*MAX_VERTICES+1])
{
  return(add_obstacle(obs));
}

/*
 * delete_obstacle - deletes an obstacle specified by obs from the robot 
 *                   environment 
 * parameters:
 *    long obs[2*MAX_VERTICES+1] -- 
 *                The first element of obs specifies the number of 
 *                vertices of the polygonal obstacle (must be no greater 
 *                than MAX_VERTICES). The subsequent elements of obs 
 *                specifies the x and y coordinates of the vertices, 
 *                in counter-clockwise direction.
 */
int delete_obstacle(long obs[2*MAX_VERTICES+1])
{
  int i;
  
  the_request.type = DELETEOBS_MSG;
  the_request.size = obs[0]*2+1;
  for (i=0; i<obs[0]*2+1; i++)
  {
    the_request.mesg[i] = obs[i];
  }
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * delete_Obs - is the same as delete_obstacle, for backward compatibility
 */
int delete_Obs(long obs[2*MAX_VERTICES+1])
{
  return(delete_obstacle(obs));
}

/*
 * move_obstacle - moves the obstacle obs by dx along x direction and 
 *                 dy along y direction. obs is modified.
 *
 * parameters:
 *    long obs[2*MAX_VERTICES+1] -- 
 *                The first element of obs specifies the number of 
 *                vertices of the polygonal obstacle (must be no greater 
 *                than MAX_VERTICES). The subsequent elements of obs 
 *                specifies the x and y coordinates of the vertices, 
 *                in counter-clockwise direction.
 *    long dx, dy -- the x and y distances to translate the obstacle
 */
int move_obstacle(long obs[2*MAX_VERTICES+1], long dx, long dy)
{
  int i;
  
  the_request.type = MOVEOBS_MSG;
  the_request.size = obs[0]*2+3;
  for (i=0; i<obs[0]*2+1; i++)
  {
    the_request.mesg[i] = obs[i];
  }
  the_request.mesg[2*obs[0]+1] = dx;
  the_request.mesg[2*obs[0]+2] = dy;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_obstacle_reply(&the_reply, obs));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * move_Obs - is the same as move_obstacle, for backward compatibility
 */
int move_Obs(long obs[2*MAX_VERTICES+1], long dx, long dy)
{
  return(move_obstacle(obs, dx, dy));
}

/*
 * new_world - deletes all obstacles in the current robot world
 */
int new_world(void)
{
  the_request.type = NEWWORLD_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/****************************
 *                          *
 * Graphics refresh control *
 *                          *
 ****************************/

/*
 * refresh_all - causes all temporary drawing in graphics window, including
 *               traces, sensors, and client graphics to be erased
 */
int refresh_all(void)
{
  the_request.type = REFRESHALL_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_all_traces - causes all robot traces in graphics to be erased
 */
int refresh_all_traces(void)
{
  the_request.type = REFRESHALLTRACES_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_actual_trace - causes actual robot trace in graphics to be erased
 */
int refresh_actual_trace(void)
{
  the_request.type = REFRESHACTTRACE_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_encoder_trace - causes encoder robot trace in graphics to be erased
 */
int refresh_encoder_trace(void)
{
  the_request.type = REFRESHENCTRACE_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_all_sensors - causes all sensor drawings in graphics to be erased
 */
int refresh_all_sensors(void)
{
  the_request.type = REFRESHALLSENSORS_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_bumper_sensor - causes bumper drawings in graphics to be erased
 */
int refresh_bumper_sensor(void)
{
  the_request.type = REFRESHBPSENSOR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_infrared_sensor - causes infrared drawings in graphics to be erased
 */
int refresh_infrared_sensor(void)
{
  the_request.type = REFRESHIRSENSOR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_sonar_sensor - causes sonar drawings in graphics to be erased
 */
int refresh_sonar_sensor(void)
{
  the_request.type = REFRESHSNSENSOR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_laser_sensor - causes laser drawings in graphics to be erased
 */
int refresh_laser_sensor(void)
{
  the_request.type = REFRESHLSSENSOR_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * refresh_client_graphics - causes drawings performed by any clients into
 *                           graphics window to be erased
 */
int refresh_client_graphics(void)
{
  the_request.type = REFRESHGRAPHICS_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*******************************
 *                             *
 * Miscellaneous robot control *
 *                             *
 *******************************/

/*
 * init_mask - initialize the sensor mask, Smask.
 */
void init_mask(void)
{
  int i;
  
  Smask[ SMASK_POS_DATA ] = 0;
  for (i=1; i<44; i++)
    Smask[i] = 1;
}

/*
 * init_sensors - initialize the sensor mask, Smask, and send it to the
 *                robot. It has no effect on the sensors 
 */
int init_sensors(void)
{
  int i;
  
  Smask[ SMASK_POS_DATA ] = 0;
  for (i=1; i<44; i++)
    Smask[i] = 1;
  return ct();
}


/*
 * place_robot - places the robot at configuration (x, y, th, tu). 
 *               In simulation mode, it will place both the Encoder-robot
 *               and the Actual-robot at this configuration. In real robot
 *               mode, it will call dp(x, y) and da(th, tu).
 * 
 * parameters:
 *    int x, y -- x-y position of the desired robot configuration
 *    int th, tu -- the steering and turret orientation of the robot
 *                  desired configuration
 */
int place_robot(int x, int y, int th, int tu)
{
  the_request.type = RPLACE_MSG;
  the_request.size = 4;
  the_request.mesg[0] = x;
  the_request.mesg[1] = y;
  the_request.mesg[2] = th;
  the_request.mesg[3] = tu;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * special_request - sends a special request (stored in user_send_buffer) 
 *                   to the robot and waits for the robot's response (which
 *                   will be stored in user_receive_buffer). 
 * 
 * parameters:
 *    unsigned char *user_send_buffer -- stores data to be sent to the robot
 *                                       Should be a pointer to an array of
 *                                       MAX_MSG_LENGTH elements
 *    unsigned char *user_receive_buffer -- stores data received from the robot
 *                                          Should be a pointer to an array of 
 *                                       MAX_MSG_LENGTH elements
 */
int special_request(unsigned char *user_send_buffer,
		    unsigned char *user_receive_buffer)
{
  unsigned short num_data;
  int i;
  
  the_request.type = SPECIAL_MSG;
  num_data = user_send_buffer[0]+256*user_send_buffer[1];
  if (num_data>USER_BUFFER_LENGTH-5)
  {
    printf("Data + protocol bytes exceeding %d, truncating\n",USER_BUFFER_LENGTH);
    num_data  = USER_BUFFER_LENGTH-5; /* num_data already includes the 4 bytes of user packets protocol */
  }
  the_request.size = num_data;
  for (i=0; i<num_data; i++)
  {
    the_request.mesg[i] = user_send_buffer[i];
  }
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_special_reply(&the_reply, user_receive_buffer));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*******************************
 *                             *
 * Graphic Interface Functions *
 *                             *
 *******************************/

/*
 * draw_robot - this function allows the client to draw a robot at
 *              configuration x, y, th, tu (using the robot world 
 *              coordinates). 
 * 
 * parameters:
 *    long x, y -- the x-y position of the robot.
 *    int th, tu -- the steering and turret orientation of the robot
 *    int mode - the drawing mode. If mode = 1, the robot is drawn in 
 *              BlackPixel using GXxor (using GXxor you can erase the trace 
 *              of robotby drawing over it). If mode = 2, the robot is 
 *              drawn in BlackPixel using GXxor and in addition, a small arrow
 *              is drawn at the center of the robot using GXcopy (using this 
 *              mode you can leave a trace of small arrow). If mode = 3, 
 *              the robot is drawn in BlackPixel using GXcopy. When mode > 3,
 *              the robot is drawn in color using GXxor.
 */
int draw_robot(long x, long y, int th, int tu, int mode)
{
  the_request.type = DRAWROBOT_MSG;
  the_request.size = 5;
  the_request.mesg[0] = x;
  the_request.mesg[1] = y;
  the_request.mesg[2] = th;
  the_request.mesg[3] = tu;
  the_request.mesg[4] = (long)(mode);
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * draw_line - this function allows the client to draw a line from
 *             (x_1, y_1) to (x_2, y_2) (using the robot world coordinates). 
 *
 * parameters:
 *    long x_1, y_1, x_2, y_2 -- the two end-points of the line
 *    int mode -- the mode of drawing: when mode is 1, the drawing is 
 *                done in BlackPixel using GXcopy; when mode is 2, the drawing
 *                is done in BlackPixel using GXxor, when mode > 2, the drawing
 *                is done in color using GXxor.
 */
int draw_line(long x_1, long y_1, long x_2, long y_2, int mode)
{
  the_request.type = DRAWLINE_MSG;
  the_request.size = 5;
  the_request.mesg[0] = x_1;
  the_request.mesg[1] = y_1;
  the_request.mesg[2] = x_2;
  the_request.mesg[3] = y_2;
  the_request.mesg[4] = (long)(mode);
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * draw_arc - this function allows the client to draw arc which is part
 *            of an ellipse (using the robot world coordinates). 
 *
 * parameters:
 *    long x_0, y_0, w, h -- (x_0, y_0) specifies the upper left corner of the 
 *                          rectangle bounding the ellipse while w and h
 *                          specifies the width and height of the bounding 
 *                          rectangle, respectively.
 *    int th1, th2 -- th1 and th2 specifies the angular range of the arc.
 *    int mode -- the mode of drawing: when mode is 1, the drawing is 
 *                done in BlackPixel using GXcopy; when mode is 2, the drawing
 *                is done in BlackPixel using GXxor, when mode > 2, the drawing
 *                is done in color using GXxor.
 */
int draw_arc(long x_0, long y_0, long w, long h, int th1, int th2, int mode)
{
  the_request.type = DRAWARC_MSG;
  the_request.size = 7;
  the_request.mesg[0] = x_0;
  the_request.mesg[1] = y_0;
  the_request.mesg[2] = w;
  the_request.mesg[3] = h;
  the_request.mesg[4] = th1;
  the_request.mesg[5] = th2;
  the_request.mesg[6] = (long)mode;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*************************************
 *                                   *
 * Miscellaneous Interface Functions *
 *                                   *
 *************************************/

/*
 * server_is_running - this function queries the server to see
 *                     if it is up and running.  If so, this function
 *                     returns a TRUE, otherwise it returns FALSE.
 *                     This function is replaced by connect_robot, but 
 *                     is defined here for backward compatibility
 */
int server_is_running()
{
  return(connect_robot(1));
}

/*
 * quit_server - this function allows the client to quit the server
 *               assuming this feature is enabled in the setup file
 *               of the server
 */
int quit_server(void)
{
  the_request.type = QUIT_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * real_robot - this function allows the client to switch to
 *              real robot mode in the server
 */
int real_robot(void)
{
  the_request.type = REALROBOT_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * simulated_robot - this function allows the client to switch to
 *                   simulated robot mode in the server
 */
int simulated_robot(void)
{
  the_request.type = SIMULATEDROBOT_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_simple_reply(&the_reply));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * predict_sensors - this function predicts the sensor reading of
 *                   the robot assuming it is at position (x, y)
 *                   and orientation th and tu using the map of the
 *                   simulated robot environment. The predicted sensor
 *                   data values are stored in "state" and "laser".
 * 
 * parameters:
 *    int x, y, th, tu -- the configuration of the robot
 *    long *state -- where to put the predicted state data
 *    int *laser -- where to put the predicted laser data
 */
int predict_sensors(int x, int y, int th, int tu, long *state, int *laser)
{
  the_request.type = PREDICTSENSOR_MSG;
  the_request.size = 4;
  the_request.mesg[0] = x;
  the_request.mesg[1] = y;
  the_request.mesg[2] = th;
  the_request.mesg[3] = tu;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_predict_reply(&the_reply, state, laser));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/* 
 * motion_check - this function computes the intersection of a path
 *                specified by the parameters: type, a1, ..., a7 with
 *                the obstacles in the robot's environment. If there is
 *                collision, the function returns 1 and the x-y configuration
 *                of the robot is stored in collide[0] and collide[1] while
 *                collide[2] stores the inward normal of the obstacle edge
 *                that the robot collides with (this information can be
 *                used to calculate which bumper is hit.). If there is no
 *                collision, the function returns 0.
 *
 * parameters:
 *    long type - 0 if the path is a line segment
 *                1 if the path is an arc of circle
 *    double a1 a2 - x-y coordinates of the first point of the path (the path
 *                   is directional).
 *    depending on the value of type, a3 - a7 have different meanings.
 *    if (type == 0), line segment mode
 *      double a3 a4 are the x-y coordinates of the second point of the path
 *      a5, a6, a7 have no meaning
 *    if (type == 1), arc of circle mode
 *      double a3 is the angle (in radiance) of the vector connecting the 
 *                center of the circle to the first end-point of the arc
 *      double a4 is the angle of the vector connecting the center
 *                of the circle to the second end-point of the arc
 *      double a5 is the radius of the circle
 *      double a6 a7 are the x-y coordinate of the center of the circle
 */
int motion_check(long type, double a1, double a2, double a3, double a4,
		 double a5, double a6, double a7, double collide[3])
{
  the_request.type = MCHECK_MSG;
  the_request.size = 8;
  the_request.mesg[0] = type;
  the_request.mesg[1] = (long)(a1*100.0);
  the_request.mesg[2] = (long)(a2*100.0);
  if (type == 0) /* line segment */
  {
    the_request.mesg[3] = (long)(a3*100.0);
    the_request.mesg[4] = (long)(a4*100.0);
  }
  else /* arc */
  {
    the_request.mesg[3] = (long)(a3*1000000.0);
    the_request.mesg[4] = (long)(a4*1000000.0);
    the_request.mesg[5] = (long)(a5*100.0);
    the_request.mesg[6] = (long)(a6*100.0);
    the_request.mesg[7] = (long)(a7*100.0);
  }
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_mcheck_reply(&the_reply, collide));
  }
  else
  {
    State[ STATE_ERROR ] = IPC_ERROR;  /* indicate IPC_ERROR */
    return(FALSE);
  }
}

/*
 * get_robot_conf - interactively getting the robot's conf, by clicking
 *                  the mouse in the server's Robot window
 * 
 * parameters:
 *    long *conf -- should be an array of 4 long integers. The configuration
 *                  of the robot is returned in this array.
 */
int get_robot_conf(long *conf)
{
  the_request.type = GET_CONF_MSG;
  the_request.size = 0;
  
  if (ipc_comm(&the_request, &the_reply))
  {
    return(process_conf_reply(&the_reply, conf));
  }
  else
  {
    return(FALSE);
  }
}

/*******************************************
 *                                         *
 * The following are helper functions for  *
 * developing user defined host <-> robot  *
 * communication                           *
 *                                         *
 *******************************************/

/*
 *  init_receive_buffer - sets the index to 4 which is the point
 *  at which data should begin to be extracted
 * 
 *  parameters:
 *     unsigned short *index -- is the buffer index
 */
int init_receive_buffer(unsigned short *index)
{
  *index = 4;
  return(*index);
}

/*
 *  extract_receive_buffer_header - extracts the header information:
 *  length, serial_number, and packettype from the beginning of the
 *  receive buffer.
 *
 *  parameters:
 *     unsigned short *length -- is the returns the number of chars in the buffer
 *
 *     unsigned char *serial_number -- returns the serial number to be
 *                                     assigned to the packet
 *     unsigned char *packet_type -- returns the type number to be
 *                                   assigned to the packet
 *     unsigned char *buffer -- is the receive buffer
 */
int extract_receive_buffer_header(unsigned short *length, 
				  unsigned char *serial_number, 
				  unsigned char *packet_type, 
				  unsigned char *buffer)
{
  unsigned short data;
  
  data = buffer[0] << 0;
  data |= buffer[1] << 8;
  *length = data;
  *serial_number = buffer[2];
  *packet_type = buffer[3];
  return(*length);
}

/*
 *  init_send_buffer - sets the index to 4 which is the point
 *  at which data should be inserted
 *
 *  parameters:
 *     int *index -- is the buffer index
 */
int init_send_buffer(unsigned short *index)
{
  *index = 4;
  return(*index);
}

/*
 *  stuff_send_buffer_header - loads the header information,
 *  length,serial_number, and packettype into the beginning of the
 *  buffer.  It should be called after the data has been stuffed,
 *  i.e. index represents the length of the packet.
 *
 *  parameters:
 *     unsigned short index -- is the buffer index which holds the number of chars
 *                  in the buffer
 *     unsigned char serial_number -- holds the serial number to be
 *                                    assigned to the packet
 *     unsigned char packet_type -- holds the type number to be
 *	                           assigned to the packet
 *
 *     unsigned char *buffer -- is the send buffer
 */
int stuff_send_buffer_header(unsigned short index, unsigned char serial_number, 
			     unsigned char packet_type, unsigned char *buffer)
{
  buffer[0] = (index >> 0) & 0xff;
  buffer[1] = (index >> 8) & 0xff;
  buffer[2] = serial_number;
  buffer[3] = packet_type;
  return(index);
}

/*
 *  stuffchar -  stuffs a 1 byte char into the send buffer
 *
 *  parameters:
 *     signed char data -- is the char to be stuffed
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *                              to reflect the bytes stuffed into the buffer
 */
int stuffchar(signed char data, unsigned char *buffer, unsigned short *index)
{
  if (data < 0)
  {
    data *= -1;
    data |= 0x80;
  }
  
  buffer[*index]   = data;
  *index += 1;
  return(*index);
}

/*
 *  stuff2byteint - stuffs a short int(2 bytes) into the send buffer
 *
 *  parameters:
 *     signed int data -- is the value which will be split apart and stuffed
 *	                  bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *                              to reflect the bytes stuffed into the buffer
 */
int stuff2byteint(signed short data,
		  unsigned char *buffer, unsigned short *index)
{
  if (data < 0)
  {
    data *= -1;
    data |= 0x8000;
  }
  
  buffer[*index]   = (data >> 0) & 0xff;
  *index += 1;
  buffer[*index]   = (data >> 8) & 0xff;
  *index += 1;
  
  return(*index);
}

/*
 *  stuff4byteint - stuffs a long int(4 bytes) into the send buffer
 *
 *  parameters:
 *     signed long data -- is the value which will be split apart and stuffed
 *	                   bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff4byteint(signed long data,
		  unsigned char *buffer, unsigned short *index)
{
  if (data < 0)
  {
    data *= -1;
    data |= 0x80000000L;
  }
  
  buffer[*index] = (data >> 0) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 8) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 16) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 24) & 0xff;
  *index += 1;
  
  return(*index);
}

/*
 *  stuffuchar -  stuffs an unsigned char into the send buffer
 *
 *  parameters:
 *     unsigned char data -- is the char to be stuffed
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *                              to reflect the bytes stuffed into the buffer
 */
int stuffuchar(unsigned char data, unsigned char *buffer, unsigned short *index)
{
  buffer[*index]   = data;
  
  *index += 1;
  
  return(*index);
}

/*
 *  stuff2byteuint - stuffs an unsigned short int(2 bytes) into the send buffer
 *
 *  parameters:
 *     unsigned short data -- is the value which will be split apart and 
 *                            stuffed bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff2byteuint(unsigned short data,
		   unsigned char *buffer, unsigned short *index)
{
  buffer[*index]   = (data >> 0) & 0xff;
  *index += 1;
  buffer[*index]   = (data >> 8) & 0xff;
  *index += 1;
  
  return(*index);
}

/*
 *  stuff4byteuint - stuffs an unsigned long int(4 bytes) into the send buffer
 *
 *  parameters:
 *     unsigned long data -- is the value which will be split apart and stuffed
 *	                     bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff4byteuint(unsigned long data,
		   unsigned char *buffer, unsigned short *index)
{
  buffer[*index] = (data >> 0) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 8) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 16) & 0xff;
  *index += 1;
  buffer[*index] = (data >> 24) & 0xff;
  *index += 1;
  
  return(*index);
}

/*
 *  stuffdouble - stuffs a double(8 bytes) into the send buffer
 *
 *  parameters:
 *     double data -- is the value which will be split apart and stuffed
 *	              bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuffdouble(double data, unsigned char *buffer, unsigned short *index)
{
  unsigned long long *tempp, temp;
  
  /* Assume that double is 64 bits and "long long" is 64 bits. */
  tempp = (unsigned long long *)&data;
  temp = *tempp;
  
  buffer[*index] = (temp >> 0) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 8) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 16) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 24) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 32) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 40) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 48) & 0xff;
  *index += 1;
  buffer[*index] = (temp >> 56) & 0xff;
  *index += 1;
  
  return(*index);
}

/*
 *  extractchar -  extracts a char from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed char extractchar(unsigned char *buffer, unsigned short *index)
{
  char data;
  
  data = buffer[*index];
  *index += 1;
  
  if (data & 0x80)
  {
    data &= 0x7f;
    data *= -1;
  }
  return(data);
}

/*
 *  extract2byteint -  extracts a short int(2 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed short extract2byteint(unsigned char *buffer, unsigned short *index)
{
  signed short data;
  
  data = (signed short)buffer[*index] << 0;
  *index += 1;
  data |= (signed short)buffer[*index] << 8;
  *index += 1;
  
  if (data & 0x8000)
  {
    data &= 0x7fff;
    data *= -1;
  }
  
  return(data);
}

/*
 *  extract4byteint -  extracts a long int(4 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed long extract4byteint(unsigned char *buffer, unsigned short *index)
{
  signed long data;
  
  data = (signed long)buffer[*index] << 0;
  *index += 1;
  data |= (signed long)buffer[*index] << 8;
  *index += 1;
  data |= (signed long)buffer[*index] << 16;
  *index += 1;
  data |= (signed long)buffer[*index] << 24;
  *index += 1;
  
  if (data & 0x80000000)
  {
    data &= 0x7fffffff;
    data *= -1;
  }
  
  return(data);
}

/*
 *  extractuchar -  extracts an unsigned char from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned char extractuchar(unsigned char *buffer, unsigned short *index)
{
  unsigned char data;
  
  data = buffer[*index];
  
  *index += 1;
  
  return(data);
}

/*
 *  extract2byteuint -  extracts an unsigned short int(2 bytes) from the 
 *                      receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned short extract2byteuint(unsigned char *buffer, unsigned short *index)
{
  unsigned short data;
  
  data = (unsigned short)buffer[*index] << 0;
  *index += 1;
  data |= (unsigned short)buffer[*index] << 8;
  *index += 1;
  
  return(data);
}

/*
 *  extract4byteuint -  extracts an unsigned long int(4 bytes) from the 
 *                      receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned long extract4byteuint(unsigned char *buffer, unsigned short *index)
{
  unsigned long data;
  
  data = (unsigned long)buffer[*index] << 0;
  *index += 1;
  data |= (unsigned long)buffer[*index] << 8;
  *index += 1;
  data |= (unsigned long)buffer[*index] << 16;
  *index += 1;
  data |= (unsigned long)buffer[*index] << 24;
  *index += 1;
  
  return(data);
}

/*
 *  extractdouble -  extracts a double(8 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
double extractdouble(unsigned char *buffer, unsigned short *index)
{
  double data;
  unsigned long long *tempp, temp;
  
  /* Assume that double is 64 bits and long long is 64 bits. */
  
  temp = (unsigned long long)buffer[*index] << 0;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 8;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 16;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 24;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 32;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 40;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 48;
  *index += 1;
  temp |= (unsigned long long)buffer[*index] << 56;
  *index += 1;
  
  tempp = (unsigned long long *)&data;
  *tempp = temp;
  
  return(data);
}

/************************************************
 *                                              *
 * Global variable access functions for Allegro * 
 * Common Lisp interface                        *
 *                                              *
 ************************************************/

int get_state(long state[NUM_STATE])
{
  int i;
  
  for (i=0;i<NUM_STATE;i++) 
    state[i] = State[i];
  return(TRUE);
}

int get_laser(int laser[2*NUM_LASER+1])
{
  int i;

  for (i=0;i<=Laser[0];i++) 
    laser[i] = Laser[i];
  return(TRUE);
}

int get_mask(int mask[NUM_MASK])
{
  int i;

  for (i=0;i<NUM_MASK;i++) 
    mask[i] = usedSmask[i];
  return(TRUE);
}

int set_mask(int mask[NUM_MASK])
{
  int i;

  for (i=0;i<NUM_MASK;i++) 
    Smask[i] = mask[i];
  return(TRUE);
}

int set_server_machine_name(char *sname)
{
  strcpy(SERVER_MACHINE_NAME, sname);
  strcpy(Host_name, "");
  return(TRUE);
}

int set_serv_tcp_port(int port)
{
  SERV_TCP_PORT = port;
  return(TRUE);
}



/*
 *
 * 
 *         PosData Attachment
 *         ===================
 *    
 *    Here all procudures are defined that deal with the 
 * 
 *    attachment of PosData to sensory readings.
 * 
 *
 */


/***************
 * FUNCTION:     posDataRequest
 * PURPOSE:      request position information for sensors
 * ARGUMENTS:    int posRequest : 
 *               The argument of this function specifies the sensors 
 *               for which the position information (PosData) should 
 *               be attached to the sensory reading.
 *               Its value is obtained by ORing the desired defines. 
 * EXAMPLE:      To attach PosData to sonars and laser:
 *               posDataRequest ( POS_SONAR | POS_LASER );
 * ALGORITHM:    currently sets the global variable Smask[0] and
 *               then calls ct() to transmit the change to the server
 * RETURN:       TRUE if the argument was correct, else FALSE
 * SIDE EFFECT:  Smask[ SMASK_POS_DATA ]
 * CALLS:        
 * CALLED BY:    
 ***************/
int posDataRequest ( int posRequest )
{
  /* check if the argument is okay */
  if ( posRequest & 
      !( POS_INFRARED | POS_SONAR | POS_BUMPER | POS_LASER | POS_COMPASS ) )
    return ( FALSE );

  /* The value in Smask[ SMASK_POS_DATA ] is passed through entire system */
  Smask[0] = posRequest;
  ct();

  return ( TRUE );
}

/***************
 * FUNCTION:     posDataCheck
 * PURPOSE:      return the sensors for which the PosData attachment
 *               is currently requested. 
 * ARGUMENTS:    None
 * ALGORITHM:    returns the mask that is not globally accessibe and 
 *               that is set by ct() to be the value of Smask[0]
 * RETURN:       int, see posDataRequest
 *               the macros POS_*_P can be used to examine the value
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posDataCheck ( void )
{
  return ( usedSmask[ SMASK_POS_DATA ] );
}

/***************
 * FUNCTION:     posInfraredRingGet
 * PURPOSE:      copy the PosData for all infrareds to accessible memory
 * ARGUMENTS:    PosData posData [INFRAREDS] :
 *               an array of PosData structures that is filled with 
 *               PosData. The position information for each infrared
 *               containts the configuration of the robot at the time 
 *               of the sensory reading and a timestamp for the 
 *               configuration and the senosry reading .
 * ALGORITHM:    copies blocks of memory
 * RETURN:       int, return always TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posInfraredRingGet ( PosData posData[INFRAREDS] )
{
  /* copy the whole thing in one block */
  memcpy ( posData, posDataAll.infrared, INFRAREDS * sizeof ( PosData ) );

  return ( TRUE );
}


/***************
 * FUNCTION:     posInfraredGet
 * PURPOSE:      copy the PosData for a specific infrared to accessible 
 *               memory
 * ARGUMENTS:    int infraredNumber : the number of the infrared
 *               PosData *posData : the memory location that the information
 *                                  will be copied to 
 * ALGORITHM:    copies block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posInfraredGet     ( PosData *posData , int infraredNumber)
{
  /* copy the whole thing in one block */
  memcpy ( posData, &posDataAll.infrared[infraredNumber], sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posSonarRingGet
 * PURPOSE:      copy the PosData for all sonars to accessible memory
 * ARGUMENTS:    PosData posData [SONARS] :
 *               an array of PosData structures that is filled with 
 *               PosData. The position information for each sonar
 *               containts the configuration of the robot at the time 
 *               of the sensory reading and a timestamp for the 
 *               configuration and the senosry reading .
 * ALGORITHM:    copies blocks of memory
 * RETURN:       int, return always TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posSonarRingGet    ( PosData posData[SONARS] )
{
  /* copy the whole thing in one block */
  memcpy ( posData, posDataAll.sonar, SONARS * sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posSonarGet
 * PURPOSE:      copy the PosData for a specific sonar to accessible memory
 * ARGUMENTS:    int infraredNumber : the number of the sonar
 *               PosData *posData : the memory location that the information
 *                                  will be copied to 
 * ALGORITHM:    copies block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posSonarGet        ( PosData *posData , int sonarNumber)
{
  /* copy the whole thing in one block */
  memcpy ( posData, &posDataAll.sonar[sonarNumber], sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posBumperGet
 * PURPOSE:      copy PosData for the bumper to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The bumper differs from other sensors in that the 
 *               posData is only updated after one of the bumper sensors 
 *               change its value from zero to one. This means that the 
 *               posData for the bumper always contains the position and 
 *               timeStamps of the latest hit, or undefined information 
 *               if the bumper was not hit yet.
 ***************/
int posBumperGet       ( PosData *posData )
{
  /* copy the whole thing in one block */
  memcpy ( posData, &posDataAll.bumper, sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posLaserGet
 * PURPOSE:      copy PosData for the laser to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The laser is updated at a frequency of 30Hz.
 ***************/
int posLaserGet        ( PosData *posData )
{
  /* copy the whole thing in one block */
  memcpy ( posData, &posDataAll.laser, sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posCompassGet
 * PURPOSE:      copy PosData for the compass to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The compass is updated ad a frequency of 10Hz.
 ***************/
int posCompassGet      ( PosData *posData )
{
  /* copy the whole thing in one block */
  memcpy ( posData, &posDataAll.compass, sizeof ( PosData ) );

  return ( TRUE );
}

/***************
 * FUNCTION:     posTimeGet
 * PURPOSE:      get the PosData time (Intellisys 100) in milliseconds
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       int 
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         Use POS_TICKS_TO_MS and POS_MS_TO_TICKS to convert
 *               between ticks and milliseconds. Overflow after 49 days.
 ***************/
int posTimeGet         ( void )
{
  return ( (int) posDataTime );
}

/***************
 * FUNCTION:     voltCpuGet
 * PURPOSE:      get the voltage of the power supply for the CPU
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       float (the voltage in volt)
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
float voltCpuGet         ( void )
{
  return ( voltConvert ( voltageCPU , RANGE_CPU_VOLTAGE ) );
}

/***************
 * FUNCTION:     voltMotorGet
 * PURPOSE:      get the voltage of the power supply for the motors
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       float (the voltage in volt)
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
float voltMotorGet         ( void )
{
  return ( voltConvert ( voltageMotor , RANGE_MOTOR_VOLTAGE ) );
}

/***************
 * FUNCTION:     voltConvert
 * PURPOSE:      convert from the DA reading to the right voltage range
 * ARGUMENTS:    unsigned char reading: the reading of the da
 * ALGORITHM:    ---
 * RETURN:       float (the voltage in volt)
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
static float voltConvert ( unsigned char reading , float range )
{
  /* 
   * original reading is [0...255] and represents [2...5]volt.
   * the 5 volt value is converted to 12V by multiplying (range/5)
   */
  return ( ( 2.0 +  ( ( (float) (reading*3) ) / 255.0 ) ) * ( range / 5.0 ) );
}

/***************
 * FUNCTION:     posDataProcess
 * PURPOSE:      copy the PosData from the socket buffer to static memory
 * ARGUMENTS:    long *buffer : pointer to the buffer
 *               int current : where are we currently within the buffer
 *               PosData *posData : where should the data be written
 * ALGORITHM:    copies longs
 * RETURN:       static int, returns the next position in the buffer
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
static int posDataProcess (long *buffer, int current, PosData *posData)
{
  int count;

  count = current; 

  /* copy the configuration */
  posData->config.configX       = buffer[count++];
  posData->config.configY       = buffer[count++];
  posData->config.configSteer   = buffer[count++];
  posData->config.configTurret  = buffer[count++];
  posData->config.velTrans      = buffer[count++];
  posData->config.velSteer      = buffer[count++];
  posData->config.velTurret     = buffer[count++];
  posData->config.timeStamp     = (TimeData) buffer[count++];
  
  /* copy the timeStamp of the sensory reading */
  posData->timeStamp            = (TimeData) buffer[count++];

  return ( count );
}

/***************
 * FUNCTION:     timeDataProcess
 * PURPOSE:      copy the Intellisys 100 time from the socket buffer to 
 *               static memory
 * ARGUMENTS:    long *buffer : pointer to the buffer
 *               int current : where are we currently within the buffer
 *               posTimeData *time : where the data is written
 * ALGORITHM:    ---
 * RETURN:       static int, returns the next position in the buffer
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
static int timeDataProcess ( long *buffer, int current, TimeData *theTime )
{
  *theTime = (unsigned long) buffer[current];

  return ( current + 1 );
}

/***************
 * FUNCTION:     voltDataProcess
 * PURPOSE:      copy the voltages from the socket buffer to static memory
 * ARGUMENTS:    long *buffer : pointer to the buffer
 *               int current : where are we currently within the buffer
 *               unsigned char *voltCPU : the memory for the CPU voltage
 *               unsigned char *voltMotor : the memory for the motor voltage
 * ALGORITHM:    ---
 * RETURN:       static int, returns the next position in the buffer
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
static int voltDataProcess (long *buffer, int current, 
			    unsigned char *voltCPU, unsigned char *voltMotor)
{
  int counter = current;

  *voltCPU   = (unsigned char) buffer[counter++];
  *voltMotor = (unsigned char) buffer[counter++];

  return (counter);
}


/****************************************************************/


long arm_zr(short override)
{
  long result;

  short b_index, b_length;
  unsigned char serial_number;
  unsigned char packet_type;
  unsigned char user_send_buffer[256];
  unsigned char user_receive_buffer[256];

  init_send_buffer(&b_index);

  stuff2byteuint(override, user_send_buffer, &b_index);
  stuff_send_buffer_header(b_index, 0, ARM_ZR, user_send_buffer);
  
  special_request(user_send_buffer, user_receive_buffer);

  init_receive_buffer(&b_index);
  extract_receive_buffer_header(&b_length, &serial_number, &packet_type,
				user_receive_buffer);

  result=extract4byteuint(user_receive_buffer, &b_index);
  return result;
}

long arm_ws(short l, short g, long timeout, long *time_remain)
{
  long result;

  short b_index, b_length;
  unsigned char serial_number;
  unsigned char packet_type;
  unsigned char user_send_buffer[256];
  unsigned char user_receive_buffer[256];

  init_send_buffer(&b_index);

  stuff2byteuint(l, user_send_buffer, &b_index);
  stuff2byteuint(g, user_send_buffer, &b_index);
  stuff4byteuint(timeout, user_send_buffer, &b_index);
  stuff_send_buffer_header(b_index, 0, ARM_WS, user_send_buffer);
  
  special_request(user_send_buffer, user_receive_buffer);

  init_receive_buffer(&b_index);
  extract_receive_buffer_header(&b_length, &serial_number, &packet_type,
				user_receive_buffer);

  result=extract4byteuint(user_receive_buffer, &b_index);
  if (time_remain)
    *time_remain=extract4byteuint(user_receive_buffer, &b_index);

  return result;
}

long arm_mv(long l_mode, long l_v, long g_mode, long g_v)
{
  long result;

  short b_index, b_length;
  unsigned char serial_number;
  unsigned char packet_type;
  unsigned char user_send_buffer[256];
  unsigned char user_receive_buffer[256];

  init_send_buffer(&b_index);

  stuff4byteuint(l_mode, user_send_buffer, &b_index);
  stuff4byteuint(l_v, user_send_buffer, &b_index);
  stuff4byteuint(g_mode, user_send_buffer, &b_index);
  stuff4byteuint(g_v, user_send_buffer, &b_index);
  stuff_send_buffer_header(b_index, 0, ARM_MV, user_send_buffer);
  
  special_request(user_send_buffer, user_receive_buffer);

  init_receive_buffer(&b_index);
  extract_receive_buffer_header(&b_length, &serial_number, &packet_type,
				user_receive_buffer);

  result=extract4byteuint(user_receive_buffer, &b_index);

  return result;
}
