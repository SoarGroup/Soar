#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*******************************************************************************
   Almost all of the functions in this file are almost certainly deprecated,
   with the exception of load_file(), which can be found at the end of the file.
   This is the only function in interface.cpp which is needed by other files.
   As such, the following directive has been added to ensure that everything
   else is ignored. -AJC (8/30/02)
*******************************************************************************/
#ifdef _DEPRECATED_

/*************************************************************************
 *
 *  file:  tilde.cpp
 *
 * =======================================================================
 * tilde.cpp -- Tilde expansion code (~/foo := $HOME/foo). 
 *
 * This file was munged from the GNU GDB software.  The 
 * copyright has been retained to identify the origin of this code.
 * Copyright (C) 1988,1989, 1991 Free Software Foundation, Inc.
 *
 * This file is part of GNU Readline, a library for reading lines
 * of text with interactive input and history editing.
 *
 * Readline is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Readline is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 * =======================================================================
 */

#include "tilde.h"
#include "kernel.h"
#include "print.h"
#include "mem.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef savestring
/* This function is defined in mem.h, but a different version was provided here.
   I'm not sure why, but I have maintained that difference to avoid introducting
   undesirable changes in behavior. */
  #define savestring(thisAgent, x) (char *)strcpy (xmalloc (thisAgent, 1 + strlen (x)), x)
#endif

/* The default value of tilde_additional_prefixes.  This is set to
   whitespace preceding a tilde so that simple programs which do not
   perform any word separation get desired behaviour. */
static char *default_prefixes[] =
  { " ~", "\t~", (char *)NULL };

/* The default value of tilde_additional_suffixes.  This is set to
   whitespace or newline so that simple programs which do not
   perform any word separation get desired behaviour. */
static char *default_suffixes[] =
  { " ", "\n", (char *)NULL };

/* If non-null, this contains the address of a function to call if the
   standard meaning for expanding a tilde fails.  The function is called
   with the text (sans tilde, as in "foo"), and returns a malloc()'ed string
   which is the expansion, or a NULL pointer if there is no expansion. */
Function *tilde_expansion_failure_hook = (Function *)NULL;

/* When non-null, this is a NULL terminated array of strings which
   are duplicates for a tilde prefix.  Bash uses this to expand
   `=~' and `:~'. */
char **tilde_additional_prefixes = default_prefixes;

/* When non-null, this is a NULL terminated array of strings which match
   the end of a username, instead of just "/".  Bash sets this to
   `:' and `=~'. */
char **tilde_additional_suffixes = default_suffixes;

/* Find the start of a tilde expansion in STRING, and return the index of
   the tilde which starts the expansion.  Place the length of the text
   which identified this tilde starter in LEN, excluding the tilde itself. */

static int tilde_find_prefix (char *string, int *len) {

register int i, j, string_len;
register char **prefixes = tilde_additional_prefixes;

  string_len = strlen (string);
  *len = 0;

  if (!*string || *string == '~')
    return (0);

  if (prefixes) {
    for (i = 0; i < string_len; i++) {
      for (j = 0; prefixes[j]; j++) {
	if (strncmp (string + i, prefixes[j], strlen (prefixes[j])) == 0) {
	  *len = strlen (prefixes[j]) - 1;
          return (i + *len);
	}
      }
    }
  }
  return (string_len);
}

/* Find the end of a tilde expansion in STRING, and return the index of
   the character which ends the tilde definition.  */

static int tilde_find_suffix (char *string) {

register int i, j, string_len;

  register char **suffixes = tilde_additional_suffixes;
  string_len = strlen (string);

  for (i = 0; i < string_len; i++) {
    if (string[i] == '/' || !string[i])
      break;
    for (j = 0; suffixes && suffixes[j]; j++) {
      if (strncmp (string + i, suffixes[j], strlen (suffixes[j])) == 0)
        return (i);
    }
  }
  return (i);
}

/* Return a new string which is the result of tilde expanding FILENAME. */

char *tilde_expand (agent* thisAgent, char *filename) {

// changed this function pointer
// to have a char * parameter
// to match the function definition
// included below
char *result, *tilde_expand_word (agent*, char *); 
int result_size, result_index;

  result_size = result_index = 0;
  result = (char *)NULL;

  /* Scan through FILENAME expanding tildes as we come to them. */
  while (1) {
    register int start, end;
    char *tilde_word, *expansion;
    int len;

    /* Make START point to the tilde which starts the expansion. */
    start = tilde_find_prefix (filename, &len);

    /* Copy the skipped text into the result. */
    /* This test is always true the first time, since result_index
       is 0, result_size is 0, and start is >= 0.  So we malloc here.  */
    if ((result_index + start + 1) > result_size) {
      result_size += (start + 20);
      if (result == NULL)
        result = (char *)xmalloc  (thisAgent,         1 + result_size);
      else
        result = (char *)xrealloc (thisAgent, result, 1 + result_size);
    }
    strncpy (result + result_index, filename, start);
    result_index += start;

    /* Advance FILENAME upto the starting tilde. */
    filename += start;

    /* Make END be the index of one after the last character of the
       username. */
    end = tilde_find_suffix (filename);

    /* If both START and END are zero, we are all done. */
    if (!start && !end)
      break;

    /* Expand the entire tilde word, and copy it into RESULT. */
    tilde_word = (char *)xmalloc (thisAgent, 1 + end);
    strncpy (tilde_word, filename, end);
    tilde_word[end] = '\0';
    filename += end;
    expansion = tilde_expand_word (thisAgent, tilde_word);
    free (tilde_word);
    len = strlen (expansion);
    if ((result_index + len + 1) > result_size)
      result = (char *)xrealloc (thisAgent, result, 1 + (result_size += (len + 20)));
    strcpy (result + result_index, expansion);
    result_index += len;
    free (expansion);
  }
  result[result_index] = '\0';
  return (result);
}

/* Do the work of tilde expansion on FILENAME.  FILENAME starts with a
   tilde.  If there is no expansion, call tilde_expansion_failure_hook. */

char *tilde_expand_word (agent* thisAgent, char *filename) {
  
  char *dirname = filename ? savestring (thisAgent, filename) : (char *)NULL;

  if (dirname && *dirname == '~') {
    char *temp_name;
    if (!dirname[1] || dirname[1] == '/') {
      /* Prepend $HOME to the rest of the string. */
      char *temp_home = (char *)getenv ("HOME");

      temp_name = (char *)xmalloc (thisAgent, 1 + strlen (&dirname[1])
                                   + (temp_home? strlen (temp_home) : 0));
      temp_name[0] = '\0';
      if (temp_home)
        strcpy (temp_name, temp_home);
      strcat (temp_name, &dirname[1]);
      free (dirname);
      dirname = savestring (thisAgent, temp_name);
      free (temp_name);
    } else {

/* Originally, these directives read as follows:
   #if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH)
   the last condition was added to make the functionality under Linux the 
   same as under windows, and also to remedy compilation issues with the 
   endpwent function. (g++ generates an undeclared identifier error if I 
   try to use it.) In any case, if you change this back to the way it was,
   be sure to include pwd.h, as it is needed for getpwnam(), endpwent(), 
   etc.
*/
#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH) && !defined(UNIX)
    struct passwd *getpwnam (char *), *user_entry;
#endif

    char *username = (char *)xmalloc (thisAgent, 257);
    int i, c;

    for (i = 1; c = dirname[i]; i++) {
      if (c == '/')
        break;
      else
        username[i - 1] = c;
    }
    username[i - 1] = '\0';

#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH) && !defined(UNIX)
    if (!(user_entry = getpwnam (username))) {
	      
      /* If the calling program has a special syntax for
	 expanding tildes, and we couldn't find a standard
	 expansion, then let them try. */
#endif
      if (tilde_expansion_failure_hook) {
        char *expansion;

		assert(!"This code should never be reached because tilde_expansion_failure_hook is never set to anything but null.");

        expansion = (char *)(*tilde_expansion_failure_hook) (/*username*/); // Username was commented out because of compilation problems
																		    // see previous assertion for more info.
        if (expansion) {
          temp_name = (char *)xmalloc (thisAgent, 1 + strlen (expansion)
                                         + strlen (&dirname[i]));
          strcpy (temp_name, expansion);
          strcat (temp_name, &dirname[i]);
          free (expansion);
          goto return_name;
        }
      }
      /* We shouldn't report errors. */
#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH) && !defined(UNIX)
    } else {
      temp_name = (char *)xmalloc(thisAgent, 1 + strlen (user_entry->pw_dir)
                                    + strlen (&dirname[i]));
      strcpy (temp_name, user_entry->pw_dir);
      strcat (temp_name, &dirname[i]);
#endif
      return_name:
      free (dirname);
      dirname = savestring (thisAgent, temp_name);
      free (temp_name);
#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH) && !defined(UNIX)
    }
    endpwent ();
#endif
    free(username);
    }
  }
  return (dirname);
}

#if defined (TEST)
#undef NULL
#include <stdio.h>

main (int argc, char **argv) {
  
agent* thisAgent;
char *result, line[512];
  int done = 0;
  while (!done) {
    printf ("~expand: ");
    fflush (stdout);
    if (!gets (line))
      strcpy (line, "done");
    if ((strcmp (line, "done") == 0) || 
        (strcmp (line, "quit") == 0) ||
	(strcmp (line, "exit") == 0)) {
      done = 1;
      break;
    }
    result = tilde_expand (thisAgent, line);
    printf ("  --> %s\n", result);
    free (result);
  }
  exit (0);
}

/* Moved the following #endif up from the end of the file in GDB! */

#endif /* TEST */

static void memory_error_and_abort (agent* thisAgent); /* this prototype needed by xmalloc */

static char *xmalloc (agent* thisAgent, int bytes) {

  char *temp = (char *)malloc ((size_t)bytes);

  if (!temp)
    memory_error_and_abort (thisAgent);
  return (temp);
}

static char *xrealloc (agent* thisAgent, char *pointer, int bytes) {
 
 char *temp;

  if (!pointer)
    temp = (char *)malloc ((size_t)bytes);
  else
    temp = (char *)realloc (pointer, (size_t)bytes);

  if (!temp)
    memory_error_and_abort (thisAgent);
  return (temp);
}

static void memory_error_and_abort (agent* thisAgent) {
/*  was: fprintf (stderr, "readline: Out of virtual memory!\n"); */
  print (thisAgent, "readline: Out of virtual memory!\n");
  GenerateErrorXML(thisAgent, "readline: Out of virtual memory!");
  abort ();
}

/*
 * Local variables:
 * compile-command: "gcc -g -DTEST -o tilde tilde.c"
 * end:
 */

#endif /* _DEPRECATED_ */
