/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ======================================================================
                             lexer.h

  The lexer reads strings and returns a stream of lexemes.  Get_lexeme() is
  the main routine; it looks for the next lexeme in the input, and stores
  it in the member variable "lexeme".

====================================================================== */

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include "stdint.h"
#include "agent.h"

/**
 * Types of tokens read by the lexer
 */
enum lexer_token_type {
  EOF_LEXEME,                        /**< end-of-file */
  IDENTIFIER_LEXEME,                 /**< identifier */
  VARIABLE_LEXEME,                   /**< variable */
  STR_CONSTANT_LEXEME,               /**< string constant */
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
    /**
     * A class representing a single lexeme.
     */
    class Lexeme {
        friend class Lexer;
    public:
        Lexeme() :
            type(NULL_LEXEME),
            int_val(0),
            float_val(0.0),
            id_letter('A'),
            id_number(0){}
        ~Lexeme(){}
        enum lexer_token_type type;         /**< what kind of lexeme it is */
        int64_t int_val;                    /**< for INT_CONSTANT_LEXEME's */
        double float_val;                   /**< for FLOAT_CONSTANT_LEXEME's */
        char id_letter;                     /**< for IDENTIFIER_LEXEME's */
        uint64_t id_number;                 /**< for IDENTIFIER_LEXEME's */
        /**
         * @return the text of the lexeme
         */
        const char* string(){return lex_string.c_str();}
        /**
         * @return the length of the lexeme string
         */
        size_t length() { return lex_string.length(); }
    private:
        /** text of the lexeme */
        std::string lex_string;
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
         * Return a single lexeme created from the input string;
         *
         * @param input string to analyze for lexeme value/type
         */
        static Lexeme get_lexeme_from_string (agent* thisAgent, const char* input);

        /**
         * Tell the lexer whether to allow identifiers to be read.
         * @param allow True to allow identifiers to be read; false
         * to set the type of any identifier lexeme to SYM_CONSTANT_LEXEME
         * instead.
         */
        void set_allow_ids(bool allow);
        bool get_allow_ids();
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
          const char *s,
          size_t length_of_s,
          bool *possible_id,
          bool *possible_var,
          bool *possible_sc,
          bool *possible_ic,
          bool *possible_fc,
          bool *rereadable);

        /**
         * The last character read from the input string.
         */
        int current_char;
        /**
         * The last lexeme read from the input string (set by get_lexeme()).
         */
        Lexeme current_lexeme;
    private:
        /**
         * The second-to-last character read from the input string.
         */
        int                 prev_char;
        const char*         production_string;
        //0 means top level, no left parens seen
        int                 parentheses_level;
        bool                allow_ids;
        agent*              thisAgent;

        //length of "-->" and "<=>". If a longer one is added, be
        //sure to update this!
        static const int length_of_longest_special_lexeme = 3;

        //structures required for lexing
        static bool initialized;
        static bool constituent_char[256];//character is a symbol constituent
        static bool whitespace[256];      //character is whitespace
        static bool number_starters[256]; //character can initiate a number
        typedef void(soar::Lexer::*lex_func_ptr)();
        static lex_func_ptr lexer_routines[256];
        //initializes all lexing structures once at startup
        static bool init ();

        /**
         * Get the next character from the current input file
         * and put it into the member variable current_char.
         * Set current_char to EOF if the input string is null
         * or '\0' is found. The old value of current_char is
         * stored in prev_char.
         */
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

        bool determine_type_of_constituent_string ();
        void record_position_of_start_of_lexeme();
        /**
         * Appends the current character to the current lexeme
         * and then retrieves the next character.
         */
        void store_and_advance();
        /**
         * Calls store_and_advance until a non-constituent character
         * is found (constituent characters are alphanumerics and
         * $%&*+-/:<=>?_@).
         */
        void read_constituent_string ();
        /**
         * This is called when the current character is "." and we believe
         * the current lexeme will be a floating point number. Calls
         * store_and_advance for every character left in the number (
         * including e, E, + and - for scientific notation).
         *
         * @return true if a floating point number was found, false otherwise
         *
         * TODO: perhaps this return value can eliminate some calls to determine_type_of_constituent_string
         */
        bool read_rest_of_floating_point_number ();
    };
}

#endif // LEXER_H
