#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

ParseXMLString::ParseXMLString(char const* pInputLine, size_t startPos)
{
	m_pInputLine = pInputLine ;
	m_Pos = startPos ;
	m_StartTokenPos = m_Pos ;
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
