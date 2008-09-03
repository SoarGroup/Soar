#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  xml.cpp
 *
 * =======================================================================
 *  
 * Contains methods for generating XML objects in response to kernel commands.
 *
 * The commands are modelled after the existing kernel functions which are tied to generating
 * string output.  In the past we added code to some of those functions so they'd
 * generate string output and also XML output (as a string: <aaa>...</aaa>).  To capture
 * the XML output a caller would register for the XML callback, generate the XML as a string,
 * parse the XML back to an object and return it.
 *
 * These methods generate XML as an object, so there are no strings being created
 * and subsequently parsed and no need to "intercept" the XML callback channel (which is
 * really for XML trace output to the debugger, not for results from commands).
 * This new approach is more efficient to both create and to subsequently use.
 *
 * =======================================================================
 */

#include "rete.h"
#include "kernel.h"
#include "mem.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "explain.h"
#include "symtab.h"
#include "agent.h"
#include "print.h"
#include "production.h"
#include "init_soar.h"
#include "instantiations.h"
#include "rhsfun.h"
#include "lexer.h"

#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes

// So we can call directly to it to create XML output.
// If you need a kernel w/o any external dependencies, removing this include
// and the implementation of these XML methods should do the trick.
#include "cli_CommandLineInterface.h"	

using namespace xmlTraceNames ;

/////////////////////////////////////////////////////////////////
//
// XML Generation functions.
// 
// These are currently local to rete while I'm working on matches.
// They should eventually move to their own file with a new header.
//
/////////////////////////////////////////////////////////////////
void xmlBeginTag(char const* pTag)
{
	// Should be using callbacks like this, so we don't need to link the CLI to the kernel in order for the kernel to compile correctly.
	// This callback would remove the need for "cli::GetCLI()" calls
	/*
	soar_invoke_callbacks(thisAgent, thisAgent, 
						XML_GENERATION_CALLBACK,
						(soar_call_data) NULL);
	*/
	cli::GetCLI()->XMLBeginTag(pTag) ;
}

void xmlEndTag(char const* pTag)
{
	cli::GetCLI()->XMLEndTag(pTag) ;
}

void xmlString(char const* pAttribute, char const* pValue)
{
	cli::GetCLI()->XMLAddAttribute(pAttribute, pValue) ;
}

void xmlSymbol(agent* thisAgent, char const* pAttribute, Symbol* pSymbol)
{
	// Passing 0, 0 as buffer to symbol to string causes it to use internal, temporary buffers
	// which is fine because we immediately copy that string in XMLAddAttribute.
	cli::GetCLI()->XMLAddAttribute(pAttribute, symbol_to_string(thisAgent, pSymbol, true, 0, 0)) ;
}

// These "moveCurrent" methods allow us to move the entry point for new XML
// around in the existing structure.  That's not often required but occassionally is helpful.
void xmlMoveCurrentToParent()
{
	cli::GetCLI()->XMLMoveCurrentToParent() ;
}

void xmlMoveCurrentToChild(int index)
{
	cli::GetCLI()->XMLMoveCurrentToChild(index) ;
}

void xmlMoveCurrentToLastChild()
{
	cli::GetCLI()->XMLMoveCurrentToLastChild() ;
}

void xmlULong(char const* pAttribute, unsigned long value)
{
	char buf[51];
	snprintf(buf, 50, "%lu", value);
	cli::GetCLI()->XMLAddAttribute(pAttribute, buf) ;
}

void xmlInt(char const* pAttribute, int value)
{
	char buf[51];
	snprintf(buf, 50, "%d", value);
	cli::GetCLI()->XMLAddAttribute(pAttribute, buf) ;
}

void xmlAddSimpleTag(char const* pTag)
{
	cli::GetCLI()->XMLBeginTag(pTag) ;
	cli::GetCLI()->XMLEndTag(pTag) ;
}

void xmlAttValue(char const* pTag, char const* pAttribute, char const* pValue)
{
	cli::GetCLI()->XMLBeginTag(pTag) ;
	cli::GetCLI()->XMLAddAttribute(pAttribute, pValue) ;
	cli::GetCLI()->XMLEndTag(pTag) ;
}

void xml_wme (agent* thisAgent, wme *w) {
  // <wme tag="123" id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>
  xmlBeginTag(kTagWME) ;

  xmlULong(kWME_TimeTag, w->timetag) ;
  xmlSymbol(thisAgent, kWME_Id, w->id);
  xmlSymbol(thisAgent, kWME_Attribute, w->attr) ;
  xmlSymbol(thisAgent, kWME_Value, w->value) ;
  xmlString(kWME_ValueType, symbol_to_typeString(thisAgent, w->value)) ;
  if (w->acceptable) xmlString(kWMEPreference, "+");

  xmlEndTag(kTagWME) ;
}
