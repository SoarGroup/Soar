/*
 *  Player - One Hell of a Robot Server
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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
///////////////////////////////////////////////////////////////////////////
//
// Desc: USARSim (simulator) Bot functions
// Author: Jijun Wang
// Date: 11 May 2004
// Modified: 
// 3 Mars 2005     Erik Winter 	added Ringhorne IR
// 11 Mars 2005    Erik Winter 	added RinghornePyro
// 14 Mars 2005    Erik Winter 	added call to RHPyro config
// 14 Mars 2005    Erik Winter   Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter   Continued porting, it compiles but gives segmentation faults 
// 16 Mars 2005    Erik Winter   No more segmentation faults, can get sensor data but not geometry information 
// 21 April 2005   Andreas Pfeil Made Driver loadable as a plugin and only 
//                               compatible to player 1.6
// 15 June 2005    Stefan Markov            Added support for providing robot name when 
//                                          spawning a bot.
// 12 July 2005    Stefan Markov            Added support for providing robot rotation
//                                          when spawning a bot.
// 12 July 2006    Stefan Stiene            Upgrade the whole thing to player 2.0.2
//                 and Nils Rosemann
// 22 Nov 2006     Stefan Stiene
//                 and Florian Halbritter   Added support for UsarSim RangeScanner3d
///////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>  /* for strncpy(3),memcpy(3) */
#include <unistd.h> /* close(2),fcntl(2),getpid(2),usleep(3),execlp(3),fork(2)*/
#include <fcntl.h>  /* for fcntl(2) */
#include <sys/socket.h>  /* for accept(2) */
#include <sys/types.h>  /* for socket(2) */
#include <netdb.h> /* for gethostbyaddr(3) */
#include <netinet/in.h> /* for struct sockaddr_in, SOCK_STREAM */
#include <libplayertcp/socket_util.h> /* for create_and_bind_socket() */
#include <pthread.h>  /* for pthread stuff */
#include <sys/select.h> /* for select (2) call */

// Include a header file containing all definitions of structures etc. not yet
// included in the player headerfile of the standard release.
#include "temp_laser3d_structures.h"

#include <time.h>

#include "us_bot.h"
#include "us_laser.h"
#include "us_position.h"
// Include new laser 3d header file.
#include "us_laser3d.h"
#include "debug.h"
 static int bot_index = 0;
///////////////////////////////////////////////////////////////////////////
// Instantiate an instance of this driver
Driver* UsBot_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsBot - Init\n");
  return ((Driver*) (new UsBot(cf, section)));
}


///////////////////////////////////////////////////////////////////////////
// Register driver
void UsBot_Register(DriverTable* table)
{
  table->AddDriver("us_bot", UsBot_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////////////////////
UsBot::UsBot(ConfigFile* cf, int section) : 
  Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_SIMULATION_CODE)
{
  fprintf(stderr,"UsBot -- Constructor\n");
  sock = -1;
  devices = 0;
  //Why are we doing strncpy() here?
  //perhaps you can do setup things here
  strncpy(this->host, cf->ReadString(section, "host", DEFAULT_GAMEBOTS_IP), sizeof(this->host));
  this->port = cf->ReadInt(section, "port", DEFAULT_GAMEBOTS_PORT);
  initPose.px = cf->ReadTupleLength(section,"pos",0,0);
  initPose.py = cf->ReadTupleLength(section,"pos",1,0);
  initPose.pz = cf->ReadTupleLength(section,"pos",2,0);
  initPose.proll = cf->ReadTupleAngle(section,"rot",0,0);
  initPose.ppitch = cf->ReadTupleAngle(section,"rot",1,0);
  initPose.pyaw = cf->ReadTupleAngle(section,"rot",2,0);
  strncpy(this->botClass, cf->ReadString(section, "bot", DEFAULT_GAMEBOTS_CLASS), sizeof(this->botClass));
  strncpy(this->botName, cf->ReadString(section, "botname", DEFAULT_GAMEBOTS_CLASS), sizeof(this->botName));
  //Move these initializations in front of the function body ?

  drvPosition = NULL;
  bPositionSubscribed = false;
  bEncoderSubscribed  = false;

  position3d = NULL;
  bNewPosition3d = false;
  bLockPosition3d = false;

  /*sonar = NULL;
  sonar_geom = NULL;
  bNewSonar = false;
  bLockSonar = false;
  bGeoSonar = false;
  */
  ir = NULL;
  ir_geom = NULL;
  bNewIr = false;
  bLockIr = false;
  
  pyro = NULL;
  pyro_geom = NULL;
  pyro_conf = NULL;
  bNewPyro = false;
  bLockPyro = false;
  bGeoPyro = false;
  bConfPyro = false;
  /*
  ptz = NULL;
  bNewPtz = false;
  bLockPtz = false;
  */
  
  fiducial = NULL;
  bNewFiducial = false;
  bLockFiducial = false;
  
  victim_fiducial = NULL;
  bNewVictimFiducial = false;
  bLockVictimFiducial = false;

  inu = NULL;
  bNewINU = false;
  bLockINU = false;

  bConfRobot = false;
  steeringType = NULL;
  robotDimensions = NULL;
  maxWheelSeparation = -1;
  wheelRadius = -1;
  COG[0] = 0.0;
  COG[1] = 0.0;
  COG[2] = 0.0;
  wheelBase = 0.0;

  Setup();
  bot_index++;
  return;
}
/**
 * Destructor
 */
UsBot::~UsBot()
{
  // TODO: delete all maps
  fprintf(stderr,"UsBot -- Destructor\n");
  Shutdown();
  return;
}
/**
 * Setup the UsBot -- create communication with Gamebots
 */
int UsBot::Setup()
{
  static struct sockaddr_in server;
  struct hostent* entp;
  int j;
  printf("Player Gamebots server connection initializing (%s:%d)...", host, port);
  fflush(stdout);
  /******************************************************************/
  // Use getaddrinfo(3) to be halfway IPv6 capable?
  // fill in addr structure
  server.sin_family = PF_INET;
  // this is okay to do, because gethostbyname(3) does no lookup if the
  // 'host' * arg is already an IP addr
  if((entp = gethostbyname(host)) == NULL) {
    fprintf(stderr, "UsBot::Setup(): \"%s\" is unknown host; "
		  "can't connect to gamebots\n", host);
    return(1);
  }
  memcpy(&server.sin_addr, entp->h_addr_list[0], entp->h_length);
  server.sin_port = htons(port);
  // ok, we'll make this a bit smarter.  first, we wait a baseline amount
  // of time, then try to connect periodically for some predefined number
  // of times
  usleep(USBOT_STARTUP_USEC);
  
  for(j = 0;j < USBOT_STARTUP_CONN_LIMIT; j++) {
    // make a new socket, because connect() screws with the old one somehow
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("UsBot::Setup(): socket(2) failed");
      return(1);
    }
    // hook it up
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) break;
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
  if(j == USBOT_STARTUP_CONN_LIMIT) {
    perror("UsBot::Setup(): connect(2) failed");
    return(1);
  }
  puts("Done.");
  // Until here.
  /***********************************************************************/
  /*****************************/
  // Left out - changed "readline" procedure to use select.
  // make it nonblocking
  //   if(fcntl(sock,F_SETFL,O_NONBLOCK) < 0) {
  //     perror("UsBot::Setup(): fcntl(2) failed");
  //     return(1);
  //   }
  //Until here.
  SpawnBot();  // Spawn a bot into UT world
  StartThread();                        // now spawn reading thread
  return(0);
}


/**
 * Shutdown the UsBot
 */
int UsBot::Shutdown()
{
  fprintf(stderr,"UsBot -- Shutdown\n");
  /* if Setup() was never called, don't do anything */
  if(sock == -1)
    return(0);
  
  StopThread();

  sock = -1;
  puts("UsBot device has been shutdown");
  return(0);
}
/**
 * The main function that reads data from Gamebots
 */
void UsBot::Main()
{
  // use this to hold temp data
  char buf[USBOT_MAX_MSG_LEN+1];
  char* tmpstr;
   /* make sure we kill Festival on exiting */
  pthread_cleanup_push(QuitUsBot,this);  
  // loop and read
  while(1) {
    // perhaps we have to do something like this so us_bot doesn't take
    // all computation time in a very fast while loop if no device wants data
    pthread_testcancel(); // test if we are supposed to cancel
    // TODO: at the beginning there is no answer from usarsim
    while(!(queue.empty()))
    { //if there is a usarsim command send it
      tmpstr = queue.back();  //Took Command out of queue
      PLAYER_MSG1(9,"bot cmd: %s", tmpstr);
      if(write(sock,tmpstr,strlen(tmpstr)) == -1)// send command to Gamebots
      {
        perror("USBot Thread: write() failed sending string; exiting.");
        break;
      }
      delete[] tmpstr;
      queue.pop_back();
    }    
    //Change this to use select(2) on a blocking port?
    //- This would allow to read lines continuously after each other.
    // read a line   
    unsigned int numread = 0;
    buf[numread] = 0;
    // space: number of bytes left in buffer - one less than the size to be 
    // able to null-terminate string
    int space = sizeof(buf) - numread;

    bool timeout = false;
    while ( (!strchr(buf, '\n')) && (space>0) ){
      // Set read timeout to 100 ms... - so that we stop reading a line, 
      // if we did not get anything for 100ms. This can lead to a big delay
      // (100ms * sizeof(buf)), if chars arrive one-by-one, but it should be 
      // okay.
	 // if you want to get the time uncomment the following line
	 //start = clock(); 
      struct timeval tv = { 0, 100*1000 };
      fd_set fd;
      FD_ZERO(&fd);
      FD_SET(sock, &fd);

      int r = select(sock+1, &fd, NULL, NULL, &tv);
      if(r < 0){
	   fprintf(stderr, "%s: While waiting for socket: %s", __PRETTY_FUNCTION__, strerror(errno));
      } else if (r == 0){
	   // Timeout
	   timeout = true;
	   break;
      }
      // read as much as there is for the socket.
      if ((r = read(sock, &buf[numread], 1)) < 0){
	   perror("when reading from network");
	   break;
      }
      // update numread
      numread += r;
      // update how much space is left in the buffer
      space = sizeof(buf) - numread;
      // null-terminate buffer to allow strchr to work properly
      buf[numread]='\0'; 
    }
    //If we exit from here, we'll have read exactly one line into the buffer
    //or encountered a timeout/read error.
    if (timeout){
      PLAYER_MSG0(6,"Read Timeout while trying to read from server");
    }else{
      ParseData(buf);
    }
  }
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}
/**
 *
 */
int UsBot::SpawnBot()//char* botClass, char* botPos, char* botName)
{
  char* cmd = new char[USBOT_MAX_CMD_LEN];
  cmd[0] = 0;
  sprintf(cmd,"INIT {ClassName %s} {Name %s} {Location %f %f %f} {Rotation %f %f %f}\r\n",botClass,botName,initPose.px,initPose.py,initPose.pz,initPose.proll,initPose.ppitch,initPose.pyaw);
  AddCommand(cmd);
  return 0;
}
/**
 *
 */
int UsBot::AddCommand(char* command)
{
  queue.push_back(command);
  return 0;
}
/**
 *
 */
void UsBot::ParseData(char* data)
{
  char *pBody;
  int type = us_get_type(data,&pBody);
  if (type==-1) return;
  // fakelocalization pose
  if((type & devices & US_STATUS) && (location!=NULL)) {
    if (!bLockLocation || WaitUnlock(&bLockLocation)) {
      us_get_groundTruth(pBody,location);
      bNewLocation = true;
    }
  }
  // parse ticks of wheel encoders
  if(type & devices & US_DATA_ENCODER)
  {
    int ticks_left, ticks_right;
    us_get_enc(pBody, ticks_left, ticks_right);

    if(bEncoderSubscribed && drvPosition != NULL) {
      drvPosition->setTicks(ticks_left, ticks_right);
    }
  }
  // parse data of odometry device
  if(type & devices & US_DATA_POSITION)
  {
    player_position2d_data_t *position = new player_position2d_data_t;
    us_get_position(pBody, position);
    
    if(bPositionSubscribed && drvPosition != NULL) {
      drvPosition->setPosition(position);
    }
  }
  if ((type & devices & US_DATA_POSITION3D) && (position3d!=NULL))
  {
    bNewPosition3d = false;
    if (!bLockPosition3d || WaitUnlock(&bLockPosition3d))
      us_get_position3d(pBody,position3d);
    bNewPosition3d = true;
  }
  if ((type & devices & US_DATA_ODOM) && (position3d!=NULL)){
      bNewPosition3d = false;
      if (!bLockPosition3d || WaitUnlock(&bLockPosition3d))
	  us_get_position3d(pBody,position3d);
      bNewPosition3d = true;
  }
  if (type & devices & US_DATA_PTZ)
  {
    for(unsigned int i=0; i < ptz.size();i++) {
	 if(ptz[i]==NULL)
	 {
	   break;
	 }
	 bool dummy = bLockPtz[i];
	 if (!bLockPtz[i] || WaitUnlock(&dummy)) {
	   if( us_get_ptz(pBody,cam_name.at(i),ptz.at(i),ptz_geom[i]) == 0){
		bNewPtz.at(i) = true;
		bGeoPtz.at(i) = true;  //ptz sen also has geo information
	   }
	 }
    }
  }
  else if ((type & devices & US_DATA_INU) && (position3d!=NULL)){
      bNewINU = false;
      if (!bLockINU || WaitUnlock(&bLockINU))
	  us_get_inu(pBody,position3d);
      bNewINU = true;
  }
  else if ((type & devices & US_DATA_FIDUCIAL) && (fiducial!=NULL))
  {
      bNewFiducial = false;
      if (!bLockFiducial || WaitUnlock(&bLockFiducial))
	  us_get_fiducial(pBody,fiducial);
      bNewFiducial = true;
  } 
  else if ((type & devices & US_DATA_VICTIM_FIDUCIAL) && (victim_fiducial!=NULL))
  {
      bNewVictimFiducial = false;
      if (!bLockVictimFiducial || WaitUnlock(&bLockVictimFiducial))
	  us_get_victim_fiducial(pBody,victim_fiducial);
      bNewVictimFiducial = true;
  }
  else if (type & devices & US_DATA_SONAR)
  {
    for(unsigned int i=0; i < sonar.size();i++) {
	 if(sonar[i]==NULL) {
	   break;
	 }
	 bool dummy = bLockSonar[i];
	 if (!bLockSonar[i] || WaitUnlock(&dummy)) {
	   if( us_get_sonar(pBody,sonar_name.at(i),sonar.at(i)) == 0){
		bNewSonar[i] = true;
	   }
	 }
    }
  }
  // parse and publish geometry data for 2D lasers
  else if (type & devices & US_GEOM_LASER)
  {
    //PLAYER_MSG0(4, "parsing laser geom");
    map<char*, player_laser_geom_t*>* mGeom = new map<char*, player_laser_geom_t*>;
    int rc = us_get_laser_geom_all(pBody, mGeom);

    if(rc > -1)
    {
      PLAYER_MSG0(8, "us_bot: parsed laser geom");
      map<char*, player_laser_geom_t*>::iterator iter = (*mGeom).begin();
      // iterate through the list of geom objects
      for(; iter != (*mGeom).end(); iter++)
      {
        // make the data available for the lasers
        PLAYER_MSG1(8, "us_bot: geom for %s", (*iter).first);
        string s((*iter).first);
        if (mLaser[s] != NULL) {
          mLaser[s]->SetGeom((*iter).second);
        } else {
          delete (*iter).second;
        }
        delete (*iter).first;
      }
    } else {
      PLAYER_ERROR1("us_bot: laser geom parsing error %d", rc);
    }
    delete mGeom;
  }
  // Get the geometry model of laser 3d.
  else if (type & devices & US_GEOM_LASER3D)
  {
    //PLAYER_MSG0(4, "parsing laser3d geom");
    map<char*, player_laser3d_geom_t*>* mGeom = new map<char*, player_laser3d_geom_t*>;
    int rc = us_get_laser3d_geom_all(pBody, mGeom);

    if(rc > -1)
    {
      PLAYER_MSG0(8, "us_bot: parsed laser3d geom");
      map<char*, player_laser3d_geom_t*>::iterator iter = (*mGeom).begin();
      // iterate through the list of geom objects
      for(; iter != (*mGeom).end(); iter++)
      {
        // make the data available for the lasers
        PLAYER_MSG1(8, "us_bot: geom for %s", (*iter).first);
        string s((*iter).first);
        if (mLaser3d[s] != NULL) {
          mLaser3d[s]->SetGeom((*iter).second);
        } else {
          delete (*iter).second;
        }
        delete (*iter).first;
      }
    } else {
      PLAYER_ERROR1("us_bot: laser3d geom parsing error %d", rc);
    }
    delete mGeom;
  }
  // parse and publish scan data of 2D lasers
  else if (type & devices & US_DATA_LASER)
  {
    char* name = new char[128];
    player_laser_data_t* data = new player_laser_data_t;
    if(us_get_laser(pBody, name, data) == 0)
    {
      string s(name);
      if (mLaserSubscribed[s]) {
        mLaser[s]->SetData(data);
      }
      else {
        delete data;
      }
    }
    delete name;
  }
  // Get the scan data from laser 3d.
  // The pointcloud3d data structure is not yet included in the standard
  // player header file, consequently we needed temp_laser3d_structures.h.
  else if (type & devices & US_DATA_LASER3D)
  {
    char* name = new char[128];
    player_pointcloud3d_data_t* data = new player_pointcloud3d_data_t;
    if(us_get_laser3d(pBody, name, data) == 0)
    {
      string s(name);
      if(mLaser3dSubscribed[s]) {
        mLaser3d[s]->SetData(data);
      }
      else {
        delete data;
      }	 
    }
    delete name;
  }  
  // parse and publish config data for 2D lasers
  else if (type & devices & US_CONF_LASER)
  {
    //PLAYER_MSG0(8, "parsing laser conf");
    map<char*, player_laser_config_t*>* mConf = new map<char*, player_laser_config_t*>;
    int rc = us_get_laser_config_all(pBody, mConf);

    if(rc > -1)
    {
      PLAYER_MSG0(8, "us_bot: parsed laser conf");
      map<char*, player_laser_config_t*>::iterator iter = (*mConf).begin();
      // iterate through the list of config objects
      for(; iter != (*mConf).end(); iter++)
      {
        // make the data available for the lasers
        PLAYER_MSG1(8, "us_bot: conf for %s", (*iter).first);
        string s((*iter).first);
        if (mLaser[s] != NULL) {
          mLaser[s]->SetConf((*iter).second);
        } else {
          delete (*iter).second;
        }
        delete (*iter).first;
      }
    } else {
      PLAYER_ERROR1("us_bot: laser config parsing error %d", rc);
    }
    delete mConf;
  }
  // Get the configuration of a 3d laser scanner.
  // A data structure corresponding to the config parameters
  // of a 3d range scanner has not yet been included in the
  // standard player header file, therefore temp_laser3d_structures.h
  // has been included.
  else if (type & devices & US_CONF_LASER3D)
  {
    //PLAYER_MSG0(8, "parsing laser conf");
    map<char*, player_laser3d_config_t*>* mConf = new map<char*, player_laser3d_config_t*>;
    int rc = us_get_laser3d_config_all(pBody, mConf);

    if(rc > -1)
    {
      PLAYER_MSG0(8, "us_bot: parsed laser3d conf");
      map<char*, player_laser3d_config_t*>::iterator iter = (*mConf).begin();
      // iterate through the list of config objects
      for(; iter != (*mConf).end(); iter++)
      {
        // make the data available for the lasers
        PLAYER_MSG1(8, "us_bot: conf for %s", (*iter).first);
        string s((*iter).first);
        if (mLaser3d[s] != NULL) {
          mLaser3d[s]->SetConf((*iter).second);
        } else {
          delete (*iter).second;
        }
        delete (*iter).first;
      }
    } else {
      PLAYER_ERROR1("us_bot: laser3d config parsing error %d", rc);
    }
    delete mConf;
  }
  else if ((type & devices & RH_DATA_IR) && (ir!=NULL))
  {
    bNewIr = false;
    if (!bLockIr || WaitUnlock(&bLockIr))
      rh_get_ir(pBody,ir);
    bNewIr = true;
  }
  else if ((type & devices & RH_DATA_PYRO) && (pyro!=NULL))
  {
    bNewPyro = false;
    if (!bLockPyro || WaitUnlock(&bLockPyro))
      rh_get_pyro(pBody,pyro);
    bNewPyro = true;
  }
  else if (type & devices & US_GEOM_SONAR)
  {
    for(unsigned int i = 0; i < bGeoSonar.size(); i++) {
	 if(us_get_sonar_geom(pBody,sonar_name[i],sonar_geom[i],bGeoSonar.at(i))!=-1){
	   bGeoSonar.at(i) = true;
	 }
	 else {
	   PLAYER_MSG0(1,"ERROR NO GROUND TRUTH DURING SONAR INITIALISATION\n");
	 }
    }
  }
  /*
  else if (type & devices & US_GEOM_PTZ)
  {
    cout<<"ptz Geom"<<endl;
    for(unsigned int i = 0; i < bGeoPtz.size(); i++) {
	 cout<<i<<endl;
	 if(us_get_ptz_geom(pBody,ptz_name[i],ptz_geom[i])!=-1){ 
	   bGeoPtz.at(i) = true;
	 }
    }
  }
  */
  else if (type & devices & US_CONF_CAMERA)
  {
    for(unsigned int i = 0; i < bGeoPtz.size(); i++) {
	 if(us_get_camera_config(pBody,cam_name[i],ptz[i])!=-1){ 
	   bNewPtzZoom.at(i) = true;
	 }
    }
  }
  else if ((type & devices & RH_GEOM_IR) && (ir_geom!=NULL))
  {
    bGeoIr = false;
    rh_get_ir_geom(pBody,ir_geom);
    bGeoIr = true;
  }
  else if ((type & devices & RH_GEOM_PYRO) && (pyro_geom!=NULL))
  {
    bGeoPyro = false;
    rh_get_pyro_geom(pBody,pyro_geom);
    bGeoPyro = true;
  }
  else if ((type & devices & RH_CONF_PYRO) && (pyro_conf!=NULL))
  {
    bConfPyro = false;
    rh_get_pyro_config(pBody,pyro_conf);
    bConfPyro = true;
  }
  else if ((type & devices & US_CONF_ROBOT) &&
           steeringType !=NULL)
  {
    bConfRobot = false;
    PLAYER_MSG0(3,"us bot get conf robot");
    if(us_get_robot_config(pBody,steeringType, robotMass, maxSpeed,
                           maxTorque, maxFrontSteer,maxRearSteer) == 0)
    {
	 PLAYER_MSG0(3,"us bot has conf");
     bConfRobot = true;
    }
    else PLAYER_MSG0(1,"ERROR PARSING ROBOT CONF\n"); 
  } 
  else if ((type & devices & US_GEOM_ROBOT) &&
           robotGeom != NULL && robotDimensions != NULL) {
      if(us_get_robot_geom(pBody,robotDimensions,
                             COG,wheelRadius,
                             maxWheelSeparation, wheelBase) == 0) 
      {
      robotGeom->pose.px = 0;//initPose.px;
          robotGeom->pose.py = 0;//-initPose.py;
          robotGeom->pose.pyaw = 0;//-initPose.pyaw; 
          robotGeom->size.sw = robotDimensions->sw;
          robotGeom->size.sl = robotDimensions->sl;
          if(position3d != NULL) {
              robotGeom3d->pose.px = 0;
              robotGeom3d->pose.py = 0;
              robotGeom3d->pose.pz = 0.0;
              robotGeom3d->pose.ppitch = 0;//initPose.ppitch + position3d->pos.ppitch;
              robotGeom3d->pose.pyaw = 0;//-initPose.pyaw + position3d->pos.pyaw;
              robotGeom3d->pose.proll = 0;//initPose.proll + position3d->pos.proll;
              robotGeom3d->size.sw = robotDimensions->sw;
              robotGeom3d->size.sl = robotDimensions->sl;
          }
          bGeoRobot = true;
      } else {
          PLAYER_MSG0(1,"ERROR PARSING ROBOT GEO\n"); 
      }
  }
}
/*
 *
 */
bool UsBot::WaitUnlock(bool* lock) {
  int count = 20;
  int delay = USBOT_DELAY_USEC/10;

  while(*lock && count-->0) usleep(delay);

  return count>0;
}
/*
 *
 */
void QuitUsBot(void* usarsimdevice) {
}

/*
 * Register a driver object so we can access it
 */
void UsBot::RegisterDriver(char* type, char* name, Driver* drv)
{
  if(strcmp(type, "RangeScanner") == 0) {
    mLaser[string(name)] = (UsLaser*)drv;
  }else if(strcmp(type, "RangeScanner3d") == 0) {
    mLaser3d[string(name)] = (UsLaser3d*)drv;
  } else if(strcmp(type, "Position") == 0) {
    drvPosition = (UsPosition*)drv;
  } else {
    PLAYER_ERROR2("unhandled driver type: %s, %s", type, name);
  }
}

/*
 * Subcribes a driver for data updates
 */
void UsBot::SubscribeDriver(char* type, char* name)
{
  if(strcmp(type, "RangeScanner") == 0)
  {
    mLaserSubscribed[string(name)] = true;
    devices |= US_DATA_LASER;
  }
  else if(strcmp(type, "RangeScanner3d") == 0)
  {
    mLaser3dSubscribed[string(name)] = true;
    devices |= US_DATA_LASER3D;
  }
  else if(strcmp(type, "Position") == 0)
  {
    this->bPositionSubscribed = true;
    devices |= US_DATA_POSITION;
  }
  else if(strcmp(type, "Encoder") == 0)
  {
    this->bEncoderSubscribed = true;
    devices |= US_DATA_ENCODER;
  }
  else {
    PLAYER_ERROR2("unhandled driver subscribe: %s, %s", type, name);
  }
}

/*
 * Unsubscribes a driver from updates
 */
void UsBot::UnsubscribeDriver(char* type, char* name)
{
  if(strcmp(type, "RangeScanner") == 0)
  {
    mLaserSubscribed.erase(string(name));
    if(mLaserSubscribed.size() == 0) {
      this->devices &= ~(US_DATA_LASER);
    }
  }
  else if(strcmp(type, "RangeScanner3d") == 0)
  {
    mLaser3dSubscribed.erase(string(name));
    if(mLaser3dSubscribed.size() == 0) {
      this->devices &= ~(US_DATA_LASER3D);
    }
  }
  else if(strcmp(type, "Position") == 0)
  {
    this->bPositionSubscribed = false;
    this->devices &= ~(US_DATA_POSITION);
  }
  else if(strcmp(type, "Encoder") == 0)
  {
    this->bEncoderSubscribed = false;
    this->devices &= ~(US_DATA_ENCODER);
  }
  else {
    PLAYER_ERROR2("unhandled driver unsubscribe: %s, %s", type, name);
  }
}

/*
 * Request geometry information from USARSim, will be pushed to driver
 */
void UsBot::RequestGeom(char* type, char* name)
{
  if(strcmp(type, "RangeScanner") == 0) {
      this->devices |= US_GEOM_LASER;
  }
  else if(strcmp(type, "RangeScanner3d") == 0) {
      this->devices |= US_GEOM_LASER3D;
  }
  else {
    PLAYER_ERROR2("unhandled geom type: %s, %s", type, name);
  }

  char* cmd = new char[USBOT_MAX_CMD_LEN];
  // request data only for one specific sensor
  //sprintf(cmd, "GETGEO {Type %s} {Name %s}\r\n", type, name);
  // request data for all sensor of same type
  sprintf(cmd, "GETGEO {Type %s}\r\n", type);
  this->AddCommand(cmd);
}
/*
 * Request config information from USARSim, will be pushed to driver
 */
void UsBot::RequestConf(char* type, char* name)
{
  if(strcmp(type, "RangeScanner") == 0) {
      this->devices |= US_CONF_LASER;
  }
  if(strcmp(type, "RangeScanner3d") == 0) {
      this->devices |= US_CONF_LASER3D;
  }
  else {
    PLAYER_ERROR2("unhandled config type: %s, %s", type, name);
  }
  char* cmd = new char[USBOT_MAX_CMD_LEN];
  // request data only for one specific sensor
  //sprintf(cmd, "GETCONF {Type %s} {Name %s}\r\n", type, name);
  // request data for all sensor of same type
  sprintf(cmd, "GETCONF {Type %s}\r\n", type);
  this->AddCommand(cmd);
}


