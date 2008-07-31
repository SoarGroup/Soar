/*! \mainpage
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003
 *     Brian Gerkey
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \class PBSDriver
 * \brief Class for Hokuyo PBS 03JN plug-in driver
 * \par Authors:
 * Simon Kracht and Carsten Nielsen (based on sicks3000 by Toby Collett)
 * Aalborg University, Section for Automation and Control
 * \par Year:
 * 2006
 *
 * Plug-in driver for the Hokuyo PBS 03JN IR range finder.
 *
 * The PBSDriver controls the PBS 03JN IR range finder and interprets its data output.
 * The driver operation defaults are:
 * - The full range (190 degree) output is continously sampled and published (121 range readings)
 * - Sample time: 300 ms
 * - Serial port: dev/ttyS01
 * - Baud rate : 57600
 * - Pose of range finder related to the body (e.g. robot) on which it is fixed: [x,y,yaw] = [0,0,0]
 *
 * For detailed description of PBS 03JN refer to http://www.senteksolutions.com/ or http://www.hokuyo-aut.jp/
 *
 * \par Compile-time dependencies:
 *
 * - none
 *
 * \par Name:
 *
 * - pbs_driver
 *
 * \par Provides:
 *
 * - interface_laser
 *
 * \par Requires:
 *
 * - none
 *
 * \par Configuration requests:
 * \par
 *  \b - PLAYER_LASER_REQ_GET_GEOM
 *
 * \par Configuration file example:
 *
 * \code
 * driver
 * (
 * name "pbs_driver"
 * provides ["laser:0"]
 * )
 * \endcode
 *
 */

#include <assert.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> // for htons etc

#include <libplayercore/playercore.h>
#include <libplayercore/playertime.h>
#include <libplayercore/globals.h>


extern PlayerTime* GlobalTime;

#define DWORD unsigned int
#define DEFAULT_PBS_PORT "/dev/ttyS0"
#define NUMBER_OF_RANGE_READINGS 121



// The laser device class.
class PBSDriver : public Driver
{

  public:

   // Constructor
    PBSDriver(ConfigFile* cf, int section);
    ~PBSDriver();

    int Setup();
    int Shutdown();

    // MessageHandler
    int ProcessMessage(QueuePointer &resp_queue,
                       player_msghdr * hdr,
                       void * data);

  private:

    // Main function for device thread.
    virtual void Main();

 	// Get the time (in ms)
	DWORD getTickCount();

	bool sendDistAcq(unsigned char * pbsPackage);
	bool sendAuthCode(unsigned char * pbsPackage);
	bool SendAcqLinkCode(void);
	void closePBS();
	int parsePackage(DWORD timeout, unsigned char *pbsPackage);
	void PrintPackage(int length, unsigned char * pbsPackage);
	void PrintPackageMm(int length, float * pbsPackage);
	unsigned long crcbitbybitfast(unsigned char* p, unsigned long len);
	unsigned long reflect (unsigned long crc, int bitnum);
	int decodePBS(unsigned char *stuff, unsigned char *decodedstuff, int length);
	int encodePBS(unsigned char *uncoded, unsigned char *encoded, int length);
	int ConvertToM(unsigned char *pPbsDataHex, float *pPbsDataMm, int length);
	unsigned int combineTwoCharsToInt(unsigned char MSB, unsigned char LSB);
	bool InitializeCom(const char *ComNumber);
	int Write_Buffer(unsigned char *chars, DWORD dwToWrite);
	int Read_Single_Char(unsigned char *result);

    	//Holder for  incoming byte from the serial receive routine
	unsigned char chPBS;

	//Coded data from PBS without header (0x02) and footer (0x03)
	unsigned char pbsBuf[400];
	unsigned long checksum,checksum2;



	// temp variables
	unsigned char linkAuthSend[15], linkAuthSend2[17], linkAuthUn[12], linkAuthEn[10], lenAuth;
	unsigned char linkAuthCodeCRC[2], linkAuthCodeEn[7];
	unsigned char linkAuthCodeClose[8], linkAuthCodeCloseSend[15],linkAuthCodeCloseSend2[17];


	int ifd; /* File descriptor for the port */


  protected:


    // PBS pose in robot cs.
    double pose[3];
    double size[2];

    // Name of device used to communicate with the PBS
    const char *device_name;

    // PBS device file descriptor
    int PBS_fd;






    // storage for outgoing data
    player_laser_data_t data_packet;


};


/// A factory creation function.
/**
    Declared outside of the class so that it can be invoked without any object context (alternatively, you can
    declare it static in the class).  In this function, we create and return
    (as a generic Driver*) a pointer to a new instance of this driver.
*/
Driver* PBSDriver_Init(ConfigFile* cf, int section)
{
	//! Create and return a new instance of this driver
  	return((Driver*)(new PBSDriver(cf, section)));
}




/// A driver registration function.
/**
    Again declared outside of the class so that it can be invoked without
    object context.  In this function, we add the driver into the given
    driver table, indicating which interface the driver can support and how
    to create a driver instance.
*/
void PBSDriver_Register(DriverTable* table)
{
  table->AddDriver("pbs03jn", PBSDriver_Init);
}

// Error macros
#define RETURN_ERROR(erc, m) {PLAYER_ERROR(m); return erc;}


/// Constructor
/** Retrieve options from the configuration file and do any */
PBSDriver::PBSDriver(ConfigFile* cf, int section)
    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_LASER_CODE)
{

  // PBS data.
  memset(&data_packet,0,sizeof(data_packet));
  data_packet.min_angle = DTOR(-105.88);
  data_packet.max_angle = DTOR(105.88);
  data_packet.resolution = DTOR(1.75);
  data_packet.max_range = 3.5;
  data_packet.ranges_count = NUMBER_OF_RANGE_READINGS;
  data_packet.ranges = new float [NUMBER_OF_RANGE_READINGS];
  data_packet.intensity_count = 0;
  data_packet.intensity = NULL;
  data_packet.id = 1;


  // PBS geometry.
  this->pose[0] = 0;
  this->pose[1] = 0;
  this->pose[2] = 0;
  this->size[0] = 0.15;
  this->size[1] = 0.15;



  return;
}

PBSDriver::~PBSDriver()
{
  delete [] data_packet.ranges;
}

/// Set up the device.
/**
    Return 0 if things go well, and -1 otherwise.
*/
int PBSDriver::Setup()
{
  PLAYER_MSG1(2, "PBS initialising (%s)", this->device_name);

//! Setup the COM port
if(InitializeCom(DEFAULT_PBS_PORT))
			{
				printf("Serial port opened\n");

			}
			else
			{
				printf("Serial could not be opened\n");

			}

  PLAYER_MSG0(2, "PBS ready");

   //! Start the device thread; spawns a new thread and executes
  //! RobotinoDriver::Main(), which contains the main loop for the driver.
  StartThread();

  return 0;
}


/// Shutdown the device
int PBSDriver::Shutdown()
{
   //! Stop and join the driver thread
  StopThread();

 // CNIE: Her mangler en lukning af comporten

  PLAYER_MSG0(2, "PBS shutdown");

  return(0);
}

/// Process messages
int PBSDriver::ProcessMessage(QueuePointer &resp_queue,
                           player_msghdr * hdr,
                           void * data)
{
 //! Send a response if necessary, using Publish().
  //! If you handle the message successfully, return 0.  Otherwise,
  //! return -1, and a NACK will be sent for you, if a response is required.

   // If request for range data has been received
   if (Message::MatchMessage (hdr,PLAYER_MSGTYPE_DATA,
    			  PLAYER_LASER_DATA_SCAN,
    			  this->device_addr))
    {

       // Construct data package to be send
      player_laser_data_t data;
      data.ranges_count = NUMBER_OF_RANGE_READINGS;
      unsigned int i;
      for(i=0; i<data.ranges_count; i++)
      {
      	data.ranges[i] = data_packet.ranges[i];
      }

      data.intensity_count = 0;

      for(i=0; i<data.ranges_count; i++)
      {
      data.intensity[i] = 0; //CNIE: dette kunne med fordel gøres én gang i constructer
      }
      data.id = 0;

      // Publish the data package
      this->Publish (this->device_addr, resp_queue,
    	     PLAYER_MSGTYPE_RESP_ACK,
    	     PLAYER_LASER_DATA_SCAN,
    	     (void *) &data,
    	     sizeof (data), NULL);

      printf("\n driver sending range data \n");

      return (0);
    }

	// If request for geometry data has been received
	else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_LASER_REQ_GET_GEOM,
                                 this->device_addr))
  {
    // Construct geometry data package
    player_laser_geom_t geom;
    memset(&geom, 0, sizeof(geom));
    geom.pose.px = this->pose[0];
    geom.pose.py = this->pose[1];
    geom.pose.pyaw = this->pose[2];
    geom.size.sl = this->size[0];
    geom.size.sw = this->size[1];

    // Publish geometry data
    this->Publish(this->device_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LASER_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);
    printf("\nSending geometry package\n");
    return(0);
  }


  // Don't know how to handle this message.
  return(-1);
}


/// Main function for device thread
/** Due to the communication protocol between the PBS and a PC, the main function is implemented as a state machine.
*/
void PBSDriver::Main()
{


	enum state_t {AcqLinkCode, LinkAuth, DataAcq, Error};
	state_t state = AcqLinkCode;

//! Constants and Timeouts

	//Time the program was started
	DWORD ProgStartTime = getTickCount();

	//Sampletime
	DWORD sampletime = 300; //milliseconds

	//Time to sleep when to maintain a given samplerate
	DWORD SleepTime = 0; //milliseconds

	// The time when a distance data package last was received
	DWORD LastSampleTime = getTickCount();

	// The time when 'Link auth request' was sent
	DWORD LinkAuthSendTime = 0;

	// Timeout when listening for 'Acq. of link auth. code'-reply
	DWORD AcqLinkCodeTimeout = 300; //milliseconds

	// Timeout when listening for 'Distance Data Acq.'-reply (which is he meas. data)
	DWORD SendAcqDataTimeout = 300; //milliseconds

	//Time between the state is changed to "Send Link auth" in the state "Request Distance Data"
	DWORD LinkAuthSendTimeout = 2000; //milliseconds.

	// Max number of retries when sending 'Link Auth.' request
	static int LinkAuthRetriesMAX = 4;

	// Counter for determining the maximal number of times in a row a wrong Data package can be returned before it is considered a failure
	static int wrongDataPackageCnt = 0;

	// Counter for determining the maximal number of times a link auth can be sent before the state is changed back to AcqLinkCode
	static int LinkAuthRetries = 0;

	// Number of times a request for Acq. Link Code has be sent
	static int AcqLinkCodeCnt = 0;

	// Counter for determining the number of wrong distance data packages.
	static int ReceiveDataFailCnt = 0;

	//The state must not be changed if af Distance Data request has not been relied
	bool DistDataReceived = false;

     	unsigned int length, lengthMm;
     	unsigned char pbsPackage[400];

  // Decoded data from PBS converted to mm
  float pbsPackageMm[1024];



	//Holder for Link Auth. Code received from PBS
	unsigned char linkAuthCode[9];


 //Condition for accepting package as data
	#define DATAPACKAGE_RECEIVED ((pbsPackage[0] == 0xA2) && (pbsPackage[1] == 0x69))

	//Condition for accepting package as link auth reply
	#define LINKAUTHREPLY_RECEIVED ((pbsPackage[0] == 0xA0) && (pbsPackage[1] == 0x5A))
		//Condition for link status: connected
		#define IS_CONNECTED pbsPackage[2] == 1

	//Condition for accepting package as Link Auth code
	#define AUTHCODE_RECEIVED ((pbsPackage[0] == 0xA0) && (pbsPackage[1] == 0x69))

#define GETTIME getTickCount()-ProgStartTime




  //! The main loop; interact with the device here
  while(true)
  {

    		switch(state)
		{

		case AcqLinkCode:
			//Send a request for the Link Auth. code
			if(SendAcqLinkCode())
			{
				printf("Acquisition of LINK auth code transmitted, %d \n",AcqLinkCodeCnt);
				AcqLinkCodeCnt++;
				//Now we must receive the Auth code. First we listen for new packages until timeout,
				//then we check if the package received really is the auth code

				length = parsePackage(AcqLinkCodeTimeout, pbsPackage);
				if(length)
				{
					//Check if received is auth code
					if(AUTHCODE_RECEIVED)
					{
						//printf("%d: package is link Acquisition Authentication Code, which is: ", GETTIME);
						//PrintPackage(length, pbsPackage);
						//Save the Link Auth code
						int i;
						for(i=2 ; i<=10 ; i++)
						{
							linkAuthCode[i-2] = pbsPackage[i];
						}

						// Shift state to be able to send a link Acquisition Authentication Code reply

						state = LinkAuth;
						AcqLinkCodeCnt=0;

				}
					else
					{
						printf("%d: Package was not a auth. code, but: ",GETTIME);

						PrintPackage(length, pbsPackage);
					}

				}
				else
				{
					printf("A valid package was not returned before the timeout (%d ms) \n", AcqLinkCodeTimeout);
				}
			}
			else
			{
				printf("Error when sending Acquisition of LINK auth code\n");
			}

			break;

		case LinkAuth:
			// Checking if Authentication Code reply has been sent
			printf("%d: Sending Authentification Code reply %d/%d \n",GETTIME, LinkAuthRetries,LinkAuthRetriesMAX);
			// The Link Authentication Code reply is sent

			if(sendAuthCode(linkAuthCode))
			{
				printf("Link Authentication Code reply was sent\n");
			}
			else
			{
				printf("Something went wrong when sending link Authentication Code reply\n");
			}
			// Check if link is established until timeout occurs
			length = parsePackage(AcqLinkCodeTimeout, pbsPackage);
			if(length)
			{
				//Check if received is "link established" package
				if(LINKAUTHREPLY_RECEIVED)
				{
					LinkAuthSendTime = getTickCount();//Used to calc time since last Link Auth (must be sent at least each third seconed)
					// Check if the package contains connected or disconnected
					if(IS_CONNECTED)
					{
						LinkAuthRetries = 1;
						//We can now use distance data acquisition
						state = DataAcq;
						wrongDataPackageCnt = 0;
						printf("Link connected\n");

					}
					else
					{
						printf("%d: package is 'Link level': DISCONNECTED\n", GETTIME);
						state = AcqLinkCode;
						LinkAuthRetries = 1;
					}

				}
				else
				{
					printf("%d Package was NOT 'link established',but  ",GETTIME);
					// BEGIN HACK: Sometimes we receive data even when we should receive Link Auth...
					//we dont want to throw good data away...
					if(DATAPACKAGE_RECEIVED)
					{
						printf("data. We publish it anyway...\n");
//						IPCpublishPBSdata(); //Udkommenteret af CNIE (IPC)

					}
					//END HACK
					else//we have received some strange package...
						PrintPackage(length, pbsPackage);

					if(LinkAuthRetries==LinkAuthRetriesMAX)
					{
						printf("MAX number of retries. Trying a request for the Link Auth. code\n");
						LinkAuthRetries = 1;
						state = AcqLinkCode;
					}
					else
					{
						if (state==LinkAuth)
						{
							printf("Trying to send a Link Auth req again...\n");
							LinkAuthRetries++;
						}
						else
						{
							LinkAuthRetries = 0;
						}

					}

				}
			}
			break;

		case DataAcq: //State is "data acquisition". Edges to "Link Authorisation"
			//Ugly way to implement a sampletime. Note that getTickCount has a somewhat low precision
			SleepTime = getTickCount()-LastSampleTime;
			if(!(SleepTime > sampletime))
			{

				//printf("Sleeping in: %d\n",sampletime-SleepTime);
				usleep((sampletime-SleepTime)*1000);

			}
			DistDataReceived=false;
			//Now we send a request for distance data
			if(sendDistAcq(pbsPackage))
			{
				//printf("'Request for Distance Data' was sent\n");
					//Store time to maintain a samplerate
					LastSampleTime = getTickCount();

			}
			else
			{
				printf("Something went wrong when sending 'Request for Distance Data' \n");
			}
			// Check if distance data is received until timeout occurs
			length = parsePackage(SendAcqDataTimeout, pbsPackage);

      			if(length)
			{

				//Check if received is data package
				if(DATAPACKAGE_RECEIVED)
				{
					DistDataReceived = true;
					printf("%d - Data ",getTickCount()-ProgStartTime); //
				//	PrintPackage(length, pbsPackage);
					lengthMm = ConvertToM(pbsPackage, pbsPackageMm, length);
      				//	PrintPackageMm(lengthMm, pbsPackageMm);

					//copy PBS data to data packet
      					unsigned int i;
      					for(i=0; i < lengthMm; i++)
      					{
	      					if(pbsPackageMm[i]<data_packet.max_range)
	      					{
	      						data_packet.ranges[i] = pbsPackageMm[i];
						}
						else
						{
						data_packet.ranges[i] = data_packet.max_range;
						}
      					}

      				//	Check data packet
      				//	for(i=0; i < PLAYER_LASER_MAX_SAMPLES; i++)
      				//	{
	      			//		printf("ranges[%d]: %.3f ",i,data_packet.ranges[i]);
	      			//	}

					data_packet.id ++;
				        this->Publish(this->device_addr,
		                        PLAYER_MSGTYPE_DATA,
		                        PLAYER_LASER_DATA_SCAN,
		                        (void*)&data_packet, sizeof(data_packet), NULL);
					printf("Data package published: %d \n", data_packet.id);


					wrongDataPackageCnt = 0;
					// State is maintained

				}
				else
				{
					printf("%d package was NOT data. Trying again\n",GETTIME);
					PrintPackage(length, pbsPackage);
					if(wrongDataPackageCnt == 5)
					{
						printf("To many wrong Data acquisition Retries in a row - Trying Link Auth.\n");
						state = LinkAuth;
					}
					wrongDataPackageCnt++;

				}
			}
			// If to many wrong packages has been received in a row, we change the state to Link Auth
			else
			{
				ReceiveDataFailCnt++;
				if(ReceiveDataFailCnt==3)
				{
					state = LinkAuth;
					ReceiveDataFailCnt = 0;
				}
			}
			// Check time since 'Link auth' package was sent
			// printf("time since link auth was sent %d\n",getTickCount()-LinkAuthSendTime);

			if(((getTickCount()-LinkAuthSendTime) > LinkAuthSendTimeout) && (DistDataReceived==true))
			{
				state = LinkAuth;
			}
			break;

		case Error:
			exit(0);
			break;
		}



    // test if we are supposed to cancel
    pthread_testcancel();

    // process any pending messages
    ProcessMessages();


  }
}





int PBSDriver::parsePackage(DWORD timeout, unsigned char * pbsPackage)
{

	int len = 0; //Number of bytes returned by decodePBS (defines the length of actual data in pbsPackage)
//	int lenMm = 0;
  int cnt = 0;//Number of bytes received (6-bit format)
	bool IncommingPackage = false;
	DWORD StartTime = getTickCount();//Returns the time in milliseconds since last reboot (max 49 days)



	while((getTickCount()-StartTime) < timeout)
	{
		//read one byte
		if(Read_Single_Char(&chPBS ))
		{
			 printf("Theres a byte: 0x%x \n",chPBS);

			// 0x02 is the "new package" keyword
			if(chPBS==0x02)
			{
				//printf("New package \n");
				cnt = 0;
				IncommingPackage = true;
			}
			//0x03 is the "end of package keyword
			else if((chPBS==0x03) && (IncommingPackage == true))
			{
				printf("Decoding package of size %d\n",cnt);
				len = decodePBS(pbsBuf, pbsPackage, cnt);


				while((Read_Single_Char(&chPBS))==1 )
				{
					//printf("Crap.\n");
					//usleep(1000);

				//printf("Something was still in the readbuffer!\n");
				}
				return len;
				//printf("RX - %.2d: " , getTickCount()-ProgStartTime);

			}
			else
			{
				if (IncommingPackage == true)
				{
					pbsBuf[cnt] = chPBS;
					// Sorting of packages. We are only interested in link auth code, link auth, and Dist data
					if ( (cnt==0) && !(chPBS==0x48) )
					{
						IncommingPackage = false;
					}
					cnt++;

				}
			}
		}
		else
		{
			return 0;
		}

	}
	return 0;
}


/// Function for printing the decoded output of a package
/** Prints the output to the screen for debugging purposes */
void PBSDriver::PrintPackage(int length, unsigned char * pbsPackage)
{
	int i;

	printf(" Package: \n");

	// Print decoded data to screen
    for(i=0;i<length;i++)
	{
	    printf("0x%x ", pbsPackage[i]);
	}

   printf("\nTest length pbspackage: %d\n\n", length);
   printf("\n-----------------------------------------\n");

}


/// Function for printing the decoded output of a package in milimeters
/**	Prints the output to the screen for debugging purposes*/
void PBSDriver::PrintPackageMm(int length, float * pbsPackageMm)
{
	int i;


  printf("Package in mm: \n");

    // Print data in mm to screen*/
    for(i=0;i<length;i++){
	    	printf("MM[%d]: %.3f ",i, pbsPackageMm[i]);
 }
printf("\n-----------------------------------------\n");

}

/// Function for acquiring distance data from PBS
/** When distance data is needed this function is invoked. Note that the link
to the PBS must be previously set up*/
bool PBSDriver::sendDistAcq(unsigned char * pbsPackage){

	// This is the distance acquisition command. Always looks like this
  	static unsigned char distanceAcq[8] = {0x02,0x48,0x46,0x46,0x28,0x38,0x40,0x03};

	if(Write_Buffer(&distanceAcq[0], 8)==1)
	{

		return true;
	}
	else
	{
		return false;
	}
}


/// Function for sending the authentication code to PBS
/** Invoked when the authentication code from PBS is received in order to set up the com link. Note that an acquisiton
of authentication code must be send previously. The code is calculated and transmitted back to the PBS*/
bool PBSDriver::sendAuthCode(unsigned char * linkAuthCode)
{
	bool status=1;
	int i=0;
	unsigned char temp2;
	unsigned char temp1;

	// Calculating the cheksum needed to make the link auth code
	checksum =  crcbitbybitfast(linkAuthCode, 8);

	temp1 = checksum & 0x00FF;
	temp2 = (checksum & 0xFF00) >> 8;

	// Calculate the new checksum with the link auth code in it
	linkAuthCodeEn[0] = 0xA0;
	linkAuthCodeEn[1] = 0x5A;
	linkAuthCodeEn[2] = 0x01;
	linkAuthCodeEn[3] = temp1;
	linkAuthCodeEn[4] = temp2;

	// the following is to test if the connection can be closed correctly
	linkAuthCodeClose[0] = 0xA0;
	linkAuthCodeClose[1] = 0x5A;
	linkAuthCodeClose[2] = 0x00;
	linkAuthCodeClose[3] = temp1;
	linkAuthCodeClose[4] = temp2;

	checksum = crcbitbybitfast(linkAuthCodeEn,5);
	checksum2 = crcbitbybitfast(linkAuthCodeClose,5);

	// This HAS to be reversed
	temp1 = checksum & 0x00FF;
	temp2 = (checksum & 0xFF00) >> 8;
	// Plug it in the last two bytes in reverse order
	linkAuthCodeEn[5] = temp1;
	linkAuthCodeEn[6] = temp2;

	// This is to calculate the close command
	temp1 = checksum2 & 0x00FF;
	temp2 = (checksum2 & 0xFF00) >> 8;
	linkAuthCodeClose[5] = temp1;
	linkAuthCodeClose[6] = temp2;

	lenAuth = encodePBS(linkAuthCodeEn, linkAuthSend, 7);
    	lenAuth = encodePBS(linkAuthCodeClose,linkAuthCodeCloseSend,7);

	linkAuthSend2[0] = 0x02;
	for(i=0;i<=lenAuth;i++)
	{
		linkAuthSend2[i+1] = linkAuthSend[i];
	}
	linkAuthSend2[i] = 0x03;

	linkAuthCodeCloseSend2[0]=0x02;
	for(i=0;i<=lenAuth;i++)
	{
		linkAuthCodeCloseSend2[i+1] = linkAuthCodeCloseSend[i];
	}
	linkAuthCodeCloseSend2[i] = 0x03;
    	Write_Buffer(&linkAuthSend2[0], 12);

	return status;
}


/// Function for sending acquisition of link code
/** When sent to the PBS the PBS responds with the link code */
bool PBSDriver::SendAcqLinkCode(void)
{
	// First to to send the first Acquisition of LINK auth code, this is always fixed
	// The following line is link Acq command that the pc has to send to the PBS.
  	static unsigned char linkAcqEn[8] = {0x02,0x48,0x26,0x44,0x58,0x34,0x30,0x03};
	Write_Buffer(&linkAcqEn[0], 8);
	return true;
}


/// Function for closing PBS
/** Terminates the link between PC and PBS*/
void PBSDriver::closePBS()
{
    printf("sending close command\n");
    Write_Buffer(linkAuthCodeCloseSend2, 12);
    printf("close command sent\n");
}


/// Function for obtaining the present time
DWORD PBSDriver::getTickCount() {
    tms tm;
    return times(&tm)*1000/sysconf(_SC_CLK_TCK);
}

/// Function for closing reflecting the lower 'bitnum' bits of crc
/** Used for authentication code generation */
unsigned long PBSDriver::reflect (unsigned long crc, int bitnum) {

    unsigned long i, j=1, crcout=0;

	for (i=(unsigned long)1<<(bitnum-1); i; i>>=1) {
	    if (crc & i) crcout|=j;
	    j<<= 1;
	}
	return (crcout);
}

/// Function for calculating crc
/** Used for authentication code generation */
unsigned long PBSDriver::crcbitbybitfast(unsigned char* p, unsigned long len) {

// CRC parameters (default values are for CRC-32):

    const int order = 16;
    const unsigned long polynom = 0x1021;
    const unsigned long crcxor = 0x0000;
    const int refin = 1;
    const int refout = 1;
    unsigned long crcinit_direct = 0;


    unsigned long crcmask;
    unsigned long crchighbit;
    unsigned long i, j, c, bit;
    unsigned long crc = crcinit_direct;

    crcmask = ((((unsigned long)1<<(order-1))-1)<<1)|1;
    crchighbit = (unsigned long)1<<(order-1);

    for (i=0; i<len; i++) {

	c = (unsigned long)*p++;
	if (refin) c = reflect(c, 8);

	for (j=0x80; j; j>>=1) {

	    bit = crc & crchighbit;
	    crc<<= 1;
	    if (c & j) bit^= crchighbit;
	    if (bit) crc^= polynom;
	}
    }

    if (refout) crc=reflect(crc, order);
	crc^= crcxor;
	crc&= crcmask;

	return(crc);
}


/// Function for decoding PBS data
/** Decodes the PBS package according to the PBS/PC communication protocol */
  int PBSDriver::decodePBS(unsigned char *stuff, unsigned char *decodedstuff, int length){

    int i,j,k;
    j=0;
    k=0;

    // First we clean up the list by subtracting the 0x20
    for (i=0; i<length; i++){

	if (((stuff[i] & 0x40) >> 6) == 1){
	    stuff[i] = (stuff[i] & 0xBF);
	    stuff[i] = (stuff[i] | 0x20);
	    stuff[i] = stuff[i] << 2;
	}
	else{
	    stuff[i] = stuff[i] & 0xDF;
	    stuff[i] = stuff[i] << 2;
	}
    }


// Then we decode the list and put everything together
    for (i=0; i<=length; i++){

	if (j == 0){
	    if(i==length-1){
		decodedstuff[k] = stuff[i] +((stuff[i+1] & 0xC0 >> 6));
		break;
	    }

	    //printf("the stuff for the first part %x \n",stuff[i]);
	    decodedstuff[k] = stuff[i] + ((stuff[i+1] & 0xC0) >> 6);
	    //printf("the decoded stuff for the first part %x \n",decodedstuff[k]);
	    k=k+1;
	}

	if (j == 1){

	    if(i==length-1){
		decodedstuff[k] = (((stuff[i] & 0x3C)) << 2)  + ((stuff[i+1] & 0xF0) >> 4);
		break;
	    }

	    decodedstuff[k] = (((stuff[i] & 0x3C)) << 2)  + ((stuff[i+1] & 0xF0) >> 4);
	    //printf("the decoded stuff for the second part %x \n",decodedstuff[k]);
	    k=k+1;
	}

	if (j == 2){
	    if(i==length-1){
		decodedstuff[k] = ((stuff[i] & 0x0C) << 4) + ((stuff[i+1] & 0xFC) >> 2);
		break;
	    }

	    decodedstuff[k] = ((stuff[i] & 0x0C) << 4) + ((stuff[i+1] & 0xFC) >> 2);
	    //printf("the decoded stuff for the third part %x \n",decodedstuff[k]);
	    k=k+1;
	}

	j=j+1;

	if (j == 4){
	    j = 0;
	}

	if(j ==3){}
    }

// Return the length of the decoded list
    return k;

}

/// Function for encoding data to be send to PBS
/** Encodes the PBS package according to the PBS/PC communication protocol */
int PBSDriver::encodePBS(unsigned char *uncoded, unsigned char *encoded, int length) {

    int i,j,k;

    j=0;
    k=0;
    length = length +1;

    for (i=0;i<=length;i++){

	if (j==0){

	    if(i==length){
		break;
	    }
	    encoded[k] = ((uncoded[i] & 0xFC) >> 2) + 0x20;
	    k=k+1;
	}

	if (j==1){

	    if(i==length){
		encoded[k] = ((uncoded[i-1] << 4) & 0x30) + 0x20;
		break;
	    }
	    encoded[k] = (((uncoded[i-1] & 0x03) << 4) + ((uncoded[i] & 0xF0) >> 4)) + 0x20;
	    k=k+1;
	}

	if (j==2){

	    if(i==length){
		encoded[k] = ((uncoded[i-1] << 2) & 0x3c) + 0x20;
		break;
	    }
	    encoded[k] = (((uncoded[i-1] & 0x0F) << 2) + ((uncoded[i] & 0xC0) >> 6)) + 0x20;
	    k=k+1;

	}

	if (j==3){
	    if(i==length){
		encoded[k] = ((uncoded[i-1] & 0x3F) + 0x20);
		break;
	    }
	    encoded[k] = (uncoded[i-1] & 0x3F) + 0x20;
	    k=k+1;
	}

	j=j+1;

	if (j==4){
	    j=0;
	    i=i-1;
	}

    }
    return k;
}


/// Function for converting PBS data from hex values to mm
int PBSDriver::ConvertToM(unsigned char *pPbsDataHex, float *pPbsDataMm, int length){
int i = 0, k=1, LengthConv =0, OffsetInit = 2, OffsetEnd = 4;

for(i=OffsetInit+1 ; i<length-OffsetEnd ; i = i+2){
       pPbsDataMm[i-k-OffsetInit] = ((float)combineTwoCharsToInt(pPbsDataHex[i],pPbsDataHex[i-1]))/1000;
       LengthConv = i-k;
       k++;
}

printf("Data converted to mm. Length of new package is: %d shorts\n",LengthConv);
return (LengthConv);
}

/// Function for combining two chars to an integer
unsigned int PBSDriver::combineTwoCharsToInt(unsigned char MSB, unsigned char LSB){
        return (MSB<<8) | LSB;
}


/// Function for setting up the com port
bool PBSDriver::InitializeCom(const char *Port_Name)
{
	//char *Port_Name = ComNumber; // Serial port name.

	ifd = open(Port_Name, O_RDWR | O_NOCTTY | O_NDELAY);

	if (ifd == -1)
	{
		return 0;
		perror("open_port: Unable to open serial port");
	}
	else
	{
		fcntl(ifd, F_SETFL, 0);

		struct termios options;
		// Get the current options for the port...
		tcgetattr(ifd, &options);
		// Set the baud rates to 57600...
		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);
		// Enable the receiver and set local mode...
		options.c_cflag |= CLOCAL | CREAD;

		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS7;

		options.c_iflag = IGNPAR;
		options.c_oflag = 0;

		options.c_lflag = 0;

		// Char read timer in with ts = 0.1s
		options.c_cc[VTIME]    = 1;   /* inter-character timer unused */
        	options.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */


		tcflush(ifd, TCIFLUSH);
		// Set the new options for the port...
		tcsetattr(ifd, TCSANOW, &options);

		return 1;
    }

}

/// Function for writing data to the com port
int PBSDriver::Write_Buffer(unsigned char *chars, DWORD dwToWrite) {
	int n = write(ifd, chars, dwToWrite);
	if (n < 0) {
		fputs("write failed!\n", stderr);
		return 0;
	}
	return 1;
}

/// Function for reading data from the com port
int PBSDriver::Read_Single_Char(unsigned char *result) {
	int iIn = read(ifd, result, 1);

	if (iIn < 0) {
		if (errno == EAGAIN) {
			printf("SERIAL EAGAIN ERROR\n");
			return 0;
		} else {
			printf("SERIAL read error %d %s\n", errno, strerror(errno));
			return 0;
		}
	}
	return iIn;
}


