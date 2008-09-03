/////////////////////////////////////////////////////////////////
// ClientXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
// This class is used to represent XML messages
// that are being returned to the client.
//
// The implementation is a reduced subset of ElementXML
// (the parts required for getting values without setting them).
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_XML_H
#define SML_CLIENT_XML_H

#include <string>

namespace sml {

class ElementXML ;
class Agent ;
class ClientTraceXML ;

class ClientXML
{
	friend class Agent ;

private:
	ElementXML* m_pElementXML ;

protected:
	// This constructor is protected so that client doesn't try to build these objects.
	// So far they're only built internally and then passed to the client.
	// NOTE: We take ownership of the ElementXML object we are passed and
	// delete it when this client object is deleted.
	ClientXML(ElementXML* pXML) ;

public:
	ClientXML() ;
	virtual ~ClientXML() ;

	/*************************************************************
    * @brief Creates a new reference to the underlying XML object.
	*
	* This allows a caller to "keep a copy" of an object they are
	* passed.
	*************************************************************/
	ClientXML(ClientXML* pXML) ;

	/*************************************************************
    * @brief Cast this object to a subclass.
	*
	* These methods are helpful when we're working in other
	* languages (e.g. Java) through SWIG.  SWIG creates intermediate
	* objects which wrap the C++ object and we want to retain the ability
	* to cast the underlying C++ object.  These methods achieve that.
	*
	* NOTE: These methods always succeed because the subclasses contain no
	* data.  They just contain specific access method.  So we can cast any
	* XML message over to any of these subclasses and if it's not a match
	* when we ask for IsTagX() etc they'll just return false.
	* If the subclasses contained data we'd want this to be a dynamic cast
	* and only make the cast if "this" was of the target type.
	*
    * @returns This object cast to the subclass.
	*************************************************************/
	virtual ClientTraceXML* ConvertToTraceXML() { return (ClientTraceXML*) this ; }

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
	bool GetChild(ClientXML* pChild, int index) const ;

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
    * @brief Converts the XML object to a string.
	*
	* @param includeChildren	Includes all children in the XML output.
	*
	* @returns The string form of the object.
	*************************************************************/
	char* GenerateXMLString(bool includeChildren) const ;

	/*************************************************************
    * @brief Returns the length of string needed to represent this object (does not include the trailing null, so add one for that)
	*	*
	* @param includeChildren	Includes all children in the XML output.
	*************************************************************/
	int DetermineXMLStringLength(bool includeChildren) const ;

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
} ;

} //closes namespace

#endif //SML_CLIENT_XML_H
