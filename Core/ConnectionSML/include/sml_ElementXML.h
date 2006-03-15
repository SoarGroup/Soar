/////////////////////////////////////////////////////////////////
// ElementXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
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
// This class is a simple wrapper around the ElementXML DLL, a separate library.
// This arrangement allows us to pass references to objects created in that library around safely.
//
/////////////////////////////////////////////////////////////////

#ifndef ELEMENTXML_H
#define ELEMENTXML_H

// A null pointer
#ifndef NULL
#define NULL 0
#endif

// The end of a null terminated string
#ifndef NUL
#define NUL 0
#endif

#ifndef unused
#define unused(x) (void)(x)
#endif

#include "sml_Errors.h"
#include "ElementXMLHandle.h"

namespace sml
{
class Connection ;

/*************************************************************
* @brief The ElementXML class represents an element in an XML stream.
*		 Through its children, it can represent an entire XML document.
*************************************************************/
class ElementXML
{
	// Let Connection have access to Fast methods (which are protected because they take care to use correctly).
	friend class Connection ;
	friend class XMLTrace ;

protected:
	ElementXML_Handle m_hXML ;	// Reference to the underlying object which is created from the ElementXML DLL.

public:
	/*************************************************************
	* @brief XML ids can only contain letters, numbers, “.” “-“ and “_”.
	*************************************************************/
	static bool ElementXML::IsValidID(char const* str) ;

public:
	////////////////////////////////////////////////////////////////
	//
	// Constructors and destructors
	//
	////////////////////////////////////////////////////////////////

    /*************************************************************
    * @brief Default constructor.
    *************************************************************/
	ElementXML();

	/*************************************************************
    * @brief Construct an object to manage an existing handle.
	*		 Takes ownership of this handle.
	*		 This does not affect the referencing counting of the
	*		 handle.
	*
	* @param hXML	The handle to an existing ElementXML object
    *************************************************************/
	ElementXML(ElementXML_Handle hXML) ;

	/*************************************************************
    * @brief Destructor.
	*
	*		 If this object is managing a handle when it is deleted,
	*		 that handle's ref count is reduced.
    *************************************************************/
	virtual ~ElementXML(void);

	/*************************************************************
    * @brief Release our reference to this object, possibly
	*        causing it to be deleted.
	*
	*		 NOTE: This doesn't affect the life of this object,
	*		 but the life of the object it is managing.
	*		 You generally don't need to call this.
	*
	* @returns The new reference count (0 implies the object was deleted)
    *************************************************************/
	int ReleaseRefOnHandle() ;

	/*************************************************************
	* @brief Add a new reference to this object.
	*        The object will only be deleted after calling
	*        releaseRef() one more time than addRef() has been
	*        called.
	*        A newly created object has a reference count of 1 automatically.
	*
	*		 NOTE: This doesn't affect the life of this object,
	*		 but the life of the object it is managing.
	*		 You generally don't need to call this.
	*
	* @returns The new reference count (will be at least 2).
	*************************************************************/
	int AddRefOnHandle() ;

	/*************************************************************
	* @returns Reports the current reference count (must be > 0)
	*************************************************************/
	int GetRefCount() ;

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
	void Attach(ElementXML_Handle hXML) ;

	/*************************************************************
    * @brief Releases ownership of the handle.
	*
	*		 The caller must call releaseRef() on this handle
	*		 when it is done with it.
    *************************************************************/
	ElementXML_Handle Detach() ;

	/*************************************************************
    * @brief Provides access to the handle this object is managing.
	*
	*		 This object retains ownership of the handle and the
	*		 caller to this function should not call releaseRef() on it.
	*		 (If the caller wishes to keep a reference to this returned
	*		  handle it should call addRef() on it).
    *************************************************************/
	ElementXML_Handle GetXMLHandle() const ;

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

    /*************************************************************
    * @brief Returns true if the tag name matches.
	*
	* @param pTagName	The tag to test against.
	*
    * @returns true if equal (case sensitive)
	*************************************************************/
	bool IsTag(char const* pTagName) const ;

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
	bool SetComment(char const* pComment) ;

	/*************************************************************
	* @brief Returns the comment for this element.
	*
	* @returns The comment string for this element (or NULL if there is none)
	*************************************************************/
	char const* GetComment() const ;

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
	ElementXML_Handle AddChild(ElementXML* pChild) ;

    /*************************************************************
    * @brief Returns the number of children of this element.
	*************************************************************/
	int GetNumberChildren() const ;

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
	bool GetChild(ElementXML* pChild, int index) const ;

    /*************************************************************
    * @brief Returns the parent of this element by placing it in pParent.
	*
	* The caller must delete the parent when it is finished with it.
	*
	* @returns false if has no parent.
	*************************************************************/
	bool GetParent(ElementXML* pParent) const ;

	/*************************************************************
	* @brief Returns a copy of this object.
	*		 Generally, this shouldn't be necessary as ref counting
	*		 allows multiple clients to point to the same object.
	*
	*		 Call delete on the returned object when you are done with it.
	*************************************************************/
	ElementXML* MakeCopy() const ;

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
	bool AddAttribute(char* attributeName, char* attributeValue, bool copyName = true, bool copyValue = true);

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
	* @param  copyData		If true, characterData will be copied.  If false, we take ownership of characterData
	*************************************************************/
	void SetCharacterData(char* characterData, bool copyData = true) ;
	void SetCharacterData(char const* characterData)	{ SetCharacterData(CopyString(characterData), false) ; }

	/*************************************************************
    * @brief Setting the chracter data in this way indicates that this element’s character data should be treated as a binary buffer
	*		 (so it may contain chars from 0-255, not just ASCII characters).
	*
	* @param characterData	The binary buffer (allocated with allocateString())
	* @param length			The length of the buffer (note: allocateString(len) allocates len+1 bytes--and the len+1 needs to be passed in here.
	* @param copyData		If true, characterData will be copied.  If false, we take ownership of characterData
	*************************************************************/
	void SetBinaryCharacterData(char* characterData, int length, bool copyData = true) ;
	void SetBinaryCharacterData(char const* characterData, int length) { SetBinaryCharacterData(CopyBuffer(characterData, length), length, false) ; }

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
	bool ConvertCharacterDataToBinary() ;

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
	static char* AllocateString(int length) ;

    /*************************************************************
    * @brief Utility function to release memory allocated by this element and returned to the caller.
	*
	* @param string		The string to release.  Passing NULL is valid and does nothing.
	*************************************************************/
	static void DeleteString(char* pString) ;

    /*************************************************************
    * @brief	Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
	*
	* @param string		The string to copy.  Passing NULL is valid and returns NULL.
	*************************************************************/
	static char* CopyString(char const* original) ;

	/*************************************************************
    * @brief	Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
	*			You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
	*
	* @param string		The buffer to copy.  Passing NULL is valid and returns NULL.
	* @param length		The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
	*************************************************************/
	static char* CopyBuffer(char const* original, int length) ;

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
	static ElementXML* ParseXMLFromString(char const* pString) ;

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
	static ElementXML* ParseXMLFromStringSequence(char const* pString, size_t startPos, size_t* endPos) ;

	/*************************************************************
	* @brief Parse an XML document from a file and return an ElementXML object
	*		 for the document.
	*
	* @param  pFilename	The file to load
	* @returns NULL if parsing failed, otherwise the ElementXML representing XML doc
	*************************************************************/
	static ElementXML* ParseXMLFromFile(char const* pFilename) ;

	/*************************************************************
	* @brief Returns an error message describing reason for error in last parse.
	*
	* Call here if the parsing functions return NULL to find out what went wrong.
	*************************************************************/
	static char const* GetLastParseErrorDescription() ;

protected:
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

	bool AddAttributeFast(char const* attributeName, char const* attributeValue) { return AddAttributeFast(attributeName, CopyString(attributeValue), false) ; } 

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


};

} // end of namespace

#endif // ELEMENTXML_H
