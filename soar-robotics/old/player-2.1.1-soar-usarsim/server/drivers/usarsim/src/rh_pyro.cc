#include "rh_pyro.h"
// Initialization function
Driver* RhPyro_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"RhPyro - Init\n");
  return ((Driver*) (new RhPyro(cf, section)));
}



// a driver registration function
void RhPyro_Register(DriverTable* table)
{
  table->AddDriver("rh_pyro", RhPyro_Init);

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Constructor
RhPyro::RhPyro(ConfigFile* cf, int section)    : Driver(cf, section)
{
  fprintf(stderr,"RhPyro - Constructor\n");
  // Nothing to do...
  //this->bot = NULL;
  this->bot_index = cf->ReadInt(section, "simulation_index", -1);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
RhPyro::~RhPyro()
{
  // Nothing to do...
  return;
}
/**
 *
 */
void RhPyro::Main() {
  
  while(true) {
    pthread_testcancel();
    ProcessMessages();
  }
}
/**
 *
 */
int RhPyro::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  //hdr parsen
  // wie ist data zu interpretieren

  //gegebenfalls resp_queue antworten mit publish
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int RhPyro::Setup()
{
  fprintf(stderr,"RhIR - Setup\n");

  this->bot.robot = this->device_addr.robot;
  this->bot.interf = PLAYER_SIMULATION_CODE;
  this->bot.index = this->bot_index;
  Device *dummy = deviceTable->GetDevice(bot);
  dummy->Subscribe(InQueue);
  return 0;


  // short count = 0;


  // Prepare data for bot
  //bot->pyro = (player_fiducial_data_t *)calloc(1, sizeof(player_fiducial_data_t));
  //bot->pyro_geom = (player_fiducial_geom_t *)calloc(1, sizeof(player_fiducial_geom_t));
  //bot->pyro_conf = (player_fiducial_fov_t *)calloc(1, sizeof(player_fiducial_fov_t));
  //bot->devices |= RH_DATA_PYRO|RH_GEOM_PYRO|RH_CONF_PYRO;
  
  
  // Get pyro configuration from bot
  // bot->AddCommand("GETCONF {Type RhPyro}\r\n");
  /*bot->pyro_conf->subtype = PLAYER_FIDUCIAL_GET_FOV;
  while (!bot->bConfPyro && count<USBOT_STARTUP_CONN_LIMIT) {
    usleep(USBOT_DELAY_USEC);
    count++;
  }
  if (!bot->bConfPyro) {
    PLAYER_ERROR("unable to get pyro configuration");
    return -1;
  } 
  return 0;
  */
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int RhPyro::Shutdown()
{
  fprintf(stderr,"RhPyro - Shutdown\n");
  // free data for bot
  //bot->devices &= ~(RH_DATA_PYRO|RH_GEOM_PYRO);
  //free(bot->pyro);
  //free(bot->pyro_geom);
  //free(bot->pyro_conf);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Check for new dataplayer_position_data_t
/*
void RhPyro::Update()
{
//fprintf(stderr,"rhpyroUPDATE");
  if (bot->bNewPyro)
  {
    bot->bLockPyro = true;
    this->PutData(bot->pyro, sizeof(player_fiducial_data_t), NULL);
    bot->bLockPyro = false;
    bot->bNewPyro = false;
  }
  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Commands
/*
void RhPyro::PutCommand(player_device_id_t id, void* src, size_t len, struct timeval* timestamp)
{
  // no command
  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Handle requests
/*
int RhPyro::PutConfig(player_device_id_t id, void *client, void* req, size_t req_len, struct timeval* timestamp)
{
fprintf(stderr,"rhpyroputconfig");
  switch (((char*) req)[0])
  {
    case PLAYER_FIDUCIAL_GET_GEOM:
      HandleGetGeom(client, req, req_len);
      break;
  /* if there's space, put it in the queue */

    //case PLAYER_PYRO_POWER_REQ:
    //  HandleMotorPower(client, req, req_len);
    //  break;
/*    
   case PLAYER_FIDUCIAL_GET_FOV:
   case PLAYER_FIDUCIAL_SET_FOV:
      HandleGetConfig(client, req, req_len);
      break;

    default:
      if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
        PLAYER_ERROR("PutReply() failed");
      break;
  }
  return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Handle geometry requests.
/*
void RhPyro::HandleGetGeom(void *client, void *req, int reqlen)
{
  int count = 0;
  bot->AddCommand("GETGEO {Type RhPyro}\r\n");
  while (!bot->bGeoPyro && count<USBOT_STARTUP_CONN_LIMIT) {
    usleep(USBOT_DELAY_USEC);
    count++;
  }
  if (!bot->bGeoPyro && PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  else if (bot->bGeoPyro && PutReply(client, PLAYER_MSGTYPE_RESP_ACK,  bot->pyro_geom, sizeof(player_fiducial_geom_t), NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  

  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Handle motor power
/*
void RhPyro::HandleMotorPower(void *client, void *req, int reqlen)
{
      if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
    PLAYER_ERROR("PutReply() failed");

  return;
}
*/
////////////////////////////////////////////////////////////////////////////////
// Handle motor power
/*
void RhPyro::HandleGetConfig(void *client, void *req, int reqlen)
{
  if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
    PLAYER_ERROR("PutReply() failed");

  return;
}
*/
/*
// Player plugin initialization
extern "C"{
  int player_driver_init(DriverTable *table){
    RhPyro_Register(table);
    return 0;
  }
}
*/
