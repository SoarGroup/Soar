// ServerList.h: interface for the CTServerList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERLIST_H__3DE32751_6F03_45D4_B6E4_0EBCC3A366C1__INCLUDED_)
#define AFX_SERVERLIST_H__3DE32751_6F03_45D4_B6E4_0EBCC3A366C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "STI_CommonAPI.h"		// For STI_Handle

// Need to disable some warnings to compile VC++'s STL header at warning level 4.
#ifdef _MSC_VER
#pragma warning(push,3)
#endif

#include <list>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef std::list<STI_Handle>	ServerList ;
typedef ServerList::iterator	ServerIter ;

class CTServerList  
{
protected:
	ServerList	m_ServerList ;
public:
	CTServerList() {} ;
	~CTServerList() { CloseAllDown() ; } ;

	// Add a server to the list
	void AddServer(STI_Handle hServer) ;

	// Remove a server from the list
	void RemoveServer(STI_Handle hServer) ;

	// Check if a server is in the list
	bool HasServer(STI_Handle hServer) ;

	// This is the main purpose for this class.
	// Shutting down any remaining connections that the owner
	// forgot to close down themselves.
	void CloseAllDown() ;
};

#endif // !defined(AFX_SERVERLIST_H__3DE32751_6F03_45D4_B6E4_0EBCC3A366C1__INCLUDED_)
