/* utilities.h

   This file contains various soar utilities
   which are not specifically part of the api.

*/

#ifndef UTILITIES_H
#define UTILITIES_H

#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif

int getInt( char *string, int *i );
Bool string_match (char * string1, char * string2);
Bool string_match_up_to (char * string1, char * string2, int positions);
void parse_command_string(char * str, char * argv[], int & argc);

#endif
