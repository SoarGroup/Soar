/////////////////////////////////////////////////////////////////
// preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "agent.h"
#include "decide.h"
#include "exploration.h"
#include "lexer.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "trace.h"
#include "working_memory.h"

using namespace cli;
using namespace sml;

/*
*    This procedure parses a string to determine if it is a
*      lexeme for an existing attribute.
*
* Side effects:
*    None.
*
===============================
*/

bool read_attribute_from_string(agent* thisAgent, Symbol* id, char* the_lexeme, Symbol** attr)
{
    Symbol* attr_tmp;
    slot* s;

    /* skip optional '^' if present.  KJC added to Ken's code */
    if (*the_lexeme == '^')
    {
        the_lexeme++;
    }

    soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, the_lexeme);

    switch (lexeme.type)
    {
        case STR_CONSTANT_LEXEME:
            attr_tmp = thisAgent->symbolManager->find_str_constant(lexeme.string());
            break;
        case INT_CONSTANT_LEXEME:
            attr_tmp = thisAgent->symbolManager->find_int_constant(lexeme.int_val);
            break;
        case FLOAT_CONSTANT_LEXEME:
            attr_tmp = thisAgent->symbolManager->find_float_constant(lexeme.float_val);
            break;
        case IDENTIFIER_LEXEME:
            attr_tmp = thisAgent->symbolManager->find_identifier(lexeme.id_letter,
                                       lexeme.id_number);
            break;
        case VARIABLE_LEXEME:
            attr_tmp = read_identifier_or_context_variable(thisAgent, &lexeme);
            if (!attr_tmp)
            {
                return false;
            }
            break;
        default:
            return false;
    }
    s = find_slot(id, attr_tmp);
    if (s)
    {
        *attr = attr_tmp;
        return true;
    }
    else
    {
        *attr = attr_tmp;
    }
    return false;
}

/*
*
*    This procedure prints a preference and the production
*      which is the source of the preference.
*
* Results:
*    Tcl status code.
*
* Side effects:
*    Prints the preference and its source production.
*
* NOTE:  The called of this routine should be stepping thru slots only,
*        (not stepping thru WMEs) and therefore input wmes and arch-wmes
*        are already excluded and we can print :I when o_support is false.
*
===============================
*/
void print_preference_and_source(agent* thisAgent, preference* pref,
                                 bool print_source,
                                 wme_trace_type wtt,
                                 double* selection_probability = 0)
{
    thisAgent->outputManager->printa(thisAgent, "  ");
    if (pref->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
    {
        print_object_trace(thisAgent, pref->value);
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent, "(%y ^%y %y) ", pref->id, pref->attr, pref->value);
    }
    if (pref->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  " %c", preference_to_char(pref->type));
    }
    if (preference_is_binary(pref->type))
    {
        print_object_trace(thisAgent, pref->referent);
    }
    char dest[output_string_size]; /* from agent.h */
    if (selection_probability)
    {
        SNPRINTF(dest, sizeof(dest), "%#.16g", pref->numeric_value);
        dest[sizeof(dest) - 1] = '\0'; /* ensure null termination */
        {
            /* --- strip off trailing zeros --- */
            char* start_of_exponent;
            char* end_of_mantissa;
            start_of_exponent = dest;
            while ((*start_of_exponent != 0) && (*start_of_exponent != 'e'))
            {
                start_of_exponent++;
            }
            end_of_mantissa = start_of_exponent - 1;
            while (*end_of_mantissa == '0')
            {
                end_of_mantissa--;
            }
            end_of_mantissa++;
            while (*start_of_exponent)
            {
                *end_of_mantissa++ = *start_of_exponent++;
            }
            *end_of_mantissa = 0;
        }
    } else {
        dest[0] = '0';
        dest[1] = 0;
    }
    if (selection_probability)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  " =%s", dest);
    }
    if (pref->o_supported)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  " :O ");
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent,  " :I ");
    }
    if (selection_probability)
    {
        SNPRINTF(dest, sizeof(dest), "%#.2f", (*selection_probability) * 100.0);
        dest[sizeof(dest) - 1] = '\0'; /* ensure null termination */
    } else {
        dest[0] = '0';
        dest[1] = 0;
    }
    if (selection_probability)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "(%s%)", dest);
    }
    thisAgent->outputManager->printa_sf(thisAgent,  "\n");
    if (print_source)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "    From ");
        print_instantiation_with_wmes(thisAgent, pref->inst, wtt, -1);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");
    }
}

int soar_ecPrintPreferences(agent* thisAgent, char* szId, char* szAttr, bool object, bool print_prod, wme_trace_type wtt)
{

    Symbol* id, *attr = 0;
    slot* s = 0 ;
    preference* p;
    wme* w;
    int i;

    if (!read_id_or_context_var_from_string(thisAgent, szId, &id))
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Could not find the id '%s'\n", szId);
        return -1;
    }

    /// This is badbad style using the literal string, but it all has to
    /// change soon, so I'll cheat for now.  If we have an ID that isn't a
    /// state (goal), then don't use the default ^operator.  Instead, search
    /// for wmes with that ID as a value.  See below
    if (!id->id->isa_goal && !strcmp(szAttr, "operator"))
    {
        attr = NIL;
    }
    else
    {
        if (szAttr && !object)    // default ^attr is ^operator, unless specified --object on cmdline
        {
            if (!read_attribute_from_string(thisAgent, id, szAttr, &attr))
            {
                // NOT tested:  but here goes...
                // This is code to determine whether ^attr arg is misspelled
                // or an arch-wme.  Had to modify read_attribute_from_string() to
                // always set the attr:  symbol exists but no slot, or attr = NIL.
                if (attr)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "  This is probably an io- or arch-wme and does not have preferences\n");
                    return 0;
                }
                thisAgent->outputManager->printa_sf(thisAgent,  "Could not find prefs for the id,attribute pair: %s %s\n", szId, szAttr);
                return -2;
            }
            s = find_slot(id, attr);
            if (!s && !object)
            {
                // Should we check for input wmes and arch-wmes ?? ...covered above...
                thisAgent->outputManager->printa_sf(thisAgent,  "Could not find preferences for %s ^%s.", szId, szAttr);
                return -3;
            }
        }
    }

    // We have one of three cases now, as of v8.6.3
    //     1.  --object is specified:  return prefs for all wmes comprising object ID
    //                    (--depth not yet implemented...)
    //     2.  non-state ID is given:  return prefs for wmes whose <val> is ID
    //     3.  default (no args):  return prefs of slot (id, attr)  <s> ^operator

    if (object)
    {
        // step thru dll of slots for ID, printing prefs for each one
        for (s = id->id->slots; s != NIL; s = s->next)
        {
            if (s->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Preferences for %y ^%y:", s->id, s->attr);
            }
            else
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Support for %y ^%y:\n", s->id, s->attr);
            }
            for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
            {
                if (s->preferences[i])
                {
                    if (s->isa_context_slot)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent,  "\n%ss:\n", preference_name(i));
                    }
                    for (p = s->preferences[i]; p; p = p->next)
                    {
                        print_preference_and_source(thisAgent, p, print_prod, wtt);
                    }
                }
            }
        }
        if (id->id->impasse_wmes)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Arch-created wmes for %y :\n", id);
        }
        for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        {
            print_wme(thisAgent, w);
        }
        if (id->id->input_wmes)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Input (IO) wmes for %y :\n", id);
        }
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            print_wme(thisAgent, w);
        }

        return 0;
    }
    else if (!id->id->isa_goal && !attr)
    {
        // find wme(s?) whose value is <ID> and print prefs if they exist
        // ??? should write print_prefs_for_id(thisAgent, id, print_prod, wtt);
        // return;
        for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
        {
            if (w->value == id)
            {
                if (w->value == thisAgent->symbolManager->soarSymbols.operator_symbol)
                {
                    thisAgent->outputManager->printa_sf(thisAgent,  "Preferences for (%u: ", w->timetag);
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent,  "Support for (%u: ", w->timetag);
                }
                thisAgent->outputManager->printa_sf(thisAgent, "%y ^%y %y)\n", w->id, w->attr, w->value);
                if (w->preference)
                {
                    s = find_slot(w->id, w->attr);
                    if (!s)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent,  "    This is an arch-wme and has no prefs.\n");
                    }
                    else
                    {
                        for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
                        {
                            if (s->preferences[i])
                            {
                                // print(thisAgent, "\n%ss:\n", Soar_Instance::Get_Soar_Instance().preference_name(i));
                                for (p = s->preferences[i]; p; p = p->next)
                                {
                                    if (p->value == id)
                                    {
                                        if (i == ACCEPTABLE_PREFERENCE_TYPE)
                                        {
                                            double prob = exploration_probability_according_to_policy(thisAgent, s, s->preferences[i], p);
                                            print_preference_and_source(thisAgent, p, print_prod, wtt, &prob);
                                        }
                                        else
                                        {
                                            print_preference_and_source(thisAgent, p, print_prod, wtt);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //print it
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent,  "    This is an input-wme and has no prefs.\n");
                }
            }
        }
        return 0;
    }

    //print prefs for specified slot
    thisAgent->outputManager->printa_sf(thisAgent, "\nPreferences for %y ^%y:\n", id, attr);

    for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
    {
        if (s->preferences[i])
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "\n%ss:\n", preference_name(i));
            for (p = s->preferences[i]; p; p = p->next)
            {
                print_preference_and_source(thisAgent, p, print_prod, wtt);
            }
        }
    }

    if (id->id->isa_goal && !strcmp(szAttr, "operator"))
    {
        // voigtjr march 2010
        // print selection probabilities re: issue 18
        // run preference semantics "read only" via _consistency_check
        // returns a list of candidates without deciding which one in the event of indifference
        preference* cand = 0;
        byte impasse_type = run_preference_semantics(thisAgent, s, &cand, true);

        // if the impasse isn't NONE_IMPASSE_TYPE, there's an impasse and we don't want to print anything
        // if we have no candidates, we don't want to print anything
        if ((impasse_type == NONE_IMPASSE_TYPE) && cand)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "\nselection probabilities:\n");

            for (p = cand; p; p = p->next_candidate)
            {
                double prob = exploration_probability_according_to_policy(thisAgent, s, cand, p);
                print_preference_and_source(thisAgent, p, print_prod, wtt, &prob);
            }
        }
    }

    return 0;
}

bool CommandLineInterface::DoPreferences(const ePreferencesDetail detail, bool object, const std::string* pId, const std::string* pAttribute)
{
    static const int PREFERENCES_BUFFER_SIZE = 128;

    char id[PREFERENCES_BUFFER_SIZE];
    char attr[PREFERENCES_BUFFER_SIZE];

    const char* idString = pId ? pId->c_str() : 0;
    const char* attrString = pAttribute ? pAttribute->c_str() : 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    // Establish defaults
    thisAgent->bottom_goal->to_string(true, false, id, PREFERENCES_BUFFER_SIZE);
    thisAgent->symbolManager->soarSymbols.operator_symbol->to_string(true, false, attr, PREFERENCES_BUFFER_SIZE);

    // Override defaults
    if (idString)
    {
        strncpy(id, idString, PREFERENCES_BUFFER_SIZE);

        // Notice: attrString arg ignored if no idString passed
        if (attrString)
        {
            strncpy(attr, attrString, PREFERENCES_BUFFER_SIZE);
        }
    }

    bool print_productions = false;
    wme_trace_type wtt = NONE_WME_TRACE;

    switch (detail)
    {
        case PREFERENCES_ONLY:
            print_productions = false;
            wtt = NONE_WME_TRACE;
            break;
        case PREFERENCES_NAMES:
            print_productions = true;
            wtt = NONE_WME_TRACE;
            break;
        case PREFERENCES_TIMETAGS:
            print_productions = true;
            wtt = TIMETAG_WME_TRACE;
            break;
        default:
        // falls through
        case PREFERENCES_WMES:
            print_productions = true;
            wtt = FULL_WME_TRACE;
            break;
    }

    if (soar_ecPrintPreferences(thisAgent, id, attr, object, print_productions, wtt))
    {
        return SetError("An Error occured trying to print the prefs.");
    }
    return true;
}
