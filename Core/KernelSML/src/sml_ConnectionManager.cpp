#include "portability.h"

/////////////////////////////////////////////////////////////////
// ConnectionManager class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Used by a server (the kernel in our case usually) to manager
// a listener socket and then keep a list of connections that
// come in over that listener.
//
/////////////////////////////////////////////////////////////////

#include "sml_ConnectionManager.h"

#include "sml_Utils.h"
#include "sml_ListenerThread.h"
#include "sml_ReceiverThread.h"
#include "sml_KernelSML.h"

#include <time.h>   // To get clock

using namespace sml ;
using namespace sock ;

static void WakeConnectionManagerCallback(void* pWakeData)
{
    if (pWakeData)
    {
        static_cast<ConnectionManager*>(pWakeData)->SignalIncomingData() ;
    }
}

ConnectionManager::ConnectionManager(int port, KernelSML* pKernel)
{
    m_WakeupSocket = NO_CONNECTION ;
    m_WakeupPending.store(false) ;
    memset(&m_WakeupAddr, 0, sizeof(m_WakeupAddr)) ;
    InitWakeupSocket() ;
    
    // Start the thread that wraps the listener socket running.
    // (Unless passed port 0 -- which suppresses this)
    m_ListenerThread = NULL ;
    if (port != 0)
    {
        m_ListenerThread = new ListenerThread(this, port, pKernel) ;
        m_ListenerThread->Start() ;
    }
    
    // Start the thread that checks for incoming messages on the remote connections
    // (once the listener has been used to create them).
    m_ReceiverThread = new ReceiverThread(this) ;
    m_ReceiverThread->Start() ;
    
    m_bTraceCommunications = false ;
}
ConnectionManager::~ConnectionManager()
{
    // If we still have an active listener, shut down the connections.
    // This isn't recommended.  It seems if we do this when the
    // main process is executing, other threads may already have been forced to exit
    // so this shutdown won't be as clean.  Nothing should crash, but we could cause
    // timeouts etc.
    if (m_ListenerThread)
    {
        Shutdown() ;
    }
    CloseWakeupSocket() ;
}

void ConnectionManager::InitWakeupSocket()
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
    if (s == INVALID_SOCKET)
    {
        return ;
    }
    sockaddr_in addr ;
    memset(&addr, 0, sizeof(addr)) ;
    addr.sin_family = AF_INET ;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;
    addr.sin_port = 0 ;
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        NET_CLOSESOCKET(s) ;
        return ;
    }
    socklen_t len = sizeof(m_WakeupAddr) ;
    if (getsockname(s, reinterpret_cast<sockaddr*>(&m_WakeupAddr), &len) != 0)
    {
        NET_CLOSESOCKET(s) ;
        return ;
    }
    // Work around Windows firewall/stack quirk where getsockname can return 0.0.0.0
    if (m_WakeupAddr.sin_addr.s_addr == htonl(INADDR_ANY))
    {
        m_WakeupAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;
    }
#ifdef _WIN32
    unsigned long nonBlocking = 1 ;
    ioctlsocket(s, FIONBIO, &nonBlocking) ;
#else
    int flags = fcntl(s, F_GETFL, 0) ;
    if (flags >= 0)
    {
        fcntl(s, F_SETFL, flags | O_NONBLOCK) ;
    }
#endif
    m_WakeupSocket = s ;
}

void ConnectionManager::CloseWakeupSocket()
{
    if (m_WakeupSocket != NO_CONNECTION)
    {
        NET_CLOSESOCKET(m_WakeupSocket) ;
        m_WakeupSocket = NO_CONNECTION ;
    }
}

void ConnectionManager::DrainWakeupSocket()
{
    if (m_WakeupSocket == NO_CONNECTION)
    {
        return ;
    }
    char buf[64] ;
    // Clear m_WakeupPending BEFORE draining non-blocking socket so any concurrent
    // SignalIncomingData() during/after drain is guaranteed not to be lost.
    m_WakeupPending.store(false) ;
    while (recvfrom(m_WakeupSocket, buf, sizeof(buf), 0, NULL, NULL) > 0)
    {
    }
}

void ConnectionManager::SignalIncomingData()
{
    if (m_WakeupSocket == NO_CONNECTION)
    {
        return ;
    }
    if (!m_WakeupPending.exchange(true))
    {
        char byte = 1 ;
        sendto(m_WakeupSocket, &byte, 1, 0, reinterpret_cast<sockaddr*>(&m_WakeupAddr), sizeof(m_WakeupAddr)) ;
    }
}

// Cause the receiver thread to quit.
void ConnectionManager::StopReceiverThread()
{
    // Stop the receiver thread (and wait until is has stopped)
    if (m_ReceiverThread)
    {
        m_ReceiverThread->Stop(false) ;
        SignalIncomingData() ;
        m_ReceiverThread->Stop(true) ;
    }
}

/*************************************************************
* @brief Turning this on means we'll start dumping output about messages
*        being sent and received.  Currently this only applies to remote connections.
*************************************************************/
void ConnectionManager::SetTraceCommunications(bool state)
{
    m_bTraceCommunications = state ;
    
    // Serialize thread access to the connections list
    soar_thread::Lock lock(&m_ConnectionsMutex) ;
    
    // Set the trace state on any existing connections
    // (this list will usually be empty when we set this variable, but this is for completeness).
    for (ConnectionsIter iter = m_Connections.begin() ; iter != m_Connections.end() ; iter++)
    {
        Connection* pConnection = *iter ;
        pConnection->SetTraceCommunications(state) ;
    }
}

// Close all connections
void ConnectionManager::Shutdown()
{
//  sml::PrintDebug("Shutting down connection manager") ;

    // Stop the thread (and wait until it has stopped)
    if (m_ListenerThread)
    {
        m_ListenerThread->Stop(true) ;
        
        // Remove the listener
        delete m_ListenerThread ;
        m_ListenerThread = NULL ;
    }
    
//  sml::PrintDebug("Listener stopped") ;

    // Stop the receiver thread (and wait until is has stopped)
    if (m_ReceiverThread)
    {
        m_ReceiverThread->Stop(false) ;
        SignalIncomingData() ;
        m_ReceiverThread->Stop(true) ;
        
        //  sml::PrintDebug("Receiver stopped") ;
        
        // Remove the thread
        delete m_ReceiverThread ;
        m_ReceiverThread = NULL ;
    }
    
    // Serialize thread access to the connections list
    soar_thread::Lock lock(&m_ConnectionsMutex) ;
    
    for (ConnectionsIter iter = m_Connections.begin() ; iter != m_Connections.end() ; iter++)
    {
        Connection* pConnection = *iter ;
        pConnection->CloseConnection() ;
        
        // Remove any events that this connection is listening to
        KernelSML* pKernelSML = static_cast<KernelSML*>(pConnection->GetUserData());
        pKernelSML->RemoveAllListeners(pConnection) ;
        
        // Not clear that we can just delete connections as we might be inside a connection callback
        // when the shutdown comes.  So for safety, move connections to a closed list instead of deleting.
        // For now this will leak the connection object.  Later if we are confident we can delete this list.
        //delete pConnection ;
        m_ClosedConnections.push_back(pConnection) ;
    }
    
    m_Connections.clear() ;
    
    // Now clean up all closed connections.
    for (ConnectionsIter iter = m_ClosedConnections.begin() ; iter != m_ClosedConnections.end() ; iter++)
    {
        Connection* pConnection = *iter ;
        delete pConnection ;
    }
    m_ClosedConnections.clear() ;
    
//  sml::PrintDebug("Completed shutdown") ;
}

void ConnectionManager::AddConnection(Connection* pConnection)
{
    // Serialize thread access to the connections list
    soar_thread::Lock lock(&m_ConnectionsMutex) ;
    
    // Assign a unique ID for this connection
    std::ostringstream buffer;
    buffer << "id_0x" << pConnection;
    std::string bufferStdString = buffer.str();
    const char* bufferCString = bufferStdString.c_str();
    pConnection->SetID(bufferCString) ;
    pConnection->SetName("unknown") ;
    pConnection->SetStatus(sml_Names::kStatusCreated) ;
    pConnection->SetWakeCallback(&WakeConnectionManagerCallback, this) ;
    
    m_Connections.push_back(pConnection) ;
    SignalIncomingData() ;
}

Connection* ConnectionManager::GetConnectionByIndex(int i)
{
    // Just to be consistent with allowing out of range values to be passed in here.
    if (i < 0)
    {
        return NULL ;
    }
    
    // Serialize thread access to the connections list
    // which is shared with the listener socket/thread
    soar_thread::Lock lock(&m_ConnectionsMutex) ;
    
    // Get the n'th connection
    ConnectionsIter iter;
    for (iter = m_Connections.begin() ; iter != m_Connections.end() && i > 0 ; iter++)
    {
        i-- ;
    }
    
    if (iter == m_Connections.end())
    {
        return NULL ;
    }
    
    return *iter ;
}

void ConnectionManager::RemoveConnection(Connection* pConnection)
{
    // Serialize thread access to the connections list
    // which is shared with the listener socket/thread
    soar_thread::Lock lock(&m_ConnectionsMutex) ;
    
    // Remove the connection from our list
    m_Connections.remove(pConnection) ;
}

void ConnectionManager::SetAgentStatus(char const* pStatus)
{
    int index = 0 ;
    Connection* pConnection = NULL ;
    
    while ((pConnection = GetConnectionByIndex(index)) != NULL)
    {
        pConnection->SetAgentStatus(pStatus) ;
        index++ ;
    }
}

// Go through all connections and read any incoming commands from the sockets.
// The messages are sent to the callback registered with the connection when it was created (ReceivedCall currently).
// Those calls could take a long time to execute (e.g. a call to Run Soar).
// Returns true if we received at least one message.
bool ConnectionManager::ReceiveAllMessages()
{
    int index = 0 ;
    bool receivedOneMessage = false ;
    
    // We need to search this list of connections and call each in turn.
    // But we also want to allow the listener thread to add new connections while we're doing this.
    // (E.g. we might want to attach a debugger to a process that's already executing a "run" command inside "receiveMessages").
    // So we use this slightly cumbersome approach of looking up the connection based on an integer index.
    // The lookup is thread safe and if the connection list changes between lookups that's fine as this function
    // will return NULL once we go out of bounds.  (You could argue that we might miss calling a connection on a particular pass using this method
    // but that should be ok).
    Connection* pConnection = GetConnectionByIndex(index++) ;
    
    while (pConnection)
    {
        // Check to see if this connection has already been closed
        // (which includes if the other side has dropped its half of the socket)
        if (!pConnection->IsClosed())
        {
            receivedOneMessage = pConnection->ReceiveMessages(true) || receivedOneMessage ;
        }
        else
        {
            // If the connection has closed, delete it from the list of active connections
            RemoveConnection(pConnection) ;
            
            // Remove any events that this connection is listening to
            KernelSML* pKernelSML = static_cast<KernelSML*>(pConnection->GetUserData());
            pKernelSML->RemoveAllListeners(pConnection) ;
            
            // Not clear that we can just delete connections as we might be inside a connection callback
            // when the shutdown comes.  So for safety, move connections to a closed list instead of deleting.
            // For now this will leak the connection object.  Later if we are confident we can delete this list.
            //delete pConnection ;
            m_ClosedConnections.push_back(pConnection) ;
            
            // Since we just removed this connection we should look up the same index value again
            // so we don't skip any.  This isn't really that important.
            index-- ;
        }
        
        // Get the next connection (will return NULL once we reach the end of the list)
        pConnection = GetConnectionByIndex(index++) ;
    }
    
    // So far we don't have some sort of shutdown message the remote connections
    // can send, but if we decide to implement it this allows us to return it.
    return receivedOneMessage ;
}

bool ConnectionManager::WaitForIncomingData(int secondsWait, int millisecondsWait)
{
    assert(millisecondsWait < 1000 && "specified milliseconds must be less than 1000") ;
    
    fd_set readSet ;
    FD_ZERO(&readSet) ;
    SOCKET maxSock = NO_CONNECTION ;
    int socketCount = 0 ;
    
    if (m_WakeupSocket != NO_CONNECTION)
    {
#ifdef _MSC_VER
#pragma warning(push, 3)
#endif
        FD_SET(m_WakeupSocket, &readSet) ;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        maxSock = m_WakeupSocket ;
        socketCount++ ;
    }
    
    {
        soar_thread::Lock lock(&m_ConnectionsMutex) ;
        for (ConnectionsIter iter = m_Connections.begin() ; iter != m_Connections.end() ; ++iter)
        {
            Connection* pConnection = *iter ;
            if (!pConnection || pConnection->IsClosed())
            {
                continue ;
            }
            if (pConnection->HasPendingIncomingMessages())
            {
                return true ;
            }
            SOCKET hSock = pConnection->GetSocketHandle() ;
            if (hSock != NO_CONNECTION)
            {
#ifndef _WIN32
                if (hSock >= FD_SETSIZE)
                {
                    continue ;
                }
#endif
#ifdef _MSC_VER
#pragma warning(push, 3)
#endif
                FD_SET(hSock, &readSet) ;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
                if (maxSock == NO_CONNECTION || hSock > maxSock)
                {
                    maxSock = hSock ;
                }
                socketCount++ ;
            }
        }
    }
    
    if (socketCount == 0)
    {
        sml::Sleep(secondsWait, millisecondsWait) ;
        return false ;
    }
    
    TIMEVAL waitTime ;
    waitTime.tv_sec = secondsWait ;
    waitTime.tv_usec = millisecondsWait * 1000 ;
    
    int res = select(static_cast<int>(maxSock) + 1, &readSet, NULL, NULL, &waitTime) ;
    if (res > 0)
    {
        if (m_WakeupSocket != NO_CONNECTION && FD_ISSET(m_WakeupSocket, &readSet))
        {
            DrainWakeupSocket() ;
        }
        return true ;
    }
    return false ;
}

int ConnectionManager::GetListenerPort()
{
    if (m_ListenerThread == 0)
    {
        return 0;
    }
    return m_ListenerThread->GetPort();
}
