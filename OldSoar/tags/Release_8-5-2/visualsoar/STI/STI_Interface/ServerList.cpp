// ServerList.cpp: implementation of the CTServerList class.
//
//////////////////////////////////////////////////////////////////////

#include "ServerList.h"
#include "ServerData.h"
#include "../SoarSocket/Debug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


void CTServerList::AddServer(STI_Handle hServer)
{
	if (hServer)
		m_ServerList.push_back(hServer) ;
}

void CTServerList::RemoveServer(STI_Handle hServer)
{
	if (hServer)
		m_ServerList.remove(hServer) ;
}

bool CTServerList::HasServer(STI_Handle hServer)
{
	for (ServerIter i = m_ServerList.begin() ; i != m_ServerList.end() ; i++)
	{
		STI_Handle val = *i ;
		if (val == hServer)
			return true ;
	}

	return false ;
}

// This is the main purpose for this class.
// Shutting down any remaining connections that the owner
// forgot to close down themselves.
void CTServerList::CloseAllDown()
{
	for (ServerIter i = m_ServerList.begin() ; i != m_ServerList.end() ; i++)
	{
		PrintDebug("Error: Closing down a server that was not shut down by its owner") ;

		STI_Handle val = *i ;
		((CTServerData*)val)->CloseAll() ;
	}

	m_ServerList.clear() ;
}


