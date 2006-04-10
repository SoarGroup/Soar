#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ElementXMLdll: Defines the entry point for this DLL.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This library is responsible for representing an XML document as an object (actually a tree of objects).
//
// A client can send a stream of XML data which this class parses to create the object representation of the XML.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// XML output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of XML which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
/////////////////////////////////////////////////////////////////

#include "ElementXMLdll.h"

// Check for memory leaks
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef _WIN32
bool __stdcall DllMain( void * hModule, 
                       unsigned long  ul_reason_for_call, 
                       void * lpReserved
					 )
{
	//_crtBreakAlloc = 243;
	unused(hModule) ;
	unused(ul_reason_for_call) ;
	unused(lpReserved) ;

#ifdef _DEBUG
	// Define this ourselves to save bringing in the entire windows headers for this one value.
#ifndef DLL_PROCESS_DETACH
#define DLL_PROCESS_DETACH 0
#endif

	// Dump out any memory leaks to the output window in the Visual C++ debugger and to stdout.
	// Only do this when we are unloaded.
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

		_CrtDbgReport(_CRT_WARN, NULL, NULL, "ElementXML", "Checking memory in ElementXML\n");
		_CrtDumpMemoryLeaks();
	}
#endif

    return 1;
}
#endif
