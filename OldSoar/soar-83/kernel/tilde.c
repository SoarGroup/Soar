/*************************************************************************
 *
 *  file:  tilde.c
 *
 * =======================================================================
 * tilde.c -- Tilde expansion code (~/foo := $HOME/foo). 
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
 *
 * Copyright 1995-2003 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */


/* This following 22 lines were included from sysdep-norm.h */
/* System-dependent stuff, for ``normal'' systems */

#ifdef MSDOS
#define __MSDOS__
#endif

#ifdef __POWERPC__ /* This is a hack, but I don't know any better way to know
					  that we're on a Mac. */
#define MACINTOSH
#endif

#ifdef THINK_C
#define __MSDOS__
#include <string.h>
#include <types.h>
#else
#ifdef __SC__
#define __MSDOS__
#include <string.h>
#include <types.h>
#else
#if !defined(MACINTOSH)
#include <sys/types.h>			/* Needed by dirent.h */
#endif
#endif
#endif

#if defined (USG) && defined (TIOCGWINSZ)
#include <sys/stream.h>
#if defined (USGr4) || defined (USGr3)
#include <sys/ptem.h>
#endif /* USGr4 */
#endif /* USG && TIOCGWINSZ */

#ifndef __SC__
#ifndef THINK_C

/* The following dirent checks come from the GNU */
/* autoconf 2.0 documentation                    */

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <sys/ndir.h>
# endif
#endif

#endif /* THINK_C */
#endif /* __SC__  */

typedef struct dirent dirent;

#ifndef __MSDOS__
#ifdef __hpux
#ifndef _INCLUDE_POSIX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#endif
#endif /* __hpux */
#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <pwd.h>
#endif /* !__SC__ && !THINK_C && !WIN32 */
#ifdef __hpux
#undef _INCLUDE_POSIX_SOURCE
#endif /* __hpux */
#endif /* __MSDOS__ */

#include <stdlib.h>

#ifndef savestring
#define savestring(x) (char *)strcpy (xmalloc (1 + strlen (x)), (x))
#endif

typedef int Function ();
#if !defined (NULL)
#  define NULL 0x0
#endif

/* Changed the order of the following test from the GDB source! */

#if defined (TEST)
extern char *xmalloc (), *xrealloc ();
#else
static char *xmalloc (), *xrealloc ();
#endif /* TEST */

extern int strncmp( const char *string1, const char *string2, size_t count );
extern char *strncpy( char *strDest, const char *strSource, size_t count );
extern void print (char *format, ... );
extern size_t strlen( const char *string );
extern char *strcpy( char *strDestination, const char *strSource );
extern char *strcat( char *strDestination, const char *strSource );

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

char *tilde_expand (char *filename) {

char *result, *tilde_expand_word ();
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
        result = (char *)xmalloc  (        1 + result_size);
      else
        result = (char *)xrealloc (result, 1 + result_size);
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
    tilde_word = (char *)xmalloc (1 + end);
    strncpy (tilde_word, filename, end);
    tilde_word[end] = '\0';
    filename += end;
    expansion = tilde_expand_word (tilde_word);
    free (tilde_word);
    len = strlen (expansion);
    if ((result_index + len + 1) > result_size)
      result = (char *)xrealloc (result, 1 + (result_size += (len + 20)));
    strcpy (result + result_index, expansion);
    result_index += len;
    free (expansion);
  }
  result[result_index] = '\0';
  return (result);
}

/* Do the work of tilde expansion on FILENAME.  FILENAME starts with a
   tilde.  If there is no expansion, call tilde_expansion_failure_hook. */

char *tilde_expand_word (char *filename) {
  
  char *dirname = filename ? savestring (filename) : (char *)NULL;

  if (dirname && *dirname == '~') {
    char *temp_name;
    if (!dirname[1] || dirname[1] == '/') {
      /* Prepend $HOME to the rest of the string. */
      char *temp_home = (char *)getenv ("HOME");

      temp_name = (char *)xmalloc (1 + strlen (&dirname[1])
                                   + (temp_home? strlen (temp_home) : 0));
      temp_name[0] = '\0';
      if (temp_home)
        strcpy (temp_name, temp_home);
      strcat (temp_name, &dirname[1]);
      free (dirname);
      dirname = savestring (temp_name);
      free (temp_name);
    } else {

#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH)
    struct passwd *getpwnam (), *user_entry;
#endif

    char *username = (char *)xmalloc (257);
    int i, c;

    for (i = 1; c = dirname[i]; i++) {
      if (c == '/')
        break;
      else
        username[i - 1] = c;
    }
    username[i - 1] = '\0';

#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH)
    if (!(user_entry = getpwnam (username))) {
	      
      /* If the calling program has a special syntax for
	 expanding tildes, and we couldn't find a standard
	 expansion, then let them try. */
#endif
      if (tilde_expansion_failure_hook) {
        char *expansion;

        expansion = (char *)(*tilde_expansion_failure_hook) (username);
        if (expansion) {
          temp_name = (char *)xmalloc (1 + strlen (expansion)
                                         + strlen (&dirname[i]));
          strcpy (temp_name, expansion);
          strcat (temp_name, &dirname[i]);
          free (expansion);
          goto return_name;
        }
      }
      /* We shouldn't report errors. */
#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH)
    } else {
      temp_name = (char *)xmalloc(1 + strlen (user_entry->pw_dir)
                                    + strlen (&dirname[i]));
      strcpy (temp_name, user_entry->pw_dir);
      strcat (temp_name, &dirname[i]);
#endif
      return_name:
      free (dirname);
      dirname = savestring (temp_name);
      free (temp_name);
#if !defined(__MSDOS__) && !defined(WIN32) && !defined(MACINTOSH)
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
    result = tilde_expand (line);
    printf ("  --> %s\n", result);
    free (result);
  }
  exit (0);
}

/* Moved the following #endif up from the end of the file in GDB! */

#endif /* TEST */

static void memory_error_and_abort ();

static char *xmalloc (int bytes) {

  char *temp = (char *)malloc ((size_t)bytes);

  if (!temp)
    memory_error_and_abort ();
  return (temp);
}

static char *xrealloc (char *pointer, int bytes) {
 
 char *temp;

  if (!pointer)
    temp = (char *)malloc ((size_t)bytes);
  else
    temp = (char *)realloc (pointer, (size_t)bytes);

  if (!temp)
    memory_error_and_abort ();
  return (temp);
}

static void memory_error_and_abort () {
 
/*  was: fprintf (stderr, "readline: Out of virtual memory!\n"); */
  print ("readline: Out of virtual memory!\n"); 
  abort ();
}

/*
 * Local variables:
 * compile-command: "gcc -g -DTEST -o tilde tilde.c"
 * end:
 */

