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
 * Desc: Message class and message queues
 * CVS:  $Id: message.h 6450 2008-05-21 01:08:44Z thjc $
 * Author: Toby Collett - Jan 2005
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <pthread.h>

#include <libplayercore/player.h>

class MessageQueue;

/** @brief An autopointer for the message queue

Using an autopointer allows the queue to be released by the client and still exist 
until no drivers have messages relating to the queue still pending.

**/
class QueuePointer
{
  public:
    /// Create a NULL autopointer;
    QueuePointer();
    /// Create an empty message queue and an auto pointer to it.
    QueuePointer(bool _Replace, size_t _Maxlen);
    /// Destroy our reference to the message queue.
	~QueuePointer();
	/// Create a new reference to a message queue
	QueuePointer(const QueuePointer & CopyFrom);
	
	/// assign reference to our message queue
	QueuePointer & operator = (const QueuePointer & rhs);
	/// retrieve underlying object for use
	MessageQueue * operator -> ();
	/// retrieve underlying object for use, may return a null pointer
	MessageQueue * get() const;
	/// retrieve underlying object for use
	MessageQueue & operator * ();
	/// check if pointers are equal
	bool operator == (const QueuePointer & rhs);
	/// check if pointers are equal
	bool operator == (void * pointer);
	/// check if pointers are equal
	bool operator != (const QueuePointer & rhs);
	/// check if pointers are equal
	bool operator != (void * pointer);
	
  private:
    /// Decrement ref count
    void DecRef();

    /// The queue we are pointing to
    MessageQueue * Queue;

    /// Reference count.
    unsigned int * RefCount;

    /// Used to lock access to refcount.
    pthread_mutex_t * Lock;
};



/** @brief Reference-counted message objects

All Player messages are transferred between drivers as pointers to Message
objects.  These objects are reference-counted so that messages can be
delivered to multiple recipients with minimal memory overhead.

Messages are not usually manipulated directly in driver code.  The details
of allocating, filling, parsing, and deleting Message objects are handled
by the Driver and MessageQueue classes.

The only method of interest to driver authors is the helper MatchMessage(),
which can be used in a Driver::ProcessMessage method to determine if a
message header matches a given signature.
*/
class Message
{
  public:
    /// Create a new message. If copy is set to false then the pointer is claimed by the message, 
    /// otherwise it is copied.
    Message(const struct player_msghdr & Header,
            void* data,
            bool copy = true);

    /// Create a new message with an associated queue. If copy is set to false then the pointer is 
    /// claimed by the message, otherwise it is copied.
    Message(const struct player_msghdr & Header,
            void* data,
            QueuePointer &_queue,
            bool copy = true);

    /// Copy pointers from existing message and increment refcount.
    Message(const Message & rhs);

    /// Destroy message, dec ref counts and delete data if ref count == 0
    ~Message();

    /** @brief Helper for message processing.

    Returns true if @p hdr matches the supplied @p type, @p subtype,
    and @p addr.  Set type and/or subtype to -1 for don't care.
    */
    static bool MatchMessage(player_msghdr_t* hdr,
                             int type,
                             int subtype,
                             player_devaddr_t addr)
    {
      return(((type < 0) || (hdr->type == (uint8_t)type)) &&
             ((subtype < 0) || (hdr->subtype == (uint8_t)subtype)) &&
             (hdr->addr.host == addr.host) &&
             (hdr->addr.robot == addr.robot) &&
             (hdr->addr.interf == addr.interf) &&
             (hdr->addr.index == addr.index));
    }

    /** @brief Helper for message processing.

    This version matches for any message destination (i.e. all a drivers reigstered interfaces)
    Returns true if @p hdr matches the supplied @p type, @p subtype.  Set type and/or subtype to -1 for don't care.
    */
    static bool MatchMessage(player_msghdr_t* hdr,
                             int type,
                             int subtype)
    {
      return(((type < 0) || (hdr->type == (uint8_t)type)) &&
             ((subtype < 0) || (hdr->subtype == (uint8_t)subtype)));
    }    
    
    /// Get pointer to header.
    player_msghdr_t * GetHeader() {return &Header;};
    /// Get pointer to payload.
    void* GetPayload() {return (void*)Data;};
    /// Size of message data.
    unsigned int GetDataSize() {return Header.size;};
    /// Compare type, subtype, device, and device_index.
    bool Compare(Message &other);
    /// Decrement ref count
    void DecRef();

    /// queue to which any response to this message should be directed
    QueuePointer Queue;

    /// Reference count.
    unsigned int * RefCount;

  private:
    void CreateMessage(const struct player_msghdr & Header,
            void* data,
            bool copy = true);
	  
    /// message header
    player_msghdr_t Header;
    /// Pointer to the message data.
    uint8_t * Data;
    /// Used to lock access to Data.
    pthread_mutex_t * Lock;
};

/**
 This class is a helper for maintaining doubly-linked queues of Messages.
*/
class MessageQueueElement
{
  public:
    /// Create a queue element with NULL prev and next pointers.
    MessageQueueElement();
    /// Destroy a queue element.
    ~MessageQueueElement();

    /// The message stored in this queue element.
    Message* msg;
  private:
    /// Pointer to previous queue element.
    MessageQueueElement * prev;
    /// Pointer to next queue element.
    MessageQueueElement * next;

    friend class MessageQueue;
};

/** We keep a singly-linked list of (addr,type,subtype,replace) tuples.
 * When a new message comes in, we check its (addr,type,subtype) signature
 * against this list to find a replace rule.  If its not in the list, the
 * default rule is used: never replace config requests or replies, replace
 * data and command msgs if the Replace flag is set.
 */
class MessageReplaceRule
{
  private:
    // The address to match (not using a player_devaddr_t so that we can
    // use -1 to indicate don't care)
    int host, robot, interf, index;
    // The type and subtype to match (-1 is don't care)
    int type, subtype;
  public:
    MessageReplaceRule(int _host, int _robot, int _interf, int _index,
                       int _type, int _subtype, int _replace) :
            host(_host), robot(_robot), interf(_interf), index(_index),
            type(_type), subtype(_subtype), replace(_replace), next(NULL) {}

    bool Match(player_msghdr_t* hdr)
    {
      return(((this->host < 0) ||
              ((uint32_t)this->host == hdr->addr.host)) &&
             ((this->robot < 0) ||
              ((uint32_t)this->robot == hdr->addr.robot)) &&
             ((this->interf < 0) ||
              ((uint16_t)this->interf == hdr->addr.interf)) &&
             ((this->index < 0) ||
              ((uint16_t)this->index == hdr->addr.index)) &&
             ((this->type < 0) ||
              ((uint8_t)this->type == hdr->type)) &&
             ((this->subtype < 0) ||
              ((uint8_t)this->subtype == hdr->subtype)));
    }

    bool Equivalent (int _host, int _robot, int _interf, int _index, int _type, int _subtype)
    {
      return (host == _host && robot == _robot && interf ==_interf && index == _index &&
          type == _type && subtype == _subtype);
    }

    // To replace, or not to replace
    // That is the question
    int replace;
    // Next rule in the list
    MessageReplaceRule* next;
};

/** @brief A doubly-linked queue of messages.

Player Message objects are delivered by being pushed on and popped off
queues of this type.  Importantly, every driver has one, Driver::InQueue.
All messages sent to the driver arrive on this queue.  However, the driver
rarely pops messages off its queue directly; instead the driver calls
Driver::ProcessMessages, which hands off each incoming message to
Driver::ProcessMessage (which the driver has presumably re-implemented).

Every queue has a maximum length that is determined when it is created; for
drivers this happens in the constructor Driver::Driver.  This length
determines the maximum number of messages that can be pushed onto the
queue.  Since messages vary greatly in size, there is not a direct
correspondence between the length of a queue and the memory that it
occupies.  Furthermore, since messages are reference counted and may be
shared across multiple queues, it is not necessarily meaningful to consider
how much memory a given queue "occupies."

The queue supports configurable message replacement.  This functionality is
useful when, for example, a driver wants new incoming commands to overwrite
old ones, rather than to have them queue up.  To decide whether an incoming
message should replace an existing message that has the same address
(host:robot:interface:index), type, and subtype, the following logic is
applied:

- If a matching replacement rule was set via AddReplaceRule(), that rule is
  followed.
- Else:
  - If the message type is PLAYER_MSGTYPE_REQ, PLAYER_MSGTYPE_RESP_ACK, or
    PLAYER_MSGTYPE_RESP_NACK, the message is not replaced.
  - Else:
    - If MessageQueue::Replace is false (it is set in the constructor and
      can be changed via SetReplace()), the message is not replaced.
    - Else:
      - The message is replaced.

Most drivers can simply choose true or false in their constructors (this
setting is passed on to the MessageQueue constructor).  However, drivers
that support multiple interfaces may use AddReplaceRule() to establish
different replacement rules for messages that arrive for different
interfaces.  For example, the @ref driver_p2os driver has incoming commands
to the wheelmotors overwrite each other, but queues up commands to the
manipulator arm.

The queue also supports filtering based on device address.  After
SetFilter() is called, Pop() will only return messages that match the given
filter.  Use ClearFilter() to return to normal operation.  This filter is
not usually manipulated directly in driver code; it's main use inside
Device::Request.

*/
class MessageQueue
{
  public:
    /// Create an empty message queue.
    MessageQueue(bool _Replace, size_t _Maxlen);
    /// Destroy a message queue.
    ~MessageQueue();
    /// Check whether a queue is empty
    bool Empty() { return(this->head == NULL); }
    /** Push a message onto the queue.  Returns the success state of the Push
    operation (true if successful, false otherwise). */
    bool Push(Message& msg);

    /// Put it at the front of the queue, without checking replacement rules
    /// or size limits.
    /// This method is used to insert responses to requests for data.
    /// The caller may have already called Lock() on this queue
    void PushFront(Message & msg, bool haveLock);

    /// Push a message at the back of the queue, without checking replacement 
    /// rules or size limits.
    /// This method is used internally to insert most messages.
    /// The caller may have already called Lock() on this queue
    void PushBack(Message & msg, bool haveLock);

    /** Pop a message off the queue.
    Pop the head (i.e., the first-inserted) message from the queue.
    Returns pointer to said message, or NULL if the queue is empty */
    Message* Pop();
    /** Set the @p Replace flag, which governs whether data and command
    messages of the same subtype from the same device are replaced in
    the queue. */
    void SetReplace(bool _Replace) { this->Replace = _Replace; };
    /** Add a replacement rule to the list.  The first 6 arguments
     * determine the signature that a message will have to match in order
     * for this rule to be applied.  If an incoming message matches this
     * signature, the last argument determines whether replacement will
     * occur.  Set any of the first 6 arguments to -1 to indicate don't
     * care. */
    void AddReplaceRule(int _host, int _robot, int _interf, int _index,
                        int _type, int _subtype, int _replace);
    /** Add a replacement rule to the list.  Use this version if you
     * already have the device address assembled in a player_devaddr_t
     * structure.  The tradeoff is that you cannot use -1 to indicate don't
     * care for those values (the fields in that structure are unsigned).
     * */
    void AddReplaceRule(const player_devaddr_t &device,
                        int _type, int _subtype, int _replace);
    /// @brief Check whether a message with the given header should replace
    /// any existing message of the same signature, be ignored or accepted.
    int CheckReplace(player_msghdr_t* hdr);
    /** Wait on this queue.  This method blocks until new data is available
    (as indicated by a call to DataAvailable()). 
            
    If TimeOut is set to a positive value this method will return false if the
    timeout occurs before and update is recieved.
     */
    bool Wait(double TimeOut=0.0);
    /** Signal that new data is available.  Calling this method will
     release any threads currently waiting on this queue. */
    void DataAvailable(void);
    /// @brief Check whether a message passes the current filter.
    bool Filter(Message& msg);
    /// @brief Clear (i.e., turn off) message filter.
    void ClearFilter(void);
    /// @brief Set filter values
    void SetFilter(int host, int robot, int interf, int index,
                   int type, int subtype);
    /** Set the @p pull flag */
    void SetPull (bool _pull) { this->pull = _pull; }

    /// @brief Get current length of queue, in elements.
    size_t GetLength(void);

    /// @brief Set the data_requested flag
    void SetDataRequested(bool d, bool haveLock);

  private:
    /// @brief Lock the mutex associated with this queue.
    void Lock() {pthread_mutex_lock(&lock);};
    /// @brief Unlock the mutex associated with this queue.
    void Unlock() {pthread_mutex_unlock(&lock);};
    /** Remove element @p el from the queue, and rearrange pointers
    appropriately. */
    void Remove(MessageQueueElement* el);
    /// @brief Head of the queue.
    MessageQueueElement* head;
    /// @brief Tail of the queue.
    MessageQueueElement* tail;
    /// @brief Mutex to control access to the queue, via Lock() and Unlock().
    pthread_mutex_t lock;
    /// @brief Maximum length of queue in elements.
    size_t Maxlen;
    /// @brief Singly-linked list of replacement rules
    MessageReplaceRule* replaceRules;
    /// @brief When a (data or command) message doesn't match a rule in
    /// replaceRules, should we replace it?
    bool Replace;
    /// @brief Current length of queue, in elements.
    size_t Length;
    /// @brief A condition variable that can be used to signal, via
    /// DataAvailable(), other threads that are Wait()ing on this
    /// queue.
    pthread_cond_t cond;
    /// @brief Mutex to go with condition variable cond.
    pthread_mutex_t condMutex;
    /// @brief Current filter values
    bool filter_on;
    int filter_host, filter_robot, filter_interf,
        filter_index, filter_type, filter_subtype;
    /// @brief Flag for if in pull mode. If false, push mode. Push is default mode,
    /// but pull is the recommended method to avoid getting delays in data on the client.
    bool pull;
    /// @brief Flag for data was requested (in PULL mode), but none has yet
    /// been delivered
    bool data_requested;
    /// @brief Flag that data was sent (in PULL mode)
    bool data_delivered;
    /// @brief Count of the number of messages discarded due to queue overflow.
    bool drop_count;
};




#endif
