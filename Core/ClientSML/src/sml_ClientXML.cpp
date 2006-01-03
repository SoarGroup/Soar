#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ClientXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
// This class is used to represent XML messages
// that are being returned to the client.
//
/////////////////////////////////////////////////////////////////
#include "sml_ClientXML.h"
#include "sml_ElementXML.h"

#include <assert.h>

using namespace sml ;

ClientXML::ClientXML()
{
	m_pElementXML = NULL ;
}

// This constructor is protected so that client doesn't try to build these objects.
// So far they're only built internally and then passed to the client.
ClientXML::ClientXML(ElementXML* pXML)
{
	m_pElementXML = pXML ;
}

ClientXML::~ClientXML()
{
	delete m_pElementXML ;
	m_pElementXML = NULL ;
}

/*************************************************************
* @brief Creates a new reference to the underlying XML object.
*
* This allows a caller to "keep a copy" of an object they are
* passed.
*************************************************************/
ClientXML::ClientXML(ClientXML* pXML)
{
	m_pElementXML = NULL ;

	if (pXML->m_pElementXML == NULL)
		return ;

	// Create a new ElementXML wrapper pointing to the same handle
	// as the object we are copying.  Then increase the ref count
	// on the underlying handle, ensuring it will only be deleted once
	// we and the original pXML object have been deleted.
	m_pElementXML = new ElementXML(pXML->m_pElementXML->GetXMLHandle()) ;
	m_pElementXML->AddRefOnHandle() ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* ClientXML::GetTagName() const
{
	return m_pElementXML->GetTagName() ;
}

/*************************************************************
* @brief Returns true if the tag name matches.
*
* @param pTagName	The tag to test against.
*
* @returns true if equal (case sensitive)
*************************************************************/
bool ClientXML::IsTag(char const* pTagName) const
{
	return m_pElementXML->IsTag(pTagName) ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int ClientXML::GetNumberChildren() const
{
	return m_pElementXML->GetNumberChildren() ;
}

/*************************************************************
* @brief Returns the n-th child of this element by placing it in pChild.
*
* Children are guaranteed to be returned in the order they were added.
*
* The caller must delete the child when it is finished with it.
*
* @param pChild	A ChildXML object into which the n-th child is placed.
* @param index	The 0-based index of the child to return.
* 
* @returns false if index is out of range.
*************************************************************/
bool ClientXML::GetChild(ClientXML* pChild, int index) const
{
	// If we're reusing a child object delete what it used to contain
	if (pChild->m_pElementXML != NULL)
	{
		delete pChild->m_pElementXML ;
		pChild->m_pElementXML = NULL ;
	}

	// Retrieve the child and attach it to the pChild object we were passed.
	ElementXML* pXML = new ElementXML() ;
	bool ok = m_pElementXML->GetChild(pXML, index) ;

	if (ok)
	{
		pChild->m_pElementXML = pXML ;
	}
	else
	{
		delete pXML ;
	}

	return ok ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.  
*************************************************************/
int ClientXML::GetNumberAttributes() const
{
	return m_pElementXML->GetNumberAttributes() ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*		 Attributes may not be returned in the order they were added.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* ClientXML::GetAttributeName(int index) const
{
	return m_pElementXML->GetAttributeName(index) ;
}

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* ClientXML::GetAttributeValue(int index) const
{
	return m_pElementXML->GetAttributeValue(index) ;
}

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName	The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* ClientXML::GetAttribute(const char* attName) const
{
	return m_pElementXML->GetAttribute(attName) ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns	Returns the character data for this element.  This can return null if the element has no character data.
*			The character data returned will not include any XML escape sequences (e.g. &lt;). 
*			It will include the original special characters (e.g. "<").
*************************************************************/
char const* ClientXML::GetCharacterData() const
{
	return m_pElementXML->GetCharacterData() ;
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*		 rather than a null-terminated character string.
*************************************************************/
bool ClientXML::IsCharacterDataBinary() const
{
	return m_pElementXML->IsCharacterDataBinary() ;
}

/*************************************************************
* @brief Converts a character data buffer into binary data.
*
* If binary data is stored in an XML file it will encoded in
* some manner (e.g. as a string of hex digits).
* When read back in, we may need to indicate that this particular
* set of character data is encoded binary (converting it back from hex to binary).
*
* I hope we can do this during the parsing phase, but that may not
* always be possible, so this allows us to do so manually.
* The downside of the manual method is that you need to know which
* character data will be encoded as binary (but that should be OK based on the tag names).
*
* Calling this function on a buffer that has already been decoded has no effect.
*
* @returns True if buffer is binary after conversion.
*************************************************************/
bool ClientXML::ConvertCharacterDataToBinary()
{
	return m_pElementXML->ConvertCharacterDataToBinary() ;
}

/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
int	 ClientXML::GetCharacterDataLength() const
{
	return m_pElementXML->GetCharacterDataLength() ;
}

/*************************************************************
* @brief Converts the XML object to a string.
*
* @param includeChildren	Includes all children in the XML output.
*
* @returns The string form of the object.
*************************************************************/
char* ClientXML::GenerateXMLString(bool includeChildren) const
{
	return m_pElementXML->GenerateXMLString(includeChildren) ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
*	*
* @param includeChildren	Includes all children in the XML output.
*************************************************************/
int ClientXML::DetermineXMLStringLength(bool includeChildren) const
{
	return m_pElementXML->DetermineXMLStringLength(includeChildren) ;
}

/*************************************************************
* @brief Utility function to allocate memory that the client will pass to the other ElementXML functions.
*
* @param length		The length is the number of characters in the string, so length+1 bytes will be allocated
*					(so that a trailing null is always included).  Thus passing length 0 is valid and will allocate a single byte.
*************************************************************/
char* ClientXML::AllocateString(int length)
{
	return ElementXML::AllocateString(length) ;
}

/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string		The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
void ClientXML::DeleteString(char* pString)
{
	ElementXML::DeleteString(pString) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string		The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
char* ClientXML::CopyString(char const* original)
{
	return ElementXML::CopyString(original) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* ClientXML::CopyBuffer(char const* original, int length)
{
	return ElementXML::CopyBuffer(original, length) ;
}
