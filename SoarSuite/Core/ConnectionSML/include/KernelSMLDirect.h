/////////////////////////////////////////////////////////////////
// KernelSMLDirect file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This file defines a few methods that allow a client that's in-process
// to call directly to KernelSML without using the normal message passing scheme.
//
// These methods allow us to optimize the elements that are most time critical,
// which is I/O when using an embedded connection.
//
// None of these methods are required for correct functioning, but by adding
// them we can improve overall performance significantly by short-circuiting the
// standard command path.
//
/////////////////////////////////////////////////////////////////

#ifndef KERNEL_SML_DIRECT_H
#define KERNEL_SML_DIRECT_H

#include "sml_Handles.h"

// For test
//#define WIN_STATIC_LINK

// get definition of EXPORT
#include "Export.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function types, so we can more easily load the methods dynamically.
// (The value being defined is the function pointer *DirectXXXFunction)
typedef Direct_WME_Handle			(*DirectAddWMEStringFunction)(Direct_WorkingMemory_Handle, Direct_WMObject_Handle, long, char const*, char const*) ;
typedef Direct_WME_Handle			(*DirectAddWMEIntFunction)(Direct_WorkingMemory_Handle, Direct_WMObject_Handle, long, char const*, int) ;
typedef Direct_WME_Handle			(*DirectAddWMEDoubleFunction)(Direct_WorkingMemory_Handle, Direct_WMObject_Handle, long, char const*, double) ;
typedef void						(*DirectRemoveWMEFunction)(Direct_WorkingMemory_Handle, Direct_WME_Handle, long) ;

typedef Direct_WME_Handle			(*DirectAddIDFunction)(Direct_WorkingMemory_Handle, Direct_WMObject_Handle, long, char const*) ;
typedef Direct_WME_Handle			(*DirectLinkIDFunction)(Direct_WorkingMemory_Handle, Direct_WMObject_Handle, long, char const*, Direct_WMObject_Handle) ;
typedef Direct_WMObject_Handle		(*DirectGetThisWMObjectFunction)(Direct_WorkingMemory_Handle, Direct_WME_Handle) ;

typedef Direct_WorkingMemory_Handle (*DirectGetWorkingMemoryFunction)(char const*, bool) ;
typedef Direct_WMObject_Handle		(*DirectGetRootFunction)(char const*, bool) ;
typedef void						(*DirectRunFunction)(char const*, bool, int, int, int) ;

typedef void						(*DirectReleaseWMEFunction)(Direct_WorkingMemory_Handle, Direct_WME_Handle, long) ;
typedef void						(*DirectReleaseWMObjectFunction)(Direct_WMObject_Handle) ;

/*************************************************************
* @brief	Add a wme.
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute The attribute name to use
* @param value		The value to use
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddWME_String(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long, char const* pAttribute, char const* value) ;
EXPORT Direct_WME_Handle sml_DirectAddWME_Int(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long, char const* pAttribute, int value) ;
EXPORT Direct_WME_Handle sml_DirectAddWME_Double(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long, char const* pAttribute, double value) ;

/*************************************************************
* @brief	Remove a wme.  This function also releases the IWme*
*			making it no longer valid.
* @param wm			The working memory object (either input or output)
* @param wme		The wme we're removing
*************************************************************/
EXPORT void sml_DirectRemoveWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme, long) ;

/*************************************************************
* @brief	Creates a new identifier (parent ^attribute <new-id>).
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long, char const* pAttribute) ;

/*************************************************************
* @brief	Creates a new wme with an existing id as its value
*			(parent ^attribute <existing-id>)
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
* @param orig		The original identifier (whose value we are copying)
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectLinkID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long, char const* pAttribute, Direct_WMObject_Handle orig) ;

/*************************************************************
* @brief	The AddID methods return a WME, but for gSKI you need
*			a WMObject to work with them, so this gets the identifier
*			(WMObject) from a wme.
*			(parent ^attribute <id>) - returns <id> (not parent)
*************************************************************/
EXPORT Direct_WMObject_Handle sml_DirectGetThisWMObject(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme) ;

//EXPORT Direct_Agent_Handle sml_DirectGetAgent(char const* pAgentName) ;
EXPORT Direct_WorkingMemory_Handle sml_DirectGetWorkingMemory(char const* pAgentName, bool input) ;
EXPORT Direct_WMObject_Handle sml_DirectGetRoot(char const* pAgentName, bool input) ;

EXPORT void sml_DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSize, int count) ;

EXPORT void sml_DirectReleaseWME(Direct_WorkingMemory_Handle, Direct_WME_Handle wme, long) ;
EXPORT void sml_DirectReleaseWMObject(Direct_WMObject_Handle parent) ;

#ifdef __cplusplus
} // extern C
#endif

#endif	// KERNEL_SML_DIRECT_H
