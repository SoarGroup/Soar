/** Basic interface code for the NAV200, player interface provided in
sicknav200.cc

This driver interface code was written by Kathy Fung, the player
interfaces was later added by Toby Collett.

*/

#ifndef _NAV200_H
#define _NAV200_H

#include <libplayercore/playercore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
//#include <stdint.h>


#define STX     0x02
#define MAXLEN	255
#define BUFFER_SIZE 256
#define HEADER_SIZE 4
#define FOOTER_SIZE 1

typedef struct Nav200Command
{
	uint8_t header;
	uint8_t length;
	uint8_t mode;
	uint8_t function;
	uint8_t data [MAXLEN-HEADER_SIZE-FOOTER_SIZE+1];
	int dataLength;
	uint8_t BCC;
}Nav200Command;

// typedef struct ReflectorInfo
// {
//   uint8_t layer;
//   uint8_t number;
// }ReflectorInfo;

typedef struct PositionXY
{//position is in mm
  int x;
  int y;
}PositionXY;


typedef struct ReflectorData
{
  uint8_t layer;
  uint8_t number; // reflector number
  PositionXY pos;
}ReflectorData;


typedef struct LaserPos
{
  PositionXY pos; // position of the laser scanner
  short orientation;
  uint8_t quality;
  uint8_t number; // number of reflectors used
}LaserPos;

typedef struct ErrorBytes
{
  uint8_t F0; // function byte of the last command
  uint8_t F1; // error class
  uint8_t F2; // error group
  uint8_t F3; // error specification
}ErrorBytes;




class Nav200
{
public:

  friend class SickNAV200;
  Nav200();
  ~Nav200();

  int Initialise(Driver* device, Device* opaque, player_devaddr_t opaque_id);
  int Terminate();

  int ProcessData();

  // standby mode
  bool EnterStandby();
  int GetVersionNumber();
  char* GetVersionString(); //String pointer return is only valid till the next request to Nav200
  short GetDeviceSerial();
  bool rotateDirection(uint8_t direction);
  bool GetReflectorPosition(uint8_t layer, uint8_t number, PositionXY & reflector);
  bool ChangeReflectorPosition(uint8_t layer, uint8_t number, int newX, int newY);
  bool InsertReflectorPosition(uint8_t layer, uint8_t number, int X, int Y);
  bool DeleteReflectorPosition(uint8_t layer, uint8_t number, PositionXY & reflector);

  // read and set reflector radii
  int GetReflectorRadius(uint8_t layer);
  bool SetReflectorRadius(uint8_t layer, uint8_t radius);

  // mapping mode
  bool EnterMapping();
  int StartMapping(uint8_t layer, int X, int Y, short orientation, uint8_t radius);
  int StartMappingMeasurement(uint8_t layer, uint8_t scans, int X, int Y, short orientation, uint8_t radius);
  int StartNegativeMappingMeasurement(uint8_t layer, uint8_t scans, int X, int Y, short orientation, uint8_t radius);
  bool MappingPosition(uint8_t layer, uint8_t number, PositionXY & reflector);

  // positioning mode
  bool EnterPositioning();
  bool EnterPositioningInput(uint8_t NumberOfMeasurements);
  bool GetPositionAuto(LaserPos & laserPosition);
  bool GetPositionSpeed(short speedX, short speedY, LaserPos & laserPosition);
  bool GetPositionSpeedVelocity(short speedX, short speedY, short velocity, LaserPos & laserPosition);
  bool GetPositionSpeedVelocityAbsolute(short speedX, short speedY, short velocity, LaserPos & laserPosition);
  bool ChangeLayer(uint8_t layer);
  bool ChangeLayerDefPosition(uint8_t layer, int X, int Y, short orientation);
  bool SetActionRadii(int min, int max);
  bool SelectNearest(uint8_t N_nearest);

  // upload mode
  bool EnterUpload();
  bool GetUploadTrans(uint8_t layer, ReflectorData & reflector);
  // download mode
  bool EnterDownload();
  bool DownloadReflector(uint8_t layer, uint8_t number, int X, int Y);


protected:
  // serial port descriptor
  int fd;
  struct termios oldtio;

  uint8_t receivedBuffer[BUFFER_SIZE];
  int bytesReceived;
  Nav200Command packet;
  ErrorBytes error;

  void PrintErrorMsg(void);

  int ReadFromNav200(int timeout_usec=5000000);
  int WriteCommand(char mode, char function, int dataLength, uint8_t * data);
  uint8_t CreateCRC(uint8_t* data, ssize_t len);
  
  // SickNav200 Driver info
  Driver *sn200;
  
  // Opaque info - for setting filter
  Device *opaque;
  player_devaddr_t opaque_id;
  
};



#endif
