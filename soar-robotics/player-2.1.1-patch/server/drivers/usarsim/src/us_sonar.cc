#include "us_sonar.h"

// Initialization function
Driver* UsSonar_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsSonar - Init\n");
  return ((Driver*) (new UsSonar(cf, section)));
}


// a driver registration function
void UsSonar_Register(DriverTable* table)
{
  table->AddDriver("us_sonar", UsSonar_Init);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
UsSonar::UsSonar(ConfigFile* cf, int section)  :Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_SONAR_CODE)
{
  fprintf(stderr,"UsSonar - Constructor\n");
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                         PLAYER_SIMULATION_CODE, -1,NULL) != 0)
    {
	 this->SetError(-1);
	 return;
    }
  strncpy(this->name, cf->ReadString(section, "sonar_name", ""), sizeof(this->name));
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsSonar::Setup()
{
  printf("us_sonar setup\n");
  fflush(stdout);
  usleep(USBOT_STARTUP_USEC);
  Device* bot_device;// bot device 
  bool dummy = false;
  int dummyInt = 0;
  
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
    return(-1);
  }
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  //alocate sonar fields
  bot->bGeoSonar.push_back(dummyInt);
  bot->bNewSonar.push_back(dummy);
  bot->bLockSonar.push_back(dummy);
  bot->sonar_name.push_back(name);
  bot->sonar.push_back((player_sonar_data_t *)calloc(1, sizeof(player_sonar_data_t)));
  bot->sonar_geom.push_back((player_sonar_geom_t *)calloc(1, sizeof(player_sonar_geom_t)));
  // say us_bot that it shall parse sonar data
  bot->devices |= US_DATA_SONAR|US_GEOM_SONAR;
  // the sonar index for this sonar (multiple sonars)
  sonar_index = bot->sonar_name.size() - 1;
  // Start the device thread
  StartThread();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
UsSonar::~UsSonar()
{
  // Nothing to do...
  return;
}

/**
 *
 */
void UsSonar::Main() {
  //printf("us_sonar main\n");
  //fflush(stdout);
  while(true) {
    pthread_testcancel();
    ProcessMessages();
    PublishNewData();
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
}
/**
 *
 */
int UsSonar::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_SONAR_REQ_GET_GEOM, 
                           this->device_addr)) {
    short count = 0;
    char* cmd = new char[USBOT_MAX_CMD_LEN];
    sprintf(cmd,"GETGEO {Type Sonar}\r\n");
    bot->AddCommand(cmd); //publishusarsim
    while (!bot->bGeoSonar[sonar_index] == true && count < USBOT_STARTUP_CONN_LIMIT) {
	 usleep(USBOT_DELAY_USEC);
	 count++;
    }
    if (!bot->bGeoSonar[sonar_index] == true) {
	 PLAYER_ERROR("PutReply() failed");
	 this->Publish(this->device_addr,
				PLAYER_MSGTYPE_RESP_NACK,
				PLAYER_SONAR_REQ_GET_GEOM,
				bot->sonar_geom[sonar_index],
				sizeof(*bot->sonar_geom[sonar_index]),NULL);
	 return -1;
    }
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_RESP_ACK,
			   PLAYER_SONAR_REQ_GET_GEOM,
			   bot->sonar_geom[sonar_index],
			   sizeof(*bot->sonar_geom[sonar_index]),NULL);
    return 0;
  }
  else {
    printf("sonar hdr: %d : %d : %d : %d \n",hdr->addr.host,
		 hdr->addr.robot,
		 hdr->addr.interf,
		 hdr->addr.index);
    printf("sonar this: %d : %d : %d : %d \n",this->device_addr.host,
		 this->device_addr.robot,
		 this->device_addr.interf,
		 this->device_addr.index);
    printf("type %d subtype %d not handled\n",hdr->type, hdr->subtype);
    fflush(stdout);
    return -1;
  }
}
////////////////////////////////////////////////////////////////////////////////
// PublishNewData
void UsSonar::PublishNewData() {
  if (bot->bNewSonar[sonar_index]) { //if new data available send it
    bot->bLockSonar[sonar_index] = true;
    CopySonarData(bot->sonar[sonar_index],&mySonarData);
    //cout<<"sonar rangesCount "<<mySonarData.ranges_count<<endl;
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_DATA,
			   PLAYER_SONAR_DATA_RANGES,
			   &mySonarData,sizeof(mySonarData),NULL);
    
    bot->bLockSonar[sonar_index] = false;
    bot->bNewSonar[sonar_index] = false;
  }
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsSonar::Shutdown()
{
  fprintf(stderr,"UsSonar - Shutdown\n");
  // free data for bot
  //bot->devices &= ~(US_DATA_SONAR|US_GEOM_SONAR);
  //free(bot->sonar);
  //free(bot->sonar_geom);
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// Make a (local) copy of the provided sonar data
void UsSonar::CopySonarData(player_sonar_data_t *src, player_sonar_data_t *dest)
{
	dest->ranges_count = src->ranges_count;
	// copy ranges
	for(uint i=0; i<dest->ranges_count; i++) {
		dest->ranges[i] = src->ranges[i];
	}
	/*for(int i=0; i<dest->intensity_count; i++) {
		dest->intensity[i] = src->intensity[i];
	}*/
}
