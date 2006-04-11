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

#ifndef ELEMENTXML_IMPL_H
#define ELEMENTXML_IMPL_H

// A null pointer
#ifndef NULL
#define NULL 0
#endif

// The end of a null terminated string
#ifndef NUL
#define NUL 0
#endif

#include <string>
#include <list>
#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <vector>
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

#ifdef _MSC_VER
// Visual Studio 2005 requires this
#define strdup _strdup
#endif

#ifndef unused
#define unused(x) (void)(x)
#endif

namespace sml
{

// Forward declarations
class ElementXMLImpl ;
class MessageGenerator ;

// Used to store a list of ElementXMLImpl nodes (use a vector for rapid index access)
typedef std::vector<ElementXMLImpl*>	xmlList ;
typedef xmlList::iterator			xmlListIter ;
typedef xmlList::const_iterator		xmlListConstIter ;

// I think we'll just use char* internally for strings, but having a typedef means we can
// change this decision later with less typing.
typedef char*			xmlString ;
typedef char const*		xmlStringConst ; 

// Used to store a list of strings.  Switched to vector to improve performance (we only add one by one and then delete the lot)
typedef std::vector<xmlString>			xmlStringList ;
typedef xmlStringList::iterator			xmlStringListIter ;
typedef xmlStringList::const_iterator	xmlStringListConstIter ;

// We need a comparator to make the map we're about to define work with char*
struct strCompareElementXMLImpl
{
  inline bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

// Used to store a map from attribute name to attribute value
typedef std::map<xmlStringConst, xmlStringConst, strCompareElementXMLImpl>	xmlAttributeMap ;
typedef xmlAttributeMap::iterator										xmlAttributeMapIter ;
typedef xmlAttributeMap::const_iterator									xmlAttributeMapConstIter ;

/*************************************************************
* @brief The ElementXMLImpl class represents an element in an XML stream.
*		 Through its children, it can represent an entire XML document.
*************************************************************/
class ElementXMLImpl
{
	// Let MessageGenerator have access to Fast methods (which are protected because they take care to use correctly).
	friend class MessageGenerator ;

protected:
	int				m_ErrorCode ;		// Used to report any errors.
	bool			m_UseCData ;		// If true, should store character data in a CDATA section when encoding as XML.
	xmlStringConst	m_TagName ;			// The tag name (e.g. in <name>...</name> the tag name is "name")
	xmlString		m_CharacterData ;	// The character data (e.g. in <name>Albert Einstein</name> the char data is "Albert Einstein")
	xmlAttributeMap	m_AttributeMap ;	// Mapping from attribute-name to attribute-value (e.g. in <name first="Albert"> first is an attribute with value "Albert")
	xmlList			m_Children ;		// List of children of this element
	xmlString		m_Comment ;			// Used to attach a comment to this object.  It will appear ahead of the element when stored/retrieved.
	volatile int	m_RefCount ;		// Reference count.  Set to 1 on initialization.  When reaches 0 the object is deleted.
	bool			m_DataIsBinary ;	// If true, then the character data is treated as a binary buffer (can contain embedded nulls) and the binary length is needed
	int				m_BinaryDataLength ;// Gives the length of the character data buffer, when it's being treated as a binary buffer.  (only valid if m_IsDataBinary is true).
	ElementXMLImpl*	m_pParent ;			// The parent of this object (can be NULL)

	xmlStringList	m_StringsToDelete ;	// List of strings we now own and should delete when we are destroyed.

	/*************************************************************
    * @brief Destructor.  This is private so we are forced to
	*        use the release method, which supports ref-counting.
    *************************************************************/
protected:
	virtual ~ElementXMLImpl(void);

public:
	/*************************************************************
	* @brief XML ids can only contain letters, numbers, “.” “-“ and “_”.
	*************************************************************/
	static bool ElementXMLImpl::IsValidID(xmlStringConst str) ;

public:
	////////////////////////////////////////////////////////////////
	//
	// Constructors and destructors
	//
	////////////////////////////////////////////////////////////////

    /*************************************************************
    * @brief Default constructor.
    *************************************************************/
	ElementXMLImpl(void);

	/*************************************************************
    * @brief Release our reference to this object, possibly
	*        causing it to be deleted.
	*
	* @returns The new reference count (0 implies the object was deleted)
    *************************************************************/
	int ReleaseRef() ;

	/*************************************************************
	* @brief Add a new reference to this object.
	*        The object will only be deleted after calling
	*        releaseRef() one more time than addRef() has been
	*        called.
	*        A newly created object has a reference count of 1 automatically.
	*
	* @returns The new reference count (will be at least 2).
	*************************************************************/
	int AddRef() ;

	/*************************************************************
	* @returns Reports the current reference count (must be > 0)
    *************************************************************/
	int GetRefCount() ;

	/*************************************************************
	* @returns If an error occurs, this code can provide further details.
	*		   (Not currently used -- but provided for later expansion)
    *************************************************************/
	int GetLastError() { return m_ErrorCode ; }

	/*************************************************************
	* @returns If an error occurs, this provides a text description of
	*		   the error.
	*		   (Not currently used -- but provided for later expansion)
    *************************************************************/
	char const* GetLastErrorDescription() { return NULL ; }

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
	bool SetTagName(char* tagName, bool copyName = true) ;

    /*************************************************************
    * @brief Helper overload -- if we're passed a const, must copy it.
	*************************************************************/
	bool SetTagName(char const* tagName) { return SetTagName(CopyString(tagName), false) ; }

    /*************************************************************
    * @brief Gets the tag name for this element.
	*
    * @returns The tag name.
	*************************************************************/
	char const* GetTagName() const ;

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
	void AddChild(ElementXMLImpl* pChild) ;

    /*************************************************************
    * @brief Returns the number of children of this element.
	*************************************************************/
	int GetNumberChildren() const ;

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
	ElementXMLImpl const* GetChild(int index) const ;

	/*************************************************************
	* @brief Returns the parent of this element.
	*
	* The caller should *not* call releaseRef() on this parent.
	* If you wish to keep it, you can call addRef() (and then later releaseRef()).
	*
	* @returns NULL if has no parent.
	*************************************************************/
	ElementXMLImpl const* GetParent() const ;

	/*************************************************************
	* @brief Returns a copy of this object.
	*		 Generally, this shouldn't be necessary as ref counting
	*		 allows multiple clients to point to the same object.
	*
	*		 Call ReleaseRef() on the returned object when you are done with it.
	*************************************************************/
	ElementXMLImpl* MakeCopy() const ;

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
	bool AddAttribute(char * attributeName, char* attributeValue, bool copyName = true, bool copyValue = true);

    /*************************************************************
    * @brief Helper overloads -- if we're passed a const, must copy it.
	*************************************************************/
	bool AddAttribute(char const* attributeName, char* attributeValue)		 { return AddAttribute(CopyString(attributeName), attributeValue, false, true) ; }
	bool AddAttribute(char const* attributeName, char const* attributeValue) { return AddAttribute(CopyString(attributeName), CopyString(attributeValue), false, false) ; }

    /*************************************************************
    * @brief Get the number of attributes attached to this element.  
	*************************************************************/
	int GetNumberAttributes() const ;

    /*************************************************************
    * @brief Get the name of the n-th attribute of this element.
	*		 Attributes may not be returned in the order they were added.
	*
	* @param index	The 0-based index of the attribute to return.
	*************************************************************/
	const char* GetAttributeName(int index) const ;

    /*************************************************************
    * @brief Get the value of the n-th attribute of this element.
	*
	* @param index	The 0-based index of the attribute to return.
	*************************************************************/
	const char* GetAttributeValue(int index) const ;

    /*************************************************************
    * @brief Get the value of the named attribute of this element.
	*
	* @param attName	The name of the attribute to look up.
	* @returns The value of the named attribute (or null if this attribute doesn't exist).
	*************************************************************/
	const char* GetAttribute(const char* attName) const ;

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
	bool SetComment(const char* comment) ;

    /*************************************************************
    * @brief Returns the comment for this element.
	*
	* @returns The comment string for this element (or NULL if there is none)
	*************************************************************/
	char const* GetComment() ;

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
	void SetCharacterData(char* characterData, bool copyData = true) ;

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
	void SetBinaryCharacterData(char* characterData, int length, bool copyData = true) ;

    /*************************************************************
    * @brief Get the character data for this element.
	*
	* @returns	Returns the character data for this element.  This can return null if the element has no character data.
	*			The character data returned will not include any XML escape sequences (e.g. &lt;). 
	*			It will include the original special characters (e.g. "<").
	*************************************************************/
	char const* GetCharacterData() const ;

	/*************************************************************
    * @brief Returns true if the character data should be treated as a binary buffer
	*		 rather than a null-terminated character string.
	*************************************************************/
	bool IsCharacterDataBinary() const ;

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
	bool ElementXMLImpl::ConvertCharacterDataToBinary() ;

	/*************************************************************
	* @brief Converts the stored binary data into a string of
	*		 characters (hex for now, or base64 later)
	*		 which can be safely stored in XML text.
	*
	* @returns true if buffer is characters after conversion.
	*************************************************************/
	bool ElementXMLImpl::ConvertBinaryDataToCharacters() ;

    /*************************************************************
    * @brief Returns the length of the character data.
	*
	* If the data is a binary buffer this is the size of that buffer.
	* If the data is a null terminated string this is the length of the string + 1 (for the null).
	*************************************************************/
	int	 GetCharacterDataLength() const ;

    /*************************************************************
    * @brief Setting this value to true indicates that this element’s character data should be stored in a CDATA section.
	*		 By default this value will be false.
	*
	*		 This value is ignored if the character data is marked as binary data.
	*
	* @param useCData	true if this element’s character data should be stored in a CDATA section.
	*************************************************************/
	void SetUseCData(bool useCData) ;

	/*************************************************************
    * @brief Returns true if this element's character data should be stored in a CDATA section when streamed to XML.
	*************************************************************/
	bool GetUseCData() const ;

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
	char* GenerateXMLString(bool includeChildren) const ;

    /*************************************************************
    * @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
	*	*
	* @param includeChildren	Includes all children in the XML output.
	*************************************************************/
	int DetermineXMLStringLength(bool includeChildren) const ;

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
	static inline char* AllocateString(int length)
	{
		// Switching to malloc and free (from new[] and delete[]), specifically so that we can use strdup() for CopyString
		// which gets called a lot.  Using the library implementation (which should be in assembler) will
		// be a lot faster than doing this manually.
		xmlString str = (xmlString)malloc(length+1) ;
		str[0] = 0 ;

		return str ;
	}

    /*************************************************************
    * @brief Utility function to release memory allocated by this element and returned to the caller.
	*
	* @param string		The string to release.  Passing NULL is valid and does nothing.
	*************************************************************/
	static inline void DeleteString(char* string)
	{
		if (string == NULL)
			return ;

		free(string) ;
	}

    /*************************************************************
    * @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
	*
	* @param string		The string to copy.  Passing NULL is valid and returns NULL.
	*************************************************************/
	static inline char* CopyString(char const* original)
	{
		if (original == NULL)
			return NULL ;

		return strdup(original) ;
	}

	/*************************************************************
    * @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
	*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
	*
	* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
	* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
	*************************************************************/
	static char* CopyBuffer(char const* original, int length) ;

//protected:
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
	bool AddAttributeFast(char const* attributeName, char* attributeValue, bool copyValue = true);

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
	bool AddAttributeFastFast(char const* attributeName, char const* attributeValue);

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
	bool SetTagNameFast(char const* tagName) ;

protected:
	/*************************************************************
    * @brief Converts the XML object to a string.
	*
	* Note: maxLength is currently ignored for speed, but I'm leaving
	*		it in the list of params, so we can make this safe later
	*		if we wish (for security etc).
	*
	* @param pStr				The XML object is stored in this string.
	* @param maxLength			The max length of the string (not counting trailing null)
	* @param includeChildren	Includes all children in the XML output.
	* @returns	Pointer to the end of the string.
	*************************************************************/
	char* GenerateXMLString(char* pStr, int maxLength, bool includeChildren) const ;


};

} // end of namespace

#endif // ElementXMLImpl_H
