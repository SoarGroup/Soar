// STI_Tool.cpp : Defines the entry point for the DLL application.
//

#include "STI_Tool.h"
#include "STI_CommonAPI.h"
#include "Constants.h"
#include "STI_CommandIDs.h"
#include "../SoarSocket/Check.h"
#include "../SoarSocket/Debug.h"

/////////////////////////////////////////////////////////////////////
// Function name  : STI_SendProduction
// 
// Return type    : STI_TOOL_API bool 	
// Argument       : char const* pProductionName	
// Argument       : char const* pProductionBody	
// 
// Description	  : Send a production to the runtime(s).
//
/////////////////////////////////////////////////////////////////////
STI_TOOL_API bool STI_SendProduction(STI_Handle hServer, char const* pProductionName, char const* pProductionBody)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pProductionName && pProductionBody) ;

	// Use body + 1 to ensure capture trailing null in the data
	return STI_SendCommandAsynch1(hServer, STI_kSendProduction, 0, strlen(pProductionBody)+1, 0, 0, 0, 0, 0, 0,
								  pProductionName, pProductionBody) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_SendFile
// 
// Return type    : STI_TOOL_API bool 		// false if sending command fails
// Argument       : STI_Handle hServer		// Handle to server instance
// Argument       : char const* pFileName	// Path to the file to send
// 
// Description	  : Send a file of productions to the runtime(s).
//		The contents of the file are *not* sent, just the filename.
//		To support remote connections this would need to be changed to send
//		the contents of the file--should be a different command "Send File Contents".
//
/////////////////////////////////////////////////////////////////////
STI_TOOL_API bool STI_SendFile(STI_Handle hServer, char const* pFileName)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pFileName) ;

	return STI_SendCommandAsynch1(hServer, STI_kSendFile, 0, 0, 0, 0, 0, 0, 0, 0,
								  pFileName, NULL) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_ProductionMatches
// 
// Return type    : STI_TOOL_API bool 				// false if sending command fails
// Argument       : STI_Handle hServer				// Handle to server instance
// Argument       : char const* pProductionName		// Name of production to run matches on
// 
// Description	  : Run matches command on this production.
//		The production should already be loaded in the runtime before
//		this command is issued.  If not, the runtime will report an error.
//
/////////////////////////////////////////////////////////////////////
STI_TOOL_API bool STI_ProductionMatches(STI_Handle hServer, char const* pProductionName)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pProductionName) ;

	return STI_SendCommandAsynch1(hServer, STI_kProductionMatches, 0, 0, 0, 0, 0, 0, 0, 0,
								  pProductionName, NULL) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_ExciseProduction
// 
// Return type    : STI_TOOL_API bool 				// false if sending command fails
// Argument       : STI_Handle hServer				// Handle to server instance
// Argument       : char const* pProductionName		// Name of production to run matches on
// 
// Description	  : Excises (removes) a production from the runtime's production memory.
//
/////////////////////////////////////////////////////////////////////
STI_TOOL_API bool STI_ExciseProduction(STI_Handle hServer, char const* pProductionName)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pProductionName) ;

	return STI_SendCommandAsynch1(hServer, STI_kExciseProduction, 0, 0, 0, 0, 0, 0, 0, 0,
								  pProductionName, NULL) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_SendRawCommand
// 
// Return type    : STI_TOOL_API bool 				// false if sending command fails
// Argument       : STI_Handle hServer				// Handle to server instance
// Argument       : char const* pCommandString		// Raw command string
// 
// Description	  : The command string is sent directly to the runtime
//					where it will be executed.
//
/////////////////////////////////////////////////////////////////////
STI_TOOL_API bool STI_SendRawCommand(STI_Handle hServer, char const* pCommandString)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pCommandString) ;
	
	return STI_SendCommandAsynch1(hServer, STI_kSendRawCommand, 0, 0, 0, 0, 0, 0, 0, 0,
								  pCommandString, NULL) ;
}
