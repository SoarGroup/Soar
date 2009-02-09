/** @ingroup drivers */
/** @{ */
/** @defgroup driver_mbicp mbicp
 * @brief ScanMatching

This driver implements the metric-based ICP scan-matching algorithm.

J. Minguez, L. Montesano, and F. Lamiraux, "Metric-based iterative
closest point scan matching for sensor displacement estimation," IEEE
Transactions on Robotics, vol. 22, no. 5, pp. 1047 \u2013 1054, 2006. 

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- @ref interface_position2d : source of pose and velocity information
- @ref interface_laser : Pose-stamped laser scans (subtype 
PLAYER_LASER_DATA_SCANPOSE)

@par Configuration requests

- none

@par Configuration file options

- max_laser_range (float)
  - Default: 7.9 m    
  - Maximum laser range.

- laserPose_x (float)
  - Default: 0.16 m    
  - Offset of the laser on the edge x (in the robot's system of reference).

- laserPose_y (float)
  - Default: 0.0 m    
  - Offset of the laser on the edge y (in the robot's system of reference).
  
- laserPose_th (float)
  - Default: 0.0 rad    
  - Offset of the laser on th (in the robot's system of reference).

- radial_window	(float)
  - Default: 0.3 m    
  - Maximum distance difference between points of different scans. Points 
    with greater Br cannot be correspondent (eliminate spurius asoc.).
 
- angular_window (float)
  - Default: 0.523333333 rad
  - Maximum angle diference between points of different scans. Points 
    with greater Bw cannot be correspondent (eliminate spurius asoc.). 
  
- L (float)
  - Default: 3.00   
  - Value of the metric. When L tends to infinity you are using the 
    standart ICP. When L tends to 0 you use the metric (more importance 
    to rotation), when L tends to infinity you are using Euclidian metric.

- laserStep (integer)
  - Default: 1   
  - Selects points of each scan with an step laserStep.
    When laserStep=1 uses all the points of the scans
    When laserStep=2 uses one each two ... 
    This is an speed up parameter. 
    
- MaxDistInter (float)
  - Default: 0.5 m   
  - Maximum distance to interpolate between points in the ref scan. Consecutive 
    points with less Euclidean distance than MaxDistInter are considered 
    to be a segment.

- filter (float)
  - Default: 0.95   
  - In [0,1] sets the % of asociations NOT considered spurious. E.g. if 
    filter=0.9 you use 90% of the associations. The associations 
    are ordered by distance and the (1-filter) with greater distance 
    are not used. This type of filtering is called "trimmed-ICP".
    
- ProjectionFilter (int)
  - Default: 1   
  - Eliminate the points that cannot be seen given the two scans 
    (see Lu&Millios 97). It works well for angles < 45 deg. 
    1 : activates the filter.
    0 : desactivates the filter.

- AsocError (float)
  - Default: 0.1   
  - In [0,1]. Sets the % of minimun associations to run the algorithm.
    One way to check if the algorithm diverges is to supervise 
    if the number of associatios goes below a thresold. When the number 
    of associations is below AsocError, the main function will return
    error in associations step.
 
- MaxIter (int)
  - Default: 50   
  - Sets the maximum number of iterations for the algorithm to exit. The 
    more iterations, the more chance you give the algorithm to be more accurate.

- errorRatio (float)
  - Default: 0.0001 m
  - In [0,1] sets the maximum error ratio between iterations to exit. In 
    iteration K, let be errorK the residual of the minimization. 
    Error_th=(errorK-1/errorK). When error_th tends to 1 more precise is 
    the solution of the scan matching.

- IterSmoothConv (int)
  - Default: 2   
  - Number of consecutive iterations that satisfity the error criteria 
    (the two above criteria) (error_th) OR (errorx_out && errory_out && errt_out).
    With this parameter >1 avoids random solutions and estabilices the algorithm. 

- errx_out (float)
  - Default: 0.0001 m   
  - Minimum error in x of the asociations to exit. In each iteration, the error 
    is the residual of the minimization in each component. The condition is 
    (errorKx<errx_out && errorKx<erry_out && errorKx<errt_out). When errorK 
    tends to 0 the more precise is the solution of the scan matching
    
- erry_out (float)
  - Default: 0.0001 m   
  - Minimum error in x of the asociations to exit. In each iteration, the error 
    is the residual of the minimization in each component. The condition is 
    (errorKx<errx_out && errorKx<erry_out && errorKx<errt_out). When errorK 
    tends to 0 the more precise is the solution of the scan matching

- errt_out (float)
  - Default: 0.0001 m   
  - Minimum error in x of the asociations to exit. In each iteration, the error 
    is the residual of the minimization in each component. The condition is 
    (errorKx<errx_out && errorKx<erry_out && errorKx<errt_out). When errorK 
    tends to 0 the more precise is the solution of the scan matching
    
@par Example

@verbatim
driver
(
  name "mbicp"
  provides ["position2d:1"]
  requires ["position2d:0" "laser:1"]

  max_laser_range		7.9
  laserPose_x			0.16
  laserPose_y			0
  laserPose_th			0
  
  radial_window  	       	0.3    
  angular_window 	       	0.523333333	
  
  L		       		3.00    
  laserStep	       		1       
  MaxDistInter	       		0.5     
  filter 	       		0.95    
  ProjectionFilter	       	1       
  AsocError	      		0.1     
  MaxIter	       		50      
  
  errorRatio	       		0.0001  
  errx_out	       		0.0001  
  erry_out	       		0.0001  
  errt_out	       		0.0001  
  IterSmoothConv 	       	2       
)

@endverbatim

@author Javier Minguez (underlying algorithm)
*/
/** @} */


#include <libplayercore/playercore.h>

#include "calcul.h"
#include "MbICP.h"

#define LASER_MAX_SAMPLES 1024

class mbicp : public Driver
{

public:
    
    mbicp( ConfigFile* cf, int section);
    virtual ~mbicp();

    virtual int Setup();
    virtual int Shutdown();

    virtual int ProcessMessage(QueuePointer &resp_queue,
                               player_msghdr * hdr,
                               void * data);
private:

	float	max_laser_range;
	float	Bw;
	float	Br;
	float	L;
	int	laserStep;
	float	MaxDistInter;
	float	filter;
	int	ProjectionFilter;
	float	AsocError;
	int	MaxIter;
	float	errorRatio;
	float	errx_out;
	float	erry_out;
	float	errt_out;
	int	IterSmoothConv;
 	Tsc	laserPoseTsc;
 
	player_pose2d_t		lastPoseOdom,
				currentPose,
    				previousPose,
    				scanmatchingPose;

	player_laser_data_t	currentScan,
    				previousScan;
				
	bool		havePrevious;

	//Compute scanMatching
	void compute();

	//Transform structures between player and Tdata
	Tsc		playerPose2Tsc(player_pose2d_t posicion);
	player_pose2d_t 	Tsc2playerPose(Tsc posicion);
	void 		playerLaser2Tpfp(player_laser_data_t laserData,Tpfp *laserDataTpfp);

 	// Main function for device thread.
    	virtual void Main();

    	int SetupDevice();
    	int ShutdownDevice();

    	// Odometry.
    	void ProcessOdom(player_msghdr_t* hdr, player_position2d_data_t &data);

    	// SubtypeLaser
    	void ProcessSubtypeLaser(player_msghdr_t* hdr, player_laser_data_scanpose_t &data);

    	// Check for new commands from server
    	void ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_pos_t &);

    	// Setup ScanMatching
    	void setupScanMatching();

    	// Position
    	player_devaddr_t posicion_addr;

    	// Odometry and laser
    	Device *odom;
    	player_devaddr_t odom_addr;

    	Device *laser;
    	player_devaddr_t laser_addr;

};

////////////////////////////////////////////////////////////////////////////////
Driver* mbicp_Init(ConfigFile* cf, int section){

   return((Driver*)(new mbicp(cf, section)));

}


////////////////////////////////////////////////////////////////////////////////
void mbicp_Register(DriverTable* table){

   table->AddDriver("mbicp", mbicp_Init);

}


////////////////////////////////////////////////////////////////////////////////
int mbicp::Setup(){

   havePrevious = false;

   // Initialise the underlying position device.
   if(SetupDevice() != 0)
      return -1;

   setupScanMatching();

   puts("Setup Scanmatching");
   // Start the driver thread.
   StartThread();
   return 0;

}


////////////////////////////////////////////////////////////////////////////////
void mbicp::setupScanMatching(){

Init_MbICP_ScanMatching( 
			this->max_laser_range,
			this->Bw,	 
 			this->Br,	 
			this->L,	 
			this->laserStep,
			this->MaxDistInter,
			this->filter,	 
			this->ProjectionFilter,
			this->AsocError,
			this->MaxIter,  
			this->errorRatio,
			this->errx_out, 
			this->erry_out,  
			this->errt_out,  
			this->IterSmoothConv);
}


////////////////////////////////////////////////////////////////////////////////
int mbicp::Shutdown(){
   // Stop the driver thread.
   StopThread();

   // Stop the odom device.
   ShutdownDevice();
   return 0;
}


////////////////////////////////////////////////////////////////////////////////
mbicp::mbicp( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
           PLAYER_POSITION2D_CODE){


	this->max_laser_range		=  cf->ReadFloat(section, "max_laser_range", 7.9);
	this->Bw			=  cf->ReadFloat(section, "angular_window", 1.57/3.0);
	this->Br			=  cf->ReadFloat(section, "radial_window", 0.3);
	this->L				=  cf->ReadFloat(section, "L", 3.00);
	this->laserStep			=  cf->ReadInt(section, "laserStep", 1);
	this->MaxDistInter		=  cf->ReadFloat(section, "MaxDistInter", 0.5);
	this->filter			=  cf->ReadFloat(section, "filter", 0.85);
	this->ProjectionFilter		=  cf->ReadInt(section, "ProjectionFilter", 1);
	this->AsocError			=  cf->ReadFloat(section, "AsocError", 0.1);
	this->MaxIter			=  cf->ReadInt(section, "MaxIter", 50);
	this->errorRatio		=  cf->ReadFloat(section, "errorRatio", 0.0001);
	this->errx_out			=  cf->ReadFloat(section, "errx_out", 0.0001);
	this->erry_out			=  cf->ReadFloat(section, "erry_out", 0.0001);
	this->errt_out			=  cf->ReadFloat(section, "errt_out", 0.0001);
	this->IterSmoothConv		=  cf->ReadInt(section, "IterSmoothConv", 2);
	this->laserPoseTsc.x 		=  cf->ReadFloat(section, "laserPose_x", 0.16);
  	this->laserPoseTsc.y 		=  cf->ReadFloat(section, "laserPose_y", 0);
  	this->laserPoseTsc.tita 	=  cf->ReadFloat(section, "laserPose_th", 0);


   	if(cf->ReadDeviceAddr(&(posicion_addr), section, "provides",
         	PLAYER_POSITION2D_CODE, -1, NULL) != 0){
     		this->SetError(-1);
     		return;
   	}

   	odom = NULL;
   	if(cf->ReadDeviceAddr(&odom_addr, section, "requires",
       		PLAYER_POSITION2D_CODE, -1, NULL) != 0){
      		SetError(-1);
      		return;
   	}

   	laser = NULL;
   	if(cf->ReadDeviceAddr(&laser_addr, section, "requires",
       		PLAYER_LASER_CODE, -1, NULL) != 0){
       		SetError(-1);
   	}

   	return;
}


////////////////////////////////////////////////////////////////////////////////
mbicp::~mbicp(){
   return;
}


////////////////////////////////////////////////////////////////////////////////
int mbicp::SetupDevice(){

   if(!(odom = deviceTable->GetDevice(odom_addr))){
      PLAYER_ERROR("Unable to locate suitable position device");
      return -1;
   }
   if(odom->Subscribe(InQueue) != 0){
      PLAYER_ERROR("Unable to subscribe to position device");
      return -1;
   }

   if(!(laser = deviceTable->GetDevice(laser_addr))){
      PLAYER_ERROR("Unable to locate suitable laser device");
      return -1;
   }
   if(laser->Subscribe(InQueue) != 0){
      PLAYER_ERROR("Unable to subscribe to laser device");
      return -1;
   }

   return 0;
}


////////////////////////////////////////////////////////////////////////////////
int mbicp::ShutdownDevice(){
   player_position2d_cmd_vel_t cmd;

   memset(&cmd, 0, sizeof(cmd));
   // Stop the robot (locks the motors) if the motor state is set to
   // disabled.  The P2OS driver does not respect the motor state.
   cmd.vel.px = 0;
   cmd.vel.py = 0;
   cmd.vel.pa = 0;
 
   odom->PutMsg(InQueue, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL,
                (void*)&cmd,sizeof(cmd),NULL);

   odom->Unsubscribe(InQueue);
   laser->Unsubscribe(InQueue);
   puts("Shutdown mbicp");
   return 0;
}


////////////////////////////////////////////////////////////////////////////////
void mbicp::Main(){
	while (true){
      		// Wait till we get new data
      		Wait();
      		
		// Test if we are supposed to cancel this thread.
      		pthread_testcancel();

      		// Process any pending requests.
      		ProcessMessages();
   	}
}


////////////////////////////////////////////////////////////////////////////////
int mbicp::ProcessMessage(QueuePointer &resp_queue,player_msghdr * hdr, void * data){

   	// PLAYER_LASER_DATA_SCANPOSE
   	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,PLAYER_LASER_DATA_SCANPOSE, laser_addr))
	{
      		assert(hdr->size == sizeof(player_laser_data_scanpose_t));
      		ProcessSubtypeLaser(hdr, *reinterpret_cast<player_laser_data_scanpose_t *> (data));
   	}else

   	// PLAYER_POSITION2D_DATA_STATE
   	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,PLAYER_POSITION2D_DATA_STATE, odom_addr))
	{
      		assert(hdr->size == sizeof(player_position2d_data_t));
      		ProcessOdom(hdr, *reinterpret_cast<player_position2d_data_t *> (data));
   	}else

   	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,PLAYER_POSITION2D_DATA_GEOM, odom_addr))
	{
		assert(hdr->size == sizeof(player_position2d_data_t));
      		player_msghdr_t newhdr = *hdr;
      		newhdr.addr = device_addr;
      		Publish(&newhdr, (void*)&data);
   	}else 

   	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,/*PLAYER_POSITION2D_CMD_VEL*/ -1, device_addr))
	{
      		assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
      		// make a copy of the header and change the address
      		player_msghdr_t newhdr = *hdr;
      		newhdr.addr = odom_addr;
      		odom->PutMsg(InQueue, &newhdr, (void*)data);
   	}else
	{ 
      		if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, device_addr)){
	        	// Pass the request on to the underlying position device and wait for
         		// the reply.
         		Message* msg;

         		if(!(msg = odom->Request(InQueue, hdr->type, hdr->subtype, (void*)data,hdr->size, &hdr->timestamp)))
			{
            			PLAYER_WARN1("failed to forward config request with subtype: %d\n",hdr->subtype);
            			return(-1);
         		}
         		player_msghdr_t* rephdr = msg->GetHeader();
         		void* repdata = msg->GetPayload();
         		// Copy in our address and forward the response
         		rephdr->addr = device_addr;
         		Publish(resp_queue, rephdr, repdata);
         		delete msg;
		}
   	}
   	return 0;
}

////////////////////////////////////////////////////////////////////////////////
void mbicp::ProcessOdom(player_msghdr_t* hdr,player_position2d_data_t &data)
{

	
	Tsc	outComposicion1,
		outComposicion2,
		outInversion1;
			
	Tsc	lastPoseOdomTsc,
		previousPoseTsc,
		scanmatchingPoseTsc;		


	lastPoseOdom.px = data.pos.px;
	lastPoseOdom.py = data.pos.py;
	lastPoseOdom.pa = data.pos.pa;

	if (havePrevious)
	{
		lastPoseOdomTsc 	= playerPose2Tsc(lastPoseOdom);
		previousPoseTsc 	= playerPose2Tsc(previousPose);
		scanmatchingPoseTsc	= playerPose2Tsc(scanmatchingPose);

		inversion_sis(&previousPoseTsc, &outInversion1);
		composicion_sis(&outInversion1, &lastPoseOdomTsc, &outComposicion1);
		composicion_sis(&scanmatchingPoseTsc, &outComposicion1, &outComposicion2);
	

		data.pos.px	= outComposicion2.x;
		data.pos.py	= outComposicion2.y;
		data.pos.pa	= outComposicion2.tita;


	}	
	
	player_msghdr_t newhdr = *hdr;
   	newhdr.addr = device_addr;
   	Publish(&newhdr, (void*)&data);
}


////////////////////////////////////////////////////////////////////////////////
void mbicp::ProcessSubtypeLaser(player_msghdr_t* hdr,player_laser_data_scanpose_t &data){

	lastPoseOdom.px = data.pose.px;
	lastPoseOdom.py = data.pose.py;
	lastPoseOdom.pa = data.pose.pa;

	currentPose	= lastPoseOdom;

	currentScan.min_angle		= data.scan.min_angle;		
	currentScan.max_angle		= data.scan.max_angle;	
	currentScan.resolution		= data.scan.resolution;
	currentScan.max_range		= data.scan.max_range;	
	currentScan.ranges_count	= data.scan.ranges_count;
	currentScan.intensity_count	= data.scan.intensity_count;	
	currentScan.id					= data.scan.id;
	
	for (unsigned int i=0; i < currentScan.ranges_count; i++){
		currentScan.ranges[i] = data.scan.ranges[i];
		currentScan.intensity[i] = data.scan.intensity[i];
	}

	if (havePrevious && (	currentPose.px != previousPose.px || 
				currentPose.py != previousPose.py ||
				currentPose.pa != previousPose.pa))
	{
		compute();
	}

	else if (!havePrevious)
	{
		previousScan 		= currentScan;
		previousPose		= currentPose;
		scanmatchingPose	= currentPose;
		havePrevious		= true;
	}					
}


////////////////////////////////////////////////////////////////////////////////
void mbicp::compute()
{
	Tsc	previousPoseTsc,
		currentPoseTsc,
		scanmatchingPoseTsc,
		solutionTsc;
	
	Tsc	outComposicion1,
		outComposicion2,
		outComposicion3,
		outComposicion9,
		outComposicion10,
		outComposicion11,
		outInversion1,
		outInversion4;
			
	Tpfp	previousScanTpfp[LASER_MAX_SAMPLES],
		currentScanTpfp[LASER_MAX_SAMPLES];
			
	int	salidaMbicp;		
			

	currentPoseTsc		= playerPose2Tsc(currentPose);
	previousPoseTsc 	= playerPose2Tsc(previousPose);
	scanmatchingPoseTsc 	= playerPose2Tsc(scanmatchingPose);


	composicion_sis(&previousPoseTsc, &laserPoseTsc, &outComposicion1);
	composicion_sis(&currentPoseTsc, &laserPoseTsc, &outComposicion2);
	inversion_sis(&outComposicion1, &outInversion1);
	composicion_sis(&outInversion1, &outComposicion2, &outComposicion3);

	playerLaser2Tpfp(previousScan,previousScanTpfp);
	playerLaser2Tpfp(currentScan,currentScanTpfp);

	salidaMbicp = MbICPmatcher(previousScanTpfp,currentScanTpfp,&outComposicion3, &solutionTsc);	

	if (salidaMbicp == 1){

		composicion_sis(&laserPoseTsc,&solutionTsc,&outComposicion9);
		inversion_sis(&laserPoseTsc, &outInversion4);
		composicion_sis(&outComposicion9,&outInversion4,&outComposicion10);
		composicion_sis(&scanmatchingPoseTsc, &outComposicion10, &outComposicion11);

		scanmatchingPoseTsc.x = outComposicion11.x;
		scanmatchingPoseTsc.y = outComposicion11.y;
		scanmatchingPoseTsc.tita = outComposicion11.tita;

	}
	else{

		if (salidaMbicp == 2)
			fprintf(stderr,"2 : Everything OK but reached the Maximum number of iterations\n");		
		else{
			if (salidaMbicp == -1)
				fprintf(stderr,"Failure in the association step\n");
			if (salidaMbicp == -2)
				fprintf(stderr,"Failure in the minimization step\n");
		}
		composicion_sis(&laserPoseTsc,&outComposicion3,&outComposicion9);
		inversion_sis(&laserPoseTsc, &outInversion4);
		composicion_sis(&outComposicion9,&outInversion4,&outComposicion10);
		composicion_sis(&scanmatchingPoseTsc, &outComposicion10, &outComposicion11);

		scanmatchingPoseTsc.x = outComposicion11.x;
		scanmatchingPoseTsc.y = outComposicion11.y;
		scanmatchingPoseTsc.tita = outComposicion11.tita;

	}

	scanmatchingPose	= Tsc2playerPose(scanmatchingPoseTsc);
	previousScan		= currentScan;
	previousPose		= currentPose;
}


////////////////////////////////////////////////////////////////////////////////
Tsc mbicp::playerPose2Tsc(player_pose2d_t posicion)
{
	Tsc	posicionTsc;
	
	posicionTsc.x = posicion.px;
  	posicionTsc.y = posicion.py;
  	posicionTsc.tita = posicion.pa;
  	return(posicionTsc);
}


////////////////////////////////////////////////////////////////////////////////
player_pose2d_t mbicp::Tsc2playerPose(Tsc posicion)
{
	player_pose2d_t	posicionPlayer;
	
	posicionPlayer.px = posicion.x;
  	posicionPlayer.py = posicion.y;
  	posicionPlayer.pa = posicion.tita;
  	return(posicionPlayer);
}


////////////////////////////////////////////////////////////////////////////////
void mbicp::playerLaser2Tpfp(player_laser_data_t laserData,Tpfp *laserDataTpfp)
{
	for(unsigned int i=0; i< laserData.ranges_count; i++){
		laserDataTpfp[i].r	= laserData.ranges[i];
		laserDataTpfp[i].t	= laserData.min_angle + (i*laserData.resolution);
	}
}
