/////////////////////////////////////////////////////////////////
// CommandLineInterfacedll.cpp : Defines the entry point for the DLL application.
//
/////////////////////////////////////////////////////////////////

#include "CommandLineInterfacedll.h"

#ifdef _WIN32

// Check for memory leaks
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

bool __stdcall DllMain( void * hModule, 
                       unsigned long  ul_reason_for_call, 
                       void * lpReserved
					 )
{
	//_crtBreakAlloc = 526;
	unused(hModule) ;
	unused(ul_reason_for_call) ;
	unused(lpReserved) ;

	// Define this ourselves to save bringing in the entire windows headers for this one value.
#ifndef DLL_PROCESS_DETACH
#define DLL_PROCESS_DETACH 0
#endif

	// Dump out any memory leaks to the output window in the Visual C++ debugger and to stdout.
	// Only do this when we are unloaded.
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		// We have a singleton kernel object.
		// Explicitly clean it up here so we can test for memory leaks (otherwise it will
		// always show up as a leak--when in fact it's designed to be kept around until now when
		// the DLL is unloaded).
		//sml::KernelSML::DeleteSingleton() ;

#ifdef _DEBUG
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

		_CrtDbgReport(_CRT_WARN, NULL, NULL, "KernelSML", "Checking memory in KernelSML\n");
		_CrtDumpMemoryLeaks();
#endif
	}

    return 1;
}
#endif
