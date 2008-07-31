/* -*- Mode: C; tab-width: 2; c-basic-offset: 2; indent-tabs: t -*- */

/**
	* KartoWriter.
	* Copyright (C) 2007, SRI International.  All Rights Reserved.
    *
	* This program is free software; you can redistribute it and/or
	* modify it under the terms of the GNU General Public License
	* as published by the Free Software Foundation; either version 2
	* of the License, or (at your option) any later version.
    *
	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.
    *
	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
    *
	*  Author(s): Regis Vincent (vincent@ai.sri.com)
	**/
/** @ingroup drivers */
/** @{ */
/** @defgroup driver_logger
	* @brief Karto logger plugin

	@par Compile-time dependencies

	- none

	@par Provides

	- @ref log

	@par Requires

	- laser sonar position2d

	@par Configuration requests

	- autorecord (boolean)
		- default: 1 (enable)
		- activate the logging at startup, to disable it, use 0

	- gzip (boolean)
		- default: 0 (disable)
		- compress the log file

	@par Configuration file options
	- file (string)
	- Default: "output.xml"
	- where to log

	@par Configuration requests

	- PLAYER_LOG_SET_WRITE_STATE
	- PLAYER_LOG_GET_STATE
	- PLAYER_LOG_SET_FILENAME (not yet implemented)

	@par Example

	@verbatim
	driver
	(
	name "readlog"
	filename  "jwing.log"
	speed 1
	provides ["log:0" "laser:0" "laser:1" "position2d:0"]
	autoplay 1
	alwayson 1
	)

	driver
	(
	name "kartowriter"
	plugin "libkartowriter_plugin"
	requires [ "laser:0" "laser:1" "position2d:0"]
	provides ["log:1" ]
	file "output.xml"
	alwayson 1
	)
@endverbatim

	@author Regis Vincent

*/
/** @} */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <libplayercore/playercore.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <libplayercore/addr_util.h>
#include "sys/types.h"	/* for time_t */
#include "time.h"	/* for struct tm */

#include <list>
#define ID_MAX_SIZE 1024
#define DRIVER_NAME "kartowriter"
#define DEFAULT_UID "http://unknown"
#define MAX_HOSTNAME 256

using namespace std;

class LogDevice
	{
private:

public:
	Device* pDevice;
	char* pUid;
	player_devaddr_t addr;

	// constructor
	LogDevice(const player_devaddr_t& playerAddr);
	//destructor
	virtual ~LogDevice();
	//prints the Device unique ID
	const char* getUID();
};

LogDevice::LogDevice(const player_devaddr_t& playerAddr) : pDevice(NULL), pUid(NULL), addr(playerAddr)
	{
	pUid = strdup(getUID());
	}

LogDevice::~LogDevice() {
	free(pUid);
}


//
// Main Class
//

class KartoLogger : public Driver
{
private:

	list<LogDevice*> devices;

	char kartoFilename[MAX_FILENAME_SIZE];
	FILE* kartoFile;

	bool debug; // Debug flag to print more message
	bool enableLogger; // Enabling flag
	bool compress; // Compress flag  (use gzip at the end or not)
	player_pose2d_t currentPose;

	// When we start logging
	double startTime;

	// Side effect set and unset the file descriptor kartoFile.
	int OpenLog();
	int CloseLog();

	// Specific functions to write the correct XML format depending
	// of the data
	void WritePosition(player_msghdr_t* hdr, player_position2d_data_t* data);
	void WriteLaserScan(player_msghdr_t* hdr, player_laser_data_t* scan);
  void WriteLaserScanPose(player_msghdr_t* pHdr, player_laser_data_scanpose_t* pScan);
	void WriteSonarScan(player_msghdr_t* hdr, player_sonar_data_t* scan);

	// Print the geometry (and all specific info) of any LogDevices
	int WriteGeometry(LogDevice* pDev);
	void ComputeQuaternion(double heading, double attitude, double bank ,double* x,double* y, double* z, double* w);

public:

	// constructor
	KartoLogger( ConfigFile* pCf, int section);

	virtual ~KartoLogger();
	virtual void Main();

	int Setup();
	int Shutdown();

	const char* getUID(player_devaddr_t dev);
	virtual int Unsubscribe(Device* pId);

	// MessageHandler
	int ProcessMessage(QueuePointer &pQueue, player_msghdr* pHdr, void* pData);
};

void KartoLogger::Main()
{
	while(true)
	{
		usleep(20000);

		pthread_testcancel();

		this->ProcessMessages();
			if(!this->enableLogger)
				{
					break;
				}
	}
	printf("exiting...\n");
}

//
// Unsubscribe the device to Player
//
int KartoLogger::Unsubscribe(Device* pDevice)
{
		if (pDevice != NULL)
			{
			return pDevice->Unsubscribe(this->InQueue);
		}
		else
		{
			return -1;
		}
}

//
// Main function KartoLogger::ProcessMessage
// We expect control message for the PLAYER_LOG_CODE interface:
// /** Types of log device: read */
// #define  PLAYER_LOG_TYPE_READ       1
// /** Types of log device: write */
// #define  PLAYER_LOG_TYPE_WRITE      2
//
// /** Request/reply subtype: set write state */
// #define PLAYER_LOG_REQ_SET_WRITE_STATE  1
// /** Request/reply subtype: set read state */
// #define PLAYER_LOG_REQ_SET_READ_STATE   2
// /** Request/reply subtype: get state */
// #define PLAYER_LOG_REQ_GET_STATE        3
// /** Request/reply subtype: rewind */
// #define PLAYER_LOG_REQ_SET_READ_REWIND  4
// #define PLAYER_LOG_REQ_SET_READ_REWIND  4
// /** Request/reply subtype: set filename to write */
// #define PLAYER_LOG_REQ_SET_FILENAME     5
// see libplayercore/player.h
// Warning !!!
// If player code changes this code need to be
// updated.
// Since the logger is registred to many devices
// we currently only suppport laser, sonar and position2d interfaces.
// For each msg of type PLAYER_MSGTYPE_DATA (which are the message
// containing the data from the subscribed devices) we are differentiating
// on the interface type and then calling the right write functions:
// WritePosition, WriteLaserScan, WriteSonarScan
//
// Logic:
//   First we check for the type of message
//   If there a REQuest type, we check for the correct size.
//     Then we change the value (enablingLogger) and send ACK
//   If it's a data message (LAYER_MSGTYPE_DATA),
//     Call the right Write function depending of the type.
//
// Return values:
//   0 everything is fine
//   -1 message was not the right format or length and the message is discarded
//
int KartoLogger::ProcessMessage(QueuePointer &pQueue, player_msghdr* pHdr, void* pData)
{
	//writelog control
	if(Message::MatchMessage(pHdr, PLAYER_MSGTYPE_REQ,
		PLAYER_LOG_REQ_SET_WRITE_STATE,
		this->device_addr))
	{
		if(pHdr->size != sizeof(player_log_set_write_state_t))
		{
			PLAYER_ERROR2("request is wrong length (%d != %d); ignoring",
										pHdr->size, sizeof(player_log_set_write_state_t));
			return -1;
		}
		player_log_set_write_state_t* pWriteState = (player_log_set_write_state_t*)pData;

		if(pWriteState->state)
		{
			puts("KartoLogger is now logging...");
			this->enableLogger = true;
		}
		else
		{
			puts("KartoLogger has stopped logging...");
			this->enableLogger = false;
		}

	// send an empty ACK
		this->Publish(this->device_addr, pQueue,
			PLAYER_MSGTYPE_RESP_ACK,
			PLAYER_LOG_REQ_SET_WRITE_STATE);
		return 0;
	}
	else if(Message::MatchMessage(pHdr, PLAYER_MSGTYPE_REQ,
		PLAYER_LOG_REQ_GET_STATE, this->device_addr))
	{
		if(pHdr->size != 0)
		{
			PLAYER_ERROR2("request is wrong length (%d != %d); ignoring",
				pHdr->size, 0);
			return -1;
		}

		player_log_get_state_t playerState;

		playerState.type = PLAYER_LOG_TYPE_WRITE;
		if(this->enableLogger)
			{
			playerState.state = 1;
			}
		else
			{
			playerState.state = 0;
			}

		this->Publish(this->device_addr,
			pQueue,
			PLAYER_MSGTYPE_RESP_ACK,
			PLAYER_LOG_REQ_GET_STATE,
			(void*)&playerState, sizeof(playerState), NULL);
		return 0;
	}
	else if(pHdr->type == PLAYER_MSGTYPE_DATA)
	{
	// If logging is stopped, then don't log
		if(!this->enableLogger)
			{
				return 0;
			}
		player_interface_t iface;
		::lookup_interface_code(pHdr->addr.interf, &iface);
		switch (iface.interf)
			{
			case PLAYER_LASER_CODE:
				if (pHdr->subtype == PLAYER_LASER_DATA_SCANPOSE) {
					this->WriteLaserScanPose(pHdr, (player_laser_data_scanpose_t*)pData);
				}
				else {
					this->WriteLaserScan(pHdr, (player_laser_data_t*)pData);
					}
				break;
			case PLAYER_POSITION2D_CODE:
				this->WritePosition(pHdr, (player_position2d_data_t*)pData);
				break;
			case PLAYER_SONAR_CODE:
				this->WriteSonarScan(pHdr, (player_sonar_data_t*)pData);
				break;
			default:
				// do nothing ignore the message
				PLAYER_ERROR1("Don't know this interface: %d\n",iface.interf);
				break;
			}
	}
	return 0;
}


//
// KartoLogger::Shutdown : Delete instances of our KartoLogger
//


int KartoLogger::Shutdown()
{
	// Stop the driver thread.
	this->enableLogger=false;
	CloseLog();
	printf("KartoWriter  has been shutdown\n");
	return 0;
}

//
// KartoLogger::OpenLog()
//
int KartoLogger::OpenLog() {
	this->kartoFile = fopen(this->kartoFilename, "w+");
	if(this->kartoFile == NULL)
	{
		PLAYER_ERROR2("unable to open [%s]: %s\n", this->kartoFilename, strerror(errno));
		return -1;
	}
	time_t	t;
	time(&t);
	GlobalTime->GetTimeDouble(&(this->startTime));
	fprintf(this->kartoFile,"<?xml version='1.0' encoding='utf-8'?>\n<!DOCTYPE KartoLogger SYSTEM \"http://karto.ai.sri.com/dtd/KartoLogger.dtd\" >\n<KartoLogger version=\"1.0\">\n<UTCTime>\n\t%s</UTCTime>\n",ctime(&t));
	return 0;
}


//
// KartoLogger::CloseLog()
//
int KartoLogger::CloseLog() {
	if(this->kartoFile)
	{
		fprintf(this->kartoFile,"</DeviceStates>\n</KartoLogger>\n");
		fflush(this->kartoFile);
		fclose(this->kartoFile);
		if (this->compress)
			printf("Sorry gzip compression is not yet implemented\n");
		this->kartoFile = NULL;
	}
	return 0;
}

//
//KartoLogger::Setup Initialize KartoLogger using the KartoLoggerInit function
// return code is 0 everything is fine, -1 error
//
int	KartoLogger::Setup()
{
	int ret;
	ret = OpenLog();
	if(ret != 0)
		{
			return ret;
		}
	fprintf(this->kartoFile,"<DeviceList>\n");
	for (list<LogDevice*>::iterator iter = devices.begin(); iter != devices.end(); iter++)
	{
		(*iter)->pDevice = deviceTable->GetDevice((*iter)->addr);
		printf(" Setup for %s\n",(*iter)->pUid);
		if (!(*iter)->pDevice) {
			PLAYER_ERROR("unable to locate suitable device");
			return -1;
		}
		if((*iter)->pDevice->Subscribe(this->InQueue) != 0)
		{
			PLAYER_ERROR("unable to subscribe to device");
			return -1;
		}
		this->WriteGeometry((*iter));
	}
	fprintf(this->kartoFile,"</DeviceList>\n<DeviceStates>\n");
// Start device thread
	this->StartThread();
	return ret;
}

//
// KartoLogger::Destructor
//
KartoLogger::~KartoLogger()
{
	Shutdown();
}

//
// KartoLogger:KartoLogger constructor
// TODO:initialize all variables
KartoLogger::KartoLogger( ConfigFile* cf, int section) : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN,PLAYER_LOG_CODE ),
                                                         kartoFile(NULL), debug(false),enableLogger(false),
		                                                     compress(false), startTime(0.0)
{
	player_devaddr_t addr;

	for (int i = 0; i < cf->GetTupleCount(section, "requires"); i++)
	{
		if (cf->ReadDeviceAddr(&addr, section, "requires", -1, i, NULL) != 0)
		{
			this->SetError(-1);
			return;
		}
		LogDevice* pDev = new LogDevice(addr);
		if (pDev != NULL)
		{
			devices.push_front(pDev);
		}
		else
		{
			PLAYER_ERROR("Can't create LogDevice (out of memory)");
		}
	}
	strncpy(kartoFilename,
		cf->ReadString(section, "file", "output.xml"),
		sizeof(kartoFilename));
	if (cf->ReadInt(section, "debug", 0) == 1)
		debug = true;

	if (cf->ReadInt(section, "autorecord", 1) > 0)
		this->enableLogger = true;
	else
		this->enableLogger = false;

	if (cf->ReadInt(section, "gzip", 0) > 0)
		this->compress = true;
	else
		this->compress = false;

}

//
// KartoLogger_Init
//
// a factory creation function
Driver* KartoLogger_Init( ConfigFile* cf, int section)
{
	Driver* pDriver = (Driver*)(new KartoLogger(cf, section));
	assert(pDriver != NULL);
	return pDriver;

}


//
// KartoLogger_Register
//
// a driver registration function
void KartoLogger_Register(DriverTable* table)
{
	table->AddDriver(DRIVER_NAME, KartoLogger_Init);
}

//
// KartoLogger:
//
int KartoLogger::WriteGeometry(LogDevice* pDev) {
	Message* pMsg = NULL;
	switch(pDev->addr.interf) {
		case PLAYER_LASER_CODE: {
		if(!(pMsg = pDev->pDevice->Request(this->InQueue,
			PLAYER_MSGTYPE_REQ,
			PLAYER_LASER_REQ_GET_GEOM,
			NULL, 0, NULL, false)))
		{
			PLAYER_ERROR("failed to get laser geometry");
			return -1;
		}
		// Store the pose
		player_laser_geom_t laser_geom = *((player_laser_geom_t*)pMsg->GetPayload());
		char hostname[256];
		packedaddr_to_dottedip(hostname,256,pDev->addr.host);
		double x,y,z,w;
		ComputeQuaternion(laser_geom.pose.pyaw,0,0,&x,&y,&z,&w);
		fprintf(this->kartoFile,"<LaserRangeFinder>\n\t<ID>%s</ID>\n\t<Pose>\n\t\t<Position>\n\t\t\t<X>%.3f</X>\n\t\t\t<Y>0.0</Y>\n\t\t\t<Z>%.3f</Z>\n\t\t</Position>\n\t\t<Orientation>\n\t\t\t<X>%f</X>\n\t\t\t<Y>%f</Y>\n\t\t\t<Z>%f</Z>\n\t\t\t<W>%f</W>\n\t\t</Orientation>\n\t</Pose>\n</LaserRangeFinder>\n",pDev->getUID(),laser_geom.pose.py,laser_geom.pose.px,x,y,z,w);
		}
		break;
		case PLAYER_SONAR_CODE:
		// Get the sonar poses
		if(!(pMsg = pDev->pDevice->Request(this->InQueue,
			PLAYER_MSGTYPE_REQ,
			PLAYER_SONAR_REQ_GET_GEOM,
			NULL, 0, NULL,false)))
		{
			PLAYER_ERROR("failed to get sonar geometry");
			return -1;
		}

		// Store the sonar poses
		// player_sonar_geom_t* sonar_geom = *((player_sonar_geom_t*)pMsg->GetPayload());
		break;
		case PLAYER_POSITION2D_CODE:
			fprintf(this->kartoFile,"<Drive>\n\t<ID>%s</ID>\n\t<Pose>\n\t\t<Position>\n\t\t\t<X>0</X>\n\t\t\t<Y>0</Y>\n\t\t\t<Z>0.0</Z>\n\t\t</Position>\n\t\t<Orientation>\n\t\t\t<X>0</X>\n\t\t\t<Y>0</Y>\n\t\t\t<Z>0</Z>\n\t\t\t<W>1</W>\n\t\t</Orientation>\n\t</Pose>\n</Drive>\n",pDev->getUID());
		break;
	}
	if (pMsg != NULL)
	 {
		delete pMsg;
	}
	return 0;
}

//
// KartoLogger:getUID
// No need to check for buffer overflow on the sprintf since hostname is max 256, dev.robot is uint32, dev.index is uint16
const char* KartoLogger::getUID(player_devaddr_t dev) {
	char* id = (char*)malloc(ID_MAX_SIZE);
	if (id == NULL)
		{
			return DEFAULT_UID;
		}
	char hostname[MAX_HOSTNAME];
	packedaddr_to_dottedip(hostname,MAX_HOSTNAME,dev.host);
	switch (dev.interf) {
		case PLAYER_LASER_CODE:
			sprintf(id,"%s:%d:laser:%d",hostname,dev.robot,dev.index);
			break;
		case PLAYER_SONAR_CODE:
			sprintf(id,"%s:%d:sonar:%d",hostname,dev.robot,dev.index);
			break;
		case PLAYER_POSITION2D_CODE:
			sprintf(id,"%s:%d:position2d:%d",hostname,dev.robot,dev.index);
			break;
		default:
			sprintf(id,"%s:%d:unknown:%d",hostname,dev.robot,dev.index);
	}
	return id;
}

//
// KartoLogger:getUID()
// No need to check for buffer overflow on the sprintf since hostname is max 256, addr->robot is uint32, addr->index is uint16
const char* LogDevice::getUID() {
	if (this->pUid != NULL)
		{
				return (this->pUid);
		}
		char* id = (char*)malloc(ID_MAX_SIZE);
		if (id == NULL)
			{
				return DEFAULT_UID;
			}
		char hostname[MAX_HOSTNAME];
		packedaddr_to_dottedip(hostname,MAX_HOSTNAME,this->addr.host);
		switch (this->addr.interf) {
		case PLAYER_LASER_CODE:
			sprintf(id,"%s:%d:laser:%d",hostname,this->addr.robot,this->addr.index);
			break;
		case PLAYER_SONAR_CODE:
			sprintf(id,"%s:%d:sonar:%d",hostname,this->addr.robot,this->addr.index);
			break;
		case PLAYER_POSITION2D_CODE:
			sprintf(id,"%s:%d:position2d:%d",hostname,this->addr.robot,this->addr.index);
			break;
		default:
			sprintf(id,"%s:%d:unknown:%d",hostname,this->addr.robot,this->addr.index);
		}
		return (id);
}

//
// KartoLogger:
//
void
	KartoLogger::WriteLaserScan(player_msghdr_t* pHdr, player_laser_data_t* pScan)
{
	double t;
	double x,y,z,w;
	ComputeQuaternion(this->currentPose.pa,0,0,&x,&y,&z,&w);
	GlobalTime->GetTimeDouble(&t);
	t = t - this->startTime;
	if (debug) {
		fprintf(this->kartoFile,"<LocalizedRangeScan>\n\t<DeviceID>%s</DeviceID>\n\t<Time>%.3lf</Time>\n\t\n\t<MinAngle>%.4f</MinAngle>\n\t<MaxAngle>%.4f</MaxAngle>\n\t<Resolution>%f</Resolution>\n\t\t<DistanceMeasurements>\n",getUID(pHdr->addr),t,pScan->min_angle, pScan->max_angle,pScan->resolution);
	}
	else {
		fprintf(this->kartoFile,"<RangeScan>\n\t<DeviceID>%s</DeviceID>\n\t<Time>%.3lf</Time>\n\t\n\t<MinAngle>%.4f</MinAngle>\n\t<MaxAngle>%.4f</MaxAngle>\n\t<Resolution>%f</Resolution>\n\t\t<DistanceMeasurements>\n",getUID(pHdr->addr),t,pScan->min_angle, pScan->max_angle,pScan->resolution);
	}
	for (unsigned int i = 0; i < pScan->ranges_count; i++)
	{
		//	fprintf(this->kartoFile,"\t\t<float>%.3f</float>\n\t\t<Intensity>%d</Intensity>\n",scan->ranges[i],scan->intensity[i]);
		fprintf(this->kartoFile,"\t\t<float>%.3f</float>\n",pScan->ranges[i]);
	}
	fprintf(this->kartoFile,"\t\t</DistanceMeasurements>\n");
  if (debug) {
		fprintf(this->kartoFile,"\t<Pose>\n\t\t<Position>\n\t\t<X>%.3f</X>\n\t\t<Y>0.0</Y>\n\t\t<Z>%.3f</Z>\n\t\t</Position>\n\t\t<Orientation>\n\t\t<X>%f</X>\n\t\t<Y>%f</Y>\n\t\t<Z>%f</Z>\n\t\t<W>%f</W>\n\t\t</Orientation>\n\t</Pose>\n",this->currentPose.py,this->currentPose.px,x,y,z,w);
		fprintf(this->kartoFile,"</LocalizedRangeScan>\n");
	}
	else {
		fprintf(this->kartoFile,"</RangeScan>\n");
	}

}


//
// KartoLogger::WriteLaserScanPose
//
void
KartoLogger::WriteLaserScanPose(player_msghdr_t* pHdr, player_laser_data_scanpose_t* pScan)
{
	double t;
	GlobalTime->GetTimeDouble(&t);
	double x,y,z,w;
	ComputeQuaternion(pScan->pose.pa,0,0,&x,&y,&z,&w);
	t = t - this->startTime;
	fprintf(this->kartoFile,"<LocalizedRangeScan>\n\t<DeviceID>%s</DeviceID>\n\t<Time>%.3lf</Time>\n\t\n\t<MinAngle>%.4f</MinAngle>\n\t<MaxAngle>%.4f</MaxAngle>\n\t<Resolution>%f</Resolution>\n\t\t<DistanceMeasurements>\n",getUID(pHdr->addr),t,pScan->scan.min_angle, pScan->scan.max_angle,pScan->scan.resolution);
	for (unsigned int i = 0; i < pScan->scan.ranges_count; i++)
	{
		//	fprintf(this->kartoFile,"\t\t<float>%.3f</float>\n\t\t<Intensity>%d</Intensity>\n",scan->ranges[i],scan->intensity[i]);
		fprintf(this->kartoFile,"\t\t<float>%.3f</float>\n",pScan->scan.ranges[i]);
	}
	fprintf(this->kartoFile,"\t\t</DistanceMeasurements>\n");
	fprintf(this->kartoFile,"<Pose>\n\t\t<Position>\n\t\t<X>%.3f</X>\n\t\t<Y>0.0</Y>\n\t\t<Z>%.3f</Z>\n\t\t</Position>\n\t\t<Orientation>\n\t\t<X>%f</X>\n\t\t<Y>%f</Y>\n\t\t<Z>%f</Z>\n\t\t<W>%f</W>\n\t\t</Orientation>\n\t</Pose>\n",pScan->pose.py,pScan->pose.px,x,y,z,w);
	fprintf(this->kartoFile,"</LocalizedRangeScan>\n");
}

//
// KartoLogger::WritePosition
//
void KartoLogger::WritePosition(player_msghdr_t* pHdr, player_position2d_data_t* pData)
{
	double t;
	GlobalTime->GetTimeDouble(&t);
	t = t - this->startTime;
	this->currentPose = pData->pos;
	double x,y,z,w;
	ComputeQuaternion(this->currentPose.pa,0,0,&x,&y,&z,&w);
	fprintf(this->kartoFile,"<DrivePose>\n\t<DeviceID>%s</DeviceID>\n\t<Time>%.3lf</Time>\n\t<Pose>\n\t\t<Position>\n\t\t<X>%.3f</X>\n\t\t<Y>0.0</Y>\n\t\t<Z>%.3f</Z>\n\t\t</Position>\n\t\t<Orientation>\n\t\t<X>%f</X>\n\t\t<Y>%f</Y>\n\t\t<Z>%f</Z>\n\t\t<W>%f</W>\n\t\t</Orientation>\n\t</Pose>\n</DrivePose>\n",getUID(pHdr->addr),t,this->currentPose.py,this->currentPose.px,x,y,z,w);
}


//
// KartoLogger::WriteSonarScan
//
void KartoLogger::WriteSonarScan(player_msghdr_t* pHdr, player_sonar_data_t* pScan)
{

	fprintf(this->kartoFile,"<RangeScan timestamp=\"%.3lf\">\n\t<SensorID>%s</SensorID>\n\t<RangeCount>%d</RangeCount>\n",pHdr->timestamp,getUID(pHdr->addr),pScan->ranges_count);
	for(unsigned int i = 0; i < pScan->ranges_count; i++)
	{
		fprintf(this->kartoFile,"\t\t<Range timestamp=\"%.3lf\">%.3f</Range>\n",pHdr->timestamp,pScan->ranges[i]);
	}
	fprintf(this->kartoFile,"</RangeScan>\n");
}


//
// KartoLogger::ComputeQuaternion  - Compute the quaternion value for a give heading
//
void KartoLogger::ComputeQuaternion(double heading, double attitude, double bank , double* x, double* y, double* z, double* w) {
	double c1 = cos(heading / 2.0);
	double c2 = cos(attitude / 2.0);
	double c3 = cos(bank / 2.0);
	double s1 = sin(heading / 2.0);
	double s2 = sin(attitude / 2.0);
	double s3 = sin(bank / 2.0);
	*w = (c1 * c2 * c3) - (s1 * s2 * s3);
	*x = (s1 * s2 * c3) + (c1 * c2 * s3);
	*y = (s1 * c2 * c3) + (c1 * s2 * s3);
	*z = (c1 * s2 * c3) - (s1 * c2 * s3);
}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
/*
extern "C" {
	int player_driver_init(DriverTable* table)
	{
		puts("KartoLogger driver initializing");
		KartoLogger_Register(table);
		puts("KartoLogger initialization done");
		return 0;
	}
}
*/
