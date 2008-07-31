/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
 *
 *  LifoMCom device by Matthew Brewer <mbrewer@andrew.cmu.edu> (updated for 1.3 by 
 *  Reed Hedges <reed@zerohour.net>) at the Laboratory for Perceptual 
 *  Robotics, Dept. of Computer Science, University of Massachusetts,
 *  Amherst.
 *
 * This program is free software; you can redistribute it and/or modify
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

#ifndef _MCOMDEVICE_HH_
#define _MCOMDEVICE_HH_

#include <libplayercore/playercore.h>


/*
 * This device is designed for exchanging information between clients.
 * A client sends a message of a given "type" and "channel". This 
 * device stores adds the message to that channel's stack.
 * A second client can then request data of a given "type" and "channel"
 * if Pop is called the last piece of data added to the buffer is returned
 * and then deleted.
 * If Read is called the last piece of data added is returned, and left there
 * This is a filo, or a stack first in last out -
 * This way if were reading drive command for example we can be sure to
 * get a "STOP" and interupt a "FWD" before it's been read.
 * Player's "configuration"-style message passing is used.
 */



class LifoMCom : public Driver 
{
private:

    class Buffer {
    private:
        player_mcom_data_t dat[MCOM_N_BUFS];
        int top;
      int capacity;
    public:
        int type;
        char channel[MCOM_CHANNEL_LEN];
        Buffer();
        ~Buffer();
        void Push(player_mcom_data_t newdat);
        player_mcom_data_t Pop();
        player_mcom_data_t Read();
        void Clear();
        void print();
      void SetCapacity(int cap);
      int GetCapacity() { return capacity; }
    };

    struct Link{
        Buffer buf;
        Link* next;
    };

    class LinkList {
    private:
        Link * top;
    public:
        LinkList();
        ~LinkList();
        void Push(player_mcom_data_t d,int type, char channel[MCOM_CHANNEL_LEN]);
        player_mcom_data_t Pop(int type, char channel[MCOM_CHANNEL_LEN]);
        player_mcom_data_t Read(int type,char channel[MCOM_CHANNEL_LEN]);
        void Clear(int type,char channel[MCOM_CHANNEL_LEN]);
      void SetCapacity(int type, char channel[MCOM_CHANNEL_LEN], 
		       unsigned char cap);
      Link * FindLink(int type, char channel[MCOM_CHANNEL_LEN]);
    };

    LinkList Data;

public:


    // Called when we recieve a config request; overrides Driver::PutConfig
/*    virtual int PutConfig(player_device_id_t id, void *client, 
                          void* src, size_t len,
                          struct timeval* timestamp);*/

    // These do nothing but are abstract in Driver, so here they are
    // Process incoming messages from clients 
    int ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, size_t * resp_len);

    // Constructor
    LifoMCom(ConfigFile* cf, int section);

    virtual int Setup() {
        printf("startup...\n");
        return 0;
    }

    virtual int Shutdown() {
        printf("shutdown ...\n");
        return 0;
    }
};




Driver* LifoMCom_Init(char* interface, ConfigFile* cf, int section);

// a driver registration function
void LifoMCom_Register(DriverTable* table);



#endif //ifdef _MCOMDEVICE_HH_

