// STI_Interface.cpp : Defines the entry point for the DLL application.
//

#include "STI_Interface.h"
#include "../SoarSocket/Check.h"

/////////////////////////////////////////////////////////////////////
// Function name  : DllMain
// 
// Return type    : BOOL APIENTRY 	
// Argument       : HANDLE hModule	
// Argument       : DWORD  ul_reason_for_call	
// Argument       : LPVOID lpReserved	
// 
// Description	  : Standard entry point to a DLL.
//
/////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "stdafx.h"
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	UNUSED(lpReserved) ;
	UNUSED(hModule) ;

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

