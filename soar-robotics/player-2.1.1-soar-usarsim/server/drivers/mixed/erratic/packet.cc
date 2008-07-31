// -*- mode:C++; tab-width:2; c-basic-offset:2; indent-tabs-mode:1; -*-

/**
	*  Copyright (C) 2006
	*     Videre Design
	*  Copyright (C) 2000  
	*     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
	*
	*  Videre Erratic robot driver for Player
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
**/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "packet.h"
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h> /* for exit() */
//#include <sys/poll.h>

#include "erratic.h"

void ErraticPacket::Print() {
	if (packet) {
		printf("\"");
		for(int i=0;i<size;i++) {
			printf("%u ", packet[i]);
		}
		puts("\"");
	}
}

void ErraticPacket::PrintHex() {
	if (packet) {
		printf("\"Hex: ");
		for(int i=0;i<size;i++) {
			printf("%.2x ", packet[i]);
		}
		puts("\"");
	}
}


bool ErraticPacket::Check() {
	uint16_t chksum = CalcChkSum();
	uint16_t received_chksum = packet[size-2] << 8 | packet[size-1];

	if ( chksum == received_chksum ) 
		{
			if (debug_mode) 
			{
				printf("Good packet: ");
				PrintHex();
			}
			return(true);
		}

	if (debug_mode) {
		printf("This packet failed checksum control (%i instead of %i): ", received_chksum, chksum);
		PrintHex();
	}
	
	return(false);
}

uint16_t ErraticPacket::CalcChkSum() {
	uint8_t *buffer = &packet[3];
	uint16_t n = size - 5;
	uint16_t c = 0;

	while (n > 1) {
		c+= (*(buffer)<<8) | *(buffer+1);
		n -= 2;
		buffer += 2;
	}
	if (n > 0) c = c ^ (uint16_t)*buffer;

	return(c);
}

// You can supply a timeout in milliseconds
int ErraticPacket::Receive( int fd, uint16_t wait ) {
	uint8_t prefix[3];
	uint32_t skipped;
	uint16_t cnt;

	if (debug_mode)
		printf("Check for packets in Receive()\n");

	memset(packet,0,sizeof(packet));
	struct pollfd readpoll;
	readpoll.fd = fd; 
	readpoll.events = POLLIN | POLLPRI;
	readpoll.revents = 0;
	
#ifdef USE_SELECT
	fd_set read_set, error_set;
	struct timeval tv;
	FD_ZERO(&read_set); 
	FD_ZERO(&error_set);
	FD_SET(fd, &read_set); 
	FD_SET(fd, &error_set);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (wait >= 1000)
		tv.tv_sec = wait/1000;
	else
		tv.tv_usec = wait*1000;
#endif

	// This block will terminate or move on when there is data to read
	if (wait) {
		while (1) {
#ifdef USE_SELECT
			int ret = select(fd+1, &read_set, 0, &error_set, &tv);
			if (ret)
				{
					if (debug_mode)
						printf("Data waiting\n");
					break;
				}
			else
				if (debug_mode)
					printf("No data\n");
#endif

			int8_t stuff = poll(&readpoll, 1, wait);
			
			if (stuff < 0) {
				if (errno == EINTR) {
					continue;
				}
				return 1;
			}
			
			if (stuff == 0) {
				return (receive_result_e)timeout;
			}

			if (readpoll.revents & POLLIN)
				break;
			
			printf("Serial port error\n");
			return (receive_result_e)failure;
		}
	}
	
	do {
		memset(prefix,0,sizeof(prefix));
		
		skipped = 0;
		while(1) {
			cnt = 0;
			
			// Once we've started receiving a packet, we have a tight timeout
			// Trouble with FTDI USB interface: needs longer timeout
			while( cnt!=1 ) {
				if (wait) {
					while (1) {

#ifdef USE_SELECT						
						FD_ZERO(&read_set); 
						FD_ZERO(&error_set);
						FD_SET(fd, &read_set); 
						FD_SET(fd, &error_set);
						tv.tv_sec = 0;
						tv.tv_usec = 100*1000; // in usec's

						int ret = select(fd+1, &read_set, 0, &error_set, &tv);
						if (ret)
							break;
#endif

						int8_t stuff = poll(&readpoll, 1, 100);

						if (stuff < 0) {
							if (errno == EINTR) {
								continue;
							}
							return 1;
						}

						if (stuff == 0) {
							return (receive_result_e)timeout;
						}

						if (readpoll.revents & POLLIN)
							break;

						printf("Serial port error\n");
						return (receive_result_e)failure;
					}
				}
				
				
				int newcnt = read( fd, &prefix[2], 1 );
				if (debug_mode)
					printf("Read %d byte: %02x\n", newcnt, prefix[2]);

				if (newcnt < 0 && errno == EAGAIN)
					continue;
				else if (newcnt < 0) {
					perror("Erratic::Receive:read:");
					return(1);
				}

				cnt++;
			}

			if (prefix[0]==0xFA && prefix[1]==0xFB) break;

			prefix[0]=prefix[1];
			prefix[1]=prefix[2];
			skipped++;
			
			if (skipped > 200) return (receive_result_e)timeout;
		}
		if (skipped>2 && debug_mode) printf("Skipped %d bytes\n", skipped);

		size = prefix[2]+3;
		memcpy( packet, prefix, 3);

		cnt = 0;
		while( cnt!=prefix[2] ) 
		{
			if (wait) {
				while (1) {
#ifdef USE_SELECT
					FD_ZERO(&read_set); 
					FD_ZERO(&error_set);
					FD_SET(fd, &read_set); 
					FD_SET(fd, &error_set);
					tv.tv_sec = 0;
					tv.tv_usec = 100*1000;				// in usec's

					int ret = select(fd+1, &read_set, 0, &error_set, &tv);
					if (ret)
						break;
#endif

					int8_t stuff = poll(&readpoll, 1, 100);

					if (stuff < 0) {
						if (errno == EINTR) {
							continue;
						}
						return 1;
					}

					if (stuff == 0) {
						return (receive_result_e)timeout;
					}

					if (readpoll.revents & POLLIN)
						break;

					printf("Serial port error\n");
					return (receive_result_e)failure;
				}
			}
			
			int newcnt = read( fd, &packet[3+cnt], prefix[2]-cnt );
			if (debug_mode)
				{
					printf("Read %d bytes packet\n", newcnt);
					if (newcnt > 0)
						{
							for (int i=0; i<newcnt; i++)
								printf("%02x ", packet[3+i]);
							printf("\n");
						}
				}

			if (newcnt < 0 && errno == EAGAIN)
				continue;
			else if (newcnt < 0) {
				perror("Erratic::Receive:read:");
				return(1);
			}

			cnt += newcnt;
		}
	} while (!Check());  
	return(0);
}


int ErraticPacket::Build( unsigned char *data, unsigned char datasize ) {
	uint16_t chksum;

	size = datasize + 5;

	/* header */
	packet[0]=0xFA;
	packet[1]=0xFB;

	if ( size > 198 ) {
		printf("Erratic driver error: Packet to robot can't be larger than 200 bytes");
		return(1);
	}
	packet[2] = datasize + 2;

	memcpy( &packet[3], data, datasize );

	chksum = CalcChkSum();
	//if (chksum < 0) chksum += 0x8000;
	packet[3+datasize] = chksum >> 8;
	packet[3+datasize+1] = chksum;// & 0xFF;

	/*if (debug_mode) {
		int16_t last = packet[3+datasize+1];
		int32_t test = (packet[3+datasize] << 8) | last;
		
		printf("chksum that will be sent:%i , received: %i\n", chksum, test);
	}*/
	
	return(0);
}

int ErraticPacket::Send( int fd) {
	int cnt=0;

	//printf("Send(): ");
	//if (packet[3] != 0x0b && packet[3] != 0x15)
	//  PrintHex();

	//if (debug_mode)
	//	Print();


	while(cnt!=size)
	{
		if((cnt += write( fd, packet+cnt, size-cnt )) < 0) 
		{
			perror("Send");
			return(1);
		}
	}
	return(0);
}
