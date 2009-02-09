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
 * $Id: wbr914.h 4135 2007-08-23 19:58:48Z gerkey $
 *
 *   the P2OS device.  it's the parent device for all the P2 'sub-devices',
 *   like gripper, position, sonar, etc.  there's a thread here that
 *   actually interacts with P2OS via the serial line.  the other
 *   "devices" communicate with this thread by putting into and getting
 *   data out of shared buffers.
 */
#ifndef _WBR914_H
#define _WBR914_H

#include <termios.h>
#include <pthread.h>
#include <sys/time.h>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

// Default max speeds
#define MOTOR_DEF_MAX_SPEED 0.3
#define MOTOR_DEF_MAX_TURNSPEED DTOR(100)
#define ACCELERATION_DEFAULT 100
#define DECELERATION_DEFAULT 250

/* PMD3410 command codes */
#define NOOP                0x00
#define SETPOS              0x10
#define SETVEL              0x11
#define UPDATE              0x1A
#define GETCMDPOS           0x1D
#define GETCMDVEL           0x1E
#define GETEVENTSTATUS      0x31
#define RESETEVENTSTATUS    0x34
#define GETACTUALPOS        0x37
#define RESET               0x39
#define SETACTUALPOS        0x4D
#define GETSAMPLETIME       0x61
#define SETPHASECOUNTS      0x75
#define SETMOTORCMD         0x77
#define SETLIMITSWITCHMODE  0x80
#define WRITEDIGITAL        0x82
#define READDIGITAL         0x83
#define GETVERSION          0x8F
#define SETACCEL            0x90
#define SETDECEL            0x91
#define SETPROFILEMODE      0xA0
#define GETACTUALVEL        0xAD
#define SETSTOPMODE         0xD0
#define SETMOTORMODE        0xDC
#define SETOUTPUTMODE       0xE0
#define READANALOG          0xEF

#define MOTOR_0             ((unsigned char)0x00)
#define MOTOR_1             ((unsigned char)0x01)

/* Robot configuration */
#define LEFT_MOTOR          MOTOR_1
#define RIGHT_MOTOR         MOTOR_0

/* Connection stuff */
#define DEFAULT_M3_PORT "/dev/ttyUSB0"

#define DELAY_US 10000

typedef enum {
  TrapezoidalProfile = 0,
  VelocityContouringProfile,
  SCurveProfile,
} ProfileMode_t;

typedef enum {
  NoStopMode = 0,
  AbruptStopMode,
  SmoothStopMode
} StopMode;

/* robot-specific info */
#define DEFAULT_MOTOR_0_DIR             -1
#define DEFAULT_MOTOR_1_DIR             1
#define DEFAULT_AXLE_LENGTH             (0.301)

#define MAX_TICKS               48000
#define WHEEL_DIAMETER          (0.125)
#define WHEEL_RADIUS            (WHEEL_DIAMETER/2.0)
#define WHEEL_CIRC              (M_PI*WHEEL_DIAMETER)
#define MOTOR_STEP              (1.8/MOTOR_TICKS_PER_STEP)
#define GEAR_RATIO              (4.8)
#define WHEEL_STEP              (MOTOR_STEP/GEAR_RATIO)
#define M_PER_TICK              (WHEEL_RADIUS/WHEEL_STEP)
#define MOTOR_TICKS_PER_STEP    (64.0)
#define MOTOR_TICKS_PER_REV     (200.0*MOTOR_TICKS_PER_STEP)
#define NUM_IR_SENSORS          8

/* for safety */
#define MAX_WHEELSPEED          8000
#define MPS_PER_TICK            1              // TODO: what is this?

#define FULL_STOP       0
#define STOP            1

#define DEFAULT_PERCENT_TORQUE  75



typedef struct
{
  player_position2d_data_t position;
  player_ir_data_t         ir;
  player_aio_data_t        aio;
  player_dio_data_t        dio;
} __attribute__ ((packed)) player_data_t;


class wbr914 : public Driver 
{
  public:

    wbr914(ConfigFile* cf, int section);
    virtual ~wbr914();

    virtual int  Subscribe(player_devaddr_t id);
    virtual int  Unsubscribe(player_devaddr_t id);

    /* the main thread */
    virtual void Main();

    virtual int  Setup();
    virtual int  Shutdown();

    // MessageHandler
    virtual int  ProcessMessage(QueuePointer &resp_queue, 
				player_msghdr * hdr, 
				void * data);

  // Private Member Functions
  private:
    bool RecvBytes( unsigned char*s, int len );
    int  ReadBuf(unsigned char* s, size_t len);
    int  WriteBuf(unsigned char* s, size_t len);
    int  sendCmdCom( unsigned char address, unsigned char c, 
		     int cmd_num, unsigned char* arg, 
		     int ret_num, unsigned char * ret );
    int  sendCmd0( unsigned char address, unsigned char c, 
		   int ret_num, unsigned char * ret );
    int  sendCmd16( unsigned char address, unsigned char c, 
		    int16_t arg, int ret_num, unsigned char * ret );
    int  sendCmd32( unsigned char address, unsigned char c, 
		    int32_t arg, int ret_num, unsigned char * ret );

    int32_t BytesToInt32( unsigned char *ptr );
    int16_t BytesToInt16( unsigned char *ptr );

    int  ResetRawPositions();
    int  HandleConfig(QueuePointer &resp_queue,
			      player_msghdr * hdr,
			      void* data);

    // Command handlers
    int  HandleCommand(player_msghdr * hdr, void * data);
    void HandleVelocityCommand(player_position2d_cmd_vel_t* cmd );
    void HandleDigitalOutCommand( player_dio_data_t* doutCmd );
    void SetDigitalData( player_dio_data_t * d );

    // Robot data retrievers
    void GetAllData( void );
    void GetPositionData( player_position2d_data_t* d );
    void GetIRData( player_ir_data_t * d );
    void GetAnalogData( player_aio_data_t * d );
    void GetDigitalData( player_dio_data_t * d );

    void PublishData(void);

    /* Robot commands */
    const char* GetPMDErrorString( int rc );
    int  InitRobot();
    void UpdateM3();
    void Stop( int StopMode );

    bool EnableMotors( bool enable );

    void SetVelocity( uint8_t chan, float mps );
    void SetVelocity( float mpsL, float mpsR );
    void SetVelocityInTicks( int32_t left, int32_t right );
    void GetVelocityInTicks( int32_t* left, int32_t* right );

    void Move( uint8_t chan, float meters );
    void Move( float metersL, float metersR );

    void SetPosition( uint8_t chan, float meters );
    void SetPosition( float metersL, float metersR );
    void SetActualPositionInTicks( int32_t left, int32_t right );
    void SetActualPosition( float left, float right );
    void GetPositionInTicks( int32_t* left, int32_t* right );

    void SetAccelerationProfile();
    void StopRobot();
    int  GetAnalogSensor(int s, short * val );
    void GetDigitalIn( unsigned short* digIn );
    void SetDigitalOut( unsigned short digOut );
    void SetOdometry( player_position2d_set_odom_req_t* od );
    void SetContourMode( ProfileMode_t prof );
    void SetMicrosteps();

      
    /* Conversions */
    int32_t Meters2Ticks( float meters );
    float Ticks2Meters( int32_t ticks );
    int32_t MPS2Vel( float mps );
    float Vel2MPS( int32_t vel );

 // Private Data members
 private:
    // Comm info for connection to M3 controller
    struct termios     _old_tio;
    bool               _tioChanged;

    int                _fd;
    bool               _fd_blocking;
    const char*        _serial_port; // name of serial port device
    int                _baud;

    player_data_t    _data;

    player_devaddr_t position_id;
    player_devaddr_t ir_id;
    player_devaddr_t aio_id;
    player_devaddr_t dio_id;

    // bookkeeping to track whether an interface has been subscribed
    int position_subscriptions;
    int ir_subscriptions;
    int aio_subscriptions;
    int dio_subscriptions;

    int param_idx;  // index in the RobotParams table for this robot
    int direct_wheel_vel_control; // false -> separate trans and rot vel

    player_position2d_cmd_vel_t last_position_cmd;

    // Max motor speeds (mm/sec,deg/sec)
    int motor_max_speed;
    int motor_max_turnspeed;

    // Max motor accel/decel (mm/sec/sec, deg/sec/sec)
    short motor_max_trans_accel, motor_max_trans_decel;
    short motor_max_rot_accel, motor_max_rot_decel;

    // Geometry
    // Robot Geometry
    player_position2d_geom_t  _robot2d_geom;
    player_position3d_geom_t  _robot3d_geom;
    player_ir_pose_t          _ir_geom;

    // Odometry stuff
    int32_t last_lpos;
    int32_t last_rpos;
    double  _x;
    double  _y;
    double  _yaw;

    // State
    bool    _stopped;
    bool    _motorsEnabled;
    int     _debug;
    int     _usCycleTime;
    double  _velocityK;
    double  _positionK;
    int     _percentTorque;

    uint16_t _lastDigOut;
};


#endif
