/////////////////////////////////////////////////////////////////
// print command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "gsysparam.h"
#include "xml.h"
#include "print.h"
#include "trace.h"
#include "wmem.h"
#include "rhsfun.h"
#include "utilities.h"

using namespace cli;
using namespace sml;

void print_stack_trace(agent* thisAgent, bool print_states, bool print_operators)
{
    // We don't want to keep printing forever (in case we're in a state no change cascade).
    const int maxStates = 500;
    int stateCount = 0 ;

    for (Symbol* g = thisAgent->top_goal; g != NIL; g = g->id.lower_goal) 
    {
        stateCount++ ;

        if (stateCount > maxStates)
            continue ;

        if (print_states)
        {
            print_stack_trace (thisAgent, g, g, FOR_STATES_TF, false);
            print (thisAgent, "\n");
        }
        if (print_operators && g->id.operator_slot->wmes) 
        {
            print_stack_trace (thisAgent, g->id.operator_slot->wmes->value, g, FOR_OPERATORS_TF, false);
            print (thisAgent, "\n");
        }
    }

    if (stateCount > maxStates)
        print (thisAgent, "...Stack goes on for another %d states\n", stateCount - maxStates);
}

void do_print_for_production (agent* thisAgent, production *prod, bool intern, bool print_filename, bool full_prod) 
{
    if (print_filename) 
    {
        if (full_prod)
            print_string(thisAgent, "# source file: ");

        if (prod->filename) 
            print_string(thisAgent, prod->filename);
        else 
            print_string(thisAgent, "_unknown_");

        if (full_prod)
            print(thisAgent, "\n");
        else
            print_string(thisAgent, ": ");
    }

    if (full_prod) 
        print_production(thisAgent, prod, intern);
    else
    {
        print_with_symbols(thisAgent, "%y ",prod->name);

        if ( prod->rl_rule )
        {
            // Do extra logging if this agent is in delta bar delta mode.
            if (thisAgent->rl_params->decay_mode->get_value() == rl_param_container::delta_bar_delta_decay) {
                print_with_symbols( thisAgent, " %y", make_float_constant( thisAgent, prod->rl_delta_bar_delta_beta ) );
                print_with_symbols( thisAgent, " %y", make_float_constant( thisAgent, prod->rl_delta_bar_delta_h ) );
            }
            print_with_symbols( thisAgent, " %y", make_float_constant( thisAgent, prod->rl_update_count ) );
            print_with_symbols( thisAgent, " %y", rhs_value_to_symbol( prod->action_list->referent ) );
        }
    }
    print(thisAgent, "\n");
}

void print_productions_of_type(agent* thisAgent, bool intern, bool print_filename, bool full_prod, unsigned int productionType)
{
    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
        do_print_for_production(thisAgent, prod, intern, print_filename, full_prod);
}

void print_rl_rules(agent* thisAgent, bool intern, bool print_filename, bool full_prod)
{
    assert (thisAgent);

    for ( production *prod = thisAgent->all_productions_of_type[ DEFAULT_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
    {
        if ( prod->rl_rule )
            do_print_for_production( thisAgent, prod, intern, print_filename, full_prod );
    }

    for ( production *prod = thisAgent->all_productions_of_type[ USER_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
    {
        if ( prod->rl_rule )
            do_print_for_production( thisAgent, prod, intern, print_filename, full_prod );
    }

    for ( production *prod = thisAgent->all_productions_of_type[ CHUNK_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
    {
        if ( prod->rl_rule )
            do_print_for_production( thisAgent, prod, intern, print_filename, full_prod );
    }
}

// compare_attr is used for cstdlib::qsort below. 
int compare_attr (const void * e1, const void * e2)
{
    wme **p1, **p2;

    char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

    p1 = (wme **) e1;
    p2 = (wme **) e2;

    // passing null thisAgent is OK as long as dest is guaranteed != 0
    symbol_to_string (0, (*p1)->attr, TRUE, s1, MAX_LEXEME_LENGTH*2+20);
    symbol_to_string (0, (*p2)->attr, TRUE, s2, MAX_LEXEME_LENGTH*2+20);

    return strcmp (s1, s2);
}

/* This should probably be in the Soar kernel interface. */
#define NEATLY_PRINT_BUF_SIZE 10000
void neatly_print_wme_augmentation_of_id (agent* thisAgent, wme *w, int indentation)
{
    char buf[NEATLY_PRINT_BUF_SIZE], *ch;

    xml_object(thisAgent, w);

    strcpy(buf, " ^");
    ch = buf;
    while (*ch) 
        ch++;
    symbol_to_string(thisAgent, w->attr, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
    *(ch++) = ' ';
    symbol_to_string(thisAgent, w->value, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
    if (w->metadata & METADATA_ACCEPTABLE) 
    { 
        strcpy(ch, " +"); 
        while (*ch) 
            ch++; 
    }

    if (get_printer_output_column(thisAgent) + (ch - buf) >= 80)
    {
        print(thisAgent, "\n");
        print_spaces(thisAgent, indentation+6);
    }
    print_string(thisAgent, buf);
}

// RPM 4/07: Note, mark_depths_augs_of_id must be called before the root call to print_augs_of_id
//           Thus, this should probably only be called from do_print_for_identifier
void print_augs_of_id (agent* thisAgent, Symbol *id, int depth, int maxdepth, bool intern, bool tree, tc_number tc) 
{
    slot *s;
    wme *w;

    wme **list;    /* array of WME pointers, AGR 652 */
    int num_attr;  /* number of attributes, AGR 652 */
    int attr;      /* attribute index, AGR 652 */

    /* AGR 652  The plan is to go through the list of WMEs and find out how
    many there are.  Then we malloc an array of that many pointers.
    Then we go through the list again and copy all the pointers to that array.
    Then we qsort the array and print it out.  94.12.13 */

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
        return;
    if (id->id.tc_num==tc) 
        return;  // this has already been printed, so return RPM 4/07 bug 988
    if (id->id.depth > depth) 
        return;  // this can be reached via an equal or shorter path, so return without printing RPM 4/07 bug 988

    // if we're here, then we haven't printed this id yet, so print it

    depth = id->id.depth; // set the depth to the depth via the shallowest path, RPM 4/07 bug 988
    int indent = (maxdepth-depth)*2; // set the indent based on how deep we are, RPM 4/07 bug 988

    id->id.tc_num = tc;  // mark id as printed

    /* --- first, count all direct augmentations of this id --- */
    num_attr = 0;
    for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
        num_attr++;
    for (w=id->id.input_wmes; w!=NIL; w=w->next) 
        num_attr++;
    for (s=id->id.slots; s!=NIL; s=s->next) 
    {
        for (w=s->wmes; w!=NIL; w=w->next) 
            num_attr++;
        for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) 
            num_attr++;
    }

    /* --- next, construct the array of wme pointers and sort them --- */
    list = (wme**)allocate_memory(thisAgent, num_attr*sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
    attr = 0;
    for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
        list[attr++] = w;
    for (w=id->id.input_wmes; w!=NIL; w=w->next)
        list[attr++] = w;
    for (s=id->id.slots; s!=NIL; s=s->next) 
    {
        for (w=s->wmes; w!=NIL; w=w->next)
            list[attr++] = w;
        for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
            list[attr++] = w;
    }
    qsort (list, num_attr, sizeof (wme *), compare_attr); 


    /* --- finally, print the sorted wmes and deallocate the array --- */

    // RPM 4/07 If this is a tree print, then for each wme in the list, print it and its children
    if(tree) 
    {
        for (attr=0; attr < num_attr; attr++) 
        {
            w = list[attr];
            print_spaces (thisAgent, indent);
            if (intern) print_wme (thisAgent, w);
            else print_wme_without_timetag (thisAgent, w);

            if (depth>1)
            { // we're not done yet
                /* --- call this routine recursively --- */
                print_augs_of_id (thisAgent, w->attr, depth-1, maxdepth, intern, tree, tc);
                print_augs_of_id (thisAgent, w->value, depth-1, maxdepth, intern, tree, tc);
            }
        }
        // RPM 4/07 This is not a tree print, so for each wme in the list, print it
        // Then, after all wmes have been printed, print the children
    } 
    else 
    {
        for (attr=0; attr < num_attr; attr++) 
        {
            w = list[attr];
            print_spaces (thisAgent, indent);

            if(intern) 
            {
                print_wme (thisAgent, w);
            } 
            else 
            {
                print_with_symbols (thisAgent, "(%y", id);

                // XML format of an <id> followed by a series of <wmes> each of which shares the original ID.
                // <id id="s1"><wme tag="123" attr="foo" attrtype="string" val="123" valtype="string"></wme><wme attr="bar" ...></wme></id>
                xml_begin_tag(thisAgent, soar_TraceNames::kWME_Id);
                xml_att_val(thisAgent, soar_TraceNames::kWME_Id, id);

                for (attr=0; attr < num_attr; attr++) 
                {
                    w = list[attr];
                    neatly_print_wme_augmentation_of_id (thisAgent, w, indent);
                }

                xml_end_tag(thisAgent, soar_TraceNames::kWME_Id);

                print (thisAgent, ")\n");
            }
        }

        // If there is still depth left, recurse
        if (depth > 1)
        {
            for (attr=0; attr < num_attr; attr++) 
            {
                w = list[attr];
                /* --- call this routine recursively --- */
                print_augs_of_id (thisAgent, w->attr, depth-1, maxdepth, intern, tree, tc);
                print_augs_of_id (thisAgent, w->value, depth-1, maxdepth, intern, tree, tc);
            }
        }
    }

    // deallocate the array
    free_memory(thisAgent, list, MISCELLANEOUS_MEM_USAGE);
}

/* RPM 4/07 bug 988
This function traverses the ids we are going to print and marks each with its shallowest depth
That is, if an id can be reached by multiple paths, this will find the shortest one and save
the depth of that path on the id.  Thus, when we print, the wmes will be indented properly,
making it much easier to read, and avoiding bugs (see bug 988).
*/
void mark_depths_augs_of_id (agent* thisAgent, Symbol *id, int depth, tc_number tc) 
{
    slot *s;
    wme *w;

    /* AGR 652  The plan is to go through the list of WMEs and find out how
    many there are.  Then we malloc an array of that many pointers.
    Then we go through the list again and copy all the pointers to that array.
    Then we qsort the array and print it out.  94.12.13 */

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return;
    if (id->id.tc_num==tc && id->id.depth >= depth) 
        return;  // this has already been printed at an equal-or-lower depth, RPM 4/07 bug 988

    id->id.depth = depth; // set the depth of this id
    id->id.tc_num = tc;

    /* --- if depth<=1, we're done --- */
    if (depth<=1)
        return;

    /* --- call this routine recursively --- */
    for (w=id->id.input_wmes; w!=NIL; w=w->next) 
    {
        mark_depths_augs_of_id (thisAgent, w->attr, depth-1, tc);
        mark_depths_augs_of_id (thisAgent, w->value, depth-1, tc);
    }
    for (w=id->id.impasse_wmes; w!=NIL; w=w->next) 
    {
        mark_depths_augs_of_id (thisAgent, w->attr, depth-1, tc);
        mark_depths_augs_of_id (thisAgent, w->value, depth-1, tc);
    }
    for (s=id->id.slots; s!=NIL; s=s->next) 
    {
        for (w=s->wmes; w!=NIL; w=w->next)
        {
            mark_depths_augs_of_id (thisAgent, w->attr, depth-1, tc);
            mark_depths_augs_of_id (thisAgent, w->value, depth-1, tc);
        }
        for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
        {
            mark_depths_augs_of_id (thisAgent, w->attr, depth-1, tc);
            mark_depths_augs_of_id (thisAgent, w->value, depth-1, tc);
        }
    }
}

void do_print_for_identifier (agent* thisAgent, Symbol *id, int depth, bool intern, bool tree)
{
    tc_number tc;

    // RPM 4/07: first mark the nodes with their shallowest depth
    //           then print them at their shallowest depth
    tc = get_new_tc_number(thisAgent);
    mark_depths_augs_of_id (thisAgent, id, depth, tc);
    tc = get_new_tc_number(thisAgent);
    print_augs_of_id (thisAgent, id, depth, depth, intern, tree, tc);
}

void do_print_for_production_name (agent* thisAgent, const char *prod_name, bool intern, bool print_filename, bool full_prod) 
{
    Symbol *sym;

    sym = find_sym_constant (thisAgent, thisAgent->lexeme.string);
    if (sym && sym->sc.production) 
        do_print_for_production(thisAgent, sym->sc.production, intern, print_filename, full_prod);
    else 
        print (thisAgent, "No production named %s\n", prod_name);
}

void do_print_for_wme (agent* thisAgent, wme *w, int depth, bool intern, bool tree) {
    if (intern && (depth==0)) 
    {
        print_wme (thisAgent, w);
        print (thisAgent, "\n");
    } 
    else
        do_print_for_identifier(thisAgent, w->id, depth, intern, tree);
}

/* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
otherwise return 2 and set dest_sym to find_symbol() result. --- */
int read_pattern_component (agent* thisAgent, Symbol **dest_sym) 
{
    if (strcmp(thisAgent->lexeme.string,"*") == 0) 
        return 1;
    switch (thisAgent->lexeme.type) 
    {
    case SYM_CONSTANT_LEXEME:
        *dest_sym = find_sym_constant (thisAgent, thisAgent->lexeme.string); return 2;
    case INT_CONSTANT_LEXEME:
        *dest_sym = find_int_constant (thisAgent, thisAgent->lexeme.int_val); return 2;
    case FLOAT_CONSTANT_LEXEME:
        *dest_sym = find_float_constant (thisAgent, thisAgent->lexeme.float_val); return 2;
    case IDENTIFIER_LEXEME:
        *dest_sym = find_identifier (thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number); return 2;
    case VARIABLE_LEXEME:
        *dest_sym = read_identifier_or_context_variable(thisAgent);
        if (*dest_sym) 
            return 2;
        return 0;
    default:
        print (thisAgent, "Expected identifier or constant in wme pattern\n");
        return 0;
    }
}

list *read_pattern_and_get_matching_wmes (agent* thisAgent) 
{
    int parentheses_level;
    list *wmes;
    wme *w;
    Symbol *id, *attr, *value;
    int id_result, attr_result, value_result;
    bool acceptable;

    if (thisAgent->lexeme.type!=L_PAREN_LEXEME)
    {
        print (thisAgent, "Expected '(' to begin wme pattern not string '%s' or char '%c'\n", thisAgent->lexeme.string, thisAgent->current_char);
        return NIL;
    }
    parentheses_level = current_lexer_parentheses_level(thisAgent);

    get_lexeme(thisAgent);
    id_result = read_pattern_component (thisAgent, &id);
    if (! id_result) 
    {
        skip_ahead_to_balanced_parentheses (thisAgent, parentheses_level-1);
        return NIL;
    }
    get_lexeme(thisAgent);
    if (thisAgent->lexeme.type!=UP_ARROW_LEXEME) 
    {
        print (thisAgent, "Expected ^ in wme pattern\n");
        skip_ahead_to_balanced_parentheses (thisAgent, parentheses_level-1);
        return NIL;
    }
    get_lexeme(thisAgent);
    attr_result = read_pattern_component (thisAgent, &attr);
    if (! attr_result) 
    {
        skip_ahead_to_balanced_parentheses (thisAgent, parentheses_level-1);
        return NIL;
    }
    get_lexeme(thisAgent);
    value_result = read_pattern_component (thisAgent, &value);
    if (! value_result) 
    {
        skip_ahead_to_balanced_parentheses (thisAgent, parentheses_level-1);
        return NIL;
    }
    get_lexeme(thisAgent);
    if (thisAgent->lexeme.type==PLUS_LEXEME) 
    {
        acceptable = TRUE;
        get_lexeme(thisAgent);
    } 
    else
    {
        acceptable = FALSE;
    }
    if (thisAgent->lexeme.type!=R_PAREN_LEXEME) {
        print (thisAgent, "Expected ')' to end wme pattern\n");
        skip_ahead_to_balanced_parentheses (thisAgent, parentheses_level-1);
        return NIL;
    }

	// JUSTIN FIXME expand to include metadata

    wmes = NIL;
    for (w=thisAgent->all_wmes_in_rete; w!=NIL; w=w->rete_next)
    {
        if (((id_result==1) || (id==w->id)) &&
				((attr_result==1) || (attr==w->attr)) &&
				((value_result==1) || (value==w->value)) &&
				(acceptable && (w->metadata & METADATA_ACCEPTABLE)))
			push (thisAgent, w, wmes);
    }
    return wmes;  
}

void soar_alternate_input(agent *ai_agent, const char *ai_string, const char *ai_suffix, bool ai_exit)
{
    ai_agent->alternate_input_string = ai_string;
    ai_agent->alternate_input_suffix = ai_suffix;
    ai_agent->current_char = ' ';
    ai_agent->alternate_input_exit = ai_exit;
    return;
}

void print_symbol(agent* thisAgent, const char* arg, bool print_filename, bool intern, bool tree, bool full_prod, int depth, bool exact)
{
    cons *c;
    Symbol *id;
    wme* w;
    list* wmes;

    get_lexeme_from_string(thisAgent, arg);

    switch (thisAgent->lexeme.type) 
    {
    case SYM_CONSTANT_LEXEME:
        do_print_for_production_name(thisAgent, arg, intern, print_filename, full_prod);
        break;

    case INT_CONSTANT_LEXEME:
        for (w=thisAgent->all_wmes_in_rete; w!=NIL; w=w->rete_next)
        {
            if (w->timetag == static_cast<uint64_t>(thisAgent->lexeme.int_val))
            {
                do_print_for_wme(thisAgent, w, depth, intern, tree);
                break;
            }
        }
        if (!w) 
            print(thisAgent, "No wme %ld in working memory.", thisAgent->lexeme.int_val);
        break;

    case IDENTIFIER_LEXEME:
    case VARIABLE_LEXEME:
        id = read_identifier_or_context_variable(thisAgent);
        if (id) 
            do_print_for_identifier(thisAgent, id, depth, intern, tree);
        break;

    case QUOTED_STRING_LEXEME:
        /* Soar-Bugs #54 TMH */
        soar_alternate_input(thisAgent, arg, ") ", true);
        /* ((agent *)clientData)->alternate_input_string = argv[next_arg];
        * ((agent *)clientData)->alternate_input_suffix = ") ";
        */
        get_lexeme(thisAgent);
        wmes = read_pattern_and_get_matching_wmes(thisAgent);
        soar_alternate_input(thisAgent, NIL, NIL, FALSE); 
        thisAgent->current_char = ' ';
        if (exact)
        {
            // When printing exact, we want to list only those wmes who match.
            // Group up the wmes in objects (id ^attr value ^attr value ...)
            std::map< Symbol*, std::list< wme* > > objects;
            for (c = wmes; c != NIL; c = c->rest)
            {
                wme* current = static_cast<wme *>(c->first);
                objects[current->id].push_back(current);
            }
            // Loop through objects and print its wmes
            std::map< Symbol*, std::list<wme*> >::iterator iter = objects.begin();
            while ( iter != objects.end() ) 
            {
                std::list<wme*> wmelist = iter->second;
                std::list<wme*>::iterator wmeiter = wmelist.begin();

                // If we're printing internally, we just print the wme
                // taken from print_wme_without_timetag
                if (!intern) 
                    print_with_symbols(thisAgent, "(%y", iter->first);

                while ( wmeiter != wmelist.end() ) 
                {
                    wme* w = *wmeiter;
                    if (intern) 
                    {
                        // This does everything for us in the internal case, including xml
                        print_wme(thisAgent, w);
                    }
                    else 
                    {
                        // taken from print_wme_without_timetag
                        print_with_symbols (thisAgent, " ^%y %y", w->attr, w->value);
                        if (w->metadata & METADATA_ACCEPTABLE) 
                            print_string (thisAgent, " +");

                        // this handles xml case for the wme
                        xml_object( thisAgent, w, XML_WME_NO_TIMETAG );
                    }
                    ++wmeiter;
                }

                if (!intern)
                    print_string(thisAgent, ")\n");
                ++iter;
            }
        } 
        else 
        {
            for (c = wmes; c != NIL; c = c->rest) 
                do_print_for_wme(thisAgent, static_cast<wme *>(c->first), depth, intern, tree);
        }
        free_list(thisAgent, wmes);
        break;

    default:
        // TODO: Report error? Unrecognized arg?
        return;
    } /* end of switch statement */
}

bool CommandLineInterface::DoPrint(PrintBitset options, int depth, const std::string* pArg) 
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (depth < 0)
        depth = agnt->default_wme_depth;

    if (options.test(PRINT_STACK)) 
    {
        // if neither states option nor operators option are set, set them both
        if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS)) 
        {
            options.set(PRINT_STATES);
            options.set(PRINT_OPERATORS);
        }

        print_stack_trace(agnt, options.test(PRINT_STATES), options.test(PRINT_OPERATORS));
        return true;
    }

    // Cache the flags since it makes function calls huge
    bool intern = options.test(PRINT_INTERNAL);
    bool tree = options.test(PRINT_TREE);
    bool filename = options.test(PRINT_FILENAME);
    bool full = options.test(PRINT_FULL) || intern; // internal implies full
    bool exact = options.test(PRINT_EXACT);

    if (pArg) 
    {
        // Default with arg is full print
        full = !options.test(PRINT_NAME);

        // Watch out, this case handles all args identifier/pattern/timetag/production_name
        m_VarPrint = options.test(PRINT_VARPRINT); // This is a member because it affects the print callback.
        print_symbol(agnt, pArg->c_str(), filename, intern, tree, full, depth, exact);
        m_VarPrint = false;
        return true;
    } 

    // Print all productions if there are no other production flags set
    if (options.test(PRINT_ALL) || 
        (!options.test(PRINT_CHUNKS) &&
        !options.test(PRINT_DEFAULTS) &&
        !options.test(PRINT_JUSTIFICATIONS) &&
        !options.test(PRINT_USER) &&
        !options.test(PRINT_RL) &&
        !options.test(PRINT_TEMPLATE))) 
    {
        options.set(PRINT_CHUNKS);
        options.set(PRINT_DEFAULTS);
        options.set(PRINT_JUSTIFICATIONS);
        options.set(PRINT_USER);
        //options.set(PRINT_RL);     // these aren't really a different "type", it's just extra data
        options.set(PRINT_TEMPLATE);
    } 

    if (options.test(PRINT_CHUNKS)) 
        print_productions_of_type(agnt, intern, filename, full, CHUNK_PRODUCTION_TYPE);
    if (options.test(PRINT_DEFAULTS)) 
        print_productions_of_type(agnt, intern, filename, full, DEFAULT_PRODUCTION_TYPE);
    if (options.test(PRINT_JUSTIFICATIONS)) 
        print_productions_of_type(agnt, intern, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
    if (options.test(PRINT_USER)) 
        print_productions_of_type(agnt, intern, filename, full, USER_PRODUCTION_TYPE);
    if (options.test(PRINT_RL)) 
        print_rl_rules(agnt, intern, filename, full);
    if (options.test(PRINT_TEMPLATE)) 
        print_productions_of_type(agnt, intern, filename, full, TEMPLATE_PRODUCTION_TYPE);

    return true;
}
