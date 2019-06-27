#include "portability.h"

/////////////////////////////////////////////////////////////////
// ParseJSONFile class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class is used to parse an JSON document from a file/string and
// create an ElementJSON object that represents it.
//
// This class reads from a file.
//
/////////////////////////////////////////////////////////////////

#include "ParseJSONFile.h"
#include "ElementJSONImpl.h"

#include <cstdio>

using namespace soarjson;

ParseJSONFile::ParseJSONFile(FILE* pInputFile)
{
    m_pInputFile = pInputFile ;
    m_ReachedEOF = false ;
    m_IsEOF      = false ;
    m_Pos        = 0 ;
    m_LineLength = 0 ;
    
    InitializeLexer() ;
}

ParseJSONFile::~ParseJSONFile(void)
{
}

void ParseJSONFile::ReadLine()
{
    if (!m_pInputFile)
    {
        RecordError("Invalid file") ;
        return ;
    }
    
    // We only set the EOF flag to true once we have read everything in the file
    if (m_ReachedEOF)
    {
        m_IsEOF = true ;
        return ;
    }
    
    // Read the next line
    m_LineLength = fread(m_CurrentLine, sizeof(char), kBufferSize, m_pInputFile) ;
    
    m_Pos = 0 ;
    
    // Check to see if we've read everything from the file.
    // Don't set m_IsEOF yet because we've not parsed everything in the file.
    if (feof(m_pInputFile))
    {
        m_ReachedEOF = true ;
    }
}

/************************************************************************
*
* Read the next character from the input file.
*
*************************************************************************/
void ParseJSONFile::GetNextChar()
{
    // When we're at the end of file, we're done.
    if (IsError() || IsEOF())
    {
        return ;
    }
    
    // Move the pointer along to the next char in the current input line
    m_Pos++ ;
    
    // If we moved off the end of the current line, move to the next input line.
    if (m_Pos >= m_LineLength)
    {
        ReadLine() ;
    }
}
