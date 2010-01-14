/////////////////////////////////////////////////////////////////
// EventManager class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class manages a list of connections which are interested
// in being notified when a particular event occurs in the kernel.
//
/////////////////////////////////////////////////////////////////

#include "sml_EventManager.h"

#include "sml_AgentSML.h"

using namespace sml ;

/*
	  soar_add_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
					     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 
						 HandleKernelRunEventCallback,
						 static_cast <void*> (eventInfo), 0, 
						 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
*/

void sml::flushPrintOnAgent( AgentSML* pFlushPrintOnThisAgent )
{
	if ( pFlushPrintOnThisAgent ) { 
		// we are NOT the print event
		pFlushPrintOnThisAgent->FlushPrintOutput();
	}
}
