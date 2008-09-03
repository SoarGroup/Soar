#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// ElementXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// This library is responsible for representing an XML document as an object (actually a tree of objects).
//
// A client can send a stream of XML data which this class parses to create the object representation of the XML.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// XML output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of XML which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
//
// The implementation of this class is based around a handle (pointer) to an ElementXMLImpl object stored in the ElementXML DLL.
// Why do we have this abstraction rather than just storing the ElementXMLImpl object right here?
// The reason is that we're going to pass these objects between an executable (the client) and a DLL (the kernel).
// If both are compiled on the same day with the same compiler, there's no problem passing a pointer to an object between the two.
// However, if that's not the case (e.g. we want to use a new client with an existing Soar DLL) then we need a safe way to ensure that
// the class layout being pointed to in the executable is the same as that used in the DLL.  If we've used different versions of a compiler
// or changed the ElementXMLImpl class in any way, the class layout (where the fields are in memory) won't be the same and we'll get a crash
// as soon as the object is accessed on the other side of the EXE-DLL divide.
// 
// The solution to this is to add a separate DLL (ElementXML DLL) and only pass around pointers to objects which it owns.
// That way, there is no way for the two sides of the equation to get "out of synch" because there's only one implementation of
// ElementXMLImpl in the system and it's inside ElementXML DLL.  If we change the class, we update the DLL and neither the client
// nor the Soar Kernel DLL has any idea that a change has occured as neither can directly access the data within an ElementXMLImpl object.
// 
// We wouldn't normally make all of this effort unless we thought there's a real chance this sort of problem will arise and in the case
// of the Soar kernel it's quite possible (even likely) as systems are often tied to specific versions of Soar and so old libraries and
// clients are used.  Building in a requirement that both need to be compiled at the same time means we're building in requirements that
// the source is available for both client and kernel--which may not always be the case if you consider commercial applications of either side.
//
/////////////////////////////////////////////////////////////////

#include "sml_ElementXML.h"
#include "sml_StringOps.h"
#include "ElementXMLInterface.h"

#include <assert.h>

using namespace sml ;

////////////////////////////////////////////////////////////////
//
// Static (class level) functions
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief XML ids can only contain letters, numbers, “.” “-“ and “_”.
*************************************************************/
bool ElementXML::IsValidID(char const* str)
{
	for (char const* p = str ; *p != NUL ; p++)
	{
		if (!(*p >= '0' && *p <= '9' || *p >= 'a' && *p <= 'z' || *p >= 'A' && *p <= 'Z' || *p == '.' || *p == '-' || *p == '_'))
			return false ;
	}

	return true ;
}

////////////////////////////////////////////////////////////////
//
// Constructors and destructors
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Default constructor.
*************************************************************/
ElementXML::ElementXML()
{
	m_hXML = ::sml_NewElementXML() ;
}

/*************************************************************
* @brief Construct an object to manage an existing handle.
* Takes ownership of this handle.
*************************************************************/
ElementXML::ElementXML(ElementXML_Handle hXML)
{
	m_hXML = hXML ;
}

/*************************************************************
* @brief Destructor.
*************************************************************/
ElementXML::~ElementXML(void)
{
	if (m_hXML)
	{
		int refCount = ::sml_ReleaseRef(m_hXML) ;

		// This code should be unnecessary but harmless and it makes a useful place
		// to put a break point if there are ref-counting problems with an object.
		if (refCount == 0)
		{
			m_hXML = NULL ;
		}
	}
}

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*************************************************************/
int ElementXML::ReleaseRefOnHandle()
{
	int refCount = ::sml_ReleaseRef(m_hXML) ;

	// We use a ReleaseRef() model on the ElementXML object
	// to delete it.
	if (refCount == 0)
	{
		m_hXML = NULL ;
	}

	return refCount ;
}

/*************************************************************
* @brief Add a new reference to this object.
*        The object will only be deleted after calling
*        releaseRef() one more time than addRef() has been
*        called.
*************************************************************/
int ElementXML::AddRefOnHandle()
{
	return ::sml_AddRef(m_hXML) ;
}

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
int ElementXML::GetRefCount()
{
	return ::sml_GetRefCount(m_hXML) ;
}

////////////////////////////////////////////////////////////////
//
// Ownership functions
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Takes ownership of an existing handle.
*
*		 This does not affect the referencing counting of the
*		 handle being attached.  If this object is already
*		 managing a handle releaseRef() will be called on it.
*
* @param hXML	The handle to an existing ElementXML object
*************************************************************/
void ElementXML::Attach(ElementXML_Handle hXML)
{
	if (m_hXML)
		::sml_ReleaseRef(m_hXML) ;

	m_hXML = hXML ;
}

/*************************************************************
* @brief Releases ownership of the handle.
*
*		 The caller must call releaseRef() on this handle
*		 when it is done with it.
*************************************************************/
ElementXML_Handle ElementXML::Detach()
{
	ElementXML_Handle hXML = m_hXML ;

	m_hXML = NULL ;

	return hXML ;
}

/*************************************************************
* @brief Provides access to the handle this object is managing.
*
*		 This object retains ownership of the handle and the
*		 caller to this function should not call releaseRef() on it.
*		 (If the caller wishes to keep a reference to this returned
*		  handle it should call addRef() on it).
*************************************************************/
ElementXML_Handle ElementXML::GetXMLHandle() const
{
	return m_hXML ;
}

////////////////////////////////////////////////////////////////
//
// Tag functions (e.g the tag in <name>...</name> is "name")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the tag name for this element.
*
* NOTE: The tagName becomes owned by this XML object and will be deleted
* when it is destroyed.  It should be allocated with either allocateString() or copyString().
*
* @param  tagName	Tag name can only contain letters, numbers, “.” “-“ and “_”.
* @returns	true if the tag name is valid.
*************************************************************/
bool ElementXML::SetTagName(char* tagName, bool copyName)
{
#ifdef DEBUG
	if (!IsValidID(tagName))
		return false ;
#endif

	return ::sml_SetTagName(m_hXML, tagName, copyName) ;
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
bool ElementXML::SetTagNameFast(char const* tagName)
{
#ifdef DEBUG
	if (!IsValidID(tagName))
		return false ;
#endif

	return ::sml_SetTagNameFast(m_hXML, tagName) ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* ElementXML::GetTagName() const
{
	return ::sml_GetTagName(m_hXML) ;
}

/*************************************************************
* @brief Returns true if the tag name matches.
*
* @param pTagName	The tag to test against.
*
* @returns true if equal (case sensitive)
*************************************************************/
bool ElementXML::IsTag(char const* pTagName) const
{
	if (!m_hXML)
		return false ;

	char const* pThisTag = GetTagName() ;

	if (!pThisTag)
		return false ;

	// In some cases we'll use the same exact pointer
	// and this is common enough to include for performance
	if (pThisTag == pTagName)
		return true ;

	return (IsStringEqual(pThisTag, pTagName)) ;
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
bool ElementXML::SetComment(char const* pComment)
{
	return ::sml_SetComment(m_hXML, pComment) ;
}

/*************************************************************
* @brief Returns the comment for this element.
*
* @returns The comment string for this element (or NULL if there is none)
*************************************************************/
char const* ElementXML::GetComment() const
{
	return ::sml_GetComment(m_hXML) ;
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
* NOTE: The child object is deleted immediately in this version
* although the handle it manages is added to the XML tree and
* will only be released when the parent is destroyed.
*
* @param  pChild	The child to add.  Will be released when the parent is destroyed.
*************************************************************/
ElementXML_Handle ElementXML::AddChild(ElementXML* pChild)
{
	ElementXML_Handle hChild = pChild->Detach() ;
	delete pChild ;
	
	::sml_AddChild(m_hXML, hChild) ;
	return hChild ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int ElementXML::GetNumberChildren() const
{
	return ::sml_GetNumberChildren(m_hXML) ;
}

/*************************************************************
* @brief Returns the n-th child of this element by placing it in pChild.
*
* Children are guaranteed to be returned in the order they were added.
*
* The caller must delete the child when it is finished with it.
*
* @param pChild	An ElementXML object into which the n-th child is placed.
* @param index	The 0-based index of the child to return.
* 
* @returns false if index is out of range.
*************************************************************/
bool ElementXML::GetChild(ElementXML* pChild, int index) const
{
	ElementXML_Handle hChild = ::sml_GetChild(m_hXML, index) ;

	if (!hChild)
		return false ;

	pChild->Attach(hChild) ;
	pChild->AddRefOnHandle() ;

	return true ;
}

/*************************************************************
* @brief Returns the parent of this element by placing it in pParent.
*
* The caller must delete the parent when it is finished with it.
*
* @returns false if has no parent.
*************************************************************/
bool ElementXML::GetParent(ElementXML* pParent) const
{
	ElementXML_Handle hParent = ::sml_GetParent(m_hXML) ;

	if (!hParent)
		return false ;

	pParent->Attach(hParent) ;
	pParent->AddRefOnHandle() ;

	return true ;
}

/*************************************************************
* @brief Returns a copy of this object.
*		 Generally, this shouldn't be necessary as ref counting
*		 allows multiple clients to point to the same object.
*
*		 Call delete on the returned object when you are done with it.
*************************************************************/
ElementXML* ElementXML::MakeCopy() const
{
	ElementXML_Handle hCopy = ::sml_MakeCopy(m_hXML) ;

	return new ElementXML(hCopy) ;
}

////////////////////////////////////////////////////////////////
//
// Attribute functions (e.g an attribute in <name first="doug">...</name> is first="doug")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds an attribute name-value pair.  
*
* NOTE: The caller must ensure that the attribute name does not go out of scope
* before this object is destroyed.  This requirement means the attribute name
* should generally be declared as a static constant.
*
* The attribute value will be copied and can be deleted by the caller immediately.
*
* @param attributeName	Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool ElementXML::AddAttribute(char* attributeName, char* attributeValue, bool copyName, bool copyValue)
{
	return ::sml_AddAttribute(m_hXML, attributeName, attributeValue, copyName, copyValue) ;
}

/*************************************************************
* @brief Adds an attribute name-value pair.  
*
* NOTE: The caller must ensure that the attribute name does not go out of scope
* before this object is destroyed.  This requirement means the attribute name
* should generally be declared as a static constant.
*
* The attribute value will be copied and can be deleted by the caller immediately.
*
* @param attributeName	Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool ElementXML::AddAttributeFast(char const* attributeName, char* attributeValue, bool copyValue)
{
	return ::sml_AddAttributeFast(m_hXML, attributeName, attributeValue, copyValue) ;
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
bool ElementXML::AddAttributeFastFast(char const* attributeName, char const* attributeValue)
{
	return ::sml_AddAttributeFastFast(m_hXML, attributeName, attributeValue) ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.  
*************************************************************/
int ElementXML::GetNumberAttributes() const
{
	return ::sml_GetNumberAttributes(m_hXML) ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*		 Attributes may not be returned in the order they were added.
*
* @param index	The 0-based index of the attribute to return.
* @returns NULL if index is out of range.
*************************************************************/
const char* ElementXML::GetAttributeName(int index) const
{
	return ::sml_GetAttributeName(m_hXML, index) ;
}

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* ElementXML::GetAttributeValue(int index) const
{
	return ::sml_GetAttributeValue(m_hXML, index) ;
}

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName	The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* ElementXML::GetAttribute(const char* attName) const
{
	return ::sml_GetAttribute(m_hXML, attName) ;
}

////////////////////////////////////////////////////////////////
//
// Character data functions (e.g the character data in <name>Albert Einstein</name> is "Albert Einstein")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the character data for this element.
*
* @param characterData	The character data passed in should *not* replace special characters such as “<” and “&”
*						with the XML escape sequences &lt; etc.
*						These values will be converted when the XML stream is created.
*************************************************************/
void ElementXML::SetCharacterData(char* characterData, bool copyData)
{
	::sml_SetCharacterData(m_hXML, characterData, copyData) ;
}

void ElementXML::SetBinaryCharacterData(char* characterData, int length, bool copyData)
{
	::sml_SetBinaryCharacterData(m_hXML, characterData, length, copyData) ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns	Returns the character data for this element.  This can return null if the element has no character data.
*			The character data returned will not include any XML escape sequences (e.g. &lt;). 
*			It will include the original special characters (e.g. "<").
*************************************************************/
char const* ElementXML::GetCharacterData() const
{
	return ::sml_GetCharacterData(m_hXML) ;
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*		 rather than a null-terminated character string.
*************************************************************/
bool ElementXML::IsCharacterDataBinary() const
{
	return ::sml_IsCharacterDataBinary(m_hXML) ;
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
bool ElementXML::ConvertCharacterDataToBinary()
{
	return ::sml_ConvertCharacterDataToBinary(m_hXML) ;
}

/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
int	 ElementXML::GetCharacterDataLength() const
{
	return ::sml_GetCharacterDataLength(m_hXML) ;
}

/*************************************************************
* @brief Setting this value to true indicates that this element’s character data should be stored in a CDATA section.
*		 By default this value will be false.
*
* @param useCData	true if this element’s character data should be stored in a CDATA section.
*************************************************************/
void ElementXML::SetUseCData(bool useCData)
{
	::sml_SetUseCData(m_hXML, useCData) ;
}

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to XML.
*************************************************************/
bool ElementXML::GetUseCData() const
{
	return ::sml_GetUseCData(m_hXML) ;
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
* @param insertNewlines		Add newlines to space out the tags to be more human-readable
*
* @returns The string form of the object.  Caller must delete with DeleteString().
*************************************************************/
char* ElementXML::GenerateXMLString(bool includeChildren, bool insertNewLines) const
{
	return ::sml_GenerateXMLString(m_hXML, includeChildren, insertNewLines) ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
*	*
* @param includeChildren	Includes all children in the XML output.
* @param insertNewlines		Add newlines to space out the tags to be more human-readable
*************************************************************/
int ElementXML::DetermineXMLStringLength(bool includeChildren, bool insertNewLines) const
{
	return ::sml_DetermineXMLStringLength(m_hXML, includeChildren, insertNewLines) ;
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
char* ElementXML::AllocateString(int length)
{
	return ::sml_AllocateString(length) ;
}

/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string		The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
void ElementXML::DeleteString(char* pString)
{
	::sml_DeleteString(pString) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string		The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
char* ElementXML::CopyString(char const* original)
{
	return ::sml_CopyString(original) ;
}

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* ElementXML::CopyBuffer(char const* original, int length)
{
	return ::sml_CopyBuffer(original, length) ;
}

////////////////////////////////////////////////////////////////
//
// Parsing functions
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Parse an XML document from a (long) string and return an ElementXML object
*		 for the document.
*
* @param  pString	The XML document stored in a string.
* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
*************************************************************/
ElementXML* ElementXML::ParseXMLFromString(char const* pString)
{
	ElementXML_Handle hXML = ::sml_ParseXMLFromString(pString) ;

	if (!hXML)
		return NULL ;

	return new ElementXML(hXML) ;
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
ElementXML* ElementXML::ParseXMLFromStringSequence(char const* pString, size_t startPos, size_t* endPos)
{
	ElementXML_Handle hXML = ::sml_ParseXMLFromStringSequence(pString, startPos, endPos) ;

	if (!hXML)
		return NULL ;

	return new ElementXML(hXML) ;
}

/*************************************************************
* @brief Parse an XML document from a file and return an ElementXML object
*		 for the document.
*
* @param  pFilename	The file to load
* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
*************************************************************/
ElementXML* ElementXML::ParseXMLFromFile(char const* pFilename)
{
	ElementXML_Handle hXML = ::sml_ParseXMLFromFile(pFilename) ;

	if (!hXML)
		return NULL ;

	return new ElementXML(hXML) ;
}

/*************************************************************
* @brief Returns an error message describing reason for error in last parse.
*
* Call here if the parsing functions return NULL to find out what went wrong.
*************************************************************/
char const* ElementXML::GetLastParseErrorDescription()
{
	return ::sml_GetLastParseErrorDescription() ;
}
