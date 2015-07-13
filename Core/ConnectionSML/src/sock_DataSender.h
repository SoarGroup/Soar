/////////////////////////////////////////////////////////////////
// DataSender class
//
// Author: Bob Marinier
// Date  : 5/2007
//
// Represents an abstract data sender.  It is based on the original Socket class,
// but allows us to add other kinds of connections (e.g. named pipes, RPC, etc).
//
// Instances of this class are not created directly.
//
// A server creates a listener data sender (a derived class) (e.g. a socket)
// which is used to listen for incoming connections.
//
// A client then connects to that listener data sender through the "client data sender" class.
//
// The client continues to use the client data sender object it created.
// The server is passed a new data sender when it checks for incoming connections
// on the listener data sender.
//
/////////////////////////////////////////////////////////////////

#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include <string>
#include "thread_Lock.h"

namespace sock
{

    class DataSender
    {
    
        protected:
        
            // Controls whether we dump out the messages we're sending and receiving.
            bool m_bTraceCommunications ;
            
            // The name of this datasender
            std::string name;
            
            // These objects are created through the ListenerDataSender or ClientDataSender classes.
        protected:
            DataSender()
            {
                name = "NONAME";
            };
            
        public:
            // Destructor closes the socket
            virtual     ~DataSender() {};
            
            // Note: When we try to read/write to the data sender next we may
            // find this is no longer true--it's just the last state we know about.
            virtual bool        IsAlive() = 0;
            
            // Check if data is waiting to be read
            // Returns true if data sender is closed--but then receiveMsg will know it's closed.
            // The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
            virtual bool        IsReadDataAvailable(int secondsWait = 0, int millisecondsWait = 0) = 0;
            
            // Close down our side of the data sender, locks and calls CloseInternal
            void        Close();
            
            // Get the name of this datasender
            virtual std::string GetName()
            {
                return name;
            }
            
        public:
            // Print out debug information about the messages we are sending and receiving.
            // NOTE: We still print out information about start up/shut down, errors etc. without this flag being true.
            void        SetTraceCommunications(bool state)
            {
                m_bTraceCommunications = state ;
            }
            
            // Send a string of characters.  Outgoing format will be "<4-byte length>"+string data
            bool        SendString(char const* pString) ;
            
            // Receive a string of characters.  Incoming format on socket should be "<4-byte length>"+string data
            bool        ReceiveString(std::string* pString) ;
            
        protected:
            // Lower level buffer send and receive calls.
            virtual bool        SendBuffer(char const* pSendBuffer, uint32_t bufferSize) = 0 ;
            virtual bool        ReceiveBuffer(char* pRecvBuffer, uint32_t bufferSize) = 0 ;
            
            // Specific kinds of locks have different ways of shutting themselves down, they must do it here
            virtual void CloseInternal() = 0;
            
        private:
            // Locks calls to CloseInternal
            soar_thread::Mutex m_CloseMutex;
    };
    
} // Namespace

#endif // DATA_SENDER_H

