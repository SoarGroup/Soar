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

#include "agent.h"
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
#include "wma.h"

wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, char metadata);
typedef struct agent_struct agent;

namespace soar_module
{
	timer::timer( const char *new_name, agent *new_agent, timer_level new_level, predicate<timer_level> *new_pred, bool soar_control ): named_object( new_name ), my_agent( new_agent ), level( new_level ), pred( new_pred )
	{
		stopwatch.set_enabled( ( ( soar_control )?( &( new_agent->sysparams[ TIMERS_ENABLED ] ) ):( NULL ) ) );
		reset();
	}
	
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value )
	{
		slot *my_slot = make_slot( my_agent, id, attr );
		wme *w = make_wme( my_agent, id, attr, value, FALSE);

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

			if ( w->gds ) 
			{
				if ( w->gds->goal != NIL )
				{	             
					gds_invalid_so_remove_goal( my_agent, w );
					
					/* NOTE: the call to remove_wme_from_wm will take care of checking if GDS should be removed */
				}
			}

			remove_wme_from_wm( my_agent, w );
		}
	}

	instantiation* make_fake_instantiation( agent* my_agent, Symbol* state, wme_set* conditions, symbol_triple_list* actions )
	{
		// make fake instantiation
		instantiation* inst;
		allocate_with_pool( my_agent, &( my_agent->instantiation_pool ), &inst );
		inst->prod = NULL;
		inst->next = inst->prev = NULL;
		inst->rete_token = NULL;
		inst->rete_wme = NULL;
		inst->match_goal = state;
		inst->match_goal_level = state->id.level;
		inst->reliable = true;
		inst->backtrace_number = 0;
		inst->in_ms = FALSE;
		inst->GDS_evaluated_already = FALSE;

		// create preferences
		inst->preferences_generated = NULL;
		{
			preference* pref;

			for ( symbol_triple_list::iterator a_it=actions->begin(); a_it!=actions->end(); a_it++ )
			{
				pref = make_preference( my_agent, ACCEPTABLE_PREFERENCE_TYPE, (*a_it)->id, (*a_it)->attr, (*a_it)->value, NIL );
				pref->o_supported = true;
				symbol_add_ref( pref->id );
				symbol_add_ref( pref->attr );
				symbol_add_ref( pref->value );

				pref->inst = inst;
				pref->inst_next = pref->inst_prev = NULL;

				insert_at_head_of_dll( inst->preferences_generated, pref, inst_next, inst_prev );
			}
		}

		// create conditions
		{
			condition *cond = NULL;
			condition *prev_cond = NULL;

			for ( wme_set::iterator c_it=conditions->begin(); c_it!=conditions->end(); c_it++ )
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
				cond->data.tests.id_test = make_equality_test( (*c_it)->id );
				cond->data.tests.attr_test = make_equality_test( (*c_it)->attr );
				cond->data.tests.value_test = make_equality_test( (*c_it)->value );
				cond->metadata_test.mask = 0xff;
				cond->metadata_test.value = (*c_it)->metadata;
				cond->bt.wme_ = (*c_it);

				#ifndef DO_TOP_LEVEL_REF_CTS
				if ( inst->match_goal_level > TOP_GOAL_LEVEL )
				#endif
				{
					wme_add_ref( (*c_it) );
				}			
				
				cond->bt.level = (*c_it)->id->id.level;
				cond->bt.trace = (*c_it)->preference;
				
				if ( cond->bt.trace )
				{
					#ifndef DO_TOP_LEVEL_REF_CTS
					if ( inst->match_goal_level > TOP_GOAL_LEVEL )
					#endif
					{
						preference_add_ref( cond->bt.trace );
					}
				}				

				cond->bt.prohibits = NULL;

				prev_cond = cond;
			}
		}

		return inst;
	}


	/////////////////////////////////////////////////////////////
	// Memory Pool Allocators
	/////////////////////////////////////////////////////////////

	memory_pool* get_memory_pool( agent* my_agent, size_t size )
	{
		memory_pool* return_val = NULL;

		std::map< size_t, memory_pool* >::iterator it = my_agent->dyn_memory_pools->find( size );
		if ( it == my_agent->dyn_memory_pools->end() )
		{
			memory_pool* newbie = new memory_pool;

			init_memory_pool( my_agent, newbie, size, "dynamic" );
			my_agent->dyn_memory_pools->insert( std::make_pair< size_t, memory_pool* >( size, newbie ) );

			return_val = newbie;
		}
		else
		{
			return_val = it->second;
		}

		return return_val;
	}
}

