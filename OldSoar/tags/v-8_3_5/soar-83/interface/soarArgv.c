/*
 * soarArgv.c --
 *
 *	This file contains a procedure that handles table-based
 *	argv-argc parsing.
 *
 *      This file is obsolete when Soar is built as a dynamically
 *      loadable library for Tcl.  (there is no commandline processsing)
 *
 *      The processing in this file was patterned after the tkArgv.c
 *      file in tk3.6.  Extraneous facilities were deleted and additional
 *      facilities were added.  The command line processing also does
 *      not presume that an interpreter and Tk window already exist.
 *      The additional facilities include processing of an environment 
 *      variable for command line arguments.
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


#include <string.h>
#include "soar.h"

/*
 * Forward declarations for procedures defined in this file:
 */

static int ProcessList _ANSI_ARGS_((int * argc, int * argcPtr, char **argv, 
				    char * curArg, Soar_ArgvInfo * infoPtr, 
				    int *srcIndex));
static void PrintSoarUsage _ANSI_ARGS_((Soar_ArgvInfo *argTable));

/*
 *----------------------------------------------------------------------
 *
 * Soar_ParseArgv --
 *
 *	Process an argv array according to a table of expected
 *	command-line options.
 *
 *      This function may be called several times while parsing
 *      the command line.  This is because some options trigger
 *      a return so that the options parsed so far can be processed.
 *      Some option switches can be followed by a sequence of names
 *      rather than alternating switch/value pairs.
 *
 * Results:
 *	The return value is a standard Tcl return value.
 *
 * Side effects:
 *	Depends on the arguments and their entries in argTable.  If an
 *	error occurs then an error message is printed.	Under normal 
 *      conditions, both *argcPtr and *argv are modified to return the 
 *      arguments that couldn't be processed here (they didn't match 
 *      the option table, or followed a SOAR_ARGV_REST argument).
 *
 *----------------------------------------------------------------------
 */

int
Soar_ParseArgv(argcPtr, argv, argTable, flags, srcIndex, dstIndex, 
	       interp_type, options_done)
  int *argcPtr;		          /* Number of arguments in argv.  Modified
			           * to hold # args left in argv at end. */
  char **argv;		          /* Array of arguments.  Modified to hold
			           * those that couldn't be processed here. */
  Soar_ArgvInfo *argTable;        /* Array of option descriptions */
  int flags;			  /* Or'ed combination of various flag bits,
				   * such as SOAR_ARGV_NO_LEFTOVERS. */
  int * srcIndex;                 /* Location from which to read next argument
				   * from argv. */
  int * dstIndex;     	          /* Index into argv to which next unused
				   * argument should be copied (never greater
				   * than srcIndex). */
  int * interp_type;              /* The current interp. type -- used in 
				   processing lists of interps. */
  int * options_done;             /* Flag indicating command line is 
                                     parsed. */
{
  /* Other elements */
  Soar_ArgvInfo *infoPtr;       /* Pointer to the current entry in the
			         * table of argument descriptions. */
  Soar_ArgvInfo *matchPtr;	/* Descriptor that matches current argument. */
  char * curArg;                /* Current argument */
  int argc;                     /* # arguments in argv still to process. */
  int length;			/* Number of characters in current argument. */

  argc = *argcPtr;

  while (argc > 0) {
	curArg = argv[*srcIndex];
/*	printf ("Looking at %s!\n", curArg); */
	(*srcIndex)++;
	argc--;
	length = strlen(curArg);

	/*
	 * Loop throught the argument descriptors searching for one with
	 * the matching key string.  If found, leave a pointer to it in
	 * matchPtr.
	 */

	matchPtr = NULL;
	infoPtr = argTable;

	for (; 
	     (infoPtr != NULL) && (infoPtr->type != SOAR_ARGV_END);
	     infoPtr++) {
	  if (infoPtr->key == NULL) {
	    continue;
	  }
	  /* Match chars in arg to table entry */
	  if (strncmp(infoPtr->key, curArg, length) != 0) {
	    continue;
	  }
	  /* If the arg is the same length as the table entry
	     then no ambiguity is possible so jump to process */
	  
	  if (infoPtr->key[length] == 0) {
	    matchPtr = infoPtr;
	    goto gotMatch;
	  }
	  /* If there was a previous match in an earlier pass,
	     then we have an ambiguity. */
	  if (matchPtr != NULL) {
	    printf("ambiguous option: %s\n", curArg);
	    return TCL_ERROR;
	  }
	  /* Record matching item and scan the rest of the table
	     entries to verify that there is no ambiguity. */
	  matchPtr = infoPtr;
	}

	if (matchPtr == NULL) {

	    /*
	     * Unrecognized argument.  Just copy it down, unless the caller
	     * prefers an error to be registered.
	     */

	    if (flags & SOAR_ARGV_NO_LEFTOVERS) {
	      printf("unrecognized argument: %s\n", curArg);
	      return TCL_ERROR;
	    }
	    argv[*dstIndex] = curArg;
	    (*dstIndex)++;
	    continue;
	}

	/*
	 * Take the appropriate action based on the option type
	 */

	gotMatch:
	infoPtr = matchPtr;
	switch (infoPtr->type) {
	    case SOAR_ARGV_CONSTANT:
		*((int *) infoPtr->dst) = (int) infoPtr->src;
		break;
	    case SOAR_ARGV_STRING:
		if ((argc == 0) || (argv[*srcIndex][0] == '-')) {
		  printf("option %s requires an additional argument\n", 
			 curArg);
		  return TCL_ERROR;
		} else {
		    *((char **)infoPtr->dst) = argv[*srcIndex];
		    (*srcIndex)++;
		    argc--;
		}
		break;
	    case SOAR_ARGV_REST:
		*((int *) infoPtr->dst) = *dstIndex;
		goto argsDone;
	    case SOAR_ARGV_HELP:
		PrintSoarUsage (argTable);
		return TCL_ERROR;
	    case SOAR_ARGV_TCLSH:
		if (ProcessList(&argc, argcPtr, argv, curArg, 
				infoPtr, srcIndex) != TCL_OK) 

		  {
		    return TCL_ERROR;
		  }
		else 
		  {
		    *interp_type = SOAR_ARGV_TCLSH;
		    if (argc == 0) goto argsDone;
		    return TCL_OK;
		  }
	    case SOAR_ARGV_WISH:
		if (ProcessList(&argc, argcPtr, argv, curArg, 
				infoPtr, srcIndex) != TCL_OK) 

		  {
		    return TCL_ERROR;
		  }
		else 
		  {
		    *interp_type = SOAR_ARGV_WISH;
		    if (argc == 0) goto argsDone;
		    return TCL_OK;
		  }
            case SOAR_ARGV_AGENT:
		if (ProcessList(&argc, argcPtr, argv, curArg, 
				infoPtr, srcIndex) != TCL_OK) 

		  {
		    return TCL_ERROR;
		  }
		else 
		  {
		    *interp_type = SOAR_ARGV_AGENT;
		    if (argc == 0) goto argsDone;
		    return TCL_OK;
		  }
	    default:
		printf("bad argument type %d in Soar_ArgvInfo\n",
			infoPtr->type);
		return TCL_ERROR;
	}
      }

    /*
     * If we broke out of the loop because of an OPT_REST argument,
     * copy the remaining arguments down.  We also reach here when
     * argc goes to 0.
     */

    argsDone:
    while (argc) {
	argv[*dstIndex] = argv[*srcIndex];
	(*srcIndex)++;
	(*dstIndex)++;
	argc--;
    }

    argv[*dstIndex] = (char *) NULL;
    *argcPtr = *dstIndex;
    *options_done = 1;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ProcessList --
 *
 *	Scan the command line to pick up a list of items
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The start and end pointers of the list are updated
 *
 *----------------------------------------------------------------------
 */

static int
ProcessList(argc, argcPtr, argv, curArg, infoPtr, srcIndex)
  int * argc;                   /* # arguments in argv still to process. */
  int *argcPtr;		        /* Number of arguments in argv.  Modified
			         * to hold # args left in argv at end. */
  char **argv;		        /* Array of arguments.  Modified to hold
			         * those that couldn't be processed here. */
  char * curArg;                /* Current argument */
  Soar_ArgvInfo *infoPtr;       /* Pointer to the current entry in the
			         * table of argument descriptions. */
  int * srcIndex;               /* Location from which to read next argument
				 * from argv. */
{
  soar_argv_list * sanl = (soar_argv_list *) infoPtr->dst;

  if ((*argc == 0) || (argv[*srcIndex][0] == '-')) 
    {
      printf("option %s requires an additional argument\n", curArg);
      return TCL_ERROR;
    }

  sanl->start_argv = *srcIndex;
  sanl->end_argv = *srcIndex;
  (*srcIndex)++;
  (*argc)--;
  while ((*argc > 0) && (argv[*srcIndex][0] != '-'))
    {
      sanl->end_argv = *srcIndex;
      (*srcIndex)++;
      (*argc)--;
    }
  *argcPtr = *argc;
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * PrintSoarUsage --
 *
 *	Generate a help string describing command-line options.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A help message indicating option usage is printed to stdout.
 *
 *----------------------------------------------------------------------
 */

static void
PrintSoarUsage(argTable)
    Soar_ArgvInfo *argTable;	/* Array of command-specific argument
				 * descriptions. */
{
    Soar_ArgvInfo *infoPtr;
    int width, numSpaces;
#define NUM_SPACES 20
    static char spaces[] = "                    ";

    /*
     * First, compute the width of the widest option key, so that we
     * can make everything line up.
     */

    width = 4;
    for (infoPtr = argTable; infoPtr->type != SOAR_ARGV_END; infoPtr++) {
      int length;
      if (infoPtr->key == NULL) {
	continue;
      }
      length = strlen(infoPtr->key);
      if (length > width) {
	width = length;
      }
    }

    printf("Possible options:");

    for (infoPtr = argTable; infoPtr->type != SOAR_ARGV_END; infoPtr++) {
      /* Print the switch name and a colon */
      printf("\n %s:", infoPtr->key);

      /* Print the spaces after the colon upto the switch description
	 field. */
      numSpaces = width + 1 - strlen(infoPtr->key);
      while (numSpaces > 0) {
	if (numSpaces >= NUM_SPACES) {
	  printf("%s", spaces);
	} else {
	  printf("%s", spaces+NUM_SPACES-numSpaces);
	}
	numSpaces -= NUM_SPACES;
      }
      /* Print the switch description. */
      printf("%s", infoPtr->help);
	  
      /* Print any default value, if present. */
      switch (infoPtr->type) {
      case SOAR_ARGV_STRING: {
	char *string;

	string = *((char **) infoPtr->dst);
	if (string != NULL) {
	  printf("\n\t\tDefault value: \"%s\"", string);
	}
	break;
      }
      default: {
	break;
      }
      }
    }

    printf ("\n");
  }
    
