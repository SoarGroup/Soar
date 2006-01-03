#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ParseXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class is used to parse an XML document from a file/string and
// create an ElementXMLImpl object that represents it.
//
// This class can't parse all XML, just the subset ElementXMLImpl supports.
// Things we don't support include processing instructions, comments and DTDs.
//
// Also, this class looks for the special attribute "bin_encoding=hex".
// This attribute is used to indicate when a block of character data is hex encoded binary.
// If the attribute is found, the character data is automatically decoded back to binary.
// There is no requirement to use this attribute, but having it allows the parser of the
// document to decode binary data without having to know in advance which blocks are encoded binary.
// (If you don't use this attribute, then the character data will still be in its encoded form
//  and the receiver of the document must decode it manually by calling ConvertBinaryDataToCharacters()).
//
/////////////////////////////////////////////////////////////////

#include "sml_ParseXML.h"
#include "sml_ElementXMLImpl.h"

using namespace sml ;

// We use a special attribute "bin_encoding='hex'" to indicate
// when binary data is stored in the character data stream.
// Using this is not necessary, but without it the user of
// this XML document needs to know which parts are binary and which not
// so it can decode them correctly.
static char const* kEncodingAtt = "bin_encoding" ;
//static char const* kEncodingVal = "hex" ;

ParseXML::ParseXML(void)
{
	m_Error = false ;
	m_ErrorMsg = "" ;
	m_TokenType = kSymbol ;
	m_IsEOF = false ;
	m_InCharData = false ;
}

ParseXML::~ParseXML(void)
{
}

void ParseXML::InitializeLexer()
{
	// Prime the lexical analyser with initial values
	ReadLine() ;

	// Read the first token
	GetNextToken() ;
}		

char ParseXML::GetEscapeChar()
{
	std::string escape ;

	while (!IsEOF() && GetCurrentChar() != ';')
	{
		escape += GetCurrentChar() ;
		GetNextChar() ;
	}

	// We don't consume the trailing ';' because this
	// stands in place of the character we are returning and will
	// be consumed by the caller.

	if (IsEOF())
	{
		RecordError("Error trying to parse an escape sequence.  Started with '&' but had no ending ';'") ;
		return ' ' ;
	}

	// We don't bother to add the trailing ';' to the string, so min length is 3.
	if (escape.size() < 3)
	{
		RecordError("Found an unknown escape sequence: " + escape) ;
		return ' ' ;
	}

	/*
	static char const* kLT   = "&lt;" ;
	static char const* kGT   = "&gt;" ;
	static char const* kAMP  = "&amp;" ;
	static char const* kQUOT = "&quot;" ;
	static char const* kAPOS = "&apos;" ;
	*/

	// The first char is enough to distinguish most cases
	char ch = escape[1] ;

	switch (ch)
	{
	case 'l': return '<' ;
	case 'g': return '>' ;
	case 'q': return '"' ;
	case 'a': return (escape[2] == 'm') ? '&' : '\'' ;
	}

	RecordError("Found an unknown escape sequence: " + escape) ;
	return ' ' ;
}

/************************************************************************
* 
* This is the main lexical analyser functions--gets the next
* token from the input stream (e.g. an identifier, or a symbol etc.)
* 
*************************************************************************/
void ParseXML::GetNextToken()
{
	// Used to store current char
	char ch = 0 ;

	// If we're already reported an error, stop parsing.
	if (IsError())
		return ;

	// Record where this token starts (matters when we're doing a sequence of XML docs)
	StartingNewToken() ;

	// If we're already at EOF when we ask
	// for the next token, that's an error.
	if (IsEOF() && GetTokenType() == kEOF)
	{
		RecordError("Unexpected end of file when parsing file") ;
		return ;
	}
	
	// Skip leading white space
	while (!IsEOF() && !m_InCharData && IsWhiteSpace(GetCurrentChar()))
		GetNextChar() ;
	
	// If we're at the end of file, report that as the next token.
	if (IsEOF())
	{
		SetCurrentToken("", kEOF) ;
		return ;
	}
	
	// Symbol token
	if (IsSymbol(GetCurrentChar()))
	{
		char currentChar = GetCurrentChar() ;
		SetCurrentToken(currentChar, kSymbol) ;
		
		// Consume the symbol.
		GetNextChar() ;

		// We handle </ as a single symbol to make the parsing easier
		if (!IsEOF() && currentChar == kOpenTagChar && GetCurrentChar() == kEndMarkerChar)
		{
			SetCurrentToken(kEndMarkerString, kSymbol) ;
			GetNextChar() ;
		}
		
		return ;
	}
	
	// We need to know whether we're parsing an identifier or character data
	// to know whether to stop for white space etc. or include that in the string.
	if (m_InCharData)
	{
		// Char data
		std::string data ;
		ch = GetCurrentChar() ;
		while (!IsEOF() && !IsEndOfCharData(ch) )
		{
			if (ch == kEscapeChar)
			{
				ch = GetEscapeChar() ;
			}

			data += ch;
			GetNextChar() ;

			ch = GetCurrentChar() ;
		}
		
		SetCurrentToken(data, kCharData) ;
		return ;
	}

	// Quoted string
	if (IsQuote(GetCurrentChar()))
	{
		// Buffer we'll use to build up the string
		std::string quoted ;
		
		// Consume the opening quote.
		GetNextChar() ;
		
		ch = GetCurrentChar() ;

		while (!IsEOF() && !IsQuote(ch))
		{
			if (ch == kEscapeChar)
			{
				ch = GetEscapeChar() ;
			}

			// Add everything up to the next quote.
			quoted += ch ;
			GetNextChar() ;

			ch = GetCurrentChar() ;
		}
		
		// Consume the closing quote.
		GetNextChar() ;
		
		SetCurrentToken(quoted, kQuotedString) ;
		return ;
	}

	// Identifier
	std::string identifier ;

	ch = GetCurrentChar() ;
	while (!IsEOF() && !IsWhiteSpace(ch) && !IsSymbol(ch) && !IsQuote(ch))
	{
		identifier += ch ;
		GetNextChar() ;

		ch = GetCurrentChar() ;
	}
	
	SetCurrentToken(identifier, kIdentifier) ;
}

/************************************************************************
* 
* Parse an XML element and its children.  This is the main parsing
* function.  The lexical analyser contains a references to the input
* stream being parsed.  The function calls itself recursively to
* parse its children.
* 
* @param lex		The lexical analyser for this parse.	
* 
* @return The element that has just been read from the input stream.
* 
*************************************************************************/
ElementXMLImpl* ParseXML::ParseElement()
{
	// We must start with an open tag marker
	MustBe(kOpenTagChar) ;

	// For now ignore header tags
	if (Have(kHeaderChar))
	{
		while (!IsError() && !Have(kCloseTagChar))
			GetNextToken() ;
		
		return NULL ;
	}
	
	// Create the object we'll be returning
	ElementXMLImpl* pElement = new ElementXMLImpl() ;

	// Get the tag name
	ParseString tagName ;
	MustBe(kIdentifier, tagName) ;

	pElement->SetTagName(tagName.c_str()) ;
	
	// Used to record whether data is encoded binary
	bool dataIsEncoded = false ;

	// Read all of the attributes (if there are any)
	while (!IsError() && Have(kIdentifier))
	{
		ParseString name = GetTokenValue() ;
		
		// Consume the name
		GetNextToken() ;
		
		// Must be followed by "="
		MustBe(kEqualsChar) ;
		
		// Then must be followed by a string containing the value
		ParseString value ;
		MustBe(kQuotedString, value) ;
		
		// Check for our special "encoding" attribute which is a flag to
		// say that the data is really encoded binary (not just a character string)
		if (name.compare(kEncodingAtt) == 0)
		{
			// If we support more than hex encoding later we should look at the
			// value and process it.
			dataIsEncoded = true ;

			// We don't show this attribute to the user, instead we decode the
			// character data as binary and they can call "IsDataBinary".
			continue ;
		}

		pElement->AddAttribute(name.c_str(), value.c_str()) ;
	}

	// We're about to enter character data section, so we need
	// to tell the lexer how to handle this before the call
	// to MustBe() happens (which calls GetNextToken()).
	// This flag determines whether we read to the next space or whether
	// we just read to the next special char (like "<").
	SetInCharData(true) ;

	// After reading all of the attributes, we must be at the end of the tag.
	MustBe(kCloseTagChar) ;
	
	bool endTag = false ;
	
	while (!IsError() && !endTag)
	{
		if (Have(kCharData))
		{
			// Character data
			ParseString contents = GetTokenValue() ;
			
			pElement->SetCharacterData((char*)contents.c_str(), true) ;

			// If this buffer is really binary data, convert it over.
			// Doing this just makes the user's life easier as they aren't
			// required to do this decoding manually.
			if (dataIsEncoded)
				pElement->ConvertCharacterDataToBinary() ;

			SetInCharData(false) ;	// Out of the char data now.
			GetNextToken() ;
			continue ;
		}
		
		// Check for the open tag, but don't consume it yet.
		// We consume this at the start of parsing the element.
		if (Have(kOpenTagChar, false))
		{
			SetInCharData(false) ;	// Out of the char data now.

			// Read the child element recursively.
			ElementXMLImpl* pChild = ParseElement() ;

			if (pChild)
				pElement->AddChild(pChild) ;

			continue ;
		}

		// See if we've reached the end of this element
		if (Have(kEndMarkerString, false))
		{
			SetInCharData(false) ;	// Out of the char data now.
			GetNextToken() ;		// Consume the end marker

			endTag = true ;
			continue ;
		}

		// Shouldn't reach here.
		RecordError("Looking for character data block, but failed to find it or tag marker") ;
	}

	if (!IsError())
	{
		// Once we reach the end marker "</" we just need to finish up the stream.
		MustBe(kIdentifier, tagName) ;
		
		if (tagName.compare(pElement->GetTagName()) != 0)
			RecordError("The closing tag for " + std::string(pElement->GetTagName()) + " doesn't match the opening tag") ;

		// Then we must have the close tag.
		SetInCharData(true) ;	// If we are inside a parent element the next string is character data
		MustBe(kCloseTagChar) ;
	}

	// If something went wrong return NULL.
	// The caller should then call in to get the error message.
	if (IsError())
	{
		// This deletes pElement
		pElement->ReleaseRef() ;
		return NULL ;
	}

	// And we're done with this element.
	return pElement ;
}
