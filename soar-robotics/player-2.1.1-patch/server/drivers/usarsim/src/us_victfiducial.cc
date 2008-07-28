#include "us_victfiducial.h"
// Initialization function
Driver* UsVictFiducial_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsVictFiducial - Init\n");
  return ((Driver*) (new UsVictFiducial(cf, section)));
}

// a driver registration function
void UsVictFiducial_Register(DriverTable* table)
{
  table->AddDriver("us_victfiducial", UsVictFiducial_Init);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
UsVictFiducial::UsVictFiducial(ConfigFile* cf, int section)    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_FIDUCIAL_CODE)
{
  printf("UsVictFiducial - Constructor\n");
  // DBG("UsVictFiducial - Constructor\n");
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
UsVictFiducial::~UsVictFiducial()
{
  // Nothing to do...
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsVictFiducial::Setup()
{
  fprintf(stderr,"UsVictFiduical - Setup\n");

  Device* bot_device;
  
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
    return(-1);
  }
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  // Prepare data for bot
  bot->victim_fiducial = (player_victim_fiducial_data_t *)calloc(1, sizeof(player_victim_fiducial_data_t));
  bot->devices |= US_DATA_VICTIM_FIDUCIAL;
  // Start the device thread
  StartThread();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// Main
void UsVictFiducial::Main() {
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
int UsVictFiducial::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_FIDUCIAL_REQ_GET_GEOM, 
                           this->device_addr)) {
    
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_NACK,PLAYER_FIDUCIAL_REQ_GET_GEOM,
			   data, sizeof(data),NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
						  PLAYER_FIDUCIAL_REQ_GET_GEOM, 
						  this->device_addr)) {
  
    player_fiducial_geom_t geom;
    geom.pose.px = 0;
    geom.pose.py = 0;
    geom.pose.pyaw = 0;
    geom.size.sl = 0.1;
    geom.size.sw = 0.1;
    geom.fiducial_size.sl = 0.1;
    geom.fiducial_size.sw = 0.1;
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_RESP_ACK,
			   PLAYER_FIDUCIAL_REQ_GET_GEOM,
			   &geom,
			   sizeof(geom),NULL);
    return 0;
  }
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_FIDUCIAL_REQ_GET_ID , 
                           this->device_addr)) {
    
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_NACK,PLAYER_FIDUCIAL_REQ_GET_ID,
			   data, sizeof(data),NULL);
    return 0;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsVictFiducial::Shutdown()
{
  fprintf(stderr,"UsVictFiducial - Shutdown\n");
  // free data for bot
  //bot->devices &= ~(US_DATA_VICTIM_FIDUCIAL);
  //free(bot->victim_fiducial);
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// PublishNewData
void UsVictFiducial::PublishNewData(){
  if (bot->bNewVictimFiducial ) {
    //DBG("Got New Position");
    printf("us_vict_fiducial new data\n");
    fflush(stdout);
    bot->bLockVictimFiducial = true;
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_DATA,
			   PLAYER_FIDUCIAL_DATA_SCAN,
			   (void*)bot->victim_fiducial,sizeof(player_victim_fiducial_data_t),NULL);
    bot->bLockVictimFiducial = false;
    bot->bNewVictimFiducial = false;
  }
}
