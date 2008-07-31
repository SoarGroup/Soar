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
 * $Id: clodbuster.h 4135 2007-08-23 19:58:48Z gerkey $
 *
 *   the clodbuster device.   there's a thread here that
 *   actually interacts with grasp board via the serial line.  the other
 *   "devices" communicate with this thread by putting into and getting
 *   data out of shared buffers.
 */
#ifndef _CLODBUSTERDEVICE_H
#define _CLODBUSTERDEVICE_H

#include <pthread.h>
#include <sys/time.h>

#include <libplayercore/playercore.h>

/* data for the clodbuster */
#define CLODBUSTER_CYCLETIME_USEC 50000

/* Grasp Board Command numbers */
#define SYNC 255

#define SET_SERVO_THROTTLE 0
#define SET_SERVO_FRONTSTEER 1
#define SET_SERVO_BACKSTEER 2
#define SET_SERVO_PAN 3

#define ECHO_SERVO_VALUES 64 // 0x40
#define ECHO_MAX_SERVO_LIMITS 65 //0x41
#define ECHO_MIN_SERVO_LIMITS 66 //0x42
#define ECHO_CEN_SERVO_LIMITS 67 //0x43

#define ECHO_ENCODER_COUNTS 112 // 0x70
#define ECHO_ENCODER_COUNTS_TS 113 // 0x71
#define ECHO_ADC 128 //0x80
#define READ_ID 97

#define SET_SLEEP_MODE 144 // 0x90
#define ECHO_SLEEP_MODE 145 // 0x91

#define SLEEP_MODE_ON 1
#define SLEEP_MODE_OFF 0

#define SERVO_CHANNELS 8

// I think this might be useful.. so leaving it in for the time being
/* Argument types */
#define ARGINT		0x3B	// Positive int (LSB, MSB)
#define ARGNINT		0x1B	// Negative int (LSB, MSB)
#define ARGSTR		0x2B	// String (Note: 1st byte is length!!)

#define CLODBUSTER_CONFIG_BUFFER_SIZE 256

#define DEFAULT_CLODBUSTER_PORT "/dev/ttyUSB0" // This has to be USB - serial port.

typedef struct clodbuster_encoder_data
{ 
  uint32_t time_count;
  int32_t left,right;
} __attribute__ ((packed)) clodbuster_encoder_data_t;

class PIDGains
{
 private:
     float kp, ki, kd, freq, k1, k2, k3;
     void findK()
	  {
	       float T=1.0/freq;
	       k1 = kp + .5*T*ki + kd/T;
	       k2 = -kp - 2.0*kd/T + .5*ki*T;
	       k3 = kd/T;
	       printf("Gain constants set to K1 = %f, K2 = %f, K3 = %f\n",k1,k2,k3);
	  };
 public:
     PIDGains(float kp_, float ki_, float kd_, float freq_)
	  :kp(kp_), ki(ki_), kd(kd_),freq(freq_)
	  {
	       findK();
	  };
     //~PIDGains();
     void SetKp(float k)
	  {
	       kp=k;
	       findK();
	  };
     void SetKi(float k)
	  {
	       ki=k;
	       findK();
	  };
     void SetKd(float k)
	  {
	       kd=k;
	       findK();
	  };
     void SetFreq(float f)
	  {
	       freq=f;
	       findK();
	  };
     float K1(){return(k1);};
     float K2(){return(k2);};
     float K3(){return(k3);};
};

class ClodBuster:public Driver 
{
  private:
    player_position2d_data_t position_data;
    void ResetRawPositions();
    clodbuster_encoder_data_t ReadEncoders();

    int clodbuster_fd;               // clodbuster device file descriptor
    
    // device used to communicate with GRASP IO Board
    char clodbuster_serial_port[MAX_FILENAME_SIZE]; 
   
    int kp,ki,kd;

    // did we initialize the common data segments yet?
    bool initdone;
    clodbuster_encoder_data_t encoder_offset;
    clodbuster_encoder_data_t encoder_measurement;
    clodbuster_encoder_data_t old_encoder_measurement;
    float EncV, EncOmega, EncVleft, EncVright;

    bool direct_command_control;
    unsigned char max_limits[SERVO_CHANNELS],center_limits[SERVO_CHANNELS],min_limits[SERVO_CHANNELS];
    void GetGraspBoardParams();

    // CB geometry parameters
    float WheelRadius;
    float WheelBase;
    float WheelSeparation;
    unsigned int CountsPerRev;
    float Kenc; // counts --> distance traveled

    // control parameters
    float LoopFreq;
    PIDGains *Kv, *Kw;

    void IntegrateEncoders();
    void DifferenceEncoders();

 protected:

    // Max motor speeds
    int motor_max_speed;
    int motor_max_turnspeed;
    
    // Bound the command velocities
    bool use_vel_band; 
        
  short speedDemand, turnRateDemand;
  bool newmotorspeed, newmotorturn;

  public:

    ClodBuster( ConfigFile* cf, int section);
    virtual ~ClodBuster();

    /* the main thread */
    virtual void Main();

    // Process incoming messages from clients 
    int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

    virtual int Setup();
    virtual int Shutdown();

    unsigned char SetServo(unsigned char chan, int value);
    void SetServo(unsigned char chan, unsigned char cmd);
    /*
    void CMUcamReset();
    void CMUcamTrack(int rmin=0, int rmax=0, int gmin=0,
                          int gmax=0, int bmin=0, int bmax=0);
    void CMUcamStopTracking();

    */ // don't want to have this right now.. but maybe eventually.
};


#endif
