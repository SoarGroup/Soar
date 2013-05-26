/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
					   debug.cpp

   @brief debug.cpp provides some utility functions for inspecting and
   	   	  manipulating the data structures of the Soar kernel at run
   	   	  time.

------------------------------------------------------------------ */

#include "debug.h"
#include "debug_defines.h"
#include "agent.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "soar_instance.h"
#include "lexer.h"

debug_param_container::debug_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
    epmem_commands = new soar_module::boolean_param("epmem", off, new soar_module::f_predicate<boolean>() );
    smem_commands = new soar_module::boolean_param("smem", off, new soar_module::f_predicate<boolean>() );
    sql_commands = new soar_module::boolean_param("sql", off, new soar_module::f_predicate<boolean>() );
    add(epmem_commands);
    add(smem_commands);
    add(sql_commands);
}
#ifdef SOAR_DEBUG_UTILITIES

#include "sqlite3.h"

#define DEBUG_BUFFER_SIZE 5000

/* -- Just a simple function that can be called from the debug command.  Something to put random code for testing/debugging -- */
void debug_test(int type)
{
agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if(!debug_agent) return;
  switch (type)
  {
    case 1:
//      print_internal_symbols(debug_agent);
      break;
    case 2:
      break;
    case 3:
    {
//      Symbol *newSym  = find_identifier(debug_agent, 'S', 1);
      break;
}
    case 4:
      break;

    case 5:
      break;

    case 6:
      break;

  }
}



void print_current_lexeme (agent* thisAgent)
{
  std::string lex_type_string;

  switch (thisAgent->lexeme.type) {
    case EOF_LEXEME:
      lex_type_string = "EOF_LEXEME";
      break;
    case IDENTIFIER_LEXEME:
      lex_type_string = "IDENTIFIER_LEXEME";
      break;
    case VARIABLE_LEXEME:
      lex_type_string = "VARIABLE_LEXEME";
      break;
    case SYM_CONSTANT_LEXEME:
      lex_type_string = "SYM_CONSTANT_LEXEME";
      break;
    case INT_CONSTANT_LEXEME:
      lex_type_string = "INT_CONSTANT_LEXEME";
      break;
    case FLOAT_CONSTANT_LEXEME:
      lex_type_string = "FLOAT_CONSTANT_LEXEME";
      break;
    case L_PAREN_LEXEME:
      lex_type_string = "L_PAREN_LEXEME";
      break;
    case R_PAREN_LEXEME:
      lex_type_string = "R_PAREN_LEXEME";
      break;
    case L_BRACE_LEXEME:
      lex_type_string = "L_BRACE_LEXEME";
      break;
    case R_BRACE_LEXEME:
      lex_type_string = "R_BRACE_LEXEME";
      break;
    case PLUS_LEXEME:
      lex_type_string = "PLUS_LEXEME";
      break;
    case MINUS_LEXEME:
      lex_type_string = "MINUS_LEXEME";
      break;
    case RIGHT_ARROW_LEXEME:
      lex_type_string = "RIGHT_ARROW_LEXEME";
      break;
    case GREATER_LEXEME:
      lex_type_string = "GREATER_LEXEME";
      break;
    case LESS_LEXEME:
      lex_type_string = "LESS_LEXEME";
      break;
    case EQUAL_LEXEME:
      lex_type_string = "EQUAL_LEXEME";
      break;
    case LESS_EQUAL_LEXEME:
      lex_type_string = "LESS_EQUAL_LEXEME";
      break;
    case GREATER_EQUAL_LEXEME:
      lex_type_string = "GREATER_EQUAL_LEXEME";
      break;
    case NOT_EQUAL_LEXEME:
      lex_type_string = "NOT_EQUAL_LEXEME";
      break;
    case LESS_EQUAL_GREATER_LEXEME:
      lex_type_string = "LESS_EQUAL_GREATER_LEXEME";
      break;
    case LESS_LESS_LEXEME:
      lex_type_string = "LESS_LESS_LEXEME";
      break;
    case GREATER_GREATER_LEXEME:
      lex_type_string = "GREATER_GREATER_LEXEME";
      break;
    case AMPERSAND_LEXEME:
      lex_type_string = "AMPERSAND_LEXEME";
      break;
    case AT_LEXEME:
      lex_type_string = "AT_LEXEME";
      break;
    case TILDE_LEXEME:
      lex_type_string = "TILDE_LEXEME";
      break;
    case UP_ARROW_LEXEME:
      lex_type_string = "UP_ARROW_LEXEME";
      break;
    case EXCLAMATION_POINT_LEXEME:
      lex_type_string = "EXCLAMATION_POINT_LEXEME";
      break;
    case COMMA_LEXEME:
      lex_type_string = "COMMA_LEXEME";
      break;
    case PERIOD_LEXEME:
      lex_type_string = "PERIOD_LEXEME";
      break;
    case QUOTED_STRING_LEXEME:
      lex_type_string = "QUOTED_STRING_LEXEME";
      break;
    case DOLLAR_STRING_LEXEME:
      lex_type_string = "DOLLAR_STRING_LEXEME";
      break;
    case NULL_LEXEME:
      lex_type_string = "NULL_LEXEME";
      break;
    default:
      break;
  }
  print(thisAgent, "%s: \"%s\"\n", lex_type_string.c_str(), thisAgent->lexeme.string);
}

void print_instantiation (instantiation *inst)
{
  condition *cond;
  preference *pref;
  char pref_type;

  if (inst->prod) {
    print_with_symbols  (debug_agent, "sp { %y\n", inst->prod->name);
  } else {
    print (debug_agent, "sp { [dummy production]\n");
  }

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
  {
    if (cond->type==POSITIVE_CONDITION) {
//      if (cond->bt.level > TOP_GOAL_LEVEL) {
//        print (debug_agent, " ");
//        print_wme (debug_agent, cond->bt.wme_);
        print(debug_agent, "(");
        print_test_brief(debug_agent, cond->data.tests.id_test);
        print(debug_agent, " ^");
        print_test_brief(debug_agent, cond->data.tests.attr_test);
        print(debug_agent, " ");
        print_test_brief(debug_agent, cond->data.tests.value_test);
        print(debug_agent, ")\n");
//      } else {
//        // Wmes that matched the LHS of a retraction may already be free'd; just print tt.
//        print (debug_agent, "timetag %lu", cond->bt.wme_->timetag);
//      }
    }
  }
  print (debug_agent, "--->\n");
  for (pref=inst->preferences_generated; pref!=NIL; pref=pref->next)
  {
    pref_type = preference_to_string (debug_agent, pref->type);
    print(debug_agent, "(");
    print_symbol_with_original(debug_agent, pref->id);
    print(debug_agent, " ^");
    print_symbol_with_original(debug_agent, pref->attr);
    print(debug_agent, " ");
    print_symbol_with_original(debug_agent, pref->value);

    print (debug_agent, " %c", pref_type);
    if (preference_is_binary(pref->type)) {
      print (debug_agent, " ");
      print_symbol_with_original(debug_agent, pref->referent);
    }
    if (pref->o_supported) print_string (debug_agent, " :O ");
    print_string (debug_agent, ")\n");
  }
  print_string (debug_agent, "}\n");
}

#endif

