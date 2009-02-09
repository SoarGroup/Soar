#include "us_laser.h"


///////////////////////////////////////////////////////////////////////////////
// Initialization function
Driver* UsLaser_Init(ConfigFile* cf, int section)
{
  fprintf(stderr,"UsLaser - Init\n");
  return ((Driver*) (new UsLaser(cf, section)));
}
///////////////////////////////////////////////////////////////////////////////
// a driver registration function
void UsLaser_Register(DriverTable* table)
{
  table->AddDriver("us_laser", UsLaser_Init);
  return;
}
///////////////////////////////////////////////////////////////////////////////
// Constructor
UsLaser::UsLaser(ConfigFile* cf, int section):Driver(cf, section, true,
										   PLAYER_MSGQUEUE_DEFAULT_MAXLEN,
										   PLAYER_LASER_CODE)
{
  fprintf(stderr,"UsLaser - Constructor\n");
  if (cf->ReadDeviceAddr(&this->bot_id, section, "requires",
                         PLAYER_SIMULATION_CODE, -1,NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  strncpy(this->name, cf->ReadString(section, "laser_name", ""), sizeof(this->name));
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
  bot->RegisterDriver("RangeScanner", name, this);
  
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int UsLaser::Setup()
{
  bot->SubscribeDriver("RangeScanner", name);
  PLAYER_MSG1(1,"subscribe %s", name);
  StartThread();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
UsLaser::~UsLaser()
{
  // Nothing to do...
  delete data;
  delete geom;
  delete conf;
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Main
void UsLaser::Main()
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
int UsLaser::ProcessMessage(QueuePointer& resp_queue, player_msghdr *hdr, void *data)
{
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_LASER_REQ_GET_GEOM,
                           this->device_addr)) {
    player_laser_geom_t* geom = GetGeom();
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
    printf("laser hdr: %d : %d : %d : %d \n",hdr->addr.host,
            hdr->addr.robot,
            hdr->addr.interf,
            hdr->addr.index);
    printf("laser this: %d : %d : %d : %d \n",this->device_addr.host,
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
void UsLaser::PublishNewData()
{

  player_laser_data_t* data = GetData();
  player_laser_config_t* conf = GetConf();
 //if new data available send it
  if (data != NULL && conf != NULL)
  {
    pthread_mutex_lock(&confMutex);
    pthread_mutex_lock(&dataMutex);
    data->min_angle  = conf->min_angle;
    data->max_angle  = conf->max_angle;
    data->max_range  = conf->max_range;
    data->resolution = conf->resolution;
    // workaround: usarsim is not precise enough for vfh
    if(data->resolution == 0.0174) {
      data->resolution = 0.0174533;
    }

    pthread_mutex_unlock(&confMutex);
  
    //PLAYER_MSG5(4,"min_a %f max_a %f max_r %f res %f count %d\n", data->min_angle,
    //data->max_angle, data->max_range, data->resolution,data->ranges_count);
    this->Publish(this->device_addr,
                  PLAYER_MSGTYPE_DATA,
                  PLAYER_LASER_DATA_SCAN,
                  data,sizeof(*data),NULL);
    pthread_mutex_unlock(&dataMutex);
  }
  delete data;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int UsLaser::Shutdown()
{
  //fprintf(stderr,"UsLaser - Shutdown\n");
  bot->UnsubscribeDriver("RangeScanner", this->name);
  return 0;
}

/**
 * Sets new laser data (called by US_BOT)
 *
 */
void UsLaser::SetData(player_laser_data_t* data)
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
void UsLaser::SetGeom(player_laser_geom_t* geom)
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
void UsLaser::SetConf(player_laser_config_t* conf)
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
player_laser_data_t* UsLaser::GetData()
{
  pthread_mutex_lock(&dataMutex);
  player_laser_data_t* curdata = data;
  data = NULL;
  pthread_mutex_unlock(&dataMutex);
  return curdata;
}

/**
 * Returns pointer to geometry data.
 */
player_laser_geom_t* UsLaser::GetGeom()
{
  //PLAYER_MSG0(3,"us_laser GetGeom");
  // no geom info available -> bot must fetch it
  if(geom == NULL)
  {
    int retry_count = 0;
    int sleep_count = 0;

    while(geom == NULL && retry_count < 10)
    {
      bot->RequestGeom("RangeScanner", name);
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

player_laser_config_t* UsLaser::GetConf()
{
  // PLAYER_MSG0(3,"us_laser GetConf");
  // no geom info available -> bot must fetch it
  if(conf == NULL)
  {
    int retry_count = 0;
    int sleep_count = 0;

    while(conf == NULL && retry_count < 10)
    {
      bot->RequestConf("RangeScanner", name);
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




