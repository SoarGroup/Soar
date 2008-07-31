/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al.
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

/** Mixed mode driver for Garcia robot by Acroname
  * @author Brad Kratochvil
  * @date 20050915
  * @ingroup drivers
  * @{ */

/**
  *@defgroup driver_garcia garcia
  @brief The Garcia mobile robot

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

The garcia driver captures

@par Compile-time dependencies

- &lt;acpGarcia.h&gt;

@par Provides

- @ref interface_position2d
- @ref interface_ir
- @ref interface_speech
- @ref interface_dio
- @ref interface_power
- @ref interface_ptz (not yet implemented)
- @ref interface_position1d (not yet implemented)


@par Requires

- none

@par Configuration requests

- @ref interface_position2d
  - PLAYER_POSITION2D_REQ_GET_GEOM
  - PLAYER_POSITION2D_REQ_SET_ODOM :
  - PLAYER_POSITION2D_REQ_RESET_ODOM :
  - PLAYER_POSITION2D_REQ_MOTOR_POWER :
  - PLAYER_POSITION2D_REQ_SPEED_PID :
  - PLAYER_POSITION2D_REQ_POSITION_PID :
  - PLAYER_POSITION2D_REQ_SPEED_PROF :
  - PLAYER_IR_REQ_POSE :

@par Configuration file options

- port (filename)
  - Default: "ttyS0"
  - Path to the serial port
- baud (int)
  - Default: 38400
  - Baudrate of the serial port
- speed (float)
  - Default: 0.7f
  - The speed for speaking the phrase.  Values have a range of 0.0 to 1.0 and
    will be clamped to this range. 0.0 is the slowest and 1.0 is the fastest
    speed for saying the phrase.
- pitch (float)
  - Default: 0.6f
  - The pitch for speaking the phrase.  Values have a range of 0.0 to 1.0 and
    will be clamped to this range. 0.0 is the lowest and 1.0 is the highest
    pitch for saying the phrase.
- volume (float)
  - Default: 1.0f
  - The volume for speaking the phrase.  Values have a range of 0.0 to 1.0 and
    will be clamped to this range. 0.0 is the quietest and 1.0 is the loudest
    volume for saying the phrase.

@par Example

@verbatim
driver
(
  name "garcia"
  provides ["position2d:0"
            "ir:0"
            "dio:0"
            "speech:0"]
  port "ttyS0"
  baud "38400"
)
@endverbatim

@author Brad Kratochvil

*/
/** @} */

// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/
#include "garcia_mixed.h"

#define DEG2RAD(x) (((double)(x))*0.01745329251994)
#define RAD2DEG(x) (((double)(x))*57.29577951308232)

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
using namespace std;

#include <iostream> // only used for debugging, so remove when done

const timespec NSLEEP_TIME = {0, 20000000}; // (0s, 20 ms) => max 50 hz

////////////////////////////////////////////////////////////////////////////////
// Now the driver

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
GarciaDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*)(new GarciaDriver(cf, section)));

}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void
GarciaDriver_Register(DriverTable* table)
{
  table->AddDriver("garcia", GarciaDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
GarciaDriver::GarciaDriver(ConfigFile* cf, int section)
    : Driver(cf, section),
      mLength(0.28),
      mWidth(0.20),
      mWheelBase(0.182),
      mWheelRadius(0.1)
{
  // Create position2d interface
  if (0 != cf->ReadDeviceAddr(&mPos2dAddr, section, "provides",
                              PLAYER_POSITION2D_CODE, -1, NULL))
  {
    PLAYER_ERROR("Could not read position2d ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mPos2dAddr))
  {
    PLAYER_ERROR("Could not add position2d interface ");
    SetError(-1);
    return;
  }

  // Create ir interface
  if (0 != cf->ReadDeviceAddr(&mIrAddr, section, "provides",
                              PLAYER_IR_CODE, -1, NULL))
  {
    PLAYER_ERROR("Could not read ir ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mIrAddr))
  {
    PLAYER_ERROR("Could not add ir interface ");
    SetError(-1);
    return;
  }

  // Create speech interface
  if (0 != cf->ReadDeviceAddr(&mSpeechAddr, section, "provides",
                              PLAYER_SPEECH_CODE, -1, NULL))
  {
    PLAYER_ERROR("Could not read speech ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mSpeechAddr))
  {
    PLAYER_ERROR("Could not add speech interface ");
    SetError(-1);
    return;
  }

  // Create dio interface
  if (0 != cf->ReadDeviceAddr(&(mDioAddr),section,"provides",
                              PLAYER_DIO_CODE,-1,NULL))
  {
    PLAYER_ERROR("Could not read dio ID ");
    SetError(-1);
    return;
  }
  if (0 != AddInterface(mDioAddr))
  {
    PLAYER_ERROR("Could not add dio interface ");
    SetError(-1);
    return;
  }

  // Create power interface
  if (0 != cf->ReadDeviceAddr(&mPowerAddr,
                            section,
                            "provides",
                            PLAYER_POWER_CODE,
                            -1,
                            NULL))
  {
    PLAYER_ERROR("could not read power address");
    SetError(-1);
    return;
  }

  if (0 != AddInterface(mPowerAddr))
  {
    PLAYER_ERROR("could not add power interface");
    SetError(-1);
    return;
  }

  // Read options from the configuration file
  const char* portname = cf->ReadFilename(section, "portname", "ttyS0");
  int baudrate = cf->ReadInt(section, "baudrate", 38400);

  // let's just create the config file wherever we are:
  static FILE* config_file;
  config_file = fopen("garcia_api.config", "a+");

  fprintf(config_file, "portname=%s\n", portname);
  fprintf(config_file, "baudrate=%i\n", baudrate);

  mSpeed = static_cast<float>(cf->ReadFloat(section, "speed", 0.7f));
  mPitch = static_cast<float>(cf->ReadFloat(section, "pitch", 0.6f));
  mVolume = static_cast<float>(cf->ReadFloat(section, "volume", 1.0f));

  fclose(config_file);

  return;
}

GarciaDriver::~GarciaDriver()
{
  // get rid of the Acroname config file
  //remove("garcia_api.config");
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int
GarciaDriver::Setup()
{

  cout << "Setting up Garcia driver" << flush;
  mGarcia = new acpGarcia;

  while (!mGarcia->getNamedValue("active")->getBoolVal())
  {
    cout << "." << flush;
    nanosleep(&NSLEEP_TIME, NULL);
  }

  // enable the IR sensors
  acpValue enable(1);
  mGarcia->setNamedValue("front-ranger-enable", &enable);
  mGarcia->setNamedValue("side-ranger-enable", &enable);
  mGarcia->setNamedValue("rear-ranger-enable", &enable);


  puts("finished!");

  // Start the device thread; spawns a new thread and executes
  // GarciaDriver::Main(), which contains the main loop for the driver.
  StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int
GarciaDriver::Shutdown()
{
  puts("Shutting Garcia driver down");

  // Stop and join the driver thread
  StopThread();

  // Here you would shut the device down by, for example, closing a
  // serial port.

  delete mGarcia;

  puts("Garcia driver has been shutdown");

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void
GarciaDriver::Main()
{

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Go to sleep for a while (this is a polling loop)
    nanosleep(&NSLEEP_TIME, NULL);

    // Process incoming messages
    ProcessMessages();

    // Write outgoing data
    RefreshData();

  }
  return;
}

// Process an incoming message
int
GarciaDriver::ProcessMessage(QueuePointer & resp_queue,
                             player_msghdr* hdr,
                             void* data)
{
  assert(resp_queue);
  assert(hdr);
  assert(data);

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                           PLAYER_POSITION2D_CMD_POS, mPos2dAddr))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_pos_t));
    ProcessPos2dPosCmd(hdr, *reinterpret_cast<player_position2d_cmd_pos_t *>(data));
    return(0);
  }
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                           PLAYER_POSITION2D_CMD_VEL, mPos2dAddr))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
    ProcessPos2dVelCmd(hdr, *reinterpret_cast<player_position2d_cmd_vel_t *>(data));
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                PLAYER_SPEECH_CMD_SAY, mSpeechAddr))
  {
    assert(hdr->size == sizeof(player_speech_cmd_t));
    ProcessSpeechCommand(hdr, *reinterpret_cast<player_speech_cmd_t *>(data));
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                                PLAYER_DIO_CMD_VALUES, mDioAddr))
  {
    assert(hdr->size == sizeof(player_dio_cmd_t));
    ProcessDioCommand(hdr, *reinterpret_cast<player_dio_cmd_t *>(data));
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_GET_GEOM, mPos2dAddr))
  {
    ProcessPos2dGeomReq(hdr);
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_IR_REQ_POSE, mIrAddr))
  {
    ProcessIrPoseReq(hdr);
    return(0);
  }
  else
  {
    PLAYER_ERROR1("GarciaDriver received unknown message: %s", hdr->type);
  }

  return -1;
}

void
GarciaDriver::ProcessPos2dPosCmd(player_msghdr_t* hdr,
                                 player_position2d_cmd_pos_t &data)
{
  printf("Position commands currently not implemented\n");

/*
  // this code could be used to implement the position control,
  // but it will need to be modified

  double rho(0),alpha(0),beta(0);
  double delta_x(0),delta_y(0);
  double v, omega;
  // To be locally exponentially stable:
  //  k_rho  > 0
  //  k_beta < 0
  //  k_alpha-k_rho > 0

  delta_x = x-mPosX;
  delta_y = y-mPosY;

  // If we're within 1 cm, stop (for now)
  if ((fabs(delta_x) > mTranslateDeadzone)||
      (fabs(delta_y) > mTranslateDeadzone)||
      (fabs(mPosTheta-theta) > mRotateDeadzone))
  {
    rho   = sqrt(pow(delta_x,2) + pow(delta_y,2));

    if (fabs(atan2(delta_y, delta_x) - mPosTheta) < M_PI/2)
    {

      alpha = atan2(delta_y,delta_x) - mPosTheta;
      alpha = fmod(alpha, M_PI_2); // M_PI/2 == M_PI_2

      beta  = -fmod(alpha,M_PI) - mPosTheta + theta;
      beta  = fmod(beta, M_PI);

      v     = mKRho * rho;
      omega = mKAlpha * alpha + mKBeta*beta;
    }
    else
    {
      alpha = atan2(-delta_y,-delta_x) - mPosTheta;
      alpha = fmod(alpha, M_PI_2); // M_PI/2 == M_PI_2

      beta  = -alpha - mPosTheta + theta;
      beta  = fmod(beta, M_PI);

      v     = -mKRho * rho;
      omega = mKAlpha * alpha + mKBeta*beta;
    }

    SetTargetVelocity(v, omega);
  }
  else
    SetTargetVelocity(0,0);
*/

}

void
GarciaDriver::ProcessPos2dVelCmd(player_msghdr_t* hdr,
                                 player_position2d_cmd_vel_t &data)
{
  double v(data.vel.px);
  double omega(data.vel.pa);

  acpValue vl(static_cast<float>(v - mWheelBase*omega/2.0));
  acpValue vr(static_cast<float>(v + mWheelBase*omega/2.0));

  mGarcia->setNamedValue("damped-speed-left", &vl);
  mGarcia->setNamedValue("damped-speed-right", &vr);

  // do we have to do this each time, or only once?
  acpObject* behavior;
  behavior = mGarcia->createNamedBehavior("null", "vel");
  mGarcia->queueBehavior(behavior);
}

void
GarciaDriver::ProcessSpeechCommand(player_msghdr_t* hdr,
                                   player_speech_cmd_t &data)
{
  // todo, there is currently a problem if we receive messages too quickly
  cout << data.string << endl;

  acpValue phrase(data.string);
  acpObject* behavior;

  behavior = mGarcia->createNamedBehavior("say", data.string);
  behavior->setNamedValue("phrase", &phrase);
  behavior->setNamedValue("speed", &mSpeed);
  behavior->setNamedValue("pitch", &mPitch);
  behavior->setNamedValue("volume", &mVolume);
  mGarcia->queueBehavior(behavior);
}

void
GarciaDriver::ProcessDioCommand(player_msghdr_t* hdr,
                                player_dio_cmd_t &data)
{
  PLAYER_WARN("Garcia driver currently doesn't support DIO commands");
}

void
GarciaDriver::ProcessPos2dGeomReq(player_msghdr_t* hdr)
{
  player_position2d_geom_t geom;

  geom.pose.px = 0.03; // [m]
  geom.pose.py = 0.00; // [m]
  geom.pose.pa = 0;    // [rad]
  geom.size.sl = mLength;  // [m]
  geom.size.sw = mWidth;  // [m]

  Publish(mPos2dAddr,
          PLAYER_MSGTYPE_RESP_ACK,
          PLAYER_POSITION2D_REQ_GET_GEOM,
          &geom, sizeof(geom), NULL);
}

void
GarciaDriver::ProcessIrPoseReq(player_msghdr_t* hdr)
{
  player_pose_t poses[6] = {{ 0.105, 0.045, M_PI/6}, //   front-left
                            { 0.105,-0.045,-M_PI/6}, //   front-right
                            { 0.080, 0.020, M_PI_2}, //   side-left
                            { 0.080,-0.020,-M_PI_2}, //   side-right
                            {-0.050, 0.070, M_PI},   //   rear-left
                            {-0.050,-0.070,-M_PI}};  //   rear-right

  player_ir_pose_t pose;
  pose.poses_count = 6;
  pose.poses = new double[pose.poses_count];
  memcpy(pose.poses, poses, 6*sizeof(player_pose3d_t));

  Publish(mIrAddr,
          PLAYER_MSGTYPE_RESP_ACK,
          PLAYER_IR_REQ_POSE,
          &pose);
  delete [] pose.poses;

}

void
GarciaDriver::RefreshData()
{
  // how do we update these?
  mPos2dData.pos.px  = 0.0;
  mPos2dData.pos.py  = 0.0;
  mPos2dData.pos.pa  = 0.0;

  mPos2dData.vel.px  = 0.0;
  mPos2dData.vel.py  = 0.0;
  mPos2dData.vel.pa  = 0.0;

  Publish(mPos2dAddr,
          PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
          reinterpret_cast<void*>(&mPos2dData), sizeof(mPos2dData), NULL);

  // update the IR data
  mIrData.voltages_count = 0;
  mIrData.ranges_count = 6;
  mIrData.ranges = new double[mIrData.ranges_count];
  
  mIrData.ranges[0] = mGarcia->getNamedValue("front-ranger-left")->getFloatVal();
  mIrData.ranges[1] = mGarcia->getNamedValue("front-ranger-right")->getFloatVal();
  mIrData.ranges[2] = mGarcia->getNamedValue("side-ranger-left")->getFloatVal();
  mIrData.ranges[3] = mGarcia->getNamedValue("side-ranger-right")->getFloatVal();
  mIrData.ranges[4] = mGarcia->getNamedValue("rear-ranger-left")->getFloatVal();
  mIrData.ranges[5] = mGarcia->getNamedValue("rear-ranger-right")->getFloatVal();

  Publish(mIrAddr,
          PLAYER_MSGTYPE_DATA, PLAYER_IR_DATA_RANGES,
          reinterpret_cast<void*>(&mIrData));
  delete [] mIrData.ranges;

  // do we currently have a dio device?
  static int dio_test = 0;
  mDioData.count = 16;
  mDioData.digin = ++dio_test;

  Publish(mDioAddr,
          PLAYER_MSGTYPE_DATA, PLAYER_DIO_DATA_VALUES,
          reinterpret_cast<void*>(&mDioData), sizeof(mDioData), NULL);

  mPowerData.valid = PLAYER_POWER_MASK_VOLTS | PLAYER_POWER_MASK_PERCENT;
  mPowerData.volts = mGarcia->getNamedValue("battery-voltage")->getFloatVal();
  mPowerData.percent = mGarcia->getNamedValue("battery-level")->getFloatVal();

  Publish(mPowerAddr,
          PLAYER_MSGTYPE_DATA, PLAYER_POWER_DATA_STATE,
          reinterpret_cast<void*>(&mPowerData), sizeof(mPowerData), NULL);
}
