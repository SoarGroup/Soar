// SoarSocket.cpp : Defines the entry point for the DLL application.
//

#include "SoarSocket.h"
#include "Check.h"

#ifdef _WINDOWS
#include "stdafx.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	UNUSED(hModule) ;
	UNUSED(lpReserved) ;

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

// This is an example of an exported variable
SOARSOCKET_API int nSoarSocket=0;

// This is an example of an exported function.
SOARSOCKET_API int fnSoarSocket(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see SoarSocket.h for the class definition
CSoarSocket::CSoarSocket()
{ 
	return; 
}

