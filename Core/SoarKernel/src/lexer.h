/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                             lexer.h

  The lexer reads strings and returns a stream of lexemes.  Get_lexeme() is
  the main routine; it looks for the next lexeme in the input, and stores
  it in the member variable current_lexeme.

  Restrictions:  the lexer cannot read individual input lines longer than
  MAX_LEXER_LINE_LENGTH characters.  Thus, a single lexeme can't be longer
  than that either.
====================================================================== */

#ifndef LEXER_H
#define LEXER_H

typedef char Bool;

#define MAX_LEXER_LINE_LENGTH 1000
//a little bigger to avoid any off-by-one-errors
#define MAX_LEXEME_LENGTH (MAX_LEXER_LINE_LENGTH+5) 

/**
 * Types of tokens read by the lexer
 */
enum lexer_token_type {
  EOF_LEXEME,                        /**< end-of-file */
  IDENTIFIER_LEXEME,                 /**< identifier */
  VARIABLE_LEXEME,                   /**< variable */
  SYM_CONSTANT_LEXEME,               /**< symbolic constant */
  INT_CONSTANT_LEXEME,               /**< integer constant */
  FLOAT_CONSTANT_LEXEME,             /**< floating point constant */
  L_PAREN_LEXEME,                    /**< "(" */
  R_PAREN_LEXEME,                    /**< ")" */
  L_BRACE_LEXEME,                    /**< "{" */
  R_BRACE_LEXEME,                    /**< "}" */
  PLUS_LEXEME,                       /**< "+" */
  MINUS_LEXEME,                      /**< "-" */
  RIGHT_ARROW_LEXEME,                /**< "-->" */
  GREATER_LEXEME,                    /**< ">" */
  LESS_LEXEME,                       /**< "<" */
  EQUAL_LEXEME,                      /**< "=" */
  LESS_EQUAL_LEXEME,                 /**< "<=" */
  GREATER_EQUAL_LEXEME,              /**< ">=" */
  NOT_EQUAL_LEXEME,                  /**< "<>" */
  LESS_EQUAL_GREATER_LEXEME,         /**< "<=>" */
  LESS_LESS_LEXEME,                  /**< "<<" */
  GREATER_GREATER_LEXEME,            /**< ">>" */
  AMPERSAND_LEXEME,                  /**< "&" */
  AT_LEXEME,                         /**< "@" */
  TILDE_LEXEME,                      /**< "~" */
  UP_ARROW_LEXEME,                   /**< "^" */
  EXCLAMATION_POINT_LEXEME,          /**< "!" */
  COMMA_LEXEME,                      /**< "," */
  PERIOD_LEXEME,                     /**< "." */
  QUOTED_STRING_LEXEME,              /**< string in double quotes */
  DOLLAR_STRING_LEXEME,              /**< string for shell escape */
  NULL_LEXEME                        /**< Initial value */ 
};         

namespace soar
{

    //TODO: is there anything that prevents the max length from being exceeded?
    //that would be a memory error.
    /**
     * A class representing a single lexeme.
     */
    class Lexeme {
        friend class Lexer;
    public:
        enum lexer_token_type type;         /**< what kind of lexeme it is */
        int length;                         /**< length of the above string */
        int64_t int_val;                    /**< for INT_CONSTANT_LEXEME's */
        double float_val;                   /**< for FLOAT_CONSTANT_LEXEME's */
        char id_letter;                     /**< for IDENTIFIER_LEXEME's */
        uint64_t id_number;                 /**< for IDENTIFIER_LEXEME's */
        /**
         * @return the text of the lexeme
         */
        char* string(){return lex_string;}
        /**
         * @return the length of the lexeme string
         */
        int size(){return length;}
    private:
        char lex_string[MAX_LEXEME_LENGTH+1];/**< text of the lexeme */
    };

    class Lexer
    {
    public:
        /**
         *  Create a new lexer that reads and lexes input. 
         *  @param agent TODO: Currently required for printing 
         *  purposes; should refactor and pass in a printer object 
         *  of some kind
         *  @param input the string to lex
         */ 
        Lexer(agent* thisAgent, const char* input);
        ~Lexer() {};
        /**
         * Read the input and set the current lexeme.
         */
        void get_lexeme();
        /**
         * Tell the lexer whether to allow identifiers to be read. 
         * @param allow True to allow identifiers to be read; false
         * to set the type of any identifier lexeme to SYM_CONSTANT_LEXEME
         * instead.
         */
        void set_allow_ids(Bool allow);
        Bool get_allow_ids();

        /**
         *  Print an out the current source line and column; useful for
         *  error messages. TODO: it's a no-op for now.
         */
        void print_location_of_most_recent_lexeme ();
        /**
         * Return the current level of parentheses nesting (0 means 
         * no open paren's have been encountered). 
         */
        int current_parentheses_level ();
        /**
         * Eat lexemes until current_parentheses_level matches the input
         * integer (0 means eat until back at the top level).
         */
        void skip_ahead_to_balanced_parentheses (int parentheses_level);

        /**
         * Figure out what kind(s) of symbol a given string could represent. 
         * The result is stored in the input pointer variables.
         * @param s The string to analyze
         * @param length_of_s
         * @param possible_id Could the string be an identifier?
         * @param possible_var Could the string be a variable?
         * @param possible_sc Could the string be a symbolic constant?
         * @param possible_ic Could the string be an integer constant?
         * @param possible_fc Could the string be a float constant?
         * @param rereadable Set to TRUE if the lexer would read the given 
         * string as a symbol with exactly the same name (as opposed to 
         * treating it as a special lexeme like "+", changing upper to lower 
         * case, etc.)
         */
        static void determine_possible_symbol_types_for_string (
          char *s,
          size_t length_of_s,
          Bool *possible_id,
          Bool *possible_var,
          Bool *possible_sc, //sym constant
          Bool *possible_ic, //int constant
          Bool *possible_fc, //float constant
          Bool *rereadable);

        /**
         * The last character read from the input string
         */
        int                 current_char;     // holds current input character
        /**
         * The last lexeme read from the input string (set by get_lexeme()).
         */
        Lexeme  current_lexeme;   // holds current lexeme
    private:
        const char*         production_string;
        //0 means top level, no left paren's seen 
        int                 parentheses_level;
        Bool                allow_ids;
        agent*              thisAgent;

        //length of "-->" and "<=>". If a longer one is added, be
        //sure to update this!
        static const int length_of_longest_special_lexeme = 3;  

        //structures required for lexing
        static Bool initialized;
        static Bool constituent_char[256];//character is a symbol constituent
        static Bool whitespace[256];      //character is whitespace
        static Bool number_starters[256]; //character can initiate a number
        typedef void(soar::Lexer::*lex_func_ptr)();
        static lex_func_ptr lexer_routines[256];
        //initializes all lexing structures once at startup
        static Bool init ();

        void get_next_char ();
        void consume_whitespace_and_comments();
        
        void lex_unknown ();
        void lex_eof ();
        void lex_at ();
        void lex_tilde ();
        void lex_up_arrow ();
        void lex_lbrace ();
        void lex_rbrace ();
        void lex_exclamation_point ();
        void lex_comma ();
        void lex_equal ();
        void lex_ampersand ();
        void lex_lparen ();
        void lex_rparen ();
        void lex_greater ();
        void lex_less ();
        void lex_period ();
        void lex_plus ();
        void lex_minus ();
        void lex_digit ();
        void lex_constituent_string ();
        void lex_vbar ();
        void lex_quote ();

        Bool determine_type_of_constituent_string ();
        void record_position_of_start_of_lexeme();
        void read_constituent_string ();
        void read_rest_of_floating_point_number ();
        void store_and_advance();
        void finish();
    };
}

#endif // LEXER_H
