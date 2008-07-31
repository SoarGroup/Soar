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
 * $Id: p2os.h 4356 2008-02-15 08:53:55Z thjc $
 *
 *   the P2OS device.  it's the parent device for all the P2 'sub-devices',
 *   like gripper, position, sonar, etc.  there's a thread here that
 *   actually interacts with P2OS via the serial line.  the other
 *   "devices" communicate with this thread by putting into and getting
 *   data out of shared buffers.
 */
#ifndef _P2OSDEVICE_H
#define _P2OSDEVICE_H

#include <pthread.h>
#include <sys/time.h>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

#include "packet.h"
#include "robot_params.h"

// Default max speeds
#define MOTOR_DEF_MAX_SPEED 0.5
#define MOTOR_DEF_MAX_TURNSPEED DTOR(100)

/*
 * Apparently, newer kernel require a large value (200000) here.  It only
 * makes the initialization phase take a bit longer, and doesn't have any
 * impact on the speed at which packets are received from P2OS
 */
#define P2OS_CYCLETIME_USEC 200000

/* p2os constants */

#define P2OS_NOMINAL_VOLTAGE 12.0

/* Command numbers */
#define SYNC0 0
#define SYNC1 1
#define SYNC2 2

#define PULSE 0
#define OPEN 1
#define CLOSE 2
#define ENABLE 4
#define SETA 5
#define SETV 6
#define SETO 7
#define VEL 11
#define RVEL 21
#define SETRA 23
#define SONAR 28
#define STOP 29
#define VEL2 32
#define GRIPPER 33
#define GRIPPERVAL 36
#define TTY2 42		// Added in AmigOS 1.2
#define GETAUX 43	// Added in AmigOS 1.2
#define BUMP_STALL 44
#define JOYDRIVE 47
#define GYRO 58         // Added in AROS 1.8
#define ROTKP 82        // Added in P2OS1.M
#define ROTKV 83        // Added in P2OS1.M
#define ROTKI 84        // Added in P2OS1.M
#define TRANSKP 85      // Added in P2OS1.M
#define TRANSKV 86      // Added in P2OS1.M
#define TRANSKI 87      // Added in P2OS1.M
#define TTY3 66		// Added in AmigOS 1.3
#define GETAUX2 67	// Added in AmigOS 1.3
#define ARM_INFO 70
#define ARM_STATUS 71
#define ARM_INIT 72
#define ARM_CHECK 73
#define ARM_POWER 74
#define ARM_HOME 75
#define ARM_PARK 76
#define ARM_POS 77
#define ARM_SPEED 78
#define ARM_STOP 79
#define ARM_AUTOPARK 80
#define ARM_GRIPPARK 81
#define SOUND 90
#define PLAYLIST 91

/* Server Information Packet (SIP) types */
#define STATUSSTOPPED	0x32
#define STATUSMOVING	0x33
#define	ENCODER		0x90
#define SERAUX		0xB0
#define SERAUX2		0xB8	// Added in AmigOS 1.3
#define GYROPAC         0x98    // Added AROS 1.8
#define ARMPAC    160   // ARMpac
#define ARMINFOPAC  161   // ARMINFOpac
//#define PLAYLIST	0xD0

/* Argument types */
#define ARGINT		0x3B	// Positive int (LSB, MSB)
#define ARGNINT		0x1B	// Negative int (LSB, MSB)
#define ARGSTR		0x2B	// String (Note: 1st byte is length!!)

/* gripper stuff */
#define GRIPopen   1
#define GRIPclose  2
#define GRIPstop   3
#define LIFTup     4
#define LIFTdown   5
#define LIFTstop   6
#define GRIPstore  7
#define GRIPdeploy 8
#define GRIPhalt   15
#define GRIPpress  16
#define LIFTcarry  17

/* CMUcam stuff */
#define CMUCAM_IMAGE_WIDTH	80
#define CMUCAM_IMAGE_HEIGHT	143
#define CMUCAM_MESSAGE_LEN	10

/* conection stuff */
#define DEFAULT_P2OS_PORT "/dev/ttyS0"
#define DEFAULT_P2OS_TCP_REMOTE_HOST "localhost"
#define DEFAULT_P2OS_TCP_REMOTE_PORT 8101

typedef struct player_p2os_data
{
  //Standard SIP
  player_position2d_data_t position;
  player_sonar_data_t sonar;
  player_gripper_data_t gripper;
  player_actarray_data_t lift;
  player_power_data_t power;
  player_bumper_data_t bumper;
  player_position2d_data_t compass;
  player_dio_data_t dio;
  player_aio_data_t aio;
  
  //Blobfinder
  player_blobfinder_data_t blobfinder;

  //Gyro
  player_position2d_data_t gyro;
  
  //ARMPAC
  player_actarray_data_t actArray;
  player_gripper_data_t armGripper;
} __attribute__ ((packed)) player_p2os_data_t;

// this is here because we need the above typedef's before including it.
#include <sip.h>

#include "kinecalc.h"

class SIP;

// Forward declaration of the KineCalc_Base class declared in kinecalc_base.h
//class KineCalc;

class P2OS : public Driver
{
  private:
    player_p2os_data_t p2os_data;

    player_devaddr_t position_id;
    player_devaddr_t sonar_id;
    player_devaddr_t aio_id;
    player_devaddr_t dio_id;
    player_devaddr_t gripper_id;
    player_devaddr_t lift_id;
    player_devaddr_t bumper_id;
    player_devaddr_t power_id;
    player_devaddr_t compass_id;
    player_devaddr_t gyro_id;
    player_devaddr_t blobfinder_id;
    player_devaddr_t audio_id;
    player_devaddr_t actarray_id;
    player_devaddr_t limb_id;
    player_devaddr_t armgripper_id;

    // Book keeping to only send new commands
    bool sentGripperCmd;
    uint8_t lastGripperCmd;
    uint8_t lastLiftCmd;
    player_actarray_position_cmd_t lastLiftPosCmd;
    bool sentArmGripperCmd;
    uint8_t lastArmGripperCmd;
    uint8_t lastActArrayCmd;
    player_actarray_position_cmd_t lastActArrayPosCmd;
    player_actarray_home_cmd_t lastActArrayHomeCmd;

    // bookkeeping to only send new audio I/O commands
    bool sent_audio_cmd;
    player_audio_sample_item_t last_audio_cmd;
    // PID settings
    int rot_kp, rot_kv, rot_ki, trans_kp, trans_kv, trans_ki;


    int position_subscriptions;
    int sonar_subscriptions;
    int actarray_subscriptions;

    SIP* sippacket;

    int SendReceive(P2OSPacket* pkt, bool publish_data=true);
    void ResetRawPositions();
    /* toggle sonars on/off, according to val */
    void ToggleSonarPower(unsigned char val);
    /* toggle motors on/off, according to val */
    void ToggleMotorPower(unsigned char val);
    int HandleConfig(QueuePointer & resp_queue,
                     player_msghdr * hdr,
                     void* data);
    int HandleCommand(player_msghdr * hdr, void * data);
    void StandardSIPPutData(double timestampStandardSIP);
    void GyroPutData(double timestampGyro);
    void BlobfinderPutData(double timestampSERAUX);
    void ActarrayPutData(double timestampArm);
    void HandlePositionCommand(player_position2d_cmd_vel_t position_cmd);
    int HandleGripperCommand (player_msghdr *hdr, void *data);
    int HandleLiftCommand (player_msghdr *hdr, void *data);
    int HandleArmGripperCommand (player_msghdr *hdr, void *data);
    void HandleAudioCommand(player_audio_sample_item_t audio_cmd);

    /////////////////
    // Gripper stuff
    player_pose3d_t gripperPose;
    player_bbox3d_t gripperOuterSize;
    player_bbox3d_t gripperInnerSize;
    player_bbox3d_t armGripperOuterSize;
    player_bbox3d_t armGripperInnerSize;
    void OpenGripper (void);
    void CloseGripper (void);
    void StopGripper (void);
    void OpenArmGripper (void);
    void CloseArmGripper (void);
    void StopArmGripper (void);

    /////////////////
    // Actarray stuff
    double aaLengths[6];
    double aaOrients[18];
    double aaAxes[18];
    player_point_3d_t aaBasePos;
    player_orientation_3d_t aaBaseOrient;
    inline double TicksToDegrees (int joint, unsigned char ticks);
    inline unsigned char DegreesToTicks (int joint, double degrees);
    inline double TicksToRadians (int joint, unsigned char ticks);
    inline unsigned char RadiansToTicks (int joint, double rads);
    inline double RadsPerSectoSecsPerTick (int joint, double speed);
    inline double SecsPerTicktoRadsPerSec (int joint, double secs);
    void ToggleActArrayPower (unsigned char val, bool lock = true);   // Toggle actarray power on/off
    void SetActArrayJointSpeed (int joint, double speed);             // Set a joint speed
    void HandleActArrayPosCmd (player_actarray_position_cmd_t cmd);
    void HandleActArrayHomeCmd (player_actarray_home_cmd_t cmd);
    int HandleActArrayCommand (player_msghdr * hdr, void * data);

    /////////////////
    // Limb stuff
    KineCalc *kineCalc;
    float armOffsetX, armOffsetY, armOffsetZ;
    // This is here because we don't want it zeroed every time someone fills in some other data
    player_limb_data_t limb_data;
    void HandleLimbHomeCmd (void);
    void HandleLimbStopCmd (void);
    void HandleLimbSetPoseCmd (player_limb_setpose_cmd_t cmd);
    void HandleLimbSetPositionCmd (player_limb_setposition_cmd_t cmd);
    void HandleLimbVecMoveCmd (player_limb_vecmove_cmd_t cmd);
    int HandleLimbCommand (player_msghdr * hdr, void * data);

    int param_idx;  // index in the RobotParams table for this robot
    int direct_wheel_vel_control; // false -> separate trans and rot vel
    int psos_fd;               // p2os device file descriptor
    const char* psos_serial_port; // name of serial port device
    bool psos_use_tcp;    // use TCP port instead of serial port
    const char* psos_tcp_host;  // hostname to use if using TCP
    int psos_tcp_port;  // remote port to use if using TCP


    struct timeval lastblob_tv;

    // Max motor speeds (mm/sec,deg/sec)
    int motor_max_speed;
    int motor_max_turnspeed;

    // Bound the command velocities
    bool use_vel_band;

    // Max motor accel/decel (mm/sec/sec, deg/sec/sec)
    short motor_max_trans_accel, motor_max_trans_decel;
    short motor_max_rot_accel, motor_max_rot_decel;

    int radio_modemp; // are we using a radio modem?
    int joystickp; // are we using a joystick?
    int bumpstall; // should we change the bumper-stall behavior?

    float pulse; // Pulse time
    double lastPulseTime; // Last time of sending a pulse or command to the robot
    void SendPulse (void);

  public:

    P2OS(ConfigFile* cf, int section);
	~P2OS (void);

    virtual int Subscribe(player_devaddr_t id);
    virtual int Unsubscribe(player_devaddr_t id);

    /* the main thread */
    virtual void Main();

    virtual int Setup();
    virtual int Shutdown();

    // MessageHandler
    virtual int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);

    void CMUcamReset(bool doLock = true);
    void CMUcamTrack(int rmin=0, int rmax=0, int gmin=0,
                          int gmax=0, int bmin=0, int bmax=0);
    void CMUcamStartTracking(bool doLock = true);
    void CMUcamStopTracking(bool doLock = true);
};


#endif
