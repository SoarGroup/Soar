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

/*
 * Interface to libplayertcp
 *
 * $Id: playertcp.h 6410 2008-05-10 06:29:12Z thjc $
 */

/** @defgroup libplayertcp libplayertcp
 * @brief Player TCP library

This library moves messages between Player message queues and TCP sockets.

@section datamode_protocol Data modes

Clients can use two modes for receiving data. These are PUSH and PULL modes.
There are some important differences between them. These are summarised here.
These modes only affect the clients' message queues, that is they do not
affect how messages are received from clients.

In PUSH mode all messages are sent as soon as possible. This is usually when
there is room in the operating system's buffer for the message. Clients should
read these messages as usual. For example, the libplayerc client library will
read exactly one message for each call to the client read function.

In PULL mode:
<ul>
<li>Messages are only sent to the client when they are marked as ready in the
client's message queue.
<li>PLAYER_MSGTYPE_DATA messages are not marked as ready until the client
requests data.
<li>All other messages (PLAYER_MSGTYPE_RESP_ACK and PLAYER_MSGTYPE_RESP_NACK
messages) are marked as ready upon entering the queue.
<li>When a PLAYER_PLAYER_REQ_DATA message is received, all messages in the
client's queue are marked as ready. A PLAYER_MSGTYPE_SYNCH message is pushed
onto the end of the queue.
</ul>

A client in PULL mode should request data before performing a read by sending a
PLAYER_PLAYER_REQ_DATA request message. The client should then continue to
receive and handle all messages until it receives the PLAYER_MSGTYPE_SYNCH
message. Note that the PLAYER_MSGTYPE_RESP_ACK for the PLAYER_PLAYER_REQ_DATA
will come at the <i>end</i> of all other waiting data (but <i>before</i> the
PLAYER_MSGTYPE_SYNCH message), due to the way the message queue system
functions. This means that client libraries should read and store all other
messages when waiting for a PLAYER_MSGTYPE_RESP_ACK, then process them at
the beginning of their read function after sending the PLAYER_PLAYER_REQ_DATA
message.

@todo
 - More verbose documentation on this library, including the protocol
*/
/** @ingroup libplayertcp
@{ */

#ifndef _PLAYERTCP_H_
#define _PLAYERTCP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <libplayercore/playercore.h>

/** Default TCP port */
#define PLAYERTCP_DEFAULT_PORT 6665

/** We try to read() incoming messages in chunks of this size.  We also
    calloc() and realloc() read buffers in multiples of this size. */
#define PLAYERTCP_READBUFFER_SIZE 65536

/** We try to write() outgoing messages in chunks of this size.  We also
    calloc() and realloc() write buffers in multiples of this size. */
#define PLAYERTCP_WRITEBUFFER_SIZE 65536

// Forward declarations
struct pollfd;

struct playertcp_listener;
struct playertcp_conn;

class PlayerTCP
{
  private:
    uint32_t host;
    int num_listeners;
    playertcp_listener* listeners;
    struct pollfd* listen_ufds;

    pthread_mutex_t clients_mutex;
    int size_clients;
    int num_clients;
    playertcp_conn* clients;
    struct pollfd* client_ufds;

    /** Buffer in which to store decoded incoming messages */
    char* decode_readbuffer;
    /** Total size of @p decode_readbuffer */
    int decode_readbuffersize;


  public:
    PlayerTCP();
    ~PlayerTCP();

    void Lock();
    void Unlock();

    static void InitGlobals(void);

    pthread_t thread;

    int Listen(int* ports, int num_ports, int* new_ports=NULL);
    int Listen(int port);
    QueuePointer AddClient(struct sockaddr_in* cliaddr, 
                            unsigned int local_host,
                            unsigned int local_port,
                            int newsock,
                            bool send_banner,
                            int* kill_flag,
                            bool have_lock);
    int Update(int timeout);
    int Accept(int timeout);
    void Close(int cli);
    int ReadClient(int cli);
    int ReadClient(QueuePointer q);
    int Read(int timeout, bool have_lock);
    int Write(bool have_lock);
    int WriteClient(int cli);
    void DeleteClients();
    void ParseBuffer(int cli);
    int HandlePlayerMessage(int cli, Message* msg);
    void DeleteClient(QueuePointer &q, bool have_lock);
    bool Listening(int port);
    uint32_t GetHost() {return host;};
};

/** @} */

#endif
