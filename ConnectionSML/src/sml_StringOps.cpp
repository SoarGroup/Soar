#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// StringOps
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// It's often useful to abstract over the string operations,
// in case a particular library fails to provide the implementation
// in the way we expect.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_STRINGS_H
#include <strings.h>  // strcasecmp
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "sml_StringOps.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

/*************************************************************
* @brief Returns true if strings are equal (case sensitive).
*************************************************************/
bool sml::IsStringEqual(char const* pStr1, char const* pStr2)
{
	if (pStr1 == NULL || pStr2 == NULL)
		return false ;

	return (strcmp(pStr1, pStr2) == 0) ;
}

/*************************************************************
* @brief Returns true if strings are equal (case insensitive).
*************************************************************/
bool sml::IsStringEqualIgnoreCase(char const* pStr1, char const* pStr2)
{
	if (pStr1 == NULL || pStr2 == NULL)
		return false ;

#ifdef HAVE_STRINGS_H
		return (strcasecmp(pStr1, pStr2) == 0);
#else
		return (stricmp(pStr1, pStr2) == 0) ;
#endif
}

/*************************************************************
* @brief Convert int to string.
*************************************************************/
char* sml::Int2String(long value, char* buffer, int maxChars)
{
	// A 64-bit value can be 20 digits, plus one for -ve and one for NULL.
	// Anything less than that is not safe.
	if (maxChars < kMinBufferSize)
	{
		buffer[0] = '0' ;
		return buffer ;
	}

	// Changed from ltoa -> snprintf by voigtjr
	snprintf(buffer, maxChars, "%ld", value);
	buffer[maxChars - 1] = 0; // windows doesn't guarantee null termination
	return buffer;
}

/*************************************************************
* @brief Convert double to string.
*************************************************************/
char* sml::Double2String(double value, char* buffer, int maxChars)
{
	//return gcvt(value, maxChars - 1, buffer) ; // gcvt not portable
	snprintf(buffer, maxChars, "%f", value);
	buffer[maxChars - 1] = 0; // ensure null termination as win32 behavior is unspecified
	return buffer;
}

/*************************************************************
* @brief Returns a copy of the string.
*************************************************************/
char* sml::StringCopy(char const* pStr)
{
	char* pCopy = new char[strlen(pStr)+1] ;
	return strcpy(pCopy, pStr) ;
}

/*************************************************************
* @brief Deletes copied strings
*************************************************************/
void sml::StringDelete(char* pStr)
{
	delete[] pStr ;
}
