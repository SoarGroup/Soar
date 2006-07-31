#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ElementXMLImpl class
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
// This class is closely related to the "ElementXML" class that a client would use.
// But the implementation of that class resides in this class, which is in a different DLL.
// (Passing a pointer to an object between DLLs is dangerous--both DLLs would need to have been
//  built at the same time, with the same class header file etc.  Passing a pointer to an object
//  that is owned by a separate DLL is safe, because that single DLL (ElementXML.dll in this case)
//  is the only one that really access the data in the class).
/////////////////////////////////////////////////////////////////

#include "sml_ElementXMLImpl.h"

#include <algorithm>	// For "for_each"

using namespace sml ;

////////////////////////////////////////////////////////////////
//
// Static (class level) functions
//
////////////////////////////////////////////////////////////////

static char const* kLT   = "&lt;" ;
static char const* kGT   = "&gt;" ;
static char const* kAMP  = "&amp;" ;
static char const* kQUOT = "&quot;" ;
static char const* kAPOS = "&apos;" ;

static char const* kCDataStart	= "<!CDATA" ;
static char const* kCDataEnd	= "]]>" ;

static char const* kStartTagOpen  = "<" ;
static char const* kStartTagClose = ">" ;
static char const* kEndTagOpen	  = "</" ;
static char const* kEndTagClose	  = ">" ;

static char const* kEquals		  = "=" ;
static char const* kSpace		  = " " ;
static char const* kQuote		  = "\"" ;

static char const* kCommentStartString	= "<!--" ;
static char const* kCommentEndString	= "-->" ;

static char const* kNewLine		  = "\n" ;

static char const* kEncodeHex = "bin_encoding=\"hex\"" ;

static const int kLenLT   = (int)strlen(kLT) ;
static const int kLenGT   = (int)strlen(kGT) ;
static const int kLenAMP  = (int)strlen(kAMP) ;
static const int kLenQUOT = (int)strlen(kQUOT) ;
static const int kLenAPOS = (int)strlen(kAPOS) ;

static const int kLenCDataStart = (int)strlen(kCDataStart) ;
static const int kLenCDataEnd   = (int)strlen(kCDataEnd) ;

static const int kLenStartTagOpen  = (int)strlen(kStartTagOpen) ;
static const int kLenStartTagClose = (int)strlen(kStartTagClose) ;
static const int kLenEndTagOpen    = (int)strlen(kEndTagOpen) ;
static const int kLenEndTagClose   = (int)strlen(kEndTagClose) ;

static const int kLenNewLine	   = (int)strlen(kNewLine) ;

static const int kLenEquals		   = (int)strlen(kEquals) ;
static const int kLenSpace		   = (int)strlen(kSpace) ;
static const int kLenQuote		   = (int)strlen(kQuote) ;

static const int kLenEncodeHex	   = (int)strlen(kEncodeHex) ;

static const int kLenCommentStartString = (int)strlen(kCommentStartString) ;
static const int kLenCommentEndString = (int)strlen(kCommentEndString) ;

inline static char const* ConvertSpecial(char base)
{
	switch (base)
	{
		case '<'  : return kLT ;
		case '>'  : return kGT;
		case '&'  : return kAMP ;
		case '\"' : return kQUOT ;
		case '\'' : return kAPOS ;
		default: return NULL ;
	}
}

inline static int CharLength(char base)
{
	switch (base)
	{
		case '<'  : return kLenLT ;
		case '>'  : return kLenGT;
		case '&'  : return kLenAMP ;
		case '\"' : return kLenQUOT ;
		case '\'' : return kLenAPOS ;
		default: return 1 ;
	}
}

static char hexChar[16] = { '0', '1', '2' , '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' } ;

/*************************************************************
* @brief Converts the binary data into a null-terminated string of hex characters.
*************************************************************/
static char* BinaryToHexChars(char const* pBinaryBuffer, int length)
{
	// The output will be exactly twice the length of the input
	// when we do simple hex conversion
	char* pHexString = ElementXMLImpl::AllocateString(length*2) ;

	char* pHex = pHexString ;
	for (char const* pBinary = pBinaryBuffer ; length > 0 ; length--)
	{
		char ch = *pBinary++ ;
		*pHex++ = hexChar[((ch & 0xF0) >> 4)] ;
		*pHex++ = hexChar[(ch & 0x0F)] ;
	}

	// Terminate the string
	*pHex = 0 ;

	return pHexString ;
}

static char binaryVal(char c)
{
	return (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : c - 'A' + 10 ;
}

/*************************************************************
* @brief Converts the string of hex characters into binary data.
*************************************************************/
static void HexCharsToBinary(char const* pHexString, char*& pBinaryBuffer, int& length)
{
	// This length should always be an exact multiple of 2
	int stringLength = (int)strlen(pHexString) ;

	// The output will be exactly half the length of the input
	length = (stringLength+1)/2 ;

	// Create the buffer
	pBinaryBuffer = ElementXMLImpl::AllocateString(length) ;

	char* pBinary = pBinaryBuffer ;

	for (char const* pHex = pHexString ; *pHex != 0 ; pHex += 2)
	{
		char ch = (binaryVal(*pHex) << 4) + binaryVal(*(pHex+1)) ;
		*pBinary++ = ch ;
	}
}

/*************************************************************
* @brief Counts how long a string will be when expanded as
*		 XML output.  That is allowing for replacing
*		 "<" with &lt; etc.
*************************************************************/
inline static int CountXMLLength(xmlStringConst str)
{
	int len = 0 ;
	for (char const* p = str ; *p != 0 ; p++)
	{
		len += CharLength(*p) ;
	}

	return len ;
}


/*************************************************************
* @brief Adds pAdd to pDest (just like strcat).
*	     BUT returns a pointer to the end of the combined string
*		 unlike strcat.
*
* Implementation Note:
*    1) Should test the speed against strcat() + strlen().
*	    They may be faster (if coded in assembler) than this
*		manual solution.
*************************************************************/
inline static char* AddString(char* pDest, char const* pAdd)
{
	while (*pAdd != NUL)
		*pDest++ = *pAdd++ ;

	return pDest ;
}

/*************************************************************
* @brief Adds pAdd to pDest (just like strcat).
*	     BUT returns a pointer to the end of the combined string
*		 unlike strcat.
*
*		 Also, this version replaces special characters with
*		 their escape sequence.
*************************************************************/
inline static char* AddXMLString(char* pDest, char const* pAdd)
{
	char ch = *pAdd++ ;
	while (ch != NUL)
	{
		switch (ch)
		{
			case '<'  : pDest = AddString(pDest, kLT) ; break ;
			case '>'  : pDest = AddString(pDest, kGT) ; break ;
			case '&'  : pDest = AddString(pDest, kAMP) ; break ;
			case '\"' : pDest = AddString(pDest, kQUOT) ; break ;
			case '\'' : pDest = AddString(pDest, kAPOS) ; break ;
			default	  : *pDest++ = ch ;
		}

		ch = *pAdd++ ;
	}

	return pDest ;
}


/*************************************************************
* @brief XML ids can only contain letters, numbers, “.” “-“ and “_”.
*************************************************************/
bool ElementXMLImpl::IsValidID(xmlStringConst str)
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
ElementXMLImpl::ElementXMLImpl(void)
{
	// Default to not using a CDATA section to store character data
	m_UseCData = false ;

	// Assume data is not binary until we are told otherwise.
	m_DataIsBinary	   = false ;
	m_BinaryDataLength = 0 ;

	m_TagName = NULL ;
	m_CharacterData = NULL ;
	m_Comment = NULL ;
	m_ErrorCode = 0 ;
	m_pParent = NULL ;

	// Creation of the object creates an initial reference.
	m_RefCount = 1 ;

	// Guessing at the approx number of strings we'll store makes adding
	// attributes substantially faster.
	m_StringsToDelete.reserve(20) ;
}

// Provide a static way to call release ref to get round an STL challenge.
static inline void StaticReleaseRef(ElementXMLImpl* pXML)
{
	pXML->ReleaseRef() ;
}

/*************************************************************
* @brief Destructor.
*************************************************************/
ElementXMLImpl::~ElementXMLImpl(void)
{
	// Delete the comment
	DeleteString(m_Comment) ;

	// Delete the character data
	DeleteString(m_CharacterData) ;

	// Delete all of the strings that we own
	// (We store these in a separate list because some elements in
	//  the attribute map may be constants and some strings we need to delete
	//  so we can't walk it and delete its contents).

	// Replacing a simple walking of the list with a faster algorithmic method
	// for doing the same thing.
	std::for_each(m_StringsToDelete.begin(), m_StringsToDelete.end(), &ElementXMLImpl::DeleteString) ;

	/*
	xmlStringListIter iter = m_StringsToDelete.begin() ;

	while (iter != m_StringsToDelete.end())
	{
		char* str = *iter ;
		iter++ ;

		DeleteString(str) ;
	}
	*/

	// Delete all children

	// Again, replacing this walking of the list with a faster use of std::for_each.
	// We'll use a static method and have it call the member function because it's beyond
	// me how to use std::mem_fun_ref and friends to call directly to the member.
	std::for_each(m_Children.begin(), m_Children.end(), &StaticReleaseRef) ;
	/*
	xmlListIter iterChildren = m_Children.begin() ;

	while (iterChildren != m_Children.end())
	{
		ElementXMLImpl* pChild = *iterChildren ;
		iterChildren++ ;

		pChild->ReleaseRef() ;
	}
	*/
}

/*************************************************************
* @brief Returns a copy of this object.
*		 Generally, this shouldn't be necessary as ref counting
*		 allows multiple clients to point to the same object.
*
*		 Call ReleaseRef() on the returned object when you are done with it.
*************************************************************/
ElementXMLImpl* ElementXMLImpl::MakeCopy() const
{
	ElementXMLImpl* pCopy = new ElementXMLImpl() ;

	pCopy->m_ErrorCode		  = m_ErrorCode ;
	pCopy->m_UseCData		  = m_UseCData ;
	pCopy->m_RefCount		  = 1 ;

	// Start with a null parent and override this later
	pCopy->m_pParent		  = NULL ;

	// Copy the comment
	pCopy->SetComment(m_Comment) ;

	// Copy the tag name
	pCopy->SetTagName(m_TagName) ;

	// Copy the character data
	if (m_DataIsBinary)
		pCopy->SetBinaryCharacterData(m_CharacterData, m_BinaryDataLength) ;
	else
		pCopy->SetCharacterData(m_CharacterData) ;

	// Copy the attributes
	for (xmlAttributeMapConstIter mapIter = m_AttributeMap.begin() ; mapIter != m_AttributeMap.end() ; mapIter++)
	{
		xmlStringConst att = mapIter->first ;
		xmlStringConst val = mapIter->second ;

		pCopy->AddAttribute(att, val) ;
	}

	// Copy all of the children, overwriting the parent field for them so that everything connects up correctly.
	for (xmlListConstIter iterChildren = m_Children.begin() ; iterChildren != m_Children.end() ; iterChildren++)
	{
		ElementXMLImpl* pChild = *iterChildren ;
		ElementXMLImpl* pChildCopy = pChild->MakeCopy() ;
		pChildCopy->m_pParent = pCopy ;
		pCopy->AddChild(pChildCopy) ;
	}

	return pCopy ;
}

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*************************************************************/
int ElementXMLImpl::ReleaseRef()
{
	m_RefCount-- ;

	// Have to store this locally, before we call "delete this"
	int refCount = m_RefCount ;

//	printf("Release Ref for hXML = 0x%x making ref count %d\n", (int)this, m_RefCount) ;

	if (m_RefCount == 0)
		delete this ;

	return refCount ;
}

/*************************************************************
* @brief Add a new reference to this object.
*        The object will only be deleted after calling
*        releaseRef() one more time than addRef() has been
*        called.
*************************************************************/
int ElementXMLImpl::AddRef()
{
	m_RefCount++ ;

//	printf("Add Ref for hXML = 0x%x making ref count %d\n", (int)this, m_RefCount) ;

	return m_RefCount ;
}

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
int ElementXMLImpl::GetRefCount()
{
	return m_RefCount ;
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
bool ElementXMLImpl::SetTagName(char* tagName, bool copyName)
{
	// Decide if we're taking ownership of this string or not.
	if (copyName)
		tagName = CopyString(tagName) ;

	// In this version, we take ownership of the name.
	m_StringsToDelete.push_back(tagName) ;
	
	return SetTagNameFast(tagName) ;
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
bool ElementXMLImpl::SetTagNameFast(char const* tagName)
{
#ifdef DEBUG
	if (!IsValidID(tagName))
		return false ;
#endif

	m_TagName = tagName ;

	return true ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* ElementXMLImpl::GetTagName() const
{
	return m_TagName ;
}

////////////////////////////////////////////////////////////////
//
// Child element functions.
//
// These allow a single ElementXMLImpl object to represent a complete
// XML document through its children.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds a child to the list of children of this element.
*
* @param  pChild	The child to add.  Will be released when the parent is destroyed.
*************************************************************/
void ElementXMLImpl::AddChild(ElementXMLImpl* pChild)
{
	if (pChild == NULL)
		return ;

	pChild->m_pParent = this ;
	this->m_Children.push_back(pChild) ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int ElementXMLImpl::GetNumberChildren() const
{
	return (int)this->m_Children.size() ;
}

/*************************************************************
* @brief Returns the n-th child of this element.
*
* Children are guaranteed to be returned in the order they were added.
* If index is out of range returns NULL.
*
* @param index	The 0-based index of the child to return.
*************************************************************/
ElementXMLImpl const* ElementXMLImpl::GetChild(int index) const
{
	if (index < 0 || index >= (int)m_Children.size())
		return NULL ;

	return m_Children[index] ;
}

/*************************************************************
* @brief Returns the parent of this element.
*
* The caller should *not* call releaseRef() on this parent.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @returns NULL if has no parent.
*************************************************************/
ElementXMLImpl const* ElementXMLImpl::GetParent() const
{
	return m_pParent ;
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
bool ElementXMLImpl::AddAttribute(char* attributeName, char* attributeValue, bool copyName, bool copyValue)
{
	// Decide if we're taking ownership of this string or not.
	if (copyName)
		attributeName = CopyString(attributeName) ;

	// In this version of the call, we own the attribute as well
	// as the value.  (The value gets recorded in the Fast() call).
	m_StringsToDelete.push_back(attributeName) ;

	return AddAttributeFast(attributeName, attributeValue, copyValue) ;
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
bool ElementXMLImpl::AddAttributeFast(char const* attributeName, char* attributeValue, bool copyValue)
{
	// Decide if we're taking ownership of this string or not.
	if (copyValue)
		attributeValue = CopyString(attributeValue) ;

	// In this version of the call, we only own the value.
	m_StringsToDelete.push_back(attributeValue) ;

#ifdef DEBUG
	// Run this test after we've added it to list of strings to delete,
	// otherwise we'd leak it.
	if (!IsValidID(attributeName))
		return false ;
#endif

	m_AttributeMap[attributeName] = attributeValue ;

	return true ;
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
bool ElementXMLImpl::AddAttributeFastFast(char const* attributeName, char const* attributeValue)
{
#ifdef DEBUG
	// Run this test after we've added it to list of strings to delete,
	// otherwise we'd leak it.
	if (!IsValidID(attributeName))
		return false ;
#endif

	m_AttributeMap[attributeName] = attributeValue ;

	return true ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.  
*************************************************************/
int ElementXMLImpl::GetNumberAttributes() const
{
	return (int)m_AttributeMap.size() ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*		 Attributes may not be returned in the order they were added.
*
* @param index	The 0-based index of the attribute to return.
* @returns NULL if index is out of range.
*************************************************************/
const char* ElementXMLImpl::GetAttributeName(int index) const
{
	xmlAttributeMapConstIter mapIter = m_AttributeMap.begin() ;

	while (mapIter != m_AttributeMap.end())
	{
		if (index == 0)
		{
			xmlStringConst att = mapIter->first ;

			return att ;
		}

		mapIter++ ;
		index-- ;
	}

	return NULL ;
}

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index	The 0-based index of the attribute to return.
*************************************************************/
const char* ElementXMLImpl::GetAttributeValue(int index) const
{
	xmlAttributeMapConstIter mapIter = m_AttributeMap.begin() ;

	while (mapIter != m_AttributeMap.end())
	{
		if (index == 0)
		{
			xmlStringConst val = mapIter->second ;

			return val ;
		}

		mapIter++ ;
		index-- ;
	}

	return NULL ;
}

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName	The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* ElementXMLImpl::GetAttribute(const char* attName) const
{
	// Note: We can't use the apparently simpler "return m_AttributeMap[attName]" for two reasons.
	// First, this will create an object in the map if one didn't exist before (which we don't want) and
	// Second, there is no const version of [] so we can't do this anyway (it's not const because of the first behavior...)

	// Use find to locate the object not [].
	xmlAttributeMapConstIter iter = m_AttributeMap.find(attName) ;
	
	if (iter == m_AttributeMap.end())
		return NULL ;

	return iter->second ;
}

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
bool ElementXMLImpl::SetComment(const char* comment)
{
	m_Comment = CopyString(comment) ;
	return true ;
}

/*************************************************************
* @brief Returns the comment for this element.
*
* @returns The comment string for this element (or NULL if there is none)
*************************************************************/
char const* ElementXMLImpl::GetComment()
{
	return m_Comment;
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
void ElementXMLImpl::SetCharacterData(char* characterData, bool copyData)
{
	// Decide if we're taking ownership of this string or not.
	if (copyData)
		characterData = CopyString(characterData) ;

	if (this->m_CharacterData != NULL)
		DeleteString(this->m_CharacterData) ;

	this->m_CharacterData = characterData ;
	this->m_DataIsBinary = false ;
}

void ElementXMLImpl::SetBinaryCharacterData(char* characterData, int length, bool copyData)
{
	// Decide if we're taking ownership of this string or not.
	if (copyData)
		characterData = CopyBuffer(characterData, length) ;

	if (this->m_CharacterData != NULL)
		DeleteString(this->m_CharacterData) ;

	this->m_CharacterData	 = characterData ;
	this->m_DataIsBinary	 = true ;
	this->m_BinaryDataLength = length ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns	Returns the character data for this element.  This can return null if the element has no character data.
*			The character data returned will not include any XML escape sequences (e.g. &lt;). 
*			It will include the original special characters (e.g. "<").
*************************************************************/
char const* ElementXMLImpl::GetCharacterData() const
{
	return this->m_CharacterData ;
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*		 rather than a null-terminated character string.
*************************************************************/
bool ElementXMLImpl::IsCharacterDataBinary() const
{
	return this->m_DataIsBinary ;
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
bool ElementXMLImpl::ConvertCharacterDataToBinary()
{
	if (!m_DataIsBinary && m_CharacterData)
	{
		// These are filled in by the conversion function
		char* pBinaryBuffer = NULL ;
		int   length  = 0 ;

		HexCharsToBinary(m_CharacterData, pBinaryBuffer, length) ;

		this->SetBinaryCharacterData(pBinaryBuffer, length, false) ;
	}

	return this->m_DataIsBinary ;
}

/*************************************************************
* @brief Converts the stored binary data into a string of
*		 characters (hex for now, or base64 later)
*		 which can be safely stored in XML text.
*************************************************************/
bool ElementXMLImpl::ConvertBinaryDataToCharacters()
{
	if (m_DataIsBinary && m_CharacterData)
	{
		char* pHexString = BinaryToHexChars(m_CharacterData, m_BinaryDataLength) ;

		this->SetCharacterData(pHexString, false) ;
	}

	m_DataIsBinary = false ;
	return !m_DataIsBinary ;
}

/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
int	 ElementXMLImpl::GetCharacterDataLength() const
{
	if (!m_CharacterData)
		return 0 ;

	if (m_DataIsBinary)
		return this->m_BinaryDataLength ;
	else
		return (int)strlen(m_CharacterData)+1 ;
}

/*************************************************************
* @brief Setting this value to true indicates that this element’s character data should be stored in a CDATA section.
*		 By default this value will be false.
*
* @param useCData	true if this element’s character data should be stored in a CDATA section.
*************************************************************/
void ElementXMLImpl::SetUseCData(bool useCData)
{
	this->m_UseCData = useCData ;
}

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to XML.
*************************************************************/
bool ElementXMLImpl::GetUseCData() const
{
	return this->m_UseCData ;
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
char* ElementXMLImpl::GenerateXMLString(bool includeChildren, bool insertNewLines) const
{
	// Work out how much space we will need
	int len = DetermineXMLStringLength(0, includeChildren, insertNewLines) ;

	// Allocate a string that big
	char* pStr = AllocateString(len) ;

	// Fill it in
	char* pEnd = GenerateXMLString(0, pStr, len, includeChildren, insertNewLines) ;

	// Terminate the string
	*pEnd = NUL ;

	// Return it all
	return pStr ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object.
*
* @param depth				How deep into the XML tree we are (can be used to indent)
* @param includeChildren	Includes all children in the XML output.
* @param insertNewlines		Add newlines to space out the tags to be more human-readable
*************************************************************/
int ElementXMLImpl::DetermineXMLStringLength(int depth, bool includeChildren, bool insertNewLines) const
{
	int len = 0 ;

	// The comment
	if (m_Comment)
	{
		len += kLenCommentStartString + (int)strlen(m_Comment) + kLenCommentEndString ;
	}

	if (insertNewLines)
		len += depth ;

	// The start tag
	if (m_TagName)
	{
		len += kLenStartTagOpen + (int)strlen(this->m_TagName) + kLenStartTagClose ;
	}

	// The character data
	if (m_CharacterData)
	{
		if (m_DataIsBinary)
		{
			// Binary data will be encoded as hex chars, so exactly
			// twice the length.
			len += m_BinaryDataLength * 2 ;

			// Then we add a special attribute to show this encoding
			len += kLenEncodeHex + 1 ;
		}
		else if (m_UseCData)
		{
			len += kLenCDataStart + (int)strlen(m_CharacterData) + kLenCDataEnd ;
		}
		else
		{
			len += CountXMLLength(m_CharacterData) ;
		}
	}
	
	xmlAttributeMapConstIter mapIter = m_AttributeMap.begin() ;

	while (mapIter != m_AttributeMap.end())
	{
		xmlStringConst att = mapIter->first ;
		xmlStringConst val = mapIter->second ;

		// The attribute name is an identifier and can't contain special chars
		len += kLenSpace + (int)strlen(att) + kLenEquals ;

		// The value can contain special chars
		len += kLenQuote + CountXMLLength(val) + kLenQuote ;

		mapIter++ ;
	}

	bool addedNewLine = false ;
	if (insertNewLines)
	{
		// Put a new line after the end of the opening tag if there are children
		// to be added.
		if (!includeChildren || (includeChildren && !m_Children.empty()))
		{
			addedNewLine = true ;
			len += kLenNewLine ;
		}
	}

	if (includeChildren)
	{
		xmlListConstIter iter = m_Children.begin() ;

		while (iter != m_Children.end())
		{
			ElementXMLImpl const* pChild = *iter ;
			len += pChild->DetermineXMLStringLength(depth+1, includeChildren, insertNewLines) ;

			iter++ ;
		}
	}

	// If we added a new line before children, need to indent again after children
	if (addedNewLine)
		len += depth ;

	// The end tag
	if (m_TagName)
	{
		len += kLenEndTagOpen + (int)strlen(this->m_TagName) + kLenEndTagClose ;
	}

	if (insertNewLines)
		len += kLenNewLine ;

	return len ;
}

/*************************************************************
* @brief Converts the XML object to a string.
*
* @param depth				How deep we are into the XML tree
* @param pStr				The XML object is stored in this string.
* @param maxLength			The max length of the string
* @param includeChildren	Includes all children in the XML output.
* @param insertNewlines		Add newlines to space out the tags to be more human-readable
* @returns Pointer to the end of the string.
*************************************************************/
char* ElementXMLImpl::GenerateXMLString(int depth, char* pStart, int maxLength, bool includeChildren, bool insertNewLines) const
{
	// It's useful for debugging to keep pStart around so we can
	// see the string being formed.
	// pStr will walk down the available memory.
	char* pStr = pStart ;

	// Add the comment
	if (m_Comment)
	{
		pStr = AddString(pStr, kCommentStartString) ;
		pStr = AddString(pStr, m_Comment) ;
		pStr = AddString(pStr, kCommentEndString) ;
	}

	if (insertNewLines)
	{
		// Indent by depth chars
		for (int i = 0 ; i < depth ; i++)
			pStr = AddString(pStr, " ") ;
	}

	pStr = AddString(pStr, kStartTagOpen) ;
	
	// The start tag
	if (m_TagName)
	{
		pStr = AddString(pStr, this->m_TagName) ;
	}

	// The attributes
	xmlAttributeMapConstIter mapIter = m_AttributeMap.begin() ;

	while (mapIter != m_AttributeMap.end())
	{
		xmlStringConst att = mapIter->first ;
		xmlStringConst val = mapIter->second ;

		// The attribute name is an identifier and can't contain special chars
		pStr = AddString(pStr, kSpace) ;
		pStr = AddString(pStr, att) ;
		pStr = AddString(pStr, kEquals) ;

		// The value can contain special chars
		pStr = AddString(pStr, kQuote) ;
		pStr = AddXMLString(pStr, val) ;
		pStr = AddString(pStr, kQuote) ;

		mapIter++ ;
	}

	// We use a special attribute to show this character
	// data is encoded binary data.  Alas there's no real XML standard for this.
	if (m_DataIsBinary && m_CharacterData)
	{
		pStr = AddString(pStr, kSpace) ;
		pStr = AddString(pStr, kEncodeHex) ;
	}

	pStr = AddString(pStr, kStartTagClose) ;

	// The character data
	if (m_CharacterData)
	{
		if (m_DataIsBinary)
		{
			char* pHexString = BinaryToHexChars(m_CharacterData, m_BinaryDataLength) ;

			pStr = AddString(pStr, pHexString) ;

			DeleteString(pHexString) ;
		}
		else if (m_UseCData)
		{
			pStr = AddString(pStr, kCDataStart) ;
			pStr = AddString(pStr, m_CharacterData) ;
			pStr = AddString(pStr, kCDataEnd) ;
		}
		else
		{
			pStr = AddXMLString(pStr, m_CharacterData) ;
		}
	}
	
	bool addedNewLine = false ;
	if (insertNewLines)
	{
		// Put a new line after the end of the opening tag if there are children
		// to be added.
		if (!includeChildren || (includeChildren && !m_Children.empty()))
		{
			addedNewLine = true ;
			pStr = AddString(pStr, kNewLine) ;
		}
	}

	if (includeChildren)
	{
		xmlListConstIter iter = m_Children.begin() ;

		while (iter != m_Children.end())
		{
			ElementXMLImpl const* pChild = *iter ;
			pStr = pChild->GenerateXMLString(depth+1, pStr, maxLength, includeChildren, insertNewLines) ;

			iter++ ;
		}
	}

	// If we added a new line before children, need to indent again after children
	if (addedNewLine)
	{
		// Indent by depth chars
		for (int i = 0 ; i < depth ; i++)
			pStr = AddString(pStr, " ") ;
	}

	// The end tag
	if (m_TagName)
	{
		pStr = AddString(pStr, kEndTagOpen) ;
		pStr = AddString(pStr, this->m_TagName) ;
		pStr = AddString(pStr, kEndTagClose) ;
	}

	if (insertNewLines)
		pStr = AddString(pStr, kNewLine) ;

	return pStr ;
}

////////////////////////////////////////////////////////////////
//
// String and memory functions
//
// These operations allow a client to allocate memory that ElementXMLImpl will later release,
// or similarly, allow a client to release memory that ElementXMLImpl has allocated.
//
// We may decide that a particular allocator will be used to do this (e.g. new[] and delete[]),
// but in general it's safest to use these functions.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Utility function to allocate memory that the client will pass to the other ElementXMLImpl functions.
*
* @param length		The length is the number of characters in the string, so length+1 bytes will be allocated
*					(so that a trailing null is always included).  Thus passing length 0 is valid and will allocate a single byte.
*************************************************************/
/*
char* ElementXMLImpl::AllocateString(int length)
{
	// Switching to malloc and free, specifically so that we can use strdup() for CopyString
	// which gets called a lot.  Using the library implementation (which should be in assembler) will
	// be a lot faster than doing this manually.
	xmlString str = (xmlString)malloc(length+1) ;
//	xmlString str = new char[length+1] ;
	str[0] = 0 ;

	return str ;
}
*/
/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string		The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
/*
void ElementXMLImpl::DeleteString(char* string)
{
	if (string == NULL)
		return ;

//	delete[] string ;
	free(string) ;
}
*/
/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string		The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
/*
char* ElementXMLImpl::CopyString(char const* original)
{
	if (original == NULL)
		return NULL ;

	return strdup(original) ;

	//int len = (int)strlen(original) ;
	//xmlString str = AllocateString(len) ;

	//char* q = str ;
	//for (const char* p = original ; *p != NUL ; p++)
	//{
	//	*q = *p ;
	//	q++ ;
	//}
	//
	//// Make sure it's null terminated.
	// *q = NUL ;

	//return str ;
}
*/

/*************************************************************
* @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* ElementXMLImpl::CopyBuffer(char const* original, int length)
{
	if (original == NULL || length <= 0)
		return NULL ;

	// This will allocate length bytes
	xmlString str = AllocateString(length-1) ;

	char* q = str ;
	for (const char* p = original ; length > 0 ; length--)
	{
		*q++ = *p++ ;
	}
	
	return str ;
}

