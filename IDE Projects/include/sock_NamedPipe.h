/////////////////////////////////////////////////////////////////
// NamedPipe class
//
// Author: Bob Marinier
// Date  : 5/2007
//
// Based on Socket class.
//
// Represents a named pipe.
//
// Instances of this class are not created directly.
//
// A server creates a listener named pipe (a derived class)
// which is used to listen for incoming connections on a particular pipe.
//
// A client then connects to that listener pipe (it needs to know the name)
// through the "client named pipe" class.
//
// The client continues to use the client named pipe object it created.
// The server is passed a new named pipe when it checks for incoming connections
// on the listener pipe.
//
/////////////////////////////////////////////////////////////////

#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#ifdef ENABLE_NAMED_PIPES
#include <string>

#include "sock_DataSender.h"

namespace sock
{

// Define PIPE_NON_BLOCKING to make all pipes non-blocking pipes.
//#define PIPE_NON_BLOCKING

#ifdef PIPE_NON_BLOCKING
#define PIPE_TYPE PIPE_NOWAIT
#else
#define PIPE_TYPE PIPE_WAIT
#endif


    class ListenerNamedPipe ;
    class ClientNamedPipe ;
    
    class NamedPipe : public DataSender
    {
            // Allow these classes access to our constructor
            friend class ListenerNamedPipe ;
            friend class ClientNamedPipe ;
            
        protected:
            HANDLE  m_hPipe ;
            
            // Controls whether we dump out the messages we're sending and receiving.
            bool m_bTraceCommunications ;
            
            // These objects are created through the ListenerSocket or ClientSocket classes.
        protected:
            NamedPipe();
            NamedPipe(HANDLE hPipe) ;
            
        public:
            // Destructor closes the pipe
            virtual     ~NamedPipe();
            
            // Note: When we try to read/write to the pipe next we may
            // find this is no longer true--it's just the last state we know about.
            bool        IsAlive()
            {
                return m_hPipe != INVALID_HANDLE_VALUE ;
            }
            
            // Returns handle for pipe
            HANDLE      GetPipeHandle()
            {
                return m_hPipe ;
            }
            
            // Check if data is waiting to be read
            // Returns true if pipe is closed--but then receiveMsg will know it's closed.
            // The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
            bool        IsReadDataAvailable(int secondsWait = 0, int millisecondsWait = 0) ;
            
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
            virtual bool SendBuffer(char const* pSendBuffer, uint32_t bufferSize) ;
            virtual bool ReceiveBuffer(char* pRecvBuffer, uint32_t bufferSize) ;
            
            // Close down our side of the pipe
            virtual void        CloseInternal() ;
    };
    
} // Namespace

#endif // ENABLE_NAMED_PIPES
#endif // NAMED_PIPE_H
