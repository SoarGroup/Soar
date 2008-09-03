#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ClientAnalyzedXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2005
//
// Represents an XML message that has been analyzed to identify
// where parameters and other important values occur within the message.
// This allows an efficient "random access" to the values within the message
// and provides some level of abstraction over exactly how the message is structured.
//
// It's a subset of the functionality offered by AnalyzeXML.
//
/////////////////////////////////////////////////////////////////
#include "sml_ClientAnalyzedXML.h"
#include "sml_AnalyzeXML.h"

#include <assert.h>

using namespace sml ;

ClientAnalyzedXML::ClientAnalyzedXML()
{
	m_pAnalyzeXML = new AnalyzeXML() ;
}

ClientAnalyzedXML::~ClientAnalyzedXML()
{
	delete m_pAnalyzeXML ;
}

void ClientAnalyzedXML::Attach(AnalyzeXML* pAnalyzeXML)
{
	delete m_pAnalyzeXML ;
	m_pAnalyzeXML = pAnalyzeXML ;
}

// Each of these either returns a reference to the tag or NULL (if this document doesn't contain that tag)
ElementXML const* ClientAnalyzedXML::GetCommandTag() const			{ return m_pAnalyzeXML->GetCommandTag() ; }
ElementXML const* ClientAnalyzedXML::GetResultTag() const			{ return m_pAnalyzeXML->GetResultTag() ; }
ElementXML const* ClientAnalyzedXML::GetErrorTag()	const			{ return m_pAnalyzeXML->GetErrorTag() ; }

bool ClientAnalyzedXML::IsSML() const { return m_pAnalyzeXML->IsSML() ; }

// Returns the name of the command (or NULL if no command tag or no name in that tag)
char const* ClientAnalyzedXML::GetCommandName() const
{
	return m_pAnalyzeXML->GetCommandName() ;
}

// Returns the character data from the result tag (or NULL if no result tag or no character data in that tag)
char const* ClientAnalyzedXML::GetResultString() const
{
	return m_pAnalyzeXML->GetResultString() ;
}

// Returns the character data from the result tag as an int (or default value if there is no character data in the tag)
int ClientAnalyzedXML::GetResultInt(int defaultValue) const
{
	return m_pAnalyzeXML->GetResultInt(defaultValue) ;
}

// Returns the result as a bool
bool ClientAnalyzedXML::GetResultBool(bool defaultValue) const
{
	return m_pAnalyzeXML->GetResultBool(defaultValue) ;
}

// Returns the result as a double
double ClientAnalyzedXML::GetResultFloat(double defaultValue) const
{
	return m_pAnalyzeXML->GetResultFloat(defaultValue) ;
}

// Returns the string form of the XML.  Must be released with the static DeleteString method
char* ClientAnalyzedXML::GenerateXMLString(bool includeChildren) const
{
	return m_pAnalyzeXML->GenerateXMLString(includeChildren) ;
}

void ClientAnalyzedXML::DeleteString(char* pString)
{
	return AnalyzeXML::DeleteString(pString) ;
}

/*************************************************************
* @brief Look up an argument by name.  Returns NULL if not found.
*************************************************************/
char const* ClientAnalyzedXML::GetArgString(char const* pArgName) const
{
	return m_pAnalyzeXML->GetArgString(pArgName) ;
}

/*************************************************************
* @brief As "GetArgString" but parsed as a boolean.
*************************************************************/
bool ClientAnalyzedXML::GetArgBool(char const* pArgName, bool defaultValue) const
{
	return m_pAnalyzeXML->GetArgBool(pArgName, defaultValue) ;
}

/*************************************************************
* @brief As "GetArgString" but parsed as an int.
*************************************************************/
int ClientAnalyzedXML::GetArgInt(char const* pArgName, int defaultValue) const
{
	return m_pAnalyzeXML->GetArgInt(pArgName, defaultValue) ;
}

/*************************************************************
* @brief As "GetArgString" but parsed as an float (double).
*************************************************************/
double ClientAnalyzedXML::GetArgFloat(char const* pArgName, double defaultValue) const
{
	return m_pAnalyzeXML->GetArgFloat(pArgName, defaultValue) ;
}
