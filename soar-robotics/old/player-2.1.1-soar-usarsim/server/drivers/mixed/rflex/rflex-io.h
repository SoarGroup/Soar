#ifndef RFLEX_IO_H
#define RFLEX_IO_H

#include <rflex-info.h>

#define MAX_NAME_LENGTH  256

long  bytesWaiting( int sd );

void  DEVICE_set_params( RFLEX_Device dev );

void  DEVICE_set_baudrate( RFLEX_Device dev, int brate );

int   DEVICE_connect_port( RFLEX_Device *dev );

#define BUFFER_LENGTH         512

int   writeData( int fd, unsigned char *buf, int nChars );

int   waitForAnswer( int fd, unsigned char *buf, int *len );
 
#endif
