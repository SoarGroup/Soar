// Msg.h: interface for the CTMsg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSG_H__AD2500EF_4538_475D_ACEF_795F06CBD9CA__INCLUDED_)
#define AFX_MSG_H__AD2500EF_4538_475D_ACEF_795F06CBD9CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>
#include "Check.h"

class CTCommand ;

#ifndef MAX_PARAM_VERSION_1
#define MAX_PARAM_VERSION_1	6
#endif

struct MsgStruct
{
	// Message data
	long		m_StructSize ;		// Size of this structure
	long		m_TotalMessageSize ;// Size of this structure + size of data buffer
	long		m_ByteOrder ;		// Sender sets to 0x12345678 (so receiver can check byte order)
	long		m_MessageVersion ;	// (Major version << 8 + Minor version).

	long		m_FromPort ;		// Port number of connection sending this msg
	long		m_ToPort ;			// Port number of connection receiving this msg
	long		m_UniqueID ;		// ID that's unique for messages over this connection (never 0)
	long		m_AckID ;			// If this is a message acknowledging another message--this is the other message's ID

	long		m_SystemMsg ;		// 0 for user commands, > 0 for system commands
	long		m_MsgFlags ;		// Not currently used

	// Command data
	long		m_CommandID ;
	long		m_CommandFlags ;	// Not currently used
	long		m_DataSize ;		// If non-0 gives size in bytes of arbitrary data buffer sent after command
	char*		m_pData ;			// Only valid if dataSize != 0.  Data sent after Msg is sent.
	long		m_Params[MAX_PARAM_VERSION_1] ;

	char		m_StringParam1[512] ;

	// For later expansion
	long		m_Unused1 ;			// Fields for later use w/o changing msg struct size
	long		m_Unused2 ;
	long		m_Unused3 ;
	long		m_Unused4 ;
	long		m_Unused5 ;
	long		m_Unused6 ;
	char		m_Unused7[32] ;
	char		m_Unused8[32] ;
} ;

// Can check against sizeof(Msg) to ensure all platforms
// agree on size of values being sent.
const long kMsg1Size = 10*4 + 10*4 + 512 + 6*4 + 32 + 32 ;

class CTMsg
{
protected:
	MsgStruct	m_Struct ;			// The real message data

public:
	CTMsg()  ;
	~CTMsg() ;

private:
	// Disable the copy constructor and assignment ops.  Force them to be called explicitly.
	CTMsg(CTMsg const& from) { UNUSED(from) ; assert(0) ; } ;
	CTMsg const& operator= (CTMsg const& from) { UNUSED(from) ; assert(0) ; return *this ; } ;

public:
	// Get read/write access to the internal data structure.
	MsgStruct const& GetReadStruct()  const { return m_Struct ; } ;
	MsgStruct&       GetWriteStruct()       { return m_Struct ; } ;

	void		SetVersion(short major, short minor) { m_Struct.m_MessageVersion = (major << 8) + minor ; }
	short		GetMajorVersion()					{ return (short)(m_Struct.m_MessageVersion >> 8) ; }
	short		GetMinorVersion()					{ return (short)(m_Struct.m_MessageVersion & 0xFFFF) ; }

	void		SetFromPort(long port)				{ m_Struct.m_FromPort = port ; }
	long		GetFromPort()						{ return m_Struct.m_FromPort ; }

	void		SetToPort(long port)				{ m_Struct.m_ToPort = port ; }
	long		GetToPort()							{ return m_Struct.m_ToPort ; }

	void		SetUniqueID(long id)				{ m_Struct.m_UniqueID = id ; }
	long		GetUniqueID()						{ return m_Struct.m_UniqueID ; }

	void		SetTotalSize()						{ m_Struct.m_TotalMessageSize = m_Struct.m_StructSize + m_Struct.m_DataSize ; }

	bool		IsValid() ;

	// Make a copy of the data buffer passed in and store it.
	bool		SetData(const char* pStr, long dataLength) ;

	// Accessors to get to the data buffer
	char const* GetData()     const	{ return m_Struct.m_pData ; } ;
	long		GetDataSize() const { return m_Struct.m_DataSize ; } ;

	// Allocate a new, empty buffer and return a pointer to it.
	char*		AllocateDataBuffer(bool bDeleteExistingData, long dataLength) ;

	// Copy from messages or commands
	bool CopyFromMsg(CTMsg const& from) ;
	bool CopyFromCommand(CTCommand const& from) ;

	// Print debug information
	void Dump() ;

	// Clear the current data fields, deleting the data as well if bFreeExisting is true.
	void DeleteData(bool bFreeExisting = true) ;
} ;

#endif // !defined(AFX_MSG_H__AD2500EF_4538_475D_ACEF_795F06CBD9CA__INCLUDED_)
