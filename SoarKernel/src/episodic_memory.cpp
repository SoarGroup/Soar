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
#include <iterator>
#include <fstream>
#include <set>
#include <climits>

#include "episodic_memory.h"
#include "semantic_memory.h"

#include "agent.h"
#include "prefmem.h"
#include "symtab.h"
#include "wmem.h"
#include "print.h"
#include "xml.h"
#include "instantiations.h"
#include "decide.h"

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

// variable abstraction			epmem::var

// relational interval tree		epmem::rit

// cleaning up					epmem::clean
// initialization				epmem::init

// temporal hash				epmem::hash

// storing new episodes			epmem::storage
// non-cue-based queries		epmem::ncb
// cue-based queries			epmem::cbr

// vizualization				epmem::viz

// high-level api				epmem::api


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (epmem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_param_container::epmem_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
	// learning
	learning = new soar_module::boolean_param( "learning", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
	add( learning );

	////////////////////
	// Encoding
	////////////////////

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

	// exclusions - this is initialized with "epmem" directly after hash tables
	exclusions = new soar_module::sym_set_param( "exclusions", new soar_module::f_predicate<const char *>, my_agent );
	add( exclusions );


	////////////////////
	// Storage
	////////////////////

	// database
	database = new soar_module::constant_param<db_choices>( "database", memory, new epmem_db_predicate<db_choices>( my_agent ) );
	database->add_mapping( memory, "memory" );
	database->add_mapping( file, "file" );
	add( database );

	// path
	path = new epmem_path_param( "path", "", new soar_module::predicate<const char *>(), new epmem_db_predicate<const char *>( my_agent ), my_agent );
	add( path );

	// auto-commit
	lazy_commit = new soar_module::boolean_param( "lazy-commit", soar_module::on, new epmem_db_predicate<soar_module::boolean>( my_agent ) );
	add( lazy_commit );


	////////////////////
	// Retrieval
	////////////////////

	// graph-match
	graph_match = new soar_module::boolean_param( "graph-match", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( graph_match );

	// balance
	balance = new soar_module::decimal_param( "balance", 1, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( balance );


	////////////////////
	// Performance
	////////////////////

	// timers
	timers = new soar_module::constant_param<soar_module::timer::timer_level>( "timers", soar_module::timer::zero, new soar_module::f_predicate<soar_module::timer::timer_level>() );
	timers->add_mapping( soar_module::timer::zero, "off" );
	timers->add_mapping( soar_module::timer::one, "one" );
	timers->add_mapping( soar_module::timer::two, "two" );
	timers->add_mapping( soar_module::timer::three, "three" );
	add( timers );

	// page_size
	page_size = new soar_module::constant_param<page_choices>( "page-size", page_8k, new epmem_db_predicate<page_choices>( my_agent ) );
	page_size->add_mapping( page_1k, "1k" );
	page_size->add_mapping( page_2k, "2k" );
	page_size->add_mapping( page_4k, "4k" );
	page_size->add_mapping( page_8k, "8k" );
	page_size->add_mapping( page_16k, "16k" );
	page_size->add_mapping( page_32k, "32k" );
	page_size->add_mapping( page_64k, "64k" );
	add( page_size );

	// cache_size
	cache_size = new soar_module::integer_param( "cache-size", 10000, new soar_module::gt_predicate<int64_t>( 1, true ), new epmem_db_predicate<int64_t>( my_agent ) );
	add( cache_size );

	// opt
	opt = new soar_module::constant_param<opt_choices>( "optimization", opt_speed, new epmem_db_predicate<opt_choices>( my_agent ) );
	opt->add_mapping( opt_safety, "safety" );
	opt->add_mapping( opt_speed, "performance" );
	add( opt );


	////////////////////
	// Experimental
	////////////////////

	gm_ordering = new soar_module::constant_param<gm_ordering_choices>( "graph-match-ordering", gm_order_undefined, new soar_module::f_predicate<gm_ordering_choices>() );
	gm_ordering->add_mapping( gm_order_undefined, "undefined" );
	gm_ordering->add_mapping( gm_order_dfs, "dfs" );
	gm_ordering->add_mapping( gm_order_mcv, "mcv" );
	add( gm_ordering );

	// merge
	merge = new soar_module::constant_param<merge_choices>( "merge", merge_none, new soar_module::f_predicate<merge_choices>() );
	merge->add_mapping( merge_none, "none" );
	merge->add_mapping( merge_add, "add" );
	add( merge );
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

	// db-lib-version
	db_lib_version = new epmem_db_lib_version_stat( my_agent, "db-lib-version", NULL, new soar_module::predicate< const char* >() );
	add( db_lib_version );

	// mem-usage
	mem_usage = new epmem_mem_usage_stat( my_agent, "mem-usage", 0, new soar_module::predicate<int64_t>() );
	add( mem_usage );

	// mem-high
	mem_high = new epmem_mem_high_stat( my_agent, "mem-high", 0, new soar_module::predicate<int64_t>() );
	add( mem_high );

	// cue-based-retrievals
	cbr = new soar_module::integer_stat( "queries", 0, new soar_module::f_predicate<int64_t>() );
	add( cbr );

	// nexts
	nexts = new soar_module::integer_stat( "nexts", 0, new soar_module::f_predicate<int64_t>() );
	add( nexts );

	// prev's
	prevs = new soar_module::integer_stat( "prevs", 0, new soar_module::f_predicate<int64_t>() );
	add( prevs );

	// ncb-wmes
	ncb_wmes = new soar_module::integer_stat( "ncb-wmes", 0, new soar_module::f_predicate<int64_t>() );
	add( ncb_wmes );

	// qry-pos
	qry_pos = new soar_module::integer_stat( "qry-pos", 0, new soar_module::f_predicate<int64_t>() );
	add( qry_pos );

	// qry-neg
	qry_neg = new soar_module::integer_stat( "qry-neg", 0, new soar_module::f_predicate<int64_t>() );
	add( qry_neg );

	// qry-ret
	qry_ret = new epmem_time_id_stat( "qry-ret", 0, new soar_module::f_predicate<epmem_time_id>() );
	add( qry_ret );

	// qry-card
	qry_card = new soar_module::integer_stat( "qry-card", 0, new soar_module::f_predicate<int64_t>() );
	add( qry_card );

	// qry-lits
	qry_lits = new soar_module::integer_stat( "qry-lits", 0, new soar_module::f_predicate<int64_t>() );
	add( qry_lits );

	// next-id
	next_id = new epmem_node_id_stat( "next-id", 0, new epmem_db_predicate<epmem_node_id>( my_agent ) );
	add( next_id );

	// rit-offset-1
	rit_offset_1 = new soar_module::integer_stat( "rit-offset-1", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_offset_1 );

	// rit-left-root-1
	rit_left_root_1 = new soar_module::integer_stat( "rit-left-root-1", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_left_root_1 );

	// rit-right-root-1
	rit_right_root_1 = new soar_module::integer_stat( "rit-right-root-1", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_right_root_1 );

	// rit-min-step-1
	rit_min_step_1 = new soar_module::integer_stat( "rit-min-step-1", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_min_step_1 );

	// rit-offset-2
	rit_offset_2 = new soar_module::integer_stat( "rit-offset-2", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_offset_2 );

	// rit-left-root-2
	rit_left_root_2 = new soar_module::integer_stat( "rit-left-root-2", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_left_root_2 );

	// rit-right-root-2
	rit_right_root_2 = new soar_module::integer_stat( "rit-right-root-2", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_right_root_2 );

	// rit-min-step-2
	rit_min_step_2 = new soar_module::integer_stat( "rit-min-step-2", 0, new epmem_db_predicate<int64_t>( my_agent ) );
	add( rit_min_step_2 );


	/////////////////////////////
	// connect to rit state
	/////////////////////////////

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

epmem_db_lib_version_stat::epmem_db_lib_version_stat( agent* new_agent, const char* new_name, const char* new_value, soar_module::predicate< const char* >* new_prot_pred ): soar_module::primitive_stat< const char* >( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

const char* epmem_db_lib_version_stat::get_value()
{
	return my_agent->epmem_db->lib_version();
}

//

epmem_mem_usage_stat::epmem_mem_usage_stat( agent *new_agent, const char *new_name, int64_t new_value, soar_module::predicate<int64_t> *new_prot_pred ): soar_module::integer_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

int64_t epmem_mem_usage_stat::get_value()
{
	return my_agent->epmem_db->memory_usage();
}

//

epmem_mem_high_stat::epmem_mem_high_stat( agent *new_agent, const char *new_name, int64_t new_value, soar_module::predicate<int64_t> *new_prot_pred ): soar_module::integer_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

int64_t epmem_mem_high_stat::get_value()
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

	wm_phase = new epmem_timer( "epmem_wm_phase", my_agent, soar_module::timer::two );
	add( wm_phase );

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

	query_walk = new epmem_timer( "query_walk", my_agent, soar_module::timer::three );
	add( query_walk );

	query_walk_edge = new epmem_timer( "query_walk_edge", my_agent, soar_module::timer::three );
	add( query_walk_edge );

	query_walk_interval = new epmem_timer( "query_walk_interval", my_agent, soar_module::timer::three );
	add( query_walk_interval );

	query_graph_match = new epmem_timer( "query_graph_match", my_agent, soar_module::timer::three );
	add( query_graph_match );

	query_result = new epmem_timer( "query_result", my_agent, soar_module::timer::three );
	add( query_result );

	query_cleanup = new epmem_timer( "query_cleanup", my_agent, soar_module::timer::three );
	add( query_cleanup );

	query_sql_edge = new epmem_timer( "query_sql_edge", my_agent, soar_module::timer::three );
	add( query_sql_edge );

	query_sql_start_ep = new epmem_timer( "query_sql_start_ep", my_agent, soar_module::timer::three );
	add( query_sql_start_ep );

	query_sql_start_now = new epmem_timer( "query_sql_start_now", my_agent, soar_module::timer::three );
	add( query_sql_start_now );

	query_sql_start_point = new epmem_timer( "query_sql_start_point", my_agent, soar_module::timer::three );
	add( query_sql_start_point );

	query_sql_end_ep = new epmem_timer( "query_sql_end_ep", my_agent, soar_module::timer::three );
	add( query_sql_end_ep );

	query_sql_end_now = new epmem_timer( "query_sql_end_now", my_agent, soar_module::timer::three );
	add( query_sql_end_now );

	query_sql_end_point = new epmem_timer( "query_sql_end_point", my_agent, soar_module::timer::three );
	add( query_sql_end_point );

	/////////////////////////////
	// connect to rit state
	/////////////////////////////

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

	// workaround for tree: type 1 = IDENTIFIER_SYMBOL_TYPE
	add_structure( "INSERT OR IGNORE INTO temporal_symbol_hash (id,sym_const,sym_type) VALUES (0,NULL,1)" );

	// workaround for acceptable preference wmes: id 1 = "operator+"
	add_structure( "INSERT OR IGNORE INTO temporal_symbol_hash (id,sym_const,sym_type) VALUES (1,'operator*',2)" );

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
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_range_id_end_start ON node_range (id,end DESC,start)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_range (rit_node INTEGER,start INTEGER,end INTEGER,id INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_range_lower ON edge_range (rit_node,start)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_range_upper ON edge_range (rit_node,end)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_start ON edge_range (id,start DESC)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_range_id_end_start ON edge_range (id,end DESC,start)" );

	add_structure( "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,attrib INTEGER, value INTEGER, last INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS node_unique_parent_attrib_last ON node_unique (parent_id,attrib,last)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS node_unique_parent_attrib_value_last ON node_unique (parent_id,attrib,value,last)" );

	add_structure( "CREATE TABLE IF NOT EXISTS edge_unique (parent_id INTEGER PRIMARY KEY AUTOINCREMENT,q0 INTEGER,w INTEGER,q1 INTEGER, last INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS edge_unique_q0_w_last ON edge_unique (q0,w,last)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS edge_unique_q0_w_q1_last ON edge_unique (q0,w,q1,last)" );

	add_structure( "CREATE TABLE IF NOT EXISTS lti (parent_id INTEGER PRIMARY KEY, letter INTEGER, num INTEGER, time_id INTEGER, current INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS lti_letter_num ON lti (letter,num)" );

	// adding an ascii table just to make lti queries easier when inspecting database
	add_structure( "CREATE TABLE IF NOT EXISTS ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)" );
	add_structure( "DELETE FROM ascii" );
	{
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (65,'A')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (66,'B')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (67,'C')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (68,'D')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (69,'E')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (70,'F')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (71,'G')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (72,'H')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (73,'I')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (74,'J')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (75,'K')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (76,'L')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (77,'M')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (78,'N')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (79,'O')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (80,'P')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (81,'Q')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (82,'R')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (83,'S')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (84,'T')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (85,'U')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (86,'V')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (87,'W')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (88,'X')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (89,'Y')" );
		add_structure( "INSERT INTO ascii (ascii_num, ascii_chr) VALUES (90,'Z')" );
	}

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


	add_node_unique = new soar_module::sqlite_statement( new_db, "INSERT INTO node_unique (parent_id,attrib,value,last) VALUES (?,?,?,?)" );
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


	add_edge_unique = new soar_module::sqlite_statement( new_db, "INSERT INTO edge_unique (q0,w,q1,last) VALUES (?,?,?,?)" );
	add( add_edge_unique );

	find_edge_unique = new soar_module::sqlite_statement( new_db, "SELECT parent_id, q1 FROM edge_unique WHERE q0=? AND w=?" );
	add( find_edge_unique );

	find_edge_unique_shared = new soar_module::sqlite_statement( new_db, "SELECT parent_id, q1 FROM edge_unique WHERE q0=? AND w=? AND q1=?" );
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

	get_edges = new soar_module::sqlite_statement( new_db, "SELECT f.q0, h.sym_const, f.q1, h.sym_type, lti.letter, lti.num FROM edge_unique f INNER JOIN temporal_symbol_hash h ON f.w=h.id LEFT JOIN lti ON (f.q1=lti.parent_id AND lti.time_id <= ?) WHERE f.parent_id IN (SELECT n.id FROM edge_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM edge_point p WHERE p.start = ? UNION ALL SELECT e1.id FROM edge_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM edge_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY f.q0 ASC, f.q1 ASC", new_agent->epmem_timers->ncb_edge );
	add( get_edges );

	//

	promote_id = new soar_module::sqlite_statement( new_db, "INSERT OR IGNORE INTO lti (parent_id,letter,num,time_id,current) VALUES (?,?,?,?,?)" );
	add( promote_id );

	find_lti = new soar_module::sqlite_statement( new_db, "SELECT parent_id FROM lti WHERE letter=? AND num=?" );
	add( find_lti );

	find_lti_promotion_time = new soar_module::sqlite_statement( new_db, "SELECT time_id FROM lti WHERE letter=? AND num=?" );
	add( find_lti_promotion_time );

	// current LTI stuff

	update_lti = new soar_module::sqlite_statement( new_db, "UPDATE OR IGNORE lti SET current=? WHERE letter=? AND num=?" );
	add( update_lti );

	find_edge_unique_lti = new soar_module::sqlite_statement( new_db, "SELECT eq.parent_id as parent_id, eq.q1 as q1 FROM edge_unique as eq JOIN lti ON eq.q1=lti.parent_id WHERE eq.q0=? AND eq.w=?" );
	add( find_edge_unique_lti );

	find_lti_current_time = new soar_module::sqlite_statement( new_db, "SELECT current FROM lti WHERE parent_id=?" );
	add( find_lti_current_time );

	//

	update_node_unique_last = new soar_module::sqlite_statement( new_db, "UPDATE node_unique SET last=? WHERE child_id=?" );
	add( update_node_unique_last );

	update_edge_unique_last = new soar_module::sqlite_statement( new_db, "UPDATE edge_unique SET last=? WHERE parent_id=?" );
	add( update_edge_unique_last );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (epmem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_augs_of_id
 * Author		: Nate Derbinsky
 * Notes		: This routine gets all wmes rooted at an id.
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
			return_val->push_back( w );
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
				return_val->push_back( w );
			}

			for ( w=s->acceptable_preference_wmes; w!=NIL; w=w->next )
			{
				return_val->push_back( w );
			}
		}
	}

	return return_val;
}

inline void _epmem_process_buffered_wme_list( agent* my_agent, Symbol* state, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& my_list, bool meta )
{
	if ( my_list.empty() )
	{
		return;
	}

	instantiation* inst = soar_module::make_fake_instantiation( my_agent, state, &cue_wmes, &my_list );

	for ( preference* pref=inst->preferences_generated; pref; pref=pref->inst_next )
	{
		// add the preference to temporary memory
		add_preference_to_tm( my_agent, pref );

		// and add it to the list of preferences to be removed
		// when the goal is removed
		insert_at_head_of_dll( state->id.preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev );
		pref->on_goal_list = true;

		if ( meta )
		{
			// if this is a meta wme, then it is completely local
			// to the state and thus we will manually remove it
			// (via preference removal) when the time comes
			state->id.epmem_info->epmem_wmes->push_back( pref );
		}
	}

	if ( !meta )
	{
		// otherwise, we submit the fake instantiation to backtracing
		// such as to potentially produce justifications that can follow
		// it to future adventures (potentially on new states)
		instantiation *my_justification_list = NIL;
		chunk_instantiation( my_agent, inst, false, &my_justification_list );

		// if any justifications are created, assert their preferences manually
		// (copied mainly from assert_new_preferences with respect to our circumstances)
		if ( my_justification_list != NIL )
		{
			preference *just_pref = NIL;
			instantiation *next_justification = NIL;

			for ( instantiation *my_justification=my_justification_list;
				  my_justification!=NIL;
				  my_justification=next_justification )
			{
				next_justification = my_justification->next;

				if ( my_justification->in_ms )
				{
					insert_at_head_of_dll( my_justification->prod->instantiations, my_justification, next, prev );
				}

				for ( just_pref=my_justification->preferences_generated; just_pref!=NIL; just_pref=just_pref->inst_next )
				{
					add_preference_to_tm( my_agent, just_pref );

					if ( wma_enabled( my_agent ) )
					{
						wma_activate_wmes_in_pref( my_agent, just_pref );
					}
				}
			}
		}
	}
}

inline void epmem_process_buffered_wmes( agent* my_agent, Symbol* state, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes )
{
	_epmem_process_buffered_wme_list( my_agent, state, cue_wmes, meta_wmes, true );
	_epmem_process_buffered_wme_list( my_agent, state, cue_wmes, retrieval_wmes, false );
}

inline void epmem_buffer_add_wme( soar_module::symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value )
{
	my_list.push_back( new soar_module::symbol_triple( id, attr, value ) );

	symbol_add_ref( id );
	symbol_add_ref( attr );
	symbol_add_ref( value );
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
bool epmem_get_variable( agent *my_agent, epmem_variable_key variable_id, int64_t *variable_value )
{
	soar_module::exec_result status;
	soar_module::sqlite_statement *var_get = my_agent->epmem_stmts_common->var_get;

	var_get->bind_int( 1, variable_id );
	status = var_get->execute();

	if ( status == soar_module::row )
	{
		(*variable_value) = var_get->column_int( 0 );
	}

	var_get->reinitialize();

	return ( status == soar_module::row );
}

/***************************************************************************
 * Function     : epmem_set_variable
 * Author		: Nate Derbinsky
 * Notes		: Sets an EpMem variable in the database
 **************************************************************************/
void epmem_set_variable( agent *my_agent, epmem_variable_key variable_id, int64_t variable_value )
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
int64_t epmem_rit_fork_node( int64_t lower, int64_t upper, bool /*bounds_offset*/, int64_t *step_return, epmem_rit_state *rit_state )
{
	// never called
	/*if ( !bounds_offset )
	{
		int64_t offset = rit_state->offset.stat->get_value();

		lower = ( lower - offset );
		upper = ( upper - offset );
	}*/

	// descend the tree down to the fork node
	int64_t node = EPMEM_RIT_ROOT;
	if ( upper < EPMEM_RIT_ROOT )
	{
		node = rit_state->leftroot.stat->get_value();
	}
	else if ( lower > EPMEM_RIT_ROOT )
	{
		node = rit_state->rightroot.stat->get_value();
	}

	int64_t step;
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
void epmem_rit_prep_left_right( agent *my_agent, int64_t lower, int64_t upper, epmem_rit_state *rit_state )
{
	////////////////////////////////////////////////////////////////////////////
	rit_state->timer->start();
	////////////////////////////////////////////////////////////////////////////

	int64_t offset = rit_state->offset.stat->get_value();
	int64_t node, step;
	int64_t left_node, left_step;
	int64_t right_node, right_step;

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
void epmem_rit_insert_interval( agent *my_agent, int64_t lower, int64_t upper, epmem_node_id id, epmem_rit_state *rit_state )
{
	// initialize offset
	int64_t offset = rit_state->offset.stat->get_value();
	if ( offset == EPMEM_RIT_OFFSET_INIT )
	{
		offset = lower;

		// update database
		epmem_set_variable( my_agent, rit_state->offset.var_key, offset );

		// update stat
		rit_state->offset.stat->set_value( offset );
	}

	// get node
	int64_t node;
	{
		int64_t left_root = rit_state->leftroot.stat->get_value();
		int64_t right_root = rit_state->rightroot.stat->get_value();
		int64_t min_step = rit_state->minstep.stat->get_value();

		// shift interval
		int64_t l = ( lower - offset );
		int64_t u = ( upper - offset );

		// update left_root
		if ( ( u < EPMEM_RIT_ROOT ) && ( l <= ( 2 * left_root ) ) )
		{
			left_root = static_cast<int64_t>( pow( -2.0, floor( log( static_cast<double>( -l ) ) / EPMEM_LN_2 ) ) );

			// update database
			epmem_set_variable( my_agent, rit_state->leftroot.var_key, left_root );

			// update stat
			rit_state->leftroot.stat->set_value( left_root );
		}

		// update right_root
		if ( ( l > EPMEM_RIT_ROOT ) && ( u >= ( 2 * right_root ) ) )
		{
			right_root = static_cast<int64_t>( pow( 2.0, floor( log( static_cast<double>( u ) ) / EPMEM_LN_2 ) ) );

			// update database
			epmem_set_variable( my_agent, rit_state->rightroot.var_key, right_root );

			// update stat
			rit_state->rightroot.stat->set_value( right_root );
		}

		// update min_step
		int64_t step;
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
		// if lazy, commit
		if ( my_agent->epmem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->epmem_stmts_common->commit->execute( soar_module::op_reinit );
		}

		// de-allocate statements
		delete my_agent->epmem_stmts_common;
		delete my_agent->epmem_stmts_graph;

		// de-allocate local data structures
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
			my_agent->epmem_id_ref_counts->clear();
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
	preference *pref;

	while ( !state->id.epmem_info->epmem_wmes->empty() )
	{
		pref = state->id.epmem_info->epmem_wmes->back();
		state->id.epmem_info->epmem_wmes->pop_back();

		if ( pref->in_tm )
		{
			remove_preference_from_tm( my_agent, pref );
		}
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
	{
		state = my_agent->top_goal;
	}

	while( state )
	{
		epmem_data *data = state->id.epmem_info;

		data->last_ol_time = 0;

		data->last_cmd_time = 0;
		data->last_cmd_count = 0;

		data->last_memory = EPMEM_MEMID_NONE;

		// this will be called after prefs from goal are already removed,
		// so just clear out result stack
		data->epmem_wmes->clear();

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
	{
		return;
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->init->start();
	////////////////////////////////////////////////////////////////////////////

	const char *db_path;
	if ( my_agent->epmem_params->database->get_value() == epmem_param_container::memory )
	{
		db_path = ":memory:";
	}
	else
	{
		db_path = my_agent->epmem_params->path->get_value();
	}

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

		// apply performance options
		{
			// page_size
			{
				switch ( my_agent->epmem_params->page_size->get_value() )
				{
					case ( epmem_param_container::page_1k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 1024" );
						break;

					case ( epmem_param_container::page_2k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 2048" );
						break;

					case ( epmem_param_container::page_4k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 4096" );
						break;

					case ( epmem_param_container::page_8k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 8192" );
						break;

					case ( epmem_param_container::page_16k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 16384" );
						break;

					case ( epmem_param_container::page_32k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 32768" );
						break;

					case ( epmem_param_container::page_64k ):
						temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA page_size = 65536" );
						break;
				}

				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}

			// cache_size
			{
				std::string cache_sql( "PRAGMA cache_size = " );
				char* str = my_agent->epmem_params->cache_size->get_string();
				cache_sql.append( str );
				free(str);
				str = NULL;

				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, cache_sql.c_str() );

				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}

			// optimization
			if ( my_agent->epmem_params->opt->get_value() == epmem_param_container::opt_speed )
			{
				// synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA synchronous = OFF" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;

				// journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA journal_mode = OFF" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;

				// locking_mode - no one else can view the database after our first write
				temp_q = new soar_module::sqlite_statement( my_agent->epmem_db, "PRAGMA locking_mode = EXCLUSIVE" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}
		}

		// point stuff
		epmem_time_id range_start;
		epmem_time_id time_last;

		// update validation count
		my_agent->epmem_validation++;

		// setup common structures/queries
		my_agent->epmem_stmts_common = new epmem_common_statement_container( my_agent );
		my_agent->epmem_stmts_common->structure();
		my_agent->epmem_stmts_common->prepare();

		{
			// setup graph structures/queries
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
				int64_t stored_id = NIL;
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
				my_agent->epmem_rit_state_graph[ i ].minstep.stat->set_value( LLONG_MAX );
			}
			my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].add_query = my_agent->epmem_stmts_graph->add_node_range;
			my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].add_query = my_agent->epmem_stmts_graph->add_edge_range;

			////

			// get/set RIT variables
			{
				int64_t var_val = NIL;

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
						{
							epmem_rit_insert_interval( my_agent, range_start, time_last, temp_q2->column_int( 0 ), &( my_agent->epmem_rit_state_graph[i] ) );
						}

						switch (i)
						{
							case EPMEM_RIT_STATE_NODE:
								my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 1, time_last );
								my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 2, temp_q2->column_int( 0 ) );
								my_agent->epmem_stmts_graph->update_node_unique_last->execute( soar_module::op_reinit );
								break;
							case EPMEM_RIT_STATE_EDGE:
								my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 1, time_last );
								my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 2, temp_q2->column_int( 0 ) );
								my_agent->epmem_stmts_graph->update_edge_unique_last->execute( soar_module::op_reinit );
								break;
						}
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
				int64_t w;
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

					(*ip)->push_front( std::make_pair( q1, parent_id ) );

					hp =& (*my_agent->epmem_id_repository)[ q1 ];
					if ( !(*hp) )
						(*hp) = new epmem_hashed_id_pool;
				}

				delete temp_q;
				temp_q = NULL;
			}
		}

		// if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
		if ( my_agent->epmem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->epmem_stmts_common->begin->execute( soar_module::op_reinit );
		}
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
	{
		epmem_init_db( my_agent );
	}

	// add the episode only if db is properly initialized
	if ( my_agent->epmem_db->get_status() != soar_module::connected )
	{
		return;
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->storage->start();
	////////////////////////////////////////////////////////////////////////////

	epmem_time_id time_counter = my_agent->epmem_stats->time->get_value();

	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];

		SNPRINTF( buf, 254, "NEW EPISODE: %ld", static_cast<long int>(time_counter) );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}

	// perform storage
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
		std::set<Symbol *> new_identifiers;

		// children of the current identifier
		epmem_wme_list *wmes;
		epmem_wme_list::iterator w_p;

		// initialize BFS
		my_agent->top_goal->id.epmem_id = EPMEM_NODEID_ROOT;
		my_agent->top_goal->id.epmem_valid = my_agent->epmem_validation;
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
						 ( ( (*w_p)->epmem_id == EPMEM_NODEID_BAD ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) ) &&
						 ( ( (*w_p)->value->id.epmem_id != EPMEM_NODEID_BAD ) && ( (*w_p)->value->id.epmem_valid == my_agent->epmem_validation ) ) &&
						 ( !(*w_p)->value->id.smem_lti ) )
					{
						// prevent exclusions from being recorded
						if ( my_agent->epmem_params->exclusions->in_set( (*w_p)->attr ) )
						{
							continue;
						}

						// if still here, create reservation (case 1)
						new_id_reservation = new epmem_id_reservation;
						new_id_reservation->my_id = EPMEM_NODEID_BAD;
						new_id_reservation->my_pool = NULL;

						if ( (*w_p)->acceptable )
						{
							new_id_reservation->my_hash = EPMEM_HASH_ACCEPTABLE;
						}
						else
						{
							new_id_reservation->my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
						}

						// try to find appropriate reservation
						my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ new_id_reservation->my_hash ];
						if ( (*my_id_repo) )
						{
							if ( !(*my_id_repo)->empty() )
							{
								for ( pool_p = (*my_id_repo)->begin(); pool_p != (*my_id_repo)->end(); pool_p++ )
								{	
									if ( pool_p->first == (*w_p)->value->id.epmem_id )
									{	
										new_id_reservation->my_id = pool_p->second;	 

										(*my_id_repo)->erase( pool_p );
									}	
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
					{
						continue;
					}

					if ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						// have we seen this WME during this database?
						if ( ( (*w_p)->epmem_id == EPMEM_NODEID_BAD ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) )
						{
							(*w_p)->epmem_valid = my_agent->epmem_validation;
							(*w_p)->epmem_id = EPMEM_NODEID_BAD;

							my_hash = NIL;
							my_id_repo2 = NIL;

							// if long-term identifier as value, special processing (we may need to promote, we don't add to/take from any pools)
							if ( (*w_p)->value->id.smem_lti )
							{
								// find the lti or add new one
								if ( ( (*w_p)->value->id.epmem_id == EPMEM_NODEID_BAD ) || ( (*w_p)->value->id.epmem_valid != my_agent->epmem_validation ) )
								{
									(*w_p)->value->id.epmem_id = EPMEM_NODEID_BAD;
									(*w_p)->value->id.epmem_valid = my_agent->epmem_validation;

									// try to find
									{
										my_agent->epmem_stmts_graph->find_lti->bind_int( 1, static_cast<uint64_t>( (*w_p)->value->id.name_letter ) );
										my_agent->epmem_stmts_graph->find_lti->bind_int( 2, static_cast<uint64_t>( (*w_p)->value->id.name_number ) );

										if ( my_agent->epmem_stmts_graph->find_lti->execute() == soar_module::row )
										{
											(*w_p)->value->id.epmem_id = static_cast<epmem_node_id>( my_agent->epmem_stmts_graph->find_lti->column_int( 0 ) );
										}

										my_agent->epmem_stmts_graph->find_lti->reinitialize();
									}

									// add if necessary
									if ( (*w_p)->value->id.epmem_id == EPMEM_NODEID_BAD )
									{
										(*w_p)->value->id.epmem_id = my_agent->epmem_stats->next_id->get_value();
										my_agent->epmem_stats->next_id->set_value( (*w_p)->value->id.epmem_id + 1 );
										epmem_set_variable( my_agent, var_next_id, (*w_p)->value->id.epmem_id + 1 );

										// add repository
										(*my_agent->epmem_id_repository)[ (*w_p)->value->id.epmem_id ] = new epmem_hashed_id_pool;

										// parent_id,letter,num,time_id
										my_agent->epmem_stmts_graph->promote_id->bind_int( 1, (*w_p)->value->id.epmem_id );
										my_agent->epmem_stmts_graph->promote_id->bind_int( 2, static_cast<uint64_t>( (*w_p)->value->id.name_letter ) );
										my_agent->epmem_stmts_graph->promote_id->bind_int( 3, static_cast<uint64_t>( (*w_p)->value->id.name_number ) );
										my_agent->epmem_stmts_graph->promote_id->bind_int( 4, time_counter );
										my_agent->epmem_stmts_graph->promote_id->bind_int( 5, time_counter );
										my_agent->epmem_stmts_graph->promote_id->execute( soar_module::op_reinit );
									}
								}

								// now perform deliberate edge search
								// ltis don't use the pools, so we make a direct search in the edge_unique table
								// if failure, drop below and use standard channels
								{
									// get temporal hash
									if ( (*w_p)->acceptable )
									{
										my_hash = EPMEM_HASH_ACCEPTABLE;
									}
									else
									{
										my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
									}

									// q0, w, q1
									my_agent->epmem_stmts_graph->find_edge_unique_shared->bind_int( 1, parent_id );
									my_agent->epmem_stmts_graph->find_edge_unique_shared->bind_int( 2, my_hash );
									my_agent->epmem_stmts_graph->find_edge_unique_shared->bind_int( 3, (*w_p)->value->id.epmem_id );

									if ( my_agent->epmem_stmts_graph->find_edge_unique_shared->execute() == soar_module::row )
									{
										(*w_p)->epmem_id = my_agent->epmem_stmts_graph->find_edge_unique_shared->column_int( 0 );
									}

									my_agent->epmem_stmts_graph->find_edge_unique_shared->reinitialize();
								}
							}
							else
							{
								// in the case of a known child, we already have a reservation (case 1)
								if ( ( (*w_p)->value->id.epmem_id != EPMEM_NODEID_BAD ) && ( (*w_p)->value->id.epmem_valid == my_agent->epmem_validation ) )
								{
									r_p = id_reservations.find( (*w_p) );

									if ( r_p != id_reservations.end() )
									{
										// restore reservation info
										my_hash = r_p->second->my_hash;
										my_id_repo2 = r_p->second->my_pool;

										if ( r_p->second->my_id != EPMEM_NODEID_BAD )
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
										if ( (*w_p)->acceptable )
										{
											my_hash = EPMEM_HASH_ACCEPTABLE;
										}
										else
										{
											my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
										}

										// try to get an id that matches new information
										my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ my_hash ];
										if ( (*my_id_repo) )
										{
											if ( !(*my_id_repo)->empty() )
											{
												for ( pool_p = (*my_id_repo)->begin(); pool_p != (*my_id_repo)->end(); pool_p++ )
												{
													if ( pool_p->first == (*w_p)->value->id.epmem_id )
													{
														new_id_reservation->my_id = pool_p->second;
														(*my_id_repo)->erase( pool_p );
														(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = (*my_id_repo);
													}
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
									// UNKNOWN identifier
									new_identifiers.insert( (*w_p)->value );

									// get temporal hash
									if ( (*w_p)->acceptable )
									{
										my_hash = EPMEM_HASH_ACCEPTABLE;
									}
									else
									{
										my_hash = epmem_temporal_hash( my_agent, (*w_p)->attr );
									}

									// try to find node
									my_id_repo =& (*(*my_agent->epmem_id_repository)[ parent_id ])[ my_hash ];
									if ( (*my_id_repo) )
									{
										// if something leftover, try to use it
										if ( !(*my_id_repo)->empty() )
										{
											pool_p = (*my_id_repo)->begin();

											do
											{
												if ( (*my_agent->epmem_id_ref_counts)[ pool_p->first ] == 0 )
												{
													(*w_p)->epmem_id = pool_p->second;
													(*w_p)->value->id.epmem_id = pool_p->first;
													(*w_p)->value->id.epmem_valid = my_agent->epmem_validation;
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
							}

							// add path if no success above
							if ( (*w_p)->epmem_id == EPMEM_NODEID_BAD )
							{
								if ( ( (*w_p)->value->id.epmem_id == EPMEM_NODEID_BAD ) || ( (*w_p)->value->id.epmem_valid != my_agent->epmem_validation ) )
								{
									// update next id
									(*w_p)->value->id.epmem_id = my_agent->epmem_stats->next_id->get_value();
									(*w_p)->value->id.epmem_valid = my_agent->epmem_validation;
									my_agent->epmem_stats->next_id->set_value( (*w_p)->value->id.epmem_id + 1 );
									epmem_set_variable( my_agent, var_next_id, (*w_p)->value->id.epmem_id + 1 );

									// add repository
									(*my_agent->epmem_id_repository)[ (*w_p)->value->id.epmem_id ] = new epmem_hashed_id_pool;
								}

								// insert (q0,w,q1)
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 1, parent_id );
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 2, my_hash );
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 3, (*w_p)->value->id.epmem_id );
								my_agent->epmem_stmts_graph->add_edge_unique->bind_int( 4, LLONG_MAX );
								my_agent->epmem_stmts_graph->add_edge_unique->execute( soar_module::op_reinit );

								(*w_p)->epmem_id = static_cast<epmem_node_id>( my_agent->epmem_db->last_insert_rowid() );

								if ( !(*w_p)->value->id.smem_lti )
								{
									(*my_agent->epmem_id_replacement)[ (*w_p)->epmem_id ] = my_id_repo2;
								}

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

							// at this point we have successfully added a new wme
							// whose value was an identifier.  IF the identifier was
							// unknown at the beginning of this episode, then we need
							// to update its ref count for each WME added.
							if ( new_identifiers.find( (*w_p)->value ) != new_identifiers.end() )
							{
								(*my_agent->epmem_id_ref_counts)[ (*w_p)->value->id.epmem_id ]++;
							}
						}
						else
						{
							if ( (*w_p)->value->id.smem_lti )
							{
								if ( ( (*w_p)->value->id.smem_time_id == time_counter ) && ( (*w_p)->value->id.smem_valid == my_agent->epmem_validation ) )
								{
									// parent_id,letter,num,time_id
									my_agent->epmem_stmts_graph->promote_id->bind_int( 1, (*w_p)->value->id.epmem_id );
									my_agent->epmem_stmts_graph->promote_id->bind_int( 2, static_cast<uint64_t>( (*w_p)->value->id.name_letter ) );
									my_agent->epmem_stmts_graph->promote_id->bind_int( 3, static_cast<uint64_t>( (*w_p)->value->id.name_number ) );
									my_agent->epmem_stmts_graph->promote_id->bind_int( 4, time_counter );
									my_agent->epmem_stmts_graph->promote_id->bind_int( 5, time_counter );
									my_agent->epmem_stmts_graph->promote_id->execute( soar_module::op_reinit );
								}
							}
						}

						// continue to children?
						if ( (*w_p)->value->id.tc_num != tc )
						{
							// future exploration
							parent_syms.push( (*w_p)->value );
							parent_ids.push( (*w_p)->value->id.epmem_id );
						}
					}
					else
					{
						// have we seen this node in this database?
						if ( ( (*w_p)->epmem_id == EPMEM_NODEID_BAD ) || ( (*w_p)->epmem_valid != my_agent->epmem_validation ) )
						{
							(*w_p)->epmem_id = EPMEM_NODEID_BAD;
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
								{
									(*w_p)->epmem_id = my_agent->epmem_stmts_graph->find_node_unique->column_int( 0 );
								}

								my_agent->epmem_stmts_graph->find_node_unique->reinitialize();
							}

							// act depending on new/existing feature
							if ( (*w_p)->epmem_id == EPMEM_NODEID_BAD )
							{
								// insert (parent_id,attr,value)
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 1, parent_id );
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 2, my_hash );
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 3, my_hash2 );
								my_agent->epmem_stmts_graph->add_node_unique->bind_int( 4, LLONG_MAX );
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

				my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 1, LLONG_MAX );
				my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 2, *temp_node );
				my_agent->epmem_stmts_graph->update_node_unique_last->execute( soar_module::op_reinit );

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

				my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 1, LLONG_MAX );
				my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 2, *temp_node );
				my_agent->epmem_stmts_graph->update_edge_unique_last->execute( soar_module::op_reinit );

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

					my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 1, range_end );
					my_agent->epmem_stmts_graph->update_node_unique_last->bind_int( 2, r->first );
					my_agent->epmem_stmts_graph->update_node_unique_last->execute( soar_module::op_reinit );
					// point (id, start)
					if ( range_start == range_end )
					{
						my_agent->epmem_stmts_graph->add_node_point->bind_int( 1, r->first );
						my_agent->epmem_stmts_graph->add_node_point->bind_int( 2, range_start );
						my_agent->epmem_stmts_graph->add_node_point->execute( soar_module::op_reinit );
					}
					// node
					else
					{
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );
					}

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

					my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 1, range_end );
					my_agent->epmem_stmts_graph->update_edge_unique_last->bind_int( 2, r->first );
					my_agent->epmem_stmts_graph->update_edge_unique_last->execute( soar_module::op_reinit );
					// point (id, start)
					if ( range_start == range_end )
					{
						my_agent->epmem_stmts_graph->add_edge_point->bind_int( 1, r->first );
						my_agent->epmem_stmts_graph->add_edge_point->bind_int( 2, range_start );
						my_agent->epmem_stmts_graph->add_edge_point->execute( soar_module::op_reinit );
					}
					// node
					else
					{
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );
					}

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

		// update time wme on all states
		{
			Symbol* state = my_agent->bottom_goal;
			Symbol* my_time_sym = make_int_constant( my_agent, time_counter + 1 );

			while ( state != NULL )
			{
				if ( state->id.epmem_time_wme != NIL )
				{
					soar_module::remove_module_wme( my_agent, state->id.epmem_time_wme );
				}

				state->id.epmem_time_wme = soar_module::add_module_wme( my_agent, state->id.epmem_header, my_agent->epmem_sym_present_id, my_time_sym );

				state = state->id.higher_goal;
			}

			symbol_remove_ref( my_agent, my_time_sym );
		}
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
	bool return_val = false;

	{
		soar_module::sqlite_statement *my_q = my_agent->epmem_stmts_graph->valid_episode;

		my_q->bind_int( 1, memory_id );
		my_q->execute();
		return_val = ( my_q->column_int( 0 ) > 0 );
		my_q->reinitialize();
	}

	return return_val;
}

inline void _epmem_install_id_wme( agent* my_agent, Symbol* parent, Symbol* attr, std::map< epmem_node_id, std::pair< Symbol*, bool > >* ids, epmem_node_id q1, bool val_is_short_term, char val_letter, uint64_t val_num, epmem_id_mapping* id_record, soar_module::symbol_triple_list& retrieval_wmes )
{
	std::map< epmem_node_id, std::pair< Symbol*, bool > >::iterator id_p = ids->find( q1 );
	bool existing_identifier = ( id_p != ids->end() );

	if ( val_is_short_term )
	{
		if ( !existing_identifier )
		{
			id_p = ids->insert( std::make_pair< epmem_node_id, std::pair< Symbol*, bool > >( q1, std::make_pair< Symbol*, bool >( make_new_identifier( my_agent, ( ( attr->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE )?( attr->sc.name[0] ):('E') ), parent->id.level ), true ) ) ).first;
			if ( id_record )
			{
				epmem_id_mapping::iterator rec_p = id_record->find( q1 );
				if ( rec_p != id_record->end() )
				{
					rec_p->second = id_p->second.first;
				}
			}
		}

		epmem_buffer_add_wme( retrieval_wmes, parent, attr, id_p->second.first );

		if ( !existing_identifier )
		{
			symbol_remove_ref( my_agent, id_p->second.first );
		}
	}
	else
	{
		if ( existing_identifier )
		{
			epmem_buffer_add_wme( retrieval_wmes, parent, attr, id_p->second.first );
		}
		else
		{
			Symbol* value = smem_lti_soar_make( my_agent, smem_lti_get_id( my_agent, val_letter, val_num ), val_letter, val_num, parent->id.level );

			if ( id_record )
			{
				epmem_id_mapping::iterator rec_p = id_record->find( q1 );
				if ( rec_p != id_record->end() )
				{
					rec_p->second = value;
				}
			}

			epmem_buffer_add_wme( retrieval_wmes, parent, attr, value );
			symbol_remove_ref( my_agent, value );

			ids->insert( std::make_pair< epmem_node_id, std::pair< Symbol*, bool > >( q1, std::make_pair< Symbol*, bool >( value, !( ( value->id.impasse_wmes ) || ( value->id.input_wmes ) || ( value->id.slots ) ) ) ) );
		}
	}
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
 * 				  The id_record parameter is only used in the case
 * 				  that the graph-match has a match and creates
 * 				  a mapping of identifiers that should be recorded
 * 				  during reconstruction.
 **************************************************************************/
void epmem_install_memory( agent *my_agent, Symbol *state, epmem_time_id memory_id, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes, epmem_id_mapping *id_record = NULL )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->epmem_timers->ncb_retrieval->start();
	////////////////////////////////////////////////////////////////////////////

	// get the ^result header for this state
	Symbol *result_header = state->id.epmem_result_header;

	// initialize stat
	int64_t num_wmes = 0;
	my_agent->epmem_stats->ncb_wmes->set_value( num_wmes );

	// if no memory, say so
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		epmem_buffer_add_wme( meta_wmes, result_header, my_agent->epmem_sym_retrieved, my_agent->epmem_sym_no_memory );
		state->id.epmem_info->last_memory = EPMEM_MEMID_NONE;

		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->ncb_retrieval->stop();
		////////////////////////////////////////////////////////////////////////////

		return;
	}

	// remember this as the last memory installed
	state->id.epmem_info->last_memory = memory_id;

	// create a new ^retrieved header for this result
	Symbol *retrieved_header;
	retrieved_header = make_new_identifier( my_agent, 'R', result_header->id.level );
	if ( id_record )
	{
		(*id_record)[ EPMEM_NODEID_ROOT ] = retrieved_header;
	}

	epmem_buffer_add_wme( meta_wmes, result_header, my_agent->epmem_sym_retrieved, retrieved_header );
	symbol_remove_ref( my_agent, retrieved_header );

	// add *-id wme's
	{
		Symbol *my_meta;

		my_meta = make_int_constant( my_agent, static_cast<int64_t>( memory_id ) );
		epmem_buffer_add_wme( meta_wmes, result_header, my_agent->epmem_sym_memory_id, my_meta );
		symbol_remove_ref( my_agent, my_meta );

		my_meta = make_int_constant( my_agent, static_cast<int64_t>( my_agent->epmem_stats->time->get_value() ) );
		epmem_buffer_add_wme( meta_wmes, result_header, my_agent->epmem_sym_present_id, my_meta );
		symbol_remove_ref( my_agent, my_meta );
	}

	// install memory
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
		std::map< epmem_node_id, std::pair< Symbol*, bool > > ids;
		bool dont_abide_by_ids_second = ( my_agent->epmem_params->merge->get_value() == epmem_param_container::merge_add );

		// symbols used to create WMEs
		Symbol *attr = NULL;

		// lookup query
		soar_module::sqlite_statement *my_q;

		// initialize the lookup table
		ids[ EPMEM_NODEID_ROOT ] = std::make_pair< Symbol*, bool >( retrieved_header, true );

		// first identifiers (i.e. reconstruct)
		my_q = my_agent->epmem_stmts_graph->get_edges;
		{
			// relates to finite automata: q1 = d(q0, w)
			epmem_node_id q0; // id
			epmem_node_id q1; // attribute
			int64_t w_type; // we support any constant attribute symbol

			bool val_is_short_term = false;
			char val_letter = NIL;
			int64_t val_num = NIL;

			// used to lookup shared identifiers
			// the bool in the pair refers to if children are allowed on this id (re: lti)
			std::map< epmem_node_id, std::pair< Symbol*, bool> >::iterator id_p;

			// orphaned children
			std::queue< epmem_edge* > orphans;
			epmem_edge *orphan;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			my_q->bind_int( 5, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				// q0, w, q1, w_type
				q0 = my_q->column_int( 0 );
				q1 = my_q->column_int( 2 );
				w_type = my_q->column_int( 3 );

				switch ( w_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						attr = make_int_constant( my_agent,my_q->column_int( 1 ) );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						attr = make_float_constant( my_agent, my_q->column_double( 1 ) );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						attr = make_sym_constant( my_agent, const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 1 ) ) ) );
						break;
				}

				// short vs. long-term
				val_is_short_term = ( my_q->column_type( 4 ) == soar_module::null_t );
				if ( !val_is_short_term )
				{
					val_letter = static_cast<char>( my_q->column_int( 4 ) );
					val_num = static_cast<uint64_t>( my_q->column_int( 5 ) );
				}

				// get a reference to the parent
				id_p = ids.find( q0 );
				if ( id_p != ids.end() )
				{
					// if existing lti with kids don't touch
					if ( dont_abide_by_ids_second || id_p->second.second )
					{
						_epmem_install_id_wme( my_agent, id_p->second.first, attr, &( ids ), q1, val_is_short_term, val_letter, val_num, id_record, retrieval_wmes );
						num_wmes++;
					}

					symbol_remove_ref( my_agent, attr );
				}
				else
				{
					// out of order
					orphan = new epmem_edge;
					orphan->q0 = q0;
					orphan->w = attr;
					orphan->q1 = q1;

					orphan->val_letter = NIL;
					orphan->val_num = NIL;

					orphan->val_is_short_term = val_is_short_term;
					if ( !val_is_short_term )
					{
						orphan->val_letter = val_letter;
						orphan->val_num = val_num;
					}

					orphans.push( orphan );
				}
			}
			my_q->reinitialize();
			epmem_rit_clear_left_right( my_agent );

			// take care of any orphans
			if ( !orphans.empty() )
			{
				std::queue<epmem_edge *>::size_type orphans_left;
				std::queue<epmem_edge *> still_orphans;

				do
				{
					orphans_left = orphans.size();

					while ( !orphans.empty() )
					{
						orphan = orphans.front();
						orphans.pop();

						// get a reference to the parent
						id_p = ids.find( orphan->q0 );
						if ( id_p != ids.end() )
						{
							if ( dont_abide_by_ids_second || id_p->second.second )
							{
								_epmem_install_id_wme( my_agent, id_p->second.first, orphan->w, &( ids ), orphan->q1, orphan->val_is_short_term, orphan->val_letter, orphan->val_num, id_record, retrieval_wmes );
								num_wmes++;
							}

							symbol_remove_ref( my_agent, orphan->w );

							delete orphan;
						}
						else
						{
							still_orphans.push( orphan );
						}
					}

					orphans = still_orphans;
					while ( !still_orphans.empty() )
					{
						still_orphans.pop();
					}

				} while ( ( !orphans.empty() ) && ( orphans_left != orphans.size() ) );

				while ( !orphans.empty() )
				{
					orphan = orphans.front();
					orphans.pop();

					symbol_remove_ref( my_agent, orphan->w );

					delete orphan;
				}
			}
		}

		// then node_unique
		my_q = my_agent->epmem_stmts_graph->get_nodes;
		{
			epmem_node_id child_id;
			epmem_node_id parent_id;
			int64_t attr_type;
			int64_t value_type;

			std::pair< Symbol*, bool > parent;
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

				if ( dont_abide_by_ids_second || parent.second )
				{
					// make a symbol to represent the attribute
					switch ( attr_type )
					{
						case INT_CONSTANT_SYMBOL_TYPE:
							attr = make_int_constant( my_agent, my_q->column_int( 2 ) );
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
							value = make_int_constant( my_agent,my_q->column_int( 3 ) );
							break;

						case FLOAT_CONSTANT_SYMBOL_TYPE:
							value = make_float_constant( my_agent, my_q->column_double( 3 ) );
							break;

						case SYM_CONSTANT_SYMBOL_TYPE:
							value = make_sym_constant( my_agent, const_cast<char *>( (const char *) my_q->column_text( 3 ) ) );
							break;
					}

					epmem_buffer_add_wme( retrieval_wmes, parent.first, attr, value );
					num_wmes++;

					symbol_remove_ref( my_agent, attr );
					symbol_remove_ref( my_agent, value );
				}
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

	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( memory_id != EPMEM_MEMID_NONE )
	{
		soar_module::sqlite_statement *my_q = my_agent->epmem_stmts_graph->next_episode;
		my_q->bind_int( 1, memory_id );
		if ( my_q->execute() == soar_module::row )
		{
			return_val = (epmem_time_id) my_q->column_int( 0 );
		}

		my_q->reinitialize();
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

	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( memory_id != EPMEM_MEMID_NONE )
	{
		soar_module::sqlite_statement *my_q = my_agent->epmem_stmts_graph->prev_episode;
		my_q->bind_int( 1, memory_id );
		if ( my_q->execute() == soar_module::row )
		{
			return_val = (epmem_time_id) my_q->column_int( 0 );
		}

		my_q->reinitialize();
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

//////////////////////////////////////////////////////////
// Justin's Stuff
//////////////////////////////////////////////////////////

#define JUSTIN_DEBUG false

const char* epmem_find_unique_node_query = "SELECT child_id, value, last FROM node_unique WHERE parent_id=? AND attrib=? AND ?<last ORDER BY last DESC";
const char* epmem_find_unique_node_value_query = "SELECT child_id, value, last FROM node_unique WHERE parent_id=? AND attrib=? AND value=? AND ?<last ORDER BY last DESC";
const char* epmem_find_unique_edge_query = "SELECT parent_id, q1, last FROM edge_unique WHERE q0=? AND w=? AND ?<last ORDER BY last DESC";
const char* epmem_find_unique_edge_value_query = "SELECT parent_id, q1, last FROM edge_unique WHERE q0=? AND w=? AND q1=? AND ?<last ORDER BY last DESC";
const char* epmem_dummy = "SELECT ? as start";

// Because the DB records when things are /inserted/, we need to offset
// the start by 1 to /remove/ them at the right time. Ditto to even
// include those intervals correctly
const char* epmem_find_interval_queries[2][2][3] = {
	{
		{
			"SELECT (e.start - 1) AS start FROM node_range e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC",
			"SELECT (e.start - 1) AS start FROM node_now e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC",
			"SELECT (e.start - 1) AS start FROM node_point e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC"
		},
		{
			"SELECT e.end AS end FROM node_range e WHERE e.id=? AND e.end>0 AND e.start<=? ORDER BY e.end DESC",
			"SELECT ? AS end FROM node_now e WHERE e.id=? AND e.start<=? ORDER BY e.start",
			"SELECT e.start AS end FROM node_point e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC"
		}
	},
	{
		{
			"SELECT (e.start - 1) AS start FROM edge_range e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC",
			"SELECT (e.start - 1) AS start FROM edge_now e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC",
			"SELECT (e.start - 1) AS start FROM edge_point e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC"
		},
		{
			"SELECT e.end AS end FROM edge_range e WHERE e.id=? AND e.end>0 AND e.start<=? ORDER BY e.end DESC",
			"SELECT ? AS end FROM edge_now e WHERE e.id=? AND e.start<=? ORDER BY e.start",
			"SELECT e.start AS end FROM edge_point e WHERE e.id=? AND e.start<=? ORDER BY e.start DESC"
		}
	}
};

// notice that the start and end queries in epmem_find_lti_queries are _asymetric_
// in that the the starts have ?<e.start and the ends have ?<=e.start
// this small difference means that the start of the very first interval
// (ie. the one where the start is at or before the promotion time) will be ignored
// then we can simply add a single epmem_interval_query to the queue, and it will
// terminate any LTI interval appropriately
const char* epmem_find_lti_queries[2][3] = {
	{
		"SELECT (e.start - 1) AS start FROM edge_range e WHERE e.id=? AND ?<e.start AND e.start<=? ORDER BY e.start DESC",
		"SELECT (e.start - 1) AS start FROM edge_now e WHERE e.id=? AND ?<e.start AND e.start<=? ORDER BY e.start DESC",
		"SELECT (e.start - 1) AS start FROM edge_point e WHERE e.id=? AND ?<e.start AND e.start<=? ORDER BY e.start DESC"
	},
	{
		"SELECT e.end AS end FROM edge_range e WHERE e.id=? AND e.end>0 AND ?<=e.start AND e.start<=? ORDER BY e.end DESC",
		"SELECT ? AS end FROM edge_now e WHERE e.id=? AND ?<=e.start AND e.start<=? ORDER BY e.start",
		"SELECT e.start AS end FROM edge_point e WHERE e.id=? AND ?<=e.start AND e.start<=? ORDER BY e.start DESC"
	}
};

void epmem_print_state(epmem_literal_set& literals, epmem_edge_sql_map uedge_cache[], epmem_interval_set& intervals) {
	//std::map<epmem_node_id, std::string> tsh;
	std::cout << std::endl;
	std::cout << "digraph {" << std::endl;
	std::cout << "node [style=\"filled\"]" << std::endl;
	// LITERALS
	std::cout << "subgraph cluster_literals {" << std::endl;
	std::cout << "node [fillcolor=\"#0084D1\"]" << std::endl;
	for (epmem_literal_set::iterator lit_iter = literals.begin(); lit_iter != literals.end(); lit_iter++) {
		epmem_dnf_literal* literal = *lit_iter;
		if (literal->id_sym) {
			std::cout << "\"" << literal->value_sym << "\" [label=\"" << literal->value_sym << "\"";
			if (!literal->is_edge_not_node) {
				std::cout << ", shape=\"rectangle\"";
			}
			if (literal->satisfied) {
				std::cout << ", penwidth=\"2.0\"";
			}
			if (literal->is_neg_q) {
				std::cout << ", fillcolor=\"#C5000B\"";
			}
			std::cout << "]" << std::endl;
			std::cout << "\"" << literal->id_sym << "\" -> \"" << literal->value_sym << "\" [label=\"";
			if (literal->attr == EPMEM_NODEID_BAD) {
				std::cout << "?";
			} else {
				std::cout << literal->attr;
			}
			std::cout << "\\n" << literal << "\"]" << std::endl;
		}
	}
	std::cout << "}" << std::endl;
	// NODES / NODE->NODE
	std::cout << "subgraph cluster_intervals {" << std::endl;
	std::cout << "node [fillcolor=\"#FFD320\"]" << std::endl;
	std::set<std::pair<epmem_unique_edge_query*, epmem_node_id> > drawn;
	for (epmem_interval_set::iterator inter_iter = intervals.begin(); inter_iter != intervals.end(); inter_iter++) {
		epmem_interval_query* interval = *inter_iter;
		epmem_unique_edge_query* uedge = interval->uedge;
		epmem_sql_edge triple = uedge->edge_info;
		triple.q1 = interval->q1;
		std::pair<epmem_unique_edge_query*, epmem_node_id> pair = std::make_pair(uedge, triple.q1);
		if (triple.q1 != EPMEM_NODEID_ROOT && !drawn.count(pair)) {
			drawn.insert(pair);
			if (!uedge->is_edge_not_node) {
				std::cout << "\"n" << triple.q1 << "\" [shape=\"rect\"]" << std::endl;
			}
			std::cout << "\"e" << triple.q0 << "\" -> \"" << (uedge->is_edge_not_node ? "e" : "n") << triple.q1 << "\" [label=\"" << triple.w << "\"]" << std::endl;
		}
	}
	std::cout << "}" << std::endl;
	// UEDGES / LITERAL->UEDGE
	std::cout << "subgraph cluster_uedges {" << std::endl;
	std::cout << "node [fillcolor=\"#008000\"]" << std::endl;
	std::multimap<epmem_node_id, epmem_unique_edge_query*> parent_uedge_map;
	for (int type = EPMEM_TYPE_NODE; type <= EPMEM_TYPE_EDGE; type++) {
		for (epmem_edge_sql_map::iterator uedge_iter = uedge_cache[type].begin(); uedge_iter != uedge_cache[type].end(); uedge_iter++) {
			epmem_sql_edge triple = (*uedge_iter).first;
			epmem_unique_edge_query* uedge = (*uedge_iter).second;
			if (triple.w != EPMEM_NODEID_BAD && uedge) {
				std::cout << "\"" << uedge << "\" [label=\"" << uedge << "\\n(" << triple.q0 << ", " << triple.w << ", ";
				if (triple.q1 == EPMEM_NODEID_BAD) {
					std::cout << "?";
				} else {
					std::cout << triple.q1;
				}
				std::cout << ")\"";
				if (!uedge->is_edge_not_node) {
					std::cout << ", [shape=\"rect\"";
				}
				std::cout << "]" << std::endl;
				for (epmem_literal_set::iterator lit_iter = uedge->literals.begin(); lit_iter != uedge->literals.end(); lit_iter++) {
					epmem_dnf_literal* literal = *lit_iter;
					std::cout << "\"" << literal->value_sym << "\" -> \"" << uedge << "\"" << std::endl;
				}
				parent_uedge_map.insert(std::make_pair(triple.q0, uedge));
			}
		}
	}
	std::cout << "}" << std::endl;
	// UEDGE->UEDGE / UEDGE->NODE
	drawn.clear();
	for (epmem_interval_set::iterator inter_iter = intervals.begin(); inter_iter != intervals.end(); inter_iter++) {
		epmem_interval_query* interval = *inter_iter;
		epmem_unique_edge_query* uedge = interval->uedge;
		std::pair<epmem_unique_edge_query*, epmem_node_id> pair = std::make_pair(uedge, interval->q1);
		if (uedge->edge_info.w != EPMEM_NODEID_BAD && !drawn.count(pair)) {
			drawn.insert(pair);
			pair = std::make_pair(uedge, uedge->edge_info.q0);
			if (!drawn.count(pair)) {
				drawn.insert(pair);
				std::cout << "\"" << uedge << "\" -> \"e" << uedge->edge_info.q0 << "\"" << std::endl;
			}
			std::cout << "\"" << uedge << "\" -> \"" << (uedge->is_edge_not_node ? "e" : "n") << interval->q1 << "\" [style=\"dashed\"]" << std::endl;
			std::pair<std::multimap<epmem_node_id, epmem_unique_edge_query*>::iterator, std::multimap<epmem_node_id, epmem_unique_edge_query*>::iterator> uedge_iters = parent_uedge_map.equal_range(interval->q1);
			for (std::multimap<epmem_node_id, epmem_unique_edge_query*>::iterator uedge_iter = uedge_iters.first; uedge_iter != uedge_iters.second; uedge_iter++) {
				std::cout << "\"" << uedge << "\" -> \"" << (*uedge_iter).second << "\"" << std::endl;
			}
		}
	}
	std::cout << "}" << std::endl;
}

bool epmem_gm_mcv_comparator(const epmem_dnf_literal* a, const epmem_dnf_literal* b) {
	return (a->matches.size() < b->matches.size());
}

epmem_dnf_literal* epmem_build_dnf(wme* cue_wme, int query_type, std::map<wme*, epmem_dnf_literal*>& sym_cache, epmem_literal_set& leaf_literals, epmem_symbol_int_map& symbol_incoming_count, std::set<Symbol*>& visiting, agent* my_agent, epmem_literal_deque& gm_ordering, soar_module::wme_set& cue_wmes, epmem_literal_set& cleanup_literals) {
	// if the value is being visited, this is part of a loop; return NULL
	// remove this check (and in fact, the entire visiting parameter) if cyclic cues are allowed
	if (visiting.count(cue_wme->value)) {
		return NULL;
	}
	// if the value is an identifier and we've been here before, we can return the previous literal
	if (sym_cache.count(cue_wme)) {
		return sym_cache[cue_wme];
	}

	cue_wmes.insert(cue_wme);
	Symbol* value = cue_wme->value;
	epmem_dnf_literal* literal = new epmem_dnf_literal();

	if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) { // WME is a value
		literal->is_edge_not_node = EPMEM_TYPE_NODE;
		literal->is_leaf = true;
		literal->q1 = epmem_temporal_hash(my_agent, value);
		leaf_literals.insert(literal);
	} else if (value->id.smem_lti) { // WME is an LTI
		// if we can find the LTI node id, cache it; otherwise, return failure
		my_agent->epmem_stmts_graph->find_lti->bind_int(1, static_cast<uint64_t>(value->id.name_letter));
		my_agent->epmem_stmts_graph->find_lti->bind_int(2, static_cast<uint64_t>(value->id.name_number));
		if (my_agent->epmem_stmts_graph->find_lti->execute() == soar_module::row) {
			literal->is_edge_not_node = EPMEM_TYPE_EDGE;
			literal->is_leaf = true;
			literal->q1 = my_agent->epmem_stmts_graph->find_lti->column_int(0);
			my_agent->epmem_stmts_graph->find_lti->reinitialize();
			leaf_literals.insert(literal);
		} else {
			my_agent->epmem_stmts_graph->find_lti->reinitialize();
			delete literal;
			return NULL;
		}
	} else { // WME is a normal identifier
		// we determine whether it is a leaf by checking for children
		epmem_wme_list* children = epmem_get_augs_of_id(value, get_new_tc_number(my_agent));
		literal->is_edge_not_node = EPMEM_TYPE_EDGE;
		literal->q1 = EPMEM_NODEID_BAD;

		// if the WME has no children, then it's a leaf
		// otherwise, we recurse for all children
		if (children->empty()) {
			literal->is_leaf = true;
			leaf_literals.insert(literal);
			delete children;
		} else {
			bool cycle = false;
			visiting.insert(cue_wme->value);
			for (epmem_wme_list::iterator wme_iter = children->begin(); wme_iter != children->end(); wme_iter++) {
				// check to see if this child forms a cycle
				// if it does, we skip over it
				epmem_dnf_literal* child = epmem_build_dnf(*wme_iter, query_type, sym_cache, leaf_literals, symbol_incoming_count, visiting, my_agent, gm_ordering, cue_wmes, cleanup_literals);
				if (child) {
					child->parents.insert(literal);
					literal->children.insert(child);
				} else {
					cycle = true;
				}
			}
			delete children;
			visiting.erase(cue_wme->value);
			// if all children of this WME lead to cycles, then we don't need to walk this path
			// in essence, this forces the DNF graph to be acyclic
			// this results in savings in not walking edges and intervals
			if (cycle && literal->children.empty()) {
				delete literal;
				return NULL;
			}
			literal->is_leaf = false;
			epmem_symbol_int_map::iterator rem_iter = symbol_incoming_count.find(value);
			if (rem_iter == symbol_incoming_count.end()) {
				symbol_incoming_count[value] = 1;
			} else {
				(*rem_iter).second++;
			}
		}
	}

	cleanup_literals.insert(literal);
	if (!query_type) {
		gm_ordering.push_front(literal);
	}

	literal->attr = epmem_temporal_hash(my_agent, cue_wme->attr);
	literal->id_sym = cue_wme->id;
	literal->value_sym = cue_wme->value;
	literal->is_neg_q = query_type;
	literal->weight = (literal->is_neg_q ? -1 : 1) * (my_agent->epmem_params->balance->get_value() >= 1.0 - 1.0e-8 ? 1.0 : wma_get_wme_activation(my_agent, cue_wme, true));
	literal->satisfied = false;
	sym_cache[cue_wme] = literal;
	return literal;
}

bool epmem_satisfy_literal(epmem_dnf_literal* literal, epmem_node_id parent, epmem_node_id child, double& current_score, int& current_cardinality, epmem_match_int_map& symbol_match_count, epmem_sql_edge_set activated[], epmem_symbol_int_map& symbol_incoming_count) {
	epmem_symbol_node_pair match;
	epmem_match_int_map::iterator match_iter;
	// check if the ancestors of this literal are satisfied
	bool parents_satisfied = (literal->id_sym == NULL);
	if (!parents_satisfied) {
		match = std::make_pair(literal->id_sym, parent);
		match_iter = symbol_match_count.find(match);
		parents_satisfied = (match_iter != symbol_match_count.end()) && ((*match_iter).second == symbol_incoming_count[literal->id_sym]);
	}
	// if yes
	if (parents_satisfied) {
		literal->satisfied = true;
		// add the edge as a match
		literal->matches.insert(std::make_pair(parent, child));
		if (literal->is_leaf && literal->matches.size() == 1) {
			current_score += literal->weight;
			current_cardinality += (literal->is_neg_q ? -1 : 1);
			if (JUSTIN_DEBUG) {
				std::cout << "			NEW SCORE: " << current_score << ", " << current_cardinality << std::endl;
			}
			return true;
		} else {
			bool changed_score = false;
			// increase the matches for its child symbol
			match = std::make_pair(literal->value_sym, child);
			match_iter = symbol_match_count.find(match);
			if (match_iter == symbol_match_count.end()) {
				symbol_match_count[match] = 1;
				// recurse over child literals
				for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++) {
					epmem_dnf_literal* child_lit = *child_iter;
					epmem_sql_edge temp_triple = {child, child_lit->attr+1, EPMEM_NODEID_BAD};
					epmem_sql_edge_set::iterator end = activated[child_lit->is_edge_not_node].lower_bound(temp_triple);
 					temp_triple.w = child_lit->attr;
					for (epmem_sql_edge_set::iterator edge_iter = activated[child_lit->is_edge_not_node].lower_bound(temp_triple); edge_iter != end; edge_iter++) {
						epmem_sql_edge edge = *edge_iter;
						if (edge.q1 == EPMEM_NODEID_BAD) {
							continue;
						}
						changed_score |= epmem_satisfy_literal(child_lit, edge.q0, edge.q1, current_score, current_cardinality, symbol_match_count, activated, symbol_incoming_count);
					}
				}
			} else {
				(*match_iter).second++;
			}
			return changed_score;
		}
	}
	return false;
}

bool epmem_unsatisfy_literal(epmem_dnf_literal* literal, epmem_node_id parent, epmem_node_id child, double& current_score, int& current_cardinality, epmem_match_int_map& symbol_match_count, epmem_symbol_int_map& symbol_incoming_count) {
	if (!literal->satisfied) {
		return false;
	}
	epmem_symbol_node_pair match;
	epmem_match_int_map::iterator match_iter;
	// erase the edge from this literal's matches
	literal->matches.erase(std::make_pair(parent, child));
	if (literal->is_leaf && literal->matches.size() == 0) {
		literal->satisfied = false;
		current_score -= literal->weight;
		current_cardinality -= (literal->is_neg_q ? -1 : 1);
		if (JUSTIN_DEBUG) {
			std::cout << "			NEW SCORE: " << current_score << ", " << current_cardinality << std::endl;
		}
		return true;
	} else {
		bool changed_score = false;
		// if this literal is no longer satisfied, recurse on all children
		// if this literal is still satisfied, recurse on children who is matching on descendants of this edge
		if (literal->matches.size() == 0) {
			literal->satisfied = false;
			// decrease the matches for its child symbol
			match = std::make_pair(literal->value_sym, child);
			match_iter = symbol_match_count.find(match);
			assert(match_iter != symbol_match_count.end());
			(*match_iter).second--;
			if ((*match_iter).second == 0) {
				symbol_match_count.erase(match_iter);
			}
			for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++) {
				epmem_dnf_literal* child_lit = *child_iter;
				epmem_node_pair_set::iterator node_iter = child_lit->matches.begin();
				if (node_iter != child_lit->matches.end()) {
					changed_score |= epmem_unsatisfy_literal(child_lit, (*node_iter).first, (*node_iter).second, current_score, current_cardinality, symbol_match_count, symbol_incoming_count);
				}
			}
		} else {
			epmem_node_pair node_pair = std::make_pair(child, EPMEM_NODEID_BAD);
			for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++) {
				epmem_dnf_literal* child_lit = *child_iter;
				epmem_node_pair_set::iterator node_iter = child_lit->matches.lower_bound(node_pair);
				if (node_iter != child_lit->matches.end() && (*node_iter).first == child) {
					changed_score |= epmem_unsatisfy_literal(child_lit, (*node_iter).first, (*node_iter).second, current_score, current_cardinality, symbol_match_count, symbol_incoming_count);
				}
			}
		}
		return changed_score;
	}
}

void epmem_register_uedges(epmem_node_id parent, epmem_dnf_literal* literal, epmem_unique_edge_pq& edge_pq, double& current_score, int& current_cardinality, bool& changed_score, epmem_time_id after, epmem_edge_sql_map uedge_cache[], epmem_sql_edge_set activated[], agent* my_agent) {
	// we don't need to keep track of visited literals/nodes because the literals are guaranteed to be acyclic
	// that is, the expansion to the literal's children will eventually bottom out
	// select the query
	epmem_sql_edge info;
	info.q0 = parent;
	info.w = literal->attr;
	int is_edge = literal->is_edge_not_node;
	const char* sql_statement = NULL;
	if (literal->q1 != EPMEM_NODEID_BAD) {
		info.q1 = literal->q1;
		if (is_edge) {
			sql_statement = epmem_find_unique_edge_value_query;
		} else {
			sql_statement = epmem_find_unique_node_value_query;
		}
	} else {
		info.q1 = EPMEM_NODEID_BAD;
		if (is_edge) {
			sql_statement = epmem_find_unique_edge_query;
		} else {
			sql_statement = epmem_find_unique_node_query;
		}
	}
	if (JUSTIN_DEBUG) {
		std::cout << "		RECURSING ON " << parent << " " << literal << std::endl;
	}
	bool created = false;
	epmem_edge_sql_map::iterator uedge_iter = uedge_cache[is_edge].find(info);
	if (uedge_iter == uedge_cache[is_edge].end() || (*uedge_iter).second == NULL) {
		// if the unique edge does not exist, create a new unique edge query
		soar_module::sqlite_statement* uedge_sql;
		allocate_with_pool(my_agent, &(my_agent->epmem_sql_pool), &uedge_sql);
		new(uedge_sql) soar_module::sqlite_statement(my_agent->epmem_db, sql_statement, my_agent->epmem_timers->query_sql_edge);
		uedge_sql->prepare();
		int bind_pos = 1;
		uedge_sql->bind_int(bind_pos++, info.q0);
		uedge_sql->bind_int(bind_pos++, info.w);
		if (literal->q1 != EPMEM_NODEID_BAD) {
			uedge_sql->bind_int(bind_pos++, info.q1);
		}
		uedge_sql->bind_int(bind_pos++, after);
		if (uedge_sql->execute() == soar_module::row) {
			epmem_unique_edge_query* child_uedge;
			allocate_with_pool(my_agent, &(my_agent->epmem_uedge_pool), &child_uedge);
			child_uedge->edge_info = info;
			child_uedge->is_edge_not_node = literal->is_edge_not_node;
			child_uedge->sql = uedge_sql;
			new(&(child_uedge->literals)) epmem_literal_set();
			child_uedge->literals.insert(literal);
			child_uedge->time = child_uedge->sql->column_int(2);
			edge_pq.push(child_uedge);
			uedge_cache[is_edge][info] = child_uedge;
			created = true;
		} else {
			uedge_sql->~sqlite_statement();
			free_with_pool(&(my_agent->epmem_sql_pool), uedge_sql);
		}
	} else {
		// otherwse, if the uedge has not been registered with this literal
		epmem_unique_edge_query* child_uedge = (*uedge_iter).second;
		if (!child_uedge->literals.count(literal)) {
			child_uedge->literals.insert(literal);
			// if the literal is an edge with no specified value, add the literal to all potential uedges
			if (literal->q1 == EPMEM_NODEID_BAD) {
				for (uedge_iter++; uedge_iter != uedge_cache[is_edge].end(); uedge_iter++) {
					epmem_sql_edge child_edge = (*uedge_iter).first;
					// make sure we're still looking at the right edge(s)
					if (child_edge.q0 != info.q0 || child_edge.w != info.w) {
						break;
					}
					child_uedge = (*uedge_iter).second;
					if (child_edge.q1 == EPMEM_NODEID_BAD || (child_uedge && !child_uedge->is_edge_not_node)) {
						continue;
					}
					// if the literal is a leaf, add the literal to the uedge and continue
					// otherwise, we need to recurse
					if (literal->is_leaf) {
						child_uedge->literals.insert(literal);
					} else {
						for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++) {
							epmem_register_uedges(child_edge.q1, *child_iter, edge_pq, current_score, current_cardinality, changed_score, after, uedge_cache, activated, my_agent);
						}
					}
				}
			}
			created = true;
		}
	}
	if (!created) {
		// FIXME retractions from lack of edges
	}
}

bool epmem_graph_match(epmem_literal_deque::iterator& dnf_iter, epmem_literal_deque::iterator& iter_end, epmem_literal_node_pair_map& bindings, epmem_node_symbol_map bound_nodes[], agent* my_agent, int depth = 0) {
	if (dnf_iter == iter_end) {
		return true;
	}
	epmem_dnf_literal* literal = *dnf_iter;
	if (bindings.count(literal)) {
		return false;
	}
	epmem_literal_deque::iterator next_iter = dnf_iter;
	next_iter++;
#ifdef USE_MEM_POOL_ALLOCATORS
	epmem_node_set failed_parents = epmem_node_set(std::less<epmem_node_id>(), soar_module::soar_memory_pool_allocator<epmem_node_id>(my_agent));
	epmem_node_set failed_children = epmem_node_set(std::less<epmem_node_id>(), soar_module::soar_memory_pool_allocator<epmem_node_id>(my_agent));
#else
	epmem_node_set failed_parents;
	epmem_node_set failed_children;
#endif
	// go through the list of matches, binding each one to this literal in turn
	for (epmem_node_pair_set::iterator match_iter = literal->matches.begin(); match_iter != literal->matches.end(); match_iter++) {
		epmem_node_id q0 = (*match_iter).first;
		epmem_node_id q1 = (*match_iter).second;
		if (failed_parents.count(q0)) {
			continue;
		}
		if (JUSTIN_DEBUG) {
			for (int i = 0; i < depth; i++) {
				std::cout << "\t";
			}
			std::cout << "TRYING " << literal << " " << q0 << std::endl;
		}
		bool relations_okay = true;
		// for all parents
		for (epmem_literal_set::iterator parent_iter = literal->parents.begin(); relations_okay && parent_iter != literal->parents.end(); parent_iter++) {
			epmem_dnf_literal* parent = *parent_iter;
			epmem_literal_node_pair_map::iterator bind_iter = bindings.find(parent);
			if (bind_iter != bindings.end() && (*bind_iter).second.second != q0) {
				relations_okay = false;
			}
		}
		if (!relations_okay) {
			if (JUSTIN_DEBUG) {
				for (int i = 0; i < depth; i++) {
					std::cout << "\t";
				}
				std::cout << "PARENT CONSTRAINT FAIL" << std::endl;
			}
			failed_parents.insert(q0);
			continue;
		}
		// if the node has already been bound, make sure it's bound to the same thing
		epmem_node_symbol_map::iterator binder = bound_nodes[literal->is_edge_not_node].find(q1);
		if (binder != bound_nodes[literal->is_edge_not_node].end() && (*binder).second != literal->value_sym) {
			failed_children.insert(q1);
			continue;
		}
		if (JUSTIN_DEBUG) {
			for (int i = 0; i < depth; i++) {
				std::cout << "\t";
			}
			std::cout << "TRYING " << literal << " " << q0 << " " << q1 << std::endl;
		}
		if (literal->q1 != EPMEM_NODEID_BAD && literal->q1 != q1) {
			relations_okay = false;
		}
		// for all children
		for (epmem_literal_set::iterator child_iter = literal->children.begin(); relations_okay && child_iter != literal->children.end(); child_iter++) {
			epmem_dnf_literal* child = *child_iter;
			epmem_literal_node_pair_map::iterator bind_iter = bindings.find(child);
			if (bind_iter != bindings.end() && (*bind_iter).second.first != q1) {
				relations_okay = false;
			}
		}
		if (!relations_okay) {
			if (JUSTIN_DEBUG) {
				for (int i = 0; i < depth; i++) {
					std::cout << "\t";
				}
				std::cout << "CHILD CONSTRAINT FAIL" << std::endl;
			}
			failed_children.insert(q1);
			continue;
		}
		if (JUSTIN_DEBUG) {
			for (int i = 0; i < depth; i++) {
				std::cout << "\t";
			}
			std::cout << literal << " " << q0 << " " << q1 << std::endl;
		}
		// temporarily modify the bindings and bound nodes
		bindings[literal] = std::make_pair(q0, q1);
		bound_nodes[literal->is_edge_not_node][q1] = literal->value_sym;
		// recurse on the rest of the list
		bool list_satisfied = epmem_graph_match(next_iter, iter_end, bindings, bound_nodes, my_agent, depth + 1);
		// if the rest of the list matched, we've succeeded
		// otherwise, undo the temporarily modifications and try again
		if (list_satisfied) {
			return true;
		} else {
			bindings.erase(literal);
			bound_nodes[literal->is_edge_not_node].erase(q1);
		}
	}
	// this means we've tried everything and this whole exercise was a waste of time
	// EPIC FAIL
	if (JUSTIN_DEBUG) {
		for (int i = 0; i < depth; i++) {
			std::cout << "\t";
		}
		std::cout << "EPIC FAIL" << std::endl;
	}
	return false;
}

void epmem_process_query(agent *my_agent, Symbol *state, Symbol *pos_query, Symbol *neg_query, epmem_time_list& prohibits, epmem_time_id before, epmem_time_id after, epmem_lti_map& /*ltis*/, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes) {
	// a query must contain a positive cue
	if (pos_query == NULL) {
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_bad_cmd);
		return;
	}

	// before and after, if specified, must be valid relative to each other
	if (before != EPMEM_MEMID_NONE && after != EPMEM_MEMID_NONE && before <= after) {
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_bad_cmd);
		return;
	}

	if (JUSTIN_DEBUG) {
		std::cout << std::endl << "==========================" << std::endl << std::endl;
	}

	my_agent->epmem_timers->query->start();

	// sort probibit's
	if (!prohibits.empty()) {
		std::sort(prohibits.begin(), prohibits.end());
	}

	// epmem options
	bool do_graph_match = (my_agent->epmem_params->graph_match->get_value() == soar_module::on);

	// variables needed for building the DNF
	epmem_dnf_literal* root_literal = new epmem_dnf_literal();
	epmem_literal_set leaf_literals;

	// variables needed to track satisfiability
	epmem_symbol_int_map symbol_incoming_count;
	epmem_match_int_map symbol_match_count;
#ifdef USE_MEM_POOL_ALLOCATORS
	epmem_sql_edge_set activated[2] = {
		epmem_sql_edge_set(std::less<epmem_sql_edge>(), soar_module::soar_memory_pool_allocator<epmem_sql_edge>(my_agent)),
		epmem_sql_edge_set(std::less<epmem_sql_edge>(), soar_module::soar_memory_pool_allocator<epmem_sql_edge>(my_agent))
	};
#else
	epmem_sql_edge_set activated[2] = {epmem_sql_edge_set(), epmem_sql_edge_set()};
#endif

	// variables needed for cleanup
	epmem_interval_set interval_cleanup; // FIXME replace this with sym_cache below
	std::map<wme*, epmem_dnf_literal*> sym_cache;
	epmem_edge_sql_map uedge_cache[2];
	epmem_literal_set literal_cleanup;

	// variables needed for graphmatch
	epmem_literal_deque gm_ordering;

	// build the DNF graph while checking for leaf WMEs
	{
		my_agent->epmem_timers->query_dnf->start();
		epmem_wme_list* pos_query_wmes = epmem_get_augs_of_id(pos_query, get_new_tc_number(my_agent));
		epmem_wme_list* neg_query_wmes = NULL;
		if (neg_query != NULL) {
			neg_query_wmes = epmem_get_augs_of_id(neg_query, get_new_tc_number(my_agent));
		}

		root_literal->id_sym = NULL;
		root_literal->value_sym = pos_query;
		root_literal->is_neg_q = EPMEM_NODE_POS;
		root_literal->is_edge_not_node = EPMEM_TYPE_EDGE;
		root_literal->is_leaf = false;
		root_literal->attr = EPMEM_NODEID_BAD;
		root_literal->q1 = EPMEM_NODEID_ROOT;
		root_literal->weight = 0.0;
		root_literal->satisfied = false;
		literal_cleanup.insert(root_literal);
		symbol_incoming_count[pos_query] = 1;

		std::set<Symbol*> visiting;
		visiting.insert(pos_query);
		visiting.insert(neg_query);
		for (int query_type = EPMEM_NODE_POS; query_type <= EPMEM_NODE_NEG; query_type++) {
			epmem_wme_list* query_wmes = NULL;
			switch (query_type) {
				case EPMEM_NODE_POS:
					query_wmes = pos_query_wmes;
					break;
				case EPMEM_NODE_NEG:
					query_wmes = neg_query_wmes;
					break;
			}
			if (!query_wmes) {
				continue;
			}
			// for each first level WME, build up a DNF
			while (query_wmes->size()) {
				wme* first_level = query_wmes->front();
				query_wmes->pop_front();
				epmem_dnf_literal* root = epmem_build_dnf(first_level, query_type, sym_cache, leaf_literals, symbol_incoming_count, visiting, my_agent, gm_ordering, cue_wmes, literal_cleanup);
				if (root) {
					epmem_node_id attr = epmem_temporal_hash(my_agent, first_level->attr);
					// force all first level literals to have the same id symbol
					root->id_sym = pos_query;
					root->parents.insert(root_literal);
					root_literal->children.insert(root);
				}
			}
			delete query_wmes;
		}
		my_agent->epmem_timers->query_dnf->stop();
	}

	// priority queues for interval walk
	epmem_unique_edge_pq edge_pq;
	epmem_interval_pq interval_pq;

	// various things about the current and the best episodes
	epmem_time_id best_episode = EPMEM_MEMID_NONE;
	double best_score = 0;
	bool best_graph_matched = false;
	int best_cardinality = 0;
	epmem_literal_node_pair_map best_bindings;
	epmem_time_id current_episode;
	double current_score = 0;
	int current_cardinality = 0;
	epmem_time_id next_episode;

	// the highest possible score and cardinality score
	double perfect_score = 0;
	int perfect_cardinality = 0;
	for (epmem_literal_set::iterator iter = leaf_literals.begin(); iter != leaf_literals.end(); iter++) {
		if (!(*iter)->is_neg_q) {
			perfect_score += (*iter)->weight;
			perfect_cardinality++;
		}
	}

	// set default values for before and after
	if (before == EPMEM_MEMID_NONE) {
		before = my_agent->epmem_stats->time->get_value() - 1;
	} else {
		before = before - 1; // since before's are strict
	}
	if (after == EPMEM_MEMID_NONE) {
		after = 0;
	}
	current_episode = before;

	// create dummy edges and intervals
	{
		// insert dummy unique edge and interval end point queries for DNF root
		// we make an SQL statement just so we don't have to do anything special at cleanup
		epmem_unique_edge_query* root_uedge;
		allocate_with_pool(my_agent, &(my_agent->epmem_uedge_pool), &root_uedge);
		root_uedge->edge_info.q0 = EPMEM_NODEID_BAD;
		root_uedge->edge_info.w = EPMEM_NODEID_BAD;
		root_uedge->edge_info.q1 = EPMEM_NODEID_ROOT;
		root_uedge->is_edge_not_node = EPMEM_TYPE_EDGE;
		new(&(root_uedge->literals)) epmem_literal_set();
		root_uedge->literals.insert(root_literal);
		allocate_with_pool(my_agent, &(my_agent->epmem_sql_pool), &root_uedge->sql);
		new(root_uedge->sql) soar_module::sqlite_statement(my_agent->epmem_db, epmem_dummy);
		root_uedge->sql->prepare();
		root_uedge->sql->bind_int(1, LLONG_MAX);
		root_uedge->sql->execute();
		root_uedge->time = LLONG_MAX;
		edge_pq.push(root_uedge);
		uedge_cache[EPMEM_TYPE_EDGE][root_uedge->edge_info] = root_uedge;
		epmem_interval_query* root_interval;
		allocate_with_pool(my_agent, &(my_agent->epmem_interval_pool), &root_interval);
		root_interval->q1 = EPMEM_NODEID_ROOT;
		root_interval->is_end_point = true;
		root_interval->uedge = root_uedge;
		allocate_with_pool(my_agent, &(my_agent->epmem_sql_pool), &root_interval->sql);
		new(root_interval->sql) soar_module::sqlite_statement(my_agent->epmem_db, epmem_dummy);
		root_interval->sql->prepare();
		root_interval->sql->bind_int(1, before);
		root_interval->sql->execute();
		root_interval->time = before;
		root_interval->pool = NULL;
		interval_pq.push(root_interval);
		interval_cleanup.insert(root_interval);
	}

	if (JUSTIN_DEBUG) {
		epmem_print_state(literal_cleanup, uedge_cache, interval_cleanup);
	}

	// pools of interval SQL queries
	epmem_sql_list find_interval_pool[2][2][3];
	epmem_sql_list find_lti_pool[2][3];

	// main loop of interval walk
	my_agent->epmem_timers->query_walk->start();
	while (edge_pq.size() && current_episode > after) {
		epmem_time_id next_edge;
		epmem_time_id next_interval;

		bool changed_score = false;

		my_agent->epmem_timers->query_walk_edge->start();
		next_edge = edge_pq.top()->time;

		// process all edges which were last used at this time point
		while (edge_pq.size() && (edge_pq.top()->time == next_edge || edge_pq.top()->time >= current_episode)) {
			epmem_unique_edge_query* uedge = edge_pq.top();
			edge_pq.pop();
			epmem_node_id q1 = uedge->sql->column_int(1);

			if (JUSTIN_DEBUG) {
				std::cout << "	EDGE " << uedge->edge_info.q0 << "-" << uedge->edge_info.w << "-" << q1 << std::endl;
			}

			// create queries for the unique edge children of this unique edge
			if (uedge->is_edge_not_node) {
				for (epmem_literal_set::iterator literal_iter = uedge->literals.begin(); literal_iter != uedge->literals.end(); literal_iter++) {
					epmem_dnf_literal* literal = *literal_iter;
					for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++) {
						epmem_register_uedges(q1, *child_iter, edge_pq, current_score, current_cardinality, changed_score, after, uedge_cache, activated, my_agent);
					}
				}
			}

			// create interval queries for this unique edge
			{
				bool created = false;
				int64_t edge_id = uedge->sql->column_int(0);
				epmem_time_id promo_time = EPMEM_MEMID_NONE;
				bool is_lti = (uedge->is_edge_not_node && uedge->edge_info.q1 != EPMEM_NODEID_BAD && uedge->edge_info.q1 != EPMEM_NODEID_ROOT);
				if (is_lti) {
					// find the promotion time of the LTI and
					my_agent->epmem_stmts_graph->find_lti_current_time->bind_int(1, q1);
					my_agent->epmem_stmts_graph->find_lti_current_time->execute();
					promo_time = my_agent->epmem_stmts_graph->find_lti_current_time->column_int(0);
					my_agent->epmem_stmts_graph->find_lti_current_time->reinitialize();
				}
				for (int interval_type = EPMEM_RANGE_EP; interval_type <= EPMEM_RANGE_POINT; interval_type++) {
					for (int point_type = EPMEM_RANGE_START; point_type <= EPMEM_RANGE_END; point_type++) {
						// pick a timer (any timer)
						soar_module::timer* sql_timer = NULL;
						switch (interval_type) {
							case EPMEM_RANGE_EP:
								if (point_type == EPMEM_RANGE_START) {
									sql_timer = my_agent->epmem_timers->query_sql_start_ep;
								} else {
									sql_timer = my_agent->epmem_timers->query_sql_end_ep;
								}
								break;
							case EPMEM_RANGE_NOW:
								if (point_type == EPMEM_RANGE_START) {
									sql_timer = my_agent->epmem_timers->query_sql_start_now;
								} else {
									sql_timer = my_agent->epmem_timers->query_sql_end_now;
								}
								break;
							case EPMEM_RANGE_POINT:
								if (point_type == EPMEM_RANGE_START) {
									sql_timer = my_agent->epmem_timers->query_sql_start_point;
								} else {
									sql_timer = my_agent->epmem_timers->query_sql_end_point;
								}
								break;
						}
						// create the SQL query and bind it
						// try to find an existing query first; if none exist, allocate a new one from the memory pools
						soar_module::sqlite_statement* interval_sql;
						epmem_sql_list* sql_pool = NULL;
						if (is_lti) {
							sql_pool = &find_lti_pool[point_type][interval_type];
							if (sql_pool->size()) {
								interval_sql = sql_pool->front();
								sql_pool->pop_front();
							} else {
								allocate_with_pool(my_agent, &(my_agent->epmem_sql_pool), &interval_sql);
								new(interval_sql) soar_module::sqlite_statement(my_agent->epmem_db, epmem_find_lti_queries[point_type][interval_type], sql_timer);
								interval_sql->prepare();
							}
						} else {
							sql_pool = &find_interval_pool[uedge->is_edge_not_node][point_type][interval_type];
							if (sql_pool->size()) {
								interval_sql = sql_pool->front();
								sql_pool->pop_front();
							} else {
								allocate_with_pool(my_agent, &(my_agent->epmem_sql_pool), &interval_sql);
								new(interval_sql) soar_module::sqlite_statement(my_agent->epmem_db, epmem_find_interval_queries[uedge->is_edge_not_node][point_type][interval_type], sql_timer);
								interval_sql->prepare();
							}
						}
						int bind_pos = 1;
						if (point_type == EPMEM_RANGE_END && interval_type == EPMEM_RANGE_NOW) {
							interval_sql->bind_int(bind_pos++, current_episode);
						}
						interval_sql->bind_int(bind_pos++, edge_id);
						if (is_lti) {
							// find the promotion time of the LTI, and use that as an after constraint
							interval_sql->bind_int(bind_pos++, promo_time);
						}
						interval_sql->bind_int(bind_pos++, current_episode);
						if (interval_sql->execute() == soar_module::row) {
							epmem_interval_query* interval_q;
							allocate_with_pool(my_agent, &(my_agent->epmem_interval_pool), &interval_q);
							interval_q->is_end_point = point_type;
							interval_q->uedge = uedge;
							interval_q->q1 = q1;
							interval_q->time = interval_sql->column_int(0);
							interval_q->sql = interval_sql;
							interval_q->pool = sql_pool;
							interval_pq.push(interval_q);
							epmem_sql_edge triple = uedge->edge_info;
							triple.q1 = q1;
							interval_cleanup.insert(interval_q);
							created = true;
						} else {
							interval_sql->reinitialize();
							sql_pool->push_front(interval_sql);
						}
					}
				}
				if (created) {
					if (is_lti) {
						// insert a dummy promo time start for LTIs
						epmem_interval_query* start_q;
						allocate_with_pool(my_agent, &(my_agent->epmem_interval_pool), &start_q);
						start_q->q1 = q1;
						start_q->is_end_point = 0;
						start_q->uedge = uedge;
						start_q->time = promo_time - 1;
						start_q->sql = NULL;
						start_q->pool = NULL;
						interval_pq.push(start_q);
						interval_cleanup.insert(start_q);
					}
					epmem_sql_edge triple = uedge->edge_info;
					triple.q1 = q1;
					epmem_edge_sql_map::iterator uedge_iter = uedge_cache[uedge->is_edge_not_node].find(triple);
					if (uedge_iter == uedge_cache[uedge->is_edge_not_node].end()) {
						uedge_cache[uedge->is_edge_not_node][triple] = NULL;
					}
				}
			}

			// put the unique edge query back into the queue if there's more
			if (uedge->sql && uedge->sql->execute() == soar_module::row) {
				uedge->time = uedge->sql->column_int(2);
				edge_pq.push(uedge);
			}
		}
		next_edge = (edge_pq.empty() ? after : edge_pq.top()->time);
		my_agent->epmem_timers->query_walk_edge->stop();

		// process all intervals before the next edge arrives
		my_agent->epmem_timers->query_walk_interval->start();
		while (interval_pq.size() && interval_pq.top()->time > next_edge) {
			if (JUSTIN_DEBUG) {
				std::cout << "EPISODE " << current_episode << std::endl;
			}
			// process all interval endpoints at this time step
			while (interval_pq.size() && interval_pq.top()->time >= current_episode) {
				epmem_interval_query* interval = interval_pq.top();
				interval_pq.pop();
				epmem_unique_edge_query* uedge = interval->uedge;
				epmem_sql_edge info;
				info.q0 = uedge->edge_info.q0;
				info.w = uedge->edge_info.w;
				info.q1 = interval->q1;
				int is_edge = uedge->is_edge_not_node;

				if (JUSTIN_DEBUG) {
					std::cout << "	INTERVAL (" << (interval->is_end_point ? "end" : "start") << "): " << info.q0 << "-" << info.w << "-" << info.q1 << std::endl;
				}

				bool should_recurse;
				epmem_node_list queue;
#ifdef USE_MEM_POOL_ALLOCATORS
				epmem_node_set visited = epmem_node_set(std::less<epmem_node_id>(), soar_module::soar_memory_pool_allocator<epmem_node_id>(my_agent));
#else
				epmem_node_set visited;
#endif
				if (interval->is_end_point) {
					activated[is_edge].insert(info);
					for (epmem_literal_set::iterator lit_iter = uedge->literals.begin(); lit_iter != uedge->literals.end(); lit_iter++) {
						changed_score |= epmem_satisfy_literal(*lit_iter, info.q0, info.q1, current_score, current_cardinality, symbol_match_count, activated, symbol_incoming_count);
					}
				} else {
					activated[is_edge].erase(info);
					for (epmem_literal_set::iterator lit_iter = uedge->literals.begin(); lit_iter != uedge->literals.end(); lit_iter++) {
						changed_score |= epmem_unsatisfy_literal(*lit_iter, info.q0, info.q1, current_score, current_cardinality, symbol_match_count, symbol_incoming_count);
					}
				}
				// put the interval query back into the queue if there's more
				// otherwise, reinitialize the query and put it in a pool
				if (interval->sql && interval->sql->execute() == soar_module::row) {
					interval->time = interval->sql->column_int(0);
					interval_pq.push(interval);
				} else if (interval->sql && interval->pool) {
					// put it in the right pool depending on whether its an LTI
					interval->sql->reinitialize();
					interval->pool->push_front(interval->sql);
					interval->sql = NULL;
				}
			}
			next_interval = (interval_pq.empty() ? after : interval_pq.top()->time);
			next_episode = (next_edge > next_interval ? next_edge : next_interval);

			// update the prohibits list to catch up
			while (prohibits.size() && prohibits.back() > current_episode) {
				prohibits.pop_back();
			}
			// ignore the episode if it is prohibited
			while (prohibits.size() && current_episode > next_episode && current_episode == prohibits.back()) {
				current_episode--;
				prohibits.pop_back();
			}

			if (my_agent->sysparams[TRACE_EPMEM_SYSPARAM]) {
				char buf[256];
				SNPRINTF(buf, 254, "CONSIDERING EPISODE (time, cardinality, score) (%lld, %d, %f)\n", current_episode, current_cardinality, current_score);
				print(my_agent, buf);
				xml_generate_warning(my_agent, buf);
			}

			// if
			// * the current time is still before any new intervals
			// * and the score was changed in this period
			// * and the new score is higher than the best score
			// then save the current time as the best one
			if (current_episode > next_episode && changed_score && (best_episode == EPMEM_MEMID_NONE || current_score > best_score || (current_score == best_score && !best_graph_matched))) {
				if (best_episode == EPMEM_MEMID_NONE || current_score > best_score) {
					best_episode = current_episode;
				}
				best_score = current_score;
				best_cardinality = current_cardinality;
				// we should graph match if the option is set and all leaf literals are satisfied
				if (current_cardinality == perfect_cardinality) {
					bool graph_matched = false;
					if (do_graph_match) {
						epmem_param_container::gm_ordering_choices gm_order = my_agent->epmem_params->gm_ordering->get_value();
						if (gm_order == epmem_param_container::gm_order_undefined) {
							std::sort(gm_ordering.begin(), gm_ordering.end());
						} else if (gm_order == epmem_param_container::gm_order_mcv) {
							std::sort(gm_ordering.begin(), gm_ordering.end(), epmem_gm_mcv_comparator);
						}
						epmem_literal_deque::iterator begin = gm_ordering.begin();
						epmem_literal_deque::iterator end = gm_ordering.end();
						best_bindings.clear();
						epmem_node_symbol_map bound_nodes[2];
						if (JUSTIN_DEBUG) {
							std::cout << "	GRAPH MATCH" << std::endl;
						}
						my_agent->epmem_timers->query_graph_match->start();
						if (JUSTIN_DEBUG) {
							epmem_print_state(literal_cleanup, uedge_cache, interval_cleanup);
						}
						graph_matched = epmem_graph_match(begin, end, best_bindings, bound_nodes, my_agent, 2);
						my_agent->epmem_timers->query_graph_match->stop();
					}
					if (!do_graph_match || graph_matched) {
						best_episode = current_episode;
						best_graph_matched = true;
						current_episode = EPMEM_MEMID_NONE;
					}
				}
				if (my_agent->sysparams[TRACE_EPMEM_SYSPARAM]) {
					char buf[256];
					SNPRINTF(buf, 254, "NEW KING (perfect, graph-match): (%s, %s)\n", (current_cardinality == perfect_cardinality ? "true" : "false"), (best_graph_matched ? "true" : "false"));
					print(my_agent, buf);
					xml_generate_warning(my_agent, buf);
				}
			}

			if (current_episode == EPMEM_MEMID_NONE) {
				break;
			} else {
				current_episode = next_episode;
			}
		}
		my_agent->epmem_timers->query_walk_interval->stop();
	}
	my_agent->epmem_timers->query_walk->stop();

	// if the best episode is the default, fail
	// otherwise, put the episode in working memory
	if (best_episode == EPMEM_MEMID_NONE) {
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_failure, pos_query);
		if (neg_query) {
			epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_failure, neg_query);
		}
	} else {
		my_agent->epmem_timers->query_result->start();
		Symbol* temp_sym;
		epmem_id_mapping node_map_map;
		epmem_id_mapping node_mem_map;
		// cue size
		temp_sym = make_int_constant(my_agent, leaf_literals.size());
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_cue_size, temp_sym);
		symbol_remove_ref(my_agent, temp_sym);
		// match cardinality
		temp_sym = make_int_constant(my_agent, best_cardinality);
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_match_cardinality, temp_sym);
		symbol_remove_ref(my_agent, temp_sym);
		// match score
		temp_sym = make_float_constant(my_agent, best_score);
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_match_score, temp_sym);
		symbol_remove_ref(my_agent, temp_sym);
		// normalized match score
		temp_sym = make_float_constant(my_agent, best_score / perfect_score);
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_normalized_match_score, temp_sym);
		symbol_remove_ref(my_agent, temp_sym);
		// status
		epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_success, pos_query);
		if (neg_query) {
			epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_success, neg_query);
		}
		// give more metadata if graph match is turned on
		if (do_graph_match) {
			// graph match
			temp_sym = make_int_constant(my_agent, (best_graph_matched ? 1 : 0));
			epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_graph_match, temp_sym);
			symbol_remove_ref(my_agent, temp_sym);

			// mapping
			if (best_graph_matched) {
				goal_stack_level level = state->id.epmem_result_header->id.level;
				// mapping identifier
				Symbol* mapping = make_new_identifier(my_agent, 'M', level);
				epmem_buffer_add_wme(meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_graph_match_mapping, mapping);
				symbol_remove_ref(my_agent, mapping);

				for (epmem_literal_node_pair_map::iterator iter = best_bindings.begin(); iter != best_bindings.end(); iter++) {
					// create the node
					temp_sym = make_new_identifier(my_agent, 'N', level);
					epmem_buffer_add_wme(meta_wmes, mapping, my_agent->epmem_sym_graph_match_mapping_node, temp_sym);
					symbol_remove_ref(my_agent, temp_sym);
					// point to the cue identifier
					epmem_buffer_add_wme(meta_wmes, temp_sym, my_agent->epmem_sym_graph_match_mapping_cue, (*iter).first->value_sym);
					// save the mapping point for the episode
					node_map_map[(*iter).second.second] = temp_sym;
					node_mem_map[(*iter).second.second] = NULL;
				}
			}
		}
		// reconstruct the actual episode
		epmem_install_memory(my_agent, state, best_episode, meta_wmes, retrieval_wmes, &node_mem_map);
		if (best_graph_matched) {
			for (epmem_id_mapping::iterator iter = node_mem_map.begin(); iter != node_mem_map.end(); iter++) {
				epmem_id_mapping::iterator map_iter = node_map_map.find((*iter).first);
				if (map_iter != node_map_map.end() && (*iter).second) {
					epmem_buffer_add_wme(meta_wmes, (*map_iter).second, my_agent->epmem_sym_retrieved, (*iter).second);
				}
			}
		}
		my_agent->epmem_timers->query_result->stop();
	}

	// cleanup
	my_agent->epmem_timers->query_cleanup->start();
	for (epmem_interval_set::iterator iter = interval_cleanup.begin(); iter != interval_cleanup.end(); iter++) {
		epmem_interval_query* interval = *iter;
		if (interval->sql) {
			interval->sql->~sqlite_statement();
			free_with_pool(&(my_agent->epmem_sql_pool), interval->sql);
		}
		free_with_pool(&(my_agent->epmem_interval_pool), interval);
	}
	for (int type = EPMEM_TYPE_NODE; type <= EPMEM_TYPE_EDGE; type++) {
		for (epmem_edge_sql_map::iterator iter = uedge_cache[type].begin(); iter != uedge_cache[type].end(); iter++) {
			epmem_unique_edge_query* uedge = (*iter).second;
			if (uedge) {
				if (uedge->sql) {
					uedge->sql->~sqlite_statement();
					free_with_pool(&(my_agent->epmem_sql_pool), uedge->sql);
				}
				uedge->literals.~epmem_literal_set();
				free_with_pool(&(my_agent->epmem_uedge_pool), uedge);
			}
		}
	}
	for (epmem_literal_set::iterator iter = literal_cleanup.begin(); iter != literal_cleanup.end(); iter++) {
		epmem_dnf_literal* literal = *iter;
		delete literal;
	}
	my_agent->epmem_timers->query_cleanup->stop();

	my_agent->epmem_timers->query->stop();
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Visualization (epmem::viz)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline std::string _epmem_print_sti( epmem_node_id id )
{
	std::string t1, t2;

	t1.assign( "<id" );

	to_string( id, t2 );
	t1.append( t2 );
	t1.append( ">" );

	return t1;
}

void epmem_print_episode( agent* my_agent, epmem_time_id memory_id, std::string* buf )
{
	// if this is before the first episode, initialize db components
	if ( my_agent->epmem_db->get_status() == soar_module::disconnected )
	{
		epmem_init_db( my_agent );
	}

	// if bad memory, bail
	buf->clear();
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		return;
	}

	// fill episode map
	std::map< epmem_node_id, std::string > ltis;
	std::map< epmem_node_id, std::map< std::string, std::list< std::string > > > ep;
	{
		soar_module::sqlite_statement *my_q;
		std::string temp_s, temp_s2, temp_s3;
		double temp_d;
		int64_t temp_i;

		my_q = my_agent->epmem_stmts_graph->get_edges;
		{
			epmem_node_id q0;
			epmem_node_id q1;
			int64_t w_type;
			bool val_is_short_term;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

			// query for edges
			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			my_q->bind_int( 5, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				q0 = my_q->column_int( 0 );
				q1 = my_q->column_int( 2 );
				w_type = my_q->column_int( 3 );
				val_is_short_term = ( my_q->column_type( 4 ) == soar_module::null_t );

				switch ( w_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 1 ) );
						to_string( temp_i, temp_s );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 1 );
						to_string( temp_d, temp_s );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp_s.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 1 ) ) ) );
						break;
				}

				if ( val_is_short_term )
				{
					temp_s2 = _epmem_print_sti( q1 );
				}
				else
				{
					temp_s2.assign( "@" );
					temp_s2.push_back( static_cast< char >( my_q->column_int( 4 ) ) );

					temp_i = static_cast< uint64_t >( my_q->column_int( 5 ) );
					to_string( temp_i, temp_s3 );
					temp_s2.append( temp_s3 );

					ltis[ q1 ] = temp_s2;
				}

				ep[ q0 ][ temp_s ].push_back( temp_s2 );
			}
			my_q->reinitialize();
			epmem_rit_clear_left_right( my_agent );
		}

		my_q = my_agent->epmem_stmts_graph->get_nodes;
		{
			epmem_node_id parent_id;
			int64_t attr_type, value_type;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ] ) );

			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				parent_id = my_q->column_int( 1 );
				attr_type = my_q->column_int( 4 );
				value_type = my_q->column_int( 5 );

				switch ( attr_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 2 ) );
						to_string( temp_i, temp_s );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 2 );
						to_string( temp_d, temp_s );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp_s.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 2 ) ) ) );
						break;
				}

				switch ( value_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 3 ) );
						to_string( temp_i, temp_s2 );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 3 );
						to_string( temp_d, temp_s2 );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp_s2.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 3 ) ) ) );
						break;
				}

				ep[ parent_id ][ temp_s ].push_back( temp_s2 );
			}
			my_q->reinitialize();
			epmem_rit_clear_left_right( my_agent );
		}
	}

	// output
	{
		std::map< epmem_node_id, std::string >::iterator lti_it;
		std::map< epmem_node_id, std::map< std::string, std::list< std::string > > >::iterator ep_it;
		std::map< std::string, std::list< std::string > >::iterator slot_it;
		std::list< std::string >::iterator val_it;

		for ( ep_it=ep.begin(); ep_it!=ep.end(); ep_it++ )
		{
			buf->append( "(" );

			// id
			lti_it = ltis.find( ep_it->first );
			if ( lti_it == ltis.end() )
			{
				buf->append( _epmem_print_sti( ep_it->first ) );
			}
			else
			{
				buf->append( lti_it->second );
			}

			// attr
			for ( slot_it=ep_it->second.begin(); slot_it!=ep_it->second.end(); slot_it++ )
			{
				buf->append( " ^" );
				buf->append( slot_it->first );

				for ( val_it=slot_it->second.begin(); val_it!=slot_it->second.end(); val_it++ )
				{
					buf->append( " " );
					buf->append( *val_it );
				}
			}

			buf->append( ")\n" );
		}
	}
}

void epmem_visualize_episode( agent* my_agent, epmem_time_id memory_id, std::string* buf )
{
	// if this is before the first episode, initialize db components
	if ( my_agent->epmem_db->get_status() == soar_module::disconnected )
	{
		epmem_init_db( my_agent );
	}

	// if bad memory, bail
	buf->clear();
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		return;
	}

	// init
	{
		buf->append( "digraph epmem {\n" );
	}

	// taken heavily from install
	{
		soar_module::sqlite_statement *my_q;

		// first identifiers (i.e. reconstruct)
		my_q = my_agent->epmem_stmts_graph->get_edges;
		{
			// for printing
			std::map< epmem_node_id, std::string > stis;
			std::map< epmem_node_id, std::pair< std::string, std::string > > ltis;
			std::list< std::string > edges;
			std::map< epmem_node_id, std::string >::iterator sti_p;
			std::map< epmem_node_id, std::pair< std::string, std::string > >::iterator lti_p;

			// relates to finite automata: q1 = d(q0, w)
			epmem_node_id q0; // id
			epmem_node_id q1; // attribute
			int64_t w_type; // we support any constant attribute symbol
			std::string temp, temp2, temp3, temp4;
			double temp_d;
			int64_t temp_i;

			bool val_is_short_term;
			char val_letter;
			uint64_t val_num;

			// 0 is magic
			temp.assign( "ID_0" );
			stis.insert( std::make_pair< epmem_node_id, std::string >( 0, temp ) );

			// prep rit
			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, &( my_agent->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ] ) );

			// query for edges
			my_q->bind_int( 1, memory_id );
			my_q->bind_int( 2, memory_id );
			my_q->bind_int( 3, memory_id );
			my_q->bind_int( 4, memory_id );
			my_q->bind_int( 5, memory_id );
			while ( my_q->execute() == soar_module::row )
			{
				// q0, w, q1, w_type, letter, num
				q0 = my_q->column_int( 0 );
				q1 = my_q->column_int( 2 );
				w_type = my_q->column_int( 3 );

				// "ID_Q0"
				temp.assign( "ID_" );
				to_string( q0, temp2 );
				temp.append( temp2 );

				// "ID_Q1"
				temp3.assign( "ID_" );
				to_string( q1, temp2 );
				temp3.append( temp2 );

				val_is_short_term = ( my_q->column_type( 4 ) == soar_module::null_t );
				if ( val_is_short_term )
				{
					sti_p = stis.find( q1 );
					if ( sti_p == stis.end() )
					{
						stis.insert( std::make_pair< epmem_node_id, std::string >( q1, temp3 ) );
					}
				}
				else
				{
					lti_p = ltis.find( q1 );

					if ( lti_p == ltis.end() )
					{
						// "L#"
						val_letter = static_cast<char>( my_q->column_int( 4 ) );
						to_string( val_letter, temp4 );
						val_num = static_cast<uint64_t>( my_q->column_int( 5 ) );
						to_string( val_num, temp2 );
						temp4.append( temp2 );

						ltis.insert( std::make_pair< epmem_node_id, std::pair< std::string, std::string > >( q1, std::make_pair< std::string, std::string >( temp3, temp4 ) ) );
					}
				}

				// " -> ID_Q1"
				temp.append( " -> " );
				temp.append( temp3 );

				// " [ label="w" ];\n"
				temp.append( " [ label=\"" );
				switch ( w_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 1 ) );
						to_string( temp_i, temp2 );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 1 );
						to_string( temp_d, temp2 );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp2.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 1 ) ) ) );
						break;
				}
				temp.append( temp2 );
				temp.append( "\" ];\n" );

				edges.push_back( temp );
			}
			my_q->reinitialize();
			epmem_rit_clear_left_right( my_agent );

			// identifiers
			{
				// short-term
				{
					buf->append( "node [ shape = circle ];\n" );

					for ( sti_p=stis.begin(); sti_p!=stis.end(); sti_p++ )
					{
						buf->append( sti_p->second );
						buf->append( " " );
					}

					buf->append( ";\n" );
				}

				// long-term
				{
					buf->append( "node [ shape = doublecircle ];\n" );

					for ( lti_p=ltis.begin(); lti_p!=ltis.end(); lti_p++ )
					{
						buf->append( lti_p->second.first );
						buf->append( " [ label=\"" );
						buf->append( lti_p->second.second );
						buf->append( "\" ];\n" );
					}

					buf->append( "\n" );
				}
			}

			// edges
			{
				std::list< std::string >::iterator e_p;

				for ( e_p=edges.begin(); e_p!=edges.end(); e_p++ )
				{
					buf->append( (*e_p) );
				}
			}
		}

		// then node_unique
		my_q = my_agent->epmem_stmts_graph->get_nodes;
		{
			epmem_node_id child_id;
			epmem_node_id parent_id;
			int64_t attr_type;
			int64_t value_type;

			std::list< std::string > edges;
			std::list< std::string > consts;

			std::string temp, temp2;
			double temp_d;
			int64_t temp_i;

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

				temp.assign( "ID_" );
				to_string( parent_id, temp2 );
				temp.append( temp2 );
				temp.append( " -> C_" );
				to_string( child_id, temp2 );
				temp.append( temp2 );
				temp.append( " [ label=\"" );

				// make a symbol to represent the attribute
				switch ( attr_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 2 ) );
						to_string( temp_i, temp2 );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 2 );
						to_string( temp_d, temp2 );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp2.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 2 ) ) ) );
						break;
				}

				temp.append( temp2 );
				temp.append( "\" ];\n" );
				edges.push_back( temp );

				temp.assign( "C_" );
				to_string( child_id, temp2 );
				temp.append( temp2 );
				temp.append( " [ label=\"" );

				// make a symbol to represent the value
				switch ( value_type )
				{
					case INT_CONSTANT_SYMBOL_TYPE:
						temp_i = static_cast<int64_t>( my_q->column_int( 3 ) );
						to_string( temp_i, temp2 );
						break;

					case FLOAT_CONSTANT_SYMBOL_TYPE:
						temp_d = my_q->column_double( 3 );
						to_string( temp_d, temp2 );
						break;

					case SYM_CONSTANT_SYMBOL_TYPE:
						temp2.assign( const_cast<char *>( reinterpret_cast<const char *>( my_q->column_text( 3 ) ) ) );
						break;
				}

				temp.append( temp2 );
				temp.append( "\" ];\n" );

				consts.push_back( temp );

			}
			my_q->reinitialize();
			epmem_rit_clear_left_right( my_agent );

			// constant nodes
			{
				std::list< std::string >::iterator e_p;

				buf->append( "node [ shape = plaintext ];\n" );

				for ( e_p=consts.begin(); e_p!=consts.end(); e_p++ )
				{
					buf->append( (*e_p) );
				}
			}

			// edges
			{
				std::list< std::string >::iterator e_p;

				for ( e_p=edges.begin(); e_p!=edges.end(); e_p++ )
				{
					buf->append( (*e_p) );
				}
			}
		}
	}

	// close
	{
		buf->append( "\n}\n" );
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

	const int64_t force = my_agent->epmem_params->force->get_value();
	bool new_memory = false;

	if ( force == epmem_param_container::force_off )
	{
		const int64_t trigger = my_agent->epmem_params->trigger->get_value();

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
					if ( w->timetag > my_agent->top_goal->id.epmem_info->last_ol_time )
					{
						new_memory = true;
						my_agent->top_goal->id.epmem_info->last_ol_time = w->timetag;
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
	{
		epmem_new_episode( my_agent );
	}
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
	{
		epmem_init_db( my_agent );
	}

	// respond to query only if db is properly initialized
	if ( my_agent->epmem_db->get_status() != soar_module::connected )
	{
		return;
	}

	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	epmem_wme_list *wmes;
	epmem_wme_list *cmds;
	epmem_wme_list::iterator w_p;

	soar_module::wme_set cue_wmes;
	soar_module::symbol_triple_list meta_wmes;
	soar_module::symbol_triple_list retrieval_wmes;

	epmem_time_id retrieve;
	Symbol *next;
	Symbol *previous;
	Symbol *query;
	Symbol *neg_query;
	epmem_time_list prohibit;
	epmem_time_id before, after;
	Symbol *store;
	epmem_lti_map ltis;
	bool good_cue;
	int path;

	uint64_t wme_count;
	bool new_cue;

	bool do_wm_phase = false;

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
				// clear old results
				epmem_clear_result( my_agent, state );

				do_wm_phase = true;
			}
		}

		// a command is issued if the cue is new
		// and there is something on the cue
		if ( new_cue && wme_count )
		{
			cue_wmes.clear();
			retrieval_wmes.clear();
			meta_wmes.clear();

			// initialize command vars
			retrieve = EPMEM_MEMID_NONE;
			next = NULL;
			previous = NULL;
			query = NULL;
			neg_query = NULL;
			prohibit.clear();
			before = EPMEM_MEMID_NONE;
			after = EPMEM_MEMID_NONE;
			good_cue = true;
			store = NULL;
			path = 0;

			// process top-level symbols
			for ( w_p=cmds->begin(); w_p!=cmds->end(); w_p++ )
			{
				cue_wmes.insert( (*w_p) );

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
						{
							good_cue = false;
						}
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_next )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{
							next = (*w_p)->value;
							path = 2;
						}
						else
						{
							good_cue = false;
						}
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_prev )
					{
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == 0 ) )
						{
							previous = (*w_p)->value;
							path = 2;
						}
						else
						{
							good_cue = false;
						}
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
						{
							good_cue = false;
						}
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
						{
							good_cue = false;
						}
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
						{
							good_cue = false;
						}
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
						{
							good_cue = false;
						}
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_prohibit )
					{
						if ( ( (*w_p)->value->ic.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							prohibit.push_back( (*w_p)->value->ic.value );
							path = 3;
						}
						else
						{
							good_cue = false;
						}
					}
					else if ( (*w_p)->attr == my_agent->epmem_sym_store )
					{
						// error checking; command is only valid if the value is an LTI
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( (*w_p)->value->id.smem_lti ) &&
							 ( path == 0 ) )
						{
							store = (*w_p)->value;
							path = 4;
						}
					else
					{
						good_cue = false;
					}
				}
					else if ( (*w_p)->attr == my_agent->epmem_sym_lti )
					{
						// error checking; make sure the value is an identifier (since there's substructure
						if ( ( (*w_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == 0 ) || ( path == 3 ) ) )
						{
							// prevent infinite loops
							tc_number tc = get_new_tc_number( my_agent );

							// flags
							Symbol * lti = NULL;
							bool current = true;

							epmem_wme_list::iterator lti_p;
							epmem_wme_list* lti_wmes = epmem_get_augs_of_id( (*w_p)->value, tc );

							// error check one layer deeper
							// if the attribute is wme, check value is an identifier
							// if the attribute is current, check value is a symbol and is either yes or no
							// anything outside of these cases is a bad command
							if ( lti_wmes && ( lti_wmes->size() == 1 || lti_wmes->size() == 2 ) )
							{
								for ( lti_p=lti_wmes->begin(); lti_p!=lti_wmes->end(); lti_p++)
								{
									if ( (*lti_p)->attr == my_agent->epmem_sym_wme )
									{
										if ( (*lti_p)->value->id.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE )
										{
											lti = (*lti_p)->value;
										}
										else
										{
											good_cue = false;
										}

									}
									else if ( (*lti_p)->attr == my_agent->epmem_sym_current )
									{
										if ( (*lti_p)->value->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE )
										{
											if ( (*lti_p)->value == my_agent->epmem_sym_no )
											{
												current = false;
											}
											else if ( (*lti_p)->value == my_agent->epmem_sym_yes )
											{
												current = true;
											}
											else
											{
												good_cue = false;
											}
										}
										else
										{
											good_cue = false;
										}
									}
									else
									{
										good_cue = false;
									}
								}
							}
							else
							{
								good_cue = false;
							}

							// check than an LTI has been specified and has not already been specified
							if ( !lti || ltis.count(lti) > 0 )
							{
								good_cue = false;
							}

							// if the LTI is specified
							if ( good_cue )
							{
								ltis[lti] = current;
								path = 3;
							}

							delete lti_wmes;
						}
						else
						{
							good_cue = false;
						}
					}
					else
					{
						good_cue = false;
					}

				}
			}

			// if on path 3 must have query
			if ( ( path == 3 ) && ( query == NULL ) )
			{
				good_cue = false;
			}

			// must be on a path
			if ( path == 0 )
			{
				good_cue = false;
			}

			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////

			// process command
			if ( good_cue )
			{
				// retrieve
				if ( path == 1 )
				{
					epmem_install_memory( my_agent, state, retrieve, meta_wmes, retrieval_wmes );
				}
				// previous or next
				else if ( path == 2 )
				{
					if ( next )
					{
						epmem_install_memory( my_agent, state, epmem_next_episode( my_agent, state->id.epmem_info->last_memory ), meta_wmes, retrieval_wmes );

						// add one to the next stat
						my_agent->epmem_stats->nexts->set_value( my_agent->epmem_stats->nexts->get_value() + 1 );
					}
					else
					{
						epmem_install_memory( my_agent, state, epmem_previous_episode( my_agent, state->id.epmem_info->last_memory ), meta_wmes, retrieval_wmes );

						// add one to the prev stat
						my_agent->epmem_stats->prevs->set_value( my_agent->epmem_stats->prevs->get_value() + 1 );
					}
					
					if ( state->id.epmem_info->last_memory == EPMEM_MEMID_NONE )
					{
						epmem_buffer_add_wme( meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_failure, ( ( next )?( next ):( previous ) ) );
					}
					else
					{
						epmem_buffer_add_wme( meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_success, ( ( next )?( next ):( previous ) ) );
					}
				}
				// query
				else if ( path == 3 )
				{
					epmem_process_query( my_agent, state, query, neg_query, prohibit, before, after, ltis, cue_wmes, meta_wmes, retrieval_wmes );

					// add one to the cbr stat
					my_agent->epmem_stats->cbr->set_value( my_agent->epmem_stats->cbr->get_value() + 1 );
				}
			}
			else
			{
				epmem_buffer_add_wme( meta_wmes, state->id.epmem_result_header, my_agent->epmem_sym_status, my_agent->epmem_sym_bad_cmd );
			}

			// clear prohibit list
			prohibit.clear();

			if ( !retrieval_wmes.empty() || !meta_wmes.empty() )
			{
				// process preference assertion en masse
				epmem_process_buffered_wmes( my_agent, state, cue_wmes, meta_wmes, retrieval_wmes );

				// clear cache
				{
					soar_module::symbol_triple_list::iterator mw_it;

					for ( mw_it=retrieval_wmes.begin(); mw_it!=retrieval_wmes.end(); mw_it++ )
					{
						symbol_remove_ref( my_agent, (*mw_it)->id );
						symbol_remove_ref( my_agent, (*mw_it)->attr );
						symbol_remove_ref( my_agent, (*mw_it)->value );

						delete (*mw_it);
					}
					retrieval_wmes.clear();

					for ( mw_it=meta_wmes.begin(); mw_it!=meta_wmes.end(); mw_it++ )
					{
						symbol_remove_ref( my_agent, (*mw_it)->id );
						symbol_remove_ref( my_agent, (*mw_it)->attr );
						symbol_remove_ref( my_agent, (*mw_it)->value );

						delete (*mw_it);
					}
					meta_wmes.clear();
				}

				// process wm changes on this state
				do_wm_phase = true;
			}

			// clear cue wmes
			cue_wmes.clear();
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////
			my_agent->epmem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////
		}

		// free space from command aug list
		if ( cmds )
		{
			delete cmds;
		}

		state = state->id.higher_goal;
	}

	if ( do_wm_phase )
	{
		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->wm_phase->start();
		////////////////////////////////////////////////////////////////////////////

		do_working_memory_phase( my_agent );

		////////////////////////////////////////////////////////////////////////////
		my_agent->epmem_timers->wm_phase->stop();
		////////////////////////////////////////////////////////////////////////////
	}
}

/***************************************************************************
 * Function     : epmem_go
 * Author		: Nate Derbinsky
 * Notes		: The kernel calls this function to implement Soar-EpMem:
 * 				  consider new storage and respond to any commands
 **************************************************************************/
void epmem_go( agent *my_agent, bool allow_store=true )
{

	my_agent->epmem_timers->total->start();

#ifndef EPMEM_EXPERIMENT

	if ( allow_store )
	{
		epmem_consider_new_episode( my_agent );
	}
	epmem_respond_to_cmd( my_agent );

#else // EPMEM_EXPERIMENT

#endif // EPMEM_EXPERIMENT

	my_agent->epmem_timers->total->stop();

}

bool epmem_backup_db( agent* my_agent, const char* file_name, std::string *err )
{
	bool return_val = false;

	if ( my_agent->epmem_db->get_status() == soar_module::connected )
	{
		if ( my_agent->epmem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->epmem_stmts_common->commit->execute( soar_module::op_reinit );
		}

		return_val = my_agent->epmem_db->backup( file_name, err );

		if ( my_agent->epmem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->epmem_stmts_common->begin->execute( soar_module::op_reinit );
		}
	}
	else
	{
		err->assign( "Episodic database is not currently connected." );
	}

	return return_val;
}

