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
typedef void (*DirectAddWMEStringFunction)(Direct_AgentSML_Handle, char const*, char const*, char const*, long) ; // agent, id, attr, val, timetag
typedef void (*DirectAddWMEIntFunction)(Direct_AgentSML_Handle, char const*, char const*, int, long) ; // agent, id, attr, val, timetag
typedef void (*DirectAddWMEDoubleFunction)(Direct_AgentSML_Handle, char const*, char const*, double, long) ; // agent, id, attr, val, timetag
typedef void (*DirectRemoveWMEFunction)(Direct_AgentSML_Handle, long) ; // agent, timetag

typedef void (*DirectAddIDFunction)(Direct_AgentSML_Handle, char const*, char const*, char const*, long) ; // agent, id, attr, value_id, timetag

typedef Direct_AgentSML_Handle (*DirectGetAgentSMLHandleFunction)(char const*) ; // agent name

typedef void (*DirectRunFunction)(char const*, bool, int, int, int) ;

/*************************************************************
* @brief	Add a wme.
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute The attribute name to use
* @param value		The value to use
*************************************************************/
EXPORT void sml_DirectAddWME_String(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValue, long clientTimetag) ;
EXPORT void sml_DirectAddWME_Int(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, int value, long clientTimetag) ;
EXPORT void sml_DirectAddWME_Double(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, double value, long clientTimetag) ;

/*************************************************************
* @brief	Remove a wme.  This function also releases the IWme*
*			making it no longer valid.
* @param wm			The working memory object (either input or output)
* @param wme		The wme we're removing
*************************************************************/
EXPORT void sml_DirectRemoveWME(Direct_AgentSML_Handle pAgentSML, long clientTimetag) ;

/*************************************************************
* @brief	Creates a new identifier (parent ^attribute <new-id>).
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
*************************************************************/
EXPORT void sml_DirectAddID(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValueId, long clientTimetag) ;

EXPORT Direct_AgentSML_Handle sml_DirectGetAgentSMLHandle(char const* pAgentName) ;

EXPORT void sml_DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSize, int count) ;

#ifdef __cplusplus
} // extern C
#endif

#endif	// KERNEL_SML_DIRECT_H
