/*************************************************************************
 *
 *  file:  interface.c
 *
 * =======================================================================
 *  This portion of the code is normally replaced by the Tcl interface
 *  and interface support routines, but some maintenance continues, in
 *  the event someone needs to build a version without Tcl.  However some
 *  user interface commands are likely to break or give unusual output.
 *  A minimal test that this routine still worked was done by Scott
 *  Wallace with Soar 8.1, Jan 1999.
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

/* =================================================================
                             interface.c                             

   See more comments in soarkernel.h for an overview.
   ================================================================= */

#include "soarkernel.h"

#ifdef __hpux
#undef _STRUCT_TIMEVAL
#endif /* __hpux */
#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>       /* used for "time" command */
#include <sys/resource.h>
#endif /* !__SC__ && !THINK_C && !WIN32 */
#ifdef __hpux
#undef _INCLUDE_HPUX_SOURCE
#define _STRUCT_TIMEVAL
#endif /* __hpux */

#include "scheduler.h"

#ifdef __hpux
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif /* __hpux */
#include <ctype.h>  /* AGR 562 */

#if defined(WIN32)
#include <direct.h>
#define getwd(arg) _getcwd(arg, (size_t) 9999)
#endif /* WIN32 */

#if defined(MACINTOSH)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif /* MACINTOSH */


/* This is a temporary hack during the Tcl->Soar integration.  When
 * we get to a point where we can remove the old command interpreter
 * then this dependency will not be needed.  Basically, the 
 * command_utils.c file redefines a number of support functions in
 * this file so they can be used by TclSoar.  Since these functions
 * are not part of the Soar kernel, they should be removed from this
 * directory (or added to other Soar kernel files, as appropriate).
 */

/* ===================================================================

                          Command Management

  Each user interface command has a corresponding function
  (user_interface_routine) to handle it.  These commands/functions
  should be installed at system startup time via add_command().  The
  command name string passed to add_command() must be permanently
  available (e.g., a constant in global data memory).

  When a user interface routine is called, the current lexeme is the
  command name.  The function should call the lexer to read its arguments,
  etc.  If successful, the function should return TRUE and should exit
  with the current lexeme being the closing right parenthesis (otherwise
  the dispatching function will print an error message about extra
  arguments being given).  If unsuccessful, the function should
  return FALSE.
  
  The command dispatcher calls set_lexer_allow_ids(TRUE) before dispatching
  any command.
=================================================================== */

typedef struct interface_routine_struct {
  struct interface_routine_struct *next;
  char *command_name;
  user_interface_routine f;
} interface_routine;

interface_routine *interface_routines = NIL;

void add_command (char *command_name, user_interface_routine f) {
  interface_routine *ir;

  /* --- make sure we don't already have a routine with the same name --- */
  for (ir=interface_routines; ir!=NIL; ir=ir->next)
    if (! strcmp(ir->command_name,command_name)) break;
  if (ir) {
    print ("Warning: add_command notes that %s shadows existing command.\n",
           command_name);
  }
  /* --- create new interface routine structure --- */
  ir = allocate_memory (sizeof(interface_routine), MISCELLANEOUS_MEM_USAGE);
  ir->next = interface_routines;
  interface_routines = ir;
  ir->command_name = command_name;
  ir->f = f;
}

void delete_command (char * command_name) {
  interface_routine *ir, *prev_ir;

  prev_ir = NIL; /* unnecessary, but gcc -Wall warns without it */
  for (ir=interface_routines; ir!=NIL; ir=ir->next) {
    if (! strcmp(ir->command_name, command_name)) 
	break;
    prev_ir = ir;
  }
  if (ir) {
    if (ir == interface_routines)           /* First in list  */
      interface_routines = ir->next;
    else                                    /* Down the list  */
      prev_ir->next = ir->next;
    free_memory(ir, MISCELLANEOUS_MEM_USAGE);
  } else {
      print ("Error:  delete_command called for non-existent command %s\n", 
	     command_name);
  }
}
  

/* ===================================================================
   
                         Dispatching Commands

  Dispatch_command() dispatches the appropriate user interface routine
  for the current command (i.e., the command named by the current lexeme).
  It calls set_lexer_allow_ids(TRUE) before dispatching the command,
  so if the command doesn't allow id's, it should call 
  set_lexer_allow_ids(FALSE) immediately.  Dispatch_command() returns 
  TRUE if the command was successful, FALSE if any error occurred.
   
  Repeatedly_read_and_dispatch_commands() keeps calling dispatch_command()
  until end-of-file is reached on the current input file.
  
  Load_file() sets up the lexer to read from a given open file, executes
  all the commands in that file, and then restore the lexer to reading
  the previous file.
=================================================================== */

/* AGR 628  The __SC__ is a compiler flag set by the (I think) Symantec C
   compiler.  Code delimited by __SC__ ifdefs is code specifically for
   MacSoar.  94.9.7  */

#ifdef __SC__
/* the SIOW interface may grab the prompt as part of the input */
/* string so it must be recognized and removed                 */
void remove_prompt_if_necessary(void)
{
	if (!strcmp("soar>",current_agent(lexeme).string))
		get_lexeme();
}
#endif

bool dispatch_command (void) {
  alias_struct *cur_alias;
  interface_routine *ir;
  int parentheses_level;
  bool result;

#ifdef UNIX
  /* begin variables for shell escape */
  unsigned char *dollar_ptr;
  char *expanded_string;
  dir_stack_struct *top_dir;
  /* end variables for shell escape */
#endif

  parentheses_level = current_lexer_parentheses_level();

/* AGR 568 begin */
  if (!current_agent(lex_alias)) {
    cur_alias = current_agent(alias_list);
    while (cur_alias) {
      if (strcmp(current_agent(lexeme).string, cur_alias->alias)) {
	cur_alias = cur_alias->next;
      } else {
	if (cur_alias->expansion) {
	  current_agent(lexeme) = cur_alias->expansion->lexeme;
	  current_agent(lex_alias) = cur_alias->expansion->next;
	} else {
	  print("Alias %s is not defined\n", cur_alias->alias);
	}
      }
    }
  }
/* AGR 568 end */

/* AGR 562 begin */
#ifdef UNIX
  if (current_agent(lexeme).type == DOLLAR_STRING_LEXEME) {

    /* mvp 6-20-94 */
    dollar_ptr = (unsigned char *)current_agent(lexeme).string +1;

    /* skip whitespace between '$' and actual command */
    while (isspace(*dollar_ptr)) dollar_ptr++;

    /* handle "cd" command */
    if (!strncmp(dollar_ptr, "cd", 2) &&
	((isspace(*(dollar_ptr+2))) || (*(dollar_ptr+2) == 0))) {
      dollar_ptr += 2;
      while (isspace(*dollar_ptr)) dollar_ptr++;
      expanded_string = tilde_expand((char *) dollar_ptr);
      print ("Changing to directory: %s\n", expanded_string);

      result = chdir(expanded_string);
      if (result)
	print("  FAILED.\n");
      else {
	if(getwd(expanded_string))
	  strcpy(current_agent(top_dir_stack)->directory, expanded_string);
      }
      free((void *) expanded_string);
      if (current_lexer_parentheses_level() != parentheses_level-1)
	skip_ahead_to_balanced_parentheses (parentheses_level-1);
      return TRUE;
    }

    /* handle "chdir" command */
    if (!strncmp(dollar_ptr, "chdir", 5) &&
	((isspace(*(dollar_ptr+5))) || (*(dollar_ptr+5) == 0))) {
      dollar_ptr += 5;
      while (isspace(*dollar_ptr)) dollar_ptr++;
      expanded_string = tilde_expand((char *) dollar_ptr);
      print ("Changing to directory: %s\n", expanded_string);

      result = chdir(expanded_string);
      if (result)
	print("  FAILED.\n");
      else {
	if(getwd(expanded_string))
	  strcpy(current_agent(top_dir_stack)->directory, expanded_string);
      }
      free((void *) expanded_string);
      if (current_lexer_parentheses_level() != parentheses_level-1)
	skip_ahead_to_balanced_parentheses (parentheses_level-1);
      return TRUE;
    }

    /* handle "pushd" command */
    if (!strncmp(dollar_ptr, "pushd", 5) &&
	((isspace(*(dollar_ptr+5))) || (*(dollar_ptr+5) == 0))) {
      dollar_ptr += 5;
      while (isspace(*dollar_ptr)) dollar_ptr++;
      expanded_string = tilde_expand((char *) dollar_ptr);
      expanded_string = (char *) realloc(expanded_string, MAXPATHLEN);
      print ("Pushing directory onto the stack: %s\n", expanded_string);

      result = chdir(expanded_string);
      if (result)
	print("  FAILED.\n");
      else {
	top_dir = (dir_stack_struct *) malloc(sizeof(dir_stack_struct));
	top_dir->directory = expanded_string;
	top_dir->next = current_agent(top_dir_stack);
	current_agent(top_dir_stack) = top_dir;
	if(getwd(expanded_string))
	  strcpy(current_agent(top_dir_stack)->directory, expanded_string);
      }
      if (current_lexer_parentheses_level() != parentheses_level-1)
	skip_ahead_to_balanced_parentheses (parentheses_level-1);
      return TRUE;
    }

    /* handle "popd" command */
    if (!strncmp(dollar_ptr, "popd", 4) &&
	((isspace(*(dollar_ptr+4))) || (*(dollar_ptr+4) == 0))) {
      dollar_ptr += 4;
      while (isspace(*dollar_ptr)) dollar_ptr++;

      top_dir = current_agent(top_dir_stack)->next;
      if (top_dir) {
	print ("Popping off directory: %s\n", current_agent(top_dir_stack)->directory);
	free((void *) current_agent(top_dir_stack)->directory);
	free((void *) current_agent(top_dir_stack));
	current_agent(top_dir_stack) = top_dir;

	chdir(current_agent(top_dir_stack)->directory);
	if (getwd (current_agent(top_dir_stack)->directory)) {
	  print ("Current directory now is: %s\n", current_agent(top_dir_stack)->directory);
	} else {
	  print ("Error: unable to determine current working directory.\n");
	}
      } else
	print ("Can't pop off the only entry on the directory stack.\n");
      if (current_lexer_parentheses_level() != parentheses_level-1)
	skip_ahead_to_balanced_parentheses (parentheses_level-1);
      return TRUE;
    }

    /* handle "dirs" command */
    if (!strncmp(dollar_ptr, "dirs", 4) &&
	((isspace(*(dollar_ptr+4))) || (*(dollar_ptr+4) == 0))) {
      dollar_ptr += 4;
      while (isspace(*dollar_ptr)) dollar_ptr++;

      top_dir = current_agent(top_dir_stack);
      while (top_dir) {
	print ("%s\n", top_dir->directory);
	top_dir = top_dir->next;
      }
      if (current_lexer_parentheses_level() != parentheses_level-1)
	skip_ahead_to_balanced_parentheses (parentheses_level-1);
      return TRUE;
    }

    /* If we get here, then our command is not one that needs to be
       trapped, so we can simply pass it on to system(). AGR 3-Jun-94 */

    /* mvp 6-20-94 */
    system((char *) dollar_ptr);
    if (current_lexer_parentheses_level() != parentheses_level-1)
      skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return TRUE;
  }
#endif
/* AGR 562 end */

  for (ir=interface_routines; ir!=NIL; ir=ir->next)
    if (! strcmp(ir->command_name,current_agent(lexeme).string)) break;
  if (! ir) {
    /* --- no such command --- */
    print ("Error:  unknown command %s\n", current_agent(lexeme).string);
    print_location_of_most_recent_lexeme();
    if (current_lexer_parentheses_level() != parentheses_level-1)
      skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return FALSE;
  }

  set_lexer_allow_ids (TRUE);
  result = (*(ir->f))();
  if (current_lexer_parentheses_level() != parentheses_level-1) {
    if (result) {
      print ("Ignoring extra argument(s)\n");
      print_location_of_most_recent_lexeme();
      result = FALSE;
    }
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
  }
  return result;
}

#ifdef _WINDOWS
void read_and_dispatch_command(void) {
    if (current_lexer_parentheses_level()) 
      skip_ahead_to_balanced_parentheses(0);
    /* --- consume rparen from previous command, get start of next cmd. --- */

    get_lexeme();
    if (current_agent(lexeme).type==EOF_LEXEME) return;

    /* --- if not lparen, fake one at end of the current line --- */
    if (current_agent(lexeme).type==L_PAREN_LEXEME) {
      get_lexeme(); /* consume lparen */
    } else 
      fake_rparen_at_next_end_of_line ();

    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) 
      dispatch_command();
    else {
      print ("Error:  unknown command %s\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
      skip_ahead_to_balanced_parentheses(0);
    }
    if (current_agent(lexeme).type==EOF_LEXEME) return;
}
#endif

void repeatedly_read_and_dispatch_commands (void) {

#ifdef USE_X_DISPLAY
 if (current_agent(print_prompt_flag))
 {
   print ("\n%s> ", current_agent(name));
   while (TRUE)
      handle_soar_x_events();
 }
#endif
 while (TRUE) {
    if (current_lexer_parentheses_level()) {
      char msg[128];
      strcpy(msg, "Internal error:  misbalanced parentheses in main loop.\n");
      abort_with_fatal_error(msg);
    }
    /* --- consume rparen from previous command, get start of next cmd. --- */
 
    get_lexeme();
    if (current_agent(lexeme).type==EOF_LEXEME) return;

    /* --- if not lparen, fake one at end of the current line --- */
    if (current_agent(lexeme).type==L_PAREN_LEXEME) {
      get_lexeme(); /* consume lparen */
    } else 
      fake_rparen_at_next_end_of_line ();
    
    if ((current_agent(lexeme).type == SYM_CONSTANT_LEXEME) ||
	(current_agent(lexeme).type == DOLLAR_STRING_LEXEME)) /* AGR 562 */
        {
#ifdef __SC__
	  remove_prompt_if_necessary();
#endif
	  dispatch_command();
    } else {
      print ("Error:  unknown command %s\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
      skip_ahead_to_balanced_parentheses(0);
    }
    if (current_agent(lexeme).type==EOF_LEXEME) return;
  } /* end of while TRUE */
}

void load_file (char *file_name, FILE *already_open_file) {
bool old_print_prompt_flag;

  old_print_prompt_flag = current_agent(print_prompt_flag);
  current_agent(print_prompt_flag) = FALSE;

  start_lex_from_file (file_name, already_open_file);
  repeatedly_read_and_dispatch_commands ();
  stop_lex_from_file ();

  current_agent(print_prompt_flag) = old_print_prompt_flag;
}


/* ===================================================================
   
               Help Information Management Routines

   Add_help() should be called at system startup time to specify to the
   "help" command what help info is available.  It takes a topic name and
   an array of lines of text for the helpscreen.  All these strings should
   be permanently available (e.g., constants in global data memory).

   Help_interface_routine() is called when the user types "help".  It
   look in a table for the appropriate help screen, and prints it.
=================================================================== */

typedef struct help_screen_info_struct {
  struct help_screen_info_struct *next;
  char *topic;
  char **lines_of_text;
} help_screen_info;

help_screen_info *available_helpscreens = NIL;

help_screen_info *lookup_helpscreen (char *topic) {
  help_screen_info *hsi;

  for (hsi=available_helpscreens; hsi!=NIL; hsi=hsi->next)
    if (! strcmp (topic, hsi->topic)) return hsi;
  return NIL;
}

void add_help (char *topic, char **lines_of_text) {
  help_screen_info *hsi, *prev;

  if (lookup_helpscreen (topic)) {
    print ("Internal error: attempt to add_help to existing topic %s\n",
           topic);
    return;
  }
  hsi = allocate_memory (sizeof(help_screen_info), MISCELLANEOUS_MEM_USAGE);
  hsi->topic = topic;
  hsi->lines_of_text = lines_of_text;
  /* --- insert into list available_helpscreens in alphabetical order --- */
  if ((! available_helpscreens) ||
      (strcmp (topic, available_helpscreens->topic) < 0)) {
    hsi->next = available_helpscreens;
    available_helpscreens = hsi;
  } else {
    for (prev = available_helpscreens; prev->next!=NIL; prev=prev->next)
      if (strcmp (topic, prev->next->topic) < 0) break;
    hsi->next = prev->next;
    prev->next = hsi;
  }
}

char *help_on_list_help_topics[] = {
"Command: list-help-topics",
"",
"Syntax: (list-help-topics)",
"",
"This prints out the names of all topics on which help information is",
"available.",
0 };

bool list_help_topics_interface_routine (void) {
  help_screen_info *hsi;

  print ("\nHelp is available on the following topics:\n\n");
  for (hsi=available_helpscreens; hsi!=NIL; hsi=hsi->next) {
    if (get_printer_output_column()+strlen(hsi->topic)+2 >= COLUMNS_PER_LINE)
      print_string ("\n");
    print_string (hsi->topic);
    if (hsi->next) print_string (", ");
  }
  print_string ("\n");
  get_lexeme(); /* consume "list-help-topics" */
  return TRUE;
}

char *help_on_help[] = {
"For help on a specific command, type either \"help\" or \"?\", followed by",
"the command name.",
"For a list of all available help topics, type \"list-help-topics\".",
"To print all the help screens to a file, type (print-all-help \"filename\").",
0 };

char *no_help_available[] = {
"No help on that subject is available.",
"For a list of all available help topics, type \"list-help-topics\".",
0 };

bool help_interface_routine (void) {
  help_screen_info *hsi;
  char **line;
  
  get_lexeme(); /* consume "help", look for topic name */
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) { /* get topic name */
    if (!strcmp(current_agent(lexeme).string, "all")) {
      return list_help_topics_interface_routine();
    } else {
      hsi = lookup_helpscreen (current_agent(lexeme).string);
      get_lexeme(); /* consume topic */
    }
  } else {
    /* if user didn't give a topic, give help on help */
    hsi = lookup_helpscreen ("help");
  }

  if (hsi) line = hsi->lines_of_text; else line = no_help_available;
  print ("\n");
  while (*line) {
    print ("%s\n", *line);
    line++;
  }
  
  return TRUE;
}

/* mvp 5-17-94 */
char *help_on_print_all_help[] = {
"Command: print-all-help",
"",
"Syntax: (print-all-help \"filename\")",
"",
"This prints all available help screens to the indicated file.  This is a",
"quick way to produce a reference manual (well, sort of).",
0 };

/* mvp 5-17-94 */
bool print_all_help_interface_routine (void) {
  FILE *output_file;
  help_screen_info *hsi;
  char **line;
  char file_name [MAX_LEXEME_LENGTH+1];

  get_lexeme();  /* consume "print-all-help" */

  /* --- look for filename --- */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for filename\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif

  if (current_agent(lexeme).type == R_PAREN_LEXEME) {
    print ("Expected a file name for print-all-help\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }

  strcpy (file_name, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (file_name, current_agent(lexeme).string);
    get_lexeme ();
  }

  /* --- open the file --- */
  output_file = fopen (file_name, "w");
  if (!output_file) {
    /* --- error when opening the file --- */
    print ("Error: unable to open file %s\n", file_name);
    return FALSE;
  }
  print ("Writing help screens to file %s\n", file_name);

  /* --- print each helpscreen --- */
  for (hsi=available_helpscreens; hsi!=NIL; hsi=hsi->next) {
    fprintf (output_file, "TOPIC:  %s\n\n", hsi->topic);
    line = hsi->lines_of_text;
    while (*line) {
      fprintf (output_file, "%s\n", *line);
      line++;
    }
    fprintf (output_file, "------------------------------------");
    fprintf (output_file, "------------------------------------\n");
  }

  /* --- clean up and exit --- */

/* AGR 611 begin
   We need to check the status of all our fclose commands where we're
   closing files that we write to.  The reason is
   that when you modify an AFS file, you modify the local copy, and no
   data is sent to the server until you do an fclose.  Only on an fclose
   will you find out if someone else has been messing with your file in
   the meantime.  This info from Gary (gap).  94.9.7 */

  if (fclose (output_file)) {
    printf("Error: unable to close file %s\n", file_name);
    return FALSE;
  }
/* AGR 611 end */

  return TRUE;
}

/* ===================================================================
                       Get Context Var Info

   This utility routine is used by interface routines that take context
   variable arguments (e.g., <s> for the current state).  It looks at
   the current lexeme (which must be of type VARIABLE_LEXEME), and
   checks to see if it's a context variable.  Returns:

    if lexeme is not a context variable, dest_attr_of_slot=NIL; else
      dest_attr_of_slot = {goal_symbol, problem_space_symbol, etc.}.
      dest_goal = goal identifier for the given slot (NIL if no such goal)
      dest_current_value = currently installed value (goal id itself for goals,
                           NIL if no installed value)
=================================================================== */
 
/* This routine was moved to command_utils.c for Tcl.  However, it cannot
 * reside outside the Soar kernel since it references lexeme.  This should
 * be rewritten to take a string argument and added to the Soar kernel
 * interface file (whatever that will be).
 */

void old_get_context_var_info (Symbol **dest_goal,
                           Symbol **dest_attr_of_slot,
                           Symbol **dest_current_value) {
  Symbol *v, *g;
  int levels_up;
  wme *w;
  
  v = find_variable (current_agent(lexeme).string);
  if (v==current_agent(s_context_variable)) {
    levels_up = 0;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(o_context_variable)) {
    levels_up = 0;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(ss_context_variable)) {
    levels_up = 1;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(so_context_variable)) {
    levels_up = 1;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(sss_context_variable)) {
    levels_up = 2;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(sso_context_variable)) {
    levels_up = 2;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(ts_context_variable)) {
    levels_up = current_agent(top_goal) ? current_agent(bottom_goal)->id.level-current_agent(top_goal)->id.level : 0;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(to_context_variable)) {
    levels_up = current_agent(top_goal) ? current_agent(bottom_goal)->id.level-current_agent(top_goal)->id.level : 0;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else {
    *dest_goal = NIL;
    *dest_attr_of_slot = NIL;
    *dest_current_value = NIL;
    return;
  }

  g = current_agent(bottom_goal);
  while (g && levels_up) {
    g = g->id.higher_goal;
    levels_up--;
  }
  *dest_goal = g;

  if (!g) {
    *dest_current_value = NIL;
    return;
  }

  if (*dest_attr_of_slot==current_agent(state_symbol)) {
    *dest_current_value = g;
  } else {
    w = g->id.operator_slot->wmes;
    *dest_current_value = w ? w->value : NIL;
  }
}

/* ===================================================================
                  Read Identifier or Context Variable

   Many interface routines take identifiers as arguments.  These ids
   can be given as normal ids, or as special variables such as <s> for
   the current state, etc.  This routine reads (without consuming it)
   an identifier or context variable, and returns a pointer (Symbol *)
   to the id.  (In the case of context variables, the instantiated
   variable is returned.  If any error occurs (e.g., no such id, no
   instantiation of the variable), an error message is printed and
   NIL is returned.
=================================================================== */

Symbol *old_read_identifier_or_context_variable (void) {
  Symbol *id;
  Symbol *g, *attr, *value;

  if (current_agent(lexeme).type==IDENTIFIER_LEXEME) {
    id = find_identifier (current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
    if (!id) {
      print ("There is no identifier %c%lu.\n", current_agent(lexeme).id_letter,
             current_agent(lexeme).id_number);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    return id;
  }
  if (current_agent(lexeme).type==VARIABLE_LEXEME) {
    old_get_context_var_info (&g, &attr, &value);
    if (!attr) {
      print ("Expected identifier (or context variable)\n");
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    if (!value) {
      print ("There is no current %s.\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
      print ("The current %s ", current_agent(lexeme).string);
      print_with_symbols ("(%y) is not an identifier.\n", value);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    return value;
  }
  print ("Expected identifier (or context variable)\n");
  print_location_of_most_recent_lexeme();
  return NIL;
}

/* ===================================================================
                Read Pattern And Get Matching Wmes

   This routine reads a pattern and returns a list of all wmes that
   match it.  At entry, the current lexeme should be the "("; at exit,
   the current lexeme will be the ")".  If any error occurs or if no
   wmes match the pattern, the function returns NIL.

   pattern ::= ( {identifier | '*'} ^ { attribute | '*'} { value | '*' } [+])

=================================================================== */

int old_read_pattern_component (Symbol **dest_sym) {
  /* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
     otherwise return 2 and set dest_sym to find_symbol() result. --- */
  if (strcmp(current_agent(lexeme).string,"*") == 0) return 1;
  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    *dest_sym = find_sym_constant (current_agent(lexeme).string); return 2;
  case INT_CONSTANT_LEXEME:
    *dest_sym = find_int_constant (current_agent(lexeme).int_val); return 2;
  case FLOAT_CONSTANT_LEXEME:
    *dest_sym = find_float_constant (current_agent(lexeme).float_val); return 2;
  case IDENTIFIER_LEXEME:
    *dest_sym = find_identifier (current_agent(lexeme).id_letter, current_agent(lexeme).id_number); return 2;
  case VARIABLE_LEXEME:
    *dest_sym = old_read_identifier_or_context_variable();
    if (*dest_sym) return 2;
    return 0;
  default:
    print ("Expected identifier or constant in wme pattern\n");
    print_location_of_most_recent_lexeme();
    return 0;
  }
}

list *old_read_pattern_and_get_matching_wmes (void) {
  int parentheses_level;
  list *wmes;
  wme *w;
  Symbol *id, *attr, *value;
  int id_result, attr_result, value_result;
  bool acceptable;
  
  if (current_agent(lexeme).type!=L_PAREN_LEXEME) {
    print ("Expected '(' to begin wme pattern\n");
    print_location_of_most_recent_lexeme();
    return NIL;
  }
  parentheses_level = current_lexer_parentheses_level();

  get_lexeme();
  id_result = old_read_pattern_component (&id);
  if (! id_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  if (current_agent(lexeme).type!=UP_ARROW_LEXEME) {
    print ("Expected ^ in wme pattern\n");
    print_location_of_most_recent_lexeme();
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  attr_result = old_read_pattern_component (&attr);
  if (! attr_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  value_result = old_read_pattern_component (&value);
  if (! value_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  if (current_agent(lexeme).type==PLUS_LEXEME) {
    acceptable = TRUE;
    get_lexeme();
  } else {
    acceptable = FALSE;
  }
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Expected ')' to end wme pattern\n");
    print_location_of_most_recent_lexeme();
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }

  wmes = NIL;
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next) {
    if ((id_result==1) || (id==w->id))
      if ((attr_result==1) || (attr==w->attr))
        if ((value_result==1) || (value==w->value))
          if (acceptable==w->acceptable)
            push (w, wmes);
  }
  return wmes;  
}


/* ===================================================================
   
                   Built-In User Interface Commands

=================================================================== */

bool exit_interface_routine (void);
void respond_to_load_errors (void) {
#if !defined(USE_X_DISPLAY) && !defined(_WINDOWS)

  char response [2000];

  if (current_agent(sysparams)[RESPOND_TO_LOAD_ERRORS_SYSPARAM]) {
    print ("Continue?  (Type 'n' or 'N' to abort the load, <Return> to continue):");
    gets (response);
    if ((*response == 'n') || (*response == 'N'))
      current_agent(load_errors_quit) = TRUE;  /* AGR 527c */
  }
#endif
}

/* -------------------------------------------------------------------

                         "Load-errors" Command

   Syntax: (load-errors [on | off])
------------------------------------------------------------------- */

char *help_on_load_errors[] = {
"Command: load-errors",
"",
"Syntax: (load-errors [on | off])",
"",
"With no arguments, this command prints out the current load-errors status.",
"If 'on' is given as the argument, then the user is prompted to continue or",
"not if any errors occur when loading an input file.  If 'off' is given as",
"the argument, then the user is not prompted if any errors occur when loading",
"an input file.  The default status is 'load-errors on'.  Note 'load-errors'",
"is always set to 'off' if Soar is compiled with the multi-agent/X-window",
"option.  This is necessary due to the incompatibility of the I/O streams.",
0 };

bool load_errors_interface_routine (void) {

  get_lexeme();  /* consume "load-errors" */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Load-errors status:  %s\n",
    current_agent(sysparams)[RESPOND_TO_LOAD_ERRORS_SYSPARAM] ? "on" : "off");
    return TRUE;
  }
#ifdef USE_X_DISPLAY
  print ("\nMulti-Agent/X-window option has been selected for compilation\n");
  print ("Load-Errors Mode is always set to FALSE with this option\n");
  return FALSE;
#else
  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,"on")) {
        set_sysparam (RESPOND_TO_LOAD_ERRORS_SYSPARAM, TRUE);
        get_lexeme();
        continue;
      }
      else if (!strcmp(current_agent(lexeme).string,"off")) {
        set_sysparam (RESPOND_TO_LOAD_ERRORS_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      else if (current_agent(lexeme).type != R_PAREN_LEXEME) {
        print ("Unexpected parameters passed to load-errors.\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
    }
  }

  return TRUE;

#endif
}

/* -------------------------------------------------------------------
  
                         "Reset" Command
  
   Syntax: (reset [filename])
------------------------------------------------------------------- */

char *help_on_reset[] = {
"Command: reset",
"",
"Syntax: (reset [filename])",
"",
"Reset loads the file .reset.soar by default or the named file if given.",
"The name of the file then becomes the default on subsequent calls in the same run.",
"The intention is for this file to contain any function calls or other commands",
"needed to reset the system at the end of a run.",
"Examples would include calling init-soar, excising-chunks, restarting a simulator etc.",
0 };

static char reset_filename[256] = { '.','r','e','s','e','t','.','s','o','a','r','\0', };
 
bool reset_interface_routine(void) {

FILE *file;
char *expanded_string;

  get_lexeme();     /* Consume "reset" */

  if (current_agent(lexeme).type == QUOTED_STRING_LEXEME) {
    strcpy(reset_filename,current_agent(lexeme).string);
    get_lexeme();
  }
  if (current_agent(lexeme).type != R_PAREN_LEXEME) {
    print ("Unexpected parameters passed to reset.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  expanded_string = tilde_expand(reset_filename);
  file = fopen(expanded_string,"r");
  free((void *) expanded_string);

  if (file == NULL) {
    print ("Unable to open reset file called %s.\n",reset_filename);
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  load_file(reset_filename,file);
  return TRUE;
}

/* -------------------------------------------------------------------

                         "Explain" Command

   Syntax: (explain arg*) 
            arg ::= :on | :off
            arg ::= :trace
            arg ::= <name>
            arg ::= <name> :trace
            arg ::= <name> <cond-num>
            arg ::= <cond-num>
------------------------------------------------------------------- */

char *help_on_explain[] = {
"Explain provides some interpretation of backtraces generated during",
"chunking.  Explain mode must be ON when the chunk/justification is",
"CREATED or no explanation will be available.  When explain mode is",
"on, more memory is used, and building chunks/justifications will be",
"slower.  The default for explain mode is OFF.",
"",
"Command: explain",
"",
"Syntax: (explain arg* )",
" arg ::= ' '               list chunks/justifications if explain is on",
" arg ::= :on               turn explanation mode on",
" arg ::= :off              turn explanation mode off",
" arg ::= :trace            full backtrace of all chunks/justifications",
"                          stored",
" arg ::= <name>            list all conditions & grounds for a",
"                          chunk/justification",
" arg ::= <name> :trace     give the backtrace for a named",
"                          chunk/justification",
" arg ::= <name> <cond-num> explain why this condition is in the",
"                          chunk/justification",
" arg ::= <cond-num>        explain condition for the last named",
"                          chunk/justification",
"",
"Usage :",
"The two most useful commands are 'explain <name>' and",
"'explain <name> <cond-num>'.",
"The first command lists all of the conditions for the named",
"chunk/justification, and the 'ground' which resulted in",
"inclusion in the chunk/justification.  A 'ground' is a WME which",
"was tested in the supergoal.  Just knowing which WME was tested may",
"be enough to explain why the chunk/justification exists.  If not,",
"the conditions can be listed with an integer value.  This value can",
"be used in 'explain <name> <cond-num>' to obtain a list of the",
"productions which fired to obtain this condition in the",
"chunk/justification (and the crucial WMEs tested along the way).",
"Why use an integer value to specify the condition?",
"To save a big parsing job.  Also, 'explain' remembers the name of the",
"last chunk/justification inquiry, so after entering 'explain <name>'",
"to get a list of conditions, you can just type 'explain <cond-num>'.",
0 };

/***************************************************************************
* Function     : explain_interface_routine
**************************************************************************/

bool explain_interface_routine (void) {
  int  condition_num;
  /* static char explain_chunk_name[256] = { '\0' };  AGR 564 */

  get_lexeme();        /* Consume "explain" */

  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {  
      if (!strcmp(current_agent(lexeme).string, ":on")) {
        current_agent(sysparams)[EXPLAIN_SYSPARAM] = TRUE;
        return FALSE;
      } else if (!strcmp(current_agent(lexeme).string, ":off")) {
          current_agent(sysparams)[EXPLAIN_SYSPARAM] = FALSE;
          return FALSE;
      } else if (!strcmp(current_agent(lexeme).string, ":trace")) {
          print("Full trace of all chunks/justifications\n");
          explain_full_trace();
          return FALSE;
      } else {
        strcpy(current_agent(explain_chunk_name),current_agent(lexeme.string));  /* AGR 564 */
        get_lexeme(); 
        while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
          if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
            if (!strcmp(current_agent(lexeme).string, ":trace")) {
    
              /* handle the 'explain name :trace' case */
              explain_trace_named_chunk(current_agent(explain_chunk_name));  /* AGR 564 */
              return FALSE;
            }
            else {
              print ("Unexpected parameters passed to Explain.\n");
              print_location_of_most_recent_lexeme();
              return FALSE;
            }
          }
          else if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
            /* handle the 'explain name cond' case */
            get_lexeme();
            condition_num = current_agent(lexeme).int_val;
            explain_chunk(current_agent(explain_chunk_name),condition_num);   /* AGR 564 */
            return FALSE;
          }
        }
        /* handle the 'explain name' case */
        explain_cond_list(current_agent(explain_chunk_name));  /* AGR 564 */
        return FALSE;
      }
    }
    else if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {

      /* handle the 'explain cond' case, assume a name is entered already */
      get_lexeme();
      condition_num = current_agent(lexeme).int_val;
      explain_chunk(current_agent(explain_chunk_name),condition_num);  /* AGR 564 */
      return FALSE;
    }
  }
  print ("Explain status:  ");
  if (current_agent(sysparams)[EXPLAIN_SYSPARAM]) {
     print("on\n");
     explain_list_chunks();
  } 
  else print("off\n");
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Log" Command

   Syntax: (log ["filename" [:append]])
------------------------------------------------------------------- */

/* mvp 5-17-94 */
char *help_on_log[] = {
"Command: log",
"",
#ifdef __SC__
"Syntax: (log [\"filename\" [:append]])",
#else
"Syntax: (log [filename [:append]])",
#endif
"",
"The log command turns on and off logging to a file.  When Soar is logging",
"to a file, everything you type and everything Soar prints is written to",
"the file (in addition to the screen).  This is like the (dribble) function",
"in Common Lisp.",
"",
"To start a new log file, type (log \"filename\").",
"To append to an existing file, type (log \"filename\" :append).",
"To stop logging to the current file, type (log).",
0 };

/* mvp 5-17-94 */
bool log_interface_routine (void) {
  char log_file_name [MAX_LEXEME_LENGTH+1];
  bool append;
  
  if (current_agent(logging_to_file)) stop_log_file (); /* close existing log */
  get_lexeme();  /* consume "log" */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) return TRUE;
  
#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for log filename\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif

  strcpy (log_file_name, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (log_file_name, current_agent(lexeme).string);
    get_lexeme ();
  }

  append = FALSE;
  if (!strcmp(current_agent(lexeme).string, ":append")) { append=TRUE; get_lexeme(); }

  start_log_file (log_file_name, append);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Load" Command

   Syntax:  (load "filename")
------------------------------------------------------------------- */

/* mvp 5-17-94 */
char *help_on_load[] = {
"Command: load",
"",
#ifdef __SC__
"Syntax: (load \"filename\")",
#else
"Syntax: (load filename)",
#endif
"",
"Load tells Soar to read commands from the given file instead of the",
"keyboard.  Soar will read and execute each command in the file, and then",
"go back to the keyboard.  Loads may be nested; i.e., the given file may",
"contain a command to load another file, and so on.",
0 };

/* mvp 5-17-94 */
bool load_interface_routine (void) {

  FILE *f;
  char *expanded_string;
  char load_file_name[MAX_LEXEME_LENGTH+1];

  get_lexeme();  /* consume "load", advance to quoted file name */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for filename to load\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif
  strcpy (load_file_name, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (load_file_name, current_agent(lexeme).string);
    get_lexeme ();
  }
  expanded_string = tilde_expand(load_file_name);
  chdir(current_agent(top_dir_stack)->directory);
  f = fopen (expanded_string,"r");
  if (!f) {
    /* --- error when opening the file --- */
    print ("Error: unable to open file %s\n", load_file_name);
    respond_to_load_errors ();
    return FALSE;
  }
  print ("\nLoading %s\n",load_file_name);
  load_file (expanded_string, f);
  fclose (f);
  print ("\n");
  free((void *) expanded_string);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                      "Chdir" and "cd" Commands

   Syntax:  (chdir "pathname") or (cd "pathname")
------------------------------------------------------------------- */

 /*
 * User interface command to allow changing the current (to-be-loaded-from)
 * directory. 
 */

/* mvp 5-17-94 */
char *help_on_chdir[] = {
"Commands: chdir, cd",
"",
#ifdef __SC__
"Syntax: (chdir \"path\") or (cd \"path\")",
#else
"Syntax: (chdir path) or (cd path)",
#endif
"",
"Change the current directory (which files will be loaded from) to the specified",
"directory.",
"",
#ifdef __SC__
"See also:  pwd, ls, pushd, popd, dirs",
#else
"See also:  pwd, pushd, popd, dirs",
#endif
0 };

/* mvp 5-17-94 */
bool chdir_interface_routine (void) {
  int chdir_res;
  char pathname[MAXPATHLEN];   /* AGR 536 */
  char *expanded_string;    /* AGR 562 */
  
  get_lexeme();  /* consume "chdir", advance to quoted path name */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for directory pathname\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif

  strcpy (pathname, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (pathname, current_agent(lexeme).string);
    get_lexeme ();
  }

  expanded_string = tilde_expand(pathname);
  print ("Changing to directory: %s\n", expanded_string);  /* AGR 562 */

  chdir_res =  chdir(expanded_string);  /* AGR 562 */
  if (chdir_res)
    print("  FAILED.\n");
  else {
    if(getwd(pathname))
      strcpy(current_agent(top_dir_stack)->directory, pathname);
  }  

  free((void *) expanded_string);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                           "Pwd" Command

   Syntax:  (pwd)
------------------------------------------------------------------- */

char *help_on_pwd[] = {
"Command: pwd",
"",
"Syntax: (pwd)",
"",
"[Print Working Directory]  Prints the current working directory.",
"",
#ifdef __SC__
"See also:  chdir, ls, pushd, popd, dirs",
#else
"See also:  chdir, pushd, popd, dirs",
#endif
0 };

bool pwd_interface_routine (void) {
  char pathname[MAXPATHLEN];   /* AGR 536 */
/*  char *getwd_result;    found to be unnecessary AGR 10-May-94 */

  get_lexeme();  /* consume "pwd" */

  chdir(current_agent(top_dir_stack)->directory);
  if (getwd (pathname)) {
    print ("Current directory: %s\n", pathname);
    strcpy(current_agent(top_dir_stack)->directory, pathname);
  } else {
    print ("Error: unable to determine current working directory.\n");
  }
  
  return TRUE;
}

#ifdef __SC__
/* -------------------------------------------------------------------
   
                           "ls" and "lf" Commands

   Syntax:  (ls) or (lf)
------------------------------------------------------------------- */

char *help_on_ls_or_lf[] = {
"Command: ls, lf",
"",
"Syntax: (ls) or (lf)",
"",
"List all files and subdirectories in current working directory.",
"",
"See also:  cd, pwd, pushd, popd",
0 };

bool ls_interface_routine (void) {
  char pathname[MAXPATHLEN];
  get_lexeme();  /* consume "ls" or "lf" */

  /*chdir(current_agent(path));*/
  if (getwd(pathname))
  {
  	print("\nCurrent directory is: %s\n\n",pathname);
  }
  
  if (!ls()) {
    print ("Error: internal error occurred while accessing directory.\n");
  }
  
  return TRUE;
}
#endif

/* AGR 568 begin */
/* -------------------------------------------------------------------

                          "Pushd" Command

   Syntax:  (pushd "pathname")
------------------------------------------------------------------- */

/* mvp 5-17-94 */
char *help_on_pushd[] = {
"Command: pushd",
"",
#ifdef __SC__
"Syntax: (pushd \"pathname\")",
#else
"Syntax: (pushd pathname)",
#endif
"",
"[Push Directory]  Pushes the specified directory onto the directory stack,",
"such that the new current working directory becomes the specified directory.",
"",
#ifdef __SC__
"See also:  popd, dirs, chdir, pwd, ls",
#else
"See also:  popd, dirs, chdir, pwd",
#endif
0 };


/* mvp 5-17-94 */
bool pushd_interface_routine (void) {
  char pathname[MAXPATHLEN];
  char *expanded_string;
  dir_stack_struct *top_dir;
  int chdir_result;

  get_lexeme();  /* consume "pushd", advance to quoted path name */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for directory pathname\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif

  strcpy (pathname, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (pathname, current_agent(lexeme).string);
    get_lexeme ();
  }

  expanded_string = tilde_expand(pathname);
  expanded_string = (char *) realloc(expanded_string, MAXPATHLEN*sizeof(char));
  print ("Pushing directory onto the stack: %s\n", expanded_string);

  chdir_result = chdir(expanded_string);
  if (chdir_result)
    print("  FAILED.\n");
  else {
    top_dir = (dir_stack_struct *) malloc(sizeof(dir_stack_struct));
    top_dir->directory = expanded_string;
    top_dir->next = current_agent(top_dir_stack);
    current_agent(top_dir_stack) = top_dir;
    if(getwd(pathname))
      strcpy(current_agent(top_dir_stack)->directory, pathname);
  }  

  return TRUE;
}

/* -------------------------------------------------------------------

                          "Popd" Command

   Syntax:  (popd)
------------------------------------------------------------------- */

char *help_on_popd[] = {
"Command: popd",
"",
"Syntax: (popd)",
"",
"[Pop Directory]  Pops the top entry off of the directory stack, such that",
"the entry immediately underneath it becomes the current working directory.",
"",
#ifdef __SC__
"See also:  pushd, dirs, chdir, pwd, ls",
#else
"See also:  pushd, dirs, chdir, pwd",
#endif
0 };

bool popd_interface_routine (void) {
  char pathname[MAXPATHLEN];
  dir_stack_struct *top_dir;

  get_lexeme();  /* consume "popd" */

  top_dir = current_agent(top_dir_stack)->next;
  if (top_dir) {
    print ("Popping off directory: %s\n", current_agent(top_dir_stack)->directory);
    free((void *) current_agent(top_dir_stack)->directory);
    free((void *) current_agent(top_dir_stack));
    current_agent(top_dir_stack) = top_dir;

    chdir(current_agent(top_dir_stack)->directory);
    if (getwd (pathname)) {
      print ("Current directory now is: %s\n", pathname);
      strcpy(current_agent(top_dir_stack)->directory, pathname);
    } else {
      print ("Error: unable to determine current working directory.\n");
    }
  } else
    print ("Can't pop off the only entry on the directory stack.\n");

  return TRUE;
}

/* -------------------------------------------------------------------

                          "Dirs" Command

   Syntax:  (dirs)
------------------------------------------------------------------- */

char *help_on_dirs[] = {
"Command: dirs",
"",
"Syntax: (dirs)",
"",
"[Show Directory Stack]  Lists the entries in the directory stack, starting",
"with the current working directory.",
"",
#ifdef __SC__
"See also:  pushd, popd, chdir, pwd, ls",
#else
"See also:  pushd, popd, chdir, pwd",
#endif
0 };

bool dirs_interface_routine (void) {
  dir_stack_struct *cur_dir;

  get_lexeme();  /* consume "dirs" */

  cur_dir = current_agent(top_dir_stack);
  while (cur_dir) {
    print ("%s\n", cur_dir->directory);
    cur_dir = cur_dir->next;
  }

  return TRUE;
}

/* AGR 568  The alias feature was a really ugly beast to code.  Initially,
   I decided to make the alias and the expansion to be both quoted strings,
   with the intent being to do a simple string substitution when the
   alias was used.  However, since a single string can be made up of
   many lexemes, it was impossible to do it that way, since I couldn't
   break up the string and have each lexeme be read in as it came.

   The better solution was to not use strings and instead have get_lexeme
   read in each lexeme in the expansion while the alias is being defined.
   Each lexeme would then get added to the end of a linked list.
   Then, when the alias gets used, get_lexeme() realizes that an alias
   is being used and redirects input to come from that list of lexemes,
   until it gets to the end when it just continues getting its input
   from the input stream like before.  AGR 12-May-94 */

/* -------------------------------------------------------------------

                          "Alias" Command

   Syntax:  (alias alias expansion)
------------------------------------------------------------------- */

char *help_on_alias[] = {
"Command: alias",
"",
"Syntax: (alias alias expansion)",
"Syntax: (alias)",
"",
"[Alias Command]  Allows you to specify a short sequence of keystrokes",
"(the alias) to replace a longer sequence of keystrokes (the expansion)",
"when entering Soar commands.",
"",
"For example, ``alias lht list-help-topics'' will allow you to",
"type lht to get a list of all the help topics.  Similarly, ``alias",
"sa select-agent'' will allow you to type ``sa foo'' in place of",
"``select-agent foo''.",
"",
"With no arguments, ``alias'' will show you a list of the aliases that",
"have been defined.",
"",
"See also:  unalias",
0 };

void alias_print_expansion(expansion)
     expansion_node *expansion;
{
  while (expansion) {
    if (expansion->lexeme.type == QUOTED_STRING_LEXEME) print("\"");
    print("%s ", expansion->lexeme.string);
    if (expansion->lexeme.type == QUOTED_STRING_LEXEME) print("\"");
    expansion = expansion->next;
  }
  print("\n");
}

expansion_node *alias_add_to_list(expansion, cur_lexeme)
     expansion_node *expansion, *cur_lexeme;
{
  expansion_node *temp;

  if (!expansion) return cur_lexeme;
  temp = expansion;
  while (temp->next) temp = temp->next;
  temp->next = cur_lexeme;
  return expansion;
}

void alias_free_expansion(node)
     expansion_node *node;
{
  if (node) {
    alias_free_expansion(node->next);
    free((void *) node);
  }
}

bool alias_interface_routine (void) {
  int found = 0;
  char *alias;
  expansion_node *expansion = NIL;
  expansion_node *cur_lexeme;
  alias_struct *new_alias, *cur_alias, *last_alias;
  int alias_paren_level = 0;
  int alias_leading_parens = 0;
  bool leading_parens = TRUE;
  int missing_rparens = FALSE;

  get_lexeme();  /* consume "alias", advance to alias */

  if (current_agent(current_char) == '\n') {
    cur_alias = current_agent(alias_list);
    while (cur_alias) {
      print ("alias   %s   ", cur_alias->alias);
      alias_print_expansion(cur_alias->expansion);
      cur_alias = cur_alias->next;
    }
    return TRUE;
  }

  alias = (char *) malloc(1+strlen(current_agent(lexeme).string));
  strcpy(alias, current_agent(lexeme).string);

  while ((current_agent(current_char)!='\n') || (missing_rparens)) {
    missing_rparens = FALSE;
    get_lexeme();
    switch (current_agent(lexeme).type) {
    case L_PAREN_LEXEME:
      if (leading_parens) alias_leading_parens++;
      else {
	alias_paren_level++;
	cur_lexeme = (expansion_node *) malloc(sizeof(expansion_node));
	cur_lexeme->lexeme = current_agent(lexeme);
	cur_lexeme->next = NIL;
	expansion = alias_add_to_list(expansion, cur_lexeme);
      }
      break;
    case R_PAREN_LEXEME:
      if (alias_paren_level <= 0) {
	if (alias_leading_parens <= 0) {
	  if (current_agent(current_char) != '\n') {
	    print("Ignoring unmatched right parenthesis\n");
	    print_location_of_most_recent_lexeme();
	  }
	} else alias_leading_parens--;
      } else {
	alias_paren_level--;
	cur_lexeme = (expansion_node *) malloc(sizeof(expansion_node));
	cur_lexeme->lexeme = current_agent(lexeme);
	cur_lexeme->next = NIL;
	expansion = alias_add_to_list(expansion, cur_lexeme);
      }
      break;
    default:
      leading_parens = FALSE;
      cur_lexeme = (expansion_node *) malloc(sizeof(expansion_node));
      cur_lexeme->lexeme = current_agent(lexeme);
      cur_lexeme->next = NIL;
      expansion = alias_add_to_list(expansion, cur_lexeme);
      break;
    }
    if ((current_agent(current_char)=='\n') &&
	((alias_paren_level) || (alias_leading_parens)))
      missing_rparens = TRUE;
  }
  if (current_lexer_parentheses_level()) get_lexeme();

  if (!expansion) {
    print("Alias declaration invalid - no expansion.", alias);
    free((void *) alias);
    return TRUE;
  } else if (!strcmp(expansion->lexeme.string,alias)) {
    print("You can't define an alias as itself\n");
    free((void *) alias);
    alias_free_expansion(expansion);
    return TRUE;
  }

  new_alias = (alias_struct *) malloc(sizeof(alias_struct));
  new_alias->alias = alias;
  new_alias->expansion = expansion;

/* Look for old alias with same name so as to replace it.  We don't want
   any duplicates.  */

  last_alias = cur_alias = current_agent(alias_list);
  while (cur_alias) {
    if (strcmp(new_alias->alias, cur_alias->alias)) {
      last_alias = cur_alias;
      cur_alias = cur_alias->next;
    } else {
      found = 1;
      if (last_alias != cur_alias) {
	last_alias->next = new_alias;
	new_alias->next = cur_alias->next;
      } else {
	current_agent(alias_list) = new_alias;
	new_alias->next = cur_alias->next;
      }
      if (current_agent(sysparams)[PRINT_ALIAS_SYSPARAM])  /* AGR 627 */
	print("New %s alias successfully replaced old.\n", cur_alias->alias);
      free((void *) cur_alias->alias);
      alias_free_expansion(cur_alias->expansion);
      free((void *) cur_alias);
      cur_alias = NIL;
    }
  }
  if (!found) {
    if (last_alias) {
      last_alias->next = new_alias;
      new_alias->next = NIL;
    } else {
      current_agent(alias_list) = new_alias;
      new_alias->next = NIL;
    }
    if (current_agent(sysparams)[PRINT_ALIAS_SYSPARAM])  /* AGR 627 */
      print("Alias %s successfully added.\n", new_alias->alias);
  }

  return TRUE;
}

/* -------------------------------------------------------------------

                         "Unalias" Command

   Syntax:  (unalias "alias")
------------------------------------------------------------------- */

char *help_on_unalias[] = {
"Command: unalias",
"",
"Syntax: (unalias alias)",
"",
"[Unalias Command]  Removes a previously defined alias.",
"",
"See also:  alias",
0 };

bool unalias_interface_routine (void) {
  int found = 0;
  alias_struct *cur_alias, *last_alias;

  get_lexeme();  /* consume "alias", advance to alias */
  last_alias = cur_alias = current_agent(alias_list);
  while (cur_alias) {
    if (strcmp(current_agent(lexeme).string, cur_alias->alias)) {
      last_alias = cur_alias;
      cur_alias = cur_alias->next;
    } else {
      found = 1;
      if (last_alias != cur_alias)
	last_alias->next = cur_alias->next;
      else
	current_agent(alias_list) = cur_alias->next;
      if (current_agent(sysparams)[PRINT_ALIAS_SYSPARAM])  /* AGR 627 */
      print("Alias %s successfully removed.\n", cur_alias->alias);
      free((void *) cur_alias->alias);
      alias_free_expansion(cur_alias->expansion);
      free((void *) cur_alias);
      cur_alias = NIL;
    }
  }
  if (!found) print("Alias %s not found.\n", current_agent(lexeme).string);

  get_lexeme();  /* consume alias */
  return TRUE;
}
/* AGR 568 end */

/* AGR 627 begin */
/* AGR 627  This was Frank Ritter's idea.  The idea is that you have a
   flag that you can flip, as to whether you'd like notification of when
   an alias has been defined.  These notifications take up screen space,
   which is why Frank doesn't like them, because he has lots of aliases.
   This code was ported from load-errors.  94.11.02 */

/* -------------------------------------------------------------------

                         "Print-alias" Command

   Syntax: (print-alias [on | off])
------------------------------------------------------------------- */

char *help_on_print_alias[] = {
"Command: print-alias",
"",
"Syntax: (print-alias [on | off])",
"",
"With no arguments, this command prints out the current print-alias status.",
"If 'on' is given as the argument, then the user notified when an alias",
"has been defined.  If 'off' is given as the argument, then the user is",
"not notified.",
0 };

bool print_alias_interface_routine (void) {

  get_lexeme();  /* consume "print-alias" */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Print-alias status:  %s\n",
    current_agent(sysparams)[PRINT_ALIAS_SYSPARAM] ? "on" : "off");
    return TRUE;
  }
  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,"on")) {
        set_sysparam (PRINT_ALIAS_SYSPARAM, TRUE);
        get_lexeme();
        continue;
      }
      else if (!strcmp(current_agent(lexeme).string,"off")) {
        set_sysparam (PRINT_ALIAS_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      else if (current_agent(lexeme).type != R_PAREN_LEXEME) {
        print ("Unexpected parameters passed to print-alias.\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
    }
  }
  return TRUE;
}
/* AGR 627 end */

/* -------------------------------------------------------------------
   
                    "Exit" and "Quit" Commands

   Syntax: (exit) or (quit)
------------------------------------------------------------------- */

char *help_on_exit[] = {
"Commands: exit, quit",
"",
"Syntax: (exit) or (quit)",
"",
"These two commands are synonymous; they cause Soar to terminate and return",
"control to the shell.",
0 };

bool exit_interface_routine (void) {
  print ("Exiting Soar...\n");
  exit_soar();
  return FALSE; /* unreachable, but without it, gcc -Wall warns here */
}

/* -------------------------------------------------------------------
   
                            "Pgs" Command

   Syntax: (pgs)
------------------------------------------------------------------- */

char *help_on_pgs[] = {
"Command: pgs",
"",
"Syntax: (pgs)",
"",
"Pgs (\"print goal stack\") prints Soar's current context stack.",
0 };

bool pgs_interface_routine (void) {
  Symbol *g;

  for (g=current_agent(top_goal); g!=NIL; g=g->id.lower_goal) {
    print_stack_trace (g, g, FOR_STATES_TF, FALSE);
    print ("\n");
    if (g->id.operator_slot->wmes) {
      print_stack_trace (g->id.operator_slot->wmes->value,
                         g, FOR_OPERATORS_TF, FALSE);
      print ("\n");
    }
  }
  get_lexeme();  /* consume "pgs", advance to rparen */
  return TRUE;
}

/* -------------------------------------------------------------------
   
                            "Pgso" Command

   Syntax: (pgso)
------------------------------------------------------------------- */

char *help_on_pgso[] = {
"Command: pgso",
"",
"Syntax: (pgso)",
"",
"Pgso (\"print goal stack operators\") prints only the operators",
"in Soar's current context stack.",
0 };

bool pgso_interface_routine (void) {
  Symbol *g;

  for (g=current_agent(top_goal); g!=NIL; g=g->id.lower_goal) {
    if (g->id.operator_slot->wmes) {
      print_stack_trace (g->id.operator_slot->wmes->value,
                         g, FOR_OPERATORS_TF, FALSE);
      print ("\n");
    }
  }
  get_lexeme();  /* consume "pgso", advance to rparen */
  return TRUE;
}


/* -------------------------------------------------------------------
   
                  "Excise" and "Excise-xxx" Commands

   Syntax: (excise production-name*)
           (excise-chunks)
           (excise-task)
           (excise-all)
------------------------------------------------------------------- */

char *help_on_excise[] = {
"Command: excise",
"",
"Syntax: (excise production-name*)",
"",
"This command removes the given production(s) from the system.",
"",
"See also: excise-chunks, excise-task, excise-all",
0 };

char *help_on_excise_chunks[] = {
"Command: excise-chunks",
"",
"Syntax: (excise-chunks)",
"",
"This command removes all chunks and justifications from the system.",
"",
"See also: excise, excise-task, excise-all",
0 };

char *help_on_excise_task[] = {
"Command: excise-task",
"",
"Syntax: (excise-task)",
"",
"This command removes all non-default productions from the system.  It also",
"does an (init-soar).",
"",
"See also: excise, excise-chunks, excise-all",
0 };

char *help_on_excise_all[] = {
"Command: excise-all",
"",
"Syntax: (excise-all)",
"",
"This command removes all productions from the system.  It also does an",
"(init-soar)",
"",
"See also: excise, excise-chunks, excise-task",
0 };

bool excise_interface_routine (void) {
  Symbol *sym;
  
  set_lexer_allow_ids (FALSE);
  get_lexeme();  /* consume "excise", advance to production name(s) */
  while (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    sym = find_sym_constant (current_agent(lexeme).string);
    if (sym && sym->sc.production) {
      excise_production (sym->sc.production, TRUE);
    } else {
      print ("No production named %s\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
    }
    get_lexeme(); /* consume this one, advance to next production name */
  }
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Expected Symbol for name of production to excise\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

void old_excise_all_productions_of_type (byte type) {
  while (current_agent(all_productions_of_type)[type])
    excise_production (current_agent(all_productions_of_type)[type], TRUE);
}

bool excise_chunks_interface_routine (void) {
  old_excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
  get_lexeme();  /* consume "excise-chunks" */
  return TRUE;
}

bool excise_task_interface_routine (void) {
  old_excise_all_productions_of_type (USER_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
  reinitialize_soar();  /* for excise-task, also do an init-soar */
  get_lexeme();  /* consume "excise-task" */
  return TRUE;
}

bool excise_all_interface_routine (void) {
  old_excise_all_productions_of_type (DEFAULT_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (USER_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (CHUNK_PRODUCTION_TYPE);
  old_excise_all_productions_of_type (JUSTIFICATION_PRODUCTION_TYPE);
  reinitialize_soar();  /* for excise-all, also do an init-soar */
  get_lexeme();  /* consume "excise-all" */
  return TRUE;
}

/* AGR REW1 begin */
/* -------------------------------------------------------------------
   
                       "Input-period" Command

   Syntax: (input-period [integer])
------------------------------------------------------------------- */

/* REW is Bob Wray (wrayre@eecs.umich.edu) who says:
   The command itself is called "input-period".  It takes as an
   argument a positive integer N.  For N = 0, Soar behaves exactly
   as it does now, accepting input every elaboration cycle.  This is
   the default behavior.  For N > 0, Soar accepts input every N 
   decisions.  */

char *help_on_input_period[] = {
"Command: input-period",
"",
"Syntax: (input-period [integer])",
"",
"With no arguments, this command prints the current input period.  With an",
"integer argument, it sets the input period to that argument.  If n=0, which",
"is the default, Soar behaves as it did in versions before 6.2.4, accepting",
"input every elaboration cycle.  An input period of n means that input is",
"accepted only every n decision cycles.",
0 };

bool input_period_interface_routine(void) {

  get_lexeme();  /* consume "input-period", look for optional args */

  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the input period --- */
    print ("The current input period is %d.\n", current_agent(input_period));
    return TRUE;
  }
  if (current_agent(lexeme).type!=INT_CONSTANT_LEXEME) {
    print ("Expected integer for new input period\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  if (current_agent(lexeme).int_val < 0 ) {
    print("Period should be =>0.\n");
    return FALSE;
  }
  else
     current_agent(input_period) = current_agent(lexeme).int_val;

  get_lexeme();  /* consume the argument */
  return TRUE;
}
/* AGR REW1 end */

/* -------------------------------------------------------------------
   
                          "Matches" Command

   Syntax: (matches production-name [ 0 | 1 | 2 ])
------------------------------------------------------------------- */

char *help_on_matches[] = {
"Command: matches",
"",
"Syntax: (matches production-name [ 0 | 1 | 2 ])",
"",
"This command prints partial match information for the given production.",
"The optional integer specifies the level of detail wanted:  0 (the default)",
"prints out just the partial match counts, ala 'smatches'; 1 also prints",
"the timetags of wmes at the first failing condition, ala 'full-matches';",
"and 2 prints the wmes rather than just their timetags.",
0 };

bool matches_interface_routine (void) {
  Symbol *sym;
  wme_trace_type wtt;
  
  set_lexer_allow_ids (FALSE);
  get_lexeme();  /* consume "matches", advance to production name(s) */
  if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
    print ("Expected symbol for name of production for 'matches' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  sym = find_sym_constant (current_agent(lexeme).string);
  if ((!sym) || (! sym->sc.production)) {
    print ("No production named %s\n", current_agent(lexeme).string);
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  get_lexeme(); /* consume production name, look for level */
  wtt = NONE_WME_TRACE;
  if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
    if ((current_agent(lexeme).int_val>=0) && (current_agent(lexeme).int_val<=2)) {
      if (current_agent(lexeme).int_val==0) wtt = NONE_WME_TRACE;
      if (current_agent(lexeme).int_val==1) wtt = TIMETAG_WME_TRACE;
      if (current_agent(lexeme).int_val==2) wtt = FULL_WME_TRACE;
      get_lexeme();
    } else {
      print ("Matches 'level' must be 0, 1, or 2.\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
  }
  print_partial_match_information (sym->sc.production->p_node, wtt);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                    "default-wme-depth" Command
 
   Syntax:  (default-wme-depth [integer])
------------------------------------------------------------------- */

/* AGR 646  The variable default_wme_depth used to be a simple global,
   defined here.  That meant there was only 1 such variable, which
   permeated all the Soar agents.  We figured that it was best if each
   Soar agent had its own default_wme_depth, so we moved it to
   soarkernel.h and changed references to it to current_agent(...).
   94.11.11  */

char *help_on_default_wme_depth[] = {
"Command: default-wme-depth",
"",
"Syntax: (default-wme-depth [integer])",
"",
"With no arguments, this command prints the current default print depth used",
"by the (print) command.  With an integer argument, it sets the current",
"default print depth.   This default print depth can be overridden on any",
"particular invocation of the (print) command by using the :depth flag,",
"e.g., (print :depth 10 args...).  The default print depth is initially 1.",
"",
"See also:  print",
0 };

bool default_wme_depth_interface_routine (void) {
  get_lexeme();  /* consume "default-wme-depth", advance to integer */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the default depth --- */
    print ("The current default print depth is %d.\n", current_agent(default_wme_depth));  /* AGR 646 */
    return TRUE;
  }
  if (current_agent(lexeme).type!=INT_CONSTANT_LEXEME) {
    print ("Expected integer for new default print depth\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  /* AGR 646 for next command */
  current_agent(default_wme_depth) = current_agent(lexeme).int_val;
  get_lexeme(); /* consume the integer */
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Print" Command

   Syntax:  see help screen below.
------------------------------------------------------------------- */

char *help_on_print[] = {
"Commands: print, p, spr, wm",
"",
"Syntax:  (print [:depth n] [:internal] arg*)",
"         (p ...) and (spr ...) are shorthand for (print ...)",
"         (wm ...) is shorthand for (print :depth 0 :internal ...)",
"",
"The print command is used to print items from production memory or working",
"memory.  It can take several kinds of arguments:",
"",
"  arg ::= production-name  (print that production)",
"  arg ::= identifier       (id of the object to print)",
"  arg ::= integer          (timetag of wme--the identifier from the wme",
"                            indicates the object to be printed)",
"  arg ::= pattern          (pattern--same as if you listed as arguments",
"                            the timetags of all wmes matching the pattern)",
"",
"  pattern ::= ( {identifier | '*'} ^ { attribute | '*'} { value | '*' } [+])",
"",
"The optional [:depth n] argument overrides default-wme-depth.",
"",
"The optional [:internal] argument tells Soar to print things in their",
"internal form.  For productions, this means leaving conditions in their",
"reordered (rete net) form.  For wmes, this means printing the individual",
"wmes with their timetags, rather than the objects.",
"",
":depth 0 is meaningful only for integer and pattern arguments.  It causes",
"only the matching wmes to be printed, instead of all wmes whose id is an id",
"in one of the matching wmes.",
"",
"See also:  default-wme-depth",
0 };

/* These functions need to be copied to command_utils.c or
 * moved to a Soar kernel interface file. */

void old_neatly_print_wme_augmentation_of_id (wme *w, int indentation) {
  char buf[10000], *ch;

  strcpy (buf, " ^");
  ch = buf;
  while (*ch) ch++;
  symbol_to_string (w->attr, TRUE, ch); while (*ch) ch++;
  *(ch++) = ' ';
  symbol_to_string (w->value, TRUE, ch); while (*ch) ch++;
  if (w->acceptable) { strcpy (ch, " +"); while (*ch) ch++; }

  if (get_printer_output_column() + (ch - buf) >= 80) {
    print ("\n");
    print_spaces (indentation+6);
  }
  print_string (buf);
}

/* AGR 652 begin */
/* Comment from Bob:  
   Also, if speed becomes an issue, you might put in a special case check
   for the common case where both p1 and p2 are SYM_CONSTANT's, in which
   case you could avoid the symbol_to_string() calls entirely and just
   call strcmp() directly on the attribute names.  (Maybe just leave this
   as a comment for now, just in case somebody complains about print speed
   some day.)  */

/* The header for old_compare_attr needs to be defined in this way because
   otherwise we get compiler warnings at the qsort lines about the 4th
   argument being an incompatible pointer type.  */

int old_compare_attr (const void * e1, const void * e2)
{
  wme **p1, **p2;

  char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

  p1 = (wme **) e1;
  p2 = (wme **) e2;

  symbol_to_string ((*p1)->attr, TRUE, s1);
  symbol_to_string ((*p2)->attr, TRUE, s2);

  return (strcmp (s1, s2));
}

void old_print_augs_of_id (Symbol *id, int depth, bool internal,
                       int indent, tc_number tc) {
  slot *s;
  wme *w;

  wme **list;    /* array of WME pointers, AGR 652 */
  int num_attr;  /* number of attributes, AGR 652 */
  int attr;      /* attribute index, AGR 652 */

/* AGR 652  The plan is to go through the list of WMEs and find out how
   many there are.  Then we malloc an array of that many pointers.
   Then we go through the list again and copy all the pointers to that array.
   Then we qsort the array and print it out.  94.12.13 */

  if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return;
  if (id->id.tc_num==tc) return;
  id->id.tc_num = tc;

  /* --- first, count all direct augmentations of this id --- */
  num_attr = 0;
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next) num_attr++;
  for (w=id->id.input_wmes; w!=NIL; w=w->next) num_attr++;
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next) num_attr++;
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) num_attr++;
  }

  /* --- next, construct the array of wme pointers and sort them --- */
  list = allocate_memory(num_attr*sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
  attr = 0;
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
    list[attr++] = w;
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    list[attr++] = w;
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next)
      list[attr++] = w;
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
      list[attr++] = w;
  }
  qsort (list, num_attr, sizeof (wme *), old_compare_attr);

  /* --- finally, print the sorted wmes and deallocate the array --- */
  if (internal) {
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      print_spaces (indent);
      print_wme (w);
    }
  } else {
    print_spaces (indent);
    print_with_symbols ("(%y", id);
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      old_neatly_print_wme_augmentation_of_id (w, indent);
    }
    print (")\n");
  }
  free_memory(list, MISCELLANEOUS_MEM_USAGE);
/* AGR 652 end */

  /* --- if depth<=1, we're done --- */
  if (depth<=1) return;

  /* --- call this routine recursively --- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next) {
    old_print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
    old_print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
  }
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next) {
    old_print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
    old_print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
  }
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next) {
      old_print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
      old_print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
    }
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) {
      old_print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
      old_print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
    }
  }
}

void old_do_print_for_production_name (char *prod_name, bool internal) {
  Symbol *sym;

  sym = find_sym_constant (current_agent(lexeme).string);
  if (sym && sym->sc.production) {
    print_production (sym->sc.production, internal);
    print ("\n");
  } else {
    print ("No production named %s\n", prod_name);
    print_location_of_most_recent_lexeme();
  }
}

void old_do_print_for_identifier (Symbol *id, int depth, bool internal) {
  tc_number tc;

  tc = get_new_tc_number();
  old_print_augs_of_id (id, depth, internal, 0, tc);
}

void old_do_print_for_wme (wme *w, int depth, bool internal) {
  tc_number tc;

  if (internal && (depth==0)) {
    print_wme (w);
  } else {
    tc = get_new_tc_number();
    old_print_augs_of_id (w->id, depth, internal, 0, tc);
  }
}

bool print_interface_routine (void) {
  bool internal;
  int depth;
  Symbol *id;
  wme *w;
  list *wmes;
  cons *c;

  internal = FALSE;
  depth = current_agent(default_wme_depth);  /* AGR 646 */

  /* --- if the user typed "wm", change initial internal, depth values --- */
  if (!strcmp(current_agent(lexeme).string,"wm")) {
    internal = TRUE;
    depth = 0;
  }

  get_lexeme();  /* consume command name, advance to optional flags */

  /* --- read optional :depth and :internal flags --- */
  while (TRUE) {
    if (current_agent(lexeme).type == R_PAREN_LEXEME) {
      print ("Expected argument to 'print' command\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
  
    if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) break;

    if (!strcmp(current_agent(lexeme).string,":depth")) {
      get_lexeme();
      if (current_agent(lexeme).type != INT_CONSTANT_LEXEME) {
        print ("Expected integer for value of :depth argument\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      depth = current_agent(lexeme).int_val;
      get_lexeme();
      continue;
    }
    if (!strcmp(current_agent(lexeme).string,":internal")) {
      internal = TRUE;
      get_lexeme();
      continue;
    }
    break;
  }

  /* --- repeat: read one arg and print it --- */
  while (current_agent(lexeme).type != R_PAREN_LEXEME) {
    switch (current_agent(lexeme).type) {

    case SYM_CONSTANT_LEXEME:
      old_do_print_for_production_name (current_agent(lexeme).string, internal);
      get_lexeme();
      break;

    case INT_CONSTANT_LEXEME:
      for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
        if (w->timetag == (unsigned) (current_agent(lexeme).int_val)) break;
      if (w) {
        old_do_print_for_wme (w, depth, internal);
      } else {
        print ("No wme %ld in working memory\n", current_agent(lexeme).int_val);
      }
      get_lexeme();
      break;

    case IDENTIFIER_LEXEME:
    case VARIABLE_LEXEME:
      id = old_read_identifier_or_context_variable();
      if (id) old_do_print_for_identifier (id, depth, internal);
      get_lexeme();
      break;

    case L_PAREN_LEXEME:
      wmes = old_read_pattern_and_get_matching_wmes ();
      for (c=wmes; c!=NIL; c=c->rest)
        old_do_print_for_wme (c->first, depth, internal);
      free_list (wmes);
      get_lexeme();
      break;

    default:
      print ("Illegal argument to 'print' command\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    } /* end of switch statement */
  } /* end of while loop */
  return TRUE;
}

/* AGR DJP1 begin */
/* -------------------------------------------------------------------

                            "Pf" Command

   Syntax:  see help screen below.
------------------------------------------------------------------- */

char *help_on_pf[] = {
"Command: pf",
"",
"Syntax: (pf arg lhs-conditions-or-rhs-actions)",
"        arg ::= [:rhs] [:chunks|:nochunks] [:command <command>]",
"",
"PF is a production finding facility.  It allows you to",
"find productions that either test a particular LHS pattern",
"or produce particular RHS preferences.",
"",
"The syntax of the lhs-conditions or rhs-actions is exactly",
"their syntax within SP's.  In addition, the symbol '*' may",
"be used as a wildcard for an attribute or value.  Note that",
"variable names do not have to match the specific names used",
"in productions.",
"",
"A command may optionally be supplied, which is applied to each",
"of the productions that are found.  Currently, only excise is",
"supported.",
"",
"Specifying :chunks means only chunks are searched (and :nochunks means",
"no chunks are searched).",
"",
"Examples:",
"",
"   Find productions that test that some object gumby has attribute",
"   ^alive t, and test an operator named foo:",
"       pf (<s> ^gumby <gv> ^operator.name foo)(<gv> ^alive t)",
"",
"   Find productions that propose foo:",
"       pf :rhs (<x> ^operator <op> +)(<op> ^name foo)",
"",
"   Find productions that test the attribute ^pokey:",
"       pf (<x> ^pokey *)",
"",
"   Find and excise productions that test the operator foo",
"       pf :command excise (<s> ^operator.name foo)",
0};

/* HERE are the key routines for matching a condition while keeping/producing
   a list of bindings intact.  */

typedef struct binding_structure {
  Symbol *from, *to;
} Old_Binding;

Symbol *old_get_binding (Symbol *f, list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest) {
    if (((Old_Binding *) c->first)->from == f)
      return ((Old_Binding *) c->first)->to;
  }
  return NIL;
}

void old_reset_old_binding_point(list **bindings, list **current_binding_point) {
  cons *c,*c_next;

  c = *bindings;
  while (c != *current_binding_point) {
    c_next = c->rest;
    free_memory(c->first,MISCELLANEOUS_MEM_USAGE);
    free_cons (c);
    c = c_next;
  }

  bindings = current_binding_point;
}

void old_free_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    free_memory(c->first,MISCELLANEOUS_MEM_USAGE);
  free_list(bindings);
}

void old_print_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    print_with_symbols ("   (%y -> %y)\n",((Old_Binding *) c->first)->from,((Old_Binding *) c->first)->to);
}


bool old_symbols_are_equal_with_bindings (Symbol *s1, Symbol *s2, list **bindings) {
  Old_Binding *b;
  Symbol *bvar;

  /* SBH/MVP 7-5-94 */
  if ((s1 == s2) && (s1->common.symbol_type != VARIABLE_SYMBOL_TYPE))
    return TRUE;

  /* "*" matches everything. */
  if ((s1->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
      (!strcmp(s1->sc.name,"*"))) return TRUE;
  if ((s2->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
      (!strcmp(s2->sc.name,"*"))) return TRUE;


  if ((s1->common.symbol_type != VARIABLE_SYMBOL_TYPE) ||
      (s2->common.symbol_type != VARIABLE_SYMBOL_TYPE))
    return FALSE;
  /* Both are variables */
  bvar = old_get_binding(s1,*bindings);
  if (bvar == NIL) {
    b = (Old_Binding *) allocate_memory(sizeof(Old_Binding),MISCELLANEOUS_MEM_USAGE);
    b->from = s1;
    b->to = s2;
    push(b,*bindings);
    return TRUE;
  }
  else if (bvar == s2) {
    return TRUE;
  }
  else return FALSE;
}

/* ----------------------------------------------------------------
   Returns TRUE iff the two tests are identical given a list of bindings.
   Augments the bindings list if necessary.
---------------------------------------------------------------- */

bool old_tests_are_equal_with_bindings (test t1, test t2, list **bindings) {
  cons *c1, *c2;
  complex_test *ct1, *ct2;
  bool goal_test,impasse_test;

  /* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
  if (test_is_blank_test(t1)) return(test_is_blank_test(t2));

  /* If the pattern doesn't include "(state", but the test from the
     production does, strip it out of the production's. */
  if ((!test_includes_goal_or_impasse_id_test(t1,TRUE,FALSE)) &&
      test_includes_goal_or_impasse_id_test(t2,TRUE,FALSE)) {
    goal_test = FALSE;
    impasse_test = FALSE;
    t2 = copy_test_removing_goal_impasse_tests(t2, &goal_test, &impasse_test);
  }

  if (test_is_blank_or_equality_test(t1)) {
    if (!(test_is_blank_or_equality_test(t2) && !(test_is_blank_test(t2)))) return FALSE;
    else return (old_symbols_are_equal_with_bindings(referent_of_equality_test(t1),
                                                 referent_of_equality_test(t2),
                                                 bindings));
  }

  ct1 = complex_test_from_test(t1);
  ct2 = complex_test_from_test(t2);

  if (ct1->type != ct2->type) return FALSE;

  switch(ct1->type) {
  case GOAL_ID_TEST: return TRUE;
  case IMPASSE_ID_TEST: return TRUE;

  case DISJUNCTION_TEST:
    for (c1=ct1->data.disjunction_list, c2=ct2->data.disjunction_list;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->rest, c2=c2->rest)
      if (c1->first != c2->first) return FALSE;
    if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
    return FALSE;

  case CONJUNCTIVE_TEST:
    for (c1=ct1->data.conjunct_list, c2=ct2->data.conjunct_list;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->rest, c2=c2->rest)
      if (! old_tests_are_equal_with_bindings(c1->first,c2->first,bindings)) return FALSE;
    if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
    return FALSE;

  default:  /* relational tests other than equality */
    if (old_symbols_are_equal_with_bindings(ct1->data.referent,ct2->data.referent,bindings)) return TRUE;
    return FALSE;
  }
}
bool old_conditions_are_equal_with_bindings (condition *c1, condition *c2, list **bindings) {
  if (c1->type != c2->type) return FALSE;
  switch (c1->type) {
  case POSITIVE_CONDITION:
  case NEGATIVE_CONDITION:
    if (! old_tests_are_equal_with_bindings (c1->data.tests.id_test,
                           c2->data.tests.id_test,bindings))
      return FALSE;
    if (! old_tests_are_equal_with_bindings (c1->data.tests.attr_test,
                           c2->data.tests.attr_test,bindings))

      return FALSE;
    if (! old_tests_are_equal_with_bindings (c1->data.tests.value_test,
                           c2->data.tests.value_test,bindings))
      return FALSE;
    if (c1->test_for_acceptable_preference !=
        c2->test_for_acceptable_preference)
      return FALSE;
    return TRUE;

  case CONJUNCTIVE_NEGATION_CONDITION:
    for (c1=c1->data.ncc.top, c2=c2->data.ncc.top;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->next, c2=c2->next)
      if (! old_conditions_are_equal_with_bindings (c1,c2,bindings)) return FALSE;
    if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
    return FALSE;
  }
  return FALSE; /* unreachable, but without it, gcc -Wall warns here */
}

/* Routine for LHS. */
void old_read_pattern_and_get_matching_productions (list **current_pf_list, bool show_bindings,
                                                bool just_chunks,bool no_chunks) {
  condition *c, *clist, *top, *bottom, *pc;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  bool match, match_this_c;


  bindings = NIL;
  current_binding_point = NIL;

  print("Parsing as a lhs...\n");
  clist = (condition *) parse_lhs();
  if (!clist) {
    print("Error: not a valid condition list.\n");
    current_pf_list = NIL;
    return;
  }
  print("Valid condition list:\n");
  print_condition_list(clist,0,FALSE);
  print("\nMatches:\n");

  /* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
   */

  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
        (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
     for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL; prod=prod->next) {

      /* Now the complicated part. */
      /* This is basically a full graph-match.  Like the rete.  Yikes! */
      /* Actually it's worse, because there are so many different KINDS of
         conditions (negated, >/<, acc prefs, ...) */
      /* Is there some way I could *USE* the rete for this?  -- for lhs
         positive conditions, possibly.  Put some fake stuff into WM
         (i.e. with make-wme's), see what matches all of them, and then
         yank out the fake stuff.  But that won't work for RHS or for
         negateds.       */

      /* Also note that we need bindings for every production.  Very expensive
         (but don't necc. need to save them -- maybe can just print them as we go). */

      match = TRUE;
      p_node_to_conditions_and_nots (prod->p_node, NIL, NIL, &top, &bottom,
                                     NIL, NIL);

      old_free_binding_list(bindings);
      bindings = NIL;

      for (c=clist;c!=NIL;c=c->next) {
        match_this_c= FALSE;
        current_binding_point = bindings;

        for (pc = top; pc != NIL; pc=pc->next) {
          if (old_conditions_are_equal_with_bindings(c,pc,&bindings)) {
            match_this_c = TRUE;
            break;}
          else {
            /* Remove new, incorrect bindings. */
            old_reset_old_binding_point(&bindings,&current_binding_point);
            bindings= current_binding_point;
          }
        }
        if (!match_this_c) {match = FALSE; break;}
      }
      if (match) {
        push(prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols("%y, with bindings:\n",prod->name);
          old_print_binding_list(bindings);}
        else
          print_with_symbols("%y\n",prod->name);
      }
    }
}


bool old_funcalls_match(list *fc1, list *fc2) {
  /* I have no idea how to do this. */
  return FALSE;
}

bool old_actions_are_equal_with_bindings (action *a1, action *a2, list **bindings) {
  if (a1->type == FUNCALL_ACTION) {
    if ((a2->type == FUNCALL_ACTION)) {
      if (old_funcalls_match(rhs_value_to_funcall_list(a1->value),
                         rhs_value_to_funcall_list(a2->value))) {
        return TRUE;}
      else return FALSE;
    }
    else return FALSE;
  }
  if (a2->type == FUNCALL_ACTION) return FALSE;

  /* Both are make_actions. */

  if (a1->preference_type != a2->preference_type) return FALSE;

  if (!old_symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->id),
                                       rhs_value_to_symbol(a2->id),
                                       bindings)) return FALSE;

  if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr))) {
    if (!old_symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->attr),
					     rhs_value_to_symbol(a2->attr),
					     bindings)) return FALSE;
  } else {
    if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr))) {
      if (!old_funcalls_match(rhs_value_to_funcall_list(a1->attr),
                              rhs_value_to_funcall_list(a2->attr)))
        return FALSE;
    }
  }

  /* Values are different. They are rhs_value's. */

  if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value))) {
    if (old_symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->value),
                                         rhs_value_to_symbol(a2->value),
                                         bindings)) return TRUE;
    else return FALSE;
  }
  if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value))) {
    if (old_funcalls_match(rhs_value_to_funcall_list(a1->value),
                       rhs_value_to_funcall_list(a2->value)))
      return TRUE;
    else return FALSE;}
  return FALSE;
}

/* Routine for RHS. */
void old_read_rhs_pattern_and_get_matching_productions (list **current_pf_list, bool show_bindings,
                                                    bool just_chunks, bool no_chunks) {
  action *a, *alist, *pa;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  bool match, match_this_a, parsed_ok;
  action *rhs; 
  condition *top_cond, *bottom_cond;

  bindings = NIL;
  current_binding_point = NIL;

  print("Parsing as a rhs...\n");
  parsed_ok=parse_rhs(&alist);
  if (!parsed_ok) {
    print("Error: not a valid rhs.\n");
    current_pf_list = NIL;
    return;
  }

  print("Valid RHS:\n");
  print_action_list(alist,0,FALSE);
  print("\nMatches:\n");

  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
        (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
     for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL;
          prod=prod->next) {
      match = TRUE;

      old_free_binding_list(bindings);
      bindings = NIL;

      p_node_to_conditions_and_nots (prod->p_node, NIL, NIL, &top_cond,
                                     &bottom_cond, NIL, &rhs);
      deallocate_condition_list (top_cond);
      for (a=alist;a!=NIL;a=a->next) {
        match_this_a= FALSE;
        current_binding_point = bindings;

        for (pa = rhs; pa != NIL; pa=pa->next) {
          if (old_actions_are_equal_with_bindings(a,pa,&bindings)) {
            match_this_a = TRUE;
            break;}
          else {
            /* Remove new, incorrect bindings. */
            old_reset_old_binding_point(&bindings,&current_binding_point);
            bindings= current_binding_point;
          }
        }
        if (!match_this_a) {match = FALSE; break;}
      }
      deallocate_action_list (rhs);
      if (match) {
        push(prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols("%y, with bindings:\n",prod->name);
          old_print_binding_list(bindings);}
        else
          print_with_symbols("%y\n",prod->name);
      }
    }
}

/* DJP 11/14/94 start */
void apply_command_to_list(char *command, list *current_pf_list)
{
  cons *c;
  production *prod ;

  if (!strcmp(command,"excise"))
    {
       for (c=current_pf_list;c!=NIL;c=c->rest)
         {
            prod = c->first ;
            excise_production(prod,TRUE) ; /* TRUE means print # sign */
         }
    }
}
/* DJP 11/14/94 end */

bool pf_interface_routine (void) {
  bool lhs, rhs, show_bindings, use_command,just_chunks,no_chunks;
  list *current_pf_list;
  char command[MAX_LEXEME_LENGTH+1] ;


  lhs = TRUE; /* default: search lhs. */
  rhs = FALSE;
  show_bindings = FALSE;
  use_command = FALSE ;
  just_chunks = FALSE ;
  no_chunks   = FALSE ;

  get_lexeme();  /* consume command name, advance to optional flags */

  /* --- read optional :lhs, :rhs, or :show-bindings flag --- */
  while (TRUE) {
    if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) break;
    if (!strcmp(current_agent(lexeme).string,":lhs")) {
      get_lexeme();
    }
    if (!strcmp(current_agent(lexeme).string,":rhs")) {
      rhs = TRUE;
      lhs = FALSE;
      get_lexeme();
      continue;
    }
    if (!strcmp(current_agent(lexeme).string,":show-bindings")) {
      show_bindings = TRUE;
      get_lexeme();
      continue;
    }
    if (!strcmp(current_agent(lexeme).string,":chunks")) {
       just_chunks = TRUE ;
       get_lexeme() ;
       continue ;
    }
    if (!strcmp(current_agent(lexeme).string,":nochunks")) {
       no_chunks = TRUE ;
       get_lexeme() ;
       continue ;
    }
    /* DJP 11/14/94 start */
    if (!strcmp(current_agent(lexeme).string,":command")) {
       get_lexeme() ; /* Consume :command */
       if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME)
         {
            if (!strcmp(current_agent(lexeme).string,"excise"))
              {
                 use_command = TRUE ;
                 strcpy(command,current_agent(lexeme).string) ;
                 get_lexeme() ;
              }
         }
       if (!use_command) /* Didn't find a valid command */
         {
            print("Command for pf to apply not recognized\n") ;
            print_location_of_most_recent_lexeme();
            return FALSE;
         }
       continue;
    }
    /* DJP 11/14/94 end */

    break;
  }

  current_pf_list = NIL;

  switch (current_agent(lexeme).type) {

  case L_PAREN_LEXEME:
  case MINUS_LEXEME:
    if (lhs)
      old_read_pattern_and_get_matching_productions (&current_pf_list,show_bindings,just_chunks,no_chunks);
    if (rhs)
      old_read_rhs_pattern_and_get_matching_productions (&current_pf_list,show_bindings,just_chunks,no_chunks);
    if (current_pf_list == NIL) print("No matches.\n");
    else if (use_command) apply_command_to_list(command,current_pf_list) ; /* DJP 11/14/94
*/
    free_list(current_pf_list);
    break;
  default:
    print ("Illegal argument to 'pf' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } /* end of switch statement */
  return TRUE;
}

/* AGR DJP1 end */


/* AGR 649 begin */
/* -------------------------------------------------------------------

                         "Memories" Command

   Syntax:  see help screen below.
------------------------------------------------------------------- */
/* "Memories" command-line function.
   Written 1/5/94
   Author: Scott Huffman
*/

char *help_on_memories[] = {
"Command: memories",
"",
"Syntax: (memories production-name)",
"        (memories arg* [count])",
"        arg  ::=  :chunk | :user | :default | :justification",
"",
"This command prints information about memory use, in tokens, of partial",
"matches of productions.  If a production-name is given, memory use for",
"that production is printed.  If no production name is given, memories",
"will list 'count' productions of the specified type (or all types,",
"if no type is specified).  If 'count' is omitted, memory use of",
"all productions is printed.",
0 };

bool memories_interface_routine (void) {
  Symbol *sym;
  void old_print_memories();
  int mems_to_print[NUM_PRODUCTION_TYPES], i;
  bool set_mems;
  
  set_lexer_allow_ids (FALSE);
  get_lexeme();  /* consume "matches", advance */

  for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
    mems_to_print[i] = FALSE;
  set_mems = FALSE;
  
  /* Production name: */
  while (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    if (!strcmp(current_agent(lexeme).string, ":chunk")) {
      mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
      set_mems = TRUE;
      get_lexeme();}
    else if (!strcmp(current_agent(lexeme).string, ":user")) {
      mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
      set_mems = TRUE;
      get_lexeme();}
    else if (!strcmp(current_agent(lexeme).string, ":default")) {
      mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
      set_mems = TRUE;
      get_lexeme();}
    else if (!strcmp(current_agent(lexeme).string, ":justification")) {
      mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
      set_mems = TRUE;
      get_lexeme();}
    else {
      sym = find_sym_constant (current_agent(lexeme).string);
      if ((!sym) || (! sym->sc.production)) {
	print ("No production named %s\n", current_agent(lexeme).string);
	print_location_of_most_recent_lexeme();
	return FALSE;
      }
      print("\n Memory use for %s: %d\n\n", current_agent(lexeme).string,
	    count_rete_tokens_for_production(sym->sc.production));
      get_lexeme(); /* consume production name */
      return TRUE;}
  }

  if (!set_mems) {
    mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
    mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
    mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
    mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
  }

  /* Integer argument: */
  if ((current_agent(lexeme).type==INT_CONSTANT_LEXEME) &&
      (current_agent(lexeme).int_val > 0)) {
    old_print_memories(current_agent(lexeme).int_val, mems_to_print);
    get_lexeme(); /* consume number */
    return TRUE;}

  /* No argument: */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    old_print_memories(20, mems_to_print);
    return TRUE;}

  /* Bad argument: */
  else {
    print ("Unexpected parameters passed to Memories.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;}
}

typedef struct old_production_memory_use_struct {
  Symbol *name;
  int mem;
  struct old_production_memory_use_struct *next;
} old_production_memory_use;

void old_print_memories (int num_to_print, int to_print[]) {
  int i, num_prods;
  old_production_memory_use *temp, *first, *tempnext;
  production *prod;
  old_production_memory_use *old_print_memories_insert_in_list();

  print("\nMemory use for productions:\n\n");

  /* Start by doing ALL of them. */
  first = NULL;
  num_prods = 0;

  for (i=0; i < NUM_PRODUCTION_TYPES; i++)
    if (to_print[i]) 
      for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL;
           prod=prod->next) {
	 temp = allocate_memory (sizeof (old_production_memory_use),
				 MISCELLANEOUS_MEM_USAGE);
	 temp->next = NULL;
	 temp->name = prod->name;

	 if (prod->p_node)
	   temp->mem = count_rete_tokens_for_production(prod);
	 else temp->mem = 0;

	 first = old_print_memories_insert_in_list(temp,first);
	 num_prods++;
        }

  i = 0;
  if (num_to_print < 0) num_to_print = num_prods;

  for (temp = first; ((temp != NULL) && (i < num_to_print)); temp = tempnext) {
    print_with_symbols("%y: ", temp->name);
    print("%d\n",temp->mem);
    tempnext = temp->next;
    free_memory(temp,MISCELLANEOUS_MEM_USAGE);
    i++;
   }
}

old_production_memory_use *old_print_memories_insert_in_list(old_production_memory_use *new,
						     old_production_memory_use *list) {
  old_production_memory_use *ctr, *prev;

  /* Add to beginning. */
  if ((list == NULL) || (new->mem >= list->mem)) {
    new->next = list;
    return new;}

  /* Add to middle. */
  prev = list;
  for (ctr = list->next; ctr != NULL; ctr = ctr->next) {
    if (new->mem >= ctr->mem) {
      prev->next = new;
      new->next = ctr;
      return list;
    }
    prev = ctr;
  }

  /* Add to end. */
  prev->next = new;
  new->next = NULL;
  return list;
}

/* AGR 649 end */


/* -------------------------------------------------------------------
   
                          "D" Command

   Syntax:  (d [integer])
------------------------------------------------------------------- */

char *help_on_d[] = {
"Command: d",
"",
"Syntax: (d [integer])",
"",
"With an integer argument, this command runs Soar for that number of decision",
"cycles.  With no arguments, it runs Soar forever (or until Soar halts,",
"receives an interrupt, etc.).",
"",
"See also:  run, go",
0 };

bool d_interface_routine (void) {
  long num_requested;

  get_lexeme();  /* consume "d" */
  if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
    num_requested = current_agent(lexeme).int_val;
    get_lexeme(); /* consume the integer */
  } else if (current_agent(lexeme).type==R_PAREN_LEXEME) { 
    num_requested = -1;
  } else {
    print ("Bad argument for 'd' command.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  run_for_n_decision_cycles (num_requested);
  print ("\n");
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Run" and "R" Commands

   Syntax:  (run [integer])
            (r ...) is shorthand for (run ...)
------------------------------------------------------------------- */

char *help_on_run[] = {
"Commands: run, r",
"",
"Syntax: (run [integer])",
"        (r ...) is shorthand for (run ...)",
"",
"With an integer argument, this command runs Soar for that number of",
"elaboration cycles.  (For this command, decision phase is counted as an",
"elaboration cycle.)  With no arguments, this runs Soar forever (or until",
"Soar halts, receives an interrupt, etc.).",
"",
"See also:  d, go",
0 };

bool run_interface_routine (void) {
  long num_requested;

  get_lexeme();  /* consume "run" */
  if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
    num_requested = current_agent(lexeme).int_val;
    get_lexeme(); /* consume the integer */
  } else if (current_agent(lexeme).type==R_PAREN_LEXEME) { 
    num_requested = -1;
  } else {
    print ("Bad argument for 'run' command.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  run_for_n_elaboration_cycles (num_requested);
  print ("\n");
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Go" Command

   Syntax:  (go [integer | 'forever'] [type])
            type ::= p | e | d | s | o | context-variable
------------------------------------------------------------------- */

char *help_on_go[] = {
"Command: go",
"",
"Syntax: (go [integer | 'forever'] [type])",
"        type ::= 'p' | 'e' | 'd' | 's' | 'o' | context-variable",
"",
"This is the most general command for running Soar.  It takes two optional",
"arguments, one specifying how many things to run, and one specifying what",
"type of things to run.  The following types are available:",
"",
"p - run Soar for n phases.  A phase is either an input phase, preference",
"    phase, working memory phase, output phase, or decision phase.",
"e - run Soar for n elaboration cycles.  (For purposes of this command,",
"    decision phase is counted as an elaboration cycle.)",
"d - run Soar for n decision cycles.",
"s - run Soar until the nth time a state is selected.",
"o - run Soar until the nth time an operator is selected.",
"context-variable - run Soar until the nth time a selection is made for that",
"    particular context slot, or until the context stack pops to above that",
"    context.",
"",
"Go remembers each argument you give it each time.  If you don't give",
"arguments next time, it uses the ones from the previous time.",
"",
"Examples:",
"  (go 5 d)  --> run for 5 decision cycles",
"  (go e)    --> run for another 5 elaboration cycles",
"  (go 1 s)  --> run until the next state is selected (i.e., until the next",
"                time an impasse arises)",
"  (go <so>) --> run until the next superoperator is selected (or until the",
"                supergoal goes away)",
"  (go 3 <o>) --> run for 3 operator selections at this level (continuing",
"                 through any subgoals that arise)",
"",
"See also:  d, run",
0 };


bool old_parse_go_command (void) {
  Symbol *g, *attr, *value;
  
  get_lexeme();  /* consume "go" */
  while (TRUE) {
    if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
      current_agent(go_number) = current_agent(lexeme).int_val;
      get_lexeme();
      continue;
    }
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,"forever")) {
        current_agent(go_number) = -1;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"p")) {
        current_agent(go_type) = GO_PHASE;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"e")) {
        current_agent(go_type) = GO_ELABORATION;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"d")) {
        current_agent(go_type) = GO_DECISION;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"s")) {
        current_agent(go_type) = GO_STATE;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"o")) {
        current_agent(go_type) = GO_OPERATOR;
        get_lexeme();
        continue;
      }
    }
    if (current_agent(lexeme).type==VARIABLE_LEXEME) {
      old_get_context_var_info (&g, &attr, &value);
      if (!attr) {
        print ("Expected a context variable.\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      if (!g) {
        print ("That goal stack level doesn't exist right now.\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      current_agent(go_type) = GO_SLOT;
      current_agent(go_slot_level) = g->id.level;
      current_agent(go_slot_attr) = attr;
      get_lexeme();
      continue;
    }
    break; /* if it didn't match anything so far, break out of the loop */
  } /* end of while (TRUE) */

  if (current_agent(lexeme).type != R_PAREN_LEXEME) {
    print ("Bad argument for 'go' command.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }

  return TRUE;
}


void
old_execute_go_selection (void)
{
  switch (current_agent(go_type)) {
  case GO_PHASE:
    run_for_n_phases (current_agent(go_number));
    break;
  case GO_ELABORATION:
    run_for_n_elaboration_cycles (current_agent(go_number));
    break;
  case GO_DECISION:
    run_for_n_decision_cycles (current_agent(go_number));
    break;
  case GO_STATE:
    run_for_n_selections_of_slot (current_agent(go_number), current_agent(state_symbol));
    break;
  case GO_OPERATOR:
    run_for_n_selections_of_slot (current_agent(go_number), current_agent(operator_symbol));
    break;
  case GO_SLOT:
    run_for_n_selections_of_slot_at_level (current_agent(go_number), current_agent(go_slot_attr),
                                           current_agent(go_slot_level));
    break;
  }
}


bool go_interface_routine (void) {
  bool parse_result;

  parse_result = old_parse_go_command();
  if (parse_result == TRUE) {
    old_execute_go_selection();
  }
  print ("\n");
  return parse_result;
}


/* -------------------------------------------------------------------
   
                          "Init-Soar" Command

   Syntax:  (init-soar)
------------------------------------------------------------------- */

char *help_on_init_soar[] = {
"Command: init-soar",
"",
"Syntax: (init-soar)",
"",
"This command re-initializes Soar.  It removes all wmes from working memory,",
"wiping out the goal stack, and resets all statistics (except the counts of",
"how many times each individual production has fired, used by the",
"\"firing-counts\" command).",
0 };

bool init_soar_interface_routine (void) {
  reinitialize_soar();
  get_lexeme();  /* consume "init-soar", advance to rparen */
  return TRUE;
}


/* MVP 6-8-94 */
/* -------------------------------------------------------------------

			"Setvar" Command

			Syntax:  (setvar)
------------------------------------------------------------------- */
char *help_on_setvar[] = {
"Command: setvar",
"",
"Syntax: (setvar [var])",
"",
"This command sets a variable's Logical Status to TRUE.",
"With no argument, it prints a list of variables which",
"are currently set to TRUE.",
0 };
  
bool setvar_interface_routine (void) {
  char *the_string;
  cons *c;

  get_lexeme(); /* consume "setvar" */

  if (current_agent(lexeme).type == R_PAREN_LEXEME) {
    print ("Variables that are Set:\n");
    for (c=current_agent(variables_set); c!=NIL; c=c->rest)
      print ("%s\n", c->first);
  } else if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME ||
              !strcmp (current_agent(lexeme).string, "*")) {
    print ("Expected a variable for setvar\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else {
    the_string = make_memory_block_for_string (current_agent(lexeme).string);
    for (c=current_agent(variables_set); c!=NIL; c=c->rest) {
      if (!strcmp (current_agent(lexeme).string, c->first))
        break;
    }
    if (!c)
      push (the_string, current_agent(variables_set));
    print ("Variable %s is Set\n", current_agent(lexeme).string);
    get_lexeme();
  }
  return TRUE;
}

/* MVP 6-8-94 */
/* -------------------------------------------------------------------
  
			"UnSetvar" Command
  
			Syntax:  (unsetvar {var || *})
------------------------------------------------------------------- */
  
char *help_on_unsetvar[] = {
"Command: unsetvar",
"",
"Syntax: (unsetvar {var || *})",
"",
"This command sets a variable's Logical Status to FALSE.",
0 };
  
bool remove_setvar_fn (cons *c) {
  if (!strcmp (current_agent(lexeme).string, c->first))
    return TRUE;
  else
    return FALSE;
}
  
bool unsetvar_interface_routine (void) {
  list *extracted_stuff;
  
  get_lexeme(); /* consume "unsetvar" */
  if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
    print ("Expected a variable for unsetvar\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else if (!strcmp (current_agent(lexeme).string, "*")) {
    free_list (current_agent(variables_set));
    current_agent(variables_set) = NIL;
    print ("All variables are now UnSet\n");
    get_lexeme ();
    return TRUE;
  } else {
    print ("Variable %s is UnSet\n", current_agent(lexeme).string);
    extracted_stuff = extract_list_elements (&current_agent(variables_set),
                                             remove_setvar_fn);
    free_list (extracted_stuff);
    get_lexeme ();
    return TRUE;
  }
}

/* MVP 6-8-94 */
/* -------------------------------------------------------------------
  
			"If" Command
  
			Syntax:  (if var (action))
------------------------------------------------------------------- */
char *help_on_if[] = {
"Command: if",
"",
"Syntax: (if var (action))",
"",
0 };
  
bool if_interface_routine (void) {
  bool the_result;
  cons *c;
  
  get_lexeme(); /* consume "if" */
  if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
    print ("Expected a variable for if\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else {
    the_result = FALSE;
    for (c=current_agent(variables_set); c!=NIL; c=c->rest) {
      if (!strcmp (current_agent(lexeme).string, c->first)) {
        the_result = TRUE;
        break;
      }
    }
    if (!the_result)
      return FALSE;
  }
/* begin test for AND and more variables */
  while (TRUE) {
    get_lexeme();  /* consume variable */
    if ((current_agent(lexeme).type == SYM_CONSTANT_LEXEME) &&
	(!strcmp (current_agent(lexeme).string, "and"))) {
      get_lexeme();  /* consume the "and" */
      the_result = FALSE;
      for (c=current_agent(variables_set); c!=NIL; c=c->rest) {
	if (!strcmp (current_agent(lexeme).string, c->first)) {
	  the_result = TRUE;
	  break;
	}
      }
      if (!the_result) return FALSE;
    } else break;
  }
/* end test for AND and more variables */
  if (current_agent(lexeme).type == R_PAREN_LEXEME) {
    print ("Expected an action for if\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else if (current_agent(lexeme).type == L_PAREN_LEXEME) {
    get_lexeme ();
    if (current_agent(lexeme).type == R_PAREN_LEXEME) {
      current_agent(current_file)->fake_rparen_at_eol = FALSE;
      get_lexeme ();
      dispatch_command ();
    } else {
      current_agent(current_file)->parentheses_level--;
      dispatch_command ();
      get_lexeme ();
    }
  } else
    dispatch_command ();
  
  return TRUE;
}

/* MVP 6-8-94 */
/* -------------------------------------------------------------------

			"If-Not" Command

			Syntax:  (if-not var (action))
------------------------------------------------------------------- */

char *help_on_if_not[] = {
"Command: if-not",
"",
"Syntax: (if-not var (action))",
"",
0 };

bool if_not_interface_routine (void) {
  cons *c;

  get_lexeme(); /* consume "if-not" */
  if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
    print ("Expected a variable for if-not\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else {
    for (c=current_agent(variables_set); c!=NIL; c=c->rest) {
      if (!strcmp (current_agent(lexeme).string, c->first))
        return FALSE;
    }
  }
/* begin test for AND and more variables */
/* if-not a and b and c  <->  if (not a) and (not b) and (not c) */
  while (TRUE) {
    get_lexeme();  /* consume variable */
    if ((current_agent(lexeme).type == SYM_CONSTANT_LEXEME) &&
	(!strcmp (current_agent(lexeme).string, "and"))) {
      get_lexeme();  /* consume the "and" */
      for (c=current_agent(variables_set); c!=NIL; c=c->rest) {
	if (!strcmp (current_agent(lexeme).string, c->first))
	  return FALSE;
      }
    } else break;
  }
/* end test for AND and more variables */
  if (current_agent(lexeme).type == R_PAREN_LEXEME) {
    print ("Expected an action for if-not\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  } else if (current_agent(lexeme).type == L_PAREN_LEXEME) {
    get_lexeme ();
    if (current_agent(lexeme).type == R_PAREN_LEXEME) {
      current_agent(current_file)->fake_rparen_at_eol = FALSE;
      get_lexeme ();
      dispatch_command ();
    } else {
      current_agent(current_file)->parentheses_level--;
      dispatch_command ();
      get_lexeme ();
    }
  } else
    dispatch_command ();

  return TRUE;
}

/* -------------------------------------------------------------------
   
                            "Learn" Command

   Syntax:  (learn arg* )
            arg  ::=  on | specify | off
            arg  ::=  all-goals | bottom-up
            arg  ::=  noprint | print | full-print
            arg  ::=  notrace | trace | full-trace
------------------------------------------------------------------- */

char *help_on_learn[] = {
"Command: learn",
"",
"Syntax: (learn arg* )",
"        arg  ::=  on | specify | off",  /* AGR MVL1 */
"        arg  ::=  all-goals | bottom-up",
"        arg  ::=  noprint | print | full-print",
"        arg  ::=  notrace | trace | full-trace",
"",
"With no arguments, this command prints out the current learning status.",
"Any of the following arguments may be given:",
"   on         - turns learning on",
"   specify    - learns in chunky problem spaces",  /* AGR MVL1 */
"   off        - turns all learning off",
"   all-goals  - when learning is on, this allows learning at all goal stack",
"                levels (in contrast to bottom-up learning)",
"   bottom-up  - when learning is on, this allows learning at only the lowest",
"                goal stack level; i.e., a chunk is learned at a given level",
"                only if no chunk has yet been learned at a lower level.",
"   noprint    - equivalent to (watch :chunk-names off :chunks off)",
"   print      - equivalent to (watch :chunk-names  on :chunks off)",
"   full-print - equivalent to (watch :chunk-names  on :chunks  on", 
"                                     :justification-names  on :justifications  on)",
"   notrace    - equivalent to (watch :firings chunk off :backtracing off)",
"   trace      - equivalent to (watch :firings chunk  on :backtracing off)",
"   full-trace - equivalent to (watch :firings chunk  on :backtracing  on",
"                                     :chunk-names  on :chunks  on", 
"                                     :justification-names  on :justifications  on)",
"",
"See also: chunk-free-problem-spaces, watch",
0 };

bool learn_interface_routine (void) {
  get_lexeme();  /* consume "learn" */
  
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Current learn settings:\n");
/* AGR MVL1 begin */
    if ((! current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) &&
	(! current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM]))
      print ("   %s\n", current_agent(sysparams)[LEARNING_ON_SYSPARAM] ? "on" : "off");
    else
      print ("   %s\n", current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] ? "specify" : "except");

/* AGR MVL1 end */
    print ("   %s\n", current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] ? "all-goals" : "bottom-up");

    if (current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] && 
        current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] &&
        current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] && 
        current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM])
      print ("   full-print\n");
    else if (!current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] && 
             !current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM])
      print ("   noprint\n");
    else if (current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] && 
             !current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM])
      print ("   print\n");

    if (!current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] && 
             !current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM])
      print ("   notrace\n");
    else if (current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] && 
             !current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM])
      print ("   trace\n");
    else if (current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] &&
             current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM] &&
             current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] &&
             current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] &&
             current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] &&
             current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM])
      print ("   full-trace\n");
    return TRUE;
  }

  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,"on")) {
	set_sysparam (LEARNING_ON_SYSPARAM, TRUE); 
	set_sysparam (LEARNING_ONLY_SYSPARAM, FALSE);  /* AGR MVL1 */
	set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
	set_sysparam (TRACE_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	set_sysparam (TRACE_BACKTRACING_SYSPARAM, FALSE);
	get_lexeme();
	continue;
      }
/* AGR MVL1 begin */
      if (!strcmp(current_agent(lexeme).string,"specify")) {
	set_sysparam (LEARNING_ON_SYSPARAM, TRUE); 
	set_sysparam (LEARNING_ONLY_SYSPARAM, TRUE);
	set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
	set_sysparam (TRACE_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	set_sysparam (TRACE_BACKTRACING_SYSPARAM, FALSE);
	get_lexeme();
	continue;
      }
/* AGR MVL1 end */
      if (!strcmp(current_agent(lexeme).string,"off")) {
	set_sysparam (LEARNING_ON_SYSPARAM, FALSE);
	set_sysparam (LEARNING_ONLY_SYSPARAM, FALSE);  /* AGR MVL1 */
	set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, FALSE);
	set_sysparam (TRACE_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (TRACE_BACKTRACING_SYSPARAM, FALSE);
	get_lexeme();
	continue;
      }
      if (!strcmp(current_agent(lexeme).string,"all-goals")) {
        set_sysparam (LEARNING_ALL_GOALS_SYSPARAM, TRUE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"bottom-up")) {
        set_sysparam (LEARNING_ALL_GOALS_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"noprint")) {
        set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, FALSE);
        set_sysparam (TRACE_CHUNKS_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"print")) {
        set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
        set_sysparam (TRACE_CHUNKS_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"full-print")) {
        set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
        set_sysparam (TRACE_CHUNKS_SYSPARAM, TRUE);
        set_sysparam (TRACE_JUSTIFICATION_NAMES_SYSPARAM, TRUE);
        set_sysparam (TRACE_JUSTIFICATIONS_SYSPARAM, TRUE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"notrace")) {
        set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
        set_sysparam (TRACE_BACKTRACING_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"trace")) {
        set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
        set_sysparam (TRACE_BACKTRACING_SYSPARAM, FALSE);
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"full-trace")) {
        set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
        set_sysparam (TRACE_BACKTRACING_SYSPARAM, TRUE);
        set_sysparam (TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
        set_sysparam (TRACE_CHUNKS_SYSPARAM, TRUE);
        set_sysparam (TRACE_JUSTIFICATION_NAMES_SYSPARAM, TRUE);
        set_sysparam (TRACE_JUSTIFICATIONS_SYSPARAM, TRUE);
        get_lexeme();
        continue;
      }
    }
    print ("Error: unrecognized argument to 'learn' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

#ifdef ATTENTION_LAPSE
/* RMJ */

/* -------------------------------------------------------------------
   
                            "attention-lapse" Command

   Syntax:  (attention-lapse arg* )
            arg  ::=  on | off
------------------------------------------------------------------- */

char *help_on_attention_lapse[] = {
"Command: attention-lapse",
"",
"Syntax: (learn arg* )",
"        arg  ::=  on | off",
"",
"With no arguments, this command prints out the current attentional",
"lapsing status.  Any of the following arguments may be given:",
"   on         - turns attentional lapsing on",
"   off        - turns attentional lapsing off",
"",
"See also:",
0 };

bool attention_lapse_interface_routine (void) {
  get_lexeme();  /* consume "attention-lapse" */
  
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Current attention-lapse setting:\n");
    print ("   %s\n", current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] ? "on" : "off");
    return TRUE;
  }

  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,"on")) {
	set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, TRUE); 
	get_lexeme();
        wake_from_attention_lapse();
	continue;
      }
      if (!strcmp(current_agent(lexeme).string,"off")) {
	set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, FALSE);
	get_lexeme();
	continue;
      }
    }
    print ("Error: unrecognized argument to 'learn' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

#endif  /* ATTENTION_LAPSE */

/* -------------------------------------------------------------------
   
                    "Chunk-free-problem-spaces" Command

   Syntax:  (chunk-free-problem-spaces)
            (chunk-free-problem-spaces :add space-name)
            (chunk-free-problem-spaces :remove space-name)
------------------------------------------------------------------- */

char *help_on_chunk_free_problem_spaces[] = {
"Command: chunk-free-problem-spaces",
"",
"Syntax: (chunk-free-problem-spaces)",
"        (chunk-free-problem-spaces :add space-name)",
"        (chunk-free-problem-spaces :remove space-name)",
"",
"With no arguments, this command prints the current list of problem spaces",
"declared chunk-free.  With arguments, it adds or removes a given problem",
"space from this list.  No chunks will be built in a problem space if that",
"space is declared chunk-free.",
"",
"See also: learn",
0 };

Symbol *old_space_to_remove_from_cfps;

bool old_cfps_removal_test_function (cons *c) {
  return (c->first == old_space_to_remove_from_cfps);
}

bool chunk_free_problem_spaces_interface_routine (void) {
  cons *c;
  Symbol *sym;
  list *extracted_stuff;
  
  get_lexeme();  /* consume "chunk-free-problem-spaces" */
  
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Problem spaces declared chunk-free:\n");
    for (c=current_agent(chunk_free_problem_spaces); c!=NIL; c=c->rest)
      print_with_symbols ("  %y\n", (Symbol *)(c->first));
    return TRUE;
  }

  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    if (!strcmp(current_agent(lexeme).string,":add")) {
      get_lexeme();  /* consume ":add", advance to the space name */
      if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
        print ("Error: expected a symbol for the problem space name\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      sym = make_sym_constant (current_agent(lexeme).string);
      if (! member_of_list (sym, current_agent(chunk_free_problem_spaces))) {
        symbol_add_ref (sym);
        push (sym, current_agent(chunk_free_problem_spaces));
      }
      symbol_remove_ref (sym);
      get_lexeme();
      return TRUE;
    }
    if (!strcmp(current_agent(lexeme).string,":remove")) {
      get_lexeme();  /* consume ":remove", advance to the space name */
      if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
        print ("Error: expected a symbol for the problem space name\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      sym = find_sym_constant (current_agent(lexeme).string);
      if (sym) {
        old_space_to_remove_from_cfps = sym;
        extracted_stuff = extract_list_elements (&current_agent(chunk_free_problem_spaces),
                                                 old_cfps_removal_test_function);
        deallocate_symbol_list_removing_references (extracted_stuff);
      }
      get_lexeme();
      return TRUE;
    }
  }
  print ("Error: unrecognized argument to 'chunk-free-problem-spaces'\n");
  print_location_of_most_recent_lexeme();
  return FALSE;
}

/* AGR MVL1 begin */
/* -------------------------------------------------------------------
   
                    "Chunky-problem-spaces" Command

   Syntax:  (chunky-problem-spaces)
            (chunky-problem-spaces :add space-name)
            (chunky-problem-spaces :remove space-name)
------------------------------------------------------------------- */

char *help_on_chunky_problem_spaces[] = {
"Command: chunky-problem-spaces",
"",
"Syntax: (chunky-problem-spaces)",
"        (chunky-problem-spaces :add space-name)",
"        (chunky-problem-spaces :remove space-name)",
"",
"With no arguments, this command prints the current list of problem spaces",
"declared chunky.  With arguments, it adds or removes a given problem",
"space from this list.  Chunks will only be built in a problem space if that",
"space is declared chunky (as long as learn is set to specify).",
"",
"See also: learn",
0 };

Symbol *space_to_remove_from_chunkyps;

bool chunkyps_removal_test_function (cons *c) {
  return (c->first == space_to_remove_from_chunkyps);
}

bool chunky_problem_spaces_interface_routine (void) {
  cons *c;
  Symbol *sym;
  list *extracted_stuff;
  
  get_lexeme();  /* consume "chunky-problem-spaces" */
  
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Problem spaces declared chunky:\n");
    for (c=current_agent(chunky_problem_spaces); c!=NIL; c=c->rest)
      print_with_symbols ("  %y\n", (Symbol *)(c->first));
    return TRUE;
  }

  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    if (!strcmp(current_agent(lexeme).string,":add")) {
      get_lexeme();  /* consume ":add", advance to the space name */
      if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
        print ("Error: expected a symbol for the problem space name\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      sym = make_sym_constant (current_agent(lexeme).string);
      if (! member_of_list (sym, current_agent(chunky_problem_spaces))) {
        symbol_add_ref (sym);
        push (sym, current_agent(chunky_problem_spaces));
      }
      symbol_remove_ref (sym);
      get_lexeme();
      return TRUE;
    }
    if (!strcmp(current_agent(lexeme).string,":remove")) {
      get_lexeme();  /* consume ":remove", advance to the space name */
      if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
        print ("Error: expected a symbol for the problem space name\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      sym = find_sym_constant (current_agent(lexeme).string);
      if (sym) {
        space_to_remove_from_chunkyps = sym;
        extracted_stuff = extract_list_elements (&current_agent(chunky_problem_spaces),
                                                 chunkyps_removal_test_function);
        deallocate_symbol_list_removing_references (extracted_stuff);
      }
      get_lexeme();
      return TRUE;
    }
  }
  print ("Error: unrecognized argument to 'chunky-problem-spaces'\n");
  print_location_of_most_recent_lexeme();
  return FALSE;
}
/* AGR MVL1 end */

/* -------------------------------------------------------------------
   
                          "Watch" Command

   Syntax:  see helpscreen below
------------------------------------------------------------------- */

char *help_on_watch[] = {
"Command: watch",
"",
"Syntax: (watch arg*)",
"        arg  ::=  -1 | 0 | 0.5 | 1 | 1.5 | 2 | 3 | task",
"        arg  ::=  :context {on|off}", 
"        arg  ::=  :phases {on|off}", 
"        arg  ::=  :firings [default|user|chunk|nonchunk|all] {on|off}",
"        arg  ::=     :firings-wmes {0|1|2}",
"        arg  ::=     :firings-preferences {on|off}",
"        arg  ::=  :wmes {on|off}",
"        arg  ::=  :chunk-names {on|off}",
"        arg  ::=  :justification-names {on|off}",
"        arg  ::=  :chunks {on|off}",
"        arg  ::=  :justifications {on|off}",
"        arg  ::=  :backtracing {on|off}",
"",
"This command controls what information gets printed in the run-time trace.",
"With no arguments, it just prints out the current watch status.  The numeric",
"arguments have roughly the same semantics as in Soar 5; for details, see",
"(help watch-levels).  The various :keyword arguments are used to modify the",
"current watch settings.  For example, (watch :context on) turns on the",
"tracing of context slot decisions; (watch :context off) turns it off again.",
"For information about what each keyword does, see (help watch-keywords).",
"",
"See also:  watch-keywords, watch-levels, learn, pwatch",
0 };

char *help_on_watch_keywords[] = {
"The following keyword arguments may be given to the 'watch' command:",
"",
"  :context -- controls whether context slot decisions are printed ",
"  :phases -- controls whether phase names are printed",
"  :firings [default|user|chunk|nonchunk|all] -- controls which production",
"      firings and retractions are printed.  The optional argument (which",
"      defaults to 'all') specifies which types of productions the 'on/off'",
"      argument refers to.",
"  :firings-wmes -- controls the level of detail given about the wmes matched",
"      by productions whose firings and retractions are being traced.  Level",
"      0 means no information about the wmes is printed.  Level 1 means the",
"      wme timetags are printed.  Level 2 means the whole wmes are printed.",
"  :firings-preferences -- controls whether the preferences generated by",
"      the traced productions are printed when those productions fire or",
"      retract.  When a production fires, all the preferences it generates",
"      are printed.  When it retracts, only the ones being removed from",
"      preference memory are printed (i.e., the i-supported ones).",
"  :wmes -- controls whether changes to working memory are printed",
"  :chunk-names -- controls whether names of newly built chunks are printed",
"  :justification-names -- ditto, only for justifications (internal chunks)",
"  :chunks -- controls whether newly built chunks are printed",
"  :justifications -- ditto, only for justifications",
"  :backtracing -- controls whether backtracing information is printed",
"",
"These keyword arguments are provided so you can have more complete control",
"over what gets traced.  The numeric arguments to 'watch', as well as the",
"'print' and 'trace' arguments to 'learn', provide simple ways of getting",
"some useful watch settings.  See (help watch-levels) for details.",
"",
"See also:  watch, watch-levels, learn",
0 };

char *help_on_watch_levels[] = {
"The watch levels (-1, 0, 0.5, 1, 1.5, 2, 3) in Soar 6 have roughly the same",
"semantics as in Soar 5.  The table below gives the corresponding keyword",
"parameter settings for each level.",
"",
"            Watch Level:    -1    0  0.5    1  1.5    2    3",
"                          ---- ---- ---- ---- ---- ---- ----",
"  :context                 off   on   on   on   on   on   on",
"  :phases                  off  off   on   on   on   on   on",
"  :firings nonchunk        off  off   on   on   on   on   on",
"    :firings-wmes            0    0    0    1    2    2    2",
"    :firings-preferences   off  off  off  off  off  off   on",
"  :wmes                    off  off  off  off  off   on   on",
"",
"(watch task) is also provided in Soar 6 for Soar 5 compatibility, and is",
"equivalent to (watch :firings default off).",
"",
"Learn noprint/print/full-print and notrace/trace/full-trace also translate",
"into keyword parameter settings, as shown in the tables below:",
"",
"         Learn:   noprint  print  full-print",
"                  -------  -----  ----------",
"  :chunk-names        off     on          on",
"  :chunks             off    off          on",
"",
"         Learn:   notrace  trace  full-trace",
"                  -------  -----  ----------",
"  :firings chunk      off     on          on",
"  :backtracing        off    off          on",
"",
"See also:  watch, watch-keywords, learn",
0 };

bool old_set_watch_setting (int dest_sysparam_number) {
  get_lexeme(); /* consume :keyword name */
  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    if (!strcmp(current_agent(lexeme).string,"on")) {
      set_sysparam (dest_sysparam_number, TRUE);
      get_lexeme(); /* consume on/off flag */
      return TRUE;
    }
    if (!strcmp(current_agent(lexeme).string,"off")) {
      set_sysparam (dest_sysparam_number, FALSE);
      get_lexeme(); /* consume on/off flag */
      return TRUE;
    }
  }
  print ("Expected 'on' or 'off' for new watch setting\n");
  print_location_of_most_recent_lexeme();
  return FALSE;
}

bool watch_interface_routine (void) {
  get_lexeme();  /* consume "watch" */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print ("Current watch settings:\n");
    print ("  Context decisions:  %s\n",
           current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
    print ("  Phases:  %s\n",
           current_agent(sysparams)[TRACE_PHASES_SYSPARAM] ? "on" : "off");
    print ("  Firings/retractions\n");
    print ("    default productions:  %s\n",
           current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
    print ("    user productions:  %s\n",
           current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
    print ("    chunks:  %s\n",
           current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
    print ("    justifications:  %s\n",
           current_agent(sysparams)[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
    print ("  WME detail level:  %d\n",
           current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
    print ("  Preferences generated by firings/retractions:  %s\n",
           current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
    print ("  Working memory changes:  %s\n",
           current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
    print ("  Chunk names:  %s\n",
           current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "on" : "off");
    print ("  Justification names:  %s\n",
           current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] ? "on" : "off");
    print ("  Chunks:  %s\n",
           current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] ? "on" : "off");
    print ("  Justifications:  %s\n",
           current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
    print ("  Backtracing:  %s\n",
           current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");
    return TRUE;
  }
  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (!strcmp(current_agent(lexeme).string,":context")) {
        if (old_set_watch_setting (TRACE_CONTEXT_DECISIONS_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":phases")) {
        if (old_set_watch_setting (TRACE_PHASES_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":wmes")) {
        if (old_set_watch_setting (TRACE_WM_CHANGES_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":chunk-names")) {
        if (old_set_watch_setting (TRACE_CHUNK_NAMES_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":justification-names")) {
        if (old_set_watch_setting (TRACE_JUSTIFICATION_NAMES_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":chunks")) {
        if (old_set_watch_setting (TRACE_CHUNKS_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":justifications")) {
        if (old_set_watch_setting (TRACE_JUSTIFICATIONS_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":backtracing")) {
        if (old_set_watch_setting (TRACE_BACKTRACING_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,":firings-preferences")) {
        if (old_set_watch_setting (TRACE_FIRINGS_PREFERENCES_SYSPARAM)) continue;
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string,"task")) {
        get_lexeme(); /* consume "task" */
        set_sysparam (TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,":firings")) {
        bool types[NUM_PRODUCTION_TYPES];
        int i;
        for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=TRUE;
        get_lexeme();
        /* --- read optional type indicator --- */
        if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
          if (!strcmp(current_agent(lexeme).string,"default")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=FALSE;
            types[DEFAULT_PRODUCTION_TYPE]=TRUE;
            get_lexeme();
          } else if (!strcmp(current_agent(lexeme).string,"user")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=FALSE;
            types[USER_PRODUCTION_TYPE]=TRUE;
            get_lexeme();
          } else if (!strcmp(current_agent(lexeme).string,"chunk")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=FALSE;
            types[CHUNK_PRODUCTION_TYPE]=TRUE;
            get_lexeme();
          } else if (!strcmp(current_agent(lexeme).string,"nonchunk")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=FALSE;
            types[DEFAULT_PRODUCTION_TYPE]=TRUE;
            types[USER_PRODUCTION_TYPE]=TRUE;
            get_lexeme();
          } else if (!strcmp(current_agent(lexeme).string,"all")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=TRUE;
            get_lexeme();
          }
        }
        /* --- read on/off flag --- */
        if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
          if (!strcmp(current_agent(lexeme).string,"on")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++)
              if (types[i])
                set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+i,TRUE);
            get_lexeme(); /* consume on/off flag */
            continue;
          }
          if (!strcmp(current_agent(lexeme).string,"off")) {
            for (i=0; i<NUM_PRODUCTION_TYPES; i++)
              if (types[i])
                set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+i,FALSE);
            get_lexeme(); /* consume on/off flag */
            continue;
          }
        }
        print ("Expected 'on' or 'off' for new watch setting\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      } /* end of if current_agent(lexeme).string is "firings" */

      if (!strcmp(current_agent(lexeme).string,":firings-wmes")) {
        get_lexeme(); /* consume "firings-wmes" */
        if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
          if (current_agent(lexeme).int_val==0) {
            set_sysparam (TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
                          NONE_WME_TRACE);
            get_lexeme();
            continue;
          }
          if (current_agent(lexeme).int_val==1) {
            set_sysparam (TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
                          TIMETAG_WME_TRACE);
            get_lexeme();
            continue;
          }
          if (current_agent(lexeme).int_val==2) {
            set_sysparam (TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
                          FULL_WME_TRACE);
            get_lexeme();
            continue;
          }
        }
        print ("Expected 0, 1, or 2 for new watch :firings-wmes setting\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
    }
    if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
      if (current_agent(lexeme).int_val==-1) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, FALSE);
        set_sysparam(TRACE_PHASES_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
        get_lexeme(); /* consume the number */
        continue;
      }
      if (current_agent(lexeme).int_val==0) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
        get_lexeme(); /* consume the number */
        continue;
      }
      if (current_agent(lexeme).int_val==1) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
        get_lexeme(); /* consume the number */
        continue;
      }
      if (current_agent(lexeme).int_val==2) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, TRUE);
        get_lexeme(); /* consume the number */
        continue;
      }
      if (current_agent(lexeme).int_val==3) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, TRUE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, TRUE);
        get_lexeme(); /* consume the number */
        continue;
      }
    }
    if (current_agent(lexeme).type==FLOAT_CONSTANT_LEXEME) {
      if (current_agent(lexeme).float_val==0.5) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
        get_lexeme(); /* consume the number */
        continue;
      }
      if (current_agent(lexeme).float_val==1.5) {
        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
        get_lexeme(); /* consume the number */
        continue;
      }
    }
    print ("Invalid argument to 'watch' command.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

/* -------------------------------------------------------------------
   
                        "Preferences" Command

   Syntax:  (preferences id attribute [level])
------------------------------------------------------------------- */

char *help_on_preferences[] = {
"Command: preferences",
"",
"Syntax: (preferences id attribute [level])",
"",
"This command prints all the preferences for the slot given by the 'id' and",
"'attribute' arguments.  The optional 'level' argument must be 0, 1, 2, or 3",
"(0 is the default); it indicates the level of detail requested:",
"  level 0 -- prints just the preferences themselves",
"  level 1 -- also prints the names of the productions that generated them",
"  level 2 -- also prints the timetags of the wmes matched by the productions",
"  level 3 -- prints the whole wmes, not just their timetags.",
0 };

void old_print_preference_and_source (preference *pref,
                                  bool print_source,
                                  wme_trace_type wtt) {
  print_string ("  ");
  print_object_trace (pref->value);
  print (" %c", preference_type_indicator (pref->type));
  if (preference_is_binary(pref->type)) print_object_trace (pref->referent);
  if (pref->o_supported) print (" [O]");
  print ("\n");
  if (print_source) {
    print ("    From ");
    print_instantiation_with_wmes (pref->inst, wtt);
    print ("\n");
  }
}

bool preferences_interface_routine (void) {
  Symbol *id, *attr;
  bool print_productions;
  wme_trace_type wtt;
  slot *s;
  preference *p;
  char id_str[MAX_LEXEME_LENGTH+1];     /* AGR 546 */
  char attr_str[MAX_LEXEME_LENGTH+1];   /* AGR 546 */

  get_lexeme();  /* consume "preferences", advance to production name(s) */

  /* --- read id --- */
  id = old_read_identifier_or_context_variable();
  if (!id) return FALSE;
  strcpy(id_str, current_agent(lexeme).string);   /* AGR 546 */
  get_lexeme();  /* consume the id */

  /* --- read attribute --- */
  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    attr = find_sym_constant (current_agent(lexeme).string); break;
  case INT_CONSTANT_LEXEME:
    attr = find_int_constant (current_agent(lexeme).int_val); break;
  case FLOAT_CONSTANT_LEXEME:
    attr = find_float_constant (current_agent(lexeme).float_val); break;
  case IDENTIFIER_LEXEME:
    attr = find_identifier (current_agent(lexeme).id_letter, current_agent(lexeme).id_number); break;
  case VARIABLE_LEXEME:
    attr = old_read_identifier_or_context_variable();
    if (!attr) return FALSE;
    break;
  default:
    print ("Expected either an identifier or a constant for the attribute\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  strcpy(attr_str, current_agent(lexeme).string);  /* AGR 546 */
  get_lexeme();  /* consume the attribute */

  /* --- read the optional level indicator --- */
  print_productions = FALSE;
  wtt = NONE_WME_TRACE;
  if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
    if (current_agent(lexeme).int_val==0) {
      print_productions = FALSE;
      wtt = NONE_WME_TRACE;
    } else if (current_agent(lexeme).int_val==1) {
      print_productions = TRUE;
      wtt = NONE_WME_TRACE;
    } else if (current_agent(lexeme).int_val==2) {
      print_productions = TRUE;
      wtt = TIMETAG_WME_TRACE;
    } else if (current_agent(lexeme).int_val==3) {
      print_productions = TRUE;
      wtt = FULL_WME_TRACE;
    } else {
      print ("'Level' argument must be 0, 1, 2, or 3\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
    get_lexeme();
  }

  /* --- print the preferences --- */
  s = find_slot (id, attr);
  if (!s) {
/*    print ("There is no such slot.\n");  AGR 546 */
    print ("There are no preferences for %s ^%s.", id_str, attr_str); /* AGR 546 */
    return TRUE;
  }

  print_with_symbols ("Preferences for %y ^%y:\n", id, attr);

  if (s->preferences[REQUIRE_PREFERENCE_TYPE]) {
    print ("\nRequires:\n");
    for (p=s->preferences[REQUIRE_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[PROHIBIT_PREFERENCE_TYPE]) {
    print ("\nProhibits:\n");
    for (p=s->preferences[PROHIBIT_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[ACCEPTABLE_PREFERENCE_TYPE]) {
    print ("\nAcceptables:\n");
    for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[REJECT_PREFERENCE_TYPE]) {
    print ("\nRejects:\n");
    for (p=s->preferences[REJECT_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[BEST_PREFERENCE_TYPE]) {
    print ("\nBests:\n");
    for (p=s->preferences[BEST_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[WORST_PREFERENCE_TYPE]) {
    print ("\nWorsts:\n");
    for (p=s->preferences[WORST_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[BETTER_PREFERENCE_TYPE]) {
    print ("\nBetters:\n");
    for (p=s->preferences[BETTER_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[WORSE_PREFERENCE_TYPE]) {
    print ("\nWorses:\n");
    for (p=s->preferences[WORSE_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]) {
    print ("\nUnary Indifferents:\n");
    for (p=s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]) {
    print ("\nBinary Indifferents:\n");
    for (p=s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[UNARY_PARALLEL_PREFERENCE_TYPE]) {
    print ("\nUnary Parallels:\n");
    for (p=s->preferences[UNARY_PARALLEL_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[BINARY_PARALLEL_PREFERENCE_TYPE]) {
    print ("\nBinary Parallels:\n");
    for (p=s->preferences[BINARY_PARALLEL_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  if (s->preferences[RECONSIDER_PREFERENCE_TYPE]) {
    print ("\nReconsiders:\n");
    for (p=s->preferences[RECONSIDER_PREFERENCE_TYPE]; p; p=p->next)
      old_print_preference_and_source (p, print_productions, wtt);
  }

  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Ms" Command

  Syntax: (ms arg*)
           arg  ::= a | r
           arg  ::= 0 | 1 | 2
------------------------------------------------------------------- */

char *help_on_ms[] = {
"Command: ms",
"",
"Syntax: (ms arg* )",
"        arg ::= a | r",
"        arg ::= 0 | 1 | 2",
"",
"This command prints the current Match Set, i.e., a list of productions",
"that are about to fire or retract in the next preference phase.",
"",
"With no arguments, this command prints out the production names for both ",
"Assertions and Retractions.",
"",
"The optional character specifies listing of either Assertions or Retractions.",
"",
"The optional integer specifies the level of detail wanted:  0 (the default)",
"prints out just the production names; 1 also prints the timetags of wmes",
"matched; and 2 prints the wmes rather than just their timetags.",
0 };

bool ms_interface_routine (void) {
  wme_trace_type prev_wtt, wtt;
  ms_trace_type  mst;

  wtt = NONE_WME_TRACE;
  prev_wtt = NO_WME_TRACE_SET;
  mst = MS_ASSERT_RETRACT;
  get_lexeme();  /* consume "ms", look for level */
 
  while (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
      if (prev_wtt != NO_WME_TRACE_SET) {
        print ("Error: do not specify more than one of ' ', '0', '1', or '2' arguments to 'ms' command\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      else if (current_agent(lexeme).int_val==0) {
        wtt = NONE_WME_TRACE;
        prev_wtt = wtt;
        get_lexeme();
        continue;
      }
      else if (current_agent(lexeme).int_val==1) {
        wtt = TIMETAG_WME_TRACE;
        prev_wtt = wtt;
        get_lexeme();
        continue;
      }
      else if (current_agent(lexeme).int_val==2) {
        wtt = FULL_WME_TRACE;
        prev_wtt = wtt;
        get_lexeme();
        continue;
    } else {
      print ("Error: 'ms' level of detail must be '0', '1', or '2'\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
  } else if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      if (mst != MS_ASSERT_RETRACT) {
        print ("Error: do not specify more than one of ' ', 'a', or 'r' arguments to 'ms' command\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      if (!strcmp(current_agent(lexeme).string, "a")) mst = MS_ASSERT;
      else if (!strcmp(current_agent(lexeme).string, "r")) mst = MS_RETRACT;
      else {
        print ("Error: unrecognized argument to 'ms' command\n");
        print_location_of_most_recent_lexeme();
        return FALSE;
      }
      get_lexeme();
      continue;
    }
    else {
      print ("Error: unrecognized argument to 'ms' command\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
  }
  print_match_set (wtt, mst);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Sp" Command

   Syntax:  see helpscreen below.
------------------------------------------------------------------- */

char *help_on_sp[] = {
"Command: sp",
"",
"Syntax: (sp production-name",
"          [ \"optional-documentation-string\" ]",
"          [ flag ]*",
"          LHS",
"          -->",
"          RHS)",       
"",
"       flag  ::=  :o-support",
"       flag  ::=  :i-support",
"       flag  ::=  :default",
"       flag  ::=  :chunk",
"",
"This command adds a new production to the system.  (If another production",
"with the same name already exists, it is excised.)  The optional flags",
"are as follows:",
"   :o-support -- specifies that all the RHS actions are to be given",
"                 o-support when the production fires",
"   :no-support -- specifies that all the RHS actions are only to be given",
"                  i-support when the production fires",
"   :default -- specifies that this production is a default production (this",
"               matters for (excise-task) and (watch task))",
"   :chunk -- specifies that this production is a chunk (this matters for",
"             (learn trace))",
"",
"See also:  lhs-grammar, rhs-grammar",
0 };

bool sp_interface_routine (void) {
  production *p;

  set_lexer_allow_ids (FALSE);
  get_lexeme();  /* consume "sp", advance to production name */
  p = parse_production();
  if (p) print ("*");
  if (p) return TRUE; else return FALSE;
}

/* -------------------------------------------------------------------
   
                    "Max-elaborations" Command
 
   Syntax:  (max-elaborations [integer])
------------------------------------------------------------------- */

char *help_on_max_elaborations[] = {
"Command: max-elaborations",
"",
"Syntax: (max-elaborations [integer])",
"",
"With no arguments, this command prints the current value of the system",
"variable 'max-elaborations'.  With an integer argument, it sets the current",
"value.   This variable controls the maximum number of elaboration cycles",
"allowed in a single decision cycle.  After this many elabloration cycles",
"have been executed, Soar proceeds to decision phase even if quiescence",
"hasn't really been reached yet.  (Max-elaborations is initially 100.)",
0 };

bool max_elaborations_interface_routine (void) {
  get_lexeme();  /* consume "max-elaborations", advance to integer */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the current value --- */
    print ("Max-elaborations is %ld.\n", current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM]);
    return TRUE;
  }
  if (current_agent(lexeme).type!=INT_CONSTANT_LEXEME) {
    print ("Expected integer for new value of max-elaborations.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  set_sysparam (MAX_ELABORATIONS_SYSPARAM, current_agent(lexeme).int_val);
  get_lexeme(); /* consume the integer */
  return TRUE;
}

/* -------------------------------------------------------------------

   "Max-chunks" Command

   Syntax:  (max-chunks [integer])
------------------------------------------------------------------- */

char *help_on_max_chunks[] = {
"Command: max-chunks",
"",
"Syntax: (max-chunks [integer])",
"",
"With no arguments, this command prints the current value of the system",
"variable 'max-chunks'.  With an integer argument, it sets the current",
"value.   This variable controls the maximum number of chunks",
"allowed in a single decision cycle.  After this many chunks",
"have been executed, Soar proceeds to decision phase even if quiescence",
"hasn't really been reached yet.  (Max-chunks is initially 50.)",
0 };

bool max_chunks_interface_routine (void) {
  get_lexeme();  /* consume "max-chunks", advance to integer */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the current value --- */
    print ("Max-chunks is %ld.\n", current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
    return TRUE;
  }
  if (current_agent(lexeme).type!=INT_CONSTANT_LEXEME) {
    print ("Expected integer for new value of max-chunks.\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  set_sysparam (MAX_CHUNKS_SYSPARAM, current_agent(lexeme).int_val);
  get_lexeme(); /* consume the integer */
  return TRUE;
}

/* -------------------------------------------------------------------

                        "User-select" Command

   Syntax:  (user-select [first | last | ask | random | t | nil])
------------------------------------------------------------------- */

/* AGR 615  Adding the "last" option to this command was pretty simple
   and the changes are integrated into this entire function.  94.11.08 */

char *help_on_user_select[] = {
"Command: user-select",
"",
"Syntax: (user-select [first | last | ask | random | t | nil])",
"",
"With no arguments, this command prints the current setting of user-select.",
"With an argument, it sets user-select to the given value.  This controls",
"how Soar's decision procedure chooses between multiple indifferent items:",
"   first -- just choose the first one found (deterministically)",
"   last -- just choose the last one found (deterministically)",
"   ask -- ask the user to choose one of the items",
"   random -- choose one randomly",
"   t -- synonymous with 'ask'",
"   nil -- synonymous with 'random'",
0 };

bool user_select_interface_routine (void) {
  get_lexeme();  /* consume "user-select", advance to mode */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the current value --- */
    print ("User-select is currently set to:  ");
    switch (current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]) {
    case USER_SELECT_FIRST: print ("first"); break;
    case USER_SELECT_LAST: print ("last"); break;
    case USER_SELECT_ASK: print ("ask"); break;
    case USER_SELECT_RANDOM: print ("random"); break;
    }
    print ("\n");
    return TRUE;
  }
  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    if (!strcmp(current_agent(lexeme).string,"first")) {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
      get_lexeme();
      return TRUE;
    }
    if (!strcmp(current_agent(lexeme).string,"last")) {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
      get_lexeme();
      return TRUE;
    }
    if ( (!strcmp(current_agent(lexeme).string,"ask")) ||
         (!strcmp(current_agent(lexeme).string,"t")) ) {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_ASK);
      get_lexeme();
      return TRUE;
    }
    if ( (!strcmp(current_agent(lexeme).string,"random")) ||
         (!strcmp(current_agent(lexeme).string,"nil")) ) {
      set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
      get_lexeme();
      return TRUE;
    }
  }
  print ("Expected first, ask, or random for new value of user-select.\n");
  print_location_of_most_recent_lexeme();
  return FALSE;
}

/* -------------------------------------------------------------------
   
                        "Soarnews" Command
 
   Syntax:  (soarnews)
------------------------------------------------------------------- */

char *help_on_soarnews[] = {
"Command: soarnews",
"",
"Syntax: (soarnews)",
"",
"This command prints news about the current release of Soar.",
0 };

bool soarnews_interface_routine (void) {
  get_lexeme();  /* consume "soarnews" */

  print ("News for Soar version %s\n", soar_version_string);
  print (soar_news_string);

  return TRUE;
}

/* -------------------------------------------------------------------
  
  "List-productions", "list-chunks", and "list-justifications" Commands

   Syntax: (list-productions [prod-type*] [:internal] ["filename" [:append]])
           prod-type ::= default | user | chunk | justification
           (list-chunks [:internal] ["filename" [:append]])
           (list-justifications [:internal] ["filename" [:append]])
------------------------------------------------------------------- */

char *help_on_list_productions[] = {
"Commands: list-productions, list-chunks, list-justifications",
"",
"Syntax: (list-productions [prod-type*] [:internal] [\"filename\" [:append]])",
"    prod-type ::= default | user | chunk | justification",
"    (list-chunks) is shorthand for (list-productions chunk)",
"    (list-justifications) is shorthand for (list-productions justification)",
"",
"This command prints all productions of the indicated types.  (If no",
"prod-type's are given, all productions except justifications are printed.)",
"",
"The optional [:internal] argument tells Soar to print productions in their",
"internal reordered (rete net) form.",
"",
"If a filename is given, the productions are printed to that file; otherwise",
"they are printed to the screen.  If :append is given, the file is appended",
"to, rather than overwritten.",
0 };

bool list_productions_interface_routine (void) {
  char filename[MAX_LEXEME_LENGTH+1];
  bool prod_type_present, filename_present, internal;
  bool append = FALSE;
  bool types[NUM_PRODUCTION_TYPES]; /* tells which types of prod's to list */
  int i;

  /* --- get settings of types[] --- */
  for (i=0; i<NUM_PRODUCTION_TYPES; i++) types[i]=FALSE; /* init to FALSE */
  if (!strcmp(current_agent(lexeme).string,"list-chunks")) {
    types[CHUNK_PRODUCTION_TYPE] = TRUE;
    get_lexeme(); /* consume "list-chunks" */
  } else if (!strcmp(current_agent(lexeme).string,"list-justifications")) {
    types[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
    get_lexeme(); /* consume "list-justifications" */
  } else {
    get_lexeme(); /* consume "list-productions" */
    /* --- read prod-type* --- */
    prod_type_present = FALSE;
    while (TRUE) {
      if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) break;
      if (!strcmp(current_agent(lexeme).string,"default")) {
        prod_type_present = TRUE;
        types[DEFAULT_PRODUCTION_TYPE] = TRUE;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"user")) {
        prod_type_present = TRUE;
        types[USER_PRODUCTION_TYPE] = TRUE;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"chunk")) {
        prod_type_present = TRUE;
        types[CHUNK_PRODUCTION_TYPE] = TRUE;
        get_lexeme();
        continue;
      }
      if (!strcmp(current_agent(lexeme).string,"justification")) {
        prod_type_present = TRUE;
        types[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
        get_lexeme();
        continue;
      }
      break;
    } /* end of while (TRUE) */
    if (!prod_type_present) {
      /* --- no prod-type* was present --- */
      types[DEFAULT_PRODUCTION_TYPE] = TRUE;
      types[USER_PRODUCTION_TYPE] = TRUE;
      types[CHUNK_PRODUCTION_TYPE] = TRUE;
    }
  }

  /* --- got types[], so now look for :internal flag --- */
  internal = FALSE;
  if ((current_agent(lexeme).type==SYM_CONSTANT_LEXEME) &&
      (!strcmp(current_agent(lexeme).string,":internal")))
    { internal = TRUE; get_lexeme(); }

  /* mvp 5-17-94 - this may be incorrect */
  /* --- look for filename and :append --- */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    filename_present = FALSE;
  } 
#ifdef UNIX
    else {
      filename_present = TRUE;
      strcpy (filename, "\0");
      while (TRUE) {
        if (current_agent(lexeme).type==R_PAREN_LEXEME)
          break;
        strcat (filename, current_agent(lexeme).string);
        get_lexeme ();
      }
      append = FALSE;
      if (!strcmp(current_agent(lexeme).string, ":append")) { append=TRUE; get_lexeme(); }
  } 
#else
    else if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
      print ("Expected string in quotes for filename\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
  }
#endif

  /* --- all set, now print all the productions --- */
  {
    FILE *output_file;
    production *prod;

    output_file = NIL; /* unnecessary, but gcc -Wall warns without it */
    if (filename_present) {
      output_file = fopen (filename, (append ?  "a" : "w") );
      if (!output_file) {
        /* --- error when opening the file --- */
        print ("Error: unable to open file %s\n",filename);
        return FALSE;
      }
      print ("Writing productions to file %s\n", filename);
      start_redirection_to_file (output_file);
    }

   /* AGR 641 begin */
   /* AGR 641  We want to change it so chunks are printed out starting with
   the lowest numbered chunks.  That means we need to go to the end of
   the list and go back.  94.11.02 */

    for (i=0; i<NUM_PRODUCTION_TYPES; i++) {
      if (types[i]) {
	/* first go to end of list to reverse the order  GAP */
	for (prod=current_agent(all_productions_of_type)[i]; 
	     prod != NIL && prod->next!=NIL; 
	     prod=prod->next)
	  /* intentionally null */ ;

	while (prod != NIL){
	  print_production (prod, internal);
	  print ("\n");
	  prod=prod->prev;
	}
      }  /* if type is to be outputted */
    }  /* for all types */
    /* AGR 641 end */

    if (filename_present) {
      stop_redirection_to_file ();

      /* AGR 611 begin */
      if (fclose (output_file)) {
	printf("Error: unable to close file %s\n", filename);
	return FALSE;
      }
      /* AGR 611 end */

    }
  }

  return TRUE;
}

/* -------------------------------------------------------------------
   
                   "Add-wme" and "Remove-wme" Commands

   Syntax: (add-wme id ^ { attribute | '*'} { value | '*' } [+])
           (remove-wme integer)
------------------------------------------------------------------- */

char *help_on_add_or_remove_wme[] = {
"Commands: add-wme, remove-wme",
"",
"Syntax: (add-wme id ^ { attribute | '*'} { value | '*' } [+])",
"        (remove-wme integer)",
"",
"These commands surgically modify Soar's working memory.  Add-wme adds a",
"new wme with the given id, attribute, value, and optional acceptable",
"preference.  The given id must be an existing identifier.  If '*' is given",
"in place of the attribute or value, Soar creates a new identifier (gensym)",
"for that field.  Remove-wme removes the wme with the given timetag.",
"",
"WARNING: these commands are inherently unstable and may have weird side",
"effects (possibly even including system crashes).  For example, the chunker",
"can't backtrace through wmes created via add-wme.  Removing input wmes or",
"context/impasse wmes may have unexpected side effects.  You've been warned.",
0 };

bool add_wme_interface_routine (void) {
  Symbol *id, *attr, *value;
  bool acceptable_preference;
  wme *w;

  get_lexeme();  /* consume "add-wme" */
  id = old_read_identifier_or_context_variable();
  if (!id) return FALSE;
  get_lexeme();  /* consume id */
  if (current_agent(lexeme).type!=UP_ARROW_LEXEME) {
    print ("Expected ^ between id and attribute\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  get_lexeme();  /* consume ^ */
  
  /* --- get attribute or '*' --- */
  if (strcmp(current_agent(lexeme).string,"*") == 0) {
    attr = make_new_identifier ('I', id->id.level);
  } else {
    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
      attr = make_sym_constant (current_agent(lexeme).string); break;
    case INT_CONSTANT_LEXEME:
      attr = make_int_constant (current_agent(lexeme).int_val); break;
    case FLOAT_CONSTANT_LEXEME:
      attr = make_float_constant (current_agent(lexeme).float_val); break;
    case IDENTIFIER_LEXEME:
    case VARIABLE_LEXEME:
      attr = old_read_identifier_or_context_variable();
      if (!attr) return FALSE;
      symbol_add_ref (attr);
      break;
    default:
      print ("Expected constant, identifier, or '*' for attribute\n");
      print_location_of_most_recent_lexeme();
      return FALSE;
    }
  }
  get_lexeme(); /* consume attribute */

  /* --- get value or '*' --- */
  if (strcmp(current_agent(lexeme).string,"*") == 0) {
    value = make_new_identifier ('I', id->id.level);
  } else {
    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
      value = make_sym_constant (current_agent(lexeme).string); break;
    case INT_CONSTANT_LEXEME:
      value = make_int_constant (current_agent(lexeme).int_val); break;
    case FLOAT_CONSTANT_LEXEME:
      value = make_float_constant (current_agent(lexeme).float_val); break;
    case IDENTIFIER_LEXEME:
    case VARIABLE_LEXEME:
      value = old_read_identifier_or_context_variable();
      if (!value) { symbol_remove_ref (attr); return FALSE; }
      symbol_add_ref (value);
      break;
    default:
      print ("Expected constant, identifier, or '*' for value\n");
      print_location_of_most_recent_lexeme();
      symbol_remove_ref (attr);
      return FALSE;
    }
  }
  get_lexeme(); /* consume value */

  /* --- get optional acceptable preference indicator --- */
  acceptable_preference = FALSE;
  if (current_agent(lexeme).type==PLUS_LEXEME) { acceptable_preference = TRUE; get_lexeme(); }

  /* --- now create and add the wme --- */
  w = make_wme (id, attr, value, acceptable_preference);
  symbol_remove_ref (attr);
  symbol_remove_ref (value);
  insert_at_head_of_dll (id->id.input_wmes, w, next, prev);
  add_wme_to_wm (w);
  do_buffered_wm_and_ownership_changes();
  
  return TRUE;
}

bool remove_wme_interface_routine (void) {
  wme *w, *w2;
  Symbol *id;
  slot *s;
  
  get_lexeme();  /* consume "remove-wme" */
  if (current_agent(lexeme).type!=INT_CONSTANT_LEXEME) {
    print ("Expected integer for timetag of wme to remove\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
    if (w->timetag == (unsigned)(current_agent(lexeme).int_val)) break;
  if (!w) {
    print ("No wme %ld in working memory\n", current_agent(lexeme).int_val);
    return FALSE;
  }
  get_lexeme();  /* consume timetag */

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

  /* --- now remove w from working memory --- */
  remove_wme_from_wm (w);
  do_buffered_wm_and_ownership_changes();

  return TRUE;
}

/* -------------------------------------------------------------------
   
                        "Firing-counts" Command

   Syntax: (firing-counts [integer])
           (firing-counts production-name ...)
------------------------------------------------------------------- */

char *help_on_firing_counts[] = {
"Command: firing-counts",
"",
"Syntax: (firing-counts [integer])",
"        (firing-counts production-name ...)",
"",
"This command prints how many times certain productions have fired.  With",
"no arguments, it lists all the productions sorted according to how many",
"times they have fired.  If an integer argument (call it k) is given, only",
"the top k productions are listed.  If k=0, only the productions which",
"haven't fired at all are listed.  Note that firing counts are not reset",
"by an (init-soar); the counts indicate the number of firings since the",
"productions were loaded or built.",
"",
"Note:  this is slow, because the sorting takes time O(n*log n)",
"",
"With one or more production names as arguments, this command prints how",
"many times each of those productions fired.",
0 };

/*int old_compare_firing_counts (e1,e2)
     const void *e1;
     const void *e2;*/
int old_compare_firing_counts (const void *e1,const void *e2) 
		 /* rmarinie: changed style of declaration from KNR's "old style";
				was: int old_compare_firing_counts (e1,e2) with next two lines
				uncommented */


{
  production *p1, *p2;
  unsigned long count1, count2;
  p1 = *((production **)e1);
  p2 = *((production **)e2);
  count1 = p1->firing_count;
  count2 = p2->firing_count;
  return (count1<count2) ? -1 : (count1>count2) ? 1 : 0;
}

bool firing_counts_interface_routine (void) {
  Symbol *sym;
  long num_prods, num_requested;
  production *((*all_prods)[]), **ap_item, *p;
  
  set_lexer_allow_ids (FALSE); /* only takes production names, never ids */
  get_lexeme();  /* consume "firing-counts" */

  /* --- handle production name arguments --- */
  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    while (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
      sym = find_sym_constant (current_agent(lexeme).string);
      if (sym && sym->sc.production) {
        print ("%6lu:  %s\n", sym->sc.production->firing_count, current_agent(lexeme).string);
      } else {
        print ("No production named %s\n", current_agent(lexeme).string);
        print_location_of_most_recent_lexeme();
      }
      get_lexeme();
    }
    return TRUE;
  }

  /* --- handle integer (or no) arguments --- */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    num_requested = 20;
  } else if (current_agent(lexeme).type==INT_CONSTANT_LEXEME) {
    num_requested = current_agent(lexeme).int_val;
    get_lexeme();
  } else {
    print ("Illegal argument to 'firing-counts' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }

  num_prods = current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE];
  if (num_prods==0) return TRUE;  /* so we don't barf on zero later */

  /* --- make an array of pointers to all the productions --- */
  all_prods = allocate_memory (num_prods * sizeof (production *),
                               MISCELLANEOUS_MEM_USAGE);

  /* MVP - 6-8-94 where is it freed ? */

  ap_item = &((*all_prods)[0]);
  for (p=current_agent(all_productions_of_type)[DEFAULT_PRODUCTION_TYPE]; p!=NIL; p=p->next)
    *(ap_item++) = p;
  for (p=current_agent(all_productions_of_type)[USER_PRODUCTION_TYPE]; p!=NIL; p=p->next)
    *(ap_item++) = p;
  for (p=current_agent(all_productions_of_type)[CHUNK_PRODUCTION_TYPE]; p!=NIL; p=p->next)
    *(ap_item++) = p;

  /* --- sort that array according to firing counts --- */
  qsort (all_prods, num_prods, sizeof (production *), old_compare_firing_counts);

  /* --- now print out the results --- */
  if (num_requested==0) {
    ap_item = &((*all_prods)[0]);
    while ((*ap_item)->firing_count==0) {
      print_with_symbols ("     0:  %y\n", (*ap_item)->name);
      ap_item++;
    }

    /* MVP 6-8-94 try this to plug memory leak */
    free_memory (all_prods, MISCELLANEOUS_MEM_USAGE);

    return TRUE;
  }
  if ((num_requested < 0) || (num_requested > num_prods))
    num_requested = num_prods;
  ap_item = &((*all_prods)[num_prods-1]);
  while (num_requested) {
    print ("%6lu:  ", (*ap_item)->firing_count);
    print_with_symbols ("%y\n", (*ap_item)->name);
    ap_item--;
    num_requested--;
  }

  /* MVP 6-8-94 also try this to plug memory leak */
  free_memory (all_prods, MISCELLANEOUS_MEM_USAGE);

  return TRUE;
}

/* -------------------------------------------------------------------
   
                      "Pwatch" and "Unpwatch" Commands

   Syntax: (pwatch [production-name ...])
   Syntax: (unpwatch [production-name ...])
------------------------------------------------------------------- */

char *help_on_pwatch_and_unpwatch[] = {
"Commands: pwatch, unpwatch",
"",
"Syntax: (pwatch [production-name ...])",
"        (unpwatch [production-name ...])",
"",
"These commands enable and disable tracing the firings and retractions of",
"individual productions.  (This mechanism is orthogonal to the watch :firings",
"mechanism.  See (help watch) for more information.)",
"",
"Pwatch, with no arguments, lists the productions currently being traced.",
"With one or more production name arguments, it enables tracing of those",
"productions.  Tracing persists until disabled by an unpwatch command, or",
"until the production is excised.",
"",
"Unpwatch undoes the effects of pwatch.  With no arguments, it disables all",
"previously enabled production traces.  With one or more production name",
"arguments, it disables just those traces.",
"",
"See also:  watch",
0 };

bool pwatch_interface_routine (void) {
  Symbol *sym;
  cons *c;
  
  set_lexer_allow_ids (FALSE); /* only takes production names, never ids */
  get_lexeme();  /* consume "pwatch" */

  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- list current pwatchs --- */
    for (c=current_agent(productions_being_traced); c!=NIL; c=c->rest)
      print_with_symbols (" %y\n", ((production *)(c->first))->name);
    return TRUE;
  }

  /* --- handle production name arguments --- */
  while (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    sym = find_sym_constant (current_agent(lexeme).string);
    if (sym && sym->sc.production) {
      add_pwatch (sym->sc.production);
    } else {
      print ("No production named %s\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
    }
    get_lexeme();
  }

  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Bad argument to 'pwatch' command--expected a production name\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

bool unpwatch_interface_routine (void) {
  production *prod;
  Symbol *sym;
  
  set_lexer_allow_ids (FALSE); /* only takes production names, never ids */
  get_lexeme();  /* consume "unpwatch" */

  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- remove all current pwatchs --- */
    while (current_agent(productions_being_traced)) {
      prod = current_agent(productions_being_traced)->first;
      remove_pwatch (prod);
    }
    return TRUE;
  }

  /* --- handle production name arguments --- */
  while (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    sym = find_sym_constant (current_agent(lexeme).string);
    if (sym && sym->sc.production) {
      remove_pwatch (sym->sc.production);
    } else {
      print ("No production named %s\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
    }
    get_lexeme();
  }

  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Bad argument to 'unpwatch' command--expected a production name\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Warnings" Command
 
   Syntax:  (warnings [on|off])
------------------------------------------------------------------- */

char *help_on_warnings[] = {
"Command: warnings",
"",
"Syntax: (warnings [on|off])",
"",
"(Warnings on) enables the printing of warning messages.  This is the",
"default.  (Warnings off) turns off most warning messages.  (Warnings)",
"prints an indication of whether warning messages are enabled or not.",
0 };

bool warnings_interface_routine (void) {
  get_lexeme();  /* consume "warnings", advance to integer */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    /* --- invoked with no arguments, so just print the currents status --- */
    print ("Warnings are %s.\n",
           current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM] ? "on" : "off");
    return TRUE;
  }
  if (current_agent(lexeme).type!=SYM_CONSTANT_LEXEME) {
    print ("Expected 'on' or 'off' for argument to 'warnings' command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  if (!strcmp (current_agent(lexeme).string, "on")) {
    set_sysparam (PRINT_WARNINGS_SYSPARAM, TRUE);
    get_lexeme();
    return TRUE;
  }
  if (!strcmp (current_agent(lexeme).string, "off")) {
    set_sysparam (PRINT_WARNINGS_SYSPARAM, FALSE);
    get_lexeme();
    return TRUE;
  }
  print ("Expected 'on' or 'off' for argument to 'warnings' command\n");
  print_location_of_most_recent_lexeme();
  return FALSE;
}

#ifndef NO_TIME_INTERFACE
#if !defined(WIN32) && !defined(MACINTOSH)
/* -------------------------------------------------------------------
   
                          "Time" Command

------------------------------------------------------------------- */

char *help_on_time[] = {
"Command: time",
"",
"Syntax: (time (command-1 its-args...) (command-2 its-args...) ...)",
"",
"This command executes one or more other commands, then gives a detailed",
"report on how much time it took to execute them.  Note that the other",
"commands *must* be enclosed in parentheses--this is one case where you",
"can't leave off the parentheses.",
0 };

double time_difference (struct timeval *start, struct timeval *end) {
  long seconds, micros;

  seconds = end->tv_sec - start->tv_sec;
  micros = end->tv_usec - start->tv_usec;
  if (micros < 0) {
    micros += 1000000;
    seconds--;
  }
  return (double)(seconds) + (double)(micros)/1000000.0;
}

bool time_interface_routine (void) {
  int parentheses_level;
  struct rusage start_rusage, end_rusage;
  struct timeval start_real_time, end_real_time;
  double user_cpu_time, sys_cpu_time, total_cpu_time, real_time;
  
  /* --- get initial time statistics --- */
  getrusage (RUSAGE_SELF, &start_rusage);
  gettimeofday (&start_real_time,NIL);
  
  /* --- read and dispatch the series of commands --- */
  parentheses_level = current_lexer_parentheses_level();  
  get_lexeme();  /* consume "time", advance to first command */
  while (current_agent(lexeme).type==L_PAREN_LEXEME) {
    /* --- read one command, dispatch it --- */
    get_lexeme(); /* consume lparen */
    if ((current_agent(lexeme).type == SYM_CONSTANT_LEXEME) ||
	(current_agent(lexeme).type == DOLLAR_STRING_LEXEME)) { /* AGR 562 */
      dispatch_command();
      if (current_agent(lexeme).type==EOF_LEXEME) break;
    } else {
      print ("Error:  unknown command\n");
      print_location_of_most_recent_lexeme();
      /* consume just the single bad command */
      skip_ahead_to_balanced_parentheses (parentheses_level);
    }
    get_lexeme();  /* consume rparen, advance to next command */
  } /* end of while loop */
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Error: expected ) to end time command or ( to begin next thing to time\n");
    print_location_of_most_recent_lexeme();
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
  }
  
  /* --- print out the final time statistics --- */
  gettimeofday (&end_real_time, NIL);
  getrusage (RUSAGE_SELF, &end_rusage);
  user_cpu_time = time_difference (&(start_rusage.ru_utime),
                                   &(end_rusage.ru_utime));
  sys_cpu_time = time_difference (&(start_rusage.ru_stime),
                                  &(end_rusage.ru_stime));
  total_cpu_time = user_cpu_time + sys_cpu_time;
  real_time = time_difference (&start_real_time, &end_real_time);
  print ("\nCPU time (seconds): %.3f (%.3f user, %.3f system)",
         total_cpu_time, user_cpu_time, sys_cpu_time);
  print ("\nReal time: %.3f  (%.1f%%)\n",
         real_time,
         100.0 * total_cpu_time / ((real_time>0.0) ? real_time : 0.1));
  return TRUE;
}
#endif
#endif
/* -------------------------------------------------------------------
   
                          "Memory-stats" Command

   Syntax:  (memory-stats)
------------------------------------------------------------------- */

char *help_on_memory_stats[] = {
"Command: memory-stats",
"",
"Syntax: (memory-stats)",
"",
"This command prints out statistics on memory usage.",
"",
"See also:  rete-stats, stats",
0 };

bool memory_stats_interface_routine (void) {
  print_memory_statistics ();
  print_memory_pool_statistics ();
  get_lexeme();  /* consume "memory-stats", advance to rparen */
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Rete-stats" Command

   Syntax:  (rete-stats)
------------------------------------------------------------------- */

char *help_on_rete_stats[] = {
"Command: rete-stats",
"",
"Syntax: (rete-stats)",
"",
"This command prints out statistics on the rete net.",
"",
"See also:  memory-stats, stats",
0 };

bool rete_stats_interface_routine (void) {
  print_rete_statistics ();
  get_lexeme();  /* consume "rete-stats", advance to rparen */
  return TRUE;
}

/* -------------------------------------------------------------------
   
                  "Stats" and "Print-Stats" Commands

   Syntax:  (stats) or (print-stats)
------------------------------------------------------------------- */

char *help_on_stats[] = {
"Commands: stats, print-stats",
"",
"Syntax: (stats) or (print-stats) [they are synonymous]",
"",
"This command prints out some statistics on the current Soar run.",
"",
"See also:  memory-stats, rete-stats",
0 };

bool stats_interface_routine (void) {
  unsigned long wme_changes;

  /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  double total_kernel_time, total_kernel_msec, derived_kernel_time, monitors_sum,
         input_function_time,  input_phase_total_time,
        output_function_time, output_phase_total_time,
         preference_phase_total_time, wm_phase_total_time, decision_phase_total_time,
         derived_total_cpu_time;

 #ifdef DETAILED_TIMING_STATS
  double match_time, match_msec;
  double ownership_time, chunking_time;
  double other_phase_kernel_time[5], other_total_kernel_time;
 #endif
#endif
  /* REW: end 28.07.96 */

  /* MVP 6-8-94 */
  char hostname[MAX_LEXEME_LENGTH+1];

  long current_time;  

#if defined(THINK_C) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
  strcpy (hostname, "[host name unknown]");
#else
#ifdef __SC__
  strcpy (hostname, "[host name unknown]");
#else
  if (gethostname (hostname, 1000)) strcpy (hostname, "[host name unknown]");
#endif
#endif
  current_time = time(NULL);

/* REW: begin 28.07.96 */   /* See note in soarkernel.h for a description of the timers */
#ifndef NO_TIMING_STUFF
      total_kernel_time = timer_value (&current_agent(total_kernel_time));
      total_kernel_msec = total_kernel_time * 1000.0;
 
      /* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
         excluding Input Function, Output function, and pre-defined callbacks. 
         This computed time should be roughly equal to total_kernel_time, 
         as determined above.*/
       
      derived_kernel_time = timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
                    + timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

      input_function_time  = timer_value (&current_agent(input_function_cpu_time));

      output_function_time = timer_value (&current_agent(output_function_cpu_time));


      /* Total of the time spent in callback routines. */
      monitors_sum =  timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));

      derived_total_cpu_time  = derived_kernel_time + monitors_sum + input_function_time 
                                  + output_function_time;

      /* Total time spent in the input phase */
      input_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
               + timer_value (&current_agent(input_function_cpu_time));

      /* Total time spent in the preference phase */
      preference_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));

      /* Total time spent in the working memory phase */
      wm_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[WM_PHASE]));

      /* Total time spent in the output phase */
      output_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
               + timer_value (&current_agent(output_function_cpu_time));

      /* Total time spent in the decision phase */
      decision_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));

      /* The sum of these five phase timers is exactly equal to the derived_total_cpu_time */

 #ifdef DETAILED_TIMING_STATS

      match_time = timer_value (&current_agent(match_cpu_time[INPUT_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[WM_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[DECISION_PHASE]));
   
      match_msec = 1000 * match_time; 

     ownership_time = timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]));

      chunking_time = timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));

      /* O-support time should go to 0 with o-support-mode 2 */
      /* o_support_time = timer_value (&current_agent(o_support_cpu_time[INPUT_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[DECISION_PHASE])); */

      other_phase_kernel_time[INPUT_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[INPUT_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE]));

     other_phase_kernel_time[PREFERENCE_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));

      other_phase_kernel_time[WM_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[WM_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[WM_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[WM_PHASE]));

      other_phase_kernel_time[OUTPUT_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE]));

      other_phase_kernel_time[DECISION_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[DECISION_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));

      other_total_kernel_time = other_phase_kernel_time[INPUT_PHASE] 
                              + other_phase_kernel_time[PREFERENCE_PHASE]
                              + other_phase_kernel_time[WM_PHASE]
                              + other_phase_kernel_time[OUTPUT_PHASE]
                              + other_phase_kernel_time[DECISION_PHASE];

 #endif
#endif
/* REW: end 28.07.96 */      

  print ("Soar %s on %s at %s\n", soar_version_string,
	 hostname, ctime((const time_t *)&current_time));
  print ("%lu productions (%lu default, %lu user, %lu chunks)\n",
         current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
         current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
         current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE],
         current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
         current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
         current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
  print ("   + %lu justifications\n",
         current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);


/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
      /* The fields for the timers are 10.3, providing an upper limit of approximately
         11 days for the printing of the run time calculations.  Obviously, these will
         need to be increased if you plan on needing run-time data for a process that
         you expect to take longer than 11 days. :) */


      print ("                                                                 |    Derived\n");
      print ("Phases:        Input       Pref      W/M       Output   Decision |     Totals\n");
      print ("=================================================================|===========\n");
      
      print ("Kernel:   %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n", timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])), derived_kernel_time);


#ifdef DETAILED_TIMING_STATS

      print ("======================  Detailed Timing Statistics  =============|===========\n");

      print ("   Match: %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n", 
                 timer_value (&current_agent(match_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[DECISION_PHASE])) , 
                 match_time);

      print ("Own'ship: %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n", 
                 timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE])), 
                 ownership_time);

      print ("Chunking: %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n", 
                 timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE])), 
                 chunking_time);

      print ("   Other: %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n", 
                 other_phase_kernel_time[INPUT_PHASE],
                 other_phase_kernel_time[PREFERENCE_PHASE],
                 other_phase_kernel_time[WM_PHASE],
                 other_phase_kernel_time[OUTPUT_PHASE],
                 other_phase_kernel_time[DECISION_PHASE],
                 other_total_kernel_time);


#endif

      
      print ("=================================================================|===========\n");
      print ("Input fn: %10.3f                                             | %10.3f\n",  
              input_function_time, input_function_time); 

      print ("=================================================================|===========\n");
      print ("Outpt fn:                                  %10.3f            | %10.3f\n",  
              output_function_time, output_function_time);

      print ("=================================================================|===========\n");
      print ("Callbcks: %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n",  
                 timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE])), 
                 monitors_sum);

      print ("=================================================================|===========\n");
      print ("Derived----------------------------------------------------------+-----------\n");
      print ("Totals:   %10.3f %10.3f %10.3f %10.3f %10.3f | %10.3f\n\n", 
                        input_phase_total_time,
                        preference_phase_total_time,
                        wm_phase_total_time,
                        output_phase_total_time, 
                        decision_phase_total_time,
                        derived_total_cpu_time);

      print ("Values from single timers:\n");
      print (" Kernel CPU Time: %11.3f sec. \n", total_kernel_time);     
      print (" Total  CPU Time: %11.3f sec.\n\n", timer_value (&current_agent(total_cpu_time)));

#endif

#if !defined(NO_TIMING_STUFF)

      print ("%lu decision cycles (%.3f msec/dc)\n",
	     current_agent(d_cycle_count),
	     current_agent(d_cycle_count) ? total_kernel_msec/current_agent(d_cycle_count) : 0.0);
      print ("%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
	     current_agent(e_cycle_count),
	     current_agent(d_cycle_count) ? (double)current_agent(e_cycle_count)/current_agent(d_cycle_count) : 0,
	     current_agent(e_cycle_count) ? total_kernel_msec/current_agent(e_cycle_count) : 0);
      print ("%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
	     current_agent(production_firing_count),
	     current_agent(e_cycle_count) ? (double)current_agent(production_firing_count)/current_agent(e_cycle_count) : 0.0,
	     current_agent(production_firing_count) ? total_kernel_msec/current_agent(production_firing_count) : 0.0);

#else
      print ("%lu decision cycles \n", current_agent(d_cycle_count));
      print ("%lu elaboration cycles \n", current_agent(e_cycle_count));
      print ("%lu production firings \n", current_agent(production_firing_count));
#endif /* !NO_TIMING_STUFF */

      wme_changes = current_agent(wme_addition_count) + current_agent(wme_removal_count);
      print ("%lu wme changes (%lu additions, %lu removals)\n",
	     wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
#ifdef DETAILED_TIMING_STATS
  print ("    match time: %.3f msec/wm change\n",
         wme_changes ? match_msec/wme_changes : 0.0);
#endif

      print ("WM size: %lu current, %.3f mean, %lu maximum\n",
	     current_agent(num_wmes_in_rete),
	     (current_agent(num_wm_sizes_accumulated) ?
	      (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
	      0.0),
	     current_agent(max_wm_size));

#ifndef NO_TIMING_STUFF
     print ("\n");
     print ("    *** Time/<x> statistics use the total kernel time from a ***\n");
     print ("    *** single kernel timer.  Differences between this value ***\n");
     print ("    *** and the derived total kernel time  are expected. See ***\n");
     print ("    *** help  for the  stats command  for more  information. ***\n");   
#endif
     /* REW: end 28.07.96 */

  get_lexeme();  /* consume "stats", advance to rparen */
  return TRUE;
}

/* -------------------------------------------------------------------
   
         "Object-trace-format" and "Stack-trace-format" Commands

   Syntax:  (see below)
------------------------------------------------------------------- */

char *help_on_object_and_stack_traces[] = {
"Commands: object-trace-format, stack-trace-format",
"",
"Syntax:",
"    (object-trace-format :add {s|o|*} [object-name] \"format-string\")",
"    (object-trace-format :remove {s|o|*} [object-name])",
"    (object-trace-format)",
"    (stack-trace-format :add {s|o|*} [ps-name] \"format-string\")",
"    (stack-trace-format :remove {s|o|*} [ps-name])",
"    (stack-trace-format)",
"",
"Object trace formats control how Soar prints an object--e.g., a certain",
"operator, problem-space, etc.  (This is like trace-attributes in Soar 5.)",
"Stack trace formats control how Soar prints its context stack selections",
"in 'watch 0' and 'pgs' printouts.  You specify a trace format by indicating",
"two things:",
"  - a format string, indicating the printout format to be used",
"  - what things this format string can be applied to",
"",
"The format string can be any string in quotation marks.  Certain 'escape",
"sequences' can be used within the string; for example, '%dc' means print",
"the current decision cycle number.  For a list of escape sequences, see",
"(help trace-format-escapes).",
"",
"There are two ways to restrict what objects a format string applies to.  The",
"{s|o|*} argument restricts the types of objects:  's' indicates that the",
"format only applies to states; 'o' means it only applies to operators;",
"and '*' means it applies to any type of object.  The [object-name]",
"argument (for object trace formats), if given, means it only applies to",
"objects with that ^name.  The [ps-name] argument (for stack trace formats)",
"means it only applies within problem spaces with that ^name.",
"",
"With an :add argument, these commands add new trace formats (replacing any",
"existing ones with identical applicability conditions).  With a :remove",
"argument, they remove trace formats with the given applicability conditions.",
"With no arguments, they print out all current trace formats.",
"",
"See also:  trace-format-escapes",
0 };


/* ----------------------------------------------------------------------

             Help Screen for Trace Format Escape Sequences

---------------------------------------------------------------------- */

char *help_on_trace_format_escapes[] = {
"The following escape sequences can be used within trace format strings:",
"",
"%%, %[, %] - print a percent sign, left bracket, or right bracket.",
"%cs, %co - print the current state or operator using the appropriate object",
"     trace format.  (These are only meaningful in stack traces, not object",
"     traces.)",
"%dc, %ec - print the current decision cycle number or elaboration cycle",
"     number.  These are only meaningful in stack traces, not object traces;",
"     furthermore, they are not meaningful in stack traces produced by the",
"     (pgs) command.  In these cases, nothing is printed.",
"%sd - print the current subgoal depth (0=top level).  This is meaningful",
"     only in stack traces, not object traces.",
"%rsd[pattern] - repeat (subgoal depth) times: print the given pattern.  This",
"     is meaningful only in stack traces, not object traces.",
"%left[num,pattern], %right[num,pattern] - print the given pattern, left or",
"     right justified in a field of num spaces.",
"%id - print the identifier of the current object.",
"%v[foo] - print the value(s) of attribute ^foo on the current object.  If",
"     there is no ^foo on the current object, nothing is printed.",
"%v[foo.bar.baz] - same as the above, only follow the given attribute path",
"     to get the value(s).",
"%v[*] - print all values of all attributes on the current object.",
"%o[args] - same as %v, except that if the value is an identifier, it is",
"     printed using the appropriate object trace format",
"%av[args] - same as %v, except the printed value is preceeded with",
"     \"^attr \" to indicate the attribute name.",
"%ao[args] - a combination of the above two.",
"%ifdef[pattern] - print the given pattern if and only if all escape",
"     sequences inside it are \"meaningful\" or \"well-defined.\"  For",
"     example, \"%ifdef[foo has value: %v[foo]]\" will print nothing if",
"     there is no ^foo on the current object.",
"",
"See also:  object-trace-format, stack-trace-format",
0 };


/* mvp 5-17-94 */
bool trace_format_interface_routine (void) {
  bool stack_trace;
  int type_restriction;
  Symbol *name_restriction;
  bool remove;

  /* --- set stack_trace depending on which command name was given --- */
  stack_trace = FALSE;
  if (! strcmp (current_agent(lexeme).string, "stack-trace-format")) stack_trace = TRUE;
  get_lexeme();  /* consume command name */

  /* --- if no args, print all trace formats of that type --- */
  if (current_agent(lexeme).type==R_PAREN_LEXEME) {
    print_all_trace_formats (stack_trace);
    return TRUE;
  }

  /* --- first argument must be either :add or :remove --- */
  remove = FALSE;
  if (! strcmp (current_agent(lexeme).string, ":add")) remove = FALSE;
  else if (! strcmp (current_agent(lexeme).string, ":remove")) remove = TRUE;
  else {
    print ("Expected :add or :remove in trace format command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  get_lexeme();  /* consume :add or :remove */

  /* --- read second argument: s, o, or '*' --- */

  if (! strcmp (current_agent(lexeme).string, "s"))
    type_restriction = FOR_STATES_TF;
  else if (! strcmp (current_agent(lexeme).string, "o"))
    type_restriction = FOR_OPERATORS_TF;
  else if (! strcmp (current_agent(lexeme).string, "*"))
    type_restriction = FOR_ANYTHING_TF;
  else {
    print ("Expected s, o, or * in trace format command\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
  get_lexeme();  /* consume *|s|o */      

  /* --- read optional name restriction --- */
  if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) {
    name_restriction = make_sym_constant (current_agent(lexeme).string);
    get_lexeme();
  } else {
    name_restriction = NIL;
  }

  /* --- finally, execute the command --- */
  if (remove) {
    remove_trace_format (stack_trace, type_restriction, name_restriction);
  } else {
    if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
      print ("Expected string in quotes for trace format to add\n");
      print_location_of_most_recent_lexeme();
      if (name_restriction) symbol_remove_ref (name_restriction);
      return FALSE;
    }
    add_trace_format (stack_trace, type_restriction, name_restriction, 
                      current_agent(lexeme).string);
    /* MVP 7-5-94 */
    get_lexeme ();
  }
  if (name_restriction) symbol_remove_ref (name_restriction);
  return TRUE;
}

/* -------------------------------------------------------------------
   
                          "Echo" Command

   Syntax:  (echo [echoed argument ...])
------------------------------------------------------------------- */

char *help_on_echo[] = {
"Command: echo",
"",
"Syntax: (echo [echoed argument ...])",
"",
"This command echos its arguments to the screen.",
0 };

bool echo_interface_routine (void) {
  int paren_depth;

  paren_depth = current_lexer_parentheses_level() - 1;
  
  get_lexeme();

  while(current_agent(lexeme).type != R_PAREN_LEXEME 
        || current_lexer_parentheses_level() != paren_depth) {
    print ("%s", current_agent(lexeme).string);
    get_lexeme();
  }

  return TRUE;
}


/* -------------------------------------------------------------------
   
                          "Version" Command

   Syntax:  (version)
------------------------------------------------------------------- */

char *help_on_version[] = {
"Command: version",
"",
"Syntax: (version)",
"",
"This command returns the version number of soar.",
0 };

bool version_interface_routine (void) {
  get_lexeme();
  print ("Soar version: %s\n", soar_version_string);
  return TRUE;
}

/* -------------------------------------------------------------------
                          "multi-attribute" command

  Syntax: (multi-attribute symbol value)
------------------------------------------------------------------- */

char *help_on_multi_attribute[] = {
"Command: multi-attribute",
"",
"Syntax: (multi-attribute symbol [value])",
"",
"Declare 'symbol' as a multi-attribute.  If value isn't given, then",
"100 is used as the default.",
0};

void old_add_multi_attribute_or_change_value(char *sym, long val) {
  multi_attribute *m = current_agent(multi_attributes);
  Symbol *s = make_sym_constant(sym);

  while(m) {
    if(m->symbol == s) {
      m->value = val;
      symbol_remove_ref(s);
      return;
    }
    m = m->next;
  }
  /* sym wasn't in the table if we get here, so add it */
  m = (multi_attribute *)allocate_memory(sizeof(multi_attribute),
                                         MISCELLANEOUS_MEM_USAGE);
  m->value = val;
  m->symbol = s;
  m->next = current_agent(multi_attributes);
  current_agent(multi_attributes) = m;
}

bool multi_attribute_interface_routine(void) {
  char symbol_name[MAX_LEXEME_LENGTH+1];

  get_lexeme(); /* consume "multi-attribute" */

  if(current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
    strcpy(symbol_name, current_agent(lexeme).string);
    get_lexeme();
    if(current_agent(lexeme).type == INT_CONSTANT_LEXEME) {
      if(current_agent(lexeme).int_val > 1) {
        old_add_multi_attribute_or_change_value(symbol_name,
                                            current_agent(lexeme).int_val);
        get_lexeme();
        return TRUE;
      } else {
        print("Value must be greater than 1.\n");
      }
    } else {
        old_add_multi_attribute_or_change_value(symbol_name,
                                            100);
        return TRUE;
    }
  } else {
    print ("Expected symbolic constant.\n");
  }
  print_location_of_most_recent_lexeme();
  if (strcmp(current_agent(lexeme).string, ")")) {
    get_lexeme();
  }
  return FALSE;
}

/* -------------------------------------------------------------------
   
        "Fastsave-productions" and "fastload-productions" Commands

   Syntax:  (fastsave-productions "filename")
            (fastload-productions "filename")
------------------------------------------------------------------- */

char *help_on_fastsaveload[] = {
"Commands: fastsave-productions, fastload-productions",
"",
#ifdef __SC__
"Syntax: (fastsave-productions \"filename\")",
"        (fastload-productions \"filename\")",
#else
"Syntax: (fastsave-productions filename)",
"        (fastload-productions filename)",
#endif
"",
"These commands provide a fast way of saving all productions to a file and",
"reloading them later.  Fastsave can only be used after an init-soar (or",
"whenever there are no justifications present).  Fastload can only be",
"used in a \"fresh\" Soar (no productions and no WMEs).",
#ifdef UNIX
"",
"Unix systems only:  If the filename ends in \".Z\", Soar automatically pipes",
"it through compress or zcat to save disk space.",
#endif
0 };

bool fastsave_productions_interface_routine (void) {
  char *expanded_string;
  char *tilde_expand();
  char load_file_name[MAX_LEXEME_LENGTH+1];
  FILE *f;
  bool result;

  if (current_agent(all_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]) {
    print ("Can't do a fastsave while there are justifications present.\n");
    print ("Do an (init-soar) to get rid of them first.\n");
    return FALSE;
  }
  
  get_lexeme();  /* consume "fastsave...", advance to quoted file name */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for filename for fast save\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif
  
  strcpy (load_file_name, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (load_file_name, current_agent(lexeme).string);
    get_lexeme ();
  }
  expanded_string = tilde_expand(load_file_name);
  chdir(current_agent(top_dir_stack)->directory);

#ifdef UNIX
  if (!strcmp(expanded_string + strlen(expanded_string) - 2, ".Z")) {
    char command[MAXPATHLEN+100];
    /* --- prepend "compress > " to the filename --- */
    strcpy (command, "compress > ");
    strcat (command, expanded_string);
    f = (FILE *) popen (command, "w");
    if (! f) {
      /* --- error when opening the pipe or file --- */
      print ("Error: unable to run 'compress' and/or open file %s\n",
             expanded_string);
      free((void *) expanded_string);
      return FALSE;
    }
    result = save_rete_net (f);
    pclose (f);
  } else 
#endif
  {
    f = fopen (expanded_string,"w");
    if (! f) {
      /* --- error when opening the file --- */
      print ("Error: unable to open file %s\n",expanded_string);
      free((void *) expanded_string);
      return FALSE;
    }
    result = save_rete_net (f);
    fclose (f);
  }
  
  free((void *) expanded_string);
  return result;
}

bool fastload_productions_interface_routine (void) {
  char *expanded_string;
  char *tilde_expand();
  char load_file_name[MAX_LEXEME_LENGTH+1];
  int i;
  FILE *f;
  bool result;
 
  /* --- check for empty system --- */
  if (current_agent(all_wmes_in_rete)) {
    print ("Can't use fastload unless working memory is empty.\n");
    return FALSE;
  }
  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if (current_agent(num_productions_of_type)[i]) {
      print ("Can't use fastload unless production memory is empty.\n");
      return FALSE;
    }
  
  get_lexeme();  /* consume "fastload...", advance to quoted file name */

#ifndef UNIX
  if (current_agent(lexeme).type!=QUOTED_STRING_LEXEME) {
    print ("Expected string in quotes for filename for fast load\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
  }
#endif

  strcpy (load_file_name, "\0");
  while (TRUE) {
    if (current_agent(lexeme).type==R_PAREN_LEXEME)
      break;
    strcat (load_file_name, current_agent(lexeme).string);
    get_lexeme ();
  }
  expanded_string = tilde_expand(load_file_name);
  chdir(current_agent(top_dir_stack)->directory);

#ifdef UNIX
  if (!strcmp(expanded_string + strlen(expanded_string) - 2, ".Z")) {
    char command[MAXPATHLEN+100];
    /* --- prepend "zcat " to the filename --- */
    strcpy (command, "zcat ");
    strcat (command, expanded_string);
    f = (FILE *) popen (command, "r");
    if (! f) {
      /* --- error when opening the pipe or file --- */
      print ("Error: unable to run 'zcat' and/or open file %s\n",
             expanded_string);
      free((void *) expanded_string);
      return FALSE;
    }
    result = load_rete_net (f);
    pclose (f);
  } else 
#endif
  {
    f = fopen (expanded_string,"r");
    if (! f) {
      /* --- error when opening the file --- */
      print ("Error: unable to open file %s\n",expanded_string);
      free((void *) expanded_string);
      return FALSE;
    }
    result = load_rete_net (f);
    fclose (f);
  }
  
  free((void *) expanded_string);
  return result;
}

/* ===================================================================
                   O-support-mode command (experimental)
   Syntax:  (o-support-mode {0 | 1 | 2 | 3})
=================================================================== */

bool o_support_mode_interface_routine (void) {
  get_lexeme();
  if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) {
    if ( (current_agent(lexeme).int_val >= 0) &&
         (current_agent(lexeme).int_val <= 3) ) {
      current_agent(o_support_calculation_type) =
        current_agent(lexeme).int_val;
      get_lexeme();
    }
  }
  print ("O support mode: %d.\n", current_agent(o_support_calculation_type));
  return TRUE;
}


/* ===================================================================
              Attribute-preferences-mode command (experimental)
   Syntax:  (attribute-preferences-mode {0 | 1 | 2})
=================================================================== */

bool attribute_preferences_mode_interface_routine (void) {
  get_lexeme();
  if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) {
    if ( (current_agent(lexeme).int_val >= 0) &&
         (current_agent(lexeme).int_val <= 2) ) {
      current_agent(attribute_preferences_mode) =
        current_agent(lexeme).int_val;
      get_lexeme();
    }
  }
  print ("Attribute preferences mode: %d.\n", current_agent(attribute_preferences_mode));
  return TRUE;
}

/* ===================================================================
   
                     Built-In Debugging Commands

=================================================================== */

bool internal_symbols_interface_routine (void) {
  print_internal_symbols ();
  get_lexeme();  /* consume "internal-symbols", advance to rparen */
  return TRUE;
}

/* ===================================================================
   
             Initialize Built-In User Interface Commands

=================================================================== */

void init_built_in_commands (void) {

  add_command ("help", help_interface_routine);
  add_command ("?", help_interface_routine);
  add_help ("help", help_on_help);
  add_help ("?", help_on_help);

/* AGR REW1 begin */
  add_command ("input-period", input_period_interface_routine);
  add_help ("input-period", help_on_input_period);
/* AGR REW1 end */

  add_command ("list-help-topics", list_help_topics_interface_routine);
  add_help ("list-help-topics", help_on_list_help_topics);

  add_command ("print-all-help", print_all_help_interface_routine);
  add_help ("print-all-help", help_on_print_all_help);

  add_command ("exit", exit_interface_routine);
  add_command ("quit", exit_interface_routine);
  add_help ("exit", help_on_exit);
  add_help ("quit", help_on_exit);

  add_command ("log", log_interface_routine);
  add_help ("log", help_on_log);

  add_command ("load", load_interface_routine);
  add_help ("load", help_on_load);

  add_command ("chdir", chdir_interface_routine);
  add_command ("cd", chdir_interface_routine);
  add_help ("chdir", help_on_chdir);
  add_help ("cd", help_on_chdir);

  add_command ("pwd", pwd_interface_routine);
  add_help ("pwd", help_on_pwd);

/* AGR 568 begin */
  add_command ("popd", popd_interface_routine);
  add_help ("popd", help_on_popd);
  add_command ("pushd", pushd_interface_routine);
  add_help ("pushd", help_on_pushd);
  add_command ("dirs", dirs_interface_routine);
  add_help ("dirs", help_on_dirs);

  add_command ("alias", alias_interface_routine);
  add_help ("alias", help_on_alias);
  add_command ("unalias", unalias_interface_routine);
  add_help ("unalias", help_on_unalias);
/* AGR 568 end */

  add_command("print-alias", print_alias_interface_routine);  /* AGR 627 */
  add_help("print-alias", help_on_print_alias);  /* AGR 627 */

  add_command ("pgs", pgs_interface_routine);
  add_help ("pgs", help_on_pgs);

  add_command ("pgso", pgso_interface_routine);
  add_help ("pgso", help_on_pgso);

  add_command ("excise", excise_interface_routine);
  add_help ("excise", help_on_excise);
  add_command ("excise-chunks", excise_chunks_interface_routine);
  add_help ("excise-chunks", help_on_excise_chunks);
  add_command ("excise-task", excise_task_interface_routine);
  add_help ("excise-task", help_on_excise_task);
  add_command ("excise-all", excise_all_interface_routine);
  add_help ("excise-all", help_on_excise_all);

  add_command ("matches", matches_interface_routine);
  add_help ("matches", help_on_matches);

  add_command ("default-wme-depth", default_wme_depth_interface_routine);
  add_help ("default-wme-depth", help_on_default_wme_depth);
  add_command ("print", print_interface_routine);
  add_command ("p", print_interface_routine);
  add_command ("spr", print_interface_routine);
  add_command ("wm", print_interface_routine);
  add_help ("print", help_on_print);
  add_help ("p", help_on_print);
  add_help ("spr", help_on_print);
  add_help ("wm", help_on_print);

  add_command ("go", go_interface_routine);
  add_help ("go", help_on_go);
  add_command ("d", d_interface_routine);
  add_help ("d", help_on_d);
  add_command ("run", run_interface_routine);
  add_command ("r", run_interface_routine);
  add_help ("run", help_on_run);
  add_help ("r", help_on_run);

  add_command ("init-soar", init_soar_interface_routine);
  add_help ("init-soar", help_on_init_soar);  

  add_command ("learn", learn_interface_routine);
  add_help ("learn", help_on_learn);

#ifdef ATTENTION_LAPSE
  /* RMJ */
  add_command ("attention-lapse", attention_lapse_interface_routine);
  add_help ("attention-lapse", help_on_attention_lapse);
#endif  /* ATTENTION_LAPSE */

  add_command ("chunk-free-problem-spaces",
               chunk_free_problem_spaces_interface_routine);
  add_help ("chunk-free-problem-spaces", help_on_chunk_free_problem_spaces);

/* AGR MVL1 begin */
  add_command ("chunky-problem-spaces",
               chunky_problem_spaces_interface_routine);
  add_help ("chunky-problem-spaces", help_on_chunky_problem_spaces);
/* AGR MVL1 end */

  add_command ("watch", watch_interface_routine);
  add_help ("watch", help_on_watch);
  add_help ("watch-keywords", help_on_watch_keywords);
  add_help ("watch-levels", help_on_watch_levels);

  add_command ("preferences", preferences_interface_routine);
  add_help ("preferences", help_on_preferences);

  add_command ("ms", ms_interface_routine);
  add_help ("ms", help_on_ms);

  add_command ("max-elaborations", max_elaborations_interface_routine);
  add_help ("max-elaborations", help_on_max_elaborations);

  add_command ("max-chunks", max_chunks_interface_routine);
  add_help ("max-chunks", help_on_max_chunks);
  add_command("reset", reset_interface_routine);
  add_help("reset", help_on_reset);
  add_command("load-errors", load_errors_interface_routine);
  add_help("load-errors", help_on_load_errors);

  add_command ("user-select", user_select_interface_routine);
  add_help ("user-select", help_on_user_select);

  add_command ("soarnews", soarnews_interface_routine);
  add_help ("soarnews", help_on_soarnews);

  add_command ("list-productions", list_productions_interface_routine);
  add_command ("list-chunks", list_productions_interface_routine);
  add_command ("list-justifications", list_productions_interface_routine);
  add_help ("list-productions", help_on_list_productions);
  add_help ("list-chunks", help_on_list_productions);
  add_help ("list-justifications", help_on_list_productions);

  add_command ("add-wme", add_wme_interface_routine);
  add_command ("remove-wme", remove_wme_interface_routine);
  add_help ("add-wme", help_on_add_or_remove_wme);
  add_help ("remove-wme", help_on_add_or_remove_wme);

  add_command ("firing-counts", firing_counts_interface_routine);
  add_help ("firing-counts", help_on_firing_counts);

  add_command ("pwatch", pwatch_interface_routine);
  add_command ("unpwatch", unpwatch_interface_routine);
  add_help ("pwatch", help_on_pwatch_and_unpwatch);
  add_help ("unpwatch", help_on_pwatch_and_unpwatch);

  add_command ("warnings", warnings_interface_routine);
  add_help ("warnings", help_on_warnings);
  
#ifndef NO_TIME_INTERFACE
#if !defined(WIN32) && !defined(MACINTOSH)
  add_command ("time", time_interface_routine);
  add_help ("time", help_on_time);
#endif
#endif

  add_command ("memory-stats", memory_stats_interface_routine);
  add_help ("memory-stats", help_on_memory_stats);
  add_command ("rete-stats", rete_stats_interface_routine);
  add_help ("rete-stats", help_on_rete_stats);
  add_command ("stats", stats_interface_routine);
  add_command ("print-stats", stats_interface_routine);
  add_help ("stats", help_on_stats);
  add_help ("print-stats", help_on_stats);

  add_command ("object-trace-format", trace_format_interface_routine);
  add_command ("stack-trace-format", trace_format_interface_routine);
  add_help ("object-trace-format", help_on_object_and_stack_traces);
  add_help ("stack-trace-format", help_on_object_and_stack_traces);
  add_help ("trace-format-escapes", help_on_trace_format_escapes);

  add_command ("echo", echo_interface_routine);
  add_help ("echo", help_on_echo);

  add_command ("version", version_interface_routine);
  add_help ("version", help_on_version);

  add_command ("multi-attribute", multi_attribute_interface_routine);
  add_help ("multi-attribute", help_on_multi_attribute);

/* AGR 649 begin */
  add_command ("memories", memories_interface_routine);
  add_help ("memories", help_on_memories);
/* AGR 649 end */

  add_help("explain",help_on_explain);
  add_command("explain",explain_interface_routine);

  add_command ("setvar", setvar_interface_routine);
  add_help ("setvar", help_on_setvar);

  add_command ("unsetvar", unsetvar_interface_routine);
  add_help ("unsetvar", help_on_unsetvar);

  add_command ("if", if_interface_routine);
  add_help ("if", help_on_if);

  add_command ("if-not", if_not_interface_routine);
  add_help ("if-not", help_on_if_not);

  /* MVP end */

  /* MVP 7-1-94 */
  add_command ("pf", pf_interface_routine);
  add_help ("pf", help_on_pf);

  init_multi_agent_built_in_commands();

#ifdef __SC__
  add_command ("ls", ls_interface_routine);
  add_help ("ls", help_on_ls_or_lf);
  add_command ("lf", ls_interface_routine);
  add_help ("lf", help_on_ls_or_lf);
#endif

  /* RBD 2-19-95 */
  add_command ("fastsave-productions", fastsave_productions_interface_routine);
  add_command ("fastload-productions", fastload_productions_interface_routine);
  add_help ("fastsave-productions", help_on_fastsaveload);
  add_help ("fastload-productions", help_on_fastsaveload);

  add_command ("o-support-mode", o_support_mode_interface_routine);
  add_command ("attribute-preferences-mode", attribute_preferences_mode_interface_routine);

  /* --- additional undocumented commands for debugging purposes --- */
  add_command ("internal-symbols", internal_symbols_interface_routine);

  add_command ("sp", sp_interface_routine);
  add_help ("sp", help_on_sp);

}

#ifdef ATTENTION_LAPSE
/* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.  But
   we need to put a placeholder function here just to be safe.
*/

#ifndef USE_TCL
long init_lapse_duration(struct timeval *tv) {
   return 0;
}
#endif

#endif

