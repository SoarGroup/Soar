#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.cpp
 *
 * =======================================================================
 * Description  :  Useful functions for Soar modules
 * =======================================================================
 */

#include "soar_module.h"

#include "gdatastructs.h"

#include "instantiations.h"
#include "tempmem.h"
#include "prefmem.h"
#include "mem.h"
#include "print.h"
#include "decide.h"
#include "xml.h"
#include "wmem.h"
#include "agent.h"
#include "soar_TraceNames.h"

wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, Bool acceptable);
typedef struct agent_struct agent;

namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value )
	{
		slot *my_slot = make_slot( my_agent, id, attr );
		wme *w = make_wme( my_agent, id, attr, value, false );

		insert_at_head_of_dll( my_slot->wmes, w, next, prev );
		add_wme_to_wm( my_agent, w );

		return w;
	}

	void remove_module_wme( agent *my_agent, wme *w )
	{
		slot *my_slot = find_slot( w->id, w->attr );

		if ( my_slot )
		{
			remove_from_dll( my_slot->wmes, w, next, prev );

			if ( my_agent->operand2_mode )
			{
				if ( w->gds ) 
				{
					if ( w->gds->goal != NIL )
					{	             
						if ( my_agent->soar_verbose_flag || my_agent->sysparams[TRACE_WM_CHANGES_SYSPARAM] )
						{
							char buf[256];
							SNPRINTF( buf, 254, "remove_module_wme: Removing state S%d because element in GDS changed.", w->gds->goal->id.level );
							print( my_agent, buf );
							print( my_agent, " WME: " ); 
							
							xml_begin_tag( my_agent, soar_TraceNames::kTagVerbose );
							xml_att_val( my_agent, soar_TraceNames::kTypeString, buf );
							print_wme( my_agent, w );
							xml_end_tag( my_agent, soar_TraceNames::kTagVerbose );
						}
						
						gds_invalid_so_remove_goal( my_agent, w );
						
						/* NOTE: the call to remove_wme_from_wm will take care of checking if GDS should be removed */
					}
				}
			}

			remove_wme_from_wm( my_agent, w );
		}
	}

	preference *make_fake_preference( agent *my_agent, Symbol *state, wme *w, wme_set *conditions )
	{
		// if we are on the top state, don't make the preference
		if ( state == my_agent->top_state )
			return NIL;

		// make fake preference
		preference *pref = make_preference( my_agent, ACCEPTABLE_PREFERENCE_TYPE, w->id, w->attr, w->value, NIL );
		pref->o_supported = TRUE;
		symbol_add_ref( pref->id );
		symbol_add_ref( pref->attr );
		symbol_add_ref( pref->value );

		// make fake instantiation
		instantiation *inst;
		allocate_with_pool( my_agent, &( my_agent->instantiation_pool ), &inst );
		pref->inst = inst;
		pref->inst_next = pref->inst_prev = NULL;
		inst->preferences_generated = pref;
		inst->prod = NULL;
		inst->next = inst->prev = NULL;
		inst->rete_token = NULL;
		inst->rete_wme = NULL;
		inst->match_goal = state;
		inst->match_goal_level = state->id.level;
		inst->okay_to_variablize = TRUE;
		inst->backtrace_number = 0;
		inst->in_ms = FALSE;
		
		condition *cond = NULL;
		condition *prev_cond = NULL;	
		{
			wme_set::iterator p = conditions->begin();

			while ( p != conditions->end() )
			{
				// construct the condition
				allocate_with_pool( my_agent, &( my_agent->condition_pool ), &cond );
				cond->type = POSITIVE_CONDITION;
				cond->prev = prev_cond;
				cond->next = NULL;
				if ( prev_cond != NULL )
				{
					prev_cond->next = cond;
				}
				else
				{
					inst->top_of_instantiated_conditions = cond;
					inst->bottom_of_instantiated_conditions = cond;
					inst->nots = NULL;
				}
				cond->data.tests.id_test = make_equality_test( (*p)->id );
				cond->data.tests.attr_test = make_equality_test( (*p)->attr );
				cond->data.tests.value_test = make_equality_test( (*p)->value );
				cond->test_for_acceptable_preference = TRUE;
				cond->bt.wme_ = (*p);
				wme_add_ref( (*p) );
				cond->bt.level = (*p)->id->id.level;
				cond->bt.trace = (*p)->preference;
				if ( cond->bt.trace )
					preference_add_ref( cond->bt.trace );
				cond->bt.prohibits = NULL;

				prev_cond = cond;

				p++;
			}
		}

		// add the preference to preference/temporary memory
		add_preference_to_tm( my_agent, pref );

		return pref;
	}
}

