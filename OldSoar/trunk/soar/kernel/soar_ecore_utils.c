#include "soarkernel.h"
#include "soar_ecore_utils.h"

extern Symbol *make_symbol_for_current_lexeme(void);

void cb_soar_PrintToFile(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{

    FILE *f = (FILE *) data;

    the_agent = the_agent;      /* stops compiler warning */

    fputs((char *) call_data, f);
}

bool wme_filter_component_match(Symbol * filterComponent, Symbol * wmeComponent)
{
    if ((filterComponent->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) && (!strcmp(filterComponent->sc.name, "*")))
        return TRUE;
    else
        return (bool) (filterComponent == wmeComponent);
}

bool passes_wme_filtering(wme * w, bool isAdd)
{
    cons *c;
    wme_filter *wf;

    /*  print ("testing wme for filtering: ");  print_wme(w); */

    if (!current_agent(wme_filter_list))
        return TRUE;            /* no filters defined -> everything passes */
    for (c = current_agent(wme_filter_list); c != NIL; c = c->rest) {
        wf = (wme_filter *) c->first;
        /*     print_with_symbols("  trying filter: %y ^%y %y\n",wf->id,wf->attr,wf->value); */
        if (((isAdd && wf->adds) || ((!isAdd) && wf->removes))
            && wme_filter_component_match(wf->id, w->id)
            && wme_filter_component_match(wf->attr, w->attr)
            && wme_filter_component_match(wf->value, w->value))
            return TRUE;
    }
    return FALSE;               /* no defined filters match -> w passes */
}

int parse_filter_type(const char *s, bool * forAdds, bool * forRemoves)
{
    if (!strcmp(s, "-adds")) {
        *forAdds = TRUE;
        *forRemoves = FALSE;
        return SOAR_OK;
    } else if (!strcmp(s, "-removes")) {
        *forAdds = FALSE;
        *forRemoves = TRUE;
        return SOAR_OK;
    } else if (!strcmp(s, "-both")) {
        *forAdds = TRUE;
        *forRemoves = TRUE;
        return SOAR_OK;
    }
    return SOAR_ERROR;
}

/* kjh(CUSP-B2) end */

#ifdef USE_CAPTURE_REPLAY

void capture_input_wme(enum captured_action_type action, soarapi_wme * sw, wme * w)
{

    switch (action) {

    case ADD_WME:
        fprintf(current_agent(capture_fileID),
                "%ld : %d : add-wme : %ld", current_agent(d_cycle_count), ADD_WME, sw->timetag);

        fprintf(current_agent(capture_fileID), " : %s %s %s : ", sw->id, sw->attr, sw->value);

        fprintf(current_agent(capture_fileID), "%s ", symbol_to_string(w->id, TRUE, NULL, 0 ));

        fprintf(current_agent(capture_fileID), "%s ", symbol_to_string(w->attr, TRUE, NULL, 0));

        fprintf(current_agent(capture_fileID), "%s\n", symbol_to_string(w->value, TRUE, NULL, 0));

        break;

    case REMOVE_WME:
        fprintf(current_agent(capture_fileID),
                "%ld : %d : remove-wme :   : %ld\n", current_agent(d_cycle_count), REMOVE_WME, sw->timetag);
        break;

    }

}

void replay_input_wme(soar_callback_agent agent, soar_callback_data dummy, soar_call_data mode)
{

    /* this routine is registered as a callback when user issues a
     * "replay-input -open fname" command in Soar.  The file is
     * opened in the interface routine ReplayInputCmd.
     */

    long tt;
    psoar_wme psw;
    captured_action *c_action;
    soarapi_wme *c_wme;
    agent;  /* hopefully quells compiler warning */
    dummy;  /* hopefully quells compiler warning */

    if (mode != (soar_call_data) NORMAL_INPUT_CYCLE)
        return;

    /*print ("replaying input for cycle %d\n", current_agent(d_cycle_count) ); */

    if (current_agent(d_cycle_count) > current_agent(dc_to_replay)) {
        print("\n\nWarning: end of replay has been reached.\n");
        return;
    }
    for (c_action = current_agent(replay_actions)[current_agent(d_cycle_count)];
         c_action != NULL; c_action = c_action->next) {

        switch (c_action->action) {

        case ADD_WME:
            c_wme = ((soarapi_wme *) c_action->args);

            tt = soar_cAddWme(c_wme->id, c_wme->attr, c_wme->value, FALSE, &psw);

            current_agent(replay_timetags)[c_wme->timetag] = tt;

            if (c_wme->timetag != tt && !current_agent(timetag_mismatch)) {

                print("\n\nWarning: replayed timetag mismatch (%d %d)!!", c_wme->timetag, tt);

                current_agent(timetag_mismatch) = TRUE;

            }

            break;

        case REMOVE_WME:

            soar_cRemoveWmeUsingTimetag(current_agent(replay_timetags)[((soarapi_wme *) c_action->args)->timetag]);
            break;

        default:
            print("Warning: Tried to replay an input with an unknown action type\n");
            break;

        }

    }
    return;
}

#endif                          /* USE_CAPTURE_REPLAY */

/*
 *----------------------------------------------------------------------
 *
 * compare_firing_counts --
 *
 *	This procedure compares two productions to determine
 *      how they compare vis-a-vis firing counts.
 *
 * Results:
 *	Returns -1 if the first production has fired less
 *               0 if they two productions have fired the same
 *               1 if the first production has fired more
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int compare_firing_counts(const void *e1, const void *e2)
{
    production *p1, *p2;
    unsigned long count1, count2;

    p1 = *((production **) e1);
    p2 = *((production **) e2);

    count1 = p1->firing_count;
    count2 = p2->firing_count;

    return (count1 < count2) ? -1 : (count1 > count2) ? 1 : 0;
}

production_memory_use *print_memories_insert_in_list(production_memory_use * new, production_memory_use * list)
{
    production_memory_use *ctr, *prev;

    /* Add to beginning. */
    if ((list == NULL) || (new->mem >= list->mem)) {
        new->next = list;
        return new;
    }

    /* Add to middle. */
    prev = list;
    for (ctr = list->next; ctr != NULL; ctr = ctr->next) {
        if (new->mem >= ctr->mem) {
            prev->next = new;
            new->next = ctr;
            return list;
        }
        prev = ctr;
    }

    /* Add to end. */
    prev->next = new;
    new->next = NULL;
    return list;
}

int read_wme_filter_component(const char *s, Symbol ** sym)
{
    get_lexeme_from_string(s);
    if (current_agent(lexeme).type == IDENTIFIER_LEXEME) {

        if ((*sym = find_identifier(current_agent(lexeme).id_letter, current_agent(lexeme).id_number)) == NIL) {
            return -1;          /* Identifier does not exist */
        }
    } else
        *sym = make_symbol_for_current_lexeme();
    return 0;
}

/* Soar-Bugs #54 TMH */
/*
 *----------------------------------------------------------------------
 *
 * soar_alternate_input --
 *
 *	This procedure initializes alternate input buffers for a
 *      soar agent.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	The soar agents alternate input values are updated and its
 *      current character is reset to a whitespace value.
 *
 *----------------------------------------------------------------------
 */

void soar_alternate_input(agent * ai_agent, const char *ai_string, const char *ai_suffix, bool ai_exit)
{
    ai_agent->alternate_input_string = ai_string;
    ai_agent->alternate_input_suffix = ai_suffix;
    ai_agent->current_char = ' ';
    ai_agent->alternate_input_exit = ai_exit;
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * read_attribute_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for an existing attribute.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_attribute_from_string(Symbol * id, const char *the_lexeme, Symbol * *attr)
{
    Symbol *attr_tmp;
    slot *s;

    /* skip optional '^' if present.  KJC added to Ken's code */
    if (*the_lexeme == '^') {
        the_lexeme++;
    }

    get_lexeme_from_string(the_lexeme);

    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
        attr_tmp = find_sym_constant(current_agent(lexeme).string);
        break;
    case INT_CONSTANT_LEXEME:
        attr_tmp = find_int_constant(current_agent(lexeme).int_val);
        break;
    case FLOAT_CONSTANT_LEXEME:
        attr_tmp = find_float_constant(current_agent(lexeme).float_val);
        break;
    case IDENTIFIER_LEXEME:
        attr_tmp = find_identifier(current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
        break;
    case VARIABLE_LEXEME:
        attr_tmp = read_identifier_or_context_variable();
        if (!attr_tmp)
            return SOAR_ERROR;
        break;
    default:
        return SOAR_ERROR;
    }
    s = find_slot(id, attr_tmp);
    if (s) {
        *attr = attr_tmp;
        return SOAR_OK;
    } else
        return SOAR_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * print_preference_and_source --
 *
 *	This procedure prints a preference and the production
 *      which is the source of the preference.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Prints the preference and its source production.
 *
 *----------------------------------------------------------------------
 */

void print_preference_and_source(preference * pref, bool print_source, wme_trace_type wtt)
{
    print_string("  ");
    print_object_trace(pref->value);
    print(" %c", preference_type_indicator(pref->type));
    if (preference_is_binary(pref->type))
        print_object_trace(pref->referent);
    if (pref->o_supported)
        print(" :O ");
    print("\n");
    if (print_source) {
        print("    From ");
        print_instantiation_with_wmes(pref->inst, wtt);
        print("\n");
    }
}
