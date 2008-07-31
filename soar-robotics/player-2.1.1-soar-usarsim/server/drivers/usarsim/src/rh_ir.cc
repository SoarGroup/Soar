#include "rh_ir.h"
// Initialization function
Driver* RhIr_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"RhIR - Init\n");
  return ((Driver*) (new RhIR(cf, section)));
}


// a driver registration function
void RhIr_Register(DriverTable* table)
{
  table->AddDriver("rh_ir", RhIr_Init);

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
RhIR::RhIR(ConfigFile* cf, int section)    : Driver(cf, section)
{
  fprintf(stderr,"RhIR - Constructor\n");
  // Nothing to do...
  //this->bot = NULL;
  this->bot_index = cf->ReadInt(section, "simulation_index", -1);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
RhIR::~RhIR()
{
  // Nothing to do...
  return;
}
/**
 *
 */
void RhIR::Main() {
  
  while(true) {
    pthread_testcancel();
    ProcessMessages();
  }
}
/**
 *
 */
int RhIR::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data) {
  //hdr parsen
  // wie ist data zu interpretieren

  //gegebenfalls resp_queue antworten mit publish
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int RhIR::Setup()
{
  fprintf(stderr,"RhIR - Setup\n");

  this->bot.robot = this->device_addr.robot;
  this->bot.interf = PLAYER_SIMULATION_CODE;
  this->bot.index = this->bot_index;
  Device *dummy = deviceTable->GetDevice(bot);
  dummy->Subscribe(InQueue);
  return 0;
  
  //Prepare data for bot
  //bot->ir = (player_ir_data_t *)calloc(1, sizeof(player_ir_data_t));
  //bot->ir_geom = (player_ir_pose_req_t *)calloc(1, sizeof(player_ir_pose_req_t));
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int RhIR::Shutdown()
{
  fprintf(stderr,"RhIR - Shutdown\n");
  // free data for bot
  // bot->devices &= ~(RH_DATA_IR|RH_GEOM_IR);
  //free(bot->ir);
  //free(bot->ir_geom);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Check for new dataplayer_position_data_t
/*
void RhIR::Update()
{
  if (bot->bNewIr)
  {
    bot->bLockIr = true;
    //printf("RhIR::Update(): ir %d", ntohs(bot->ir->ranges[0]));
    this->PutData(bot->ir, sizeof(player_ir_data_t), NULL);
    bot->bLockIr = false;
    bot->bNewIr = false;
  }
  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Commands
/*
void RhIR::PutCommand(player_device_id_t id, void* src, size_t len, struct timeval* timestamp)
{
  // no command
  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Handle requests
/*
int RhIR::PutConfig(player_device_id_t id, void *client, void* req, size_t req_len, struct timeval* timestamp)
{
printf("RhIR::PutConfig()\n");
  switch (((char*) req)[0])
  {
    case PLAYER_IR_POSE_REQ:
      HandleGetGeom(client, req, req_len);
      break;
  /* if there's space, put it in the queue */
/*
    case PLAYER_IR_POWER_REQ:
      HandleMotorPower(client, req, req_len);
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
void RhIR::HandleGetGeom(void *client, void *req, int reqlen)
{
  int count = 0;
  //bot->ir_geom->subtype = PLAYER_IR_POSE_REQ;
  bot->AddCommand("GETGEO {Type RhIr}\r\n");
  while (!bot->bGeoIr && count<USBOT_STARTUP_CONN_LIMIT) {
    usleep(USBOT_DELAY_USEC);
    count++;
  }
  if (!bot->bGeoIr && PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  else if (bot->bGeoIr && PutReply(client, PLAYER_MSGTYPE_RESP_ACK,  bot->ir_geom, sizeof(player_ir_pose_req_t), NULL) != 0)
    PLAYER_ERROR("PutReply() failed");

  return;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Handle motor power
/*
void RhIR::HandleMotorPower(void *client, void *req, int reqlen)
{
      if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
    PLAYER_ERROR("PutReply() failed");

  return;
}
*/
/*
// Plugin initialization
extern "C" {
  int player_driver_init(DriverTable *table){
    RhIr_Register(table);
    return 0;
  }
}
*/
