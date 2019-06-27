/////////////////////////////////////////////////////////////////
// ElementJSONInterface file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This file offers a C level interface to the ElementJSON code.
// The real functionality is within a C++ ElementJSON class but it's easier to export
// a C level interface than a C++ one.
//
// This ElementJSON library is responsible for representing an JSON document as an object (actually a tree of objects).
//
// A client can send a stream of JSON data which this class parses to create the object representation of the JSON.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// JSON output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of JSON which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
/////////////////////////////////////////////////////////////////

#ifndef ELEMENTJSONINTERFACE_H
#define ELEMENTJSONINTERFACE_H

#include "ElementJSONHandle.h"

// get definition of EXPORT
#include "Export.h"

//DJP: A quick test of statically linked version of ElementJSON
/*
#define WIN_STATIC_LINK

// Hard coding the lib here so I don't have to go and update all of the other projects

#ifdef WIN_STATIC_LINK
#undef EXPORT
#define EXPORT
#pragma comment (lib, "E:/SoarMich/SoarIO/ElementJSON/lib/ElementJSON-static.lib")
#endif
*/

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////
//
// Constructors and destructors
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Default constructor.
*************************************************************/
EXPORT ElementJSON_Handle soarjson_NewElementJSON() ;

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*
* @returns The new reference count (0 implies the object was deleted)
*************************************************************/
EXPORT int soarjson_ReleaseRef(ElementJSON_Handle hJSON) ;

/*************************************************************
* @brief Add a new reference to this object.
*        The object will only be deleted after calling
*        releaseRef() one more time than addRef() has been
*        called.
*        A newly created object has a reference count of 1 automatically.
*
* @returns The new reference count (will be at least 2).
*************************************************************/
EXPORT int soarjson_AddRef(ElementJSON_Handle hJSON) ;

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
EXPORT int soarjson_GetRefCount(ElementJSON_Handle hJSON) ;

////////////////////////////////////////////////////////////////
//
// Tag functions (e.g the tag in <name>...</name> is "name")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the tag name for this element.
*
* @param  tagName   Tag name can only contain letters, numbers, “.” “-“ and “_”.
* @param  copyName  If true, tagName will be copied.  If false, we take ownership of tagName.
* @returns  true if the tag name is valid.
*************************************************************/
EXPORT bool soarjson_SetTagName(ElementJSON_Handle hJSON, char* tagName, bool copyName) ;

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
EXPORT char const* soarjson_GetTagName(ElementJSON_Handle const hJSON) ;

////////////////////////////////////////////////////////////////
//
// Child element functions.
//
// These allow a single ElementJSON object to represent a complete
// JSON document through its children.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds a child to the list of children of this element.
*
* @param  pChild    The child to add.  Will be released when the parent is destroyed.
*************************************************************/
EXPORT void soarjson_AddChild(ElementJSON_Handle hJSON, ElementJSON_Handle hChild) ;

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
EXPORT int soarjson_GetNumberChildren(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Returns the n-th child of this element.
*
* Children are guaranteed to be returned in the order they were added.
* If index is out of range returns NULL.
* The caller should *not* call releaseRef() on this child.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* We label the return value as a const and many of the functions defined here
* take "const" arguments.  Since we're just passing ints around this doesn't
* really do anything for type safety, but it may help clarify the ownership.
*
* @param index  The 0-based index of the child to return.
*************************************************************/
EXPORT ElementJSON_Handle const soarjson_GetChild(ElementJSON_Handle const hJSON, int index) ;

/*************************************************************
* @brief Returns the parent of this element.
*
* The caller should *not* call releaseRef() on this parent.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @returns NULL if has no parent.
*************************************************************/
EXPORT ElementJSON_Handle const soarjson_GetParent(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Returns a copy of this object.
*        Generally, this shouldn't be necessary as ref counting
*        allows multiple clients to point to the same object.
*
*        Call ReleaseRef() on the returned object when you are done with it.
*************************************************************/
EXPORT ElementJSON_Handle const soarjson_MakeCopy(ElementJSON_Handle const hJSON) ;

////////////////////////////////////////////////////////////////
//
// Attribute functions (e.g an attribute in <name first="doug">...</name> is first="doug")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* @param attributeName  Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @param  copyName      If true, atttributeName will be copied.  If false, we take ownership of attributeName
* @param  copyValue     If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
EXPORT bool soarjson_AddAttribute(ElementJSON_Handle hJSON, char* attributeName, char* attributeValue, bool copyName, bool copyValue);

/*************************************************************
* @brief Get the number of attributes attached to this element.
*************************************************************/
EXPORT int soarjson_GetNumberAttributes(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*        Attributes may not be returned in the order they were added.
*
* @param index  The 0-based index of the attribute to return.
*************************************************************/
EXPORT char const* soarjson_GetAttributeName(ElementJSON_Handle const hJSON, int index) ;

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index  The 0-based index of the attribute to return.
*************************************************************/
EXPORT char const* soarjson_GetAttributeValue(ElementJSON_Handle const hJSON, int index) ;

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName    The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
EXPORT char const* soarjson_GetAttribute(ElementJSON_Handle const hJSON, char const* attName) ;

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
* @param characterData  The character data passed in should *not* replace special characters such as “<” and “&”
*                       with the JSON escape sequences &lt; etc.
*                       These values will be converted when the JSON stream is created.
* @param  copyData      If true, characterData will be copied.  If false, we take ownership of characterData
*************************************************************/
EXPORT void soarjson_SetCharacterData(ElementJSON_Handle hJSON, char* characterData, bool copyData) ;

/*************************************************************
* @brief Setting the chracter data in this way indicates that this element’s character data should be treated as a binary buffer
*        (so it may contain chars from 0-255, not just ASCII characters).
*
* NOTE: The characterData will be deleted by this object when it is deleted.
* It should be allocated with either allocateString() or copyString().
* Be careful with the lengths -- allocateString(len) allocates len+1 bytes and the
* length of the entire buffer should be passed in here (i.e. len+1 in this example).
*
* @param characterData  The binary buffer (allocated with allocateString())
* @param length         The length of the buffer
* @param copyData       If true, characterData will be copied.  If false, we take ownership of characterData
*************************************************************/
EXPORT void soarjson_SetBinaryCharacterData(ElementJSON_Handle hJSON, char* characterData, int length, bool copyData) ;

/*************************************************************
* @brief Get the character data for this element.
*
* @returns  Returns the character data for this element.  If the element has no character data, returns zero-length string.
*           The character data returned will not include any JSON escape sequences (e.g. &lt;).
*           It will include the original special characters (e.g. "<").
*************************************************************/
EXPORT char const* soarjson_GetCharacterData(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*        rather than a null-terminated character string.
*************************************************************/
EXPORT bool soarjson_IsCharacterDataBinary(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Converts a character data buffer into binary data.
*
* If binary data is stored in an JSON file it will encoded in
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
EXPORT bool soarjson_ConvertCharacterDataToBinary(ElementJSON_Handle hJSON) ;

/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
EXPORT int   soarjson_GetCharacterDataLength(ElementJSON_Handle const hJSON) ;

/*************************************************************
* @brief Setting this value to true indicates that this element’s character data should be stored in a CDATA section.
*        By default this value will be false.
*
*        This value is ignored if the character data is marked as binary data.
*
* @param useCData   true if this element’s character data should be stored in a CDATA section.
*************************************************************/
EXPORT void soarjson_SetUseCData(ElementJSON_Handle hJSON, bool useCData) ;

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to JSON.
*************************************************************/
EXPORT bool soarjson_GetUseCData(ElementJSON_Handle const hJSON) ;

////////////////////////////////////////////////////////////////
//
// Generator
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Converts the JSON object to a string.
*
* @param includeChildren    Includes all children in the JSON output.
* @param insertNewlines     Add newlines to space out the tags to be more human-readable
*
* @returns The string form of the object.  Caller must delete with DeleteString().
*************************************************************/
EXPORT char* soarjson_GenerateJSONString(ElementJSON_Handle const hJSON, bool includeChildren, bool insertNewLines) ;

/*************************************************************
* @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
*
* @param includeChildren    Includes all children in the JSON output.
* @param insertNewlines     Add newlines to space out the tags to be more human-readable
*************************************************************/
EXPORT int soarjson_DetermineJSONStringLength(ElementJSON_Handle const hJSON, bool includeChildren, bool insertNewLines) ;

////////////////////////////////////////////////////////////////
//
// String and memory functions
//
// These operations allow a client to allocate memory that ElementJSON will later release,
// or similarly, allow a client to release memory that ElementJSON has allocated.
//
// We may decide that a particular allocator will be used to do this (e.g. new[] and delete[]),
// but in general it's safest to use these functions.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Utility function to allocate memory that the client will pass to the other ElementJSON functions.
*
* @param length     The length is the number of characters in the string, so length+1 bytes will be allocated
*                   (so that a trailing null is always included).  Thus passing length 0 is valid and will allocate a single byte.
*************************************************************/
EXPORT char* soarjson_AllocateString(int length) ;

/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string     The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
EXPORT void soarjson_DeleteString(char* pString) ;

/*************************************************************
* @brief    Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string     The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
EXPORT char* soarjson_CopyString(char const* original) ;

/*************************************************************
* @brief    Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*           You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string     The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length     The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
EXPORT char* soarjson_CopyBuffer(char const* original, int length) ;

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name must remain in scope for the life of this object.
*       In practice, this generally means it must be a static constant.
*
* @param attributeName  Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @param  copyValue     If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
EXPORT bool soarjson_AddAttributeFast(ElementJSON_Handle hJSON, char const* attributeName, char* attributeValue, bool copyValue);

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name and value must remain in scope for the life of this object.
*       In practice, this generally means it must be a static constant.
*
* @param attributeName  Attribute name can only contain letters, numbers, “.” “-“ and “_”.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
EXPORT bool soarjson_AddAttributeFastFast(ElementJSON_Handle hJSON, char const* attributeName, char const* attributeValue);

/*************************************************************
* @brief Set the tag name for this element.
*
* NOTE: The caller must ensure that the tag name does not go out of scope
* before this object is destroyed.  This requirement means the tag name
* should generally be declared as a static constant.
*
* @param  tagName   Tag name can only contain letters, numbers, “.” “-“ and “_”.
* @returns  true if the tag name is valid.
*************************************************************/
EXPORT bool soarjson_SetTagNameFast(ElementJSON_Handle hJSON, char const* tagName) ;

////////////////////////////////////////////////////////////////
//
// Error reporting functions.
//
////////////////////////////////////////////////////////////////
EXPORT int soarjson_GetLastError(ElementJSON_Handle hJSON) ;

EXPORT char const* soarjson_GetLastErrorDescription(ElementJSON_Handle hJSON) ;

////////////////////////////////////////////////////////////////
//
// Parsing functions
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Parse an JSON document from a (long) string and return an ElementJSON object
*        for the document.
*
* @param  pString   The JSON document stored in a string.
* @returns NULL if parsing failed, otherwise the ElementJSON representing JSON doc
*************************************************************/
EXPORT ElementJSON_Handle soarjson_ParseJSONFromString(char const* pString) ;

/*************************************************************
* @brief Parse an JSON document from a (long) string and return an ElementJSON object
*        for the document.  This version supports a sequence of JSON strings which
*        need to be parsed in order (rather than all being part of one document).
*
* @param  pString   The JSON document stored in a string.
* @param  startPos  We'll start parsing the current JSON document from this position (0 == beginning of the string)
* @param  endPos    This value is filled in at the end of the parse and indicates where the parse ended. (if endPos == strlen(pString) we're done)
* @returns NULL if parsing failed, otherwise the ElementJSON representing JSON doc
*************************************************************/
EXPORT ElementJSON_Handle soarjson_ParseJSONFromStringSequence(char const* pString, size_t startPos, size_t* endPos) ;

/*************************************************************
* @brief Parse an JSON document from a file and return an ElementJSON object
*        for the document.
*
* @param  pFilename The file to load
* @returns NULL if parsing failed, otherwise the ElementJSON representing JSON doc
*************************************************************/
EXPORT ElementJSON_Handle soarjson_ParseJSONFromFile(char const* pFilename) ;

/*************************************************************
* @brief Returns an error message describing reason for error in last parse.
*
* Call here if the parsing functions return NULL to find out what went wrong.
*************************************************************/
EXPORT char const* soarjson_GetLastParseErrorDescription() ;


#ifdef __cplusplus
} // extern C
#endif

#endif  // ELEMENTJSONINTERFACE_H
