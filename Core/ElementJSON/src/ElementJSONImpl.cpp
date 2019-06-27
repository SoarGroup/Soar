#include "portability.h"

/////////////////////////////////////////////////////////////////
// ElementJSONImpl class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// This library is responsible for representing an JSON document as an object (actually a tree of objects).
//
// A client can send a stream of JSON data which this class parses to create the object representation of the JSON.
// Or the client can call to this library directly, creating the object representation without ever producing the actual
// JSON output (this is just for improved efficiency when the client and the Soar kernel are embedded in the same process).
//
// This class will not support the full capabilities of JSON which is now a complex language.
// It will support just the subset that is necessary for SML (Soar Markup Language) which is intended to be its primary customer.
//
// This class is closely related to the "ElementJSON" class that a client would use.
// But the implementation of that class resides in this class, which is in a different DLL.
// (Passing a pointer to an object between DLLs is dangerous--both DLLs would need to have been
//  built at the same time, with the same class header file etc.  Passing a pointer to an object
//  that is owned by a separate DLL is safe, because that single DLL (ElementJSON.dll in this case)
//  is the only one that really access the data in the class).
/////////////////////////////////////////////////////////////////

#include "ElementJSONImpl.h"

#include <algorithm>    // For "for_each"

#ifndef HAVE_ATOMICS
bool global_locks_initialized = false;
static const size_t NUM_LOCKS = 16;

// define NUM_BITS
#if defined(_WIN64) || defined(__LP64__)
static const size_t NUM_BITS = 4;
#else
static const size_t NUM_BITS = 3;
#endif // define NUM_BITS

#ifdef _MSC_VER
//#define DEBUG_TRY 1
CRITICAL_SECTION global_locks[NUM_LOCKS];
#else // !_MSC_VER
#include <pthread.h>
pthread_mutex_t global_locks[NUM_LOCKS];
#endif // !_MSC_VER

//#define DEBUG_LOCKS 1
#if defined(DEBUG_LOCKS)
size_t tickers[NUM_LOCKS];
#endif // DEBUG_LOCKS

#endif // !HAVE_ATOMICS

static inline void elementjson_atomic_init()
{
#ifndef HAVE_ATOMICS
    if (!global_locks_initialized)
    {
        for (size_t i = 0; i < NUM_LOCKS; ++i)
        {
#ifdef _MSC_VER
            //tickers[i] = 0;
            InitializeCriticalSection(&(global_locks[i]));
#else // !_MSC_VER
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutex_init(&(global_locks[i]), &attr);
#endif // !_MSC_VER
        }
        global_locks_initialized = true;
    }
#endif // !HAVE_ATOMICS
}

static inline long elementjson_atomic_inc(volatile long*  v)
{
#ifndef HAVE_ATOMICS
    uintptr_t i = reinterpret_cast<uintptr_t>(v);
    i >>= NUM_BITS;
    i %= NUM_LOCKS;
    
#ifdef _MSC_VER
#ifdef DEBUG_TRY
    if (TryEnterCriticalSection(&(global_locks[i])) == 0)
#endif // DEBUG_TRY
        // with DEBUG_TRY defined, setting a breakpoint on this line will stop things
        // right when two threads are trying to access the ref count at the same time
        EnterCriticalSection(&(global_locks[i]));
#else // _MSC_VER
#ifdef DEBUG_TRY
    if (pthread_mutex_trylock(&(global_locks[i])) != 0)
#endif // DEBUG_TRY
        // with DEBUG_TRY defined, setting a breakpoint on this line will stop things
        // right when two threads are trying to access the ref count at the same time
        pthread_mutex_lock(&(global_locks[i]));
#endif // _MSC_VER
        
#if defined(DEBUG_LOCKS)
    tickers[i] += 1;
    std::cout << "tickers[";
    for (size_t j = 0; j < NUM_LOCKS; ++j)
    {
        std::cout << tickers[j] << ",";
    }
    std::cout << "]" << std::endl;
#endif // DEBUG_LOCKS
    *v = atomic_inc(v);
    
#ifdef _MSC_VER
    LeaveCriticalSection(&(global_locks[i]));
#else // !_MSC_VER
    pthread_mutex_unlock(&(global_locks[i]));
#endif // !_MSC_VER
    return *v;
#else // HAVE_ATOMICS
    return atomic_inc(v);
#endif // !HAVE_ATOMICS
}

static inline long elementjson_atomic_dec(volatile long* v)
{
#ifndef HAVE_ATOMICS
    uintptr_t i = reinterpret_cast<uintptr_t>(v);
    i >>= NUM_BITS;
    i %= NUM_LOCKS;
    
#ifdef _MSC_VER
#ifdef DEBUG_TRY
    if (TryEnterCriticalSection(&(global_locks[i])) == 0)
#endif // DEBUG_TRY
        // with DEBUG_TRY defined, setting a breakpoint on this line will stop things
        // right when two threads are trying to access the ref count at the same time
        EnterCriticalSection(&(global_locks[i]));
#else // _MSC_VER
#ifdef DEBUG_TRY
    if (pthread_mutex_trylock(&(global_locks[i])) != 0)
#endif // DEBUG_TRY
        // with DEBUG_TRY defined, setting a breakpoint on this line will stop things
        // right when two threads are trying to access the ref count at the same time
        pthread_mutex_lock(&(global_locks[i]));
#endif // _MSC_VER
        
    *v = atomic_dec(v);
    
#ifdef _MSC_VER
    LeaveCriticalSection(&(global_locks[i]));
#else // !_MSC_VER
    pthread_mutex_unlock(&(global_locks[i]));
#endif // !_MSC_VER
    return *v;
#else // HAVE_ATOMICS
    return atomic_dec(v);
#endif // HAVE_ATOMICS
}

using namespace soarjson;

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

static char const* kCDataStart  = "<!CDATA" ;
static char const* kCDataEnd    = "]]>" ;

static char const* kStartTagOpen  = "<" ;
static char const* kStartTagClose = ">" ;
static char const* kEndTagOpen    = "</" ;
static char const* kEndTagClose   = ">" ;

static char const* kEquals        = "=" ;
static char const* kSpace         = " " ;
static char const* kQuote         = "\"" ;

static char const* kNewLine       = "\n" ;

static char const* kEncodeHex = "bin_encoding=\"hex\"" ;

static const int kLenLT   = static_cast<int>(strlen(kLT)) ;
static const int kLenGT   = static_cast<int>(strlen(kGT)) ;
static const int kLenAMP  = static_cast<int>(strlen(kAMP)) ;
static const int kLenQUOT = static_cast<int>(strlen(kQUOT)) ;
static const int kLenAPOS = static_cast<int>(strlen(kAPOS)) ;

static const int kLenCDataStart = static_cast<int>(strlen(kCDataStart)) ;
static const int kLenCDataEnd   = static_cast<int>(strlen(kCDataEnd)) ;

static const int kLenStartTagOpen  = static_cast<int>(strlen(kStartTagOpen)) ;
static const int kLenStartTagClose = static_cast<int>(strlen(kStartTagClose)) ;
static const int kLenEndTagOpen    = static_cast<int>(strlen(kEndTagOpen)) ;
static const int kLenEndTagClose   = static_cast<int>(strlen(kEndTagClose)) ;

static const int kLenNewLine       = static_cast<int>(strlen(kNewLine)) ;

static const int kLenEquals        = static_cast<int>(strlen(kEquals)) ;
static const int kLenSpace         = static_cast<int>(strlen(kSpace)) ;
static const int kLenQuote         = static_cast<int>(strlen(kQuote)) ;

static const int kLenEncodeHex     = static_cast<int>(strlen(kEncodeHex)) ;

//inline static char const* ConvertSpecial(char base)
//{
//    switch (base)
//    {
//        case '<'  :
//            return kLT ;
//        case '>'  :
//            return kGT;
//        case '&'  :
//            return kAMP ;
//        case '\"' :
//            return kQUOT ;
//        case '\'' :
//            return kAPOS ;
//        default:
//            return NULL ;
//    }
//}

inline static int CharLength(char base)
{
    switch (base)
    {
        case '<'  :
            return kLenLT ;
        case '>'  :
            return kLenGT;
        case '&'  :
            return kLenAMP ;
        case '\"' :
            return kLenQUOT ;
        case '\'' :
            return kLenAPOS ;
        default:
            return 1 ;
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
    char* pHexString = ElementJSONImpl::AllocateString(length * 2) ;
    
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
    int stringLength = static_cast<int>(strlen(pHexString)) ;
    
    // The output will be exactly half the length of the input
    length = (stringLength + 1) / 2 ;
    
    // Create the buffer
    pBinaryBuffer = ElementJSONImpl::AllocateString(length) ;
    
    char* pBinary = pBinaryBuffer ;
    
    for (char const* pHex = pHexString ; *pHex != 0 ; pHex += 2)
    {
        char ch = (binaryVal(*pHex) << 4) + binaryVal(*(pHex + 1)) ;
        *pBinary++ = ch ;
    }
}

/*************************************************************
* @brief Counts how long a string will be when expanded as
*        JSON output.  That is allowing for replacing
*        "<" with &lt; etc.
*************************************************************/
inline static int CountJSONLength(jsonStringConst str)
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
*        BUT returns a pointer to the end of the combined string
*        unlike strcat.
*
* Implementation Note:
*    1) Should test the speed against strcat() + strlen().
*       They may be faster (if coded in assembler) than this
*       manual solution.
*************************************************************/
inline static char* AddString(char* pDest, char const* pAdd)
{
    while (*pAdd != NUL)
    {
        *pDest++ = *pAdd++ ;
    }
    
    return pDest ;
}

/*************************************************************
* @brief Adds pAdd to pDest (just like strcat).
*        BUT returns a pointer to the end of the combined string
*        unlike strcat.
*
*        Also, this version replaces special characters with
*        their escape sequence.
*************************************************************/
inline static char* AddJSONString(char* pDest, char const* pAdd)
{
    char ch = *pAdd++ ;
    while (ch != NUL)
    {
        switch (ch)
        {
            case '<'  :
                pDest = AddString(pDest, kLT) ;
                break ;
            case '>'  :
                pDest = AddString(pDest, kGT) ;
                break ;
            case '&'  :
                pDest = AddString(pDest, kAMP) ;
                break ;
            case '\"' :
                pDest = AddString(pDest, kQUOT) ;
                break ;
            case '\'' :
                pDest = AddString(pDest, kAPOS) ;
                break ;
            default   :
                *pDest++ = ch ;
        }
        
        ch = *pAdd++ ;
    }
    
    return pDest ;
}


/*************************************************************
* @brief JSON ids can only contain letters, numbers, . - and _.
*************************************************************/
bool ElementJSONImpl::IsValidID(jsonStringConst str)
{
    for (char const* p = str ; *p != NUL ; p++)
    {
        if (!((*p >= '0' && *p <= '9')
                || (*p >= 'a' && *p <= 'z')
                || (*p >= 'A' && *p <= 'Z')
                || (*p == '.')
                || (*p == '-')
                || (*p == '_')))
        {
            return false ;
        }
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
ElementJSONImpl::ElementJSONImpl(void)
{
    // Default to not using a CDATA section to store character data
    m_UseCData = false ;
    
    // Assume data is not binary until we are told otherwise.
    m_DataIsBinary     = false ;
    m_BinaryDataLength = 0 ;
    
    m_TagName = NULL ;
    m_CharacterData = NULL ;
    m_ErrorCode = 0 ;
    m_pParent = NULL ;
    
    // Creation of the object creates an initial reference.
    m_RefCount = 1 ;
    
    // Guessing at the approx number of strings we'll store makes adding
    // attributes substantially faster.
    m_StringsToDelete.reserve(20) ;
    
    elementjson_atomic_init();
}

// Provide a static way to call release ref to get round an STL challenge.
static inline void StaticReleaseRef(ElementJSONImpl* pJSON)
{
    pJSON->ReleaseRef() ;
}

/*************************************************************
* @brief Destructor.
*************************************************************/
ElementJSONImpl::~ElementJSONImpl(void)
{
    // Delete the character data
    DeleteString(m_CharacterData) ;
    
    // Delete all of the strings that we own
    // (We store these in a separate list because some elements in
    //  the attribute map may be constants and some strings we need to delete
    //  so we can't walk it and delete its contents).
    
    // Replacing a simple walking of the list with a faster algorithmic method
    // for doing the same thing.
    std::for_each(m_StringsToDelete.begin(), m_StringsToDelete.end(), &ElementJSONImpl::DeleteString) ;
    
    /*
    jsonStringListIter iter = m_StringsToDelete.begin() ;
    
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
    jsonListIter iterChildren = m_Children.begin() ;
    
    while (iterChildren != m_Children.end())
    {
        ElementJSONImpl* pChild = *iterChildren ;
        iterChildren++ ;
    
        pChild->ReleaseRef() ;
    }
    */
}

/*************************************************************
* @brief Returns a copy of this object.
*        Generally, this shouldn't be necessary as ref counting
*        allows multiple clients to point to the same object.
*
*        Call ReleaseRef() on the returned object when you are done with it.
*************************************************************/
ElementJSONImpl* ElementJSONImpl::MakeCopy() const
{
    ElementJSONImpl* pCopy = new ElementJSONImpl() ;
    
    pCopy->m_ErrorCode        = m_ErrorCode ;
    pCopy->m_UseCData         = m_UseCData ;
    
    pCopy->m_RefCount         = 1 ;
    
    // Start with a null parent and override this later
    pCopy->m_pParent          = NULL ;
    
    // Copy the tag name
    pCopy->SetTagName(m_TagName) ;
    
    // Copy the character data
    if (m_DataIsBinary)
    {
        pCopy->SetBinaryCharacterData(m_CharacterData, m_BinaryDataLength) ;
    }
    else
    {
        pCopy->SetCharacterData(m_CharacterData) ;
    }
    
    // Copy the attributes
    for (jsonAttributeMapConstIter mapIter = m_AttributeMap.begin() ; mapIter != m_AttributeMap.end() ; mapIter++)
    {
        jsonStringConst att = mapIter->first ;
        jsonStringConst val = mapIter->second ;
        
        pCopy->AddAttribute(att, val) ;
    }
    
    // Copy all of the children, overwriting the parent field for them so that everything connects up correctly.
    for (jsonListConstIter iterChildren = m_Children.begin() ; iterChildren != m_Children.end() ; iterChildren++)
    {
        ElementJSONImpl* pChild = *iterChildren ;
        ElementJSONImpl* pChildCopy = pChild->MakeCopy() ;
        pChildCopy->m_pParent = pCopy ;
        pCopy->AddChild(pChildCopy) ;
    }
    
    return pCopy ;
}

/*************************************************************
* @brief Release our reference to this object, possibly
*        causing it to be deleted.
*************************************************************/
int ElementJSONImpl::ReleaseRef()
{
    // Have to store this locally, before we call "delete this"
    volatile long refCount = elementjson_atomic_dec(&m_RefCount);
    
    if (refCount == 0)
    {
        delete this ;
    }
    
    return static_cast<int>(refCount) ;
}

/*************************************************************
* @brief Add a new reference to this object.
*        The object will only be deleted after calling
*        releaseRef() one more time than addRef() has been
*        called.
*************************************************************/
int ElementJSONImpl::AddRef()
{
    elementjson_atomic_inc(&m_RefCount);
    
    return static_cast<int>(m_RefCount) ;
}

/*************************************************************
* @returns Reports the current reference count (must be > 0)
*************************************************************/
int ElementJSONImpl::GetRefCount()
{
    return static_cast<int>(m_RefCount) ;
}

////////////////////////////////////////////////////////////////
//
// Tag functions (e.g the tag in <name>...</name> is "name")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the tag name for this element.
*
* NOTE: The tagName becomes owned by this JSON object and will be deleted
* when it is destroyed.  It should be allocated with either allocateString() or copyString().
*
* @param  tagName   Tag name can only contain letters, numbers, . - and _.
* @returns  true if the tag name is valid.
*************************************************************/
bool ElementJSONImpl::SetTagName(char* tagName, bool copyName)
{
    // Decide if we're taking ownership of this string or not.
    if (copyName)
    {
        tagName = CopyString(tagName) ;
    }
    
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
* @param  tagName   Tag name can only contain letters, numbers, . - and _.
* @returns  true if the tag name is valid.
*************************************************************/
bool ElementJSONImpl::SetTagNameFast(char const* tagName)
{
#ifdef DEBUG
    if (!IsValidID(tagName))
    {
        return false ;
    }
#endif
    
    m_TagName = tagName ;
    
    return true ;
}

/*************************************************************
* @brief Gets the tag name for this element.
*
* @returns The tag name.
*************************************************************/
char const* ElementJSONImpl::GetTagName() const
{
    return m_TagName ;
}

////////////////////////////////////////////////////////////////
//
// Child element functions.
//
// These allow a single ElementJSONImpl object to represent a complete
// JSON document through its children.
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Adds a child to the list of children of this element.
*
* @param  pChild    The child to add.  Will be released when the parent is destroyed.
*************************************************************/
void ElementJSONImpl::AddChild(ElementJSONImpl* pChild)
{
    if (pChild == NULL)
    {
        return ;
    }
    
    pChild->m_pParent = this ;
    this->m_Children.push_back(pChild) ;
}

/*************************************************************
* @brief Returns the number of children of this element.
*************************************************************/
int ElementJSONImpl::GetNumberChildren() const
{
    return static_cast<int>(this->m_Children.size()) ;
}

/*************************************************************
* @brief Returns the n-th child of this element.
*
* Children are guaranteed to be returned in the order they were added.
* If index is out of range returns NULL.
*
* @param index  The 0-based index of the child to return.
*************************************************************/
ElementJSONImpl const* ElementJSONImpl::GetChild(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_Children.size()))
    {
        return NULL ;
    }
    
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
ElementJSONImpl const* ElementJSONImpl::GetParent() const
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
* @param attributeName  Attribute name can only contain letters, numbers, . - and _.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool ElementJSONImpl::AddAttribute(char* attributeName, char* attributeValue, bool copyName, bool copyValue)
{
    // Decide if we're taking ownership of this string or not.
    if (copyName)
    {
        attributeName = CopyString(attributeName) ;
    }
    
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
* @param attributeName  Attribute name can only contain letters, numbers, . - and _.
* @param attributeValue Can be any string.
* @returns true if attribute name is valid (debug mode only)
*************************************************************/
bool ElementJSONImpl::AddAttributeFast(char const* attributeName, char* attributeValue, bool copyValue)
{
    // Decide if we're taking ownership of this string or not.
    if (copyValue)
    {
        attributeValue = CopyString(attributeValue) ;
    }
    
    // In this version of the call, we only own the value.
    m_StringsToDelete.push_back(attributeValue) ;
    
#ifdef DEBUG
    // Run this test after we've added it to list of strings to delete,
    // otherwise we'd leak it.
    if (!IsValidID(attributeName))
    {
        return false ;
    }
#endif
    
    m_AttributeMap[attributeName] = attributeValue ;
    
    return true ;
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
bool ElementJSONImpl::AddAttributeFastFast(char const* attributeName, char const* attributeValue)
{
#ifdef DEBUG
    // Run this test after we've added it to list of strings to delete,
    // otherwise we'd leak it.
    if (!IsValidID(attributeName))
    {
        return false ;
    }
#endif
    
    m_AttributeMap[attributeName] = attributeValue ;
    
    return true ;
}

/*************************************************************
* @brief Get the number of attributes attached to this element.
*************************************************************/
int ElementJSONImpl::GetNumberAttributes() const
{
    return static_cast<int>(m_AttributeMap.size()) ;
}

/*************************************************************
* @brief Get the name of the n-th attribute of this element.
*        Attributes may not be returned in the order they were added.
*
* @param index  The 0-based index of the attribute to return.
* @returns NULL if index is out of range.
*************************************************************/
const char* ElementJSONImpl::GetAttributeName(int index) const
{
    jsonAttributeMapConstIter mapIter = m_AttributeMap.begin() ;
    
    while (mapIter != m_AttributeMap.end())
    {
        if (index == 0)
        {
            jsonStringConst att = mapIter->first ;
            
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
* @param index  The 0-based index of the attribute to return.
*************************************************************/
const char* ElementJSONImpl::GetAttributeValue(int index) const
{
    jsonAttributeMapConstIter mapIter = m_AttributeMap.begin() ;
    
    while (mapIter != m_AttributeMap.end())
    {
        if (index == 0)
        {
            jsonStringConst val = mapIter->second ;
            
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
* @param attName    The name of the attribute to look up.
* @returns The value of the named attribute (or null if this attribute doesn't exist).
*************************************************************/
const char* ElementJSONImpl::GetAttribute(const char* attName) const
{
    // Note: We can't use the apparently simpler "return m_AttributeMap[attName]" for two reasons.
    // First, this will create an object in the map if one didn't exist before (which we don't want) and
    // Second, there is no const version of [] so we can't do this anyway (it's not const because of the first behavior...)
    
    // Use find to locate the object not [].
    jsonAttributeMapConstIter iter = m_AttributeMap.find(attName) ;
    
    if (iter == m_AttributeMap.end())
    {
        return NULL ;
    }
    
    return iter->second ;
}

////////////////////////////////////////////////////////////////
//
// Character data functions (e.g the character data in <name>Albert Einstein</name> is "Albert Einstein")
//
////////////////////////////////////////////////////////////////

/*************************************************************
* @brief Set the character data for this element.
*
* @param characterData  The character data passed in should *not* replace special characters such as < and &
*                       with the JSON escape sequences &lt; etc.
*                       These values will be converted when the JSON stream is created.
*************************************************************/
void ElementJSONImpl::SetCharacterData(char* characterData, bool copyData)
{
    // Decide if we're taking ownership of this string or not.
    if (copyData)
    {
        characterData = CopyString(characterData) ;
    }
    
    if (this->m_CharacterData != NULL)
    {
        DeleteString(this->m_CharacterData) ;
    }
    
    this->m_CharacterData = characterData ;
    this->m_DataIsBinary = false ;
}

void ElementJSONImpl::SetBinaryCharacterData(char* characterData, int length, bool copyData)
{
    // Decide if we're taking ownership of this string or not.
    if (copyData)
    {
        characterData = CopyBuffer(characterData, length) ;
    }
    
    if (this->m_CharacterData != NULL)
    {
        DeleteString(this->m_CharacterData) ;
    }
    
    this->m_CharacterData    = characterData ;
    this->m_DataIsBinary     = true ;
    this->m_BinaryDataLength = length ;
}

/*************************************************************
* @brief Get the character data for this element.
*
* @returns  Returns the character data for this element.  If the element has no character data, returns zero-length string.
*           The character data returned will not include any JSON escape sequences (e.g. &lt;).
*           It will include the original special characters (e.g. "<").
*************************************************************/
char const* ElementJSONImpl::GetCharacterData() const
{
    if (this->m_CharacterData)
    {
        return this->m_CharacterData;
    }
    return "";
}

/*************************************************************
* @brief Returns true if the character data should be treated as a binary buffer
*        rather than a null-terminated character string.
*************************************************************/
bool ElementJSONImpl::IsCharacterDataBinary() const
{
    return this->m_DataIsBinary ;
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
bool ElementJSONImpl::ConvertCharacterDataToBinary()
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
*        characters (hex for now, or base64 later)
*        which can be safely stored in JSON text.
*************************************************************/
bool ElementJSONImpl::ConvertBinaryDataToCharacters()
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
int  ElementJSONImpl::GetCharacterDataLength() const
{
    if (!m_CharacterData)
    {
        return 0 ;
    }
    
    if (m_DataIsBinary)
    {
        return this->m_BinaryDataLength ;
    }
    else
    {
        return static_cast<int>(strlen(m_CharacterData)) + 1 ;
    }
}

/*************************************************************
* @brief Setting this value to true indicates that this elements character data should be stored in a CDATA section.
*        By default this value will be false.
*
* @param useCData   true if this elements character data should be stored in a CDATA section.
*************************************************************/
void ElementJSONImpl::SetUseCData(bool useCData)
{
    this->m_UseCData = useCData ;
}

/*************************************************************
* @brief Returns true if this element's character data should be stored in a CDATA section when streamed to JSON.
*************************************************************/
bool ElementJSONImpl::GetUseCData() const
{
    return this->m_UseCData ;
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
char* ElementJSONImpl::GenerateJSONString(bool includeChildren, bool insertNewLines) const
{
    // Work out how much space we will need
    int len = DetermineJSONStringLength(0, includeChildren, insertNewLines) ;
    
    // Allocate a string that big
    char* pStr = AllocateString(len) ;
    
    // Fill it in
    char* pEnd = GenerateJSONString(0, pStr, len, includeChildren, insertNewLines) ;
    
    // Terminate the string
    *pEnd = NUL ;
    
    // Return it all
    return pStr ;
}

/*************************************************************
* @brief Returns the length of string needed to represent this object.
*
* @param depth              How deep into the JSON tree we are (can be used to indent)
* @param includeChildren    Includes all children in the JSON output.
* @param insertNewlines     Add newlines to space out the tags to be more human-readable
*************************************************************/
int ElementJSONImpl::DetermineJSONStringLength(int depth, bool includeChildren, bool insertNewLines) const
{
    int len = 0 ;
    
    if (insertNewLines)
    {
        len += depth ;
    }
    
    // The start tag
    if (m_TagName)
    {
        len += kLenStartTagOpen + static_cast<int>(strlen(this->m_TagName)) + kLenStartTagClose ;
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
            len += kLenCDataStart + static_cast<int>(strlen(m_CharacterData)) + kLenCDataEnd ;
        }
        else
        {
            len += CountJSONLength(m_CharacterData) ;
        }
    }
    
    jsonAttributeMapConstIter mapIter = m_AttributeMap.begin() ;
    
    while (mapIter != m_AttributeMap.end())
    {
        jsonStringConst att = mapIter->first ;
        jsonStringConst val = mapIter->second ;
        
        // The attribute name is an identifier and can't contain special chars
        len += kLenSpace + static_cast<int>(strlen(att)) + kLenEquals ;
        
        // The value can contain special chars
        len += kLenQuote + CountJSONLength(val) + kLenQuote ;
        
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
        jsonListConstIter iter = m_Children.begin() ;
        
        while (iter != m_Children.end())
        {
            ElementJSONImpl const* pChild = *iter ;
            len += pChild->DetermineJSONStringLength(depth + 1, includeChildren, insertNewLines) ;
            
            iter++ ;
        }
    }
    
    // If we added a new line before children, need to indent again after children
    if (addedNewLine)
    {
        len += depth ;
    }
    
    // The end tag
    if (m_TagName)
    {
        len += kLenEndTagOpen + static_cast<int>(strlen(this->m_TagName)) + kLenEndTagClose ;
    }
    
    if (insertNewLines)
    {
        len += kLenNewLine ;
    }
    
    return len ;
}

/*************************************************************
* @brief Converts the JSON object to a string.
*
* @param depth              How deep we are into the JSON tree
* @param pStr               The JSON object is stored in this string.
* @param maxLength          The max length of the string
* @param includeChildren    Includes all children in the JSON output.
* @param insertNewlines     Add newlines to space out the tags to be more human-readable
* @returns Pointer to the end of the string.
*************************************************************/
char* ElementJSONImpl::GenerateJSONString(int depth, char* pStart, int maxLength, bool includeChildren, bool insertNewLines) const
{
    // It's useful for debugging to keep pStart around so we can
    // see the string being formed.
    // pStr will walk down the available memory.
    char* pStr = pStart ;
    
    if (insertNewLines)
    {
        // Indent by depth chars
        for (int i = 0 ; i < depth ; i++)
        {
            pStr = AddString(pStr, " ") ;
        }
    }
    
    pStr = AddString(pStr, kStartTagOpen) ;
    
    // The start tag
    if (m_TagName)
    {
        pStr = AddString(pStr, this->m_TagName) ;
    }
    
    // The attributes
    jsonAttributeMapConstIter mapIter = m_AttributeMap.begin() ;
    
    while (mapIter != m_AttributeMap.end())
    {
        jsonStringConst att = mapIter->first ;
        jsonStringConst val = mapIter->second ;
        
        // The attribute name is an identifier and can't contain special chars
        pStr = AddString(pStr, kSpace) ;
        pStr = AddString(pStr, att) ;
        pStr = AddString(pStr, kEquals) ;
        
        // The value can contain special chars
        pStr = AddString(pStr, kQuote) ;
        pStr = AddJSONString(pStr, val) ;
        pStr = AddString(pStr, kQuote) ;
        
        mapIter++ ;
    }
    
    // We use a special attribute to show this character
    // data is encoded binary data.  Alas there's no real JSON standard for this.
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
            pStr = AddJSONString(pStr, m_CharacterData) ;
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
        jsonListConstIter iter = m_Children.begin() ;
        
        while (iter != m_Children.end())
        {
            ElementJSONImpl const* pChild = *iter ;
            pStr = pChild->GenerateJSONString(depth + 1, pStr, maxLength, includeChildren, insertNewLines) ;
            
            iter++ ;
        }
    }
    
    // If we added a new line before children, need to indent again after children
    if (addedNewLine)
    {
        // Indent by depth chars
        for (int i = 0 ; i < depth ; i++)
        {
            pStr = AddString(pStr, " ") ;
        }
    }
    
    // The end tag
    if (m_TagName)
    {
        pStr = AddString(pStr, kEndTagOpen) ;
        pStr = AddString(pStr, this->m_TagName) ;
        pStr = AddString(pStr, kEndTagClose) ;
    }
    
    if (insertNewLines)
    {
        pStr = AddString(pStr, kNewLine) ;
    }
    
    return pStr ;
}

/*************************************************************
* @brief    Performs an allocation and then copies the contents of the passed in buffer to the newly allocated buffer.
*           You need to use this rather than copyString if copying binary data (because it can contained embedded nulls).
*
* @param string     The buffer to copy.  Passing NULL is valid and returns NULL.
* @param length     The length of the buffer to copy (this exact length will be allocated--no trailing NULL is added).
*************************************************************/
char* ElementJSONImpl::CopyBuffer(char const* original, int length)
{
    if (original == NULL || length <= 0)
    {
        return NULL ;
    }
    
    // This will allocate length bytes
    jsonString str = AllocateString(length - 1) ;
    
    char* q = str ;
    for (const char* p = original ; length > 0 ; length--)
    {
        *q++ = *p++ ;
    }
    
    return str ;
}
