/* utilities.cpp

   This file contains various soar utilities
   which are not specifically part of the api.

*/

#include "sk.h"
#include "definitions.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int getInt( char *string, int *i ) {

  long l;
  char *c;

  if ( *string == '\0' ) return SOAR_ERROR;

  l = strtol( string, &c, 10 );

  if ( *c == '\0' ) {
    *i = (int) l;
    return SOAR_OK;
  }

  return SOAR_ERROR;
}

Bool string_match (char * string1, char * string2)
{
  if ((string1 == NULL) && (string2 == NULL))
    return TRUE;

  if (   (string1 != NULL) 
      && (string2 != NULL) 
      && !(strcmp(string1, string2)))
    return TRUE;
  else
    return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * string_match_up_to --
 *
 *	This procedure compares two strings to see if there 
 *      are equal up to the indicated number of positions.
 *
 * Results:
 *	TRUE if the strings match over the given number of
 *      characters, FALSE otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Bool string_match_up_to (char * string1, char * string2, int positions)
{
  int i,num;

  /*  what we really want is to require a match over the length of
      the shorter of the two strings, with positions being a minimum */

  num = strlen(string1);
  if (num > strlen(string2)) num = strlen(string2);
  if (positions < num)  positions = num;
  
  for (i = 0; i < positions; i++)
    {
      if (string1[i] != string2[i]) 
	return FALSE;
    }

  return TRUE;      
}

/* 

  This is a function that I created specifically for use with the test bed.
  parse_command_string expects a string containing the arguments for a
  command (str). It then separates the arguments in this string with NULL
  characters and populates argv with character pointers, each of which
  refers to the first character of one of the arguments in str. argv[0]
  is left untouched. (The calling function may assign to it a pointer to
  the name of the command.) argc is set equal to the total number of
  arguments, including the command itself.

  Note that argc must have an initial value of zero for this function to work.
  Furthermore, argv must be a valid array of character pointers. (No dynamic
  memory allocation is performed here.)

  For example, if str == "-arg1 25 -arg2 72", the contents of argv and argc
  after calling this function will be as follows:

  argv: 
	
	{ ??, "-arg1", "25", "-arg2", "72"};

  argc: 

	5;

*/
void parse_command_string(char * str, char * argv[], int & argc)
{
	if (strlen(str))
	{
		if (!isspace(*str))
			argv[argc++] = str;

		for (int i = 1; i < strlen(str) - 1; i++)
		{               
			if (isspace(str[i]))
			{
				str[i] = '\0';
				if (!isspace(str[i + 1]))
					argv[argc++] = str + i + 1;
			}
		}
	}                           
}
