#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// KernelSMLdll.cpp : Defines the entry point for the DLL application.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// This library is responsible for sending and receiving SML commands (SML is an XML dialect).
//
// It communicates to the kernel through gSKI, converting commands such as "print s4" into appropriate
// gSKI calls and then packaging and returning the output to the caller.
//
// The SML can be sent as an XML stream over a socket or as an ElementXML object that is created by the client
// and passed directly to this library (this latter version improves efficiency and is only possible when the kernel is
// embedded in the client application).
/////////////////////////////////////////////////////////////////

#include "KernelSMLdll.h"
#include "sml_KernelSML.h"

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
#ifdef _DEBUG
//	_crtBreakAlloc = 165;
#endif

	unused(hModule) ;
	unused(ul_reason_for_call) ;
	unused(lpReserved) ;

// Define this ourselves to save bringing in the entire windows headers for this one value.
#ifndef DLL_PROCESS_DETACH
#define DLL_PROCESS_DETACH 0
#endif
#ifndef DLL_PROCESS_ATTACH
#define DLL_PROCESS_ATTACH   1    
#endif

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Record the module handle so that later we can find where
		// this DLL was loaded from.
		sml::KernelSML::SetModuleHandle(hModule) ;
	}

	// Dump out any memory leaks to the output window in the Visual C++ debugger and to stdout.
	// Only do this when we are unloaded.
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		// We have a singleton kernel object.
		// Explicitly clean it up here so we can test for memory leaks (otherwise it will
		// always show up as a leak--when in fact it's designed to be kept around until now when
		// the DLL is unloaded).
		sml::KernelSML::DeleteSingleton() ;

#ifdef _DEBUG
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

// Disabled memory leak detection now we're statically linked to gSKI and the kernel as they're
// leaking a lot and it's going to take a while to fix them all.  Hopefully we'll turn
// it back on at some point and chase them all down.
//		_CrtDbgReport(_CRT_WARN, NULL, NULL, "KernelSML", "Checking memory in KernelSML\n");
//		_CrtDumpMemoryLeaks();
#endif
	}

    return 1;
}
#endif
