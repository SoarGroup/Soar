/////////////////////////////////////////////////////////////////
// XMLTrace class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : April 2005
//
// Represents a piece of structured trace information generated
// during the course of a run.
//
// This class is related to the ClientTraceXML class.  This class
// is optimized for creating trace objects while the client class
// supports reading the trace information.
//
// This XML structure is unusual because we want to add the same
// attributes to potentially a number of different tags.
// So we're looking for an API like this:
//
// beginTag(kWme);
// addAttribute(kTimeTag, mytimetag);
// addAttribute(kAtt, myatt);
// addAttribute(kVal, myval);
// endTag();
// 
// where the "addAttribute" methods are working with the "current" object.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_XML_TRACE_H
#define SML_XML_TRACE_H

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

namespace sml
{

class XMLTrace
{
private:
	// The root XML object
	ElementXML*	m_XML ;

	// The tag we're currently building
	ElementXML*	m_pCurrentTag ;

public:
	XMLTrace() ;

	// Alternative contstuctor where we specify the base tag name
	XMLTrace(char const* pTagName) ;

	virtual ~XMLTrace() ;

	/*************************************************************
	* @brief	Reinitialize this XML trace object so we can
	*			re-use it.
	*************************************************************/
	void Reset() ;

	/*************************************************************
	* @brief	Returns true if this tag contains no children
	*			(i.e. it has just been reset).
	*************************************************************/
	bool IsEmpty() ;

	/*************************************************************
	* @brief	Start a new tag.
	*
	* Subsequent calls to AddAttribute() will work with this tag
	* until EndTag() is called.
	*
	* NOTE: The tag name must remain in scope forever (i.e. it should be
	* a constant).  This allows us to save time by not copying the string.
	*************************************************************/
	void BeginTag(char const* pTagName) ;
	
	/*************************************************************
	* @brief	Terminate the current tag.
	*
	* The tag name is just used for error checking to make sure
	* everything is properly balanced.
	*************************************************************/
	void EndTag(char const* pTagName) ;

	/*************************************************************
	* @brief	Adds an attribute to the current tag.
	*
	* NOTE: The attribute name must remain in scope forever (i.e. it should be
	* a constant).  This allows us to save time by not copying the string.
	* Naturally, the value doesn't have this restriction.
	*************************************************************/
	void AddAttribute(char const* pAttributeName, char const* pValue) ;

	/*************************************************************
    * @brief Releases ownership of the underlying XML object.
	*
	*		 The caller must call releaseRef() on this handle
	*		 when it is done with it.
	*
	*		This should only be called when the message is actually
	*		being sent.  It shouldn't be used when building up the message.
	*
	*		NOTE: After doing this the XMLTrace object should either be
	*		deleted or Reset() should be called on it.
    *************************************************************/
	ElementXML_Handle Detach() ;

	/*************************************************************
	* @brief Releases ownership of the underlying XML object.
	*
	* As for Detach() but returns an object rather than a handle.
	*************************************************************/
	ElementXML* DetatchObject() ;
} ;

}

#endif	// SML_XML_TRACE_H
