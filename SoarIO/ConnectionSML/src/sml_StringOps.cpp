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

#include <string.h>
#include <stdlib.h>

#include "sml_StringOps.h"

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

	return (stricmp(pStr1, pStr2) == 0) ;
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

	// The 10 here is the base.
	return ltoa(value, buffer, 10) ;
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
