#include "soarkernel.h"
#include "soar_core_utils.h"
#include "rhsfun.h"
#include "rhsfun_examples.h"

extern int soar_agent_ids[];
extern int agent_counter;
extern int next_available_agent_id();
extern void init_soar_agent();

/*
 *----------------------------------------------------------------------
 *
 * read_id_or_context_var_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for an identifier or context variable.
 * 
 *      Many interface routines take identifiers as arguments.  
 *      These ids can be given as normal ids, or as special variables 
 *      such as <s> for the current state, etc.  This routine reads 
 *      (without consuming it) an identifier or context variable, 
 *      and returns a pointer (Symbol *) to the id.  (In the case of 
 *      context variables, the instantiated variable is returned.  If 
 *      any error occurs (e.g., no such id, no instantiation of the 
 *      variable), an error message is printed and NIL is returned.
 *
 * Results:
 *	Pointer to a symbol for the variable or NIL.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_id_or_context_var_from_string(const char *the_lexeme, Symbol * *result_id)
{
    Symbol *id;
    Symbol *g, *attr, *value;

    get_lexeme_from_string(the_lexeme);

    if (current_agent(lexeme).type == IDENTIFIER_LEXEME) {
        id = find_identifier(current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
        if (!id) {
            return SOAR_ERROR;
        } else {
            *result_id = id;
            return SOAR_OK;
        }
    }

    if (current_agent(lexeme).type == VARIABLE_LEXEME) {
        get_context_var_info(&g, &attr, &value);

        if ((!attr) || (!value)) {
            return SOAR_ERROR;
        }

        if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
            return SOAR_ERROR;
        }

        *result_id = value;
        return SOAR_OK;
    }

    return SOAR_ERROR;
}

/*
 *  This function should be replaced by the one above and added to the
 *  Soar kernel. 
 */

Symbol *read_identifier_or_context_variable(void)
{
    Symbol *id;
    Symbol *g, *attr, *value;

    if (current_agent(lexeme).type == IDENTIFIER_LEXEME) {
        id = find_identifier(current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
        if (!id) {
            print("There is no identifier %c%lu.\n", current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
            print_location_of_most_recent_lexeme();
            return NIL;
        }
        return id;
    }
    if (current_agent(lexeme).type == VARIABLE_LEXEME) {
        get_context_var_info(&g, &attr, &value);
        if (!attr) {
            print("Expected identifier (or context variable)\n");
            print_location_of_most_recent_lexeme();
            return NIL;
        }
        if (!value) {
            print("There is no current %s.\n", current_agent(lexeme).string);
            print_location_of_most_recent_lexeme();
            return NIL;
        }
        if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
            print("The current %s ", current_agent(lexeme).string);
            print_with_symbols("(%y) is not an identifier.\n", value);
            print_location_of_most_recent_lexeme();
            return NIL;
        }
        return value;
    }
    print("Expected identifier (or context variable)\n");
    print_location_of_most_recent_lexeme();
    return NIL;
}

/*
 *----------------------------------------------------------------------
 *
 * name_to_production --
 *
 *	This procedure determines if a string matches an existing
 *      production name.  If so, the production is returned.
 *
 * Results:
 *	Returns the production if a match is found, NIL otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

production *name_to_production(const char *string_to_test)
{
    Symbol *sym;

    sym = find_sym_constant(string_to_test);

    if (sym && sym->sc.production)
        return sym->sc.production;
    else
        return NIL;
}

void do_print_for_identifier(Symbol * id, int depth, bool internal)
{
    tc_number tc;

    tc = get_new_tc_number();
    print_augs_of_id(id, depth, internal, 0, tc);
}

/*
 *----------------------------------------------------------------------
 *
 * get_lexeme_from_string --
 *
 *	A hack to get the Soar lexer to take a string
 *      as a lexeme and setup the agent lexeme structure.  It
 *      is assumed that the lexeme is composed of Soar
 *      "constituent" characters -- i.e., does not contain any
 *      Soar special characters.  
 *
 *      See lexer.c for more information.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	String copied to lexeme structure,  string length
 *      computed, and lexeme type determined.
 *      Overwrites previous lexeme contents. 
 *
 *----------------------------------------------------------------------
 */

void get_lexeme_from_string(const char *the_lexeme)
{
    int i;
    const char *c;
    bool sym_constant_start_found = FALSE;
    bool sym_constant_end_found = FALSE;

    for (c = the_lexeme, i = 0; *c; c++, i++) {
        if (*c == '|') {
            if (!sym_constant_start_found) {
                i--;
                sym_constant_start_found = TRUE;
            } else {
                i--;
                sym_constant_end_found = TRUE;
            }
        } else {
            current_agent(lexeme).string[i] = *c;
        }
    }

    current_agent(lexeme).string[i] = '\0';     /* Null terminate lexeme string */

    current_agent(lexeme).length = i;

    if (sym_constant_end_found) {
        current_agent(lexeme).type = SYM_CONSTANT_LEXEME;
    } else {
        determine_type_of_constituent_string();
    }
}

void get_context_var_info(Symbol ** dest_goal, Symbol ** dest_attr_of_slot, Symbol ** dest_current_value)
{

    get_context_var_info_from_string(current_agent(lexeme).string, dest_goal, dest_attr_of_slot, dest_current_value);
}

void get_context_var_info_from_string(char *str,
                                      Symbol ** dest_goal, Symbol ** dest_attr_of_slot, Symbol ** dest_current_value)
{

    Symbol *v, *g;
    int levels_up;
    wme *w;

    v = find_variable(str);
    if (v == current_agent(s_context_variable)) {
        levels_up = 0;
        *dest_attr_of_slot = current_agent(state_symbol);
    } else if (v == current_agent(o_context_variable)) {
        levels_up = 0;
        *dest_attr_of_slot = current_agent(operator_symbol);
    } else if (v == current_agent(ss_context_variable)) {
        levels_up = 1;
        *dest_attr_of_slot = current_agent(state_symbol);
    } else if (v == current_agent(so_context_variable)) {
        levels_up = 1;
        *dest_attr_of_slot = current_agent(operator_symbol);
    } else if (v == current_agent(sss_context_variable)) {
        levels_up = 2;
        *dest_attr_of_slot = current_agent(state_symbol);
    } else if (v == current_agent(sso_context_variable)) {
        levels_up = 2;
        *dest_attr_of_slot = current_agent(operator_symbol);
    } else if (v == current_agent(ts_context_variable)) {
        levels_up =
            current_agent(top_goal) ? current_agent(bottom_goal)->id.level - current_agent(top_goal)->id.level : 0;
        *dest_attr_of_slot = current_agent(state_symbol);
    } else if (v == current_agent(to_context_variable)) {
        levels_up =
            current_agent(top_goal) ? current_agent(bottom_goal)->id.level - current_agent(top_goal)->id.level : 0;
        *dest_attr_of_slot = current_agent(operator_symbol);
    } else {
        *dest_goal = NIL;
        *dest_attr_of_slot = NIL;
        *dest_current_value = NIL;
        return;
    }

    g = current_agent(bottom_goal);
    while (g && levels_up) {
        g = g->id.higher_goal;
        levels_up--;
    }
    *dest_goal = g;

    if (!g) {
        *dest_current_value = NIL;
        return;
    }

    if (*dest_attr_of_slot == current_agent(state_symbol)) {
        *dest_current_value = g;
    } else {
        w = g->id.operator_slot->wmes;
        *dest_current_value = w ? w->value : NIL;
    }
}

wme **get_augs_of_id(Symbol * id, tc_number tc, int *num_attr)
{
    slot *s;
    wme *w;

    wme **list;                 /* array of WME pointers, AGR 652 */
    int attr;                   /* attribute index, AGR 652 */
    int n;

/* AGR 652  The plan is to go through the list of WMEs and find out how
   many there are.  Then we malloc an array of that many pointers.
   Then we go through the list again and copy all the pointers to that array.
   Then we qsort the array and print it out.  94.12.13 */

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return NULL;
    if (id->id.tc_num == tc)
        return NULL;
    id->id.tc_num = tc;

    /* --- first, count all direct augmentations of this id --- */
    n = 0;
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        n++;
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        n++;
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            n++;
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            n++;
    }

    /* --- next, construct the array of wme pointers and sort them --- */
    list = allocate_memory(n * sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
    attr = 0;
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        list[attr++] = w;
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        list[attr++] = w;
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            list[attr++] = w;
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            list[attr++] = w;
    }
    qsort(list, n, sizeof(wme *), compare_attr);

    *num_attr = n;
    return list;

}

void print_augs_of_id(Symbol * id, int depth, bool internal, int indent, tc_number tc)
{
    slot *s;
    wme *w;

    wme **list;                 /* array of WME pointers, AGR 652 */
    int num_attr;               /* number of attributes, AGR 652 */
    int attr;                   /* attribute index, AGR 652 */

    list = get_augs_of_id(id, tc, &num_attr);
    if (!list)
        return;

    /* --- finally, print the sorted wmes and deallocate the array --- */
    if (internal) {
        for (attr = 0; attr < num_attr; attr++) {
            w = list[attr];
            print_spaces(indent);
            print_wme(w);
        }
    } else {
        print_spaces(indent);
        print_with_symbols("(%y", id);
        for (attr = 0; attr < num_attr; attr++) {
            w = list[attr];
            neatly_print_wme_augmentation_of_id(w, indent);
        }
        print(")\n");
    }
    free_memory(list, MISCELLANEOUS_MEM_USAGE);
/* AGR 652 end */

    /* --- if depth<=1, we're done --- */
    if (depth <= 1)
        return;

    /* --- call this routine recursively --- */
    for (w = id->id.input_wmes; w != NIL; w = w->next) {
        print_augs_of_id(w->attr, depth - 1, internal, indent + 2, tc);
        print_augs_of_id(w->value, depth - 1, internal, indent + 2, tc);
    }
    for (w = id->id.impasse_wmes; w != NIL; w = w->next) {
        print_augs_of_id(w->attr, depth - 1, internal, indent + 2, tc);
        print_augs_of_id(w->value, depth - 1, internal, indent + 2, tc);
    }
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next) {
            print_augs_of_id(w->attr, depth - 1, internal, indent + 2, tc);
            print_augs_of_id(w->value, depth - 1, internal, indent + 2, tc);
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next) {
            print_augs_of_id(w->attr, depth - 1, internal, indent + 2, tc);
            print_augs_of_id(w->value, depth - 1, internal, indent + 2, tc);
        }
    }
}

/* AGR 652 begin */
/* Comment from Bob:  
   Also, if speed becomes an issue, you might put in a special case check
   for the common case where both p1 and p2 are SYM_CONSTANT's, in which
   case you could avoid the symbol_to_string() calls entirely and just
   call strcmp() directly on the attribute names.  (Maybe just leave this
   as a comment for now, just in case somebody complains about print speed
   some day.)  */

/* The header for compare_attr needs to be defined in this way because
   otherwise we get compiler warnings at the qsort lines about the 4th
   argument being an incompatible pointer type.  */

#define S1_SIZE MAX_LEXEME_LENGTH*2+20
#define S2_SIZE MAX_LEXEME_LENGTH*2+20
int compare_attr(const void *e1, const void *e2)
{
    wme **p1, **p2;

    char s1[S1_SIZE], s2[S2_SIZE];

    p1 = (wme **) e1;
    p2 = (wme **) e2;

    symbol_to_string((*p1)->attr, TRUE, s1, S1_SIZE);
    symbol_to_string((*p2)->attr, TRUE, s2, S2_SIZE);

    return (strcmp(s1, s2));
}

/* This should probably be in the Soar kernel interface. */
#define NEATLY_PRINT_BUF_SIZE 10000
void neatly_print_wme_augmentation_of_id(wme * w, int indentation)
{
    char buf[NEATLY_PRINT_BUF_SIZE], *ch;

    strncpy(buf, " ^", NEATLY_PRINT_BUF_SIZE);
    buf[NEATLY_PRINT_BUF_SIZE - 1] = 0;
    ch = buf;
    while (*ch)
        ch++;
    symbol_to_string(w->attr, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf));
    while (*ch)
        ch++;
    *(ch++) = ' ';
    symbol_to_string(w->value, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf));
    while (*ch)
        ch++;

    if (w->acceptable) {
        strncpy(ch, " +", NEATLY_PRINT_BUF_SIZE - (ch - buf));
        ch[NEATLY_PRINT_BUF_SIZE - (ch - buf) - 1] = 0;
        while (*ch)
            ch++;
    }

    if (get_printer_output_column() + (ch - buf) >= 80) {
        print("\n");
        print_spaces(indentation + 6);
    }
    print_string(buf);
}

/*
 *----------------------------------------------------------------------
 *
 * string_match --
 *
 *	This procedure compares two strings to see if there 
 *      are equal.
 *
 * Results:
 *	TRUE if the strings match, FALSE otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool string_match(const char *string1, const char *string2)
{
    if ((string1 == NULL) && (string2 == NULL))
        return TRUE;

    if ((string1 != NULL)
        && (string2 != NULL)
        && !(strcmp(string1, string2)))
        return TRUE;
    else
        return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * string_match_up_to --
 *
 *	This procedure compares two strings to see if there 
 *      are equal up to the indicated number of positions.
 *
 * Results:
 *	TRUE if the strings match over the given number of
 *      characters, FALSE otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool string_match_up_to(const char *string1, const char *string2, int positions)
{
    unsigned int i, num;

    /*  what we really want is to require a match over the length of
       the shorter of the two strings, with positions being a minimum */

    num = strlen(string1);
    if (num > strlen(string2))
        num = strlen(string2);
    if ((unsigned int) positions < num)
        positions = num;

    for (i = 0; i < (unsigned int) positions; i++) {
        if (string1[i] != string2[i])
            return FALSE;
    }

    return TRUE;
}

void soar_default_create_agent_procedure(const char *agent_name)
{
    int i;                      /* loop index */
    char cur_path[MAXPATHLEN];  /* AGR 536 */

    agent *curr_agent;
    agent *this_agent;

    this_agent = (agent *) malloc(sizeof(agent));
    memset(this_agent, 0, sizeof(*this_agent));

    curr_agent = soar_agent;
    soar_agent = this_agent;

    agent_counter++;
    agent_count++;

    current_agent(id) = next_available_agent_id();
    current_agent(name) = savestring(agent_name);

    /* mvp 5-17-94 */
    current_agent(variables_set) = NIL;

    /* String redirection */
    /*
       current_agent(using_output_string)             = FALSE;
       current_agent(output_string)                           = NIL;
     */

    /* Who needs it if there's an alternate_input_string? */
    /*
       current_agent(input_string)                            = NIL;
       current_agent(using_input_string)              = FALSE;
     */

    current_agent(alias_list) = NIL;    /* AGR 568 */
    current_agent(all_wmes_in_rete) = NIL;
    current_agent(alpha_mem_id_counter) = 0;
    current_agent(alternate_input_string) = NIL;
    current_agent(alternate_input_suffix) = NIL;
    current_agent(alternate_input_exit) = FALSE;        /* Soar-Bugs #54 */
    current_agent(backtrace_number) = 0;
    current_agent(beta_node_id_counter) = 0;
    current_agent(bottom_goal) = NIL;
    current_agent(changed_slots) = NIL;
    current_agent(chunk_count) = 1;
    current_agent(chunk_free_problem_spaces) = NIL;
    current_agent(chunky_problem_spaces) = NIL; /* AGR MVL1 */
    strncpy(current_agent(chunk_name_prefix), "chunk", kChunkNamePrefixMaxLength);      /* kjh (B14) */
    current_agent(chunk_name_prefix)[kChunkNamePrefixMaxLength - 1] = 0;
    current_agent(context_slots_with_changed_acceptable_preferences) = NIL;
    current_agent(current_file) = NIL;
    current_agent(current_phase) = INPUT_PHASE;
    current_agent(current_symbol_hash_id) = 0;
    current_agent(current_variable_gensym_number) = 0;
    current_agent(current_wme_timetag) = 1;
    current_agent(default_wme_depth) = 1;       /* AGR 646 */
    current_agent(disconnected_ids) = NIL;
    current_agent(existing_output_links) = NIL;
    current_agent(output_link_changed) = FALSE; /* KJC 11/9/98 */
    /* current_agent(explain_flag)                       = FALSE; */
    current_agent(go_number) = 1;
    current_agent(go_type) = GO_DECISION;
    current_agent(grounds_tc) = 0;
    current_agent(highest_goal_whose_context_changed) = NIL;
    current_agent(ids_with_unknown_level) = NIL;
    current_agent(input_period) = 0;    /* AGR REW1 */
    current_agent(input_cycle_flag) = TRUE;     /* AGR REW1 */
    current_agent(justification_count) = 1;
    current_agent(lex_alias) = NIL;     /* AGR 568 */
    current_agent(link_update_mode) = UPDATE_LINKS_NORMALLY;
    current_agent(locals_tc) = 0;
    current_agent(logging_to_file) = FALSE;
    current_agent(max_chunks_reached) = FALSE;  /* MVP 6-24-94 */
    current_agent(mcs_counter) = 1;
    current_agent(memory_pools_in_use) = NIL;
    current_agent(ms_assertions) = NIL;
    current_agent(ms_retractions) = NIL;
    current_agent(multi_attributes) = NIL;
    current_agent(num_existing_wmes) = 0;
    current_agent(num_wmes_in_rete) = 0;
    current_agent(potentials_tc) = 0;
    current_agent(prev_top_state) = NIL;
    current_agent(print_prompt_flag) = TRUE;
    current_agent(printer_output_column) = 1;
    current_agent(production_being_fired) = NIL;
    current_agent(productions_being_traced) = NIL;
    current_agent(promoted_ids) = NIL;
    current_agent(reason_for_stopping) = "Startup";
    current_agent(redirecting_to_file) = FALSE;
    current_agent(slots_for_possible_removal) = NIL;
    current_agent(stop_soar) = TRUE;
    current_agent(system_halted) = FALSE;
    current_agent(token_additions) = 0;
    current_agent(top_dir_stack) = NIL; /* AGR 568 */
    current_agent(top_goal) = NIL;
    current_agent(top_state) = NIL;
    current_agent(wmes_to_add) = NIL;
    current_agent(wmes_to_remove) = NIL;
    current_agent(wme_filter_list) = NIL;

    /* REW: begin 09.15.96 */

    current_agent(did_PE) = FALSE;

#ifndef SOAR_8_ONLY
    current_agent(operand2_mode) = TRUE;
#endif

    current_agent(soar_verbose_flag) = FALSE;
    current_agent(FIRING_TYPE) = IE_PRODS;
    current_agent(ms_o_assertions) = NIL;
    current_agent(ms_i_assertions) = NIL;

    /* REW: end   09.15.96 */

    /* REW: begin 08.20.97 */
    current_agent(active_goal) = NIL;
    current_agent(active_level) = 0;
    current_agent(previous_active_level) = 0;

    /* Initialize Waterfall-specific lists */
    current_agent(nil_goal_retractions) = NIL;
    /* REW: end   08.20.97 */

    /* KJC: delineate between Pref/WM(propose) and Pref/WM (apply) 10.05.98 */
    current_agent(applyPhase) = FALSE;

    /* REW: begin 10.24.97 */
    current_agent(waitsnc) = FALSE;
    current_agent(waitsnc_detect) = FALSE;
    /* REW: end   10.24.97 */

    if (!sys_getwd(cur_path, MAXPATHLEN))
        print("Unable to set current directory while initializing agent.\n");

    current_agent(top_dir_stack) = (dir_stack_struct *) malloc(sizeof(dir_stack_struct));       /* AGR 568 */

    current_agent(top_dir_stack)->directory = (char *) malloc(MAXPATHLEN * sizeof(char));       /* AGR 568 */

    current_agent(top_dir_stack)->next = NIL;   /* AGR 568 */

    strncpy(current_agent(top_dir_stack)->directory, cur_path, MAXPATHLEN);     /* AGR 568 */
    current_agent(top_dir_stack)->directory[MAXPATHLEN - 1] = 0;

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++) {
        current_agent(all_productions_of_type)[i] = NIL;
        current_agent(num_productions_of_type)[i] = 0;
    }

    current_agent(o_support_calculation_type) = 4;      /* bugzilla bug 339 */
    current_agent(numeric_indifferent_mode) = NUMERIC_INDIFFERENT_MODE_AVG;     /* KJC 7/00 */
    current_agent(attribute_preferences_mode) = 0;      /* RBD 4/17/95 */

#ifdef USE_AGENT_DBG_FILE
    current_agent(dbgFile) = fopen("soarDbg", "w");
#endif
#ifdef WARN_IF_TIMERS_REPORT_ZERO
    current_agent(warn_on_zero_timers) = TRUE;
#endif
#ifdef USE_CAPTURE_REPLAY
    current_agent(capture_fileID) = NIL;
    current_agent(replay_fileID) = NIL;
    current_agent(timetag_mismatch) = FALSE;
#endif
#ifdef DC_HISTOGRAM
    current_agent(dc_histogram_sz) = 0;

    /* Set the offset high when this funct is not in use to speed up
     * d.c. increment
     */
    current_agent(dc_histogram_offset) = 1000000;
    current_agent(dc_histogram_freq) = 10;
#endif                          /* DC_HISTOGRAM */

#ifdef KT_HISTOGRAM
    current_agent(kt_histogram_sz) = 450;
    current_agent(kt_histogram_tv) = (struct timeval *)
        malloc(current_agent(kt_histogram_sz) * sizeof(struct timeval));
#endif                          /* KT_HISTOGRAM */

    /* Also sets callback_error to 0 */
    soar_init_callbacks((soar_callback_agent) soar_agent);

    init_soar_agent();
    /* Add agent to global list   */
    /* of all agents.             */
    push(soar_agent, all_soar_agents);

    soar_invoke_global_callbacks(soar_agent, GLB_AGENT_CREATED, (soar_call_data) soar_agent);

    /* 
     * If this is not the first agent created, reset the current agent
     * pointer
     */
    if (curr_agent != NULL) {
        soar_agent = curr_agent;
    }

}

void soar_default_destroy_agent_procedure(psoar_agent delete_agent)
{
    cons *c;
    cons *prev = NULL;          /* Initialized to placate gcc -Wall */
    agent *the_agent;
    bool already_deleted;

    already_deleted = TRUE;
    /* Splice agent structure out of global list of agents. */
    for (c = all_soar_agents; c != NIL; c = c->rest) {
        the_agent = (agent *) c->first;
        if (the_agent == delete_agent) {
            if (c == all_soar_agents) {
                all_soar_agents = c->rest;
            } else {
                prev->rest = c->rest;
            }
            already_deleted = FALSE;
            break;
        }
        prev = c;
    }

    /* save current-agent in temp variable */
    if (delete_agent == soar_agent)
        the_agent = (struct agent_struct *) c->rest;
    else
        the_agent = soar_agent;

    /* change the current agent to the one to delete */
    soar_agent = delete_agent;
    remove_built_in_rhs_functions();
    remove_bot_rhs_functions();

    if (already_deleted) {
        char msg[MESSAGE_SIZE];
        snprintf(msg, MESSAGE_SIZE, "Tried to delete invalid agent (%p).\n", soar_agent);
        msg[MESSAGE_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */
        abort_with_fatal_error(msg);
    }

    /* Free agent id */
    soar_agent_ids[((agent *) soar_agent)->id] = TOUCHED;

    /* Free structures stored in agent structure */
    free(((agent *) soar_agent)->name);

    /* KNOWN MEMORY LEAK! Need to track down and free ALL structures */
    /* pointed to be fields in the agent structure.                  */

    /* Free soar agent structure */
    free((void *) soar_agent);

    soar_agent = the_agent;
    agent_count--;

}
