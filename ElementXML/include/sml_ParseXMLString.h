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
#include "sml_ParseXML.h"

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

	// Location when we start the current token
	size_t		m_StartTokenPos ;

	/************************************************************************
	* 
	* Read the next character from the input string.
	*
	*************************************************************************/
	virtual inline void GetNextChar()
	{
		// When we're at the end of file, we're done.
		if (IsError() || IsEOF())
			return ;

		// Move the pointer along to the next char in the current input line
		m_Pos++ ;
		
		// If we moved off the end of the current line, move to the next input line.
		if (m_Pos >= m_LineLength)
			ReadLine() ;
	}

	virtual void ReadLine() ;

	/************************************************************************
	* 
	* Returns the current character from the input stream.
	* 
	*************************************************************************/
	virtual inline char GetCurrentChar()
	{
		return m_pInputLine[m_Pos] ;
	}
	
	// To support reading a stream of XML documents from a single string/file
	// we need to mark when a new token is being read, because we end up reading
	// the first token from the next stream at the end of the current document and need
	// to be able to backup.  (This has no impact if we're just reading one XML document from a stream).
	virtual void		StartingNewToken()
	{
		m_StartTokenPos = m_Pos ;
	}

public:
	ParseXMLString(char const* pInputLine, size_t startPos);
	size_t getEndPosition() { return m_StartTokenPos ; }
	virtual ~ParseXMLString(void);
};

}	// namespace

#endif // PARSE_XML_H
