/*************************************************************************
 *
 *  file:  lexer.c
 *
 * =======================================================================
 *
 *                              lexer.c
 *
 *  The lexer reads files and returns a stream of lexemes.  Get_lexeme() is
 *  the main routine; it looks for the next lexeme in the input, and stores
 *  it in the global variable "lexeme".  See the structure definition below.
 *
 *  Restrictions:  the lexer cannot read individual input lines longer than
 *  MAX_LEXER_LINE_LENGTH characters.  Thus, a single lexeme can't be longer
 *  than that either.
 *
 *  The lexer maintains a stack of files being read, in order to handle nested
 *  loads.  Start_lex_from_file() and stop_lex_from_file() push and pop the
 *  stack.  Immediately after start_lex_from_file(), the current lexeme (global
 *  variable) is undefined.  Immediately after stop_lex_from_file(), the 
 *  current lexeme is automatically restored to whatever it was just before
 *  the corresponding start_lex_from_file() call.
 *  
 *  Determine_possible_symbol_types_for_string() is a utility routine which
 *  figures out what kind(s) of symbol a given string could represent.
 *  
 *  Print_location_of_most_recent_lexeme() is used to print an indication
 *  of where a parser error occurred.  It tries to print out the current
 *  source line with a pointer to where the error was detected.
 *  
 *  Current_lexer_parentheses_level() returns the current level of parentheses
 *  nesting (0 means no open paren's have been encountered).
 *  Skip_ahead_to_balanced_parentheses() eats lexemes until the appropriate
 *  closing paren is found (0 means eat until back at the top level).
 *  
 *  Fake_rparen_at_next_end_of_line() tells the lexer to insert a fake
 *  R_PAREN_LEXEME token the next time it reaches the end of a line.
 *  
 *  Set_lexer_allow_ids() tells the lexer whether to allow identifiers to
 *  be read.  If FALSE, things that look like identifiers will be returned
 *  as SYM_CONSTANT_LEXEME's instead.
 *
 *  BUGBUG There are still problems with Soar not being very friendly
 *  when users have typos in productions, particularly with mismatched
 *  braces and parens.  see also parser.c
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
/* ======================================================================
                             lexer.c

    See comments in soarkernel.h for an overview.
   ====================================================================== */

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "soarkernel.h"

bool constituent_char[256];   /* is the character a symbol constituent? */
bool whitespace[256];         /* is the character whitespace? */
bool number_starters[256];    /* could the character initiate a number? */

/* ======================================================================
                       Start/Stop Lex from File
                       
  The lexer maintains a stack of files being read, in order to handle nested
  loads.  Start_lex_from_file() and stop_lex_from_file() push and pop the
  stack.  Immediately after start_lex_from_file(), the current lexeme (agent
  variable) is undefined.  Immediately after stop_lex_from_file(), the 
  current lexeme is automatically restored to whatever it was just before
  the corresponding start_lex_from_file() call.
====================================================================== */

void start_lex_from_file (char *filename, FILE *already_opened_file) {
  lexer_source_file *lsf;

  lsf = allocate_memory (sizeof(lexer_source_file),
                         MISCELLANEOUS_MEM_USAGE);
  lsf->saved_lexeme = current_agent(lexeme);
  lsf->saved_current_char = current_agent(current_char);
  lsf->parent_file = current_agent(current_file);
  current_agent(current_file) = lsf;
  lsf->filename = make_memory_block_for_string (filename);
  lsf->file = already_opened_file;
  lsf->fake_rparen_at_eol = FALSE;
  lsf->allow_ids = TRUE;
  lsf->parentheses_level = 0;
  lsf->column_of_start_of_last_lexeme = 0;
  lsf->line_of_start_of_last_lexeme = 0;
  lsf->current_line = 0;
  lsf->current_column = 0;
  lsf->buffer[0] = 0;
  current_agent(current_char) = ' ';   /* whitespace--to force immediate read of first line */
}

void stop_lex_from_file (void) {
  lexer_source_file *lsf;

  if (reading_from_top_level()) {
    print ("Internal error: tried to stop_lex_from_file at top level\n");
    return;
  }
  lsf = current_agent(current_file);
  current_agent(current_file) = current_agent(current_file)->parent_file;
  current_agent(current_char) = lsf->saved_current_char;
  current_agent(lexeme) = lsf->saved_lexeme;

  free_memory_block_for_string (lsf->filename);
  free_memory (lsf, MISCELLANEOUS_MEM_USAGE);
}

/* ======================================================================
                             Get next char

  Get_next_char() gets the next character from the current input file and
  puts it into the agent variable current_char.
====================================================================== */

void get_next_char (void) {
  char *s;

#ifdef USE_TCL
  /* Soar-Bugs #54, TMH */
  if ( current_agent(alternate_input_exit) &&
      (current_agent(alternate_input_string) == NULL) &&
      (current_agent(alternate_input_suffix) == NULL)   ) {
    current_agent(current_char) = EOF_AS_CHAR;
    control_c_handler(0);
    return;
  }
#endif

  if (current_agent(using_input_string)) {
    if (*(current_agent(input_string))!='\0')
      current_agent(current_char) = *(current_agent(input_string)++);
    return;
  }

#ifdef USE_X_DISPLAY
  if (x_input_buffer != NIL) {
    current_agent(current_char) = x_input_buffer[x_input_buffer_index++];
    if (current_agent(current_char) == '\n') {
      x_input_buffer = NIL;
    }
  } else {
    current_agent(current_char) = current_agent(current_file)->buffer 
                           [current_agent(current_file)->current_column++];
  }
#elif defined(USE_TCL)
  if (current_agent(alternate_input_string) != NULL)
    {
      current_agent(current_char) = *current_agent(alternate_input_string)++;

      if (current_agent(current_char) == '\0') 
        {
          current_agent(alternate_input_string) = NIL;
          current_agent(current_char) = 
            *current_agent(alternate_input_suffix)++;
        }
    }
  else if (current_agent(alternate_input_suffix) != NULL)
    {
      current_agent(current_char) = *current_agent(alternate_input_suffix)++;

      if (current_agent(current_char) == '\0') 
        {
          current_agent(alternate_input_suffix) = NIL;

          /* Soar-Bugs #54, TMH */
          if ( current_agent(alternate_input_exit) ) {
            current_agent(current_char) = EOF_AS_CHAR;
            control_c_handler(0);
            return;
	  }

          current_agent(current_char) = current_agent(current_file)->buffer 
            [current_agent(current_file)->current_column++];
        }
    } 
  else 
    {
      current_agent(current_char) = current_agent(current_file)->buffer 
        [current_agent(current_file)->current_column++];
    }
#elif _WINDOWS
  if (current_agent(current_file)->file==stdin) { 
    switch (current_agent(current_line)[current_agent(current_line_index)]) {
      case EOF_AS_CHAR:
      case 0:
			if (!AvailableWindowCommand()) {
	  			current_agent(current_line)[current_agent(current_line_index)=0]=current_agent(current_char)=EOF_AS_CHAR;
		 	    break;
			} else {
	  			s=GetWindowCommand();
	  			strcpy(current_agent(current_line),s+1);
	  			if (current_agent(print_prompt_flag)=(s[0]=='1')) 
			    print("%s",s+1);
	  			free(s);
	  			current_agent(current_line_index)=0;
			}
      default:
			current_agent(current_char)=current_agent(current_line)[current_agent(current_line_index)++];
			break;
    }
  } else 
    current_agent(current_char) = current_agent(current_file)->buffer 
			   [current_agent(current_file)->current_column++];
#else
  current_agent(current_char) = current_agent(current_file)->buffer 
                           [current_agent(current_file)->current_column++];
#endif

  if (current_agent(current_char)) return;

  if ((current_agent(current_file)->current_column == BUFSIZE) &&
      (current_agent(current_file)->buffer[BUFSIZE-2] != '\n') &&
      (current_agent(current_file)->buffer[BUFSIZE-2] != EOF_AS_CHAR)) {
    char msg[512];
    sprintf (msg,
	     "lexer.c: Error:  line too long (max allowed is %d chars)\nFile %s, line %lu\n",
	     MAX_LEXER_LINE_LENGTH, current_agent(current_file)->filename,
	     current_agent(current_file)->current_line);
    abort_with_fatal_error(msg);
  }

  s = fgets (current_agent(current_file)->buffer, BUFSIZE, current_agent(current_file)->file);

  if (s) {
    current_agent(current_file)->current_line++;
    if (reading_from_top_level()) {
      tell_printer_that_output_column_has_been_reset ();
      if (current_agent(logging_to_file))
        print_string_to_log_file_only (current_agent(current_file)->buffer);
    }
  } else {
    /* s==NIL means immediate eof encountered or read error occurred */
    if (! feof(current_agent(current_file)->file)) {
      if(reading_from_top_level()) {
#ifndef _WINDOWS
        control_c_handler(0);  /* AGR 581 */
#endif
        return;
      } else {
        print ("I/O error while reading file %s; ignoring the rest of it.\n",
               current_agent(current_file)->filename);
      }
    }
    current_agent(current_file)->buffer[0] = EOF_AS_CHAR;
    current_agent(current_file)->buffer[1] = 0;
  }
  current_agent(current_char) = current_agent(current_file)->buffer[0];
  current_agent(current_file)->current_column = 1;
}

/* ======================================================================

                         Lexer Utility Routines

====================================================================== */

#define record_position_of_start_of_lexeme() { \
  current_agent(current_file)->column_of_start_of_last_lexeme = \
    current_agent(current_file)->current_column - 1; \
  current_agent(current_file)->line_of_start_of_last_lexeme = \
    current_agent(current_file)->current_line; }

/*  redefined for Soar 7, want case-sensitivity to match Tcl.  KJC 5/96 
#define store_and_advance() { \
  current_agent(lexeme).string[current_agent(lexeme).length++] = (isupper((char)current_agent(current_char)) ? \
                                    tolower((char)current_agent(current_char)) : \
                                    (char)current_agent(current_char)); \
  get_next_char(); }
*/
#define store_and_advance() { \
  current_agent(lexeme).string[current_agent(lexeme).length++] = \
    (char)current_agent(current_char); \
  get_next_char(); }

#define finish() { current_agent(lexeme).string[current_agent(lexeme).length]=0; }

void read_constituent_string (void) {
#ifdef __SC__
	char *buf;
	int i,len;
#endif

  while ((current_agent(current_char)!=EOF_AS_CHAR) &&
         constituent_char[(unsigned char)current_agent(current_char)])
    store_and_advance();
  finish();
}  

void read_rest_of_floating_point_number (void) {
  /* --- at entry, current_char=="."; we read the "." and rest of number --- */
  store_and_advance();
  while (isdigit(current_agent(current_char))) store_and_advance(); /* string of digits */
  if ((current_agent(current_char)=='e')||(current_agent(current_char)=='E')) {
    store_and_advance();                             /* E */
    if ((current_agent(current_char)=='+')||(current_agent(current_char)=='-'))
      store_and_advance();                       /* optional leading + or - */
    while (isdigit(current_agent(current_char))) store_and_advance(); /* string of digits */
  }
  finish();

#ifdef __SC__
  if (strcmp("soar>",current_agent(lexeme).string)) { /* if the lexeme doesn't equal "soar>" */
  	if (!(strncmp("soar>",current_agent(lexeme).string,5))) { /* but the first 5 chars are "soar>" */
		/* then SIOW messed up so ignore the "soar>" */
	   buf = (char *)allocate_memory((len=(strlen(current_agent(lexeme).string)+1))*sizeof(char),STRING_MEM_USAGE);
	   for (i=0;i<=len;i++) {
	   	   buf[i] = current_agent(lexeme).string[i];
	   }
	   for (i=5;i<=len;i++) {
	   	   current_agent(lexeme).string[i-5] = buf[i];
	   }
	   free_memory_block_for_string(buf);
	}
  }
#endif
}

/* --- BUGBUG: these routines are here because the ANSI routine strtod() isn't
   available at CMU and the ANSI routine strtoul() isn't at ISI --- */
#if !defined(__NeXT__) && !defined(__linux__) && !defined(MACINTOSH)
extern double atof();
#endif /* #ifndef __NeXT__ */

double my_strtod (char *ch, char **p, int base) {
  /* BUGBUG without ANSI's strtod(), there's no way to check for floating
     point overflow here.  If someone types "1.5E2000" weird things could
     happen. */
  return atof(ch);
}

unsigned long my_strtoul (char *ch, char **p, int base) {
  long result;
  
  errno = 0;
  result = strtol (ch,p,base);
  if (errno) return 0;
  if (result < 0) {
    errno = ERANGE;
    return 0;
  }
  return (unsigned long) result;
}

void determine_type_of_constituent_string (void) {
  bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
  bool rereadable;

  determine_possible_symbol_types_for_string (current_agent(lexeme).string,
                                              current_agent(lexeme).length,
                                              &possible_id,
                                              &possible_var,
                                              &possible_sc,
                                              &possible_ic,
                                              &possible_fc,
                                              &rereadable);

  /* --- check whether it's a variable --- */
  if (possible_var) {
    current_agent(lexeme).type = VARIABLE_LEXEME;
    return;
  }

  /* --- check whether it's an integer --- */
  if (possible_ic) {
    errno = 0;
    current_agent(lexeme).type = INT_CONSTANT_LEXEME;
    current_agent(lexeme).int_val = strtol (current_agent(lexeme).string,NULL,10);
    if (errno) {
      print ("Error: bad integer (probably too large)\n");
      print_location_of_most_recent_lexeme();
      current_agent(lexeme).int_val = 0;
    }
    return;
  }
    
  /* --- check whether it's a floating point number --- */
  if (possible_fc) {
    errno = 0;
    current_agent(lexeme).type = FLOAT_CONSTANT_LEXEME;
    current_agent(lexeme).float_val = (float) my_strtod (current_agent(lexeme).string,NULL,10); 
    if (errno) {
      print ("Error: bad floating point number\n");
      print_location_of_most_recent_lexeme();
      current_agent(lexeme).float_val = 0.0;
    }
    return;
  }
  
  /* --- check if it's an identifier --- */
  if (current_agent(current_file)->allow_ids && possible_id) {
    current_agent(lexeme).id_letter = toupper(current_agent(lexeme).string[0]);
    errno = 0;
    current_agent(lexeme).type = IDENTIFIER_LEXEME;
    current_agent(lexeme).id_number = my_strtoul (&(current_agent(lexeme).string[1]),NULL,10);
    if (errno) {
      print ("Error: bad number for identifier (probably too large)\n");
      print_location_of_most_recent_lexeme();
      current_agent(lexeme).id_number = 0;
    }
    return;
  }

  /* --- otherwise it must be a symbolic constant --- */
  if (possible_sc) {
    current_agent(lexeme).type = SYM_CONSTANT_LEXEME;
    if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM]) {
      if (current_agent(lexeme).string[0] == '<') {
        if (current_agent(lexeme).string[1] == '<') {
           print ("Warning: Possible disjunctive encountered in reading symbolic constant\n");
           print ("         If a disjunctive was intended, add a space after <<\n");
           print ("         If a constant was intended, surround constant with vertical bars\n");
           print_location_of_most_recent_lexeme();
	 } else {
           print ("Warning: Possible variable encountered in reading symbolic constant\n");
           print ("         If a constant was intended, surround constant with vertical bars\n");
           print_location_of_most_recent_lexeme();
         }
      } else {
        if (current_agent(lexeme).string[current_agent(lexeme).length-1] == '>') {
          if (current_agent(lexeme).string[current_agent(lexeme).length-2] == '>') {
           print ("Warning: Possible disjunctive encountered in reading symbolic constant\n");
           print ("         If a disjunctive was intended, add a space before >>\n");
           print ("         If a constant was intended, surround constant with vertical bars\n");
           print_location_of_most_recent_lexeme();
	 } else {
           print ("Warning: Possible variable encountered in reading symbolic constant\n");
           print ("         If a constant was intended, surround constant with vertical bars\n");
           print_location_of_most_recent_lexeme();
         }
	}
      }
    }
    return;
  }

#if defined(USE_TCL)
  current_agent(lexeme).type = QUOTED_STRING_LEXEME;
#else
  char msg[128];
  strcpy (msg, "Internal error: can't determine_type_of_constituent_string\n");
  abort_with_fatal_error(msg);
#endif
}

void do_fake_rparen (void) {
  record_position_of_start_of_lexeme();
  current_agent(lexeme).type = R_PAREN_LEXEME;
  current_agent(lexeme).length = 1;
  current_agent(lexeme).string[0] = ')';
  current_agent(lexeme).string[1] = 0;
  if (current_agent(current_file)->parentheses_level > 0) current_agent(current_file)->parentheses_level--;
  current_agent(current_file)->fake_rparen_at_eol = FALSE;
}

/* ======================================================================
                        Lex such-and-such Routines

  These routines are called from get_lexeme().  Which routine gets called
  depends on the first character of the new lexeme being read.  Each routine's
  job is to finish reading the lexeme and store the necessary items in 
  the agent variable "lexeme".
====================================================================== */

void (*(lexer_routines[256]))(void);

void lex_eof (void) {
  if (current_agent(current_file)->fake_rparen_at_eol) {
    do_fake_rparen();
    return;
  }
  store_and_advance();
  finish();
  current_agent(lexeme).type = EOF_LEXEME;
}

void lex_at (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = AT_LEXEME;
}

void lex_tilde (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = TILDE_LEXEME;
}

void lex_up_arrow (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = UP_ARROW_LEXEME;
}

void lex_lbrace (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = L_BRACE_LEXEME;
}

void lex_rbrace (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = R_BRACE_LEXEME;
}

void lex_exclamation_point (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = EXCLAMATION_POINT_LEXEME;
}

void lex_comma (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = COMMA_LEXEME;
}

void lex_equal (void) {
  /* Lexeme might be "=", or symbol */
  /* Note: this routine relies on = being a constituent character */

  read_constituent_string();
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = EQUAL_LEXEME; return; }
  determine_type_of_constituent_string();
}

void lex_ampersand (void) {
  /* Lexeme might be "&", or symbol */
  /* Note: this routine relies on & being a constituent character */

  read_constituent_string();
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = AMPERSAND_LEXEME; return; }
  determine_type_of_constituent_string();
}

void lex_lparen (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = L_PAREN_LEXEME;
  current_agent(current_file)->parentheses_level++;
}

void lex_rparen (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = R_PAREN_LEXEME;
  if (current_agent(current_file)->parentheses_level > 0) current_agent(current_file)->parentheses_level--;
}

void lex_greater (void) {
  /* Lexeme might be ">", ">=", ">>", or symbol */
  /* Note: this routine relies on =,> being constituent characters */

  read_constituent_string();
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = GREATER_LEXEME; return; }
  if (current_agent(lexeme).length==2) {
    if (current_agent(lexeme).string[1]=='>') { current_agent(lexeme).type = GREATER_GREATER_LEXEME; return;}
    if (current_agent(lexeme).string[1]=='=') { current_agent(lexeme).type = GREATER_EQUAL_LEXEME; return; }
  }
  determine_type_of_constituent_string();
}
    
void lex_less (void) {
  /* Lexeme might be "<", "<=", "<=>", "<>", "<<", or variable */
  /* Note: this routine relies on =,<,> being constituent characters */

  read_constituent_string();
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = LESS_LEXEME; return; }
  if (current_agent(lexeme).length==2) {
    if (current_agent(lexeme).string[1]=='>') { current_agent(lexeme).type = NOT_EQUAL_LEXEME; return; }
    if (current_agent(lexeme).string[1]=='=') { current_agent(lexeme).type = LESS_EQUAL_LEXEME; return; }
    if (current_agent(lexeme).string[1]=='<') { current_agent(lexeme).type = LESS_LESS_LEXEME; return; }
  }
  if (current_agent(lexeme).length==3) {
    if ((current_agent(lexeme).string[1]=='=')&&(current_agent(lexeme).string[2]=='>'))
      { current_agent(lexeme).type = LESS_EQUAL_GREATER_LEXEME; return; }
  }
  determine_type_of_constituent_string();

}

void lex_period (void) {
  store_and_advance();
  finish();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (isdigit(current_agent(current_char))) read_rest_of_floating_point_number();
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = PERIOD_LEXEME; return; }
  determine_type_of_constituent_string();
}

void lex_plus (void) {
  /* Lexeme might be +, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  bool could_be_floating_point;
  
  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_agent(current_char)=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<current_agent(lexeme).length; i++)
      if (! isdigit(current_agent(lexeme).string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = PLUS_LEXEME; return; }
  determine_type_of_constituent_string();
}
      
void lex_minus (void) {
  /* Lexeme might be -, -->, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  bool could_be_floating_point;

  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_agent(current_char)=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<current_agent(lexeme).length; i++)
      if (! isdigit(current_agent(lexeme).string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  if (current_agent(lexeme).length==1) { current_agent(lexeme).type = MINUS_LEXEME; return; }
  if (current_agent(lexeme).length==3) {
    if ((current_agent(lexeme).string[1]=='-')&&(current_agent(lexeme).string[2]=='>'))
      { current_agent(lexeme).type = RIGHT_ARROW_LEXEME; return; }
  }
  determine_type_of_constituent_string();
}

void lex_digit (void) {
  int i;
  bool could_be_floating_point;

  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_agent(current_char)=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<current_agent(lexeme).length; i++)
      if (! isdigit(current_agent(lexeme).string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  determine_type_of_constituent_string();
}

void lex_unknown (void) {
  if(reading_from_top_level() && current_agent(current_char) == 0) {
  }
  else {
    print ("Error:  Unknown character encountered by lexer, code=%d\n", 
           current_agent(current_char));
    print ("File %s, line %lu, column %lu.\n", current_agent(current_file)->filename,
           current_agent(current_file)->current_line, 
           current_agent(current_file)->current_column);
    if (! reading_from_top_level()) {
      respond_to_load_errors ();
      if (current_agent(load_errors_quit))
	current_agent(current_char) = EOF_AS_CHAR;
    }
  }
    get_next_char();
    get_lexeme();
}

void lex_constituent_string (void) {
  read_constituent_string();
  determine_type_of_constituent_string();
}

void lex_vbar (void) {
  current_agent(lexeme).type = SYM_CONSTANT_LEXEME;
  get_next_char();
  do {
    if ((current_agent(current_char)==EOF_AS_CHAR)||
        (current_agent(lexeme).length==MAX_LEXEME_LENGTH)) {
      print ("Error:  opening '|' without closing '|'\n");
      print_location_of_most_recent_lexeme();
      /* BUGBUG if reading from top level, don't want to signal EOF */
      current_agent(lexeme).type = EOF_LEXEME;
      current_agent(lexeme).string[0]=EOF_AS_CHAR;
      current_agent(lexeme).string[1]=0;
      current_agent(lexeme).length = 1;
      return;
    }
    if (current_agent(current_char)=='\\') {
      get_next_char();
      current_agent(lexeme).string[current_agent(lexeme).length++] = (char)current_agent(current_char);
      get_next_char();
    } else if (current_agent(current_char)=='|') {
      get_next_char();
      break;
    } else {
      current_agent(lexeme).string[current_agent(lexeme).length++] = (char)current_agent(current_char);
      get_next_char();
    }
  } while(TRUE);
  current_agent(lexeme).string[current_agent(lexeme).length]=0;
}

void lex_quote (void) {
  current_agent(lexeme).type = QUOTED_STRING_LEXEME;
  get_next_char();
  do {
    if ((current_agent(current_char)==EOF_AS_CHAR)||(current_agent(lexeme).length==MAX_LEXEME_LENGTH)) {
      print ("Error:  opening '\"' without closing '\"'\n");
      print_location_of_most_recent_lexeme();
      /* BUGBUG if reading from top level, don't want to signal EOF */
      current_agent(lexeme).type = EOF_LEXEME;
      current_agent(lexeme).string[0]=EOF_AS_CHAR;
      current_agent(lexeme).string[1]=0;
      current_agent(lexeme).length = 1;
      return;
    }
    if (current_agent(current_char)=='\\') {
      get_next_char();
      current_agent(lexeme).string[current_agent(lexeme).length++] = (char)current_agent(current_char);
      get_next_char();
    } else if (current_agent(current_char)=='"') {
      get_next_char();
      break;
    } else {
      current_agent(lexeme).string[current_agent(lexeme).length++] = (char)current_agent(current_char);
      get_next_char();
    }
  } while(TRUE);
  current_agent(lexeme).string[current_agent(lexeme).length]=0;
}

/* AGR 562 begin */

/* There are 2 functions here, for 2 different schemes for handling the
   shell escape.
   Scheme 1:  A '$' signals that all the rest of the text up to the '\n'
   is to be passed to the system() command verbatim.  The whole string,
   including the '$' as its first character, is stored in a single
   lexeme which has the type DOLLAR_STRING_LEXEME.
   Scheme 2:  A '$' is a single lexeme, much like a '(' or '&'.  All the
   subsequent lexemes are gotten individually with calls to get_lexeme().
   This makes it easier to parse the shell command, so that commands like
   cd, pushd, popd, etc. can be trapped and the equivalent Soar commands
   executed instead.  The problem with this scheme is that pulling the
   string apart into lexemes eliminates any special spacing the user may
   have done in specifying the shell command.  For that reason, my current
   plan is to follow scheme 1.  AGR 3-Jun-94  */

void lex_dollar (void) {
  current_agent(lexeme).type = DOLLAR_STRING_LEXEME;
  current_agent(lexeme).string[0] = '$';
  current_agent(lexeme).length = 1;
  get_next_char();   /* consume the '$' */
  while ((current_agent(current_char)!='\n') &&
	 (current_agent(current_char)!=EOF_AS_CHAR) &&
	 (current_agent(lexeme).length < MAX_LEXEME_LENGTH-1)) {
    current_agent(lexeme).string[current_agent(lexeme).length++] =
      current_agent(current_char);
    get_next_char();
  }
  current_agent(lexeme).string[current_agent(lexeme).length] = '\0';
}

/*
void lex_dollar (void) {
  store_and_advance();
  finish();
  current_agent(lexeme).type = DOLLAR_STRING_LEXEME;
}
*/

/* AGR 562 end */

/* ======================================================================
                             Get lexeme

  This is the main routine called from outside the lexer.  It reads past 
  any whitespace, then calls some lex_xxx routine (using the lexer_routines[]
  table) based on the first character of the lexeme.
====================================================================== */

void get_lexeme (void) {

  /* AGR 568 begin */
  if (current_agent(lex_alias)) {
    current_agent(lexeme) = current_agent(lex_alias)->lexeme;
    current_agent(lex_alias) = current_agent(lex_alias)->next;
    return;
  }
  /* AGR 568 end */

  current_agent(lexeme).length = 0;
  current_agent(lexeme).string[0] = 0;

#ifndef USE_X_DISPLAY
if (current_agent(lexeme).type==EOF_LEXEME && 
    reading_from_top_level() &&
    current_lexer_parentheses_level()==0 &&  /* AGR 534 */
    current_agent(print_prompt_flag))
#ifdef USE_TCL
  {}
#else

 /* REW: begin 09.15.96 */
 if (current_agent(operand2_mode) == TRUE)
   print ("\nOPERAND %s> ", current_agent(name));
 /* REW: end   09.15.96 */
 else
   print ("\n%s> ", current_agent(name));

#endif /* USE_TCL */
#endif /* USE_X_DISPLAY */

/* AGR 534  The only time a prompt should be printed out is if there's
   a command being expected; ie. the prompt shouldn't print out if we're
   in the middle of entering a production.  So if we're in the middle of
   entering a production, then the parentheses level will be > 0, so that's
   the criteria we will use.  AGR  5-Apr-94  */

  current_agent(load_errors_quit) = FALSE;  /* AGR 527c */

  while (current_agent(load_errors_quit)==FALSE) {   /* AGR 527c */
    if (current_agent(current_char)==EOF_AS_CHAR) break;
    if (whitespace[(unsigned char)current_agent(current_char)]) {
      if (current_agent(current_char) == '\n')
      {    
         if (current_agent(current_file)->fake_rparen_at_eol) {
              do_fake_rparen();
              return;
         }
#ifndef USE_X_DISPLAY
         if (current_lexer_parentheses_level()==0 &&  /* AGR 534 */
             current_agent(print_prompt_flag))
#ifdef USE_TCL
         {}
#else

	 /* REW: begin 09.15.96 */
         if (current_agent(operand2_mode) == TRUE)
	   print ("\nOPERAND %s> ", current_agent(name));
	 /* REW: end   09.15.96 */
	 else
	   print ("\n%s> ", current_agent(name));

#endif /* USE_TCL */
#endif /* USE_X_DISPLAY */
      }
      get_next_char();
      continue;
    }

#ifdef USE_TCL 
    if (current_agent(current_char)==';') {
      /* --- skip the semi-colon, forces newline in TCL --- */
      get_next_char();  /* consume it */
      continue;
    }
    if (current_agent(current_char)=='#') {
      /* --- read from hash to end-of-line --- */
      while ((current_agent(current_char)!='\n') &&
             (current_agent(current_char)!=EOF_AS_CHAR))
        get_next_char();
      if (current_agent(current_file)->fake_rparen_at_eol) {
        do_fake_rparen();
        return;
      }
      if (current_agent(current_char)!=EOF_AS_CHAR) get_next_char();
      continue;
    }
#else
    if (current_agent(current_char)==';') {
      /* --- read from semicolon to end-of-line --- */
      while ((current_agent(current_char)!='\n') &&
             (current_agent(current_char)!=EOF_AS_CHAR))
        get_next_char();
      if (current_agent(current_file)->fake_rparen_at_eol) {
        do_fake_rparen();
        return;
      }
      if (current_agent(current_char)!=EOF_AS_CHAR) get_next_char();
      continue;
    }
    if (current_agent(current_char)=='#') {
      /* --- comments surrounded by "#|" and "|#" delimiters --- */
      record_position_of_start_of_lexeme(); /* in case of later error mesg. */
      get_next_char();
      if (current_agent(current_char)!='|') {
        print ("Error: '#' not followed by '|'\n");
        print_location_of_most_recent_lexeme();
        continue;
      }
      get_next_char();  /* consume the vbar */
      while (TRUE) {
        if (current_agent(current_char)==EOF_AS_CHAR) {
          print ("Error: '#|' without terminating '|#'\n");
          print_location_of_most_recent_lexeme();
          break;
        }
        if (current_agent(current_char)!='|') { get_next_char(); continue; }
        get_next_char();
        if (current_agent(current_char)=='#') break;
      }
      get_next_char();  /* consume the closing '#' */
      continue; /* continue outer while(TRUE), reading more whitespace */
    }
#endif  /* USE_TCL */
    break; /* if no whitespace or comments found, break out of the loop */
  }
  /* --- no more whitespace, so go get the actual lexeme --- */
  record_position_of_start_of_lexeme();
  if (current_agent(current_char)!=EOF_AS_CHAR)
    (*(lexer_routines[(unsigned char)current_agent(current_char)]))();
  else
    lex_eof();
}
  
/* ======================================================================
                            Init lexer

  This should be called before anything else in this file.  It does all 
  the necessary init stuff for the lexer, and starts the lexer reading from
  standard input.
====================================================================== */

char extra_constituents[] = "$%&*+-/:<=>?_";

void init_lexer (void) {

  unsigned int i; /* rmarinie: changed int to unsigned int to match return of strlen below */

  /* --- setup constituent_char array --- */
  for (i=0; i<256; i++)
    if (isalnum(i)) constituent_char[i]=TRUE; else constituent_char[i]=FALSE;
  for (i=0; i<strlen(extra_constituents); i++)
    constituent_char[(int)extra_constituents[i]]=TRUE;
  
  /* --- setup whitespace array --- */
  for (i=0; i<256; i++)
    if (isspace(i)) whitespace[i]=TRUE; else whitespace[i]=FALSE;

  /* --- setup number_starters array --- */
  for (i=0; i<256; i++)
    if (isdigit(i)) number_starters[i]=TRUE; else number_starters[i]=FALSE;
  number_starters['+']=TRUE;
  number_starters['-']=TRUE;
  number_starters['.']=TRUE;

  /* --- setup lexer_routines array --- */
  for (i=0; i<256; i++) lexer_routines[i] = lex_unknown;
  for (i=0; i<256; i++)
    if (constituent_char[i]) lexer_routines[i] = lex_constituent_string;
  for (i=0; i<256; i++) if (isdigit(i)) lexer_routines[i] = lex_digit;
  lexer_routines['@'] = lex_at;
  lexer_routines['('] = lex_lparen;
  lexer_routines[')'] = lex_rparen;
  lexer_routines['+'] = lex_plus;
  lexer_routines['-'] = lex_minus;
  lexer_routines['~'] = lex_tilde;
  lexer_routines['^'] = lex_up_arrow;
  lexer_routines['{'] = lex_lbrace;
  lexer_routines['}'] = lex_rbrace;
  lexer_routines['!'] = lex_exclamation_point;
  lexer_routines['>'] = lex_greater;
  lexer_routines['<'] = lex_less;
  lexer_routines['='] = lex_equal;
  lexer_routines['&'] = lex_ampersand;
  lexer_routines['|'] = lex_vbar;
  lexer_routines[','] = lex_comma;
  lexer_routines['.'] = lex_period;
  lexer_routines['"'] = lex_quote;
  lexer_routines['$'] = lex_dollar;   /* AGR 562 */

  /* --- initially we're reading from the standard input --- */
  start_lex_from_file ("[standard input]", stdin);
}

/* ======================================================================
                   Print location of most recent lexeme

  This routine is used to print an indication of where a parser or interface
  command error occurred.  It tries to print out the current source line
  with a pointer to where the error was detected.  If the current source
  line is no longer available, it just prints out the line number instead.

  BUGBUG: if the input line contains any tabs, the pointer comes out in
  the wrong place.
====================================================================== */

void print_location_of_most_recent_lexeme (void) {
  int i;
  
  if (current_agent(current_file)->line_of_start_of_last_lexeme ==
      current_agent(current_file)->current_line) {
    /* --- error occurred on current line, so print out the line --- */
    if (! reading_from_top_level()) {
      print ("File %s, line %lu:\n", current_agent(current_file)->filename,
             current_agent(current_file)->current_line);
      /*       respond_to_load_errors ();     AGR 527a */
    }
    if (current_agent(current_file)->buffer[strlen(current_agent(current_file)->buffer)-1]=='\n')
      print_string (current_agent(current_file)->buffer);
    else
      print ("%s\n",current_agent(current_file)->buffer);
    for (i=0; i<current_agent(current_file)->column_of_start_of_last_lexeme; i++)
      print_string ("-");
    print_string ("^\n");

    if (! reading_from_top_level()) {
      respond_to_load_errors (); /* AGR 527a */
      if (current_agent(load_errors_quit))
	current_agent(current_char) = EOF_AS_CHAR;
    }

/* AGR 527a  The respond_to_load_errors call came too early (above),
   and the "continue" prompt appeared before the offending line was printed
   out, so the respond_to_load_errors call was moved here.
   AGR 26-Apr-94 */

  } else {
    /* --- error occurred on a previous line, so just give the position --- */
    print ("File %s, line %lu, column %lu.\n", current_agent(current_file)->filename,
           current_agent(current_file)->line_of_start_of_last_lexeme,
           current_agent(current_file)->column_of_start_of_last_lexeme + 1);
    if (! reading_from_top_level()) {
      respond_to_load_errors ();
      if (current_agent(load_errors_quit))
	current_agent(current_char) = EOF_AS_CHAR;
    }
  }
}

/* ======================================================================
                       Parentheses Utilities

  Current_lexer_parentheses_level() returns the current level of parentheses
  nesting (0 means no open paren's have been encountered).

  Skip_ahead_to_balanced_parentheses() eats lexemes until the appropriate
  closing paren is found (0 means eat until back at the top level).
  
  Fake_rparen_at_next_end_of_line() tells the lexer to insert a fake
  R_PAREN_LEXEME token the next time it reaches the end of a line.
====================================================================== */

int current_lexer_parentheses_level (void) {
  return current_agent(current_file)->parentheses_level;
}

void skip_ahead_to_balanced_parentheses (int parentheses_level) {
  while (TRUE) {
    if (current_agent(lexeme).type==EOF_LEXEME) return;
    if ((current_agent(lexeme).type==R_PAREN_LEXEME) &&
        (parentheses_level==current_agent(current_file)->parentheses_level)) return;
    get_lexeme();
  }
}

void fake_rparen_at_next_end_of_line (void) {
  current_agent(current_file)->parentheses_level++;  
  current_agent(current_file)->fake_rparen_at_eol = TRUE;  
}

/* ======================================================================
                        Set lexer allow ids

  This routine should be called to tell the lexer whether to allow
  identifiers to be read.  If FALSE, things that look like identifiers
  will be returned as SYM_CONSTANT_LEXEME's instead.
====================================================================== */

void set_lexer_allow_ids (bool allow_identifiers) {
  current_agent(current_file)->allow_ids = allow_identifiers;
}

/* ======================================================================
               Determine possible symbol types for string

  This is a utility routine which figures out what kind(s) of symbol a 
  given string could represent.  At entry:  s, length_of_s represent the
  string.  At exit:  possible_xxx is set to TRUE/FALSE to indicate
  whether the given string could represent that kind of symbol; rereadable
  is set to TRUE indicating whether the lexer would read the given string
  as a symbol with exactly the same name (as opposed to treating it as a
  special lexeme like "+", changing upper to lower case, etc.
====================================================================== */

void determine_possible_symbol_types_for_string (char *s,
                                                 int length_of_s,
                                                 bool *possible_id,
                                                 bool *possible_var,
                                                 bool *possible_sc,
                                                 bool *possible_ic,
                                                 bool *possible_fc,
                                                 bool *rereadable) {
  char *ch;
  bool rereadability_dead, rereadability_questionable;

  *possible_id = FALSE;
  *possible_var = FALSE;
  *possible_sc = FALSE;
  *possible_ic = FALSE;
  *possible_fc = FALSE;
  *rereadable = FALSE;

  /* --- check if it's an integer or floating point number --- */
  if (number_starters[(unsigned char)(*s)]) {
    ch = s;
    if ((*ch=='+')||(*ch=='-')) ch++;  /* optional leading + or - */
    while (isdigit(*ch)) ch++;         /* string of digits */
    if ((*ch==0)&&(isdigit(*(ch-1))))
      *possible_ic = TRUE;
    if (*ch=='.') {
      ch++;                              /* decimal point */
      while (isdigit(*ch)) ch++;         /* string of digits */
      if ((*ch=='e')||(*ch=='E')) {
        ch++;                              /* E */
        if ((*ch=='+')||(*ch=='-')) ch++;  /* optional leading + or - */
        while (isdigit(*ch)) ch++;         /* string of digits */
      }
      if (*ch==0) *possible_fc = TRUE;
    }
  }

  /* --- make sure it's entirely constituent characters --- */
  for (ch=s; *ch!=0; ch++)
    if (! constituent_char[(unsigned char)(*ch)]) return;

  /* --- check for rereadability --- */
  rereadability_questionable = FALSE;
  rereadability_dead = FALSE;
  for (ch=s; *ch!=0; ch++) {
    if (islower(*ch) || isdigit(*ch)) continue; /* these guys are fine */
    if (isupper(*ch)) { rereadability_dead = TRUE; break; }
    rereadability_questionable = TRUE;
  }
  if (! rereadability_dead) {
    if ((! rereadability_questionable) ||
        (length_of_s >= LENGTH_OF_LONGEST_SPECIAL_LEXEME) ||
        ((length_of_s==1)&&(*s=='*')))
      *rereadable = TRUE;
  }

  /* --- any string of constituents could be a sym constant --- */
  *possible_sc = TRUE;
  
  /* --- check whether it's a variable --- */
  if ((*s=='<')&&(*(s+length_of_s-1)=='>')) *possible_var = TRUE;

  /* --- check if it's an identifier --- */
  if (isalpha(*s)) {
    /* --- is the rest of the string an integer? --- */
    ch = s+1;
    while (isdigit(*ch)) ch++;         /* string of digits */
    if ((*ch==0)&&(isdigit(*(ch-1)))) *possible_id = TRUE;
  }
}

