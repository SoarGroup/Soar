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

#ifndef PARSE_XML_H
#define PARSE_XML_H

#include <string>
#include <stdio.h>

using namespace std ;

namespace sml {

// I think we'll want to implement our own parseString class
// which is (a) using allocateString() calls (so we can just pass off the results)
// and (b) is optimized for appending (so we keep a pointer to the end of the string)
// and (c) optimized to likely lengths we'll need (e.g. start with 32 or 64 chars for identifiers and values)
// Let's start with std::string until we know what capabilities we need.
typedef std::string	ParseString ;

class ElementXMLImpl ;

#define kEndMarkerString "</"

class ParseXML
{
protected:
	enum TokenType { kSymbol, kIdentifier, kQuotedString, kCharData, kEOF } ;
	enum { kOpenTagChar = '<', kCloseTagChar = '>', kEndMarkerChar = '/', kEqualsChar = '=', kHeaderChar = '?', kEscapeChar = '&' } ;

protected:
	// Used to report an error.  We only report the first one we find and then we abort the parse.
	bool		m_Error ;
	std::string	m_ErrorMsg ;

	// The current token is the symbol/string/identifier we're currently looking at
	ParseString m_TokenValue ;
	TokenType	m_TokenType ;
	bool		m_InCharData ;	// True when reading character data

	// Set to true when we go to read the next line after the last line in the file.
	// (So it's not true when we're reading the last, incomplete line).
	bool		m_IsEOF ;

	// Functions for operating on the current token
	TokenType	GetTokenType()					{ return m_TokenType ; }
	void		SetTokenType(TokenType type)	{ m_TokenType = type ; }

	// BADBAD: We need to think more about how to return this value.
	ParseString&GetTokenValue()					{ return m_TokenValue ; }

	void		SetTokenValue(ParseString val)	{ m_TokenValue = val ; }
	void		SetCurrentToken(ParseString val, TokenType type) { m_TokenValue = val ; m_TokenType = type ; }
	void		SetCurrentToken(char val, TokenType type)		 { m_TokenValue = val ; m_TokenType = type ; }
	void		SetInCharData(bool state)		{ m_InCharData = state ; }

	void		RecordError(std::string pStr)	{ if (!m_Error)
													{ m_ErrorMsg = pStr ; m_Error = true ; } }
	inline bool	IsError()						{ return m_Error ; }
	char		GetEscapeChar() ;

	/************************************************************************
	* 
	* Returns the current character from the input stream.
	* 
	*************************************************************************/
	virtual char GetCurrentChar() = 0 ;
	
	virtual void GetNextChar() = 0 ;
	virtual void ReadLine() = 0 ;

	/************************************************************************
	* 
	* Returns true if we're at the end of the file.
	* 
	*************************************************************************/
	inline bool IsEOF()
	{
		return m_IsEOF ;
	}

	/************************************************************************
	* 
	* Returns true if the character is white space.
	* 
	*************************************************************************/
	bool IsWhiteSpace(char ch)
	{
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') ;
	}
	
	/************************************************************************
	* 
	* Returns true if this is a symbol (i.e. a special char in XML, like "<")
	* 
	*************************************************************************/
	bool IsSymbol(char ch)
	{
		return (ch == kOpenTagChar || ch == kCloseTagChar ||
				ch == kHeaderChar || ch == kEqualsChar) ;
	}

	/************************************************************************
	* 
	* Returns true if this character ends a block of character data.
	* 
	*************************************************************************/
	bool IsEndOfCharData(char ch)
	{
		return (ch == kOpenTagChar) ;
	}

	/************************************************************************
	* 
	* Returns true if this is the quote character (")
	* 
	*************************************************************************/
	bool IsQuote(char ch)
	{
		return (ch == '\"') ;
	}

	/************************************************************************
	* 
	* Returns true if the current token matches the given type.
	* E.g. if (Have(kSymbol)) { // Process symbol }
	* 
	* @param type			The type to test against.
	* 
	*************************************************************************/
	bool Have(TokenType type)
	{
		return (GetTokenType() == type) ;
	}
		
	/************************************************************************
	* 
	* Returns true AND consumes the current token, if the values match.
	* We can consume the token, because the parser already knows its value.
	* E.g. if (Have("<")) { // Parse what comes after "<" }
	* 
	* @param value			The string value to test
	* 
	* @return True if value matches current token.
	* 
	*************************************************************************/
	bool Have(char const* pValue, bool advance = true)
	{
		if (m_TokenValue.compare(pValue) == 0)
		{
			if (advance)
				GetNextToken() ;

			return true ;
		}
		return false ;
	}

	// Same as above, but for a single char
	bool Have(char value, bool advance = true)
	{
		if (m_TokenValue.size() == 1 && m_TokenValue[0] == value)
		{
			if (advance)
				GetNextToken() ;

			return true ;
		}
		return false ;
	}
	
	/************************************************************************
	* 
	* Returns true AND consumes the current token if the value and type match.
	* 
	* @param value			The value to test
	* @param type			The type to test
	* 
	*************************************************************************/
	bool Have(char const* pValue, TokenType type)
	{
		if (GetTokenType() == type)
			return Have(pValue) ;
		return false ;
	}
	
	/************************************************************************
	* 
	* Checks that the current token matches the given value.
	* If not, throws an exception.
	* Used for places in the parse when you know what must come next.
	* E.g. At the end of an XML token : MustBe("/") ; MustBe(">") ;
	* 
	* @param value			The value to test
	* 
	*************************************************************************/
	void MustBe(char const* pValue)
	{
		if (!m_TokenValue.compare(pValue) == 0)
		{			
			RecordError("Looking for " + std::string(pValue) + " instead found " + GetTokenValue()) ;
		}
		
		GetNextToken() ;
	}

	// Same as above, but for a single character
	void MustBe(char value)
	{
		if (m_TokenValue.size() != 1 || m_TokenValue[0] != value)
		{
			std::string msg = "Looking for " ;
			msg += value ;
			msg += " instead found " ;
			msg += GetTokenValue() ;
			RecordError(msg) ;
		}
		
		GetNextToken() ;
	}

	/************************************************************************
	* 
	* Checks that the current token matches the given type.
	* If it does, returns the value (this is often useful when testing
	* for identifiers).
	* If it does not match, throws an exception.
	* 
	* @param type		The type to test (e.g. kSymbol)
	* @param value		Iss et to the value of the current token (if matches type).
	* 
	*************************************************************************/
	void MustBe(TokenType type, ParseString& value)
	{
		if (GetTokenType() != type)
		{
			RecordError("Found incorrect type when parsing token '" + GetTokenValue() + "'") ;
		}
		
		value = GetTokenValue() ;
		
		GetNextToken() ;
	}
	
	/************************************************************************
	* 
	* Checks that both the type and value match.
	* If not, throws an exception.
	* 
	*************************************************************************/
	void MustBe(char const* pValue, int type)
	{
		if (GetTokenType() != type)
		{
			RecordError("Found incorrect type when parsing token '" + GetTokenValue() + "'") ;
		}
		MustBe(pValue) ;
	}

	virtual void		GetNextToken() ;

	virtual void		InitializeLexer() ;

	// To support reading a stream of XML documents from a single string/file
	// we need to mark when a new token is being read, because we end up reading
	// the first token from the next stream at the end of the current document and need
	// to be able to backup.  (This has no impact if we're just reading one XML document from a stream).
	virtual void		StartingNewToken() = 0 ;

public:
	ParseXML(void);
	virtual ~ParseXML(void);

	ElementXMLImpl* ParseElement() ;
	std::string GetErrorMessage()				{ return m_ErrorMsg ; }
};

}	// namespace

#endif // PARSE_XML_H
