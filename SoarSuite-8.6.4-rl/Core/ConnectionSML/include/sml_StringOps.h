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

#include <string>
#include <vector>

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

/*************************************************************
* @brief A utility function, splits a command line into argument
*		 tokens and stores them in the argumentVector string.
*************************************************************/
int Tokenize(std::string cmdline, std::vector<std::string>& argumentVector);

/*************************************************************
* @brief Trim comments off of a line (for command parsing)
* @return true on success, false if there is a new-line before a pipe quotation ends
*************************************************************/
bool Trim(std::string& line);

}

#endif
