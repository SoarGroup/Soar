#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.cpp */

#include "stl_support.h"
#include "utilities.h"
#include "gdatastructs.h"
#include "wmem.h"
#include "print.h"

SoarSTLWMEPoolList* get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc)
{
	// notice how we give the constructor our custom SoarSTLWMEPoolList with the agent and memory type to use
	SoarSTLWMEPoolList* list = new SoarSTLWMEPoolList(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));

	slot *s;
    wme *w;

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return NULL;
    if (id->id.tc_num == tc)
        return NULL;
    id->id.tc_num = tc;

	// build list of wmes
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            list->push_back(w);
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            list->push_back(w);
    }

    return list;
}

bool read_id_or_context_var_from_string (agent* agnt, const char * the_lexeme,
	Symbol * * result_id) 
{
	Symbol *id;
	Symbol *g, *attr, *value;

	get_lexeme_from_string(agnt, the_lexeme);

	if (agnt->lexeme.type == IDENTIFIER_LEXEME) 
	{
		id = find_identifier(agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
		if (!id) 
		{
			return false;
		}
		else
		{
			*result_id = id;
			return true;
		}
	}

	if (agnt->lexeme.type==VARIABLE_LEXEME) 
	{
		get_context_var_info (agnt, &g, &attr, &value);

		if ((!attr) || (!value))
		{
			return false;
		}

		if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
		{
			return false;
		}

		*result_id = value;
		return true;
	}

	return false;
}

void get_lexeme_from_string (agent* agnt, const char * the_lexeme)
{
	int i;
	const char * c;
	bool sym_constant_start_found = FALSE;
	bool sym_constant_end_found = FALSE;

	for (c = the_lexeme, i = 0; *c; c++, i++)
	{
		if (*c == '|')
		{
			if (!sym_constant_start_found)
			{
				i--;
				sym_constant_start_found = TRUE;
			}
			else
			{
				i--;
				sym_constant_end_found = TRUE;
			}
		}
		else
		{
			agnt->lexeme.string[i] = *c;
		} 
	}

	agnt->lexeme.string[i] = '\0'; /* Null terminate lexeme string */

	agnt->lexeme.length = i;

	if (sym_constant_end_found)
	{
		agnt->lexeme.type = SYM_CONSTANT_LEXEME;
	}
	else 
	{
		determine_type_of_constituent_string(agnt);
	}
}

void get_context_var_info ( agent* agnt, Symbol **dest_goal,
	Symbol **dest_attr_of_slot,
	Symbol **dest_current_value) 
{
	Symbol *v, *g;
	int levels_up;
	wme *w;

	v = find_variable (agnt, agnt->lexeme.string);
	if (v==agnt->s_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->o_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->ss_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->so_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->sss_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->sso_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->ts_context_variable) {
		levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->to_context_variable) {
		levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else {
		*dest_goal = NIL;
		*dest_attr_of_slot = NIL;
		*dest_current_value = NIL;
		return;
	}

	g = agnt->bottom_goal;
	while (g && levels_up) {
		g = g->id.higher_goal;
		levels_up--;
	}
	*dest_goal = g;

	if (!g) {
		*dest_current_value = NIL;
		return;
	}

	if (*dest_attr_of_slot==agnt->state_symbol) {
		*dest_current_value = g;
	} else {
		w = g->id.operator_slot->wmes;
		*dest_current_value = w ? w->value : NIL;
	}
}

Symbol *read_identifier_or_context_variable (agent* agnt) 
{
	Symbol *id;
	Symbol *g, *attr, *value;

	if (agnt->lexeme.type==IDENTIFIER_LEXEME) {
		id = find_identifier (agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
		if (!id) {
			print (agnt, "There is no identifier %c%lu.\n", agnt->lexeme.id_letter,
				agnt->lexeme.id_number);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		return id;
	}
	if (agnt->lexeme.type==VARIABLE_LEXEME) 
	{
		get_context_var_info (agnt, &g, &attr, &value);
		if (!attr) {
			print (agnt, "Expected identifier (or context variable)\n");
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		if (!value) {
			print (agnt, "There is no current %s.\n", agnt->lexeme.string);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
			print (agnt, "The current %s ", agnt->lexeme.string);
			print_with_symbols (agnt, "(%y) is not an identifier.\n", value);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		return value;
	}
	print (agnt, "Expected identifier (or context variable)\n");
	print_location_of_most_recent_lexeme(agnt);
	return NIL;
}		

