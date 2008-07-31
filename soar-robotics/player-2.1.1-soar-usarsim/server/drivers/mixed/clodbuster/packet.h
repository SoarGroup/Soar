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
 * $Id: packet.h 1949 2004-01-30 19:21:15Z section314 $
 *   GRASP version of the P2OS packet class.  this class has methods 
 *   for building, printing, sending and receiving GRASP Board packets.
 *
 */

#ifndef _PACKET_H
#define _PACKET_H

#include <string.h>

#define PACKET_LEN 12

class GRASPPacket 
{
 public:
  unsigned char packet[PACKET_LEN];
  unsigned int size;
  unsigned int retsize;
  void Print();
  void PrintHex();
  int Build(unsigned char command, unsigned char data);
  int Build(unsigned char command);

  int Send( int fd );
  int Receive( int fd,unsigned char command );
 
   bool operator!= ( GRASPPacket p ) {
    if ( size != p.size) return(true);

    if ( memcmp( packet, p.packet, size ) != 0 ) return (true);

    return(false);
  }
};

#endif
