// CommandQueue.h: interface for the CTCommandQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDQUEUE_H__0CF694D1_87C8_4968_985F_4A75617D12DE__INCLUDED_)
#define AFX_COMMANDQUEUE_H__0CF694D1_87C8_4968_985F_4A75617D12DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Need to disable some warnings to compile VC++'s <list> header at warning level 4.
#ifdef _MSC_VER
#pragma warning(push,3)
#endif

#include <list>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Use a forward declaration rather than including header--makes for faster compilations.
class CTCommand ;

// BADBAD: Perhaps use a deque here?  It's not really a list.
typedef std::list<CTCommand*> CommandList ;
typedef std::list<CTCommand*>::iterator CommandIter ;

class CTCommandQueue  
{
protected:
	CommandList m_Commands ;

public:
	CTCommandQueue();
	virtual ~CTCommandQueue();

	bool			 CopyCommandAndPushBack(CTCommand& command) ;
	bool			 GetFrontCommand(CTCommand& command) ;
	CTCommand const* GetFrontCommandPointer() ;
	void			 PopFrontCommand() ;
	bool			 IsCommandAvailable() ;

	// Lock + Unlock control ensure single thread access to the queue data.
	void Lock() {} ;	// Need to implement if use separate thread to pump messages
	void Unlock() {} ;	// Need to implement if use separate thread to pump messages

protected:
	void DeleteContents() ;
};

#endif // !defined(AFX_COMMANDQUEUE_H__0CF694D1_87C8_4968_985F_4A75617D12DE__INCLUDED_)
