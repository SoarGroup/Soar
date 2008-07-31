#ifndef KHEPERA_SERIAL_H
#define KHEPERA_SERIAL_H

#include <pthread.h>
#include <termios.h>

#define KHEPERA_DEFAULT_BAUD B38400
#define KHEPERA_BUFFER_LEN 255
#define KHEPERA_SERIAL_TIMEOUT_USECS 100000


class KheperaSerial
{
public:
	
	KheperaSerial(char * port, int rate = KHEPERA_DEFAULT_BAUD);
	~KheperaSerial();

	bool Open() {return fd >0;};
	int KheperaCommand(char command, int InCount, int * InValues, int OutCount, int * OutValues);

	void Lock();
	void Unlock();
protected:
	// serial port descriptor
	int fd;
	struct termios oldtio;
	
	// read/write buffer
	char buffer[KHEPERA_BUFFER_LEN+1];

	int WriteInts(char command, int Count = 0, int * Values = NULL);
	int ReadInts(char Header, int Count = 0, int * Values = NULL);
	
	pthread_mutex_t lock;
};

#endif
