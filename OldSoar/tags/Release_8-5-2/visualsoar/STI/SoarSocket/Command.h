// CTCommand.h: interface for the CTCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTCommand_H__7431BE11_7C38_46EB_82D1_E5DEE0734DF1__INCLUDED_)
#define AFX_CTCommand_H__7431BE11_7C38_46EB_82D1_E5DEE0734DF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>
#include "CommandStruct.h"
#include "Utils.h"
#include "Check.h"

#ifdef _MSC_VER
// Disable warning that optimizer has removed an unused inline function.
#pragma warning(disable: 4514)
#endif

class CTMsg ;

class CTCommand  
{
protected:
	CommandStruct	m_Struct ;			// The real command data

public:
	CTCommand() ;
	~CTCommand() ;

private:
	// Disable the copy constructor and assignment ops.  Force them to be called explicitly.
	CTCommand(CTCommand const& from) { UNUSED(from) ; assert(0) ; } ;
	CTCommand const& operator= (CTCommand const& from) { UNUSED(from) ; assert(0) ; return *this ; } ;

public:
	CommandStruct const& GetReadStruct() const		{ return m_Struct ; } ;
	CommandStruct&       GetWriteStruct()			{ return m_Struct ; } ;

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

	void		SetCommandID(long id)				{ m_Struct.m_CommandID = id ; } 
	long		GetCommandID() const				{ return m_Struct.m_CommandID ; }

	void		SetCommandFlags(long flags)			{ m_Struct.m_CommandFlags = flags ; }
	long		GetCommandFlags() const				{ return m_Struct.m_CommandFlags ; }

	bool		IsValid() ;

	// Note: Param is a 1-based index to match naming (e.g. to get param2 pass 2)
	void		SetParam(long param, long value)	{ assert(param >= 1 && param <= MAX_PARAM_VERSION_1) ;
													  m_Struct.m_Params[param-1] = value ; }

	long		GetParam(long param) const			{ assert(param >= 1 && param <= MAX_PARAM_VERSION_1) ;
													  return m_Struct.m_Params[param-1] ; }

	void		SetSystemMsg()						{ m_Struct.m_SystemMsg = kSystemMsg ; }
	long		GetSystemMsg() const				{ return m_Struct.m_SystemMsg ; }
	bool		IsSystemMsg()						{ return (m_Struct.m_SystemMsg == kSystemMsg) ; }
	
	void		SetStringParam1(const char* pStr)	{ SafeStrncpy(m_Struct.m_StringParam1, pStr, sizeof(m_Struct.m_StringParam1)) ; }
	char const*	GetStringParam1()  const			{ return m_Struct.m_StringParam1 ; }

	bool		SetDataFromCopy(const char* pCopyFromStr, long dataLength) ;
	char const*	GetData() const						{ return m_Struct.m_pData ; } ;
	long		GetDataSize() const					{ return m_Struct.m_DataSize ; } ;

	char*		AllocateDataBuffer(bool bDeleteExistingData, long dataLength) ;

	bool		CopyFromCommand(CTCommand const& from) ;
	bool		CopyFromMsg(CTMsg const& from) ;
	void		Dump() ;

	void		DeleteData(bool bFreeDataBlock = true) ;
};

#endif // !defined(AFX_CTCommand_H__7431BE11_7C38_46EB_82D1_E5DEE0734DF1__INCLUDED_)
