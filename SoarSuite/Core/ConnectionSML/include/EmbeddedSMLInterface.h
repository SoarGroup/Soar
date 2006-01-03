/////////////////////////////////////////////////////////////////
// EmbeddedSMLInterface file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This file provides a C level interface into that a module (DLL) must support if it can
// be loaded as an "embedded connection".  So the KernelSML and ClientSML modules will
// both support this.
//
// E.g. The KernelSML module receives commands in SML (a dialect of XML), sends them to the Soar kernel
// and then returns the results in SML.
//
// The SML can be passed directly as an object into this library (if the client and kernel happen
// to be in the same process) or the SML can be sent as a regular XML stream.
//
/////////////////////////////////////////////////////////////////

#ifndef EMBEDDED_SML_INTERFACE_H
#define EMBEDDED_SML_INTERFACE_H

#include "sml_Handles.h"
#include "ElementXMLHandle.h"

// For test
//#define WIN_STATIC_LINK

#ifdef _WIN32
#ifdef _USRDLL
#define EXPORT __declspec(dllexport)
#else
#ifndef WIN_STATIC_LINK
#define EXPORT __declspec(dllimport)
#else
#define EXPORT
#endif	// STATIC
#endif	// DLL
#else
#define EXPORT
#endif	// WIN32

#ifdef __cplusplus
extern "C" {
#endif

// Define the ProcessMessageFunction to take a connection, an incoming message and some action
// to take on it and return the response message.
typedef ElementXML_Handle (*ProcessMessageFunction)(Connection_Receiver_Handle, ElementXML_Handle, int) ;

// Define the CreateEmbeddedConnectionFunction to take a sender's connection and the sender's message handling function
// and return a new connection handle.
typedef Connection_Receiver_Handle (*CreateEmbeddedConnectionFunction)(Connection_Sender_Handle, ProcessMessageFunction, int, int) ;


////////////////////////////////////////////////////////////////
//
// Embedded SML Interface
//
// A library must implement this interface if we wish to create
// an embedded SML connection to it.
// 
// The interface is described in terms of a sender and a receiver.
// The module implementing this interface is the receiver.
// The module calling to this interface is the sender.
// 
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief	The sender will call once to this interface to
*			create the initial connection.
*
* This module creates its own Connection object and passes it back.
* That object will be passed back to us during future calls.
*
* This module also receives a pointer to the sender's Connection object and
* a pointer to the sender's "ProcessMessage" function.
*
* When the sender wishes to send a message to the receiver (i.e this module)
* it will use the Connection object just created here and the ProcessMessage function
* defined just below.
*
* When the receiver (i.e. this module) wishes to send a message to the sender
* it will use the Connection object passed in, together with the ProcessMessage function
* passed in.
*
* Everything about the connection is symmetric, so it's important to keep clear
* which connection belongs to which side of the communication.  The important thing
* to remember is that is module X creates Connection Y then only module X will
* ever call functions on Connection Y...even if the Connection pointer is passed around
* in the meantime.
*	
* @param hSenderConnection		The sender's connection object (this is opaque to us--i.e. we can't call methods on it, just pass it back later).
* @param pSenderProcessMessage	The sender's ProcessMessage function (which this module will call when we're sending messages out).
* @param connectionType			What form of embedded connection to create (valid values are defined below)
* @param portToListenOn			What port to listen for incoming remote connections on
*
* @returns Returns a handle to a new connection object.  This module will receive this back in calls to our ProcessMessage function.
*************************************************************/

#define SML_SYNCH_CONNECTION	1	// Incoming messages are executed immediately on the caller's thread
#define SML_ASYNCH_CONNECTION	2	// Incoming messages are queued and executed later on the receiver's thread

EXPORT Connection_Receiver_Handle sml_CreateEmbeddedConnection(Connection_Sender_Handle hSenderConnection, ProcessMessageFunction pSenderProcessMessage, int connectionType, int portToListenOn) ;

/*************************************************************
* @brief	This function is called by the sender in order to
*			pass a message to us (the receiver).
*
* The message is almost always just a normal SML message passed in hIncomingMsg.
* But it can also be a request to shut down the connection or do other meta operations.
* This is determined by the "action" parameter.
*
* The use of this single method to support meta operations (like closing down the connection)
* may look like a kludge, but it is intended to maintain maximum flexibility.  A new capability
* can be added by defining a new action value without breaking existing code (because the
* function interface will not be required to change).
*
* @param hReceiverConnection	Our module's connection object created in CreateEmbeddedConnection.  We will access methods from this object.
* @param hIncomingMsg			The SML message to process
* @param action					The way to process the message (valid values are defined below).  Invalid values should be ignored (just return NULL).
*
* @returns A new SML message object which is the response to the incoming message.  Can be NULL for some types of incoming message.
*************************************************************/

// The only actions we support so far
#define SML_MESSAGE_ACTION_SYNCH	1		// Respond to the message immediately (on the caller's thread)
#define SML_MESSAGE_ACTION_CLOSE	2		// Close down the connection
#define SML_MESSAGE_ACTION_ASYNCH	3		// Messages are executed on the receiver's thread, not on the senders, so there is no immediate response.
#define SML_MESSAGE_ACTION_TRACE_ON 4		// Turn on full tracing of messages (making this a special message means we can do this as a runtime choice)
#define SML_MESSAGE_ACTION_TRACE_OFF 5		// Turn off full tracing of messages (making this a special message means we can do this as a runtime choice)

EXPORT ElementXML_Handle sml_ProcessMessage(Connection_Receiver_Handle hReceiverConnection, ElementXML_Handle hIncomingMsg, int action) ;

#ifdef __cplusplus
} // extern C
#endif

#endif	// KERNEL_SML_INTERFACE_H
