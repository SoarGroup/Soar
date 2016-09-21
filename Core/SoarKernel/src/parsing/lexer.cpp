/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  lexer.cpp
 *
 * ===================================================================== */


#include "lexer.h"

#include "agent.h"
#include "output_manager.h"
#include "print.h"
#include "misc.h"
#include "xml.h"

#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

using soar::Lexer;
using soar::Lexeme;

//static lexing structures
Lexer::lex_func_ptr Lexer::lexer_routines[256];
bool Lexer::constituent_char[256];
bool Lexer::whitespace[256];
bool Lexer::number_starters[256];
//initialize them all here
bool Lexer::initialized = init();

void Lexer::get_next_char () {
    //    char *s;

    if(current_char == EOF) {
        prev_char = EOF;
        return;
    }

    if (production_string == NULL) {
        current_char = EOF;
        prev_char = EOF;
        return;
    }
    prev_char = current_char;
    current_char = *production_string++;
    if (current_char == '\0')
    {
        current_char = EOF;
        return;
    }
}

/* ======================================================================
                         Lexer Utility Routines
====================================================================== */

inline void Lexer::record_position_of_start_of_lexeme()
{
  //TODO: rewrite this, since the lexer no longer keeps track of files
  // current_file->column_of_start_of_last_lexeme =
  //   current_file->current_column - 1;
  // current_file->line_of_start_of_last_lexeme =
  //   current_file->current_line;
}

inline void Lexer::store_and_advance()
{
    current_lexeme.lex_string.append(1, char(current_char));
    get_next_char();
}

void Lexer::read_constituent_string () {
  while ((current_char!=EOF) &&
         constituent_char[static_cast<unsigned char>(current_char)])
    store_and_advance();
}

// At entry, current_char=="."; we read the "." and rest of number.
bool Lexer::read_rest_of_floating_point_number () {
  // Save the lexer input location, first; if we see unseparated
  // constituent characters at the end, then restore the state and return
  // false, meaning the input was not a floating point number after all.

  //store state
  std::string old_lexeme =  current_lexeme.lex_string;
  int old_cur_char = current_char;
  int old_prev_char = prev_char;
  const char* old_prod_string = production_string;

  store_and_advance();
  while (isdigit(current_char)){
    store_and_advance(); /* string of digits */
  }
  if ((current_char=='e')||(current_char=='E')) {
    store_and_advance();                             /* E */
    if ((current_char=='+')||(current_char=='-'))
      store_and_advance();                       /* optional leading + or - */
    while (isdigit(current_char)) store_and_advance(); /* string of digits */
  }
  // if characters were read restore state if it wasn't a float after all
  if(constituent_char[(int)current_char] && !isspace(prev_char)){
    current_lexeme.lex_string = old_lexeme;
    current_char = old_cur_char;
    prev_char = old_prev_char;
    production_string = old_prod_string;
    return false;
  }
  return true;
}

bool Lexer::determine_type_of_constituent_string () {
    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool rereadable;

    determine_possible_symbol_types_for_string (current_lexeme.string(),
        current_lexeme.length(),
        &possible_id,
        &possible_var,
        &possible_sc,
        &possible_ic,
        &possible_fc,
        &rereadable);

    if (possible_var) {
        current_lexeme.type = VARIABLE_LEXEME;
        return true;
    }

    if (possible_ic) {
        errno = 0;
        current_lexeme.type = INT_CONSTANT_LEXEME;
        current_lexeme.int_val = strtol (current_lexeme.string(),NULL,10);
        if (errno) {
            thisAgent->outputManager->printa(thisAgent, "Error: bad integer (probably too large)\n");
            current_lexeme.int_val = 0;
        }
        return (errno == 0);
    }

    if (possible_fc) {
        errno = 0;
        current_lexeme.type = FLOAT_CONSTANT_LEXEME;
        current_lexeme.float_val = strtod (current_lexeme.string(),NULL);
        if (errno) {
            thisAgent->outputManager->printa(thisAgent, "Error: bad floating point number\n");
            current_lexeme.float_val = 0.0;
        }
        return (errno == 0);
    }

    if (possible_id)
        {
            errno = 0;
        current_lexeme.id_letter = static_cast<char>(toupper(current_lexeme.lex_string[0]));
            current_lexeme.type = IDENTIFIER_LEXEME;
        if (!from_c_string(current_lexeme.id_number, &(current_lexeme.lex_string[1]))) {
                thisAgent->outputManager->printa(thisAgent, "Error: bad number for identifier (probably too large)\n");
                current_lexeme.id_number = 0;
                errno = 1;
                this->lex_error = true;
            }
        return (errno == 0);
    }

    if (possible_sc) {
        current_lexeme.type = STR_CONSTANT_LEXEME;
        if (thisAgent->outputManager->settings[OM_WARNINGS]) {
            if ( (current_lexeme.lex_string[0] == '<') ||
                 (current_lexeme.lex_string[current_lexeme.length()-1] == '>') )
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Warning: Suspicious string constant \"%s\"\n", current_lexeme.string());
                xml_generate_warning(thisAgent, "Warning: Suspicious string constant");
            }
        }
        return true;
    }

    current_lexeme.type = QUOTED_STRING_LEXEME;
    return true;
}

/* ======================================================================
                        Lex such-and-such Routines
  These routines are called from get_lexeme().  Which routine gets called
  depends on the first character of the new lexeme being read.  Each routine's
  job is to finish reading the lexeme and store the necessary items in
  the member variable "lexeme".
====================================================================== */

void Lexer::lex_eof () {
  store_and_advance();
  current_lexeme.type = EOF_LEXEME;
}

void Lexer::lex_at () {
    //store state
    int old_cur_char = current_char;
    int old_prev_char = prev_char;
    const char* old_prod_string = production_string;

    read_constituent_string();
    if (current_lexeme.length()==2)
    {
        if (current_lexeme.lex_string[1]=='+')
        {
            current_lexeme.type = UNARY_AT_LEXEME;
            return;
        }
        if (current_lexeme.lex_string[1]=='-')
        {
            current_lexeme.type = UNARY_NOT_AT_LEXEME;
            return;
        }
    }

    current_char = old_cur_char;
    prev_char = old_prev_char;
    production_string = old_prod_string;

  store_and_advance();
  current_lexeme.type = AT_LEXEME;
}

void Lexer::lex_tilde () {
  store_and_advance();
  current_lexeme.type = TILDE_LEXEME;
}

void Lexer::lex_up_arrow () {
  store_and_advance();
  current_lexeme.type = UP_ARROW_LEXEME;
}

void Lexer::lex_lbrace () {
  store_and_advance();
  current_lexeme.type = L_BRACE_LEXEME;
}

void Lexer::lex_rbrace () {
  store_and_advance();
  current_lexeme.type = R_BRACE_LEXEME;
}

void Lexer::lex_exclamation_point () {

    if (production_string[0]=='@')
    {
        store_and_advance();
  store_and_advance();
        current_lexeme.type = NOT_AT_LEXEME;
        return;
    }


    store_and_advance();
  current_lexeme.type = EXCLAMATION_POINT_LEXEME;

}

void Lexer::lex_comma () {
  store_and_advance();
  current_lexeme.type = COMMA_LEXEME;
}

void Lexer::lex_equal () {
  /* Lexeme might be "=", or symbol */
  /* Note: this routine relies on = being a constituent character */

  read_constituent_string();
  if (current_lexeme.length()==1) { current_lexeme.type = EQUAL_LEXEME; return; }
  determine_type_of_constituent_string();
}

void Lexer::lex_ampersand () {
  /* Lexeme might be "&", or symbol */
  /* Note: this routine relies on & being a constituent character */

  read_constituent_string();
  if (current_lexeme.length()==1) { current_lexeme.type = AMPERSAND_LEXEME; return; }
  determine_type_of_constituent_string();
}

void Lexer::lex_lparen () {
  store_and_advance();
  current_lexeme.type = L_PAREN_LEXEME;
  parentheses_level++;
}

void Lexer::lex_rparen () {
  store_and_advance();
  current_lexeme.type = R_PAREN_LEXEME;
  if (parentheses_level > 0) parentheses_level--;
}

void Lexer::lex_greater () {
  /* Lexeme might be ">", ">=", ">>", or symbol */
  /* Note: this routine relies on =,> being constituent characters */

  read_constituent_string();
  if (current_lexeme.length()==1) { current_lexeme.type = GREATER_LEXEME; return; }
  if (current_lexeme.length()==2) {
    if (current_lexeme.lex_string[1]=='>') { current_lexeme.type = GREATER_GREATER_LEXEME; return;}
    if (current_lexeme.lex_string[1]=='=') { current_lexeme.type = GREATER_EQUAL_LEXEME; return; }
  }
  determine_type_of_constituent_string();
}

void Lexer::lex_less () {
    /* Lexeme might be "<", "<=", "<=>", "<@>", "<>", "<<", or variable */
  /* Note: this routine relies on =,<,> being constituent characters */

  read_constituent_string();
  if (current_lexeme.length()==1) { current_lexeme.type = LESS_LEXEME; return; }
  if (current_lexeme.length()==2) {
    if (current_lexeme.lex_string[1]=='>') { current_lexeme.type = NOT_EQUAL_LEXEME; return; }
    if (current_lexeme.lex_string[1]=='=') { current_lexeme.type = LESS_EQUAL_LEXEME; return; }
    if (current_lexeme.lex_string[1]=='<') { current_lexeme.type = LESS_LESS_LEXEME; return; }
  }
  if (current_lexeme.length()==3) {
    if ((current_lexeme.lex_string[1]=='=')&&(current_lexeme.lex_string[2]=='>'))
      { current_lexeme.type = LESS_EQUAL_GREATER_LEXEME; return; }
  }
  determine_type_of_constituent_string();
}

/**
 * Lexes a section of input beginning with a period (".").
 * Sometimes it is ambiguous whether a string is a floating
 * point value or a dot followed by a test:
 * ^number.1 or ^<number>.1 each have a WME named "1", but
 * ^number .1 and ^number {.1 .2} have floating point values
 *
 * This ambiguity is resolved by looking at the previous character
 * and lexeme. After a space, ".#" may be lexed as a single number
 * (FLOAT_CONSTANT_LEXEME). Directly after a variable or
 * string constant with no intervening space, "." is interpreted
 * as PERIOD_LEXEME and trailing numbers are lexed separately.
 */
void Lexer::lex_period () {
  // Disambiguate floating point numbers from WME names:
  bool float_disallowed = !isspace(prev_char) &&
    (current_lexeme.type == STR_CONSTANT_LEXEME ||
      current_lexeme.type == VARIABLE_LEXEME);

  store_and_advance();

  if (!float_disallowed && isdigit(current_char)) {
    read_rest_of_floating_point_number();
  }
  if (current_lexeme.length()==1) {
    current_lexeme.type = PERIOD_LEXEME;
    return;
  }
  determine_type_of_constituent_string();
}

void Lexer::lex_plus () {
  /* Lexeme might be +, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  bool could_be_floating_point;

  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_char=='.') {
    could_be_floating_point = true;
    for (i=1; i<current_lexeme.length(); i++)
      if (! isdigit(current_lexeme.lex_string[i])) could_be_floating_point = false;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  if (current_lexeme.length()==1) { current_lexeme.type = PLUS_LEXEME; return; }
  determine_type_of_constituent_string();
}

void Lexer::lex_minus () {
  /* Lexeme might be -, -->, number, or symbol */
  /* Note: this routine relies on various things being constituent chars */
  int i;
  bool could_be_floating_point;

  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_char=='.') {
    could_be_floating_point = true;
    for (i=1; i<current_lexeme.length(); i++)
      if (! isdigit(current_lexeme.lex_string[i])) could_be_floating_point = false;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  if (current_lexeme.length()==1) { current_lexeme.type = MINUS_LEXEME; return; }
  if (current_lexeme.length()==3) {
    if ((current_lexeme.lex_string[1]=='-')&&(current_lexeme.lex_string[2]=='>'))
      { current_lexeme.type = RIGHT_ARROW_LEXEME; return; }
  }
  determine_type_of_constituent_string();
}

void Lexer::lex_digit () {
  int i;
  bool could_be_floating_point;

  read_constituent_string();
  /* --- if we stopped at '.', it might be a floating-point number, so be
     careful to check for this case --- */
  if (current_char=='.') {
    could_be_floating_point = true;
    for (i=1; i<current_lexeme.length(); i++)
      if (! isdigit(current_lexeme.lex_string[i])) could_be_floating_point = false;
    if (could_be_floating_point) read_rest_of_floating_point_number();
  }
  determine_type_of_constituent_string();
}

void Lexer::lex_unknown () {
  get_next_char();
  get_lexeme();
}

void Lexer::lex_constituent_string () {
  read_constituent_string();
  determine_type_of_constituent_string();
}

void Lexer::lex_vbar () {
  current_lexeme.type = STR_CONSTANT_LEXEME;
  get_next_char();
  do {
    if (current_char==EOF) {
      thisAgent->outputManager->printa(thisAgent, "Error:  opening '|' without closing '|'\n");
      /* BUGBUG if reading from top level, don't want to signal EOF */
      current_lexeme.type = EOF_LEXEME;
      current_lexeme.lex_string = std::string(1, EOF);
      return;
    }
    if (current_char=='\\') {
      get_next_char();
      store_and_advance();
    } else if (current_char=='|') {
      get_next_char();
      break;
    } else {
      store_and_advance();
    }
  } while(true);
}

void Lexer::lex_quote () {
  current_lexeme.type = QUOTED_STRING_LEXEME;
  get_next_char();
  do {
    if (current_char==EOF) {
      thisAgent->outputManager->printa(thisAgent, "Error:  opening '\"' without closing '\"'\n");
      /* BUGBUG if reading from top level, don't want to signal EOF */
      current_lexeme.type = EOF_LEXEME;
      current_lexeme.lex_string = std::string(1, EOF);
      return;
    }
    if (current_char=='\\') {
      get_next_char();
      store_and_advance();
    } else if (current_char=='"') {
      get_next_char();
      break;
    } else {
      store_and_advance();
    }
  } while(true);
}

/* ======================================================================
                             Get lexeme
  This is the main routine called from outside the lexer.  It reads past
  any whitespace, then calls some lex_xxx routine (using the lexer_routines[]
  table) based on the first character of the lexeme.
====================================================================== */

bool Lexer::get_lexeme () {

  current_lexeme.lex_string = "";

  consume_whitespace_and_comments();

  // dispatch to lexer routine by first character in lexeme
  record_position_of_start_of_lexeme();
  lex_error = false;
  if (current_char!=EOF)
    (this->*lexer_routines[static_cast<unsigned char>(current_char)])();
  else
    lex_eof();
  if (lex_error)
  {
      thisAgent->outputManager->printa(thisAgent,  "Parsing error.\n");
      return false;
  }
  return true;
}

void Lexer::consume_whitespace_and_comments()
{
  // loop until whitespace and comments are gone
  while (true) {
    if (current_char==EOF) break;
    if (whitespace[static_cast<unsigned char>(current_char)]) {
      get_next_char();
      continue;
    }

    //skip the semi-colon, forces newline in TCL
    if (current_char==';') {
      get_next_char();
      continue;
    }
    //hash is end-of-line comment; read to the end
    if (current_char=='#') {
      while ((current_char!='\n') &&
             (current_char!=EOF))
        get_next_char();
      if (current_char!=EOF) get_next_char();
      continue;
    }
    //if no whitespace or comments found, break out of the loop
    break;
  }
}

// Static initialization function to set up lexing structures and the
// lexer_routine dispatch table.
bool Lexer::init ()
{
  unsigned int i;

  /* --- setup constituent_char array --- */
  char extra_constituents[] = "$%&*+-/:<=>?_@";
  for (i=0; i<256; i++)
  {
    //
    // When i == 1, strchr returns true based on the terminating
    // character.  This is not the intent, so we exclude that case
    // here.
    //
    if((strchr(extra_constituents, i) != 0) && i != 0)
    {
      constituent_char[i]=true;
    }
    else
    {
      constituent_char[i] = (isalnum(i) != 0);
    }
  }

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
       number_starters[(int)'+']=true;
       break;
    case '-':
       number_starters[(int)'-']=true;
       break;
    case '.':
       number_starters[(int)'.']=true;
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
      lexer_routines[(int)'@'] = &Lexer::lex_at;
      break;
    case '(':
      lexer_routines[(int)'('] = &Lexer::lex_lparen;
      break;
    case ')':
      lexer_routines[(int)')'] = &Lexer::lex_rparen;
      break;
    case '+':
      lexer_routines[(int)'+'] = &Lexer::lex_plus;
      break;
    case '-':
      lexer_routines[(int)'-'] = &Lexer::lex_minus;
      break;
    case '~':
      lexer_routines[(int)'~'] = &Lexer::lex_tilde;
      break;
    case '^':
      lexer_routines[(int)'^'] = &Lexer::lex_up_arrow;
      break;
    case '{':
      lexer_routines[(int)'{'] = &Lexer::lex_lbrace;
      break;
    case '}':
      lexer_routines[(int)'}'] = &Lexer::lex_rbrace;
      break;
    case '!':
      lexer_routines[(int)'!'] = &Lexer::lex_exclamation_point;
      break;
    case '>':
      lexer_routines[(int)'>'] = &Lexer::lex_greater;
      break;
    case '<':
      lexer_routines[(int)'<'] = &Lexer::lex_less;
      break;
    case '=':
      lexer_routines[(int)'='] = &Lexer::lex_equal;
      break;
    case '&':
      lexer_routines[(int)'&'] = &Lexer::lex_ampersand;
      break;
    case '|':
      lexer_routines[(int)'|'] = &Lexer::lex_vbar;
      break;
    case ',':
      lexer_routines[(int)','] = &Lexer::lex_comma;
      break;
    case '.':
      lexer_routines[(int)'.'] = &Lexer::lex_period;
      break;
    case '"':
      lexer_routines[(int)'"'] = &Lexer::lex_quote;
      break;
    default:
      if (isdigit(i))
      {
         lexer_routines[i] = &Lexer::lex_digit;
         continue;
      }
      if (constituent_char[i])
      {
         lexer_routines[i] = &Lexer::lex_constituent_string;
         continue;
      }
      lexer_routines[i] = &Lexer::lex_unknown;
    }
  }
  return true;
}

int Lexer::current_parentheses_level () {
  return parentheses_level;
}

void Lexer::skip_ahead_to_balanced_parentheses (int parentheses_level) {
  while (true) {
    if (current_lexeme.type==EOF_LEXEME) return;
    if ((current_lexeme.type==R_PAREN_LEXEME) &&
        (this->parentheses_level==parentheses_level)) return;
    get_lexeme();
  }
}

void Lexer::determine_possible_symbol_types_for_string (const char *s,
                                                 size_t length_of_s,
                                                 bool *possible_id,
                                                 bool *possible_var,
                                                 bool *possible_sc,
                                                 bool *possible_ic,
                                                 bool *possible_fc,
                                                 bool *rereadable) {
    const char *ch;
    bool all_alphanum;

    *possible_id = false;
    *possible_var = false;
    *possible_sc = false;
    *possible_ic = false;
    *possible_fc = false;
    *rereadable = false;

    /* --- check if it's an integer or floating point number --- */
    if (number_starters[static_cast<unsigned char>(*s)]) {
        ch = s;
        if ((*ch=='+')||(*ch=='-'))
            ch++;                               /* optional leading + or - */
        while (isdigit(*ch))
            ch++;                               /* string of digits */
        if ((*ch==0)&&(isdigit(*(ch-1))))
            *possible_ic = true;
        if (*ch=='.') {
            ch++;                               /* decimal point */
            while (isdigit(*ch))
                ch++;                           /* string of digits */
            if ((*ch=='e')||(*ch=='E')) {
                ch++;                           /* E */
                if ((*ch=='+')||(*ch=='-'))
                    ch++;                       /* optional leading + or - */
                while (isdigit(*ch))
                    ch++;                       /* string of digits */
            }
            if (*ch==0)
                *possible_fc = true;
        }
    }

    /* --- make sure it's entirely constituent characters --- */
    /* --- check for rereadability --- */
    all_alphanum = true;
    for (ch=s; *ch!='\0'; ch++) {
        if (!isalnum(*ch)) {
            all_alphanum = false;
        }
        if (!constituent_char[static_cast<unsigned char>(*ch)]) return;
    }
    if ( all_alphanum ||
         (length_of_s > length_of_longest_special_lexeme) ||
         ((length_of_s==1)&&(*s=='*')) )
    {
        *rereadable = true;
    }

    /* --- any string of constituents could be a sym constant --- */
    *possible_sc = true;

    /* --- check whether it's a variable --- */
    if ((*s=='<')&&(*(s+length_of_s-1)=='>'))
        *possible_var = true;

    /* --- check if it's an identifier --- */
    // long term identifiers start with @
        ch = s;

    if (isalpha(*ch) && *(++ch) != '\0') {
        /* --- is the rest of the string an integer? --- */
        while (isdigit(*ch))
            ch++;
        if (*ch=='\0')
            *possible_id = true;
    }
}

Lexer::Lexer(agent* agent, const char* string)
{
    thisAgent = agent;
    production_string = string;
    current_char = ' ';
    parentheses_level = 0;
    lex_error = false;

    //Initializing lexeme
    current_lexeme = Lexeme();
}

Lexeme Lexer::get_lexeme_from_string (agent* thisAgent, const char* input)
{
    Lexer lexer = Lexer(thisAgent, input);
    const char * c;
    lexer.current_lexeme.lex_string = "";
    lexer.consume_whitespace_and_comments();

    // dispatch to lexer routine by first character in lexeme
    lexer.record_position_of_start_of_lexeme();

    bool sym_constant_start_found = false;
    bool sym_constant_end_found = false;

    while (lexer.current_char!=EOF)
    {
        if (lexer.current_char=='|') {
          if (!sym_constant_start_found)
          {
              sym_constant_start_found = true;
          }
          else
          {
              sym_constant_end_found = true;
          }
        lexer.get_next_char();
      } else {
        lexer.store_and_advance();
      }
    };

    if (sym_constant_end_found)
    {
        lexer.current_lexeme.type = STR_CONSTANT_LEXEME;
    }
    else
    {
        lexer.determine_type_of_constituent_string();
    }
    return lexer.current_lexeme;
}
