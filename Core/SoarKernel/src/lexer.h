/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                             lexer.h

  The lexer reads strings and returns a stream of lexemes.  Get_lexeme() is
  the main routine; it looks for the next lexeme in the input, and stores
  it in the global variable "lexeme".  See the structure definition below.

  Restrictions:  the lexer cannot read individual input lines longer than
  MAX_LEXER_LINE_LENGTH characters.  Thus, a single lexeme can't be longer
  than that either.

  Determine_possible_symbol_types_for_string() is a utility routine which
  figures out what kind(s) of symbol a given string could represent.
  
  Print_location_of_most_recent_lexeme() is used to print an indication
  of where a parser error occurred.  It tries to print out the current
  source line with a pointer to where the error was detected.
  
  Current_lexer_parentheses_level() returns the current level of parentheses
  nesting (0 means no open paren's have been encountered).
  Skip_ahead_to_balanced_parentheses() eats lexemes until the appropriate
  closing paren is found (0 means eat until back at the top level).
  
  Set_lexer_allow_ids() tells the lexer whether to allow identifiers to
  be read.  If FALSE, things that look like identifiers will be returned
  as SYM_CONSTANT_LEXEME's instead.
====================================================================== */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>	// Needed for FILE token below

typedef char Bool;
typedef struct agent_struct agent;

#define MAX_LEXER_LINE_LENGTH 1000
#define MAX_LEXEME_LENGTH (MAX_LEXER_LINE_LENGTH+5) /* a little bigger to avoid
                                                       any off-by-one-errors */

enum lexer_token_type {
  EOF_LEXEME,                        /* end-of-file */
  IDENTIFIER_LEXEME,                 /* identifier */
  VARIABLE_LEXEME,                   /* variable */
  SYM_CONSTANT_LEXEME,               /* symbolic constant */
  INT_CONSTANT_LEXEME,               /* integer constant */
  FLOAT_CONSTANT_LEXEME,             /* floating point constant */
  L_PAREN_LEXEME,                    /* "(" */
  R_PAREN_LEXEME,                    /* ")" */
  L_BRACE_LEXEME,                    /* "{" */
  R_BRACE_LEXEME,                    /* "}" */
  PLUS_LEXEME,                       /* "+" */
  MINUS_LEXEME,                      /* "-" */
  RIGHT_ARROW_LEXEME,                /* "-->" */
  GREATER_LEXEME,                    /* ">" */
  LESS_LEXEME,                       /* "<" */
  EQUAL_LEXEME,                      /* "=" */
  LESS_EQUAL_LEXEME,                 /* "<=" */
  GREATER_EQUAL_LEXEME,              /* ">=" */
  NOT_EQUAL_LEXEME,                  /* "<>" */
  LESS_EQUAL_GREATER_LEXEME,         /* "<=>" */
  LESS_LESS_LEXEME,                  /* "<<" */
  GREATER_GREATER_LEXEME,            /* ">>" */
  AMPERSAND_LEXEME,                  /* "&" */
  AT_LEXEME,                         /* "@" */
  TILDE_LEXEME,                      /* "~" */
  UP_ARROW_LEXEME,                   /* "^" */
  EXCLAMATION_POINT_LEXEME,          /* "!" */
  COMMA_LEXEME,                      /* "," */
  PERIOD_LEXEME,                     /* "." */
  QUOTED_STRING_LEXEME,              /* string in double quotes */
  DOLLAR_STRING_LEXEME,              /* string for shell escape */
  NULL_LEXEME };                     /* Initial value */          

#define LENGTH_OF_LONGEST_SPECIAL_LEXEME 3  /* length of "-->" and "<=>"--
                                               if a longer one is added, be
                                               sure to update this! */

//TODO: is there anything that prevents the max length from being exceeded?
//that would be a memory error.
struct lexeme_info {
  enum lexer_token_type type;         /* what kind of lexeme it is */
  char string[MAX_LEXEME_LENGTH+1];   /* text of the lexeme */
  int length;                         /* length of the above string */
  int64_t int_val;                     /* for INT_CONSTANT_LEXEME's */
  double float_val;                    /* for FLOAT_CONSTANT_LEXEME's */
  char id_letter;                     /* for IDENTIFIER_LEXEME's */
  uint64_t id_number;                 /* for IDENTIFIER_LEXEME's */
};

extern void determine_possible_symbol_types_for_string (char *s,
                                                        size_t length_of_s,
                                                        Bool *possible_id,
                                                        Bool *possible_var,
                                                        Bool *possible_sc,
                                                        Bool *possible_ic,
                                                        Bool *possible_fc,
                                                        Bool *rereadable);

extern void init_lexer (agent* thisAgent);

extern void get_lexeme (agent* thisAgent);
extern void print_location_of_most_recent_lexeme (agent* thisAgent);

extern int current_lexer_parentheses_level (agent* thisAgent);
extern void skip_ahead_to_balanced_parentheses (agent* thisAgent, 
												int parentheses_level);
extern void set_lexer_allow_ids (agent* thisAgent, Bool allow_identifiers);
extern Bool get_lexer_allow_ids (agent* thisAgent);

extern Bool determine_type_of_constituent_string (agent* thisAgent);

/* (RBD) the rest of this stuff shouldn't be in the module interface... */

#define BUFSIZE (MAX_LEXER_LINE_LENGTH+2) /* +2 for newline and null at end */

#endif // LEXER_H
