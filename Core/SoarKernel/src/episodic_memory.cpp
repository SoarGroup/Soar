#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  episodic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-EpMem
 * =======================================================================
 */

#include "episodic_memory.h"
#include "instantiations.h"
#include "io_soar.h"
#include "misc.h"
#include "prefmem.h"
#include "print.h"
#include "utilities.h"
#include "wmem.h"
#include "xml.h"

#include <string>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>

#include <assert.h>

#include "sqlite3.h"

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				epmem::param
// stats 						epmem::stats
// timers 						epmem::timers

// wme-related					epmem::wmes

// sqlite query					epmem::query
// sqlite transactions			epmem::transaction
// variable abstraction			epmem::var

// relational interval tree		epmem::rit

// cleaning up					epmem::clean
// initialization				epmem::init

// temporal hash				epmem::hash

// storing new episodes			epmem::storage
// non-cue-based queries		epmem::ncb
// cue-based queries			epmem::cbr

// high-level api				epmem::api


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// shared SQL to perform cue-based queries
// on true ranges, "now" ranges (i.e. wme in WM), and
// "point" ranges (in/out in sequential episodes)
const char *epmem_range_queries[2][2][3] =
{
	{
		{
			"SELECT e.start AS start FROM node_range e WHERE e.id=? ORDER BY e.start DESC",
			"SELECT e.start AS start FROM node_now e WHERE e.id=? ORDER BY e.start DESC",
			"SELECT e.start AS start FROM node_point e WHERE e.id=? ORDER BY e.start DESC"
		},
		{
			"SELECT e.end AS end FROM node_range e WHERE e.id=? ORDER BY e.end DESC",
			"SELECT ? AS end FROM node_now e WHERE e.id=?",
			"SELECT e.start AS end FROM node_point e WHERE e.id=? ORDER BY e.start DESC"
		}
	},
	{
		{
			"SELECT e.start AS start FROM edge_range e WHERE e.id=? ORDER BY e.start DESC",
			"SELECT e.start AS start FROM edge_now e WHERE e.id=? ORDER BY e.start DESC",
			"SELECT e.start AS start FROM edge_point e WHERE e.id=? ORDER BY e.start DESC"
		},
		{
			"SELECT e.end AS end FROM edge_range e WHERE e.id=? ORDER BY e.end DESC",
			"SELECT ? AS end FROM edge_now e WHERE e.id=?",
			"SELECT e.start AS end FROM edge_point e WHERE e.id=? ORDER BY e.start DESC"
		}
	},
};

// these constants hold pointers to all information
// necessary to share RIT code amongst any tables
// implementing the RIT spec (node, start, end, id)
//
// array locations are indicated by the
// EPMEM_RIT_STATE_OFFSET - EPMEM_RIT_STATE_TIMER
// predefines
epmem_rit_state epmem_rit_state_tree;

// first array adheres to the EPMEM_RIT_STATE_NODE/EDGE
// predefines
epmem_rit_state epmem_rit_state_graph[2];


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (epmem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_param_container::epmem_param_container( agent *new_agent ): param_container( new_agent )
{
	// learning
	learning = new boolean_param( "learning", on, new f_predicate<boolean>() );
	add( learning );

	// database
	database = new constant_param<db_choices>( "database", memory, new epmem_db_predicate<db_choices>( my_agent ) );
	database->add_mapping( memory, "memory" );
	database->add_mapping( file, "file" );
	add( database );

	// path
	path = new epmem_path_param( "path", "", new predicate<const char *>(), new epmem_db_predicate<const char *>( my_agent ), my_agent );
	add( path );

	// commit
	commit = new integer_param( "commit", 1, new gt_predicate<long>( 1, true ), new f_predicate<long>() );
	add( commit );

	// mode
	mode = new epmem_mode_param( "mode", graph, new epmem_db_predicate<mode_choices>( my_agent ), my_agent );
	mode->add_mapping( tree, "tree" );
	mode->add_mapping( graph, "graph" );
	add( mode );

	// graph-match
	graph_match = new epmem_graph_match_param( "graph-match", on, new f_predicate<boolean>(), my_agent );
	add( graph_match );

	// phase
	phase = new constant_param<phase_choices>( "phase", phase_output, new f_predicate<phase_choices>() );
	phase->add_mapping( phase_output, "output" );
	phase->add_mapping( phase_selection, "selection" );
	add( phase );

	// trigger
	trigger = new constant_param<trigger_choices>( "trigger", output, new f_predicate<trigger_choices>() );
	trigger->add_mapping( none, "none" );
	trigger->add_mapping( output, "output" );
	trigger->add_mapping( dc, "dc" );
	add( trigger );

	// force
	force = new constant_param<force_choices>( "force", force_off, new f_predicate<force_choices>() );
	force->add_mapping( remember, "remember" );
	force->add_mapping( ignore, "ignore" );
	force->add_mapping( force_off, "off" );
	add( force );

	// balance
	balance = new decimal_param( "balance", 0.5, new btw_predicate<double>( 0, 1, true ), new f_predicate<double>() );
	add( balance );

	// exclusions - this is initialized with "epmem" directly after hash tables
	exclusions = new set_param( "exclusions", new f_predicate<const char *>, my_agent );
	add( exclusions );

	// timers
	timers = new constant_param<soar_module::timer::timer_level>( "timers", soar_module::timer::zero, new f_predicate<soar_module::timer::timer_level>() );
	timers->add_mapping( soar_module::timer::zero, "off" );
	timers->add_mapping( soar_module::timer::one, "one" );
	timers->add_mapping( soar_module::timer::two, "two" );
	timers->add_mapping( soar_module::timer::three, "three" );
	add( timers );
};

//

epmem_path_param::epmem_path_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred, agent *new_agent ): string_param( new_name, new_value, new_val_pred, new_prot_pred ), my_agent( new_agent ) {};

void epmem_path_param::set_value( const char *new_value )
{
	if ( my_agent->epmem_first_switch )
	{
		my_agent->epmem_first_switch = false;
		my_agent->epmem_params->database->set_value( epmem_param_container::file );

		const char *msg = "Database set to file";
		print( my_agent, const_cast<char *>( msg ) );
		xml_generate_message( my_agent, const_cast<char *>( msg ) );
	}

	value->assign( new_value );
}

//

epmem_graph_match_param::epmem_graph_match_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred, agent *new_agent ): boolean_param( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {};

bool epmem_graph_match_param::validate_string( const char *new_string )
{
	bool return_val = false;

	std::map<std::string, soar_module::boolean>::iterator p;
	std::string temp_str( new_string );

	p = string_to_value->find( temp_str );

	if ( p != string_to_value->end() )
	{
		return_val = ( ( p->second == soar_module::off ) || ( my_agent->epmem_params->mode->get_value() == epmem_param_container::graph ) );
	}

	return return_val;
}

//

epmem_mode_param::epmem_mode_param( const char *new_name, epmem_param_container::mode_choices new_value, predicate<epmem_param_container::mode_choices> *new_prot_pred, agent *new_agent ): constant_param<epmem_param_container::mode_choices>( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {};

void epmem_mode_param::set_value( epmem_param_container::mode_choices new_value )
{
	if ( new_value != epmem_param_container::graph )
		my_agent->epmem_params->graph_match->set_value( soar_module::off );

	value = new_value;
}

//

template <typename T>
epmem_db_predicate<T>::epmem_db_predicate( agent *new_agent ): agent_predicate<T>( new_agent ) {};

template <typename T>
bool epmem_db_predicate<T>::operator() ( T /*val*/ ) { return ( this->my_agent->epmem_db_status != EPMEM_DB_CLOSED ); };


/***************************************************************************
 * Function     : epmem_enabled
 * Author		: Nate Derbinsky
 * Notes		: Shortcut function to system parameter
 **************************************************************************/
inline bool epmem_enabled( agent *my_agent )
{
	return ( my_agent->epmem_params->learning->get_value() == soar_module::on );
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (epmem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_stat_container::epmem_stat_container( agent *new_agent ): stat_container( new_agent )
{
	// time
	time = new integer_stat( "time", 0, new epmem_db_predicate<long>( my_agent ) );
	add( time );

	// mem-usage
	mem_usage = new epmem_mem_usage_stat( "mem-usage", 0, new predicate<long>() );
	add( mem_usage );

	// mem-high
	mem_high = new epmem_mem_high_stat( "mem-high", 0, new predicate<long>() );
	add( mem_high );

	// ncb-wmes
	ncb_wmes = new integer_stat( "ncb-wmes", 0, new f_predicate<long>() );
	add( ncb_wmes );

	// qry-pos
	qry_pos = new integer_stat( "qry-pos", 0, new f_predicate<long>() );
	add( qry_pos );

	// qry-neg
	qry_neg = new integer_stat( "qry-neg", 0, new f_predicate<long>() );
	add( qry_neg );

	// qry-ret
	qry_ret = new integer_stat( "qry-ret", 0, new f_predicate<long>() );
	add( qry_ret );

	// qry-pos
	qry_card = new integer_stat( "qry-card", 0, new f_predicate<long>() );
	add( qry_card );

	// qry-pos
	qry_lits = new integer_stat( "qry-lits", 0, new f_predicate<long>() );
	add( qry_lits );

	// next-id
	next_id = new integer_stat( "next-id", 0, new epmem_db_predicate<long>( my_agent ) );
	add( next_id );

	// rit-offset-1
	rit_offset_1 = new integer_stat( "rit-offset-1", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_offset_1 );

	// rit-left-root-1
	rit_left_root_1 = new integer_stat( "rit-left-root-1", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_left_root_1 );

	// rit-right-root-1
	rit_right_root_1 = new integer_stat( "rit-right-root-1", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_right_root_1 );

	// rit-min-step-1
	rit_min_step_1 = new integer_stat( "rit-min-step-1", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_min_step_1 );

	// rit-offset-2
	rit_offset_2 = new integer_stat( "rit-offset-2", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_offset_2 );

	// rit-left-root-2
	rit_left_root_2 = new integer_stat( "rit-left-root-2", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_left_root_2 );

	// rit-right-root-2
	rit_right_root_2 = new integer_stat( "rit-right-root-2", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_right_root_2 );

	// rit-min-step-2
	rit_min_step_2 = new integer_stat( "rit-min-step-2", 0, new epmem_db_predicate<long>( my_agent ) );
	add( rit_min_step_2 );


	/////////////////////////////
	// connect to rit state
	/////////////////////////////

	// tree
	epmem_rit_state_tree.add_query = EPMEM_STMT_ONE_ADD_NODE_RANGE;
	epmem_rit_state_tree.timer = my_agent->epmem_timers->ncb_node_rit;
	epmem_rit_state_tree.offset.stat = rit_offset_1;
	epmem_rit_state_tree.offset.var_key = var_rit_offset_1;
	epmem_rit_state_tree.leftroot.stat = rit_left_root_1;
	epmem_rit_state_tree.leftroot.var_key = var_rit_leftroot_1;
	epmem_rit_state_tree.rightroot.stat = rit_right_root_1;
	epmem_rit_state_tree.rightroot.var_key = var_rit_rightroot_1;
	epmem_rit_state_tree.minstep.stat = rit_min_step_1;
	epmem_rit_state_tree.minstep.var_key = var_rit_minstep_1;

	// graph
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].add_query = EPMEM_STMT_THREE_ADD_NODE_RANGE;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].timer = my_agent->epmem_timers->ncb_node_rit;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.stat = rit_offset_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.var_key = var_rit_offset_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.stat = rit_left_root_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.var_key = var_rit_leftroot_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.stat = rit_right_root_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.var_key = var_rit_rightroot_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.stat = rit_min_step_1;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.var_key = var_rit_minstep_1;

	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].add_query = EPMEM_STMT_THREE_ADD_EDGE_RANGE;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].timer = my_agent->epmem_timers->ncb_edge_rit;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.stat = rit_offset_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.var_key = var_rit_offset_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.stat = rit_left_root_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.var_key = var_rit_leftroot_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.stat = rit_right_root_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.var_key = var_rit_rightroot_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.stat = rit_min_step_2;
	epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.var_key = var_rit_minstep_2;
};

//

epmem_mem_usage_stat::epmem_mem_usage_stat( const char *new_name, long new_value, predicate<long> *new_prot_pred ): integer_stat( new_name, new_value, new_prot_pred ) {};

long epmem_mem_usage_stat::get_value()
{
	return (long) sqlite3_memory_used();
};

//

epmem_mem_high_stat::epmem_mem_high_stat( const char *new_name, long new_value, predicate<long> *new_prot_pred ): integer_stat( new_name, new_value, new_prot_pred ) {};

long epmem_mem_high_stat::get_value()
{
	return (long) sqlite3_memory_highwater( false );
};


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (epmem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_timer_container::epmem_timer_container( agent *new_agent ): timer_container( new_agent )
{
	// one
	
	total = new epmem_timer( "_total", my_agent, timer::one );
	add( total );

	// two

	storage = new epmem_timer( "epmem_storage", my_agent, timer::two );
	add( storage );

	ncb_retrieval = new epmem_timer( "epmem_ncb_retrieval", my_agent, timer::two );
	add( ncb_retrieval );

	query = new epmem_timer( "epmem_query", my_agent, timer::two );
	add( query );

	api = new epmem_timer( "epmem_api", my_agent, timer::two );
	add( api );

	trigger = new epmem_timer( "epmem_trigger", my_agent, timer::two );
	add( trigger );

	init = new epmem_timer( "epmem_init", my_agent, timer::two );
	add( init );

	next = new epmem_timer( "epmem_next", my_agent, timer::two );
	add( next );

	prev = new epmem_timer( "epmem_prev", my_agent, timer::two );
	add( prev );

	// three

	ncb_edge = new epmem_timer( "ncb_edge", my_agent, timer::three );
	add( ncb_edge );

	ncb_edge_rit = new epmem_timer( "ncb_edge_rit", my_agent, timer::three );
	add( ncb_edge_rit );

	ncb_node = new epmem_timer( "ncb_node", my_agent, timer::three );
	add( ncb_node );

	ncb_node_rit = new epmem_timer( "ncb_node_rit", my_agent, timer::three );
	add( ncb_node_rit );

	query_dnf = new epmem_timer( "query_dnf", my_agent, timer::three );
	add( query_dnf );

	query_graph_match = new epmem_timer( "query_graph_match", my_agent, timer::three );
	add( query_graph_match );

	query_pos_start_ep = new epmem_timer( "query_pos_start_ep", my_agent, timer::three );
	add( query_pos_start_ep );

	query_pos_start_now = new epmem_timer( "query_pos_start_now", my_agent, timer::three );
	add( query_pos_start_now );

	query_pos_start_point = new epmem_timer( "query_pos_start_point", my_agent, timer::three );
	add( query_pos_start_point );

	query_pos_end_ep = new epmem_timer( "query_pos_end_ep", my_agent, timer::three );
	add( query_pos_end_ep );

	query_pos_end_now = new epmem_timer( "query_pos_end_now", my_agent, timer::three );
	add( query_pos_end_now );

	query_pos_end_point = new epmem_timer( "query_pos_end_point", my_agent, timer::three );
	add( query_pos_end_point );

	query_neg_start_ep = new epmem_timer( "query_neg_start_ep", my_agent, timer::three );
	add( query_neg_start_ep );

	query_neg_start_now = new epmem_timer( "query_neg_start_now", my_agent, timer::three );
	add( query_neg_start_now );

	query_neg_start_point = new epmem_timer( "query_neg_start_point", my_agent, timer::three );
	add( query_neg_start_point );

	query_neg_end_ep = new epmem_timer( "query_neg_end_ep", my_agent, timer::three );
	add( query_neg_end_ep );

	query_neg_end_now = new epmem_timer( "query_neg_end_now", my_agent, timer::three );
	add( query_neg_end_now );

	query_neg_end_point = new epmem_timer( "query_neg_end_point", my_agent, timer::three );
	add( query_neg_end_point );
};

//

epmem_timer_level_predicate::epmem_timer_level_predicate( agent *new_agent ): agent_predicate<soar_module::timer::timer_level>( new_agent ) {};

bool epmem_timer_level_predicate::operator() ( soar_module::timer::timer_level val ) { return ( my_agent->epmem_params->timers->get_value() >= val ); };

//

epmem_timer::epmem_timer(const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level): soar_module::timer( new_name, new_agent, new_level, new epmem_timer_level_predicate( new_agent ) ) {};


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (epmem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_augs_of_id
 * Author		: Andy Nuxoll
 * Notes		: This routine works just like the one defined in utilities.h.
 *				  Except this one does not use C++ templates because I have an
 *				  irrational dislike for them borne from the years when the STL
 *				  highly un-portable.  I'm told this is no longer true but I'm
 *				  still bitter.
 **************************************************************************/
wme **epmem_get_augs_of_id( agent* my_agent, Symbol * id, tc_number tc, unsigned long *num_attr )
{
	slot *s;
	wme *w;
	wme **list;
	unsigned long list_position;
	unsigned long n = 0;

	// augs only exist for identifiers
	if ( id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
		return NULL;

	// don't want to get into a loop
	if ( id->id.tc_num == tc )
		return NULL;
	id->id.tc_num = tc;

	// first count number of augs, required for later allocation
	for ( w = id->id.impasse_wmes; w != NIL; w = w->next )
		n++;
	for ( w = id->id.input_wmes; w != NIL; w = w->next )
		n++;
	for ( s = id->id.slots; s != NIL; s = s->next )
	{
		for ( w = s->wmes; w != NIL; w = w->next )
			n++;
		for ( w = s->acceptable_preference_wmes; w != NIL; w = w->next )
			n++;
	}

	// allocate the list, note the size
	list = static_cast<wme**>(allocate_memory( my_agent, n * sizeof(wme *), MISCELLANEOUS_MEM_USAGE));
	( *num_attr ) = n;

	list_position = 0;
	for ( w = id->id.impasse_wmes; w != NIL; w = w->next )
       list[ list_position++ ] = w;
	for ( w = id->id.input_wmes; w != NIL; w = w->next )
		list[ list_position++ ] = w;
	for ( s = id->id.slots; s != NIL; s = s->next )
	{
		for ( w = s->wmes; w != NIL; w = w->next )
           list[ list_position++ ] = w;
		for ( w = s->acceptable_preference_wmes; w != NIL; w = w->next )
			list[ list_position++ ] = w;
	}

	return list;
}

/***************************************************************************
 * Function     : epmem_wme_has_value
 * Author		: Andy Nuxoll
 * Notes		: This routine returns TRUE if the given WMEs attribute
 *                and value are both symbols and have the names given.
 *                If either of the given names are NULL then they are
 *                assumed to be a match (i.e., a wildcard).  Thus passing
 *                NULL for both attr_name and value_name will always yield
 *                a TRUE result.
 **************************************************************************/
bool epmem_wme_has_value( wme *w, char *attr_name, char *value_name )
{
	if ( w == NULL )
		return false;

	if ( attr_name != NULL )
	{
		if ( w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
			return false;

		if ( strcmp( w->attr->sc.name, attr_name ) != 0 )
			return false;
	}

    if ( value_name != NULL )
	{
		if ( w->value->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
			return false;

		if ( strcmp( w->attr->sc.name, value_name ) != 0 )
			return false;
	}

	return true;
}

/***************************************************************************
 * Function     : epmem_get_aug_of_id
 * Author		: Andy Nuxoll
 * Notes		: This routine examines a symbol for an augmentation that
 *				  has the given attribute and value and returns it.  See
 *                epmem_wme_has_value() for info on how the correct wme is
 *                matched to the given strings.
 **************************************************************************/
wme *epmem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name )
{
	wme **wmes;
	wme *return_val = NULL;

	unsigned long len = 0;
	unsigned long i;

	wmes = epmem_get_augs_of_id( my_agent, sym, get_new_tc_number( my_agent ), &len );
	if ( wmes == NULL )
		return return_val;

	for ( i=0; i<len; i++ )
	{
		if ( epmem_wme_has_value( wmes[ i ], attr_name, value_name ) )
		{
			return_val = wmes[ i ];
			break;
		}
	}

	free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );

	return return_val;
}

/***************************************************************************
 * Function     : epmem_symbol_to_string
 * Author		: Nate Derbinsky
 * Notes		: Converts any constant symbol type to a string
 **************************************************************************/
const char *epmem_symbol_to_string( agent *my_agent, Symbol *sym )
{
	switch( sym->common.symbol_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
		case INT_CONSTANT_SYMBOL_TYPE:
		case FLOAT_CONSTANT_SYMBOL_TYPE:
			return symbol_to_string( my_agent, sym, false, NULL, NULL );
	}

	return "";
}


void _epmem_add_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value, bool meta )
{		
	wme *w = soar_module::add_module_wme( my_agent, id, attr, value );
	w->preference = soar_module::make_fake_preference( my_agent, state, w, state->id.epmem_info->cue_wmes );

	if ( w->preference )
		add_preference_to_tm( my_agent, w->preference );

	if ( meta )
		state->id.epmem_info->epmem_wmes->push( w );
}

void epmem_add_retrieved_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value )
{
	_epmem_add_wme( my_agent, state, id, attr, value, false );
}

void epmem_add_meta_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value )
{
	_epmem_add_wme( my_agent, state, id, attr, value, true );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timed Query Functions (epmem::query)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_exec_query
 * Author		: Nate Derbinsky
 * Notes		: Starts/stops a timer around query execution
 **************************************************************************/
int epmem_exec_query( sqlite3_stmt *stmt, soar_module::timer *timer )
{
	int return_val;

	timer->start();	
	return_val = sqlite3_step( stmt );	
	timer->stop();

	return return_val;
}

/***************************************************************************
 * Function     : epmem_exec_range_query
 * Author		: Nate Derbinsky
 * Notes		: Easy timer function for range query structs
 **************************************************************************/
int epmem_exec_range_query( epmem_range_query *stmt )
{
	return epmem_exec_query( stmt->stmt, stmt->timer );
}

/***************************************************************************
 * Function     : epmem_exec_shared_query
 * Author		: Nate Derbinsky
 * Notes		: Easy timer function for shared query structs
 **************************************************************************/
int epmem_exec_shared_query( epmem_shared_query *stmt )
{
	return epmem_exec_query( stmt->stmt, stmt->timer );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Transaction Functions (epmem::transaction)
//
//  SQLite has support for transactions nearly ACID
//  transactions.  Unfortunately, each commit writes
//  everything to disk (i.e. no recovery log).
//
//  Thus I have implemented support for keeping
//  transactions open for a constant number of episodes
//  as controlled by the "commit" parameter.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_in_transaction
 * Author		: Nate Derbinsky
 * Notes		: Returns true if currently in a transaction according
 * 				  to the value of commit
 **************************************************************************/
bool epmem_in_transaction( agent *my_agent )
{
	if ( my_agent->epmem_db_status == EPMEM_DB_CLOSED )
		return false;

	return ( ( my_agent->epmem_stats->time->get_value() % my_agent->epmem_params->commit->get_value() ) != 0 );
}

/***************************************************************************
 * Function     : epmem_transaction_begin
 * Author		: Nate Derbinsky
 * Notes		: Starts a transaction
 **************************************************************************/
void epmem_transaction_begin( agent *my_agent )
{
	if ( my_agent->epmem_db_status != EPMEM_DB_CLOSED )
	{
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
	}
}

/***************************************************************************
 * Function     : epmem_transaction_end
 * Author		: Nate Derbinsky
 * Notes		: Ends the current transaction
 **************************************************************************/
void epmem_transaction_end( agent *my_agent, bool commit )
{
	if ( my_agent->epmem_db_status != EPMEM_DB_CLOSED )
	{
		unsigned long end_method = ( ( commit )?( EPMEM_STMT_COMMIT ):( EPMEM_STMT_ROLLBACK ) );

		sqlite3_step( my_agent->epmem_statements[ end_method ] );
		sqlite3_reset( my_agent->epmem_statements[ end_method ] );
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Variable Functions (epmem::var)
//
// Variables are key-value pairs stored in the database
// that are necessary to maintain a store between
// multiple runs of Soar.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_variable
 * Author		: Nate Derbinsky
 * Notes		: Gets an EpMem variable from the database
 **************************************************************************/
bool epmem_get_variable( agent *my_agent, epmem_variable_key variable_id, long *variable_value )
{
	int status;

	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ], 1, variable_id );
	status = sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ] );

	if ( status == SQLITE_ROW )
		(*variable_value) = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ], 0 );

	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ] );

	return ( status == SQLITE_ROW );
}

/***************************************************************************
 * Function     : epmem_set_variable
 * Author		: Nate Derbinsky
 * Notes		: Sets an EpMem variable in the database
 **************************************************************************/
void epmem_set_variable( agent *my_agent, epmem_variable_key variable_id, long variable_value )
{
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ], 1, variable_id );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ], 2, variable_value );
	sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ] );
	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ] );
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// RIT Functions (epmem::rit)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_rit_fork_node
 * Author		: Nate Derbinsky
 * Notes		: Implements the forkNode function of RIT
 **************************************************************************/
long epmem_rit_fork_node( agent * /*my_agent*/, epmem_time_id lower, epmem_time_id upper, bool bounds_offset, long *step_return, epmem_rit_state *rit_state )
{
	if ( !bounds_offset )
	{
		long offset = rit_state->offset.stat->get_value();

		lower = ( lower - offset );
		upper = ( upper - offset );
	}

	// descend the tree down to the fork node
	long node = EPMEM_RIT_ROOT;
	if ( upper < EPMEM_RIT_ROOT )
		node = rit_state->leftroot.stat->get_value();
	else if ( lower > EPMEM_RIT_ROOT )
		node = rit_state->rightroot.stat->get_value();

	long step;
	for ( step = ( ( ( node >= 0 )?( node ):( -1 * node ) ) / 2 ); step >= 1; step /= 2 )
	{
		if ( upper < node )
			node -= step;
		else if ( node < lower )
			node += step;
		else
			break;
	}

	if ( step_return != NULL )
		(*step_return) = step;

	return node;
}

/***************************************************************************
 * Function     : epmem_rit_clear_left_right
 * Author		: Nate Derbinsky
 * Notes		: Clears the left/right relations populated during prep
 **************************************************************************/
void epmem_rit_clear_left_right( agent *my_agent )
{
	sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_LEFT ] );
	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_LEFT ] );

	sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_RIGHT ] );
	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_RIGHT ] );
}

/***************************************************************************
 * Function     : epmem_rit_add_left
 * Author		: Nate Derbinsky
 * Notes		: Adds a range to the left relation
 **************************************************************************/
void epmem_rit_add_left( agent *my_agent, epmem_time_id min, epmem_time_id max )
{
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_LEFT ], 1, min );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_LEFT ], 2, max );

	sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_LEFT ] );
	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_LEFT ] );
}

/***************************************************************************
 * Function     : epmem_rit_add_right
 * Author		: Nate Derbinsky
 * Notes		: Adds a node to the to the right relation
 **************************************************************************/
void epmem_rit_add_right( agent *my_agent, epmem_time_id id )
{
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_RIGHT ], 1, id );

	sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_RIGHT ] );
	sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_RIGHT ] );
}

/***************************************************************************
 * Function     : epmem_rit_prep_left_right
 * Author		: Nate Derbinsky
 * Notes		: Implements the computational components of the RIT
 * 				  query algorithm
 **************************************************************************/
void epmem_rit_prep_left_right( agent *my_agent, epmem_time_id lower, epmem_time_id upper, epmem_rit_state *rit_state )
{
	////////////////////////////////////////////////////////////////////////////
	rit_state->timer->start();	
	////////////////////////////////////////////////////////////////////////////

	long offset = rit_state->offset.stat->get_value();
	long node, step;
	long left_node, left_step;
	long right_node, right_step;

	lower = ( lower - offset );
	upper = ( upper - offset );

	// auto add good range
	epmem_rit_add_left( my_agent, lower, upper );

	// go to fork
	node = EPMEM_RIT_ROOT;
	step = 0;
	if ( ( lower > node ) || (upper < node ) )
	{
		if ( lower > node )
		{
			node = rit_state->rightroot.stat->get_value();
			epmem_rit_add_left( my_agent, EPMEM_RIT_ROOT, EPMEM_RIT_ROOT );
		}
		else
		{
			node = rit_state->leftroot.stat->get_value();
			epmem_rit_add_right( my_agent, EPMEM_RIT_ROOT );
		}

		for ( step = ( ( ( node >= 0 )?( node ):( -1 * node ) ) / 2 ); step >= 1; step /= 2 )
		{
			if ( lower > node )
			{
				epmem_rit_add_left( my_agent, node, node );
				node += step;
			}
			else if ( upper < node )
			{
				epmem_rit_add_right( my_agent, node );
				node -= step;
			}
			else
				break;
		}
	}

	// go left
	left_node = node - step;
	for ( left_step = ( step / 2 ); left_step >= 1; left_step /= 2 )
	{
		if ( lower == left_node )
			break;
		else if ( lower > left_node )
		{
			epmem_rit_add_left( my_agent, left_node, left_node );
			left_node += left_step;
		}
		else
			left_node -= left_step;
	}

	// go right
	right_node = node + step;
	for ( right_step = ( step / 2 ); right_step >= 1; right_step /= 2 )
	{
		if ( upper == right_node )
			break;
		else if ( upper < right_node )
		{
			epmem_rit_add_right( my_agent, right_node );
			right_node -= right_step;
		}
		else
			right_node += right_step;
	}

	////////////////////////////////////////////////////////////////////////////
	rit_state->timer->stop();
	////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : epmem_rit_insert_interval
 * Author		: Nate Derbinsky
 * Notes		: Inserts an interval in the RIT
 **************************************************************************/
void epmem_rit_insert_interval( agent *my_agent, epmem_time_id lower, epmem_time_id upper, epmem_node_id id, epmem_rit_state *rit_state )
{
	// initialize offset
	long offset = rit_state->offset.stat->get_value();
	if ( offset == EPMEM_RIT_OFFSET_INIT )
	{
		offset = lower;

		// update database
		epmem_set_variable( my_agent, rit_state->offset.var_key, offset );

		// update stat
		rit_state->offset.stat->set_value( offset );
	}

	// get node
	long node;
	{
		long left_root = rit_state->leftroot.stat->get_value();
		long right_root = rit_state->rightroot.stat->get_value();
		long min_step = rit_state->minstep.stat->get_value();

		// shift interval
		epmem_time_id l = ( lower - offset );
		epmem_time_id u = ( upper - offset );

		// update left_root
		if ( ( u < EPMEM_RIT_ROOT ) && ( l <= ( 2 * left_root ) ) )
		{
			left_root = (long) pow( -2, floor( log( (double) -l ) / EPMEM_LN_2 ) );

			// update database
			epmem_set_variable( my_agent, rit_state->leftroot.var_key, left_root );

			// update stat
			rit_state->leftroot.stat->set_value( left_root );
		}

		// update right_root
		if ( ( l > EPMEM_RIT_ROOT ) && ( u >= ( 2 * right_root ) ) )
		{
			right_root = (long) pow( 2, floor( log( (double) u ) / EPMEM_LN_2 ) );

			// update database
			epmem_set_variable( my_agent, rit_state->rightroot.var_key, right_root );

			// update stat
			rit_state->rightroot.stat->set_value( right_root );
		}

		// update min_step
		long step;
		node = epmem_rit_fork_node( my_agent, l, u, true, &step, rit_state );

		if ( ( node != EPMEM_RIT_ROOT ) && ( step < min_step ) )
		{
			min_step = step;

			// update database
			epmem_set_variable( my_agent, rit_state->minstep.var_key, min_step );

			// update stat
			rit_state->minstep.stat->set_value( min_step );
		}
	}

	// perform insert
	// ( node, start, end, id )
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state->add_query ], 1, node );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state->add_query ], 2, lower );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state->add_query ], 3, upper );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state->add_query ], 4, id );
	sqlite3_step( my_agent->epmem_statements[ rit_state->add_query ] );
	sqlite3_reset( my_agent->epmem_statements[ rit_state->add_query ] );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Clean-Up Functions (epmem::clean)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_close
 * Author		: Nate Derbinsky
 * Notes		: Performs cleanup operations when the database needs
 * 				  to be closed (end soar, manual close, etc)
 **************************************************************************/
void epmem_close( agent *my_agent )
{
	if ( my_agent->epmem_db_status != EPMEM_DB_CLOSED )
	{
		// end any pending transactions
		if ( epmem_in_transaction( my_agent ) )
			epmem_transaction_end( my_agent, true );

		// perform mode-specific cleanup as necessary
		const long mode = my_agent->epmem_params->mode->get_value();
		if ( mode == epmem_param_container::graph )
		{
			epmem_parent_id_pool::iterator p;
			epmem_hashed_id_pool::iterator p_p;

			for ( p=my_agent->epmem_id_repository->begin(); p!=my_agent->epmem_id_repository->end(); p++ )
			{
				for ( p_p=p->second->begin(); p_p!=p->second->end(); p_p++ )
				{
					delete p_p->second;
				}

				delete p->second;
			}

			my_agent->epmem_id_repository->clear();
			my_agent->epmem_id_replacement->clear();

			my_agent->epmem_identifier_to_id->clear();
			my_agent->epmem_id_to_identifier->clear();
		}

		// deallocate query statements
		for ( int i=0; i<EPMEM_MAX_STATEMENTS; i++ )
		{
			if ( my_agent->epmem_statements[ i ] != NULL )
			{
				sqlite3_finalize( my_agent->epmem_statements[ i ] );
				my_agent->epmem_statements[ i ] = NULL;
			}
		}

		// close the database
		sqlite3_close( my_agent->epmem_db );

		// initialize database status
		my_agent->epmem_db = NULL;
		my_agent->epmem_db_status = EPMEM_DB_CLOSED;
	}
}

/***************************************************************************
 * Function     : epmem_clear_result
 * Author		: Nate Derbinsky
 * Notes		: Removes any WMEs produced by EpMem resulting from
 * 				  a command
 **************************************************************************/
void epmem_clear_result( agent *my_agent, Symbol *state )
{
	wme *w;
	
	while ( !state->id.epmem_info->epmem_wmes->empty() )
	{
		w = state->id.epmem_info->epmem_wmes->top();

		if ( w->preference )
			remove_preference_from_tm( my_agent, w->preference );

		soar_module::remove_module_wme( my_agent, w );

		state->id.epmem_info->epmem_wmes->pop();
	}
}

/***************************************************************************
 * Function     : epmem_reset
 * Author		: Nate Derbinsky
 * Notes		: Performs cleanup when a state is removed
 **************************************************************************/
void epmem_reset( agent *my_agent, Symbol *state )
{
	if ( state == NULL )
		state = my_agent->top_goal;

	while( state )
	{
		epmem_data *data = state->id.epmem_info;

		data->last_ol_time = 0;
		data->last_ol_count = 0;

		data->last_cmd_time = 0;
		data->last_cmd_count = 0;

		data->last_memory = EPMEM_MEMID_NONE;

		data->cue_wmes->clear();

		// clear off any result stuff (takes care of epmem_wmes)
		epmem_clear_result( my_agent, state );

		state = state->id.lower_goal;
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization Functions (epmem::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_init_db
 * Author		: Nate Derbinsky
 * Notes		: Opens the SQLite database and performs all
 * 				  initialization required for the current mode
 *
 *                All statement preparation should be asserted
 *                to help reduce hard-to-detect errors due to
 *                typos in SQL.
 *
 *                The readonly param should only be used in
 *                experimentation where you don't want to alter
 *                previous database state.
 **************************************************************************/
void epmem_init_db( agent *my_agent, bool readonly = false )
{
	if ( my_agent->epmem_db_status != EPMEM_DB_CLOSED )
		return;

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->init->start();	
	////////////////////////////////////////////////////////////////////////////

	const char *db_path;
	if ( my_agent->epmem_params->database->get_value() == epmem_param_container::memory )
		db_path = ":memory:";
	else
		db_path = my_agent->epmem_params->path->get_value();

	// attempt connection
	my_agent->epmem_db_status = sqlite3_open_v2( db_path, &(my_agent->epmem_db), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );

	if ( my_agent->epmem_db_status )
	{
		char buf[256];
		SNPRINTF( buf, 254, "DB ERROR: %s", sqlite3_errmsg( my_agent->epmem_db ) );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}
	else
	{
		bool my_assert;
		const char *tail;
		sqlite3_stmt *create;
		epmem_time_id time_max;

		// point stuff
		epmem_time_id range_start;
		epmem_time_id time_last;

		// update validation count
		my_agent->epmem_validation++;

		// create vars table (needed before var queries)
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS vars (id INTEGER PRIMARY KEY,value NONE)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		sqlite3_step( create );
		sqlite3_finalize( create );

		// rit_left_nodes table (rit)
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS rit_left_nodes (min INTEGER, max INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		sqlite3_step( create );
		sqlite3_finalize( create );

		// rit_right_nodes table (rit)
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS rit_right_nodes (node INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		sqlite3_step( create );
		sqlite3_finalize( create );

		// create temporal hash table + index (needed before hash queries)
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS temporal_symbol_hash (id INTEGER PRIMARY KEY, sym_const NONE, sym_type INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		sqlite3_step( create );
		sqlite3_finalize( create );

		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS temporal_symbol_hash_const_type ON temporal_symbol_hash (sym_type,sym_const)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		sqlite3_step( create );
		sqlite3_finalize( create );

		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO temporal_symbol_hash (id,sym_const,sym_type) VALUES (?,?,?)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
		assert( my_assert );
		EPMEM_SQLITE_BIND_INT( create, 1, 0 );
		sqlite3_bind_null( create, 2 );
		EPMEM_SQLITE_BIND_INT( create, 3, IDENTIFIER_SYMBOL_TYPE );
		sqlite3_step( create );
		sqlite3_finalize( create );


		// common queries
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "BEGIN", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "COMMIT", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "ROLLBACK", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ROLLBACK ] ), &tail ) == SQLITE_OK );
		assert( my_assert );

		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT value FROM vars WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "REPLACE INTO vars (id,value) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ] ), &tail ) == SQLITE_OK );
		assert( my_assert );

		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO rit_left_nodes (min,max) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_LEFT ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM rit_left_nodes", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_LEFT ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO rit_right_nodes (node) VALUES (?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_RIT_ADD_RIGHT ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM rit_right_nodes", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_RIT_TRUNCATE_RIGHT ] ), &tail ) == SQLITE_OK );
		assert( my_assert );

		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM temporal_symbol_hash WHERE sym_type=? AND sym_const=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] ), &tail ) == SQLITE_OK );
		assert( my_assert );
		my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO temporal_symbol_hash (sym_type,sym_const) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] ), &tail ) == SQLITE_OK );
		assert( my_assert );

		// mode - read if existing
		long mode;
		{
			long stored_mode = NULL;
			if ( epmem_get_variable( my_agent, var_mode, &stored_mode ) )
			{
				my_agent->epmem_params->mode->set_value( (epmem_param_container::mode_choices) stored_mode );
				mode = stored_mode;
			}
			else
			{
				mode = my_agent->epmem_params->mode->get_value();
				epmem_set_variable( my_agent, var_mode, mode );
			}
		}

		// at this point initialize the database for receipt of episodes
		epmem_transaction_begin( my_agent );

		if ( mode == epmem_param_container::tree )
		{
			// variable initialization
			my_agent->epmem_stats->time->set_value( 1 );
			epmem_rit_state_tree.offset.stat->set_value( EPMEM_RIT_OFFSET_INIT );
			epmem_rit_state_tree.leftroot.stat->set_value( 0 );
			epmem_rit_state_tree.rightroot.stat->set_value( 0 );
			epmem_rit_state_tree.minstep.stat->set_value( LONG_MAX );
			my_agent->epmem_node_mins->clear();
			my_agent->epmem_node_maxes->clear();
			my_agent->epmem_node_removals->clear();

			// times table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS times (id INTEGER PRIMARY KEY)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting times
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO times (id) VALUES (?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_now table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_now (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// node_now_start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_now_start ON node_now (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_now_id_start ON node_now (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting now
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_now (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for deleting now
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM node_now WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_DELETE_NODE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_point table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_point (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_point_id_start ON node_point (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_point_start ON node_point (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting nodes
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_point (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_range table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// lowerindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_range_lower ON node_range (rit_node,start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// upperindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_range_upper ON node_range (rit_node,end)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_start ON node_range (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_end index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_end ON node_range (id,end DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting episodes
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_range (rit_node,start,end,id) VALUES (?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_RANGE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_unique table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,attrib INTEGER,value INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// index for searching
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_unique_parent_attrib_value ON node_unique (parent_id,attrib,value)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting ids
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_unique (parent_id,attrib,value) VALUES (?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding non-identifier id's
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding identifier id's
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=0", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			//

			// custom statement for validating an episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT COUNT(*) AS ct FROM times WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding the next episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id>? ORDER BY id ASC LIMIT 1", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding the prev episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id<? ORDER BY id DESC LIMIT 1", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// custom statement for range intersection query
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT i.child_id, i.parent_id, h1.sym_const, h2.sym_const, h1.sym_type, h2.sym_type FROM node_unique i, temporal_symbol_hash h1, temporal_symbol_hash h2 WHERE i.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) AND i.attrib=h1.id AND i.value=h2.id ORDER BY i.child_id ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// get/set RIT variables
			{
				long var_val;

				// offset
				if ( epmem_get_variable( my_agent, epmem_rit_state_tree.offset.var_key, &var_val ) )
					epmem_rit_state_tree.offset.stat->set_value( var_val );
				else
					epmem_set_variable( my_agent, epmem_rit_state_tree.offset.var_key, epmem_rit_state_tree.offset.stat->get_value() );

				// leftroot
				if ( epmem_get_variable( my_agent, epmem_rit_state_tree.leftroot.var_key, &var_val ) )
					epmem_rit_state_tree.leftroot.stat->set_value( var_val );
				else
					epmem_set_variable( my_agent, epmem_rit_state_tree.leftroot.var_key, epmem_rit_state_tree.leftroot.stat->get_value() );

				// rightroot
				if ( epmem_get_variable( my_agent, epmem_rit_state_tree.rightroot.var_key, &var_val ) )
					epmem_rit_state_tree.rightroot.stat->set_value( var_val );
				else
					epmem_set_variable( my_agent, epmem_rit_state_tree.rightroot.var_key, epmem_rit_state_tree.rightroot.stat->get_value() );

				// minstep
				if ( epmem_get_variable( my_agent, epmem_rit_state_tree.minstep.var_key, &var_val ) )
					epmem_rit_state_tree.minstep.stat->set_value( var_val );
				else
					epmem_set_variable( my_agent, epmem_rit_state_tree.minstep.var_key, epmem_rit_state_tree.minstep.stat->get_value() );
			}

			// get max time
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(id) FROM times", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			if ( sqlite3_step( create ) == SQLITE_ROW )
				my_agent->epmem_stats->time->set_value( ( EPMEM_SQLITE_COLUMN_INT( create, 0 ) + 1 ) );
			sqlite3_finalize( create );
			time_max = my_agent->epmem_stats->time->get_value();

			if ( !readonly )
			{
				// insert non-NOW intervals for all current NOW's
				time_last = ( time_max - 1 );
				my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id,start FROM node_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
				assert( my_assert );
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ], 2, time_last );
				while ( sqlite3_step( create ) == SQLITE_ROW )
				{
					range_start = EPMEM_SQLITE_COLUMN_INT( create, 1 );

					// point
					if ( range_start == time_last )
					{
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ], 1, EPMEM_SQLITE_COLUMN_INT( create, 0 ) );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ] );
					}
					else
						epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), &epmem_rit_state_tree );
				}
				sqlite3_finalize( create );

				// remove all NOW intervals
				my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM node_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
				assert( my_assert );
				sqlite3_step( create );
				sqlite3_finalize( create );
			}

			// get max id + max list
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(child_id) FROM node_unique", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			if ( sqlite3_column_type( create, 0 ) != SQLITE_NULL )
			{
				unsigned long num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

				my_agent->epmem_node_maxes->resize( num_ids, true );
				my_agent->epmem_node_mins->resize( num_ids, time_max );
			}
			sqlite3_finalize( create );
		}
		else if ( mode == epmem_param_container::graph )
		{
			// initialize range tracking
			my_agent->epmem_node_mins->clear();
			my_agent->epmem_node_maxes->clear();
			my_agent->epmem_node_removals->clear();

			my_agent->epmem_edge_mins->clear();
			my_agent->epmem_edge_maxes->clear();
			my_agent->epmem_edge_removals->clear();

			(*my_agent->epmem_id_repository)[ EPMEM_NODEID_ROOT ] = new epmem_hashed_id_pool();

			// initialize time
			my_agent->epmem_stats->time->set_value( 1 );

			// initialize next_id
			my_agent->epmem_stats->next_id->set_value( 1 );
			{
				long stored_id = NULL;
				if ( epmem_get_variable( my_agent, var_next_id, &stored_id ) )
					my_agent->epmem_stats->next_id->set_value( stored_id );
				else
					epmem_set_variable( my_agent, var_next_id, my_agent->epmem_stats->next_id->get_value() );
			}

			// initialize rit state
			for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
			{
				epmem_rit_state_graph[ i ].offset.stat->set_value( EPMEM_RIT_OFFSET_INIT );
				epmem_rit_state_graph[ i ].leftroot.stat->set_value( 0 );
				epmem_rit_state_graph[ i ].rightroot.stat->set_value( 1 );
				epmem_rit_state_graph[ i ].minstep.stat->set_value( LONG_MAX );
			}

			////

			// times table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS times (id INTEGER PRIMARY KEY)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting times
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO times (id) VALUES (?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_now table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_now (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_now_start ON node_now (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_now_id_start ON node_now (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_now (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for deleting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM node_now WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_NODE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// edge_now table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS edge_now (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_now_start ON edge_now (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS edge_now_id_start ON edge_now (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO edge_now (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for deleting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM edge_now WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_EDGE_NOW ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_point table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_point (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_point_id_start ON node_point (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_point_start ON node_point (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_point (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// edge_point table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS edge_point (id INTEGER,start INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS edge_point_id_start ON edge_point (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// start index
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_point_start ON edge_point (start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO edge_point (id,start) VALUES (?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_range table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// lowerindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_range_lower ON node_range (rit_node,start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// upperindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_range_upper ON node_range (rit_node,end)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_start ON node_range (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_end index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_end ON node_range (id,end DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_range (rit_node,start,end,id) VALUES (?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_RANGE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// edge_range table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS edge_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// lowerindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_range_lower ON edge_range (rit_node,start)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// upperindex
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_range_upper ON edge_range (rit_node,end)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_start index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_start ON edge_range (id,start DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// id_end index (for queries)
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_end ON edge_range (id,end DESC)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO edge_range (rit_node,start,end,id) VALUES (?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_RANGE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// node_unique table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,attrib INTEGER, value INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// index for searching
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS node_unique_parent_attrib_value ON node_unique (parent_id,attrib,value)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for finding
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_unique (parent_id,attrib,value) VALUES (?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// edge_unique table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS edge_unique (parent_id INTEGER PRIMARY KEY AUTOINCREMENT,q0 INTEGER,w INTEGER,q1 INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// index for identification
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_unique_q0_w_q1 ON edge_unique (q0,w,q1)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for finding
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT parent_id, q1 FROM edge_unique WHERE q0=? AND w=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT parent_id FROM edge_unique WHERE q0=? AND w=? AND q1=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE_SHARED ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO edge_unique (q0,w,q1) VALUES (?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// custom statement for validating an episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT COUNT(*) AS ct FROM times WHERE id=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding the next episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id>? ORDER BY id ASC LIMIT 1", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding the prev episode
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id<? ORDER BY id DESC LIMIT 1", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// range intersection query: node
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT f.child_id, f.parent_id, h1.sym_const, h2.sym_const, h1.sym_type, h2.sym_type FROM node_unique f, temporal_symbol_hash h1, temporal_symbol_hash h2 WHERE f.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) AND f.attrib=h1.id AND f.value=h2.id ORDER BY f.child_id ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// range intersection query: edge
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT f.q0, h.sym_const, f.q1, h.sym_type FROM edge_unique f INNER JOIN temporal_symbol_hash h ON f.w=h.id WHERE f.parent_id IN (SELECT n.id FROM edge_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM edge_point p WHERE p.start=? UNION ALL SELECT e1.id FROM edge_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM edge_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY f.q0 ASC, f.q1 ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// get/set RIT variables
			{
				long var_val;

				for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
				{
					// offset
					if ( epmem_get_variable( my_agent, epmem_rit_state_graph[ i ].offset.var_key, &var_val ) )
						epmem_rit_state_graph[ i ].offset.stat->set_value( var_val );
					else
						epmem_set_variable( my_agent, epmem_rit_state_graph[ i ].offset.var_key, epmem_rit_state_graph[ i ].offset.stat->get_value() );

					// leftroot
					if ( epmem_get_variable( my_agent, epmem_rit_state_graph[ i ].leftroot.var_key, &var_val ) )
						epmem_rit_state_graph[ i ].leftroot.stat->set_value( var_val );
					else
						epmem_set_variable( my_agent, epmem_rit_state_graph[ i ].leftroot.var_key, epmem_rit_state_graph[ i ].leftroot.stat->get_value() );

					// rightroot
					if ( epmem_get_variable( my_agent, epmem_rit_state_graph[ i ].rightroot.var_key, &var_val ) )
						epmem_rit_state_graph[ i ].rightroot.stat->set_value( var_val );
					else
						epmem_set_variable( my_agent, epmem_rit_state_graph[ i ].rightroot.var_key, epmem_rit_state_graph[ i ].rightroot.stat->get_value() );

					// minstep
					if ( epmem_get_variable( my_agent, epmem_rit_state_graph[ i ].minstep.var_key, &var_val ) )
						epmem_rit_state_graph[ i ].minstep.stat->set_value( var_val );
					else
						epmem_set_variable( my_agent, epmem_rit_state_graph[ i ].minstep.var_key, epmem_rit_state_graph[ i ].minstep.stat->get_value() );
					}
			}

			////

			// get max time
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(id) FROM times", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			if ( sqlite3_step( create ) == SQLITE_ROW )
				my_agent->epmem_stats->time->set_value( ( EPMEM_SQLITE_COLUMN_INT( create, 0 ) + 1 ) );
			sqlite3_finalize( create );
			time_max = my_agent->epmem_stats->time->get_value();

			// insert non-NOW intervals for all current NOW's
			// remove NOW's
			if ( !readonly )
			{
				time_last = ( time_max - 1 );

				// nodes
				{
					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id,start FROM node_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ], 2, time_last );
					while ( sqlite3_step( create ) == SQLITE_ROW )
					{
						range_start = EPMEM_SQLITE_COLUMN_INT( create, 1 );

						// point
						if ( range_start == time_last )
						{
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ], 1, EPMEM_SQLITE_COLUMN_INT( create, 0 ) );
							sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ] );
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ] );
						}
						else
							epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), &( epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );
					}
					sqlite3_finalize( create );

					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM node_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					sqlite3_step( create );
					sqlite3_finalize( create );
				}

				// edges
				{
					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id,start FROM edge_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ], 2, time_last );
					while ( sqlite3_step( create ) == SQLITE_ROW )
					{
						range_start = EPMEM_SQLITE_COLUMN_INT( create, 1 );

						// point
						if ( range_start == time_last )
						{
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ], 1, EPMEM_SQLITE_COLUMN_INT( create, 0 ) );
							sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ] );
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ] );
						}
						else
							epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), &( epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );
					}
					sqlite3_finalize( create );

					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM edge_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					sqlite3_step( create );
					sqlite3_finalize( create );
				}
			}

			// get max id + max list
			{
				// nodes
				{
					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(child_id) FROM node_unique", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					sqlite3_step( create );
					if ( sqlite3_column_type( create, 0 ) != SQLITE_NULL )
					{
						unsigned long num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

						my_agent->epmem_node_maxes->resize( num_ids, true );
						my_agent->epmem_node_mins->resize( num_ids, time_max );
					}
					sqlite3_finalize( create );
				}


				// edges
				{
					my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(parent_id) FROM edge_unique", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
					assert( my_assert );
					sqlite3_step( create );
					if ( sqlite3_column_type( create, 0 ) != SQLITE_NULL )
					{
						unsigned long num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

						my_agent->epmem_edge_maxes->resize( num_ids, true );
						my_agent->epmem_edge_mins->resize( num_ids, time_max );
					}
					sqlite3_finalize( create );
				}
			}

			// get id pools
			{
				epmem_node_id q0;
				long w;
				epmem_node_id q1;
				epmem_node_id parent_id;

				epmem_hashed_id_pool **hp;
				epmem_id_pool **ip;

				my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT q0, w, q1, parent_id FROM edge_unique", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
				assert( my_assert );

				while ( sqlite3_step( create ) == SQLITE_ROW )
				{
					q0 = EPMEM_SQLITE_COLUMN_INT( create, 0 );
					w = EPMEM_SQLITE_COLUMN_INT( create, 1 );
					q1 = EPMEM_SQLITE_COLUMN_INT( create, 2 );
					parent_id = EPMEM_SQLITE_COLUMN_INT( create, 3 );

					hp =& (*my_agent->epmem_id_repository)[ q0 ];
					if ( !(*hp) )
						(*hp) = new epmem_hashed_id_pool();

					ip =& (*(*hp))[ w ];
					if ( !(*ip) )
						(*ip) = new epmem_id_pool();

					(*(*ip))[ q1 ] = parent_id;
				}
				sqlite3_finalize( create );
			}
		}

		epmem_transaction_end( my_agent, true );
		epmem_transaction_begin( my_agent );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->init->stop();	
	////////////////////////////////////////////////////////////////////////////
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Temporal Hash Functions (epmem::hash)
//
// The rete has symbol hashing, but the values are
// reliable only for the lifetime of a symbol.  This
// isn't good for EpMem.  Hence, we implement a simple
// lookup table, relying upon SQLite to deal with
// efficiency issues.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_temporal_hash
 * Author		: Nate Derbinsky
 * Notes		: Returns a temporally unique integer representing
 *                a symbol constant.
 **************************************************************************/
long epmem_temporal_hash( agent *my_agent, Symbol *sym, bool add_on_fail = true )
{
	long return_val = NULL;

	// basic process:
	// - search
	// - if found, return
	// - else, add
	if ( sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE )
	{
		// search (type, value)
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 1, SYM_CONSTANT_SYMBOL_TYPE );
			sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 2, (const char *) sym->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );

			if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] ) == SQLITE_ROW )
				return_val = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 0 );

			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] );
		}

		// add (type, value)
		if ( !return_val && add_on_fail )
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 1, SYM_CONSTANT_SYMBOL_TYPE );
			sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 2, (const char *) sym->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );

			return_val = (long) sqlite3_last_insert_rowid( my_agent->epmem_db );
		}
	}
	else if ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
	{
		// search (type, value)
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 1, INT_CONSTANT_SYMBOL_TYPE );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 2, sym->ic.value );

			if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] ) == SQLITE_ROW )
				return_val = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 0 );

			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] );
		}

		// add (type, value)
		if ( !return_val && add_on_fail )
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 1, INT_CONSTANT_SYMBOL_TYPE );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 2, sym->ic.value );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );

			return_val = (long) sqlite3_last_insert_rowid( my_agent->epmem_db );
		}
	}
	else if ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
	{
		// search (type, value)
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 1, FLOAT_CONSTANT_SYMBOL_TYPE );
			sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 2, sym->fc.value );

			if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] ) == SQLITE_ROW )
				return_val = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ], 0 );

			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_GET_HASH ] );
		}

		// add (type, value)
		if ( !return_val && add_on_fail )
		{
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 1, FLOAT_CONSTANT_SYMBOL_TYPE );
			sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ], 2, sym->fc.value );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ADD_HASH ] );

			return_val = (long) sqlite3_last_insert_rowid( my_agent->epmem_db );
		}
	}

	return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Storage Functions (epmem::storage)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_new_episode
 * Author		: Nate Derbinsky
 * Notes		: Big picture: only process changes!
 *
 * 				  Episode storage entails recursively traversing
 * 				  working memory.  If we encounter a WME we've
 * 				  seen before (because it has an associated id),
 * 				  ignore it.  If we encounter something new, try
 * 				  to identify it (add to unique if not seen before)
 * 				  and note the start of something new.  When WMEs
 * 				  are removed from the rete (see rete.cpp)
 * 				  their loss is noted and recorded here
 * 				  (if the WME didn't re-appear).
 **************************************************************************/
void epmem_new_episode( agent *my_agent )
{
	// if this is the first episode, initialize db components
	if ( my_agent->epmem_db_status == EPMEM_DB_CLOSED )
		epmem_init_db( my_agent );

	// add the episode only if db is properly initialized
	if ( my_agent->epmem_db_status != SQLITE_OK )
		return;

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->storage->start();	
	////////////////////////////////////////////////////////////////////////////

	epmem_time_id time_counter = my_agent->epmem_stats->time->get_value();

	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%d, %d)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number, time_counter );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}

	const long mode = my_agent->epmem_params->mode->get_value();

	if ( mode == epmem_param_container::tree )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		// keeps children of the identifier of interest
		wme **wmes = NULL;
		unsigned long len = 0;
		unsigned long i;

		// future states of breadth-first search
		std::queue<Symbol *> syms;
		std::queue<epmem_node_id> ids;

		// current state
		epmem_node_id parent_id;

		// nodes to be recorded (implements tree flattening)
		std::map<epmem_node_id, bool> epmem;

		// wme hashing improves search speed
		long my_hash;		// attribute
		long my_hash2;	// value

		// prevents infinite loops
		int tc = get_new_tc_number( my_agent );

		// initialize BFS at top-state
		syms.push( my_agent->top_goal );
		ids.push( EPMEM_NODEID_ROOT );

		while ( !syms.empty() )
		{
			// get state
			parent_sym = syms.front();
			syms.pop();

			parent_id = ids.front();
			ids.pop();

			// get children of the current identifier
			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				for ( i=0; i<len; i++ )
				{
					// prevent acceptables from being recorded
					if ( wmes[i]->acceptable )
						continue;

					// prevent exclusions from being recorded
					if ( my_agent->epmem_params->exclusions->in_set( wmes[i]->attr ) )
						continue;

					// if we haven't seen this WME before
					// or we haven't seen it within this database...
					// look it up in the database
					if ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) )
					{
						wmes[i]->epmem_id = NULL;
						wmes[i]->epmem_valid = my_agent->epmem_validation;

						if ( wmes[i]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{
							my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );
							my_hash2 = epmem_temporal_hash( my_agent, wmes[i]->value );

							// parent_id=? AND attr=? AND value=?
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 1, parent_id );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 2, my_hash );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, my_hash2 );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 0 );

							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] );
						}
						else
						{
							my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );

							// parent_id=? AND attr=? AND value IS NULL
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 1, parent_id );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 2, my_hash );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 0 );

							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] );
						}
					}

					// insert on no id
					if ( wmes[i]->epmem_id == NULL )
					{
						// insert (parent_id,attr,value)
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 1, parent_id );
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 2, my_hash );

						switch ( wmes[i]->value->common.symbol_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
							case INT_CONSTANT_SYMBOL_TYPE:
							case FLOAT_CONSTANT_SYMBOL_TYPE:
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3, my_hash2 );
								break;

							case IDENTIFIER_SYMBOL_TYPE:
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3, 0 );
								break;
						}
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] );

						wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );

						// new nodes definitely start
						epmem[ wmes[i]->epmem_id ] = true;
						my_agent->epmem_node_mins->push_back( time_counter );
						my_agent->epmem_node_maxes->push_back( false );
					}
					else
					{
						// definitely don't remove
						(*my_agent->epmem_node_removals)[ wmes[i]->epmem_id ] = false;

						// we add ONLY if the last thing we did was a remove
						if ( (*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] )
						{
							epmem[ wmes[i]->epmem_id ] = true;
							(*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] = false;
						}
					}

					// keep track of identifiers (for further study)
					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						syms.push( wmes[i]->value );
						ids.push( wmes[i]->epmem_id );
					}
				}

				// free space from aug list
				free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
			}
		}

		// all inserts at once (provides unique)
		std::map<epmem_node_id, bool>::iterator e = epmem.begin();
		while ( e != epmem.end() )
		{
			// add NOW entry
			// id = ?, start = ?
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_NOW ], 1, e->first );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_NOW ], 2, time_counter );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_NOW ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_NOW ] );

			// update min
			(*my_agent->epmem_node_mins)[ e->first - 1 ] = time_counter;

			e++;
		}

		// all removals at once
		std::map<epmem_node_id, bool>::iterator r = my_agent->epmem_node_removals->begin();
		epmem_time_id range_start;
		epmem_time_id range_end;
		while ( r != my_agent->epmem_node_removals->end() )
		{
			if ( r->second )
			{
				// remove NOW entry
				// id = ?
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_DELETE_NODE_NOW ], 1, r->first );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_DELETE_NODE_NOW ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_DELETE_NODE_NOW ] );

				range_start = (*my_agent->epmem_node_mins)[ r->first - 1 ];
				range_end = ( time_counter - 1 );

				// point (id, start)
				if ( range_start == range_end )
				{
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ], 1, r->first );
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ], 2, range_start );
					sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ] );
					sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_POINT ] );
				}
				// node
				else
					epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &epmem_rit_state_tree );

				// update max
				(*my_agent->epmem_node_maxes)[ r->first - 1 ] = true;
			}

			r++;
		}
		my_agent->epmem_node_removals->clear();

		// add the time id to the times table
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ], 1, time_counter );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ] );

		my_agent->epmem_stats->time->set_value( time_counter + 1 );
	}
	else if ( mode == epmem_param_container::graph )
	{
		// prevents infinite loops
		int tc = get_new_tc_number( my_agent );
		std::map<epmem_node_id, bool> seen_ids;
		std::map<epmem_node_id, bool>::iterator seen_p;

		// breadth first search state
		std::queue<Symbol *> parent_syms;
		Symbol *parent_sym;
		std::queue<epmem_node_id> parent_ids;
		epmem_node_id parent_id;

		// seen nodes (non-identifiers) and edges (identifiers)
		std::queue<epmem_node_id> epmem_node;
		std::queue<epmem_node_id> epmem_edge;

		// temporal hash
		long my_hash;		// attribute
		long my_hash2;	// value

		// id repository
		epmem_id_pool **my_id_repo;
		epmem_id_pool *my_id_repo2;
		epmem_id_pool::iterator pool_p;
		std::map<wme *, epmem_id_reservation *> id_reservations;
		std::map<wme *, epmem_id_reservation *>::iterator r_p;
		epmem_id_reservation *new_id_reservation;

		// children of the current identifier
		wme **wmes = NULL;
		unsigned long len = 0;
		unsigned long i;

		// initialize BFS
		parent_syms.push( my_agent->top_goal );
		parent_ids.push( EPMEM_NODEID_ROOT );

		// three cases for sharing ids amongst identifiers in two passes:
		// 1. value known in phase one (try reservation)
		// 2. value unknown in phase one, but known at phase two (try assignment adhering to constraint)
		// 3. value unknown in phase one/two (if anything is left, unconstrained assignment)

		while ( !parent_syms.empty() )
		{
			parent_sym = parent_syms.front();
			parent_syms.pop();

			parent_id = parent_ids.front();
			parent_ids.pop();

			// get children WMEs
			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				// pre-assign unknown identifiers with known children (prevents ordering issues with unknown children)
				for ( i=0; i<len; i++ )
				{
					if ( ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
						 ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) ) &&
						 ( wmes[i]->value->id.epmem_id ) )
					{
						// prevent acceptables from being recorded
						if ( wmes[i]->acceptable )
							continue;

						// prevent exclusions from being recorded
						if ( my_agent->epmem_params->exclusions->in_set( wmes[i]->attr ) )
							continue;

						// if still here, create reservation (case 1)
						new_id_reservation = new epmem_id_reservation;
						new_id_reservation->my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );
						new_id_reservation->my_id = EPMEM_NODEID_ROOT;
						new_id_reservation->my_pool = NULL;

						// try to find appropriate reservation
						my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ new_id_reservation->my_hash ];
						if ( (*my_id_repo) )
						{
							if ( !(*my_id_repo)->empty() )
							{
								pool_p = (*my_id_repo)->find( wmes[i]->value->id.epmem_id );
								if ( pool_p != (*my_id_repo)->end() )
								{
									new_id_reservation->my_id = pool_p->second;									

									(*my_id_repo)->erase( pool_p );
								}
							}
						}
						else
						{
							// add repository
							(*my_id_repo) = new epmem_id_pool();							
						}

						new_id_reservation->my_pool = (*my_id_repo);
						id_reservations[ wmes[i] ] = new_id_reservation;
						new_id_reservation = NULL;
					}
				}

				for ( i=0; i<len; i++ )
				{
					// prevent acceptables from being recorded
					if ( wmes[i]->acceptable )
						continue;

					// prevent exclusions from being recorded
					if ( my_agent->epmem_params->exclusions->in_set( wmes[i]->attr ) )
						continue;

					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						// have we seen this WME during this database?
						if ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) )
						{
							wmes[i]->epmem_valid = my_agent->epmem_validation;
							wmes[i]->epmem_id = NULL;

							my_hash = NULL;							
							my_id_repo2 = NULL;

							// in the case of a known child, we already have a reservation (case 1)						
							if ( wmes[i]->value->id.epmem_id )
							{
								r_p = id_reservations.find( wmes[i] );

								if ( r_p != id_reservations.end() )
								{
									// restore reservation info
									my_hash = r_p->second->my_hash;
									my_id_repo2 = r_p->second->my_pool;

									if ( r_p->second->my_id != EPMEM_NODEID_ROOT )
									{
										wmes[i]->epmem_id = r_p->second->my_id;
										(*my_agent->epmem_id_replacement)[ wmes[i]->epmem_id ] = my_id_repo2;
									}

									// delete reservation and map entry
									delete r_p->second;
									id_reservations.erase( r_p );
								}
								// OR a shared identifier at the same level, in which case we need an exact match (case 2)
								else
								{
									// get temporal hash
									my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );

									// try to get an id that matches new information
									my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ my_hash ];
									if ( (*my_id_repo) )
									{
										if ( !(*my_id_repo)->empty() )
										{
											pool_p = (*my_id_repo)->find( wmes[i]->value->id.epmem_id );
											if ( pool_p != (*my_id_repo)->end() )
											{
												wmes[i]->epmem_id = pool_p->second;
												(*my_id_repo)->erase( pool_p );

												(*my_agent->epmem_id_replacement)[ wmes[i]->epmem_id ] = (*my_id_repo);											
											}
										}
									}
									else
									{
										// add repository
										(*my_id_repo) = new epmem_id_pool();
									}

									// keep the address for later use
									my_id_repo2 = (*my_id_repo);
								}
							}
							// case 3
							else
							{
								// get temporal hash
								my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );

								// try to find node
								my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ my_hash ];
								if ( (*my_id_repo) )
								{
									// if something leftover, use it
									if ( !(*my_id_repo)->empty() )
									{
										epmem_reverse_constraint_list::iterator rcp;
										pool_p = (*my_id_repo)->begin();

										do
										{
											rcp = my_agent->epmem_id_to_identifier->find( pool_p->first );
											if ( rcp == my_agent->epmem_id_to_identifier->end() )
											{
												(*my_agent->epmem_identifier_to_id)[ wmes[i]->value ] = pool_p->first;
												(*my_agent->epmem_id_to_identifier)[ pool_p->first ] = wmes[i]->value;

												wmes[i]->epmem_id = pool_p->second;
												wmes[i]->value->id.epmem_id = pool_p->first;
												(*my_id_repo)->erase( pool_p );
												(*my_agent->epmem_id_replacement)[ wmes[i]->epmem_id ] = (*my_id_repo);

												pool_p = (*my_id_repo)->end();
											}
											else
											{
												pool_p++;
											}
										} while ( pool_p != (*my_id_repo)->end() );
									}									
								}
								else
								{
									// add repository
									(*my_id_repo) = new epmem_id_pool();
								}

								// keep the address for later use
								my_id_repo2 = (*my_id_repo);
							}

							// add path if no success above
							if ( wmes[i]->epmem_id == NULL )
							{
								if ( wmes[i]->value->id.epmem_id == NULL )
								{
									// update next id
									wmes[i]->value->id.epmem_id = my_agent->epmem_stats->next_id->get_value();
									my_agent->epmem_stats->next_id->set_value( wmes[i]->value->id.epmem_id + 1 );
									epmem_set_variable( my_agent, var_next_id, wmes[i]->value->id.epmem_id + 1 );

									// add repository
									(*my_agent->epmem_id_repository)[ wmes[i]->value->id.epmem_id ] = new epmem_hashed_id_pool();
								}

								// insert (q0,w,q1)
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 1, parent_id );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 2, my_hash );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 3, wmes[i]->value->id.epmem_id );
								sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] );
								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] );

								wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );
								(*my_agent->epmem_id_replacement)[ wmes[i]->epmem_id ] = my_id_repo2;

								// new nodes definitely start
								epmem_edge.push( wmes[i]->epmem_id );
								my_agent->epmem_edge_mins->push_back( time_counter );
								my_agent->epmem_edge_maxes->push_back( false );
							}
							else
							{
								// definitely don't remove
								(*my_agent->epmem_edge_removals)[ wmes[i]->epmem_id ] = false;

								// we add ONLY if the last thing we did was remove
								if ( (*my_agent->epmem_edge_maxes)[ wmes[i]->epmem_id - 1 ] )
								{
									epmem_edge.push( wmes[i]->epmem_id );
									(*my_agent->epmem_edge_maxes)[ wmes[i]->epmem_id - 1 ] = false;
								}
							}
						}						

						// path id in cache?
						seen_p = seen_ids.find( wmes[i]->value->id.epmem_id );
						if ( seen_p == seen_ids.end() )
						{
							// future exploration
							parent_syms.push( wmes[i]->value );
							parent_ids.push( wmes[i]->value->id.epmem_id );
						}
					}
					else
					{
						// have we seen this node in this database?
						if ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) )
						{
							wmes[i]->epmem_id = NULL;
							wmes[i]->epmem_valid = my_agent->epmem_validation;

							my_hash = epmem_temporal_hash( my_agent, wmes[i]->attr );
							my_hash2 = epmem_temporal_hash( my_agent, wmes[i]->value );

							// try to get node id
							{
								// parent_id=? AND attr=? AND value=?
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 1, parent_id );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 2, my_hash );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, my_hash2 );

								if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
									wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 0 );

								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] );
							}

							// act depending on new/existing feature
							if ( wmes[i]->epmem_id == NULL )
							{
								// insert (parent_id,attr,value)
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 1, parent_id );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 2, my_hash );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 3, my_hash2 );

								sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] );
								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] );

								wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );

								// new nodes definitely start
								epmem_node.push( wmes[i]->epmem_id );
								my_agent->epmem_node_mins->push_back( time_counter );
								my_agent->epmem_node_maxes->push_back( false );
							}
							else
							{
								// definitely don't remove
								(*my_agent->epmem_node_removals)[ wmes[i]->epmem_id ] = false;

								// add ONLY if the last thing we did was add
								if ( (*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] )
								{
									epmem_node.push( wmes[i]->epmem_id );
									(*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] = false;
								}
							}
						}
					}
				}

				// free space from aug list
				free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
			}
		}

		// all inserts
		{
			epmem_node_id *temp_node;

			// nodes
			while ( !epmem_node.empty() )
			{
				temp_node =& epmem_node.front();

				// add NOW entry
				// id = ?, start = ?
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_NOW ], 1, (*temp_node) );
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_NOW ], 2, time_counter );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_NOW ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_NOW ] );

				// update min
				(*my_agent->epmem_node_mins)[ (*temp_node) - 1 ] = time_counter;

				epmem_node.pop();
			}

			// edges
			while ( !epmem_edge.empty() )
			{
				temp_node =& epmem_edge.front();

				// add NOW entry
				// id = ?, start = ?
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_NOW ], 1, (*temp_node) );
				EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_NOW ], 2, time_counter );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_NOW ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_NOW ] );

				// update min
				(*my_agent->epmem_edge_mins)[ (*temp_node) - 1 ] = time_counter;

				epmem_edge.pop();
			}
		}

		// all removals
		{
			std::map<epmem_node_id, bool>::iterator r;
			epmem_time_id range_start;
			epmem_time_id range_end;

			// nodes
			r = my_agent->epmem_node_removals->begin();
			while ( r != my_agent->epmem_node_removals->end() )
			{
				if ( r->second )
				{
					// remove NOW entry
					// id = ?
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_NODE_NOW ], 1, r->first );
					sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_NODE_NOW ] );
					sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_NODE_NOW ] );

					range_start = (*my_agent->epmem_node_mins)[ r->first - 1 ];
					range_end = ( time_counter - 1 );

					// point (id, start)
					if ( range_start == range_end )
					{
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ], 1, r->first );
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ], 2, range_start );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_POINT ] );
					}
					// node
					else
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );

					// update max
					(*my_agent->epmem_node_maxes)[ r->first - 1 ] = true;
				}

				r++;
			}
			my_agent->epmem_node_removals->clear();

			// edges
			r = my_agent->epmem_edge_removals->begin();
			while ( r != my_agent->epmem_edge_removals->end() )
			{
				if ( r->second )
				{
					// remove NOW entry
					// id = ?
					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_EDGE_NOW ], 1, r->first );
					sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_EDGE_NOW ] );
					sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_DELETE_EDGE_NOW ] );

					range_start = (*my_agent->epmem_edge_mins)[ r->first - 1 ];
					range_end = ( time_counter - 1 );

					// point (id, start)
					if ( range_start == range_end )
					{
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ], 1, r->first );
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ], 2, range_start );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_POINT ] );
					}
					// node
					else
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

					// update max
					(*my_agent->epmem_edge_maxes)[ r->first - 1 ] = true;
				}

				r++;
			}
			my_agent->epmem_edge_removals->clear();
		}

		// add the time id to the times table
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ], 1, time_counter );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ] );

		my_agent->epmem_stats->time->set_value( time_counter + 1 );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->storage->stop();	
	////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Non-Cue-Based Retrieval Functions (epmem::ncb)
//
// NCB retrievals occur when you know the episode you
// want to retrieve.  It is the process of converting
// the database representation to WMEs in working
// memory.
//
// This occurs at the end of a cue-based query, or
// in response to a retrieve/next/previous command.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_valid_episode
 * Author		: Nate Derbinsky
 * Notes		: Returns true if the temporal id is valid
 **************************************************************************/
bool epmem_valid_episode( agent *my_agent, epmem_time_id memory_id )
{
	const long mode = my_agent->epmem_params->mode->get_value();
	bool return_val = false;

	if ( mode == epmem_param_container::tree )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ] );
		return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ] );
	}
	else if ( mode == epmem_param_container::graph )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ] );
		return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ] );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_install_memory
 * Author		: Nate Derbinsky
 * Notes		: Reconstructs an episode in working memory.
 *
 * 				  Use RIT to collect appropriate ranges.  Then
 * 				  combine with NOW and POINT.  Merge with unique
 * 				  to get all the data necessary to create WMEs.
 *
 * 				  The id_reuse parameter is only used in the case
 * 				  that the graph-match has a match and creates
 * 				  a mapping of identifiers that should be re-used
 * 				  during reconstruction.
 **************************************************************************/
void epmem_install_memory( agent *my_agent, Symbol *state, epmem_time_id memory_id, epmem_id_mapping *id_reuse = NULL )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->ncb_retrieval->start();	
	////////////////////////////////////////////////////////////////////////////

	// get the ^result header for this state
	Symbol *result_header = state->id.epmem_result_header;

	// initialize stat
	long num_wmes = 0;
	my_agent->epmem_stats->ncb_wmes->set_value( num_wmes );

	// if no memory, say so
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_retrieved_symbol, my_agent->epmem_no_memory_symbol );
		state->id.epmem_info->last_memory = EPMEM_MEMID_NONE;

		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->ncb_retrieval->stop();
		////////////////////////////////////////////////////////////////////////////
		
		return;
	}

	// remember this as the last memory installed
	state->id.epmem_info->last_memory = memory_id;

	// create a new ^retrieved header for this result
	Symbol *retrieved_header = make_new_identifier( my_agent, 'R', result_header->id.level );
	epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_retrieved_symbol, retrieved_header );	
	symbol_remove_ref( my_agent, retrieved_header );

	// add *-id wme's
	{
		Symbol *my_meta;

		my_meta = make_int_constant( my_agent, memory_id );
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_memory_id_symbol, my_meta );		
		symbol_remove_ref( my_agent, my_meta );

		my_meta = make_int_constant( my_agent, my_agent->epmem_stats->time->get_value() );
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_present_id_symbol, my_meta );		
		symbol_remove_ref( my_agent, my_meta );
	}

	const long mode = my_agent->epmem_params->mode->get_value();

	if ( mode == epmem_param_container::tree )
	{
		epmem_id_mapping ids;
		epmem_node_id child_id;
		epmem_node_id parent_id;
		long attr_type;
		long value_type;
		Symbol *attr = NULL;
		Symbol *value = NULL;
		Symbol *parent = NULL;

		ids[ 0 ] = retrieved_header;

		epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &epmem_rit_state_tree );

		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 1, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 2, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 3, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 4, memory_id );
		while ( epmem_exec_query( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], my_agent->epmem_timers->ncb_node ) == SQLITE_ROW )
		{
			// e.id, i.parent_id, i.name, i.value, i.attr_type, i.value_type
			child_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 0 );
			parent_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 1 );
			attr_type = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 4 );
			value_type = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 5 );

			// make a symbol to represent the attribute name
			switch ( attr_type )
			{
				case INT_CONSTANT_SYMBOL_TYPE:
					attr = make_int_constant( my_agent, EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 2 ) );
					break;

				case FLOAT_CONSTANT_SYMBOL_TYPE:
					attr = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 2 ) );
					break;

				case SYM_CONSTANT_SYMBOL_TYPE:
					attr = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 2 ) ) );
					break;
			}

			// get a reference to the parent
			parent = ids[ parent_id ];

			// identifier = NULL, else attr->val
			if ( value_type == IDENTIFIER_SYMBOL_TYPE )
			{
				value = make_new_identifier( my_agent, ( ( attr_type == SYM_CONSTANT_SYMBOL_TYPE )?( attr->sc.name[0] ):('E') ), parent->id.level );
				epmem_add_retrieved_wme( my_agent, state, parent, attr, value );

				ids[ child_id ] = value;
			}
			else
			{
				switch ( value_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						value = make_int_constant( my_agent, EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 3 ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						value = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 3 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						value = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 3 ) ) );
						break;
				}

				epmem_add_retrieved_wme( my_agent, state, parent, attr, value );				
				num_wmes++;
			}

			symbol_remove_ref( my_agent, attr );
			symbol_remove_ref( my_agent, value );
		}
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ] );

		epmem_rit_clear_left_right( my_agent );
	}
	else if ( mode == epmem_param_container::graph )
	{
		// Big picture: create identifier skeleton, then hang non-identifers
		//
		// Because of shared WMEs at different levels of the storage breadth-first search,
		// there is the possibility that the unique database id of an identifier can be
		// greater than that of its parent.  Because the retrieval query sorts by
		// unique id ascending, it is thus possible to have an "orphan" - a child with
		// no current parent.  We keep track of orphans and add them later, hoping their
		// parents have shown up.  I *suppose* there could be a really evil case in which
		// the ordering of the unique ids is exactly opposite of their insertion order.
		// I just hope this isn't a common case...

		// shared identifier lookup table
		epmem_id_mapping ids;
		bool existing_identifier = false;

		// symbols used to create WMEs
		Symbol *parent = NULL;
		Symbol *attr = NULL;

		// initialize the lookup table
		if ( id_reuse )
			ids = (*id_reuse);

		// first identifiers (i.e. reconstruct)
		ids[ 0 ] = retrieved_header;
		{
			// relates to finite automata: q1 = d(q0, w)
			epmem_node_id q0; // id
			epmem_node_id q1; // attribute
			Symbol **value = NULL; // value
			long w_type; // we support any constant attribute symbol

			// used to lookup shared identifiers
			epmem_id_mapping::iterator id_p;

			// orphaned children
			std::queue<epmem_edge *> orphans;
			epmem_edge *orphan;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 1, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 2, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 3, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 4, memory_id );
			while ( epmem_exec_query( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], my_agent->epmem_timers->ncb_edge ) == SQLITE_ROW )
			{
				// q0, w, q1, w_type
				q0 = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 0 );
				q1 = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 2 );
				w_type = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 3 );

				switch ( w_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						attr = make_int_constant( my_agent, EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 1 ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						attr = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 1 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						attr = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 1 ) ) );
						break;
				}

				// get a reference to the parent
				id_p = ids.find( q0 );
				if ( id_p != ids.end() )
				{
					parent = id_p->second;

					value =& ids[ q1 ];
					existing_identifier = ( (*value) != NULL );

					if ( !existing_identifier )
						(*value) = make_new_identifier( my_agent, ( ( w_type == SYM_CONSTANT_SYMBOL_TYPE )?( attr->sc.name[0] ):('E') ), parent->id.level );

					epmem_add_retrieved_wme( my_agent, state, parent, attr, (*value) );
					num_wmes++;

					symbol_remove_ref( my_agent, attr );

					if ( !existing_identifier )
						symbol_remove_ref( my_agent, (*value) );
				}
				else
				{
					// out of order
					orphan = new epmem_edge;
					orphan->q0 = q0;
					orphan->w = attr;
					orphan->q1 = q1;

					orphans.push( orphan );
				}
			}
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ] );
			epmem_rit_clear_left_right( my_agent );

			// take care of any orphans
			while ( !orphans.empty() )
			{
				orphan = orphans.front();
				orphans.pop();

				// get a reference to the parent
				id_p = ids.find( orphan->q0 );
				if ( id_p != ids.end() )
				{
					parent = id_p->second;

					value =& ids[ orphan->q1 ];
					existing_identifier = ( (*value) != NULL );

					if ( !existing_identifier )
						(*value) = make_new_identifier( my_agent, ( ( attr->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE )?( attr->sc.name[0] ):('E') ), parent->id.level );

					epmem_add_retrieved_wme( my_agent, state, parent, orphan->w, (*value) );
					num_wmes++;

					symbol_remove_ref( my_agent, orphan->w );

					if ( !existing_identifier )
						symbol_remove_ref( my_agent, (*value) );

					delete orphan;
				}
				else
				{
					orphans.push( orphan );
				}
			}
		}

		// then node_unique
		{
			epmem_node_id child_id;
			epmem_node_id parent_id;
			long attr_type;
			long value_type;

			Symbol *value = NULL;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );

			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 1, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 2, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 3, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 4, memory_id );
			while ( epmem_exec_query( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], my_agent->epmem_timers->ncb_node ) == SQLITE_ROW )
			{
				// f.child_id, f.parent_id, f.name, f.value, f.attr_type, f.value_type
				child_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 0 );
				parent_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 1 );
				attr_type = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 4 );
				value_type = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 5 );

				// get a reference to the parent
				parent = ids[ parent_id ];

				// make a symbol to represent the attribute
				switch ( attr_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						attr = make_int_constant( my_agent, EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 2 ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						attr = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 2 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						attr = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 2 ) ) );
						break;
				}

				// make a symbol to represent the value
				switch ( value_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						value = make_int_constant( my_agent, EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 3 ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						value = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 3 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						value = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 3 ) ) );
						break;
				}

				epmem_add_retrieved_wme( my_agent, state, parent, attr, value );
				num_wmes++;

				symbol_remove_ref( my_agent, attr );
				symbol_remove_ref( my_agent, value );
			}

			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ] );
			epmem_rit_clear_left_right( my_agent );
		}
	}

	// adjust stat
	my_agent->epmem_stats->ncb_wmes->set_value( num_wmes );

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->ncb_retrieval->stop();
	////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : epmem_next_episode
 * Author		: Nate Derbinsky
 * Notes		: Returns the next valid temporal id.  This is really
 * 				  only an issue if you implement episode dynamics like
 * 				  forgetting.
 **************************************************************************/
epmem_time_id epmem_next_episode( agent *my_agent, epmem_time_id memory_id )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->next->start();
	////////////////////////////////////////////////////////////////////////////

	const long mode = my_agent->epmem_params->mode->get_value();
	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( mode == epmem_param_container::tree )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ] );
	}
	else if ( mode == epmem_param_container::graph )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ] );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->next->stop();
	////////////////////////////////////////////////////////////////////////////

	return return_val;
}

/***************************************************************************
 * Function     : epmem_previous_episode
 * Author		: Nate Derbinsky
 * Notes		: Returns the last valid temporal id.  This is really
 * 				  only an issue if you implement episode dynamics like
 * 				  forgetting.
 **************************************************************************/
epmem_time_id epmem_previous_episode( agent *my_agent, epmem_time_id memory_id )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->prev->start();	
	////////////////////////////////////////////////////////////////////////////

	const long mode = my_agent->epmem_params->mode->get_value();
	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( mode == epmem_param_container::tree )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ] );
	}
	else if ( mode == epmem_param_container::graph )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ] );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->prev->stop();
	////////////////////////////////////////////////////////////////////////////

	return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Cue-Based Retrieval (epmem::cbr)
//
// Cue-based retrievals are searches in response to
// queries and/or neg-queries.
//
// All functions below implement John/Andy's range search
// algorithm (see Andy's thesis).  The primary insight
// is to only deal with changes.  In this case, change
// occurs at the end points of ranges of node occurrence.
//
// The implementations below share a common theme:
// 1) identify wmes in the cue
// 2) get pointers to ALL b-tree leaves
//    associated with sorted occurrence-endpoints
//    of these wmes (ranges, points, now)
// 3) step through the leaves according to the range
//    search algorithm
//
// In the Working Memory Tree, the occurrence of a leaf
// node is tantamount to the occurrence of the path to
// the leaf node (since there are no shared identifiers).
// However, when we add graph functionality, path is
// important.  Moreover, identifiers that "blink" have
// ambiguous identities over time.  Thus I introduced
// the Disjunctive Normal Form (DNF) graph.
//
// The primary insight of the DNF graph is that paths to
// leaf nodes can be written as the disjunction of the
// conjunction of paths.
//
// Metaphor: consider that a human child's lineage is
// in question (perhaps for purposes of estate).  We
// are unsure as to the child's grandfather.  The grand-
// father can be either gA or gB.  If it is gA, then the
// father is absolutely fA (otherwise fB).  So the child
// could exist as (where cX is child with lineage X):
//
// (gA ^ fA ^ cA) \/ (gB ^ fB ^ cB)
//
// Note that due to family... irregularities
// (i.e. men sleeping around), a parent might contribute
// to multiple family lines.  Thus gX could exist in
// multiple clauses.  However, due to well-enforced
// incest laws (i.e. we only support acyclic graph cues),
// an individual can only occur once within a lineage/clause.
//
// We have a "match" (i.e. identify the child's lineage)
// only if all literals are "on" in a path of
// lineage.  Thus, our task is to efficiently track DNF
// satisfaction while flipping on/off a single literal
// (which may exist in multiple clauses).
//
// The DNF graph implements this intuition efficiently by
// (say it with me) only processing changes!  First we
// construct the graph by creating "literals" (gA, fA, etc)
// and maintaining parent-child relationships (gA connects
// to fA which connects to cA).  Leaf literals have no
// children, but are associated with a "match."  Each match
// has a cardinality count (positive/negative 1 depending on
// query vs. neg-query) and a WMA value (weighting).
//
// We then connect the b-tree pointers from above with
// each of the literals.  It is possible that a query
// can serve multiple literals, so we gain from sharing.
//
// Nodes within the DNF Graph need only save a "count":
// zero means neither it nor its lineage to date is
// satisfied, one means either its lineage or it is
// satisfied, two means it and its lineage is satisfied.
//
// The range search algorithm is simply walking (in-order)
// the parallel b-tree pointers.  When we get to an endpoint,
// we appropriately turn on/off all directly associated
// literals.  Based upon the current state of the literals,
// we may need to propagate changes to children.
//
// If propogation reaches and changes a match, we alter the
// current episode's cardinality/score.  Thus we achieve
// the Soar mantra of only processing changes!
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_create_leaf_node
 * Author		: Nate Derbinsky
 * Notes		: Just a quicky constructor
 **************************************************************************/
epmem_leaf_node *epmem_create_leaf_node( epmem_node_id leaf_id, double leaf_weight )
{
	epmem_leaf_node *newbie = new epmem_leaf_node();

	newbie->leaf_id = leaf_id;
	newbie->leaf_weight = leaf_weight;

	return newbie;
}

/***************************************************************************
 * Function     : epmem_incremental_row
 * Author		: Nate Derbinsky
 * Notes		: Implements a step in the range search algorithm for
 * 				  WM Tree (see above description).
 **************************************************************************/
void epmem_incremental_row( epmem_range_query_list *queries, epmem_time_id &id, long &ct, double &v, long &updown, const unsigned int list )
{
	// initialize variables
	id = queries[ list ].top()->val;
	ct = 0;
	v = 0;
	updown = 0;

	bool more_data = false;
	epmem_range_query *temp_query = NULL;

	// a step continues until we run out
	// of endpoints or we get to a new
	// endpoint
	do
	{
		// get the next range endpoint
		temp_query = queries[ list ].top();
		queries[ list ].pop();

		// update current state
		updown++;
		v += temp_query->weight;
		ct += temp_query->ct;

		// check if more endpoints exist for this leaf wme
		more_data = ( epmem_exec_range_query( temp_query ) == SQLITE_ROW );
		if ( more_data )
		{
			// if so, add back to the priority queue
			temp_query->val = EPMEM_SQLITE_COLUMN_INT( temp_query->stmt, 0 );
			queries[ list ].push( temp_query );
		}
		else
		{
			// else, away with ye
			sqlite3_finalize( temp_query->stmt );
			delete temp_query;
		}
	} while ( ( !queries[ list ].empty() ) && ( queries[ list ].top()->val == id ) );

	if ( list == EPMEM_RANGE_START )
		updown = -updown;
}

/***************************************************************************
 * Function     : epmem_shared_flip
 * Author		: Nate Derbinsky
 * Notes		: Implements flipping a literal in the DNF Graph
 * 				  (see above description).
 **************************************************************************/
void epmem_shared_flip( epmem_shared_literal *flip, const unsigned int list, long &ct, double &v, long &updown )
{
	if ( list == EPMEM_RANGE_START )
	{
		if ( ( --flip->ct ) == ( flip->max - 1 ) )
		{
			if ( flip->children )
			{
				epmem_shared_literal_list::iterator literal_p;

				for ( literal_p=flip->children->literals->begin(); literal_p!=flip->children->literals->end(); literal_p++ )
					epmem_shared_flip( (*literal_p), list, ct, v, updown );
			}
			else if ( flip->match )
			{
				if ( !( --flip->match->ct ) )
				{
					ct += flip->match->value_ct;
					v += flip->match->value_weight;

					updown++;
				}
			}
		}
	}
	else if ( list == EPMEM_RANGE_END )
	{
		if ( ( ++flip->ct ) == flip->max )
		{
			if ( flip->children )
			{
				epmem_shared_literal_list::iterator literal_p;

				for ( literal_p=flip->children->literals->begin(); literal_p!=flip->children->literals->end(); literal_p++ )
					epmem_shared_flip( (*literal_p), list, ct, v, updown );
			}
			else if ( flip->match )
			{
				if ( !( flip->match->ct++ ) )
				{
					ct += flip->match->value_ct;
					v += flip->match->value_weight;

					updown++;
				}
			}
		}
	}
}

/***************************************************************************
 * Function     : epmem_shared_increment
 * Author		: Nate Derbinsky
 * Notes		: Implements a step in the range search algorithm for
 * 				  WM Graph (see above description).
 **************************************************************************/
void epmem_shared_increment( epmem_shared_query_list *queries, epmem_time_id &id, long &ct, double &v, long &updown, const unsigned int list )
{
	// initialize variables
	id = queries[ list ].top()->val;
	ct = 0;
	v = 0;
	updown = 0;

	bool more_data;
	epmem_shared_query *temp_query;
	epmem_shared_literal_list::iterator literal_p;

	// a step continues until we run out
	// of endpoints or we get to a new
	// endpoint
	do
	{
		// get the next range endpoint
		temp_query = queries[ list ].top();
		queries[ list ].pop();

		// update current state by flipping all associated literals
		for ( literal_p=temp_query->triggers->begin(); literal_p!=temp_query->triggers->end(); literal_p++ )
			epmem_shared_flip( (*literal_p), list, ct, v, updown );

		// check if more endpoints exist for this wme
		more_data = ( epmem_exec_shared_query( temp_query ) == SQLITE_ROW );
		if ( more_data )
		{
			// if so, add back to the priority queue
			temp_query->val = EPMEM_SQLITE_COLUMN_INT( temp_query->stmt, 0 );
			queries[ list ].push( temp_query );
		}
		else
		{
			// away with ye
			sqlite3_finalize( temp_query->stmt );
			delete temp_query;
		}
	} while ( ( !queries[ list ].empty() ) && ( queries[ list ].top()->val == id ) );

	if ( list == EPMEM_RANGE_START )
		updown = -updown;
}

/***************************************************************************
 * Function     : epmem_graph_match
 * Author		: Nate Derbinsky
 * Notes		: Performs true acyclic graph match against the cue.
 * 				  Essentially CSP backtracking where shared identifier
 * 				  identities (database id) form the constraints.
 *
 * 				  Big picture:
 * 				  proceed recursively down the lineage of a literal
 * 				  if the branch is not satisfied
 * 				    if more literals exist for this wme
 * 				      try the next literal
 * 				    else
 * 				      failure
 * 				  else
 * 				    note constraining shared identifiers
 * 				    if no more wme literals exist
 * 				      potential success: return
 * 				    else
 * 				      repeat, see how far constraints survive
 * 				      if unable to finish with constraints, backtrack
 *
 * 				  Notes:
 * 				    - takes advantage of the ordering of literals (literals
 * 				      of the same WME are grouped together)
 * 					- recursion across the WMEs within the provided list is
 * 				      implemented with stacks; DFS recursion through the
 * 				      DNF graph is handled through function calls
 *
 **************************************************************************/
unsigned long epmem_graph_match( epmem_shared_literal_group *literals, epmem_constraint_list *constraints )
{
	// number of satisfied leaf WMEs in this list
	unsigned long return_val = 0;

	// stacks to maintain state within the list
	std::stack<epmem_shared_literal_list::size_type> c_ps; // literal pointers (position within a WME)
	std::stack<epmem_constraint_list *> c_cs; // constraints (previously assumed correct)
	std::stack<epmem_node_id> c_ids; // shared id of the current wme

	// literals are grouped together sequentially by WME.
	epmem_shared_wme_list::iterator c_f;

	// current values from the stacks
	epmem_shared_literal_list::size_type c_p;
	epmem_constraint_list *c_c;
	epmem_node_id c_id;

	// derived values from current values
	epmem_shared_literal *c_l;

	// used to propogate constraints without committing prematurely
	epmem_constraint_list *n_c;

	// used for constraint lookups
	epmem_constraint_list::iterator c;

	bool done = false;
	bool good_literal = false;
	bool good_pop = false;

	// shouldn't ever happen
	if ( !literals->literals->empty() )
	{
		// initialize to the beginning of the list
		c_p = 0;
		c_l = literals->literals->front();
		c_f = literals->wmes->begin();
		literals->c_wme = c_l->wme;

		// current constraints = previous constraints
		c_c = new epmem_constraint_list( *constraints );

		// get constraint for this wme, if exists
		c_id = EPMEM_NODEID_ROOT;
		if ( c_l->wme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
		{
			c = c_c->find( c_l->wme->value );
			if ( c != c_c->end() )
				c_id = c->second;
		}

		do
		{
			// determine if literal is a match
			{
				good_literal = false;

				// must be ON
				if ( c_l->ct == c_l->max )
				{
					// cue identifier
					if ( c_l->shared_id != EPMEM_NODEID_ROOT )
					{
						// check if unconstrained
						if ( c_id == EPMEM_NODEID_ROOT )
						{
							// if substructure, check
							if ( c_l->children )
							{
								// copy constraints
								n_c = new epmem_constraint_list( *c_c );

								// try DFS
								if ( epmem_graph_match( c_l->children, n_c ) == c_l->children->wmes->size() )
								{
									// on success, keep new constraints
									good_literal = true;
									(*c_c) = (*n_c);

									// update constraints with this literal
									(*c_c)[ c_l->wme->value ] = c_l->shared_id;
								}

								delete n_c;
							}
							// otherwise winner by default, pass along constraint
							else
							{
								good_literal = true;
								(*c_c)[ c_l->wme->value ] = c_l->shared_id;
							}
						}
						else
						{
							// if shared identifier, we don't need to perform recursion
							// (we rely upon previous results)
							good_literal = ( c_id == c_l->shared_id );
						}
					}
					// leaf node, non-identifier
					else
					{
						good_literal = ( c_l->match->ct != 0 );
					}
				}
			}

			if ( good_literal )
			{
				// update number of unified wmes
				return_val++;

				// proceed to next wme
				c_f++;

				// yippee (potential success)
				if ( c_f == literals->wmes->end() )
				{
					done = true;
				}
				else
				// push, try next wme with new constraints
				if ( !done )
				{
					c_ps.push( c_p );
					c_cs.push( c_c );
					c_ids.push( c_id );

					c_p = (*c_f);
					c_l = (*literals->literals)[ c_p ];
					literals->c_wme = c_l->wme;

					c_c = new epmem_constraint_list( *c_c );

					// get constraint for this wme, if exists
					c_id = EPMEM_NODEID_ROOT;
					if ( c_l->wme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						c = c_c->find( c_l->wme->value );
						if ( c != c_c->end() )
							c_id = c->second;
					}
				}
			}
			else
			{
				// if a wme fails we try the next literal in this wme.
				// if there is no next, the constraints don't work and
				// we backtrack.

				good_pop = false;
				do
				{
					// increment
					c_p++;

					// if end of the road, failure
					if ( c_p >= literals->literals->size() )
					{
						done = true;
					}
					else
					{
						// else, look at the literal
						c_l = (*literals->literals)[ c_p ];

						// if still within the wme, we can try again
						// with current constraints
						if ( c_l->wme == literals->c_wme )
						{
							good_pop = true;
						}
						else
						{
							// if nothing left on the stack, failure
							if ( c_ps.empty() )
							{
								done = true;
							}
							else
							{
								// otherwise, backtrack:
								// - pop previous state
								// - repeat trying to increment (and possibly have to recursively pop again)

								c_p = c_ps.top();
								c_ps.pop();

								delete c_c;
								c_c = c_cs.top();
								c_cs.pop();

								c_id = c_ids.top();
								c_ids.pop();

								c_f--;

								return_val--;
							}
						}
					}
				} while ( !good_pop && !done );
			}
		} while ( !done );

		// pass on new constraints
		(*constraints) = (*c_c);
	}

	// clean up
	{
		if ( c_c )
			delete c_c;

		// we've been dynamically creating new
		// constraint pointers all along
		while ( !c_cs.empty() )
		{
			c_c = c_cs.top();
			c_cs.pop();

			delete c_c;
		}
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_process_query
 * Author		: Nate Derbinsky
 * Notes		: Performs cue-based query (see section description above).
 *
 * 				  The level parameter should be used only for profiling
 * 				  in experimentation:
 * 				  - level 3 (default): full query processing
 * 				  - level 2: no installing of found memory
 * 				  - level 1: no interval search
 **************************************************************************/
void epmem_process_query( agent *my_agent, Symbol *state, Symbol *query, Symbol *neg_query, epmem_time_list *prohibit, epmem_time_id before, epmem_time_id after, int level=3 )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->query->start();
	////////////////////////////////////////////////////////////////////////////

	unsigned long len_query = 0, len_neg_query = 0;
	wme **wmes_query = NULL;
	if ( query != NULL )
		wmes_query = epmem_get_augs_of_id( my_agent, query, get_new_tc_number( my_agent ), &len_query );

	wme **wmes_neg_query = NULL;
	if ( neg_query != NULL )
		wmes_neg_query = epmem_get_augs_of_id( my_agent, neg_query, get_new_tc_number( my_agent ), &len_neg_query );

	// only perform a query if there potentially valid cue(s)
	if ( ( len_query != 0 ) || ( len_neg_query != 0 ) )
	{
		const long mode = my_agent->epmem_params->mode->get_value();

		if ( !prohibit->empty() )
			std::sort( prohibit->begin(), prohibit->end() );

		if ( mode == epmem_param_container::tree )
		{
			// BFS to get the leaf id's
			std::list<epmem_leaf_node *> leaf_ids[2];
			std::list<epmem_leaf_node *>::iterator leaf_p;
			epmem_time_list::iterator prohibit_p;
			{
				wme ***wmes;
				unsigned long len;

				std::queue<Symbol *> parent_syms;
				std::queue<epmem_node_id> parent_ids;
				std::queue<wme *> parent_wmes;
				int tc = get_new_tc_number( my_agent );

				Symbol *parent_sym;
				epmem_node_id parent_id;
				wme *parent_wme;

				// temporal hashing
				long my_hash;		// attribute
				long my_hash2;	// value

				int i;
				unsigned long j;
				bool just_started;

				// initialize pos/neg lists
				for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
				{
					switch ( i )
					{
						case EPMEM_NODE_POS:
							wmes = &wmes_query;
							len = len_query;
							parent_syms.push( query );
							parent_ids.push( EPMEM_NODEID_ROOT );
							parent_wmes.push( NULL );
							just_started = true;
							break;

						case EPMEM_NODE_NEG:
							wmes = &wmes_neg_query;
							len = len_neg_query;
							parent_syms.push( neg_query );
							parent_ids.push( EPMEM_NODEID_ROOT );
							parent_wmes.push( NULL );
							just_started = true;
							break;
					}

					while ( !parent_syms.empty() )
					{
						parent_sym = parent_syms.front();
						parent_syms.pop();

						parent_id = parent_ids.front();
						parent_ids.pop();

						parent_wme = parent_wmes.front();
						parent_wmes.pop();

						if ( !just_started )
							(*wmes) = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

						if ( (*wmes) != NULL )
						{
							if ( len )
							{
								for ( j=0; j<len; j++ )
								{
									// add to cue list
									state->id.epmem_info->cue_wmes->insert( (*wmes)[ j ] );

									// find wme id
									if ( (*wmes)[j]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
									{
										my_hash = epmem_temporal_hash( my_agent, (*wmes)[j]->attr );
										my_hash2 = epmem_temporal_hash( my_agent, (*wmes)[j]->value );

										// parent_id=? AND attr=? AND value=?
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 1, parent_id );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 2, my_hash );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, my_hash2 );

										if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
											leaf_ids[i].push_back( epmem_create_leaf_node( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 0 ), wma_get_wme_activation( my_agent, (*wmes)[j] ) ) );

										sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] );
									}
									else
									{
										my_hash = epmem_temporal_hash( my_agent, (*wmes)[j]->attr );

										// parent_id=? AND attr=?
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 1, parent_id );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 2, my_hash );

										if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] ) == SQLITE_ROW )
										{
											parent_syms.push( (*wmes)[j]->value );
											parent_ids.push( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 0 ) );
											parent_wmes.push( (*wmes)[j] );
										}

										sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] );
									}
								}
							}
							else
							{
								if ( !just_started )
									leaf_ids[i].push_back( epmem_create_leaf_node( parent_id, wma_get_wme_activation( my_agent, parent_wme ) ) );
							}

							// free space from aug list
							free_memory( my_agent, (*wmes), MISCELLANEOUS_MEM_USAGE );
						}

						just_started = false;
					}
				}
			}

			my_agent->epmem_stats->qry_pos->set_value( leaf_ids[ EPMEM_NODE_POS ].size() );
			my_agent->epmem_stats->qry_neg->set_value( leaf_ids[ EPMEM_NODE_NEG ].size() );
			my_agent->epmem_stats->qry_ret->set_value( 0 );
			my_agent->epmem_stats->qry_card->set_value( 0 );

			// useful statistics
			int cue_sizes[2] = { leaf_ids[ EPMEM_NODE_POS ].size(), leaf_ids[ EPMEM_NODE_NEG ].size() };
			int cue_size = ( cue_sizes[ EPMEM_NODE_POS ] + cue_sizes[ EPMEM_NODE_NEG ] );
			unsigned long perfect_match = leaf_ids[ EPMEM_NODE_POS ].size();

			// only perform search if necessary
			if ( ( level > 1 ) && cue_size )
			{
				// perform incremental, integrated range search
				{
					// variables to populate
					epmem_time_id king_id = EPMEM_MEMID_NONE;
					double king_score = -1000;
					unsigned long king_cardinality = 0;

					// prepare queries
					epmem_range_query_list *queries = new epmem_range_query_list[2];
					int i, j, k;
					{
						const char *tail;
						epmem_time_id time_now = my_agent->epmem_stats->time->get_value() - 1;
						int position;

						epmem_range_query *new_query = NULL;
						sqlite3_stmt *new_stmt = NULL;
						soar_module::timer *new_timer = NULL;

						for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
						{
							if ( cue_sizes[ i ] )
							{
								for ( leaf_p=leaf_ids[i].begin(); leaf_p!=leaf_ids[i].end(); leaf_p++ )
								{
									for ( j=EPMEM_RANGE_START; j<=EPMEM_RANGE_END; j++ )
									{
										for ( k=EPMEM_RANGE_EP; k<=EPMEM_RANGE_POINT; k++ )
										{
											switch ( k )
											{
												case EPMEM_RANGE_EP:
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_ep ):( my_agent->epmem_timers->query_pos_end_ep ) ):( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_ep ):( my_agent->epmem_timers->query_neg_end_ep ) ) );
													break;

												case EPMEM_RANGE_NOW:
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_now ):( my_agent->epmem_timers->query_pos_end_now ) ):( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_now ):( my_agent->epmem_timers->query_neg_end_now ) ) );
													break;

												case EPMEM_RANGE_POINT:
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_point ):( my_agent->epmem_timers->query_pos_end_point ) ):( ( j == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_point ):( my_agent->epmem_timers->query_neg_end_point ) ) );
													break;
											}

											// assign sql
											bool my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_NODE ][ j ][ k ], EPMEM_DB_PREP_STR_MAX, &new_stmt, &tail ) == SQLITE_OK );
											assert( my_assert );

											// bind values
											position = 1;

											if ( ( k == EPMEM_RANGE_NOW ) && ( j == EPMEM_RANGE_END ) )
												EPMEM_SQLITE_BIND_INT( new_stmt, position++, time_now );
											EPMEM_SQLITE_BIND_INT( new_stmt, position, (*leaf_p)->leaf_id );

											if ( epmem_exec_query( new_stmt, new_timer ) == SQLITE_ROW )
											{
												new_query = new epmem_range_query;
												new_query->val = EPMEM_SQLITE_COLUMN_INT( new_stmt, 0 );
												new_query->stmt = new_stmt;
												new_query->timer = new_timer;

												new_query->ct = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) );
												new_query->weight = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) ) * (*leaf_p)->leaf_weight;

												// add to query priority queue
												queries[ j ].push( new_query );
												new_query = NULL;
											}
											else
											{
												sqlite3_finalize( new_stmt );
											}

											new_stmt = NULL;
											new_timer = NULL;
										}
									}
								}
							}
						}
					}

					// perform range search
					{
						double balance = my_agent->epmem_params->balance->get_value();
						double balance_inv = 1 - balance;

						// dynamic programming stuff
						long sum_ct = 0;
						double sum_v = 0;
						long sum_updown = 0;

						// current pointer
						epmem_time_id current_id = EPMEM_MEMID_NONE;
						long current_ct = 0;
						double current_v = 0;
						long current_updown = 0;
						epmem_time_id current_end;
						epmem_time_id current_valid_end;
						double current_score;

						// next pointers
						epmem_time_id start_id = EPMEM_MEMID_NONE;
						epmem_time_id end_id = EPMEM_MEMID_NONE;
						epmem_time_id *next_id;
						unsigned int next_list;

						// prohibit pointer
						epmem_time_list::size_type current_prohibit = ( prohibit->size() - 1 );

						// completion (allows for smart cut-offs later)
						bool done = false;

						// initialize current as last end
						// initialize next end
						epmem_incremental_row( queries, current_id, current_ct, current_v, current_updown, EPMEM_RANGE_END );
						end_id = ( ( queries[ EPMEM_RANGE_END ].empty() )?( EPMEM_MEMID_NONE ):( queries[ EPMEM_RANGE_END ].top()->val ) );

						// initialize next start
						start_id = ( ( queries[ EPMEM_RANGE_START ].empty() )?( EPMEM_MEMID_NONE ):( queries[ EPMEM_RANGE_START ].top()->val ) );

						do
						{
							// if both lists are finished, we are done
							if ( ( start_id == EPMEM_MEMID_NONE ) && ( end_id == EPMEM_MEMID_NONE ) )
							{
								done = true;
							}
							// if we are beyond a specified after, we are done
							else if ( ( after != EPMEM_MEMID_NONE ) && ( current_id <= after ) )
							{
								done = true;
							}
							// if one list finished, go to the other
							else if ( ( start_id == EPMEM_MEMID_NONE ) || ( end_id == EPMEM_MEMID_NONE ) )
							{
								next_list = ( ( start_id == EPMEM_MEMID_NONE )?( EPMEM_RANGE_END ):( EPMEM_RANGE_START ) );
							}
							// if neither list finished, we prefer the higher id (end in case of tie)
							else
							{
								next_list = ( ( start_id > end_id )?( EPMEM_RANGE_START ):( EPMEM_RANGE_END ) );
							}

							if ( !done )
							{
								// update sums
								sum_ct += current_ct;
								sum_v += current_v;
								sum_updown += current_updown;

								// update end
								current_end = ( ( next_list == EPMEM_RANGE_END )?( end_id + 1 ):( start_id ) );
								if ( before == EPMEM_MEMID_NONE )
									current_valid_end = current_id;
								else
									current_valid_end = ( ( current_id < before )?( current_id ):( before - 1 ) );

								while ( ( current_prohibit != EPMEM_MEMID_NONE ) && ( current_valid_end >= current_end ) && ( current_valid_end <= (*prohibit)[ current_prohibit ] ) )
								{
									if ( current_valid_end == (*prohibit)[ current_prohibit ] )
										current_valid_end--;

									current_prohibit--;
								}

								// if we are beyond before AND
								// we are in a range, compute score
								// for possible new king
								if ( ( current_valid_end >= current_end ) && ( sum_updown != 0 ) )
								{
									current_score = ( balance * sum_ct ) + ( balance_inv * sum_v );

									// provide trace output
									if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
									{
										char buf[256];
										SNPRINTF( buf, 254, "CONSIDERING EPISODE (time, cardinality, score): (%d, %d, %f)", current_valid_end, sum_ct, current_score );

										print( my_agent, buf );
										xml_generate_warning( my_agent, buf );
									}

									// new king if no old king OR better score
									if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score > king_score ) )
									{
										king_id = current_valid_end;
										king_score = current_score;
										king_cardinality = sum_ct;

										// provide trace output
										if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
										{
											char buf[256];
											SNPRINTF( buf, 254, "NEW KING (perfect): (%s)", ( ( king_cardinality == perfect_match )?("true"):("false") ) );

											print( my_agent, buf );
											xml_generate_warning( my_agent, buf );
										}

										if ( king_cardinality == perfect_match )
											done = true;
									}
								}

								if ( !done )
								{
									// based upon choice, update variables
									epmem_incremental_row( queries, current_id, current_ct, current_v, current_updown, next_list );
									current_id = current_end - 1;
									current_ct *= ( ( next_list == EPMEM_RANGE_START )?( -1 ):( 1 ) );
									current_v *= ( ( next_list == EPMEM_RANGE_START )?( -1 ):( 1 ) );

									next_id = ( ( next_list == EPMEM_RANGE_START )?( &start_id ):( &end_id ) );
									(*next_id) = ( ( queries[ next_list ].empty() )?( EPMEM_MEMID_NONE ):( queries[ next_list ].top()->val ) );
								}
							}

						} while ( !done );
					}

					// cleanup
					{
						// leaf_ids
						for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
						{
							leaf_p = leaf_ids[i].begin();
							while ( leaf_p != leaf_ids[i].end() )
							{
								delete (*leaf_p);

								leaf_p++;
							}
						}

						// queries
						epmem_range_query *del_query;
						for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
						{
							while ( !queries[ i ].empty() )
							{
								del_query = queries[ i ].top();
								queries[ i ].pop();

								sqlite3_finalize( del_query->stmt );
								delete del_query;
							}
						}
						delete [] queries;
					}

					// place results in WM
					if ( king_id != EPMEM_MEMID_NONE )
					{
						Symbol *my_meta;

						my_agent->epmem_stats->qry_ret->set_value( king_id );
						my_agent->epmem_stats->qry_card->set_value( king_cardinality );

						// status
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );						

						// match score
						my_meta = make_float_constant( my_agent, king_score );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// cue-size
						my_meta = make_int_constant( my_agent, cue_size );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// normalized-match-score
						my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// match-cardinality
						my_meta = make_int_constant( my_agent, king_cardinality );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						////////////////////////////////////////////////////////////////////////////
						my_agent->epmem_timers->query->stop();
						////////////////////////////////////////////////////////////////////////////

						// actual memory
						if ( level > 2 )
							epmem_install_memory( my_agent, state, king_id );
					}
					else
					{
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );						

						////////////////////////////////////////////////////////////////////////////
						my_agent->epmem_timers->query->stop();
						////////////////////////////////////////////////////////////////////////////
					}
				}
			}
			else
			{
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );				

				////////////////////////////////////////////////////////////////////////////
				my_agent->epmem_timers->query->stop();
				////////////////////////////////////////////////////////////////////////////
			}
		}
		else if ( mode == epmem_param_container::graph )
		{
			// queries
			epmem_shared_query_list *queries = new epmem_shared_query_list[2];
			std::list<epmem_shared_literal_list *> trigger_lists;

			// match counters
			std::list<epmem_shared_match *> matches;

			// literals
			std::list<epmem_shared_literal *> literals;

			// graph match
			const long graph_match = my_agent->epmem_params->graph_match->get_value();
			epmem_shared_literal_group *graph_match_roots;
			if ( graph_match != soar_module::off )
			{
				graph_match_roots = new epmem_shared_literal_group();

				graph_match_roots->literals = new epmem_shared_literal_list();
				graph_match_roots->wmes = new epmem_shared_wme_list();
				graph_match_roots->c_wme = NULL;
			}

			unsigned long leaf_ids[2] = { 0, 0 };

			epmem_time_id time_now = my_agent->epmem_stats->time->get_value() - 1;

			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->query_dnf->start();
			////////////////////////////////////////////////////////////////////////////

			// simultaneously process cue, construct DNF graph, and add queries to priority cue
			// (i.e. prep for range search query)
			{
				// wme cache
				int tc = get_new_tc_number( my_agent );
				std::map<Symbol *, epmem_wme_cache_element *> wme_cache;
				epmem_wme_cache_element *current_cache_element;
				epmem_wme_cache_element **cache_hit;

				// literal mapping
				epmem_literal_mapping::iterator lit_map_p;
				bool shared_cue_id;

				// parent info
				std::queue<Symbol *> parent_syms;
				std::queue<epmem_node_id> parent_ids;
				std::queue<epmem_shared_literal *> parent_literals;

				Symbol *parent_sym;
				epmem_node_id parent_id;
				epmem_shared_literal *parent_literal;

				// misc query
				const char *tail;
				int position;

				// misc
				int i, k, m;
				unsigned long j;

				// associate common literals with a query
				std::map<epmem_node_id, epmem_shared_literal_list *> literal_to_node_query;
				std::map<epmem_node_id, epmem_shared_literal_list *> literal_to_edge_query;
				epmem_shared_literal_list **query_triggers;

				// associate common WMEs with a match
				std::map<wme *, epmem_shared_match *> wme_to_match;
				epmem_shared_match **wme_match;

				// temp new things
				epmem_shared_literal *new_literal = NULL;
				epmem_shared_match *new_match = NULL;
				epmem_shared_query *new_query = NULL;
				epmem_wme_cache_element *new_cache_element = NULL;
				epmem_shared_literal_list *new_trigger_list = NULL;
				epmem_shared_literal_group *new_literal_group = NULL;
				soar_module::timer *new_timer = NULL;
				sqlite3_stmt *new_stmt = NULL;

				// identity (i.e. database id)
				epmem_node_id unique_identity;
				epmem_node_id shared_identity;

				// temporal hashing
				long my_hash;		// attribute
				long my_hash2;	// value

				// fully populate wme cache (we need to know parent info a priori)
				{
					wme **starter_wmes;
					unsigned long starter_len;

					// query
					new_cache_element = new epmem_wme_cache_element;
					new_cache_element->len = len_query;
					new_cache_element->wmes = wmes_query;
					new_cache_element->parents = 0;
					new_cache_element->lits = new epmem_literal_mapping;
					wme_cache[ query ] = new_cache_element;
					new_cache_element = NULL;

					// negative query
					new_cache_element = new epmem_wme_cache_element;
					new_cache_element->len = len_neg_query;
					new_cache_element->wmes = wmes_neg_query;
					new_cache_element->parents = 0;
					new_cache_element->lits = new epmem_literal_mapping;
					wme_cache[ neg_query ] = new_cache_element;
					new_cache_element = NULL;

					for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
					{
						switch ( i )
						{
							case EPMEM_NODE_POS:
								starter_wmes = wmes_query;
								starter_len = len_query;
								break;

							case EPMEM_NODE_NEG:
								starter_wmes = wmes_neg_query;
								starter_len = len_neg_query;
								break;
						}

						if ( starter_len )
						{
							for ( j=0; j<starter_len; j++ )
							{
								if ( starter_wmes[j]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
								{
									parent_syms.push( starter_wmes[j]->value );
								}
							}

							while ( !parent_syms.empty() )
							{
								parent_sym = parent_syms.front();
								parent_syms.pop();

								cache_hit =& wme_cache[ parent_sym ];
								if ( (*cache_hit) )
								{
									(*cache_hit)->parents++;
								}
								else
								{
									new_cache_element = new epmem_wme_cache_element;
									new_cache_element->wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &( new_cache_element->len ) );
									new_cache_element->lits = new epmem_literal_mapping;
									new_cache_element->parents = 1;
									wme_cache[ parent_sym ] = new_cache_element;

									if ( new_cache_element->len )
									{
										for ( j=0; j<new_cache_element->len; j++ )
										{
											if ( new_cache_element->wmes[j]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
											{
												parent_syms.push( new_cache_element->wmes[j]->value );
											}
										}
									}

									new_cache_element = NULL;
								}
								cache_hit = NULL;
							}
							parent_sym = NULL;
						}
					}
				}

				// initialize pos/neg lists
				for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
				{
					switch ( i )
					{
						case EPMEM_NODE_POS:
							parent_syms.push( query );
							parent_ids.push( EPMEM_NODEID_ROOT );
							parent_literals.push( NULL );
							break;

						case EPMEM_NODE_NEG:
							parent_syms.push( neg_query );
							parent_ids.push( EPMEM_NODEID_ROOT );
							parent_literals.push( NULL );
							break;
					}

					while ( !parent_syms.empty() )
					{
						parent_sym = parent_syms.front();
						parent_syms.pop();

						parent_id = parent_ids.front();
						parent_ids.pop();

						parent_literal = parent_literals.front();
						parent_literals.pop();

						current_cache_element = wme_cache[ parent_sym ];

						if ( current_cache_element->len )
						{
							for ( j=0; j<current_cache_element->len; j++ )
							{
								// add to cue list
								state->id.epmem_info->cue_wmes->insert( current_cache_element->wmes[j] );

								if ( current_cache_element->wmes[j]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
								{
									my_hash = epmem_temporal_hash( my_agent, current_cache_element->wmes[j]->attr );

									// q0=? AND w=?
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 1, parent_id );
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 2, my_hash );

									while ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] ) == SQLITE_ROW )
									{
										// get identity
										unique_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 0 );
										shared_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 1 );

										// have we seen this identifier before?
										shared_cue_id = false;
										cache_hit =& wme_cache[ current_cache_element->wmes[j]->value ];
										lit_map_p = (*cache_hit)->lits->find( shared_identity );
										shared_cue_id = ( lit_map_p != (*cache_hit)->lits->end() );

										if ( shared_cue_id )
										{
											new_literal = lit_map_p->second;
										}
										else
										{
											// create new literal
											new_literal = new epmem_shared_literal;

											new_literal->ct = 0;
											new_literal->max = ( (*cache_hit)->parents * EPMEM_DNF );

											new_literal->shared_id = shared_identity;

											new_literal->wme = current_cache_element->wmes[j];
											new_literal->wme_kids = ( (*cache_hit)->len != 0 );

											new_literal->children = NULL;
											new_literal->match = NULL;

											literals.push_back( new_literal );
											(*(*cache_hit)->lits)[ shared_identity ] = new_literal;
										}
										cache_hit = NULL;

										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											// root is always on and satisfies one parental branch
											new_literal->ct++;

											// keep track of root literals for graph-match
											if ( ( !shared_cue_id ) && ( i == EPMEM_NODE_POS ) && ( graph_match != soar_module::off ) )
											{
												// enforce wme grouping
												if ( new_literal->wme != graph_match_roots->c_wme )
												{
													graph_match_roots->c_wme = new_literal->wme;
													graph_match_roots->wmes->push_back( graph_match_roots->literals->size() );
												}

												graph_match_roots->literals->push_back( new_literal );
											}
										}
										else
										{
											// if this is parent's first child we can use some good initial values
											if ( !parent_literal->children )
											{
												new_literal_group = new epmem_shared_literal_group();
												new_literal_group->literals = new epmem_shared_literal_list();
												new_literal_group->wmes = new epmem_shared_wme_list();

												new_literal_group->c_wme = new_literal->wme;
												new_literal_group->literals->push_back( new_literal );
												new_literal_group->wmes->push_back( 0 );

												parent_literal->children = new_literal_group;
												new_literal_group = NULL;
											}
											else
											{
												// otherwise, enforce wme grouping

												new_literal_group = parent_literal->children;

												if ( new_literal->wme != new_literal_group->c_wme )
												{
													new_literal_group->c_wme = new_literal->wme;
													new_literal_group->wmes->push_back( new_literal_group->literals->size() );
												}

												new_literal_group->literals->push_back( new_literal );
												new_literal_group = NULL;
											}
										}

										// create queries if necessary
										query_triggers =& literal_to_edge_query[ unique_identity ];
										if ( !(*query_triggers) )
										{
											new_trigger_list = new epmem_shared_literal_list;
											trigger_lists.push_back( new_trigger_list );

											// add all respective queries
											for ( k=EPMEM_RANGE_START; k<=EPMEM_RANGE_END; k++ )
											{
												for( m=EPMEM_RANGE_EP; m<=EPMEM_RANGE_POINT; m++ )
												{
													// assign timer
													switch ( m )
													{
														case EPMEM_RANGE_EP:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_ep ):( my_agent->epmem_timers->query_pos_end_ep ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_ep ):( my_agent->epmem_timers->query_neg_end_ep ) ) );
															break;

														case EPMEM_RANGE_NOW:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_now ):( my_agent->epmem_timers->query_pos_end_now ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_now ):( my_agent->epmem_timers->query_neg_end_now ) ) );
															break;

														case EPMEM_RANGE_POINT:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_point ):( my_agent->epmem_timers->query_pos_end_point ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_point ):( my_agent->epmem_timers->query_neg_end_point ) ) );
															break;
													}

													// assign sql
													bool my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_EDGE ][ k ][ m ], EPMEM_DB_PREP_STR_MAX, &new_stmt, &tail ) == SQLITE_OK );
													assert( my_assert );

													// bind values
													position = 1;

													if ( ( m == EPMEM_RANGE_NOW ) && ( k == EPMEM_RANGE_END ) )
														EPMEM_SQLITE_BIND_INT( new_stmt, position++, time_now );
													EPMEM_SQLITE_BIND_INT( new_stmt, position, unique_identity );

													// take first step
													if ( epmem_exec_query( new_stmt, new_timer ) == SQLITE_ROW )
													{
														new_query = new epmem_shared_query;
														new_query->val = EPMEM_SQLITE_COLUMN_INT( new_stmt, 0 );
														new_query->stmt = new_stmt;
														new_query->timer = new_timer;

														new_query->triggers = new_trigger_list;

														// add to query list
														queries[ k ].push( new_query );
														new_query = NULL;
													}
													else
													{
														sqlite3_finalize( new_stmt );
													}

													new_stmt = NULL;
													new_timer = NULL;
												}
											}

											(*query_triggers) = new_trigger_list;
											new_trigger_list = NULL;
										}
										(*query_triggers)->push_back( new_literal );

										if ( !shared_cue_id )
										{
											if ( new_literal->wme_kids )
											{
												parent_syms.push( new_literal->wme->value );
												parent_ids.push( shared_identity );
												parent_literals.push( new_literal );
											}
											else
											{
												// create match if necessary
												wme_match =& wme_to_match[ new_literal->wme ];
												if ( !(*wme_match) )
												{
													leaf_ids[i]++;

													new_match = new epmem_shared_match;
													matches.push_back( new_match );
													new_match->ct = 0;
													new_match->value_ct = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) );
													new_match->value_weight = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) ) * wma_get_wme_activation( my_agent, new_literal->wme );

													(*wme_match) = new_match;
													new_match = NULL;
												}
												new_literal->match = (*wme_match);
											}
										}

										new_literal = NULL;
									}
									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] );
								}
								else
								{
									my_hash = epmem_temporal_hash( my_agent, current_cache_element->wmes[j]->attr );
									my_hash2 = epmem_temporal_hash( my_agent, current_cache_element->wmes[j]->value );

									// parent_id=? AND attr=? AND value=?
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 1, parent_id );
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 2, my_hash );
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, my_hash2 );

									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
									{
										// get identity
										unique_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 0 );

										// create new literal
										new_literal = new epmem_shared_literal;
										new_literal->max = EPMEM_DNF;
										new_literal->shared_id = EPMEM_NODEID_ROOT;
										new_literal->wme_kids = 0;
										new_literal->wme = current_cache_element->wmes[j];
										new_literal->children = NULL;
										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											new_literal->ct = 1;

											if ( ( i == EPMEM_NODE_POS ) && ( graph_match != soar_module::off ) )
											{
												// only one literal/root non-identifier
												graph_match_roots->c_wme = new_literal->wme;
												graph_match_roots->wmes->push_back( graph_match_roots->literals->size() );
												graph_match_roots->literals->push_back( new_literal );
											}
										}
										else
										{
											new_literal->ct = 0;

											// if this is parent's first child we can use some good initial values
											if ( !parent_literal->children )
											{
												new_literal_group = new epmem_shared_literal_group();
												new_literal_group->literals = new epmem_shared_literal_list();
												new_literal_group->wmes = new epmem_shared_wme_list();

												new_literal_group->c_wme = new_literal->wme;
												new_literal_group->literals->push_back( new_literal );
												new_literal_group->wmes->push_back( 0 );

												parent_literal->children = new_literal_group;
												new_literal_group = NULL;
											}
											else
											{
												// else enforce wme grouping

												new_literal_group = parent_literal->children;

												if ( new_literal->wme != new_literal_group->c_wme )
												{
													new_literal_group->c_wme = new_literal->wme;
													new_literal_group->wmes->push_back( new_literal_group->literals->size() );
												}

												new_literal_group->literals->push_back( new_literal );
												new_literal_group = NULL;
											}
										}
										literals.push_back( new_literal );

										// create match if necessary
										wme_match =& wme_to_match[ current_cache_element->wmes[j] ];
										if ( !(*wme_match) )
										{
											leaf_ids[i]++;

											new_match = new epmem_shared_match;
											matches.push_back( new_match );
											new_match->ct = 0;
											new_match->value_ct = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) );
											new_match->value_weight = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) ) * wma_get_wme_activation( my_agent, current_cache_element->wmes[j] );

											(*wme_match) = new_match;
											new_match = NULL;
										}
										new_literal->match = (*wme_match);

										// create queries if necessary
										query_triggers =& literal_to_node_query[ unique_identity ];
										if ( !(*query_triggers) )
										{
											new_trigger_list = new epmem_shared_literal_list;
											trigger_lists.push_back( new_trigger_list );

											// add all respective queries
											for ( k=EPMEM_RANGE_START; k<=EPMEM_RANGE_END; k++ )
											{
												for( m=EPMEM_RANGE_EP; m<=EPMEM_RANGE_POINT; m++ )
												{
													// assign timer
													switch ( m )
													{
														case EPMEM_RANGE_EP:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_ep ):( my_agent->epmem_timers->query_pos_end_ep ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_ep ):( my_agent->epmem_timers->query_neg_end_ep ) ) );
															break;

														case EPMEM_RANGE_NOW:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_now ):( my_agent->epmem_timers->query_pos_end_now ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_now ):( my_agent->epmem_timers->query_neg_end_now ) ) );
															break;

														case EPMEM_RANGE_POINT:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_pos_start_point ):( my_agent->epmem_timers->query_pos_end_point ) ):( ( k == EPMEM_RANGE_START )?( my_agent->epmem_timers->query_neg_start_point ):( my_agent->epmem_timers->query_neg_end_point ) ) );
															break;
													}

													// assign sql
													bool my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_NODE ][ k ][ m ], EPMEM_DB_PREP_STR_MAX, &new_stmt, &tail ) == SQLITE_OK );
													assert( my_assert );

													// bind values
													position = 1;

													if ( ( m == EPMEM_RANGE_NOW ) && ( k == EPMEM_RANGE_END ) )
														EPMEM_SQLITE_BIND_INT( new_stmt, position++, time_now );
													EPMEM_SQLITE_BIND_INT( new_stmt, position, unique_identity );

													// take first step
													if ( epmem_exec_query( new_stmt, new_timer ) == SQLITE_ROW )
													{
														new_query = new epmem_shared_query;
														new_query->val = EPMEM_SQLITE_COLUMN_INT( new_stmt, 0 );
														new_query->stmt = new_stmt;
														new_query->timer = new_timer;

														new_query->triggers = new_trigger_list;

														// add to query list
														queries[ k ].push( new_query );
														new_query = NULL;
													}
													else
													{
														sqlite3_finalize( new_stmt );
													}

													new_stmt = NULL;
													new_timer = NULL;
												}
											}

											(*query_triggers) = new_trigger_list;
											new_trigger_list = NULL;
										}
										(*query_triggers)->push_back( new_literal );

										new_literal = NULL;
									}
									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] );
								}
							}
						}
					}
				}

				// clean up wme cache
				{
					std::map<Symbol *, epmem_wme_cache_element *>::iterator cache_p;
					for ( cache_p=wme_cache.begin(); cache_p!=wme_cache.end(); cache_p++ )
					{
						if ( cache_p->second->wmes )
							free_memory( my_agent, cache_p->second->wmes, MISCELLANEOUS_MEM_USAGE );

						delete cache_p->second->lits;

						delete cache_p->second;
					}
				}
			}

			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->query_dnf->stop();
			////////////////////////////////////////////////////////////////////////////

			my_agent->epmem_stats->qry_pos->set_value( leaf_ids[ EPMEM_NODE_POS ] );
			my_agent->epmem_stats->qry_neg->set_value( leaf_ids[ EPMEM_NODE_NEG ] );
			my_agent->epmem_stats->qry_ret->set_value( 0 );
			my_agent->epmem_stats->qry_card->set_value( 0 );
			my_agent->epmem_stats->qry_lits->set_value( literals.size() );

			// useful statistics
			int cue_size = ( leaf_ids[ EPMEM_NODE_POS ] + leaf_ids[ EPMEM_NODE_NEG ] );
			unsigned long perfect_match = leaf_ids[ EPMEM_NODE_POS ];

			// vars to set in range search
			epmem_time_id king_id = EPMEM_MEMID_NONE;
			double king_score = -1000;
			unsigned long king_cardinality = 0;
			unsigned long king_graph_match = 0;
			epmem_constraint_list king_constraints;

			// perform range search if any leaf wmes
			if ( ( level > 1 ) && cue_size )
			{
				double balance = my_agent->epmem_params->balance->get_value();
				double balance_inv = 1 - balance;

				// dynamic programming stuff
				long sum_ct = 0;
				double sum_v = 0;
				long sum_updown = 0;

				// current pointer
				epmem_time_id current_id = EPMEM_MEMID_NONE;
				long current_ct = 0;
				double current_v = 0;
				long current_updown = 0;
				epmem_time_id current_end;
				epmem_time_id current_valid_end;
				double current_score;

				// next pointers
				epmem_time_id start_id = EPMEM_MEMID_NONE;
				epmem_time_id end_id = EPMEM_MEMID_NONE;
				epmem_time_id *next_id;
				unsigned int next_list;

				// prohibit pointer
				epmem_time_list::size_type current_prohibit = ( prohibit->size() - 1 );

				// completion (allows for smart cut-offs later)
				bool done = false;

				// graph match
				unsigned long current_graph_match_counter = 0;
				epmem_constraint_list current_constraints;

				// initialize current as last end
				// initialize next end
				epmem_shared_increment( queries, current_id, current_ct, current_v, current_updown, EPMEM_RANGE_END );
				end_id = ( ( queries[ EPMEM_RANGE_END ].empty() )?( EPMEM_MEMID_NONE ):( queries[ EPMEM_RANGE_END ].top()->val ) );

				// initialize next start
				start_id = ( ( queries[ EPMEM_RANGE_START ].empty() )?( EPMEM_MEMID_NONE ):( queries[ EPMEM_RANGE_START ].top()->val ) );

				do
				{
					// if both lists are finished, we are done
					if ( ( start_id == EPMEM_MEMID_NONE ) && ( end_id == EPMEM_MEMID_NONE ) )
					{
						done = true;
					}
					// if we are beyond a specified after, we are done
					else if ( ( after != EPMEM_MEMID_NONE ) && ( current_id <= after ) )
					{
						done = true;
					}
					// if one list finished, go to the other
					else if ( ( start_id == EPMEM_MEMID_NONE ) || ( end_id == EPMEM_MEMID_NONE ) )
					{
						next_list = ( ( start_id == EPMEM_MEMID_NONE )?( EPMEM_RANGE_END ):( EPMEM_RANGE_START ) );
					}
					// if neither list finished, we prefer the higher id (end in case of tie)
					else
					{
						next_list = ( ( start_id > end_id )?( EPMEM_RANGE_START ):( EPMEM_RANGE_END ) );
					}

					if ( !done )
					{
						// update sums
						sum_ct += current_ct;
						sum_v += current_v;
						sum_updown += current_updown;

						// update end
						current_end = ( ( next_list == EPMEM_RANGE_END )?( end_id + 1 ):( start_id ) );
						if ( before == EPMEM_MEMID_NONE )
							current_valid_end = current_id;
						else
							current_valid_end = ( ( current_id < before )?( current_id ):( before - 1 ) );

						while ( ( current_prohibit != EPMEM_MEMID_NONE ) && ( current_valid_end >= current_end ) && ( current_valid_end <= (*prohibit)[ current_prohibit ] ) )
						{
							if ( current_valid_end == (*prohibit)[ current_prohibit ] )
								current_valid_end--;

							current_prohibit--;
						}

						// if we are beyond before AND
						// we are in a range, compute score
						// for possible new king
						if ( ( current_valid_end >= current_end ) && ( sum_updown != 0 ) )
						{
							current_score = ( balance * sum_ct ) + ( balance_inv * sum_v );

							// provide trace output
							if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
							{
								char buf[256];
								SNPRINTF( buf, 254, "CONSIDERING EPISODE (time, cardinality, score): (%d, %d, %f)", current_valid_end, sum_ct, current_score );

								print( my_agent, buf );
								xml_generate_warning( my_agent, buf );
							}

							if ( graph_match != soar_module::off )
							{
								// policy:
								// - king candidate MUST have AT LEAST king score
								// - perform graph match ONLY if cardinality is perfect
								// - ONLY stop if graph match is perfect

								if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score >= king_score ) )
								{
									if ( sum_ct == (long) perfect_match )
									{
										current_constraints.clear();

										////////////////////////////////////////////////////////////////////////////
										my_agent->epmem_timers->query_graph_match->start();
										////////////////////////////////////////////////////////////////////////////

										current_graph_match_counter = epmem_graph_match( graph_match_roots, &current_constraints );

										////////////////////////////////////////////////////////////////////////////
										my_agent->epmem_timers->query_graph_match->stop();
										////////////////////////////////////////////////////////////////////////////

										if ( ( king_id == EPMEM_MEMID_NONE ) ||
											 ( current_score > king_score ) ||
											 ( current_graph_match_counter == len_query ) )
										{
											king_id = current_valid_end;
											king_score = current_score;
											king_cardinality = sum_ct;
											king_graph_match = current_graph_match_counter;
											king_constraints = current_constraints;

											// provide trace output
											if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
											{
												char buf[256];
												SNPRINTF( buf, 254, "NEW KING (perfect, graph-match): (true, %s)", ( ( king_graph_match == len_query )?("true"):("false") ) );

												print( my_agent, buf );
												xml_generate_warning( my_agent, buf );
											}

											if ( king_graph_match == len_query )
												done = true;
										}
									}
									else
									{
										if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score > king_score ) )
										{
											king_id = current_valid_end;
											king_score = current_score;
											king_cardinality = sum_ct;
											king_graph_match = 0;

											// provide trace output
											if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
											{
												char buf[256];
												SNPRINTF( buf, 254, "NEW KING (perfect, graph-match): (false, false)" );

												print( my_agent, buf );
												xml_generate_warning( my_agent, buf );
											}
										}
									}
								}
							}
							else
							{
								// new king if no old king OR better score
								if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score > king_score ) )
								{
									king_id = current_valid_end;
									king_score = current_score;
									king_cardinality = sum_ct;

									// provide trace output
									if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
									{
										char buf[256];
										SNPRINTF( buf, 254, "NEW KING (perfect): (%s)", ( ( king_cardinality == perfect_match )?("true"):("false") ) );

										print( my_agent, buf );
										xml_generate_warning( my_agent, buf );
									}

									if ( king_cardinality == perfect_match )
										done = true;
								}
							}
						}

						if ( !done )
						{
							// based upon choice, update variables
							epmem_shared_increment( queries, current_id, current_ct, current_v, current_updown, next_list );
							current_id = current_end - 1;
							current_ct *= ( ( next_list == EPMEM_RANGE_START )?( -1 ):( 1 ) );
							current_v *= ( ( next_list == EPMEM_RANGE_START )?( -1 ):( 1 ) );

							next_id = ( ( next_list == EPMEM_RANGE_START )?( &start_id ):( &end_id ) );
							(*next_id) = ( ( queries[ next_list ].empty() )?( EPMEM_MEMID_NONE ):( queries[ next_list ].top()->val ) );
						}
					}

				} while ( !done );
			}

			// clean up
			{
				int i;

				// literals
				std::list<epmem_shared_literal *>::iterator literal_p;
				for ( literal_p=literals.begin(); literal_p!=literals.end(); literal_p++ )
				{
					if ( (*literal_p)->children )
					{
						delete (*literal_p)->children->literals;
						delete (*literal_p)->children->wmes;
						delete (*literal_p)->children;
					}

					delete (*literal_p);
				}

				// matches
				std::list<epmem_shared_match *>::iterator match_p;
				for ( match_p=matches.begin(); match_p!=matches.end(); match_p++ )
					delete (*match_p);

				// trigger lists
				std::list<epmem_shared_literal_list *>::iterator trigger_list_p;
				for ( trigger_list_p=trigger_lists.begin(); trigger_list_p!=trigger_lists.end(); trigger_list_p++ )
					delete (*trigger_list_p);

				// queries
				epmem_shared_query *del_query;
				for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
				{
					while ( !queries[ i ].empty() )
					{
						del_query = queries[ i ].top();
						queries[ i ].pop();

						sqlite3_finalize( del_query->stmt );
						delete del_query;
					}
				}
				delete [] queries;

				// graph match
				if ( graph_match != soar_module::off )
				{
					delete graph_match_roots->literals;
					delete graph_match_roots->wmes;
					delete graph_match_roots;
				}
			}

			// place results in WM
			if ( king_id != EPMEM_MEMID_NONE )
			{
				Symbol *my_meta;
				epmem_id_mapping *my_mapping = NULL;

				my_agent->epmem_stats->qry_ret->set_value( king_id );
				my_agent->epmem_stats->qry_card->set_value( king_cardinality );

				// status
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );				

				// match score
				my_meta = make_float_constant( my_agent, king_score );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// cue-size
				my_meta = make_int_constant( my_agent, cue_size );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// normalized-match-score
				my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// match-cardinality
				my_meta = make_int_constant( my_agent, king_cardinality );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// graph match
				if ( graph_match != soar_module::off )
				{
					// graph-match 0/1
					my_meta = make_int_constant( my_agent, ( ( king_graph_match == len_query )?(1):(0) ) );
					epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_graph_match_symbol, my_meta );					
					symbol_remove_ref( my_agent, my_meta );

					// full mapping if appropriate
					if ( ( graph_match == soar_module::on ) && ( king_graph_match == len_query ) )
					{
						Symbol *my_meta2;
						Symbol *my_meta3;

						my_meta = make_new_identifier( my_agent, 'M', state->id.epmem_result_header->id.level );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_graph_match_mapping_symbol, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						my_mapping = new epmem_id_mapping();
						for ( epmem_constraint_list::iterator c_p=king_constraints.begin(); c_p!=king_constraints.end(); c_p++ )
						{
							// create the node
							my_meta2 = make_new_identifier( my_agent, 'N', my_meta->id.level );
							epmem_add_meta_wme( my_agent, state, my_meta, my_agent->epmem_graph_match_mapping_node_symbol, my_meta2 );							
							symbol_remove_ref( my_agent, my_meta2 );

							// point to the cue identifier
							epmem_add_meta_wme( my_agent, state, my_meta2, my_agent->epmem_graph_match_mapping_cue_symbol, c_p->first );							

							// create and store away the [yet-to-be-retrieved] identifier
							my_meta3 = make_new_identifier( my_agent, c_p->first->id.name_letter, my_meta2->id.level );
							epmem_add_meta_wme( my_agent, state, my_meta2, my_agent->epmem_retrieved_symbol, my_meta3 );							
							symbol_remove_ref( my_agent, my_meta3 );
							(*my_mapping)[ c_p->second ] = my_meta3;
						}
					}
				}

				////////////////////////////////////////////////////////////////////////////
				my_agent->epmem_timers->query->stop();
				////////////////////////////////////////////////////////////////////////////

				// actual memory
				if ( level > 2 )
					epmem_install_memory( my_agent, state, king_id, my_mapping );

				if ( my_mapping )
					delete my_mapping;
			}
			else
			{
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );				

				////////////////////////////////////////////////////////////////////////////
				my_agent->epmem_timers->query->stop();
				////////////////////////////////////////////////////////////////////////////
			}
		}
	}
	else
	{
		epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );		

		free_memory( my_agent, wmes_query, MISCELLANEOUS_MEM_USAGE );
		free_memory( my_agent, wmes_neg_query, MISCELLANEOUS_MEM_USAGE );

		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->query->stop();
		////////////////////////////////////////////////////////////////////////////
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Implementation (epmem::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_consider_new_episode
 * Author		: Nate Derbinsky
 * Notes		: Based upon trigger/force parameter settings, potentially
 * 				  records a new episode
 **************************************************************************/
void epmem_consider_new_episode( agent *my_agent )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->trigger->start();
	////////////////////////////////////////////////////////////////////////////

	const long force = my_agent->epmem_params->force->get_value();
	bool new_memory = false;

	if ( force == epmem_param_container::force_off )
	{
		const long trigger = my_agent->epmem_params->trigger->get_value();

		if ( trigger == epmem_param_container::output )
		{
			slot *s;
			wme *w;
			Symbol *ol = my_agent->io_header_output;
			unsigned long wme_count = 0;

			// examine all commands on the output-link for any
			// that appeared since last memory was recorded
			for ( s = ol->id.slots; s != NIL; s = s->next )
			{
				for ( w = s->wmes; w != NIL; w = w->next )
				{
					wme_count++;

					if ( w->timetag > my_agent->bottom_goal->id.epmem_info->last_ol_time )
					{
						new_memory = true;
						my_agent->bottom_goal->id.epmem_info->last_ol_time = w->timetag;
					}
				}
			}

			// check for change in the number of WMEs (catches the case of a removed WME)
			if ( my_agent->bottom_goal->id.epmem_info->last_ol_count != wme_count )
			{
				new_memory = ( wme_count != 0 );
				my_agent->bottom_goal->id.epmem_info->last_ol_count = wme_count;
			}
			
			// I'm sticking with shallow check for now, since I imagine environment
			// status messages could be a problem.  In any case, recursive code is below.

			/*int tc = get_new_tc_number( my_agent );
			std::queue<Symbol *> syms;
			Symbol *parent_sym;
			unsigned long wme_count = 0;
			wme **wmes;			
			unsigned long len;
			unsigned long i;

			// initialize BFS at command
			syms.push( my_agent->io_header_output );

			while ( !syms.empty() )
			{
				// get state
				parent_sym = syms.front();
				syms.pop();
			
				// get children of the current identifier
				wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

				if ( wmes )
				{
					for ( i=0; i<len; i++ )
					{
						wme_count++;

						if ( wmes[i]->timetag > my_agent->bottom_goal->id.epmem_info->last_ol_time )
						{
							new_memory = true;
							my_agent->bottom_goal->id.epmem_info->last_ol_time = wmes[i]->timetag;
						}

						if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
						{
							syms.push( wmes[i]->value );
						}
					}
					
					// free space from aug list
					free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
				}
			}

			// see if any WMEs were removed
			if ( my_agent->bottom_goal->id.epmem_info->last_ol_count != wme_count )
			{
				new_memory = ( wme_count != 0 );
				my_agent->bottom_goal->id.epmem_info->last_ol_count = wme_count;
			}*/
		}
		else if ( trigger == epmem_param_container::dc )
		{
			new_memory = true;
		}
		else if ( trigger == epmem_param_container::none )
		{
			new_memory = false;
		}
	}
	else
	{
		new_memory = ( force == epmem_param_container::remember );

		my_agent->epmem_params->force->set_value( epmem_param_container::force_off );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->trigger->stop();
	////////////////////////////////////////////////////////////////////////////

	if ( new_memory )
		epmem_new_episode( my_agent );
}

/***************************************************************************
 * Function     : epmem_respond_to_cmd
 * Author		: Nate Derbinsky
 * Notes		: Implements the Soar-EpMem API
 **************************************************************************/
void epmem_respond_to_cmd( agent *my_agent )
{
	// if this is before the first episode, initialize db components
	if ( my_agent->epmem_db_status == EPMEM_DB_CLOSED )
		epmem_init_db( my_agent );

	// respond to query only if db is properly initialized
	if ( my_agent->epmem_db_status != SQLITE_OK )
		return;

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->api->start();
	////////////////////////////////////////////////////////////////////////////

	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	wme **wmes;
	const char *attr_name;
	unsigned long len;
	unsigned long i;

	epmem_time_id retrieve;
	bool next, previous;
	Symbol *query;
	Symbol *neg_query;
	epmem_time_list *prohibit;
	epmem_time_id before, after;
	bool good_cue;
	int path;
	
	unsigned long wme_count;
	bool new_cue;

	while ( state != NULL )
	{
		// make sure this state has had some sort of change to the cmd
		new_cue = false;
		wme_count = 0;
		{			
			int tc = get_new_tc_number( my_agent );
			std::queue<Symbol *> syms;
			Symbol *parent_sym;			

			// initialize BFS at command
			syms.push( state->id.epmem_cmd_header );

			while ( !syms.empty() )
			{
				// get state
				parent_sym = syms.front();
				syms.pop();
			
				// get children of the current identifier
				wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

				if ( wmes )
				{
					for ( i=0; i<len; i++ )
					{
						wme_count++;

						if ( wmes[i]->timetag > state->id.epmem_info->last_cmd_time )
						{
							new_cue = true;
							state->id.epmem_info->last_cmd_time = wmes[i]->timetag;
						}

						if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
						{
							syms.push( wmes[i]->value );
						}
					}
					
					// free space from aug list
					free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
				}
			}

			// see if any WMEs were removed
			if ( state->id.epmem_info->last_cmd_count != wme_count )
			{
				new_cue = true;
				state->id.epmem_info->last_cmd_count = wme_count;
			}

			if ( new_cue )
			{
				// clear old cue
				state->id.epmem_info->cue_wmes->clear();

				// clear old results
				epmem_clear_result( my_agent, state );
			}
		}

		// a command is issued if the cue is new
		// and there is something on the cue
		if ( new_cue && wme_count )
		{
			// initialize command vars
			retrieve = EPMEM_MEMID_NONE;
			next = false;
			previous = false;
			query = NULL;
			neg_query = NULL;
			prohibit = new epmem_time_list();
			before = EPMEM_MEMID_NONE;
			after = EPMEM_MEMID_NONE;
			good_cue = true;
			path = 0;

			// get all top-level symbols
			wmes = epmem_get_augs_of_id( my_agent, state->id.epmem_cmd_header, get_new_tc_number( my_agent ), &len );

			// process top-level symbols
			for ( i=0; i<len; i++ )
			{
				state->id.epmem_info->cue_wmes->insert( wmes[i] );
				
				if ( good_cue )
				{
					// get attribute name
					attr_name = epmem_symbol_to_string( my_agent, wmes[i]->attr );

					// collect information about known commands
					if ( !strcmp( attr_name, "retrieve" ) )
					{
						if ( ( wmes[ i ]->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( path == 0 ) &&
							 ( wmes[ i ]->value->ic.value > 0 ) )
						{							
							retrieve = wmes[ i ]->value->ic.value;
							path = 1;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "next" ) )
					{
						if ( ( wmes[ i ]->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{							
							next = true;
							path = 2;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "previous" ) )
					{
						if ( ( wmes[ i ]->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{
							previous = true;
							path = 2;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "query" ) )
					{
						if ( ( wmes[ i ]->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) &&
							 ( query == NULL ) )

						{
							query = wmes[ i ]->value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "neg-query" ) )
					{
						if ( ( wmes[ i ]->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) &&
							 ( neg_query == NULL ) )

						{
							neg_query = wmes[ i ]->value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "before" ) )
					{
						if ( ( wmes[ i ]->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							before = wmes[ i ]->value->ic.value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "after" ) )
					{
						if ( ( wmes[ i ]->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							after = wmes[ i ]->value->ic.value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( !strcmp( attr_name, "prohibit" ) )
					{
						if ( ( wmes[ i ]->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							prohibit->push_back( wmes[ i ]->value->ic.value );
							path = 3;
						}
						else
							good_cue = false;
					}
					else
						good_cue = false;
				}
			}

			// if on path 3 must have query/neg-query
			if ( ( path == 3 ) && ( query == NULL ) && ( neg_query == NULL ) )
				good_cue = false;

			// must be on a path
			if ( path == 0 )
				good_cue = false;

			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////

			// process command
			if ( good_cue )
			{
				// retrieve
				if ( path == 1 )
				{
					epmem_install_memory( my_agent, state, retrieve );
				}
				// previous or next
				else if ( path == 2 )
				{
					epmem_install_memory( my_agent, state, ( ( next )?( epmem_next_episode( my_agent, state->id.epmem_info->last_memory ) ):( epmem_previous_episode( my_agent, state->id.epmem_info->last_memory ) ) ) );
				}
				// query
				else if ( path == 3 )
				{
					epmem_process_query( my_agent, state, query, neg_query, prohibit, before, after );
				}
			}
			else
			{
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );				
			}

			// free prohibit list
			delete prohibit;

			// free space from aug list
			free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
		}

		state = state->id.higher_goal;
	}
}

/***************************************************************************
 * Function     : epmem_go
 * Author		: Nate Derbinsky
 * Notes		: The kernel calls this function to implement Soar-EpMem:
 * 				  consider new storage and respond to any commands
 **************************************************************************/
void epmem_go( agent *my_agent )
{
	my_agent->epmem_timers->total->start();

#ifndef EPMEM_EXPERIMENT

	if ( !epmem_in_transaction( my_agent ) )
		epmem_transaction_begin( my_agent );

	epmem_consider_new_episode( my_agent );
	epmem_respond_to_cmd( my_agent );

	if ( !epmem_in_transaction( my_agent ) )
		epmem_transaction_end( my_agent, true );

#else // EPMEM_EXPERIMENT

	// storing database snapshots at commit intervals
	/*
	if ( !epmem_in_transaction( my_agent ) )
		epmem_transaction_begin( my_agent );

	epmem_consider_new_episode( my_agent );
	epmem_respond_to_cmd( my_agent );

	if ( !epmem_in_transaction( my_agent ) )
	{
		epmem_transaction_end( my_agent, true );

		long counter = epmem_get_stat( my_agent, EPMEM_STAT_NCB_WMES );
		epmem_set_stat( my_agent, EPMEM_STAT_NCB_WMES, ( counter + 1 ) );
		std::string path( "cp " );
		path.append( epmem_get_parameter( my_agent, EPMEM_PARAM_PATH, EPMEM_RETURN_STRING ) );
		path.append( " " );
		path.append( epmem_get_parameter( my_agent, EPMEM_PARAM_PATH, EPMEM_RETURN_STRING ) );
		path.append( "_" );
		path.append( *to_string( counter ) );

		system( path.c_str() );
	}
	*/

	// retrieving lots of episodes (commit = number of repeats)
	/*
	{
		const int max_queries = 10;
		const int repeat = (long) epmem_get_parameter( my_agent, EPMEM_PARAM_COMMIT );
		epmem_time_id queries[ max_queries ] = { 10, 50, 100, 250, 500, 1000, 2500, 5000, 6000, 8200 };

		epmem_init_db( my_agent, true );
		epmem_transaction_begin( my_agent );

		timeval start;
		timeval total;

		for ( int j=0; j<max_queries; j++ )
		{
			reset_timer( &start );
			reset_timer( &total );

			start_timer( my_agent, &start );

			for ( int i=0; i<repeat; i++ )
			{
				epmem_install_memory( my_agent, my_agent->top_goal, queries[j], NULL );
				epmem_clear_result( my_agent, my_agent->top_goal );
			}

			stop_timer( my_agent, &start, &total );

			std::cout << (j) << "," << ( (double) timer_value( &total ) / (double) repeat ) << std::endl;
		}

		stop_timer( my_agent, &start, &total );

		epmem_transaction_end( my_agent, false );
		epmem_close( my_agent );
	}
	*/

	// cue matching over lots of cues (numerical wmes under "queries" wme, commit=level)
	/*{
		long level = (long) epmem_get_parameter( my_agent, EPMEM_PARAM_COMMIT );
		const int repeat = 20;

		epmem_time_list *prohibit = new epmem_time_list();
		unsigned long max_queries;
		wme **wmes;
		{
			Symbol *queries = NULL;
			wmes = epmem_get_augs_of_id( my_agent, my_agent->top_goal, get_new_tc_number( my_agent ), &max_queries );
			for ( int i=0; i<max_queries; i++ )
			{
				if ( ( wmes[i]->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) && !strcmp( wmes[i]->attr->sc.name, "queries" ) )
					queries = wmes[i]->value;
			}
			free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );

			wmes = epmem_get_augs_of_id( my_agent, queries, get_new_tc_number( my_agent ), &max_queries );
		}

		{
			int real_query;
			timeval start;
			timeval total;

			epmem_init_db( my_agent, true );
			epmem_transaction_begin( my_agent );

			for ( int i=0; i<max_queries; i++ )
			{
				real_query = wmes[i]->attr->ic.value;

				reset_timer( &start );
				reset_timer( &total );

				start_timer( my_agent, &start );

				for ( int j=0; j<repeat; j++ )
				{
					epmem_process_query( my_agent, my_agent->top_goal, wmes[i]->value, NULL, prohibit, EPMEM_MEMID_NONE, EPMEM_MEMID_NONE, level );
					epmem_clear_result( my_agent, my_agent->top_goal );
				}

				stop_timer( my_agent, &start, &total );

				std::cout << ( real_query ) << "," << ( (double) timer_value( &total ) / (double) repeat ) << std::endl;
			}

			epmem_transaction_end( my_agent, false );
			epmem_close( my_agent );
		}
	}*/

#endif // EPMEM_EXPERIMENT

	my_agent->epmem_timers->total->stop();	
}
