#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  lexer.cpp
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
 */
/* ======================================================================
                             lexer.c

    See comments in soarkernel.h for an overview.
   ====================================================================== */

#include "lexer.h"
#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "print.h"
#include "init_soar.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "gski_event_system_functions.h" // support for triggering XML events

//
// These three should be safe for re-entrancy.  --JNW--
//
Bool constituent_char[256];   /* is the character a symbol constituent? */
Bool whitespace[256];         /* is the character whitespace? */
Bool number_starters[256];    /* could the character initiate a number? */

/* ======================================================================
                       Start/Stop Lex from File
                       
  The lexer maintains a stack of files being read, in order to handle nested
  loads.  Start_lex_from_file() and stop_lex_from_file() push and pop the
  stack.  Immediately after start_lex_from_file(), the current lexeme (agent
  variable) is undefined.  Immediately after stop_lex_from_file(), the 
  current lexeme is automatically restored to whatever it was just before
  the corresponding start_lex_from_file() call.
====================================================================== */

void start_lex_from_file (agent* thisAgent, char *filename, 
						  FILE *already_opened_file) {
  lexer_source_file *lsf;

  lsf = static_cast<lexer_source_file_struct *>(allocate_memory (thisAgent, sizeof(lexer_source_file),
                                                                 MISCELLANEOUS_MEM_USAGE));
  lsf->saved_lexeme = thisAgent->lexeme;
  lsf->saved_current_char = thisAgent->current_char;
  lsf->parent_file = thisAgent->current_file;
  thisAgent->current_file = lsf;
  lsf->filename = make_memory_block_for_string (thisAgent, filename);
  lsf->file = already_opened_file;
  lsf->fake_rparen_at_eol = FALSE;
  lsf->allow_ids = TRUE;
  lsf->parentheses_level = 0;
  lsf->column_of_start_of_last_lexeme = 0;
  lsf->line_of_start_of_last_lexeme = 0;
  lsf->current_line = 0;
  lsf->current_column = 0;
  lsf->buffer[0] = 0;
  thisAgent->current_char = ' ';   /* whitespace--to force immediate read of first line */
}

void stop_lex_from_file (agent* thisAgent) {
  lexer_source_file *lsf;

  if (reading_from_top_level(thisAgent)) {
    print (thisAgent, "Internal error: tried to stop_lex_from_file at top level\n");
    return;
  }
  lsf = thisAgent->current_file;
  thisAgent->current_file = thisAgent->current_file->parent_file;
  thisAgent->current_char = lsf->saved_current_char;
  thisAgent->lexeme = lsf->saved_lexeme;

  free_memory_block_for_string (thisAgent, lsf->filename);
  free_memory (thisAgent, lsf, MISCELLANEOUS_MEM_USAGE);
}

/* ======================================================================
                             Get next char

  Get_next_char() gets the next character from the current input file and
  puts it into the agent variable current_char.
====================================================================== */

void get_next_char (agent* thisAgent) {
  char *s;

//#ifdef USE_TCL
  /* Soar-Bugs #54, TMH */
  if ( thisAgent->alternate_input_exit &&
      (thisAgent->alternate_input_string == NULL) &&
      (thisAgent->alternate_input_suffix == NULL)   ) {
    thisAgent->current_char = EOF_AS_CHAR;
    /* Replaced deprecated control_c_handler with an appropriate assertion */
	//control_c_handler(0);
	assert(0 && "error in lexer.cpp (control_c_handler() used to be called here)");
    return;
  }
//#endif

  if (thisAgent->using_input_string) {
    if (*(thisAgent->input_string)!='\0')
      thisAgent->current_char = *(thisAgent->input_string++);
    return;
  }

//#ifdef USE_X_DISPLAY
//  if (x_input_buffer != NIL) {
//    thisAgent->current_char = x_input_buffer[x_input_buffer_index++];
//    if (thisAgent->current_char == '\n') {
//      x_input_buffer = NIL;
//    }
//  } else {
//    thisAgent->current_char = thisAgent->current_file->buffer 
//                           [thisAgent->current_file->current_column++];
//  }
//#elif defined(USE_TCL)
  if (thisAgent->alternate_input_string != NULL)
    {
      thisAgent->current_char = *thisAgent->alternate_input_string++;

      if (thisAgent->current_char == '\0') 
        {
          thisAgent->alternate_input_string = NIL;
          thisAgent->current_char = 
            *thisAgent->alternate_input_suffix++;
        }
    }
  else if (thisAgent->alternate_input_suffix != NULL)
    {
      thisAgent->current_char = *thisAgent->alternate_input_suffix++;

      if (thisAgent->current_char == '\0') 
      {
          thisAgent->alternate_input_suffix = NIL;

          /* Soar-Bugs #54, TMH */
          if ( thisAgent->alternate_input_exit ) {
            thisAgent->current_char = EOF_AS_CHAR;
          /* Replaced deprecated control_c_handler with an appropriate assertion */
          //control_c_handler(0);
          assert(0 && "error in lexer.cpp (control_c_handler() used to be called here)");
          return;
	  }

          thisAgent->current_char = thisAgent->current_file->buffer 
            [thisAgent->current_file->current_column++];
        }
    } 
  else 
    {
      thisAgent->current_char = thisAgent->current_file->buffer 
        [thisAgent->current_file->current_column++];
    }
//#elif _WINDOWS
//  if (thisAgent->current_file->file==stdin) { 
//    switch (thisAgent->current_line[thisAgent->current_line_index]) {
//      case EOF_AS_CHAR:
//      case 0:
//			if (!AvailableWindowCommand()) {
//	  			thisAgent->current_line[thisAgent->current_line_index=0]=thisAgent->current_char=EOF_AS_CHAR;
//		 	    break;
//			} else {
//	  			s=GetWindowCommand();
//	  			strncpy(thisAgent->current_line,s+1, AGENT_STRUCT_CURRENT_LINE_BUFFER_SIZE);
//				thisAgent->current_line[AGENT_STRUCT_CURRENT_LINE_BUFFER_SIZE-1] = 0; /* ensure null termination */
//	  			if (thisAgent->print_prompt_flag=(s[0]=='1')) 
//			    print("%s",s+1);
//	  			free(s);
//	  			thisAgent->current_line_index=0;
//			}
//      default:
//			thisAgent->current_char=thisAgent->current_line[thisAgent->current_line_index++];
//			break;
//    }
//  } else 
//    thisAgent->current_char = thisAgent->current_file->buffer 
//			   [thisAgent->current_file->current_column++];
//#else
//  thisAgent->current_char = thisAgent->current_file->buffer 
//                           [thisAgent->current_file->current_column++];
//#endif

  if (thisAgent->current_char) return;

  if ((thisAgent->current_file->current_column == BUFSIZE) &&
      (thisAgent->current_file->buffer[BUFSIZE-2] != '\n') &&
      (thisAgent->current_file->buffer[BUFSIZE-2] != EOF_AS_CHAR)) {
    char msg[512];
    snprintf (msg, 512,
	     "lexer.c: Error:  line too long (max allowed is %d chars)\nFile %s, line %lu\n",
	     MAX_LEXER_LINE_LENGTH, thisAgent->current_file->filename,
	     thisAgent->current_file->current_line);
	msg[511] = 0; /* ensure null termination */

    abort_with_fatal_error(thisAgent, msg);
  }

  s = fgets (thisAgent->current_file->buffer, BUFSIZE, thisAgent->current_file->file);

  if (s) {
    thisAgent->current_file->current_line++;
    if (reading_from_top_level(thisAgent)) {
      tell_printer_that_output_column_has_been_reset (thisAgent);
      if (thisAgent->logging_to_file)
        print_string_to_log_file_only (thisAgent, thisAgent->current_file->buffer);
    }
  } else {
    /* s==NIL means immediate eof encountered or read error occurred */
    if (! feof(thisAgent->current_file->file)) {
      if(reading_from_top_level(thisAgent)) {
//#ifndef _WINDOWS
        /* Replaced deprecated control_c_handler with an appropriate assertion */
        //control_c_handler(0);
        assert(0 && "error in lexer.cpp (control_c_handler() used to be called here)");
//#endif
        return;
      } else {
        print (thisAgent, "I/O error while reading file %s; ignoring the rest of it.\n",
               thisAgent->current_file->filename);
      }
    }
    thisAgent->current_file->buffer[0] = EOF_AS_CHAR;
    thisAgent->current_file->buffer[1] = 0;
  }
  thisAgent->current_char = thisAgent->current_file->buffer[0];
  thisAgent->current_file->current_column = 1;
}

/* ======================================================================

                         Lexer Utility Routines

====================================================================== */

/*#define record_position_of_start_of_lexeme() { \
  thisAgent->current_file->column_of_start_of_last_lexeme = \
    thisAgent->current_file->current_column - 1; \
  thisAgent->current_file->line_of_start_of_last_lexeme = \
    thisAgent->current_file->current_line; }*/
inline void record_position_of_start_of_lexeme(agent* thisAgent)
{
  thisAgent->current_file->column_of_start_of_last_lexeme =
    thisAgent->current_file->current_column - 1;
  thisAgent->current_file->line_of_start_of_last_lexeme =
    thisAgent->current_file->current_line;
}

/*  redefined for Soar 7, want case-sensitivity to match Tcl.  KJC 5/96 
#define store_and_advance() { \
  thisAgent->lexeme.string[thisAgent->lexeme.length++] = (isupper((char)thisAgent->current_char) ? \
                                    tolower((char)thisAgent->current_char) : \
                                    (char)thisAgent->current_char); \
  get_next_char(); }
#define store_and_advance() { \
  thisAgent->lexeme.string[thisAgent->lexeme.length++] = \
    (char)thisAgent->current_char; \
  get_next_char(); }*/
inline void store_and_advance(agent* thisAgent)
{
  thisAgent->lexeme.string[thisAgent->lexeme.length++] =
    (char)thisAgent->current_char;
  get_next_char(thisAgent);
}

/*#define finish() { thisAgent->lexeme.string[thisAgent->lexeme.length]=0; }*/
inline void finish(agent* thisAgent)
{ 
  thisAgent->lexeme.string[thisAgent->lexeme.length]=0;
}

void read_constituent_string (agent* thisAgent) {
#ifdef __SC__
	char *buf;
	int i,len;
#endif

  while ((thisAgent->current_char!=EOF_AS_CHAR) &&
         constituent_char[(unsigned char)thisAgent->current_char])
    store_and_advance(thisAgent);
  finish(thisAgent);
}  

void read_rest_of_floating_point_number (agent* thisAgent) {
  /* --- at entry, current_char=="."; we read the "." and rest of number --- */
  store_and_advance(thisAgent);
  while (isdigit(thisAgent->current_char)) store_and_advance(thisAgent); /* string of digits */
  if ((thisAgent->current_char=='e')||(thisAgent->current_char=='E')) {
    store_and_advance(thisAgent);                             /* E */
    if ((thisAgent->current_char=='+')||(thisAgent->current_char=='-'))
      store_and_advance(thisAgent);                       /* optional leading + or - */
    while (isdigit(thisAgent->current_char)) store_and_advance(thisAgent); /* string of digits */
  }
  finish(thisAgent);

#ifdef __SC__
  if (strcmp("soar>",thisAgent->lexeme.string)) { /* if the lexeme doesn't equal "soar>" */
  	if (!(strncmp("soar>",thisAgent->lexeme.string,5))) { /* but the first 5 chars are "soar>" */
		/* then SIOW messed up so ignore the "soar>" */
	   buf = (char *)allocate_memory(thisAgent, (len=(strlen(thisAgent->lexeme.string)+1))*sizeof(char),STRING_MEM_USAGE);
	   for (i=0;i<=len;i++) {
	   	   buf[i] = thisAgent->lexeme.string[i];
	   }
	   for (i=5;i<=len;i++) {
	   	   thisAgent->lexeme.string[i-5] = buf[i];
	   }
	   free_memory_block_for_string(thisAgent, buf);
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

Bool determine_type_of_constituent_string (agent* thisAgent) {
  Bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
  Bool rereadable;

  determine_possible_symbol_types_for_string (thisAgent->lexeme.string,
                                              thisAgent->lexeme.length,
                                              &possible_id,
                                              &possible_var,
                                              &possible_sc,
                                              &possible_ic,
                                              &possible_fc,
                                              &rereadable);

  /* --- check whether it's a variable --- */
  if (possible_var) {
    thisAgent->lexeme.type = VARIABLE_LEXEME;
    return TRUE;
  }

  /* --- check whether it's an integer --- */
  if (possible_ic) {
    errno = 0;
    thisAgent->lexeme.type = INT_CONSTANT_LEXEME;
    thisAgent->lexeme.int_val = strtol (thisAgent->lexeme.string,NULL,10);
    if (errno) {
      print (thisAgent, "Error: bad integer (probably too large)\n");
      print_location_of_most_recent_lexeme(thisAgent);
      thisAgent->lexeme.int_val = 0;
    }
    return (errno == 0);
  }
    
  /* --- check whether it's a floating point number --- */
  if (possible_fc) {
    errno = 0;
    thisAgent->lexeme.type = FLOAT_CONSTANT_LEXEME;
    thisAgent->lexeme.float_val = (float) my_strtod (thisAgent->lexeme.string,NULL,10); 
    if (errno) {
      print (thisAgent, "Error: bad floating point number\n");
      print_location_of_most_recent_lexeme(thisAgent);
      thisAgent->lexeme.float_val = 0.0;
    }
    return (errno == 0);
  }
  
  /* --- check if it's an identifier --- */
  if (thisAgent->current_file->allow_ids && possible_id) {
    thisAgent->lexeme.id_letter = toupper(thisAgent->lexeme.string[0]);
    errno = 0;
    thisAgent->lexeme.type = IDENTIFIER_LEXEME;
    thisAgent->lexeme.id_number = my_strtoul (&(thisAgent->lexeme.string[1]),NULL,10);
    if (errno) {
      print (thisAgent, "Error: bad number for identifier (probably too large)\n");
      print_location_of_most_recent_lexeme(thisAgent);
      thisAgent->lexeme.id_number = 0;
    }
    return (errno == 0);
  }

  /* --- otherwise it must be a symbolic constant --- */
  if (possible_sc) {
    thisAgent->lexeme.type = SYM_CONSTANT_LEXEME;
    if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
      if (thisAgent->lexeme.string[0] == '<') {
        if (thisAgent->lexeme.string[1] == '<') {
           print (thisAgent, "Warning: Possible disjunctive encountered in reading symbolic constant\n");
           print (thisAgent, "         If a disjunctive was intended, add a space after <<\n");
           print (thisAgent, "         If a constant was intended, surround constant with vertical bars\n");

		   GenerateWarningXML(thisAgent, "Warning: Possible disjunctive encountered in reading symbolic constant.\n         If a disjunctive was intended, add a space after &lt;&lt;\n         If a constant was intended, surround constant with vertical bars.");		   
		   //TODO: should this be appended to previous XML message, or should it be a separate message?
           print_location_of_most_recent_lexeme(thisAgent);
	 } else {
           print (thisAgent, "Warning: Possible variable encountered in reading symbolic constant\n");
           print (thisAgent, "         If a constant was intended, surround constant with vertical bars\n");

		   GenerateWarningXML(thisAgent, "Warning: Possible variable encountered in reading symbolic constant.\n         If a constant was intended, surround constant with vertical bars.");
		   //TODO: should this be appended to previous XML message, or should it be a separate message?
           print_location_of_most_recent_lexeme(thisAgent);
         }
      } else {
        if (thisAgent->lexeme.string[thisAgent->lexeme.length-1] == '>') {
          if (thisAgent->lexeme.string[thisAgent->lexeme.length-2] == '>') {
           print (thisAgent, "Warning: Possible disjunctive encountered in reading symbolic constant\n");
           print (thisAgent, "         If a disjunctive was intended, add a space before >>\n");
           print (thisAgent, "         If a constant was intended, surround constant with vertical bars\n");

		   GenerateWarningXML(thisAgent, "Warning: Possible disjunctive encountered in reading symbolic constant.\n         If a disjunctive was intended, add a space before &gt;&gt;\n         If a constant was intended, surround constant with vertical bars.");
		   //TODO: should this be appended to previous XML message, or should it be a separate message?
           print_location_of_most_recent_lexeme(thisAgent);

	 } else {
           print (thisAgent, "Warning: Possible variable encountered in reading symbolic constant\n");
           print (thisAgent, "         If a constant was intended, surround constant with vertical bars\n");
		   
		   GenerateWarningXML(thisAgent, "Warning: Possible variable encountered in reading symbolic constant.\n         If a constant was intended, surround constant with vertical bars.");
		   //TODO: should this be appended to previous XML message, or should it be a separate message?
           print_location_of_most_recent_lexeme(thisAgent);

	   // TODO:  generate tagged output in print_location_of_most_recent_lexeme
         }
	}
      }
    }
    return TRUE;
  }

//#if defined(USE_TCL)
  thisAgent->lexeme.type = QUOTED_STRING_LEXEME;
  return TRUE;
//#else
//  char msg[BUFFER_MSG_SIZE];
//  strncpy (msg, "Internal error: can't determine_type_of_constituent_string\n", BUFFER_MSG_SIZE);
//  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
//  abort_with_fatal_error(thisAgent, msg);
//  return FALSE;
//#endif
}

void do_fake_rparen (agent* thisAgent) {
  record_position_of_start_of_lexeme(thisAgent);
  thisAgent->lexeme.type = R_PAREN_LEXEME;
  thisAgent->lexeme.length = 1;
  thisAgent->lexeme.string[0] = ')';
  thisAgent->lexeme.string[1] = 0;
  if (thisAgent->current_file->parentheses_level > 0) thisAgent->current_file->parentheses_level--;
  thisAgent->current_file->fake_rparen_at_eol = FALSE;
}

/* ======================================================================
                        Lex such-and-such Routines

  These routines are called from get_lexeme().  Which routine gets called
  depends on the first character of the new lexeme being read.  Each routine's
  job is to finish reading the lexeme and store the necessary items in 
  the agent variable "lexeme".
====================================================================== */

void lex_unknown (agent* thisAgent);
#define lu lex_unknown

//
// This should be safe for re-entrant code. --JNW--
//
void (*(lexer_routines[256]))(agent*) =
{
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
   lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,lu,
};

void lex_eof (agent* thisAgent) {
  if (thisAgent->current_file->fake_rparen_at_eol) {
    do_fake_rparen(thisAgent);
    return;
  }
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = EOF_LEXEME;
}

void lex_at (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = AT_LEXEME;
}

void lex_tilde (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = TILDE_LEXEME;
}

void lex_up_arrow (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = UP_ARROW_LEXEME;
}

void lex_lbrace (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = L_BRACE_LEXEME;
}

void lex_rbrace (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = R_BRACE_LEXEME;
}

void lex_exclamation_point (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = EXCLAMATION_POINT_LEXEME;
}

void lex_comma (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = COMMA_LEXEME;
}

void lex_equal (agent* thisAgent) {
  /* Lexeme might be "=", or symbol */
  /* Note: this routine relies on = being a constituent character */

  read_constituent_string(thisAgent);
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = EQUAL_LEXEME; return; }
  determine_type_of_constituent_string(thisAgent);
}

void lex_ampersand (agent* thisAgent) {
  /* Lexeme might be "&", or symbol */
  /* Note: this routine relies on & being a constituent character */

  read_constituent_string(thisAgent);
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = AMPERSAND_LEXEME; return; }
  determine_type_of_constituent_string(thisAgent);
}

void lex_lparen (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = L_PAREN_LEXEME;
  thisAgent->current_file->parentheses_level++;
}

void lex_rparen (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  thisAgent->lexeme.type = R_PAREN_LEXEME;
  if (thisAgent->current_file->parentheses_level > 0) thisAgent->current_file->parentheses_level--;
}

void lex_greater (agent* thisAgent) {
  /* Lexeme might be ">", ">=", ">>", or symbol */
  /* Note: this routine relies on =,> being constituent characters */

  read_constituent_string(thisAgent);
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = GREATER_LEXEME; return; }
  if (thisAgent->lexeme.length==2) {
    if (thisAgent->lexeme.string[1]=='>') { thisAgent->lexeme.type = GREATER_GREATER_LEXEME; return;}
    if (thisAgent->lexeme.string[1]=='=') { thisAgent->lexeme.type = GREATER_EQUAL_LEXEME; return; }
  }
  determine_type_of_constituent_string(thisAgent);
}
    
void lex_less (agent* thisAgent) {
  /* Lexeme might be "<", "<=", "<=>", "<>", "<<", or variable */
  /* Note: this routine relies on =,<,> being constituent characters */

  read_constituent_string(thisAgent);
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = LESS_LEXEME; return; }
  if (thisAgent->lexeme.length==2) {
    if (thisAgent->lexeme.string[1]=='>') { thisAgent->lexeme.type = NOT_EQUAL_LEXEME; return; }
    if (thisAgent->lexeme.string[1]=='=') { thisAgent->lexeme.type = LESS_EQUAL_LEXEME; return; }
    if (thisAgent->lexeme.string[1]=='<') { thisAgent->lexeme.type = LESS_LESS_LEXEME; return; }
  }
  if (thisAgent->lexeme.length==3) {
    if ((thisAgent->lexeme.string[1]=='=')&&(thisAgent->lexeme.string[2]=='>'))
      { thisAgent->lexeme.type = LESS_EQUAL_GREATER_LEXEME; return; }
  }
  determine_type_of_constituent_string(thisAgent);

}

void lex_period (agent* thisAgent) {
  store_and_advance(thisAgent);
  finish(thisAgent);
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (isdigit(thisAgent->current_char)) read_rest_of_floating_point_number(thisAgent);
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = PERIOD_LEXEME; return; }
  determine_type_of_constituent_string(thisAgent);
}

void lex_plus (agent* thisAgent) {
  /* Lexeme might be +, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  Bool could_be_floating_point;
  
  read_constituent_string(thisAgent);
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (thisAgent->current_char=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<thisAgent->lexeme.length; i++)
      if (! isdigit(thisAgent->lexeme.string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number(thisAgent);
  }
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = PLUS_LEXEME; return; }
  determine_type_of_constituent_string(thisAgent);
}
      
void lex_minus (agent* thisAgent) {
  /* Lexeme might be -, -->, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  Bool could_be_floating_point;

  read_constituent_string(thisAgent);
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (thisAgent->current_char=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<thisAgent->lexeme.length; i++)
      if (! isdigit(thisAgent->lexeme.string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number(thisAgent);
  }
  if (thisAgent->lexeme.length==1) { thisAgent->lexeme.type = MINUS_LEXEME; return; }
  if (thisAgent->lexeme.length==3) {
    if ((thisAgent->lexeme.string[1]=='-')&&(thisAgent->lexeme.string[2]=='>'))
      { thisAgent->lexeme.type = RIGHT_ARROW_LEXEME; return; }
  }
  determine_type_of_constituent_string(thisAgent);
}

void lex_digit (agent* thisAgent) {
  int i;
  Bool could_be_floating_point;

  read_constituent_string(thisAgent);
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (thisAgent->current_char=='.') {
    could_be_floating_point = TRUE;
    for (i=1; i<thisAgent->lexeme.length; i++)
      if (! isdigit(thisAgent->lexeme.string[i])) could_be_floating_point = FALSE;
    if (could_be_floating_point) read_rest_of_floating_point_number(thisAgent);
  }
  determine_type_of_constituent_string(thisAgent);
}

void lex_unknown (agent* thisAgent) {
  if(reading_from_top_level(thisAgent) && thisAgent->current_char == 0) {
  }
  else {
    print (thisAgent, "Error:  Unknown character encountered by lexer, code=%d\n", 
           thisAgent->current_char);
    print (thisAgent, "File %s, line %lu, column %lu.\n", thisAgent->current_file->filename,
           thisAgent->current_file->current_line, 
           thisAgent->current_file->current_column);
    if (! reading_from_top_level(thisAgent)) {
      //respond_to_load_errors (thisAgent);
      if (thisAgent->load_errors_quit)
	thisAgent->current_char = EOF_AS_CHAR;
    }
  }
    get_next_char(thisAgent);
    get_lexeme(thisAgent);
}

void lex_constituent_string (agent* thisAgent) {
  read_constituent_string(thisAgent);
  determine_type_of_constituent_string(thisAgent);
}

void lex_vbar (agent* thisAgent) {
  thisAgent->lexeme.type = SYM_CONSTANT_LEXEME;
  get_next_char(thisAgent);
  do {
    if ((thisAgent->current_char==EOF_AS_CHAR)||
        (thisAgent->lexeme.length==MAX_LEXEME_LENGTH)) {
      print (thisAgent, "Error:  opening '|' without closing '|'\n");
      print_location_of_most_recent_lexeme(thisAgent);
      /* BUGBUG if reading from top level, don't want to signal EOF */
      thisAgent->lexeme.type = EOF_LEXEME;
      thisAgent->lexeme.string[0]=EOF_AS_CHAR;
      thisAgent->lexeme.string[1]=0;
      thisAgent->lexeme.length = 1;
      return;
    }
    if (thisAgent->current_char=='\\') {
      get_next_char(thisAgent);
      thisAgent->lexeme.string[thisAgent->lexeme.length++] = (char)thisAgent->current_char;
      get_next_char(thisAgent);
    } else if (thisAgent->current_char=='|') {
      get_next_char(thisAgent);
      break;
    } else {
      thisAgent->lexeme.string[thisAgent->lexeme.length++] = (char)thisAgent->current_char;
      get_next_char(thisAgent);
    }
  } while(TRUE);
  thisAgent->lexeme.string[thisAgent->lexeme.length]=0;
}

void lex_quote (agent* thisAgent) {
  thisAgent->lexeme.type = QUOTED_STRING_LEXEME;
  get_next_char(thisAgent);
  do {
    if ((thisAgent->current_char==EOF_AS_CHAR)||(thisAgent->lexeme.length==MAX_LEXEME_LENGTH)) {
      print (thisAgent, "Error:  opening '\"' without closing '\"'\n");
      print_location_of_most_recent_lexeme(thisAgent);
      /* BUGBUG if reading from top level, don't want to signal EOF */
      thisAgent->lexeme.type = EOF_LEXEME;
      thisAgent->lexeme.string[0]=EOF_AS_CHAR;
      thisAgent->lexeme.string[1]=0;
      thisAgent->lexeme.length = 1;
      return;
    }
    if (thisAgent->current_char=='\\') {
      get_next_char(thisAgent);
      thisAgent->lexeme.string[thisAgent->lexeme.length++] = (char)thisAgent->current_char;
      get_next_char(thisAgent);
    } else if (thisAgent->current_char=='"') {
      get_next_char(thisAgent);
      break;
    } else {
      thisAgent->lexeme.string[thisAgent->lexeme.length++] = (char)thisAgent->current_char;
      get_next_char(thisAgent);
    }
  } while(TRUE);
  thisAgent->lexeme.string[thisAgent->lexeme.length]=0;
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

void lex_dollar (agent* thisAgent) {
  thisAgent->lexeme.type = DOLLAR_STRING_LEXEME;
  thisAgent->lexeme.string[0] = '$';
  thisAgent->lexeme.length = 1;
  get_next_char(thisAgent);   /* consume the '$' */
  while ((thisAgent->current_char!='\n') &&
	 (thisAgent->current_char!=EOF_AS_CHAR) &&
	 (thisAgent->lexeme.length < MAX_LEXEME_LENGTH-1)) {
    thisAgent->lexeme.string[thisAgent->lexeme.length++] =
      thisAgent->current_char;
    get_next_char(thisAgent);
  }
  thisAgent->lexeme.string[thisAgent->lexeme.length] = '\0';
}

/*
void lex_dollar (void) {
  store_and_advance();
  finish();
  thisAgent->lexeme.type = DOLLAR_STRING_LEXEME;
}
*/

/* AGR 562 end */

/* ======================================================================
                             Get lexeme

  This is the main routine called from outside the lexer.  It reads past 
  any whitespace, then calls some lex_xxx routine (using the lexer_routines[]
  table) based on the first character of the lexeme.
====================================================================== */

void get_lexeme (agent* thisAgent) {

  /* AGR 568 begin */
  if (thisAgent->lex_alias) {
    thisAgent->lexeme = thisAgent->lex_alias->lexeme;
    thisAgent->lex_alias = thisAgent->lex_alias->next;
    return;
  }
  /* AGR 568 end */

  thisAgent->lexeme.length = 0;
  thisAgent->lexeme.string[0] = 0;

//#ifndef USE_X_DISPLAY
if (thisAgent->lexeme.type==EOF_LEXEME && 
    reading_from_top_level(thisAgent) &&
    current_lexer_parentheses_level(thisAgent)==0 &&  /* AGR 534 */
    thisAgent->print_prompt_flag)
//#ifdef USE_TCL
  {}
//#else
//
// /* REW: begin 09.15.96 */
// if (thisAgent->operand2_mode == TRUE)
//   print ("\nOPERAND %s> ", thisAgent->name);
// /* REW: end   09.15.96 */
// else
//   print ("\n%s> ", thisAgent->name);
//
//#endif /* USE_TCL */
//#endif /* USE_X_DISPLAY */

/* AGR 534  The only time a prompt should be printed out is if there's
   a command being expected; ie. the prompt shouldn't print out if we're
   in the middle of entering a production.  So if we're in the middle of
   entering a production, then the parentheses level will be > 0, so that's
   the criteria we will use.  AGR  5-Apr-94  */

  thisAgent->load_errors_quit = FALSE;  /* AGR 527c */

  while (thisAgent->load_errors_quit==FALSE) {   /* AGR 527c */
    if (thisAgent->current_char==EOF_AS_CHAR) break;
    if (whitespace[(unsigned char)thisAgent->current_char]) {
      if (thisAgent->current_char == '\n')
      {    
         if (thisAgent->current_file->fake_rparen_at_eol) {
              do_fake_rparen(thisAgent);
              return;
         }
//#ifndef USE_X_DISPLAY
         if (current_lexer_parentheses_level(thisAgent)==0 &&  /* AGR 534 */
             thisAgent->print_prompt_flag)
//#ifdef USE_TCL
         {}
//#else
//
//	 /* REW: begin 09.15.96 */
//         if (thisAgent->operand2_mode == TRUE)
//	   print ("\nOPERAND %s> ", thisAgent->name);
//	 /* REW: end   09.15.96 */
//	 else
//	   print ("\n%s> ", thisAgent->name);
//
//#endif /* USE_TCL */
//#endif /* USE_X_DISPLAY */
      }
      get_next_char(thisAgent);
      continue;
    }

//#ifdef USE_TCL 
    if (thisAgent->current_char==';') {
      /* --- skip the semi-colon, forces newline in TCL --- */
      get_next_char(thisAgent);  /* consume it */
      continue;
    }
    if (thisAgent->current_char=='#') {
      /* --- read from hash to end-of-line --- */
      while ((thisAgent->current_char!='\n') &&
             (thisAgent->current_char!=EOF_AS_CHAR))
        get_next_char(thisAgent);
      if (thisAgent->current_file->fake_rparen_at_eol) {
        do_fake_rparen(thisAgent);
        return;
      }
      if (thisAgent->current_char!=EOF_AS_CHAR) get_next_char(thisAgent);
      continue;
    }
//#else
//    if (thisAgent->current_char==';') {
//      /* --- read from semicolon to end-of-line --- */
//      while ((thisAgent->current_char!='\n') &&
//             (thisAgent->current_char!=EOF_AS_CHAR))
//        get_next_char(thisAgent);
//      if (thisAgent->current_file->fake_rparen_at_eol) {
//        do_fake_rparen(thisAgent);
//        return;
//      }
//      if (thisAgent->current_char!=EOF_AS_CHAR) get_next_char(thisAgent);
//      continue;
//    }
//    if (thisAgent->current_char=='#') {
//      /* --- comments surrounded by "#|" and "|#" delimiters --- */
//      record_position_of_start_of_lexeme(); /* in case of later error mesg. */
//      get_next_char(thisAgent);
//      if (thisAgent->current_char!='|') {
//        print ("Error: '#' not followed by '|'\n");
//        print_location_of_most_recent_lexeme(thisAgent);
//        continue;
//      }
//      get_next_char(thisAgent);  /* consume the vbar */
//      while (TRUE) {
//        if (thisAgent->current_char==EOF_AS_CHAR) {
//          print ("Error: '#|' without terminating '|#'\n");
//          print_location_of_most_recent_lexeme(thisAgent);
//          break;
//        }
//        if (thisAgent->current_char!='|') { get_next_char(thisAgent); continue; }
//        get_next_char(thisAgent);
//        if (thisAgent->current_char=='#') break;
//      }
//      get_next_char(thisAgent);  /* consume the closing '#' */
//      continue; /* continue outer while(TRUE), reading more whitespace */
//    }
//#endif  /* USE_TCL */
    break; /* if no whitespace or comments found, break out of the loop */
  }
  /* --- no more whitespace, so go get the actual lexeme --- */
  record_position_of_start_of_lexeme(thisAgent);
  if (thisAgent->current_char!=EOF_AS_CHAR)
    (*(lexer_routines[(unsigned char)thisAgent->current_char]))(thisAgent);
  else
    lex_eof(thisAgent);
}
  
/* ======================================================================
                            Init lexer

  This should be called before anything else in this file.  It does all 
  the necessary init stuff for the lexer, and starts the lexer reading from
  standard input.
====================================================================== */


//
// This file badly need to be locked.  Probably not the whole thing, but certainly the last
// call to start_lext_from_file.  It does a memory allocation and other things that should
// never happen more than once.
//
void init_lexer (agent* thisAgent) 
{
  static bool initialized = false;

  if(!initialized) 
  {
     initialized = true;

     unsigned int i;

     /* --- setup constituent_char array --- */
     char extra_constituents[] = "$%&*+-/:<=>?_";
     for (i=0; i<256; i++)
     {
        //
        // When i == 1, strchr returns true based on the terminating
        // character.  This is not the intent, so we exclude that case
        // here.
        //
        if((strchr(extra_constituents, (char)(i)) != 0) && i != 0)
        {
           constituent_char[i]=TRUE;
        }
        else
        {
           constituent_char[i] = (isalnum(i) != 0);
        }
     }

   //  for (i=0; i<strlen(extra_constituents); i++)
   //  {
   //    constituent_char[(int)extra_constituents[i]]=TRUE;
   //  }
  
     /* --- setup whitespace array --- */
     for (i=0; i<256; i++)
     {
       whitespace[i] = (isspace(i) != 0);
     }

     /* --- setup number_starters array --- */
     for (i=0; i<256; i++)
     {
       switch(i)
       {
       case '+':
          number_starters[(int)'+']=TRUE;
          break;
       case '-':
          number_starters[(int)'-']=TRUE;
          break;
       case '.':
          number_starters[(int)'.']=TRUE;
          break;
       default:
          number_starters[i] = (isdigit(i) != 0);
       }
     }

     /* --- setup lexer_routines array --- */
     //
     // I go to some effort here to insure that values do not
     // get overwritten.  That could cause problems in a multi-
     // threaded sense because values could get switched to one
     // value and then another.  If a value is only ever set to
     // one thing, resetting it to the same thing should be 
     // perfectly safe.
     //
     for (i=0; i<256; i++)
     {
        switch(i)
        {
        case '@':
           lexer_routines[(int)'@'] = lex_at;
           break;
        case '(':
           lexer_routines[(int)'('] = lex_lparen;
           break;
        case ')':
           lexer_routines[(int)')'] = lex_rparen;
           break;
        case '+':
           lexer_routines[(int)'+'] = lex_plus;
           break;
        case '-':
           lexer_routines[(int)'-'] = lex_minus;
           break;
        case '~':
           lexer_routines[(int)'~'] = lex_tilde;
           break;
        case '^':
           lexer_routines[(int)'^'] = lex_up_arrow;
           break;
        case '{':
           lexer_routines[(int)'{'] = lex_lbrace;
           break;
        case '}':
           lexer_routines[(int)'}'] = lex_rbrace;
           break;
        case '!':
           lexer_routines[(int)'!'] = lex_exclamation_point;
           break;
        case '>':
           lexer_routines[(int)'>'] = lex_greater;
           break;
        case '<':
           lexer_routines[(int)'<'] = lex_less;
           break;
        case '=':
           lexer_routines[(int)'='] = lex_equal;
           break;
        case '&':
           lexer_routines[(int)'&'] = lex_ampersand;
           break;
        case '|':
           lexer_routines[(int)'|'] = lex_vbar;
           break;
        case ',':
           lexer_routines[(int)','] = lex_comma;
           break;
        case '.':
           lexer_routines[(int)'.'] = lex_period;
           break;
        case '"':
           lexer_routines[(int)'"'] = lex_quote;
           break;
        case '$':
           lexer_routines[(int)'$'] = lex_dollar;   /* AGR 562 */
           break;
        default:
           if (isdigit(i)) 
           {
              lexer_routines[i] = lex_digit;
              continue;
           }

           if (constituent_char[i]) 
           {
              lexer_routines[i] = lex_constituent_string;
              continue;
           } 
        }
     }
  }

  /* --- initially we're reading from the standard input --- */
  start_lex_from_file (thisAgent, "[standard input]", stdin);
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

void print_location_of_most_recent_lexeme (agent* thisAgent) {
  int i;
  
  if (thisAgent->current_file->line_of_start_of_last_lexeme ==
      thisAgent->current_file->current_line) {
    /* --- error occurred on current line, so print out the line --- */
    if (! reading_from_top_level(thisAgent)) {
      print (thisAgent, "File %s, line %lu:\n", thisAgent->current_file->filename,
             thisAgent->current_file->current_line);
      /*       respond_to_load_errors ();     AGR 527a */
    }
    if (thisAgent->current_file->buffer[strlen(thisAgent->current_file->buffer)-1]=='\n')
      print_string (thisAgent, thisAgent->current_file->buffer);
    else
      print (thisAgent, "%s\n",thisAgent->current_file->buffer);
    for (i=0; i<thisAgent->current_file->column_of_start_of_last_lexeme; i++)
      print_string (thisAgent, "-");
    print_string (thisAgent, "^\n");

    if (! reading_from_top_level(thisAgent)) {
      //respond_to_load_errors (thisAgent); /* AGR 527a */
      if (thisAgent->load_errors_quit)
	thisAgent->current_char = EOF_AS_CHAR;
    }

/* AGR 527a  The respond_to_load_errors call came too early (above),
   and the "continue" prompt appeared before the offending line was printed
   out, so the respond_to_load_errors call was moved here.
   AGR 26-Apr-94 */

  } else {
    /* --- error occurred on a previous line, so just give the position --- */
    print (thisAgent, "File %s, line %lu, column %lu.\n", thisAgent->current_file->filename,
           thisAgent->current_file->line_of_start_of_last_lexeme,
           thisAgent->current_file->column_of_start_of_last_lexeme + 1);
    if (! reading_from_top_level(thisAgent)) {
      //respond_to_load_errors (thisAgent);
      if (thisAgent->load_errors_quit)
	thisAgent->current_char = EOF_AS_CHAR;
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

int current_lexer_parentheses_level (agent* thisAgent) {
  return thisAgent->current_file->parentheses_level;
}

void skip_ahead_to_balanced_parentheses (agent* thisAgent, 
										 int parentheses_level) {
  while (TRUE) {
    if (thisAgent->lexeme.type==EOF_LEXEME) return;
    if ((thisAgent->lexeme.type==R_PAREN_LEXEME) &&
        (parentheses_level==thisAgent->current_file->parentheses_level)) return;
    get_lexeme(thisAgent);
  }
}

void fake_rparen_at_next_end_of_line (agent* thisAgent) {
  thisAgent->current_file->parentheses_level++;  
  thisAgent->current_file->fake_rparen_at_eol = TRUE;  
}

/* ======================================================================
                        Set lexer allow ids

  This routine should be called to tell the lexer whether to allow
  identifiers to be read.  If FALSE, things that look like identifiers
  will be returned as SYM_CONSTANT_LEXEME's instead.
====================================================================== */

void set_lexer_allow_ids (agent* thisAgent, Bool allow_identifiers) {
  thisAgent->current_file->allow_ids = allow_identifiers;
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
                                                 Bool *possible_id,
                                                 Bool *possible_var,
                                                 Bool *possible_sc,
                                                 Bool *possible_ic,
                                                 Bool *possible_fc,
                                                 Bool *rereadable) {
  char *ch;
  Bool rereadability_dead, rereadability_questionable;

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

