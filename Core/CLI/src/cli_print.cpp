/////////////////////////////////////////////////////////////////
// print command file.
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
#include "lexer.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "reinforcement_learning.h"
#include "rhs.h"
#include "semantic_memory.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "trace.h"
#include "working_memory.h"
#include "xml.h"
#include "sml_Utils.h"

using namespace cli;
using namespace sml;

bool print_gds(agent* thisAgent)
{
    wme* w;
    Symbol* goal;


    thisAgent->outputManager->printa_sf(thisAgent,  "********************* Current GDS **************************\n");
    thisAgent->outputManager->printa_sf(thisAgent,  "stepping thru all wmes in rete, looking for any that are in a gds...\n");
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        if (w->gds)
        {
            if (w->gds->goal)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "  For Goal  %y  ", w->gds->goal);
            }
            else
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "  Old GDS value ");
            }
            thisAgent->outputManager->printa_sf(thisAgent,  "(%u: ", w->timetag);
            thisAgent->outputManager->printa_sf(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
            if (w->acceptable)
            {
                thisAgent->outputManager->printa(thisAgent, " +");
            }
            thisAgent->outputManager->printa(thisAgent, ")");
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        }
    }
    thisAgent->outputManager->printa_sf(thisAgent,  "************************************************************\n");
    for (goal = thisAgent->top_goal; goal != NIL; goal = goal->id->lower_goal)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "  For Goal  %y  ", goal);
        if (goal->id->gds)
        {
            /* Loop over all the WMEs in the GDS */
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
            for (w = goal->id->gds->wmes_in_gds; w != NIL; w = w->gds_next)
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "                (%u: ", w->timetag);
                thisAgent->outputManager->printa_sf(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
                if (w->acceptable)
                {
                    thisAgent->outputManager->printa(thisAgent, " +");
                }
                thisAgent->outputManager->printa(thisAgent, ")");
                thisAgent->outputManager->printa_sf(thisAgent,  "\n");
            }

        }
        else
        {
            thisAgent->outputManager->printa_sf(thisAgent,  ": No GDS for this goal.\n");
        }
    }

    thisAgent->outputManager->printa_sf(thisAgent,  "************************************************************\n");
    return true;
}

void print_stack_trace(agent* thisAgent, bool print_states, bool print_operators)
{
    // We don't want to keep printing forever (in case we're in a state no change cascade).
    const int maxStates = 500;
    int stateCount = 0 ;

    for (Symbol* g = thisAgent->top_goal; g != NIL; g = g->id->lower_goal)
    {
        stateCount++ ;

        if (stateCount > maxStates)
        {
            continue ;
        }

        if (print_states)
        {
            print_stack_trace(thisAgent, g, g, FOR_STATES_TF, false);
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        }
        if (print_operators && g->id->operator_slot->wmes)
        {
            print_stack_trace(thisAgent, g->id->operator_slot->wmes->value, g, FOR_OPERATORS_TF, false);
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        }
    }

    if (stateCount > maxStates)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "...Stack goes on for another %d states\n", static_cast<int64_t>(stateCount - maxStates));
    }
}

void do_print_for_production(agent* thisAgent, production* prod, bool intern, bool print_filename, bool full_prod)
{
    if (print_filename)
    {
        if (full_prod)
        {
            thisAgent->outputManager->printa(thisAgent, "# source file: ");
        }

        if (prod->filename)
        {
            thisAgent->outputManager->printa(thisAgent, prod->filename);
        }
        else
        {
            thisAgent->outputManager->printa(thisAgent, "_unknown_");
        }

        if (full_prod)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        }
        else
        {
            thisAgent->outputManager->printa(thisAgent, ": ");
        }
    }

    if (full_prod)
    {
        print_production(thisAgent, prod, intern);
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%y ", prod->name);

        if (prod->rl_rule)
        {
            // Do extra logging if this agent is in delta bar delta mode.
            if (thisAgent->RL->rl_params->decay_mode->get_value() == rl_param_container::delta_bar_delta_decay)
            {
                thisAgent->outputManager->printa_sf(thisAgent, " %y", thisAgent->symbolManager->make_float_constant(prod->rl_delta_bar_delta_beta));
                thisAgent->outputManager->printa_sf(thisAgent, " %y", thisAgent->symbolManager->make_float_constant(prod->rl_delta_bar_delta_h));
            }
            thisAgent->outputManager->printa_sf(thisAgent, " %y", thisAgent->symbolManager->make_float_constant(prod->rl_update_count));
            thisAgent->outputManager->printa_sf(thisAgent, " %y", rhs_value_to_symbol(prod->action_list->referent));
        }
    }
    thisAgent->outputManager->printa_sf(thisAgent,  "\n");
}

void print_productions_of_type(agent* thisAgent, bool intern, bool print_filename, bool full_prod, unsigned int productionType)
{
    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
    {
        do_print_for_production(thisAgent, prod, intern, print_filename, full_prod);
    }
}

void print_rl_rules(agent* thisAgent, bool intern, bool print_filename, bool full_prod)
{
    assert(thisAgent);

    for (production* prod = thisAgent->all_productions_of_type[ DEFAULT_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next)
    {
        if (prod->rl_rule)
        {
            do_print_for_production(thisAgent, prod, intern, print_filename, full_prod);
        }
    }

    for (production* prod = thisAgent->all_productions_of_type[ USER_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next)
    {
        if (prod->rl_rule)
        {
            do_print_for_production(thisAgent, prod, intern, print_filename, full_prod);
        }
    }

    for (production* prod = thisAgent->all_productions_of_type[ CHUNK_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next)
    {
        if (prod->rl_rule)
        {
            do_print_for_production(thisAgent, prod, intern, print_filename, full_prod);
        }
    }
}

// compare_attr is used for cstdlib::qsort below.
int compare_attr(const void* e1, const void* e2)
{
    wme** p1, **p2;

    char s1[output_string_size + 10], s2[output_string_size + 10];

    p1 = (wme**) e1;
    p2 = (wme**) e2;

    // passing null thisAgent is OK as long as dest is guaranteed != 0
    (*p1)->attr->to_string(true, false, s1, output_string_size + 10);
    (*p2)->attr->to_string(true, false, s2, output_string_size + 10);

    return strcmp(s1, s2);
}

/* This should probably be in the Soar kernel interface. */
#define NEATLY_PRINT_BUF_SIZE 10000
void neatly_print_wme_augmentation_of_id(agent* thisAgent, wme* w, int indentation)
{
    char buf[NEATLY_PRINT_BUF_SIZE], *ch;

    xml_object(thisAgent, w);

    strcpy(buf, " ^");
    ch = buf;
    while (*ch)
    {
        ch++;
    }
    w->attr->to_string(true, true, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf));
    while (*ch)
    {
        ch++;
    }
    *(ch++) = ' ';
    w->value->to_string(true, true, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf));
    while (*ch)
    {
        ch++;
    }
    if (w->acceptable)
    {
        strcpy(ch, " +");
        while (*ch)
        {
            ch++;
        }
    }

    if (thisAgent->outputManager->get_printer_output_column(thisAgent) + (ch - buf) >= 80)
    {
        thisAgent->outputManager->start_fresh_line(thisAgent);
        thisAgent->outputManager->print_spaces(thisAgent, indentation + 6);
    }
    thisAgent->outputManager->printa(thisAgent, buf);
}

// RPM 4/07: Note, mark_depths_augs_of_id must be called before the root call to print_augs_of_id
//           Thus, this should probably only be called from do_print_for_identifier
void print_augs_of_id(agent* thisAgent, Symbol* id, int depth, int maxdepth, bool intern, bool tree, tc_number tc)
{
    slot* s;
    wme* w;

    wme** list;    /* array of WME pointers, AGR 652 */
    int num_attr;  /* number of attributes, AGR 652 */
    int attr;      /* attribute index, AGR 652 */

    /* AGR 652  The plan is to go through the list of WMEs and find out how
    many there are.  Then we malloc an array of that many pointers.
    Then we go through the list again and copy all the pointers to that array.
    Then we qsort the array and print it out.  94.12.13 */

    if (!id->is_sti())
    {
        return;
    }
    if (id->tc_num == tc)
    {
        return;    // this has already been printed, so return RPM 4/07 bug 988
    }
    if (id->id->depth > depth)
    {
        return;    // this can be reached via an equal or shorter path, so return without printing RPM 4/07 bug 988
    }

    // if we're here, then we haven't printed this id yet, so print it

    depth = id->id->depth; // set the depth to the depth via the shallowest path, RPM 4/07 bug 988
    int indent = (maxdepth - depth) * 2; // set the indent based on how deep we are, RPM 4/07 bug 988

    id->tc_num = tc;  // mark id as printed

    /* --- first, count all direct augmentations of this id --- */
    num_attr = 0;
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        num_attr++;
    }
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        num_attr++;
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            num_attr++;
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            num_attr++;
        }
    }

    /* --- next, construct the array of wme pointers and sort them --- */
    list = (wme**)thisAgent->memoryManager->allocate_memory(num_attr * sizeof(wme*), MISCELLANEOUS_MEM_USAGE);
    attr = 0;
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        list[attr++] = w;
    }
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        list[attr++] = w;
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            list[attr++] = w;
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            list[attr++] = w;
        }
    }
    qsort(list, num_attr, sizeof(wme*), compare_attr);


    /* --- finally, print the sorted wmes and deallocate the array --- */

    // RPM 4/07 If this is a tree print, then for each wme in the list, print it and its children
    if (tree)
    {
        for (attr = 0; attr < num_attr; attr++)
        {
            w = list[attr];
            thisAgent->outputManager->print_spaces(thisAgent, indent);
            if (intern)
            {
                print_wme(thisAgent, w);
            }
            else
            {
                print_wme_without_timetag(thisAgent, w);
            }

            if (depth > 1)
            {
                // we're not done yet
                /* --- call this routine recursively --- */
                print_augs_of_id(thisAgent, w->attr, depth - 1, maxdepth, intern, tree, tc);
                print_augs_of_id(thisAgent, w->value, depth - 1, maxdepth, intern, tree, tc);
            }
        }
        // RPM 4/07 This is not a tree print, so for each wme in the list, print it
        // Then, after all wmes have been printed, print the children
    }
    else
    {
        for (attr = 0; attr < num_attr; attr++)
        {
            w = list[attr];
            thisAgent->outputManager->print_spaces(thisAgent, indent);

            if (intern)
            {
                print_wme(thisAgent, w);
            }
            else
            {
                thisAgent->outputManager->printa_sf(thisAgent, "(%y", id);

                // XML format of an <id> followed by a series of <wmes> each of which shares the original ID.
                // <id id="s1"><wme tag="123" attr="foo" attrtype="string" val="123" valtype="string"></wme><wme attr="bar" ...></wme></id>
                xml_begin_tag(thisAgent, soar_TraceNames::kWME_Id);
                xml_att_val(thisAgent, soar_TraceNames::kWME_Id, id);

                for (attr = 0; attr < num_attr; attr++)
                {
                    w = list[attr];
                    neatly_print_wme_augmentation_of_id(thisAgent, w, indent);
                }

                xml_end_tag(thisAgent, soar_TraceNames::kWME_Id);

                thisAgent->outputManager->printa_sf(thisAgent,  ")\n");
            }
        }

        // If there is still depth left, recurse
        if (depth > 1)
        {
            for (attr = 0; attr < num_attr; attr++)
            {
                w = list[attr];
                /* --- call this routine recursively --- */
                print_augs_of_id(thisAgent, w->attr, depth - 1, maxdepth, intern, tree, tc);
                print_augs_of_id(thisAgent, w->value, depth - 1, maxdepth, intern, tree, tc);
            }
        }
    }

    // deallocate the array
    thisAgent->memoryManager->free_memory(list, MISCELLANEOUS_MEM_USAGE);
}

/* RPM 4/07 bug 988
This function traverses the ids we are going to print and marks each with its shallowest depth
That is, if an id can be reached by multiple paths, this will find the shortest one and save
the depth of that path on the id.  Thus, when we print, the wmes will be indented properly,
making it much easier to read, and avoiding bugs (see bug 988).
*/
void mark_depths_augs_of_id(agent* thisAgent, Symbol* id, int depth, tc_number tc)
{
    slot* s;
    wme* w;

    /* AGR 652  The plan is to go through the list of WMEs and find out how
    many there are.  Then we malloc an array of that many pointers.
    Then we go through the list again and copy all the pointers to that array.
    Then we qsort the array and print it out.  94.12.13 */

    if (!id->is_sti())
    {
        return;
    }
    if (id->tc_num == tc && id->id->depth >= depth)
    {
        return;    // this has already been printed at an equal-or-lower depth, RPM 4/07 bug 988
    }

    id->id->depth = depth; // set the depth of this id
    id->tc_num = tc;

    /* --- if depth<=1, we're done --- */
    if (depth <= 1)
    {
        return;
    }

    /* --- call this routine recursively --- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        mark_depths_augs_of_id(thisAgent, w->attr, depth - 1, tc);
        mark_depths_augs_of_id(thisAgent, w->value, depth - 1, tc);
    }
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        mark_depths_augs_of_id(thisAgent, w->attr, depth - 1, tc);
        mark_depths_augs_of_id(thisAgent, w->value, depth - 1, tc);
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            mark_depths_augs_of_id(thisAgent, w->attr, depth - 1, tc);
            mark_depths_augs_of_id(thisAgent, w->value, depth - 1, tc);
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            mark_depths_augs_of_id(thisAgent, w->attr, depth - 1, tc);
            mark_depths_augs_of_id(thisAgent, w->value, depth - 1, tc);
        }
    }
}

void do_print_for_identifier(agent* thisAgent, Symbol* id, int depth, bool intern, bool tree)
{
    tc_number tc;

    // RPM 4/07: first mark the nodes with their shallowest depth
    //           then print them at their shallowest depth
    tc = get_new_tc_number(thisAgent);
    mark_depths_augs_of_id(thisAgent, id, depth, tc);
    tc = get_new_tc_number(thisAgent);
    print_augs_of_id(thisAgent, id, depth, depth, intern, tree, tc);
}

void do_print_for_production_name(agent* thisAgent, soar::Lexeme* lexeme, const char* prod_name, bool intern, bool print_filename, bool full_prod)
{
    Symbol* sym;

    sym = thisAgent->symbolManager->find_str_constant(lexeme->string());
    if (sym && sym->sc->production)
    {
        do_print_for_production(thisAgent, sym->sc->production, intern, print_filename, full_prod);
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "No production named %s\n", prod_name);
    }
}

void do_print_for_wme(agent* thisAgent, wme* w, int depth, bool intern, bool tree)
{
    if (intern && (depth == 0))
    {
        print_wme(thisAgent, w);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");
    }
    else
    {
        do_print_for_identifier(thisAgent, w->id, depth, intern, tree);
    }
}

/* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
otherwise return 2 and set dest_sym to find_symbol() result. --- */
int read_pattern_component(agent* thisAgent, soar::Lexeme* lexeme, Symbol** dest_sym)
{
    if (strcmp(lexeme->string(), "*") == 0)
    {
        return 1;
    }
    switch (lexeme->type)
    {
        case STR_CONSTANT_LEXEME:
            *dest_sym = thisAgent->symbolManager->find_str_constant(lexeme->string());
            return 2;
        case INT_CONSTANT_LEXEME:
            *dest_sym = thisAgent->symbolManager->find_int_constant(lexeme->int_val);
            return 2;
        case FLOAT_CONSTANT_LEXEME:
            *dest_sym = thisAgent->symbolManager->find_float_constant(lexeme->float_val);
            return 2;
        case IDENTIFIER_LEXEME:
            *dest_sym = thisAgent->symbolManager->find_identifier(lexeme->id_letter, lexeme->id_number);
            return 2;
        case VARIABLE_LEXEME:
            *dest_sym = read_identifier_or_context_variable(thisAgent, lexeme);
            if (*dest_sym)
            {
                return 2;
            }
            return 0;
        default:
            thisAgent->outputManager->printa_sf(thisAgent,  "Expected identifier or constant in wme pattern\n");
            return 0;
    }
}

cons* read_pattern_and_get_matching_wmes(agent* thisAgent, const char* pattern)
{
    int parentheses_level;
    cons* wmes;
    wme* w;
    Symbol* id, *attr, *value;
    int id_result, attr_result, value_result;
    bool acceptable;
    soar::Lexer lexer(thisAgent, pattern);
    lexer.get_lexeme();
    if (lexer.current_lexeme.type!=L_PAREN_LEXEME)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Expected '(' to begin wme pattern not string '%s' or char '%c'\n", lexer.current_lexeme.string(), lexer.current_char);
        return NIL;
    }
    parentheses_level = lexer.current_parentheses_level();

    lexer.get_lexeme();
    id_result = read_pattern_component(thisAgent, &(lexer.current_lexeme), &id);
    if (! id_result)
    {
        lexer.skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    lexer.get_lexeme();
    if (lexer.current_lexeme.type != UP_ARROW_LEXEME)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Expected ^ in wme pattern\n");
        lexer.skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    lexer.get_lexeme();
    attr_result = read_pattern_component(thisAgent, &(lexer.current_lexeme), &attr);
    if (! attr_result)
    {
        lexer.skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    lexer.get_lexeme();
    value_result = read_pattern_component(thisAgent, &(lexer.current_lexeme), &value);
    if (! value_result)
    {
        lexer.skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    lexer.get_lexeme();
    if (lexer.current_lexeme.type == PLUS_LEXEME)
    {
        acceptable = true;
        lexer.get_lexeme();
    }
    else
    {
        acceptable = false;
    }
    if (lexer.current_lexeme.type != R_PAREN_LEXEME)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Expected ')' to end wme pattern\n");
        lexer.skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }

    wmes = NIL;
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        if ((id_result == 1) || (id == w->id))
            if ((attr_result == 1) || (attr == w->attr))
                if ((value_result == 1) || (value == w->value))
                    if (acceptable == (w->acceptable == true))
                    {
                        push(thisAgent, w, wmes);
                    }
    }
    return wmes;
}

void print_symbol(agent* thisAgent, const char* arg, bool print_filename, bool intern, bool tree, bool full_prod, int depth, bool exact)
{
    cons* c;
    Symbol* id;
    wme* w;
    cons* wmes;

	soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, arg);

    switch (lexeme.type)
    {
        case STR_CONSTANT_LEXEME:
            if (lexeme.string()[0] == '@')
            {
                uint64_t lLti_id = 0;
                if (lexeme.string()[1] != '\0')
                {
                    lLti_id = strtol (&(lexeme.string()[1]),NULL,10);
                    if (lLti_id)
                    {
                        lLti_id = thisAgent->SMem->lti_exists(lLti_id);
                    }
                    else 
                    {
                        // Treat the rest of the token as an LTI alias
                        std::string lti_alias(lexeme.string());
                        lti_alias = lti_alias.substr(1);
                        lLti_id = thisAgent->SMem->get_lti_with_alias(lti_alias);
                    }
                    if (lLti_id == NIL)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent,  "LTI %s not found in semantic memory.", lexeme.string());
                        break;
                    }
                }
                thisAgent->SMem->attach();
                std::string smem_print_output;

                if (lLti_id == NIL)
                {
                    thisAgent->SMem->print_store(&(smem_print_output));
                }
                else
                {
                    thisAgent->SMem->print_smem_object(lLti_id, depth, &(smem_print_output));
                }
                thisAgent->outputManager->printa(thisAgent, smem_print_output.c_str());
            } else {
                do_print_for_production_name(thisAgent, &lexeme, arg, intern, print_filename, full_prod);
            }
            break;

        case INT_CONSTANT_LEXEME:
            for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
            {
                if (w->timetag == static_cast<uint64_t>(lexeme.int_val))
                {
                    do_print_for_wme(thisAgent, w, depth, intern, tree);
                    break;
                }
            }
            if (!w)
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "No wme %d in working memory.", lexeme.int_val);
            }
            break;

        case IDENTIFIER_LEXEME:
        case VARIABLE_LEXEME:
            id = read_identifier_or_context_variable(thisAgent, &lexeme);
            if (id)
            {
                do_print_for_identifier(thisAgent, id, depth, intern, tree);
            }
            break;

        case QUOTED_STRING_LEXEME:
        {
            wmes = read_pattern_and_get_matching_wmes(thisAgent, arg);
            if (exact)
            {
                // When printing exact, we want to list only those wmes who match.
                // Group up the wmes in objects (id ^attr value ^attr value ...)
                std::map< Symbol*, std::list< wme* > > objects;
                for (c = wmes; c != NIL; c = c->rest)
                {
                    wme* current = static_cast<wme*>(c->first);
                    objects[current->id].push_back(current);
                }
                // Loop through objects and print its wmes
                std::map< Symbol*, std::list<wme*> >::iterator iter = objects.begin();
                while (iter != objects.end())
                {
                    std::list<wme*> wmelist = iter->second;
                    std::list<wme*>::iterator wmeiter = wmelist.begin();

                    // If we're printing internally, we just print the wme
                    // taken from print_wme_without_timetag
                    if (!intern)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "(%y", iter->first);
                    }

                    while (wmeiter != wmelist.end())
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
                            thisAgent->outputManager->printa_sf(thisAgent, " ^%y %y", w->attr, w->value);
                            if (w->acceptable)
                            {
                                thisAgent->outputManager->printa(thisAgent, " +");
                            }

                            // this handles xml case for the wme
                            xml_object(thisAgent, w, XML_WME_NO_TIMETAG);
                        }
                        ++wmeiter;
                    }

                    if (!intern)
                    {
                        thisAgent->outputManager->printa(thisAgent, ")\n");
                    }
                    ++iter;
                }
            }
            else
            {
                for (c = wmes; c != NIL; c = c->rest)
                {
                    do_print_for_wme(thisAgent, static_cast<wme*>(c->first), depth, intern, tree);
                }
            }
            free_list(thisAgent, wmes);
            break;
        }
        default:
            // TODO: Report error? Unrecognized arg?
            return;
    } /* end of switch statement */
}

bool CommandLineInterface::DoPrint(PrintBitset options, int depth, const std::string* pArg)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (depth < 0)
    {
        depth = thisAgent->outputManager->settings[OM_PRINT_DEPTH];
    }

    if (options.test(PRINT_STACK))
    {
        // if neither states option nor operators option are set, set them both
        if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS))
        {
            options.set(PRINT_STATES);
            options.set(PRINT_OPERATORS);
        }

        print_stack_trace(thisAgent, options.test(PRINT_STATES), options.test(PRINT_OPERATORS));
        return true;
    }
    if (options.test(PRINT_GDS))
    {
        print_gds(thisAgent);
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
        print_symbol(thisAgent, pArg->c_str(), filename, intern, tree, full, depth, exact);
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

    if (options.test(PRINT_DEFAULTS))
    {
        print_productions_of_type(thisAgent, intern, filename, full, DEFAULT_PRODUCTION_TYPE);
    }
    if (options.test(PRINT_USER))
    {
        print_productions_of_type(thisAgent, intern, filename, full, USER_PRODUCTION_TYPE);
    }
    if (options.test(PRINT_TEMPLATE))
    {
        print_productions_of_type(thisAgent, intern, filename, full, TEMPLATE_PRODUCTION_TYPE);
    }
    if (options.test(PRINT_RL))
    {
        print_rl_rules(thisAgent, intern, filename, full);
    }
    if (options.test(PRINT_JUSTIFICATIONS))
    {
        print_productions_of_type(thisAgent, intern, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
    }
    if (options.test(PRINT_CHUNKS))
    {
        print_productions_of_type(thisAgent, intern, filename, full, CHUNK_PRODUCTION_TYPE);
    }

    return true;
}
