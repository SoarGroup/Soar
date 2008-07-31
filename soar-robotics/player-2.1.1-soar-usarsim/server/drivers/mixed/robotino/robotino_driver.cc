/*! \mainpage
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  
 *     Brian Gerkey
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

/*! \class RobotinoDriver
 * \brief Class for FESTO Robotino(R) plug-in driver
 * \par Authors: 
 * Simon Kracht and Carsten Nielsen
 * Aalborg University, Section for Automation and Control
 * \par Year:
 * 2007
 *
 * Plug-in driver for the FESTO Robotino(R) robot 
 *
 * This driver makes use of the open C++ libraries bundled with
 * the FESTO Robotino(R) robot, thus providing access to most 
 * of  the robots functionalities.
 *
 * \par Compile-time dependencies:
 *
 * - none
 *
 * \par Name:
 *
 * - robotino_driver
 *
 * \par Provides:
 *
 * - position2d
 * - bumper
 * - ir
 * - power
 * - camera
 *
 * \par Requires:
 *
 * - none
 *
 * \par Configuration requests:
 * \par
 *  \b interface_position2d
 * - PLAYER_POSITION2D_REQ_GET_GEOM
 * - PLAYER_POSITION2D_REQ_SET_ODOM
 * - PLAYER_POSITION2D_REQ_MOTOR_POWER
 *  \par
 *  \b interface_bumper :
 * - PLAYER_BUMPER_REQ_GET_GEOM
 *  \par
 *  \b interface_ir :
 * - PLAYER_IR_REQ_POSE
 *  \b interface_power :
 * - PLAYER_POSITION2D_REQ_MOTOR_POWER
 * 
 * \par Configuration file example:
 *
 * \code
 * driver
 * (
 * name "robotino_driver"
 * provides ["position2d:0" "power:0" "bumper:0" "ir:0"]
 *  
 * # Options
 * ROBOTINO_TIMEOUT_MS 5000 #default 5000
 * CYCLE_TIME_US 100000     #default 100000
 * )
 *
 * #Camera
 * driver
 * (
 *  name "camerav4l"
 *  provides ["camera:0"]
 *  port "/dev/video0"
 *  source 0
 *  size [176 144]
 *  mode "RGB888"
 * )
 *
 *
 * \endcode
 *
 */

#include <unistd.h>
#include <string.h>
#include <libplayercore/playercore.h>
#include <math.h>
#include <iostream>

// FESTO Robotino(R) open C++ libraries
#include <robotinocom.h>
#include <xtimer.h>
#include <xthread.h>

// Defines
#define PI 3.141592653
#define ROBOTINO_DIAMETER 0.44//[m]
#define ROBOTINO_RADIUS ROBOTINO_DIAMETER * 0.5//[m]
#define ROBOTINO_CIRCUMFERENCE 2 * PI * ROBOTINO_RADIUS//[m]
#define ROBOTINO_CENTRE_TO_WHEEL 0.135//[m]
#define ROBOTINO_WHEEL_RADIUS 0.04//[m]
#define PULSE_TO_RPM 1.83  //[rpm] (SEC_PR_MIN*MSEC_PR_SEC) / GEAR_RATIO / PULSES_PR_REV

//! RobotinoCom functions
void errorCb (void *data, int error)
{
  std::cout << std::endl << "Robotino(R) :: RobotinoCom Error : " << RobotinoCom::errorString (error) << std::endl;
}

void connectedCb (void *data)
{
  std::cout << std::endl << "Robotino(R) :: Connecting - DONE" << std::endl;
}

void connectionClosedCb (void *data)
{
  std::cout << std::endl << "Robotino(R) :: Connection closed" << std::endl;
}

class RobotinoDriver:public Driver
{
public:
  
  // Constructor; need that
  RobotinoDriver (ConfigFile * cf, int section);
  
  // Must implement the following methods.
  virtual int Setup ();
  virtual int Shutdown ();
  
  // This method will be invoked on each incoming message
  virtual int ProcessMessage (QueuePointer &resp_queue,
			      player_msghdr * hdr, void *data);

   // Holders for desired velocities 
  float desiredVelocityX;
  float desiredVelocityY;
  float desiredVelocityA;
	
	// Holder for position (odometry) data sent from below
	player_position2d_data_t posdata;

	// Holder for position (odometry) data sent from above 
	player_position2d_set_odom_req odomCommand;

private:
  // Main function for device thread.
  virtual void Main ();
  
  player_devaddr_t position_addr;
  player_devaddr_t power_addr;
  player_devaddr_t bumper_addr;
  player_devaddr_t ir_addr;
  
  // Robotino(R) device
  RobotinoCom com;

  // Variables from config file
  unsigned int ROBOTINO_TIMEOUT_MS;
  unsigned int CYCLE_TIME_US;

};
/// A factory creation function.
/** 
    Declared outside of the class so that it can be invoked without any object context (alternatively, you can
    declare it static in the class).  In this function, we create and return
    (as a generic Driver*) a pointer to a new instance of this driver.
*/
Driver * 
RobotinoDriver_Init (ConfigFile * cf, int section)
{
  //! Create and return a new instance of this driver
  return ((Driver *) (new RobotinoDriver (cf, section)));
}

//! A driver registration function.
/** 
    Again declared outside of the class so that it can be invoked without 
    object context.  In this function, we add the driver into the given 
    driver table, indicating which interface the driver can support and how 
    to create a driver instance. 
*/
void RobotinoDriver_Register (DriverTable * table)
{
  table->AddDriver ("robotino_driver", RobotinoDriver_Init);
}
/// Constructor

/** Retrieve options from the configuration file and do any */
RobotinoDriver::RobotinoDriver (ConfigFile * cf, int section):
  Driver (cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  // Read options from the configuration file
  this->ROBOTINO_TIMEOUT_MS = cf->ReadInt(section,"ROBOTINO_TIMEOUT_MS",0);
  this->CYCLE_TIME_US = cf->ReadInt(section, "CYCLE_TIME_US", 0);

  memset (&this->position_addr, 0, sizeof (player_devaddr_t));
  memset (&this->power_addr, 0, sizeof (player_devaddr_t));
  memset (&this->bumper_addr, 0, sizeof (player_devaddr_t));
  memset (&this->ir_addr, 0, sizeof(player_devaddr_t));
 
  // Do we create a position interface?
  if (cf->ReadDeviceAddr (& (this-> position_addr),section,"provides",PLAYER_POSITION2D_CODE,-1, NULL) == 0)
    {
      if (this->AddInterface (this->position_addr) !=0)
	{
	  this->SetError (-1); 
	  return;
	}
    }
  
  // Do we create a power interface?
  if (cf->ReadDeviceAddr (&(this->power_addr),section,"provides",PLAYER_POWER_CODE,-1, NULL) == 0)
    {
      if (this->AddInterface (this->power_addr) != 0)
	{
	  this->SetError (-1); 
	  return;
	}
    }
  
  // Do we create a bumper interface?
  if (cf->ReadDeviceAddr (&(this->bumper_addr),section,"provides",PLAYER_BUMPER_CODE, -1, NULL) == 0)
    {
      if (this->AddInterface (this->bumper_addr) != 0)
	{
	  this->SetError (-1); 
	  return;
	}
    }
  // Do we create an ir interface?
  if(cf->ReadDeviceAddr(&(this->ir_addr), section, "provides",PLAYER_IR_CODE, -1, NULL) == 0)
    {
      if(this->AddInterface(this->ir_addr) != 0)
	{
	  this->SetError(-1);    
	  return;
	}
    }
} 

/// Set up the device.
/** 
    Return 0 if things go well, and -1 otherwise. 
*/
int RobotinoDriver::Setup ()
{
  std::cout << std:: endl << "Robotino(R) :: Driver initialising" << std::endl;
  
  //! Here you do whatever is necessary to setup the device, like open and configure a serial port.
  
  XTimer timer;

  if (false == com.init ())
    {
      return -1;
    }
  
  std::cout << std::endl << "Robotino(R) :: Connecting...";
  
  com.setErrorCallback (&errorCb,NULL);
  com.setConnectedCallback (&connectedCb, NULL);
  com.setConnectionClosedCallback(&connectionClosedCb, NULL);
  
  com.connectToHost ("127.0.0.1");
 
  while (com.state () == RobotinoCom::ConnectingState)
    {
      std::cout << ".";
      XThread::msleep (200);
    }
  
  // Camera settings 
  RobotinoCom::CameraParameters param = com.cameraParameters();
  param.compression = RobotinoCom::HighCompression;
  param.resolution = RobotinoCom::QVGA;
  com.setCameraParameters( param );
  
  PLAYER_MSG0(2,"Robotino : Setup done");
  
  // If errors are encountered
  if (com.error () != RobotinoCom::NoError)
    {
      Shutdown ();
    }
  
  // Set the robot timeout, in seconds. The robot will stop if no commands are sent before the timeout expires.
  com.setTimeout (this->ROBOTINO_TIMEOUT_MS);
  std::cout << std::endl << "Robotino(R) :: Timeout set to " << this->ROBOTINO_TIMEOUT_MS;  
  std::cout << std::endl << "Robotino(R) :: Driver initializing - DONE" << std::endl;

 if (com.receiveIoStatus() == false)
	{
	  PLAYER_ERROR("Robotino(R) :: Failed to receive IO status in setup");
	}  
  //! Start the device thread; spawns a new thread and executes
  //! RobotinoDriver::Main(), which contains the main loop for the driver.
  StartThread ();

  // Initialize the holders for desired velocities
  desiredVelocityX = 0;
  desiredVelocityY = 0;
  desiredVelocityA = 0;

	// Initialize the holder for position (odometry) data
	memset(&posdata,0,sizeof(posdata));


return (0);
}


/// Shutdown the device
int RobotinoDriver::Shutdown ()
{
  std::cout << std::endl << "Robotino(R) :: Shutting driver down";
  //! Stop and join the driver thread
  StopThread ();

  //! Here you would shut the device down by, for example, closing a
  //! serial port.

  com.close ();
    
  std::cout << std::endl << "Robotino(R) :: Shutting driver down - DONE" << std::endl;
  
  return (0);
}

/// Process messages
int RobotinoDriver::
ProcessMessage (QueuePointer &resp_queue,player_msghdr * hdr,void *data)
{
  //! Send a response if necessary, using Publish().
  //! If you handle the message successfully, return 0.  Otherwise,
  //! return -1, and a NACK will be sent for you, if a response is required.
  
  // Interface - position2d
  if (Message::MatchMessage (hdr,PLAYER_MSGTYPE_CMD,
			     PLAYER_POSITION2D_CMD_VEL,
			     this->position_addr))
    {
      // Get and send the latest motor commands
      player_position2d_cmd_vel_t position_cmd;
      position_cmd = *(player_position2d_cmd_vel_t *)data;
      
      /*PLAYER_MSG3 (3,"Robotino(R) :: Player motor commands %f:%f:%f",
		   position_cmd.vel.px,
		   position_cmd.vel.py,
		   position_cmd.vel.pa);*/
      
      // Desired velocities are stored
      desiredVelocityX = position_cmd.vel.px; // [m/s]
      desiredVelocityY = position_cmd.vel.py; // [m/s]
      desiredVelocityA = position_cmd.vel.pa; // [rad/s]

      position_cmd.vel.px = position_cmd.vel.px * 1000;
      position_cmd.vel.py = position_cmd.vel.py * 1000;
      position_cmd.vel.pa = position_cmd.vel.pa * (180/PI);

      /*
      
      if (position_cmd.vel.px < 0)
	{
	  position_cmd.vel.px = position_cmd.vel.px * 5;
	}
      
      if (position_cmd.vel.py < 0)
	{
	  position_cmd.vel.py = position_cmd.vel.py * 5;
	}
      */

      com.setVelocity (position_cmd.vel.px,
		       position_cmd.vel.py,
		       position_cmd.vel.pa);
      
      // Sending the set values to Robotino and receiving the sensor readings
      if (com.receiveIoStatus() == false)
	{
	  PLAYER_ERROR("Robotino(R) :: Failed to apply motor commands");
	}
      return (0);
    }
	
	
	// Command from above to set the odometry to a particular value
  	else if (Message::MatchMessage (hdr,PLAYER_MSGTYPE_REQ,
			     PLAYER_POSITION2D_REQ_SET_ODOM,
			     this->position_addr))
    {
		this->Publish(this->position_addr, resp_queue,
		    PLAYER_MSGTYPE_RESP_ACK,
		    PLAYER_POSITION2D_REQ_SET_ODOM);
      
        return (0);
	}
	
	
	
  else if (Message::MatchMessage (hdr,
				  PLAYER_MSGTYPE_REQ,
				  PLAYER_POSITION2D_REQ_GET_GEOM,
				  this->position_addr))
    {
      player_position2d_geom_t geom;
      memset(&geom, 0, sizeof(geom));

      geom.size.sl = ROBOTINO_DIAMETER;
      geom.size.sw = ROBOTINO_DIAMETER;
      
      this->Publish (this->position_addr, 
		     resp_queue,
		     PLAYER_MSGTYPE_RESP_ACK,
		     PLAYER_POSITION2D_REQ_GET_GEOM,
		     (void *) &geom,
		     sizeof (geom),
		     NULL);
      
      return (0);
    }
  
    // Interface - bumper
    else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_BUMPER_REQ_GET_GEOM,
                                this->bumper_addr))
  {
    player_bumper_geom_t geom;
    memset(&geom, 0, sizeof(geom));

    geom.bumper_def_count = 1;
    geom.bumper_def = new player_blobfinder_blob_t;

    geom.bumper_def[0].pose.px = -ROBOTINO_RADIUS/2;
    geom.bumper_def[0].pose.py = ROBOTINO_RADIUS;
    geom.bumper_def[0].pose.pyaw = 360;
    geom.bumper_def[0].length = ROBOTINO_CIRCUMFERENCE;
    geom.bumper_def[0].radius = ROBOTINO_RADIUS;

    this->Publish(this->bumper_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_BUMPER_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);

    delete geom.bumper_def;
    
    return(0);
   }
  // Interface - ir
  else if (Message::MatchMessage (hdr,PLAYER_MSGTYPE_REQ,
				  PLAYER_IR_REQ_POSE,
				  this->ir_addr))
    {
      player_ir_pose_t geom;
      memset(&geom, 0, sizeof(geom));
      geom.poses_count = com.numDistanceSensors();
      geom.poses = new player_pose3d_t [geom.poses_count];
      
      for(unsigned int intCount = 0;intCount < geom.poses_count;intCount++)
	{
	  geom.poses[intCount].pyaw = DTOR(40*intCount);
	  geom.poses[intCount].px = cos(geom.poses[intCount].pyaw)*ROBOTINO_RADIUS;
	  geom.poses[intCount].py = sin(geom.poses[intCount].pyaw)*ROBOTINO_RADIUS;
	  
	}
      this->Publish(this->ir_addr, resp_queue,
		    PLAYER_MSGTYPE_RESP_ACK,
		    PLAYER_IR_REQ_POSE,
		    (void *) &geom,
		    sizeof(geom),
		    NULL);
      delete [] geom.poses;
      return (0);
    }
    
  // Motor power request - NOTE: no action taken since motors are turned on all the time
  else if (Message::MatchMessage (hdr,PLAYER_MSGTYPE_REQ,
				  PLAYER_POSITION2D_REQ_MOTOR_POWER,
				  this->position_addr))
    {
      this->Publish(this->position_addr, resp_queue,
		    PLAYER_MSGTYPE_RESP_ACK,
		    PLAYER_POSITION2D_REQ_MOTOR_POWER);
      
      return (0);
    }
    
  else
		PLAYER_ERROR("Robotino:: Unhandled message");
    return (-1); 
}


/// Main function for device thread
void RobotinoDriver::Main ()
{
  // unsigned int intNumberOfMotors = com.numMotors();
  // float arrActualVelocities[intNumberOfMotors];

  //! The main loop; interact with the device here
  
  XTimer timerEuler;
  
  player_ir_data_t irdata;
  memset (&irdata, 0,sizeof (irdata));
  
  player_power_data_t powerdata;
  memset(&powerdata,0,sizeof(powerdata));

  player_bumper_data_t bumperdata;
  memset(&bumperdata,0,sizeof(bumperdata));
  
 // player_position2d_data_t posdata;
 // memset(&posdata,0,sizeof(posdata));
  
  // Update position2d data
  float floatTheta = 0;
  float floatPhi1 = 60*(PI/180);
  float floatPhi2 = 180*(PI/180);
  float floatPhi3 = 300*(PI/180);
  
  // Matrix elements
  float floatA11= -sin(floatPhi1 + floatTheta);
  float floatA12= cos(floatPhi1 + floatTheta);
  float floatA13= ROBOTINO_CENTRE_TO_WHEEL;
  
  float floatA21 = -sin(floatPhi2 + floatTheta);
  float floatA22 = cos(floatPhi2 + floatTheta);
  float floatA23 = ROBOTINO_CENTRE_TO_WHEEL;
  
  float floatA31 = -sin(floatPhi3 + floatTheta);
  float floatA32 = cos(floatPhi3 + floatTheta);
  float floatA33 = ROBOTINO_CENTRE_TO_WHEEL;
  
  // Calculate determinant
  float floatInvDeterminant = 1/(floatA11*(floatA33*floatA22-floatA32*floatA23)-floatA21*(floatA33*floatA12-floatA32*floatA13)+floatA31*(floatA23*floatA12-floatA22*floatA13));
  
  while (true)     
    
    {
      // Test if we are supposed to cancel
      pthread_testcancel();
      
      // Process incoming messages
      ProcessMessages();
      
      //! Interact with the device, and push out the resulting data, using Driver::Publish()
      
      
      int intNumberOfMotors = 3;
      float arrActualVelocities[3];
      int intCount = 0;
      
      // Get motor speeds
      for (intCount = 0; intCount <= intNumberOfMotors-1; intCount++)
	{
	  arrActualVelocities[intCount]=com.actualVelocity(intCount)*PULSE_TO_RPM;
	  /* PLAYER_MSG2(2,"Robotino(R) :: Velocity (RPM) %d = %f",
		      intCount,
		      arrActualVelocities[intCount]);*/
        }
      
      // Calculate velocities
      posdata.vel.px = (PI/30)*ROBOTINO_WHEEL_RADIUS*floatInvDeterminant*((floatA33*floatA22-floatA32*floatA23)*arrActualVelocities[0]-(floatA33*floatA12-floatA32*floatA13)*arrActualVelocities[1]+(floatA23*floatA12-floatA22*floatA13)*arrActualVelocities[2]);
      posdata.vel.py = (PI/30)*ROBOTINO_WHEEL_RADIUS*floatInvDeterminant*(-(floatA33*floatA21-floatA31*floatA23)*arrActualVelocities[0]+(floatA33*floatA11-floatA31*floatA13)*arrActualVelocities[1]-(floatA23*floatA11-floatA21*floatA13)*arrActualVelocities[2]);
      posdata.vel.pa = (PI/30)*ROBOTINO_WHEEL_RADIUS*floatInvDeterminant*((floatA32*floatA21-floatA31*floatA22)*arrActualVelocities[0]-(floatA32*floatA11-floatA31*floatA12)*arrActualVelocities[1]+(floatA22*floatA11-floatA21*floatA12)*arrActualVelocities[2]);
      
      // Calculate positions
      posdata.pos.px +=  posdata.vel.px * timerEuler.msecsElapsed()/1000;
      posdata.pos.py +=  posdata.vel.py * timerEuler.msecsElapsed()/1000;
      posdata.pos.pa +=  posdata.vel.pa * timerEuler.msecsElapsed()/1000;
      posdata.stall = 0;

		if(posdata.pos.pa < -PI)
		{
			posdata.pos.pa += 2*PI;
		}
		
		else if(posdata.pos.pa >PI)
		{
			posdata.pos.pa -= 2*PI;
		}
		
 
      timerEuler.start();
     
      this->Publish(this->position_addr,
		    PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
		    (void*)&posdata,sizeof(posdata), NULL);

      
      // Update power data
      powerdata.volts = com.voltageBatt1plus2();
      powerdata.watts = com.voltageBatt1plus2() * com.current();
      powerdata.valid = (PLAYER_POWER_MASK_VOLTS |
			 PLAYER_POWER_MASK_WATTS);
      
      this->Publish(this->power_addr,
		    PLAYER_MSGTYPE_DATA, PLAYER_POWER_DATA_STATE,
		    (void*)&powerdata, sizeof(powerdata), NULL);
     
      
      // Update bumper data
      bumperdata.bumpers_count = 1;
      bumperdata.bumpers = new uint8_t;
      bumperdata.bumpers[0] = com.bumper();
      
      //PLAYER_MSG1(2,"Robotino(R) :: Bumper = %d",bumperdata.bumpers[0]);
      
      this->Publish(this->bumper_addr,
		    PLAYER_MSGTYPE_DATA, PLAYER_BUMPER_DATA_STATE,
		    (void*)&bumperdata, sizeof(bumperdata), NULL);
      delete bumperdata.bumpers;
      
      // Update IR data
      // Number of IR sensors on Robotino(R)
      irdata.ranges_count = com.numDistanceSensors();
      irdata.ranges = new double [irdata.ranges_count];
      // Collect distance measurements from all IR sensors on Robotino(R)
      for(unsigned int intCount = 0;intCount < irdata.ranges_count;intCount++)
	{
	  //! Get measured distances from each sensor
	  irdata.ranges[intCount] = com.distance(intCount+1);
	  
	  irdata.ranges[intCount] = (1024 - irdata.ranges[intCount])*0.001;	  
	  /*PLAYER_MSG2(2,"Robotino(R) :: IR sensor %d = %f",
		      intCount+1,
		      irdata.ranges[intCount]);*/
	}

      this->Publish (this->ir_addr,
		     NULL,
		     PLAYER_MSGTYPE_DATA,
		     PLAYER_IR_DATA_RANGES,
		     (void *) &irdata,
		     sizeof (irdata),
		     NULL);
      delete [] irdata.ranges;
      
      // To maintain connection due to RobotinoCom timeout  
      if((desiredVelocityX == 0) && (desiredVelocityY == 0) && (desiredVelocityA == 0))
	{
	  if (com.receiveIoStatus() == false)
	    {
	      PLAYER_ERROR("Robotino(R) :: Failed to receive IO status in setup");
	    }
	}
      
      // Sleep (you might, for example, block on a read() instead)
      usleep (this->CYCLE_TIME_US);
      }
}
      
