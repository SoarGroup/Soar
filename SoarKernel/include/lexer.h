/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                             lexer.h

  The lexer reads files and returns a stream of lexemes.  Get_lexeme() is
  the main routine; it looks for the next lexeme in the input, and stores
  it in the global variable "lexeme".  See the structure definition below.

  Restrictions:  the lexer cannot read individual input lines longer than
  MAX_LEXER_LINE_LENGTH characters.  Thus, a single lexeme can't be longer
  than that either.

  The lexer maintains a stack of files being read, in order to handle nested
  loads.  Start_lex_from_file() and stop_lex_from_file() push and pop the
  stack.  Immediately after start_lex_from_file(), the current lexeme (global
  variable) is undefined.  Immediately after stop_lex_from_file(), the 
  current lexeme is automatically restored to whatever it was just before
  the corresponding start_lex_from_file() call.
  
  Determine_possible_symbol_types_for_string() is a utility routine which
  figures out what kind(s) of symbol a given string could represent.
  
  Print_location_of_most_recent_lexeme() is used to print an indication
  of where a parser error occurred.  It tries to print out the current
  source line with a pointer to where the error was detected.
  
  Current_lexer_parentheses_level() returns the current level of parentheses
  nesting (0 means no open paren's have been encountered).
  Skip_ahead_to_balanced_parentheses() eats lexemes until the appropriate
  closing paren is found (0 means eat until back at the top level).
  
  Fake_rparen_at_next_end_of_line() tells the lexer to insert a fake
  R_PAREN_LEXEME token the next time it reaches the end of a line.
  
  Set_lexer_allow_ids() tells the lexer whether to allow identifiers to
  be read.  If FALSE, things that look like identifiers will be returned
  as SYM_CONSTANT_LEXEME's instead.
====================================================================== */

#ifndef LEXER_H
#define LEXER_H

#ifdef __cplusplus
extern "C"
{
#endif

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

struct lexeme_info {
  enum lexer_token_type type;         /* what kind of lexeme it is */
  char string[MAX_LEXEME_LENGTH+1];   /* text of the lexeme */
  int length;                         /* length of the above string */
  long int_val;                       /* for INT_CONSTANT_LEXEME's */
  float float_val;                    /* for FLOAT_CONSTANT_LEXEME's */
  char id_letter;                     /* for IDENTIFIER_LEXEME's */
  unsigned long id_number;            /* for IDENTIFIER_LEXEME's */
};

extern void determine_possible_symbol_types_for_string (char *s,
                                                        int length_of_s,
                                                        Bool *possible_id,
                                                        Bool *possible_var,
                                                        Bool *possible_sc,
                                                        Bool *possible_ic,
                                                        Bool *possible_fc,
                                                        Bool *rereadable);

extern void init_lexer (agent* thisAgent);
extern void start_lex_from_file (agent* thisAgent, char *filename, 
								 FILE *already_opened_file);
extern void stop_lex_from_file (agent* thisAgent);

extern void get_lexeme (agent* thisAgent);
extern void print_location_of_most_recent_lexeme (agent* thisAgent);

extern int current_lexer_parentheses_level (agent* thisAgent);
extern void skip_ahead_to_balanced_parentheses (agent* thisAgent, 
												int parentheses_level);
extern void fake_rparen_at_next_end_of_line (agent* thisAgent);
extern void set_lexer_allow_ids (agent* thisAgent, Bool allow_identifiers);

extern Bool determine_type_of_constituent_string (agent* thisAgent);

extern double my_strtod (char *ch, char **p, int base); /* in lexer.cpp */

/* (RBD) the rest of this stuff shouldn't be in the module interface... */

#define BUFSIZE (MAX_LEXER_LINE_LENGTH+2) /* +2 for newline and null at end */

/* --- we'll use one of these structures for each file being read --- */

typedef struct lexer_source_file_struct {
  struct lexer_source_file_struct *parent_file;
  char *filename;
  FILE *file;
  Bool fake_rparen_at_eol;
  Bool allow_ids;
  int parentheses_level;    /* 0 means top level, no left paren's seen */
  int current_column;       /* column number of next char to read (0-based) */
  unsigned long current_line;   /* line number of line in buffer (1-based) */
  int column_of_start_of_last_lexeme;   /* (used for error messages) */
  unsigned long line_of_start_of_last_lexeme;
  char buffer[BUFSIZE];              /* holds text of current input line */
  struct lexeme_info saved_lexeme;   /* save/restore it during nested loads */
  char saved_current_char;           /* save/restore this too */
} lexer_source_file;

#ifdef __cplusplus
}
#endif

#endif // LEXER_H
