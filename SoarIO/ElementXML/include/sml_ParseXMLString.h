/////////////////////////////////////////////////////////////////
// ParseXMLString class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class is used to parse an XML document from a file/string and
// create an ElementXML object that represents it.
//
// This version reads from a string.
//
/////////////////////////////////////////////////////////////////

#ifndef PARSE_XML_STRING_H
#define PARSE_XML_STRING_H

#include <string>
#include <stdio.h>
#include "sml_ParseXML.h"

using namespace std ;

namespace sml {

// I think we'll want to implement our own parseString class
// which is (a) using allocateString() calls (so we can just pass off the results)
// and (b) is optimized for appending (so we keep a pointer to the end of the string)
// and (c) optimized to likely lengths we'll need (e.g. start with 32 or 64 chars for identifiers and values)
// Let's start with std::string until we know what capabilities we need.
typedef std::string	ParseString ;

class ElementXML ;

class ParseXMLString : public ParseXML
{
protected:
	// The string we are reading.
	char const* m_pInputLine ;

	// Location in the current line buffer
	size_t		m_Pos ;

	// Number of chars in the current line buffer (when pos reaches this, read another line)
	size_t		m_LineLength ;

	virtual void GetNextChar() ;
	virtual void ReadLine() ;

	/************************************************************************
	* 
	* Returns the current character from the input stream.
	* 
	*************************************************************************/
	virtual char GetCurrentChar()
	{
		return m_pInputLine[m_Pos] ;
	}
		
public:
	ParseXMLString(char const* pInputLine);
	virtual ~ParseXMLString(void);
};

}	// namespace

#endif // PARSE_XML_H
