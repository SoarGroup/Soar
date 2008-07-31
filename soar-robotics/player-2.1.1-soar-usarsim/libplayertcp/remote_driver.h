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

#ifndef REMOTE_DRIVER_H
#define REMOTE_DRIVER_H

#include <libplayercore/playercore.h>
#include "playertcp.h"

#define DEFAULT_SETUP_TIMEOUT 1.0

class TCPRemoteDriver : public Driver
{
  private:
    PlayerTCP* ptcp;
    int sock;
    QueuePointer queue, ret_queue;
    char ipaddr[256];
    int kill_flag;
    double setup_timeout;

    int SubscribeRemote(unsigned char mode);

  public:
    TCPRemoteDriver(player_devaddr_t addr, void* arg);
    virtual ~TCPRemoteDriver();

    virtual int Setup();
    virtual int Shutdown();
    virtual void Update();
    virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr, 
                               void * data);

    static Driver* TCPRemoteDriver_Init(player_devaddr_t addr, void* arg);
};

#endif
