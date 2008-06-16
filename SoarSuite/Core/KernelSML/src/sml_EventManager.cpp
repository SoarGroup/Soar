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

using namespace sml ;

/*
	  soar_add_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
					     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 
						 HandleKernelRunEventCallback,
						 static_cast <void*> (eventInfo), 0, 
						 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
*/
