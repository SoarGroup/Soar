#include "khepera_serial.h"

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


KheperaSerial::KheperaSerial(char * port, int rate)
{
	fd = -1;
	
	pthread_mutex_init(&lock,NULL);
		
	// open the serial port
	fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if ( fd<0 )
	{
		fprintf(stderr, "Could not open serial device %s\n",port);
		return;
	}
	//fcntl(fd,F_SETFL, O_NONBLOCK);
	
	// save the current io settings
	tcgetattr(fd, &oldtio);

	// rtv - CBAUD is pre-POSIX and doesn't exist on OS X
	// should replace this with ispeed and ospeed instead.
	
	// set up new settings
	struct termios newtio;
	memset(&newtio, 0,sizeof(newtio));
	newtio.c_cflag = /*(rate & CBAUD) |*/ CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = ICANON;
	
	// activate new settings
	tcflush(fd, TCIFLUSH);
	if (cfsetispeed(&newtio, rate) < 0 || 	cfsetospeed(&newtio, rate) < 0)
	{
		fprintf(stderr,"Failed to set serial baud rate: %d\n", rate);
		tcsetattr(fd, TCSANOW, &oldtio);	
		close(fd);
		fd = -1;				
		return;
	}
	tcsetattr(fd, TCSANOW, &newtio);
	tcflush(fd, TCIOFLUSH);
	
	// clear the input buffer in case junk data is on the port
	usleep(100000);
	write(fd,"\r\n",2);
	usleep(100000);
	tcflush(fd, TCIFLUSH);
	
	// try a test command
	int Values[2];
	if (KheperaCommand('E',0,NULL,2,Values) < 0)
	{
		fprintf(stderr,"Failed to initialise the Khepera Serial Port %s\n",port);
		close(fd);
		fd = -1;
	}		
}


KheperaSerial::~KheperaSerial()
{
	// restore old port settings
	if (fd > 0)
	{
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);	
	}
}

void KheperaSerial::Lock()
{
	pthread_mutex_lock(&lock);
}

void KheperaSerial::Unlock()
{
	pthread_mutex_unlock(&lock);
}

int KheperaSerial::WriteInts(char command, int Count, int * Values)
{

	if (fd < 0)
		return -1;
		
	buffer[0] = command;
	buffer[1] = '\0';
	int length;
	for (int i = 0; i < Count; ++ i)
	{
		length = strlen(buffer);
		snprintf(&buffer[length],KHEPERA_BUFFER_LEN - length - 2,",%d",Values[i]);
	}
	length = strlen(buffer);
	snprintf(&buffer[length],KHEPERA_BUFFER_LEN - length ,"\n");
	length = strlen(buffer);
	
	// ugly error handling, if write fails then shut down unit
	tcflush(fd, TCIFLUSH);
	if(write(fd, buffer, length) < length)
	{
		fprintf(stderr,"Error writing to Khepera, disabling\n");
		tcsetattr(fd, TCSANOW, &oldtio);	
		fd = -1;
		return -1;
	}
	return 0;
}


// read ints from khepera
int KheperaSerial::ReadInts(char Header, int Count, int * Values)
{
	if (fd < 0)
		return -1;
	struct timeval Start,Now;
	int TimePassed = 0;
	gettimeofday(&Start,NULL);
	int length = 0;	
	do
	{
		pthread_testcancel();
		length += read(fd, &buffer[length], KHEPERA_BUFFER_LEN - length);
		gettimeofday(&Now,NULL);
		TimePassed = (Now.tv_sec - Start.tv_sec)*1000000 + Now.tv_usec - Start.tv_usec;
	} while (buffer[length-1] != '\n' && TimePassed < KHEPERA_SERIAL_TIMEOUT_USECS);

	if (TimePassed >= KHEPERA_SERIAL_TIMEOUT_USECS)
	{
		fprintf(stderr,"Time out while reading khepera reply\n");
		return -1;
	}
	
	if (buffer[0] != Header)
		return -1;
		
	char * pos = &buffer[2];
	for (int i = 0; i < Count; ++i)
	{
		Values[i] = strtol(pos,&pos,10);
		pos++;
	}
	return 0;		
}

int KheperaSerial::KheperaCommand(char command, int InCount, int * InValues, int OutCount, int * OutValues)
{
	//printf("Serial command: %c %d %d\n",command,InCount,OutCount);
	Lock();
	int ret1 = WriteInts(command,InCount,InValues);
	usleep(10000); // delay to give the khepera time to process the command before reading the result
	int ret2 = ReadInts(command + 32,OutCount,OutValues);
	Unlock();
	return ret1 < ret2 ? ret1 : ret2;
}
