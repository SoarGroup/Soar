// Connections.h: interface for the CTConnections class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTIONS_H__CACE7B90_19AC_4E13_B895_D04548929C73__INCLUDED_)
#define AFX_CONNECTIONS_H__CACE7B90_19AC_4E13_B895_D04548929C73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Forward declaration to save including extra headers
class CTSocket ;

// Need to disable some warnings to compile VC++'s STL header at warning level 4.
#ifdef _MSC_VER
#pragma warning(push,3)
#endif

#include <list>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef std::list<CTSocket*> SocketList ;
typedef std::list<CTSocket*>::iterator SocketIter ;

class CTConnections  
{
protected:
	// The list of connections (sockets) that are currently in use.
	SocketList	m_SocketList ;

public:
	CTConnections();
	~CTConnections();

	// Accessors
	SocketList& GetSocketList()		   { return m_SocketList ; }
	long		GetNumberConnections() { return m_SocketList.size() ; }

	void		EnableAll(bool bEnable) ;

	// Get the n'th item in the list
	CTSocket*	GetSocketByIndex(long index) ;
	CTSocket*	GetSocketByName(char const* pName) ;
	CTSocket*	GetSocketByPort(short port) ;
	long		GetIndexByName(char const* pName) ;
	long		GetIndexByPort(short port) ;

	char const* GetSocketName(long index) ;
	short		GetSocketPort(long index) ;

	// Clear the contents of the list and delete all of the CTSocket objects.
	void		DeleteAllSockets() ;
};

#endif // !defined(AFX_CONNECTIONS_H__CACE7B90_19AC_4E13_B895_D04548929C73__INCLUDED_)
