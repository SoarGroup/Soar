/**
 laser.cpp V 2.0 -> RS4Leuze_laser.cpp
 
 Modified by Ernesto Homar Teniente Aviles
 Date 20 APril 2007
*/

//include
#include "RS4Leuze_laser.h"
#include <unistd.h>
#include <string.h>
#include <libplayercore/playercore.h>
/**
  Default constructor.
*/
//Claser::Claser(ClogMsg *lgMsg, bool *laser_ON, char dir_name[80]) //default constructor. 
Claser::Claser(int scan_points) //default constructor. 
 {
	points_to_scan = scan_points;

}

/**
  Destructor
*/
Claser::~Claser() 
{
	//close port
	closeSerial();
}



void Claser::closeSerial()
{
	close(serialFD);
}

void Claser::openSerial(bool *laser_ON, int Baud_rate, const char * Port)
{
	serialFD = open(Port, O_RDWR|O_NOCTTY);
	if (serialFD<0)
	{
		PLAYER_ERROR("Claser, Error opening serial port");
		*laser_ON=0;
		return;
	}

	// Configure Serial Port: termios  settings: 57600(default), No Parity, 8 data bits, 1 stop Bit (8N1)
	// values of masks in /usr/include/bits/termios.h
	tcgetattr(serialFD,&ttyset);
	ttyset.c_cflag = ( Baud_rate | CLOCAL | CREAD | CS8 ); 
	ttyset.c_iflag = ( IGNBRK ); //Ignores break condition on input
	ttyset.c_oflag = 0x0;
	ttyset.c_lflag = 0x0;

	// Set configuration immediately.
  	if (tcsetattr(serialFD, TCSANOW, &ttyset)<0) 
	{
		PLAYER_ERROR("Claser, Error opening serial port");
		*laser_ON=0;
		return;
	}

	else
	


	FD_ZERO(&rfds);			// Initialize the read set to zero
	FD_SET(serialFD, &rfds);	// Turn on the read set
	// set timer
	tv.tv_sec = 1;	
	tv.tv_usec = 0;	
	
	//Flush both pending input and untransmitted output.
 	tcflush(serialFD, TCIOFLUSH);


}
/**
  Reads one byte from serialFD and updates checksum
*/
unsigned char Claser::readByte()
{
	unsigned char localByte;
	read (serialFD, &localByte, 1);
	checksum = checksum ^ localByte; 
	return localByte;
}

/**
  sync function synchronizes with the start of the laser data
*/
void Claser::sync()
{
	int num_zeroes = 0;
		
	// We must read three consecutive 0x00 for the end of the message
	while (num_zeroes < 3)
	{    
		read(serialFD, &byte, 1);
		if(byte == 0x00) num_zeroes++; 
		else num_zeroes = 0; 
	}
	// We are now, for a short time, at the beginning of the message.	
}

/**
  scanRead reads a whole message of a laser scan, loads it to scanData vector and prints it to laserDataFile. If success returns 0 and returns 1 if failure.
*/
int Claser::scanRead()
{
	unsigned int ii;
	
	//******STEP 1: Reading message header	
	for (ii=0; ii<2; ii++)
	{
		byte = readByte();
		//cout<<"Header: "<< byte << ";" <<endl;
		if(byte != 0x00)
		{
			//cout << " Claser::scanRead(STEP 1), Error reading Laser message header" << endl;
			PLAYER_ERROR("Error reading Laser message header");
			return 1;
		}
	}
	readByte(); //Reads but doesn't analyze command byte
	option1=readByte(); //Reads Option1;			
	if (option1 & 0x03 > 1)
	{
		byte=readByte(); //Reads Option2	
	}
	if (option1 & 0x03 > 2)
	{;
		byte=readByte(); //Reads Option3
	}
	// There's a bit that determines the existance of the password field
	if ((option1 & 0x20) != 0x00)
	{
		for (ii=0; ii<8; ii++)
		{
			byte=readByte(); //Reads Password
		}
	}
	//******STEP 2: Reading data header
	scan_number = 0;
	for (ii=0; ii<8; ii++)
	{
		if (ii%2 == 0)
		{
			scan_number = scan_number * 256 + readByte();
		}
		else
		{
			byte=readByte();
			if (byte != 0xFE)
			{
				//cout << "Claser::scanRead(STEP 2), Error reading Laser message header" << endl;
				PLAYER_ERROR("Error reading Laser message header");
				return 1;
			}
		}
	}
	byte=readByte();//Resolution byte
	output_start=readByte()*256;//ouput Start
	output_start+=readByte();
	output_stop=readByte()*256;//ouput Stop
	output_stop+=readByte();

	//******STEP 3: Reading laser scan /*data*/

	//cout << "points_to_scan "<< points_to_scan << std::endl;
	for (ii=0; ii<2*points_to_scan; ii++)
	{

		if (ii%2 == 0)
		{
			// 3.1: reading the first Byte of two from de scaned point
			scanedPoint = (readByte() * 256);	
		}
		else
		{
			// 3.2: reading the second Byte of two from de scaned point, and must mask the last bit
			scanedPoint += readByte() & 0xFE;
			// 3.3 joins the two Bytes froms the scaned point, and they are converted from mm to meters
			scanData.Reading[(ii-1)/2] = (double)(scanedPoint/1000.0);
		}
	}
	
	//******STEP 4: Reading Control Byte for checksum and ending message;
	read(serialFD, &controlByte, 1);//just read without checksum
	
	/*********************!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*****/
	
        return(0);
}


void Claser::runLaser()
{
	//Claser *thisLaser = (Claser*)thisPnt;
	//zeroTimeStamp=clock(); //Initializes time stamp 
	FD_ZERO(&rfds);			// Initialize the read set to zero
	FD_SET(serialFD, &rfds);	// Turn on the read set
	// set timer
	tv.tv_sec = 1;	
	tv.tv_usec = 0;	
	if(select(serialFD+1, &rfds, NULL, NULL, &tv))
	{
		this->sync(); //Synchronizes
		this->checksum=0x00; //Reset Checksum
		this->scanRead(); //Read all scan Message
		
	}
	else
	{
        	PLAYER_ERROR("Laser disconnected!!!!!!!!!!!!!!!!");
	}
}

