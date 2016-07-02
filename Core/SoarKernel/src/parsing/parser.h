/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ======================================================================
                              parser.h

                     The Production (SP) Parser

   Init_parser() should be called at startup time.  Parse_production()
   reads an SP (starting from the production name), builds a production,
   adds it to the rete, and returns a pointer to the new production
   (or NIL if any error occurred).  Parse_lhs() reads just an LHS,
   and returns a condition list (or NIL if an error occurred).
   Parse_rhs() reads an RHS, setting "dest_rhs" to point to the resulting
   action list; it returns true if successful, false if any error occurred.
====================================================================== */

#ifndef PARSER_H
#define PARSER_H

#include "kernel.h"
#include "stl_typedefs.h"

#include "../debug_code/dprint.h"

void        init_parser(void);
production* parse_production(agent* thisAgent, const char* prod_string, unsigned char* rete_addition_result);
condition*  parse_lhs(agent* thisAgent, soar::Lexer* lexer);
bool        parse_rhs(agent* thisAgent, soar::Lexer* lexer, action **dest_rhs);
Symbol*     make_symbol_for_lexeme (agent* thisAgent, soar::Lexeme* lexeme, bool allow_lti);

bool        parse_lti(agent* thisAgent, soar::Lexer* lexer);

class LTI_Promotion_Set
{
    public:
        LTI_Promotion_Set()                                     { LTIs_Lexed = new symbol_set(); };
        ~LTI_Promotion_Set()                                    { delete LTIs_Lexed; };

        void         clear()                                    { dprint(DT_PARSER_PROMOTE, "Clearing LTIs found in sourced production.\n");
                                                                  LTIs_Lexed->clear(); }
        void         add_lexed_LTI(Symbol* pSym)                { dprint(DT_PARSER_PROMOTE, "Adding LTI found in sourced production %y.\n", pSym);
                                                                  LTIs_Lexed->insert(pSym); }
        void         promote_LTIs_sourced(agent* thisAgent);

    private:
        symbol_set*  LTIs_Lexed;
};

#endif
