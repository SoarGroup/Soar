// Connections.cpp: implementation of the CTConnections class.
//
//////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "Connections.h"
#include "Socket.h"
#include "OSspecific.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTConnections::CTConnections()
{

}

CTConnections::~CTConnections()
{
	DeleteAllSockets() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::GetSocketByIndex
// 
// Return type    : CTSocket* 	
// Argument       : long index	
// 
// Description	  : Looks up a socket based on its
//					position in the list.
//
/////////////////////////////////////////////////////////////////////
CTSocket* CTConnections::GetSocketByIndex(long index)
{
	if (index < 0 || index >= (long)m_SocketList.size())
		return NULL ;

	SocketIter i = m_SocketList.begin() ;

	// Find the item in the list
	while (index != 0)
	{
		i++ ;
		index-- ;
	}

	// Shouldn't be possible to overshoot due to range check at start
	assert(i != m_SocketList.end()) ;

	// Retrieve the socket
	CTSocket* pSocket = *i ;

	return pSocket ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::GetSocketByName
// 
// Return type    : CTSocket* 	
// Argument       : char const* pName	
// 
// Description	  : Looks up a socket by matching it's name.
//					The match ignores case.
//
/////////////////////////////////////////////////////////////////////
CTSocket* CTConnections::GetSocketByName(char const* pName)
{
	// Find the socket with a matching name.
	// Ignores case.
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pSocket = *i ;
		
		if (STRICMP(pSocket->GetName(), pName) == 0)
			return pSocket ;
	}

	return NULL ;
}

long CTConnections::GetIndexByName(char const* pName)
{
	// Find the socket with a matching name.
	// Ignores case.
	long pos = -1 ;
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pSocket = *i ;
		pos++ ;
		
		if (STRICMP(pSocket->GetName(), pName) == 0)
			return pos ;
	}

	return -1 ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::GetSocketByPort
// 
// Return type    : CTSocket* 	
// Argument       : short port	
// 
// Description	  : Looks up a socket based on its port
//					(i.e. the port that the remote connection is listening on).
//
/////////////////////////////////////////////////////////////////////
CTSocket* CTConnections::GetSocketByPort(short port)
{
	// Find the socket with a matching port number.
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pSocket = *i ;
		
		if (pSocket->GetPort() == port)
			return pSocket ;
	}

	return NULL ;
}

long CTConnections::GetIndexByPort(short port)
{
	// Find the socket with a matching port number.
	long pos = -1 ;
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pSocket = *i ;
		pos++ ;

		if (pSocket->GetPort() == port)
			return pos ;
	}

	return -1 ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::EnableAll
// 
// Return type    : void 	
// Argument       : bool bEnable	
// 
// Description	  : Enable (or disable) all connections.
//
/////////////////////////////////////////////////////////////////////
void CTConnections::EnableAll(bool bEnable)
{
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pSocket = *i ;
		pSocket->Enable(bEnable) ;
	}
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::GetSocketName
// 
// Return type    : char const* 	
// Argument       : long index	
// 
// Description	  : Returns the name of a socket
//
/////////////////////////////////////////////////////////////////////
char const* CTConnections::GetSocketName(long index)
{
	CTSocket* pSocket = GetSocketByIndex(index) ;

	if (pSocket)
		return pSocket->GetName() ;
	else
		return NULL ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::GetSocketPort
// 
// Return type    : short 	
// Argument       : long index	
// 
// Description	  : Returns the port for a socket
//					This was the port the remote connection was listening on.
//
/////////////////////////////////////////////////////////////////////
short CTConnections::GetSocketPort(long index)
{
	CTSocket* pSocket = GetSocketByIndex(index) ;

	if (pSocket)
		return pSocket->GetPort() ;
	else
		return -1 ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTConnections::DeleteSockets
// 
// Return type    : void 	
// 
// Description	  : Clear the socket list and delete all of the
//					CTSocket objects.
//
/////////////////////////////////////////////////////////////////////
void CTConnections::DeleteAllSockets()
{
	// Empty the list
	for (SocketIter i = m_SocketList.begin() ; i != m_SocketList.end() ; i++)
	{
		CTSocket* pCTSocket = *i ;
		delete pCTSocket ;
	}

	m_SocketList.clear() ;
}
