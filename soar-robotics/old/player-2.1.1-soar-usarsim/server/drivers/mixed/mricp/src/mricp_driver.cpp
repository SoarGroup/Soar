/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Andras Szekely, Aaron Skelsey, Kent Williams
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
 * mricp_driver.cpp, v3.0 21/12/2007 
 * This is a Map Reference ICP plugin Driver for real time Map building and 
 * Localization using Iterative Closest Point laser scan matching and 
 * odom correction. Currently The driver is in stable release stage, more modifications
 * might be added later on.
 *           By Tarek Taha and Jonathan Paxman / Centre of Autonomous Systems University of Technology Sydney
 */
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <netinet/in.h>
// Player includes
#include <libplayercore/playercore.h>

#include <assert.h>
#include "icp.h"
#include "map.h"
#include "lasermodel.h"
#include "Timer.h"

using namespace std;
using namespace Geom2D;

#define MAP_IDX(mf, i, j) ((mf->map_size) * (j) + (i))// compute linear index for given map coords
#define MAP_VALID(mf, i, j) ((i >= 0) && (i < mf->map_size*2/mf->map_resolution) && (j >= 0) && (j <= mf->map_size*2/mf->map_resolution))
#define MAXLASERS 4
#define MAXRANGES 10
/** @ingroup drivers */
/** @{ */
/** @defgroup ComponentNavigator MRICP
 * @brief Map Reference ICP

This is a Map Reference ICP plugin Driver for real time Map building and 
Localization using Iterative Closest Point laser scan matching and 
odom correction. Currently The driver is in stable release stage, more modifications
might be added later on.

This driver is created to support UTS CAS mobile Platforms. This driver can be quite 
usefull in a lot of applications where odom correction or matching laser scans is 
essential. I would appreciate any feedback and recommendations that can lead to improving
the performance of this driver.

@par Compile-time dependencies

- none

@par Provides

The MRICP driver provides the following device interfaces, some of
them named:

- "Position" @ref player_interface_position
  - This interface returns odometry data.
- "Opaque"   @ref player_icp_plugin_interface 
  - This is a dummy interface supporting requests and commands to the ICP / TODO
- "Map"   @ref player_map_interface
  - This interface supports map data requests / TODO

@par Supported configuration requests

- "Position" @ref player_interface_position:
  - PLAYER_POSITION_SET_ODOM_REQ
  - PLAYER_POSITION_RESET_ODOM_REQ
  - PLAYER_POSITION_GET_GEOM_REQ
  - "Map" @ref player_interface_map:
  - PLAYER_POSITION_GET_GEOM_DATA
- @ref player_icp_plugin_interface (not clear now, i will know it later)
  - PLAYER_ICP_X_REQ
  - PLAYER_ICP_Y_REQ


@par Configuration file options

- MAXR (double)
  - Default: "7.8"
  - Maximium Laser Range 
- MINR (double)
  - Default: "0.05"
  - Minimium Laser Range
- period (double)
  - Default: "0.5"
  - Time in sec between scans to be matched.
- map_resolution (double)
  - Default: "0.05"
  - Pixel resolution in meter of the map to be build 
- map_size (double)
  - Default: 20
  - This is defined from the origin to the boundary, so is actually half the size of the 
    map PATCH, not the whole map.
- interpolate (bool)
  - Default "1"
  - 0 - Simple ICP, 1 - ICP with interpolation
- NIT (int)
  - Default "10"
  - Number of iterations for each scan-matching.
- gate1 (float)
  - Default "0.5"
  - 1st data association gate for each point in scan
- gate2 (float)
  - Default "0.05"
  - 2nd data association gate for each point in scan
- debug (bool)
  - Defult: 0 
  - Display Debug Messeges
- Log (bool)
  - Default: 0
  - Loggs the Odom Data (x,y,theta,ltics,rtics,lspeed,rspeed)
- map_path(string)
  - Default: "maps/"
  - Specifies the locations where patches and logs are to be saved
- start_in(int)
  - Default : 2
  - Delay Before starting, unit is in seconds
- robot_id(int)
  - Default : 0
  - The Robot id assigned for map identification
- number_of_laser(int)
  - Default : 1
  - The number of lasers to be used in the scan matching (index starts from 0) all lasers
    should be declared in the requires section
- playerv_debug (bool)
  - Default : 0
  - If enabled, the map occupancy will be represented by +1, 0, -1 (occupied, unoccupied,
    unknown), other wise, the probability will be scaled from 0 - 255
- laserX_ranges tuple of (int) where X is an int 
  - Default : [-90 90]
  - Determines the acceptable laser scan ranges, even number of elements should exist in 
    the tuple, smaller range should be first followed by the larger range.
    You will have to manually assign the value of X:
    eg. in ur configuration file u should have something like this for 2 lasers:
        number_of_lasers 2
        laser0_ranges [-90 -70 -50 -30 -10 90]
        laser1_ranges [-120 120]
    - this represent the following acceptable ranges:
      for Laser:0 [-90 -70] [-50 -30] [-10 90]
      for laser:1 [-120 120]
- use_max_range (float)
  - Default: 0
  - Specify if you want to use ranges more than the max to update empty spaces in
    Occupancy grid map, if it's not zero , then the range specified will be used to
    update the cells within that range only (usefull to reduce the effect of false returns)
- sparse_scans_rate (int)
  - Default: 1
  - Specifies the number of laser scan samples resolution, 1 means all beams, 2 means every
    take one every 2 and so on. (OG will not be affected by this, all samples will be use for OG)
    it reduces the CPU usage.
- use_odom (bool)
  - Default: 0
  - Specify if you want to use the underlying poisition driver's odom in laser scan correction
    the existance of an underlying position driver no longer means that u are using the odom
    automatically since it can be used for passing velocity commands.
- free_space_prob (float) between 0 and 1
  - Default: 0.4
  - The probability to be used for updating the free space , lower values will help in reducing
    the false readings effect.
- map_saving_period (float)
  - Default : 10 sec
  - Determines how long time we should wait before saving the map.
@par Example 

@verbatim
driver
(
  name "mricp"
  provides ["position2d:1" "map:0"]
  requires ["position2d:0" "laser:0"]
  number_of_lasers 1
  laser0_ranges [-120 120]
  playerv_debug 0
  period 0.2
  MAXR 3.9
  MINR 0.05
  map_resolution 0.05
  map_saving_period 5
  map_size 10
  use_max_range 4
  sparse_scans_rate 3
  map_path "logs/"
  debug 0
  alwayson 1
  log 1
)
@endverbatim

@par Authors

Tarek Taha - Centre of Autonomous Systems - University of Technology Sydney
*/
/** @} */
  /////////////////////////////////////////////////////////////
 ///                   MRICP DRIVER Class                  ///
/////////////////////////////////////////////////////////////
typedef struct laser_range
{
	int begins;
	int ends;
} laser_ranges_t;
bool is_file(string fname)
{
	//cout<<fname<<endl;
	struct stat stat_buf;
 	if (stat(fname.c_str(),&stat_buf) != 0) return false;
  	return (stat_buf.st_mode & S_IFMT) == S_IFREG;
}
bool is_directory(string fname)
{
	struct stat stat_buf;
  	if (stat(fname.c_str(),&stat_buf) != 0) return false;
  	return (stat_buf.st_mode & S_IFMT) == S_IFDIR;
}
class MrIcp : public Driver
 {
	// Must implement the following methods.
  	public :	
	    virtual int Setup();
	    virtual int Shutdown();
	    virtual int ProcessMessage(QueuePointer& resp_queue, player_msghdr * hdr, void * data);
	// Constructor
	public:  	MrIcp(ConfigFile* cf, int section); 
	// Main function for device thread.
    private:	
    		virtual void Main();
			int  HandleConfigs(QueuePointer& resp_queue,player_msghdr * hdr,void * data);
			int  HandleCommands(QueuePointer& resp_queue,player_msghdr * hdr,void * data);
			int  HandleData(QueuePointer& resp_queue, player_msghdr * hdr, void * data);			
 		 	void RefreshData();     //refreshs and sends data    
 		 	void AddToMap(vector <Point> points_to_add,Pose p); // Add points to Map
 		 	void ResetMap();		// Reset the Map and empty all the point cloud
 		 	Point TransformToGlobal(Point ,Pose p);
 		 	Pose  TransformToGlobal(Pose ,Pose p);
			Point ConvertToPixel(Point p);
			Point ConvertPixel(Point p);
 		 	int  InRange(double angle,int laser_index);
 		 	void BuildMap();
 		 	int SetupLaser(int);
			void SetupPositionDriver();
			void GenerateLocalMap(Pose pse);
			void ConnectPatches();
			mapgrid_t ComputeRangeProb(double range,bool);
			int ProcessMapInfoReq(QueuePointer& resp_queue,player_msghdr * hdr,void * data);
			int ProcessMapDataReq(QueuePointer& resp_queue,player_msghdr * hdr,void * data);
	// Position interface / IN
  	private: 	
  			player_devaddr_t          position_in_addr;
			player_position2d_geom_t  geom;
			Device  			     *position_device;
	// Position interface / OUT
  	private: 	
  			player_devaddr_t         position_out_addr;
  		 	player_position2d_data_t position_out_data;
	// Lasers interfaces
  	private: 	
			// Supports MAXLASERS Lasers
  			player_devaddr_t       laser_addr[MAXLASERS];
			player_laser_geom_t    *laser_geom;
			// Used to communicate with the laser Driver
			Device                 *laser_device[MAXLASERS]; 
	// Map interface
  	private:	
  			player_devaddr_t 	   map_addr;
  		 	player_map_data_t 	   map_data;
			player_map_info_t      map_info;
	// Variables
	public :
			FILE *file,*config_file;
			char * map_path;
			int   robot_id,scan_count,nit,nu,map_number,number_of_lasers,
				  range_count[MAXLASERS],sparse_scans_rate;
			// defines a set of ranges (max 10) for each attached laser
		 	laser_range range[MAXLASERS][MAXRANGES];
			float maxr,minr,period,map_resolution,gate1,gate2,map_size,map_saving_period,
				  use_max_range,local_map_margine;
		    float PoseX,PoseY,PoseTheta; // Laser Pose
			double px, py, pa,speed,turn_rate,delta_time,start_in,free_space_prob;
			bool log,debug,interpolate,sample_initialized,
			     playerv_debug,position_in_exists,use_odom,reset_timer,warning_misalign;
			struct timeval last_time[MAXLASERS],current_time,laser_timestamp,position_timestamp,
				   loop_start,loop_end,map_timestamp,last_delta,map_current,map_saved;
			ICP icp;
			Pose laser_pose[MAXLASERS],pose_1,pose_2,delta_pose,global_pose,global_pose_prev,
				 global_diff,relative_pose,P;
			vector<Point> laser_set,laser_set_1,laser_set_2,local_map,occ_laser_set,map_points;
			MAP *map;
			Timer delta_t_estimation;
};
Driver* MrIcp_Init(ConfigFile* cf, int section) // Create and return a new instance of this driver
{
  	return ((Driver*) (new MrIcp(cf, section)));
}

void MrIcp_Register(DriverTable* table)
{
  	table->AddDriver("mricp", MrIcp_Init);
}

MrIcp::MrIcp(ConfigFile* cf, int section)  : Driver(cf, section)
{
	char config_temp[40];
  	this->maxr =             cf->ReadFloat(section,"MAXR",7.8);
  	this->start_in =         cf->ReadFloat(section,"start_in",2);
  	this->minr =             cf->ReadFloat(section,"MINR",0.05);
  	this->period =           cf->ReadFloat(section,"period",0.1);
  	this->map_resolution =   cf->ReadFloat(section,"map_resolution",0.05); // In METERS
  	this->map_size = 	     cf->ReadFloat(section,"map_size",20);         // In METERS
  	this->nit =              cf->ReadInt  (section,"NIT",10);
  	this->robot_id =         cf->ReadInt  (section,"robot_id",0);
  	this->number_of_lasers = cf->ReadInt  (section,"number_of_lasers",1);
  	this->gate1 =            cf->ReadFloat(section,"gate1",0.5);
  	this->gate2 =            cf->ReadFloat(section,"gate2",0.05);
  	this->interpolate =      cf->ReadInt  (section, "interpolate", 1);
  	this->map_path	=(char *)cf->ReadString(section,"map_path","maps/");
	this->debug = 			 cf->ReadInt(section,"debug",0); 
	this->log =   			 cf->ReadInt(section,  "log",0); 
	this->use_odom = 		 cf->ReadInt(section,  "use_odom",0); 
	this->playerv_debug =    cf->ReadInt(section,  "playerv_debug",0);
	this->use_max_range =    cf->ReadFloat(section,  "use_max_range",0);
	this->sparse_scans_rate= cf->ReadInt(section,  "sparse_scans_rate",1);
	this->free_space_prob =  cf->ReadFloat(section,"free_space_prob",0.4);
	this->map_saving_period= cf->ReadFloat(section,"map_saving_period",10);

	if (sparse_scans_rate <= 0 )
	{
		cout <<"\nSparse Scans Rate should be positive integer > 0";
		exit(1);
	}
	if(free_space_prob < 0 || free_space_prob >1)
	{
		cout <<"\nFree space probability should be between 0 and 1";
		exit(1);
	}
	for(int k = 0; k < number_of_lasers ; k++)
	{
		sprintf(config_temp,"%s%d%s","laser",k,"_ranges");
		// Expects you to provide "laserx_ranges" (where x is the laser index) 
		// in the configuration file
		this->range_count[k] =	 cf->GetTupleCount(section,config_temp);
		if ((range_count[k]%2)!=0)
		{
			cout<<"\n ERROR: Number of tuples in the ranges for Laser:"<<k<< "should be even !!!";
			exit(1);
		}
		int index=0;
		for(int i=0;i<range_count[k];i+=2)
		{
			this->range[k][index].begins =	 cf->ReadTupleInt(section,config_temp,i ,-90);
			this->range[k][index].ends   =	 cf->ReadTupleInt(section,config_temp,i+1,90);
			if(this->range[k][index].begins > this->range[k][index].ends)
			{
				cout<<"\n ERROR: the beginning range SHOULd be less than the end range !!!";
				exit(1);				
			}
			cout<<"\n Laser:"<<k<<" Range:"<<index<<" begins:"<<range[k][index].begins;
			cout<<" ends:"<<range[k][index].ends;
			index ++;
		}
		range_count[k]/=2;
	}
    // Adding position interface
    // Do we create a robot position interface?
    if(cf->ReadDeviceAddr(&(this->position_out_addr), section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0)
    {
		if (this->AddInterface(this->position_out_addr))
	  	{
	    	this->SetError(-1);    
	    	return;
	  	}
  	  	if(this->debug)
			cout<<"\n Position out Interface Loaded";
  	}
  	// Adding MAP interface
  	// Do we create a MAP interface?
    if(cf->ReadDeviceAddr(&(this->map_addr), section, "provides", PLAYER_MAP_CODE, -1, NULL) == 0)
    {
		if (this->AddInterface(this->map_addr))
	  	{
	    	this->SetError(-1);    
	    	return;
	  	}
		if(this->debug)
			cout<<"\n MAP Interface Loaded";	  	
  	}  
  	// Adding LASER interfaces
    if(this->debug)
    	cout<<"N of Lasers:"<<number_of_lasers;
	for(int i=0; i<this->number_of_lasers;i++)
	{
  		if(cf->ReadDeviceAddr(&this->laser_addr[i], section, "requires", PLAYER_LASER_CODE,-1, NULL) == 0)
  		{
			SetupLaser(i); // Here we initialize the talk to the laser driver
			cout<<"\n LASER Interface Loaded Success index:"<<i; fflush(stdout);
	  		if(this->debug)
				cout<<"\n LASER Interface Loaded";
  		}
  		else
		{
			cout<<"\n Error Reading Laser on index:"<<i;
			fflush(stdout);
	    	this->SetError(-1);    
	    	return;
		}
	}
   // Adding position interface
   // Do we create a position interface?
	if (cf->ReadDeviceAddr(&this->position_in_addr, section, "requires",PLAYER_POSITION2D_CODE, -1, NULL) == 0)
	{
		SetupPositionDriver();
		if(this->debug)
			cout<<"\n Position IN Interface Loaded";
	}   
  	else
  	{
  		if (this->use_odom)
  		  		cout<<"\n Can't use odom when you don't have an underlying position driver !!!";
  		position_in_exists = false;
  		this->position_device = NULL;
  	}
	if(this->debug)
		cout<<"\n	--->>>Gate1 ="<<gate1<<" Gate2="<<gate2<<" NIT="<<nit<<" MAXR="<<maxr<<" MINR="<<minr<<endl;
  	return;
}

int MrIcp::Setup()
{
	char filename[40],command[40];
	printf("\n- Setting UP MRICP Plugin Driver.");
	if(!is_directory(map_path))
	{
		sprintf(command,"%s%s","mkdir ",map_path); 
		if(system(command)==-1)
		{
			perror("\n Serious Error Happened while trying to create the folder");
			exit(1);
		}
		else
			cout<<"\nFolder Created Successfully";
	}
	// allocate space for map cells
  	//assert(this->map_data = (char*)malloc(sizeof(char)*this->map_size*this->map_size));
  	for(int i=0;i<number_of_lasers;i++)
		gettimeofday(&last_time[i],NULL);
	this->global_pose.p.x = this->global_pose.p.y = this->global_pose.phi = 0;
	// what extra area(margine) in the stored map around the local map should be included in the ICP 
	this->local_map_margine = 0.5;
	this->px=0;
	this->py=0;
	this->pa=0;
	nu = 0; map_number = 1;
	gchar * g_filename=g_strdup_printf("%sMAP_PATCH0",map_path);
	this->map = new MAP(g_filename,this->map_resolution,this->map_size*2);
	this->map->CreateMap();
	this->map->ResetProb();
	sprintf(filename,"%spatch_config.txt",map_path);
	config_file = fopen(filename,"wb");
	// Initial Patch Settings	
	fprintf(config_file,"%s %.3f %.3f %.3f\n","MAP_PATCH0",0.0,0.0,0.0); 
	delta_pose.p.x=0;
	delta_pose.p.y=0;
	delta_pose.phi=0;
	reset_timer= true;
	sample_initialized = FALSE;
	if(log)
	{
        sprintf(filename,"%sicplog.txt",map_path);
		file=fopen(filename,"wb");
	}
	usleep((int)(this->start_in*1e6));
  	this->StartThread();
	return(0);
};
void MrIcp::SetupPositionDriver()
{
	Pose initial_pose={{0,0,0}};
	// Subscribe to the underlyin odometry device
	if(!(this->position_device = deviceTable->GetDevice(this->position_in_addr)))
	{
		PLAYER_ERROR("unable to locate suitable position device");
	    return ;
	}
	if(this->position_device->Subscribe(this->InQueue) != 0)
	{
		PLAYER_ERROR("unable to subscribe to position device");
	    return ;
	}
	position_in_exists = true; 
 	// Get the odometry geometry
  	Message* msg;
  	if(!(msg = this->position_device->Request(this->InQueue,PLAYER_MSGTYPE_REQ,PLAYER_POSITION2D_REQ_GET_GEOM,NULL, 0, NULL,false)) 
  	  ||(msg->GetHeader()->size != sizeof(player_position2d_geom_t)))
  	{
    	PLAYER_ERROR("failed to get geometry of underlying position device");
    	if(msg)
      		delete msg;
    	return;
  	}
  	memcpy(&geom,(player_position2d_geom_t *)msg->GetPayload(),sizeof(geom));
// 	geom = (player_position2d_geom_t *)msg->GetPayload();
//	initial_pose = GetOdomReading();
	this->px = initial_pose.p.x;
	this->py = initial_pose.p.y;
	this->pa = initial_pose.phi;
};

int MrIcp::SetupLaser(int index)
{
	// Subscribe to the Laser device
  	if (!(this->laser_device[index] = deviceTable->GetDevice(this->laser_addr[index])))
  	{
    	PLAYER_ERROR("unable to locate suitable laser device");
    	return -1;
  	}
  	if (this->laser_device[index]->Subscribe(this->InQueue) != 0)
  	{
    	PLAYER_ERROR("unable to subscribe to laser device");
    	return -1;
  	}
  	// Ask for the laser's geometry
  	Message* msg;
  	if((msg = laser_device[index]->Request(this->InQueue,PLAYER_MSGTYPE_REQ,PLAYER_LASER_REQ_GET_GEOM,NULL, 0, NULL,false)))
  	{
    	laser_geom = (player_laser_geom_t *)msg->GetPayload();
	  	// Get the laser pose relative to the robot center of Rotation
  		laser_pose[index].p.x = laser_geom->pose.px;
  		laser_pose[index].p.y = laser_geom->pose.py;
  		laser_pose[index].phi = laser_geom->pose.pyaw;
	  	//if (this->debug)
		  	cout<<"\n Laser["<<index<<"] Pose --> X="<<laser_pose[index].p.x<<" Y="<<laser_pose[index].p.y<<" Theta="<<laser_pose[index].phi;
    	delete msg;
    	return 0;
  	}
  	return -1;
};
void MrIcp::ResetMap()
{
	char filename[40];
	gchar * savefile;
	sample_initialized = FALSE;
	laser_set_1.clear();
	laser_set_2.clear();
	map_points.clear();
	//map->SavePixelBufferToFile();
	map->SavePgm();
	map->ClearData();
	sprintf(filename,"MAP_PATCH%d",map_number++);
  	savefile=g_strdup_printf("%s%s",map_path,filename);
  	map->Mapname = (char *)realloc(map->Mapname,strlen(savefile)*(sizeof(savefile[0])));
 	strcpy(map->Mapname,savefile);
	this->map->ResetProb();
	// Save Patch's Name + origin in terms of the previous Patch
	fprintf(config_file,"%s %.3f %.3f %.3f\n",filename,global_pose.p.x,global_pose.p.y,global_pose.phi);
	this->global_pose.p.x = this->global_pose.p.y = this->global_pose.phi = 0;
}
int MrIcp::Shutdown()
{
	// Stop and join the driver thread
	cout<<"\n- Shutting Down MRICP Driver - Cleaning up Mess ..\n"; fflush(stdout);
	for(int i=0;i<number_of_lasers;i++)
	{
    	this->laser_device[i]->Unsubscribe(this->InQueue);
    	this->laser_device[i] = NULL;
	}
	if(position_in_exists)
		this->position_device->Unsubscribe(this->InQueue);
    //this->map->SavePixelBufferToFile();
    this->map->SavePgm();
    delete this->map;
	cout<<"\n	--->>> MAP Buffer Deleted->"; fflush(stdout);
	this->occ_laser_set.clear();
    this->map_points.clear();
    this->laser_set_1.clear();
    this->laser_set_2.clear();
    this->local_map.clear();
	cout<<" Vectors Cleared ->"; fflush(stdout);
	if(log)
		fclose(file);
	fclose(config_file);
	cout<<" Files Closed ->"; fflush(stdout);
	//ConnectPatches();
  	this->StopThread();
	cout<<" Thread Killed ->"; fflush(stdout);
	cout<<" ... ShutDown FINISED\n"; fflush(stdout);
	return(0);
}; 
// this function will run in a separate thread
void MrIcp::Main()
{
	Timer loop_timer,map_timer,test;
	double time_elapsed;
	// Synchronously cancelable thread.
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
	
	/* To use the Timer.Synch() in a loop you will have to 
	 * reset the timer at the beginning of the loop then call
	 * the Timer.Synch(period) at the end of the loop with 
	 * period representing the synchronization time in msec,
	 * this method will check what is the total time spend in
	 * this loop and then sleep the remaining period time 
	 * (if the time taken by the loop is less than the allowed period)
	 */
	while(1) 
	{
		loop_timer.Reset();
		test.Reset();
		pthread_testcancel();     // Thread cancellation point.
	    this->ProcessMessages();
		BuildMap();		          // Launch the MrICP on two laser scans
		RefreshData();            // Update Data
		time_elapsed = loop_timer.TimeElapsed();
		if(this->debug)
			cout<<"\n Min Loop took="<<time_elapsed/1e3<<"usec";
		loop_timer.Synch(this->period*1e3);
		time_elapsed = map_timer.TimeElapsed();
		if( time_elapsed >= this->map_saving_period*1e6)
		{
		    this->map->SavePgm();
			map_timer.Reset();
		}
		time_elapsed = loop_timer.TimeElapsed();
		if(this->debug)
			cout<<"\n Time Synch="<<time_elapsed/1e3<<"usec";	
	}
	pthread_exit(NULL);
}
/*! Forwards the Messeges  from the messege queue to their
 *  specific handler
 */
int MrIcp::ProcessMessage(QueuePointer& resp_queue, player_msghdr * hdr, void * data)
{
  	// Forward the Messages
  	switch (hdr->type)
  	{
  		case PLAYER_MSGTYPE_REQ:
	    	return(this->HandleConfigs(resp_queue,hdr,data));  			
	    case PLAYER_MSGTYPE_CMD:
	    	return(this->HandleCommands(resp_queue,hdr,data));	    	
	    case PLAYER_MSGTYPE_DATA:
	    	return(this->HandleData(resp_queue,hdr,data));
	    default:
	    	return -1;
  	}
};
/*! Gets the Laser and Position Data from the underlying Devices
 *  If they are provided.
 */
int MrIcp::HandleData(QueuePointer& resp_queue, player_msghdr * hdr, void * idata)
{
  	struct timeval currtime;
  	double t1,t2, min_angle, scan_res,r,b,time_diff;
	Point p;
	//Clear the previous Laser set used for the occupance grid generation
	occ_laser_set.clear();
	laser_set.clear();
  	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,PLAYER_POSITION2D_DATA_STATE, this->position_in_addr))
  	{
  		player_position2d_data_t* data = reinterpret_cast<player_position2d_data_t*> (idata);
  		// Compute new robot pose from Underlying Position Interface.
  		P.p.x = data->pos.px;
  		P.p.y = data->pos.py;
  		P.phi = data->pos.pa;
		if(this->debug)
			cout<<"\n	--->>> Odom pose from Position:0 XYTheta=["<<P.p.x<<"]["<<P.p.y<<"]["<<P.phi<<"]";  		
		return 0;
  	}
  	for(int index=0;index<number_of_lasers;index++)
  	{
		if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,PLAYER_LASER_DATA_SCAN, this->laser_addr[index]))
		{
			gettimeofday(&currtime,NULL);
			t1 = ((double) currtime.tv_sec  + (double) currtime.tv_usec/1e6);
			t2 = ((double) last_time[index].tv_sec + (double) last_time[index].tv_usec/1e6);
		    time_diff = t1 -t2;
		  	if (time_diff < 0.1)
		  	{
		  		if(this->debug)
		  			cout<<"\n	--->>> Time Since Last Read is= "<<(t1-t2)<<" msec";
		    	continue;
		  	}
		  	this->last_time[index] = currtime;
	  		player_laser_data_t* data = reinterpret_cast<player_laser_data_t*> (idata);
	  		min_angle = data->min_angle;
	  		scan_res  = data->resolution;
	  		this->scan_count = data->ranges_count;
		    if(this->debug)
		    	cout<<"\n Scan Count="<<this->scan_count;
		  	for(int i=0;i < scan_count ;i++)
		  	{
		    	// convert to mm, then m, according to given range resolution
			    r = data->ranges[i];
		    	b = min_angle + (i * scan_res); 		    // compute bearing
		    	//cout<<"\n Bearing:"<<RTOD(b);
		    	if(InRange(b,index)!=0)
		    		continue;
			    // Transfer from Polar to Cartesian coordinates
		    	p.x = r * cos(b);
		    	p.y = r * sin(b);
		    	p.laser_index = index;
		    	// Transfer all the laser Reading into the Robot Coordinate System
		    	p = TransformToGlobal(p,laser_pose[index]);
				// Filter max ranges for alignment and use all for Occ-grid
		    	if (this->use_max_range!=0)
		    	{
			    	if (r >= this->minr && r <= this->maxr && (i%sparse_scans_rate)==0)
		  				laser_set.push_back(p);
		  			occ_laser_set.push_back(p);
		  		}
		  		else
		  		{
		    		// Use only informative data for the scan allignement and Occ-grid
			    	if (r >= this->minr && r <= this->maxr)
			    	{
			    		// Get samples according to sparse resolution
			    		if ((i%sparse_scans_rate)==0)
		  					laser_set.push_back(p);
		  				// Use All the samples for OG-Map
		  				occ_laser_set.push_back(p);
			    	}
		  		}
		  	}
			return 0;
		}
  	}
  	return -1;
};  
int MrIcp::HandleConfigs(QueuePointer &resp_queue,player_msghdr * hdr,void * data)
{
	// Handle Position REQ
	// I didn't like the stupid MessageMatch Method
	// check for position config requests
	if(
	   (hdr->type == (uint8_t)PLAYER_MSGTYPE_REQ) && (hdr->addr.host   == position_out_addr.host)   &&
	   (hdr->addr.robot  == position_out_addr.robot) &&  (hdr->addr.interf == position_out_addr.interf) &&
       (hdr->addr.index  == position_out_addr.index)
       )
    {
		switch (hdr->subtype)
		{
		case PLAYER_POSITION2D_REQ_GET_GEOM:  
			/* Return the robot geometry. */
		    if(hdr->size != 0)
		    {
		      PLAYER_WARN("Arg get robot geom is wrong size; ignoring");
		      return(-1);
			}
	  		if(position_in_exists)
	  		{
		    	this->Publish(this->position_out_addr, resp_queue,PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION2D_REQ_GET_GEOM, (void*)&geom);
			    return 0;
	  		}
	  		else // Default Value if Position:0 not found
	  		{
			    geom.pose.px = 0.3;
			    geom.pose.py = 0.0;
			    geom.pose.pyaw = 0.0;
		    	geom.size.sl = 1.2;
		    	geom.size.sw = 0.65;
		    	this->Publish(this->position_out_addr, resp_queue,PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION2D_REQ_GET_GEOM,
			    			  (void*)&geom, sizeof(geom), NULL);
			    return 0;
	  		}
			return -1;
		case PLAYER_POSITION2D_REQ_SET_ODOM:
		    if(hdr->size != sizeof(player_position2d_set_odom_req_t))
		    {
		      puts("Arg get robot geom is wrong size; ignoring");
		      return(-1);
			}
	    	player_position2d_set_odom_req_t * set_odom_req;
	    	set_odom_req = (player_position2d_set_odom_req_t*) data;
	    	this->Publish(this->position_in_addr, resp_queue,PLAYER_MSGTYPE_REQ,PLAYER_POSITION2D_REQ_SET_ODOM,
		    			  (void*)set_odom_req, sizeof(set_odom_req), NULL);
			break;
		case PLAYER_POSITION2D_REQ_RESET_ODOM:
		    	/* reset position to 0,0,0: no args */
			    if(hdr->size != 0)
			    {
			      PLAYER_WARN("Arg to reset position request is wrong size; ignoring");
			      return(-1);
			    }
				else
				ResetMap();
			    this->Publish(this->position_out_addr, resp_queue,PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
			    return(0);
		case PLAYER_POSITION2D_REQ_MOTOR_POWER:
			    /* motor state change request
			     *   1 = enable motors
			     *   0 = disable motors (default)
			     */
			    if(hdr->size != sizeof(player_position2d_power_config_t))
			    {
			      PLAYER_WARN("Arg to motor state change request wrong size; ignoring");
			      return(-1);
			    }
		    	this->Publish(this->position_in_addr, resp_queue,PLAYER_MSGTYPE_REQ,PLAYER_POSITION2D_REQ_SET_ODOM,
		    			  (void*)data, sizeof(data), NULL);
			    
	        	break;
		default:
				PLAYER_WARN("\nreceived unknown config type ");
	    		return -1;
		}
  	}
	// check for MAP config requests
	if(
	   (hdr->type == (uint8_t)PLAYER_MSGTYPE_REQ) && (hdr->addr.host   == map_addr.host)   &&
	   (hdr->addr.robot  == map_addr.robot) &&  (hdr->addr.interf == map_addr.interf) &&
       (hdr->addr.index  == map_addr.index)
       )
    {
    	switch(hdr->subtype)
  		{ 
    		case PLAYER_MAP_REQ_GET_INFO:
      			return ProcessMapInfoReq(resp_queue,hdr,data);
		    case PLAYER_MAP_REQ_GET_DATA:
		      	return ProcessMapDataReq(resp_queue,hdr,data);
		    default:
		    	return -1;
  		}
  	}
  	return -1;
}
int  MrIcp::HandleCommands(QueuePointer& resp_queue,player_msghdr * hdr,void * data)
{
  	if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,PLAYER_POSITION2D_CMD_VEL,this->position_out_addr))
  	{
	 	if (!this->position_in_exists)
	 	{
	  		cout<<"\n	--->>>No Input Position Driver Initialized";
	  		fflush(stdout);	 		
	  		return -1;
	 	}
	 	player_position2d_cmd_vel_t* cmd = reinterpret_cast<player_position2d_cmd_vel_t *> (data);
	 	// cout<<"\n	--->>Me Strong, ME handle commands :)";
        this->position_device->PutMsg(this->InQueue,
                             PLAYER_MSGTYPE_CMD,
                             PLAYER_POSITION2D_CMD_VEL,
                             (void*)cmd,sizeof(cmd),NULL);    	
		return 0;
	}
  	return -1;
}
// Handle map info request
int MrIcp::ProcessMapInfoReq(QueuePointer& resp_queue,player_msghdr * hdr,void * data)
{
  	// Is it a request for map meta-data?
	cout<<"\n Processing Map Info request!!!"; fflush(stdout);
  	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_MAP_REQ_GET_INFO,this->device_addr) && this->map->occ_grid)
  	{
  		cout<<"\n Processing Map Info request Inside!!!"; fflush(stdout);
    	if(hdr->size != 0)
    	{
	      	PLAYER_ERROR2("request is wrong length (%d != %d); ignoring",hdr->size, sizeof(player_laser_config_t));
	      	return(-1);
    	}
    	map_info.scale = this->map_resolution;
    	map_info.width = ((uint32_t) (int)ceil(2*map_size/map_resolution));
    	map_info.height =((uint32_t) (int)ceil(2*map_size/map_resolution + 1));
	    // Did the user specify an origin?
    	map_info.origin.px = -map_size;
    	map_info.origin.py = -map_size;
    	map_info.origin.pa = 0.0;
	    this->Publish(this->map_addr, resp_queue,PLAYER_MSGTYPE_RESP_ACK,PLAYER_MAP_REQ_GET_INFO,
	                  (void*)&map_info, sizeof(map_info), NULL);
	    return(0);
  }
  return -1;
}

// Handle map data request
int MrIcp::ProcessMapDataReq(QueuePointer& resp_queue,player_msghdr * hdr,void * data)
{
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_MAP_REQ_GET_DATA,
                           this->device_addr))
  	{
    	player_map_data_t* mapreq = (player_map_data_t*)data;

	    player_map_data_t mapresp ;
   
	    int i, j;
	    int oi, oj, si, sj,last_row = map->size_y -1;;
		int16_t temp;
	  	double prob;
	    // Construct reply
	    oi = mapresp.col = mapreq->col;
	    oj = mapresp.row = mapreq->row;
	    si = mapresp.width = mapreq->width;
	    sj = mapresp.height = mapreq->height;
		long int tile_size = si*sj;
	    mapresp.data = (int8_t*)calloc(1,tile_size);
	    
  		// Grab the Information from the occupancy data
	  	for(j = oj; j < (sj+oj); j++)
	  	{
	  		// Proccess Last Row with the patch data
	  		if(j == last_row)
	  		{
	  			if(this->playerv_debug)
	  				continue;
	  			// Saving Creation Map Time Stamp 
	 			gettimeofday(&map_timestamp,NULL);
				// Storing Map ID can be between -127 and 127 (assumed +ve all the time)
				mapresp.data[0 + (j-oj) * si] = map_number;
				// Storing Robot ID can be between -127 and 127 (assumed +ve all the time)			
				mapresp.data[1 + (j-oj) * si] = robot_id;
				/* Storing Pose X */
				int8_t* offset = mapresp.data + (j-oj)*si+2; 
				temp = (int)(global_pose.p.x*1e3);
				memcpy(offset,&temp, sizeof(temp));
				offset += sizeof(temp);   
				// Storing Pose Y 
				temp = (int)(global_pose.p.y*1e3);
				memcpy(offset,&temp, sizeof(temp));
				offset += sizeof(temp);   
				// Storing Pose Phi 
				temp = (int)(global_pose.phi*1e3);
				memcpy(offset,&temp, sizeof(temp));
				offset += sizeof(temp);   
				/* Encapsulating Map time stamp
				 * Timeval consists of two value, tv_sec and tv_usec both of
				 * long unsigned int unit32_t (4 bytes), representing time since
				 * the epoch.
				 */
				uint32_t time_temp;
				// Seconds
				time_temp = map_timestamp.tv_sec;
				memcpy(offset,&time_temp, sizeof(time_temp));
				offset += sizeof(time_temp);   
				// usec
				time_temp = map_timestamp.tv_usec;
				memcpy(offset,&time_temp, sizeof(time_temp));
				offset += sizeof(time_temp);   
				/* The Rest of the Row is ignored for now, more data will
				 * be encapsulated later on
				 */
	  			continue;
	  		}
	    	for(i= oi; i < (si+oi); i++)
	    	{
	      		if(((i-oi) * (j-oj)) <= tile_size )
	      		{
	        		if(MAP_VALID(this, i, j))
	        		{
	       				prob = map->occ_grid[i][j].prob_occ;
	       				//cout<<"\n prob ="<<prob<<" found occupied"<<found_occ;
	       				if(this->playerv_debug)
	       				{
		        			if(prob > 0.9)
		        				mapresp.data[(i-oi) + (j-oj) * si] = +1;
		        			else if(prob < 0.1)
		        				mapresp.data[(i-oi) + (j-oj) * si] = -1;
		        			else
		        				mapresp.data[(i-oi) + (j-oj) * si] =  0;
	       				}
	       				else
	       				{
	       					uint8_t value = (uint8_t)(double(255)*prob);
	       					memcpy(mapresp.data + (i-oi) + (j-oj)*si,&value, sizeof(value));
	       				}
	        		}
	        		else
	        		{
	          			PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
	          			mapresp.data[i + j * si] = 0;
		        		cout<<"\nData Sent";fflush(stdout);
	        		}
	      		}
	     		else
	      		{
	        		PLAYER_WARN("requested tile is too large; truncating");
	        		cout<<"\nMap Too Large";fflush(stdout);
	        		if(i == 0)
	        		{
	          			mapresp.width = (si-1);
	          			mapresp.height = (j-1);
	        		}
	        		else
	       			{
	          			mapresp.width = (i);
	          			mapresp.height = (j);
	        		}
	      		}
	    	}
	  }
       	mapresp.data_count = mapresp.width * mapresp.height;
    	cout<<"\n	--->>> Columns="<<oi<<" Rows="<<oj<<" width="<<si<<" height="<<sj;
       	this->Publish(this->device_addr, resp_queue,PLAYER_MSGTYPE_RESP_ACK,PLAYER_MAP_REQ_GET_DATA,(void*)(&mapresp), sizeof(mapresp), NULL);
		free(mapresp.data);
       	return(0);
	}	
  	return -1;
};

void MrIcp::RefreshData()
{
  	// Write position data//
	this->position_out_data.pos.px = this->px; 
	this->position_out_data.pos.py = this->py; 
	this->position_out_data.pos.pa = this->pa; 
	this->position_out_data.vel.px = this->speed;
 	this->position_out_data.vel.py = this->turn_rate;
    Publish(this->position_out_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,(void*)&position_out_data); 
	return;
};
// Check if the laser beam is in the allowed range
// This is useful if you have something or part of the robot obstructing the field of view
// of the laser
int MrIcp::InRange(double angle,int laser_index)
{
	for(int i=0;i<range_count[laser_index];i++)
	{
		if(range[laser_index][i].begins <= RTOD(angle) && RTOD(angle) <= range[laser_index][i].ends)
			return 0;
	}
	return -1; // Beam to be ignored
};
Point MrIcp::TransformToGlobal(Point p,Pose pose)
{
	// Rotate + Translate
	Point temp = p;
	p.x = temp.x*cos(pose.phi) - temp.y*sin(pose.phi) + pose.p.x ;
	p.y = temp.x*sin(pose.phi) + temp.y*cos(pose.phi) + pose.p.y ;
	return p;
};
Pose MrIcp::TransformToGlobal(Pose p,Pose pose)
{
	// Rotate + Translate
	Point temp = p.p;
	p.p.x = temp.x*cos(pose.phi) - temp.y*sin(pose.phi) + pose.p.x ;
	p.p.y = temp.x*sin(pose.phi) + temp.y*cos(pose.phi) + pose.p.y ;
	p.phi = NORMALIZE(p.phi + pose.phi);
	return p;
};
// transfers from Pixel to the Map coordinate
Point MrIcp :: ConvertPixel(Point  p) 
{
	p.x = ( p.x*this->map_resolution - this->map_size) ;
	p.y = (-p.y*this->map_resolution + this->map_size) ;
	return p;
};
// transfers from Map into the Pixel Coordinate 
Point MrIcp :: ConvertToPixel(Point p) 
{
	//  This is a NxN Map with N = 2*map_size
	p.x = rint (( p.x + this->map_size)/this->map_resolution);
	p.y = rint ((-p.y + this->map_size)/this->map_resolution);
	return p;
};
mapgrid_t MrIcp::ComputeRangeProb(double range,bool free)
{
	double bad_range=0.;
	mapgrid_t prob;
	// 2 , 3 or 5 cm error based on the laser specifications
	// for the hokouo this should be modified to a constant of 2 cm
	if (range <=2)
		bad_range = 0.02;
	else if (range>2 && range<=4)
		bad_range = 0.03;
	else if (range>4)
		bad_range = 0.05;
	if (free)
	{
		prob.prob_free = range /(range + bad_range); 
		prob.prob_occ  =  1 - prob.prob_free;
	}
	else
	{
		prob.prob_occ  = range /(range + bad_range);
		prob.prob_free = 1 - prob.prob_occ; 
	}
	if(range == 0)
	{
		prob.prob_free = 1;
		prob.prob_occ  = 0;
	}
	return prob;	
}
void MrIcp::AddToMap(vector<Point> laser_data,Pose pose)
{
	Point p,pixel_point,d,in_p;
	Pose relative_laser_pose;
	double dist,color_prob,gradient;
	double x_free,x_occ,x_unknown,range,normalizer;
  	mapgrid_t sensor_prob;
	int steps;
	for(unsigned int i=0;i<laser_data.size();i++)
	{
		p = TransformToGlobal(laser_data[i],pose);
		pixel_point = ConvertToPixel(p);
		if(pixel_point.x > (map_size*2.0/map_resolution)-1 || pixel_point.y >(2.0*map_size/map_resolution) - 2 // -2 because the last row will hold the meta data
		|| pixel_point.x < 0 || pixel_point.y < 0)
		{
			//cout<<"\n	--->>> Map Size Limitations Exceeded Creating New Patch <<<---";
			cout<<"\n	--->>> Map Size Limitations , New Data Ignored <<<---";
			//ResetMap();
			fflush(stdout);
			return;
		}
		// Round the pose to the closest resolution
//		p.x = map_resolution*rint(p.x/map_resolution);
//		p.y = map_resolution*rint(p.y/map_resolution);
		// line parameters
		relative_laser_pose = TransformToGlobal(laser_pose[laser_data[i].laser_index],pose);
		d.x = p.x - relative_laser_pose.p.x; // Ray distance X in meters
		d.y = p.y - relative_laser_pose.p.y; // Ray distance Y in meters
		// Ray Total length meters
		dist = sqrt(d.x * d.x + d.y * d.y);
		steps = (int) floor(dist / map_resolution) + 1;
		d.x /= (steps);
		d.y /= (steps);
		// Traverse the line segment and update the grid
		Point temp;
		// The free Cells
		for (int j = 0; j < steps; j++)
		{
		  in_p.x = relative_laser_pose.p.x + d.x * j;
		  in_p.y = relative_laser_pose.p.y + d.y * j;
		  temp = ConvertToPixel(in_p);
		  // Prior Probability Knowledge
		  x_free = this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_free;
		  x_occ  = this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ;
		  x_unknown = 1 - (x_free + x_occ);
		  range = sqrt(pow(in_p.x - relative_laser_pose.p.x,2)+pow(in_p.y - relative_laser_pose.p.y,2));
		  // Range Sensor Probability
		  // Bayesian Update
		  if (dist > this->maxr)
		  {
		  	if(range > this->use_max_range)
		  		break;
		  	sensor_prob.prob_free = free_space_prob;
		  	sensor_prob.prob_occ  = (1 - free_space_prob)/2.0;
		  }
		  else
		  {
		 	sensor_prob = ComputeRangeProb(range,1);		
		  }
 	  	  normalizer  = x_free*sensor_prob.prob_free + x_occ*sensor_prob.prob_occ +
		  			    x_unknown*(1-(sensor_prob.prob_free + sensor_prob.prob_occ ));
		  this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_free = x_free*sensor_prob.prob_free/normalizer;
		  this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ  = x_occ *sensor_prob.prob_occ/normalizer;	  		  
		  //cout<<"\n Prob OCC ="<<x_occ *sensor_prob.prob_occ/normalizer<<" Prob Free="<<x_free*sensor_prob.prob_free/normalizer;
		  // Draw the Probability Gradient into the buffer
		  color_prob =  this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ;
		  if(color_prob>1 || color_prob<0)
		  {
		  	cout <<"\n WTF : UNEXPECTED Probability !!! "<< color_prob;
		  }
		  gradient = 255.0 - color_prob * 255.0;
		  map->DrawPixel((int)(gradient),(int)(gradient),(int)(gradient),(int)temp.x,(int)temp.y);  
		}
		// The end point is occupied
		if (dist < this->maxr) 
		{
		  	in_p.x = p.x;
		  	in_p.y = p.y;
		  	temp = in_p;
		  	temp = ConvertToPixel(in_p);
		  	// Prior Probability
		  	x_free = this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_free;
		  	x_occ  = this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ;
		  	x_unknown = 1 - (x_free + x_occ);
		  	range = sqrt(pow(in_p.x - relative_laser_pose.p.x,2)+pow(in_p.y - relative_laser_pose.p.y,2));
		  	// Range Sensor Probability
		  	sensor_prob = ComputeRangeProb(range,0);
		  	normalizer = x_free*sensor_prob.prob_free + x_occ*sensor_prob.prob_occ +
		  			   x_unknown*(1-(sensor_prob.prob_free + sensor_prob.prob_occ ));
		  	// Bayesian Update
		  	this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_free = x_free*sensor_prob.prob_free/normalizer;
		  	this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ  = x_occ *sensor_prob.prob_occ/normalizer;	  		  
			//cout<<"\n Prob OCC ="<<x_occ *sensor_prob.prob_occ/normalizer<<" Prob Free="<<x_free*sensor_prob.prob_free/normalizer;
		  	// Draw the Probability Gradient into the buffer
			color_prob =  this->map->occ_grid[(int)(temp.x)][(int)(temp.y)].prob_occ;
			gradient = 255.0 - color_prob * 255.0;
			//cout<<" Gradient= "<<gradient;
			map->DrawPixel((int)(gradient),(int)(gradient),(int)(gradient),(int)temp.x,(int)temp.y);  
			// Add the Point to the global set
			if (! this->map->occ_grid[(int)(pixel_point.x)][(int)(pixel_point.y)].added)	
			{
				this->map->occ_grid[(int)(pixel_point.x)][(int)(pixel_point.y)].added = true;
				this->map_points.push_back(p);
			}
		}
	}
};
// Only get the Existing Map points that are useful for Allignement
void MrIcp::GenerateLocalMap(Pose pse)
{
	double farest_laser_dist=0,dist,num_pixels;
	Point location,grid_start,temp;
	location.x = pse.p.x;
	location.y = pse.p.y;
	local_map.clear();
	for(int i=0;i<this->number_of_lasers;i++)
	{
		// Get the distance from the Robot's Origin to the Laser Position
		dist = sqrt(pow(laser_pose[i].p.x,2)+pow(laser_pose[i].p.y,2)); 
		if( dist > farest_laser_dist )
			farest_laser_dist = dist;
	}
//	for (unsigned int i=0;i<map_points.size();i++)
//	{
//		if (sqrt (pow(pse.p.x - map_points[i].x,2) + pow(pse.p.y - map_points[i].y,2)) <= (maxr + farest_laser_dist + local_map_margine))
//			local_map.push_back(map_points[i]);
//	}
	num_pixels = (farest_laser_dist + this->maxr + local_map_margine) /this->map_resolution;
	location = ConvertToPixel(location);
	grid_start.x = location.x - num_pixels; 
	if(grid_start.x < 0) 
		grid_start.x = 0;
	grid_start.y = location.y - num_pixels; 
	if(grid_start.y < 0) 
		grid_start.y = 0;
    //cout<<"\nStart grid: "<<grid_start.x<<" y:"<<grid_start.y<<" pixels:"<<num_pixels; fflush(stdout);
	for(int i= (int)(grid_start.x) ; i< (2*num_pixels + grid_start.x); i++)
		for(int j=(int)(grid_start.y);j<(2*num_pixels + grid_start.y); j++)
		{
			 // y is -2 because last row is meta data
			if(i<(map->size_x - 1)  && j<(map->size_y - 2)) 
				if (map->occ_grid[i][j].prob_occ > 0.9)
				{
					temp.x = i;
					temp.y = j;
					local_map.push_back(ConvertPixel(temp));
				}
		}
};
void MrIcp::BuildMap()     
{
//	double estimated_delta_d,estimated_delta_phi;
	this->delta_time = delta_t_estimation.TimeElapsed()/1e6;
	if(this->reset_timer)
		delta_t_estimation.Reset();
	if (!sample_initialized)
	{
		laser_set_1 = laser_set;
		if (laser_set_1.size() != 0)
			sample_initialized = TRUE;
		else
			return;
		// Read Pose if postion driver exists
		if(this->position_device)	pose_1 = P; 
		global_pose.p.x = global_pose.p.y = global_pose.phi = 0;
		AddToMap(laser_set_1,global_pose);
		gettimeofday(&last_delta,NULL);
		return;
	}
	laser_set_2 =  laser_set;
	if (laser_set_2.size() == 0 || laser_set_1.size() == 0)
		return;
	
	if(this->debug)
		cout<<"\n Laser Set 1 Size:"<<laser_set_1.size()<<" Laser Set 2 Size:"<<laser_set_2.size();
	
	// Read Pose if position driver exists
	if(this->use_odom)
	{
		pose_2 = P; 
		delta_pose.phi = NORMALIZE(pose_2.phi - pose_1.phi);
		delta_pose.p.x =  (pose_2.p.x - pose_1.p.x)*cos(pose_1.phi) + (pose_2.p.y - pose_1.p.y)*sin(pose_1.phi) ;
		delta_pose.p.y = -(pose_2.p.x - pose_1.p.x)*sin(pose_1.phi) + (pose_2.p.y - pose_1.p.y)*cos(pose_1.phi) ;
	}
	else
	{
		delta_pose.phi = 0; delta_pose.p.x = 0; delta_pose.p.y = 0;
	}
	if (this->debug)
	{
		cout<<"\n POSE 1 XYQ["<<pose_1.p.x<<"]["<<pose_1.p.y<<"]["<<pose_1.phi<<"]  ";
		cout<<" POSE 2 XYQ["<<pose_2.p.x<<"]["<<pose_2.p.y<<"]["<<pose_2.phi<<"]";
	}
	// Estimate the movement based on the last linear and angular velocity
	//estimated_delta_d  = this->delta_time * this->speed;
	//estimated_delta_phi= this->delta_time * this->turn_rate;
	//cout<<"\n Delta t:"<<delta_time<<" Estimeted d:"<<estimated_delta_d<<" Estimated Phi:"<<estimated_delta_phi;
	
	// Check what is the displacement estimated by ICP
	delta_pose = icp.align(laser_set_1,laser_set_2,delta_pose, gate1, nit, interpolate);
	if(delta_pose.p.x ==-1 && delta_pose.p.y ==-1 && delta_pose.phi==-1)
	{
		cout <<"\nWARNING: possible misalignment ICP: 1 - skipping scan";
		//laser_set_1 = laser_set_2;
		return;
	}
	global_pose = TransformToGlobal(delta_pose,global_pose);
	// Is the ICP estimation more than what we excpect ?? If yes then ignore this set
	/*if(sqrt(pow(delta_pose.p.x,2)+pow(delta_pose.p.y,2)) > (estimated_delta_d +0.1 ) || abs(delta_pose.phi) > abs(estimated_delta_phi + DTOR(5)))
	{
		cout<<"\n Delta Pose:1 x:"<<delta_pose.p.x <<" Y="<<delta_pose.p.y<<" Phi"<<delta_pose.phi;
		cout <<"\nWARNING: possible misalignment - skipping scan";
		this->reset_timer = false;
		return;
	}*/
	this->reset_timer = true;
	if (!this->map_points.size())
	{
		sample_initialized = false;
		return;
	}
	this->speed = sqrt(pow(delta_pose.p.x,2) + pow(delta_pose.p.y,2))/this->delta_time;
	this->turn_rate = delta_pose.phi/this->delta_time;	

	GenerateLocalMap(global_pose);
	global_pose = icp.align(this->local_map,laser_set_2,global_pose, gate1, nit, interpolate);
	if(global_pose.p.x ==-1 && global_pose.p.y ==-1 && global_pose.phi==-1)
	{
		cout <<"\nWARNING: possible misalignment ICP: 2 - skipping scan";
		global_pose.p.x = global_pose_prev.p.x;
		global_pose.p.y = global_pose_prev.p.y;
		global_pose.phi = global_pose_prev.phi;
		//laser_set_1 = laser_set_2;
		return;
	}
	if(this->debug)
		cout<<"\n Delta Pose:2 x:"<<global_pose.p.x<<" y:"<<global_pose.p.x<<" phi:"<<global_pose.phi;
	global_pose = icp.align(this->local_map,laser_set_2,global_pose, gate2, nit, interpolate);
	if(global_pose.p.x ==-1 && global_pose.p.y ==-1 && global_pose.phi==-1)
	{
		cout <<"\nWARNING: possible misalignment ICP: 3 - skipping scan";
		global_pose.p.x = global_pose_prev.p.x;
		global_pose.p.y = global_pose_prev.p.y;
		global_pose.phi = global_pose_prev.phi;
		//laser_set_1 = laser_set_2;
		return;
	}
	if(this->debug)
		cout<<"\n Delta Pose:3 x:"<<global_pose.p.x<<" y:"<<global_pose.p.x<<" phi:"<<global_pose.phi;

	// Serve Data to Position Interface
	this->global_pose_prev.p.x = this->px = global_pose.p.x;
	this->global_pose_prev.p.y = this->py = global_pose.p.y;
	this->global_pose_prev.phi = this->pa = NORMALIZE(global_pose.phi);
	// Use ALL the Laser data for the occupancy grid update
	AddToMap(occ_laser_set,global_pose);

	// Perform the ICP on Next Laser Scan
	laser_set_1 = laser_set_2;
	if(this->position_device)	
		pose_1 = pose_2;
};

void MrIcp::ConnectPatches()
{
	int patch_number=0;
	gchar * patch;
	patch = g_strdup_printf("%sMAP_PATCH%d.png",map_path,patch_number++);
	while(is_file(patch))
	{
		cout<<patch<<endl; fflush(stdout);
		patch = g_strdup_printf("%sMAP_PATCH%d.png",map_path,patch_number++);
	}
}
 


