#include "us_fakelocalize.h"
// Initialization function
Driver* UsFakeLocalize_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new UsFakeLocalize( cf, section)));
}
// a driver registration function
void UsFakeLocalize_Register(DriverTable* table)
{
  table->AddDriver("us_fakelocalize", UsFakeLocalize_Init);
}
////////////////////////////////////////////////////////////////////////////////
// Constructor
UsFakeLocalize::UsFakeLocalize( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  // Must have an input sim
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                       PLAYER_SIMULATION_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
  this->bot = NULL;

  origin.px = cf->ReadTupleLength(section,"origin",0,0);
  origin.py = cf->ReadTupleLength(section,"origin",1,0);
  //origin.pa = cf->ReadTupleAngle(section,"origin",2,0);
  origin.pa = 0.0;
  
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsFakeLocalize::Setup()
{
  printf("us_fakeLocalization setup\n");
  fflush(stdout);
  Device* bot_device;
  
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
    return(-1);
  }
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  bot->devices |= US_DATA_INS;
  this->StartThread();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// 
int UsFakeLocalize::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data)
{
  printf("us_fakeLocalization ProcessMessage\n");
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_POSITION2D_REQ_GET_GEOM, 
                           this->device_addr)) {
    printf("us_fakeLocalization ProcessMessage match\n");
    player_position2d_geom_t geom;
    geom.pose.px = 0.0;
    geom.pose.py = 0.0;
    geom.pose.pyaw = 0.0;
    geom.size.sw = 0.0;
    geom.size.sl = 0.0;
    //bot->position = (player_position2d_data_t *)calloc(1, sizeof(player_position2d_data_t));
    //say bot that he shall parse and send position data;
    bot->devices |= US_DATA_POSITION;
    
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION2D_REQ_GET_GEOM,
			   &geom, sizeof(geom),NULL); 
    return 0;
  }
  printf("fakelocalize hdr: %d : %d : %d : %d \n",hdr->addr.host, hdr->addr.robot, hdr->addr.interf,hdr->addr.index);
  printf("fakelocalize this: %d : %d : %d : %d \n",this->device_addr.host,
	    this->device_addr.robot,
	    this->device_addr.interf,
	    this->device_addr.index);
  printf("type %d subtype %d not handled\n",hdr->type, hdr->subtype);
  fflush(stdout);
  return -1;
}
////////////////////////////////////////////////////////////////////////////////
// publish new data if available
void UsFakeLocalize::PublishNewData() {
  if (bot->bNewLocation) {
    bot->bLockLocation = true;
    myPosition.pos.px = bot->location->hypoths[0].mean.px + origin.px;
    myPosition.pos.py = bot->location->hypoths[0].mean.py + origin.py;
    myPosition.pos.pa = bot->location->hypoths[0].mean.pa;
    PLAYER_MSG3(6, "us_fakelocalize: %f %f %f", myPosition.pos.px, myPosition.pos.py, myPosition.pos.pa);
    Publish(device_addr, 
		  PLAYER_MSGTYPE_DATA,
		  PLAYER_LOCALIZE_DATA_HYPOTHS,
		  &myPosition,
		  sizeof(myPosition));
    bot->bLockLocation = false;
    bot->bNewLocation = false;
  }
}
////////////////////////////////////////////////////////////////////////////////
// main loop
void UsFakeLocalize::Main() {
  while(true) {
    pthread_testcancel();
    ProcessMessages();
    PublishNewData();
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }  
}
////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsFakeLocalize::Shutdown()
{
  // Unsubscribe from devices.
  return 0;
}
