/*
 * =======================================================================
 *  File:  soarCommands.c
 *
 * This file includes the definitions of the Soar Command set.
 *
 * =======================================================================
 *
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
/*----------------------------------------------------------------------
 *
 * PLEASE NOTE!  Only functions implementing commands should appear
 * in this file.  All supporting functions should be placed in 
 * soarCommandUtils.c.
 *
 *----------------------------------------------------------------------
 */

#include "soar.h"
#include "soarCommands.h"
#include "soarCommandUtils.h"
#include "soarScheduler.h"

#if defined(WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define popen(command, mode) _popen((command), (mode))
#define pclose(stream) _pclose(stream)
#else
#include <unistd.h>
#endif

extern void gds_invalid_so_remove_goal(wme* w);

/*
 *----------------------------------------------------------------------
 *
 * AddWmeCmd --
 *
 *      This is the command procedure for the "add-wme" command, 
 *      which adds an element to working memory.
 *
 *      This command surgically modifies Soar's working memory.  
 *      Add-wme adds a new wme with the given id, attribute, value, 
 *      and optional acceptable preference.  The given id must be an 
 *      existing identifier.  If '*' is given in place of the 
 *      attribute or value, Soar creates a new identifier (gensym)
 *      for that field.  
 *
 *      WARNING: this command is inherently unstable and may have 
 *      weird side effects (possibly even including system crashes).  
 *      For example, the chunker can't backtrace through wmes created 
 *      via add-wme.  Removing input wmes or context/impasse wmes may 
 *      have unexpected side effects.  You've been warned.
 *
 *      See also:  remove-wme
 *
 * Syntax:  add-wme id [^] { attribute | '*'} { value | '*' } [+]
 *
 * Results:
 *      Returns a standard Tcl completion code and the timetag of the
 *      created wme (which can be passed to remove-wme).
 *
 * Side effects:
 *      Adds the given wme to working memory.
 *      If capturing input, copies the args to the capture file. (v8.3+)
 *
 *----------------------------------------------------------------------
 */

int AddWmeCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, char *argv[])
{
  static char * too_few_args  = "Too few arguments.\nUsage: add-wme id [^] { attribute | '*'} { value | '*' } [+]";
  static char * too_many_args = "Too many arguments.\nUsage: add-wme id [^] { attribute | '*'} { value | '*' } [+]";

  char *id_str, *attr_str, *val_str; /* for capturing input wmes       */
  Symbol *id, *attr, *value;       /* Symbols used to create the WME */
  bool acceptable_preference;      /* Acceptable preference seen?    */
  wme *w;                          /* The created WME                */
  int next_arg;                    /* Index into argv used to point  */
                                   /* to next command argument       */

  Soar_SelectGlobalInterpByInterp(interp);
  
  if (argc < 4) {
      interp->result = too_few_args;
      return TCL_ERROR;
  }
  
  /* --- get id --- */
  
  if (read_id_or_context_var_from_string(argv[1], &id) 
      == TCL_ERROR)   {
      sprintf(interp->result,
		  "Known id or context var expected in add-wme command, not %s.",
		  argv[1]);
      return TCL_ERROR;
  }
  
  /* save the ID string */
  id_str = argv[1];

  /* --- get optional '^', if present --- */

  /* With Tcl parsing, the ^ could be separate or a prefix of   */
  /* the attribute lexeme.                                      */
  /* Unlike non-Tcl version, don't err if it's missing.              */
  
  if (string_match("^", argv[2]))  {
      next_arg = 3;
  } else {
      next_arg = 2;
      if (*argv[2] == '^') {
		  argv[2]++;                 /* Skip leading ^ */
	  }
  }
  
  /* --- get attribute or '*' --- */
  
  if (string_match("*", argv[next_arg])) {
      attr = make_new_identifier ('I', id->id.level);
  } else {
      get_lexeme_from_string(argv[next_arg]);
	  
      switch (current_agent(lexeme).type) {
	  case SYM_CONSTANT_LEXEME:
		  attr = make_sym_constant (current_agent(lexeme).string); 
		  break;
	  case INT_CONSTANT_LEXEME:
		  attr = make_int_constant (current_agent(lexeme).int_val); 
		  break;
	  case FLOAT_CONSTANT_LEXEME:
		  attr = make_float_constant (current_agent(lexeme).float_val); 
		  break;
	  case IDENTIFIER_LEXEME:
	  case VARIABLE_LEXEME:      
		  attr = read_identifier_or_context_variable();
		  if (!attr) {
			  return TCL_ERROR;
		  }
		  symbol_add_ref (attr);
		  break;
	  default:
		  sprintf (interp->result,
			  "Expected constant, identifier, or '*' for attribute, not %s.",
			  argv[next_arg]);
		  return TCL_ERROR;
	  }
  }

  /* save the attr string (might be a *) */
  attr_str = argv[next_arg];
  
  /* --- get value or '*' --- */

  next_arg++;
  if (next_arg >= argc) {
      symbol_remove_ref (attr); 
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  if (string_match("*", argv[next_arg]))     {
      value = make_new_identifier ('I', id->id.level);
  } else {
      get_lexeme_from_string(argv[next_arg]);
      switch (current_agent(lexeme).type) {
	  case SYM_CONSTANT_LEXEME:
		  value = make_sym_constant (current_agent(lexeme).string); 
		  break;
	  case INT_CONSTANT_LEXEME:
		  value = make_int_constant (current_agent(lexeme).int_val); 
		  break;
	  case FLOAT_CONSTANT_LEXEME:
		  value = make_float_constant (current_agent(lexeme).float_val); 
		  break;
	  case IDENTIFIER_LEXEME:
	  case VARIABLE_LEXEME:
		  value = read_identifier_or_context_variable();
		  if (!value) { 
			  symbol_remove_ref (attr); 
			  return TCL_ERROR; 
		  }
		  symbol_add_ref (value);
		  break;
	  default:
		  sprintf (interp->result,
			  "Expected constant, identifier, or '*' for value, not %s.",
			  argv[next_arg]);
		  symbol_remove_ref (attr);
		  return TCL_ERROR;
	  }
  }

  /* save the value string (might be a *) */
  val_str = argv[next_arg];

  /* --- get optional acceptable preference indicator --- */

  next_arg++;

  if (string_match("+", argv[next_arg]))
    { 
      acceptable_preference = TRUE; 
      next_arg++;
    }
  else if (next_arg != argc)      /* An arg is present and != "+" */
    {
      symbol_remove_ref (attr); 
      symbol_remove_ref (value); 
      sprintf (interp->result,
	       "Only '+' allowed as optional preference, not %s.",
	       argv[next_arg]);
      return TCL_ERROR;      
    }
  else
    {
      acceptable_preference = FALSE;
    }

  if (next_arg < argc)                     /* Not at end of argv */
    {
      symbol_remove_ref (attr); 
      symbol_remove_ref (value); 
      interp->result = too_many_args;
      return TCL_ERROR;      
    }

  /* --- now create and add the wme --- */

  w = make_wme (id, attr, value, acceptable_preference);
  symbol_remove_ref (attr);
  symbol_remove_ref (value);
  insert_at_head_of_dll (id->id.input_wmes, w, next, prev);
  add_wme_to_wm (w);

    
  /* KJC 11/99 begin: */
  /* if input capturing is enabled, save any input wmes to capture file */
  if (current_agent(capture_fileID) && 
	  (current_agent(current_phase) == INPUT_PHASE)) 
	  capture_input_wme(w, id_str, attr_str, val_str, "add");
  /* KJC 11/99 end */


/* REW: begin 28.07.96 */
  /* OK.  This is an ugly hack.  Basically we want to keep track of kernel
     time and callback time.  add-wme is called from either a callback
     (for input routines) or from the command line (or someplace else?).
     Here, I'm just assuming that we'll normally call add-wme from an
     input routine so I turn off the input_function_timer and turn on
     the kernel timers before the call to the low-level function:
     do_buffered_wm_and_ownership_changes.

     This assumption is problematic because anytime add-wme is called 
     from some place other than the input function, there is a potential
     to get some erroneous (if start_kernel_tv wasn't set for the 
     input function) or just bad (if start_kernel_tv isn't defined)
     timing data.   The real problem is that this very high-level 
     routine is going deep into the kernel.  We can either ignore
     this and just call the time spent doing wm changes here time
     spent outside the kernel or we can try to do the accounting,
     what this hack is a first-attempt at doing.  

     However, my testing turned up no problems -- I was able to add
     and remove WMEs without messing up the timers.  So it`s a 
     hack that seems to work.  For now.  (there is a plan to 
     add routines for specifically adding and deleting input
     WMEs which should help clear up this isse)               REW */
 
/* #if 0 /* kjc 12/99 see comments and code below  */
#ifndef NO_TIMING_STUFF
    if (current_agent(current_phase) == INPUT_PHASE) {
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
       stop_timer (&current_agent(start_kernel_tv), 
                   &current_agent(input_function_cpu_time));
       start_timer (&current_agent(start_kernel_tv));
       start_timer (&current_agent(start_phase_tv)); 
    }
#endif
/* REW: end 28.07.96 */

  do_buffered_wm_and_ownership_changes();


/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  if (current_agent(current_phase) == INPUT_PHASE) {
       stop_timer (&current_agent(start_phase_tv), 
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
       stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
       start_timer (&current_agent(start_kernel_tv));
  }
#endif
/* REW: end 28.07.96 */
/* #endif */

#if 0
  /* KJC 12/99 changed timing hack to only be included when add-wme
   * is called from OTHER than the INPUT_PHASE, since the input phase
   * finishes by calling do_buffered_wm_and_ownership_changes and
   * we shouldn't need to do it for every single add-wme during input.
   * But, I'm not sure that starting/stopping the phase timer is 
   * the correct thing to do now either.
   */
 
  if (current_agent(current_phase) != INPUT_PHASE) {

      #ifndef NO_TIMING_STUFF
	  start_timer (&current_agent(start_kernel_tv));
      start_timer (&current_agent(start_phase_tv)); 
      #endif

	  do_buffered_wm_and_ownership_changes();

      #ifndef NO_TIMING_STUFF
      stop_timer (&current_agent(start_phase_tv), 
                  &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
      stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
      start_timer (&current_agent(start_kernel_tv));
      #endif
  }
#endif
  /* --- return wme timetag and final wme as result */

  soar_push_callback((soar_callback_agent) clientData, 
		     PRINT_CALLBACK,
		     (soar_callback_fn) Soar_AppendResult, 
		     (soar_callback_data) NULL,
		     (soar_callback_free_fn) NULL);
  /* if a log file is open, then we need to push a dummy callback
   * so that we don't get extraneous prints mucking up the log file.
   * Addresses bug # 248.  KJC 01/00 */
  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
	  soar_push_callback((soar_callback_agent) clientData, 
		     LOG_CALLBACK,
		     (soar_callback_fn) Soar_DiscardPrint, 
		     (soar_callback_data) NULL,
		     (soar_callback_free_fn) NULL);
  }

  print_wme_for_tcl(w);
 
  soar_pop_callback((soar_callback_agent) clientData, PRINT_CALLBACK);
  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
	  soar_pop_callback((soar_callback_agent) clientData, LOG_CALLBACK);
  }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * AttributePreferencesModeCmd --
 *
 *	This is the command procedure for the "attribute-preferences-mode"
 *      command, which controls the handling of preferences (other than 
 *      acceptable and reject preferences) for non-context slots.  It takes
 *      a single numeric argument: 
 *          0  --> Handle them the normal (Soar 6) way.
 *          1  --> Handle them the normal (Soar 6) way, but print a warning
 *                 message whenever a preference other than + or - is created
 *                 for a non-context slot.
 *          2  --> Whenever a preferences other than + or - is created for a 
 *                 non-context slot, print an error message and ignore
 *                 (discard) that preference.  For non-context slots, the set
 *                 of values installed in working memory is always equal to
 *                 the set of acceptable values minus the set of rejected
 *                 values.
 *
 * Syntax:  attribute-preferences-mode {0 | 1 | 2}
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	Sets current_agent(attribute_preferences_mode).
 *
 *----------------------------------------------------------------------
 */

int AttributePreferencesModeCmd (ClientData clientData, 
				 Tcl_Interp * interp,
				 int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 2)
    {
      interp->result =
        "Too many arguments.\nUsage: attribute-preferences-mode 0|1|2";
      return TCL_ERROR;
    }

  if (argc == 2) {
	 if (current_agent(operand2_mode) && (strcmp(argv[1],"2"))) {
		/* we're in Soar8 mode, but tried setting mode != 2 */
      sprintf(interp->result,
	     "Unallowed argument to %s: %s.\nIn soar8 mode, attribute-preferences-mode is obsolete;\nthe code automatically operates as if attr-pref-mode == 2.",
	      argv[0], argv[1]);
      return TCL_ERROR;
	 }
    if (! strcmp(argv[1], "0")) {
      current_agent(attribute_preferences_mode) = 0;
    } else if (! strcmp(argv[1], "1")) {
      current_agent(attribute_preferences_mode) = 1;
    } else if (! strcmp(argv[1], "2")) {
      current_agent(attribute_preferences_mode) = 2;
    } else {
      sprintf(interp->result,
	      "Unrecognized argument to %s: %s.  Integer 0, 1, or 2 expected.",
	      argv[0], argv[1]);
      return TCL_ERROR;
    }
  }

  sprintf(interp->result, "%d", current_agent(attribute_preferences_mode));
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * CaptureInputCmd --
 *
 *      This is the command procedure for the "capture-input" command
 *      which records input wme commands (add|remove) from the INPUT phase.
 *
 *      This command may be used to start and stop the recording of
 *      input wmes as created by an external simulation.  wmes are
 *      recorded decision cycle by decision cycle.  Use the command
 *      replay-input to replay the sequence.
 *
 * Syntax:  capture-input <action>
 *          <action> ::= -open pathname 
 *          <action> ::= -query
 *          <action> ::= -close
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes captured input files.  
 *
 *----------------------------------------------------------------------
 */

int CaptureInputCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  char * too_few = "Too few arguments, should be: capture-input [-open pathname | -query | -close]";
  char * too_many = "Too many arguments, should be: capture-input [-open pathname | -query | -close]";

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 2)  {
      interp->result = "The capture file is ";
      if (current_agent(capture_fileID)) {
		  Tcl_AppendResult(interp, 
			  "open.  Use capture-input -close to close the file.", 
			  (char *) NULL); 
	  } else {
		  Tcl_AppendResult(interp, "closed.", (char *) NULL); 
	  }
	  
      Tcl_AppendResult(interp, "", (char *) NULL);
	  
      return TCL_OK;      
  }
  
  if (argc > 3) {
      interp->result = too_many;
      return TCL_ERROR;      
  }
  
  if (string_match_up_to("-query", argv[1], 2)) {
      if (argc == 2) {
		  if (current_agent(capture_fileID)) {
			  interp->result = "open";
		  }
		  else {
			  interp->result = "closed";
		  }
	  } else if (argc > 2) {
		  interp->result = too_many;
		  return TCL_ERROR;      
	  }

  } else if (string_match_up_to("-close", argv[1], 2)) {
      if (argc == 2) {
		  if (current_agent(capture_fileID)) {
			  fclose(current_agent(capture_fileID));
			  current_agent(capture_fileID) = NIL;
			  interp->result = "capture file closed";
		  } else {
			  interp->result = "Attempt to close non-existant capture file";
			  return TCL_ERROR;
		  }
	  } else if (argc > 2) {
		  interp->result = too_many;
		  return TCL_ERROR;      
	  }
  } else { 
	  /* Either we have a file open request or there is an error */
      char * filename;
      char * mode;
      
	  if (argc == 3) {
		  if (string_match_up_to("-open", argv[1], 2))	{
			  filename = argv[2];
			  mode = "w";
		  } else if (string_match_up_to("-open", argv[2], 2))	{
			  filename = argv[1];
			  mode = "w";
		  } else	{
			  sprintf(interp->result,
				  "capture-input: unrecognized arguments: %s %s",
				  argv[1], argv[2]);
			  return TCL_ERROR;
		  }
	  } else { 
		  interp->result = too_few;
		  return TCL_ERROR; 
	  }

      {   /* we have -open and a filename;  if wm empty, open the file */
		  Tcl_DString buffer;
		  char * fullname;
		  FILE * f;
		  
          #if defined(MACINTOSH)
		  fullname = filename;
          #else
		  fullname = Tcl_TildeSubst(interp, filename, &buffer);
          #endif
		  
		  if (fullname == NULL) {
			  return TCL_ERROR;
		  }

		  /* --- check for empty system --- */
		  if (current_agent(all_wmes_in_rete)) {
			  sprintf(interp->result,
				  "Can't start capturing input wmes unless working memory is empty.");
			  return TCL_ERROR;
		  }

		  if (!(f = fopen(fullname, mode)))  {
			  Tcl_DStringFree(&buffer);
			  sprintf (interp->result,
				  "capture-input: Error: unable to open '%s'",
				  filename);
			  return TCL_ERROR;
		  } else {
			  current_agent(capture_fileID) = f;
			  Tcl_DStringFree(&buffer);
			  sprintf (interp->result,
				  "input capture file '%s' opened",
				  filename);
			  
			  fprintf(f, "##soar captured input file.\n");
		  }
	  }  
  }
  
  return TCL_OK;
}


/* kjh (B14) begin */
/*
 *----------------------------------------------------------------------
 *
 * ChunkNameFormatCmd --
 *
 *      This is the command procedure for the "chunk-name-format" 
 *      command, which specifies the format of names of newly created 
 *      chunks.
 *
 * Syntax:  chunk-name-format  [-short|-long]
 *                             [-prefix [<prefix>]]
 *                             [-count [<start-chunk-number>]]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets indicated format style, prefix, and starting chunk number.
 *      Prints current <prefix> if -prefix is used without a <prefix>.
 *      Prints current <start-chunk-number> if -count is used without
 *        a <start-chunk-number>.
 *
 *----------------------------------------------------------------------
 */

int ChunkNameFormatCmd (ClientData clientData, 
	                Tcl_Interp * interp,
	                int argc, char *argv[])
{
  unsigned long tmp_chunk_count;
  int i;
  bool seen_long_or_short;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1) {
    interp->result = "No arguments given.\nUsage: chunk-name-format [-short|-long] [-prefix [<prefix>]] [-count [<start-chunk-number>]]";
    return TCL_ERROR;
  }

  seen_long_or_short = FALSE;
  for (i = 1; i < argc; i++) {
    if        (string_match_up_to(argv[i], "-short",  2)) {
      if (seen_long_or_short) {
        sprintf(interp->result, 
             "-long and -short are exclusive options", argv[i]);
        return TCL_ERROR;
      } else {
        seen_long_or_short = TRUE;
        set_sysparam(USE_LONG_CHUNK_NAMES, FALSE);
      }
    } else if (string_match_up_to(argv[i], "-long",   2)) {
      if (seen_long_or_short) {
        sprintf(interp->result, 
             "-long and -short are exclusive options", argv[i]);
        return TCL_ERROR;
      } else {
        seen_long_or_short = TRUE;
        set_sysparam(USE_LONG_CHUNK_NAMES, TRUE);
      }
    } else if (string_match_up_to(argv[i], "-prefix", 2)) {
      if ((i+1 >= argc) || (*argv[i+1] == '-'))
        print("%s\n",current_agent(chunk_name_prefix));
      else {
        if (strchr(argv[i+1],'*')) {
          sprintf(interp->result, "Prefix-string may not contain a '*' character.");
          return TCL_ERROR;
        } else
          strcpy(current_agent(chunk_name_prefix),argv[++i]);
      }
    } else if (string_match_up_to(argv[i], "-count",  2)) {
      if ((i+1 >= argc) || (*argv[i+1] == '-'))
        print("%lu\n",current_agent(chunk_count));
      else if (sscanf(argv[i+1],"%lu",&tmp_chunk_count) == 1) {
        i++;
        if        (tmp_chunk_count < 0) {
          sprintf(interp->result, 
             "chunk-name-format: start-chunk-number must be > 0");
          return TCL_ERROR;
        } else if (tmp_chunk_count >= (unsigned long)current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]) { 
          sprintf(interp->result, 
             "chunk-name-format: start-chunk-number must be < max chunk system parameter (%ld)", 
             current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
          return TCL_ERROR;
        } else if (tmp_chunk_count < current_agent(chunk_count)) {
          sprintf(interp->result, 
             "chunk-name-format: start-chunk-number cannot be less than current chunk count (%ld)", 
             current_agent(chunk_count));
          return TCL_ERROR;
        } else {
          current_agent(chunk_count) = tmp_chunk_count;
        }
      } else {
        sprintf(interp->result, 
           "chunk-name-format: expected number after -count; got \"%s\"", argv[i]);
        return TCL_ERROR;
      }
    } else {
      sprintf(interp->result, 
           "Unrecognized argument to chunk-name-format: %s", argv[i]);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}
/* kjh (B14) end */

/*
 *----------------------------------------------------------------------
 *
 * DefWmeDepthCmd --
 *
 *      This is the command procedure for the "default-wme-depth" 
 *      command, which sets/prints the default print depth.
 *
 *      With no arguments, this command prints the current default 
 *      print depth used by the "print" command.  With an integer 
 *      argument, it sets the current default print depth.   This 
 *      default print depth can be overridden on any particular 
 *      invocation of the "print" command by using the -depth flag,
 *      e.g., print -depth 10 args....  The default print depth is 
 *      initially 1.
 *      See also:  print
 *
 * Syntax:  default-wme-depth [integer]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the default print depth for the agent.
 *
 *----------------------------------------------------------------------
 */

int DefWmeDepthCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, char *argv[])
{
  int depth;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result, "%d", current_agent(default_wme_depth));
      return TCL_OK;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments, should be: default-wme-depth [integer]";
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[1], &depth) == TCL_OK)
    {
      current_agent(default_wme_depth) = depth;
    }
  else
    {
      sprintf(interp->result, 
	      "Expected integer for new default print depth: %s", 
	      argv[1]);
      return TCL_OK;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyAgentCmd --
 *
 *	This is the command procedure for the "destroy-agent" command, 
 *	which destroys a Soar agent and its associated Tcl interpreter.
 *
 * Syntax:  destroy-agent agent-name
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	Destroys a Soar agent and its Tcl interpreter.  If this 
 *      command is issued in the interpreter being destroyed, then
 *      control switches to the global Soar agent.
 *
 *----------------------------------------------------------------------
 */

int 
DestroyAgentCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  cons * c;
  char * agent_name;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 2)
    {        
      sprintf(interp->result, "Too many arguments");
      return TCL_ERROR;
    }

  if (argc < 2)
    {
      interp->result = "Expected agent name but none given.";
      return TCL_ERROR;
    }

  agent_name = argv[1];

  for(c = all_soar_agents; c != NIL; c = c->rest) 
    {
      if (!strcmp(agent_name, ((agent *)c->first)->name)) 
	{
	  if (soar_agent == (agent *) c->first)
	    { 
	      sprintf(interp->result, 
		      "Attempt to delete current interpreter (%s) ignored.", 
		      argv[1]);
	      return TCL_ERROR;      	      
	    }	
	  destroy_soar_agent((agent *) c->first);
	  sprintf(interp->result, "Destroying agent %s", agent_name);
	  return TCL_OK;
	}
    }

  /* No agent found, the name doesn't match any agents */
  sprintf(interp->result, "Unknown agent name given: %s.", argv[1]);

  return TCL_ERROR;      

}

/*
 *----------------------------------------------------------------------
 *
 * EchoCmd --
 *
 *      This is the command procedure for the "echo" command, which 
 *      prints text to the currently specified output-strings-destination.
 *      IF logging is enabled, the text is also printed to the log
 *      destination.
 *
 * Syntax:  echo strings
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the given strings according to the currently specified
 *      output-strings-destination
 *
 *----------------------------------------------------------------------
 */

int EchoCmd (ClientData clientData, 
	     Tcl_Interp * interp,
	     int argc, char *argv[])
{
  int i;
  bool newline = TRUE;

  Soar_SelectGlobalInterpByInterp(interp);

  for (i = 1; i < argc; i++)
    {
      if (string_match_up_to("-nonewline", argv[i], 2))
	{
	  newline = FALSE;
	}
      else 
	{
	  Soar_LogAndPrint((agent *) clientData, argv[i]);
	  if ((i + 1) < argc) 
	    {
	      Soar_LogAndPrint((agent *) clientData, " ");
	    }
	}
    }

  if (newline)
    {
      Soar_LogAndPrint((agent *) clientData, "\n");
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExciseCmd --
 *
 *      This is the command procedure for the "excise" command, 
 *      which removes productions from Soar's memory.
 *
 *      excise -chunks removes all chunks and justifications
 *      from the agent.  excise -task removes all non-default
 *      productions from the agent and performs an init-soar
 *      command.  excise -all removes all productions from the
 *      agent.  The switches may be abbreviated to 2 characters
 *      (e.g, -c for chunks).
 *
 * Syntax:  excise production-name | -chunks | -default | -task 
 *                                 |-user | -all
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Removes the indicated productions from the agent memory.
 *      For -all and -task, does an init-soar.
 *
 *----------------------------------------------------------------------
 */

int ExciseCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, char *argv[])
{
  int i;
  production * prod;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result,"No arguments given.\nUsage: excise production-name | -chunks | -default | -task | -user | -all");
      return TCL_ERROR;
    }

  for (i = 1; i < argc; i++)
    {
      if (string_match_up_to(argv[i], "-chunks", 2))
	{
	  excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
	  excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
	  sprintf(interp->result," ");
	}
      else if (string_match_up_to(argv[i], "-default", 2))
	{
	  excise_all_productions_of_type (DEFAULT_PRODUCTION_TYPE);
	  sprintf(interp->result," ");
	}
      else if (string_match_up_to(argv[i], "-task", 2))
	{
	  excise_all_productions_of_type (USER_PRODUCTION_TYPE);
	  excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
	  excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
	  reinitialize_soar();
	  print("init-soar done\n");
	  sprintf(interp->result," ");
	}
      else if (string_match_up_to(argv[i], "-user", 2))
	{
	  excise_all_productions_of_type (USER_PRODUCTION_TYPE);
	  sprintf(interp->result," ");
	}
      else if (string_match_up_to(argv[i], "-all", 2))
	{
	  excise_all_productions_of_type (DEFAULT_PRODUCTION_TYPE);
	  excise_all_productions_of_type (USER_PRODUCTION_TYPE);
	  excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
	  excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
	  reinitialize_soar();
	  print("init-soar done\n");
	  sprintf(interp->result," ");
	}
      else if ((prod = name_to_production(argv[i])))
	{
	  excise_production(prod,(bool)(TRUE && current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));
	  sprintf(interp->result," ");
	}
      else if (string_match_up_to(argv[i], "-", 1))
	{
	  sprintf(interp->result, 
		  "Unrecognized argument to excise: %s", argv[i]);
	  return TCL_ERROR;
	}
      else
	{
	  sprintf(interp->result, 
		  "Production not found: %s", argv[i]);
	  return TCL_ERROR;
	}
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExplainBacktracesCmd --
 *
 *      This is the command procedure for the "explain-backtraces" command.
 *
 *      Explain provides some interpretation of backtraces generated 
 *      during chunking.  Explain mode must be ON when the chunk/
 *      justification is CREATED or no explanation will be available.
 *      Explain mode is toggled using the save_backtraces variable.
 *      When explain mode is on, more memory is used, and building 
 *      chunks/justifications will be slower.
 *
 *      The two most useful commands are 'explain-backtraces <name>' and
 *      'explain-backtraces <name> <cond-num>'.
 *      The first command lists all of the conditions for the named
 *      chunk/justification, and the 'ground' which resulted in
 *      inclusion in the chunk/justification.  A 'ground' is a WME 
 *      which was tested in the supergoal.  Just knowing which WME was 
 *      tested may be enough to explain why the chunk/justification 
 *      exists.  If not, the conditions can be listed with an integer 
 *      value.  This value can be used in 'explain <name> <cond-num>' 
 *      to obtain a list of the productions which fired to obtain this 
 *      condition in the chunk/justification (and the crucial WMEs 
 *      tested along the way).  Why use an integer value to specify the 
 *      condition?  To save a big parsing job. 
 *
 * Syntax: explain-backtraces arg
 *         arg := ''                 list chunks/justifications if 
 *                                   explain is on
 *         arg := <name>             list all conditions & grounds for 
 *                                   a chunk/justification
 *         arg := <name> -full       give the backtrace for a named
 *                                   chunk/justification
 *         arg := <name> <cond-num>  explain why this condition is in 
 *                                   the chunk/justification
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int ExplainBacktracesCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      explain_list_chunks();
      return TCL_OK;
    }

  {
    int cond_num;
    
    get_lexeme_from_string(argv[1]);

    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) 
      {
	if (argc > 2) 
	  {
	    if (string_match("-full", argv[2]))
	      {
		/* handle the 'explain name -full' case */
		
		explain_trace_named_chunk(current_agent(lexeme.string));
	      }
	    else if (Tcl_GetInt(interp, argv[2], &cond_num) == TCL_OK)
	      {
		/* handle the 'explain name <cond-num>' case */

		explain_chunk(current_agent(lexeme.string), cond_num);
	      }
	    else
	      {
		sprintf(interp->result,
			"Unexpected argument to %s %s: %s.  Should be -full or integer.",
			argv[0], argv[1], argv[2]);
		return TCL_ERROR;
	      }      
	  }
	else
	  {
	    /* handle the 'explain name' case */

	    explain_cond_list(current_agent(lexeme.string));
	  }
      }
    else
      {
	sprintf(interp->result,
		"Unexpected argument to %s: %s.  Should be symbolic constant or integer.",
		argv[0], argv[1]);
	return TCL_ERROR;
      }
  }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * FiringCountsCmd --
 *
 *      This is the command procedure for the "firing-counts" command, 
 *      which prints out how many times a production has fired.
 *
 *      This command prints how many times certain productions have 
 *      fired.  With no arguments, it lists all the productions sorted 
 *      according to how many times they have fired.  If an integer 
 *      argument (call it k) is given, only the top k productions are 
 *      listed.  If k=0, only the productions which haven't fired at 
 *      all are listed.  Note that firing counts are not reset by an 
 *      (init-soar); the counts indicate the number of firings since the
 *      productions were loaded or built.
 *
 *      NB:  this is slow, because the sorting takes time O(n*log n)
 *
 *      With one or more production names as arguments, this command 
 *      prints how many times each of those productions fired.
 *
 * Syntax:  firing-counts [[integer] | production-name* ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints information about production firings.
 *
 *----------------------------------------------------------------------
 */

int FiringCountsCmd (ClientData clientData, 
		     Tcl_Interp * interp,
		     int argc, char *argv[])
{
  int num_requested;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 1)
    {
      if (Tcl_GetInt(interp, argv[1], &num_requested) != TCL_OK)
	{
	  int i;

	  Tcl_ResetResult(interp);      /* Clear error message from */
                                        /* failed int parse         */
	  for(i = 1; i < argc; i++)
	    {
	      production * p;

	      p = name_to_production(argv[i]);
	      if (p)
		{
		  print ("%6lu:  %s\n", p->firing_count, argv[i]);
		}
	      else
		{
		  sprintf (interp->result,
			   "No production named %s", 
			   argv[i]);
		  return TCL_ERROR;
		}
	    }
	  return TCL_OK;
	}
    }

  print_production_firings(interp, ((argc == 1) ? 20 : num_requested));

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * FormatWatchCmd --
 *
 *      This is the command procedure for the "format-watch" 
 *      command, which defines the format to use when printing 
 *      objects and the Soar goal stack.
 *
 *      Object trace formats control how Soar prints an object--
 *      e.g., a certain operator, problem-space, etc.  (This is 
 *      like trace-attributes in Soar 5.)  Stack trace formats 
 *      control how Soar prints its context stack selections
 *      in 'watch 0' and 'pgs' printouts.  You specify a trace 
 *      format by indicating two things:
 *      - a format string, indicating the printout format to be used
 *      - what things this format string can be applied to
 *
 *      The format string can be any string.
 *      Certain 'escape sequences' can be used within the string; 
 *      for example, '%dc' means print the current decision cycle 
 *      number.
 * 
 *      There are two ways to restrict what objects a format string 
 *      applies to.  The {s|o|*} argument restricts the types 
 *      of objects:  's' indicates that the format only applies to 
 *      states; 'o' means it only applies to operators; and
 *      '*' means it applies to any type of object.  The [object-name]
 *      argument (for object trace formats), if given, means it only 
 *      applies to objects with that ^name.  The [ps-name] argument 
 *      (for stack trace formats) means it only applies within problem 
 *      spaces with that ^name.
 * 
 *      With an -add argument, these commands add new trace formats 
 *      (replacing any existing ones with identical applicability 
 *      conditions).  With a -remove argument, they remove trace 
 *      formats with the given applicability conditions.  With no 
 *      arguments, they print out all current trace formats.
 *
 *      The following escape sequences can be used within trace format 
 *      strings.  The S indicates the sequence is ONLY usable in
 *      stack traces:
 *
 *      %cs                S- print the current state
 *      %co                S- print the current operator 
 *                          * The %cg, %cp, %cs, and %co sequences use 
 *                            the appropriate object trace format.
 *      %dc                S- print the current decision cycle number 
 *      %ec                S- print the current elaboration cycle number
 *                          * The %dc and %ec sequences are NOT
 *                          * meaningful in stack traces produced by 
 *                            the "pgs" command.  In these cases, nothing 
 *                            is printed.
 *      %sd                S- print the current subgoal depth
 *                          * The %sd sequence uses 0 as the top level).
 *      %rsd[pattern]      S- repeat (subgoal depth) times: print the 
 *                            given pattern.
 *      %left[num,pattern]  - print the pattern left justified in a
 *                            field of num spaces.
 *      %right[num,pattern] - print the pattern right justified in a
 *                            field of num spaces.
 *      %id                 - print the identifier of the current object.
 *      %v[foo]             - print the value(s) of attribute ^foo on the 
 *                            current object.  If there is no ^foo on the 
 *                            current object, nothing is printed.
 *      %v[foo.bar.baz]     - same as the above, only follow the given 
 *                            attribute path to get the value(s).
 *      %v[*]               - print all values of all attributes on the 
 *                            current object.
 *      %o[args]            - same as %v, except that if the value is an 
 *                            identifier it is printed using the 
 *                            appropriate object trace format.
 *      %av[args]           - same as %v, except the printed value is 
 *                            preceeded with "^attr " to indicate the 
 *                            attribute name.
 *      %ao[args]           - a combination of the above two.
 *      %ifdef[pattern]     - print the given pattern if and only if all 
 *                            escape sequences inside it are "meaningful" 
 *                            or "well-defined."  For example, 
 *                            "%ifdef[foo has value: %v[foo]]" will print 
 *                            nothing if there is no ^foo on the current 
 *                            object.
 *      %nl                 - print a newline.
 *      %%                  - print a percent sign.
 *      %[                  - print a left bracket.
 *      %]                  - print a right bracket.
 *
 * Syntax:
 *      format-watch {-object | -stack} 
 *                   [{ { -add    {s|o|*} [name] format}
 *                   |  { -remove {s|o|*} [name]       }
 *                    }
 *                   ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the indicated format.
 *
 *----------------------------------------------------------------------
 */

int FormatWatchCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, char *argv[])
{
  static char * too_few_args  = "Too few arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";
  static char * too_many_args = "Too many arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";

  bool stack_trace;
  int type_restriction;
  Symbol *name_restriction = NIL;
  bool remove;
  int format_arg = 0;  /* Initialized to placate gcc -Wall */

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  /* --- set stack_trace depending on which option was given --- */  
  if (string_match("-stack", argv[1]))
    {
      stack_trace = TRUE;
    }
  else if (string_match("-object", argv[1]))
    {
      stack_trace = FALSE;
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized option to format-watch : %s",
	      argv[1]);
      return TCL_ERROR;
    }

  /* --- if no further args, print all trace formats of that type --- */
  
  if (argc == 2)
    {
      print_all_trace_formats_tcl (stack_trace);
      return TCL_OK;
    }

  /* --- next argument must be either -add or -remove --- */
  if (string_match("-add", argv[2]))
    {
      remove = FALSE;
    }
  else if (string_match("-remove", argv[2]))
    {
      remove = TRUE;
    }
  else 
    {
      sprintf(interp->result,
	      "Unrecognized option to format-watch %s: %s",
	      argv[1], argv[2]);
      return TCL_ERROR;
    }

  if (argc == 3)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  /* --- read context item argument: s, o, or '*' --- */
  
  if (string_match("s", argv[3]))
      {
	type_restriction = FOR_STATES_TF;
      }
  else if (string_match("o", argv[3]))
      {
	type_restriction = FOR_OPERATORS_TF;
      }
  else if (string_match("*", argv[3]))
      {
	type_restriction = FOR_ANYTHING_TF;
      }
  else 
      {
	sprintf(interp->result,
		"Unrecognized option to %s %s %s: %s",
		argv[0], argv[1], argv[2], argv[3]);
	return TCL_ERROR;	
      }

  if (argc > 4)
    {
      get_lexeme_from_string(argv[4]);

      /* --- read optional name restriction --- */
      if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) 
	{
	  name_restriction = make_sym_constant (argv[4]);
	  format_arg = 5;
        }
      else
	{
	  format_arg = 4;
	}

      if (   (remove  && (argc > format_arg))
	  || (!remove && (argc > (format_arg + 1))))
	{
	  interp->result = too_many_args;
	  return TCL_ERROR;
	}
    }

  /* --- finally, execute the command --- */
  if (remove) 
    {
      remove_trace_format (stack_trace, type_restriction, name_restriction);
    } 
  else 
    {
      if (argc == (format_arg + 1))
	{
	  add_trace_format (stack_trace, 
			    type_restriction, 
			    name_restriction,
			    argv[format_arg]);
	}
      else
	{
	  if (name_restriction)
	    {
	      symbol_remove_ref (name_restriction);
	    }
	  
	  sprintf(interp->result,
		  "Missing format string");

	  return TCL_ERROR;
	}
    }

  if (name_restriction) 
    {
      symbol_remove_ref (name_restriction);
    }

  return TCL_OK;
}

/* REW: begin 09.15.96 */
/*
 *----------------------------------------------------------------------
 *
 * GDS_PrintCmd --
 *
 * Debug routine for examing the GDS when necessary.  This is horribly 
 * inefficient and should not generally be used except when something 
 * is going wrong and you want to take a peak at the GDS
 *
 * Syntax:    gds_print
 *
 * Results:   returns standard Tcl completion code.
 *
 * Side Effects:  None.
 *
 *
 *----------------------------------------------------------------------
 */

int GDS_PrintCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, char *argv[])
{
  wme *w;
  Symbol *goal;

  
  print("********************* Current GDS **************************\n");
  print("stepping thru all wmes in rete, looking for any that are in a gds...\n");
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next) {
    if (w->gds){
      if (w->gds->goal) {
        print_with_symbols ("  For Goal  %y  ", w->gds->goal);
      } else {
	             print("  Old GDS value ");
      }
        print ("(%lu: ", w->timetag);
        print_with_symbols ("%y ^%y %y", w->id, w->attr, w->value);
        if (w->acceptable) print_string (" +");
        print_string (")");
        print ("\n");
    }
  }
  print("************************************************************\n");
  for (goal=current_agent(top_goal); goal!=NIL; goal=goal->id.lower_goal){
    print_with_symbols ("  For Goal  %y  ", goal);
    if (goal->id.gds){
	       /* Loop over all the WMEs in the GDS */
               print ("\n");
               for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next){
                 print ("                (%lu: ", w->timetag);
                 print_with_symbols ("%y ^%y %y", w->id, w->attr, w->value);
                 if (w->acceptable) print_string (" +");
                 print_string (")");
                 print ("\n");
	       }
	       
    } else print(": No GDS for this goal.\n");
  }
    
  print("************************************************************\n");
  return TCL_OK;
}
/* REW: end   09.15.96 */

/*
 *----------------------------------------------------------------------
 *
 * IndifferentSelectionCmd --
 *
 *      This is the command procedure for the "indifferent-selection" 
 *      command which controls indifferent preference arbitration.
 *
 *      With no arguments, this command prints the current setting of 
 *      indifferent-selection.  With an argument, it sets indifferent-selection
 *      to the given value.  This controls how Soar's decision procedure 
 *      chooses between multiple indifferent items:
 *        -first -- just choose the first one found (deterministically)
 *        -last -- choose the last one found (deterministically)
 *        -ask -- ask the user to choose one of the items
 *        -random -- choose one randomly
 *
 * Syntax:  indifferent-selection [-first | -ask | -last | -random ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Modifies the setting of the indifferent-selection option.
 *
 *----------------------------------------------------------------------
 */

/* AGR 615  Adding the "last" option to this command was pretty simple
   and the changes are integrated into this entire function.  94.11.08 */

int IndifferentSelectionCmd (ClientData clientData, 
			     Tcl_Interp * interp,
			     int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      switch (current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]) 
	{
	case USER_SELECT_FIRST:  interp->result = "-first";  break;
	case USER_SELECT_LAST:   interp->result = "-last";   break;
	case USER_SELECT_ASK:    interp->result = "-ask";    break;
	case USER_SELECT_RANDOM: interp->result = "-random"; break;
	}
      return TCL_OK;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments, should be: user-select [-first | -last | -ask | -random ]";
      return TCL_ERROR;
    }

  if (string_match_up_to(argv[1], "-ask", 2))
    {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_ASK);
    }
  else if (string_match_up_to(argv[1], "-first", 2))
    {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
    }
  else if (string_match_up_to(argv[1], "-last", 2))
    {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
    }
  else if (string_match_up_to(argv[1], "-random", 2))
    {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
    }
  else
    {
      sprintf(interp->result, 
	      "Unrecognized argument to indifferent-selection: %s", argv[1]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InitSoarCmd --
 *
 *      This is the command procedure for the "init-soar" command, 
 *      (re)initializes the Soar agent.
 * 
 *      This command re-initializes Soar.  It removes all wmes from 
 *      working memory, wiping out the goal stack, and resets all 
 *      statistics (except the counts of, how many times each 
 *      individual production has fired, used by the "firing-counts" 
 *      command).
 *
 * Syntax:  init-soar
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Empties working memory, removes the goal stack, and resets
 *      statistics.
 *
 *----------------------------------------------------------------------
 */

int InitSoarCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 1)
    {
      interp->result = "Too many arguments, should be: init-soar";
      return TCL_ERROR;
    }

  reinitialize_soar();
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InputPeriodCmd --
 *
 *      This is the command procedure for the "input-period" 
 *      command, which sets/prints the Soar input period.
 *
 *      With no arguments, this command prints the current period
 *      used to control the input rate to the Soar agent.  With an
 *      integer argument, it sets the current input period.  If
 *      the argument is 0, Soar behaves as it did in versions before
 *      6.2.4, accepting input every elaboration cycle.  An input
 *      period of "n" means that input is accepted only every "n"
 *      decision cycles.  The input period is initially set to 0.
 *      Negative integer values are disallowed.
 *
 * Syntax:  input-period [integer]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets/prints the input period for the agent.
 *
 *----------------------------------------------------------------------
 */

int InputPeriodCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, char *argv[])
{
  int period;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result, "%d", current_agent(input_period));
      return TCL_OK;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments, should be: input-period [integer]";
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[1], &period) == TCL_OK)
    {
      if (period >= 0)
	{
	  current_agent(input_period) = period;
	}
      else
	{
	  sprintf(interp->result, 
		  "Integer for new input period must be >= 0, not %s", 
		  argv[1]);
	  return TCL_ERROR;
	}
    }
  else
    {
      sprintf(interp->result, 
	      "Expected integer for new input period: %s", 
	      argv[1]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * InternalSymbolsCmd --
 *
 *      This is the command procedure for the "internal-symbols"
 *      command which prints information about the Soar agent symbol
 *      table.
 *
 * Syntax:  internal-symbols
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the current Soar agent symbol table
 *
 *----------------------------------------------------------------------
 */
int InternalSymbolsCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 1)
    {
      interp->result = "Too many arguments, should be: internal-symbols";
      return TCL_ERROR;
    }

  print_internal_symbols();

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * IOCmd --
 *
 *      This is the command procedure for the "io" command, 
 *      which manages the attachment of scripts to handle Soar I/O.
 *
 * Syntax:  io <add-specification> | 
 *             <removal-specification> |
 *             <list-specification> |
 *
 *          <add-specification>     := -add -input  script [id] |
 *                                     -add -output script  id
 *          <removal-specification> := -delete [-input | -output] id
 *          <list-specification>    := -list [-input | -output]
 *
 * Results:
 *      Returns a standard Tcl completion code. 
 *
 * Side effects:
 *      Adds and/or removes indicated I/O procedure from the system.
 *      This command may also print information about defined I/O
 *      procedures.  Returns the name of the new I/O procedure
 *      if one is created.
 *
 *----------------------------------------------------------------------
 */

static int io_proc_counter = 1;

int IOCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, char *argv[])
{
  static char * too_few_args_string = "Too few arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
  static char * too_many_args_string = "Too many arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
  char * io_id;
  char   buff[10];          /* What size is good here? */

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args_string;
      return TCL_ERROR;
    }
  if (string_match_up_to(argv[1], "-add", 2))
    {
      if (argc < 4)
	{
	  interp->result = too_few_args_string;
	  return TCL_ERROR;
	}

      if (argc > 5)
	{
	  interp->result = too_many_args_string;
	  return TCL_ERROR;
	}

      if (argc == 5)
	{
	  io_id = argv[4];
	}
      else
	{
	  sprintf(buff, "m%d", io_proc_counter++);
	  io_id = buff;
	}

      {
	if (string_match_up_to(argv[2], "-input", 2))
	  {
	    add_input_function((agent *) clientData, 
			       soar_input_callback_to_tcl,
			       (soar_callback_data) savestring(argv[3]), 
			       soar_callback_data_free_string,
			       (soar_callback_id) io_id);
	  }
	else if (string_match_up_to(argv[2], "-output", 2))
	  {
            /* Soar-Bugs #131, id required for output - TMH */
            if (argc < 5)
	    {
	      interp->result = too_few_args_string;
	      return TCL_ERROR;
	    }

	    add_output_function((agent *) clientData, 
				soar_output_callback_to_tcl,
				(soar_callback_data) savestring(argv[3]), 
				soar_callback_data_free_string,
				(soar_callback_id) io_id);
	  }
	else
	  {
	    sprintf(interp->result,
		    "%s: Unrecognized IO type: %s %s",
		    argv[0], argv[1], argv[2]);
	    return TCL_ERROR;
	  }

	interp->result = io_id;
	return TCL_OK;
      }
    }
  else if (string_match_up_to(argv[1], "-delete", 2))
    {
      switch (argc) {
      case 2:
      case 3:	  
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 4:	  /* Delete single callback for given event */
	  {
	    if (string_match_up_to(argv[2], "-input", 2))
	      {
		remove_input_function((agent *) clientData, argv[3]);
	      }
	    else if (string_match_up_to(argv[2], "-output", 2))
	      {
		remove_output_function((agent *) clientData, argv[3]);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized io type: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }
    }
  else if (string_match_up_to(argv[1], "-list", 2))
    {
      switch (argc) {
      case 2:	  
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 3:	  
	{
	  SOAR_CALLBACK_TYPE ct;
	  
	  if (string_match_up_to(argv[2], "-input", 2))
	    {
	      ct = INPUT_PHASE_CALLBACK;
	    }
	  else if (string_match_up_to(argv[2], "-output", 2))
	    {
	      ct = OUTPUT_PHASE_CALLBACK;
	    }
	  else
	    {
	      sprintf(interp->result,
		      "Attempt to list unrecognized io type: %s",
		      argv[2]);
	      return TCL_ERROR;
	    }

	  soar_push_callback((soar_callback_agent) clientData,
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_AppendResult, 
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	   
	 /* if a log file is open, then we need to push a dummy callback
      * so that we don't get extraneous prints mucking up the log file.
      * Addresses bug # 248.  KJC 01/00 */
	  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
		  soar_push_callback((soar_callback_agent) clientData, 
		     LOG_CALLBACK,
		     (soar_callback_fn) Soar_DiscardPrint, 
		     (soar_callback_data) NULL,
		     (soar_callback_free_fn) NULL);
	  }

	  soar_list_all_callbacks_for_event((agent *) clientData, ct);

	  soar_pop_callback((soar_callback_agent) clientData, PRINT_CALLBACK);
	  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
		  soar_pop_callback((soar_callback_agent) clientData, LOG_CALLBACK);
	  }
	}
	break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }

    }
  else
    {
      sprintf(interp->result, 
	      "Unrecognized option to io command: %s", argv[1]);
      return TCL_ERROR;
    }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * LearnCmd --
 *
 *      This is the command procedure for the "learn" command, which 
 *
 *      With no arguments, this command prints out the current learning 
 *      status.  Any of the following arguments may be given:
 *
 *        on         - turns all learning on 
 *        off        - turns all learning off 
 *        only       - learning is off except as specified by RHS force-learn
 *        except     - learning is on except as specified by RHS dont-learn
 *        list       - print lists of force-learn and dont-learn states
 *        all-goals  - when learning is on, this allows learning at all 
 *                     goal stack levels (in contrast to bottom-up 
 *                     learning)
 *        bottom-up  - when learning is on, this allows learning at only 
 *                     the lowest goal stack level; i.e., a chunk is 
 *                     learned at a given level only if no chunk has yet 
 *                     been learned at a lower level.
 *
 * See also: chunk-free-problem-spaces, watch
 *
 * Syntax:  learn arg*
 *            arg  ::=  -on | -only | -except | -off 
 *            arg  ::=  -all-goals | -bottom-up
 *            arg  ::=  -list
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets booleans for whether or not chunking done.
 *
 *----------------------------------------------------------------------
 */

int LearnCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print_current_learn_settings();
      return TCL_OK;
    }

  {int i;

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {
	   set_sysparam (LEARNING_ON_SYSPARAM, TRUE); 
	   set_sysparam (LEARNING_ONLY_SYSPARAM, FALSE);
	   set_sysparam (LEARNING_EXCEPT_SYSPARAM, FALSE);
	 }
       else if (string_match_up_to("-only", argv[i], 3))
	 {
	   set_sysparam (LEARNING_ON_SYSPARAM, TRUE); 
	   set_sysparam (LEARNING_ONLY_SYSPARAM, TRUE);
	   set_sysparam (LEARNING_EXCEPT_SYSPARAM, FALSE);
	 }
       else if (string_match_up_to("-except", argv[i], 2))
	 {
	   set_sysparam (LEARNING_ON_SYSPARAM, TRUE); 
	   set_sysparam (LEARNING_ONLY_SYSPARAM, FALSE);
	   set_sysparam (LEARNING_EXCEPT_SYSPARAM, TRUE);
	 }
       else if (string_match_up_to("-off", argv[i], 3))
	 {
	   set_sysparam (LEARNING_ON_SYSPARAM, FALSE); 
	   set_sysparam (LEARNING_ONLY_SYSPARAM, FALSE);
	   set_sysparam (LEARNING_EXCEPT_SYSPARAM, FALSE);
	 }
       else if (string_match_up_to("-all-levels", argv[i], 2)) 
	 {
	   set_sysparam (LEARNING_ALL_GOALS_SYSPARAM, TRUE);
	 }
       else if (string_match_up_to("-bottom-up", argv[i], 2))
	 {
	   set_sysparam (LEARNING_ALL_GOALS_SYSPARAM, FALSE);
	 }
       else if (string_match_up_to("-list", argv[i], 2))
	 {
	   cons * c;
	   char buff[1024];

	   print_current_learn_settings();
	   Tcl_AppendResult(interp,
			    "force-learn states (when learn = -only):\n",
			    (char *) NULL);
	   for (c = current_agent(chunky_problem_spaces); 
		c != NIL; 
		c = c->rest)
	     {
	       symbol_to_string((Symbol *) (c->first), TRUE, buff);
	       Tcl_AppendElement(interp, buff);
	     }
	   Tcl_AppendResult(interp,
			    "\ndont-learn states (when learn = -except):\n",
			    (char *) NULL);
	   for (c = current_agent(chunk_free_problem_spaces); 
		c != NIL; 
		c = c->rest)
	     {
	       symbol_to_string((Symbol *) (c->first), TRUE, buff);
	       Tcl_AppendElement(interp, buff);
	     }
	   return TCL_OK;
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to learn command: %s",
		   argv[i]);
	   return TCL_ERROR;
	 }
     }
 }

  return TCL_OK;
}

#ifdef ATTENTION_LAPSE
/* RMJ */

/*
 *----------------------------------------------------------------------
 *
 * AttentionLapseCmd --
 *
 *      This is the command procedure for the "attention-lapse" command.
 *      With no arguments, this command prints out the current attentional 
 *      lapsing status.  Any of the following arguments may be given:
 *
 *        on         - turns attentional lapsing on 
 *        off        - turns attentional lapsing off 
 *
 * See also: wake-from-attention-lapse, start-attention-lapse
 *
 * Syntax:  attention-lapse arg*
 *            arg  ::=  -on | -off 
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean for whether or not attentional lapsing will occur.
 *
 *----------------------------------------------------------------------
 */

int AttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print_current_attention_lapse_settings();
      return TCL_OK;
    }

  {int i;

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {
	   set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, TRUE); 
           wake_from_attention_lapse();
	 }
       else if (string_match_up_to("-off", argv[i], 3))
	 {
	   set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, FALSE); 
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to attention-lapse command: %s",
		   argv[i]);
	   return TCL_ERROR;
	 }
     }
 }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * WakeFromAttentionLapseCmd --
 *
 *      This is the command procedure for the "wake-from-attention-lapse"
 *      command, which is primarily intended to be called from the RHS
 *      of a production rule.
 *      This sets the "attention-lapsing" variable to FALSE (0), and
 *      starts tracking the amount of real time that has passed since
 *      the last lapse.
 *
 * See also: attention-lapse, start-attention-lapse
 *
 * Syntax:  wake-from-attention-lapse
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean "attention-lapsing" variable to 0; resets
 *      "attention_lapse_tracker" to current real time of day.
 *
 *----------------------------------------------------------------------
 */

int WakeFromAttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1) {
     wake_from_attention_lapse();
     return TCL_OK;
  } else {
      interp->result = "Too many arguments, should be: wake-from-attention-lapse";
     return TCL_ERROR;
  }
}

/*
 *----------------------------------------------------------------------
 *
 * StartAttentionLapseCmd --
 *
 *      This is the command procedure for the "start-attention-lapse"
 *      command, which should not normally be called by the user or
 *      an agent (attention lapses normally get started automatically
 *      by the architecture).
 *      This sets the "attention-lapsing" variable to TRUE (1), and
 *      starts tracking the amount of real time that should pass before
 *      ending the lapse (with wake_from_attention_lapse).  The duration
 *      of the lapse is the number of milleseconds specified by the
 *      argument to this command (in real time).
 *
 * See also: attention-lapse, wake-from-attention-lapse
 *
 * Syntax:  start-attention-lapse integer
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean "attention-lapsing" variable to 1; resets
 *      "attention_lapse_tracker" to current real time of day plus
 *      the number of milleseconds specified by the integer argument.
 *
 *----------------------------------------------------------------------
 */

int StartAttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  int duration;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 2) {
      interp->result = "Too few arguments, should be: start-attention-lapse integer";
     return TCL_ERROR;
  } else if (argc > 2) {
      interp->result = "Too many arguments, should be: start-attention-lapse integer";
     return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &duration) == TCL_OK)
    {
      start_attention_lapse((long)duration);
    }
  else
    {
      sprintf(interp->result, 
	      "Expected integer for attention lapse duration: %s", 
	      argv[1]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

#endif  /* ATTENTION_LAPSE */

/*
 *----------------------------------------------------------------------
 *
 * LogCmd --
 *
 *      This is the command procedure for the "log" command which 
 *      records session information to a log file.
 *
 *      This command may be used to open and close log files.  With
 *      the -add argument, specific strings can also be added to 
 *      the log file.
 *
 * Syntax:  log <action>
 *          <action> ::= [-new | -existing] pathname 
 *          <action> ::= -add string
 *          <action> ::= -query
 *          <action> ::= -off
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes log files.
 *
 *----------------------------------------------------------------------
 */

int LogCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  char * too_few = "Too few arguments, should be: log [-new | -existing] pathname | log -add string | log -query | log -off";
  char * too_many = "Too many arguments, should be: log [-new | -existing] pathname | log -add string | log -query | log -off";

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 2)
    {
      interp->result = "The log file is ";
      if (soar_exists_callback((soar_callback_agent) clientData,
			       LOG_CALLBACK))
	{
	  Tcl_AppendResult(interp, 
			   "open.  Use log -off to close the file.", 
			   (char *) NULL); 
	}
      else
	{
	  Tcl_AppendResult(interp, "closed.", (char *) NULL); 
	}
      Tcl_AppendResult(interp, "", (char *) NULL);

      return TCL_OK;      
    }

  if (argc > 3)
    {
      interp->result = too_many;
      return TCL_ERROR;      
    }

  if (string_match_up_to("-add", argv[1], 2))
    {
      if (argc == 3)
	{
	  Soar_Log((agent *) clientData, argv[2]);
	}
      else if (argc < 3)
	{
	  interp->result = too_few;
	  return TCL_ERROR;      
	}
      else 
	{
	  interp->result = too_many;
	  return TCL_ERROR;      
	}
    }
  else if (string_match_up_to("-query", argv[1], 2))
    {
      if (argc == 2)
	{
	  if (soar_exists_callback((soar_callback_agent) clientData,
				   LOG_CALLBACK))
	    {
	      interp->result = "open";
	    }
	  else
	    {
	      interp->result = "closed";
	    }
	}
      else if (argc < 2)
	{
	  interp->result = too_few;
	  return TCL_ERROR;      
	}
      else 
	{
	  interp->result = too_many;
	  return TCL_ERROR;      
	}
    }
  else if (string_match_up_to("-off", argv[1], 2))
    {
      if (argc == 2)
	{
	  if (soar_exists_callback((soar_callback_agent) clientData,
				   LOG_CALLBACK))
	    {
	      Soar_Log((agent *) clientData, "\n**** log closed ****\n");
	      soar_pop_callback((soar_callback_agent) clientData,
				LOG_CALLBACK);

              /* Soar-Bugs #74 TMH */
 	      interp->result = "log file closed";

	    }
	  else
	    {
	      interp->result = "Attempt to close non-existant log file";
	      return TCL_ERROR;
	    }
	}
      else if (argc > 2)
	{
	  interp->result = too_many;
	  return TCL_ERROR;      
	}
    }
  else
    { /* Either we have a file open/append request or there is an error */
      
      char * filename;
      char * mode;

      if (argc == 2)
	{
	  filename = argv[1];
	  mode = "w";
	} 
      else if (string_match_up_to("-new", argv[1], 2))
	{
	  filename = argv[2];
	  mode = "w";
	}
      else if (string_match_up_to("-new", argv[2], 2))
	{
	  filename = argv[1];
	  mode = "w";
	}
      else if (string_match_up_to("-existing", argv[1], 2))
	{
	  filename = argv[2];
	  mode = "a";
	}
      else if (string_match_up_to("-existing", argv[2], 2))
	{
	  filename = argv[1];
	  mode = "a";
	}
      else
	{
	  sprintf(interp->result,
		  "log: unrecognized arguments: %s %s",
		  argv[1], argv[2]);
	  return TCL_ERROR;
	}

      {
	Tcl_DString buffer;
	char * fullname;
	FILE * f;

#if defined(MACINTOSH)
	fullname = filename;
#else
	fullname = Tcl_TildeSubst(interp, filename, &buffer);
#endif

	if (fullname == NULL)
	  {
	    return TCL_ERROR;
	  }

	if (!(f = fopen(fullname, mode)))
	  {
	    Tcl_DStringFree(&buffer);
	    sprintf (interp->result,
		     "log: Error: unable to open '%s'",
		     filename);
	    return TCL_ERROR;
	  }
	else
	  {
            /* Soar-Bugs #74 TMH */
       	    Tcl_DStringFree(&buffer);
	    sprintf (interp->result,
		     "log file '%s' opened",
		     filename);

	    /* Install callback */
	    soar_push_callback((soar_callback_agent) clientData, 
			       LOG_CALLBACK,
			       (soar_callback_fn) Soar_PrintToFile,
			       (soar_callback_data) f,
			       (soar_callback_free_fn) Soar_FClose);
	    Soar_Log((agent *) clientData, "**** log opened ****\n");
	  }
      }
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MatchesCmd --
 *
 *      This is the command procedure for the "matches" command, 
 *      which prints partial match information for a selected
 *      production.  It also prints information about the current
 *      match set.
 *
 *      The match set is a list of productions that are about to 
 *      fire or retract in the next preference phase.  With no 
 *      arguments, this command prints out the production names for 
 *      both Assertions and Retractions.  The first optional argument
 *      specifies listing of either Assertions or Retractions.
 *
 *      The last optional argument specifies the level of detail wanted:  
 *      -counts (the default for single productions) prints out just 
 *      the partial match counts; -names (the default for match sets) 
 *      prints out just the production names; -timetags also prints
 *      the timetags of wmes matched; and -wmes prints the wmes rather
 *       than just their timetags.
 *
 * Syntax:  matches production-name [ -count | -timetags | -wmes]
 * Syntax:  matches production-name [ 0 | 1 | 2 ]
 * Syntax:  ms [-assertions | -retractions] [-names| -timetags | -wmes]
 * Syntax:  ms [-assertions | -retractions] [ 0 | 1 | 2 ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints production match information.
 *
 *----------------------------------------------------------------------
 */

int MatchesCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, char *argv[])
{
  production * prod = NULL;
  production * prod_named;
  wme_trace_type wtt = NONE_WME_TRACE;
  ms_trace_type  mst = MS_ASSERT_RETRACT;
  int curr_arg = 1;

  Soar_SelectGlobalInterpByInterp(interp);

  while (curr_arg < argc)
    {
      if (string_match_up_to("-assertions", argv[curr_arg], 2))
	{
	  mst = MS_ASSERT;
	}
      else if (string_match_up_to("-retractions", argv[curr_arg], 2))
	{
	  mst = MS_RETRACT;
	}
      else if (   string_match_up_to("-names", argv[curr_arg], 2)
	       || string_match_up_to("-count", argv[curr_arg], 2)
	       || string_match("0", argv[curr_arg]))
	{
	  wtt = NONE_WME_TRACE;
	}
      else if (   string_match_up_to("-timetags", argv[curr_arg], 2)
	       || string_match("1", argv[curr_arg]))
	{
	  wtt = TIMETAG_WME_TRACE;
	}
      else if (   string_match_up_to("-wmes", argv[curr_arg], 2)
	       || string_match("2", argv[curr_arg]))
	{
	  wtt = FULL_WME_TRACE;
	}
      else if ((prod_named = name_to_production(argv[curr_arg])))
	{
	  prod = prod_named;
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized option to matches command: %s", 
		  argv[curr_arg]);
	  return TCL_ERROR;
	}

      curr_arg++;
    }

  if (prod != NULL) 
    {
      print_partial_match_information (prod->p_node, wtt);
    }
  else 
    {
      print_match_set (wtt, mst);
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MaxChunksCmd --
 *
 *      This is the command procedure for the "max-chunks"  command, 
 *      which sets/prints the maximum number of chunks allowed.
 *
 *      With no arguments, this command prints the current value 
 *      of the system variable "max-chunks".  With an integer argument, 
 *      it sets the current value.   This variable controls the maximum 
 *      number of chunks allowed in a single decision cycle.  After this 
 *      many chunks have been executed, Soar proceeds to decision phase 
 *      even if quiescence hasn't really been reached yet.  (Max-chunks 
 *      is initially 50.)  The maximum number of chunks can also
 *      be limited by setting the soar variable max_chunks.
 *
 * Syntax:  max-chunks [integer]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the maximum chunks allowed.
 *
 *----------------------------------------------------------------------
 */

int MaxChunksCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, char *argv[])
{
  int num;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result, "%ld", 
	      current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
      return TCL_OK;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments, should be: max-chunks [integer]";
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[1], &num) == TCL_OK)
    {
      set_sysparam (MAX_CHUNKS_SYSPARAM, num);
    }
  else
    {
      sprintf(interp->result, 
	      "Expected integer for new maximum chunks count: %s", 
	      argv[1]);
      return TCL_OK;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MaxElaborationsCmd --
 *
 *      This is the command procedure for the "max-elaborations" 
 *      command, which sets/prints the maximum elaboration cycles
 *      allowed.
 *
 *      With no arguments, this command prints the current value of 
 *      the system variable "max-elaborations".  With an integer 
 *      argument, it sets the current value.   This variable controls 
 *      the maximum number of elaboration cycles allowed in a single 
 *      decision cycle.  After this many elabloration cycles have been 
 *      executed, Soar proceeds to decision phase even if quiescence
 *      hasn't really been reached yet.  (Max-elaborations is initially 
 *      100.)
 *
 * Syntax:  max-elaborations [integer]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the maximum elaborations allowed.
 *
 *----------------------------------------------------------------------
 */

int MaxElaborationsCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, char *argv[])
{
  int num;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)    {
      sprintf(interp->result, "%ld", 
	      current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM]);
      return TCL_OK;
  }

  if (argc > 2)    {
      interp->result = "Too many arguments, should be: max-elaborations [integer]";
      return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &num) == TCL_OK)    {
      set_sysparam (MAX_ELABORATIONS_SYSPARAM, num);
  } else  {
      sprintf(interp->result, 
	      "Expected integer for new maximum elaborations count: %s", 
	      argv[1]);
      return TCL_OK;
  }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MaxNilOutputCmd --
 *
 *      This is the command procedure for the "max-nil-output-cycles" 
 *      command, which sets/prints the maximum output cycles that
 *      go by with no output when using run-til-output-generated.
 *
 *      With no arguments, this command prints the current value of 
 *      the system variable "max-nil-output-cycles".  With an integer 
 *      argument, it sets the current value.   This variable controls 
 *      the maximum number of nil output cycles allowed in a single 
 *      run-til-output loop.  After this many output cycles have been 
 *      executed with no output being put on the output link, Soar
 *      stops and waits for the next command.  The default value is 15.
 *
 * Syntax:  max-nil-output-cycles [integer]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the maximum nil output cycles allowed when using
 *      run-til-output-generated.
 *
 *----------------------------------------------------------------------
 */

int MaxNilOutputCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, char *argv[])
{
  int num;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)    {
      sprintf(interp->result, "%ld", 
	      current_agent(sysparams)[MAX_NIL_OUTPUT_CYCLES_SYSPARAM]);
      return TCL_OK;
  }

  if (argc > 2)    {
      interp->result = "Too many arguments, should be: max-nil-output-cycles [integer]";
      return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &num) == TCL_OK)    {
      set_sysparam (MAX_NIL_OUTPUT_CYCLES_SYSPARAM, num);
  } else  {
      sprintf(interp->result, 
	      "Expected integer for new maximum output cycle count: %s", 
	      argv[1]);
      return TCL_OK;
  }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MemoriesCmd --
 *
 *      This is the command procedure for the "memories" command, 
 *      which prints information about memory use, in tokens, of 
 *      partial matches of productions.  If a production-name is 
 *      given, memory use for that production is printed.  If no 
 *      production name is given, memories will list 'count' 
 *      productions of the specified type (or all types, if no 
 *      type is specified).  If 'count' is omitted, memory use of
 *      all productions is printed.
 *
 * Syntax:  memories [arg*]
 *          arg  ::=  -chunk | -user | -default | -justification 
 *                           | production-name | count
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints information about memory usage of partial matches.
 *
 *----------------------------------------------------------------------
 */

int MemoriesCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  int i;
  int num;
  int num_items = -1;
  int mems_to_print[NUM_PRODUCTION_TYPES];
  bool set_mems = FALSE;
  production * prod;

  Soar_SelectGlobalInterpByInterp(interp);

  for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
    mems_to_print[i] = FALSE;

  for (i = 1; i < argc; i++)
    {
      if (string_match_up_to(argv[i], "-chunks", 2))
	{
	  mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
	  set_mems = TRUE;
	}
      else if (string_match_up_to(argv[i], "-user", 2))
	{
	  mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
	  set_mems = TRUE;
	}
      else if (string_match_up_to(argv[i], "-defaults", 2))
	{
	  mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
	  set_mems = TRUE;
	}
      else if (string_match_up_to(argv[i], "-justifications", 2))
	{
	  mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
	  set_mems = TRUE;
	}
      else if ((prod = name_to_production(argv[i])))
	{
	  print("\n Memory use for %s: %ld\n\n", argv[i], 
		count_rete_tokens_for_production(prod));
	  set_mems = TRUE;
  	}
      else if (Tcl_GetInt(interp, argv[i], &num) == TCL_OK)
	{
	  if (num <= 0)
	    {
	      sprintf(interp->result, 
		      "Count argument to memories must be a positive integer, not: %s", 
		      argv[i]);
	      return TCL_ERROR;
	    }
	  else
	    {
	      num_items = num;
	    }
	}
      else
	{
	  sprintf(interp->result, 
		  "Unrecognized argument to memories: %s", argv[i]);
	  return TCL_ERROR;
	}
    }

  if (!set_mems) {
    mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
    mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
    mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
    mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
  }
  /*printf("chunkflag = %d\nuserflag = %d\ndefflag = %d\njustflag = %d\n",
   *     mems_to_print[CHUNK_PRODUCTION_TYPE],
   *     mems_to_print[USER_PRODUCTION_TYPE],
   *	 mems_to_print[DEFAULT_PRODUCTION_TYPE],
   *	 mems_to_print[JUSTIFICATION_PRODUCTION_TYPE]);
   */ 
  print_memories(num_items, mems_to_print);
  
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MonitorCmd --
 *
 *      This is the command procedure for the "monitor" command, 
 *      which manages the attachment of scripts to Soar events.
 *
 * Syntax:  monitor <add-specification> | 
 *                  <removal-specification> |
 *                  <list-specification> |
 *                  -test |
 *                  -clear
 *
 *          <add-specification>     := -add soar-event script [id]
 *          <removal-specification> := -delete soar-event [id]
 *          <list-specification>    := -list [soar-event]
 *
 * Results:
 *      Returns a standard Tcl completion code. 
 *
 * Side effects:
 *      Adds and/or removes indicated monitor from the system.
 *      This command may also print information about defined
 *      monitors.  Returns the name of the new monitor attachments
 *      if one is created.
 *
 *----------------------------------------------------------------------
 */

static int monitor_counter = 1;

int MonitorCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, char *argv[])
{
  static char * too_few_args_string = "Too few arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | clear]";
  static char * too_many_args_string = "Too many arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | -clear]";
  char * monitor_id;
  char   buff[10];          /* What size is good here? */

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args_string;
      return TCL_ERROR;
    }

  if (string_match_up_to(argv[1], "-add", 2))
    {
      if (argc < 4)
	{
	  interp->result = too_few_args_string;
	  return TCL_ERROR;
	}

      if (argc > 5)
	{
	  interp->result = too_many_args_string;
	  return TCL_ERROR;
	}

      if (argc == 5)
	{
	  monitor_id = argv[4];
	}
      else
	{
	  sprintf(buff, "m%d", monitor_counter++);
	  monitor_id = buff;
	}

      {
	SOAR_CALLBACK_TYPE ct;
	
	ct = soar_callback_name_to_enum(argv[2], TRUE);
	if (ct)
	  {
	    soar_add_callback((agent *) clientData, 
			      ct, 
			      soar_callback_to_tcl, 
			      (soar_callback_data) savestring(argv[3]), 
			      soar_callback_data_free_string,
			      (soar_callback_id) monitor_id);
	    interp->result = monitor_id;
	    return TCL_OK;
	  }
	else
	  {
	    sprintf(interp->result,
		    "Attempt to add unrecognized callback event: %s",
		    argv[2]);
	    return TCL_ERROR;
	  }
      }
    }
  else if (string_match_up_to(argv[1], "-delete", 2))
    {
      switch (argc) {
      case 2:
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 3:	  /* Delete all callbacks of the given type */
	  {
	    SOAR_CALLBACK_TYPE ct;

	    ct = soar_callback_name_to_enum(argv[2], TRUE);
	    if (ct)
	      {
		soar_remove_all_callbacks_for_event((agent *) clientData, ct);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized callback event: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      case 4:	  /* Delete single callback for given event */
	  {
	    SOAR_CALLBACK_TYPE ct;

	    ct = soar_callback_name_to_enum(argv[2], TRUE);
	    if (ct)
	      {
		soar_remove_callback((agent *) clientData, ct, argv[3]);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized callback event: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }
    }
  else if (string_match_up_to(argv[1], "-list", 2))
    {
      switch (argc) {
      case 2:	  
	soar_list_all_callbacks((agent *) clientData, TRUE);
	break;
      case 3:	  
	{
	  SOAR_CALLBACK_TYPE ct;
	  
	  ct = soar_callback_name_to_enum(argv[2], TRUE);
	  if (ct)
	    {
	      soar_push_callback((soar_callback_agent) clientData,
				 PRINT_CALLBACK,
				 (soar_callback_fn) Soar_AppendResult, 
				 (soar_callback_data) NULL,
				 (soar_callback_free_fn) NULL);

		  /* if a log file is open, then we need to push a dummy callback
           * so that we don't get extraneous prints mucking up the log file.
           * Addresses bug # 248.  KJC 01/00 */
		  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
			  soar_push_callback((soar_callback_agent) clientData, 
	  	                LOG_CALLBACK,
		                (soar_callback_fn) Soar_DiscardPrint, 
		                (soar_callback_data) NULL,
		                (soar_callback_free_fn) NULL);
		  }

	      soar_list_all_callbacks_for_event((agent *) clientData, ct);

	      soar_pop_callback((soar_callback_agent) clientData,PRINT_CALLBACK);
		  if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
			  soar_pop_callback((soar_callback_agent) clientData, 
				                  LOG_CALLBACK);
		  }

	    }
	  else
	    {
	      sprintf(interp->result,
		      "Attempt to list unrecognized callback event: %s",
		      argv[2]);
	      return TCL_ERROR;
	    }
	}
	break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }

    }
  else if (string_match_up_to(argv[1], "-test", 2))
    {
      soar_test_all_monitorable_callbacks((agent *)clientData);
    }
  else if (string_match_up_to(argv[1], "-clear", 2))
    {
      /* Delete all callbacks of all types */
      soar_remove_all_monitorable_callbacks((agent *) clientData);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized option to monitor command: %s",
	      argv[1]);
      return TCL_ERROR;

    }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * MultiAttrCmd --
 *
 *      This is the command procedure for the "multi-attributes" command, 
 *      which enables a symbol to have multiple attribute values.
 * 
 *      If given, the value must be a positive integer > 1.  The 
 *      default is 10.  If no args are given on the cmdline, the list
 *      of symbols that are multi-attributed is printed.
 *
 * Syntax: multi-attributes symbol [value]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Defines the symbol to be multi-attributed or prints a list
 *      of symbols that are declared to multi-attributed.
 *
 *----------------------------------------------------------------------
 */

int MultiAttrCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, char *argv[])
{
  int num;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print_multi_attribute_symbols();
      return TCL_OK;
    }

  if (argc > 3)
    {
      interp->result = "Too many arguments, should be: multi-attribute [symbol] [value]";
      return TCL_ERROR;
    }

  get_lexeme_from_string(argv[1]);

  if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME)
    {
      sprintf(interp->result,
	      "Expected symbolic constant for symbol but got: %s\nUsage: multi-attributes [symbol] [value]\n",
	      argv[1]);      
      return TCL_ERROR;
    }

  if (argc == 3)
    {
      if (Tcl_GetInt(interp, argv[2], &num) != TCL_OK)
	{
	  sprintf(interp->result,
		  "Non-integer given for attribute count: %s\nUsage: multi-attributes [symbol] [value]\n",
		  argv[2]);      
	  return TCL_ERROR;
	}
      else
	{
	  if (num > 1)
	    {
	      add_multi_attribute_or_change_value(argv[1], num);
	    } 
	  else 
	    {
	      sprintf(interp->result,
		      "Integer must be greater than 1 but was %s",
		      argv[2]);      
	      return TCL_ERROR;
	    }
	}
    }
  else
    {
      add_multi_attribute_or_change_value(argv[1], 10);
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * OSupportModeCmd --
 *
 *	This is the command procedure for the "o-support-mode" command, 
 *	which controls the way o-support calculations are done (for the
 *      current agent).  It takes a single numeric argument: 
 *          0  --> do o-support calculations the normal (Soar 6) way.
 *          1  --> do o-support calculations the normal (Soar 6) way, but
 *                 print a warning message whenever a preference is created
 *                 that would get different support under Doug's proposal.
 *          2  --> do o-support calculations according to Doug Pearson's
 *                 proposal.  (See osupport.c for details.)
 *          3  --> for Soar 8, prods that have only operator elaborations
 *                 get i-support.  prods that mix operator elaborations
 *                 with operator applications get o-support and a warning
 *                 is printed to the user.  This is the default in v8.3
 *
 * Syntax:  o-support-mode {0 | 1 | 2 | 3}
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	Sets current_agent(o_support_calculation_type).
 *
 *----------------------------------------------------------------------
 */

int OSupportModeCmd (ClientData clientData, 
		     Tcl_Interp * interp,
		     int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 2)
    {
      interp->result = "Too many arguments.\nUsage: o-support-mode 0|1|2|3";
      return TCL_ERROR;
    }

  if (argc == 2) {
    if (! strcmp(argv[1], "0")) {
      current_agent(o_support_calculation_type) = 0;
    } else if (! strcmp(argv[1], "1")) {
      current_agent(o_support_calculation_type) = 1;
    } else if (! strcmp(argv[1], "2")) {
      current_agent(o_support_calculation_type) = 2;
    } else if (! strcmp(argv[1], "3")) {
      current_agent(o_support_calculation_type) = 3;
    } else {
      sprintf(interp->result,
	      "Unrecognized argument to %s: %s.  Integer 0, 1, 2, or 3 expected.",
	      argv[0], argv[1]);
      return TCL_ERROR;
    }
  }

  sprintf(interp->result, "%d", current_agent(o_support_calculation_type));
  return TCL_OK;
}

/* REW: begin 09.15.96 */
/*
 *----------------------------------------------------------------------
 *
 * Operand2Cmd --
 *
 *      This is the command procedure for the "soar8" command, which 
 *
 *      With no arguments, this command prints out the current operand state.
 *      Any of the following arguments may be given:
 *
 *        on         - turns operand2 on
 *        off        - turns operand2 off
 *
 * Syntax:  operand2 arg*
 *            arg  ::=  -on | -off 
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      re-initializes soar and puts it in the requested mode.
 *      default o_support_mode = 3 for -on; = 0 for -off
 *      Also modifies the soar_version_string.
 *
 *----------------------------------------------------------------------
 */

int Operand2Cmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  char buffer[1000];

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result,"Soar8 Mode is %s", (current_agent(operand2_mode) == TRUE) ? "ON":"OFF");
      return TCL_OK;
    }

  {int i;

  /* --- check for empty system --- */
  if (current_agent(all_wmes_in_rete)) {
    sprintf(interp->result,
	    "Can't change modes unless working memory is empty.");
    return TCL_ERROR;
  }
  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if (current_agent(num_productions_of_type)[i]) 
      {
	sprintf(interp->result,
		"Can't change modes unless production memory is empty.");
	return TCL_ERROR;
      }
  

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {

	   if (strcmp((const char *)MICRO_VERSION_NUMBER,"")) {
	     sprintf(buffer,
		     "Soar%d.%d.%d %s on : reinitializing Soar",
		     MAJOR_VERSION_NUMBER, 
		     MINOR_VERSION_NUMBER, 
		     MICRO_VERSION_NUMBER,
		     OPERAND2_MODE_NAME);
	   } else {
	     sprintf(buffer,
		     "Soar%d.%d %s on : reinitializing Soar",
		     MAJOR_VERSION_NUMBER, 
		     MINOR_VERSION_NUMBER,
		     OPERAND2_MODE_NAME);
	   }

       current_agent(operand2_mode) = TRUE;
	   if current_agent(o_support_calculation_type == 0) {
		   current_agent(o_support_calculation_type) = 3;
		   sprintf(buffer,"%s.  o_support_mode = 3", buffer);
	   }
	   print("%s\n",buffer);
       reinitialize_soar();
	 }

       else if (string_match_up_to("-fail", argv[i], 3))
	 {
	   char msg[128];
	   strcpy(msg,"soarCommands.c:  soar8 - fail issued.  aborting\n");
	   abort_with_fatal_error(msg);
	 }
	 
       else if (string_match_up_to("-off", argv[i], 3))
	 {
	   if (strcmp((const char *)MICRO_VERSION_NUMBER,"")) {
	     sprintf(buffer,
		     "Soar%d.%d.%d - running in Soar7 mode:  reinitializing Soar",
		     MAJOR_VERSION_NUMBER, 
		     MINOR_VERSION_NUMBER, 
		     MICRO_VERSION_NUMBER);
	   } else {
	     sprintf(buffer,
		     "Soar%d.%d - running in Soar7 mode: reinitializing Soar",
		     MAJOR_VERSION_NUMBER, 
		     MINOR_VERSION_NUMBER);
	   }

       current_agent(operand2_mode) = FALSE;
	   if current_agent(o_support_calculation_type == 3) {
		   current_agent(o_support_calculation_type) = 0;
		   sprintf(buffer,"%s.  o_support_mode = 0", buffer);
	   }
	   print("%s\n",buffer);
       reinitialize_soar();
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to the soar8 command: %s\nShould be soar8 [-on|-off]",
		   argv[i]);
	   return TCL_ERROR;
	 }
     }
  }

  return TCL_OK;
}
/* REW: end   09.15.96 */

/*
 *----------------------------------------------------------------------
 *
 * OutputStringsDestCmd --
 *
 *      This is the command procedure for the "output-strings-destination"
 *      command which redirects strings printed by Soar_PrintCmd to the
 *      selected destination.
 *
 *      If output-strings-destination is set to -append-to-result and 
 *      the C code performs an assignment to interp->result then
 *      the intermediate results will be lost (memory leak?).
 *
 * Syntax:  output-strings-destination [-push [ [-text-widget widget-name 
 *                                                           [interp-name]]
 *                                   | [-channel channel-id]
 * RMJ 7-1-97 *                      | [-procedure procedure-name]
 *                                   | -discard 
 *                                   | -append-to-result 
 *                                   ]
 *                            | -pop ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Changes the destination of Soar_Print commands.
 *
 *----------------------------------------------------------------------
 */
int OutputStringsDestCmd (ClientData clientData, 
			  Tcl_Interp * interp,
			  int argc, char *argv[])
{
  static char * too_few_args = "Too few arguments, should be: output-strings-destination [ -push [[-text-widget widget-name [interp-name]] | [-channel channel-id] | [-procedure tcl-procedure-name] | -discard |-append-to-result] | -pop]";

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  if (string_match("-push", argv[1]))
    {
      if (string_match("-text-widget", argv[2]))
	{
	  if (argc == 3)
	    {
	      interp->result = too_few_args;
	      return TCL_ERROR;
	    }
	  else
	    {
	      /* We assume that we'll be printing to the same interp. */
	      Tcl_Interp * print_interp = interp;
	      Soar_TextWidgetPrintData * data;

	      if (argc > 4)
		{
		  /* Too many arguments */
		  interp->result = "Too many arguments";
		  return TCL_ERROR;
		}

	      data = Soar_MakeTextWidgetPrintData (print_interp, argv[3]);
	      soar_push_callback((soar_callback_agent) clientData, 
				 PRINT_CALLBACK,
				 (soar_callback_fn) Soar_PrintToTextWidget,
				 (soar_callback_data) data,
				 (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
     	    }
	}
/* RMJ 7-1-97 */
      else if (string_match("-procedure", argv[2]))
	{
	  if (argc == 3)
	    {
	      interp->result = too_few_args;
	      return TCL_ERROR;
	    }
	  else
	    {
	      /* We assume that we'll be printing to the same interp. */
	      Tcl_Interp * print_interp = interp;
	      Soar_TextWidgetPrintData * data;

	      if (argc > 4)
		{
		  /* Too many arguments */
		  interp->result = "Too many arguments";
		  return TCL_ERROR;
		}

	      data = Soar_MakeTextWidgetPrintData (print_interp, argv[3]);
	      soar_push_callback((soar_callback_agent) clientData, 
				 PRINT_CALLBACK,
				 (soar_callback_fn) Soar_PrintToTclProc,
				 (soar_callback_data) data,
				 (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
     	    }
	}
      else if (string_match("-channel", argv[2]))
        {
		Tcl_Channel channel;
		int mode;

		if (argc == 3) {
			interp->result = too_few_args;
			return TCL_ERROR;
		}

		if ((channel = Tcl_GetChannel(interp, argv[3], &mode)) == NULL
		||  ! (mode & TCL_WRITABLE)) {
			sprintf(interp->result, "%s is not a valid channel for writing.", argv[3]);
			return TCL_ERROR;
		}

		soar_push_callback((soar_callback_agent) clientData,
			PRINT_CALLBACK,
			(soar_callback_fn) Soar_PrintToChannel,
			(soar_callback_data) channel,
			(soar_callback_free_fn) NULL);

	}
      else if (string_match("-discard", argv[2]))
	{
	  soar_push_callback((soar_callback_agent) clientData, 
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_DiscardPrint,
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	}
      else if (string_match("-append-to-result", argv[2]))
	{
	  soar_push_callback((soar_callback_agent) clientData, 
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_AppendResult,
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to %s %s: %s",
		  argv[0], argv[1], argv[2]);
	  return TCL_ERROR;      
	}
    }
  else if (string_match("-pop", argv[1]))
    {
      soar_pop_callback((soar_callback_agent) clientData,
			PRINT_CALLBACK);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to %s: %s",
	      argv[0], argv[1]);
      return TCL_ERROR;      
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ProductionFindCmd --
 *
 *      This is the command procedure for the "production-find"
 *      command, which finds Soar productions by characteristic.
 *
 *      pf is a production finding facility.  It allows you to
 *      find productions that either test a particular LHS pattern
 *      or produce particular RHS preferences.
 *
 *      The syntax of the lhs-conditions or rhs-actions is exactly
 *      their syntax within SP's.  In addition, the symbol '*' may
 *      be used as a wildcard for an attribute or value.  Note that
 *      variable names do not have to match the specific names used
 *      in productions.
 * 
 *      Specifying -chunks means only chunks are searched (and 
 *      -nochunks means no chunks are searched).
 *
 *      Note that leading blanks in the clause will cause pf to fail.
 *
 * Examples:
 *
 *      Find productions that test that some object gumby has 
 *      attribute ^alive t, and test an operator named foo:
 *        production-find {(<s> ^gumby <gv> ^operator.name foo)(<gv> ^alive t)}
 *
 *      Find productions that propose foo:
 *        production-find -rhs {(<x> ^operator <op> +)(<op> ^name foo)}
 *
 *      Find productions that test the attribute ^pokey:
 *        production-find {(<x> ^pokey *)}
 *
 *      Find productions that test the operator foo
 *        production-find {(<s> ^operator.name foo)}
 *
 * Syntax: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the productions found
 *
 *----------------------------------------------------------------------
 */

int ProductionFindCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, char *argv[])
{  
  int i;

  list *current_pf_list = NIL;

  bool lhs = TRUE;
  bool rhs = FALSE;
  bool show_bindings = FALSE;
  bool just_chunks = FALSE;
  bool no_chunks = FALSE;
  bool clause_found = FALSE;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = "No arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}";
      return TCL_ERROR;
    }

  /* the args parsing should really be done in a while loop, where we
     have better control over the loop vars, which we'll use later.
     But using clause_found will do for now...  */

  for (i = 1; i < argc; i++)
    {
      if (string_match_up_to(argv[i], "-lhs", 2))
	{
	  lhs = TRUE;
	}
      else if (string_match_up_to(argv[i], "-rhs", 2))
	{
	  rhs = TRUE;
	  lhs = FALSE;
	}
      else if (string_match_up_to(argv[i], "-show-bindings", 2))
	{
	  show_bindings = TRUE;
	}
      else if (string_match_up_to(argv[i], "-chunks", 2))
	{
	  just_chunks = TRUE ;
	}
      else if (string_match_up_to(argv[i], "-nochunks", 2))
	{
	  no_chunks = TRUE ;
	}
      else if (strchr(argv[i], '(') != 0)
	/* strchr allows for leading whitespace */
	{
	  /* it's the clause */
	  clause_found = TRUE ;
	  break;
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to %s command: %s",
		  argv[0], argv[i]);
	  return TCL_ERROR;
	}
    }

  if (!clause_found)
    {
      interp->result = "No clause found.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}";
      return TCL_ERROR;
    }

  if ((*argv[i] == '-') || (strchr(argv[i],'(') != 0))
    {
      if (argc > i + 1) 
	{
	  interp->result = "Too many arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}";
	  return TCL_ERROR;
	}
      if ((*argv[i] == '-') && (argc < i + 1))
	{
	  interp->result = "Too few arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}";
	  return TCL_ERROR;
	}
      if (lhs)
	{
      /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
	  /* Soar-Bugs #54 TMH */
	  /*  soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE); */
	   ((agent *)clientData)->alternate_input_string = argv[i];
	   ((agent *)clientData)->alternate_input_suffix = ") ";
	  
	  get_lexeme();
	  read_pattern_and_get_matching_productions (&current_pf_list,
						     show_bindings,
						     just_chunks, no_chunks);
	  /* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE);  */
	  current_agent(current_char) = ' ';
	}
      if (rhs)
	{
     /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
     /* Soar-Bugs #54 TMH */
     /* soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE);  */
	  ((agent *)clientData)->alternate_input_string = argv[i];
	  ((agent *)clientData)->alternate_input_suffix = ") ";
	  
	  get_lexeme();
	  read_rhs_pattern_and_get_matching_productions (&current_pf_list,
							 show_bindings,
							 just_chunks, 
							 no_chunks);
	  /* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE); */
	  current_agent(current_char) = ' ';
	}
      if (current_pf_list == NIL) 
	{
	  print("No matches.\n");
	}

      free_list(current_pf_list);
    }
  else
    {
      sprintf(interp->result,
	      "Unknown argument to %s command: %s",
	      argv[0], argv[i]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * PreferencesCmd --
 *
 *      This is the command procedure for the "preferences" command, 
 *      which prints all the preferences for the given slot.
 *
 *      This command prints all the preferences for the slot given 
 *      by the 'id' and 'attribute' arguments.  The optional 'detail' 
 *      argument must be one of the following (-none is the default):
 *
 *        -none     -- prints just the preferences themselves
 *        -names    -- also prints the names of the productions that 
 *                     generated them
 *        -timetags -- also prints the timetags of the wmes matched by 
 *                     the productions
 *        -wmes     -- prints the whole wmes, not just their timetags.
 *
 * kjh (CUSP-B7) begin
 * Syntax:  preferences [id] [attribute] [-none | -names | -timetags | -wmes]
 * Syntax:  preferences [id] [attribute] [ 0 | 1 | 2 | 3 ]
 * kjh (CUSP-B7) end
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints preference information.
 *
 *----------------------------------------------------------------------
 */

/* kjh (CUSP-B7): Replace samed named procedure in soarCommands.c */
int PreferencesCmd (ClientData clientData,
                    Tcl_Interp * interp,
                    int argc, char *argv[])
{
/* kjh (CUSP-B7) begin */
  static char * too_many_args = "Too many arguments.\nUsage: preferences [id] [attribute] [detail]";
  static char * wrong_args = "Usage: preferences [id] [attribute] [detail]";

  Symbol *id, *id_tmp, *attr, *attr_tmp;
  bool print_productions;
  wme_trace_type wtt;
  slot *s;
  preference *p;
  int i;

  Soar_SelectGlobalInterpByInterp(interp);

  /* Establish argument defaults: */
  id                = current_agent(bottom_goal);
  id_tmp            = NIL;
  attr              = current_agent(operator_symbol);
  attr_tmp          = NIL;
  print_productions = FALSE;
  wtt               = NONE_WME_TRACE;

  /* Parse provided arguments: */
  switch (argc) {
        case 1:
      /* No arguments; defaults suffice. */
    break;
    case 2:
          /* One argument; replace one of the defaults: */
      if (  (read_id_or_context_var_from_string(argv[1], &id_tmp)
             == TCL_ERROR)
         && (read_attribute_from_string(id, argv[1], &attr_tmp)
             == TCL_ERROR)
         && (read_pref_detail_from_string(argv[1], &print_productions, &wtt)
             == TCL_ERROR))  {             
              interp->result = wrong_args;
              return TCL_ERROR;
      }
    break;
    case 3:
          /* Two arguments; replace two of the defaults: */
      if (read_id_or_context_var_from_string(argv[1], &id_tmp) == TCL_ERROR) {
        id_tmp = id;
        if (read_attribute_from_string(id,argv[1], &attr_tmp) == TCL_ERROR) {
          interp->result = wrong_args;
          return TCL_ERROR;
        }
      }
      if (  (read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
             == TCL_ERROR)
         && (read_pref_detail_from_string(argv[2], &print_productions, &wtt)
             == TCL_ERROR))  {             
        interp->result = wrong_args;
        return TCL_ERROR;
      }
    break;
    case 4:
          /* Three arguments; replace (all) three of the defaults: */
      if (  (read_id_or_context_var_from_string(argv[1], &id_tmp)
             == TCL_ERROR)
         || (read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
             == TCL_ERROR)
         || (read_pref_detail_from_string(argv[3], &print_productions, &wtt)
             == TCL_ERROR))  {             
              interp->result = wrong_args;
              return TCL_ERROR;
      }
    break;
    default:
          /* Too many arguments; complain: */
      interp->result = too_many_args;
      return TCL_ERROR;
    break;
  }

/* kjh (CUSP-B7) end */

  /* --- print the preferences --- */
  if (id_tmp != NIL)
    id = id_tmp;
  if (attr_tmp != NIL)
    attr = attr_tmp;

  if (id == NIL)
    return(TCL_OK);

  s = find_slot (id, attr);
  if (!s)
    {
      sprintf(interp->result,
              "There are no preferences for %s ^%s.",
              argv[1], argv[2]);
    return TCL_ERROR;
    }

  print_with_symbols ("Preferences for %y ^%y:\n", id, attr);

  for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
    {
      if (s->preferences[i])
        {
          print ("\n%ss:\n", preference_name[i]);
          for (p = s->preferences[i]; p; p = p->next)
            {
              print_preference_and_source (p, print_productions, wtt);
            }
        }
    }

  return TCL_OK;
}


int OLD_PreferencesCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, char *argv[])
{
  static char * too_few_args  = "Too few arguments.\nUsage: preferences id attribute [detail]";
  static char * too_many_args = "Too many arguments.\nUsage: preferences id attribute [detail]";

  Symbol *id, *attr;
  bool print_productions;
  wme_trace_type wtt;
  slot *s;
  preference *p;
  int i;
  
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 3)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  if (argc > 4)
    {
      interp->result = too_many_args;
      return TCL_ERROR;
    }

  /* --- read id --- */

  if (read_id_or_context_var_from_string(argv[1], &id)
      == TCL_ERROR)
    {
      sprintf(interp->result,
	      "Known id or context var expected in preferences command, not %s.",
	      argv[1]);
      return TCL_ERROR;
    }

  /* --- get optional '^', if present --- */

  /* Unlike non-Tcl version, don't err if missing.              */
  
  if (*argv[2] == '^')
    {
      argv[2]++;                 /* Skip leading ^ */
    }
      
  /* --- read attribute --- */

  get_lexeme_from_string(argv[2]);

  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    attr = find_sym_constant (current_agent(lexeme).string); 
    break;
  case INT_CONSTANT_LEXEME:
    attr = find_int_constant (current_agent(lexeme).int_val); 
    break;
  case FLOAT_CONSTANT_LEXEME:
    attr = find_float_constant (current_agent(lexeme).float_val); 
    break;
  case IDENTIFIER_LEXEME:
    attr = find_identifier (current_agent(lexeme).id_letter, 
			    current_agent(lexeme).id_number); 
    break;
  case VARIABLE_LEXEME:
    attr = read_identifier_or_context_variable();
    if (!attr) 
      return TCL_ERROR;
    break;
  default:
    sprintf (interp->result,
	     "Expected constant or identifier for attribute, not %s.",
		   argv[2]);
    return TCL_ERROR;
  }

  if (argc == 3)
    { /* --- no optional level indicator --- */
      print_productions = FALSE;
      wtt = NONE_WME_TRACE;
    }
  else
    { /* --- read the optional level indicator --- */

      if (   string_match_up_to(argv[3], "-none", 3)
	  || string_match(argv[3], "0"))
	{
	  print_productions = FALSE;
	  wtt = NONE_WME_TRACE;
	}
      else if (   string_match_up_to(argv[3], "-names", 3)
	       || string_match(argv[3], "1"))
	{
	  print_productions = TRUE;
	  wtt = NONE_WME_TRACE;
	}
      else if (   string_match_up_to(argv[3], "-timetags", 2)
	       || string_match(argv[3], "2"))
	{
	  print_productions = TRUE;
	  wtt = TIMETAG_WME_TRACE;
	}
      else if (   string_match_up_to(argv[3], "-wmes", 2)
	       || string_match(argv[3], "3"))
	{
	  print_productions = TRUE;
	  wtt = FULL_WME_TRACE;
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized option to %s command: %s", 
		  argv[0], argv[3]);
	  return TCL_ERROR;
 
	}
    }

  /* --- print the preferences --- */

  s = find_slot (id, attr);
  if (!s) 
    {
      sprintf(interp->result,
	      "There are no preferences for %s ^%s.",
	      argv[1], argv[2]);
    return TCL_ERROR;
    }

  print_with_symbols ("Preferences for %y ^%y:\n", id, attr);

  for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
    {
      if (s->preferences[i]) 
	{
	  print ("\n%ss:\n", preference_name[i]);
	  for (p = s->preferences[i]; p; p = p->next)
	    {
	      print_preference_and_source (p, print_productions, wtt);
	    }
	}
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * PrintCmd --
 *
 *      This is the command procedure for the "print" command, which 
 *      prints various Soar items.
 * 
 *      The print command is used to print items from production 
 *      memory or working memory.  It can take several kinds of 
 *      arguments.  When printing items from working memory, the
 *      objects are printed unless the -internal flag is used, in
 *      in which case the wmes themselves are printed.
 *
 *      See also:  default-wme-depth
 *
 * Syntax:  print <stack>|<items>
 *          <stack>  ::= -stack [-state | -operator]*
 *          <items>  ::= [-depth n] [-internal] [-filename] <arg>*
 *          <arg>    ::= production-name  print that production
 *          <arg>    ::= production type  [-name | -full] -all | -chunks | 
 *                                        -defaults | -user | -justifications
 *          <arg>    ::= identifier       id of the object to print
 *          <arg>    ::= integer          timetag of wme--the identifier 
 *                                        from the wme indicates the object 
 *                                        to be printed
 *          <arg>    ::= <pattern>        pattern--same as if you listed 
 *                                        as arguments the timetags of all 
 *                                        wmes matching the pattern,
 *                                        often results in multiple copies
 *                                        of the same object being printed
 * 
 *        <pattern> ::= '{' '('  {identifier | '*'} 
 *                             ^ {attribute  | '*'} 
 *                               {value      | '*'} 
 *                               [+] ')' '}'
 *
 *      The optional [-depth n] argument overrides default-wme-depth.
 *      The optional [-internal] argument tells Soar to print things 
 *      in their internal form.  For productions, this means leaving 
 *      conditions in their reordered (rete net) form.  For wmes, this 
 *      means printing the individual wmes with their timetags, rather 
 *      than the objects.  The optional [-filename] argument tells Soar
 *      to print the filename for source'd productions.
 * 
 *      -depth 0 is meaningful only for integer and pattern arguments.  
 *      It causes only the matching wmes to be printed, instead of all 
 *      wmes whose id is an id in one of the matching wmes.
 *
 *      -name | -full apply to printing productions.  For the specified
 *      production type, only the production name will be printed,
 *      unless the -full flag is specified.  For named productions, the
 *      default is to print the entire production unless -name is specified.
 *      (-name for an individual prod seems silly, but it's included
 *      for completeness...)
 *
 *      The -depth n, -internal, -name, and -full flags apply only to the
 *      args that appear after them on the command line.
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the selected objects.
 *
 * kjh CUSP(B11) =  
 *     "Soar should know the file name from which a given production was loaded."  
 *
 *      The changes here to PrintCmd are meant to be 
 *      more illustrative of how to provide a production's file name to the
 *      user than they are meant to be final changes to PrintCmd, which 
 *      is undergoing concurrent work at this time.
 *
 *      print -filename <production-name>+
 *
 *----------------------------------------------------------------------
 */

int PrintCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  static char * too_few_args  = "Too few arguments.\nUsage: print [-depth n] [-internal] arg*";

  bool internal;
  bool name_only, full_prod;
  bool output_arg; /* Soar-Bugs #161 */
  bool print_filename;  /* CUSP (B11) kjh */

  int depth;
  production * prod;
  Symbol *id;
  wme *w;
  list *wmes;
  cons *c;
  int i,next_arg;

  Soar_SelectGlobalInterpByInterp(interp);

  internal = FALSE;
  depth = current_agent(default_wme_depth);   /* AGR 646 */
  name_only = FALSE;
  full_prod = FALSE;
  print_filename = FALSE;  /* kjh CUSP(B11) */
  output_arg = FALSE; /* Soar-Bugs #161 */

  if (argc == 1)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  next_arg = 1;

  /* --- see if we have the -stack option --- */
  if (string_match_up_to("-stack", argv[next_arg],4)) 
    {
      bool print_states;
      bool print_operators;

      /* Determine context items to print */

      if (argc == 2)                    /* No options given */
	{
	  print_states    = TRUE;
	  print_operators = TRUE;
	}
      else
	{
	  int i;

	  print_states    = FALSE;
	  print_operators = FALSE;

	  next_arg++;
	  for (i = next_arg; i < argc; i++)
	    {
	      if (string_match_up_to(argv[i], "-states", 2))
		print_states = TRUE;
	      else if (string_match_up_to(argv[i], "-operators", 2))
		print_operators      = TRUE;
	      else
		{
		  sprintf(interp->result,
			  "Unknown option passed to print -stack: (%s).\nUsage print -stack [-state | -operator]*",
			  argv[next_arg]);
		  return TCL_ERROR;
		}
	    }
	}

      {
	Symbol *g;

	for (g = current_agent(top_goal); g != NIL; g = g->id.lower_goal) 
	  {
	    if (print_states)
	      {
		print_stack_trace (g, g, FOR_STATES_TF, FALSE);
		print ("\n");
	      }
	    if (print_operators && g->id.operator_slot->wmes) 
	      {
		print_stack_trace (g->id.operator_slot->wmes->value,
				   g, FOR_OPERATORS_TF, FALSE);
		print ("\n");
	      }
	  }
      }

      return TCL_OK;      
    } /* End of string_match "-stack" */


  /* --- repeat: read one arg and print it --- */
  while (next_arg < argc)
    {

      /* --- read optional -depth flag --- */
      if (string_match_up_to("-depth", argv[next_arg],4))
	{
	  if ((argc - next_arg) <= 1)
	    {
	      interp->result = too_few_args;
	      return TCL_ERROR;
	    }
	  else
	    {
	      if (Tcl_GetInt(interp, argv[++next_arg], &depth) != TCL_OK)
		{
		  sprintf (interp->result,
			   "Integer expected after %s, not %s.",
			   argv[next_arg-1], argv[next_arg]);
		  return TCL_ERROR;
		}
	    }
	}

      /* --- read optional -internal flag --- */
      else if (string_match_up_to("-internal", argv[next_arg],2))
	{
	  internal = TRUE;
	}

      /* kjh CUSP(B11) begin */
      /* --- read optional -filename flag --- */
      
      else if (string_match_up_to("-filename", argv[next_arg],2)) {
	print_filename = TRUE;
      }
      /* kjh CUSP(B11) end */

      /* --- read optional -name flag --- */
      else if (string_match_up_to("-name", argv[next_arg],2))
	{
	  name_only = TRUE;
	  full_prod = FALSE; /* Soar-Bugs #161 */
	}
      /* --- read optional -full flag --- */
      else if (string_match_up_to("-full", argv[next_arg],2))
	{
	  full_prod = TRUE;
	  name_only = FALSE; /* Soar-Bugs #161 */
	}
      else if (string_match_up_to("-all", argv[next_arg],2))
	{
          output_arg = TRUE; /* Soar-Bugs #161 */
	  for (i=0; i < NUM_PRODUCTION_TYPES; i++) {
	  /* want chunks printed out starting with lowest-numbered chunks.
	     so go to the end of the list and step backward. 94.11.02 */
	    for (prod=current_agent(all_productions_of_type)[i];
		 prod != NIL && prod->next != NIL;
		 prod = prod->next)
	      /* intentionally null */ ;
	    while (prod != NIL) {
	      /* if (!full_prod) {
	       *   print_with_symbols("%y",prod->name);
	       * } else {
	       *   print_production (prod, internal);
	       * }
	       * print("\n");
	       */
              /* next line CUSP B11 kjh */
              do_print_for_production(prod,internal,
				      print_filename,full_prod);
	      prod=prod->prev;
	    }
	  }
	}
      else if (string_match_up_to("-defaults", argv[next_arg],4))
	{
          output_arg = TRUE; /* Soar-Bugs #161 */
	  for (prod=current_agent(all_productions_of_type)
		 [DEFAULT_PRODUCTION_TYPE];
	       prod != NIL;
	       prod = prod->next)
	    {
	      /* if (!full_prod) {
	       *   print_with_symbols("%y",prod->name);
	       * } else {
	       *   print_production (prod, internal);
	       * }
	       * print("\n");
	       */
              /* CUSP B11 kjh */
              do_print_for_production(prod,internal,
				      print_filename,full_prod);
	    }
	}
      else if (string_match_up_to("-user", argv[next_arg],2))
	{
          output_arg = TRUE; /* TEST for Soar-Bugs #161 */
	  for (prod=current_agent(all_productions_of_type)
		 [USER_PRODUCTION_TYPE];
	       prod != NIL;
	       prod = prod->next)
	    {
	      /* if (!full_prod) {
	       * 	print_with_symbols("%y",prod->name);
	       * } else {
	       * 	print_production (prod, internal);
	       * }
	       * print("\n");
	       */
              /* CUSP B11 kjh */
              do_print_for_production(prod,internal,
				      print_filename,full_prod);
	    }
	}
      else if (string_match_up_to("-chunks", argv[next_arg],2))
	{
          output_arg = TRUE; /* Soar-Bugs #161 */
	  /* want chunks printed out starting with lowest-numbered chunks.
	     so go to the end of the list and step backward. 94.11.02 */
	  for (prod=current_agent(all_productions_of_type)
		 [CHUNK_PRODUCTION_TYPE];
	       prod != NIL && prod->next != NIL;
	       prod = prod->next)
	    /* intentionally null */ ;
	  while (prod != NIL) {
	    /* if (!full_prod) {
	     *    print_with_symbols("%y",prod->name);
	     * } else {
	     *    print_production (prod, internal);
	     * }
	     * print("\n");
	     */
	    /* next line CUSP B11 kjh */
            do_print_for_production(prod,internal,
				    print_filename,full_prod);
	    prod=prod->prev;
	  }
	}
      else if (string_match_up_to("-justifications", argv[next_arg],2))
	{
          output_arg = TRUE; /* Soar-Bugs #161 */
	  /* want justifications printed out starting with lowest-numbered,
	     so go to the end of the list and step backward. 94.11.02 */
	  for (prod=current_agent(all_productions_of_type)
		 [JUSTIFICATION_PRODUCTION_TYPE];
	       prod != NIL && prod->next != NIL;
	       prod = prod->next)
	    /* intentionally null */ ;
	  while (prod != NIL) {
	    /* if (!full_prod) {
	     *   print_with_symbols("%y",prod->name);
	     * } else {
	     *   print_production (prod, internal);
	     * }
	     * print("\n");
	     */
	    /* next line CUSP B11 kjh */
            do_print_for_production(prod,internal,
				    print_filename,full_prod);
	    prod=prod->prev;
	    }
	}
      else if (string_match_up_to("-",argv[next_arg],1))
	{
	  sprintf(interp->result,
		  "Unrecognized option to print command: %s",
		  argv[next_arg]);
	  return TCL_ERROR;
	}
      else { /* check for production name or wme */

	get_lexeme_from_string(argv[next_arg]);

	switch (current_agent(lexeme).type) {
	case SYM_CONSTANT_LEXEME:
          output_arg = TRUE; /* Soar-Bugs #161 */
	  if (!name_only || print_filename) {
        /* kjh CUSP(B11) begin */
   	    do_print_for_production_name (argv[next_arg], internal, 
                                          print_filename, full_prod);
	  } else {
	    print("%s\n",argv[next_arg]);
	  }
	  break;
	  
	case INT_CONSTANT_LEXEME:
          output_arg = TRUE; /* Soar-Bugs #161 */
	  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
	    if (w->timetag == (unsigned long)current_agent(lexeme).int_val) break; 
		/* rmarinie: added explicit conversion from long to unsigned long
			 above: in general current_agent(lexeme).int_val can be
			 negative, but in this case it doesn't make sense (i.e. timetags
			 are always positive), so this conversion makes more sense.
			 This decision was arrived at after discussion with kcoulter.
		*/

	  if (w) {
	    do_print_for_wme (w, depth, internal);
	  } else {
	    sprintf(interp->result,
		    "No wme %ld in working memory", 
		    current_agent(lexeme).int_val);
	    return TCL_ERROR;
	  }
	  break;

	case IDENTIFIER_LEXEME:
	case VARIABLE_LEXEME:
          output_arg = TRUE; /* Soar-Bugs #161 */
	  id = read_identifier_or_context_variable();
	  if (id) do_print_for_identifier (id, depth, internal);
	  break;
      
	case QUOTED_STRING_LEXEME:
          output_arg = TRUE; /* Soar-Bugs #161 */
          /* Soar-Bugs #54 TMH */
          soar_alternate_input((agent *)clientData, argv[next_arg], ") ",TRUE);
	  /* ((agent *)clientData)->alternate_input_string = argv[next_arg];
	   * ((agent *)clientData)->alternate_input_suffix = ") ";
	   */
	  get_lexeme();
	  wmes = read_pattern_and_get_matching_wmes ();
          soar_alternate_input((agent *)clientData, NIL, NIL, FALSE); 
	  current_agent(current_char) = ' ';
	  for (c=wmes; c!=NIL; c=c->rest)
	    do_print_for_wme (c->first, depth, internal);
	  free_list (wmes);
	  break;

	default:
	  sprintf(interp->result,
		  "Unrecognized argument to %s command: %s",
		  argv[0], argv[next_arg]);
	  return TCL_ERROR;
	} /* end of switch statement */
	output_arg = TRUE;  /* Soar-bugs #161 */
      } /* end of if-else stmt */
      next_arg++;
  } /* end of while loop */

  /* Soar-Bugs #161 */
  if (!output_arg) {
    interp->result = too_few_args;
    return TCL_ERROR;
  } else
    return TCL_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * PwatchCmd --
 *
 *      This is the command procedure for the "pwatch" command
 *      which enables the tracing of production firing.
 *
 *      This command enables tracing of the firings and retractions 
 *      of individual productions.  (This mechanism is orthogonal to 
 *      the watch -firings mechanism.  See the "watch" command for more 
 *      information.  Pwatch, with no arguments, lists the productions 
 *      currently being traced.  With one or more production name 
 *      arguments, it enables tracing of those productions.  Using the 
 *      -on or -off option explicitly turns tracing on or off for the
 *      given productions.  Tracing persists until disabled, or until
 *      the production is excised.
 *
 *      See also:  watch, excise
 *
 * Syntax:  pwatch [-on | -off] production-name*
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Enables the tracing of the specified productions.
 *
 *----------------------------------------------------------------------
 */

int PwatchCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, char *argv[])
{
  bool trace_productions = TRUE;
  int next_arg = 1;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      cons *c;

      for (c=current_agent(productions_being_traced); c!=NIL; c=c->rest)
	print_with_symbols (" %y\n", ((production *)(c->first))->name);  

      return TCL_OK;
    }

  /* Check for optional argument */

  if (string_match_up_to(argv[1], "-on", 3))
    {
      trace_productions = TRUE;
      next_arg++;
    }
  if (string_match_up_to(argv[1], "-off", 3))
    {
      trace_productions = FALSE;
      next_arg++;
    }

  if ((argc == 2) && (next_arg != 1))
    {
      /* Turn on/off all productions */

      if (trace_productions)
	{
	  /* List all productions that are currently being traced */
	  cons *c;

	  for (c=current_agent(productions_being_traced); c!=NIL; c=c->rest)
	    print_with_symbols (" %y\n", ((production *)(c->first))->name);  

	  return TCL_OK;
	}
      else
	{
	  /* Stop tracing all productions */
	  cons *c, *next;

	  /*
           * We don't use c=c->rest in the increment step because 
           * remove_pwatch may release c as a side-effect.
           */

	  for (c=current_agent(productions_being_traced); c!=NIL; c=next)
	    {
	      production * prod;

	      next = c->rest;
	      prod = current_agent(productions_being_traced)->first;
	      remove_pwatch(prod);
	    }
	  
	  return TCL_OK;
	}
    }

  /* Otherwise, we have a list of productions to process */
  {
    int i;

    for (i = next_arg; i < argc; i++)
      {
	production * prod;
	
	if ((prod = name_to_production(argv[i])))
	  {
	    if (trace_productions)
	      {
		add_pwatch(prod);
	      }
	    else
	      {
		remove_pwatch(prod);
	      }
	  }
	else
	  {
	    sprintf(interp->result, "No production named %s", argv[i]);
	    return TCL_ERROR;
	  }
      }
    
    return TCL_OK;
  }
}

/*
 *----------------------------------------------------------------------
 *
 * QuitCmd --
 *
 *      This is the command procedure for the "quit" command, 
 *      which exits Soar after cleaning up.
 * 
 * Syntax:  quit
 *
 * Results:
 *      Does not return.
 *
 * Side effects:
 *      Exits the Soar system.
 *
 *----------------------------------------------------------------------
 */

int QuitCmd (ClientData clientData, 
	     Tcl_Interp * interp,
	     int argc, char *argv[])
{
  static char cmd[] = "exit";
  if (argc > 1)
    {
      interp->result = "Too many arguments, should be: quit";
      return TCL_ERROR;
    }

  print ("Exiting Soar...\n");
  just_before_exit_soar();

  /* Soar-Bugs #58, TMH */
  while (soar_exists_callback((soar_callback_agent) clientData,
			      LOG_CALLBACK))
  {
    Soar_Log((agent *) clientData, "\n**** quit cmd issued ****\n");
    soar_pop_callback((soar_callback_agent) clientData,
	LOG_CALLBACK);
  }

  (void) Tcl_Eval(interp, cmd);
  return TCL_OK; /* Unreachable, but here to placate the compiler */
}

#ifdef NADA
/* kjh(CUSP-B10) begin */
/*
 *----------------------------------------------------------------------
 *
 * RecordCmd --
 *
 *      This is the command procedure for the "record" command which 
 *      records session input to a file for later "replay."
 *
 *      This command may be used to open and close record files.
 *
 * Syntax:  record [[-new | -existing] fileName |-off]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes record files.
 *
 *----------------------------------------------------------------------
 */

int RecordCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  char * too_few = "Too few arguments; should be: record [[-new | -existing] fileName |-off]";
  char * too_many = "Too many arguments; should be: record [[-new | -existing] fileName |-off]";

  if (argc < 2) {
    interp->result = "The record file is ";
    if (soar_exists_callback((soar_callback_agent) clientData, RECORD_CALLBACK)) {
      Tcl_AppendResult(interp, 
			   "open.  Use record -off to close the file.", 
			   (char *) NULL); 
    } else {
      Tcl_AppendResult(interp, "closed.", (char *) NULL); 
    }
    Tcl_AppendResult(interp, "", (char *) NULL);
    return TCL_OK;      
  }
  if (argc > 3) {
    interp->result = too_many;
    return TCL_ERROR;      
  }
  if (string_match("-off", argv[1])) {
    if (argc == 2) {
      if (soar_exists_callback((soar_callback_agent) clientData, RECORD_CALLBACK)) {
        Soar_LogAndPrint((agent *) clientData, "---- recording off ----\n");
        soar_pop_callback((soar_callback_agent) clientData, RECORD_CALLBACK);
      } else {
        if (current_agent(replaying))
          Soar_EndReplay((agent *) clientData);
        else {
          interp->result = "Recording already is off.";
          return TCL_ERROR;
        }
      }
    } else if (argc > 2) {
      interp->result = too_many;
      return TCL_ERROR;      
    }
  } else { /* Either we have a file open/append request or there is an error */
      char * filename;
      char * mode;
      Tcl_DString buffer;
      char * fullname;
      FILE * f;

      if (argc != 3) {
        interp->result = "syntax error: record [[-new | -existing] fileName |-off]";
        return TCL_ERROR;
      } else if (string_match("-new", argv[1])) {
	filename = argv[2];
	mode = "w";
      } else if (string_match("-existing", argv[1])) {
	filename = argv[2];
	mode = "a";
      } else {
        interp->result = "syntax error: record [[-new | -existing] fileName |-off]";
        return TCL_ERROR;
      }

#ifdef _CodeWarrior_
      fullname = filename;
#else
      fullname = Tcl_TildeSubst(interp, filename, &buffer);
#endif
      if (fullname == NULL) {
        return TCL_ERROR;
      }
      if (!(f = fopen(fullname, mode))) {
        Tcl_DStringFree(&buffer);
        sprintf (interp->result,
                 "record: Error: unable to open '%s'", filename);
        return TCL_ERROR;
      } else {
        /* Install callback */
        soar_push_callback((soar_callback_agent) clientData, 
                           RECORD_CALLBACK,
                           (soar_callback_fn) Soar_RecordToFile,
                           (soar_callback_data) f,
                           (soar_callback_free_fn) Soar_FClose);
        Soar_LogAndPrint((agent *) clientData, "---- recording on ----\n");
      }
  }
  return TCL_OK;
}
/*
 *----------------------------------------------------------------------
 *
 * ReplayCmd --
 *
 *      This is the command procedure for the "replay" command which 
 *      replays session input from a file created earlier by "record."
 *
 * Syntax:  replay fileName
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      The commands in fileName can indeed have side effects.
 *
 *----------------------------------------------------------------------
 */

int ReplayCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  char * too_few  = "Too few arguments; should be: replay fileName";
  char * too_many = "Too many arguments; should be: replay fileName";
  char * fullname;
  Tcl_DString buffer;
  FILE * f;

  if (argc > 3) {
      interp->result = too_many;
      return TCL_ERROR;      
  }
  if (argc < 2)  {
      interp->result = too_few;
      return TCL_ERROR;      
  }

  if (soar_exists_callback((soar_callback_agent) clientData, RECORD_CALLBACK)) {
    interp->result = "Recording is in progress.  Turn off first with \"record -off\".";
    return TCL_ERROR;      
  }
#ifdef _CodeWarrior_
  fullname = argv[1];
#else
  fullname = Tcl_TildeSubst(interp, argv[1], &buffer);
#endif
  if (fullname == NULL) {
    return TCL_ERROR;
  }
  if (!(f = fopen(fullname, "r"))) {
    Tcl_DStringFree(&buffer);
    sprintf (interp->result, "Error: replay: unable to open '%s'", fullname);
    return TCL_ERROR;
  } else { /* Install callback */
    soar_pop_callback((soar_callback_agent) clientData, READ_CALLBACK);
    soar_push_callback((soar_callback_agent) clientData, 
                       READ_CALLBACK,
                       (soar_callback_fn) Soar_ReadFromFile,
                       (soar_callback_data) f,
                       (soar_callback_free_fn) Soar_FClose);
    Soar_LogAndPrint((agent *) clientData, "---- replay beginning ----\n");
    current_agent(replaying) = true;
  }
  return TCL_OK;
}
/* kjh(CUSP-B10) end */
#endif

/*
 *----------------------------------------------------------------------
 *
 * ReplayInputCmd --
 *
 *      This is the command procedure for the "replay-input" command
 *      which reads input wme commands (add|remove) from a file.
 *
 *      This command may be used to start and stop the reading of
 *      input wmes from a file created by the "capture-input" command.  
 *      The routine replay-input-wme is registered as an input function
 *      to read input wmes from the file decision cycle by decision cycle.
 *      If an EOF is reached, the file is closed and the callback removed.
 *      Use the command capture-input to create the sequence.
 *
 * Syntax:  replay-input <action>
 *          <action> ::= -open pathname 
 *          <action> ::= -query
 *          <action> ::= -close
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes captured input files.
 *      Registers (or removes) input function callback.
 *
 *----------------------------------------------------------------------
 */

int ReplayInputCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  char  header[80];
  char * too_few = "Too few arguments, should be: replay-input [-open pathname | -query | -close]";
  char * too_many = "Too many arguments, should be: replay-input [-open pathname | -query | -close]";

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 2) {
      interp->result = "The replay file is ";
      if (current_agent(replay_fileID))	{
		  Tcl_AppendResult(interp, 
			  "open.  Use replay-input -close to close the file.", 
			  (char *) NULL); 
	  } else	{
		  Tcl_AppendResult(interp, "closed.", (char *) NULL); 
	  }
      Tcl_AppendResult(interp, "", (char *) NULL);
	  
      return TCL_OK;      
  }
  
  if (argc > 3)    {
      interp->result = too_many;
      return TCL_ERROR;      
  }
 
  if (string_match_up_to("-query", argv[1], 2))    {
      if (argc == 2) {
		  if (current_agent(replay_fileID)) {
			  interp->result = "open";
		  } else {
			  interp->result = "closed";
		  }
	  } else  {
		  interp->result = too_many;
		  return TCL_ERROR;      
	  }

  } else if (string_match_up_to("-close", argv[1], 2)) {
      if (argc == 2) {
		  if (current_agent(replay_fileID))  {
			  fclose(current_agent(replay_fileID));
			  current_agent(replay_fileID) = NIL;
			  remove_input_function((agent *) clientData,"replay_input");
			  interp->result = "replay file closed";
			  /* free the timetags array */
		  } else {
			  interp->result = "Attempt to close non-existant capture file";
			  return TCL_ERROR;
		  }
	  } else if (argc > 2) {
		  interp->result = too_many;
		  return TCL_ERROR;      
	  }
 
  } else { 
	  /* Either we have a file open request or there is an error */
      char * filename;
      char * mode;

	  if (argc == 3) {
		  if (string_match_up_to("-open", argv[1], 2)) {
			  filename = argv[2];
			  mode = "r";
		  } else if (string_match_up_to("-open", argv[2], 2)) {
			  filename = argv[1];
			  mode = "r";
		  } else {
			  sprintf(interp->result,
				  "replay-input: unrecognized arguments: %s %s",
				  argv[1], argv[2]);
			  return TCL_ERROR;
		  }
	  } else {
		  interp->result = too_few;
		  return TCL_ERROR;      
	  }
	  
	  
      {
		  /* we have -open and a filename;  */
		  Tcl_DString buffer;
		  char * fullname;
		  FILE * f;
       		  
          #if defined(MACINTOSH)
		  fullname = filename;
          #else
		  fullname = Tcl_TildeSubst(interp, filename, &buffer);
          #endif
		  
		  if (fullname == NULL) {
			  return TCL_ERROR;
		  }
		  

		  if (!(f = fopen(fullname, mode)))  {
			  Tcl_DStringFree(&buffer);
			  sprintf (interp->result,
				  "replay-input: Error: unable to open '%s'",
				  filename);
			  return TCL_ERROR;
		  } else  {
			  long loc;
			  char input[1024];
			  int numargs, cycle;
			  unsigned long old_timetag;
			  current_agent(replay_fileID) = f;
			  Tcl_DStringFree(&buffer);
			  fgets(header,28,f);
			  if (strcmp(header,"##soar captured input file.") == 0) {
				  sprintf (interp->result,
					  "input replay file '%s' opened",
					  filename);
				  add_input_function((agent *) clientData,
					  (soar_callback_fn) replay_input_wme,
					  NULL, 
					  NULL,
					  "replay_input");
				  /* save the fileptr location and find highest timetag */
				  loc = ftell(f);
				  while (!feof(f)) { 
					  numargs = fscanf(f,"%i : %i :", &cycle, &old_timetag);
					  fgets(input,1024,f);
				  }
				  /* malloc an array for indexing timetags and reset file ptr. */
				  current_agent(replay_timetags) = (unsigned long *) allocate_memory((old_timetag * sizeof(old_timetag)),
					  current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
				  fseek(f,loc,SEEK_SET);
			  } else {
				  sprintf (interp->result,
					  "file '%s' not a captured wme file: %s",
					  filename,header);
				  fclose(f);
				  current_agent(replay_fileID) = NIL;
				  return TCL_ERROR;
			  }
		  }
	  }  
  }

  return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * RemoveWmeCmd --
 *
 *      This is the command procedure for the "remove-wme" command, 
 *      which removes a wme from Soar's memory.
 *
 *      WARNING: this command is inherently unstable and may have 
 *      weird side effects (possibly even including system crashes).  
 *      For example, the chunker can't backtrace through wmes created 
 *      via add-wme.  Removing input wmes or context/impasse wmes may 
 *      have unexpected side effects.  You've been warned.
 *
 *      See also:  add-wme
 *
 * Syntax:  remove-wme integer
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Removes the selected working memory element from Soar.
 *      If capturing input, copies the cmd to the capture file. (v8.3+)
 *
 *----------------------------------------------------------------------
 */

int RemoveWmeCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, char *argv[])
{
	int num;
	
	Soar_SelectGlobalInterpByInterp(interp);
	
	if (argc == 1) {
		interp->result = "Too few arguments, should be: remove-wme integer";
		return TCL_ERROR;
    }
	
	if (argc > 2) {
		interp->result = "Too many arguments, should be: remove-wme integer";
		return TCL_ERROR;
    }
	
	if (Tcl_GetInt(interp, argv[1], &num) == TCL_OK) {
		wme *w, *w2;
		Symbol *id;
		slot *s;
		
		for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
			if (w->timetag == (unsigned long) num) 
				break;
			
			if (!w) {
				sprintf (interp->result, "No wme %d in working memory\n", num);
				return TCL_ERROR;
			}
			
			id = w->id;
			
			/* --- remove w from whatever list of wmes it's on --- */
			for (w2=id->id.input_wmes; w2!=NIL; w2=w2->next)
				if (w==w2) break;
			if (w2) remove_from_dll (id->id.input_wmes, w, next, prev);
			for (w2=id->id.impasse_wmes; w2!=NIL; w2=w2->next)
				if (w==w2) break;
			if (w2) remove_from_dll (id->id.impasse_wmes, w, next, prev);
			for (s=id->id.slots; s!=NIL; s=s->next) {
				for (w2=s->wmes; w2!=NIL; w2=w2->next)
					if (w==w2) break;
				if (w2) remove_from_dll (s->wmes, w, next, prev);    
				for (w2=s->acceptable_preference_wmes; w2!=NIL; w2=w2->next)
					if (w==w2) break;
				if (w2) remove_from_dll (s->acceptable_preference_wmes, w, next, prev);
			}
			
			/* KJC 11/99 begin: */
			/* if input capturing is enabled, save any input changes to capture file */
			if (current_agent(capture_fileID) && 
				(current_agent(current_phase) == INPUT_PHASE))
				capture_input_wme(w, NULL, NULL, NULL, "remove");
			/* KJC 11/99 end */

			/* REW: begin 09.15.96 */
			if (current_agent(operand2_mode)){
				if (w->gds) {
					if (w->gds->goal != NIL){
						if (current_agent(soar_verbose_flag)) printf("\nRemoveWME: Removing goal %d because element in GDS changed.\n", w->gds->goal->id.level);
						gds_invalid_so_remove_goal(w);
						/* NOTE: the call to remove_wme_from_wm will take care of checking if
						GDS should be removed */
					}
				}
			}
			/* REW: end   09.15.96 */

			/* --- now remove w from working memory --- */
			remove_wme_from_wm (w);

			/* REW: begin 28.07.96 */
			/* See AddWme for description of what's going on here */
			/* ditto for KJC timing changes...*/
            /* #if 0 */
	        #ifndef NO_TIMING_STUFF
			if (current_agent(current_phase) == INPUT_PHASE) {
				/* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
				stop_timer (&current_agent(start_kernel_tv), 
					&current_agent(input_function_cpu_time));
				start_timer (&current_agent(start_kernel_tv));
				start_timer (&current_agent(start_phase_tv)); 
			}
            #endif
			/* REW: end 28.07.96 */
			
			do_buffered_wm_and_ownership_changes();
			
			/* REW: begin 28.07.96 */
            #ifndef NO_TIMING_STUFF
			if (current_agent(current_phase) == INPUT_PHASE) {
				stop_timer (&current_agent(start_phase_tv), 
					&current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
				stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
				start_timer (&current_agent(start_kernel_tv));
			}
            #endif
			/* REW: end 28.07.96 */   
            /* #endif  /* KJC timer changes 12/99 */

#if 0
			/* KJC new timer changes: 12/99 */
			if (current_agent(current_phase) != INPUT_PHASE) {
                #ifndef NO_TIMING_STUFF
				start_timer (&current_agent(start_kernel_tv));
				start_timer (&current_agent(start_phase_tv)); 
                #endif  

				do_buffered_wm_and_ownership_changes();
				
                #ifndef NO_TIMING_STUFF
				stop_timer (&current_agent(start_phase_tv), 
					&current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
				stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
				start_timer (&current_agent(start_kernel_tv));
                #endif
			}
#endif
			    
	} else { /* couldn't parse a valid arg */
		sprintf(interp->result,
			"Unrecognized argument to remove-wme command: %s",
			argv[1]);
		return TCL_ERROR;
    }
	
	return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ReteNetCmd --
 *
 *      This is the command procedure for the "rete-net" command, 
 *      which saves and restores the state of the Rete network.
 *
 * Syntax:  rete-net option filename
 *            option ::= -save | -load
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Loads or saves the Rete network using the file "filename"
 *
 *----------------------------------------------------------------------
 */

int ReteNetCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, char *argv[])
{
  char * pipe_command;
  char * access_method;
  bool (*rete_net_op) (FILE *);
  bool using_compression_filter;

  Tcl_DString buffer;
  Tcl_DString command_line;
  char * fullname;
  FILE * f;
  bool result;
  bool loading;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 3)
    {
      interp->result = "Too few arguments.\nUsage: rete-net {-save | -load} filename.";
      return TCL_ERROR;
    }

  if (argc > 3)
    {
      interp->result = "Too many arguments.\nUsage: rete-net {-save | -load} filename.";
      return TCL_ERROR;
    }
  
  if (string_match("-save", argv[1]))
    {
      if (current_agent(all_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]) 
	{
	  interp->result = "Can't save rete while there are justifications.";
	  return TCL_ERROR;
	}

      pipe_command  = "compress > ";
      access_method = "wb";
      rete_net_op   = save_rete_net;        /* A function pointer */
      loading       = FALSE;
    }
  else if (string_match("-load", argv[1]))
    {
      int i;

      /* --- check for empty system --- */
      if (current_agent(all_wmes_in_rete)) {
	interp->result = "Can't load rete unless working memory is empty.";
	return TCL_ERROR;
      }

      for (i=0; i<NUM_PRODUCTION_TYPES; i++)
	if (current_agent(num_productions_of_type)[i]) 
	  {
	    interp->result = "Can't load rete unless production memory is empty.";
	    return TCL_ERROR;
	  }
  
      pipe_command  = "zcat ";
      access_method = "rb";
      rete_net_op   = load_rete_net;        /* A function pointer */
      loading       = TRUE;
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to rete-net command: %s",
	      argv[1]);
      return TCL_ERROR;
    }

  fullname = Tcl_TildeSubst(interp, argv[2], &buffer);
  if (fullname == NULL)
    {
      return TCL_ERROR;
    }
  
  /* cd to dir on top of stack ? */

  Tcl_DStringInit(&command_line);

#if !defined(MACINTOSH) /* Mac doesn't have pipes */
  if (   (!(strcmp((char *) (argv[2] + strlen(argv[2]) - 2), ".Z")))
      || (!(strcmp((char *) (argv[2] + strlen(argv[2]) - 2), ".z"))))
    {
      /* The popen can succeed if given an non-existant file   */
      /* creating an unusable pipe.  So we check to see if the */
      /* file exists first, on a load action.                  */
      if (loading)
	{
	  if (!(f = fopen(fullname, access_method)))
	    {
	      /* --- error when opening the file --- */
	      Tcl_DStringFree(&command_line);
	      Tcl_DStringFree(&buffer);
	      sprintf (interp->result,
		       "rete-net: Error: unable to open '%s'",
		       argv[2]);
	      return TCL_ERROR;
	    }
	  else
	    {
	      fclose(f);
	    }
	}

      Tcl_DStringAppend(&command_line, pipe_command, strlen(pipe_command));
      Tcl_DStringAppend(&command_line, fullname, strlen(fullname));
      f = (FILE *) popen(Tcl_DStringValue(&command_line), access_method);
      using_compression_filter = TRUE;
    }
  else
#endif /* !MACINTOSH */
    {
      Tcl_DStringAppend(&command_line, fullname, strlen(fullname));
      f = fopen(Tcl_DStringValue(&command_line), access_method);
      using_compression_filter = FALSE;
    }

  if (! f) 
    {
      /* --- error when opening the pipe or file --- */
      Tcl_DStringFree(&command_line);
      Tcl_DStringFree(&buffer);
      sprintf (interp->result,
	       "rete-net: Error: unable to open '%s'",
	       argv[2]);
      return TCL_ERROR;
    }

  result = (*rete_net_op)(f);

#if !defined(MACINTOSH)
  if (using_compression_filter == TRUE)
    {
      pclose(f);
    }
  else
#endif /* !MACINTOSH */
    {
      fclose(f);
    }

  Tcl_DStringFree(&command_line);
  Tcl_DStringFree(&buffer);
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * RunCmd --
 *
 *      This is the command procedure for the "run" command which
 *      runs the Soar agent.  Run will run all soar agents, unless
 *      the [-self] flag is issued.
 * 
 *       This is the command for running Soar agents.  It 
 *       takes two optional arguments, one specifying how many 
 *       things to run, and one specifying what type of things to 
 *       run.  The following types are available:
 * 
 *       p  - run Soar for n phases.  A phase is either an input 
 *            phase, preference phase, working memory phase, output 
 *            phase, or decision phase.
 *       e  - run Soar for n elaboration cycles.  (For purposes of 
 *            this command, decision phase is counted as an 
 *            elaboration cycle.)
 *       d  - run Soar for n decision cycles.
 *       s  - run Soar until the nth time a state is selected.
 *       o  - run Soar until the nth time an operator is selected.
 *       out- run Soar by decision cycles until output is generated.
 *       context-variable - run Soar until the nth time a selection 
 *            is made for that particular context slot, or until the 
 *            context stack pops to above that context.
 * 
 *       Unlike Soar 6 "go" , "run" has no memory of settings from previous
 *       run commands. 
 * 
 *       Examples:
 *        run 5 d   --> run all agents for 5 decision cycles
 *        run d -self --> run the current agent only for 5 decision cycles
 *        run 3     --> run all agents for 3 decision cycles
 *        run e     --> run all agents for 1 elaboration cycle
 *        run 1 s   --> run all agents until the next state is selected 
 *                      (i.e., until the next time an impasse arises)
 *        run <so>  --> run until the next superoperator is selected 
 *                      (or until the supergoal goes away)
 *        run 3 <o> --> run for 3 operator selections at this level 
 *                      (continuing through any subgoals that arise)
 *
 * Syntax:  run [integer | 'forever'] [type] [-self]
 *          type ::= p | e | d | s | o | context-variable
 *          if no integer given, but [type] is specified, n = 1 assumed
 *          if no [type] given, but [int] specified, decisions assumed
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Runs the Soar agents for the indicated duration
 *
 *----------------------------------------------------------------------
 */

int RunCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, char *argv[])
{
  agent * the_agent;
  agent * prev_agent;
  cons  * c;
  int parse_result;
  int code;
  long               go_number;
  enum go_type_enum  go_type;
  Symbol *           go_slot_attr;
  goal_stack_level   go_slot_level;
  bool               self_only = FALSE;
  bool               single_agent = FALSE;

  
  static bool executing = FALSE;

  Soar_SelectGlobalInterpByInterp(interp);

  if (executing == TRUE) 
    {
      /* 
       * Disallow attempts to recursively enter run-related actions.
       * This would lead to seg faults since the agent code is not
       * re-entrant.  Note that this is a general problem, so this
       * does not solve the problem in a general fashion.  This
       * strategy should probably be used on long running commands,
       * however, unless a more general solution is found.
       *
       * This possibility is easy to generate when "run" is tied to
       * a button on the GUI and the user clicks faster than "run"
       * can run.
       */

      return TCL_OK;
    }
  else
    {
      executing = TRUE;
    }

  the_agent = (agent *) clientData;

  c = all_soar_agents;
  if (c->rest == NIL) single_agent = TRUE;

  go_number     = 1;                      /* was the_agent->go_number */
  go_type       = GO_DECISION;            /* was the_agent->go_type   */
  /*  go_slot_attr  = the_agent->go_slot_attr;
      go_slot_level = the_agent->go_slot_level;
   */
  parse_result = parse_run_command(interp, argc, argv,
				  &go_number, &go_type,
				  &go_slot_attr, &go_slot_level,
				  &self_only);
  if (parse_result == TCL_OK) 
    {

      if ((self_only) || (single_agent))
	{
	  the_agent->go_number     = go_number;
	  the_agent->go_type       = go_type;
	  the_agent->go_slot_attr  = go_slot_attr;
	  the_agent->go_slot_level = go_slot_level;
      
	  execute_go_selection(the_agent, go_number, go_type,
			       go_slot_attr, go_slot_level);
	}
      else  /* run all agents */
	{
	  /* set the params for all agents...
	   * this will have to be different for context slots ???
	   */

	  prev_agent = the_agent;
	  for (c = all_soar_agents; c != NIL; c = c->rest) {
	    the_agent = (agent *) c->first;
	    the_agent->go_number     = 1;       /* was go_number */
	    the_agent->go_type       = go_type;
	    the_agent->go_slot_attr  = go_slot_attr;
	    the_agent->go_slot_level = go_slot_level;
	  }
	  the_agent = prev_agent;

	  code = schedule_agents(interp, go_number);
	}
    }
  else
    {
      executing = FALSE;
      return TCL_ERROR;
    }

  interp->result = "\n";
  executing = FALSE;
  return parse_result;
}

/*
 *----------------------------------------------------------------------
 *
 * SpCmd --
 *
 *      This is the command procedure for the "sp" command, which 
 *      defines a new Soar production.
 *
 *      This command adds a new production to the system.  (If 
 *      another production with the same name already exists, it is 
 *      excised.)  The optional flags are as follows: 
 *
 *        :o-support -- specifies that all the RHS actions are to 
 *                      be given o-support when the production fires
 *       :no-support -- specifies that all the RHS actions are only 
 *                      to be given i-support when the production fires
 *          :default -- specifies that this production is a default 
 *                      production (this matters for (excise-task) and 
 *                      (watch task))
 *            :chunk -- specifies that this production is a chunk (this 
 *                      matters for (learn trace))
 *
 * See also:  lhs-grammar, rhs-grammar
 *
 * Syntax: sp {rule-body}
 *
 *         rule-body := name 
 *                      ["documentation-string"] 
 *                      [ flag ]*
 *                      LHS
 *                      -->
 *                      RHS
 *
 *         flag  ::=  :o-support
 *         flag  ::=  :i-support
 *         flag  ::=  :default
 *         flag  ::=  :chunk
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Runs the agent for the indicated number of decision cycles.
 *
 *----------------------------------------------------------------------
 */

int SpCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, char *argv[])
{
  /*  agent* pAgent = (agent*) clientData; /* do we need this? */
  production *p;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = "Too few arguments.\nUsage: sp {rule}.";
      return TCL_ERROR;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments.\nUsage: sp {rule}.";
      return TCL_ERROR;
    }

  /* Use a callback instead? */
  /* ((agent *)clientData)->alternate_input_string = argv[1];
   * ((agent *)clientData)->alternate_input_suffix = ") ";
   */  /* Soar-Bugs #54 TMH */
  soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE); 
  set_lexer_allow_ids (FALSE);
  get_lexeme();
  p = parse_production();
  set_lexer_allow_ids (TRUE);
  soar_alternate_input((agent *)clientData, NIL, NIL, FALSE);
  if (p) 
    {
      /* kjh CUSP(B11) begin, store filename for production */
#ifdef _CodeWarrior_
      char cmd[] = " \
set fname [file tail    [info script]]; \
set dname [file dirname [info script]]; \
if {$fname == \"\"} {return \"\"}; \
if {$dname == \":\"} { \
  return \"[pwd]:$fname\"; \
} elseif {[string index $dname 0 ] == \":\"} { \
  set hdir [pwd]; \
  cd $dname; \
  set result \"[pwd]:$fname\"; \
  cd $hdir; \
  return $result; \
} else { \
  return \"$dname:$fname\"; \
}";
#else
      char cmd[] = " \
set fname [file tail    [info script]]; \
set dname [file dirname [info script]]; \
if {$fname == \"\"} {return \"\"}; \
if {$dname == \".\"} { \
  return \"[pwd]/$fname\"; \
} elseif {[string index $dname 0 ] != \"/\"} { \
  set hdir [pwd]; \
  cd $dname; \
  set result \"[pwd]/$fname\"; \
  cd $hdir; \
  return $result; \
} else { \
  return \"$dname/$fname\"; \
}";

#endif
      /* if (Tcl_Eval(interp, "info script") == TCL_OK) { */
      if (Tcl_Eval(interp, cmd) == TCL_OK) {
        p->filename = make_memory_block_for_string (interp->result);
      }
      /* kjh CUSP(B11) end */

      if (current_agent(sysparams)[TRACE_LOADING_SYSPARAM]) print("*");
      interp->result = "\n";	  

      /* kjh CUSP(B14) begin */
      if (p->type == CHUNK_PRODUCTION_TYPE) {
        /* Extract chunk_count from chunk name.
         * It will always be the number before the first '*' or '\0' 
         * and after a '-'
         * (e.g. chunk-14*d8*tie*2 or chunk-123).
         * This hack eliminates long delays that would arise when generating
         *  new chunk names after having loaded in old chunks.  By extracting
         *  chunk_count here, the time spent looping and checking for pre-
         *  existing names is minimized.
         */
        char *chunk_name, *c;
        unsigned long this_chunk_count;
        
        chunk_name = p->name->sc.name;
        c = chunk_name;
        while (*c && (*c != '*'))
          c++;
        do 
          c--;
        while (*c && (*c != '-'));
        c++;
        if (sscanf(c,"%lu",&this_chunk_count) != 1)
          print("Warning: failed to extract chunk_num from chunk \"%s\"\n",chunk_name);
        else if (this_chunk_count > current_agent(chunk_count)) {
          current_agent(chunk_count) = this_chunk_count;
          print("updated chunk_num=%lu\n",current_agent(chunk_count));
        }
      }
      /* kjh CUSP(B14) end */  
 
      return TCL_OK;
    }
  else
    {
      /* DJP : Modified to make all sp errors appear to be non-fatal */
      /*       This is necessary because currently warnings are causing */
      /*       Soar to stop loading files, which is clearly wrong.  */
      /*       E.g. ignoring production P1 because it's a duplicate of P2 */

      /* return TCL_ERROR; */

      /* Replaced TCL_ERROR with TCL_OK and copied interp->result from above */
      interp->result = "\n";
      return TCL_OK;

      /* THIS MAY NEED A BETTER SOLUTION ??? -- KJC */
    }
}

/*
 *----------------------------------------------------------------------
 *
 * StatsCmd --
 *
 *      This is the command procedure for the "stats" command, which 
 *      prints out internal statistical information.
 *
 * Syntax: stats [-system <stype> | -memory <mtype> | -rete <rtype>] 
 *         <stype> ::= -default-production-count
 *                     -user-production-count
 *                     -chunk-count
 *                     -justification-count
 *                     -all-productions-count
 *                     -dc-count
 *                     -ec-count
 *                     -ecs/dc
 *                     -firings-count
 *                     -firings/ec
 *                     -wme-change-count
 *                     -wme-addition-count
 *                     -wme-removal-count
 *                     -wme-count
 *                     -wme-avg-count
 *                     -wme-max-count
 *                     -total-time             |T
 *                     -ms/dc                  |T
 *                     -ms/ec                  |T
 *                     -ms/firing              |T
 *                     -ms/wme-change          |T
 *                     -match-time             |D
 *                     -ownership-time         |D
 *                     -chunking-time          |D
 *
 *         The items marked |T are available when Soar has been
 *         compiled with the NO_TIMING_STUFF flag NOT SET and 
 *         the items marked |D are available when Soar has been
 *         compiled with the DETAILED_TIMING_STATS flag SET.
 * 
 *         <mtype> ::= -total
 *                     -overhead
 *                     -strings
 *                     -hash-table
 *                     -pool [-total | pool-name [<aspect>]]
 *                     -misc
 *        <aspect> ::= -used                   |M
 *                     -free                   |M
 *                     -item-size
 *                     -total-bytes
 *
 *          The items marked |M are available only when Soar has
 *          been compiled with the MEMORY_POOL_STATS option.
 *
 *          <rtype> ::= -total-nodes
 *                      -dummy-top-nodes
 *                      -pos-nodes
 *                      -unhashed-pos-nodes
 *                      -neg-nodes
 *                      -unhashed-neg-nodes
 *                      -cn-nodes
 *                      -cn-partner-nodes
 *                      -production-nodes
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the selected statistics
 *
 *----------------------------------------------------------------------
 */

int StatsCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (   (argc == 1) 
      || string_match_up_to("-system", argv[1], 2))
    {
      return parse_system_stats(interp, argc, argv);
    }
  else if (string_match_up_to("-memory", argv[1], 2))
    {
      return parse_memory_stats(interp, argc, argv);
    }
  else if (string_match_up_to("-rete", argv[1], 2))
    {
      return parse_rete_stats(interp, argc, argv);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to stats: %s",
	      argv[1]);
      return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * StopSoarCmd --
 *
 *      This is the command procedure for the "stop-soar" command, 
 *      halts the Soar agents.
 * 
 * Syntax:  stop-soar [-s[elf] [reason-string]]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      All agents are halted.
 *
 *----------------------------------------------------------------------
 */

int StopSoarCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc > 3) {
    interp->result = "Too many arguments, should be:\n\t stop-soar [-s[elf] {reason_string}]"; 
    return TCL_ERROR;
  } else if (argc == 1)  {
    control_c_handler(0);
  } else if (string_match_up_to("-self",argv[1],1)) {
    current_agent(stop_soar) = TRUE;
    if (argc == 3) {
      strcpy(current_agent(reason_for_stopping),argv[2]);
    } else current_agent(reason_for_stopping) = "stop-soar -self issued.\n";
    
  } else {
      sprintf(interp->result,
	      "Unrecognized argument to stop-soar: %s", argv[1]);
      return TCL_ERROR;
  }
  return TCL_OK;
}

/* RCHONG: begin 10.11 */
/*
 *----------------------------------------------------------------------
 *
 * VerboseCmd --
 *
 *      This is the command procedure for the "verbose" command, which 
 *
 *      With no arguments, this command prints out the current verbose state.
 *      Any of the following arguments may be given:
 *
 *        on         - turns verbose on
 *        off        - turns verbose off
 *
 * Syntax:  verbose arg*
 *            arg  ::=  -on | -off 
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      none.  it just sets soar_verbose_flag.
 *
 *----------------------------------------------------------------------
 */
int VerboseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  if (argc == 1)
    {
      print("VERBOSE is %s\n\n", (current_agent(soar_verbose_flag) == TRUE) ? "ON":"OFF");
      return TCL_OK;
    }

  {int i;

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {
            current_agent(soar_verbose_flag) = TRUE;
	    print("VERBOSE is %s\n\n", (current_agent(soar_verbose_flag) == TRUE) ? "ON":"OFF");
	 }

       else if (string_match_up_to("-off", argv[i], 3))
	 {
            current_agent(soar_verbose_flag) = FALSE;
	    print("VERBOSE is %s\n\n", (current_agent(soar_verbose_flag) == TRUE) ? "ON":"OFF");
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to the VERBOSE command: %s",
		   argv[i]);
	   return TCL_ERROR;
	 }
     }
 }

  return TCL_OK;
}
/* RCHONG: end 10.11 */

/* REW: begin 10.24.97 */
/*
 *----------------------------------------------------------------------
 *
 * WaitSNCCmd --
 *
 *      This is the command procedure for the "waitsnc" command, which 
 *
 *      With no arguments, this command prints out the current wait/snc state.
 *      Any of the following arguments may be given:
 *
 *        on         - turns snc into wait mode 
 *        off        - turns snc into regular mode (impasse generated)
 *
 * Syntax:  waitsnc arg*
 *            arg  ::=  -on | -off 
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      none
 *
 *----------------------------------------------------------------------
 */

int WaitSNCCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print("waitsnc is %s\n\n", (current_agent(waitsnc) == TRUE) ? "ON":"OFF");
      return TCL_OK;
    }

  {int i;

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {
            current_agent(waitsnc) = TRUE;
	    print("waitsnc is %s\n\n", (current_agent(waitsnc) == TRUE) ? "ON":"OFF");
	 }

       else if (string_match_up_to("-off", argv[i], 3))
	 {
             current_agent(waitsnc) = FALSE;
	     print("waitsnc is %s\n\n", (current_agent(waitsnc) == TRUE) ? "ON":"OFF");
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to the WaitSNC command: %s",
		   argv[i]);
	   return TCL_ERROR;
	 }
     }
 }

  return TCL_OK;
}
/* REW: end   10.24.97 */

/*
 *----------------------------------------------------------------------
 *
 * WarningsCmd --
 *
 *      This is the command procedure for the "warnings" command, 
 *      which enables/disables the printing of warning messages.
 *
 *      warnings -on enables the printing of warning messages.  This 
 *      is the default.  warnings -off turns off most warning messages.
 *      warnings prints an indication of whether warning messages 
 *      are enabled or not.
 *
 * Syntax:  warnings [-on | -off]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the warnings option.
 *
 *----------------------------------------------------------------------
 */

int WarningsCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      sprintf(interp->result, "%s",
	      current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM] 
	      ? "on" : "off");
      return TCL_OK;
    }

  if (argc > 2)
    {
      interp->result = "Too many arguments, should be: warnings [-on | -off]";
      return TCL_ERROR;
    }

  if (string_match_up_to(argv[1], "-on", 3))
    {
      set_sysparam (PRINT_WARNINGS_SYSPARAM, TRUE);
    }
  else if (string_match_up_to(argv[1], "-off", 3))
    {
    set_sysparam (PRINT_WARNINGS_SYSPARAM, FALSE);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized option to warnings command: %s",
	      argv[1]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * WatchCmd --
 *
 *      This is the command procedure for the "watch" command, which 
 *      controls run-time tracing.
 *
 *      This command controls what information gets printed in the 
 *      run-time trace.  With no arguments, it just prints out the 
 *      current watch status.  The numeric arguments are different
 *      from the semantics in Soar 5 and 6; for details, see help watch.
 *      Setting either the -on or -off switch selectively  turns  on
 *      or  off  only  that  setting.  Setting the -inclusive switch
 *      (which can be abbreviated as -inc) or setting no flag at all
 *      has the effect of setting all levels up to and including the
 *      level  specified.  For  example,    watch  productions   -on
 *      selectively    turns    on   the   tracing   of   production
 *      firings/retractions;  watch  productions  -off   selectively
 *      turns  it  off again.  watch productions [-inc] turns on the
 *      tracing of productions and also turns on tracing of all lev-
 *      els  below  productions: decisions and phases, too.  Indivi-
 *      dual watch parameters may be used to  modify  the  inclusive
 *      settings  as  well, selectively turning on or off and levels
 *      outside or inside the inclusive range.
 *
 *      The following keyword arguments may be given to the 'watch' 
 *      command:
 *
 *      0|none   -- turns off all printing about Soar's internals
 *      1|decisions -- controls whether state and operator decisions
 *                are printed as they are made
 *      2|phases -- controls whether phase names are printed as Soar 
 *                executes
 *      3|productions -- controls whether the names of productions are
 *               printed as they fire and retract.  See individual 
 *               production-type args below.
 *      4|wmes --  controls whether changes to working memory are printed
 *      5|preferences -- controls whether the preferences generated by the
 *               traced  productions  are printed when those productions
 *               fire or retract.  When  a  production  fires,  all  the
 *               preferences it generates are printed. When it retracts,
 *               only the ones being removed from preference memory  are
 *               printed (i.e., the i-supported ones).
 *
 *      -nowmes|-timetags|-fullwmes --  controls the level of detail given
 *               about the wmes matched by productions whose firings and 
 *               retractions are being traced.  Level 0|-nowmes means no 
 *               information about the wmes is printed.  Level 1|-timetags 
 *               means the wme timetags are printed.  Level 2|-fullwmes
 *               means the whole wmes are printed.
 *     -all|-default|-chunks|-justifications|-user {-noprint|-print|-fullprint}
 *               allows user to selectively print production *types*
 *               as they fire and retract.  -noprint prints nothing,
 *               -print prints the production name, -fullprint prints the
 *               entire production.  (fullprint not yet implemented)
 *               NOTE:  these args can be abbreviated by the first
 *               char ONLY, otherwise the full arg must be specified.
 *      -prefs|-noprefs -- turns on|off the printing of preferences for
 *               the productions being watched.  These args need to be
 *               made to apply to just the given production type, instead
 *               of to all productions.
 *
 *      aliases -on|-off -- controls the echoing of aliases as they are
 *               defined.
 *      backtracing -on|-off -- controls the printing of backtracing
 *               information when a chunk or justification os created.
 *      learning -noprint|-print|-fullprint -- controls the printing of
 *               chunks/justifications as they are created. -noprint is
 *               don't print anything, -names prints just the names,
 *               -fullprint prints the entire chunk/justification.
 *      loading -on|-off  -- controls the printing of '*' for each
 *               production loaded and a '#' for each production excised.
 *
 *      Currently the following args apply to all productions being
 *      watched, but they need to be changed to apply to only the
 *      specified production type.  This requires the addition of many
 *      new kernel flags, one for each arg for each production type.
 *           -nowmes|-timetags|-fullwmes
 *           -noprint|-print|-fullprint
 *           -prefs|-noprefs
 *
 *
 *  See also:  learn, pwatch, print
 *
 * Syntax: watch arg*
 *           arg  ::=  0 | 1 | 2 | 3 | 4 | 5
 *           arg  ::=  0 | none
 *           arg  ::=  decisions | phases | productions | wmes | preferences
 *                        [-on | -off | -inc[lusive]]
 *           arg  ::=  -nowmes | -timetags | -fullwmes
 *           arg  ::=  aliases  {-on|-off}
 *           arg  ::=  backtracing {-on|-off}
 *           arg  ::=  learning {-noprint|-print|-fullprint}
 *           arg  ::=  loading  {-on|-off}
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets booleans for printing information during Soar execution cycles.
 *
 *----------------------------------------------------------------------
 */

int WatchCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print_current_watch_settings(interp);
    }
  
  {
    int i;

    for (i = 1; i < argc; i++)
      {
	if (string_match("none", argv[i]) || string_match("0",argv[i]))
	  {
	    if (set_watch_level_inc(interp,0)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("1", argv[i]))
	  {
	    if (set_watch_level_inc(interp,
				    TRACE_CONTEXT_DECISIONS_SYSPARAM)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("decisions", argv[i]))
	  {
	    /* check if -on|-off|-inc follows */
	    if ( (i+1) < argc )
	      {
		if ((string_match("-on",argv[i+1])) ||
		    (string_match("-off",argv[i+1])) ||
		    (string_match_up_to("-inclusive",argv[i+1],3)))
		  {
		    if (set_watch_setting (interp,
					   TRACE_CONTEXT_DECISIONS_SYSPARAM,
					   argv[i],
					   argv[++i]) 
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
		else /* something else follows setting, so it's inclusive */
		  {
		    if (set_watch_level_inc(interp,
					    TRACE_CONTEXT_DECISIONS_SYSPARAM)
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
	      }
	    else /* nothing else on cmd line */
	      {
		if (set_watch_level_inc(interp,
					TRACE_CONTEXT_DECISIONS_SYSPARAM)
		    != TCL_OK)
		  {
		    return TCL_ERROR;
		  }
	      }
	  }
	else if (string_match("2", argv[i]))
	  {
	    if (set_watch_level_inc(interp,
				    TRACE_PHASES_SYSPARAM)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("phases", argv[i]))
	  {
	    /* check if -on|-off|-inc follows */
	    if ( (i+1) < argc )
	      {
		if ((string_match("-on",argv[i+1])) ||
		    (string_match("-off",argv[i+1])) ||
		    (string_match_up_to("-inclusive",argv[i+1],3)))
		  {
		    if (set_watch_setting (interp,
					   TRACE_PHASES_SYSPARAM,
					   argv[i],
					   argv[++i]) 
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
		else /* something else follows setting, so it's inclusive */
		  {
		    if (set_watch_level_inc(interp,
					    TRACE_PHASES_SYSPARAM)
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
	      }
	    else /* nothing else on cmd line */
	      {
		if (set_watch_level_inc(interp,
					TRACE_PHASES_SYSPARAM)
		    != TCL_OK)
		  {
		    return TCL_ERROR;
		  }
	      }
	  }
	else if (string_match("3", argv[i]))
	  {
	    if (set_watch_level_inc(interp,
				    TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("productions", argv[i]))
	  {
	    int t;

	    /* productions is a different beast, since there are four
	       separate categories and more flags possible */
	    /* check if -on|-off|-inc follows */
	    if ( (i+1) < argc )
	      {
		if (string_match("-on",argv[i+1]))
		  {
		    for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
		      set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+t,
				    TRUE);
		    i++;
		  }
		else if (string_match("-off",argv[i+1]))
		  {
		    for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
		      set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+t,
				    FALSE);
		    i++;
		  }
		else if (string_match_up_to("-inclusive",argv[i+1],3))
		  {
		    if (set_watch_level_inc(interp,
					  TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM)
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		    i++;
		  }

		/* check for specific production types */

		else if ((string_match("-all", argv[i+1])) ||
			 (string_match("-a",argv[i+1]))) {
		  i++;
		  set_watch_prod_group_setting(interp, 0,argv[i], argv[++i]);
		}
		else if ((string_match("-chunks", argv[i+1])) ||
			 (string_match("-c",argv[i+1])))  {
		  i++;
		  set_watch_prod_group_setting(interp, 1, argv[i], argv[++i]);
		}
		else if ((string_match("-defaults", argv[i+1])) ||
			 (string_match("-d",argv[i+1])))  {
		  i++;
		  set_watch_prod_group_setting(interp, 2, argv[i], argv[++i]);
		}
		else if ((string_match("-justifications", argv[i+1])) ||
			 (string_match("-j",argv[i+1])))  {
		  i++;
		  set_watch_prod_group_setting(interp, 3, argv[i], argv[++i]);
		}
		else if ((string_match("-user", argv[i+1])) ||
			 (string_match("-u",argv[i+1])))  {
		  i++;
		  set_watch_prod_group_setting(interp, 4, argv[i], argv[++i]);
		}

		else /* something else follows setting, so it's inclusive */
		  {
		    if (set_watch_level_inc(interp,
					  TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM)
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
	      }
	    else /* nothing else on cmd line */
	      {
		if (set_watch_level_inc(interp,
					TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM)
		    != TCL_OK)
		  {
		    return TCL_ERROR;
		  }
	      }
	  }
	else if (string_match("4", argv[i]))
	  {
	    if (set_watch_level_inc(interp,
				    TRACE_WM_CHANGES_SYSPARAM)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("wmes", argv[i]))
	  {
/* kjh(CUSP-B2) begin */
          char *wmes_option_syntax_msg = "\
watch wmes syntax:\n\
   wmes [ -on |\n\
          -off |\n\
          -inc[lusive] |\n\
         {-add-filter    type filter} |\n\
         {-remove-filter type filter} |\n\
         {-reset-filter  type} |\n\
         {-list-filter   type} ]\n\
        where\n\
          type   = -adds|-removes|-both\n\
          filter = {id|*} {attribute|*} {value|*}";
         
        if (i+1 >= argc) { /* nothing else on cmd line, so it's inclusive */
		    if (set_watch_level_inc(interp,
					TRACE_WM_CHANGES_SYSPARAM)
		        != TCL_OK)
		      {
		        return TCL_ERROR;
		      }
          } else if ((string_match(argv[i+1],"-on")) || 
                     (string_match(argv[i+1],"-off")) || 
	 	     (string_match_up_to("-inclusive",argv[i+1],3))) {
	         if (set_watch_setting (interp,
				                    TRACE_WM_CHANGES_SYSPARAM,
				                    argv[i],
				                    argv[i+1]) 
		         != TCL_OK)
                return TCL_ERROR;
              else
                i += 1;
	      } else if (i+2 >= argc) {
            sprintf(interp->result, wmes_option_syntax_msg);
            return TCL_ERROR;
          } else if (string_match(argv[i+1],"-add-filter")) {
            bool forAdds, forRemoves;
            if ( (i+5 >= argc)
              || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == TCL_ERROR)) {
              Tcl_AppendResult(interp, wmes_option_syntax_msg, (char *) NULL);
              return TCL_ERROR;
            } else {
              if (wmes_filter_add(interp,argv[i+3],argv[i+4],argv[i+5],forAdds,forRemoves) == TCL_ERROR) {
                Tcl_AppendResult(interp, "Filter not added.", (char *) NULL);
                return TCL_ERROR;
              } else {
                Tcl_AppendResult(interp, "Filter added.", (char *) NULL);
              }
            }
            i += 5;
          } else if (string_match(argv[i+1],"-remove-filter")) {
            bool forAdds, forRemoves;
            if ( (i+5 >= argc)
              || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == TCL_ERROR)) {
              Tcl_AppendResult(interp, wmes_option_syntax_msg, (char *) NULL);
              return TCL_ERROR;
            } else {
              if (wmes_filter_remove(interp,argv[i+3],argv[i+4],argv[i+5],forAdds,forRemoves) == TCL_ERROR) {
                Tcl_AppendResult(interp, "Filter not found.", (char *) NULL);
                return TCL_ERROR;
              } else {
                Tcl_AppendResult(interp, "Filter removed.", (char *) NULL);
              }
            }
            i += 5;
          } else if (string_match(argv[i+1],"-reset-filter")) {
            bool forAdds, forRemoves;
            if ( (i+2 >= argc)
              || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == TCL_ERROR)) {
              Tcl_AppendResult(interp, wmes_option_syntax_msg, (char *) NULL);
              return TCL_ERROR;
            } else {
              if (wmes_filter_reset(interp,forAdds,forRemoves) == TCL_ERROR) {
                Tcl_AppendResult(interp, "No filters were removed.", (char *) NULL);
                return TCL_ERROR;
              }
            }
            i += 2;
          } else if (string_match(argv[i+1],"-list-filter")) {
            bool forAdds, forRemoves;
            if ( (i+2 >= argc)
              || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == TCL_ERROR)
              || (wmes_filter_list(interp,forAdds,forRemoves) == TCL_ERROR)) {
	      Tcl_AppendResult(interp, wmes_option_syntax_msg, (char *) NULL);
              return TCL_ERROR;
            }
            i += 2;
          }
        }
/* kjh(CUSP-B2) end */
	/*  kjc note:  not sure CUSP-B2 solution accounts for other
	 *  non-wme args following "wmes" which should make it -inc 
	 */

	else if (string_match("5", argv[i]))
	  {
	    if (set_watch_level_inc(interp,
				    TRACE_FIRINGS_PREFERENCES_SYSPARAM)
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("preferences", argv[i]))
	  {
	    /* check if -on|-off|-inc follows */
	    if ( (i+1) < argc )
	      {
		if ((string_match("-on",argv[i+1])) ||
		    (string_match("-off",argv[i+1])) ||
		    (string_match_up_to("-inclusive",argv[i+1],3)))
		  {
		    if (set_watch_setting (interp,
					   TRACE_FIRINGS_PREFERENCES_SYSPARAM,
					   argv[i],
					   argv[++i]) 
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
		else /* something else follows setting, so it's inclusive */
		  {
		    if (set_watch_level_inc(interp,
					    TRACE_FIRINGS_PREFERENCES_SYSPARAM)
			!= TCL_OK)
		      {
			return TCL_ERROR;
		      }
		  }
	      }
	    else /* nothing else on cmd line */
	      {
		if (set_watch_level_inc(interp,
					TRACE_FIRINGS_PREFERENCES_SYSPARAM)
		    != TCL_OK)
		  {
		    return TCL_ERROR;
		  }
	      }
	  }
	else if ((string_match("-all", argv[i])) ||
		 (string_match("-a",argv[i])))
	  {
	    set_watch_prod_group_setting(interp, 0, argv[i], argv[++i]);
	  }
	else if ((string_match("-chunks", argv[i])) ||
		 (string_match("-c",argv[i])))
	  {
	    set_watch_prod_group_setting(interp, 1, argv[i], argv[++i]);
	  }
	else if ((string_match("-defaults", argv[i])) ||
		 (string_match("-d",argv[i])))
	  {
	    set_watch_prod_group_setting(interp, 2, argv[i], argv[++i]);
	  }
	else if ((string_match("-justifications", argv[i])) ||
		 (string_match("-j",argv[i])))
	  {
	    set_watch_prod_group_setting(interp, 3, argv[i], argv[++i]);
	  }
	else if ((string_match("-user", argv[i])) ||
		 (string_match("-u",argv[i])))
	  {
	    set_watch_prod_group_setting(interp, 4, argv[i], argv[++i]);
	  }
	else if (string_match_up_to("-nowmes", argv[i],4))
	  {
	    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
			 NONE_WME_TRACE);
	  }
	else if (string_match_up_to("-timetags", argv[i],3))
	  {
	    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
			 TIMETAG_WME_TRACE);
	  }
	else if (string_match_up_to("-fullwmes", argv[i],6))
	  {
	    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
			 FULL_WME_TRACE);
	  }
	else if (string_match_up_to("-prefs", argv[i],4))
	  {
	    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,TRUE);
	  }
	else if (string_match_up_to("-noprefs", argv[i],6))
	  {
	    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,FALSE);
	  }
/* REW: begin 10.22.97 */
	else if (string_match_up_to("gds", argv[i],6))
	  {
	    set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM,TRUE);
	  }
	else if (string_match_up_to("-nogds", argv[i],6))
	  {
	    set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM,FALSE);
	  }
/* REW: end   10.22.97 */
	else if (string_match_up_to("learning", argv[i],2))
	  {
	    /* check if -print|-noprint|-fullprint follows */
	    if ( (i+1) < argc )
	      {
		if (string_match("-print",argv[i+1]))
		  {
		    set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM,TRUE);
		    set_sysparam(TRACE_CHUNKS_SYSPARAM,     FALSE);
		    set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM,TRUE);
		    set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM,     FALSE);
		    i++;
		  }
		else if  (string_match("-noprint",argv[i+1]))
		  {
		    set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM,FALSE);
		    set_sysparam(TRACE_CHUNKS_SYSPARAM,     FALSE);
		    set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM,FALSE);
		    set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM,     FALSE);
		    i++;
		  }
		else if (string_match_up_to("-fullprint",argv[i+1],3))
		  {
		    set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM,TRUE);
		    set_sysparam(TRACE_CHUNKS_SYSPARAM,     TRUE);
		    set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM,TRUE);
		    set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM,     TRUE);
		    i++;
		  }
		else
		  { /* error: no arg for learning */
		sprintf(interp->result,
			"Missing setting for watch learning, should be -noprint|-print|-fullprint");
		return TCL_ERROR;
		  }
	      }
	    else
	      { /* error: no arg for learning */
		sprintf(interp->result,
			"Missing setting for watch learning, should be -noprint|-print|-fullprint");
		return TCL_ERROR;
	      }
	  }
	else if (string_match("backtracing", argv[i]))
	  {
	    if (set_watch_setting (interp,
				   TRACE_BACKTRACING_SYSPARAM,
				   argv[i],
				   argv[++i]) 
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("loading", argv[i]))
	  {
	    if (set_watch_setting (interp,
				   TRACE_LOADING_SYSPARAM,
				   argv[i],
				   argv[++i]) 
		!= TCL_OK)
	      {
		return TCL_ERROR;
	      }
	  }
	else if (string_match("aliases", argv[i]))
	  {
	    if (argv[i+1] == NULL)
	      {
		sprintf(interp->result,
			"Missing setting for watch alias, should be -on|-off");
		return TCL_ERROR;
	      }
	    else if (string_match("-on",argv[i+1]))
	      {
		Tcl_SetVar(interp,"print_alias_switch","on",TCL_GLOBAL_ONLY);
		i++;
	      }
	    else if (string_match("-off",argv[i+1]))
	      {
		Tcl_SetVar(interp,"print_alias_switch","off",TCL_GLOBAL_ONLY);
		i++;
	      }
	    else
	      {
		sprintf(interp->result,
			"Unrecognized argument to watch alias : %s",
			argv[i+1]);
		return TCL_ERROR;
	      }
	  }
	else
	  {
	    sprintf(interp->result,
		    "Unrecognized argument to watch command: %s",
		    argv[i]);
	    return TCL_ERROR;
	  }
      }
  }

  return TCL_OK;
}

void Soar_InstallCommands (agent * the_agent)
{
  install_tcl_soar_cmd(the_agent, "add-wme",             AddWmeCmd);
  #ifdef ATTENTION_LAPSE  /* RMJ */
  install_tcl_soar_cmd(the_agent, "attention-lapse",     AttentionLapseCmd);
  install_tcl_soar_cmd(the_agent, "start-attention-lapse", StartAttentionLapseCmd);
  install_tcl_soar_cmd(the_agent, "wake-from-attention-lapse", WakeFromAttentionLapseCmd);
  #endif  /* ATTENTION_LAPSE */
  install_tcl_soar_cmd(the_agent, "attribute-preferences-mode", AttributePreferencesModeCmd);
  install_tcl_soar_cmd(the_agent, "capture-input",             CaptureInputCmd);
  install_tcl_soar_cmd(the_agent, "chunk-name-format",   ChunkNameFormatCmd); /* kjh(CUSP-B14) */
  install_tcl_soar_cmd(the_agent, "default-wme-depth",   DefWmeDepthCmd);
  install_tcl_soar_cmd(the_agent, "echo",                EchoCmd);
  install_tcl_soar_cmd(the_agent, "excise",              ExciseCmd);
  install_tcl_soar_cmd(the_agent, "explain-backtraces",  ExplainBacktracesCmd);
  install_tcl_soar_cmd(the_agent, "firing-counts",       FiringCountsCmd);
  install_tcl_soar_cmd(the_agent, "format-watch",        FormatWatchCmd); 
  install_tcl_soar_cmd(the_agent, "indifferent-selection", IndifferentSelectionCmd);
  install_tcl_soar_cmd(the_agent, "init-soar",           InitSoarCmd);
  install_tcl_soar_cmd(the_agent, "input-period",        InputPeriodCmd);
  install_tcl_soar_cmd(the_agent, "internal-symbols",    InternalSymbolsCmd);  
  install_tcl_soar_cmd(the_agent, "io",                  IOCmd);
  install_tcl_soar_cmd(the_agent, "learn",               LearnCmd);
  install_tcl_soar_cmd(the_agent, "log",                 LogCmd);
  install_tcl_soar_cmd(the_agent, "matches",             MatchesCmd);
  install_tcl_soar_cmd(the_agent, "max-chunks",          MaxChunksCmd);
  install_tcl_soar_cmd(the_agent, "max-elaborations",    MaxElaborationsCmd);
  install_tcl_soar_cmd(the_agent, "max-nil-output-cycles", MaxNilOutputCmd);
  install_tcl_soar_cmd(the_agent, "memories",            MemoriesCmd);
  install_tcl_soar_cmd(the_agent, "monitor",             MonitorCmd);
  install_tcl_soar_cmd(the_agent, "multi-attributes",     MultiAttrCmd);
  install_tcl_soar_cmd(the_agent, "o-support-mode",      OSupportModeCmd);
  install_tcl_soar_cmd(the_agent, "output-strings-destination", OutputStringsDestCmd);
  install_tcl_soar_cmd(the_agent, "production-find",     ProductionFindCmd);
  install_tcl_soar_cmd(the_agent, "preferences",         PreferencesCmd);
  install_tcl_soar_cmd(the_agent, "print",               PrintCmd);
  install_tcl_soar_cmd(the_agent, "pwatch",              PwatchCmd);
  install_tcl_soar_cmd(the_agent, "quit",                QuitCmd);  
/*  install_tcl_soar_cmd(the_agent, "record",              RecordCmd);  /* kjh(CUSP-B10) */
/*  install_tcl_soar_cmd(the_agent, "replay",              ReplayCmd);  /* kjh(CUSP-B10) */
  install_tcl_soar_cmd(the_agent, "replay-input",        ReplayInputCmd);
  install_tcl_soar_cmd(the_agent, "remove-wme",          RemoveWmeCmd);
  install_tcl_soar_cmd(the_agent, "rete-net",            ReteNetCmd);
  install_tcl_soar_cmd(the_agent, "run",                 RunCmd);
  install_tcl_soar_cmd(the_agent, "sp",                  SpCmd);
  install_tcl_soar_cmd(the_agent, "stats",               StatsCmd);
  install_tcl_soar_cmd(the_agent, "stop-soar",           StopSoarCmd);
  install_tcl_soar_cmd(the_agent, "warnings",            WarningsCmd);
  install_tcl_soar_cmd(the_agent, "watch",               WatchCmd);
/* REW: begin 09.15.96 */
  install_tcl_soar_cmd(the_agent, "gds_print",           GDS_PrintCmd);
  /* REW: 7.1/waterfall:soarAppInit.c  merge */
  install_tcl_soar_cmd(the_agent, "verbose",            VerboseCmd);
  install_tcl_soar_cmd(the_agent, "soar8",              Operand2Cmd);
  install_tcl_soar_cmd(the_agent, "waitsnc",            WaitSNCCmd);
/* REW: end   09.15.96 */
}

