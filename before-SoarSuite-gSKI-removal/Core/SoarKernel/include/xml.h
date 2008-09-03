/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                 xml.h
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

======================================================================= */

#ifndef SOAR_XML_H
#define SOAR_XML_H

#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes

extern void xmlBeginTag(char const* pTag) ;
extern void xmlEndTag(char const* pTag) ;
extern void xmlString(char const* pAttribute, char const* pValue) ;
extern void xmlSymbol(agent* thisAgent, char const* pAttribute, Symbol* pSymbol) ;
extern void xmlMoveCurrentToParent() ;
extern void xmlMoveCurrentToChild(int index) ;
extern void xmlMoveCurrentToLastChild() ;
extern void xmlULong(char const* pAttribute, unsigned long value) ;
extern void xmlInt(char const* pAttribute, int value) ;
extern void xmlAddSimpleTag(char const* pTag) ;
extern void xmlAttValue(char const* pTag, char const* pAttribute, char const* pValue) ;
extern void xml_wme (agent* thisAgent, wme *w) ;

#endif
