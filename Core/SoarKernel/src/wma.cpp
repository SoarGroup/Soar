#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  wma.cpp
 *
 * =======================================================================
 * Description  :  Various functions for WMA
 * =======================================================================
 */

#include "wma.h"

#include <set>
#include <cmath>
#include <cstdlib>

#include "wmem.h"
#include "instantiations.h"
#include "explain.h"
#include "rete.h"
#include "decide.h"
#include "prefmem.h"

#include "misc.h"

#include "print.h"

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				wma::param
// stats 						wma::stats
//
// initialization				wma::init
//
// decay						wma::decay
// forgetting					wma::forget
// update						wma::update
//
// api							wma::api


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (wma::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

wma_activation_param::wma_activation_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent ): soar_module::boolean_param( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {};

void wma_activation_param::set_value( soar_module::boolean new_value ) 
{ 
	if ( new_value != value )
	{
		value = new_value;

		if ( new_value == soar_module::on )
		{
			wma_init( my_agent );
		}
		else
		{
			wma_deinit( my_agent );
		}
	}
};

//

wma_decay_param::wma_decay_param( const char *new_name, double new_value, soar_module::predicate<double> *new_val_pred, soar_module::predicate<double> *new_prot_pred ): soar_module::decimal_param( new_name, new_value, new_val_pred, new_prot_pred ) {};

void wma_decay_param::set_value( double new_value ) { value = -new_value; };

//

template <typename T>
wma_activation_predicate<T>::wma_activation_predicate( agent *new_agent ): soar_module::agent_predicate<T>( new_agent ) {};

template <typename T>
bool wma_activation_predicate<T>::operator() ( T /*val*/ ) { return wma_enabled( this->my_agent ); };

//

wma_param_container::wma_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
	// WMA on/off
	activation = new wma_activation_param( "activation", soar_module::off, new soar_module::f_predicate<soar_module::boolean>(), new_agent );
	add( activation );

	// decay-rate
	decay_rate = new wma_decay_param( "decay-rate", -0.8, new soar_module::btw_predicate<double>( 0, 1, true ), new wma_activation_predicate<double>( my_agent ) );
	add( decay_rate );
	
	// are WMEs removed from WM when activation gets too low?
	forgetting = new soar_module::boolean_param( "forgetting", soar_module::off, new wma_activation_predicate<soar_module::boolean>( my_agent ) );
	add( forgetting );
};

//

bool wma_enabled( agent *my_agent )
{
	return ( my_agent->wma_params->activation->get_value() == soar_module::on );
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (wma::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

wma_stat_container::wma_stat_container( agent *new_agent ): soar_module::stat_container( new_agent )
{
	// update-error
	dummy = new soar_module::integer_stat( "dummy", 0, new soar_module::f_predicate<int64_t>() );
	add( dummy );
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization Functions (wma::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void wma_init( agent *my_agent )
{
	if ( my_agent->wma_initialized )
	{
		return;
	}

	// Pre-compute the integer powers of the decay exponent in order to avoid
	// repeated calls to pow() at runtime
	{
		double decay_rate = my_agent->wma_params->decay_rate->get_value();
		
		for( int i=0; i<WMA_POWER_SIZE; i++ )
		{
			my_agent->wma_power_array[ i ] = pow( static_cast<double>( i + 1 ), decay_rate );
		}
	}

	// note initialization
	my_agent->wma_initialized = true;
}

void wma_deinit( agent *my_agent )
{
	if ( !my_agent->wma_initialized )
	{
		return;
	}

	// clear touched
	my_agent->wma_touched_elements->clear();

	// clear forgetting priority queue
	my_agent->wma_forget_pq->clear();

	my_agent->wma_initialized = false;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Decay Functions (wma::decay)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline unsigned int wma_history_next( unsigned int current )
{
	return ( ( current == ( WMA_DECAY_HISTORY - 1 ) )?( 0 ):( current + 1 ) );
}

inline unsigned int wma_history_prev( unsigned int current )
{
	return ( ( current == 0 )?( WMA_DECAY_HISTORY - 1 ):( current - 1 ) );
}

inline bool wma_should_have_decay_element( wme* w )
{
	return ( ( w->preference ) && ( w->preference->reference_count ) && ( w->preference->o_supported ) );
}

inline double wma_calculate_decay_activation( agent* my_agent, wma_decay_element* decay_el, wma_d_cycle current_cycle )
{
	wma_history* history = &( decay_el->touches );
		
	if ( history->history_ct )
	{
		double return_val = 0.0;

		double* powers = my_agent->wma_power_array;		

		unsigned int p = history->next_p;
		unsigned int counter = history->history_ct;
		wma_d_cycle cycle_diff;
		bool did_something = false;

		while ( counter )
		{
			p = wma_history_prev( p );

			cycle_diff = ( current_cycle - history->access_history[ p ].d_cycle );
			if ( cycle_diff < WMA_POWER_SIZE )
			{
				return_val += ( history->access_history[ p ].num_references * powers[ cycle_diff ] );
				did_something = true;
			}			
			
			counter--;
		}

		if ( did_something )
		{
			return_val = log( return_val );
		}
		else
		{
			return_val = WMA_ACTIVATION_CUTOFF;
		}

		return return_val;
	}
	else
	{
		return WMA_ACTIVATION_NONE;
	}
}

inline wma_reference wma_calculate_initial_boost( agent* my_agent, wme* w )
{
	wma_reference return_val = 0;
	condition *cond;
	wme *cond_wme;
	wma_wme_set::iterator wme_p;
	
	tc_number tc = ( my_agent->wma_tc_counter++ );

	uint64_t num_cond_wmes = 0;
	double combined_activation = 0.0;

	for ( cond=w->preference->inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next )
	{
		if ( ( cond->type == POSITIVE_CONDITION ) && ( cond->bt.wme_->wma_tc_value != tc ) )
		{
			cond_wme = cond->bt.wme_;
			cond_wme->wma_tc_value = tc;

			if ( cond_wme->wma_decay_el )
			{
				if ( !cond_wme->wma_decay_el->just_created )
				{
					num_cond_wmes++;
					combined_activation += wma_get_wme_activation( my_agent, cond_wme );
				}
			}
			else if ( cond_wme->preference )
			{
				if ( cond_wme->preference->wma_o_set )
				{
					for ( wme_p=cond_wme->preference->wma_o_set->begin(); wme_p!=cond_wme->preference->wma_o_set->end(); wme_p++ )
					{
						if ( ( (*wme_p)->wma_tc_value != tc ) && ( !(*wme_p)->wma_decay_el || !(*wme_p)->wma_decay_el->just_created ) )
						{
							num_cond_wmes++;
							combined_activation += wma_get_wme_activation( my_agent, (*wme_p) );

							(*wme_p)->wma_tc_value = tc;
						}
					}
				}
			}
			else
			{
				num_cond_wmes++;
				combined_activation += wma_get_wme_activation( my_agent, cond_wme );
			}
		}		
	}

	if ( num_cond_wmes )
	{
		return_val = static_cast<wma_reference>( floor( exp( combined_activation / num_cond_wmes ) ) );
	}

	return return_val;
}

void wma_activate_wme( agent* my_agent, wme* w, wma_reference num_references, wma_wme_set* o_set )
{	
	// o-supported, non-architectural WME
	if ( wma_should_have_decay_element( w ) )
	{
		wma_decay_element* temp_el = w->wma_decay_el;

		// if decay structure doesn't exist, create it
		if ( !temp_el )
		{
			allocate_with_pool( my_agent, &( my_agent->wma_decay_element_pool ), &temp_el );		
			
			temp_el->this_wme = w;			
			temp_el->just_removed = false;			
			
			temp_el->just_created = true;
			temp_el->num_references = wma_calculate_initial_boost( my_agent, w );
			
			temp_el->touches.history_ct = 0;
			temp_el->touches.next_p = 0;

			w->wma_decay_el = temp_el;
		}

		// add to o_set if necessary
		if ( o_set )
		{
			o_set->insert( w );
		}
		// otherwise update the decay element
		else
		{
			temp_el->num_references += num_references;
			my_agent->wma_touched_elements->insert( w );
		}
	}
	// i-supported, non-architectural WME
	else if ( ( w->preference ) && ( w->preference->reference_count ) )
	{		
		wma_wme_set* my_o_set = w->preference->wma_o_set;
		wma_wme_set::iterator wme_p;

		// if doesn't have an o_set, populate
		if ( !my_o_set )
		{
			my_o_set = new wma_wme_set;
			w->preference->wma_o_set = my_o_set;

			for ( condition* c=w->preference->inst->top_of_instantiated_conditions; c; c=c->next )
			{
				if ( c->type == POSITIVE_CONDITION )
				{
					wma_activate_wme( my_agent, c->bt.wme_, 0, my_o_set );
				}
			}

			for ( wme_p=my_o_set->begin(); wme_p!=my_o_set->end(); wme_p++ )
			{
				// add a ref to wmes on this list
				wme_add_ref( (*wme_p) );
			}
		}	

		// iterate over the o_set
		for ( wme_p=my_o_set->begin(); wme_p!=my_o_set->end(); wme_p++ )
		{
			// if populating o_set, add
			if ( o_set )
			{
				o_set->insert( (*wme_p) );
			}
			// otherwise, "activate" the wme if it is
			// non-architectural (avoids dereferencing
			// the wme preference)
			else
			{
				if ( (*wme_p)->wma_decay_el )
				{
					(*wme_p)->wma_decay_el->num_references += num_references;
					my_agent->wma_touched_elements->insert( (*wme_p) );
				}
			}
		}
	}
	// architectural
	else if ( !w->preference )
	{
		// only action is to add it to the o_set
		if ( o_set )
		{
			o_set->insert( w );
		}
	}
}

inline void wma_forgetting_remove_from_p_queue( agent* my_agent, wma_decay_element* decay_el );
void wma_deactivate_element( agent* my_agent, wme* w )
{
	wma_decay_element* temp_el = w->wma_decay_el;

	if ( temp_el )
	{	
		if ( !temp_el->just_removed )
		{			
			my_agent->wma_touched_elements->erase( w );

			if ( my_agent->wma_params->forgetting->get_value() == soar_module::on )
			{
				wma_forgetting_remove_from_p_queue( my_agent, temp_el );
			}

			temp_el->just_removed = true;
		}
	}
}

void wma_remove_decay_element( agent* my_agent, wme* w )
{
	wma_decay_element* temp_el = w->wma_decay_el;
	
	if ( temp_el )
	{
		// Deactivate the wme first
		if ( !temp_el->just_removed )
		{
			wma_deactivate_element( my_agent, w );
		}		

		free_with_pool( &( my_agent->wma_decay_element_pool ), temp_el );
		w->wma_decay_el = NULL;
	}
}

void wma_remove_pref_o_set( agent* my_agent, preference* pref )
{
	if ( pref && pref->wma_o_set )
	{
		wma_wme_set* victim = pref->wma_o_set;
		pref->wma_o_set = NULL;
		
		for ( wma_wme_set::iterator p=victim->begin(); p!=victim->end(); p++ )
		{
			wme_remove_ref( my_agent, (*p) );
		}

		delete victim;
	}
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Forgetting Functions (wma::forget)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline void wma_forgetting_add_to_p_queue( agent* my_agent, wma_decay_element* decay_el, wma_d_cycle new_cycle )
{
	if ( decay_el )
	{
		decay_el->forget_cycle = new_cycle;
		(*my_agent->wma_forget_pq)[ new_cycle ].insert( decay_el );
	}
}

inline void wma_forgetting_remove_from_p_queue( agent* my_agent, wma_decay_element* decay_el )
{
	if ( decay_el )
	{
		// try to find set for the element per cycle		
		wma_forget_p_queue::iterator pq_p = my_agent->wma_forget_pq->find( decay_el->forget_cycle );
		if ( pq_p != my_agent->wma_forget_pq->end() )
		{
			wma_decay_set::iterator d_p = pq_p->second.find( decay_el );
			if ( d_p != pq_p->second.end() )
			{
				pq_p->second.erase( d_p );

				if ( pq_p->second.empty() )
				{
					my_agent->wma_forget_pq->erase( pq_p );
				}
			}
		}
	}
}

inline void wma_forgetting_move_in_p_queue( agent* my_agent, wma_decay_element* decay_el, wma_d_cycle new_cycle )
{
	if ( decay_el && ( decay_el->forget_cycle != new_cycle ) )
	{
		wma_forgetting_remove_from_p_queue( my_agent, decay_el );
		wma_forgetting_add_to_p_queue( my_agent, decay_el, new_cycle );
	}
}

// naive algorithm:
// - pretend you get no further updates, calculate how long you'd last
inline wma_d_cycle wma_forgetting_estimate_cycle( agent* my_agent, wma_decay_element* decay_el )
{	
	wma_d_cycle return_val = static_cast<wma_d_cycle>( my_agent->d_cycle_count );
	double predicted_activation;
	
	do
	{
		
		predicted_activation = wma_calculate_decay_activation( my_agent, decay_el, ++return_val );

	} while ( predicted_activation > WMA_ACTIVATION_CUTOFF );
	
	return return_val;	
}

inline bool wma_forgetting_forget_wme( agent *my_agent, wme *w )
{	
	bool return_val = false;
	
	if ( w->preference )
	{
		preference* p = w->preference->slot->all_preferences;
		preference* next_p;

		while ( p )
		{
			next_p = p->all_of_slot_next;

			if ( p->o_supported && p->in_tm && ( p->value == w->value ) )
			{
				remove_preference_from_tm( my_agent, p );
				return_val = true;				
			}

			p = next_p;
		}
	}

	return return_val;
}

inline bool wma_forgetting_update_p_queue( agent* my_agent )
{
	bool return_val = false;
	
	if ( !my_agent->wma_forget_pq->empty() )
	{
		wma_forget_p_queue::iterator pq_p = my_agent->wma_forget_pq->begin();
		wma_d_cycle current_cycle = my_agent->d_cycle_count;

		if ( pq_p->first == current_cycle )
		{
			for ( wma_decay_set::iterator d_p=pq_p->second.begin(); d_p!=pq_p->second.end(); d_p++ )
			{
				if ( wma_calculate_decay_activation( my_agent, (*d_p), current_cycle ) <= WMA_ACTIVATION_CUTOFF )
				{
					if ( wma_forgetting_forget_wme( my_agent, (*d_p)->this_wme ) )
					{
						return_val = true;
					}
				}
				else
				{
					wma_forgetting_move_in_p_queue( my_agent, (*d_p), wma_forgetting_estimate_cycle( my_agent, (*d_p) ) );
				}
			}

			my_agent->wma_forget_pq->erase( pq_p );
		}
	}

	return return_val;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Activation Update Functions (wma::update)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void wma_activate_wmes_in_pref( agent* my_agent, preference* pref )
{
	wme* w;
	
	if ( pref->type == ACCEPTABLE_PREFERENCE_TYPE )
	{
		w = pref->slot->wmes;
		while ( w )
		{
			// id and attr should already match so just compare the value
			if ( w->value == pref->value )
			{
				wma_activate_wme( my_agent, w );
			}

			w = w->next;
		}
	}
}

void wma_activate_wmes_tested_in_prods( agent* my_agent )
{
	ms_change *msc;
	token temp_token, *t;

	for ( msc=my_agent->ms_o_assertions; msc!=NIL; msc=msc->next )
	{
		temp_token.parent = msc->tok;
		temp_token.w = msc->w;
		t = &temp_token;

		while ( t != my_agent->dummy_top_token )
		{
			if (t->w != NIL)
			{
				wma_activate_wme( my_agent, t->w );				
			}

			t = t->parent;
		}
	}

	for ( msc=my_agent->ms_i_assertions; msc!=NIL; msc=msc->next )
	{
		temp_token.parent = msc->tok;
		temp_token.w = msc->w;
		t = &temp_token;

		while ( t != my_agent->dummy_top_token )
		{
			if ( t->w != NIL )
			{
				wma_activate_wme( my_agent, t->w );
			}

			t = t->parent;
		}
	}
}

inline void wma_update_decay_histories( agent* my_agent )
{
	wma_wme_set::iterator wme_p;
	wma_decay_element* temp_el;
	wma_d_cycle current_cycle = my_agent->d_cycle_count;
	bool forgetting = ( my_agent->wma_params->forgetting->get_value() == soar_module::on );

	// add to history for changed elements
	for ( wme_p=my_agent->wma_touched_elements->begin(); wme_p!=my_agent->wma_touched_elements->end(); wme_p++ )
	{
		temp_el = (*wme_p)->wma_decay_el;			
		
		// set history
		temp_el->touches.access_history[ temp_el->touches.next_p ].d_cycle = current_cycle;
		temp_el->touches.access_history[ temp_el->touches.next_p ].num_references = temp_el->num_references;
		
		// update counters
		if ( temp_el->touches.history_ct < WMA_DECAY_HISTORY )
		{
			temp_el->touches.history_ct++;
		}
		temp_el->touches.next_p = wma_history_next( temp_el->touches.next_p );

		// reset cycle counter
		temp_el->num_references = 0;

		// update forgetting stuff as needed
		if ( forgetting )
		{
			if ( temp_el->just_created )
			{
				wma_forgetting_add_to_p_queue( my_agent, temp_el, wma_forgetting_estimate_cycle( my_agent, temp_el ) );
			}
			else
			{
				wma_forgetting_move_in_p_queue( my_agent, temp_el, wma_forgetting_estimate_cycle( my_agent, temp_el ) );
			}
		}

		temp_el->just_created = false;
	}
	my_agent->wma_touched_elements->clear();
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Functions (wma::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

double wma_get_wme_activation( agent* my_agent, wme* w )
{
	double return_val = static_cast<double>( WMA_ACTIVATION_NONE );

	if ( w->wma_decay_el )
	{
		return_val = wma_calculate_decay_activation( my_agent, w->wma_decay_el, my_agent->d_cycle_count );
	}

	return return_val;
}

void wma_go( agent* my_agent )
{
	// update history for all touched elements
	wma_update_decay_histories( my_agent );			

	// check forgetting queue
	if ( my_agent->wma_params->forgetting->get_value() == soar_module::on )
	{
		if ( wma_forgetting_update_p_queue( my_agent ) )
		{
			if ( my_agent->sysparams[ TRACE_WM_CHANGES_SYSPARAM ] )
			{
				const char *msg = "\n\nWMA: BEGIN FORGOTTEN WME LIST\n\n";
				
				print( my_agent, const_cast<char *>( msg ) );
				xml_generate_message( my_agent, const_cast<char *>( msg ) );
			}

			do_working_memory_phase( my_agent );

			if ( my_agent->sysparams[ TRACE_WM_CHANGES_SYSPARAM ] )
			{
				const char *msg = "\nWMA: END FORGOTTEN WME LIST\n\n";
				
				print( my_agent, const_cast<char *>( msg ) );
				xml_generate_message( my_agent, const_cast<char *>( msg ) );
			}
		}
	}
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
