

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhsfun.cpp
 *
 * =======================================================================
 *                   RHS Function Management
 *
 * The system maintains a list of available RHS functions.  Functions
 * can appear on the RHS of productions either as values (in make actions
 * or as arguments to other function calls) or as stand-alone actions
 * (e.g., "write" and "halt").  When a function is executed, its C code
 * is called with one parameter--a (consed) list of the arguments (symbols).
 * The C function should return either a symbol (if all goes well) or NIL
 * (if an error occurred, or if the function is a stand-alone action).
 *
 * All available RHS functions should be setup at system startup time via
 * calls to add_rhs_function().  It takes as arguments the name of the
 * function (a symbol), a pointer to the corresponding C function, the
 * number of arguments the function expects (-1 if the function can take
 * any number of arguments), and flags indicating whether the function can
 * be a RHS value or a stand-alone action.
 *
 * Lookup_rhs_function() takes a symbol and returns the corresponding
 * rhs_function structure (or NIL if there is no such function).
 *
 * Init_built_in_rhs_functions() should be called at system startup time
 * to setup all the built-in functions.
 * =======================================================================
 */

#include "rhs_functions.h"

#include "agent.h"
#include "decide.h"
#include "ebc.h"

#include "io_link.h"
#include "mem.h"
#include "print.h"
#include "instantiation.h"
#include "output_manager.h"
#include "production.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "xml.h"

#include <map>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>

using namespace soar_TraceNames;

/* The last argument to add_rhs_function determines whether EBC literalizes the arguments of the function, i.e. forces the identity sets
 * of any arguments that are variables to map to the null identity set.  This will cause any variables that map to that identity set to
 * to not be variablized in the final chunk.
 *
 * Currently, ONLY MATH FUNCTIONS cause literalization of their arguments.
 *
 *   - One could argue that make-constant-symbol, trim, capitalize_symbol should also literalize arguments, but it seems unlikely that
 *     their products would affect behavior.  Moreover, an agent that used those functions to print a lot of trace messages could end
 *     up literalizing a lot of variables unnecessarily.
 *   - One could also argue that ifeq should be literalized since it could return an identifier. It looks the original author of ifeq
 *     made it for pretty printing and not a way to get conditional rhs actions, which we probably don't want to support.  So I grouped
 *     this with the other text producing rhs functions that don't cause literalization.
 *
 * Technically, we only need to literalize the arguments of a rhs function if another condition later tests that the value produced meets a
 * constraint.  If they don't have constraints, we could actually compose the functions used into the variablized rhs actions of the final
 * chunk.  If we add that capability to EBC down the line, the semantics of this parameter will change to indicate whether to compose (if no
 * constraints) or literalize (if later constraints on value exist).  If we add that, we should definitely include if-eq, make-constant-symbol,
 * trim, and capitalize_symbol, so that they can be composed into the learned rule rather than be hard-coded (possibly inaccurate) strings. */

void add_rhs_function(agent* thisAgent,
                      Symbol* name,
                      rhs_function_routine f,
                      int num_args_expected,
                      bool can_be_rhs_value,
                      bool can_be_stand_alone_action,
                      void* user_data,
                      bool literalize_arguments)
{
    rhs_function* rf;

    if ((!can_be_rhs_value) && (!can_be_stand_alone_action))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Internal error: attempt to add_rhs_function that can't appear anywhere\n");
        return;
    }

    for (rf = thisAgent->rhs_functions; rf != NIL; rf = rf->next)
    {
        if (rf->name == name)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Internal error: attempt to add_rhs_function that already exists: %y\n", name);
            return;
        }
    }

    rf = static_cast<rhs_function_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(rhs_function), MISCELLANEOUS_MEM_USAGE));

    /* Insertion into the list */
    rf->next = thisAgent->rhs_functions;
    thisAgent->rhs_functions = rf;

    /* Rest of the stuff */
    rf->name = name;
    rf->f = f;
    rf->num_args_expected = num_args_expected;
    rf->can_be_rhs_value = can_be_rhs_value;
    rf->can_be_stand_alone_action = can_be_stand_alone_action;
    rf->user_data = user_data;
    rf->literalize_arguments = can_be_rhs_value ? literalize_arguments : false;
}

rhs_function* lookup_rhs_function(agent* thisAgent, Symbol* name)
{
    rhs_function* rf;

    for (rf = thisAgent->rhs_functions; rf != NIL; rf = rf->next)
    {
        if (rf->name == name)
        {
            return rf;
        }
    }
    return NIL;
}

void remove_rhs_function(agent* thisAgent, Symbol* name)    /* code from Koss 8/00 */
{

    rhs_function* rf = NIL, *prev;

    /* Find registered function by name */
    for (prev = NIL, rf = thisAgent->rhs_functions;
            rf != NIL;
            prev = rf, rf = rf->next)
    {
        if (rf->name == name)
        {
            break;
        }
    }

    /* If not found, there's a problem */
    if (rf == NIL)
    {
        fprintf(stderr, "Internal error: attempt to remove_rhs_function that does not exist.\n");
        thisAgent->outputManager->printa_sf(thisAgent, "Internal error: attempt to remove_rhs_function that does not exist: %y\n", name);
    }

    /* Else, remove it */
    else
    {

        /* Head of list special */
        if (prev == NIL)
        {
            thisAgent->rhs_functions = rf->next;
        }
        else
        {
            prev->next = rf->next;
        }
//        if (rf->cached_print_str) free_memory_block_for_string(thisAgent, rf->cached_print_str);
        thisAgent->memoryManager->free_memory(rf, MISCELLANEOUS_MEM_USAGE);
    }

    // DJP-FREE: The name reference needs to be released now the function is gone
    thisAgent->symbolManager->symbol_remove_ref(&name);
}

/* ====================================================================

               Code for Executing Built-In RHS Functions

====================================================================  */

/* --------------------------------------------------------------------
                                Write

   Takes any number of arguments, and prints each one.
-------------------------------------------------------------------- */

Symbol* write_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    if (thisAgent->outputManager->settings[OM_AGENT_WRITES])
    {
        Symbol* arg;
        char* string;
        growable_string gs = make_blank_growable_string(thisAgent); // for XML generation

        for (; args != NIL; args = args->rest)
        {
            arg = static_cast<symbol_struct*>(args->first);
            /* --- Note use of false here--print the symbol itself, not a rereadable
           version of it --- */
            string = arg->to_string();
            add_to_growable_string(thisAgent, &gs, string); // for XML generation
            thisAgent->outputManager->printa(thisAgent, string);
        }

        xml_object(thisAgent, kTagRHS_write, kRHS_String, text_of_growable_string(gs));

        free_growable_string(thisAgent, gs);
    }

    return NIL;
}

Symbol* trace_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    if (thisAgent->outputManager->settings[OM_AGENT_WRITES])
    {
        Symbol* arg;
        char* string;

        Symbol* lChannelSym = static_cast<Symbol*>(args->first);
        if (!lChannelSym->is_int() || (lChannelSym->ic->value < 1) || (lChannelSym->ic->value > maxAgentTraces))
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%eError: First argument of agent's trace command must be an integer channel number between 1 and %d.  %y is invalid.\n", maxAgentTraces, lChannelSym);
            return NIL;
        }
        args = args->rest;
        if (thisAgent->output_settings->agent_traces_enabled[(lChannelSym->ic->value - 1)])
        {
            growable_string gs = make_blank_growable_string(thisAgent); // for XML generation

            for (; args != NIL; args = args->rest)
            {
                arg = static_cast<symbol_struct*>(args->first);
                /* --- Note use of false here--print the symbol itself, not a rereadable version of it --- */
                string = arg->to_string();
                add_to_growable_string(thisAgent, &gs, string); // for XML generation
                thisAgent->outputManager->printa(thisAgent, string);
            }

            xml_object(thisAgent, kTagRHS_write, kRHS_String, text_of_growable_string(gs));

            free_growable_string(thisAgent, gs);
        }
    }

    return NIL;
}
/* --------------------------------------------------------------------
                                Crlf

   Just returns a str_constant whose print name is a line feed.
-------------------------------------------------------------------- */

Symbol* crlf_rhs_function_code(agent* thisAgent, cons* /*args*/, void* /*user_data*/)
{
    thisAgent->symbolManager->symbol_add_ref(thisAgent->symbolManager->soarSymbols.crlf_symbol);
    return thisAgent->symbolManager->soarSymbols.crlf_symbol;

}

/* --------------------------------------------------------------------
                                Halt

   Just sets a flag indicating that the system has halted.
-------------------------------------------------------------------- */

Symbol* halt_rhs_function_code(agent* thisAgent, cons* /*args*/, void* /*user_data*/)
{
    thisAgent->system_halted = true;
    soar_invoke_callbacks(thisAgent,
                          AFTER_HALT_SOAR_CALLBACK,
                          0);

    return NIL;
}

/* --------------------------------------------------------------------
                         Make-constant-symbol

   Returns a newly generated str_constant.  If no arguments are given,
   the constant will start with "constant".  If one or more arguments
   are given, the constant will start with a string equal to the
   concatenation of those arguments.
-------------------------------------------------------------------- */

Symbol* make_constant_symbol_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    std::stringstream buf;
    char* string;
    cons* c;

    if (!args)
    {
        buf << "constant";
    }
    else
    {
        for (c = args; c != NIL; c = c->rest)
        {
            string = static_cast<symbol_struct*>(c->first)->to_string();
            buf << string;
        }
    }
    if ((!args) && (!thisAgent->symbolManager->find_str_constant(buf.str().c_str())))
    {
        return thisAgent->symbolManager->make_str_constant(buf.str().c_str());
    }
    return thisAgent->symbolManager->generate_new_str_constant(buf.str().c_str(), &thisAgent->mcs_counter);
}


/* --------------------------------------------------------------------
                               Timestamp

   Returns a newly generated str_constant whose name is a representation
   of the current local time.
-------------------------------------------------------------------- */

Symbol* dc_rhs_function_code(agent* thisAgent, cons* /*args*/, void* /*user_data*/)
{
    return thisAgent->symbolManager->make_int_constant(thisAgent->decision_phases_count);
}

Symbol* timestamp_rhs_function_code(agent* thisAgent, cons* /*args*/, void* /*user_data*/)
{
    time_t now;
    struct tm* temp;
#define TIMESTAMP_BUFFER_SIZE 100
    char buf[TIMESTAMP_BUFFER_SIZE];

    now = time(NULL);
#ifdef THINK_C
    temp = localtime((const time_t*)&now);
#else
#ifdef __SC__
    temp = localtime((const time_t*)&now);
#else
#ifdef __ultrix
    temp = localtime((const time_t*)&now);
#else
#ifdef MACINTOSH
    temp = localtime((const time_t*) &now);
#else
    temp = localtime(&now);
#endif
#endif
#endif
#endif
    SNPRINTF(buf, TIMESTAMP_BUFFER_SIZE, "%d/%d/%d-%02d:%02d:%02d",
             temp->tm_mon + 1, temp->tm_mday, temp->tm_year,
             temp->tm_hour, temp->tm_min, temp->tm_sec);
    buf[TIMESTAMP_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    return thisAgent->symbolManager->make_str_constant(buf);
}

/* --------------------------------------------------------------------
                              Accept

   Waits for the user to type a line of input; then returns the first
   symbol from that line.
-------------------------------------------------------------------- */

Symbol* accept_rhs_function_code(agent* thisAgent, cons* /*args*/, void* /*user_data*/)
{
    char buf[2000], *s;
    Symbol* sym;

    while (true)
    {
        s = fgets(buf, 2000, stdin);
        //    s = Soar_Read(thisAgent, buf, 2000); /* kjh(CUSP-B10) */
        if (!s)
        {
            /* s==NIL means immediate eof encountered or read error occurred */
            return NIL;
        }
        s = buf;
        sym = get_next_io_symbol_from_text_input_line(thisAgent, &s);
        if (sym)
        {
            break;
        }
    }
    thisAgent->symbolManager->symbol_add_ref(sym);
    release_io_symbol(thisAgent, sym);  /* because it was obtained using get_io_... */
    return sym;
}


Symbol*
get_lti_id_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* sym, * returnSym;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' function called with no arguments.\n");
        return NIL;
    }

    sym = static_cast<Symbol*>(args->first);
    if (!sym->is_lti())
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: %y is not linked to a semantic identifier.\n", sym);
        return NIL;
    }

    if (args->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' takes exactly 1 argument.\n");
        return NIL;
    }

    returnSym = thisAgent->symbolManager->make_int_constant(sym->id->LTI_ID);
    return returnSym;
}

Symbol*
set_lti_id_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* sym, *ltiIDSym, *returnSym;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' rhs function called with no arguments.\n");
        return NIL;
    }

    sym = static_cast<Symbol*>(args->first);
    if (!sym->is_sti())
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' rhs function cannot accept %y because it is not a Soar identifier\n", sym);
        return NIL;
    }

    if (args->rest)
    {
        if (args->rest->rest)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' rhs function takes exactly 2 arguments.\n");
            return NIL;
        }
        ltiIDSym = static_cast<Symbol*>(args->rest->first);
        if (!ltiIDSym->is_int())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' rhs function cannot accept %y as an LTI ID because it is not an integer\n", ltiIDSym);
            return NIL;
        }

    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: '@' rhs function takes exactly 2 arguments.\n");
        return NIL;
    }

    if (thisAgent->SMem->lti_exists(ltiIDSym->ic->value))
    {
        sym->id->LTI_ID = ltiIDSym->ic->value;
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eWarning: Long-term memory @%u does not exist.  Could not link short-term memory %y.\n",  static_cast<uint64_t>(ltiIDSym->ic->value), sym);
    }
    return NIL;
}

/* ---------------------------------------------------------------------
  Capitalize a Symbol
------------------------------------------------------------------------ */

Symbol*
capitalize_symbol_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    char* symbol_to_capitalize;
    Symbol* sym, * returnSym;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'capitalize-symbol' function called with no arguments.\n");
        return NIL;
    }

    sym = static_cast<Symbol*>(args->first);
    if (sym->symbol_type != STR_CONSTANT_SYMBOL_TYPE)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: non-symbol (%y) passed to capitalize-symbol function.\n", sym);
        return NIL;
    }

    if (args->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'capitalize-symbol' takes exactly 1 argument.\n");
        return NIL;
    }

    symbol_to_capitalize = strdup(sym->to_string());
    *symbol_to_capitalize = static_cast<char>(toupper(*symbol_to_capitalize));
    returnSym = thisAgent->symbolManager->make_str_constant(symbol_to_capitalize);
    free(symbol_to_capitalize);
    return returnSym;
}

/* AGR 520 begin     6-May-94 */
/* ------------------------------------------------------------
   2 general purpose rhs functions by Gary.
------------------------------------------------------------

They are invoked in the following manner, and I use them
to produce nice traces.

   (ifeq <a> <b> abc def)
and
   (strlen <a>)

ifeq -- checks if the first argument is "eq" to the second argument
        if it is then it returns the third argument else the fourth.
        It is useful in similar situations to the "?" notation in C.
        Contrary to earlier belief, all 4 arguments are required.

        examples:
           (sp trace-juggling
            (goal <g> ^state.ball.position <pos>)
           -->
            (write (ifeq <pos> at-top        |    o    | ||) (crlf)
                   (ifeq <pos> left-middle   | o       | ||)
                   (ifeq <pos> right-middle  |       o | ||) (crlf)
                   (ifeq <pos> left-hand     |o        | ||)
                   (ifeq <pos> right-hand    |        o| ||) (crlf)
                                             |V_______V|    (crlf))
            )

            This outputs with a single production one of the following
            pictures depending on the ball's position (providing the ball
            is not dropped of course. Then it outputs empty hands. :-)

                                         o
                         o                              o
           o                                                           o
           V-------V    V-------V    V-------V    V-------V    V-------V
                     or           or           or           or


           for a ball that takes this path.

               o
            o     o
           o       o
           V-------V

           Basically this is useful when you don't want the trace to
           match the internal working memory structure.

strlen <val> - returns the string length of the output string so that
               one can get the output to line up nicely. This is useful
               along with ifeq when the output string varies in length.

           example:

              (strlen |abc|) returns 3

              (write (ifeq (strlen <foo>) 3 | | ||)
                     (ifeq (strlen <foo>) 2 |  | ||)
                     (ifeq (strlen <foo>) 1 |   | ||) <foo>)

                  writes foo padding on the left with enough blanks so that
                  the length of the output is always at least 4 characters.

------------------------------------------------------------ */

Symbol* ifeq_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* arg1, *arg2;
    cons* c;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'ifeq' function called with no arguments\n");
        return NIL;
    }

    /* --- two or more arguments --- */
    arg1 = static_cast<symbol_struct*>(args->first);
    c = args->rest;
    arg2 = static_cast<symbol_struct*>(c->first);
    c = c->rest;

    if (arg1 == arg2)
    {
        thisAgent->symbolManager->symbol_add_ref(static_cast<Symbol*>(c->first));
        return static_cast<symbol_struct*>(c->first);
    }
    else if (c->rest)
    {
        thisAgent->symbolManager->symbol_add_ref(static_cast<Symbol*>(c->rest->first));
        return static_cast<symbol_struct*>(c->rest->first);
    }
    else
    {
        return NIL;
    }
}

Symbol* trim_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    char* symbol_to_trim;
    Symbol* sym, *returnSym;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'trim' function called with no arguments.\n");
        return NIL;
    }

    sym = (Symbol*) args->first;

    if (sym->symbol_type != STR_CONSTANT_SYMBOL_TYPE)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: non-symbol (%y) passed to 'trim' function.\n", sym);
        return NIL;
    }

    if (args->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'trim' takes exactly 1 argument.\n");
        return NIL;
    }

//  symbol_to_trim = sym->to_string();
//  symbol_to_trim = savestring( symbol_to_trim );

    symbol_to_trim = strdup(sym->to_string());

    std::string str(symbol_to_trim);
    size_t start_pos = str.find_first_not_of(" \t\n");
    size_t end_pos = str.find_last_not_of(" \t\n");

    if ((std::string::npos == start_pos) || (std::string::npos == end_pos))
    {
        str = "";
    }
    else
    {
        str = str.substr(start_pos, end_pos - start_pos + 1);
    }

    returnSym = thisAgent->symbolManager->make_str_constant(str.c_str());
    free(symbol_to_trim);
    return returnSym;

}

Symbol* strlen_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* arg;
    char* string;

    arg = static_cast<symbol_struct*>(args->first);

    /* --- Note use of false here--print the symbol itself, not a rereadable
       version of it --- */
    string = arg->to_string();

    return thisAgent->symbolManager->make_int_constant(static_cast<int64_t>(strlen(string)));
}
/* AGR 520     end */

/* --------------------------------------------------------------------
                              dont_learn

Hack for learning.  Allow user to denote states in which learning
shouldn't occur when "learning" is set to "except".
-------------------------------------------------------------------- */

Symbol* dont_learn_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* state;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'dont-learn' function called with no arg.\n");
        return NIL;
    }

    state = static_cast<Symbol*>(args->first);
    if (state->symbol_type != IDENTIFIER_SYMBOL_TYPE)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: non-identifier (%y) passed to dont-learn function.\n", state);
        return NIL;
    }
    else if (! state->id->isa_goal)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: identifier passed to dont-learn is not a state: %y.\n", state);
    }

    if (args->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'dont-learn' takes exactly 1 argument.\n");
        return NIL;
    }

    if (! member_of_list(state, thisAgent->explanationBasedChunker->chunk_free_problem_spaces))
    {
        push(thisAgent, state, thisAgent->explanationBasedChunker->chunk_free_problem_spaces);
        /* thisAgent->outputManager->printa_sf("State  %y  added to chunk_free_list.\n",state); */
    }
    return NIL;

}

/* --------------------------------------------------------------------
                              force_learn

Hack for learning.  Allow user to denote states in which learning
should occur when "learning" is set to "only".
-------------------------------------------------------------------- */

Symbol* force_learn_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* state;

    if (!args)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'force-learn' function called with no arg.\n");
        return NIL;
    }

    state = static_cast<Symbol*>(args->first);
    if (state->symbol_type != IDENTIFIER_SYMBOL_TYPE)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: non-identifier (%y) passed to force-learn function.\n", state);
        return NIL;
    }
    else if (! state->id->isa_goal)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: identifier passed to force-learn is not a state: %y.\n", state);
    }


    if (args->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%eError: 'force-learn' takes exactly 1 argument.\n");
        return NIL;
    }

    if (! member_of_list(state, thisAgent->explanationBasedChunker->chunky_problem_spaces))
    {
        push(thisAgent, state, thisAgent->explanationBasedChunker->chunky_problem_spaces);
        /* thisAgent->outputManager->printa_sf("State  %y  added to chunky_list.\n",state); */
    }
    return NIL;

}

/* ====================================================================
                  RHS Deep copy recursive helper functions
====================================================================  */
void recursive_deep_copy_helper(agent* thisAgent, Symbol* id_to_process, Symbol* parent_id,
                                std::unordered_map<Symbol*, Symbol*>& processedSymbols);

void recursive_wme_copy(agent* thisAgent, Symbol* parent_id, wme* curwme,
                        std::unordered_map<Symbol*, Symbol*>& processedSymbols)
{

    bool made_new_attr_symbol = false;
    bool made_new_value_symbol = false;

    Symbol* new_id = parent_id;
    Symbol* new_attr = curwme->attr;
    Symbol* new_value = curwme->value;

    /* Handling the case where the attribute is an id symbol */
    if (curwme->attr->is_sti())
    {
        /* Have I already made a new identifier for this identifier */
        std::unordered_map<Symbol*, Symbol*>::iterator it = processedSymbols.find(curwme->attr);
        if (it != processedSymbols.end())
        {
            /* Retrieve the previously created id symbol */
            new_attr = it->second;
        }
        else
        {
            /* Make a new id symbol */
            new_attr = thisAgent->symbolManager->make_new_identifier(curwme->attr->id->name_letter, 0, NIL);
            made_new_attr_symbol = true;
        }

        recursive_deep_copy_helper(thisAgent, curwme->attr, new_attr, processedSymbols);
    }

    /* Handling the case where the value is an id symbol */
    if (curwme->value->symbol_type == 1)
    {
        /* Have I already made a new identifier for this identifier */
        std::unordered_map<Symbol*, Symbol*>::iterator it = processedSymbols.find(curwme->value);
        if (it != processedSymbols.end())
        {
            /* Retrieve the previously created id symbol */
            new_value = it->second;
        }
        else
        {
            /* Make a new id symbol */
            new_value = thisAgent->symbolManager->make_new_identifier(curwme->value->id->name_letter, 0, NIL);
            made_new_value_symbol = true;
        }

        recursive_deep_copy_helper(thisAgent, curwme->value, new_value, processedSymbols);
    }

    /* Making the new wme (Note just reusing the wme data structure, these wme's actually get converted into preferences later).*/
    wme* oldGlobalWme = thisAgent->WM->glbDeepCopyWMEs;

    /* TODO: We need a serious reference counting audit of the kernel But I think
       this mirrors what happens in the instantiate rhs value and execute action
       functions. */
    thisAgent->symbolManager->symbol_add_ref(new_id);
    if (!made_new_attr_symbol)
    {
        thisAgent->symbolManager->symbol_add_ref(new_attr);
    }
    if (!made_new_value_symbol)
    {
        thisAgent->symbolManager->symbol_add_ref(new_value);
    }

    thisAgent->WM->glbDeepCopyWMEs = make_wme(thisAgent, new_id, new_attr, new_value, true);
    thisAgent->WM->glbDeepCopyWMEs->deep_copied_wme = curwme;
    thisAgent->WM->glbDeepCopyWMEs->next = oldGlobalWme;

}

void recursive_deep_copy_helper(agent* thisAgent, Symbol* id_to_process, Symbol* parent_id,
                                std::unordered_map<Symbol*, Symbol*>& processedSymbols)
{
    /* If this symbol has already been processed then ignore it and return */
    if (processedSymbols.find(id_to_process) != processedSymbols.end()) return;

    processedSymbols.insert(std::pair<Symbol*, Symbol*>(id_to_process, parent_id));

    /* Iterating over the normal slot wmes */
    for (slot* curslot = id_to_process->id->slots; curslot != 0; curslot = curslot->next)
    {
        /* Iterating over the wmes in this slot */
        for (wme* curwme = curslot->wmes; curwme != 0; curwme = curwme->next)
        {
            recursive_wme_copy(thisAgent, parent_id, curwme, processedSymbols);
        }
    }

    /* Iterating over input wmes */
    for (wme* curwme = id_to_process->id->input_wmes; curwme != 0; curwme = curwme->next)
    {
        recursive_wme_copy(thisAgent, parent_id, curwme, processedSymbols);
    }
}

/* ====================================================================
                  RHS Deep copy function
====================================================================  */
Symbol* deep_copy_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{

    /* Getting the argument symbol */
    Symbol* baseid = static_cast<Symbol*>(args->first);
    if (!baseid->is_sti())
    {
        return thisAgent->symbolManager->make_str_constant("*symbol not id*");
    }

    /* Make the new root identifier symbol.  We'll set the level in create_instantiation. */
    Symbol* retval = thisAgent->symbolManager->make_new_identifier('D', 0, NIL);

    /* Now processing the wme's associated with the passed in symbol */
    std::unordered_map<Symbol*, Symbol*> processedSymbols;
    recursive_deep_copy_helper(thisAgent, baseid,  retval, processedSymbols);

    return retval;
}

/* --------------------------------------------------------------------
                                Count

   Takes arbitrary arguments and adds one to the associated
   dynamic counters.
-------------------------------------------------------------------- */

Symbol* count_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    Symbol* arg;
    char* string;

    for (; args != NIL; args = args->rest)
    {
        arg = static_cast<symbol_struct*>(args->first);
        /* --- Note use of false here--print the symbol itself, not a rereadable version of it --- */
        string = arg->to_string();
        (*thisAgent->dyn_counters)[ string ]++;
    }

    return NIL;
}

/* --------------------------------------------------------------------
                                Wait

   Puts the curret thread to sleep for the specified number of
   milliseconds
-------------------------------------------------------------------- */

Symbol* wait_rhs_function_code(agent* thisAgent, cons* args, void* /*user_data*/)
{
    int ms = 1; // if there is no valid argument, then just default to 1
    if(args != NIL)
    {
        Symbol* arg;
        arg = static_cast<symbol_struct*>(args->first);
        if (arg->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        {
            ms = arg->ic->value;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));

    return NIL;
}

/* ====================================================================

                  Initialize the Built-In RHS Functions

====================================================================  */

void init_built_in_rhs_functions(agent* thisAgent)
{
    /* Note that there are four RHS functions that are defined in sml_RhsFunction.cpp instead of here in the kernel:  concat, exec, cmd and interrupt */

    /* Stand-alone RHS functions */
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("count"), count_rhs_function_code, -1, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("dont-learn"), dont_learn_rhs_function_code,  1, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("force-learn"), force_learn_rhs_function_code, 1, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("halt"), halt_rhs_function_code, 0, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("link-stm-to-ltm"), set_lti_id_rhs_function_code, 2, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("wait"), wait_rhs_function_code, 1, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("write"), write_rhs_function_code, -1, false, true, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("trace"), trace_rhs_function_code, -1, false, true, 0, false);

    /* RHS functions that return a simple value */
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("@"), get_lti_id_rhs_function_code, 1, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("capitalize-symbol"), capitalize_symbol_rhs_function_code, 1, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("crlf"), crlf_rhs_function_code, 0, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("dc"),  dc_rhs_function_code,  0, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("make-constant-symbol"), make_constant_symbol_rhs_function_code,  -1, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("strlen"), strlen_rhs_function_code, 1, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("timestamp"),  timestamp_rhs_function_code, 0, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("trim"),  trim_rhs_function_code, 1, true, false, 0, false);

    /* RHS functions that are more elaborate */
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("accept"), accept_rhs_function_code, 0, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("deep-copy"), deep_copy_rhs_function_code, 1, true, false, 0, false);
    add_rhs_function(thisAgent, thisAgent->symbolManager->make_str_constant("ifeq"), ifeq_rhs_function_code, 4, true, false, 0, false);

    /* EBC Manager caches these rhs functions since it may re-use them many times */
    thisAgent->explanationBasedChunker->lti_link_function = lookup_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("link-stm-to-ltm"));

    init_built_in_rhs_math_functions(thisAgent);
}

void remove_built_in_rhs_functions(agent* thisAgent)
{
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("count"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("dont-learn"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("force-learn"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("halt"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("link-stm-to-ltm"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("wait"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("write"));

    remove_rhs_function(thisAgent,thisAgent->symbolManager->soarSymbols.at_symbol);
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("capitalize-symbol"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("crlf"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("dc"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("make-constant-symbol"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("strlen"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("timestamp"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("trim"));

    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("accept"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("deep-copy"));
    remove_rhs_function(thisAgent, thisAgent->symbolManager->find_str_constant("ifeq"));

    remove_built_in_rhs_math_functions(thisAgent);

}
