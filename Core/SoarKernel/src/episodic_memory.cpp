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

#include <cmath>
#include <algorithm>

#include "episodic_memory.h"

#include "agent.h"
#include "prefmem.h"
#include "symtab.h"
#include "wmem.h"
#include "print.h"
#include "xml.h"

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				epmem::param
// stats 						epmem::stats
// timers 						epmem::timers
// statements					epmem::statements

// wme-related					epmem::wmes

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


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (epmem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_param_container::epmem_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
	// learning
	learning = new soar_module::boolean_param( "learning", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( learning );

	// database
	database = new soar_module::constant_param<db_choices>( "database", memory, new epmem_db_predicate<db_choices>( my_agent ) );
	database->add_mapping( memory, "memory" );
	database->add_mapping( file, "file" );
	add( database );

	// path
	path = new epmem_path_param( "path", "", new soar_module::predicate<const char *>(), new epmem_db_predicate<const char *>( my_agent ), my_agent );
	add( path );

	// commit
	commit = new soar_module::integer_param( "commit", 1, new soar_module::gt_predicate<long>( 1, true ), new soar_module::f_predicate<long>() );
	add( commit );

	// mode
	mode = new epmem_mode_param( "mode", graph, new epmem_db_predicate<mode_choices>( my_agent ), my_agent );
	mode->add_mapping( tree, "tree" );
	mode->add_mapping( graph, "graph" );
	add( mode );

	// graph-match
	graph_match = new epmem_graph_match_param( "graph-match", soar_module::on, new soar_module::f_predicate<soar_module::boolean>(), my_agent );
	add( graph_match );

	// phase
	phase = new soar_module::constant_param<phase_choices>( "phase", phase_output, new soar_module::f_predicate<phase_choices>() );
	phase->add_mapping( phase_output, "output" );
	phase->add_mapping( phase_selection, "selection" );
	add( phase );

	// trigger
	trigger = new soar_module::constant_param<trigger_choices>( "trigger", output, new soar_module::f_predicate<trigger_choices>() );
	trigger->add_mapping( none, "none" );
	trigger->add_mapping( output, "output" );
	trigger->add_mapping( dc, "dc" );
	add( trigger );

	// force
	force = new soar_module::constant_param<force_choices>( "force", force_off, new soar_module::f_predicate<force_choices>() );
	force->add_mapping( remember, "remember" );
	force->add_mapping( ignore, "ignore" );
	force->add_mapping( force_off, "off" );
	add( force );

	// balance
	balance = new soar_module::decimal_param( "balance", 0.5, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( balance );

	// exclusions - this is initialized with "epmem" directly after hash tables
	exclusions = new soar_module::set_param( "exclusions", new soar_module::f_predicate<const char *>, my_agent );
	add( exclusions );

	// timers
	timers = new soar_module::constant_param<soar_module::timer::timer_level>( "timers", soar_module::timer::zero, new soar_module::f_predicate<soar_module::timer::timer_level>() );
	timers->add_mapping( soar_module::timer::zero, "off" );
	timers->add_mapping( soar_module::timer::one, "one" );
	timers->add_mapping( soar_module::timer::two, "two" );
	timers->add_mapping( soar_module::timer::three, "three" );
	add( timers );
}

//

epmem_path_param::epmem_path_param( const char *new_name, const char *new_value, soar_module::predicate<const char *> *new_val_pred, soar_module::predicate<const char *> *new_prot_pred, agent *new_agent ): soar_module::string_param( new_name, new_value, new_val_pred, new_prot_pred ), my_agent( new_agent ) {}

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

epmem_graph_match_param::epmem_graph_match_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent ): soar_module::boolean_param( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

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

epmem_mode_param::epmem_mode_param( const char *new_name, epmem_param_container::mode_choices new_value, soar_module::predicate<epmem_param_container::mode_choices> *new_prot_pred, agent *new_agent ): soar_module::constant_param<epmem_param_container::mode_choices>( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

void epmem_mode_param::set_value( epmem_param_container::mode_choices new_value )
{
	if ( new_value != epmem_param_container::graph )
	{
		my_agent->epmem_params->graph_match->set_value( soar_module::off );
	}

	value = new_value;
}

//

template <typename T>
epmem_db_predicate<T>::epmem_db_predicate( agent *new_agent ): soar_module::agent_predicate<T>( new_agent ) {}

template <typename T>
bool epmem_db_predicate<T>::operator() ( T /*val*/ ) { return ( this->my_agent->epmem_db->get_status() == soar_module::connected ); }


/***************************************************************************
 * Function     : epmem_enabled
 * Author		: Nate Derbinsky
 * Notes		: Shortcut function to system parameter
 **************************************************************************/
bool epmem_enabled( agent *my_agent )
{
	return ( my_agent->epmem_params->learning->get_value() == soar_module::on );
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (epmem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_stat_container::epmem_stat_container( agent *new_agent ): soar_module::stat_container( new_agent )
{
	// time
	time = new epmem_time_id_stat( "time", 0, new epmem_db_predicate<epmem_time_id>( my_agent ) );
	add( time );

	// mem-usage
	mem_usage = new epmem_mem_usage_stat( my_agent, "mem-usage", 0, new soar_module::predicate<intptr_t>() );
	add( mem_usage );

	// mem-high
	mem_high = new epmem_mem_high_stat( my_agent, "mem-high", 0, new soar_module::predicate<intptr_t>() );
	add( mem_high );

	// ncb-wmes
	ncb_wmes = new soar_module::integer_stat( "ncb-wmes", 0, new soar_module::f_predicate<long>() );
	add( ncb_wmes );

	// qry-pos
	qry_pos = new soar_module::integer_stat( "qry-pos", 0, new soar_module::f_predicate<long>() );
	add( qry_pos );

	// qry-neg
	qry_neg = new soar_module::integer_stat( "qry-neg", 0, new soar_module::f_predicate<long>() );
	add( qry_neg );

	// qry-ret
	qry_ret = new epmem_time_id_stat( "qry-ret", 0, new soar_module::f_predicate<epmem_time_id>() );
	add( qry_ret );

	// qry-card
	qry_card = new soar_module::integer_stat( "qry-card", 0, new soar_module::f_predicate<long>() );
	add( qry_card );

	// qry-lits
	qry_lits = new soar_module::integer_stat( "qry-lits", 0, new soar_module::f_predicate<long>() );
	add( qry_lits );

	// next-id
	next_id = new epmem_node_id_stat( "next-id", 0, new epmem_db_predicate<epmem_node_id>( my_agent ) );
	add( next_id );

	// rit-offset-1
	rit_offset_1 = new soar_module::intptr_stat( "rit-offset-1", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_offset_1 );

	// rit-left-root-1
	rit_left_root_1 = new soar_module::intptr_stat( "rit-left-root-1", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_left_root_1 );

	// rit-right-root-1
	rit_right_root_1 = new soar_module::intptr_stat( "rit-right-root-1", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_right_root_1 );

	// rit-min-step-1
	rit_min_step_1 = new soar_module::intptr_stat( "rit-min-step-1", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_min_step_1 );

	// rit-offset-2
	rit_offset_2 = new soar_module::intptr_stat( "rit-offset-2", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_offset_2 );

	// rit-left-root-2
	rit_left_root_2 = new soar_module::intptr_stat( "rit-left-root-2", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_left_root_2 );

	// rit-right-root-2
	rit_right_root_2 = new soar_module::intptr_stat( "rit-right-root-2", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_right_root_2 );

	// rit-min-step-2
	rit_min_step_2 = new soar_module::intptr_stat( "rit-min-step-2", 0, new epmem_db_predicate<intptr_t>( my_agent ) );
	add( rit_min_step_2 );


	/////////////////////////////
	// connect to rit state
	/////////////////////////////

	// tree
	my_agent->epmem_rit_state_tree.offset.stat = rit_offset_1;
	my_agent->epmem_rit_state_tree.offset.var_key = var_rit_offset_1;
	my_agent->epmem_rit_state_tree.leftroot.stat = rit_left_root_1;
	my_agent->epmem_rit_state_tree.leftroot.var_key = var_rit_leftroot_1;
	my_agent->epmem_rit_state_tree.rightroot.stat = rit_right_root_1;
	my_agent->epmem_rit_state_tree.rightroot.var_key = var_rit_rightroot_1;
	my_agent->epmem_rit_state_tree.minstep.stat = rit_min_step_1;
	my_agent->epmem_rit_state_tree.minstep.var_key = var_rit_minstep_1;

	// graph		
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.stat = rit_offset_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.var_key = var_rit_offset_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.stat = rit_left_root_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.var_key = var_rit_leftroot_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.stat = rit_right_root_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.var_key = var_rit_rightroot_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.stat = rit_min_step_1;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.var_key = var_rit_minstep_1;	
	
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.stat = rit_offset_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.var_key = var_rit_offset_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.stat = rit_left_root_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.var_key = var_rit_leftroot_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.stat = rit_right_root_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.var_key = var_rit_rightroot_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.stat = rit_min_step_2;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.var_key = var_rit_minstep_2;
}

//

epmem_mem_usage_stat::epmem_mem_usage_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred ): soar_module::intptr_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

intptr_t epmem_mem_usage_stat::get_value()
{
	return my_agent->epmem_db->memory_usage();
}

//

epmem_mem_high_stat::epmem_mem_high_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred ): soar_module::intptr_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

intptr_t epmem_mem_high_stat::get_value()
{
	return my_agent->epmem_db->memory_highwater();
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (epmem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_timer_container::epmem_timer_container( agent *new_agent ): soar_module::timer_container( new_agent )
{
	// one
	
	total = new epmem_timer( "_total", my_agent, soar_module::timer::one );
	add( total );

	// two

	storage = new epmem_timer( "epmem_storage", my_agent, soar_module::timer::two );
	add( storage );

	ncb_retrieval = new epmem_timer( "epmem_ncb_retrieval", my_agent, soar_module::timer::two );
	add( ncb_retrieval );

	query = new epmem_timer( "epmem_query", my_agent, soar_module::timer::two );
	add( query );

	api = new epmem_timer( "epmem_api", my_agent, soar_module::timer::two );
	add( api );

	trigger = new epmem_timer( "epmem_trigger", my_agent, soar_module::timer::two );
	add( trigger );

	init = new epmem_timer( "epmem_init", my_agent, soar_module::timer::two );
	add( init );

	next = new epmem_timer( "epmem_next", my_agent, soar_module::timer::two );
	add( next );

	prev = new epmem_timer( "epmem_prev", my_agent, soar_module::timer::two );
	add( prev );

	hash = new epmem_timer( "epmem_hash", my_agent, soar_module::timer::two );
	add( hash );

	// three

	ncb_edge = new epmem_timer( "ncb_edge", my_agent, soar_module::timer::three );
	add( ncb_edge );

	ncb_edge_rit = new epmem_timer( "ncb_edge_rit", my_agent, soar_module::timer::three );
	add( ncb_edge_rit );

	ncb_node = new epmem_timer( "ncb_node", my_agent, soar_module::timer::three );
	add( ncb_node );

	ncb_node_rit = new epmem_timer( "ncb_node_rit", my_agent, soar_module::timer::three );
	add( ncb_node_rit );

	query_dnf = new epmem_timer( "query_dnf", my_agent, soar_module::timer::three );
	add( query_dnf );

	query_graph_match = new epmem_timer( "query_graph_match", my_agent, soar_module::timer::three );
	add( query_graph_match );

	query_pos_start_ep = new epmem_timer( "query_pos_start_ep", my_agent, soar_module::timer::three );
	add( query_pos_start_ep );

	query_pos_start_now = new epmem_timer( "query_pos_start_now", my_agent, soar_module::timer::three );
	add( query_pos_start_now );

	query_pos_start_point = new epmem_timer( "query_pos_start_point", my_agent, soar_module::timer::three );
	add( query_pos_start_point );

	query_pos_end_ep = new epmem_timer( "query_pos_end_ep", my_agent, soar_module::timer::three );
	add( query_pos_end_ep );

	query_pos_end_now = new epmem_timer( "query_pos_end_now", my_agent, soar_module::timer::three );
	add( query_pos_end_now );

	query_pos_end_point = new epmem_timer( "query_pos_end_point", my_agent, soar_module::timer::three );
	add( query_pos_end_point );

	query_neg_start_ep = new epmem_timer( "query_neg_start_ep", my_agent, soar_module::timer::three );
	add( query_neg_start_ep );

	query_neg_start_now = new epmem_timer( "query_neg_start_now", my_agent, soar_module::timer::three );
	add( query_neg_start_now );

	query_neg_start_point = new epmem_timer( "query_neg_start_point", my_agent, soar_module::timer::three );
	add( query_neg_start_point );

	query_neg_end_ep = new epmem_timer( "query_neg_end_ep", my_agent, soar_module::timer::three );
	add( query_neg_end_ep );

	query_neg_end_now = new epmem_timer( "query_neg_end_now", my_agent, soar_module::timer::three );
	add( query_neg_end_now );

	query_neg_end_point = new epmem_timer( "query_neg_end_point", my_agent, soar_module::timer::three );
	add( query_neg_end_point );

	/////////////////////////////
	// connect to rit state
	/////////////////////////////

	// tree		
	my_agent->epmem_rit_state_tree.timer = ncb_node_rit;

	// graph
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].timer = ncb_node_rit;
	my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].timer = ncb_edge_rit;
}

//

epmem_timer_level_predicate::epmem_timer_level_predicate( agent *new_agent ): soar_module::agent_predicate<soar_module::timer::timer_level>( new_agent ) {}

bool epmem_timer_level_predicate::operator() ( soar_module::timer::timer_level val ) { return ( my_agent->epmem_params->timers->get_value() >= val ); }

//

epmem_timer::epmem_timer(const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level): soar_module::timer( new_name, new_agent, new_level, new epmem_timer_level_predicate( new_agent ) ) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statement Functions (epmem::statements)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_common_statement_container::epmem_common_statement_container( agent *new_agent ): soar_module::sqlite_statement_container( new_agent->epmem_db )
{
	soar_module::sqlite_database *new_db = new_agent->epmem_db;

	//

	add_structure( "CREATE TABLE IF NOT EXISTS vars (id INTEGER PRIMARY KEY,value NONE)" );
	add_structure( "CREATE TABLE IF NOT EXISTS rit_left_nodes (min INTEGER, max INTEGER)" );
	add_structure( "CREATE TABLE IF NOT EXISTS rit_right_nodes (node INTEGER)" );
	add_structure( "CREATE TABLE IF NOT EXISTS temporal_symbol_hash (id INTEGER PRIMARY KEY, sym_const NONE, sym_type INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS temporal_symbol_hash_const_type ON temporal_symbol_hash (sym_type,sym_const)" );
	
	// workaround for tree: 1 = IDENTIFIER_SYMBOL_TYPE
	add_structure( "INSERT OR IGNORE INTO temporal_symbol_hash (id,sym_const,sym_type) VALUES (0,NULL,1)" );

	//
	
	begin = new soar_module::sqlite_statement( new_db, "BEGIN" );
	add( begin );

	commit = new soar_module::sqlite_statement( new_db, "COMMIT" );
	add( commit );

	rollback = new soar_module::sqlite_statement( new_db, "ROLLBACK" );
	add( rollback );

	//

	var_get = new soar_module::sqlite_statement( new_db, "SELECT value FROM vars WHERE id=?" );
	add( var_get );

	var_set = new soar_module::sqlite_statement( new_db, "REPLACE INTO vars (id,value) VALUES (?,?)" );
	add( var_set );

	//

	rit_add_left = new soar_module::sqlite_statement( new_db, "INSERT INTO rit_left_nodes (min,max) VALUES (?,?)" );
	add( rit_add_left );

	rit_truncate_left = new soar_module::sqlite_statement( new_db, "DELETE FROM rit_left_nodes" );
	add( rit_truncate_left );

	rit_add_right = new soar_module::sqlite_statement( new_db, "INSERT INTO rit_right_nodes (node) VALUES (?)" );
	add( rit_add_right );

	rit_truncate_right = new soar_module::sqlite_statement( new_db, "DELETE FROM rit_right_nodes" );
	add( rit_truncate_right );

	//

	hash_get = new soar_module::sqlite_statement( new_db, "SELECT id FROM temporal_symbol_hash WHERE sym_type=? AND sym_const=?" );
	add( hash_get );

	hash_add = new soar_module::sqlite_statement( new_db, "INSERT INTO temporal_symbol_hash (sym_type,sym_const) VALUES (?,?)" );
	add( hash_add );
}


epmem_tree_statement_container::epmem_tree_statement_container( agent *new_agent ): soar_module::sqlite_statement_container( new_agent->epmem_db )
{
	soar_module::sqlite_database *new_db = new_agent->epmem_db;

	//

	add_structure( "CREATE TABLE IF NOT EXISTS times (id INTEGER PRIMARY KEY)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_now (id INTEGER,start INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_now_start ON node_now (start)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_now_id_start ON node_now (id,start DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_point (id INTEGER,start INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_point_id_start ON node_point (id,start DESC)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_point_start ON node_point (start)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_range_lower ON node_range (rit_node,start)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_range_upper ON node_range (rit_node,end)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_start ON node_range (id,start DESC)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_end ON node_range (id,end DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,attrib INTEGER,value INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_unique_parent_attrib_value ON node_unique (parent_id,attrib,value)" );

	//

	add_time = new soar_module::sqlite_statement( new_db, "INSERT INTO times (id) VALUES (?)" );
	add( add_time );

	//

	add_node_now = new soar_module::sqlite_statement( new_db, "INSERT INTO node_now (id,start) VALUES (?,?)" );
	add( add_node_now );

	delete_node_now = new soar_module::sqlite_statement( new_db, "DELETE FROM node_now WHERE id=?" );
	add( delete_node_now );

	add_node_point = new soar_module::sqlite_statement( new_db, "INSERT INTO node_point (id,start) VALUES (?,?)" );
	add( add_node_point );

	add_node_range = new soar_module::sqlite_statement( new_db, "INSERT INTO node_range (rit_node,start,end,id) VALUES (?,?,?,?)" );
	add( add_node_range );

	//

	add_node_unique = new soar_module::sqlite_statement( new_db, "INSERT INTO node_unique (parent_id,attrib,value) VALUES (?,?,?)" );
	add( add_node_unique );

	find_node_unique = new soar_module::sqlite_statement( new_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=?" );
	add( find_node_unique );

	find_identifier = new soar_module::sqlite_statement( new_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=0" );
	add( find_identifier );

	//

	valid_episode = new soar_module::sqlite_statement( new_db, "SELECT COUNT(*) AS ct FROM times WHERE id=?" );
	add( valid_episode );

	next_episode = new soar_module::sqlite_statement( new_db, "SELECT id FROM times WHERE id>? ORDER BY id ASC LIMIT 1" );
	add( next_episode );

	prev_episode = new soar_module::sqlite_statement( new_db, "SELECT id FROM times WHERE id<? ORDER BY id DESC LIMIT 1" );
	add( prev_episode );

	get_episode = new soar_module::sqlite_statement( new_db, "SELECT i.child_id, i.parent_id, h1.sym_const, h2.sym_const, h1.sym_type, h2.sym_type FROM node_unique i, temporal_symbol_hash h1, temporal_symbol_hash h2 WHERE i.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) AND i.attrib=h1.id AND i.value=h2.id ORDER BY i.child_id ASC", new_agent->epmem_timers->ncb_node );
	add( get_episode );
}

epmem_graph_statement_container::epmem_graph_statement_container( agent *new_agent ): soar_module::sqlite_statement_container( new_agent->epmem_db )
{
	soar_module::sqlite_database *new_db = new_agent->epmem_db;

	//

	add_structure( "CREATE TABLE IF NOT EXISTS times (id INTEGER PRIMARY KEY)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_now (id INTEGER,start INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_now_start ON node_now (start)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_now_id_start ON node_now (id,start DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_now (id INTEGER,start INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_now_start ON edge_now (start)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_now_id_start ON edge_now (id,start DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_point (id INTEGER,start INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_point_id_start ON node_point (id,start DESC)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_point_start ON node_point (start)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_point (id INTEGER,start INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_point_id_start ON edge_point (id,start DESC)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_point_start ON edge_point (start)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_range_lower ON node_range (rit_node,start)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_range_upper ON node_range (rit_node,end)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_start ON node_range (id,start DESC)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_end ON node_range (id,end DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_range_lower ON edge_range (rit_node,start)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_range_upper ON edge_range (rit_node,end)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_start ON edge_range (id,start DESC)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_end ON edge_range (id,end DESC)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,attrib INTEGER, value INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_unique_parent_attrib_value ON node_unique (parent_id,attrib,value)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_unique (parent_id INTEGER PRIMARY KEY AUTOINCREMENT,q0 INTEGER,w INTEGER,q1 INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_unique_q0_w_q1 ON edge_unique (q0,w,q1)" );

	//

	add_time = new soar_module::sqlite_statement( new_db, "INSERT INTO times (id) VALUES (?)" );
	add( add_time );

	//

	add_node_now = new soar_module::sqlite_statement( new_db, "INSERT INTO node_now (id,start) VALUES (?,?)" );
	add( add_node_now );

	delete_node_now = new soar_module::sqlite_statement( new_db, "DELETE FROM node_now WHERE id=?" );
	add( delete_node_now );

	add_node_point = new soar_module::sqlite_statement( new_db, "INSERT INTO node_point (id,start) VALUES (?,?)" );
	add( add_node_point );

	add_node_range = new soar_module::sqlite_statement( new_db, "INSERT INTO node_range (rit_node,start,end,id) VALUES (?,?,?,?)" );
	add( add_node_range );


	add_node_unique = new soar_module::sqlite_statement( new_db, "INSERT INTO node_unique (parent_id,attrib,value) VALUES (?,?,?)" );
	add( add_node_unique );

	find_node_unique = new soar_module::sqlite_statement( new_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND attrib=? AND value=?" );
	add( find_node_unique );

	//

	add_edge_now = new soar_module::sqlite_statement( new_db, "INSERT INTO edge_now (id,start) VALUES (?,?)" );
	add( add_edge_now );

	delete_edge_now = new soar_module::sqlite_statement( new_db, "DELETE FROM edge_now WHERE id=?" );
	add( delete_edge_now );

	add_edge_point = new soar_module::sqlite_statement( new_db, "INSERT INTO edge_point (id,start) VALUES (?,?)" );
	add( add_edge_point );

	add_edge_range = new soar_module::sqlite_statement( new_db, "INSERT INTO edge_range (rit_node,start,end,id) VALUES (?,?,?,?)" );
	add( add_edge_range );


	add_edge_unique = new soar_module::sqlite_statement( new_db, "INSERT INTO edge_unique (q0,w,q1) VALUES (?,?,?)" );
	add( add_edge_unique );

	find_edge_unique = new soar_module::sqlite_statement( new_db, "SELECT parent_id, q1 FROM edge_unique WHERE q0=? AND w=?" );
	add( find_edge_unique );

	find_edge_unique_shared = new soar_module::sqlite_statement( new_db, "SELECT parent_id FROM edge_unique WHERE q0=? AND w=? AND q1=?" );
	add( find_edge_unique_shared );

	//

	valid_episode = new soar_module::sqlite_statement( new_db, "SELECT COUNT(*) AS ct FROM times WHERE id=?" );
	add( valid_episode );

	next_episode = new soar_module::sqlite_statement( new_db, "SELECT id FROM times WHERE id>? ORDER BY id ASC LIMIT 1" );
	add( next_episode );

	prev_episode = new soar_module::sqlite_statement( new_db, "SELECT id FROM times WHERE id<? ORDER BY id DESC LIMIT 1" );
	add( prev_episode );


	get_nodes = new soar_module::sqlite_statement( new_db, "SELECT f.child_id, f.parent_id, h1.sym_const, h2.sym_const, h1.sym_type, h2.sym_type FROM node_unique f, temporal_symbol_hash h1, temporal_symbol_hash h2 WHERE f.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) AND f.attrib=h1.id AND f.value=h2.id ORDER BY f.child_id ASC", new_agent->epmem_timers->ncb_node );
	add( get_nodes );

	get_edges = new soar_module::sqlite_statement( new_db, "SELECT f.q0, h.sym_const, f.q1, h.sym_type FROM edge_unique f INNER JOIN temporal_symbol_hash h ON f.w=h.id WHERE f.parent_id IN (SELECT n.id FROM edge_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM edge_point p WHERE p.start=? UNION ALL SELECT e1.id FROM edge_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM edge_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY f.q0 ASC, f.q1 ASC", new_agent->epmem_timers->ncb_edge );
	add( get_edges );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (epmem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_augs_of_id
 * Author		: Nate Derbinsky
 * Notes		: This routine gets all non-acceptable preference wmes
 *                associated with an id.
 **************************************************************************/
epmem_wme_list *epmem_get_augs_of_id( Symbol * id, tc_number tc )
{
	slot *s;
	wme *w;
	epmem_wme_list *return_val = new epmem_wme_list;

	// augs only exist for identifiers
	if ( ( id->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
		 ( id->id.tc_num != tc ) )
	{
		id->id.tc_num = tc;

		// impasse wmes
		for ( w=id->id.impasse_wmes; w!=NIL; w=w->next )
		{
			if ( !w->acceptable )
			{
				return_val->push_back( w );
			}
		}

		// input wmes
		for ( w=id->id.input_wmes; w!=NIL; w=w->next )
		{
			return_val->push_back( w );
		}

		// regular wmes
		for ( s=id->id.slots; s!=NIL; s=s->next )
		{
			for ( w=s->wmes; w!=NIL; w=w->next )
			{
				if ( !w->acceptable )
				{				
					return_val->push_back( w );
				}
			}
		}
	}

	return return_val;
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
	if ( my_agent->epmem_db->get_status() != soar_module::connected )
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
	if ( my_agent->epmem_db->get_status() == soar_module::connected )
	{
		my_agent->epmem_stmts_common->begin->execute( soar_module::op_reinit );		
	}
}

/***************************************************************************
 * Function     : epmem_transaction_end
 * Author		: Nate Derbinsky
 * Notes		: Ends the current transaction
 **************************************************************************/
void epmem_transaction_end( agent *my_agent, bool commit )
{
	if ( my_agent->epmem_db->get_status() == soar_module::connected )
	{		
		soar_module::sqlite_statement *end_type = ( ( commit )?( my_agent->epmem_stmts_common->commit ):( my_agent->epmem_stmts_common->rollback ) );
		end_type->execute( soar_module::op_reinit );		
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
bool epmem_get_variable( agent *my_agent, epmem_variable_key variable_id, intptr_t *variable_value )
{
	soar_module::exec_result status;
	soar_module::sqlite_statement *var_get = my_agent->epmem_stmts_common->var_get;

	var_get->bind_int( 1, variable_id );
	status = var_get->execute();	

	if ( status == soar_module::row )
		(*variable_value) = var_get->column_int( 0 );

	var_get->reinitialize();	

	return ( status == soar_module::row );
}

/***************************************************************************
 * Function     : epmem_set_variable
 * Author		: Nate Derbinsky
 * Notes		: Sets an EpMem variable in the database
 **************************************************************************/
void epmem_set_variable( agent *my_agent, epmem_variable_key variable_id, intptr_t variable_value )
{
	soar_module::sqlite_statement *var_set = my_agent->epmem_stmts_common->var_set;

	var_set->bind_int( 1, variable_id );
	var_set->bind_int( 2, variable_value );

	var_set->execute( soar_module::op_reinit );	
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
intptr_t epmem_rit_fork_node( intptr_t lower, intptr_t upper, bool /*bounds_offset*/, intptr_t *step_return, epmem_rit_state *rit_state )
{
	// never called
	/*if ( !bounds_offset )
	{
		intptr_t offset = rit_state->offset.stat->get_value();

		lower = ( lower - offset );
		upper = ( upper - offset );
	}*/

	// descend the tree down to the fork node
	intptr_t node = EPMEM_RIT_ROOT;
	if ( upper < EPMEM_RIT_ROOT )
	{
		node = rit_state->leftroot.stat->get_value();
	}
	else if ( lower > EPMEM_RIT_ROOT )
	{
		node = rit_state->rightroot.stat->get_value();
	}

	intptr_t step;
	for ( step = ( ( ( node >= 0 )?( node ):( -1 * node ) ) / 2 ); step >= 1; step /= 2 )
	{
		if ( upper < node )
		{
			node -= step;
		}
		else if ( node < lower )
		{
			node += step;
		}
		else
		{
			break;
		}
	}

	// never used
	// if ( step_return != NULL )
	{
		(*step_return) = step;
	}

	return node;
}

/***************************************************************************
 * Function     : epmem_rit_clear_left_right
 * Author		: Nate Derbinsky
 * Notes		: Clears the left/right relations populated during prep
 **************************************************************************/
void epmem_rit_clear_left_right( agent *my_agent )
{
	my_agent->epmem_stmts_common->rit_truncate_left->execute( soar_module::op_reinit );
	my_agent->epmem_stmts_common->rit_truncate_right->execute( soar_module::op_reinit );	
}

/***************************************************************************
 * Function     : epmem_rit_add_left
 * Author		: Nate Derbinsky
 * Notes		: Adds a range to the left relation
 **************************************************************************/
void epmem_rit_add_left( agent *my_agent, epmem_time_id min, epmem_time_id max )
{
	my_agent->epmem_stmts_common->rit_add_left->bind_int( 1, min );
	my_agent->epmem_stmts_common->rit_add_left->bind_int( 2, max );
	my_agent->epmem_stmts_common->rit_add_left->execute( soar_module::op_reinit );	
}

/***************************************************************************
 * Function     : epmem_rit_add_right
 * Author		: Nate Derbinsky
 * Notes		: Adds a node to the to the right relation
 **************************************************************************/
void epmem_rit_add_right( agent *my_agent, epmem_time_id id )
{
	my_agent->epmem_stmts_common->rit_add_right->bind_int( 1, id );
	my_agent->epmem_stmts_common->rit_add_right->execute( soar_module::op_reinit );	
}

/***************************************************************************
 * Function     : epmem_rit_prep_left_right
 * Author		: Nate Derbinsky
 * Notes		: Implements the computational components of the RIT
 * 				  query algorithm
 **************************************************************************/
void epmem_rit_prep_left_right( agent *my_agent, intptr_t lower, intptr_t upper, epmem_rit_state *rit_state )
{
	////////////////////////////////////////////////////////////////////////////
	rit_state->timer->start();	
	////////////////////////////////////////////////////////////////////////////

	intptr_t offset = rit_state->offset.stat->get_value();
	intptr_t node, step;
	intptr_t left_node, left_step;
	intptr_t right_node, right_step;

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
			{
				break;
			}
		}
	}

	// go left
	left_node = node - step;
	for ( left_step = ( step / 2 ); left_step >= 1; left_step /= 2 )
	{
		if ( lower == left_node )
		{
			break;
		}
		else if ( lower > left_node )
		{
			epmem_rit_add_left( my_agent, left_node, left_node );
			left_node += left_step;
		}
		else
		{
			left_node -= left_step;
		}
	}

	// go right
	right_node = node + step;
	for ( right_step = ( step / 2 ); right_step >= 1; right_step /= 2 )
	{
		if ( upper == right_node )
		{
			break;
		}
		else if ( upper < right_node )
		{
			epmem_rit_add_right( my_agent, right_node );
			right_node -= right_step;
		}
		else
		{
			right_node += right_step;
		}
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
void epmem_rit_insert_interval( agent *my_agent, intptr_t lower, intptr_t upper, epmem_node_id id, epmem_rit_state *rit_state )
{
	// initialize offset
	intptr_t offset = rit_state->offset.stat->get_value();
	if ( offset == EPMEM_RIT_OFFSET_INIT )
	{
		offset = lower;

		// update database
		epmem_set_variable( my_agent, rit_state->offset.var_key, offset );

		// update stat
		rit_state->offset.stat->set_value( offset );
	}

	// get node
	intptr_t node;
	{
		intptr_t left_root = rit_state->leftroot.stat->get_value();
		intptr_t right_root = rit_state->rightroot.stat->get_value();
		intptr_t min_step = rit_state->minstep.stat->get_value();

		// shift interval
		intptr_t l = ( lower - offset );
		intptr_t u = ( upper - offset );

		// update left_root
		if ( ( u < EPMEM_RIT_ROOT ) && ( l <= ( 2 * left_root ) ) )
		{
			left_root = static_cast<intptr_t>( pow( -2.0, floor( log( static_cast<double>( -l ) ) / EPMEM_LN_2 ) ) );

			// update database
			epmem_set_variable( my_agent, rit_state->leftroot.var_key, left_root );

			// update stat
			rit_state->leftroot.stat->set_value( left_root );
		}

		// update right_root
		if ( ( l > EPMEM_RIT_ROOT ) && ( u >= ( 2 * right_root ) ) )
		{
			right_root = static_cast<intptr_t>( pow( 2.0, floor( log( static_cast<double>( u ) ) / EPMEM_LN_2 ) ) );

			// update database
			epmem_set_variable( my_agent, rit_state->rightroot.var_key, right_root );

			// update stat
			rit_state->rightroot.stat->set_value( right_root );
		}

		// update min_step
		intptr_t step;
		node = epmem_rit_fork_node( l, u, true, &step, rit_state );

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
	rit_state->add_query->bind_int( 1, node );
	rit_state->add_query->bind_int( 2, lower );
	rit_state->add_query->bind_int( 3, upper );
	rit_state->add_query->bind_int( 4, id );
	rit_state->add_query->execute( soar_module::op_reinit );
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
	if ( my_agent->epmem_db->get_status() == soar_module::connected )
	{
		// end any pending transactions
		if ( epmem_in_transaction( my_agent ) )
			epmem_transaction_end( my_agent, true );

		// de-allocate common statements
		delete my_agent->epmem_stmts_common;

		// perform mode-specific cleanup as necessary
		epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();
		if ( mode == epmem_param_container::tree )
		{
			delete my_agent->epmem_stmts_tree;
		}
		else if ( mode == epmem_param_container::graph )
		{
			delete my_agent->epmem_stmts_graph;
			
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

		// close the database
		my_agent->epmem_db->disconnect();
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
 *                The readonly param should only be used in
 *                experimentation where you don't want to alter
 *                previous database state.
 **************************************************************************/
void epmem_init_db( agent *my_agent, bool readonly = false )
{
	if ( my_agent->epmem_db->get_status() != soar_module::disconnected )
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
	my_agent->epmem_db->connect( db_path );	

	if ( my_agent->epmem_db->get_status() == soar_module::problem )
	{
		char buf[256];
		SNPRINTF( buf, 254, "DB ERROR: %s", my_agent->epmem_db->get_errmsg() );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}
	else
	{		
		epmem_time_id time_max;
		soar_module::sqlite_statement *temp_q = NULL;
		soar_module::sqlite_statement *temp_q2 = NULL;

		// point stuff
		epmem_time_id range_start;
		epmem_time_id time_last;

		// update validation count
		my_agent->epmem_validation++;

		// setup common structures/queries
		my_agent->epmem_stmts_common = new epmem_common_statement_container( my_agent );
		my_agent->epmem_stmts_common->structure();
		my_agent->epmem_stmts_common->prepare();

		// mode - read if existing
		epmem_param_container::mode_choices mode;
		{
			intptr_t stored_mode = NIL;
			if ( epmem_get_variable( my_agent, var_mode, &stored_mode ) )
			{
				my_agent->epmem_params->mode->set_value( (epmem_param_container::mode_choices) stored_mode );
				mode = (epmem_param_container::mode_choices) stored_mode;
			}
			else
			{
				mode = (epmem_param_container::mode_choices) my_agent->epmem_params->mode->get_value();
				epmem_set_variable( my_agent, var_mode, mode );
			}
		}

		if ( mode == epmem_param_container::tree )
		{
			// setup tree structures/queries
			my_agent->epmem_stmts_tree = new epmem_tree_statement_container( my_agent );
			my_agent->epmem_stmts_tree->structure();
			my_agent->epmem_stmts_tree->prepare();

			// variable initialization
			my_agent->epmem_stats->time->set_value( 1 );
			my_agent->epmem_rit_state_tree.add_query = my_agent->epmem_stmts_tree->add_node_range;
			my_agent->epmem_rit_state_tree.offset.stat->set_value( EPMEM_RIT_OFFSET_INIT );
			my_agent->epmem_rit_state_tree.leftroot.stat->set_value( 0 );
			my_agent->epmem_rit_state_tree.rightroot.stat->set_value( 0 );
			my_agent->epmem_rit_state_tree.minstep.stat->set_value( LONG_MAX );
			my_agent->epmem_node_mins->clear();
			my_agent->epmem_node_maxes->clear();
			my_agent->epmem_node_removals->clear();

			////

			// get/set RIT variables
			{
				intptr_t var_val = NIL;

				// offset
				if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_tree.offset.var_key, &var_val ) )
				{
					my_agent->epmem_rit_state_tree.offset.stat->set_value( var_val );
				}
				else
				{
					epmem_set_variable( my_agent, my_agent->epmem_rit_state_tree.offset.var_key, my_agent->epmem_rit_state_tree.offset.stat->get_value() );
				}

				// leftroot
				if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_tree.leftroot.var_key, &var_val ) )
				{
					my_agent->epmem_rit_state_tree.leftroot.stat->set_value( var_val );
				}
				else
				{
					epmem_set_variable( my_agent, my_agent->epmem_rit_state_tree.leftroot.var_key, my_agent->epmem_rit_state_tree.leftroot.stat->get_value() );
				}

				// rightroot
				if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_tree.rightroot.var_key, &var_val ) )
				{
					my_agent->epmem_rit_state_tree.rightroot.stat->set_value( var_val );
				}
				else
				{
					epmem_set_variable( my_agent, my_agent->epmem_rit_state_tree.rightroot.var_key, my_agent->epmem_rit_state_tree.rightroot.stat->get_value() );
				}

				// minstep
				if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_tree.minstep.var_key, &var_val ) )
				{
					my_agent->epmem_rit_state_tree.minstep.stat->set_value( var_val );
				}
				else
				{
					epmem_set_variable( my_agent, my_agent->epmem_rit_state_tree.minstep.var_key, my_agent->epmem_rit_state_tree.minstep.stat->get_value() );
				}
			}

			// get max time
			{
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "SELECT MAX(id) FROM times" );
				temp_q->prepare();
				if ( temp_q->execute() == soar_module::row )
					my_agent->epmem_stats->time->set_value( temp_q->column_int( 0 ) + 1 );
				
				delete temp_q;
				temp_q = NULL;
			}
			time_max = my_agent->epmem_stats->time->get_value();

			if ( !readonly )
			{
				time_last = ( time_max - 1 );
				
				// insert non-NOW intervals for all current NOW's
				temp_q = my_agent->epmem_stmts_tree->add_node_point;
				temp_q->bind_int( 2, time_last );
				
				temp_q2 = new soar_module::sqlite_statement( my_agent->epmem_db, "SELECT id,start FROM node_now" );
				temp_q2->prepare();
				while ( temp_q2->execute() == soar_module::row )
				{
					range_start = temp_q2->column_int( 1 );

					// point
					if ( range_start == time_last )
					{
						temp_q->bind_int( 1, temp_q2->column_int( 0 ) );
						temp_q->execute( soar_module::op_reinit );
					}
					else
						epmem_rit_insert_interval( my_agent, range_start, time_last, temp_q2->column_int( 0 ), &( my_agent->epmem_rit_state_tree ) );
				}
				delete temp_q2;
				temp_q2 = NULL;
				temp_q = NULL;	
				

				// remove all NOW intervals
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "DELETE FROM node_now" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}

			// get max id + max list
			temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "SELECT MAX(child_id) FROM node_unique" );
			temp_q->prepare();
			temp_q->execute();
			if ( temp_q->column_type( 0 ) != soar_module::null_t )
			{
				std::vector<bool>::size_type num_ids = temp_q->column_int( 0 );

				my_agent->epmem_node_maxes->resize( num_ids, true );
				my_agent->epmem_node_mins->resize( num_ids, time_max );
			}
			delete temp_q;
			temp_q = NULL;
		}
		else if ( mode == epmem_param_container::graph )
		{
			// setup tree structures/queries
			my_agent->epmem_stmts_graph = new epmem_graph_statement_container( my_agent );
			my_agent->epmem_stmts_graph->structure();
			my_agent->epmem_stmts_graph->prepare();
			
			// initialize range tracking
			my_agent->epmem_node_mins->clear();
			my_agent->epmem_node_maxes->clear();
			my_agent->epmem_node_removals->clear();

			my_agent->epmem_edge_mins->clear();
			my_agent->epmem_edge_maxes->clear();
			my_agent->epmem_edge_removals->clear();

			(*my_agent->epmem_id_repository)[ EPMEM_NODEID_ROOT ] = new epmem_hashed_id_pool;

			// initialize time
			my_agent->epmem_stats->time->set_value( 1 );

			// initialize next_id
			my_agent->epmem_stats->next_id->set_value( 1 );
			{
				intptr_t stored_id = NULL;
				if ( epmem_get_variable( my_agent, var_next_id, &stored_id ) )
				{
					my_agent->epmem_stats->next_id->set_value( stored_id );
				}
				else
				{
					epmem_set_variable( my_agent, var_next_id, my_agent->epmem_stats->next_id->get_value() );
				}
			}

			// initialize rit state
			for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
			{
				my_agent->epmem_rit_state_graph[ i ].offset.stat->set_value( EPMEM_RIT_OFFSET_INIT );
				my_agent->epmem_rit_state_graph[ i ].leftroot.stat->set_value( 0 );
				my_agent->epmem_rit_state_graph[ i ].rightroot.stat->set_value( 1 );
				my_agent->epmem_rit_state_graph[ i ].minstep.stat->set_value( LONG_MAX );
			}
			my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].add_query = my_agent->epmem_stmts_graph->add_node_range;
			my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].add_query = my_agent->epmem_stmts_graph->add_edge_range;		

			////

			// get/set RIT variables
			{
				intptr_t var_val = NIL;

				for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
				{
					// offset
					if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].offset.var_key, &var_val ) )
					{
						my_agent->epmem_rit_state_graph[ i ].offset.stat->set_value( var_val );
					}
					else
					{
						epmem_set_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].offset.var_key, my_agent->epmem_rit_state_graph[ i ].offset.stat->get_value() );
					}

					// leftroot
					if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].leftroot.var_key, &var_val ) )
					{
						my_agent->epmem_rit_state_graph[ i ].leftroot.stat->set_value( var_val );
					}
					else
					{
						epmem_set_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].leftroot.var_key, my_agent->epmem_rit_state_graph[ i ].leftroot.stat->get_value() );
					}

					// rightroot
					if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].rightroot.var_key, &var_val ) )
					{
						my_agent->epmem_rit_state_graph[ i ].rightroot.stat->set_value( var_val );
					}
					else
					{
						epmem_set_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].rightroot.var_key, my_agent->epmem_rit_state_graph[ i ].rightroot.stat->get_value() );
					}

					// minstep
					if ( epmem_get_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].minstep.var_key, &var_val ) )
					{
						my_agent->epmem_rit_state_graph[ i ].minstep.stat->set_value( var_val );
					}
					else
					{
						epmem_set_variable( my_agent, my_agent->epmem_rit_state_graph[ i ].minstep.var_key, my_agent->epmem_rit_state_graph[ i ].minstep.stat->get_value() );
					}
				}
			}

			////

			// get max time
			{
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "SELECT MAX(id) FROM times" );
				temp_q->prepare();
				if ( temp_q->execute() == soar_module::row )
					my_agent->epmem_stats->time->set_value( temp_q->column_int( 0 ) + 1 );
				
				delete temp_q;
				temp_q = NULL;
			}
			time_max = my_agent->epmem_stats->time->get_value();

			// insert non-NOW intervals for all current NOW's
			// remove NOW's
			if ( !readonly )
			{
				time_last = ( time_max - 1 );

				const char *now_select[] = { "SELECT id,start FROM node_now", "SELECT id,start FROM edge_now" };
				soar_module::sqlite_statement *now_add[] = { my_agent->epmem_stmts_graph->add_node_point, my_agent->epmem_stmts_graph->add_edge_point };
				const char *now_delete[] = { "DELETE FROM node_now", "DELETE FROM edge_now" };
				
				for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
				{
					temp_q = now_add[i];
					temp_q->bind_int( 2, time_last );
					
					temp_q2 = new soar_module::sqlite_statement( my_agent->epmem_db, now_select[i] );
					temp_q2->prepare();
					while ( temp_q2->execute() == soar_module::row )
					{
						range_start = temp_q2->column_int( 1 );

						// point
						if ( range_start == time_last )
						{
							temp_q->bind_int( 1, temp_q2->column_int( 0 ) );
							temp_q->execute( soar_module::op_reinit );
						}
						else
							epmem_rit_insert_interval( my_agent, range_start, time_last, temp_q2->column_int( 0 ), &( my_agent->epmem_rit_state_graph[i] ) );
					}
					delete temp_q2;
					temp_q2 = NULL;
					temp_q = NULL;	
					

					// remove all NOW intervals
					temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, now_delete[i] );
					temp_q->prepare();
					temp_q->execute();
					delete temp_q;
					temp_q = NULL;
				}
			}

			// get max id + max list
			{
				const char *minmax_select[] = { "SELECT MAX(child_id) FROM node_unique", "SELECT MAX(parent_id) FROM edge_unique" };
				std::vector<bool> *minmax_max[] = { my_agent->epmem_node_maxes, my_agent->epmem_edge_maxes };
				std::vector<epmem_time_id> *minmax_min[] = { my_agent->epmem_node_mins, my_agent->epmem_edge_mins };				

				for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
				{
					temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, minmax_select[i] );
					temp_q->prepare();
					temp_q->execute();
					if ( temp_q->column_type( 0 ) != soar_module::null_t )
					{
						std::vector<bool>::size_type num_ids = temp_q->column_int( 0 );

						minmax_max[i]->resize( num_ids, true );
						minmax_min[i]->resize( num_ids, time_max );
					}
					
					delete temp_q;
					temp_q = NULL;
				}
			}

			// get id pools
			{
				epmem_node_id q0;
				intptr_t w;
				epmem_node_id q1;
				epmem_node_id parent_id;

				epmem_hashed_id_pool **hp;
				epmem_id_pool **ip;

				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "SELECT q0, w, q1, parent_id FROM edge_unique" );
				temp_q->prepare();

				while ( temp_q->execute() == soar_module::row )
				{
					q0 = temp_q->column_int( 0 );
					w = temp_q->column_int( 1 );
					q1 = temp_q->column_int( 2 );
					parent_id = temp_q->column_int( 3 );					

					hp =& (*my_agent->epmem_id_repository)[ q0 ];
					if ( !(*hp) )
						(*hp) = new epmem_hashed_id_pool;

					ip =& (*(*hp))[ w ];
					if ( !(*ip) )
						(*ip) = new epmem_id_pool;

					(*(*ip))[ q1 ] = parent_id;
				}
				
				delete temp_q;
				temp_q = NULL;
			}
		}

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
epmem_hash_id epmem_temporal_hash( agent *my_agent, Symbol *sym, bool add_on_fail = true )
{
	epmem_hash_id return_val = NIL;

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->hash->start();
	////////////////////////////////////////////////////////////////////////////
	
	if ( ( sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) ||
		 ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) ||
		 ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) )
	{
		if ( ( !sym->common.epmem_hash ) || ( sym->common.epmem_valid != my_agent->epmem_validation ) )
		{		
			sym->common.epmem_hash = NIL;
			sym->common.epmem_valid = my_agent->epmem_validation;
			
			// basic process:
			// - search
			// - if found, return
			// - else, add
			
			my_agent->epmem_stmts_common->hash_get->bind_int( 1, sym->common.symbol_type );
			switch ( sym->common.symbol_type )
			{
				case SYM_CONSTANT_SYMBOL_TYPE:
					my_agent->epmem_stmts_common->hash_get->bind_text( 2, static_cast<const char *>( sym->sc.name ) );
					break;

				case INT_CONSTANT_SYMBOL_TYPE:
					my_agent->epmem_stmts_common->hash_get->bind_int( 2, sym->ic.value );
					break;

				case FLOAT_CONSTANT_SYMBOL_TYPE:
					my_agent->epmem_stmts_common->hash_get->bind_double( 2, sym->fc.value );
					break;
			}

			if ( my_agent->epmem_stmts_common->hash_get->execute() == soar_module::row )
			{
				return_val = static_cast<epmem_hash_id>( my_agent->epmem_stmts_common->hash_get->column_int( 0 ) );
			}

			my_agent->epmem_stmts_common->hash_get->reinitialize();

			//

			if ( !return_val && add_on_fail )
			{
				my_agent->epmem_stmts_common->hash_add->bind_int( 1, sym->common.symbol_type );
				switch ( sym->common.symbol_type )
				{
					case SYM_CONSTANT_SYMBOL_TYPE:
						my_agent->epmem_stmts_common->hash_add->bind_text( 2, static_cast<const char *>( sym->sc.name ) );
						break;

					case INT_CONSTANT_SYMBOL_TYPE:
						my_agent->epmem_stmts_common->hash_add->bind_int( 2, sym->ic.value );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						my_agent->epmem_stmts_common->hash_add->bind_double( 2, sym->fc.value );
						break;
				}

				my_agent->epmem_stmts_common->hash_add->execute( soar_module::op_reinit );
				return_val = static_cast<epmem_hash_id>( my_agent->epmem_db->last_insert_rowid() );
			}			

			// cache results for later re-use
			sym->common.epmem_hash = return_val;
			sym->common.epmem_valid = my_agent->epmem_validation;
		}

		return_val = sym->common.epmem_hash;
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->hash->stop();
	////////////////////////////////////////////////////////////////////////////

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
	if ( my_agent->epmem_db->get_status() == soar_module::disconnected )
		epmem_init_db( my_agent );

	// add the episode only if db is properly initialized
	if ( my_agent->epmem_db->get_status() != soar_module::connected )
		return;

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->storage->start();	
	////////////////////////////////////////////////////////////////////////////

	epmem_time_id time_counter = my_agent->epmem_stats->time->get_value();

	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%lu, %ld)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number, time_counter );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}

	epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();

	if ( mode == epmem_param_container::tree )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		// keeps children of the identifier of interest
		epmem_wme_list *wmes;
		epmem_wme_list::iterator w_p;		

		// future states of breadth-first search
		std::queue<Symbol *> syms;
		std::queue<epmem_node_id> ids;

		// current state
		epmem_node_id parent_id;

		// nodes to be recorded (implements tree flattening)
		std::map<epmem_node_id, bool> epmem;

		// wme hashing improves search speed
		epmem_hash_id my_hash = NIL;	// attribute
		epmem_hash_id my_hash2 = NIL;	// value

		// prevents infinite loops
		tc_number tc = get_new_tc_number( my_agent );

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
			wmes = epmem_get_augs_of_id( parent_sym, tc );
			
			{
				for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
				{
					// prevent exclusions from being recorded
					if ( my_agent->epmem_params->exclusions->in_set( (*w_p)->attr ) )
						continue;

					// if we haven't seen this WME before
					// or we haven't seen it within this database...
					// look it up in the database
					if ( ( (*w_p)->epmem_id == NIL ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) )
					{
						(*w_p)->epmem_id = NIL;
						(*w_p)->epmem_valid = my_agent->epmem_validation;

						if ( (*w_p)->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{
							my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
							my_hash2 = epmem_temporal_hash( my_agent, (*w_p)->value );

							// parent_id=? AND attr=? AND value=?
							my_agent->epmem_stmts_tree->find_node_unique->bind_int( 1, parent_id );
							my_agent->epmem_stmts_tree->find_node_unique->bind_int( 2, my_hash );
							my_agent->epmem_stmts_tree->find_node_unique->bind_int( 3, my_hash2 );

							if ( my_agent->epmem_stmts_tree->find_node_unique->execute() == soar_module::row )
								(*w_p)->epmem_id = my_agent->epmem_stmts_tree->find_node_unique->column_int( 0 );


							my_agent->epmem_stmts_tree->find_node_unique->reinitialize();
						}
						else
						{
							my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );

							// parent_id=? AND attr=? AND value IS NULL
							my_agent->epmem_stmts_tree->find_identifier->bind_int( 1, parent_id );
							my_agent->epmem_stmts_tree->find_identifier->bind_int( 2, my_hash );

							if ( my_agent->epmem_stmts_tree->find_identifier->execute() == soar_module::row )
								(*w_p)->epmem_id = my_agent->epmem_stmts_tree->find_identifier->column_int( 0 );

							my_agent->epmem_stmts_tree->find_identifier->reinitialize();
						}
					}

					// insert on no id
					if ( (*w_p)->epmem_id == NIL )
					{
						// insert (parent_id,attr,value)
						my_agent->epmem_stmts_tree->add_node_unique->bind_int( 1, parent_id );
						my_agent->epmem_stmts_tree->add_node_unique->bind_int( 2, my_hash );

						switch ( (*w_p)->value->common.symbol_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
							case INT_CONSTANT_SYMBOL_TYPE:
							case FLOAT_CONSTANT_SYMBOL_TYPE:
								my_agent->epmem_stmts_tree->add_node_unique->bind_int( 3, my_hash2 );								
								break;

							case IDENTIFIER_SYMBOL_TYPE:
								my_agent->epmem_stmts_tree->add_node_unique->bind_int( 3, 0 );								
								break;
						}
						my_agent->epmem_stmts_tree->add_node_unique->execute( soar_module::op_reinit );

						(*w_p)->epmem_id = (epmem_node_id) my_agent->epmem_db->last_insert_rowid();

						// new nodes definitely start
						epmem[ (*w_p)->epmem_id ] = true;
						my_agent->epmem_node_mins->push_back( time_counter );
						my_agent->epmem_node_maxes->push_back( false );
					}
					else
					{
						// definitely don't remove
						(*my_agent->epmem_node_removals)[ (*w_p)->epmem_id ] = false;

						// we add ONLY if the last thing we did was a remove
						if ( (*my_agent->epmem_node_maxes)[ (*w_p)->epmem_id - 1 ] )
						{
							epmem[ (*w_p)->epmem_id ] = true;
							(*my_agent->epmem_node_maxes)[ (*w_p)->epmem_id - 1 ] = false;
						}
					}

					// keep track of identifiers (for further study)
					if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						syms.push( (*w_p)->value );
						ids.push( (*w_p)->epmem_id );
					}
				}

				// free space from aug list
				delete wmes;
			}
		}

		// all inserts at once (provides unique)
		std::map<epmem_node_id, bool>::iterator e = epmem.begin();
		while ( e != epmem.end() )
		{
			// add NOW entry
			// id = ?, start = ?
			my_agent->epmem_stmts_tree->add_node_now->bind_int( 1, e->first );
			my_agent->epmem_stmts_tree->add_node_now->bind_int( 2, time_counter );
			my_agent->epmem_stmts_tree->add_node_now->execute( soar_module::op_reinit );

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
				my_agent->epmem_stmts_tree->delete_node_now->bind_int( 1, r->first );
				my_agent->epmem_stmts_tree->delete_node_now->execute( soar_module::op_reinit );				

				range_start = (*my_agent->epmem_node_mins)[ r->first - 1 ];
				range_end = ( time_counter - 1 );

				// point (id, start)
				if ( range_start == range_end )
				{
					my_agent->epmem_stmts_tree->add_node_point->bind_int( 1, r->first );
					my_agent->epmem_stmts_tree->add_node_point->bind_int( 2, range_start );
					my_agent->epmem_stmts_tree->add_node_point->execute( soar_module::op_reinit );					
				}
				// node
				else
					epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( my_agent->epmem_rit_state_tree ) );

				// update max
				(*my_agent->epmem_node_maxes)[ r->first - 1 ] = true;
			}

			r++;
		}
		my_agent->epmem_node_removals->clear();

		// add the time id to the times table
		my_agent->epmem_stmts_tree->add_time->bind_int( 1, time_counter );
		my_agent->epmem_stmts_tree->add_time->execute( soar_module::op_reinit );

		my_agent->epmem_stats->time->set_value( time_counter + 1 );
	}
	else if ( mode == epmem_param_container::graph )
	{
		// prevents infinite loops
		tc_number tc = get_new_tc_number( my_agent );
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
		epmem_hash_id my_hash;	// attribute
		epmem_hash_id my_hash2;	// value

		// id repository
		epmem_id_pool **my_id_repo;
		epmem_id_pool *my_id_repo2;
		epmem_id_pool::iterator pool_p;
		std::map<wme *, epmem_id_reservation *> id_reservations;
		std::map<wme *, epmem_id_reservation *>::iterator r_p;
		epmem_id_reservation *new_id_reservation;

		// children of the current identifier
		epmem_wme_list *wmes;
		epmem_wme_list::iterator w_p;

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
			wmes = epmem_get_augs_of_id( parent_sym, tc );
			
			{
				// pre-assign unknown identifiers with known children (prevents ordering issues with unknown children)
				for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
				{
					if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
						 ( ( (*w_p)->epmem_id == NIL ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) ) &&
						 ( (*w_p)->value->id.epmem_id ) )
					{
						// prevent exclusions from being recorded
						if ( my_agent->epmem_params->exclusions->in_set( (*w_p)->attr ) )
							continue;

						// if still here, create reservation (case 1)
						new_id_reservation = new epmem_id_reservation;
						new_id_reservation->my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
						new_id_reservation->my_id = EPMEM_NODEID_ROOT;
						new_id_reservation->my_pool = NULL;

						// try to find appropriate reservation
						my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ new_id_reservation->my_hash ];
						if ( (*my_id_repo) )
						{
							if ( !(*my_id_repo)->empty() )
							{
								pool_p = (*my_id_repo)->find( (*w_p)->value->id.epmem_id );
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
							(*my_id_repo) = new epmem_id_pool;
						}

						new_id_reservation->my_pool = (*my_id_repo);
						id_reservations[ (*w_p) ] = new_id_reservation;
						new_id_reservation = NULL;
					}
				}

				for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
				{
					// prevent exclusions from being recorded
					if ( my_agent->epmem_params->exclusions->in_set( (*w_p)->attr ) )
						continue;

					if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						// have we seen this WME during this database?
						if ( ( (*w_p)->epmem_id == NIL ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) )
						{
							(*w_p)->epmem_valid = my_agent->epmem_validation;
							(*w_p)->epmem_id = NIL;

							my_hash = NULL;							
							my_id_repo2 = NULL;

							// in the case of a known child, we already have a reservation (case 1)						
							if ( (*w_p)->value->id.epmem_id )
							{
								r_p = id_reservations.find( (*w_p) );

								if ( r_p != id_reservations.end() )
								{
									// restore reservation info
									my_hash = r_p->second->my_hash;
									my_id_repo2 = r_p->second->my_pool;

									if ( r_p->second->my_id != EPMEM_NODEID_ROOT )
									{
										(*w_p)->epmem_id = r_p->second->my_id;
										(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = my_id_repo2;
									}

									// delete reservation and map entry
									delete r_p->second;
									id_reservations.erase( r_p );
								}
								// OR a shared identifier at the same level, in which case we need an exact match (case 2)
								else
								{
									// get temporal hash
									my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );

									// try to get an id that matches new information
									my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ my_hash ];
									if ( (*my_id_repo) )
									{
										if ( !(*my_id_repo)->empty() )
										{
											pool_p = (*my_id_repo)->find( (*w_p)->value->id.epmem_id );
											if ( pool_p != (*my_id_repo)->end() )
											{
												(*w_p)->epmem_id = pool_p->second;
												(*my_id_repo)->erase( pool_p );

												(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = (*my_id_repo);											
											}
										}
									}
									else
									{
										// add repository
										(*my_id_repo) = new epmem_id_pool;
									}

									// keep the address for later use
									my_id_repo2 = (*my_id_repo);
								}
							}
							// case 3
							else
							{
								// get temporal hash
								my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );

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
												(*my_agent->epmem_identifier_to_id)[ (*w_p)->value ] = pool_p->first;
												(*my_agent->epmem_id_to_identifier)[ pool_p->first ] = (*w_p)->value;

												(*w_p)->epmem_id = pool_p->second;
												(*w_p)->value->id.epmem_id = pool_p->first;
												(*my_id_repo)->erase( pool_p );
												(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = (*my_id_repo);

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
									(*my_id_repo) = new epmem_id_pool;
								}

								// keep the address for later use
								my_id_repo2 = (*my_id_repo);
							}

							// add path if no success above
							if ( (*w_p)->epmem_id == NIL )
							{
								if ( (*w_p)->value->id.epmem_id == NIL )
								{
									// update next id
									(*w_p)->value->id.epmem_id = my_agent->epmem_stats->next_id->get_value();
									my_agent->epmem_stats->next_id->set_value( (*w_p)->value->id.epmem_id + 1 );
									epmem_set_variable( my_agent, var_next_id, (*w_p)->value->id.epmem_id + 1 );

									// add repository
									(*my_agent->epmem_id_repository)[ (*w_p)->value->id.epmem_id ] = new epmem_hashed_id_pool;
								}

								// insert (q0,w,q1)
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 1, parent_id );
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 2, my_hash );
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 3, (*w_p)->value->id.epmem_id );
								my_agent->epmem_stmts_graph->add_edge_unique->execute( soar_module::op_reinit );

								(*w_p)->epmem_id = (epmem_node_id) my_agent->epmem_db->last_insert_rowid();
								(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = my_id_repo2;

								// new nodes definitely start
								epmem_edge.push( (*w_p)->epmem_id );
								my_agent->epmem_edge_mins->push_back( time_counter );
								my_agent->epmem_edge_maxes->push_back( false );
							}
							else
							{
								// definitely don't remove
								(*my_agent->epmem_edge_removals)[ (*w_p)->epmem_id ] = false;

								// we add ONLY if the last thing we did was remove
								if ( (*my_agent->epmem_edge_maxes)[ (*w_p)->epmem_id - 1 ] )
								{
									epmem_edge.push( (*w_p)->epmem_id );
									(*my_agent->epmem_edge_maxes)[ (*w_p)->epmem_id - 1 ] = false;
								}
							}
						}						

						// path id in cache?
						seen_p = seen_ids.find( (*w_p)->value->id.epmem_id );
						if ( seen_p == seen_ids.end() )
						{
							// future exploration
							parent_syms.push( (*w_p)->value );
							parent_ids.push( (*w_p)->value->id.epmem_id );
						}
					}
					else
					{
						// have we seen this node in this database?
						if ( ( (*w_p)->epmem_id == NIL ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) )
						{
							(*w_p)->epmem_id = NULL;
							(*w_p)->epmem_valid = my_agent->epmem_validation;

							my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
							my_hash2 = epmem_temporal_hash( my_agent, (*w_p)->value );

							// try to get node id
							{
								// parent_id=? AND attr=? AND value=?
								my_agent->epmem_stmts_graph->find_node_unique->bind_int( 1, parent_id );
								my_agent->epmem_stmts_graph->find_node_unique->bind_int( 2, my_hash );
								my_agent->epmem_stmts_graph->find_node_unique->bind_int( 3, my_hash2 );

								if ( my_agent->epmem_stmts_graph->find_node_unique->execute() == soar_module::row )
									(*w_p)->epmem_id = my_agent->epmem_stmts_graph->find_node_unique->column_int( 0 );

								my_agent->epmem_stmts_graph->find_node_unique->reinitialize();								
							}

							// act depending on new/existing feature
							if ( (*w_p)->epmem_id == NIL )
							{
								// insert (parent_id,attr,value)
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 1, parent_id );
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 2, my_hash );
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 3, my_hash2 );
								my_agent->epmem_stmts_graph->add_node_unique->execute( soar_module::op_reinit );

								(*w_p)->epmem_id = (epmem_node_id) my_agent->epmem_db->last_insert_rowid();

								// new nodes definitely start
								epmem_node.push( (*w_p)->epmem_id );
								my_agent->epmem_node_mins->push_back( time_counter );
								my_agent->epmem_node_maxes->push_back( false );
							}
							else
							{
								// definitely don't remove
								(*my_agent->epmem_node_removals)[ (*w_p)->epmem_id ] = false;

								// add ONLY if the last thing we did was add
								if ( (*my_agent->epmem_node_maxes)[ (*w_p)->epmem_id - 1 ] )
								{
									epmem_node.push( (*w_p)->epmem_id );
									(*my_agent->epmem_node_maxes)[ (*w_p)->epmem_id - 1 ] = false;
								}
							}
						}
					}
				}

				// free space from aug list
				delete wmes;
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
				my_agent->epmem_stmts_graph->add_node_now->bind_int( 1, (*temp_node) );
				my_agent->epmem_stmts_graph->add_node_now->bind_int( 2, time_counter );
				my_agent->epmem_stmts_graph->add_node_now->execute( soar_module::op_reinit );

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
				my_agent->epmem_stmts_graph->add_edge_now->bind_int( 1, (*temp_node) );
				my_agent->epmem_stmts_graph->add_edge_now->bind_int( 2, time_counter );
				my_agent->epmem_stmts_graph->add_edge_now->execute( soar_module::op_reinit );

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
					my_agent->epmem_stmts_graph->delete_node_now->bind_int( 1, r->first );
					my_agent->epmem_stmts_graph->delete_node_now->execute( soar_module::op_reinit );

					range_start = (*my_agent->epmem_node_mins)[ r->first - 1 ];
					range_end = ( time_counter - 1 );

					// point (id, start)
					if ( range_start == range_end )
					{
						my_agent->epmem_stmts_graph->add_node_point->bind_int( 1, r->first );
						my_agent->epmem_stmts_graph->add_node_point->bind_int( 2, range_start );
						my_agent->epmem_stmts_graph->add_node_point->execute( soar_module::op_reinit );
					}
					// node
					else
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );

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
					my_agent->epmem_stmts_graph->delete_edge_now->bind_int( 1, r->first );
					my_agent->epmem_stmts_graph->delete_edge_now->execute( soar_module::op_reinit );

					range_start = (*my_agent->epmem_edge_mins)[ r->first - 1 ];
					range_end = ( time_counter - 1 );

					// point (id, start)
					if ( range_start == range_end )
					{
						my_agent->epmem_stmts_graph->add_edge_point->bind_int( 1, r->first );
						my_agent->epmem_stmts_graph->add_edge_point->bind_int( 2, range_start );
						my_agent->epmem_stmts_graph->add_edge_point->execute( soar_module::op_reinit );						
					}
					// node
					else
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

					// update max
					(*my_agent->epmem_edge_maxes)[ r->first - 1 ] = true;
				}

				r++;
			}
			my_agent->epmem_edge_removals->clear();
		}

		// add the time id to the times table
		my_agent->epmem_stmts_graph->add_time->bind_int( 1, time_counter );
		my_agent->epmem_stmts_graph->add_time->execute( soar_module::op_reinit );

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
	epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();
	bool return_val = false;
	soar_module::sqlite_statement *my_q;

	if ( mode == epmem_param_container::tree )
	{
		my_q = my_agent->epmem_stmts_tree->valid_episode;
		
		my_q->bind_int( 1, memory_id );
		my_q->execute();
		return_val = ( my_q->column_int( 0 ) > 0 );
		my_q->reinitialize();
	}
	else if ( mode == epmem_param_container::graph )
	{
		my_q = my_agent->epmem_stmts_graph->valid_episode;
		
		my_q->bind_int( 1, memory_id );
		my_q->execute();
		return_val = ( my_q->column_int( 0 ) > 0 );
		my_q->reinitialize();
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
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_sym_retrieved, my_agent->epmem_sym_no_memory );
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
	epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_sym_retrieved, retrieved_header );	
	symbol_remove_ref( my_agent, retrieved_header );

	// add *-id wme's
	{
		Symbol *my_meta;

		my_meta = make_int_constant( my_agent, static_cast<long>( memory_id ) );
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_sym_memory_id, my_meta );		
		symbol_remove_ref( my_agent, my_meta );

		my_meta = make_int_constant( my_agent, static_cast<long>( my_agent->epmem_stats->time->get_value() ) );
		epmem_add_meta_wme( my_agent, state, result_header, my_agent->epmem_sym_present_id, my_meta );		
		symbol_remove_ref( my_agent, my_meta );
	}

	epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();

	if ( mode == epmem_param_container::tree )
	{
		epmem_id_mapping ids;
		epmem_node_id child_id;
		epmem_node_id parent_id;
		intptr_t attr_type;
		intptr_t value_type;
		Symbol *attr = NULL;
		Symbol *value = NULL;
		Symbol *parent = NULL;

		soar_module::sqlite_statement *my_q = my_agent->epmem_stmts_tree->get_episode;

		ids[ 0 ] = retrieved_header;

		epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_tree ) );

		my_q->bind_int( 1, memory_id );
		my_q->bind_int( 2, memory_id );
		my_q->bind_int( 3, memory_id );
		my_q->bind_int( 4, memory_id );		
		while ( my_q->execute() == soar_module::row )
		{
			// e.id, i.parent_id, i.name, i.value, i.attr_type, i.value_type
			child_id = my_q->column_int( 0 );
			parent_id = my_q->column_int( 1 );
			attr_type = my_q->column_int( 4 );
			value_type = my_q->column_int( 5 );

			// make a symbol to represent the attribute name
			switch ( attr_type )
			{
				case INT_CONSTANT_SYMBOL_TYPE:
					attr = make_int_constant( my_agent, static_cast<long>( my_q->column_int( 2 ) ) );
					break;

				case FLOAT_CONSTANT_SYMBOL_TYPE:
					attr = make_float_constant( my_agent, my_q->column_double( 2 ) );
					break;

				case SYM_CONSTANT_SYMBOL_TYPE:
					attr = make_sym_constant( my_agent, const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 2 ) ) ) );
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
						value = make_int_constant( my_agent, static_cast<long>( my_q->column_int( 3 ) ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						value = make_float_constant( my_agent, my_q->column_double( 3 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						value = make_sym_constant( my_agent, const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 3 ) ) ) );
						break;
				}

				epmem_add_retrieved_wme( my_agent, state, parent, attr, value );				
				num_wmes++;
			}

			symbol_remove_ref( my_agent, attr );
			symbol_remove_ref( my_agent, value );
		}
		my_q->reinitialize();

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

		soar_module::sqlite_statement *my_q;

		// initialize the lookup table
		if ( id_reuse )
			ids = (*id_reuse);

		// first identifiers (i.e. reconstruct)
		my_q = my_agent->epmem_stmts_graph->get_edges;
		ids[ 0 ] = retrieved_header;
		{
			// relates to finite automata: q1 = d(q0, w)
			epmem_node_id q0; // id
			epmem_node_id q1; // attribute
			Symbol **value = NULL; // value
			intptr_t w_type; // we support any constant attribute symbol

			// used to lookup shared identifiers
			epmem_id_mapping::iterator id_p;

			// orphaned children
			std::queue<epmem_edge *> orphans;
			epmem_edge *orphan;			

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				// q0, w, q1, w_type
				q0 = my_q->column_int( 0 );
				q1 = my_q->column_int( 2 );
				w_type = my_q->column_int( 3 );

				switch ( w_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						attr = make_int_constant( my_agent, static_cast<long>( my_q->column_int( 1 ) ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						attr = make_float_constant( my_agent, my_q->column_double( 1 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						attr = make_sym_constant( my_agent, const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 1 ) ) ) );
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
			my_q->reinitialize();			
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
		my_q = my_agent->epmem_stmts_graph->get_nodes;
		{
			epmem_node_id child_id;
			epmem_node_id parent_id;
			intptr_t attr_type;
			intptr_t value_type;

			Symbol *value = NULL;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );

			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				// f.child_id, f.parent_id, f.name, f.value, f.attr_type, f.value_type
				child_id = my_q->column_int( 0 );
				parent_id = my_q->column_int( 1 );
				attr_type = my_q->column_int( 4 );
				value_type = my_q->column_int( 5 );

				// get a reference to the parent
				parent = ids[ parent_id ];

				// make a symbol to represent the attribute
				switch ( attr_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						attr = make_int_constant( my_agent, static_cast<long>( my_q->column_int( 2 ) ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						attr = make_float_constant( my_agent, my_q->column_double( 2 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						attr = make_sym_constant( my_agent, const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 2 ) ) ) );
						break;
				}

				// make a symbol to represent the value
				switch ( value_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						value = make_int_constant( my_agent, static_cast<long>( my_q->column_int( 3 ) ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						value = make_float_constant( my_agent, my_q->column_double( 3 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						value = make_sym_constant( my_agent, const_cast<char *>( (const char *) my_q->column_text( 3 ) ) );
						break;
				}

				epmem_add_retrieved_wme( my_agent, state, parent, attr, value );
				num_wmes++;

				symbol_remove_ref( my_agent, attr );
				symbol_remove_ref( my_agent, value );
			}
			my_q->reinitialize();
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

	epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();
	epmem_time_id return_val = EPMEM_MEMID_NONE;
	soar_module::sqlite_statement *my_q;

	if ( memory_id != EPMEM_MEMID_NONE )
	{
		if ( mode == epmem_param_container::tree )
		{
			my_q = my_agent->epmem_stmts_tree->next_episode;
			my_q->bind_int( 1, memory_id );
			if ( my_q->execute() == soar_module::row )
				return_val = (epmem_time_id) my_q->column_int( 0 );

			my_q->reinitialize();
		}
		else if ( mode == epmem_param_container::graph )
		{
			my_q = my_agent->epmem_stmts_graph->next_episode;
			my_q->bind_int( 1, memory_id );
			if ( my_q->execute() == soar_module::row )
				return_val = (epmem_time_id) my_q->column_int( 0 );

			my_q->reinitialize();			
		}
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

	epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();
	epmem_time_id return_val = EPMEM_MEMID_NONE;
	soar_module::sqlite_statement *my_q;

	if ( memory_id != EPMEM_MEMID_NONE )
	{
		if ( mode == epmem_param_container::tree )
		{
			my_q = my_agent->epmem_stmts_tree->prev_episode;
			my_q->bind_int( 1, memory_id );
			if ( my_q->execute() == soar_module::row )
				return_val = (epmem_time_id) my_q->column_int( 0 );

			my_q->reinitialize();
		}
		else if ( mode == epmem_param_container::graph )
		{
			my_q = my_agent->epmem_stmts_graph->prev_episode;
			my_q->bind_int( 1, memory_id );
			if ( my_q->execute() == soar_module::row )
				return_val = (epmem_time_id) my_q->column_int( 0 );

			my_q->reinitialize();
		}
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
	epmem_leaf_node *newbie = new epmem_leaf_node;

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
		more_data = ( temp_query->stmt->execute() == soar_module::row );
		if ( more_data )
		{
			// if so, add back to the priority queue
			temp_query->val = temp_query->stmt->column_int( 0 );
			queries[ list ].push( temp_query );
		}
		else
		{
			// else, away with ye
			delete temp_query->stmt;
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
void epmem_shared_flip( epmem_shared_literal_pair *flip, const unsigned int list, long &ct, double &v, long &updown )
{
	long ct_change = ( ( list == EPMEM_RANGE_START )?( -1 ):( 1 ) );
	
	// if recursive propogation, count change
	// is dependent upon wme count
	bool alter_ct = true;
	if ( flip->lit->wme_ct )
	{	
		alter_ct = false;
		
		// find the wme
		epmem_shared_wme_counter **wme_book =& (*flip->lit->wme_ct)[ flip->wme ];
		if ( !(*wme_book) )
		{
			(*wme_book) = new epmem_shared_wme_counter;
			(*wme_book)->wme_ct = 0;
			
			(*wme_book)->lit_ct = new epmem_shared_literal_counter;
		}

		// find the shared count
		unsigned long *lit_ct =& (*(*wme_book)->lit_ct)[ flip->q0 ];
		(*lit_ct) += ct_change;

		// if appropriate, change the wme count
		if ( (*lit_ct) == ( (unsigned long) ( ( list == EPMEM_RANGE_START )?( EPMEM_DNF - 1 ):( EPMEM_DNF ) ) ) )
		{
			(*wme_book)->wme_ct += ct_change;

			alter_ct = ( (*wme_book)->wme_ct == ( (unsigned long) ( ( list == EPMEM_RANGE_START )?( 0 ):( 1 ) ) ) );
		}
	}

	if ( alter_ct )
	{
		unsigned long max_compare = ( ( list == EPMEM_RANGE_START )?( flip->lit->max - 1 ):( flip->lit->max ) );

		flip->lit->ct += ct_change;
		if ( flip->lit->ct == max_compare )
		{
			if ( flip->lit->children )
			{
				epmem_shared_literal_pair_list::iterator literal_p;

				for ( literal_p=flip->lit->children->literals->begin(); literal_p!=flip->lit->children->literals->end(); literal_p++ )
				{
					epmem_shared_flip( (*literal_p), list, ct, v, updown );
				}
			}
			else if ( flip->lit->match )
			{
				unsigned long match_compare = ( ( list == EPMEM_RANGE_START )?( 0 ):( 1 ) );
				flip->lit->match->ct += ct_change;

				if ( flip->lit->match->ct == match_compare )
				{
					ct += flip->lit->match->value_ct;
					v += flip->lit->match->value_weight;

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
	epmem_shared_literal_pair_list::iterator literal_p;

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
		more_data = ( temp_query->stmt->execute() == soar_module::row );
		if ( more_data )
		{
			// if so, add back to the priority queue
			temp_query->val = temp_query->stmt->column_int( 0 );
			queries[ list ].push( temp_query );
		}
		else
		{
			// away with ye
			delete temp_query->stmt;
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

	// literals are grouped together sequentially by WME.
	epmem_shared_wme_index::iterator c_f;

	// current values from the stacks
	epmem_shared_literal_list::size_type c_p;
	epmem_constraint_list *c_c = NULL;
	epmem_node_id c_id;

	// derived values from current values
	epmem_shared_literal_pair *c_l;

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
		c_f = literals->wme_index->begin();
		literals->c_wme = c_l->wme;

		// current constraints = previous constraints
		c_c = new epmem_constraint_list( *constraints );

		// get constraint for this identifier, if exists
		c_id = EPMEM_NODEID_ROOT;
		if ( c_l->lit->wme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
		{
			c = c_c->find( c_l->lit->wme->value );
			if ( c != c_c->end() )
				c_id = c->second;
		}

		do
		{
			// determine if literal is a match
			{
				good_literal = false;
				n_c = NULL;

				// must be ON
				if ( c_l->lit->ct == c_l->lit->max )
				{
					// cue identifier
					if ( c_l->lit->shared_id != EPMEM_NODEID_ROOT )
					{
						// check if unconstrained
						if ( c_id == EPMEM_NODEID_ROOT )
						{
							// if substructure, check
							if ( c_l->lit->children )
							{
								// copy constraints
								n_c = new epmem_constraint_list( *c_c );

								// try DFS
								if ( epmem_graph_match( c_l->lit->children, n_c ) == c_l->lit->children->wme_index->size() )
								{
									// on success, keep new constraints
									good_literal = true;									

									// update constraints with this literal
									(*n_c)[ c_l->lit->wme->value ] = c_l->lit->shared_id;
								}
								else
								{
									delete n_c;
								}								
							}
							// otherwise winner by default, pass along constraint
							else
							{
								good_literal = true;

								n_c = new epmem_constraint_list( *c_c );
								(*n_c)[ c_l->lit->wme->value ] = c_l->lit->shared_id;
							}
						}
						else
						{
							// if shared identifier, we don't need to perform recursion
							// (we rely upon previous results)
							good_literal = ( c_id == c_l->lit->shared_id );

							if ( good_literal )
							{
								n_c = new epmem_constraint_list( *c_c );
							}
						}
					}
					// leaf node, non-identifier
					else
					{
						good_literal = ( c_l->lit->match->ct != 0 );

						if ( good_literal )
						{
							n_c = new epmem_constraint_list( *c_c );
						}
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
				if ( c_f == literals->wme_index->end() )
				{
					(*c_c) = (*n_c);
					delete n_c;

					done = true;
				}
				else
				// push, try next wme with new constraints
				if ( !done )
				{
					c_ps.push( c_p );
					c_cs.push( c_c );					

					c_p = (*c_f);
					c_l = (*literals->literals)[ c_p ];
					literals->c_wme = c_l->wme;

					c_c = n_c;

					// get constraint for this identifier, if exists
					c_id = EPMEM_NODEID_ROOT;
					if ( c_l->lit->wme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						c = c_c->find( c_l->lit->wme->value );
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
					if ( c_p < literals->literals->size() )
					{						
						// else, look at the literal
						c_l = (*literals->literals)[ c_p ];

						// if still within the wme, we can try again
						// with current constraints
						if ( c_l->wme == literals->c_wme )
						{
							good_pop = true;

							// get constraint for this identifier, if exists
							c_id = EPMEM_NODEID_ROOT;
							if ( c_l->lit->wme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
							{
								c = c_c->find( c_l->lit->wme->value );
								if ( c != c_c->end() )
									c_id = c->second;
							}
						}
					}
						
					if ( !good_pop )
					{
						// if nothing left on the stack, failure
						if ( c_ps.empty() )
						{
							done = true;
						}
						else
						{
							// otherwise, backtrack:
							// - pop previous state, remove last constraint
							// - repeat trying to increment (and possibly have to recursively pop again)

							// recover state
							c_p = c_ps.top();
							c_ps.pop();

							c_l = (*literals->literals)[ c_p ];
							literals->c_wme = c_l->wme;

							c_f--;
							return_val--;

							// recover constraints
							delete c_c;
							c_c = c_cs.top();
							c_cs.pop();
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
	
	epmem_wme_list *wmes_query = NULL;
	if ( query != NULL )
		wmes_query = epmem_get_augs_of_id( query, get_new_tc_number( my_agent ) );

	epmem_wme_list *wmes_neg_query = NULL;
	if ( neg_query != NULL )
		wmes_neg_query = epmem_get_augs_of_id( neg_query, get_new_tc_number( my_agent ) );

	// only perform a query if there potentially valid cue(s)
	if ( wmes_query || wmes_neg_query )
	{
		epmem_param_container::mode_choices mode = my_agent->epmem_params->mode->get_value();

		if ( !prohibit->empty() )
			std::sort( prohibit->begin(), prohibit->end() );

		if ( mode == epmem_param_container::tree )
		{
			// BFS to get the leaf id's
			std::list<epmem_leaf_node *> leaf_ids[2];
			std::list<epmem_leaf_node *>::iterator leaf_p;
			epmem_time_list::iterator prohibit_p;
			{
				epmem_wme_list **wmes = NULL;

				std::queue<Symbol *> parent_syms;
				std::queue<epmem_node_id> parent_ids;
				std::queue<wme *> parent_wmes;
				tc_number tc = get_new_tc_number( my_agent );

				Symbol *parent_sym;
				epmem_node_id parent_id;
				wme *parent_wme;

				// temporal hashing
				epmem_hash_id my_hash;	// attribute
				epmem_hash_id my_hash2;	// value

				int i;
				epmem_wme_list::iterator w_p;
				bool just_started = true;

				// initialize pos/neg lists
				for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
				{
					switch ( i )
					{
						case EPMEM_NODE_POS:
							wmes = &wmes_query;							
							parent_syms.push( query );
							parent_ids.push( EPMEM_NODEID_ROOT );
							parent_wmes.push( NULL );
							just_started = true;
							break;

						case EPMEM_NODE_NEG:
							wmes = &wmes_neg_query;							
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
							(*wmes) = epmem_get_augs_of_id( parent_sym, tc );

						if ( (*wmes) )
						{							
							if ( !(*wmes)->empty() )
							{
								for ( w_p=(*wmes)->begin(); w_p!=(*wmes)->end(); w_p++ )
								{
									// add to cue list
									state->id.epmem_info->cue_wmes->insert( (*w_p) );

									// find wme id
									if ( (*w_p)->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
									{
										my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
										my_hash2 = epmem_temporal_hash( my_agent, (*w_p)->value );

										// parent_id=? AND attr=? AND value=?
										my_agent->epmem_stmts_tree->find_node_unique->bind_int( 1, parent_id );
										my_agent->epmem_stmts_tree->find_node_unique->bind_int( 2, my_hash );
										my_agent->epmem_stmts_tree->find_node_unique->bind_int( 3, my_hash2 );

										if ( my_agent->epmem_stmts_tree->find_node_unique->execute() == soar_module::row )
											leaf_ids[i].push_back( epmem_create_leaf_node( my_agent->epmem_stmts_tree->find_node_unique->column_int( 0 ), wma_get_wme_activation( my_agent, (*w_p) ) ) );

										my_agent->epmem_stmts_tree->find_node_unique->reinitialize();
									}
									else
									{
										my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );

										// parent_id=? AND attr=?
										my_agent->epmem_stmts_tree->find_identifier->bind_int( 1, parent_id );
										my_agent->epmem_stmts_tree->find_identifier->bind_int( 2, my_hash );

										if ( my_agent->epmem_stmts_tree->find_identifier->execute() == soar_module::row )
										{
											parent_syms.push( (*w_p)->value );
											parent_ids.push( (epmem_node_id) my_agent->epmem_stmts_tree->find_identifier->column_int( 0 ) );
											parent_wmes.push( (*w_p) );
										}

										my_agent->epmem_stmts_tree->find_identifier->reinitialize();
									}
								}
							}
							else
							{
								if ( !just_started )
									leaf_ids[i].push_back( epmem_create_leaf_node( parent_id, wma_get_wme_activation( my_agent, parent_wme ) ) );
							}

							// free space from aug list
							delete (*wmes);

							just_started = false;
						}						
					}
				}
			}

			my_agent->epmem_stats->qry_pos->set_value( static_cast<long>( leaf_ids[ EPMEM_NODE_POS ].size() ) );
			my_agent->epmem_stats->qry_neg->set_value( static_cast<long>( leaf_ids[ EPMEM_NODE_NEG ].size() ) );
			my_agent->epmem_stats->qry_ret->set_value( 0 );
			my_agent->epmem_stats->qry_card->set_value( 0 );

			// useful statistics
			std::list<epmem_leaf_node *>::size_type cue_sizes[2] = { leaf_ids[ EPMEM_NODE_POS ].size(), leaf_ids[ EPMEM_NODE_NEG ].size() };
			std::list<epmem_leaf_node *>::size_type cue_size = ( cue_sizes[ EPMEM_NODE_POS ] + cue_sizes[ EPMEM_NODE_NEG ] );
			std::list<epmem_leaf_node *>::size_type perfect_match = leaf_ids[ EPMEM_NODE_POS ].size();

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
						epmem_time_id time_now = my_agent->epmem_stats->time->get_value() - 1;
						int position;

						epmem_range_query *new_query = NULL;
						soar_module::sqlite_statement *new_stmt = NULL;
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
											new_stmt = new soar_module::sqlite_statement( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_NODE ][ j ][ k ], new_timer );
											new_stmt->prepare();

											// bind values
											position = 1;

											if ( ( k == EPMEM_RANGE_NOW ) && ( j == EPMEM_RANGE_END ) )
												new_stmt->bind_int( position++, time_now );
											new_stmt->bind_int( position, (*leaf_p)->leaf_id );											

											if ( new_stmt->execute() == soar_module::row )
											{
												new_query = new epmem_range_query;
												new_query->val = new_stmt->column_int( 0 );
												new_query->stmt = new_stmt;												

												new_query->ct = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) );
												new_query->weight = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) ) * (*leaf_p)->leaf_weight;

												// add to query priority queue
												queries[ j ].push( new_query );
												new_query = NULL;
											}
											else
											{
												delete new_stmt;
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
						unsigned int next_list = NIL;

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

								while ( ( current_prohibit < prohibit->size() ) && ( current_valid_end >= current_end ) && ( current_valid_end <= (*prohibit)[ current_prohibit ] ) )
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
										SNPRINTF( buf, 254, "CONSIDERING EPISODE (time, cardinality, score): (%ld, %ld, %f)", current_valid_end, sum_ct, current_score );

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
								
								delete del_query->stmt;								
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
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_success );						

						// match score
						my_meta = make_float_constant( my_agent, king_score );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_match_score, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// cue-size
						my_meta = make_int_constant( my_agent, static_cast<long>( cue_size ) );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_cue_size, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// normalized-match-score
						my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_normalized_match_score, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						// match-cardinality
						my_meta = make_int_constant( my_agent, king_cardinality );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_match_cardinality, my_meta );						
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
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_failure );						

						////////////////////////////////////////////////////////////////////////////
						my_agent->epmem_timers->query->stop();
						////////////////////////////////////////////////////////////////////////////
					}
				}
			}
			else
			{
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_failure );				

				////////////////////////////////////////////////////////////////////////////
				my_agent->epmem_timers->query->stop();
				////////////////////////////////////////////////////////////////////////////
			}
		}
		else if ( mode == epmem_param_container::graph )
		{
			// queries
			epmem_shared_query_list *queries = new epmem_shared_query_list[2];
			std::list<epmem_shared_literal_pair_list *> trigger_lists;

			// match counters
			std::list<epmem_shared_match *> matches;

			// literals
			std::list<epmem_shared_literal *> literals;

			// pairs
			epmem_shared_literal_pair_list pairs;

			// graph match
			const long graph_match = my_agent->epmem_params->graph_match->get_value();
			epmem_shared_literal_group *graph_match_roots = NULL;
			if ( graph_match != soar_module::off )
			{
				graph_match_roots = new epmem_shared_literal_group;

				graph_match_roots->literals = new epmem_shared_literal_pair_list;
				graph_match_roots->wme_index = new epmem_shared_wme_index;
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
				tc_number tc = get_new_tc_number( my_agent );
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
				int position;

				// misc
				int i, k, m;				
				epmem_wme_list::iterator w_p;

				// associate common literals with a query
				std::map<epmem_node_id, epmem_shared_literal_pair_list *> literal_to_node_query;
				std::map<epmem_node_id, epmem_shared_literal_pair_list *> literal_to_edge_query;
				epmem_shared_literal_pair_list **query_triggers;

				// associate common WMEs with a match
				std::map<wme *, epmem_shared_match *> wme_to_match;
				epmem_shared_match **wme_match;

				// temp new things
				epmem_shared_literal *new_literal = NULL;
				epmem_shared_match *new_match = NULL;
				epmem_shared_query *new_query = NULL;
				epmem_wme_cache_element *new_cache_element = NULL;
				epmem_shared_literal_pair_list *new_trigger_list = NULL;
				epmem_shared_literal_group *new_literal_group = NULL;
				soar_module::timer *new_timer = NULL;
				soar_module::sqlite_statement *new_stmt = NULL;
				epmem_shared_literal_pair *new_literal_pair;
				epmem_shared_wme_counter **new_wme_counter;

				// identity (i.e. database id)
				epmem_node_id unique_identity;
				epmem_node_id shared_identity;

				// temporal hashing
				epmem_hash_id my_hash;	// attribute
				epmem_hash_id my_hash2;	// value

				// fully populate wme cache (we need to know parent info a priori)
				{
					epmem_wme_list *starter_wmes = NULL;

					// query
					new_cache_element = new epmem_wme_cache_element;					
					new_cache_element->wmes = wmes_query;
					new_cache_element->parents = 0;
					new_cache_element->lits = new epmem_literal_mapping;
					wme_cache[ query ] = new_cache_element;
					new_cache_element = NULL;

					// negative query
					new_cache_element = new epmem_wme_cache_element;					
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
								break;

							case EPMEM_NODE_NEG:
								starter_wmes = wmes_neg_query;								
								break;
						}

						if ( starter_wmes )
						{
							for ( w_p=starter_wmes->begin(); w_p!=starter_wmes->end(); w_p++ )
							{
								if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
								{
									parent_syms.push( (*w_p)->value );
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
									new_cache_element->wmes = epmem_get_augs_of_id( parent_sym, tc );									
									new_cache_element->lits = new epmem_literal_mapping;
									new_cache_element->parents = 1;
									wme_cache[ parent_sym ] = new_cache_element;

									if ( new_cache_element->wmes->size() )
									{
										for ( w_p=new_cache_element->wmes->begin(); w_p!=new_cache_element->wmes->end(); w_p++ )										
										{
											if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
											{
												parent_syms.push( (*w_p)->value );
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

						if ( current_cache_element->wmes )
						{
							for ( w_p=current_cache_element->wmes->begin(); w_p!=current_cache_element->wmes->end(); w_p++ )							
							{
								// add to cue list
								state->id.epmem_info->cue_wmes->insert( (*w_p) );

								if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
								{
									my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );

									// q0=? AND w=?
									my_agent->epmem_stmts_graph->find_edge_unique->bind_int( 1, parent_id );
									my_agent->epmem_stmts_graph->find_edge_unique->bind_int( 2, my_hash );								

									while ( my_agent->epmem_stmts_graph->find_edge_unique->execute() == soar_module::row )
									{
										// get identity
										unique_identity = my_agent->epmem_stmts_graph->find_edge_unique->column_int( 0 );
										shared_identity = my_agent->epmem_stmts_graph->find_edge_unique->column_int( 1 );

										// have we seen this identifier before?
										shared_cue_id = false;
										cache_hit =& wme_cache[ (*w_p)->value ];
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
											new_literal->max = (*cache_hit)->parents;
											new_literal->wme_ct = new epmem_shared_wme_book;

											new_literal->shared_id = shared_identity;

											new_literal->wme = (*w_p);
											new_literal->wme_kids = ( (*cache_hit)->wmes->size() != 0 );

											new_literal->children = NULL;
											new_literal->match = NULL;

											literals.push_back( new_literal );
											(*(*cache_hit)->lits)[ shared_identity ] = new_literal;
										}
										cache_hit = NULL;

										new_literal_pair = new epmem_shared_literal_pair;
										new_literal_pair->lit = new_literal;
										new_literal_pair->unique_id = unique_identity;
										new_literal_pair->q0 = parent_id;
										new_literal_pair->q1 = shared_identity;
										new_literal_pair->wme = (*w_p);
										pairs.push_back( new_literal_pair );

										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											// root is always on and satisfies one parental branch
											new_wme_counter =& (*new_literal->wme_ct)[ (*w_p) ];											
											
											(*new_wme_counter) = new epmem_shared_wme_counter;											
											(*new_wme_counter)->wme_ct = 0;
											(*new_wme_counter)->lit_ct = new epmem_shared_literal_counter;
											(*(*new_wme_counter)->lit_ct)[ EPMEM_NODEID_ROOT ] = 1;

											// keep track of root literals for graph-match
											if ( ( !shared_cue_id ) && ( i == EPMEM_NODE_POS ) && ( graph_match != soar_module::off ) )
											{
												// enforce wme grouping
												if ( new_literal->wme != graph_match_roots->c_wme )
												{
													graph_match_roots->c_wme = new_literal->wme;
													graph_match_roots->wme_index->push_back( graph_match_roots->literals->size() );
												}											

												graph_match_roots->literals->push_back( new_literal_pair );												
											}
										}
										else
										{
											// if this is parent's first child we can use some good initial values
											if ( !parent_literal->children )
											{
												new_literal_group = new epmem_shared_literal_group;
												new_literal_group->literals = new epmem_shared_literal_pair_list;
												new_literal_group->wme_index = new epmem_shared_wme_index;

												new_literal_group->c_wme = new_literal->wme;
												new_literal_group->wme_index->push_back( 0 );																							
												new_literal_group->literals->push_back( new_literal_pair );

												parent_literal->children = new_literal_group;
											}
											else
											{
												// otherwise, enforce wme grouping

												new_literal_group = parent_literal->children;

												if ( new_literal->wme != new_literal_group->c_wme )
												{
													new_literal_group->c_wme = new_literal->wme;
													new_literal_group->wme_index->push_back( new_literal_group->literals->size() );
												}

												new_literal_group->literals->push_back( new_literal_pair );
											}
										}

										// create queries if necessary
										query_triggers =& literal_to_edge_query[ unique_identity ];
										if ( !(*query_triggers) )
										{
											new_trigger_list = new epmem_shared_literal_pair_list;
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
													new_stmt = new soar_module::sqlite_statement( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_EDGE ][ k ][ m ], new_timer );
													new_stmt->prepare();

													// bind values
													position = 1;

													if ( ( m == EPMEM_RANGE_NOW ) && ( k == EPMEM_RANGE_END ) )
														new_stmt->bind_int( position++, time_now );
													new_stmt->bind_int( position, unique_identity );													

													// take first step
													if ( new_stmt->execute() == soar_module::row )
													{
														new_query = new epmem_shared_query;
														new_query->val = new_stmt->column_int( 0 );
														new_query->stmt = new_stmt;

														new_query->unique_id = unique_identity;

														new_query->triggers = new_trigger_list;

														// add to query list
														queries[ k ].push( new_query );
														new_query = NULL;
													}
													else
													{
														delete new_stmt;
													}

													new_stmt = NULL;
													new_timer = NULL;
												}
											}

											(*query_triggers) = new_trigger_list;
											new_trigger_list = NULL;
										}
										(*query_triggers)->push_back( new_literal_pair );

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
									my_agent->epmem_stmts_graph->find_edge_unique->reinitialize();
								}
								else
								{
									my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
									my_hash2 = epmem_temporal_hash( my_agent, (*w_p)->value );

									// parent_id=? AND attr=? AND value=?
									my_agent->epmem_stmts_graph->find_node_unique->bind_int( 1, parent_id );
									my_agent->epmem_stmts_graph->find_node_unique->bind_int( 2, my_hash );
									my_agent->epmem_stmts_graph->find_node_unique->bind_int( 3, my_hash2 );									

									if ( my_agent->epmem_stmts_graph->find_node_unique->execute() == soar_module::row )
									{
										// get identity
										unique_identity = my_agent->epmem_stmts_graph->find_node_unique->column_int( 0 );

										// create new literal
										new_literal = new epmem_shared_literal;

										new_literal->ct = 0;
										new_literal->max = EPMEM_DNF;
										new_literal->wme_ct = NULL;

										new_literal->shared_id = EPMEM_NODEID_ROOT;
										new_literal->wme_kids = 0;
										new_literal->wme = (*w_p);
										new_literal->children = NULL;

										literals.push_back( new_literal );

										new_literal_pair = new epmem_shared_literal_pair;
										new_literal_pair->lit = new_literal;
										new_literal_pair->unique_id = unique_identity;
										new_literal_pair->q0 = parent_id;
										new_literal_pair->q1 = NULL;
										new_literal_pair->wme = (*w_p);
										pairs.push_back( new_literal_pair );

										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											new_literal->ct++;

											if ( ( i == EPMEM_NODE_POS ) && ( graph_match != soar_module::off ) )
											{
												// only one literal/root non-identifier
												graph_match_roots->c_wme = new_literal->wme;
												graph_match_roots->wme_index->push_back( graph_match_roots->literals->size() );
												
												graph_match_roots->literals->push_back( new_literal_pair );
											}
										}
										else
										{
											// if this is parent's first child we can use some good initial values
											if ( !parent_literal->children )
											{
												new_literal_group = new epmem_shared_literal_group;
												new_literal_group->literals = new epmem_shared_literal_pair_list;												
												new_literal_group->wme_index = new epmem_shared_wme_index;

												new_literal_group->c_wme = new_literal->wme;
												new_literal_group->wme_index->push_back( 0 );
												new_literal_group->literals->push_back( new_literal_pair );

												parent_literal->children = new_literal_group;												
											}
											else
											{
												// else enforce wme grouping

												new_literal_group = parent_literal->children;

												if ( new_literal->wme != new_literal_group->c_wme )
												{
													new_literal_group->c_wme = new_literal->wme;
													new_literal_group->wme_index->push_back( new_literal_group->literals->size() );
												}

												new_literal_group->literals->push_back( new_literal_pair );
											}
										}										

										// create match if necessary
										wme_match =& wme_to_match[ (*w_p) ];
										if ( !(*wme_match) )
										{
											leaf_ids[i]++;

											new_match = new epmem_shared_match;
											matches.push_back( new_match );
											new_match->ct = 0;
											new_match->value_ct = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) );
											new_match->value_weight = ( ( i == EPMEM_NODE_POS )?( 1 ):( -1 ) ) * wma_get_wme_activation( my_agent, (*w_p) );

											(*wme_match) = new_match;
											new_match = NULL;
										}
										new_literal->match = (*wme_match);

										// create queries if necessary
										query_triggers =& literal_to_node_query[ unique_identity ];
										if ( !(*query_triggers) )
										{
											new_trigger_list = new epmem_shared_literal_pair_list;
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
													new_stmt = new soar_module::sqlite_statement( my_agent->epmem_db, epmem_range_queries[ EPMEM_RIT_STATE_NODE ][ k ][ m ], new_timer );
													new_stmt->prepare();

													// bind values
													position = 1;

													if ( ( m == EPMEM_RANGE_NOW ) && ( k == EPMEM_RANGE_END ) )
														new_stmt->bind_int( position++, time_now );
													new_stmt->bind_int( position, unique_identity );													

													// take first step
													if ( new_stmt->execute() == soar_module::row )
													{
														new_query = new epmem_shared_query;
														new_query->val = new_stmt->column_int( 0 );
														new_query->stmt = new_stmt;

														new_query->unique_id = unique_identity;

														new_query->triggers = new_trigger_list;

														// add to query list
														queries[ k ].push( new_query );
														new_query = NULL;
													}
													else
													{
														delete new_stmt;
													}

													new_stmt = NULL;
													new_timer = NULL;
												}
											}

											(*query_triggers) = new_trigger_list;
											new_trigger_list = NULL;
										}										
										(*query_triggers)->push_back( new_literal_pair );

										new_literal = NULL;
									}
									my_agent->epmem_stmts_graph->find_node_unique->reinitialize();
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
							delete cache_p->second->wmes;

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
			my_agent->epmem_stats->qry_lits->set_value( static_cast<long>( literals.size() ) );

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
				unsigned int next_list = NIL;

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

						while ( ( current_prohibit < prohibit->size() ) && ( current_valid_end >= current_end ) && ( current_valid_end <= (*prohibit)[ current_prohibit ] ) )
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
								SNPRINTF( buf, 254, "CONSIDERING EPISODE (time, cardinality, score): (%ld, %ld, %f)", current_valid_end, sum_ct, current_score );

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
											 ( current_graph_match_counter == graph_match_roots->wme_index->size() ) )
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
												SNPRINTF( buf, 254, "NEW KING (perfect, graph-match): (true, %s)", ( ( king_graph_match == graph_match_roots->wme_index->size() )?("true"):("false") ) );

												print( my_agent, buf );
												xml_generate_warning( my_agent, buf );
											}

											if ( king_graph_match == graph_match_roots->wme_index->size() )
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

				// pairs
				epmem_shared_literal_pair_list::iterator pair_p;
				for ( pair_p=pairs.begin(); pair_p!=pairs.end(); pair_p++ )
				{
					delete (*pair_p);
				}

				// literals
				std::list<epmem_shared_literal *>::iterator literal_p;
				epmem_shared_wme_book::iterator wme_book_p;
				for ( literal_p=literals.begin(); literal_p!=literals.end(); literal_p++ )
				{
					if ( (*literal_p)->children )
					{
						delete (*literal_p)->children->literals;
						delete (*literal_p)->children->wme_index;
						delete (*literal_p)->children;						
					}

					if ( (*literal_p)->wme_ct )
					{
						for ( wme_book_p=(*literal_p)->wme_ct->begin(); wme_book_p!=(*literal_p)->wme_ct->end(); wme_book_p++ )
						{
							delete wme_book_p->second->lit_ct;
							delete wme_book_p->second;
						}
						
						delete (*literal_p)->wme_ct;
					}

					delete (*literal_p);
				}

				// matches
				std::list<epmem_shared_match *>::iterator match_p;
				for ( match_p=matches.begin(); match_p!=matches.end(); match_p++ )
				{
					delete (*match_p);
				}

				// trigger lists
				std::list<epmem_shared_literal_pair_list *>::iterator trigger_list_p;
				for ( trigger_list_p=trigger_lists.begin(); trigger_list_p!=trigger_lists.end(); trigger_list_p++ )
				{
					delete (*trigger_list_p);
				}

				// queries
				epmem_shared_query *del_query;
				for ( i=EPMEM_NODE_POS; i<=EPMEM_NODE_NEG; i++ )
				{
					while ( !queries[ i ].empty() )
					{
						del_query = queries[ i ].top();
						queries[ i ].pop();

						delete del_query->stmt;						
						delete del_query;
					}
				}
				delete [] queries;

				// graph match
				if ( graph_match != soar_module::off )
				{
					delete graph_match_roots->literals;
					delete graph_match_roots->wme_index;
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
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_success );				

				// match score
				my_meta = make_float_constant( my_agent, king_score );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_match_score, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// cue-size
				my_meta = make_int_constant( my_agent, cue_size );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_cue_size, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// normalized-match-score
				my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_normalized_match_score, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// match-cardinality
				my_meta = make_int_constant( my_agent, king_cardinality );
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_match_cardinality, my_meta );				
				symbol_remove_ref( my_agent, my_meta );

				// graph match
				if ( graph_match != soar_module::off )
				{
					// graph-match 0/1
					my_meta = make_int_constant( my_agent, ( ( king_graph_match == perfect_match )?(1):(0) ) );
					epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_graph_match, my_meta );					
					symbol_remove_ref( my_agent, my_meta );

					// full mapping if appropriate
					if ( ( graph_match == soar_module::on ) && ( king_graph_match == perfect_match ) )
					{
						Symbol *my_meta2;
						Symbol *my_meta3;

						my_meta = make_new_identifier( my_agent, 'M', state->id.epmem_result_header->id.level );
						epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_graph_match_mapping, my_meta );						
						symbol_remove_ref( my_agent, my_meta );

						my_mapping = new epmem_id_mapping;
						for ( epmem_constraint_list::iterator c_p=king_constraints.begin(); c_p!=king_constraints.end(); c_p++ )
						{
							// create the node
							my_meta2 = make_new_identifier( my_agent, 'N', my_meta->id.level );
							epmem_add_meta_wme( my_agent, state, my_meta, my_agent->epmem_sym_graph_match_mapping_node, my_meta2 );							
							symbol_remove_ref( my_agent, my_meta2 );

							// point to the cue identifier
							epmem_add_meta_wme( my_agent, state, my_meta2, my_agent->epmem_sym_graph_match_mapping_cue, c_p->first );							

							// create and store away the [yet-to-be-retrieved] identifier
							my_meta3 = make_new_identifier( my_agent, c_p->first->id.name_letter, my_meta2->id.level );
							epmem_add_meta_wme( my_agent, state, my_meta2, my_agent->epmem_sym_retrieved, my_meta3 );							
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
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_failure );				

				////////////////////////////////////////////////////////////////////////////
				my_agent->epmem_timers->query->stop();
				////////////////////////////////////////////////////////////////////////////
			}
		}
	}
	else
	{
		epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_bad_cmd );		

		if ( wmes_query )
			delete wmes_query;

		if ( wmes_neg_query )
			delete wmes_neg_query;

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

			// examine all commands on the output-link for any
			// that appeared since last memory was recorded
			for ( s = ol->id.slots; s != NIL; s = s->next )
			{
				for ( w = s->wmes; w != NIL; w = w->next )
				{
					if ( w->timetag > my_agent->bottom_goal->id.epmem_info->last_ol_time )
					{
						new_memory = true;
						my_agent->bottom_goal->id.epmem_info->last_ol_time = w->timetag;
					}
				}
			}
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
	if ( my_agent->epmem_db->get_status() == soar_module::disconnected )
		epmem_init_db( my_agent );

	// respond to query only if db is properly initialized
	if ( my_agent->epmem_db->get_status() != soar_module::connected )
		return;

	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	epmem_wme_list *wmes;
	epmem_wme_list *cmds;
	epmem_wme_list::iterator w_p;	

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
		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->api->start();
		////////////////////////////////////////////////////////////////////////////
		
		// make sure this state has had some sort of change to the cmd
		new_cue = false;
		wme_count = 0;
		cmds = NULL;
		{			
			tc_number tc = get_new_tc_number( my_agent );
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
				wmes = epmem_get_augs_of_id( parent_sym, tc );
				{
					for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
					{
						wme_count++;

						if ( (*w_p)->timetag > state->id.epmem_info->last_cmd_time )
						{
							new_cue = true;
							state->id.epmem_info->last_cmd_time = (*w_p)->timetag;
						}

						if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
						{
							syms.push( (*w_p)->value );
						}
					}
					
					// free space from aug list
					if ( cmds == NIL )
					{
						cmds = wmes;
					}
					else
					{
						delete wmes;
					}
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
			prohibit = new epmem_time_list;
			before = EPMEM_MEMID_NONE;
			after = EPMEM_MEMID_NONE;
			good_cue = true;
			path = 0;

			// process top-level symbols
			for ( w_p=cmds->begin(); w_p!=cmds->end(); w_p++ )
			{
				state->id.epmem_info->cue_wmes->insert( (*w_p) );
				
				if ( good_cue )
				{
					// collect information about known commands
					if ( (*w_p)->attr == my_agent->epmem_sym_retrieve )
					{
						if ( ( (*w_p)->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( path == 0 ) &&
							 ( (*w_p)->value->ic.value > 0 ) )
						{							
							retrieve = (*w_p)->value->ic.value;
							path = 1;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_next )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{							
							next = true;
							path = 2;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_prev )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{
							previous = true;
							path = 2;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_query )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) &&
							 ( query == NULL ) )

						{
							query = (*w_p)->value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_negquery )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) &&
							 ( neg_query == NULL ) )

						{
							neg_query = (*w_p)->value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_before )
					{
						if ( ( (*w_p)->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							before = (*w_p)->value->ic.value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_after )
					{
						if ( ( (*w_p)->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							after = (*w_p)->value->ic.value;
							path = 3;
						}
						else
							good_cue = false;
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_prohibit )
					{
						if ( ( (*w_p)->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							prohibit->push_back( (*w_p)->value->ic.value );
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
				epmem_add_meta_wme( my_agent, state, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_bad_cmd );				
			}

			// free prohibit list
			delete prohibit;			
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////
		}

		// free space from command aug list
		if ( cmds )
			delete cmds;

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

		epmem_time_list *prohibit = new epmem_time_list;
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
