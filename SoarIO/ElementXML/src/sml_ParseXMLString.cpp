/////////////////////////////////////////////////////////////////
// ParseXMLString class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class is used to parse an XML document from a file/string and
// create an ElementXML object that represents it.
//
// This class reads from a string.
//
/////////////////////////////////////////////////////////////////

#include "sml_ParseXMLString.h"
#include "sml_ElementXMLImpl.h"

using namespace sml ;

ParseXMLString::ParseXMLString(char const* pInputLine)
{
	m_pInputLine = pInputLine ;
	m_Pos = 0 ;
	m_LineLength = strlen(m_pInputLine) ;

	InitializeLexer() ;
}

ParseXMLString::~ParseXMLString(void)
{
}

void ParseXMLString::ReadLine()
{
	if (!m_pInputLine)
	{
		RecordError("Invalid input string") ;
		return ;
	}

	// We only have one input string, so just check for EOF in
	// calls to read more lines.
	if (m_Pos >= m_LineLength)
	{
		m_IsEOF = true ;
		return ;
	}
}

/************************************************************************
* 
* Read the next character from the input file.
*
*************************************************************************/
void ParseXMLString::GetNextChar()
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

