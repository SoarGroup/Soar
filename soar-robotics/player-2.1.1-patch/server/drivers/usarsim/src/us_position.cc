#include "us_position.h"

////////////////////////////////////////////////////////////////////////////////
//Player Initialization function
Driver* UsPosition_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsPosition - Init\n");
  return ((Driver*) (new UsPosition(cf, section)));
}
////////////////////////////////////////////////////////////////////////////////
// Player driver registration function
void UsPosition_Register(DriverTable* table)
{
  table->AddDriver("us_position", UsPosition_Init);
  return;
}
// Constructor
UsPosition::UsPosition(ConfigFile* cf, int section) : Driver(cf, section)
{
  fprintf(stderr,"UsPosition - Constructor\n");
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                         PLAYER_SIMULATION_CODE, -1,NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  memset(&this->odometry_addr, 0, sizeof(player_devaddr_t));
  memset(&this->command_addr, 0, sizeof(player_devaddr_t));

  //Is an odometry position interface requested?
  if(cf->ReadDeviceAddr(&(this->odometry_addr), section, "provides",
				    PLAYER_POSITION2D_CODE, -1, "odometry") == 0)
  {
    if(this->AddInterface(this->odometry_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  //Is a command position interface requested?
  if(cf->ReadDeviceAddr(&(this->command_addr), section, "provides",
                      PLAYER_POSITION2D_CODE, -1, "command") == 0)
  {
    //Can we not allocate space for a command buffer?
    if(this->AddInterface(this->command_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  strncpy(this->odo_name, cf->ReadString(section,"odo_name","Odometry"),sizeof(this->odo_name));
  pthread_mutex_init(&posMutex, NULL);

  // bot device
  Device* bot_device;
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
  }
  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  // make this position driver known for the bot
  bot->RegisterDriver("Position", odo_name, this);
  old_trans = 0.0;
  return;
}

// Set up the device (called by server thread).
int UsPosition::Setup()
{
  bot->SubscribeDriver("Position", odo_name);
  bot->SubscribeDriver("Encoder", odo_name);
  
  // TODO: do we have to re-allocate these variables on every subscribe?
  if(bot->robotDimensions == NULL) {
    bot->robotDimensions = (player_bbox3d_t*)calloc(1, sizeof(player_bbox3d_t));
  }
  //bot->position = (player_position2d_data_t *)calloc(1, sizeof(player_position2d_data_t));
  bot->robotGeom = (player_position2d_geom_t *)calloc(1, sizeof(player_position2d_geom_t));
  if(bot->steeringType == NULL) {
    bot->steeringType = new char[128];
  }
  bot->devices |= US_CONF_ROBOT;
  bot->devices |= US_GEOM_ROBOT;
  char* cmd = new char[USBOT_MAX_CMD_LEN];
  sprintf(cmd,"GETCONF {Type Robot}\r\n");
  bot->AddCommand(cmd);
  char* cmd2 = new char[USBOT_MAX_CMD_LEN];
  sprintf(cmd2,"GETGEO {Type Robot}\r\n");
  bot->AddCommand(cmd2);
  int count =0;
  while (!bot->bConfRobot == true && !bot->bGeoRobot == true && count < USBOT_STARTUP_CONN_LIMIT)
  {
      usleep(USBOT_DELAY_USEC);
      count++;
  }
  setSteerType();
  
  StartThread();  // Start the device thread
  return 0;
}

// Destructor
UsPosition::~UsPosition()
{
  return; // Nothing to do...
}

// Main
void UsPosition::Main() { 
  while(true) {
    pthread_testcancel();
    ProcessMessages();
    PublishNewData();
    usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
}

// ProcessMessages
int UsPosition::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, this->device_addr))
  {   
    PLAYER_MSG0(3,"POSITION REQ GEO\n");

    if (!bot->bGeoRobot == true)
    {
      PLAYER_MSG0(3,"POSITION REQ GEO NACK\n");
      this->Publish(this->device_addr,resp_queue,
                    PLAYER_MSGTYPE_RESP_NACK,
                    PLAYER_POSITION2D_REQ_GET_GEOM,
                    (void*)bot->robotGeom,
                    sizeof(player_position2d_geom_t),NULL);
      return -1;
    }
    PLAYER_MSG0(3,"POSITION REQ GEO ACK\n");
    this->Publish(this->device_addr,resp_queue, 
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_GET_GEOM,
                  (void*)bot->robotGeom,
                  sizeof(player_position2d_geom_t),NULL);
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
            PLAYER_POSITION2D_REQ_MOTOR_POWER,
            this->device_addr))
  {
    bot->devices |= US_DATA_POSITION;
    this->Publish(this->device_addr,resp_queue, 
      PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION2D_REQ_MOTOR_POWER,
      data, sizeof(data),NULL);
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
            PLAYER_POSITION2D_REQ_RESET_ODOM,
            this->device_addr))
  {
    player_position2d_set_odom_req_t odo_cmd = *reinterpret_cast<player_position2d_set_odom_req_t *> (data);
    char* cmd = new char[USBOT_MAX_CMD_LEN];
    sprintf(cmd,"SET {Type Odometry} {Name %s} {Opcode RESET} \r\n",odo_name);
    bot->AddCommand(cmd);
    this->Publish(this->device_addr,resp_queue, 
        PLAYER_MSGTYPE_RESP_ACK,PLAYER_POSITION2D_REQ_RESET_ODOM,
        data, sizeof(data),NULL);
    return 0;
  }
  /****************** VELOCITY-MODE  *****************
   * normale drive method for SKID robots 
   * bend drive method for ACKERMANN robots
   * (one rear and one front powered wheel)
   */
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                  PLAYER_POSITION2D_CMD_VEL,
                                  this->device_addr))
  {
    player_position2d_cmd_vel_t position_cmd = *reinterpret_cast<player_position2d_cmd_vel_t *> (data);
    float trans = position_cmd.vel.px;
    float rotate = position_cmd.vel.pa;
    char* cmd = new char[USBOT_MAX_CMD_LEN];
    //don't send
    //DRIVE {Speed 0.000000} {FrontSteer 0.000000} {RearSteer 0.000000}
    //more than ones
    if(trans == 0.0 && old_trans == 0.0) return 0;
    if(steer_type == SKIDSTEERED)
    {
      if ((bot->maxWheelSeparation == -1) || (bot->wheelRadius == -1)) {
        sprintf(cmd,"DRIVE {Left %f} {Right %f}\r\n",trans-rotate,trans+rotate);
      }
      else
      {//will only work if robot is stopped while turning
        sprintf(cmd,"DRIVE {Left %f} {Right %f}\r\n",
          (trans - 100.0 * bot->maxWheelSeparation * rotate) / bot->wheelRadius,
          (trans + 100.0 * bot->maxWheelSeparation * rotate) / bot->wheelRadius);
      }
    }
    else if(steer_type == ACKERMANNSTEERED)
    {
      if(trans > 0) {
        sprintf(cmd,"DRIVE {Speed %f} {FrontSteer %f} {RearSteer %f}\n\r",trans,-rotate,rotate);
      }
      else {
        sprintf(cmd,"DRIVE {Speed %f} {FrontSteer %f} {RearSteer %f}\n\r",trans,rotate,-rotate);
      }
    }

    PLAYER_MSG1(4,"position vel cmd: %s", cmd);
    old_trans = trans;
    bot->AddCommand(cmd);	 //publishusarsim
    set_trans = position_cmd.vel.px;
    set_rot = position_cmd.vel.pa;
    return 0;
  }

  /****************** CAR / HOLO-MODE *****************
   * only for Ackermann steered robots
   * Holo mode for one rear and one front steered wheel
   * normal car mode for two front steered wheels
   */
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                  PLAYER_POSITION2D_CMD_CAR,
                                  this->device_addr))
  {
    player_position2d_cmd_car_t position_cmd = *reinterpret_cast<player_position2d_cmd_car_t *> (data);

    if(steer_type == SKIDSTEERED) {
      return -1;
    }
    
    float vel = position_cmd.velocity;
    if(vel == 0.0 && old_trans == 0.0) return 0;

    float angle = position_cmd.angle;
    char* cmd = new char[USBOT_MAX_CMD_LEN];
    if((bot->maxWheelSeparation != -1) && (bot->wheelRadius != -1)){
      vel /= bot->wheelRadius;
    }
    if(steer_type == ACKERMANNSTEERED)
    {
      // Set control commands:
      sprintf(cmd,"DRIVE {Speed %f} {FrontSteer %f} {RearSteer %f}\n\r",vel,angle,angle);
      // Print out control commands, if something happened:
      PLAYER_MSG1(4,"position bend cmd: %s", cmd);
      // Add commands to robot's queue:
      bot->AddCommand(cmd);
    }
    old_trans = vel;
    return 0;
  }
  else
  {
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

// PublishNewData
void UsPosition::PublishNewData()
{
  if (pos != NULL)
  {
    /*player_position2d_data_t command;
    memset(&command, 0, sizeof(player_position2d_data_t));

    command.vel.px = set_trans;
    command.vel.pa = set_rot;
    //TODO ? publish odometry command ?*/
    //bot->bLockPosition = true;
    /*
    this->Publish(this->device_addr,NULL, 
          PLAYER_MSGTYPE_DATA,
          PLAYER_POSITION2D_DATA_STATE,
          &command,sizeof(player_position2d_data_t),NULL);
    */
    pthread_mutex_lock(&posMutex);
    this->Publish(this->device_addr, 
                  PLAYER_MSGTYPE_DATA,
                  PLAYER_POSITION2D_DATA_STATE,
                  (void*)this->pos, sizeof(player_position2d_data_t),NULL);
    delete pos;
    pos = NULL;
    pthread_mutex_unlock(&posMutex);
  }
}

// Shutdown the device (called by server thread).
int UsPosition::Shutdown()
{
  bot->UnsubscribeDriver("Position", odo_name);
  bot->UnsubscribeDriver("Encoder", odo_name);
  fprintf(stderr,"UsPosition - Shutdown\n");
  delete this->pos;
  this->pos = NULL;
  return 0;
}

bool UsPosition::setSteerType()
{
  if(bot->robotGeom == NULL) return false;
  PLAYER_MSG1(3,"bot->steeringType %s",bot->steeringType);
  if(strstr(bot->steeringType,"SkidSteered") != NULL) steer_type = SKIDSTEERED;
  else if(strstr(bot->steeringType,"AckermanSteered") != NULL) steer_type = ACKERMANNSTEERED;
  return true;
}

void UsPosition::setTicks(int ticks_l, int ticks_r)
{
  PLAYER_MSG2(2,"ticks: %d %d",ticks_l,ticks_r);
  this->ticks_l = ticks_l;
  this->ticks_r = ticks_r;
}

void UsPosition::setPosition(player_position2d_data_t* pos)
{
  pthread_mutex_lock(&posMutex);
  if(this->pos != NULL) {
    delete this->pos;
  }
  this->pos = pos;
  pthread_mutex_unlock(&posMutex);
}
