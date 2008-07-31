#include "nav200.h"



Nav200::Nav200()
{
}


Nav200::~Nav200()
{
}

int Nav200::Initialise(Driver* sn2002, Device* opaque2, player_devaddr_t opaque_id2 )
{
  this->sn200 = sn2002;
  this->opaque_id = opaque_id2;
  this->opaque = opaque2;
  bytesReceived = 0;

  return 0;
}

int Nav200::Terminate()
{
  return 0;
}

/*****************
* Standby mode
*******************/

bool Nav200::EnterStandby()
{
  WriteCommand('S','A',0,NULL);
  if (ReadFromNav200() > 0)
  {
    if (packet.length==5 && packet.mode == 'S' && packet.function == 'A')
      return true;
  }
  return false;
}

int Nav200::GetVersionNumber()
{
  WriteCommand('S','V',0,NULL);
  if (ReadFromNav200() > 0)
  {
    if (packet.length==8 && packet.mode == 'S' && packet.function == 'V')
    {
      //printf("Version: %d.%d.%d\n",packet.data[0],packet.data[1],packet.data[2]);
      return (*reinterpret_cast<int*> (packet.data))&0xFFFFFF;
    }
  }
  return 0;

}

char* Nav200::GetVersionString()
{
  WriteCommand('S','T',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.mode == 'S' && packet.function == 'T')
    {
      packet.data[packet.dataLength]='\0';
      return(reinterpret_cast<char*>(packet.data));
    }
  }
  return NULL;
}

short Nav200::GetDeviceSerial()
{
  WriteCommand('S','S',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.length==7 && packet.mode == 'S' && packet.function == 'S')
      return *reinterpret_cast<short*> (packet.data);
  }
  return 0;
}

bool Nav200::rotateDirection(uint8_t direction)
{
  WriteCommand('S','U',1,&direction);
  if(ReadFromNav200()>0)
  {
    if(packet.length==6 && packet.mode == 'S' && packet.function == 'U' /*&& *reinterpret_cast<uint8_t*> (packet.data) == direction*/)
    {
      return true;
    }
  }
  return false;
}

bool Nav200::GetReflectorPosition(uint8_t layer, uint8_t number, PositionXY & reflector)
{
  uint8_t data[2] = {layer,number};
  WriteCommand('S','R',2,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'S' && packet.function == 'R' && packet.data[0]==layer && packet.data[1]==number)
    {
        reflector.x = *reinterpret_cast<int*> (packet.data+2);
        reflector.y = *reinterpret_cast<int*> (packet.data+6);
        return true;
    }
  }
  return false;
}

bool Nav200::ChangeReflectorPosition(uint8_t layer, uint8_t number, int newX, int newY)
{
  uint8_t command[10];
  command[0] = layer;
  command[1] = number;
  *reinterpret_cast<int*> (&command[2]) = newX;
  *reinterpret_cast<int*> (&command[6]) = newY;
  WriteCommand('S','C',10,command);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'S' && packet.function == 'C' && packet.data[0]==layer && packet.data[1]==number && *reinterpret_cast<int*> (packet.data+2) == newX && *reinterpret_cast<int*> (packet.data+6) == newY)
      return true;
  }
  return false;
}

bool Nav200::InsertReflectorPosition(uint8_t layer, uint8_t number, int X, int Y)
{
  uint8_t command[10];
  command[0] = layer;
  command[1] = number;
  *reinterpret_cast<int*> (&command[2]) = X;
  *reinterpret_cast<int*> (&command[6]) = Y;
  WriteCommand('S','I',10,command);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'S' && packet.function == 'I' && packet.data[0]==layer && packet.data[1]==number && *reinterpret_cast<int*> (packet.data+2) == X && *reinterpret_cast<int*> (packet.data+6) == Y)
      return true;
  }
  return false;
}

bool Nav200::DeleteReflectorPosition(uint8_t layer, uint8_t number, PositionXY & reflector)
{
  uint8_t data[2] = {layer,number};
  WriteCommand('S','D',2,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'S' && packet.function == 'D' && packet.data[0]==layer && packet.data[1]==number)
    {
        reflector.x = *reinterpret_cast<int*> (packet.data+2);
        reflector.y = *reinterpret_cast<int*> (packet.data+6);
//         printf("deleted reflector is %d\n",reflector.x);
        return true;
    }
  }
  return false;
}


/*********************************
* Read and set reflector radii
***********************************/
int Nav200::GetReflectorRadius(uint8_t layer)
{
//   ReflectorSize reflector;
  WriteCommand('R','G',1,&layer);
  if(ReadFromNav200()>0)
  {
    if(packet.length==7 && packet.mode == 'R' && packet.function == 'G' && packet.data[0] == layer)
    {
      return packet.data[1];
    }
  }
  return -1;
}

bool Nav200::SetReflectorRadius(uint8_t layer, uint8_t radius)
{
  uint8_t data[2] = {layer,radius};
  WriteCommand('R','S',2,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==7 && packet.mode == 'R' && packet.function == 'G' && packet.data[0] == layer && packet.data[1] == radius)
      return true;
  }
  return false;
}

/*****************
* Mapping mode
*******************/
bool Nav200::EnterMapping()
{
  WriteCommand('M','A',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.length==5 && packet.mode == 'M' && packet.function == 'A')
      return true;
  }
  return false;
}

int Nav200::StartMapping(uint8_t layer, int X, int Y, short orientation, uint8_t radius)
{
  uint8_t data[12];
  data[0] = layer;
  *reinterpret_cast<int*> (&data[1]) = X;
  *reinterpret_cast<int*> (&data[5]) = Y;
  *reinterpret_cast<short*> (&data[9]) = orientation;
  data[11] = radius;
  WriteCommand('M','S',12,data);
  if(ReadFromNav200(100000000)>0)
  {
    if(packet.length==7 && packet.mode == 'M' && packet.function == 'S' && packet.data[0] == layer)
      return packet.data[1];
  }
  return -1;
}

int Nav200::StartMappingMeasurement(uint8_t layer, uint8_t scans, int X, int Y, short orientation, uint8_t radius)
{
  uint8_t data[13];
  data[0] = layer;
  data[1] = scans;
  *reinterpret_cast<int*> (&data[2]) = X;
  *reinterpret_cast<int*> (&data[6]) = Y;
  *reinterpret_cast<short*> (&data[10]) = orientation;
  data[12] = radius;
  WriteCommand('M','M',13,data);
  if(ReadFromNav200(100000000)>0)
  {
    if(packet.length==7 && packet.mode == 'M' && packet.function == 'S' && packet.data[0] == layer)
      return packet.data[1];
  }
  return -1;
}

int Nav200::StartNegativeMappingMeasurement(uint8_t layer, uint8_t scans, int X, int Y, short orientation, uint8_t radius)
{
  uint8_t data[13];
  data[0] = layer;
  data[1] = scans;
  *reinterpret_cast<int*> (&data[2]) = X;
  *reinterpret_cast<int*> (&data[6]) = Y;
  *reinterpret_cast<short*> (&data[10]) = orientation;
  data[12] = radius;
  WriteCommand('M','N',13,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==7 && packet.mode == 'M' && packet.function == 'S' && packet.data[0] == layer)
      return packet.data[1];
  }
  return -1;
}

bool Nav200::MappingPosition(uint8_t layer, uint8_t number, PositionXY & reflector)
{
  uint8_t data[2] = {layer,number};
  WriteCommand('M','R',2,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'M' && packet.function == 'R' && packet.data[0] == layer && packet.data[1] == number)
    {
        reflector.x = *reinterpret_cast<int*> (packet.data+2);
        reflector.y = *reinterpret_cast<int*> (packet.data+6);
        return true;
    }
  }
  return false;
}

/*****************
* Positioning mode
*******************/
bool Nav200::EnterPositioning()
{
  WriteCommand('P','A',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.length==5 && packet.mode == 'P' && packet.function == 'A')
      return true;
  }
  return false;
}

bool Nav200::EnterPositioningInput(uint8_t NumberOfMeasurements)
{
  WriteCommand('P','N',1,&NumberOfMeasurements);
  if(ReadFromNav200()>0)
  {
    if(packet.length==5 && packet.mode == 'P' && packet.function == 'A')
      return true;
  }
  return false;
}

bool Nav200::GetPositionAuto(LaserPos & laserPosition)
{
  WriteCommand('P','P',0,NULL);

  if(ReadFromNav200()>0)
  {
    if(packet.length==17 && packet.mode == 'P' && packet.function == 'P')
    {
      laserPosition.pos.x = *reinterpret_cast<int*> (packet.data);
      laserPosition.pos.y = *reinterpret_cast<int*> (packet.data+4);
      laserPosition.orientation = *reinterpret_cast<short*> (packet.data+8);
      laserPosition.quality = packet.data[10];
      laserPosition.number = packet.data[11];
      return true;
    }
  }
  return false;
}

bool Nav200::GetPositionSpeed(short speedX, short speedY, LaserPos & laserPosition)
{
  uint8_t data[4];
  *reinterpret_cast<short*> (&data[0]) = speedX;
  *reinterpret_cast<short*> (&data[2]) = speedY;
  WriteCommand('P','v',4,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==17 && packet.mode == 'P' && packet.function == 'P')
    {
      laserPosition.pos.x = *reinterpret_cast<int*> (packet.data);
      laserPosition.pos.y = *reinterpret_cast<int*> (packet.data+4);
      laserPosition.orientation = *reinterpret_cast<short*> (packet.data+8);
      laserPosition.quality = packet.data[10];
      laserPosition.number = packet.data[11];
      return true;
    }
  }
  return false;
}

bool Nav200::GetPositionSpeedVelocity(short speedX, short speedY, short velocity, LaserPos & laserPosition)
{
  uint8_t data[6];
  *reinterpret_cast<short*> (&data[0]) = speedX;
  *reinterpret_cast<short*> (&data[2]) = speedY;
  *reinterpret_cast<short*> (&data[4]) = velocity;
  WriteCommand('P','w',6,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==17 && packet.mode == 'P' && packet.function == 'P')
    {
      laserPosition.pos.x = *reinterpret_cast<int*> (packet.data);
      laserPosition.pos.y = *reinterpret_cast<int*> (packet.data+4);
      laserPosition.orientation = *reinterpret_cast<short*> (packet.data+8);
      laserPosition.quality = packet.data[10];
      laserPosition.number = packet.data[11];
      return true;
    }
  }
  return false;
}


bool Nav200::GetPositionSpeedVelocityAbsolute(short speedX, short speedY, short velocity, LaserPos & laserPosition)
{
  uint8_t data[6];
  *reinterpret_cast<short*> (&data[0]) = speedX;
  *reinterpret_cast<short*> (&data[2]) = speedY;
  *reinterpret_cast<short*> (&data[4]) = velocity;
  WriteCommand('P','V',6,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==17 && packet.mode == 'P' && packet.function == 'P')
    {
      laserPosition.pos.x = *reinterpret_cast<int*> (packet.data);
      laserPosition.pos.y = *reinterpret_cast<int*> (packet.data+4);
      laserPosition.orientation = *reinterpret_cast<short*> (packet.data+8);
      laserPosition.quality = packet.data[10];
      laserPosition.number = packet.data[11];
      return true;
    }
  }
  return false;
}

bool Nav200::ChangeLayer(uint8_t layer)
{
  WriteCommand('P','L',1,&layer);
  if(ReadFromNav200()>0)
  {
    if(packet.length==6 && packet.mode=='P' && packet.function=='L' && packet.data[0] == layer)
      return true;
  }
  return false;
}

bool Nav200::ChangeLayerDefPosition(uint8_t layer, int X, int Y, short orientation)
{
  uint8_t data[11];
  data[0] = layer;
  *reinterpret_cast<int*> (&data[1]) = X;
  *reinterpret_cast<int*> (&data[5]) = Y;
  *reinterpret_cast<short*> (&data[9]) = orientation;
  WriteCommand('P','M',11,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==16 && packet.mode == 'P' && packet.function == 'M' && packet.data[0] == layer && *reinterpret_cast<int*> (packet.data+1) == X && *reinterpret_cast<int*> (packet.data+5) == Y && *reinterpret_cast<short*> (packet.data+9) == orientation)
      return true;
  }
  return false;
}

bool Nav200::SetActionRadii(int min, int max)
{
  uint8_t data[8];
  *reinterpret_cast<int*> (&data[0]) = min;
  *reinterpret_cast<int*> (&data[4]) = max;
  WriteCommand('P','O',8,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length==13 && packet.mode == 'P' && packet.function == 'O' && *reinterpret_cast<int*> (packet.data)==min && *reinterpret_cast<int*> (packet.data+4)==max)
      return true;
  }
  return false;
}

bool Nav200::SelectNearest(uint8_t N_nearest)
{
  WriteCommand('P','C',1,&N_nearest);
  if(ReadFromNav200()>0)
  {
    if(packet.length==6 && packet.mode == 'P' && packet.function == 'C' && packet.data[0] == N_nearest)
      return true;
  }
  return false;
}

/*****************
* upload mode
*******************/
bool Nav200::EnterUpload()
{
  WriteCommand('U','A',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.length==5 && packet.mode == 'U' && packet.function == 'A')
      return true;
  }
  return false;
}

bool Nav200::GetUploadTrans(uint8_t layer, ReflectorData & reflector)
{
  WriteCommand('U','R',1,&layer);
  if(ReadFromNav200()>0)
  {
    if(packet.length==15 && packet.mode == 'U' && packet.function == 'R' && packet.data[0] == layer)
    {
      reflector.layer = packet.data[0];
      reflector.number = packet.data[1];
      reflector.pos.x = *reinterpret_cast<int*> (packet.data+2);
      reflector.pos.y = *reinterpret_cast<int*> (packet.data+6);
      return true;
    }
  }
  return false;
}

/*****************
* download mode
******************/
bool Nav200::EnterDownload()
{
  WriteCommand('D','A',0,NULL);
  if(ReadFromNav200()>0)
  {
    if(packet.length==5 && packet.mode == 'D' && packet.function == 'A')
      return true;
  }
  return false;
}

bool Nav200::DownloadReflector(uint8_t layer, uint8_t number, int X, int Y)
{
  uint8_t data[10];
  data[0] = layer;
  data[1] = number;
  *reinterpret_cast<int*> (&data[2]) = X;
  *reinterpret_cast<int*> (&data[6]) = Y;
  WriteCommand('D','R',10,data);
  if(ReadFromNav200()>0)
  {
    if(packet.length == 7 && packet.mode == 'D' && packet.function == 'R' && packet.data[0] == layer && packet.data[1] == number)
    {
      return true;
    }
  }
  return false;
}

/*****************
* Error Messages
******************/
void Nav200::PrintErrorMsg(void)
{
  fprintf(stderr,"ERROR: Last command entry %c:", error.F0); 
  switch(error.F1)
  {
    case 1:   fprintf(stderr,"User's software error: "); break;
    case 2:   fprintf(stderr,"Error class: Transputer: "); break;
    case 3:   fprintf(stderr,"Error in the connection between TPU and sensor: "); break;
    case 4:   fprintf(stderr,"Spurious measurement: "); break;
    case 5:   fprintf(stderr,"Spurious angular measurement: "); break;
    case 6:   fprintf(stderr,"Error in connection to reflector memory: "); break;
    case 7:   fprintf(stderr,"Error in connection between user computer and TPU (RS-232 interface): "); break;
  }
  switch(error.F2)
  {
    case 1:   fprintf(stderr,"input/output, telegram traffic. "); break;
    case 2:   fprintf(stderr,"standby mode. "); break;
    case 3:   fprintf(stderr,"handling reference reflectors. "); break;
    case 4:   fprintf(stderr,"download mode. "); break;
    case 5:   fprintf(stderr,"upload mode. "); break;
    case 6:   fprintf(stderr,"mapping mode. "); break;
    case 7:   fprintf(stderr,"positioning mode. "); break;
    case 8:   fprintf(stderr,"test mode. "); break;
    case 9:   fprintf(stderr,"navigation operation, general. "); break;

  }
  switch(error.F3)
  {
    case 1:   fprintf(stderr,"Unknown command.\n"); break;
    case 2:   fprintf(stderr,"Command(function) not implemented.\n"); break;
    case 3:   fprintf(stderr,"Wrong Command\n"); break;
    case 4:   fprintf(stderr,"Reflector not available\n"); break;
    case 5:   fprintf(stderr,"Wrong reflector number\n"); break;
    case 6:   fprintf(stderr,"Wrong data block\n"); break;
    case 7:   fprintf(stderr,"Input not possible\n"); break;
    case 8:   fprintf(stderr,"Invalid layer\n"); break;
    case 9:   fprintf(stderr,"No reflectors in layer\n"); break;
    case 10:   fprintf(stderr,"All reflectors transferred\n"); break;
    case 11:   fprintf(stderr,"Communication error\n"); break;
    case 12:   fprintf(stderr,"Error in the initialisation of reference reflectors\n"); break;
    case 13:   fprintf(stderr,"Wrong radii during input of effective range\n"); break;
    case 14:   fprintf(stderr,"Flash EPROM not functional\n"); break;
    case 15:   fprintf(stderr,"Wrong reflector radius\n"); break;
  }

}

////////////////////////////////////////////////////////////////////////////////
// Write a packet to Nav200
int Nav200::WriteCommand(char mode, char function, int dataLength, uint8_t * data)
{
/*  if (fd < 0)
    return -1;*/

  int length = dataLength+5;
  uint8_t * buffer = new uint8_t[length];

  // Create header
  buffer[0] = STX;
  buffer[1] = length;
  buffer[2] = mode;
  buffer[3] = function;

  // Copy data
  memcpy(buffer + 4, data, dataLength);

  // Create footer (CRC)
  buffer[length-1] = CreateCRC(buffer, 4 + dataLength);

/*  // Make sure both input and output queues are empty
  tcflush(fd, TCIOFLUSH);*/

/*  // switch to blocking IO for the write
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0 || fcntl(fd,F_SETFL,flags &~O_NONBLOCK) < 0)
  {
    fprintf(stderr,"Error changing to blocking write (%d - %s), disabling\n",errno,strerror(errno));
    tcsetattr(fd, TCSANOW, &oldtio);
    fd = -1;
    delete [] buffer;
    return -1;
  }
*/
  player_opaque_data_t mData;
  mData.data_count = length;
  mData.data = buffer;

  // before we send a command to the NAV200, flush and data coming in from the remote file
  sn200->InQueue->SetFilter(opaque_id.host, opaque_id.robot, opaque_id.interf, opaque_id.index, PLAYER_MSGTYPE_DATA, PLAYER_OPAQUE_DATA_STATE);
  
  //puts("waiting for data");
  sn200->ProcessMessages();
  
  // before we send a command to the NAV200, flush and data coming in from the remote file
  if (bytesReceived)
  {

    do
    {
        PLAYER_WARN1("Buffer was not empty on NAV200 (%d), flushing",bytesReceived);
        bytesReceived = 0;
    	usleep(100000);
    	sn200->ProcessMessages();
    } while (bytesReceived != 0);
  }
  sn200->InQueue->ClearFilter();
  
  opaque->PutMsg(sn200->InQueue, PLAYER_MSGTYPE_CMD, PLAYER_OPAQUE_CMD_DATA, reinterpret_cast<void*>(&mData), 0, NULL);
  
/*  if((length && (write(fd, buffer, length)) < length))
  {
    fprintf(stderr,"Error writing to FOB (%d - %s), disabling\n",errno,strerror(errno));
    tcsetattr(fd, TCSANOW, &oldtio);
    fd = -1;
    delete [] buffer;
    return -1;
  }*/

/*  // restore flags
  if (fcntl(fd,F_SETFL,flags) < 0)
  {
    fprintf(stderr,"Error restoring file mode (%d - %s), disabling\n",errno,strerror(errno));
    tcsetattr(fd, TCSANOW, &oldtio);
    fd = -1;
    delete [] buffer;
    return -1;
  }*/

  delete [] buffer;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Read a packet from the Nav200
int Nav200::ReadFromNav200(int timeout_usec)
{
  
  int dataLength = 0;

  struct timeval start, now;
  gettimeofday(&start,NULL);
  
  sn200->InQueue->SetFilter(opaque_id.host, opaque_id.robot, opaque_id.interf, opaque_id.index, PLAYER_MSGTYPE_DATA, PLAYER_OPAQUE_DATA_STATE);
  
  for (;;)
  {
    //puts("waiting for data");
    sn200->ProcessMessages();
    
    // do we have enough for a header?
    while (bytesReceived > 4)
    {
      //PLAYER_MSG4(2, "recieved STX %d data len %d mode %c fun %c", receivedBuffer[0], receivedBuffer[1], receivedBuffer[2], receivedBuffer[3]);
      if (receivedBuffer[0] != STX)
      { 
    	bool found = false;
        // bad thing, we dont have the correct start to a message
        for(int i=1; i<bytesReceived; i++)
        { // find where STX is
          if(receivedBuffer[i]==STX)
          { // move so STX is at start
            memmove(receivedBuffer, receivedBuffer+i, bytesReceived-i);
            bytesReceived-=i;
            found = true;
            break;
          }
        }
        // If none of the data points are STX then all are (essentially) removed
        if (!found)
        {
        	bytesReceived = 0;
        }
        continue;
      }
  
      // we have valid start byte
      // get length
      dataLength = receivedBuffer[1];
  
      // check we have enough data for full length, if not then break
      if (dataLength > bytesReceived)
      {
        break;
      }
      // calc CRC and check it
      // if bad CRC, assume same as bad STX
      if (CreateCRC(receivedBuffer, dataLength)) // change that later!!
      {// bad thing, we dont have the correct start to a message
        fprintf(stderr,"bad CRC!!!\n");
        bool found = false;
        for(int i=1; i<bytesReceived; i++)
        { // find where STX is
          if(receivedBuffer[i]==STX)
          { // move so STX is at start
            memmove(receivedBuffer, receivedBuffer+i, bytesReceived-i);
            bytesReceived-=i;
            found = true;
            break;
          }
        }
        // If none of the data points are STX then all are (essentially) removed
        if (!found)
        {
        	bytesReceived = 0;
        }
        continue;
      }
      // if good CRC we actually have a packet so assemble our NAV200 message and return it to the application
      else
      {
          packet.header = receivedBuffer[0];
          packet.length = receivedBuffer[1];
          packet.mode = receivedBuffer[2];
          packet.function = receivedBuffer[3];
          packet.dataLength = packet.length-HEADER_SIZE-FOOTER_SIZE;
          memcpy(packet.data, receivedBuffer+4, packet.dataLength);
          packet.BCC = receivedBuffer[4+packet.dataLength];
    
          memmove(receivedBuffer, receivedBuffer+dataLength, bytesReceived-dataLength);
          bytesReceived-=dataLength;
          sn200->InQueue->ClearFilter();
          
          if(receivedBuffer[3]=='E'){
	          error.F0 = receivedBuffer[4];
	          error.F1 = receivedBuffer[5];
	          error.F2 = receivedBuffer[6];
	          error.F3 = receivedBuffer[7];
	  
	          //check out what the error is and it out
	          PrintErrorMsg();
	          return -2;
        }

        return 1;
      }
    }
    gettimeofday(&now,NULL);
    if ((now.tv_sec - start.tv_sec) * 1e6 + now.tv_usec - start.tv_usec > timeout_usec)
    {
      fprintf(stderr,"Timed out waiting for packet %d uSecs passed\n",static_cast<int>((now.tv_sec - start.tv_sec) * 1e6 + now.tv_usec - start.tv_usec));
      sn200->InQueue->ClearFilter();
      return -1;
    }
    usleep(1000);
  }
  sn200->InQueue->ClearFilter();
  return 0;
}


uint8_t Nav200::CreateCRC(uint8_t* data, ssize_t len)
{
  uint8_t result = 0;
  for (int ii = 0; ii < len; ++ii)
    result ^= data[ii];
  return result;
}
