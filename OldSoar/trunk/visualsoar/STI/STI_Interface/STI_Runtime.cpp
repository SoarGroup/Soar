// STI_Runtime.cpp : Defines the entry point for the DLL application.
//

#include "STI_Runtime.h"
#include "STI_CommonAPI.h"
#include "Constants.h"
#include "STI_CommandIDs.h"
#include "../SoarSocket/Check.h"
#include "../SoarSocket/Debug.h"

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EditProduction
// 
// Return type    : STI_RUNTIME_API bool 	
// Argument       : char const* pProductionName	
// 
// Description	  : Send the edit production command to the tool(s).
//
/////////////////////////////////////////////////////////////////////
STI_RUNTIME_API bool STI_EditProduction(STI_Handle hServer, char const* pProductionName)
{
	CHECK_RET_FALSE(hServer) ;
	CHECK_RET_FALSE(pProductionName) ;

	return STI_SendCommandAsynch1(hServer, STI_kEditProduction, 0, 0, 0, 0, 0, 0, 0, 0, pProductionName, NULL) ;
}

