// Command.cpp: implementation of the CTCommand class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "Command.h"
#include "Msg.h"
#include "Debug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCommand::CTCommand()
{
	memset(&m_Struct, 0, sizeof(m_Struct)) ;
	m_Struct.m_DataSize = 0 ;
	m_Struct.m_pData = NULL ; 

	SetVersion(kMajorVersion, kMinorVersion) ;
	m_Struct.m_StructSize = sizeof(m_Struct) ;
	m_Struct.m_TotalMessageSize = sizeof(m_Struct) ;
	m_Struct.m_ByteOrder = 0x12345678 ;	// Allows receiver to determine by order
}

CTCommand::~CTCommand()
{
	DeleteData() ;
}

bool CTCommand::IsValid()
{
	bool valid = (m_Struct.m_ByteOrder == 0x12345678) &&
		 (m_Struct.m_StructSize == sizeof(MsgStruct)) &&
		 (GetMajorVersion() == 1) ;

	return valid ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::DeleteData
// 
// Return type    : void 	
// Argument       : bool bFreeDataBlock /* == true */	
// 
// Description	  : Clear the data fields in the structure
//					and free the data block (unless bFreeExisting is false).
//
/////////////////////////////////////////////////////////////////////
void CTCommand::DeleteData(bool bFreeDataBlock /* == true */)
{
	if (bFreeDataBlock)
	{
		delete m_Struct.m_pData ;
		m_Struct.m_pData = NULL ;
	}

	m_Struct.m_DataSize = 0 ;

	SetTotalSize() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::CopyFromCommand
// 
// Return type    : bool 	
// Argument       : CTCommand const& from	
// 
// Description	  : Utility function to copy from a command.
//					Not using a copy constructor so we don't
//					do this by accident.
//
/////////////////////////////////////////////////////////////////////
bool CTCommand::CopyFromCommand(CTCommand const& from)
{
	// Free any data buffer we are pointing to.
	DeleteData() ;
	
	// Memberwise copy of the data
	m_Struct = from.GetReadStruct() ;

	// Clear the fields that we need to duplicate (in case duplication fails)
	m_Struct.m_pData = NULL ;
	m_Struct.m_DataSize = 0 ;

	bool ok = SetDataFromCopy(from.GetData(), from.GetDataSize()) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::CopyFromMsg
// 
// Return type    : bool 	
// Argument       : CTMsg const& from	
// 
// Description	  : Utility function to copy from a message.
//					Not using a copy constructor so we don't
//					do this by accident.
//
/////////////////////////////////////////////////////////////////////
bool CTCommand::CopyFromMsg(CTMsg const& from)
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
	bool ok = SetDataFromCopy(from.GetData(), from.GetDataSize()) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::AllocateDataBuffer
// 
// Return type    : char* 	
// Argument       : bool bDeleteExistingData	
// Argument       : long dataLength	
// 
// Description	  : Allocate the internal data buffer and return
//					a pointer to it.
//
/////////////////////////////////////////////////////////////////////
char* CTCommand::AllocateDataBuffer(bool bDeleteExistingData, long dataLength)
{
	if (bDeleteExistingData)
		DeleteData() ;

	// TODO: Need to typedef BYTE
	m_Struct.m_pData = new char[dataLength] ;
	m_Struct.m_DataSize = dataLength ;

	// Check we got the memory
	if (m_Struct.m_pData == NULL) { m_Struct.m_DataSize = 0 ; SetTotalSize() ; return NULL ; }

	SetTotalSize() ;

	return m_Struct.m_pData ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::SetDataFromCopy
// 
// Return type    : bool 				// returns true if allocation succeeds.
// Argument       : const char* pCopyFromStr	
// Argument       : long dataLength	
// 
// Description	  : Delete the existing data buffer, create a new one and copy
//					pStr into the new buffer.
//
/////////////////////////////////////////////////////////////////////
bool CTCommand::SetDataFromCopy(const char* pCopyFromStr, long dataLength)
{
	DeleteData() ;

	if (dataLength > 0 && pCopyFromStr != NULL)
	{
		// TODO: Need to typedef BYTE
		m_Struct.m_pData = new char[dataLength] ;
		m_Struct.m_DataSize = dataLength ;
		
		// Check we got the memory
		if (m_Struct.m_pData == NULL) { m_Struct.m_DataSize = 0 ; SetTotalSize() ; return false ; }

		// Copy the data
		memcpy(m_Struct.m_pData, pCopyFromStr, dataLength) ;
	}

	SetTotalSize() ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTCommand::Dump
// 
// Return type    : void 	
// 
// Description	  : Print debug information about this object.
//
/////////////////////////////////////////////////////////////////////
void CTCommand::Dump()
{
	CTDEBUG_ENTER_METHOD("CTCommand::Dump");

	char data[1024] ;
	
	if (this->GetData())
		SafeStrncpy(data, this->GetData(), 100) ;
	else
		strcpy(data, "--No data--") ;

	PrintDebugFormat("Command ID %d StringParam1 <%s> DataSize %d Data[0-100] <%s>",
		m_Struct.m_CommandID, m_Struct.m_StringParam1, this->GetDataSize(), data) ;
}

