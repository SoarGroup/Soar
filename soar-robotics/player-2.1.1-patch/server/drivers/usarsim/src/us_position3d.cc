#include "us_position3d.h"
// Initialization function
Driver* UsPosition3d_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsPosition3d - Init\n");
  return ((Driver*) (new UsPosition3d(cf, section)));
}

// a driver registration function
void UsPosition3d_Register(DriverTable* table)
{
  table->AddDriver("us_position3d", UsPosition3d_Init);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
UsPosition3d::UsPosition3d(ConfigFile* cf, int section) : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION3D_CODE)
{
  printf("UsPosition3d - Constructor\n");
  // DBG("UsPosition3d - Constructor\n");
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
					PLAYER_SIMULATION_CODE, -1,NULL) != 0)
    {
	 this->SetError(-1);
	 return;
    }
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
UsPosition3d::~UsPosition3d()
{
  // Nothing to do...
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsPosition3d::Setup()
{
  printf("UsPosition3d - Setup\n");
  // Find my bot
  Device* bot_device;
  
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
    return(-1);
  }
  
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  // Prepare data for bot
  bot->position3d = (player_position3d_data_t *)calloc(1, sizeof(player_position3d_data_t));
  bot->robotGeom3d = (player_position3d_geom_t *)calloc(1, sizeof(player_position3d_geom_t));
  if(bot->robotDimensions == NULL)
    bot->robotDimensions = (player_bbox3d_t*)calloc(1, sizeof(player_bbox3d_t));
  if(bot->steeringType == NULL)
    bot->steeringType = new char[128];
  bot->devices |= US_DATA_POSITION3D|US_DATA_INU;
  // Start the device thread
  StartThread();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
int UsPosition3d::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_POSITION3D_REQ_GET_GEOM, 
                           this->device_addr)) {

    char* cmd = new char[USBOT_MAX_CMD_LEN];
    int count = 0;
    bot->devices |= US_CONF_ROBOT;
    PLAYER_MSG0(1,"POSITION REQ GEO\n");
    sprintf(cmd,"GETCONF {Type Robot}\r\n");
    bot->AddCommand(cmd);

    while (!bot->bConfRobot == true && count < USBOT_STARTUP_CONN_LIMIT) {
	 usleep(USBOT_DELAY_USEC);
	 count++;
    }
    if (!bot->bConfRobot == true) {
	 PLAYER_MSG0(1,"POSITION REQ GEO NACK\n");
	 this->Publish(this->device_addr,resp_queue, 
				PLAYER_MSGTYPE_RESP_NACK,
				PLAYER_POSITION3D_REQ_GET_GEOM,
				(void*)bot->robotGeom3d,
				sizeof(player_position3d_geom_t),NULL);
	 return -1;
    }
    PLAYER_MSG0(1,"POSITION REQ GEO ACK\n");
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION3D_REQ_GET_GEOM,
			   (void*)bot->robotGeom3d, sizeof(player_position3d_geom_t),NULL); 
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
						   PLAYER_POSITION3D_REQ_MOTOR_POWER,
						   this->device_addr)) {
    //cout<<"us_position PLAYER_MSGTYPE_REQ,PLAYER_POSITION2D_MOTOR_POWER"<<endl;
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION3D_REQ_MOTOR_POWER,
			   data, sizeof(data),NULL); 
    return 0;
  }
  else {
    printf("position hdr: %d : %d : %d : %d \n",hdr->addr.host,
		 hdr->addr.robot,
		 hdr->addr.interf,
		 hdr->addr.index);
    printf("position this: %d : %d : %d : %d \n",this->device_addr.host,
		 this->device_addr.robot,
		 this->device_addr.interf,
		 this->device_addr.index);
    printf("type %d subtype %d not handled\n",hdr->type, hdr->subtype);
    fflush(stdout);
    return -1;
  }
}
////////////////////////////////////////////////////////////////////////////////
// Main
void UsPosition3d::Main() {
  //printf("us_laser main\n");
  //fflush(stdout);
  while(true) {
    pthread_testcancel();
    ProcessMessages();
    PublishNewData();
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
}
////////////////////////////////////////////////////////////////////////////////
// PublishNewData
void UsPosition3d::PublishNewData() {
  if (bot->bNewPosition3d) { //if new data available send it
    bot->bLockPosition3d = true;
    //CopyLaserData(bot->laser[laser_index],&myLaserData);
    //cout<<"laser resolution "<<myLaserData.resolution<<endl;
    //cout<<"laser minAngle "<<myLaserData.min_angle<<endl;
    //cout<<"laser rangesCount "<<myLaserData.ranges_count<<endl;
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_DATA,
			   PLAYER_POSITION3D_DATA_STATE,
			   (void*)bot->position3d,sizeof(player_position3d_data_t),NULL);
    bot->bLockPosition3d = false;
    bot->bNewPosition3d = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsPosition3d::Shutdown()
{
  fprintf(stderr,"UsPosition3d - Shutdown\n");
  // free data for bot
  //bot->devices &= ~(US_DATA_POSITION3D)&~(US_DATA_INU);
  //free(bot->position3d);
  return 0;
}
