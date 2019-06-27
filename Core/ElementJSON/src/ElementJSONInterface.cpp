#include "portability.h"

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

#include "ElementJSONInterface.h"
#include "ElementJSONImpl.h"
#include "ParseJSONFile.h"
#include "ParseJSONString.h"

#include <string>
#include <cstdio>

using namespace soarjson;

// We store the last parsing error message here.
// This is a bit inelegant, but makes it easier for the client to use this
// interface from any language.
static std::string s_LastParseErrorMessage ;

inline static ElementJSONImpl* GetElementFromHandle(ElementJSON_Handle hJSON)
{
    return reinterpret_cast<ElementJSONImpl*>(hJSON) ;
}

////////////////////////////////////////////////////////////////
//
// Constructors and destructors
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Default constructor.
*************************************************************/
ElementJSON_Handle soarjson_NewElementJSON()
{
    return reinterpret_cast<ElementJSON_Handle>(new ElementJSONImpl()) ;
}

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*
* @returns The new reference count (0 implies the object was deleted)
*************************************************************/
int soarjson_ReleaseRef(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->ReleaseRef() ;
}

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
int soarjson_GetRefCount(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetRefCount() ;
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
int soarjson_AddRef(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->AddRef() ;
}

////////////////////////////////////////////////////////////////
//
// Tag functions (e.g the tag in <name>...</name> is "name")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the tag name for this element.
*
* @param  tagName   Tag name can only contain letters, numbers, . - and _.
* @param  copyName  If true, tagName will be copied.  If false, we take ownership of tagName.
* @returns  true if the tag name is valid.
*************************************************************/
bool soarjson_SetTagName(ElementJSON_Handle hJSON, char* tagName, bool copyName)
{
    return GetElementFromHandle(hJSON)->SetTagName(tagName, copyName) ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* soarjson_GetTagName(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetTagName() ;
}

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
void soarjson_AddChild(ElementJSON_Handle hJSON, ElementJSON_Handle hChild)
{
    return GetElementFromHandle(hJSON)->AddChild(GetElementFromHandle(hChild)) ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int soarjson_GetNumberChildren(ElementJSON_Handle const hJSON)
{
    return GetElementFromHandle(hJSON)->GetNumberChildren() ;
}

/*************************************************************
* @brief Returns the n-th child of this element.
*
* Children are guaranteed to be returned in the order they were added.
* If index is out of range returns NULL.
* The caller should *not* call releaseRef() on this child.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @param index  The 0-based index of the child to return.
*************************************************************/
ElementJSON_Handle const soarjson_GetChild(ElementJSON_Handle const hJSON, int index)
{
    // FIXME can't use reinterpret cast here??
    return (ElementJSON_Handle)GetElementFromHandle(hJSON)->GetChild(index) ;
}

/*************************************************************
* @brief Returns the parent of this element.
*
* The caller should *not* call releaseRef() on this parent.
* If you wish to keep it, you can call addRef() (and then later releaseRef()).
*
* @returns NULL if has no parent.
*************************************************************/
ElementJSON_Handle const soarjson_GetParent(ElementJSON_Handle const hJSON)
{
    // FIXME can't use reinterpret cast here??
    return (ElementJSON_Handle)GetElementFromHandle(hJSON)->GetParent() ;
}

/*************************************************************
* @brief Returns a copy of this object.
*        Generally, this shouldn't be necessary as ref counting
*        allows multiple clients to point to the same object.
*
*        Call ReleaseRef() on the returned object when you are done with it.
*************************************************************/
ElementJSON_Handle const soarjson_MakeCopy(ElementJSON_Handle const hJSON)
{
    return reinterpret_cast<ElementJSON_Handle>(GetElementFromHandle(hJSON)->MakeCopy()) ;
}

////////////////////////////////////////////////////////////////
//
// Attribute functions (e.g an attribute in <name first="doug">...</name> is first="doug")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* @param attributeName  Attribute name can only contain letters, numbers, . - and _.
* @param attributeValue Can be any string.
* @param  copyName      If true, atttributeName will be copied.  If false, we take ownership of attributeName
* @param  copyValue     If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool soarjson_AddAttribute(ElementJSON_Handle hJSON, char* attributeName, char* attributeValue, bool copyName, bool copyValue)
{
    return GetElementFromHandle(hJSON)->AddAttribute(attributeName, attributeValue, copyName, copyValue) ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.
*************************************************************/
int soarjson_GetNumberAttributes(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetNumberAttributes() ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*        Attributes may not be returned in the order they were added.
*
* @param index  The 0-based index of the attribute to return.
*************************************************************/
const char* soarjson_GetAttributeName(ElementJSON_Handle hJSON, int index)
{
    return GetElementFromHandle(hJSON)->GetAttributeName(index) ;
}

/*************************************************************
* @brief Get the value of the n-th attribute of this element.
*
* @param index  The 0-based index of the attribute to return.
*************************************************************/
const char* soarjson_GetAttributeValue(ElementJSON_Handle hJSON, int index)
{
    return GetElementFromHandle(hJSON)->GetAttributeValue(index) ;
}

/*************************************************************
* @brief Get the value of the named attribute of this element.
*
* @param attName    The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* soarjson_GetAttribute(ElementJSON_Handle hJSON, const char* attName)
{
    return GetElementFromHandle(hJSON)->GetAttribute(attName) ;
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
* @param characterData  The character data passed in should *not* replace special characters such as < and &
*                       with the JSON escape sequences &lt; etc.
*                       These values will be converted when the JSON stream is created.
* @param  copyData      If true, characterData will be copied.  If false, we take ownership of characterData
*************************************************************/
void soarjson_SetCharacterData(ElementJSON_Handle hJSON, char* characterData, bool copyData)
{
    return GetElementFromHandle(hJSON)->SetCharacterData(characterData, copyData) ;
}

/*************************************************************
* @brief Setting the chracter data in this way indicates that this elements character data should be treated as a binary buffer
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
void soarjson_SetBinaryCharacterData(ElementJSON_Handle hJSON, char* characterData, int length, bool copyData)
{
    return GetElementFromHandle(hJSON)->SetBinaryCharacterData(characterData, length, copyData) ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns  Returns the character data for this element.  If the element has no character data, returns zero-length string.
*           The character data returned will not include any JSON escape sequences (e.g. &lt;).
*           It will include the original special characters (e.g. "<").
*************************************************************/
char const* soarjson_GetCharacterData(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetCharacterData() ;
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*        rather than a null-terminated character string.
*************************************************************/
bool soarjson_IsCharacterDataBinary(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->IsCharacterDataBinary() ;
}

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
bool soarjson_ConvertCharacterDataToBinary(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->ConvertCharacterDataToBinary() ;
}


/*************************************************************
* @brief Returns the length of the character data.
*
* If the data is a binary buffer this is the size of that buffer.
* If the data is a null terminated string this is the length of the string + 1 (for the null).
*************************************************************/
int  soarjson_GetCharacterDataLength(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetCharacterDataLength() ;
}

/*************************************************************
* @brief Setting this value to true indicates that this elements character data should be stored in a CDATA section.
*        By default this value will be false.
*
*        This value is ignored if the character data is marked as binary data.
*
* @param useCData   true if this elements character data should be stored in a CDATA section.
*************************************************************/
void soarjson_SetUseCData(ElementJSON_Handle hJSON, bool useCData)
{
    return GetElementFromHandle(hJSON)->SetUseCData(useCData) ;
}

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to JSON.
*************************************************************/
bool soarjson_GetUseCData(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetUseCData() ;
}

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
char* soarjson_GenerateJSONString(ElementJSON_Handle const hJSON, bool includeChildren, bool insertNewLines)
{
    return GetElementFromHandle(hJSON)->GenerateJSONString(includeChildren, insertNewLines) ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
*
* @param includeChildren    Includes all children in the JSON output.
* @param insertNewlines     Add newlines to space out the tags to be more human-readable
*************************************************************/
int soarjson_DetermineJSONStringLength(ElementJSON_Handle const hJSON, bool includeChildren, bool insertNewLines)
{
    return GetElementFromHandle(hJSON)->DetermineJSONStringLength(0, includeChildren, insertNewLines);
}

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
char* soarjson_AllocateString(int length)
{
    return ElementJSONImpl::AllocateString(length) ;
}

/*************************************************************
* @brief Utility function to release memory allocated by this element and returned to the caller.
*
* @param string     The string to release.  Passing NULL is valid and does nothing.
*************************************************************/
void soarjson_DeleteString(char* pString)
{
    ElementJSONImpl::DeleteString(pString) ;
}

/*************************************************************
* @brief    Performs an allocation and then copies the contents of the passed in string to the newly allocated string.
*
* @param string     The string to copy.  Passing NULL is valid and returns NULL.
*************************************************************/
char* soarjson_CopyString(char const* original)
{
    return ElementJSONImpl::CopyString(original) ;
}

/*************************************************************
* @brief    Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*           You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string     The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length     The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* soarjson_CopyBuffer(char const* original, int length)
{
    return ElementJSONImpl::CopyBuffer(original, length) ;
}

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name must remain in scope for the life of this object.
*       In practice, this generally means it must be a static constant.
*
* @param attributeName  Attribute name can only contain letters, numbers, . - and _.
* @param attributeValue Can be any string.
* @param  copyValue     If true, atttributeName will be copied.  If false, we take ownership of attributeValue
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool soarjson_AddAttributeFast(ElementJSON_Handle hJSON, char const* attributeName, char* attributeValue, bool copyValue)
{
    return GetElementFromHandle(hJSON)->AddAttributeFast(attributeName, attributeValue, copyValue) ;
}

/*************************************************************
* @brief Adds an attribute name-value pair.
*
* NOTE: The attribute name and value must remain in scope for the life of this object.
*       In practice, this generally means it must be a static constant.
*
* @param attributeName  Attribute name can only contain letters, numbers, . - and _.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool soarjson_AddAttributeFastFast(ElementJSON_Handle hJSON, char const* attributeName, char const* attributeValue)
{
    return GetElementFromHandle(hJSON)->AddAttributeFastFast(attributeName, attributeValue) ;
}

/*************************************************************
* @brief Set the tag name for this element.
*
* NOTE: The caller must ensure that the tag name does not go out of scope
* before this object is destroyed.  This requirement means the tag name
* should generally be declared as a static constant.
*
* @param  tagName   Tag name can only contain letters, numbers, . - and _.
* @returns  true if the tag name is valid.
*************************************************************/
bool soarjson_SetTagNameFast(ElementJSON_Handle hJSON, char const* tagName)
{
    return GetElementFromHandle(hJSON)->SetTagNameFast(tagName) ;
}

////////////////////////////////////////////////////////////////
//
// Error reporting functions.
//
////////////////////////////////////////////////////////////////
int soarjson_GetLastError(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetLastError() ;
}

char const* soarjson_GetLastErrorDescription(ElementJSON_Handle hJSON)
{
    return GetElementFromHandle(hJSON)->GetLastErrorDescription() ;
}

/*************************************************************
* @brief Parse an JSON document from a (long) string and return an ElementJSON object
*        for the document.
*
* @param  pString   The JSON document stored in a string.
* @returns NULL if parsing failed, otherwise the ElementJSON representing JSON doc
*************************************************************/
ElementJSON_Handle soarjson_ParseJSONFromString(char const* pString)
{
    if (!pString)
    {
        return NULL ;
    }
    
    ParseJSONString parser(pString, 0) ;
    ElementJSONImpl* pJSON = parser.ParseElement() ;
    
    if (!pJSON)
    {
        s_LastParseErrorMessage = parser.GetErrorMessage() ;
    }
    
    return reinterpret_cast<ElementJSON_Handle>(pJSON) ;
}

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
ElementJSON_Handle soarjson_ParseJSONFromStringSequence(char const* pString, size_t startPos, size_t* endPos)
{
    if (!pString || !endPos)
    {
        return NULL ;
    }
    
    ParseJSONString parser(pString, startPos) ;
    
    ElementJSONImpl* pJSON = parser.ParseElement() ;
    
    *endPos = parser.getEndPosition() ;
    
    if (!pJSON)
    {
        s_LastParseErrorMessage = parser.GetErrorMessage() ;
    }
    
    return reinterpret_cast<ElementJSON_Handle>(pJSON) ;
}

/*************************************************************
* @brief Parse an JSON document from a file and return an ElementJSON object
*        for the document.
*
* @param  pFilename The file to load
* @returns NULL if parsing failed, otherwise the ElementJSON representing JSON doc
*************************************************************/
ElementJSON_Handle soarjson_ParseJSONFromFile(char const* pFilename)
{
    if (!pFilename)
    {
        return NULL ;
    }
    
    FILE* pFile = fopen(pFilename, "rb") ;
    
    if (!pFile)
    {
        s_LastParseErrorMessage = "File " + std::string(pFilename) + " could not be opened" ;
        return NULL ;
    }
    
    ParseJSONFile parser(pFile) ;
    ElementJSONImpl* pJSON = parser.ParseElement() ;
    
    fclose(pFile) ;
    
    if (!pJSON)
    {
        s_LastParseErrorMessage = parser.GetErrorMessage() ;
    }
    
    return reinterpret_cast<ElementJSON_Handle>(pJSON) ;
}

/*************************************************************
* @brief Returns an error message describing reason for error in last parse.
*
* Call here if the parsing functions return NULL to find out what went wrong.
*************************************************************/
char const* soarjson_GetLastParseErrorDescription()
{
    return s_LastParseErrorMessage.c_str() ;
}
