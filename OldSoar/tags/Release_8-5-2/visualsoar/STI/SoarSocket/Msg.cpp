// Msg.cpp: implementation of the CTMsg class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <memory.h>
#include "Msg.h"
#include "Command.h"
#include "Debug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTMsg::CTMsg()
{
	memset(&m_Struct, 0, sizeof(m_Struct)) ;
	m_Struct.m_DataSize = 0 ;
	m_Struct.m_pData = NULL ;

	m_Struct.m_StructSize = sizeof(m_Struct) ;
	m_Struct.m_TotalMessageSize = sizeof(m_Struct) ;
	m_Struct.m_ByteOrder = 0x12345678 ;	// Allows receiver to determine by order
}

CTMsg::~CTMsg()
{
	DeleteData() ;
}

// Check if a message is valid (i.e. something we can understand)
bool CTMsg::IsValid()
{
	bool valid = (m_Struct.m_ByteOrder == 0x12345678) &&
		 (m_Struct.m_StructSize == sizeof(MsgStruct)) &&
		 (GetMajorVersion() == 1) ;

	return valid ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::DeleteData
// 
// Return type    : void 	
// Argument       : bool bFreeExisting /* == true */	// Whether to m_pData or not
// 
// Description	  : Clear the data fields in the structure
//					and free the data block (unless bFreeExisting is false).
//
/////////////////////////////////////////////////////////////////////
void CTMsg::DeleteData(bool bFreeExisting /* == true */)
{
	if (bFreeExisting)
		delete m_Struct.m_pData ;

	m_Struct.m_pData = NULL ;
	m_Struct.m_DataSize = 0 ;

	SetTotalSize() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::CopyFromMsg
// 
// Return type    : bool 	
// Argument       : CTMsg const& from	
// 
// Description	  : Utility function to copy from a message.
//					Not using a copy constructor so we don't
//					do this by accident.
//
/////////////////////////////////////////////////////////////////////
bool CTMsg::CopyFromMsg(CTMsg const& from)
{
	// Free any data buffer we are pointing to.
	DeleteData() ;
	
	// Memberwise copy of the data
	m_Struct = from.GetReadStruct() ;

	// Clear the fields that we need to duplicate (in case duplication fails)
	m_Struct.m_pData = NULL ;
	m_Struct.m_DataSize = 0 ;

	bool ok = SetData(from.GetData(), from.GetDataSize()) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::CopyFromCommand
// 
// Return type    : bool 	
// Argument       : CTCommand const& from	
// 
// Description	  : Utility function to copy from a command to a message
//
/////////////////////////////////////////////////////////////////////
bool CTMsg::CopyFromCommand(CTCommand const& from)
{
	// Free any data buffer we are pointing to.
	DeleteData() ;

	// Copy the data (BADBAD: Relies on them being the same structure for now)
	assert(sizeof(m_Struct) == sizeof(from.GetReadStruct())) ;
	memcpy(&m_Struct, &from.GetReadStruct(), sizeof(m_Struct)) ;

	// Clear the fields that we need to duplicate (in case duplication fails)
	m_Struct.m_pData = NULL ;
	m_Struct.m_DataSize = 0 ;

	// Make a copy of the data buffer
	bool ok = SetData(from.GetData(), from.GetDataSize()) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::AllocateDataBuffer
// 
// Return type    : char* 	
// Argument       : bool bDeleteExistingData	
// Argument       : long dataLength	
// 
// Description	  : Allocate the internal data buffer and return
//					a pointer to it.
//
/////////////////////////////////////////////////////////////////////
char* CTMsg::AllocateDataBuffer(bool bDeleteExistingData, long dataLength)
{
	if (bDeleteExistingData)
		DeleteData() ;

	m_Struct.m_pData = new char[dataLength] ;
	m_Struct.m_DataSize = dataLength ;

	// Check we got the memory
	if (m_Struct.m_pData == NULL) { m_Struct.m_DataSize = 0 ; SetTotalSize() ; return NULL ; }

	SetTotalSize() ;

	return m_Struct.m_pData ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::SetData
// 
// Return type    : bool 				// returns true if allocation succeeds.
// Argument       : const char* pStr	
// Argument       : long dataLength	
// 
// Description	  : Delete the existing data buffer, create a new one and copy
//					pStr into the new buffer.
//
/////////////////////////////////////////////////////////////////////
bool CTMsg::SetData(const char* pStr, long dataLength)
{
	DeleteData() ;

	if (dataLength > 0 && pStr != NULL)
	{
		m_Struct.m_pData = new char[dataLength] ;
		m_Struct.m_DataSize = dataLength ;
		
		// Check we got the memory
		if (m_Struct.m_pData == NULL) { m_Struct.m_DataSize = 0 ; SetTotalSize() ; return false ; }

		// Copy the data
		memcpy(m_Struct.m_pData, pStr, dataLength) ;
	}

	SetTotalSize() ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTMsg::Dump
// 
// Return type    : void 	
// 
// Description	  : Print debug information about this object.
//
/////////////////////////////////////////////////////////////////////
void CTMsg::Dump()
{
	CTDEBUG_ENTER_METHOD("CTMsg::Dump");

	PrintDebugFormat("Msg ID %d StringParam1 <%s>",
		m_Struct.m_CommandID, m_Struct.m_StringParam1) ;
}