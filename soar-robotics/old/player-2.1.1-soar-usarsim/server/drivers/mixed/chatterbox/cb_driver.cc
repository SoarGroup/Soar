 /*
 *  Chatterbox robot Player plugin driver
 *  Copyright (C) 2008
 *    Richard Vaughan
 *
 *  Based on Player's multidriver.cc example by Andrew Howard
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
 * Author: Richard Vaughan
 * Date: 25 January 2008
 * CVS: $Id: cb_driver.cc 4346 2008-02-07 02:23:00Z rtv $
 */

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>

extern "C" { 
#include "cb_i2c.h" // Jens's library to talk to the CB board over i2c.
}

#include <libplayercore/playercore.h>
//#include <libplayercore/player_interfaces.h>

const unsigned int IRCOUNT = 6;
const unsigned int LEDCOUNT = 5;

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class ChatterboxDriver : public Driver
{
  public:
    
    // Constructor; need that
    ChatterboxDriver(ConfigFile* cf, int section);

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();
    virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr, 
                               void * data);

  private:
    // Main function for device thread.
    virtual void Main();

    // sonar interface
    player_devaddr_t ir_addr;

    // lights interface
    player_devaddr_t blinkenlight_addr;
  
  // perform a colorful display
  void LightshowColors();
  
  // perform a Cylon display
  void LightshowCylon();
  
  // program to use to play audio files
  char* audioplayer;

  // play an audio file
  void PlayAudioFile( char* wavfile );
};


// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver* ChatterboxDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*) (new ChatterboxDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void Chatterbox_Register(DriverTable* table)
{
  table->AddDriver("chatterbox", ChatterboxDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

// plugin code

// /* need the extern to avoid C++ name-mangling  */
// extern "C"
// {
//   int player_driver_init(DriverTable* table)
//   {
//     Chatterbox_Register(table);
//     return(0);
//   }
// }


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
ChatterboxDriver::ChatterboxDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{
  puts( "Autolab Chatterbox" );

  // establish connection with the CB board
  init(); 

  // Create my IR interface
  if (cf->ReadDeviceAddr(&(this->ir_addr), 
			 section, 
                         "provides", 
			 PLAYER_IR_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  
  if (this->AddInterface(this->ir_addr))
  {
    this->SetError(-1);    
    return;
  }
  
  
  if (cf->ReadDeviceAddr(&(this->blinkenlight_addr), 
			 section, 
			 "provides", 
			 PLAYER_BLINKENLIGHT_CODE, -1, NULL) != 0)
    {
      this->SetError(-1);
      return;
    }
  
  if (this->AddInterface(this->blinkenlight_addr))
    {
      this->SetError(-1);    
      return;
    }
  

  // configure the sound player and play an audio greeting if requested
  audioplayer = strdup( cf->ReadString( section, "audioplayer", 0 ));
  char* audiogreeting = strdup( cf->ReadString( section, "audiogreeting", 0 ));
  if( audioplayer && audiogreeting )
    PlayAudioFile( audiogreeting );
  
  // play a lightshow greeting if requested
  char* lightgreeting = strdup( cf->ReadString( section, "lightgreeting", 0  ));      
  if( lightgreeting )
    {
      if( strcmp( lightgreeting, "cylon" ) == 0 )
	LightshowCylon();
      else if( strcmp( lightgreeting, "colors" ) == 0 )
	LightshowColors();
    }
}

void ChatterboxDriver::LightshowColors()
{
  unsigned long sleeptime = 100000L;
  
  // fade up and down red
  for( double u=0; u<M_PI; u+=0.3 )
    {	    
      for( int l=0; l<LEDCOUNT; l++ )
	{
	  setLed( l, sin(u)*255.0,0,0 );       
	  usleep(sleeptime);
	}
    }
  
  // fade up and down blue
  for( double u=0; u<M_PI; u+=0.3 )
    {	    
      for( int l=0; l<LEDCOUNT; l++ )
	{
	  setLed( l, 0,0,sin(u)*255.0 );       
	  usleep(sleeptime);
	}
    }

  // black
  for( uint8_t l=0; l<LEDCOUNT; l++ )
    setLed( l, 0,0,0 );

  // random green
  for( int b=0; b<50; b++ )
    {
      int now = random()%LEDCOUNT; 
      setLed( now, 0,255,0 );
      usleep(10000);
      setLed( now, 0,0,0 );
    }       

  // white
  for( uint8_t l=0; l<LEDCOUNT; l++ )
    setLed( l, 255,255,255 );
  usleep( 1000000 );

  // black
  for( uint8_t l=0; l<LEDCOUNT; l++ )
    setLed( l, 0,0,0 );
}

void ChatterboxDriver::LightshowCylon()
{
  unsigned long sleeptime = 100000L;

  for( uint8_t l=0; l<LEDCOUNT; l++ )
    setLed( l, 0,0,0 );

  for(int sweeps=0; sweeps<4; sweeps++ )
    {
      setLed( 4, 255,0,0 );
      usleep(sleeptime); 
      setLed( 4, 0,0,0 ); 
      setLed( 0, 255,0,0 );
      usleep(sleeptime); 
      setLed( 0, 0,0,0 ); 
      setLed( 1, 255,0,0 );
      usleep(sleeptime); 
      setLed( 1, 0,0,0 ); 
      setLed( 2, 255,0,0 );
      usleep(sleeptime); 
      setLed( 2, 0,0,0 ); 
      setLed( 1, 255,0,0 );
      usleep(sleeptime); 
      setLed( 1, 0,0,0 ); 
      setLed( 0, 255,0,0 );
      usleep(sleeptime); 
      setLed( 0, 0,0,0 ); 
    }

  for( uint8_t l=0; l<LEDCOUNT; l++ )
    setLed( l, 0,0,0 );
}

void ChatterboxDriver::PlayAudioFile( char* wavfile )
{
  if( ! audioplayer )
    return;
  
  char buf[256];
  snprintf( buf, 255, "%s %s &", audioplayer, wavfile ); 
  system( buf );
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int ChatterboxDriver::Setup()
{   
  puts("Chatterbox: Setup");

  // turn on the rangefinders
  enableIr( 1 ); 
    
  puts("Chatterbox: starting thread.");

  // Start the device thread; spawns a new thread and executes
  // ChatterboxDriver::Main(), which contains the main loop for the driver.
  this->StartThread();

  puts("Chatterbox: setup done.");

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int ChatterboxDriver::Shutdown()
{
  puts("Shutting down Chatterbox driver...");
  
  // turn off the LEDs
  for( unsigned int l=0; l<LEDCOUNT; l++ )    
    setLed( l, 0,0,0 );        

  // Stop and join the driver thread
  this->StopThread();

  puts("done.");

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void ChatterboxDriver::Main() 
{
  float ranges[IRCOUNT];
  float voltages[IRCOUNT];

  player_ir_data_t irdata;
  irdata.ranges_count = IRCOUNT;
  irdata.ranges = ranges;
  irdata.voltages_count = IRCOUNT;
  irdata.voltages = voltages;

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages.  Calls ProcessMessage() on each pending
    // message.
    this->ProcessMessages();

    // read ranges from the IRs    
    float d, v;
    for( unsigned int i=0; i<IRCOUNT; i++ ) 
      {
	if (readDistance(i, &d, &v) == 1) 
	  {
	    irdata.ranges[i] = d;
	    irdata.voltages[i] = v;
	  }
	else
	  {
	    irdata.ranges[i] = -1; // indicate bad data
	    irdata.voltages[i] = -1; 
	  }
      }
 
    // publish the new range data to Player to deliver to clients
    this->Publish(this->ir_addr, 
		  PLAYER_MSGTYPE_DATA, 
		  PLAYER_IR_DATA_RANGES,
		  (void*)&irdata, 
		  0, 
		  NULL);    

    // TODO - gather and publish more data?
  }
  return;
}


int ChatterboxDriver::ProcessMessage(QueuePointer & resp_queue, 
                                player_msghdr * hdr, 
                                void * data)
{
  // a bit ugly, polling for each light in turn. might be a better way to do it...
  for( unsigned int l=0; l<LEDCOUNT; l++ )
    {
      if(Message::MatchMessage(hdr, 
			       PLAYER_MSGTYPE_CMD, 
			       PLAYER_BLINKENLIGHT_CMD_COLOR, 
			       this->blinkenlight_addr ))
	{
	  player_blinkenlight_cmd_color_t *cmd = 
	    (player_blinkenlight_cmd_color_t*)data;      	
	  
	  //if( cmd->id >= 0 && cmd->id < LEDCOUNT )
	  setLed( cmd->id, cmd->color.red, cmd->color.green, cmd->color.blue );        

	  return 0; // handled OK
	}

//       if(Message::MatchMessage(hdr, 
// 			       PLAYER_MSGTYPE_CMD, 
// 			       PLAYER_BLINKENLIGHT_CMD_POWER, 
// 			       this->blinkenlight_addr ))
// 	{
// 	  player_blinkenlight_cmd_power_t *cmd = 
// 	    (player_blinkenlight_cmd_power_t*)data;      	

	  
// 	  setLed( l, cmd->color.red, cmd->color.green, cmd->color.blue );        
// 	  return 0; // handled OK
// 	}
    }
      

  if(Message::MatchMessage(hdr, 
			   PLAYER_MSGTYPE_REQ, 
			   PLAYER_IR_REQ_POSE, 
                           this->ir_addr))
    {
      // reply with the poses of the 6 sensors
      
      player_pose3d_t poses[IRCOUNT];
      bzero( poses, IRCOUNT*sizeof(player_pose3d_t));

      poses[0].px = 0;
      poses[0].py = 0;
      poses[0].pyaw = 0;

      poses[1].px = 0;
      poses[1].py = 0.076;
      poses[1].pyaw = DTOR(20);

      poses[2].px = -0.040;
      poses[2].py =   0.076;
      poses[2].pyaw = DTOR(90);

      poses[3].px = -0.085;
      poses[3].py = 0;
      poses[3].pyaw = DTOR(180);

      poses[4].px = -0.040;
      poses[4].py =  -0.076;
      poses[4].pyaw = DTOR(270);

      poses[5].px = 0;
      poses[5].py = -0.076;
      poses[5].pyaw = DTOR(340);

      
      player_ir_pose_t reply;
      reply.poses = poses;
      reply.poses_count = IRCOUNT;
      
      Publish( this->ir_addr, resp_queue, 
	       PLAYER_MSGTYPE_RESP_ACK, 
	       PLAYER_IR_REQ_POSE,
	       (void*)&reply, sizeof(reply), NULL );
      
      return(0);
    }
  
  else if( Message::MatchMessage(hdr, 
			PLAYER_MSGTYPE_REQ, 
			PLAYER_IR_REQ_POWER, 
			this->ir_addr))
    {
      puts( "TODO: handle IR power request" );
      return -1;
    }
  
  // TODO handle more messages

  //  else
  // Don't know how to handle this message.
  printf( "Warning: Chatterbox plugin doesn't support msg with type/subtype %d/%d\n",
	       hdr->type, hdr->subtype);
  return(-1);
}

