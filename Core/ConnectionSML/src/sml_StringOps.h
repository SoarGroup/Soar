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

namespace sml
{

    /*************************************************************
    * @brief Returns true if strings are equal (case sensitive).
    *************************************************************/
    inline bool IsStringEqual(char const* pStr1, char const* pStr2)
    {
        if (pStr1 == NULL || pStr2 == NULL)
        {
            return false ;
        }
        
        return strcmp(pStr1, pStr2) == 0 ;
    }
    
    /*************************************************************
    * @brief Returns true if strings are equal (case insensitive).
    *************************************************************/
    inline bool IsStringEqualIgnoreCase(char const* pStr1, char const* pStr2)
    {
        if (pStr1 == NULL || pStr2 == NULL)
        {
            return false ;
        }
        
        return strcasecmp(pStr1, pStr2) == 0 ;
    }
    
    /*************************************************************
    * @brief A utility function, splits a command line into argument
    *        tokens and stores them in the argumentVector string.
    *************************************************************/
    extern int Tokenize(std::string cmdline, std::vector<std::string>& argumentVector);
    
    /*************************************************************
    * @brief Trim leading whitespace off of a line (for command parsing)
    *************************************************************/
    extern void TrimLeadingWhitespace(std::string& line);
    
    /*************************************************************
    * @brief Trim comments off of a line (for command parsing)
    * @return true on success, false if there is a new-line before a pipe quotation ends
    *************************************************************/
    extern bool TrimComments(std::string& line);
    
}

#endif
