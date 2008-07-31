/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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

/*
 * $Id: packet.cc 3999 2007-03-01 20:41:47Z gerkey $
 *   GRASP version of the P2OS packet class.  this class has methods 
 *   for building, printing, sending and receiving GRASP Board packets.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <packet.h>
#include <unistd.h>
#include <stdlib.h> /* for exit() */
#include "clodbuster.h"

//extern bool debug;

void GRASPPacket::Print() {
  if (packet) {
    printf("\"");
    for(int i=0;i<(int)size;i++) {
      printf("%u ", packet[i]);
    }
    puts("\"");
  }
}

void GRASPPacket::PrintHex() {
  if (packet) {
    printf("\"");
    for(int i=0;i<(int)size;i++) {
      printf("0x%.2x ", packet[i]);
    }
    puts("\"");
  }
}



int GRASPPacket::Receive( int fd,unsigned char command) 
{
  switch(command)
  { 
    case ECHO_SERVO_VALUES:
    case ECHO_MAX_SERVO_LIMITS:
    case ECHO_MIN_SERVO_LIMITS:
    case ECHO_CEN_SERVO_LIMITS:
      retsize=8;
      break;
  case ECHO_ENCODER_COUNTS: retsize=6; break;
  case ECHO_ENCODER_COUNTS_TS:
  case ECHO_ADC: retsize=10;break;
  case READ_ID:retsize=1;break;
  case ECHO_SLEEP_MODE: retsize=1;break;
  default:
    retsize=0;return(1);
  }

  memset(packet,0,retsize);

    int cnt = 0;
   
    cnt=read( fd, packet,  retsize); 
    if (cnt!=(int)retsize)
      printf("wrong read size: asked %d got %d\n",retsize,cnt);
       
  return(0);
}

int GRASPPacket::Build(unsigned char command, unsigned char data)
{
  /* header */
  packet[0]=0xFF;
  packet[1]=command;
  packet[2]=data;
  size=3; 
  return(0);
}

int GRASPPacket::Build(unsigned char command)
{
  /* header */
  packet[0]=0xFF;
  packet[1]=command;
  packet[2]=0x00;
  size=3; 
  return(0);
}

int GRASPPacket::Send(int fd) 
{
  int cnt=0;
  cnt = write( fd, packet, size );
 
   if(cnt !=(int)size) 
    {
      perror("Send");
      return(1);
    }
  
/*  if(debug)
  {
    struct timeval dummy;
    GlobalTime->GetTime(&dummy);
    printf("GRASPPacket::Send():%ld:%ld\n", dummy.tv_sec, dummy.tv_usec);
  }*/
  return(0);
}
