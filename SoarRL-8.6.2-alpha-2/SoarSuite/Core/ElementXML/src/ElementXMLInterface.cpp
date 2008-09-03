#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ElementXMLInterface file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This file offers a C level interface to the ElementXML code.
// The real functionality is within a C++ ElementXML class but it's easier to export
// a C level interface than a C++ one.
//
// This ElementXML library is responsible for representing an XML document as an object (actually a tree of objects).
//
// A client can send a stream of XML data which this class parses to create the object representation of the XML.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// XML output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of XML which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
/////////////////////////////////////////////////////////////////

#include "ElementXMLInterface.h"
#include "sml_ElementXMLImpl.h"
#include "sml_ParseXMLFile.h"
#include "sml_ParseXMLString.h"

#include <string>

// We store the last parsing error message here.
// This is a bit inelegant, but makes it easier for the client to use this
// interface from any language.
static std::string s_LastParseErrorMessage ;

using namespace sml ;

inline static ElementXMLImpl* GetElementFromHandle(ElementXML_Handle hXML)
{
	return (ElementXMLImpl*)hXML ;
}

////////////////////////////////////////////////////////////////
//
// Constructors and destructors
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Default constructor.
*************************************************************/
ElementXML_Handle sml_NewElementXML()
{
	return (ElementXML_Handle)new ElementXMLImpl() ;
}

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*
* @returns The new reference count (0 implies the object was deleted)
*************************************************************/
int sml_ReleaseRef(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->ReleaseRef() ;
}

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
int sml_GetRefCount(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetRefCount() ;
}

/*************************************************************
* @brief Add a new reference to this object.
*        The object will only be deleted after calling
*        releaseRef() one more time than addRef() has been
*        called.
*        A newly created object has a reference count of 1 automatically.
*
* @returns The new reference count (will be at least 2).
*************************************************************/
int sml_AddRef(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->AddRef() ;
}

////////////////////////////////////////////////////////////////
//
// Tag functions (e.g the tag in <name>...</name> is "name")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the tag name for this element.
*
* @param  tagName	Tag name can only contain letters, numbers, “.” “-“ and “_”.
* @param  copyName	If true, tagName will be copied.  If false, we take ownership of tagName.
* @returns	true if the tag name is valid.
*************************************************************/
bool sml_SetTagName(ElementXML_Handle hXML, char* tagName, bool copyName)
{
	return GetElementFromHandle(hXML)->SetTagName(tagName, copyName) ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* sml_GetTagName(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetTagName() ;
}

////////////////////////////////////////////////////////////////
//
// Comment functions (<!-- .... --> marks the bounds of a comment)
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Associate a comment with this XML element.
*		 The comment is written in front of the element when stored/parsed.
*
* This type of commenting isn't completely general.  You can't have multiple
* comment blocks before an XML element, nor can you have trailing comment blocks
* where there is no XML element following the comment.  However, both of these are
* unusual situations and would require a significantly more complex API to support
* so it seems unnecessary.
*
* @param Comment	The comment string.
*************************************************************/
bool sml_SetComment(ElementXML_Handle hXML, char const* pComment)
{
	return GetElementFromHandle(hXML)->SetComment(pComment) ;
}

/*************************************************************
* @brief Returns the comment for this element.
*
* @returns The comment string for this element (or NULL if there is none)
*************************************************************/
char const* sml_GetComment(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetComment() ;
}

////////////////////////////////////////////////////////////////
//
// Child element functions.
//
// These allow a single ElementXML object to represent a complete
// XML document through its children.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds a child to the list of children of this element.
*
* @param  pChild	The child to add.  Will be released when the parent is destroyed.
*************************************************************/
void sml_AddChild(ElementXML_Handle hXML, ElementXML_Handle hChild)
{
	return GetElementFromHandle(hXML)->AddChild(GetElementFromHandle(hChild)) ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int sml_GetNumberChildren(ElementXML_Handle const hXML)
{
	return GetElementFromHandle(hXML)->GetNumberChildren() ;
}

/*************************************************************
* @brief Returns the n-th child of this element.
*
* Children are guaranteed to be returned in the order they were added.
* If index is out of range returns NULL.
* The caller should *not* call releaseRef() on this child.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @param index	The 0-based index of the child to return.
*************************************************************/
ElementXML_Handle const sml_GetChild(ElementXML_Handle const hXML, int index)
{
	return (ElementXML_Handle)GetElementFromHandle(hXML)->GetChild(index) ;
}

/*************************************************************
* @brief Returns the parent of this element.
*
* The caller should *not* call releaseRef() on this parent.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @returns NULL if has no parent.
*************************************************************/
ElementXML_Handle const sml_GetParent(ElementXML_Handle const hXML)
{
	return (ElementXML_Handle)GetElementFromHandle(hXML)->GetParent() ;
}

/*************************************************************
* @brief Returns a copy of this object.
*		 Generally, this shouldn't be necessary as ref counting
*		 allows multiple clients to point to the same object.
*
*		 Call ReleaseRef() on the returned object when you are done with it.
*************************************************************/
ElementXML_Handle const sml_MakeCopy(ElementXML_Handle const hXML)
{
	return (ElementXML_Handle)GetElementFromHandle(hXML)->MakeCopy() ;
}

////////////////////////////////////////////////////////////////
//
// Attribute functions (e.g an attribute in <name first="doug">...</name> is first="doug")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* @param attributeName	Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @param  copyName		If true, atttributeName will be copied.  If false, we take ownership of attributeName
* @param  copyValue		If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool sml_AddAttribute(ElementXML_Handle hXML, char* attributeName, char* attributeValue, bool copyName, bool copyValue)
{
	return GetElementFromHandle(hXML)->AddAttribute(attributeName, attributeValue, copyName, copyValue) ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.  
*************************************************************/
int sml_GetNumberAttributes(ElementXML_Handle hXML) 
{
	return GetElementFromHandle(hXML)->GetNumberAttributes() ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*		 Attributes may not be returned in the order they were added.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* sml_GetAttributeName(ElementXML_Handle hXML, int index)
{
	return GetElementFromHandle(hXML)->GetAttributeName(index) ;
}

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* sml_GetAttributeValue(ElementXML_Handle hXML, int index)
{
	return GetElementFromHandle(hXML)->GetAttributeValue(index) ;
}

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName	The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* sml_GetAttribute(ElementXML_Handle hXML, const char* attName)
{
	return GetElementFromHandle(hXML)->GetAttribute(attName) ;
}

////////////////////////////////////////////////////////////////
//
// Character data functions (e.g the character data in <name>Albert Einstein</name> is "Albert Einstein")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the character data for this element.
*
* NOTE: The characterData will be deleted by this object when it is deleted.
* It should be allocated with either allocateString() or copyString().
*
* @param characterData	The character data passed in should *not* replace special characters such as “<” and “&”
*						with the XML escape sequences &lt; etc.
*						These values will be converted when the XML stream is created.
* @param  copyData		If true, characterData will be copied.  If false, we take ownership of characterData
*************************************************************/
void sml_SetCharacterData(ElementXML_Handle hXML, char* characterData, bool copyData)
{
	return GetElementFromHandle(hXML)->SetCharacterData(characterData, copyData) ;
}

/*************************************************************
* @brief Setting the chracter data in this way indicates that this element’s character data should be treated as a binary buffer
*		 (so it may contain chars from 0-255, not just ASCII characters).
*
* NOTE: The characterData will be deleted by this object when it is deleted.
* It should be allocated with either allocateString() or copyString().
* Be careful with the lengths -- allocateString(len) allocates len+1 bytes and the
* length of the entire buffer should be passed in here (i.e. len+1 in this example).
*
* @param characterData	The binary buffer (allocated with allocateString())
* @param length			The length of the buffer
* @param copyData		If true, characterData will be copied.  If false, we take ownership of characterData
*************************************************************/
void sml_SetBinaryCharacterData(ElementXML_Handle hXML, char* characterData, int length, bool copyData)
{
	return GetElementFromHandle(hXML)->SetBinaryCharacterData(characterData, length, copyData) ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns	Returns the character data for this element.  This can return null if the element has no character data.
*			The character data returned will not include any XML escape sequences (e.g. &lt;). 
*			It will include the original special characters (e.g. "<").
*************************************************************/
char const* sml_GetCharacterData(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetCharacterData() ;
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*		 rather than a null-terminated character string.
*************************************************************/
bool sml_IsCharacterDataBinary(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->IsCharacterDataBinary() ;
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
bool sml_ConvertCharacterDataToBinary(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->ConvertCharacterDataToBinary() ;
}


/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
int	 sml_GetCharacterDataLength(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetCharacterDataLength() ;
}

/*************************************************************
* @brief Setting this value to true indicates that this element’s character data should be stored in a CDATA section.
*		 By default this value will be false.
*
*		 This value is ignored if the character data is marked as binary data.
*
* @param useCData	true if this element’s character data should be stored in a CDATA section.
*************************************************************/
void sml_SetUseCData(ElementXML_Handle hXML, bool useCData)
{
	return GetElementFromHandle(hXML)->SetUseCData(useCData) ;
}

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to XML.
*************************************************************/
bool sml_GetUseCData(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetUseCData() ;
}

////////////////////////////////////////////////////////////////
//
// Generator
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Converts the XML object to a string.
*
* @param includeChildren	Includes all children in the XML output.
*
* @returns The string form of the object.  Caller must delete with DeleteString().
*************************************************************/
char* sml_GenerateXMLString(ElementXML_Handle const hXML, bool includeChildren)
{
	return GetElementFromHandle(hXML)->GenerateXMLString(includeChildren) ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
*	*
* @param includeChildren	Includes all children in the XML output.
*************************************************************/
int sml_DetermineXMLStringLength(ElementXML_Handle const hXML, bool includeChildren)
{
	return GetElementFromHandle(hXML)->DetermineXMLStringLength(includeChildren);
}

////////////////////////////////////////////////////////////////
//
// String and memory functions
//
// These operations allow a client to allocate memory that ElementXML will later release,
// or similarly, allow a client to release memory that ElementXML has allocated.
//
// We may decide that a particular allocator will be used to do this (e.g. new[] and delete[]),
// but in general it's safest to use these functions.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Utility function to allocate memory that the client will pass to the other ElementXML functions.
*
* @param length		The length is the number of characters in the string, so length+1 bytes will be allocated
*					(so that a trailing null is always included).  Thus passing length 0 is valid and will allocate a single byte.
*************************************************************/
char* sml_AllocateString(int length)
{
	return ElementXMLImpl::AllocateString(length) ;
}

/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string		The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
void sml_DeleteString(char* pString)
{
	ElementXMLImpl::DeleteString(pString) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string		The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
char* sml_CopyString(char const* original)
{
	return ElementXMLImpl::CopyString(original) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* sml_CopyBuffer(char const* original, int length)
{
	return ElementXMLImpl::CopyBuffer(original, length) ;
}

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name must remain in scope for the life of this object.
*		In practice, this generally means it must be a static constant.
*
* @param attributeName	Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @param  copyValue		If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool sml_AddAttributeFast(ElementXML_Handle hXML, char const* attributeName, char* attributeValue, bool copyValue)
{
	return GetElementFromHandle(hXML)->AddAttributeFast(attributeName, attributeValue, copyValue) ;
}

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name and value must remain in scope for the life of this object.
*		In practice, this generally means it must be a static constant.
*
* @param attributeName	Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool sml_AddAttributeFastFast(ElementXML_Handle hXML, char const* attributeName, char const* attributeValue)
{
	return GetElementFromHandle(hXML)->AddAttributeFastFast(attributeName, attributeValue) ;
}

/*************************************************************
* @brief Set the tag name for this element.
*
* NOTE: The caller must ensure that the tag name does not go out of scope
* before this object is destroyed.  This requirement means the tag name
* should generally be declared as a static constant.
*
* @param  tagName	Tag name can only contain letters, numbers, “.” “-“ and “_”.
* @returns	true if the tag name is valid.
*************************************************************/
bool sml_SetTagNameFast(ElementXML_Handle hXML, char const* tagName)
{
	return GetElementFromHandle(hXML)->SetTagNameFast(tagName) ;
}

////////////////////////////////////////////////////////////////
//
// Error reporting functions.
//
////////////////////////////////////////////////////////////////
int sml_GetLastError(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetLastError() ;
}

char const* sml_GetLastErrorDescription(ElementXML_Handle hXML)
{
	return GetElementFromHandle(hXML)->GetLastErrorDescription() ;
}

/*************************************************************
* @brief Parse an XML document from a (long) string and return an ElementXML object
*		 for the document.
*
* @param  pString	The XML document stored in a string.
* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
*************************************************************/
ElementXML_Handle sml_ParseXMLFromString(char const* pString)
{
	if (!pString)
		return NULL ;

	ParseXMLString parser(pString, 0) ;
	ElementXMLImpl* pXML = parser.ParseElement() ;

	if (!pXML)
		s_LastParseErrorMessage = parser.GetErrorMessage() ;

	return (ElementXML_Handle)pXML ;
}

/*************************************************************
* @brief Parse an XML document from a (long) string and return an ElementXML object
*		 for the document.  This version supports a sequence of XML strings which
*		 need to be parsed in order (rather than all being part of one document).
*
* @param  pString	The XML document stored in a string.
* @param  startPos  We'll start parsing the current XML document from this position (0 == beginning of the string)
* @param  endPos    This value is filled in at the end of the parse and indicates where the parse ended. (if endPos == strlen(pString) we're done)
* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
*************************************************************/
ElementXML_Handle sml_ParseXMLFromStringSequence(char const* pString, size_t startPos, size_t* endPos)
{
	if (!pString || !endPos)
		return NULL ;

	ParseXMLString parser(pString, startPos) ;

	ElementXMLImpl* pXML = parser.ParseElement() ;
	
	*endPos = parser.getEndPosition() ;

	if (!pXML)
		s_LastParseErrorMessage = parser.GetErrorMessage() ;

	return (ElementXML_Handle)pXML ;
}

/*************************************************************
* @brief Parse an XML document from a file and return an ElementXML object
*		 for the document.
*
* @param  pFilename	The file to load
* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
*************************************************************/
ElementXML_Handle sml_ParseXMLFromFile(char const* pFilename)
{
	if (!pFilename)
		return NULL ;

	FILE* pFile = fopen(pFilename, "rb") ;

	if (!pFile)
	{
		s_LastParseErrorMessage = "File " + std::string(pFilename) + " could not be opened" ;
		return NULL ;
	}

	ParseXMLFile parser(pFile) ;
	ElementXMLImpl* pXML = parser.ParseElement() ;

	fclose(pFile) ;

	if (!pXML)
		s_LastParseErrorMessage = parser.GetErrorMessage() ;

	return (ElementXML_Handle)pXML ;
}

/*************************************************************
* @brief Returns an error message describing reason for error in last parse.
*
* Call here if the parsing functions return NULL to find out what went wrong.
*************************************************************/
char const* sml_GetLastParseErrorDescription()
{
	return s_LastParseErrorMessage.c_str() ;
}

