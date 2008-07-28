#include "us_laser3d.h"

///////////////////////////////////////////////////////////////////////////////
// Initialization function
Driver* UsLaser3d_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsLaser3d - Init\n");
  return ((Driver*) (new UsLaser3d(cf, section)));
}
///////////////////////////////////////////////////////////////////////////////
// a driver registration function
void UsLaser3d_Register(DriverTable* table)
{
  table->AddDriver("us_laser3d", UsLaser3d_Init);
  return;
}
///////////////////////////////////////////////////////////////////////////////
// Constructor
UsLaser3d::UsLaser3d(ConfigFile* cf, int section):Driver(cf, section, true,
										   PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
										   PLAYER_POINTCLOUD3D_CODE)
{
  fprintf(stderr,"UsLaser3d - Constructor\n");
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                         PLAYER_SIMULATION_CODE, -1,NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  strncpy(this->name, cf->ReadString(section, "laser3d_name", ""), sizeof(this->name));
  // init pointer to local data
  data = NULL;
  geom = NULL;
  conf = NULL;
  
  Device* bot_device;// bot device
  if(!(bot_device = deviceTable->GetDevice(this->bot_id)))
  {
    PLAYER_ERROR("unable to locate suitable bot device");
  }
  // init mutexes
  pthread_mutex_init(&confMutex, NULL);
  pthread_mutex_init(&dataMutex, NULL);
  pthread_mutex_init(&geomMutex, NULL);

  //now I can access the fields in driver directly
  bot = ((UsBot*)bot_device->driver);
  bot->RegisterDriver("Laser3d", name, this);
  
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsLaser3d::Setup()
{
  bot->SubscribeDriver("Laser3d", name);
  PLAYER_MSG1(1,"subscribe %s", name);
  StartThread();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
UsLaser3d::~UsLaser3d()
{
  delete data;
  delete geom;
  delete conf;
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Main
void UsLaser3d::Main()
{
  while(true) {
    pthread_testcancel();
    usleep(USBOT_MAIN_LOOP_USEC);
    ProcessMessages();
    PublishNewData();
    //usleep(USBOT_STARTUP_INTERVAL_USEC);
  }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessMessages
int UsLaser3d::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data)
{
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_LASER_REQ_GET_GEOM,
                           this->device_addr)) {
    player_laser3d_geom_t* geom = GetGeom();
    if (geom == NULL)
    {
      PLAYER_ERROR("PutReply() failed");
      this->Publish(this->device_addr,
                    PLAYER_MSGTYPE_RESP_NACK,
                    PLAYER_LASER_REQ_GET_GEOM,
                    data,sizeof(data),NULL);
      return -1;
    }
    pthread_mutex_lock(&geomMutex);
    this->Publish(this->device_addr,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LASER_REQ_GET_GEOM,
                  geom,
                  sizeof(*geom),NULL);
    pthread_mutex_unlock(&geomMutex);
    return 0;
  }
  else
  {
    printf("laser3d hdr: %d : %d : %d : %d \n",hdr->addr.host,
            hdr->addr.robot,
            hdr->addr.interf,
            hdr->addr.index);
    printf("laser3d this: %d : %d : %d : %d \n",this->device_addr.host,
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
void UsLaser3d::PublishNewData()
{

  player_pointcloud3d_data_t* data = GetData();
  player_laser3d_config_t* conf = GetConf();
 //if new data available send it
  if (data != NULL && conf != NULL)
  {
    pthread_mutex_lock(&confMutex);
    pthread_mutex_lock(&dataMutex);
    pthread_mutex_unlock(&confMutex);
    
    this->Publish(this->device_addr,
                  PLAYER_MSGTYPE_DATA,
                  PLAYER_POINTCLOUD3D_CODE,
                  data,sizeof(*data),NULL);
    pthread_mutex_unlock(&dataMutex);
  }
  delete data;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsLaser3d::Shutdown()
{
  //fprintf(stderr,"UsLaser - Shutdown\n");
  bot->UnsubscribeDriver("Laser3d", this->name);
  return 0;
}

/**
 * Sets new laser data (called by US_BOT)
 *
 */
void UsLaser3d::SetData(player_pointcloud3d_data_t* data)
{
  pthread_mutex_lock(&dataMutex);
  if(this->data != NULL) {
    delete this->data;
  }
  this->data = data;
  pthread_mutex_unlock(&dataMutex);
}


/**
 * Sets new geometry information (called by US_BOT)
 */
void UsLaser3d::SetGeom(player_laser3d_geom_t* geom)
{
  pthread_mutex_lock(&geomMutex);
  if(this->geom != NULL) {
    delete this->geom;
  }
  this->geom = geom;
  pthread_mutex_unlock(&geomMutex);
}

/**
 * Sets new geometry information (called by US_BOT)
 */
void UsLaser3d::SetConf(player_laser3d_config_t* conf)
{
  pthread_mutex_lock(&confMutex);
  if(this->conf != NULL) {
    delete this->conf;
  }
  this->conf = conf;
  pthread_mutex_unlock(&confMutex);
}

/**
 * Returns pointer to most up to date laser data object. May return NULL
 * if no data is available since last method call.
 *
 */
player_pointcloud3d_data_t* UsLaser3d::GetData()
{
  pthread_mutex_lock(&dataMutex);
  player_pointcloud3d_data_t* curdata = data;
  data = NULL;
  pthread_mutex_unlock(&dataMutex);
  return curdata;
}

/**
 * Returns pointer to geometry data.
 */
player_laser3d_geom_t* UsLaser3d::GetGeom()
{
  //PLAYER_MSG0(3,"us_laser GetGeom");
  // no geom info available -> bot must fetch it
  if(geom == NULL)
  {
    int retry_count = 0;
    int sleep_count = 0;

    while(geom == NULL && retry_count < 10)
    {
      bot->RequestGeom("RangeScanner3d", name);
      // TODO: we need constants for sleep and retry limit
      while(geom == NULL && sleep_count < 1000)
      {
        sleep_count++;
        usleep(1000);
      }
      retry_count++;
      sleep_count = 0;
    }
    assert(geom != NULL);
  }
  return geom;
}

player_laser3d_config_t* UsLaser3d::GetConf()
{
  // PLAYER_MSG0(3,"us_laser GetConf");
  // no geom info available -> bot must fetch it
  if(conf == NULL)
  {
    int retry_count = 0;
    int sleep_count = 0;

    while(conf == NULL && retry_count < 10)
    {
      bot->RequestConf("RangeScanner3d", name);
      // TODO: we need constants for sleep and retry limit
      while(conf == NULL && sleep_count < 1000)
      {
        sleep_count++;
        usleep(1000);
      }
      retry_count++;
      sleep_count = 0;
    }
    assert(conf != NULL);
  }
  return conf;
}




