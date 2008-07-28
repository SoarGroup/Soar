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
// Desc: USARSim (simulator) UTBot client functions
// Author: Jijun Wang
// Date: 11 May 2004
//
// 3 Mars 2005     Erik Winter 	added Ringhorne IR
// 11 Mars 2005    Erik Winter 	added RinghornePyro
// 14 Mars 2005    Erik Winter 	added call to RHPyro config
// 14 Mars 2005    Erik Winter 	Started porting USARSim to Player1.6, changed the constructor
// 18 Mars 2005    Erik Winter     Changed ir_geom from player_ir_pose_req_t
//                                 to player_ir_pose__t in the 1.6 version 
// 21 April 2005   Andreas Pfeil   Made Driver loadable as a plugin and only 
//                                 compatible to player 1.6
// 21 Juli 2006 Stefan Stiene and
//              Nils Rosemann      update the driver to player-2.0.2
// 20 Nov 2006  Nils Kehrein Modified laser<->bot interaction
// 22 Nov 2006  Stefan Stiene and
//              Florian Halbritter Added support for UsarSim RangeScanner3d.
///////////////////////////////////////////////////////////////////////////

#ifndef US_BOT_H
#define US_BOT_H

#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>

using std::vector;
using std::map;
using std::string;

#if 0
// Error macros
#define PLAYER_ERROR(m) \
    printf("player error : %s:%s():\n    "m"\n", __FILE__, __FUNCTION__)

#define PLAYER_ERROR1(m, a) \
    printf("player error : %s:%s():\n    "m"\n", __FILE__, __FUNCTION__, a)

#define PLAYER_ERROR2(m, a, b) \
    printf("player error : %s:%s():\n    "m"\n", __FILE__, __FUNCTION__, a, b)
#endif

#include <libplayercore/playercore.h>

// Include a header file containing all definitions of structures etc. not yet
// included in the player headerfile of the standard release.
#include "temp_laser3d_structures.h"

#include "us_parser.h"
/*
 * Forward declaration of classes to make the compile happy
 */
class UsLaser;
class UsPosition;
class UsLaser3d;

// default Gamebots address
#define DEFAULT_GAMEBOTS_IP "127.0.0.1"
#define DEFAULT_GAMEBOTS_PORT 3000

// default bot parameters
#define DEFAULT_GAMEBOTS_POS "0,0,0"
#define DEFAULT_GAMEBOTS_ROT "0,0,0"
#define DEFAULT_GAMEBOTS_CLASS "P2AT"
#define DEFAULT_TIRE_RADIUS "-1"
#define DEFAULT_ROBOT_RADIUS "-1"

/* the following setting mean that we first try to connect after 1 seconds,
 * then try every 100ms for 6 more seconds before giving up */
#define USBOT_STARTUP_USEC 1000000 /* wait before first connection attempt */
#define USBOT_MAIN_LOOP_USEC 100
#define USBOT_STARTUP_INTERVAL_USEC 100000 /* wait between connection attempts */
#define USBOT_STARTUP_CONN_LIMIT 60 /* number of attempts to make */

/* delay 10ms inside loop */
#define USBOT_DELAY_USEC 10000

#define USBOT_MAX_QUE_LEN 32
#define USBOT_MAX_MSG_LEN 4096
#define USBOT_MAX_CMD_LEN 1024
///This class implements the UTBot client handling.
/**
 * This class stands for one robot in the USARSim simulation environment. It connects to the USARSim\n
 * server (UsBot::Setup()).\n
 * After that it reads in an endless loop (UsBot::Main()) the USARSim Sensor, Geo, Conf,... Strings.\n
 * This String is send to the us_parser(UsBot::ParseData()). The parser determines the type of the String and accoring to\n
 * this type and the fact if a player driver subscribed for this data (UsBot::devices) the UsBot decides\n
 * which method in us_parser is called (or no method).\n
 * If you want to sent a string to USARSim (GETGEO, GETCONF,...)you can use the UsBot::AddCommand() function.\n
 *\n
 * The player configfile entry looks like this:\n
 *driver\n
 *(\n
 *  name "us_bot"\n
 *  provides ["simulation:0"]\n
 *  port 3000\n
 *  host "127.0.0.1"\n
 *  pos "-1,-13,0.7"\n
 *  rot "0,0,0"\n
 *  bot "USARBot.P2AT"\n
 *  botname "robot1"\n
 *  robot_radius "0.5"\n
 *  tire_radius "0.1"\n
 *)\n
 * The port and host values are the ones you run the USARSim server.\n
 * pos == start position in the map\n
 * rot == start rotation in the map\n
 * bot == bot class of this robot\n
 * botname == botname\n
 * robot_radius and tire_radius are needed to compute the DRIVE commands
 */
class UsBot : public Driver
{
private:
  /// a queue to hold commands will be sent to Gamebots
  vector<char*> queue;
  /// switch witch determines if we preload geo and conf information of our sensors
  bool preloadSensorInfo;
  /**
   * this method parses the usarsim string
   * called in the main loop
   */
  void ParseData(char* data);
  /**
   * this method waits a little amount of time
   * to give the devices time to send the data away
   */
  bool WaitUnlock(bool* lock);
  
public:
  /// Gamebots Address 
  char host[MAX_FILENAME_SIZE];
  /// Gamebots port
  int port;
  /// socket for Gamebots
  int sock;
  /// the subscribed devices
  int devices;
  
  /// Initial bot spawning parameter pose
  player_pose3d_t initPose;
  /// Initial bot spawning parameter class name
  char botClass[MAX_FILENAME_SIZE];
  /// Initial bot spawning parameter roboter name
  char botName[MAX_FILENAME_SIZE];

  ///
  /// actual robot position according to odometrie
  /// contains x (position->px) , y (position->py)
  /// and theta (position->pa)\n
  /// Its the difference between the INIT Position
  /// or the last odometry reset position and the actual
  /// position based on the USARSim Odometry driver or
  /// USARSim encoder sensors.
  /// position driver
  UsPosition *drvPosition;
  bool bPositionSubscribed;
  bool bEncoderSubscribed;

  /*map<string, UsPosition*> mPosition;
  map<string, bool> mPositionSubscribed;

  map<string, UsPosition*> mEncoder;
  map<string, bool> mEncoderSubscribed;*/

  
  /// robot dimensions
  player_position2d_geom_t *robotGeom;
  // robot position based on odometry
  //player_position2d_data_t *position;
  // if true new position available
  //bool bNewPosition;
  // if true block writing on UsBot::position
  // so UsPosition::PublishNewData() has time to send the position away
  //bool bLockPosition;
  ///
  /// actual robot position in global coordinate system.
  /// Since it uses the USARSim status message its a ground truth
  /// position.\n
  /// The player driver us_fakelocalize uses this value
  ///
  /// ground truth robot position in global coordinate system
  player_localize_data_t *location;
  /// if new ground truth location available 
  bool bNewLocation;
  /// if true block writing on UsBot::location
  /// so UsFakeLocalize::PublishNewData() has time to send the location away
  bool bLockLocation;
  /**
   * This variable contains the same information as the
   * UsBot::position variable but stores it in an player_position3d_data_t
   * struct. The z coordinate is set to zero,\n
   * The only difference is, that the speed is computed in different ways.
   * (parser::us_get_position3d() and parser::us_get_position2d())  
   */
  ///
  player_position3d_geom_t *robotGeom3d;
  ///2D position in 3D interface
  player_position3d_data_t *position3d;
  /// new 3D position available
  bool bNewPosition3d;
  /// if true lock UsBot::position3d so that UsPosition3d::PublishNewData() can send it away
  bool bLockPosition3d;
  /**
   * 3d data from the USARSim inu sensor. At the moment there is no
   * player driver who needs this information.\n
   * Todo integrate the inu data in the UsPosition3d driver
   */
  player_position3d_data_t *inu;
  /// new inu data available
  bool bNewINU;
  /// if true lock UsBot::inu so that UsPosition3d::PublishNewData() can send it away
  bool bLockINU;
  
  vector<player_sonar_data_t *> sonar;
  ///each sonar get one name field in this vector
  vector<char *> sonar_name;
  ///each sonar get one laser geom field in this vector\n
  vector<player_sonar_geom_t *> sonar_geom;
  ///bNewSonar[i] == true -> new data available for sonar i
  vector<bool> bNewSonar;
  ///bLockSonar[i] == true -> UsSonar i sends data away -> lock sonar
  vector<bool> bLockSonar;
  ///bGeoSonar[i] == true -> new sonar geometry available for sonar i
  vector<bool> bGeoSonar;
  ///
  player_ir_data_t *ir;
  ///
  player_ir_pose_t *ir_geom;
  ///
  bool bNewIr, bLockIr, bGeoIr;
  ///
  player_fiducial_data_t *pyro;
  ///
  player_fiducial_geom_t *pyro_geom;
  ///
  player_fiducial_fov_t *pyro_conf;
  ///
  bool bNewPyro, bLockPyro, bGeoPyro, bConfPyro;

  /* --- Fields for UsLaser3d ---*/
  // Since the corresponding some of the data structures have not yet
  // been included in the standard player header file,
  // temp_laser3d_structures.h needed to be included.
  /// map holds laser3d names and the associated laser3d object
  map<string, UsLaser3d*> mLaser3d;
  /// map identifies which laser3ds are currently subscribed
  map<string, bool> mLaser3dSubscribed;
  /* --- /Fields for UsLaser3d ---*/

  /// map holds laser names and the associated laser object
  map<string, UsLaser*> mLaser;
  /// map identifies which lasers are currently subscribed
  map<string, bool> mLaserSubscribed;
  
  ///pan, tilt, zoom data of the ptz (camera)
  vector<player_ptz_data_t *> ptz;
  ///position an dimension of the ptz (camera)
  vector<player_ptz_geom_t *> ptz_geom;
  ///mispkg ptz (camera) names we need this to get the corresponding MISPKG in parser
  vector<char *> ptz_name;
  ///camera names we need this to get the camera zoom in parser for the UsBot::ptz data field
  vector<char *> cam_name;
  ///new tilt and zoom for camera i available
  vector<bool> bNewPtz;
  ///new camera zoom for camera i available
  vector<bool> bNewPtzZoom;
  ///bLockPtz[i] == true -> UsPtz [i] sends data away -> lock UsBot::ptz [i]
  vector<bool> bLockPtz;
  ///bGeoPtz[i] == true -> new ptz geometry available for UsPtz i\n
  /**
   *contains pose and size of ptz
   */
  vector<bool> bGeoPtz;

  ///
  player_fiducial_data_t * fiducial;
  ///
  bool bNewFiducial;
  ///
  bool bLockFiducial;
  ///
  player_victim_fiducial_data_t *victim_fiducial;
  /// 
  bool bNewVictimFiducial;
  ///
  bool bLockVictimFiducial;

  ///
  bool bGeoRobot;
  ///
  bool bConfRobot;
  ///
  double robotMass;
  ///
  player_bbox3d_t *robotDimensions;
  ///
    double COG[3];
    ///
    double wheelBase;
  ///
  double maxSpeed;
  ///
  double maxTorque;
  ///
  char* steeringType;
  /// tire radius used to compute driving commands
  double wheelRadius;
  /// robot radius used to compute driving commands
  double maxWheelSeparation;
  ///
  double maxFrontSteer;
  ///
  double maxRearSteer;


  
  /**
   * Constructer
   * @param cf the player ConfigFile containing all drivers for example usarsim.cfg
   * @param section the section for this driver in the configfile
   */
  UsBot(ConfigFile* cf, int section);
  /**
   *
   */
  ~UsBot();
  /**
   * main method contains a while(true) loop
   * it reads the usarsim string from the socket
   * and calls the UsBot::ParseData() method. 
   */
  void Main();
  /**
   * this method realises the connection to usarsim
   * it is called in the UsBot Constructor
   */
  int Setup();
  /**
   * stops the us_bot thread
   */
  int Shutdown();
  /**
   * adds a usarsim command to the UsBot::queue.
   * this command is send in the main loop
   * @param command the usarsim command
   */
  int AddCommand(char* command);
  /**
   * this method is called in UsBot::Setup() and generates a
   * usarsim INIT command and calls the UsBot::AddCommand(char * command)
   * method with this command.
   */
  int SpawnBot();

  /**
   * Registers a driver, so it is known to the bot
   * This method is called by driver objects because the
   * bot must be able to call methods of the laser object
   * @param type String identifying driver type, e.g. "RangeScanner"
   * @param name Unique name of the driver object, e.g. "Laser_Rear"
   * @param laser drv Pointer to dirver object
   */
  void RegisterDriver(char* type, char* name, Driver* drv);
  /**
   * Subscribes a driver to get data (activates parsing)
   * @param type String identifying driver type, e.g. "RangeScanner"
   * @param name Unique name of the driver object, e.g. "Laser_Rear"
   */
  void SubscribeDriver(char* type, char* name);
  /**
   * Unsubscribe a driver from data updates (deactivates parsing)
   * @param type String identifying driver type, e.g. "RangeScanner"
   * @param name Unique name of the driver object, e.g. "Laser_Rear"
   */
  void UnsubscribeDriver(char* type, char* name);
  /**
   * Request geometry data for a certain sensor
   * @param type Sensor type, e.g. "RangeScanner"
   * @param name Unique sensor name, e.g. "Laser_Rear"
   */
  void RequestGeom(char* type, char* name);
  /**
   * Request config data for a certain sensor (works currently only for lasers)
   * @param type Sensor type, e.g. RangeScanner
   * @param name Unique sensor name
   */
  void RequestConf(char* type, char* name);

};

void QuitUsBot(void* usarsimdevice);
#endif //US_BOT_H
