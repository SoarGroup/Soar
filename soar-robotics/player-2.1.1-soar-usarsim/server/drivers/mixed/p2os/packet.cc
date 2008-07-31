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
 * $Id: packet.cc 4356 2008-02-15 08:53:55Z thjc $
 *   part of the P2OS parser.  this class has methods for building,
 *   printing, sending and receiving P2OS packets.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <packet.h>
#include <unistd.h>
#include <stdlib.h> /* for exit() */

void P2OSPacket::Print() {
  if (packet) {
    printf("\"");
    for(int i=0;i<size;i++) {
      printf("%u ", packet[i]);
    }
    puts("\"");
  }
}

void P2OSPacket::PrintHex() {
  if (packet) {
    printf("\"");
    for(int i=0;i<size;i++) {
      printf("0x%.2x ", packet[i]);
    }
    puts("\"");
  }
}


bool P2OSPacket::Check() {
  short chksum;
  chksum = CalcChkSum();

  if ( chksum == packet[size-2] << 8 | packet[size-1]) 
    return(true);


  return(false);
}

int P2OSPacket::CalcChkSum() {
  unsigned char *buffer = &packet[3];
  int c = 0;
  int n;

  n = size - 5;

  while (n > 1) {
    c+= (*(buffer)<<8) | *(buffer+1);
    c = c & 0xffff;
    n -= 2;
    buffer += 2;
  }
  if (n>0) c = c^ (int)*(buffer++);
  
  return(c);
}

int P2OSPacket::Receive( int fd ) 
{
  unsigned char prefix[3];
  //int skipped=0;
  int cnt;

  memset(packet,0,sizeof(packet));

  do 
  {
    memset(prefix,0,sizeof(prefix));
    //memset( prefix, 0, 3);

    while(1) 
    {
      cnt = 0;
      while( cnt!=1 ) 
      {
        if ( (cnt+=read( fd, &prefix[2], 1 )) < 0 ) 
        {
          perror("Error reading packet header from robot connection: P2OSPacket():Receive():read():");
          return(1);
        }
      }

      if (prefix[0]==0xFA && prefix[1]==0xFB) break;
      
      GlobalTime->GetTimeDouble(&timestamp);
      
      prefix[0]=prefix[1];
      prefix[1]=prefix[2];
      //skipped++;
    }
    //if (skipped>3) printf("Skipped %d bytes\n", skipped);
    
    size = prefix[2]+3;
    memcpy( packet, prefix, 3);

    cnt = 0;
    while( cnt!=prefix[2] ) 
    {
      if ( (cnt+=read( fd, &packet[3+cnt],  prefix[2]-cnt )) < 0 ) 
      {
        perror("Error reading packet body from robot connection: P2OSPacket():Receive():read():");
        return(1);
      }  
    }
  } while (!Check());  
  return(0);
}

int P2OSPacket::Build( unsigned char *data, unsigned char datasize ) {
  short chksum;

  size = datasize + 5;

  /* header */
  packet[0]=0xFA;
  packet[1]=0xFB;
  
  if ( size > 198 ) {
    puts("Packet to P2OS can't be larger than 200 bytes");
    return(1);
  }
  packet[2] = datasize + 2;

  memcpy( &packet[3], data, datasize );
  
  chksum = CalcChkSum();
  packet[3+datasize] = chksum >> 8;
  packet[3+datasize+1] = chksum & 0xFF;

  if (!Check()) {
    puts("DAMN");
    return(1);
  }
  return(0);
}

int P2OSPacket::Send( int fd) 
{
  int cnt=0;
  
  //printf("Send(): ");
  //PrintHex();
  while(cnt!=size)
  {
    if((cnt += write( fd, packet, size )) < 0) 
    {
      perror("Send");
      return(1);
    }
  }
  return(0);
}
