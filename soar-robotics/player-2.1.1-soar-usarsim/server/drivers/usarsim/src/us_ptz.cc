#include "us_ptz.h"
/**
 * Initialization function
 */ 
Driver* UsPtz_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsPtz - Init\n");
  return ((Driver*) (new UsPtz(cf, section)));
}

/**
 * a driver registration function
 */
void UsPtz_Register(DriverTable* table)
{
  table->AddDriver("us_ptz", UsPtz_Init);
  
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
UsPtz::UsPtz(ConfigFile* cf, int section)    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PTZ_CODE)

{
  fprintf(stderr,"UsPtz - Constructor\n");

  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                         PLAYER_SIMULATION_CODE, -1,NULL) != 0)
    {
	 this->SetError(-1);
	 return;
    } 
  strncpy(this->ptz_name, cf->ReadString(section, "ptz_name", ""), sizeof(this->ptz_name));
  strncpy(this->cam_name, cf->ReadString(section, "cam_name", ""), sizeof(this->cam_name));
  this->mode = PLAYER_PTZ_POSITION_CONTROL;
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsPtz::Setup()
{
  Device* bot_device;
  bool dummy = false;
  
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
    return(-1);
  }
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  // Prepare data for bot
  bot->bGeoPtz.push_back(dummy);
  bot->bNewPtz.push_back(dummy);
  bot->bNewPtzZoom.push_back(dummy);
  bot->bLockPtz.push_back(dummy);
  bot->ptz_name.push_back(ptz_name);
  bot->cam_name.push_back(cam_name);
  bot->ptz.push_back((player_ptz_data_t *)calloc(1, sizeof(player_ptz_data_t)));
  bot->ptz_geom.push_back((player_ptz_geom_t *)calloc(1, sizeof(player_ptz_geom_t)));
  ptz_index = bot->ptz_name.size() - 1;
  bot->devices |= US_DATA_PTZ|US_GEOM_PTZ|US_CONF_CAMERA;
  // Start the device thread
  StartThread();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
UsPtz::~UsPtz()
{
  // Nothing to do...
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Main
void UsPtz::Main() {
  while(true) {
    char* cmd = new char[USBOT_MAX_CMD_LEN];
    sprintf(cmd,"GETCONF {Type Camera} {Name %s}\r\n",cam_name);
    bot->AddCommand(cmd); //publishusarsim
    char* cmd2 = new char[USBOT_MAX_CMD_LEN];
    sprintf(cmd2,"GETGEO {Type Camera} {Name %s}\r\n",cam_name);
    bot->AddCommand(cmd2); //publishusarsim
    pthread_testcancel();
    ProcessMessages();
    PublishNewData();
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
}
/**
 *
 */
int UsPtz::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_PTZ_REQ_GENERIC, 
                           this->device_addr)) {
    this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_NACK,PLAYER_PTZ_REQ_GENERIC,
			   data, sizeof(data),NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
					  PLAYER_PTZ_REQ_CONTROL_MODE, 
                           this->device_addr)) {
   player_ptz_req_control_mode_t control = *reinterpret_cast<player_ptz_req_control_mode_t *> (data);
   mode = control.mode;
   this->Publish(this->device_addr,resp_queue, 
			   PLAYER_MSGTYPE_RESP_ACK,PLAYER_PTZ_REQ_CONTROL_MODE,
			   data, sizeof(data),NULL); 
   return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
						  PLAYER_PTZ_REQ_GEOM, 
						  this->device_addr)) {
   char* cmd = new char[USBOT_MAX_CMD_LEN];
    sprintf(cmd,"GETGEO {Type Camera} {Name %s}\r\n",cam_name);
    bot->AddCommand(cmd); //publishusarsim
    
    short count = 0;
    while (!bot->bGeoPtz[ptz_index] && count < USBOT_STARTUP_CONN_LIMIT) {
	 usleep(USBOT_DELAY_USEC);
	 count++;
    }
    if (!bot->bGeoPtz[ptz_index]) {
	 PLAYER_ERROR("PutReply() failed");
	 this->Publish(this->device_addr,
				PLAYER_MSGTYPE_RESP_NACK,
				PLAYER_PTZ_REQ_GEOM,
				data,
				sizeof(data),NULL);
	 return -1;
    }
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_RESP_ACK,
			   PLAYER_PTZ_REQ_GEOM,
			   bot->ptz_geom[ptz_index],
			   sizeof(*bot->ptz_geom[ptz_index]),NULL);
    return 0;
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,
						  PLAYER_PTZ_CMD_STATE, 
						  this->device_addr)) {
    player_ptz_cmd_t cmd =  *reinterpret_cast<player_ptz_cmd_t *> (data);
    int order = mode == PLAYER_PTZ_VELOCITY_CONTROL?1:0;
    float pan, tilt, zoom;
    char* command1 = new char[USBOT_MAX_CMD_LEN];
    char* command2 = new char[USBOT_MAX_CMD_LEN];
    if (mode == PLAYER_PTZ_VELOCITY_CONTROL) {
	 pan = -1 * cmd.panspeed;
	 tilt = cmd.tiltspeed;
    }
    else {
	 pan = -1 * cmd.pan;
	 tilt = cmd.tilt;
    }
    zoom = cmd.zoom;
    sprintf(command1,"MISPKG {Name %s} {Rotation 0,%f,%f} {Order %d}\r\n",ptz_name,pan,tilt,order);
    //cout<<command1<<endl;
    //fprintf(stderr,"PTZ:[%s]\n",command);
    bot->AddCommand(command1);
    sprintf(command2,"SET {Type Camera} {Name %s} {Opcode Zoom} {%f}\r\n",cam_name,zoom);
    //cout<<command2<<endl;
    //fprintf(stderr,"PTZ:[%s]\n",command);
    bot->AddCommand(command2);
    return 0;
  }
  else {
     printf("ptz hdr: %d : %d : %d : %d \n",hdr->addr.host,
		 hdr->addr.robot,
		 hdr->addr.interf,
		 hdr->addr.index);
    printf("ptz this: %d : %d : %d : %d \n",this->device_addr.host,
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
void UsPtz::PublishNewData(){
  if (bot->bNewPtz[ptz_index] && bot->bNewPtzZoom[ptz_index]) {
    //DBG("Got New Position");
    //printf("us_ptz new data\n");
    //fflush(stdout);
    bot->bLockPtz[ptz_index] = true;
    this->Publish(this->device_addr,
			   PLAYER_MSGTYPE_DATA,
			   PLAYER_PTZ_DATA_STATE,
			   (void*)bot->ptz[ptz_index],sizeof(player_ptz_data_t),NULL);
    bot->bLockPtz[ptz_index] = false;
    bot->bNewPtz[ptz_index] = false;
    bot->bNewPtzZoom[ptz_index] = false;
  }
}
  
////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsPtz::Shutdown()
{
  fprintf(stderr,"UsPtz - Shutdown\n");
  // free data for bot
  //bot->devices &= ~(US_DATA_PTZ);
  //free(bot->ptz);
  return 0;
}


