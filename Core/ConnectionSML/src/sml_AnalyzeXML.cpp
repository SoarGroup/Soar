#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// AnalyzeXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// An SML document consists of a series of elements.  We want to be
// able to ask questions like "give me the <result> tag" element and
// either get it or get NULL to say it's not in the document.
//
// We could do that by searching the SML document to answer each
// request, but that's time consuming.  Instead, we'll run through the
// entire document once and create references to the appropriate pieces
// and then each request becomes a look up.
//
// This class represents those references into the document.
/////////////////////////////////////////////////////////////////

#include "sml_AnalyzeXML.h"
#include "sml_ElementXML.h"
#include "sml_Names.h"
#include "sml_StringOps.h"
#include "ElementXMLInterface.h"

using namespace sml ;

AnalyzeXML::AnalyzeXML()
{
	m_hRootObject = NULL ;
	m_pCommand	= NULL ;		// The Command tag (for calls)
	m_pResult	= NULL ;		// The Result tag (for responses)
	m_pError	= NULL ;		// The Error tag
	m_IsSML		= false ;
}

AnalyzeXML::~AnalyzeXML(void)
{
	// Release our reference to the original XML message
	// so it can be reclaimed
	if (m_hRootObject != NULL)
		::sml_ReleaseRef(m_hRootObject) ;

	delete m_pCommand ;
	delete m_pResult ;
	delete m_pError ;
}

// Returns the string form of the XML.  Must be released with the static DeleteString method
char* AnalyzeXML::GenerateXMLString(bool includeChildren, bool insertNewLines) const
{
	return ::sml_GenerateXMLString(m_hRootObject, includeChildren, insertNewLines) ;
}

void AnalyzeXML::DeleteString(char* pString)
{
	return ::sml_DeleteString(pString) ;
}

char const* AnalyzeXML::GetCommandName() const
{
	if (m_pCommand) return m_pCommand->GetAttribute(sml_Names::kCommandName) ;
	return NULL ;
}

char const* AnalyzeXML::GetResultString() const
{
	if (m_pResult) return m_pResult->GetCharacterData() ;
	return NULL ;
}

int AnalyzeXML::GetResultInt(int defaultValue) const
{
	if (!m_pResult || m_pResult->GetCharacterData() == NULL)
		return defaultValue ;

	// BADBAD: If char data is not valid, we'll get back "0" from atoi, when
	// we should really return defaultValue.  Perhaps we should use sscanf instead?
	int value = atoi(m_pResult->GetCharacterData()) ;

	return value ;
}

// Returns the result as a bool
bool AnalyzeXML::GetResultBool(bool defaultValue) const
{
	char const* pResult = GetResultString() ;

	if (pResult == NULL)
		return defaultValue ;

	// If the default is true, we only need to test for false explicitly.
	// Anything else is true and vice versa if the default is false.
	// (This just saves an unnecessary string comparison).
	if (defaultValue)
		return(!IsStringEqualIgnoreCase(pResult, sml_Names::kFalse)) ;
	else
		return(IsStringEqualIgnoreCase(pResult, sml_Names::kTrue)) ;
}

// Returns the result as a double
double AnalyzeXML::GetResultFloat(double defaultValue) const
{
	if (!m_pResult || m_pResult->GetCharacterData() == NULL)
		return defaultValue ;

	double value = atof(m_pResult->GetCharacterData()) ;

	return value ;
}

/*************************************************************
* @brief As "GetArgValue" but parsed as a boolean.
*************************************************************/
bool AnalyzeXML::GetArgBool(char const* pArgName, int argPos, bool defaultValue) const
{
	char const* pValue = m_ArgMap.GetArgValue(pArgName, argPos) ;

	if (!pValue)
		return defaultValue ;

	// If the default is true, we only need to test for false explicitly.
	// Anything else is true and vice versa if the default is false.
	// (This just saves an unnecessary string comparison).
	if (defaultValue)
		return(!IsStringEqualIgnoreCase(pValue, sml_Names::kFalse)) ;
	else
		return(IsStringEqualIgnoreCase(pValue, sml_Names::kTrue)) ;
}

/*************************************************************
* @brief As "GetArgValue" but parsed as an int.
*************************************************************/
int AnalyzeXML::GetArgInt(char const* pArgName, int argPos, int defaultValue) const
{
	char const* pValue = m_ArgMap.GetArgValue(pArgName, argPos) ;

	if (!pValue)
		return defaultValue ;

	// BADBAD: If pValue is not valid, we'll get back "0" from atoi, when
	// we should really return defaultValue.  Perhaps we should use sscanf instead?
	int value = atoi(pValue) ;

	return value ;
}

/*************************************************************
* @brief As "GetArgValue" but parsed as an double.
*************************************************************/
double AnalyzeXML::GetArgFloat(char const* pArgName, int argPos, double defaultValue) const
{
	char const* pValue = m_ArgMap.GetArgValue(pArgName, argPos) ;

	if (!pValue)
		return defaultValue ;

	double value = atof(pValue) ;

	return value ;
}


/*************************************************************
* @brief Make a sequential pass and find some of the interesting tags.
*
* Go through the XML document and see which tags we recognize.
* For those, we create references within this document.
* Tags we don't recognize we ignore, but the caller is still free
* to get them directly from the original document.  We're just
* speeding things up for the command tags.
*
* @param pRootXML	The SML document to analyze
*************************************************************/
void AnalyzeXML::Analyze(ElementXML const* pRootXML)
{
	// If we've already used this object to run an analysis before
	// we need to delete any existing references and start over
	// (this is not really recommended as creating an AnalyzeXML object is pretty quick and painless)
	if (m_hRootObject)
	{
		::sml_ReleaseRef(m_hRootObject) ;

		delete m_pCommand ;
		delete m_pResult ;
		delete m_pError ;

		m_pCommand = 0 ;
		m_pResult = 0 ;
		m_pError = 0 ;
		m_IsSML		= false ;
	}

	// We need to keep this handle around for the life
	// of the AnalyzeXML object.
	m_hRootObject = pRootXML->GetXMLHandle() ;
	::sml_AddRef(m_hRootObject) ;

	ElementXML child(NULL) ;
	ElementXML* pChild = &child ;

	ElementXML const* pXML = pRootXML ;

	// Find the SML tag first
	if (pXML->IsTag(sml_Names::kTagSML))
	{
		m_IsSML = true ;

		// Examine the children of the SML tag
		int nChildren = pXML->GetNumberChildren() ;
		for (int i = 0 ; i < nChildren ; i++)
		{
			// Get each child in turn
			pXML->GetChild(pChild, i) ;

			if (pChild->IsTag(sml_Names::kTagCommand))
			{
				// Record the command and then find its arguments
				this->m_pCommand = new ElementXML(pChild->Detach()) ;

				AnalyzeArgs(m_pCommand) ;
			}
			else if (pChild->IsTag(sml_Names::kTagError))
			{
				// Record the error tag
				this->m_pError = new ElementXML(pChild->Detach()) ;
			}
			else if (pChild->IsTag(sml_Names::kTagResult))
			{
				// Record the result tag
				this->m_pResult = new ElementXML(pChild->Detach()) ;

				// Also find any arguments for the results (just like the command)
				AnalyzeArgs(m_pResult) ;
			}
		}
	}
}

void AnalyzeXML::AnalyzeArgs(ElementXML const* pParent)
{
	ElementXML child(NULL) ;
	ElementXML* pChild = &child ;

	// Examine the children of the Command tag
	int nChildren = pParent->GetNumberChildren() ;
	for (int i = 0 ; i < nChildren ; i++)
	{
		// Get each child in turn
		pParent->GetChild(pChild, i) ;

		if (pChild->IsTag(sml_Names::kTagArg))
		{
			// This child is an argument tag so record it
			// based on argument name and position.
			// We have to record the args in the order they exist in the
			// document for this to work.
			this->m_ArgMap.RecordArg(pChild->GetXMLHandle()) ;
		}
	}
		
}
