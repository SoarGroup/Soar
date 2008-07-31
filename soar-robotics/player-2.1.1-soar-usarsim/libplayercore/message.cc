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
 * CVS:  $Id: message.cc 6578 2008-06-15 21:40:15Z thjc $
 * Author: Toby Collett - Jan 2005
 */

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <libplayercore/message.h>
#include <libplayercore/player.h>
#include <libplayercore/error.h>
#include <libplayercore/interface_util.h>
#include <libplayerxdr/playerxdr.h>
#include <replace/replace.h>

Message::Message(const struct player_msghdr & aHeader,
                  void * data,
                  bool copy) 
{
  CreateMessage(aHeader, data, copy);
}

Message::Message(const struct player_msghdr & aHeader,
                 void * data,
                 QueuePointer &_queue,
                 bool copy) : Queue(_queue)
{
  CreateMessage(aHeader, data, copy);
}

Message::Message(const Message & rhs)
{
  assert(rhs.Lock);
  pthread_mutex_lock(rhs.Lock);

  assert(rhs.RefCount);
  assert(*(rhs.RefCount));
  Lock = rhs.Lock;
  Data = rhs.Data;
  Header = rhs.Header;
  Queue = rhs.Queue;
  RefCount = rhs.RefCount;
  (*RefCount)++;

  pthread_mutex_unlock(rhs.Lock);
}

Message::~Message()
{
  this->DecRef();
}

void Message::CreateMessage(const struct player_msghdr & aHeader,
                  void * data,
                  bool copy) 
{
  this->Lock = new pthread_mutex_t;
  assert(this->Lock);
  pthread_mutex_init(this->Lock,NULL);
  this->RefCount = new unsigned int;
  assert(this->RefCount);
  *this->RefCount = 1;

  // copy the header and then the data into out message data buffer
  memcpy(&this->Header,&aHeader,sizeof(struct player_msghdr));
  if (data == NULL)
  {
    Data = NULL;
    Header.size = 0;
    return;
  }
  // Force header size to be same as data size
  player_sizeof_fn_t sizeoffunc;
  if((sizeoffunc = playerxdr_get_sizeoffunc(Header.addr.interf, Header.type, Header.subtype)) != NULL)
  {
    Header.size = (*sizeoffunc)(data);
  }

  if (copy)
  {
    player_clone_fn_t clonefunc = NULL;
    if((clonefunc = playerxdr_get_clonefunc(Header.addr.interf, Header.type, Header.subtype)) != NULL)
    {
      if ((this->Data = (uint8_t*)(*clonefunc)(data)) == NULL)
      {
        PLAYER_ERROR3 ("failed to clone message %s: %s, %d", interf_to_str (Header.addr.interf), msgtype_to_str (Header.type), Header.subtype);
      }
    }
    else
    {
      PLAYER_ERROR3 ("failed to find clone function for  message %s: %s, %d", interf_to_str (Header.addr.interf), msgtype_to_str (Header.type), Header.subtype);
    }
  }
  else
  {
    this->Data = (uint8_t*)data;
  }
}

bool
Message::Compare(Message &other)
{
  player_msghdr_t* thishdr = this->GetHeader();
  player_msghdr_t* otherhdr = other.GetHeader();
  return(Message::MatchMessage(thishdr,
                               otherhdr->type,
                               otherhdr->subtype,
                               otherhdr->addr));
};

void
Message::DecRef()
{
  pthread_mutex_lock(Lock);
  (*RefCount)--;
  assert((*RefCount) >= 0);
  if((*RefCount)==0)
  {
    if (Data)
      playerxdr_free_message (Data, Header.addr.interf, Header.type, Header.subtype);
    Data = NULL;
    delete RefCount;
    RefCount = NULL;
    pthread_mutex_unlock(Lock);
    pthread_mutex_destroy(Lock);
    delete Lock;
    Lock = NULL;
  }
  else
    pthread_mutex_unlock(Lock);
}

MessageQueueElement::MessageQueueElement()
{
  msg = NULL;
  prev = next = NULL;
}

MessageQueueElement::~MessageQueueElement()
{
}

MessageQueue::MessageQueue(bool _Replace, size_t _Maxlen)
{
  this->Replace = _Replace;
  this->Maxlen = _Maxlen;
  this->head = this->tail = NULL;
  this->Length = 0;
  pthread_mutex_init(&this->lock,NULL);
  pthread_mutex_init(&this->condMutex,NULL);
  pthread_cond_init(&this->cond,NULL);
  this->ClearFilter();
  this->filter_on = false;
  this->replaceRules = NULL;
  this->pull = false;
  this->data_requested = false;
  this->data_delivered = false;
  this->drop_count = 0;
}

MessageQueue::~MessageQueue()
{
  // clear the queue
  MessageQueueElement *e, *n;
  for(e = this->head; e;)
  {
    delete e->msg;
    n = e->next;
    delete e;
    e = n;
  }

  // clear the list of replacement rules
  MessageReplaceRule* tmp;
  MessageReplaceRule* curr = this->replaceRules;
  while(curr)
  {
    tmp = curr->next;
    delete curr;
    curr = tmp;
  }

  pthread_mutex_destroy(&this->lock);
  pthread_mutex_destroy(&this->condMutex);
  pthread_cond_destroy(&this->cond);
}

/// @brief Add a replacement rule to the list
void
MessageQueue::AddReplaceRule(int _host, int _robot, int _interf, int _index,
                             int _type, int _subtype, int _replace)
{
  MessageReplaceRule* curr;
  for(curr=this->replaceRules;curr;curr=curr->next)
  {
    // Check for an existing rule with the same criteria; replace if found
    if (curr->Equivalent (_host, _robot, _interf, _index, _type, _subtype))
    {
      curr->replace = _replace;
      return;
    }
	if (curr->next == NULL)
		// Break before for() increments if this is the last one in the list
		break;
  }
  if(!curr)
  {
    curr = replaceRules = new MessageReplaceRule(_host, _robot, _interf, _index,
                                  _type, _subtype, _replace);
//	assert(curr);  // This is bad. What happens if there's a memory allocation failure when not built with debug?
    if (!curr)
      PLAYER_ERROR ("memory allocation failure; could not add new replace rule");
  }
  else
  {
    curr->next = new MessageReplaceRule(_host, _robot, _interf, _index,
                                        _type, _subtype, _replace);
//	assert(curr->next);  // This is bad. What happens if there's a memory allocation failure when not built with debug?
    if (!curr->next)
      PLAYER_ERROR ("memory allocation failure; could not add new replace rule");
  }
}

/// @brief Add a replacement rule to the list
void
MessageQueue::AddReplaceRule(const player_devaddr_t &device,
                             int _type, int _subtype, int _replace)
{
  this->AddReplaceRule (device.host, device.robot, device.interf, device.index,
                        _type, _subtype, _replace);
}

int
MessageQueue::CheckReplace(player_msghdr_t* hdr)
{
  // First look through the replacement rules
  for(MessageReplaceRule* curr=this->replaceRules;curr;curr=curr->next)
  {
  	assert(curr);
    if(curr->Match(hdr))
      return(curr->replace);
  }

  // Didn't find it; follow the default rule

  // Don't replace config requests or replies
  if((hdr->type == PLAYER_MSGTYPE_REQ) ||
     (hdr->type == PLAYER_MSGTYPE_RESP_ACK) ||
     (hdr->type == PLAYER_MSGTYPE_RESP_NACK))
    return(PLAYER_PLAYER_MSG_REPLACE_RULE_ACCEPT);
  // Replace data and command according to the this->Replace flag
  else if((hdr->type == PLAYER_MSGTYPE_DATA) ||
          (hdr->type == PLAYER_MSGTYPE_CMD))
  {
    return(this->Replace ? PLAYER_PLAYER_MSG_REPLACE_RULE_REPLACE : PLAYER_PLAYER_MSG_REPLACE_RULE_ACCEPT);
  }
  else
  {
    PLAYER_ERROR1("encountered unknown message type %u", hdr->type);
    return(false);
  }
}

// Waits on the condition variable associated with this queue.
bool
MessageQueue::Wait(double TimeOut)
{
  bool result = true;
  MessageQueueElement* el;

  // don't wait if there's data on the queue
  this->Lock();
  // start at the head and traverse the queue until a filter-friendly
  // message is found
  for(el = this->head; el; el = el->next)
  {
    if(!this->filter_on || this->Filter(*el->msg))
      break;
  }
  this->Unlock();
  if(el)
    return result;

  // need to push this cleanup function, cause if a thread is cancelled while
  // in pthread_cond_wait(), it will immediately relock the mutex.  thus we
  // need to unlock ourselves before exiting.
  pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock,
                       (void*)&this->condMutex);
  pthread_mutex_lock(&this->condMutex);
  if (TimeOut > 0)
  {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    tp.tv_sec += static_cast<int> (floor(TimeOut));
    tp.tv_nsec += static_cast<int> ((TimeOut - floor(TimeOut))*1e9);
    int ret = pthread_cond_timedwait(&this->cond, &this->condMutex, &tp);
    // if we got an error or timed out
    if (ret != 0)
      result = false;
  }
  else
  {
    pthread_cond_wait(&this->cond,&this->condMutex);  
  }
  
  pthread_mutex_unlock(&this->condMutex);
  pthread_cleanup_pop(0);
  return result;
}

bool
MessageQueue::Filter(Message& msg)
{
  player_msghdr_t* hdr = msg.GetHeader();
  return(((this->filter_host < 0) ||
          ((unsigned int)this->filter_host == hdr->addr.host)) &&
         ((this->filter_robot < 0) ||
          ((unsigned int)this->filter_robot == hdr->addr.robot)) &&
         ((this->filter_interf < 0) ||
          ((unsigned int)this->filter_interf == hdr->addr.interf)) &&
         ((this->filter_index < 0) ||
          ((unsigned int)this->filter_index == hdr->addr.index)) &&
         (((this->filter_type < 0) &&
           ((hdr->type == PLAYER_MSGTYPE_RESP_ACK) ||
            (hdr->type == PLAYER_MSGTYPE_RESP_NACK))) ||
          ((unsigned int)this->filter_type == hdr->type)) &&
         ((this->filter_subtype < 0) ||
          ((unsigned int)this->filter_subtype == hdr->subtype)));
}

void
MessageQueue::SetFilter(int host, int robot, int interf,
                        int index, int type, int subtype)
{
  this->filter_host = host;
  this->filter_robot = robot;
  this->filter_interf = interf;
  this->filter_index = index;
  this->filter_type = type;
  this->filter_subtype = subtype;
  this->filter_on = true;
}

size_t
MessageQueue::GetLength(void)
{
  size_t len;
  this->Lock();
  len = this->Length;
  this->Unlock();
  return(len);
}

void
MessageQueue::ClearFilter(void)
{
  this->filter_on = false;
}


// Signal that new data is available (calls pthread_cond_broadcast()
// on this device's condition variable, which will release other
// devices that are waiting on this one).
void
MessageQueue::DataAvailable(void)
{
  pthread_mutex_lock(&this->condMutex);
  pthread_cond_broadcast(&this->cond);
  pthread_mutex_unlock(&this->condMutex);
}
    
/// @brief Set the data_requested flag
void 
MessageQueue::SetDataRequested(bool d, bool haveLock)
{ 
  if(!haveLock)
    this->Lock();
  this->data_requested = d; 
  this->data_delivered = false; 
  if(!haveLock)
    this->Unlock();
}

/// Put it at the front of the queue, without checking replacement rules
/// or size limits.
/// This method is used to insert responses to requests for data.
/// The caller may have already called Lock() on this queue
void
MessageQueue::PushFront(Message & msg, bool haveLock)
{
  if(!haveLock)
    this->Lock();
  MessageQueueElement* newelt = new MessageQueueElement();
  newelt->msg = new Message(msg);
  if(!this->tail)
  {
    this->head = this->tail = newelt;
    newelt->prev = newelt->next = NULL;
  }
  else
  {
    newelt->prev = NULL;
    newelt->next = this->head;
    this->head->prev = newelt;
    this->head = newelt;
  }
  this->Length++;
  if(!haveLock)
    this->Unlock();
}

/// Push a message at the back of the queue, without checking replacement 
/// rules or size limits.
/// This method is used internally to insert most messages.
/// The caller may have already called Lock() on this queue
void
MessageQueue::PushBack(Message & msg, bool haveLock)
{
  if(!haveLock)
    this->Lock();
  MessageQueueElement* newelt = new MessageQueueElement();
  newelt->msg = new Message(msg);
  if(!this->tail)
  {
    this->head = this->tail = newelt;
    newelt->prev = newelt->next = NULL;
  }
  else
  {
    this->tail->next = newelt;
    newelt->prev = this->tail;
    newelt->next = NULL;
    this->tail = newelt;
  }
  this->Length++;
  if(!haveLock)
    this->Unlock();
}

bool
MessageQueue::Push(Message & msg)
{
  player_msghdr_t* hdr;

  assert(*msg.RefCount);
  this->Lock();
  hdr = msg.GetHeader();
  // Should we try to replace an older message of the same signature?
  int replaceOp = this->CheckReplace(hdr);
  // if our queue is over size discard any data or command packets
  // if we discard requests or replies this will potentially lock up the client so we will let those through
  if (PLAYER_PLAYER_MSG_REPLACE_RULE_IGNORE == replaceOp)
  {
    // drop silently
    this->Unlock();
    return(true);
  }
  if (PLAYER_PLAYER_MSG_REPLACE_RULE_ACCEPT == replaceOp && (hdr->type == PLAYER_MSGTYPE_DATA ||
          hdr->type == PLAYER_MSGTYPE_CMD) && this->Length >= this->Maxlen)
  {
    // record the fact that we are dropping a message
    this->drop_count++;
    this->Unlock();
    return(true);
  }
  else if (replaceOp == PLAYER_PLAYER_MSG_REPLACE_RULE_REPLACE)
  {
    for(MessageQueueElement* el = this->tail;
        el != NULL;
        el = el->prev)
    {
      if(el->msg->Compare(msg))
      {
        this->Remove(el);
        delete el->msg;
        delete el;
        break;
      }
    }
  }

  this->PushBack(msg,true);

  // If it was a response, then mark it , to prompt
  // processing of the queue.
  if(!this->data_requested &&
     (hdr->type == PLAYER_MSGTYPE_RESP_ACK) ||
     (hdr->type == PLAYER_MSGTYPE_RESP_NACK))
    this->SetDataRequested(true,true);

  this->Unlock();
  if(!this->filter_on || this->Filter(msg))
    this->DataAvailable();
  return(true);
}

Message*
MessageQueue::Pop()
{
  MessageQueueElement* el;
  Lock();

  // Look for the last response in the queue, starting at the tail.
  // If any responses are pending, we always send all messages up to and
  // including the last response.
  MessageQueueElement* resp_el=NULL;
  if(!this->filter_on && !this->data_requested)
  {
    for(el = this->tail; el; el = el->prev)
    {
      if((el->msg->GetHeader()->type == PLAYER_MSGTYPE_RESP_NACK) ||
         (el->msg->GetHeader()->type == PLAYER_MSGTYPE_RESP_ACK))
      {
        resp_el = el;
        break;
      }
    }
  }

  // start at the head and traverse the queue until a filter-friendly
  // message is found
  for(el = this->head; el; el = el->next)
  {
    if(resp_el ||
       ((!this->filter_on || this->Filter(*el->msg)) && 
        (!this->pull || this->data_requested)))
    {
      if(el == resp_el)
        resp_el = NULL;
      if(this->data_requested &&
         (el->msg->GetHeader()->type == PLAYER_MSGTYPE_DATA))
        this->data_delivered = true;
      this->Remove(el);
      Unlock();
      Message* retmsg = el->msg;
      delete el;
      return(retmsg);
    }
  }

  // queue is empty.  if that data had been requested in pull mode, and
  // some has been delivered, then mark the end of this frame with a 
  // sync message
  if(this->pull && this->data_requested && this->data_delivered)
  {
    struct player_msghdr syncHeader;
    syncHeader.addr.host = 0;
    syncHeader.addr.robot = 0;
    syncHeader.addr.interf = PLAYER_PLAYER_CODE;
    syncHeader.addr.index = 0;
    syncHeader.type = PLAYER_MSGTYPE_SYNCH;
    // flag the synch with overflow subtype and count if an overflow occured
    Message* syncMessage = NULL;
    if (this->drop_count == 0)
    {
      syncHeader.subtype = PLAYER_PLAYER_SYNCH_OK;
      syncMessage = new Message(syncHeader, 0, 0);
    }
    else
    {
      syncHeader.subtype = PLAYER_PLAYER_SYNCH_OVERFLOW;
      syncMessage = new Message(syncHeader, &this->drop_count, true);
      this->drop_count = 0;
    }
    this->SetDataRequested(false,true);
    Unlock();
    return(syncMessage);
  }
  else
  {
    Unlock();
    return(NULL);
  }
}

void
MessageQueue::Remove(MessageQueueElement* el)
{
  if(el->prev)
    el->prev->next = el->next;
  else
    this->head = el->next;
  if(el->next)
    el->next->prev = el->prev;
  else
    this->tail = el->prev;
  this->Length--;
}

/// Create a null pointer
QueuePointer::QueuePointer()
{
  Lock = NULL;
  RefCount = NULL;
  Queue = NULL;
}

/// Create an empty message queue and an auto pointer to it.
QueuePointer::QueuePointer(bool _Replace, size_t _Maxlen)
{
  this->Lock = new pthread_mutex_t;
  assert(this->Lock);
  pthread_mutex_init(this->Lock,NULL);
  this->Queue = new MessageQueue(_Replace, _Maxlen);
  assert(this->Queue);

  this->RefCount = new unsigned int;
  assert(this->RefCount);
  *this->RefCount = 1;		
}

/// Destroy our reference to the message queue.
/// and our queue if there are no more references
QueuePointer::~QueuePointer()
{
	DecRef();
}

/// Create a new reference to a message queue
QueuePointer::QueuePointer(const QueuePointer & rhs)
{
  if (rhs.Queue == NULL)
  {
    Lock = NULL;
    RefCount = NULL;
    Queue = NULL;      	
  }
  else
  {
    assert(rhs.Lock);
    pthread_mutex_lock(rhs.Lock);

    assert(rhs.Queue);
    assert(rhs.RefCount);
    assert(*(rhs.RefCount));
    Lock = rhs.Lock;
    Queue = rhs.Queue;
    RefCount = rhs.RefCount;
    (*RefCount)++;
    pthread_mutex_unlock(Lock);	
  }
}
	
/// assign reference to our message queue
QueuePointer & QueuePointer::operator = (const QueuePointer & rhs)
{
  // first remove our current reference
  DecRef();
  
  if (rhs.Queue == NULL)
  	return *this;
  
  // then copy the rhs
  assert(rhs.Lock);
  pthread_mutex_lock(rhs.Lock);

  assert(rhs.Queue);
  assert(rhs.RefCount);
  assert(*(rhs.RefCount));
  Lock = rhs.Lock;
  Queue = rhs.Queue;
  RefCount = rhs.RefCount;
  (*RefCount)++;
  pthread_mutex_unlock(Lock);	
  return *this;
}

MessageQueue * QueuePointer::get() const 
{
	return Queue;
}


/// retrieve underlying object for use
MessageQueue * QueuePointer::operator -> ()
{
  assert(Queue);
  return Queue;		
}

/// retrieve underlying object for use
MessageQueue & QueuePointer::operator * ()
{
  assert(Queue);
  return *Queue;		
}

/// check if pointers are equal
bool QueuePointer::operator == (const QueuePointer & rhs)
{
  return rhs.Queue == Queue;	
}

/// check if pointers are equal
bool QueuePointer::operator == (void * pointer)
{
  return Queue == pointer;	
}

/// check if pointers are equal
bool QueuePointer::operator != (const QueuePointer & rhs)
{
  return rhs.Queue != Queue;	
}

/// check if pointers are equal
bool QueuePointer::operator != (void * pointer)
{
  return Queue != pointer;	
}

void QueuePointer::DecRef()
{
  if (Queue == NULL)
    return;
    
  pthread_mutex_lock(Lock);
  (*RefCount)--;
  assert((*RefCount) >= 0);
  if((*RefCount)==0)
  {
    delete Queue;
    Queue = NULL;
    delete RefCount;
    RefCount = NULL;
    pthread_mutex_unlock(Lock);
    pthread_mutex_destroy(Lock);
    delete Lock;
    Lock = NULL;
  }
  else
  {
    Queue = NULL;
    RefCount = NULL;
    pthread_mutex_unlock(Lock);		
    Lock = NULL;
  }
}
