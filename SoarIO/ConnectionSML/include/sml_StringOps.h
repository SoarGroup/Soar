/////////////////////////////////////////////////////////////////
// StringOps
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// It's often useful to abstract over the string operations,
// in case a particular library fails to provide the normal implementation
// or we decide to switch functions later (e.g. from case sensitive to
// case insensitive matching).
//
/////////////////////////////////////////////////////////////////

#ifndef STRING_OPS_H
#define STRING_OPS_H

namespace sml {

/*************************************************************
* @brief Returns true if strings are equal (case sensitive).
*************************************************************/
bool IsStringEqual(char const* pStr1, char const* pStr2) ;

/*************************************************************
* @brief Returns true if strings are equal (case insensitive).
*************************************************************/
bool IsStringEqualIgnoreCase(char const* pStr1, char const* pStr2) ;

/*************************************************************
* @brief Convert int to string.
*		 Minimum buffer size is 25 (for a 64-bit int).
*************************************************************/
const int kMinBufferSize = 25 ;
char* Int2String(long value, char* buffer, int maxChars) ;

/*************************************************************
* @brief Convert double to string.
*************************************************************/
char* Double2String(double value, char* buffer, int maxChars);

/*************************************************************
* @brief Returns a copy of the string.
*		 Some libraries may not have strdup().  If so we
*		 can fix it here.
*************************************************************/
char* StringCopy(char const* pStr) ;

/*************************************************************
* @brief Deletes copied strings
*************************************************************/
void StringDelete(char* pStr) ;

}

#endif
