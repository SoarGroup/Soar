/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004  Brian Gerkey gerkey@stanford.edu    
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
 * $Id: localbb.cpp 6371 2008-04-23 03:44:00Z thjc $
 *
 * 
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_LocalBB LocalBB
 * @brief Local memory implementation of a blackboard. The data entries are stored internally in a hash-map.
 * Internally information is stored in two hash-maps. One hash-map contains a map of labels to the entry data.
 * This stores the actual data. The second hash-map stores a map of device queues which are listening to an entry.
 * These are the devices that are sent events when an entry is updated.
 * CAVEATS:
 *  -There is no checking to see if a device is already subscribed to a key. If a device subscribes to a key twice, it will receive two updates.
 *  -All listening devices are sent updates when an entry is set, even if that device set the entry.

 @par Provides

 - @ref interface_blackboard

 @par Requires

 - None

 @par Configuration requests

 - None

 @par Configuration file options

 @par Example 

 @verbatim
 driver
 (
		name "localbb"
		provides [ "blackboard:0" ]
 )
 @endverbatim

 @author Ben Morelli

 */

/** @} */

#include <sys/types.h> // required by Darwin
#include <stdlib.h>
#include <libplayercore/playercore.h>
#include <libplayercore/error.h>
#include <libplayerc++/playererror.h>
#include <vector>
#include <map>
#include <strings.h>
#include <iostream>

/**@struct EntryData
 * @brief Custom blackboard-entry data representation used internally by the driver. */
typedef struct EntryData
{
	/** Constructor. Sets all members to 0 or NULL. */
  EntryData() { type = 0; subtype = 0; data_count = 0; data = NULL; timestamp_sec = 0; timestamp_usec = 0; }
  //~EntryData() { if (data != NULL) delete [] data; } Why doesn't it like this?

  /** Message type */
  uint16_t type;
  /** Message sub-type */
  uint16_t subtype;

  /** Data size */
  uint32_t data_count;
  /** Data */
  uint8_t* data;
  /** Time entry created. Seconds since epoch. */
  uint32_t timestamp_sec;
  /** Time entry created. Microseconds field. */
  uint32_t timestamp_usec;
} EntryData;

/**@struct BlackBoardEntry
 * @brief Custom blackboard entry representation used internally by the driver.*/
typedef struct BlackBoardEntry
{
	/** Constructor. Sets key and group to empty strings. Data should be automatically set to empty values. */
  BlackBoardEntry() { key = ""; group = ""; }

  /** Entry label */
  string key;
  /** Secondary identifier. */
  string group;
  /** Entry data */
  EntryData data;
} BlackBoardEntry;


// Function prototypes
/** @brief Convienience function to convert from the internal blackboard entry representation to the player format. The user must clean-up the player blackboard entry. Delete the key and data. */
player_blackboard_entry_t ToPlayerBlackBoardEntry(const BlackBoardEntry &entry);

/** @brief Convienience function to convert from the player blackboard entry to the internal representation. This entry will be cleaned-up automatically by the destructor.*/
BlackBoardEntry FromPlayerBlackBoardEntry(const player_blackboard_entry_t &entry);

player_blackboard_entry_t ToPlayerBlackBoardEntry(const BlackBoardEntry &entry)
{
  player_blackboard_entry_t result;
  memset(&result, 0, sizeof(player_blackboard_entry_t));
  
  result.type = entry.data.type;
  result.subtype = entry.data.subtype;

  result.key_count = strlen(entry.key.c_str()) + 1;
  result.key = new char[result.key_count]; //strdup(entry.key.c_str());
  memcpy(result.key, entry.key.c_str(), result.key_count);
  assert(result.key_count > 0);

  result.group_count = strlen(entry.group.c_str()) + 1;
  result.group = new char[result.group_count];
  memcpy(result.group, entry.group.c_str(), result.group_count);
  assert(result.group_count > 0);
  
  result.data_count = entry.data.data_count;
  result.data = new uint8_t[result.data_count];
  memcpy(result.data, entry.data.data, result.data_count);
  result.timestamp_sec = entry.data.timestamp_sec;
  result.timestamp_usec = entry.data.timestamp_usec;
  return result;
}
 
BlackBoardEntry FromPlayerBlackBoardEntry(const player_blackboard_entry_t &entry)
{
  BlackBoardEntry result;
  result.data.type = entry.type;
  result.data.subtype = entry.subtype;
  result.key = string(entry.key);
  result.group = string(entry.group);
  result.data.data_count = entry.data_count;
  result.data.data = new uint8_t[result.data.data_count];
  memcpy(result.data.data, entry.data, result.data.data_count);
  result.data.timestamp_sec = entry.timestamp_sec;
  result.data.timestamp_usec = entry.timestamp_usec;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/**@class LocalBB
 * @brief Local memory blackboard driver. Stores entries in a hash-map in local memory. */
class LocalBB : public Driver
{
	public:
		/**
		 * @brief Default constructor. 
		 * @param cf ConfigFile, the player configuration file information
		 * @param section The section of the configuration file
		 */
		LocalBB(ConfigFile* cf, int section);
		/** @brief Default destructor. */
		~LocalBB();
		/** @brief Driver initialisation. */
		int Setup();
		/** @brief Driver de-initialisation. */
		int Shutdown();

		// MessageHandler
		/** @brief Process incoming messages.
		 * Check the incoming message types. If the message type is PLAYER_BLACKBOARD_REQ_SUBSCRIBE_TO_KEY, PLAYER_BLACKBOARD_REQ_UNSUBSCRIBE_FROM_KEY or PLAYER_BLACKBOARD_REQ_SET_ENTRY, call
		 * the appropriate private method to process it. Otherwise it is an unknown message.
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @param data Message data.
		 * @return 0 for success, -1 on error.
		 */
		int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
	private:
		// Message subhandlers
		/** @brief Process a subscribe to key message.
		 * Adds the response queue to the list of devices listening to that key in the map.
		 * Retrieves the entry for the given key.
		 * Publishes the entry. 
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @param data Message data.
		 * @return 0 for success, -1 on error.
		 */
		int ProcessSubscribeKeyMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
		/** @brief Process an unsubscribe from key message.
		 * Removes the response queue from the list of devices listening to that key in the map.
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @param data Message data.
		 * @return 0 for success, -1 on error.
		 * */
		int ProcessUnsubscribeKeyMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
		/** @brief Process a set entry message.
		 * Set the entry message in the local hashmap.
		 * Send out update events to other listening devices.
		 * Send back an empty ack.
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @return 0 for success, -1 on error.
		 */
		int ProcessSetEntryMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
		/** @brief Process a subscribe to group message.
		 * Adds the response queue to the list of devices listenening to that group in the map.
		 * Retrieves the entries for that group.
		 * Publishes the entries.
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @param data Message data.
		 * @return 0 for success, -1 on error. 
		 */
		int ProcessSubscribeGroupMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
		/** @brief Process an unsubscribe from group message.
		 * Removes the response queue from the list of devices listening to that group in the map.
		 * @param resp_queue Player response queue.
		 * @param hdr Message header.
		 * @param data Message data.
		 * @return 0 for success, -1 on error.
		 * */
		int ProcessUnsubscribeGroupMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);

		// Blackboard handler functions
		/** @brief Add the key and queue combination to the listeners hash-map and return the entry for the key.
		 * @param key Entry key.
		 * @param group Second identifier.
		 * @param resp_queue Player response queue of the subscriber. 
		 */
		BlackBoardEntry SubscribeKey(const string &key, const string &group, const QueuePointer &resp_queue);
		/** @brief Remove the key and queue combination from the listeners hash-map.
		 * @param key Entry key.
		 * @param group Second identifier.
		 * @param qp Player response queue of the unsubscriber.
		 * @return Blackboard entry containing the value of the key and group. 
		 */
		void UnsubscribeKey(const string &key, const string &group, const QueuePointer &qp);
		/** @brief Add a group to the group listeners hash-map and return all entries of that group
		 * @param group Entry gruop
		 * @param qp resp_queue Player response queue of the subscriber
		 * @return Vector of blackboard entries of that group
		 */
		vector<BlackBoardEntry> SubscribeGroup(const string &group, const QueuePointer &qp);
		/**
		 * @brief Remove the group from the group listeners hash-map.
		 * @param group Entry group
		 * @param qp Player response queue of the unsubscriber
		 */
		void UnsubscribeGroup(const string &group, const QueuePointer &qp);
		
		/** @brief Set the entry in the entries hashmap. *
		* @param entry BlackBoardEntry that must be put in the hashmap.
		*/
		void SetEntry(const BlackBoardEntry &entry);
		
		// Helper functions
		/** @brief Check that the message has a valid size.
		 * @param hdr Header of the message to check. 
		 */
		bool CheckHeader(player_msghdr * hdr);

		// Internal blackboard data
		/** Map of labels to entry data.
		* map<group, map<key, entry> >
		*/
		map<string, map<string, BlackBoardEntry> > entries;
		/** Map of labels to listening queues.
		* map<group, map<key, vector<device queue> > >
		*/
		map<string, map<string, vector<QueuePointer> > > listeners;
		
		/** Map of groups to queues subscribed to groups.
		* map<group, vector<device queue> >
		*/
		map<string, vector<QueuePointer> > group_listeners;
};

////////////////////////////////////////////////////////////////////////////////
// Factory method.
Driver* LocalBB_Init(ConfigFile* cf, int section)
{
	return ((Driver*)(new LocalBB(cf, section)));
}

////////////////////////////////////////////////////////////////////////////////
// Driver registration function
void LocalBB_Register(DriverTable* table)
{
	table->AddDriver("localbb", LocalBB_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.
LocalBB::LocalBB(ConfigFile* cf, int section) :
	Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_BLACKBOARD_CODE)
{
	// No settings needed currently.
}

////////////////////////////////////////////////////////////////////////////////
// Destructor.
LocalBB::~LocalBB()
{
	// maps clean up themselves
}

////////////////////////////////////////////////////////////////////////////////
// Load resources.
int LocalBB::Setup()
{
	PLAYER_MSG0(2, "LocalBB ready");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Clean up resources.
int LocalBB::Shutdown()
{
	PLAYER_MSG0(2, "LocalBB shut down");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message.
int LocalBB::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	// Request for a subscription
	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLACKBOARD_REQ_SUBSCRIBE_TO_KEY, this->device_addr))
	{
		return ProcessSubscribeKeyMessage(resp_queue, hdr, data);
	}
	// Request for unsubscribe
	else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLACKBOARD_REQ_UNSUBSCRIBE_FROM_KEY, this->device_addr))
	{
		return ProcessUnsubscribeKeyMessage(resp_queue, hdr, data);
	}
	// Request to update an entry
	else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLACKBOARD_REQ_SET_ENTRY, this->device_addr))
	{
		return ProcessSetEntryMessage(resp_queue, hdr, data);
	}
	else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLACKBOARD_REQ_SUBSCRIBE_TO_GROUP, this->device_addr))
	{
		return ProcessSubscribeGroupMessage(resp_queue, hdr, data);
	}
	else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLACKBOARD_REQ_UNSUBSCRIBE_FROM_GROUP, this->device_addr))
	{
		return ProcessUnsubscribeGroupMessage(resp_queue, hdr, data);	
	}
	// Don't know how to handle this message
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Subscribe a device to a key.
int LocalBB::ProcessSubscribeKeyMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	if (!CheckHeader(hdr))
		return -1;

	// Add the device to the listeners map
	player_blackboard_entry_t *request = reinterpret_cast<player_blackboard_entry_t*>(data);
	BlackBoardEntry current_value = SubscribeKey(request->key, request->group, resp_queue);

	// Get the entry for the given key
	player_blackboard_entry_t response = ToPlayerBlackBoardEntry(current_value);
	size_t response_size = sizeof(player_blackboard_entry_t) + response.key_count + response.group_count + response.data_count;

	// Publish the blackboard entry
	this->Publish(
		this->device_addr,
		resp_queue,
		PLAYER_MSGTYPE_RESP_ACK,
		PLAYER_BLACKBOARD_REQ_SUBSCRIBE_TO_KEY,
		&response,
		response_size,
		NULL);

	if (response.key)
	{
		delete [] response.key;
	}
	if (response.group)
	{
		delete [] response.group;
	}
	if (response.data)
	{
		delete [] response.data;
	}	

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Unsubscribe a device from a key.
int LocalBB::ProcessUnsubscribeKeyMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	if (!CheckHeader(hdr))
		return -1;

	// Remove the device from the listeners map
	player_blackboard_entry_t *request = reinterpret_cast<player_blackboard_entry_t*>(data);
	UnsubscribeKey(request->key, request->group, resp_queue);

	// Send back an empty ack
	this->Publish(
		this->device_addr,
		resp_queue,
		PLAYER_MSGTYPE_RESP_ACK,
		PLAYER_BLACKBOARD_REQ_UNSUBSCRIBE_FROM_KEY,
		NULL,
		0,
		NULL);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Subscribe a device to a group. Send out data messages for the current group entries.
int LocalBB::ProcessSubscribeGroupMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	if (!CheckHeader(hdr))
			return -1;

	// Add the device to the listeners map
	player_blackboard_entry_t *request = reinterpret_cast<player_blackboard_entry_t*>(data);
	vector<BlackBoardEntry> current_values = SubscribeGroup(request->group, resp_queue);

	// Get the entries for the given key and send the data updates
	for (vector<BlackBoardEntry>::iterator itr=current_values.begin(); itr != current_values.end(); itr++)
	{
		BlackBoardEntry current_value = *itr;
		player_blackboard_entry_t response = ToPlayerBlackBoardEntry(current_value);
		size_t response_size = sizeof(player_blackboard_entry_t) + response.key_count + response.group_count + response.data_count;

		// Publish the blackboard entries
		this->Publish(this->device_addr,
			resp_queue,
			PLAYER_MSGTYPE_DATA,
			PLAYER_BLACKBOARD_DATA_UPDATE,
			&response,
			response_size,
			NULL);

		if (response.key)
		{
			delete [] response.key;
		}
		if (response.group)
		{
			delete [] response.group;
		}
		if (response.data)
		{
			delete [] response.data;
		}	
	}
	
	// Then send an empty ack
	this->Publish(
		this->device_addr,
		resp_queue,
		PLAYER_MSGTYPE_RESP_ACK,
		PLAYER_BLACKBOARD_REQ_SUBSCRIBE_TO_GROUP,
		NULL,
		0,
		NULL);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Unsubscribe a device from a group.
int LocalBB::ProcessUnsubscribeGroupMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	if (!CheckHeader(hdr))
		return -1;

	// Remove the device from the group listeners map
	player_blackboard_entry_t *request = reinterpret_cast<player_blackboard_entry_t*>(data);
	UnsubscribeGroup(request->group, resp_queue);

	// Send back an empty ack
	this->Publish(
		this->device_addr,
		resp_queue,
		PLAYER_MSGTYPE_RESP_ACK,
		PLAYER_BLACKBOARD_REQ_UNSUBSCRIBE_FROM_KEY,
		NULL,
		0,
		NULL);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set an entry and send out update events to all listeners.
int LocalBB::ProcessSetEntryMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
	if (!CheckHeader(hdr))
		return -1;

	player_blackboard_entry_t *request = reinterpret_cast<player_blackboard_entry_t*>(data);
	BlackBoardEntry entry = FromPlayerBlackBoardEntry(*request);

	SetEntry(entry);
	
	// Send out update events to other listening devices for key group combinations
	vector<QueuePointer> &devices = listeners[entry.group][entry.key];

	for (vector<QueuePointer>::iterator itr=devices.begin(); itr != devices.end(); itr++)
	{
		QueuePointer device_queue = (*itr);
		this->Publish(this->device_addr,
			device_queue,
			PLAYER_MSGTYPE_DATA,
			PLAYER_BLACKBOARD_DATA_UPDATE,
			data,
			hdr->size,
			NULL);
	}
	
	// Send out update events to just groups
	vector<QueuePointer> &devices_groups = group_listeners[entry.group];
	for (vector<QueuePointer>::iterator itr=devices_groups.begin(); itr != devices_groups.end(); itr++)
	{
		QueuePointer device_queue = (*itr);
		this->Publish(this->device_addr,
			device_queue,
			PLAYER_MSGTYPE_DATA,
			PLAYER_BLACKBOARD_DATA_UPDATE,
			data,
			hdr->size,
			NULL);
	}
	
	// Send back an empty ack
	this->Publish(this->device_addr,
		resp_queue,
		PLAYER_MSGTYPE_RESP_ACK,
		PLAYER_BLACKBOARD_REQ_SET_ENTRY,
		NULL,
		0,
		NULL);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Add a device to the listener list for a key. Return the current value of the entry.
BlackBoardEntry LocalBB::SubscribeKey(const string &key, const string &group, const QueuePointer &resp_queue)
{
	listeners[group][key].push_back(resp_queue);
	BlackBoardEntry entry = entries[group][key];
	if (entry.key == "")
	{
		entry.key = key;
	}
	entry.group = group;
	return entry;
}

////////////////////////////////////////////////////////////////////////////////
// Remove a device from the listener list for a key.
void LocalBB::UnsubscribeKey(const string &key, const string &group, const QueuePointer &qp)
{
	vector<QueuePointer> &devices = listeners[group][key];

	for (vector<QueuePointer>::iterator itr = devices.begin(); itr != devices.end(); itr++)
	{
		if ((*itr) == qp)
		{
			devices.erase(itr);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Add a device to the group listener map. Return vector of entries for that group.
vector<BlackBoardEntry> LocalBB::SubscribeGroup(const string &group, const QueuePointer &qp)
{
	group_listeners[group].push_back(qp);
	vector<BlackBoardEntry> group_entries;
	
	// Add all entries for a group to the group_entries vector
	//map<group, map<key, entry> >
	map<string, BlackBoardEntry> entry_map = entries[group];
	for (map<string, BlackBoardEntry>::iterator itr = entry_map.begin(); itr != entry_map.end(); itr++)
	{
		group_entries.push_back((*itr).second);
	}
	return group_entries;
}

////////////////////////////////////////////////////////////////////////////////
// Remove a device from the group listener map
void LocalBB::UnsubscribeGroup(const string &group, const QueuePointer &qp)
{
	vector<QueuePointer> &devices = group_listeners[group];
	
	for (vector<QueuePointer>::iterator itr = devices.begin(); itr != devices.end(); itr++)
	{
		if ((*itr) == qp)
		{
			devices.erase(itr);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Set entry value in the entries map.
void LocalBB::SetEntry(const BlackBoardEntry &entry)
{
	entries[entry.group][entry.key] = entry;
}

////////////////////////////////////////////////////////////////////////////////
// Check that we have some data in the message.
bool LocalBB::CheckHeader(player_msghdr * hdr)
{
	if (hdr->size <= 0)
	{
		PLAYER_ERROR2("request is wrong length (%d <= %d); ignoring", hdr->size, 0);
		return false;
	}
	return true;
}
