/**
  laser.h  V 2.0 -> RS4Leuze_laser.cpp

 Modified by Ernesto Homar Teniente Aviles
 Date 08 May 2007
*/

#ifndef RS4Leuze_laser_h
#define RS4Leuze_laser_h

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace std;

#define MAX_SCAN_POINTS 529


typedef struct RS4Leuze_laser_readings { 
		double Reading[MAX_SCAN_POINTS];
} RS4Leuze_laser_readings_t;

//classes
/**
  Claser implements functions to read data from a laser scanner ROTOSCAN RS4-4 (Leuze corp.) connected to a serial port. 
*/
class Claser
{
	private:
		char *portName; /**<Serial Port where laser is connected*/
		int serialFD; /**<Serial port file descriptor*/
		termios ttyset; /**<termios variable to configure serial port*/
		fd_set rfds;
		int selectResult; 		/**<Laser Message fields*/
		unsigned char byte;
		unsigned char checksum;
		unsigned char option1;
		long unsigned int scan_number;
		unsigned int output_start;
		unsigned int output_stop;
		unsigned int scanedPoint;
		unsigned char controlByte;
		unsigned int points_to_scan;
		struct timeval tv;/**<termios variable time interval*/
		timeval timeStamp; /**<Time in microseconds resolution*/

	public:
		
		//Claser(ClogMsg *lgMsg, bool *laser_ON, char dir_name[80]); /**<Opens serial port*/
		Claser(int scan_points); /**<Opens serial port*/
		~Claser(); 
		void closeLaser();/**<Closes serial Port and data file*/
		unsigned char readByte(); /**<Reads one byte and updates checksum message*/
		void sync();		
		//void readScan(); /**<Sets to scanData array values of last laser scanner*/
		void writeConfig(); /**<Write configuration parameters to laser scanner device*/
		int  scanRead(); /**<reads one scan and puts it in scanData array*/
		void runLaser(); /**<Return the scan reading from the laser*/
		void closeSerial(); /**<Closes serial Port */
		void openSerial(bool *laser_ON,int Baud_rate, const char * Port);  /**<Opens serial Port and get the default paarameters or those given in the .cfg file*/
		RS4Leuze_laser_readings_t scanData;

};

#endif


