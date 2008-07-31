/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) <insert dates here>
 *     <insert author's name(s) here>
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
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/
#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <libplayercore/error.h>  // for error macros
#include <libplayercore/playerconfig.h>

void
packedaddr_to_dottedip(char* dest, size_t len, uint32_t addr)
{
  int mask = 0xff;
  int swappedaddr;

  swappedaddr = htonl(addr);

  assert(len >= (size_t)16);
  
  sprintf(dest, "%u.%u.%u.%u",
          swappedaddr>>24 & mask,
          swappedaddr>>16 & mask,
          swappedaddr>>8 & mask,
          swappedaddr>>0 & mask);
}

int
hostname_to_packedaddr(uint32_t* dest, const char* hostname)
{
#if HAVE_GETADDRINFO
  int retval;
  struct addrinfo* addr = NULL;
#else
  struct sockaddr_in saddr;
  struct hostent* entp;
#endif
  char host[256];

  memset(host,0,sizeof(host));
  if(!hostname)
  {
    if(gethostname(host,sizeof(host)) == -1)
    {
      PLAYER_ERROR("couldn't get hostname");
      return(-1);
    }
  }
  else
    strncpy(host,hostname,sizeof(host));
  // Make sure host is NULL-terminated
  host[sizeof(host)-1] = '\0';

#if HAVE_GETADDRINFO
  if((retval = getaddrinfo(host,NULL,NULL,&addr)))
  {
    printf("getaddrinfo() failed: %s", gai_strerror(retval));
    return(-1);
  }
  *dest = ((struct sockaddr_in*)addr->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(addr);
#else
  if((entp = gethostbyname(host)) == NULL)
  {
    printf("gethostbyname() failed");
    return(-1);
  }
  *dest = ((struct in_addr*)entp->h_addr_list[0])->s_addr;
#endif
  return(0);
}

