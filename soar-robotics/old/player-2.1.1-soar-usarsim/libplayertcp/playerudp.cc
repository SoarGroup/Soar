/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2005 -
 *     Brian Gerkey
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

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#if HAVE_ZLIB_H
  #include <zlib.h>
#endif

#include <replace/replace.h>
#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

#include "playerudp.h"
//#include "remote_driver.h"

int _create_and_bind_udp_socket(char blocking, unsigned int host, int portnum);

typedef struct playerudp_listener
{
  int fd;
  int port;
} playerudp_listener_t;

/** @brief A UDP Connection */
typedef struct playerudp_conn
{
  /** Marked for deletion? */
  int del;
  /** Is the connection valid? */
  int valid;
  /** File descriptor for the socket */
  int fd;
  /** Host associated with this connection.  For local devices, it's
   * localhost (or some alias); for remote devices, it's the remote host. */
  unsigned int host;
  /** Port on which the connection was originally accepted */
  int port;
  /** Remote address */
  struct sockaddr_in addr;
  /** Outgoing queue for this connection */
  QueuePointer queue;
  /** Buffer in which to store partial incoming messages */
  char* readbuffer;
  /** Total size of @p readbuffer */
  int readbuffersize;
  /** How much of @p readbuffer is currently in use (i.e., holding a
    partial message) */
  int readbufferlen;
  /** Buffer in which to store partial outgoint messages */
  char* writebuffer;
  /** Total size of @p writebuffer */
  int writebuffersize;
  /** How much of @p writebuffer is currently in use (i.e., holding a
    partial message) */
  int writebufferlen;
  /** Linked list of devices to which we are subscribed */
  Device** dev_subs;
  size_t num_dev_subs;
  /** Flag that we should set to true when we kill the client.
   * TCPRemoteDriver uses this flag to know when the connection has been
   * closed, and thus that it should stop publishing to its queue. */
  int* kill_flag;
} playerudp_conn_t;


PlayerUDP::PlayerUDP()
{
  this->thread = pthread_self();
  this->size_clients = 0;
  this->num_clients = 0;
  this->clients = (playerudp_conn_t*)NULL;
  //this->client_ufds = (struct pollfd*)NULL;

  pthread_mutex_init(&this->clients_mutex,NULL);

  this->num_listeners = 0;
  this->listeners = (playerudp_listener_t*)NULL;
  this->listen_ufds = (struct pollfd*)NULL;

  // Create a buffer to hold decoded incoming messages
  this->decode_readbuffersize = PLAYER_MAX_MESSAGE_SIZE;
  this->decode_readbuffer = (char*)malloc(this->decode_readbuffersize);
  assert(this->decode_readbuffer);
  this->decode_readbufferlen = 0;

  if(hostname_to_packedaddr(&this->host,"localhost") < 0)
  {
    PLAYER_WARN("address lookup failed for localhost");
    this->host = 0;
  }

  //deviceTable->AddRemoteDriverFn(TCPRemoteDriver::TCPRemoteDriver_Init,this);
}

PlayerUDP::~PlayerUDP()
{
  for(int i=0;i<this->num_clients;i++)
    this->Close(i);
  free(this->clients);
  //free(this->client_ufds);
  free(this->listeners);
  free(this->listen_ufds);
  free(this->decode_readbuffer);
}

int
PlayerUDP::Listen(int* ports, int num_ports)
{
  int tmp = this->num_listeners;
  this->num_listeners += num_ports;
  this->listeners = (playerudp_listener_t*)realloc(this->listeners,
                                                   this->num_listeners *
                                                   sizeof(playerudp_listener_t));
  this->listen_ufds = (struct pollfd*)realloc(this->listen_ufds,
                                              this->num_listeners *
                                              sizeof(struct pollfd));
  assert(this->listeners);
  assert(this->listen_ufds);

  for(int i=tmp;i<this->num_listeners;i++)
  {
    if((this->listeners[i].fd =
        _create_and_bind_udp_socket(0,this->host,ports[i])) < 0)
    {
      PLAYER_ERROR("_create_and_bind_udp_socket() failed");
      return(-1);
    }
    this->listeners[i].port = ports[i];

    // set up for later use of poll() to accept() connections on this port
    this->listen_ufds[i].fd = this->listeners[i].fd;
    this->listen_ufds[i].events = POLLIN;

    // set up for later use by global file watcher
    fileWatcher->AddFileWatch(this->listeners[i].fd);

  }

  return(0);
}

// Should be called with client_mutex locked
QueuePointer
PlayerUDP::AddClient(struct sockaddr_in* cliaddr,
                     unsigned int local_host,
                     unsigned int local_port,
                     int sock,
                     bool send_banner,
                     int* kill_flag)
{
  unsigned char data[PLAYER_IDENT_STRLEN];
  socklen_t addrlen = sizeof(struct sockaddr_in);

  int j = this->num_clients;
  // Do we need to allocate another spot?
  if(j == this->size_clients)
  {
    this->size_clients++;
    this->clients = (playerudp_conn_t*)realloc(this->clients,
                                               this->size_clients *
                                               sizeof(playerudp_conn_t));
    assert(this->clients);
  }

  memset(this->clients + j, 0, sizeof(playerudp_conn_t));
  // Store the client's info
  this->clients[j].valid = 1;
  this->clients[j].del = 0;
  this->clients[j].host = local_host;
  this->clients[j].port = local_port;
  this->clients[j].fd = sock;
  assert(cliaddr);
  this->clients[j].addr = *cliaddr;
  this->clients[j].dev_subs = NULL;
  this->clients[j].num_dev_subs = 0;
  this->clients[j].kill_flag = kill_flag;

  // Create an outgoing queue for this client
  this->clients[j].queue =
          QueuePointer(0,PLAYER_MSGQUEUE_DEFAULT_MAXLEN);

  // Create a buffer to hold incoming messages
  this->clients[j].readbuffersize = PLAYERUDP_READBUFFER_SIZE;
  this->clients[j].readbuffer =
          (char*)calloc(1,this->clients[j].readbuffersize);
  assert(this->clients[j].readbuffer);
  this->clients[j].readbufferlen = 0;

  // Create a buffer to hold outgoing messages
  this->clients[j].writebuffersize = PLAYERUDP_WRITEBUFFER_SIZE;
  this->clients[j].writebuffer =
          (char*)calloc(1,this->clients[j].writebuffersize);
  assert(this->clients[j].writebuffer);
  this->clients[j].writebufferlen = 0;

  this->num_clients++;

  if(send_banner)
  {
    memset(data,0,sizeof(data));
    snprintf((char*)data, sizeof(data)-1, "%s%s",
             PLAYER_IDENT_STRING, playerversion);
    if(sendto(this->clients[j].fd, (void*)data, PLAYER_IDENT_STRLEN, 0,
              (struct sockaddr*)&this->clients[j].addr, addrlen) < 0)
    {
      PLAYER_ERROR("failed to send ident string");
    }
  }

  PLAYER_MSG2(1, "accepted UDP client %d on port %d", j, this->clients[j].port);

  return(this->clients[j].queue);
}

void
PlayerUDP::Close(int cli)
{
  assert((cli >= 0) && (cli < this->num_clients));

  PLAYER_MSG2(1, "closing UDP connection to client %d on port %d",
              cli, this->clients[cli].port);

  for(size_t i=0;i<this->clients[cli].num_dev_subs;i++)
  {
    Device* dev = this->clients[cli].dev_subs[i];
    {
      if(dev)
        dev->Unsubscribe(this->clients[cli].queue);
    }
  }
  free(this->clients[cli].dev_subs);
  fileWatcher->RemoveFileWatch(this->clients[cli].fd);
  this->clients[cli].fd = -1;
  this->clients[cli].valid = 0;
  // FIXME
  // We can't delete the queue here, because there may be one or more
  // drivers that have pending requests from this client.  Such a driver
  // will try to push the reply onto this queue, causing a segfault or
  // deadlock.  So we'll empty the queue and just leave it hanging around,
  // which is a (small) memory leak.
  //delete this->clients[cli].queue;
  Message* msg;
  while((msg = this->clients[cli].queue->Pop()))
    delete msg;
  free(this->clients[cli].readbuffer);
  free(this->clients[cli].writebuffer);
  if(this->clients[cli].kill_flag)
    *(this->clients[cli].kill_flag) = 1;
}

int
PlayerUDP::Read(int timeout)
{
  struct sockaddr_in fromaddr;
  socklen_t fromlen = sizeof(fromaddr);
  int num_available;
  playerudp_conn_t* client;
  int cli;

  // Poll for incoming messages
  if((num_available = poll(this->listen_ufds, num_listeners, timeout)) < 0)
  {
    // Got interrupted by a signal; no problem
    if(errno == EINTR)
      return(0);

    // A genuine problem
    PLAYER_ERROR1("poll() failed: %s", strerror(errno));
    return(-1);
  }

  if(!num_available)
    return(0);

  for(int i=0; (i<this->num_listeners) && (num_available>0); i++)
  {
    if(this->listen_ufds[i].revents & POLLIN)
    {
      /* Read into the decode_readbuffer, which is big enough to hold a
       * single UDP datagram */
      if((this->decode_readbufferlen = recvfrom(this->listen_ufds[i].fd,
                                                this->decode_readbuffer,
                                                PLAYERUDP_READBUFFER_SIZE,
                                                0,
                                                (struct sockaddr*)&fromaddr,
                                                &fromlen)) < 0)
      {
        PLAYER_ERROR2("recvfrom() failed on port %d: %s",
                      this->listeners[i].port,
                      strerror(errno));
      }
      else
      {
        pthread_mutex_lock(&this->clients_mutex);

        // Do we know about this one already?
        for(cli=0; cli<this->num_clients; cli++)
        {
          client = this->clients + cli;
          if((client->addr.sin_addr.s_addr == fromaddr.sin_addr.s_addr) &&
             (client->addr.sin_port == fromaddr.sin_port))
          {
            // Matched.

            // An empty datagram signals a new client, even if he's
            // using an old port
            if(!this->decode_readbufferlen)
            {
              client->del = 1;
              cli = this->num_clients;
              break;
            }

            // Might we need more room to assemble the current partial message?
            if((client->readbuffersize - client->readbufferlen) <
               this->decode_readbufferlen)
            {
              // Get twice as much space.
              client->readbuffersize *= 2;
              // Did we hit the limit (or overflow and become negative)?
              if((client->readbuffersize >= PLAYERXDR_MAX_MESSAGE_SIZE) ||
                 (client->readbuffersize < 0))
              {
                PLAYER_WARN2("allocating maximum %d bytes to client %d's read buffer",
                             PLAYERXDR_MAX_MESSAGE_SIZE, cli);
                client->readbuffersize = PLAYERXDR_MAX_MESSAGE_SIZE;
              }
              client->readbuffer = (char*)realloc(client->readbuffer,
                                                  client->readbuffersize);
              assert(client->readbuffer);
              memset(client->readbuffer + client->readbufferlen, 0,
                     client->readbuffersize - client->readbufferlen);
            }

            // Having allocated more space, are we full?
            if((client->readbuffersize - client->readbufferlen) <
               this->decode_readbufferlen)
            {
              PLAYER_WARN2("client %d's buffer is full (%d bytes)",
                           cli, client->readbufferlen);
            }
            else
            {
              // Copy the new datagram into the client's buffer
              memcpy(client->readbuffer + client->readbufferlen,
                     this->decode_readbuffer, this->decode_readbufferlen);
              client->readbufferlen += this->decode_readbufferlen;

              // Try to parse the data received so far
              this->ParseBuffer(cli);
            }

            break;
          }
        }

        this->DeleteClients();

        if(cli >= this->num_clients)
        {
          // No match; must be a new client
          this->AddClient(&fromaddr,
                          this->host,
                          this->listeners[i].port,
                          this->listeners[i].fd,
                          true, NULL);

          if(this->decode_readbufferlen > 0)
          {
            PLAYER_WARN1("non-empty (%u bytes) initial message from UDP client",
                         this->decode_readbufferlen);
          }
        }

        num_available--;

        pthread_mutex_unlock(&this->clients_mutex);
      }
    }
  }

  return(0);
}

// Should be called with clients_mutex lock held
void
PlayerUDP::DeleteClients()
{
  int num_deleted=0;
  // Delete those connections that generated errors in this iteration
  for(int i=0; i<this->num_clients; i++)
  {
    if(this->clients[i].del)
    {
      this->clients[i].valid = 0;
      this->Close(i);
      num_deleted++;
    }
  }
  // Delete those connections that generated errors in this iteration
  for(int i=0; i<this->num_clients; i++)
  {
    if(this->clients[i].valid && this->clients[i].del)
    {
      this->Close(i);
      num_deleted++;
    }
  }

  this->num_clients -= num_deleted;

  // Remove the resulting blank from both lists
  for(int i=0,j=0; i<this->size_clients; i++)
  {
    if(this->clients[j].del)
    {
      memmove(this->clients + j,
              this->clients + j + 1,
              (this->size_clients - j - 1) * sizeof(playerudp_conn_t));
      //memmove(this->client_ufds + j,
              //this->client_ufds + j + 1,
              //(this->size_clients - j - 1) * sizeof(struct pollfd));
    }
    else
      j++;
  }
  assert(this->num_clients <= this->size_clients);
  memset(this->clients + this->num_clients, 0,
         (this->size_clients - this->num_clients) * sizeof(playerudp_conn_t));
  //memset(this->client_ufds + this->num_clients, 0,
         //(this->size_clients - this->num_clients) * sizeof(struct pollfd));
}

// Should be called with clients_mutex lock held
void
PlayerUDP::DeleteClient(MessageQueue* q)
{
  // Find the client and mark it for deletion.
  int i;
  for(i=0;i<this->num_clients;i++)
  {
    if(this->clients[i].queue == q)
    {
      this->clients[i].del = 1;
      break;
    }
  }
}

bool
PlayerUDP::Listening(int port)
{
  for(int i=0;i<this->num_listeners;i++)
  {
    if(port == this->listeners[i].port)
      return(true);
  }
  return(false);
}

int
PlayerUDP::WriteClient(int cli)
{
  int numwritten;
  playerudp_conn_t* client;
  Message* msg;
  player_pack_fn_t packfunc;
  player_msghdr_t hdr;
  void* payload;
  int encode_msglen;
  socklen_t addrlen = sizeof(struct sockaddr_in);
#if HAVE_ZLIB_H
  player_map_data_t* zipped_data=NULL;
#endif

  //printf("WriteClient(%d)\n", cli);

  client = this->clients + cli;
  for(;;)
  {
    // try to send any bytes leftover from last time.
    if(client->writebufferlen)
    {
      numwritten = sendto(client->fd,
                          client->writebuffer,
                          MIN(client->writebufferlen,
                              PLAYERUDP_WRITEBUFFER_SIZE), 0,
                          (struct sockaddr*)&client->addr, addrlen);

      if(numwritten < 0)
      {
        if(errno == EAGAIN)
        {
          // buffers are full
          return(0);
        }
        else
        {
          PLAYER_MSG1(2,"sendto() failed: %s", strerror(errno));
          return(-1);
        }
      }
      else if(numwritten == 0)
      {
        PLAYER_MSG0(2,"wrote zero bytes");
        return(-1);
      }

      memmove(client->writebuffer, client->writebuffer + numwritten,
              client->writebufferlen - numwritten);
      client->writebufferlen -= numwritten;
    }
    // try to pop a pending message
    else if((msg = client->queue->Pop()))
    {
      // Note that we make a COPY of the header.  This is so that we can
      // edit the size field before sending it out, without affecting other
      // instances of the message on other queues.
      hdr = *msg->GetHeader();
      payload = msg->GetPayload();
      // Locate the appropriate packing function
      if(!(packfunc = playerxdr_get_packfunc(hdr.addr.interf,
                                         hdr.type, hdr.subtype)))
      {
        // TODO: Allow the user to register a callback to handle unsupported
        // messages
        PLAYER_WARN4("skipping message from %s:%u with unsupported type %s:%u",
                     interf_to_str(hdr.addr.interf), hdr.addr.index, msgtype_to_str(hdr.type), hdr.subtype);
      }
      else
      {
        // Make sure there's room in the buffer for the encoded messsage.
        // 4 times the message is a safe upper bound
        size_t maxsize = PLAYERXDR_MSGHDR_SIZE + (4 * msg->GetDataSize());
        if(maxsize > (size_t)(client->writebuffersize))
        {
          // Get at least twice as much space
          client->writebuffersize = MAX((size_t)(client->writebuffersize * 2),
                                        maxsize);
          // Did we hit the limit (or overflow and become negative)?
          if((client->writebuffersize >= PLAYERXDR_MAX_MESSAGE_SIZE) ||
             (client->writebuffersize < 0))
          {
            PLAYER_WARN1("allocating maximum %d bytes to outgoing message buffer",
                         PLAYERXDR_MAX_MESSAGE_SIZE);
            client->writebuffersize = PLAYERXDR_MAX_MESSAGE_SIZE;
          }
          client->writebuffer = (char*)realloc(client->writebuffer,
                                               client->writebuffersize);
          assert(client->writebuffer);
          memset(client->writebuffer, 0, client->writebuffersize);
        }

        // HACK: special handling for map data to compress it before sending
        // them out over the network.
        if((hdr.addr.interf == PLAYER_MAP_CODE) &&
           (hdr.type == PLAYER_MSGTYPE_RESP_ACK) &&
           (hdr.subtype == PLAYER_MAP_REQ_GET_DATA))
        {
#if HAVE_ZLIB_H
            player_map_data_t* raw_data = (player_map_data_t*)payload;
            zipped_data = (player_map_data_t*)calloc(1,sizeof(player_map_data_t));
            assert(zipped_data);

            // copy the metadata
            *zipped_data = *raw_data;
            uLongf count = compressBound(raw_data->data_count);
            zipped_data->data = (int8_t*)malloc(count);

            // compress the tile
            if(compress((Bytef*)zipped_data->data,&count,
                        (const Bytef*)raw_data->data, raw_data->data_count) != Z_OK)
            {
              PLAYER_ERROR("failed to compress map data");
              free(zipped_data);
              client->writebufferlen = 0;
              delete msg;
              return(0);
            }

            zipped_data->data_count = count;

            // swap the payload pointer to point at the zipped version
            payload = (void*)zipped_data;
#else
          PLAYER_WARN("not compressing map data, because zlib was not found at compile time");
#endif
        }

        // Encode the body first
        if((encode_msglen =
            (*packfunc)(client->writebuffer + PLAYERXDR_MSGHDR_SIZE,
                        maxsize - PLAYERXDR_MSGHDR_SIZE,
                        payload, PLAYERXDR_ENCODE)) < 0)
        {
          PLAYER_WARN4("encoding failed on message from %s:%u with type %s:%u",
                       interf_to_str(hdr.addr.interf), hdr.addr.index, msgtype_to_str(hdr.type), hdr.subtype);
#if HAVE_ZLIB_H
          if(zipped_data)
          {
            free(zipped_data->data);
            free(zipped_data);
            zipped_data=NULL;
          }
#endif
          client->writebufferlen = 0;
          delete msg;
          return(0);
        }

        // Rewrite the size in the header with the length of the encoded
        // body, then encode the header.
        hdr.size = encode_msglen;
	if((encode_msglen =
	    player_msghdr_pack(client->writebuffer,
			       PLAYERXDR_MSGHDR_SIZE, &hdr,
			       PLAYERXDR_ENCODE)) < 0)
        {
          PLAYER_ERROR("failed to encode msg header");
#if HAVE_ZLIB_H
          if(zipped_data)
          {
            free(zipped_data->data);
            free(zipped_data);
            zipped_data=NULL;
          }
#endif
          client->writebufferlen = 0;
          delete msg;
          return(0);
        }

        client->writebufferlen = PLAYERXDR_MSGHDR_SIZE + hdr.size;
      }
      delete msg;
#if HAVE_ZLIB_H
      if(zipped_data)
      {
        free(zipped_data->data);
        free(zipped_data);
        zipped_data=NULL;
      }
#endif
    }
    else
    {
      //puts("no messages");
      return(0);
    }
  }
}

int
PlayerUDP::Write()
{
  pthread_mutex_lock(&this->clients_mutex);

  for(int i=0;i<this->num_clients;i++)
  {
    if(this->WriteClient(i) < 0)
    {
      PLAYER_WARN1("failed to write to client %d\n", i);
      this->clients[i].del = 1;
    }
  }


  this->DeleteClients();
  pthread_mutex_unlock(&this->clients_mutex);

  return(0);
}

void
PlayerUDP::ParseBuffer(int cli)
{
  player_msghdr_t hdr;
  playerudp_conn_t* client=NULL;
  player_pack_fn_t packfunc=NULL;
  int msglen=0;
  int decode_msglen=0;
  Device* device=NULL;

  assert((cli >= 0) && (cli < this->num_clients));
  client = this->clients + cli;

  // Process one message in each iteration
  for(;;)
  {
    // Do we have enough bytes to read the header?
    if(client->readbufferlen < PLAYERXDR_MSGHDR_SIZE)
      return;

    // Try to read the header
    if(player_msghdr_pack(client->readbuffer,
                          PLAYERXDR_MSGHDR_SIZE,
                          &hdr, PLAYERXDR_DECODE) < 0)
    {
      PLAYER_WARN("failed to unpack header on incoming message");
      return;
    }

    msglen = PLAYERXDR_MSGHDR_SIZE + hdr.size;

    // Is the message of a legal size?
    if(msglen > PLAYERXDR_MAX_MESSAGE_SIZE)
    {
      PLAYER_WARN2("incoming message is larger than max (%d > %d); truncating",
                   msglen, PLAYERXDR_MAX_MESSAGE_SIZE);
      msglen = PLAYERXDR_MAX_MESSAGE_SIZE;
    }

    // Is it all here yet?
    if(msglen > client->readbufferlen)
      return;

    // Using TCP, the host and robot (port) information is in the connection
    // and so we don't require that the client fill it in.
    hdr.addr.host = client->host;
    hdr.addr.robot = client->port;
    device = deviceTable->GetDevice(hdr.addr,false);
    if(!device && (hdr.addr.interf != PLAYER_PLAYER_CODE))
    {
      PLAYER_WARN6("skipping message of type %s:%u to unknown device %u:%u:%s:%u",
                   msgtype_to_str(hdr.type), hdr.subtype,
                   hdr.addr.host, hdr.addr.robot,
                   interf_to_str(hdr.addr.interf), hdr.addr.index);
    }
    else
    {
      // Iff there's a payload to pack, locate the appropriate packing
      // function
      if( hdr.size > 0 &&
	  !(packfunc = playerxdr_get_packfunc(hdr.addr.interf,
					  hdr.type,
					  hdr.subtype)))
      {
	  // TODO: Allow the user to register a callback to handle unsupported
	  // messages
	  PLAYER_WARN4("skipping message to %s:%u with unsupported type %s:%u",
		       interf_to_str(hdr.addr.interf), hdr.addr.index, msgtype_to_str(hdr.type), hdr.subtype);
      }
      else
      {
        if( packfunc )
        {
          decode_msglen =
            (*packfunc)(client->readbuffer + PLAYERXDR_MSGHDR_SIZE,
			msglen - PLAYERXDR_MSGHDR_SIZE,
			(void*)this->decode_readbuffer,
			PLAYERXDR_DECODE);
        }
        else // no packing function? this had better be an empty message
        {
          if( hdr.size == 0 )
	        decode_msglen = 0; // an empty message decoded is still empty
          else
	      decode_msglen = -1; // indicate error
        }

        if( decode_msglen < 0 )
        {
          PLAYER_WARN4("decoding failed on message to %s:%u with type %s:%u",
		       interf_to_str(hdr.addr.interf), hdr.addr.index, msgtype_to_str(hdr.type), hdr.subtype);
        }
        else
        {
          // update the message size and send it off
          hdr.size = decode_msglen;
          void * msg_data = hdr.size? this->decode_readbuffer: NULL;

          if(hdr.addr.interf == PLAYER_PLAYER_CODE)
          {
            Message* msg = new Message(hdr, msg_data, client->queue);
            assert(msg);
            this->HandlePlayerMessage(cli, msg);
            delete msg;

            // Non-obvious thing: as a result of HandlePlayerMessage(), the
            // list of clients can get realloc()ed, which can
            // invalidate our client pointer.  So we'll recompute it.
            client = this->clients + cli;
          }
          else
          {
            // HACK: special handling for map data to uncompress it after sending
            // over the network.
            if((hdr.addr.interf == PLAYER_MAP_CODE) &&
               (hdr.type == PLAYER_MSGTYPE_RESP_ACK) &&
               (hdr.subtype == PLAYER_MAP_REQ_GET_DATA))
            {
#if HAVE_ZLIB_H
              player_map_data_t* zipped_data =
                      (player_map_data_t*)this->decode_readbuffer;
              player_map_data_t* raw_data =
                      (player_map_data_t*)calloc(1,sizeof(player_map_data_t));
              assert(raw_data);

              // copy the metadata
              raw_data->col = zipped_data->col;
              raw_data->row = zipped_data->row;
              raw_data->width = zipped_data->width;
              raw_data->height = zipped_data->height;
              uLongf count = 10*zipped_data->data_count;
              raw_data->data = (int8_t*)calloc(count,sizeof(int8_t));

              // uncompress the tile
              if(uncompress((Bytef*)raw_data->data,&count,
                            (const Bytef*)zipped_data->data,
                            (uLongf)zipped_data->data_count) != Z_OK)
              {
                PLAYER_ERROR("failed to uncompress map data");
              }
              else
              {
                raw_data->data_count = count;
                device->PutMsg(client->queue, &hdr, raw_data);
              }
              free(raw_data->data);
              free(raw_data);
#else
              PLAYER_WARN("not uncompressing map data, because zlib was not found at compile time");
              device->PutMsg(client->queue, &hdr, msg_data);
#endif
            }
            else
              device->PutMsg(client->queue, &hdr, msg_data);
          }
          // Need to ensure that the copy of any dynamic data made during unpacking
          // is cleaned up (putting message bodies into a Message class, as with PutMsg,
          // makes another copy of this data that will be cleaned up when that Message
          // class destructs).
          playerxdr_cleanup_message(this->decode_readbuffer, hdr.addr.interf, hdr.type, hdr.subtype);
        }
      }
    }

    // Move past the processed message
    memmove(client->readbuffer,
            client->readbuffer + msglen,
            client->readbufferlen - msglen);
    client->readbufferlen -= msglen;
  }
}

int
PlayerUDP::HandlePlayerMessage(int cli, Message* msg)
{
  player_msghdr_t* hdr;
  void* payload;
  playerudp_conn_t* client;
  int sub_result;

  player_msghdr_t resphdr;
  Message* resp;

  assert((cli >= 0) && (cli < this->num_clients));
  client = this->clients + cli;

  hdr = msg->GetHeader();
  payload = msg->GetPayload();

  resphdr = *hdr;
  GlobalTime->GetTimeDouble(&resphdr.timestamp);

  switch(hdr->type)
  {
    case PLAYER_MSGTYPE_REQ:
      switch(hdr->subtype)
      {
        // Device subscription request
        case PLAYER_PLAYER_REQ_DEV:
        {
          player_device_req_t* devreq;
          Device* device;
          player_device_req_t devresp;

          devreq = (player_device_req_t*)payload;

          // Using TCP, the host and robot (port) information is
          // in the connection and so we don't require that the client
          // fill it in.
          devreq->addr.host = this->host;
          devreq->addr.robot = client->port;
          if(!(device = deviceTable->GetDevice(devreq->addr,false)))
          {
            PLAYER_WARN2("skipping subscription to unknown device %s:%u",
                         interf_to_str(devreq->addr.interf), devreq->addr.index);
            resphdr.type = PLAYER_MSGTYPE_RESP_NACK;
            player_device_req_t devresp = *devreq;
            devresp.access = PLAYER_ERROR_MODE;
            devresp.driver_name_count = 0;

            // Make up and push out the reply
            resp = new Message(resphdr, &devresp);
            assert(resp);
            client->queue->Push(*resp);
            delete resp;
          }
          else
          {
            resphdr.type = PLAYER_MSGTYPE_RESP_ACK;

            memset(&devresp,0,sizeof(player_device_req_t));
            devresp.addr = devreq->addr;
            devresp.access = PLAYER_ERROR_MODE;
            devresp.driver_name_count = 0;

            // copy in the driver name
            strncpy(devresp.driver_name,device->drivername,
                    sizeof(devresp.driver_name));
            devresp.driver_name[sizeof(devresp.driver_name)-1] = '\0';
            devresp.driver_name_count = strlen(devresp.driver_name) + 1;
            // (un)subscribe the client to the device
            switch(devreq->access)
            {
              case PLAYER_OPEN_MODE:
                // Subscribe to the device
                sub_result = device->Subscribe(client->queue);

                // Non-obvious thing: as a result of Subscribe(), the
                // list of clients can get realloc()ed, which can
                // invalidate our client pointer.  So we'll recompute it.
                client = this->clients + cli;

                if(sub_result < 0)
                {
                  PLAYER_WARN2("subscription failed for device %s:%u",
                               interf_to_str(devreq->addr.interf), devreq->addr.index);
                }
                else
                {
                  devresp.access = devreq->access;
                  // record that we subscribed
                  size_t i;
                  for(i=0;i<client->num_dev_subs;i++)
                  {
                    if(!client->dev_subs[i])
                      break;
                  }
                  if(i==client->num_dev_subs)
                  {
                    client->num_dev_subs++;
                    client->dev_subs =
                            (Device**)realloc(client->dev_subs,
                                              sizeof(Device*)*
                                              client->num_dev_subs);
                    assert(client->dev_subs);
                  }
                  client->dev_subs[i] = device;
                }


                break;
              case PLAYER_CLOSE_MODE:
                if(device->Unsubscribe(client->queue) != 0)
                {
                  PLAYER_WARN2("unsubscription failed for device %s:%u",
                               interf_to_str(devreq->addr.interf), devreq->addr.index);
                }
                else
                {
                  devresp.access = devreq->access;
                  // record that we unsubscribed
                  size_t i;
                  for(i=0;i<client->num_dev_subs;i++)
                  {
                    if(client->dev_subs[i] == device)
                      break;
                  }
                  if(i==client->num_dev_subs)
                    PLAYER_WARN("failed to record unsubscription");
                  else
                    client->dev_subs[i] = NULL;
                }
                break;
              default:
                PLAYER_WARN3("unknown access mode %u requested for device %s:%u",
                             devreq->access, interf_to_str(devreq->addr.interf),
                             devreq->addr.index);
                break;
            }

            // Make up and push out the reply
            resp = new Message(resphdr, (void*)&devresp,
                               sizeof(player_device_req_t));
            assert(resp);
            client->queue->Push(*resp);
            delete resp;
          }

          break;
        }

        // Request for device list
        case PLAYER_PLAYER_REQ_DEVLIST:
        {
          player_device_devlist_t devlist;

          int numdevices=0;
          for(Device* device = deviceTable->GetFirstDevice();
              device;
              device = deviceTable->GetNextDevice(device))
          {
            if(numdevices == PLAYER_MAX_DEVICES)
            {
              PLAYER_WARN("truncating available device list");
              break;
            }
            if((int)device->addr.robot == client->port && (device->addr.host==host))
              devlist.devices[numdevices++] = device->addr;
          }
          devlist.devices_count = numdevices;

          resphdr.type = PLAYER_MSGTYPE_RESP_ACK;
          // Make up and push out the reply
          resp = new Message(resphdr, (void*)&devlist,
                             sizeof(player_device_devlist_t));
          assert(resp);
          client->queue->Push(*resp);
          delete resp;
          break;
        }

        // Request for detailed info on a particular device
        case PLAYER_PLAYER_REQ_DRIVERINFO:
        {
          player_device_driverinfo_t* inforeq;
          player_device_driverinfo_t inforesp;
          Device* device;

          inforeq = (player_device_driverinfo_t*)payload;

          // Using TCP, the host and robot (port) information is
          // in the connection and so we don't require that the client
          // fill it in.
          inforeq->addr.host = this->host;
          inforeq->addr.robot = client->port;
          if(!(device = deviceTable->GetDevice(inforeq->addr,false)))
          {
            PLAYER_WARN2("skipping info request for unknown device %s:%u",
                         interf_to_str(inforeq->addr.interf), inforeq->addr.index);
            resphdr.type = PLAYER_MSGTYPE_RESP_NACK;

            // Make up and push out the reply
            resp = new Message(resphdr, NULL);
            assert(resp);
            client->queue->Push(*resp);
            delete resp;
          }
          else
          {
            memset(&inforesp,0,sizeof(inforesp));
            inforesp.addr = inforeq->addr;

            // copy in the driver name
            strncpy(inforesp.driver_name,device->drivername,
                    sizeof(inforesp.driver_name));
            inforesp.driver_name[sizeof(inforesp.driver_name)-1] = '\0';
            inforesp.driver_name_count = strlen(device->drivername) + 1;

            resphdr.type = PLAYER_MSGTYPE_RESP_ACK;
            // Make up and push out the reply
            resp = new Message(resphdr, (void*)&inforesp,
                               sizeof(player_device_driverinfo_t));
            assert(resp);
            client->queue->Push(*resp);
            delete resp;
          }
          break;
        }

        // Request for detailed info on a particular device
        case PLAYER_PLAYER_REQ_ADD_REPLACE_RULE:
        {
          player_add_replace_rule_req * req = reinterpret_cast<player_add_replace_rule_req *> (payload);
          client->queue->AddReplaceRule(-1,-1,req->interf, req->index, req->type, req->subtype, req->replace);
          resphdr.type = PLAYER_MSGTYPE_RESP_ACK;

          // Make up and push out the reply
          resp = new Message(resphdr, NULL);
          assert(resp);
          client->queue->Push(*resp);
          delete resp;
          break;
        }

        // Request change of data mode
        case PLAYER_PLAYER_REQ_DATAMODE:
        {
          player_device_datamode_req_t * req = reinterpret_cast<player_device_datamode_req_t *> (payload);
          if (req->mode == PLAYER_DATAMODE_PUSH)
            client->queue->SetPull (false);
          else if (req->mode == PLAYER_DATAMODE_PULL)
            client->queue->SetPull (true);
          else
            PLAYER_WARN1 ("unknown data mode requsted: %d", req->mode);
          // Make up and push out the reply
          resphdr.type = PLAYER_MSGTYPE_RESP_ACK;
          resp = new Message(resphdr, NULL);
          assert(resp);
          client->queue->Push(*resp);
          delete resp;
          break;
        }

        // Request data
        case PLAYER_PLAYER_REQ_DATA:
          // Make up and push out the reply
          resphdr.type = PLAYER_MSGTYPE_RESP_ACK;
          resp = new Message(resphdr, NULL);
          assert(resp);
          client->queue->PushFront(*resp,false);
          delete resp;
          // Remember that the user requested some
          client->queue->SetDataRequested(true,false);
          break;


        default:
          PLAYER_WARN1("player interface discarding message of unsupported "
                       "subtype %u", hdr->subtype);

          resphdr.type = PLAYER_MSGTYPE_RESP_NACK;
          GlobalTime->GetTimeDouble(&resphdr.timestamp);
          resphdr.size = 0;
          // Make up and push out the reply
          resp = new Message(resphdr, NULL);
          assert(resp);
          client->queue->Push(*resp);
          delete resp;
          break;
      }
      break;
    default:
      PLAYER_WARN1("player interface discarding message of unsupported type %u",
                   msgtype_to_str(hdr->type));
      resphdr.type = PLAYER_MSGTYPE_RESP_NACK;
      GlobalTime->GetTimeDouble(&resphdr.timestamp);
      resphdr.size = 0;
      // Make up and push out the reply
      resp = new Message(resphdr, NULL);
      assert(resp);
      client->queue->Push(*resp);
      delete resp;
      break;
  }
  return(0);
}

int
_create_and_bind_udp_socket(char blocking, unsigned int host, int portnum)
{
  int sock;                   /* socket we're creating */
  int flags;                  /* temp for old socket access flags */

  struct sockaddr_in serverp;

  int socktype;

  socktype = SOCK_DGRAM;

  memset(&serverp,0,sizeof(serverp));
  serverp.sin_addr.s_addr = host;
  serverp.sin_port = htons(portnum);

  /*
   * Create the INET socket.
   *
   */
  if((sock = socket(PF_INET, socktype, 0)) == -1)
  {
    perror("create_and_bind_socket:socket() failed; socket not created.");
    return(-1);
  }

  /*
   * let's say that our process should get the SIGIO's when it's
   * readable
   */
  if(fcntl(sock, F_SETOWN, getpid()) == -1)
  {
    /* I'm making this non-fatal because it always fails under Cygwin and
     * yet doesn't seem to matter -BPG */
    PLAYER_WARN("fcntl() failed while setting socket pid ownership");
  }

  if(!blocking)
  {
    /*
     * get the current access flags
     */
    if((flags = fcntl(sock, F_GETFL)) == -1)
    {
      perror("create_and_bind_socket():fcntl() while getting socket "
                      "access flags; socket not created.");
      close(sock);
      return(-1);
    }
    /*
     * OR the current flags with O_NONBLOCK (so we won't block),
     * and write them back
     */
    if(fcntl(sock, F_SETFL, flags | O_NONBLOCK ) == -1)
    {
      perror("create_and_bind_socket():fcntl() failed while setting socket "
                      "access flags; socket not created.");
      close(sock);
      return(-1);
    }
  }

  /*
   * Bind it to the port indicated
   * INADDR_ANY indicates that any network interface (IP address)
   * for the local host may be used (presumably the OS will choose the
   * right one).
   *
   * Specifying sin_port = 0 would allow the system to choose the port.
   */
  serverp.sin_family = PF_INET;
  serverp.sin_addr.s_addr = INADDR_ANY;

  if(bind(sock, (struct sockaddr*)&serverp, sizeof(serverp)) == -1)
  {
    perror ("create_and_bind_socket():bind() failed; socket not created.");
    close(sock);
    return(-1);
  }

  /*
   * return the fd for the newly bound socket
   */
  return(sock);
}
