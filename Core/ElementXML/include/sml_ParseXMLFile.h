/////////////////////////////////////////////////////////////////
// ParseXMLFile class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class is used to parse an XML document from a file/string and
// create an ElementXML object that represents it.
//
// This version reads from a file.
//
/////////////////////////////////////////////////////////////////

#ifndef PARSE_XML_FILE_H
#define PARSE_XML_FILE_H

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

class ParseXMLFile : public ParseXML
{
protected:
	enum { kBufferSize = 1024 } ;

protected:
	// The input file
	FILE*		m_pInputFile ;

	// The current string of characters that we're reading
	char	m_CurrentLine[kBufferSize] ;

	// Location in the current line buffer
	size_t		m_Pos ;

	// Number of chars in the current line buffer (when pos reaches this, read another line)
	size_t		m_LineLength ;

	// Set to true when we have read all there is in the file (but not parsed it yet).
	bool		m_ReachedEOF ;

	virtual void GetNextChar() ;
	virtual void ReadLine() ;

	/************************************************************************
	* 
	* Returns the current character from the input stream.
	* 
	*************************************************************************/
	virtual char GetCurrentChar()
	{
		return m_CurrentLine[m_Pos] ;
	}
	
	// To support reading a stream of XML documents from a single string/file
	// we need to mark when a new token is being read, because we end up reading
	// the first token from the next stream at the end of the current document and need
	// to be able to backup.  (This has no impact if we're just reading one XML document from a stream).
	virtual void		StartingNewToken()
	{
		// Not implementing anything on the file side yet for this.
		// Only add something here if we decide we want to read multiple docs from a file.
	}

public:
	ParseXMLFile(FILE* pInputFile);
	virtual ~ParseXMLFile(void);
};

}	// namespace

#endif // PARSE_XML_H
