#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-SMem
 * =======================================================================
 */

#include "semantic_memory.h"

#include "agent.h"
#include "prefmem.h"
#include "symtab.h"
#include "wmem.h"
#include "print.h"
#include "xml.h"
#include "lexer.h"
#include "instantiations.h"
#include "rhsfun.h"
#include "decide.h"

#include <list>
#include <map>
#include <queue>
#include <utility>
#include <ctype.h>
#include <fstream>

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				smem::param
// stats 						smem::stats
// timers 						smem::timers
// statements					smem::statements

// wmes							smem::wmes

// variables					smem::var
// temporal hash				smem::hash
// long-term identifiers		smem::lti

// storage						smem::storage
// non-cue-based retrieval		smem::ncb
// cue-based retrieval			smem::cbr

// initialization				smem::init
// parsing						smem::parse
// api							smem::api

// visualization				smem::viz


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (smem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_param_container::smem_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
	// learning
	learning = new soar_module::boolean_param( "learning", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
	add( learning );

	// database
	database = new soar_module::constant_param<db_choices>( "database", memory, new smem_db_predicate<db_choices>( my_agent ) );
	database->add_mapping( memory, "memory" );
	database->add_mapping( file, "file" );
	add( database );

	// path
	path = new smem_path_param( "path", "", new soar_module::predicate<const char *>(), new smem_db_predicate<const char *>( my_agent ), my_agent );
	add( path );

	// auto-commit
	lazy_commit = new soar_module::boolean_param( "lazy-commit", soar_module::on, new smem_db_predicate<soar_module::boolean>( my_agent ) );
	add( lazy_commit );

	//

	// timers
	timers = new soar_module::constant_param<soar_module::timer::timer_level>( "timers", soar_module::timer::zero, new soar_module::f_predicate<soar_module::timer::timer_level>() );
	timers->add_mapping( soar_module::timer::zero, "off" );
	timers->add_mapping( soar_module::timer::one, "one" );
	timers->add_mapping( soar_module::timer::two, "two" );
	timers->add_mapping( soar_module::timer::three, "three" );
	add( timers );

	//

	// cache
	cache = new soar_module::constant_param<cache_choices>( "cache", cache_L, new smem_db_predicate<cache_choices>( my_agent ) );
	cache->add_mapping( cache_S, "small" );
	cache->add_mapping( cache_M, "medium" );
	cache->add_mapping( cache_L, "large" );
	add( cache );

	// opt
	opt = new soar_module::constant_param<opt_choices>( "optimization", opt_speed, new smem_db_predicate<opt_choices>( my_agent ) );
	opt->add_mapping( opt_safety, "safety" );
	opt->add_mapping( opt_speed, "performance" );	
	add( opt );

	// thresh
	thresh = new soar_module::integer_param( "thresh", 100, new soar_module::predicate<long>(), new smem_db_predicate<long>( my_agent ) );
	add( thresh );
}

//

smem_path_param::smem_path_param( const char *new_name, const char *new_value, soar_module::predicate<const char *> *new_val_pred, soar_module::predicate<const char *> *new_prot_pred, agent *new_agent ): soar_module::string_param( new_name, new_value, new_val_pred, new_prot_pred ), my_agent( new_agent ) {}

void smem_path_param::set_value( const char *new_value )
{
	if ( my_agent->smem_first_switch )
	{
		my_agent->smem_first_switch = false;
		my_agent->smem_params->database->set_value( smem_param_container::file );

		const char *msg = "Database set to file";
		print( my_agent, const_cast<char *>( msg ) );
		xml_generate_message( my_agent, const_cast<char *>( msg ) );
	}

	value->assign( new_value );
}

//

template <typename T>
smem_db_predicate<T>::smem_db_predicate( agent *new_agent ): soar_module::agent_predicate<T>( new_agent ) {}

template <typename T>
bool smem_db_predicate<T>::operator() ( T /*val*/ ) { return ( this->my_agent->smem_db->get_status() == soar_module::connected ); }


bool smem_enabled( agent *my_agent )
{
	return ( my_agent->smem_params->learning->get_value() == soar_module::on );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (smem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_stat_container::smem_stat_container( agent *new_agent ): soar_module::stat_container( new_agent )
{
	// mem-usage
	mem_usage = new smem_mem_usage_stat( my_agent, "mem-usage", 0, new soar_module::predicate<intptr_t>() );
	add( mem_usage );

	// mem-high
	mem_high = new smem_mem_high_stat( my_agent, "mem-high", 0, new soar_module::predicate<intptr_t>() );
	add( mem_high );

	//

	// expansions
	expansions = new soar_module::integer_stat( "retrieves", 0, new soar_module::f_predicate<long>() );
	add( expansions );

	// cue-based-retrievals
	cbr = new soar_module::integer_stat( "queries", 0, new soar_module::f_predicate<long>() );
	add( cbr );

	// stores
	stores = new soar_module::integer_stat( "stores", 0, new soar_module::f_predicate<long>() );
	add( stores );

	//

	// chunks
	chunks = new soar_module::intptr_stat( "nodes", 0, new soar_module::f_predicate<intptr_t>() );
	add( chunks );

	// slots
	slots = new soar_module::intptr_stat( "edges", 0, new soar_module::f_predicate<intptr_t>() );
	add( slots );
}

//

smem_mem_usage_stat::smem_mem_usage_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred ): soar_module::intptr_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

intptr_t smem_mem_usage_stat::get_value()
{
	return my_agent->smem_db->memory_usage();
}

//

smem_mem_high_stat::smem_mem_high_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred ): soar_module::intptr_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

intptr_t smem_mem_high_stat::get_value()
{
	return my_agent->smem_db->memory_highwater();
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (smem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_timer_container::smem_timer_container( agent *new_agent ): soar_module::timer_container( new_agent )
{
	// one

	total = new smem_timer( "_total", my_agent, soar_module::timer::one );
	add( total );

	// two

	storage = new smem_timer( "smem_storage", my_agent, soar_module::timer::two );
	add( storage );

	ncb_retrieval = new smem_timer( "smem_ncb_retrieval", my_agent, soar_module::timer::two );
	add( ncb_retrieval );

	query = new smem_timer( "smem_query", my_agent, soar_module::timer::two );
	add( query );

	api = new smem_timer( "smem_api", my_agent, soar_module::timer::two );
	add( api );

	init = new smem_timer( "smem_init", my_agent, soar_module::timer::two );
	add( init );

	hash = new smem_timer( "smem_hash", my_agent, soar_module::timer::two );
	add( hash );

	act = new smem_timer( "three_activation", my_agent, soar_module::timer::three );
	add( act );
}

//

smem_timer_level_predicate::smem_timer_level_predicate( agent *new_agent ): soar_module::agent_predicate<soar_module::timer::timer_level>( new_agent ) {}

bool smem_timer_level_predicate::operator() ( soar_module::timer::timer_level val ) { return ( my_agent->smem_params->timers->get_value() >= val ); }

//

smem_timer::smem_timer(const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level): soar_module::timer( new_name, new_agent, new_level, new smem_timer_level_predicate( new_agent ) ) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statement Functions (smem::statements)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_statement_container::smem_statement_container( agent *new_agent ): soar_module::sqlite_statement_container( new_agent->smem_db )
{
	soar_module::sqlite_database *new_db = new_agent->smem_db;

	//

	add_structure( "CREATE TABLE " SMEM_SCHEMA "vars (id INTEGER PRIMARY KEY,value INTEGER)" );
	
	add_structure( "CREATE TABLE " SMEM_SCHEMA "symbols_type (id INTEGER PRIMARY KEY, sym_type INTEGER)" );	
	add_structure( "CREATE TABLE " SMEM_SCHEMA "symbols_int (id INTEGER PRIMARY KEY, sym_const INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "symbols_int_const ON " SMEM_SCHEMA "symbols_int (sym_const)" );
	add_structure( "CREATE TABLE " SMEM_SCHEMA "symbols_float (id INTEGER PRIMARY KEY, sym_const REAL)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "symbols_float_const ON " SMEM_SCHEMA "symbols_float (sym_const)" );
	add_structure( "CREATE TABLE " SMEM_SCHEMA "symbols_str (id INTEGER PRIMARY KEY, sym_const TEXT)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "symbols_str_const ON " SMEM_SCHEMA "symbols_str (sym_const)" );	

	add_structure( "CREATE TABLE " SMEM_SCHEMA "lti (id INTEGER PRIMARY KEY, letter INTEGER, num INTEGER, child_ct INTEGER, act_cycle INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "lti_letter_num ON " SMEM_SCHEMA "lti (letter, num)" );

	add_structure( "CREATE TABLE " SMEM_SCHEMA "web (parent_id INTEGER, attr INTEGER, val_const INTEGER, val_lti INTEGER, act_cycle INTEGER)" );
	add_structure( "CREATE INDEX " SMEM_SCHEMA "web_parent_attr_val_lti ON " SMEM_SCHEMA "web (parent_id, attr, val_const, val_lti)" );
	add_structure( "CREATE INDEX " SMEM_SCHEMA "web_attr_val_lti_cycle ON " SMEM_SCHEMA "web (attr, val_const, val_lti, act_cycle)" );
	add_structure( "CREATE INDEX " SMEM_SCHEMA "web_attr_cycle ON " SMEM_SCHEMA "web (attr, act_cycle)" );

	add_structure( "CREATE TABLE " SMEM_SCHEMA "ct_attr (attr INTEGER PRIMARY KEY, ct INTEGER)" );

	add_structure( "CREATE TABLE " SMEM_SCHEMA "ct_const (attr INTEGER, val_const INTEGER, ct INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "ct_const_attr_val ON " SMEM_SCHEMA "ct_const (attr, val_const)" );

	add_structure( "CREATE TABLE " SMEM_SCHEMA "ct_lti (attr INTEGER, val_lti INTEGER, ct INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX " SMEM_SCHEMA "ct_lti_attr_val ON " SMEM_SCHEMA "ct_lti (attr, val_lti)" );	

	// adding an ascii table just to make lti queries easier when inspecting database
	add_structure( "CREATE TABLE " SMEM_SCHEMA "ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)" );
	add_structure( "DELETE FROM " SMEM_SCHEMA "ascii" );
	{
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (65,'A')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (66,'B')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (67,'C')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (68,'D')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (69,'E')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (70,'F')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (71,'G')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (72,'H')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (73,'I')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (74,'J')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (75,'K')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (76,'L')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (77,'M')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (78,'N')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (79,'O')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (80,'P')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (81,'Q')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (82,'R')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (83,'S')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (84,'T')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (85,'U')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (86,'V')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (87,'W')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (88,'X')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (89,'Y')" );
		add_structure( "INSERT INTO " SMEM_SCHEMA "ascii (ascii_num, ascii_chr) VALUES (90,'Z')" );
	}

	//

	begin = new soar_module::sqlite_statement( new_db, "BEGIN" );
	add( begin );

	commit = new soar_module::sqlite_statement( new_db, "COMMIT" );
	add( commit );

	rollback = new soar_module::sqlite_statement( new_db, "ROLLBACK" );
	add( rollback );

	//

	var_get = new soar_module::sqlite_statement( new_db, "SELECT value FROM " SMEM_SCHEMA "vars WHERE id=?" );
	add( var_get );

	var_set = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "vars SET value=? WHERE id=?" );
	add( var_set );

	var_create = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "vars (id,value) VALUES (?,?)" );
	add( var_create );

	//

	hash_rev_int = new soar_module::sqlite_statement( new_db, "SELECT sym_const FROM " SMEM_SCHEMA "symbols_int WHERE id=?" );
	add( hash_rev_int );

	hash_rev_float = new soar_module::sqlite_statement( new_db, "SELECT sym_const FROM " SMEM_SCHEMA "symbols_float WHERE id=?" );
	add( hash_rev_float );

	hash_rev_str = new soar_module::sqlite_statement( new_db, "SELECT sym_const FROM " SMEM_SCHEMA "symbols_str WHERE id=?" );
	add( hash_rev_str );
	
	hash_get_int = new soar_module::sqlite_statement( new_db, "SELECT id FROM " SMEM_SCHEMA "symbols_int WHERE sym_const=?" );
	add( hash_get_int );

	hash_get_float = new soar_module::sqlite_statement( new_db, "SELECT id FROM " SMEM_SCHEMA "symbols_float WHERE sym_const=?" );
	add( hash_get_float );

	hash_get_str = new soar_module::sqlite_statement( new_db, "SELECT id FROM " SMEM_SCHEMA "symbols_str WHERE sym_const=?" );
	add( hash_get_str );

	hash_add_type = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "symbols_type (sym_type) VALUES (?)" );
	add( hash_add_type );

	hash_add_int = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "symbols_int (id,sym_const) VALUES (?,?)" );
	add( hash_add_int );

	hash_add_float = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "symbols_float (id,sym_const) VALUES (?,?)" );
	add( hash_add_float );

	hash_add_str = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "symbols_str (id,sym_const) VALUES (?,?)" );
	add( hash_add_str );

	//

	lti_add = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "lti (letter,num,child_ct,act_cycle) VALUES (?,?,?,?)" );
	add( lti_add );

	lti_get = new soar_module::sqlite_statement( new_db, "SELECT id FROM " SMEM_SCHEMA "lti WHERE letter=? AND num=?" );
	add( lti_get );

	lti_letter_num = new soar_module::sqlite_statement( new_db, "SELECT letter, num FROM " SMEM_SCHEMA "lti WHERE id=?" );
	add( lti_letter_num );

	lti_max = new soar_module::sqlite_statement( new_db, "SELECT letter, MAX(num) FROM " SMEM_SCHEMA "lti GROUP BY letter" );
	add( lti_max );

	//

	web_add = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "web (parent_id, attr, val_const, val_lti, act_cycle) VALUES (?,?,?,?,?)" );
	add( web_add );

	web_truncate = new soar_module::sqlite_statement( new_db, "DELETE FROM " SMEM_SCHEMA "web WHERE parent_id=?" );
	add( web_truncate );

	web_expand = new soar_module::sqlite_statement( new_db, "SELECT tsh_a.sym_type AS attr_type, tsh_a.id AS attr_hash, vcl.sym_type AS value_type, vcl.id AS value_hash, vcl.letter AS value_letter, vcl.num AS value_num, vcl.val_lti AS value_lti FROM ((" SMEM_SCHEMA "web w LEFT JOIN " SMEM_SCHEMA "symbols_type tsh_v ON w.val_const=tsh_v.id) vc LEFT JOIN " SMEM_SCHEMA "lti AS lti ON vc.val_lti=lti.id) vcl INNER JOIN " SMEM_SCHEMA "symbols_type tsh_a ON vcl.attr=tsh_a.id WHERE parent_id=?" );
	add( web_expand );

	//

	web_attr_ct = new soar_module::sqlite_statement( new_db, "SELECT attr, COUNT(*) AS ct FROM " SMEM_SCHEMA "web WHERE parent_id=? GROUP BY attr" );
	add( web_attr_ct );

	web_const_ct = new soar_module::sqlite_statement( new_db, "SELECT attr, val_const, COUNT(*) AS ct FROM " SMEM_SCHEMA "web WHERE parent_id=? AND val_const IS NOT NULL GROUP BY attr, val_const" );
	add( web_const_ct );

	web_lti_ct = new soar_module::sqlite_statement( new_db, "SELECT attr, val_lti, COUNT(*) AS ct FROM " SMEM_SCHEMA "web WHERE parent_id=? AND val_const IS NULL GROUP BY attr, val_const, val_lti" );
	add( web_lti_ct );

	//

	web_attr_all = new soar_module::sqlite_statement( new_db, "SELECT parent_id, act_cycle FROM " SMEM_SCHEMA "web w WHERE attr=? ORDER BY act_cycle DESC" );
	add( web_attr_all );

	web_const_all = new soar_module::sqlite_statement( new_db, "SELECT parent_id, act_cycle FROM " SMEM_SCHEMA "web w WHERE attr=? AND val_const=? AND val_lti IS NULL ORDER BY act_cycle DESC" );
	add( web_const_all );

	web_lti_all = new soar_module::sqlite_statement( new_db, "SELECT parent_id, act_cycle FROM " SMEM_SCHEMA "web w WHERE attr=? AND val_const IS NULL AND val_lti=? ORDER BY act_cycle DESC" );
	add( web_lti_all );

	//

	web_attr_child = new soar_module::sqlite_statement( new_db, "SELECT parent_id FROM " SMEM_SCHEMA "web WHERE parent_id=? AND attr=?" );
	add( web_attr_child );

	web_const_child = new soar_module::sqlite_statement( new_db, "SELECT parent_id FROM " SMEM_SCHEMA "web WHERE parent_id=? AND attr=? AND val_const=?" );
	add( web_const_child );

	web_lti_child = new soar_module::sqlite_statement( new_db, "SELECT parent_id FROM " SMEM_SCHEMA "web WHERE parent_id=? AND attr=? AND val_const IS NULL AND val_lti=?" );
	add( web_lti_child );

	//

	ct_attr_check = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_attr WHERE attr=?" );
	add( ct_attr_check );

	ct_const_check = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_const WHERE attr=? AND val_const=?" );
	add( ct_const_check );

	ct_lti_check = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_lti WHERE attr=? AND val_lti=?" );
	add( ct_lti_check );

	//

	ct_attr_add = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "ct_attr (attr, ct) VALUES (?,0)" );
	add( ct_attr_add );

	ct_const_add = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "ct_const (attr, val_const, ct) VALUES (?,?,0)" );
	add( ct_const_add );

	ct_lti_add = new soar_module::sqlite_statement( new_db, "INSERT INTO " SMEM_SCHEMA "ct_lti (attr, val_lti, ct) VALUES (?,?,0)" );
	add( ct_lti_add );

	//

	ct_attr_update = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "ct_attr SET ct = ct + ? WHERE attr=?" );
	add( ct_attr_update );

	ct_const_update = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "ct_const SET ct = ct + ? WHERE attr=? AND val_const=?" );
	add( ct_const_update );

	ct_lti_update = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "ct_lti SET ct = ct + ? WHERE attr=? AND val_lti=?" );
	add( ct_lti_update );

	//

	ct_attr_get = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_attr WHERE attr=?" );
	add( ct_attr_get );

	ct_const_get = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_const WHERE attr=? AND val_const=?" );
	add( ct_const_get );

	ct_lti_get = new soar_module::sqlite_statement( new_db, "SELECT ct FROM " SMEM_SCHEMA "ct_lti WHERE attr=? AND val_lti=?" );
	add( ct_lti_get );

	//

	act_set = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "web SET act_cycle=? WHERE parent_id=?" );
	add( act_set );

	act_lti_child_ct_get = new soar_module::sqlite_statement( new_db, "SELECT child_ct FROM " SMEM_SCHEMA "lti WHERE id=?" );
	add( act_lti_child_ct_get );

	act_lti_child_ct_set = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "lti SET child_ct=? WHERE id=?" );
	add( act_lti_child_ct_set );

	act_lti_set = new soar_module::sqlite_statement( new_db, "UPDATE " SMEM_SCHEMA "lti SET act_cycle=? WHERE id=?" );
	add( act_lti_set );

	act_lti_get = new soar_module::sqlite_statement( new_db, "SELECT act_cycle FROM " SMEM_SCHEMA "lti WHERE id=?" );
	add( act_lti_get );

	//

	vis_lti = new soar_module::sqlite_statement( new_db, "SELECT id, letter, num FROM " SMEM_SCHEMA "lti" );
	add( vis_lti );

	vis_value_const = new soar_module::sqlite_statement( new_db, "SELECT parent_id, tsh1.sym_type AS attr_type, tsh1.id AS attr_hash, tsh2.sym_type AS val_type, tsh2.id AS val_hash FROM " SMEM_SCHEMA "web w, " SMEM_SCHEMA "symbols_type tsh1, " SMEM_SCHEMA "symbols_type tsh2 WHERE (w.attr=tsh1.id) AND (w.val_const=tsh2.id)" );
	add( vis_value_const );

	vis_value_lti = new soar_module::sqlite_statement( new_db, "SELECT parent_id, tsh.sym_type AS attr_type, tsh.id AS attr_hash, val_lti FROM " SMEM_SCHEMA "web w, " SMEM_SCHEMA "symbols_type tsh WHERE (w.attr=tsh.id) AND (val_lti IS NOT NULL)" );
	add( vis_value_lti );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (smem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_wme_list *smem_get_direct_augs_of_id( Symbol * id, tc_number tc = NIL )
{
	slot *s;
	wme *w;
	smem_wme_list *return_val = new smem_wme_list;

	// augs only exist for identifiers
	if ( id->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
	{
		if ( tc != NIL )
		{
			if ( tc == id->id.tc_num )
			{
				return return_val;
			}
			else
			{
				id->id.tc_num = tc;
			}
		}

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

inline bool smem_symbol_is_constant( Symbol *sym )
{
	return ( ( sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) ||
		     ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) ||
		     ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) );
}

//

void _smem_add_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value, bool meta )
{
	// this fake preference is just for this state.
	// it serves the purpose of simulating a completely
	// local production firing to provide backtracing
	// information, making the result wmes dependent
	// upon the cue wmes.
	preference *pref = soar_module::make_fake_preference( my_agent, state, id, attr, value, state->id.smem_info->cue_wmes );

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
		state->id.smem_info->smem_wmes->push( pref );
	}
	else
	{
		// otherwise, we submit the fake instantiation to backtracing
		// such as to potentially produce justifications that can follow
		// it to future adventures (potentially on new states)

		instantiation *my_justification_list = NIL;
		chunk_instantiation( my_agent, pref->inst, false, &my_justification_list );

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

void smem_add_retrieved_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value )
{
	_smem_add_wme( my_agent, state, id, attr, value, false );
}

void smem_add_meta_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value )
{
	_smem_add_wme( my_agent, state, id, attr, value, true );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Variable Functions (smem::var)
//
// Variables are key-value pairs stored in the database
// that are necessary to maintain a store between
// multiple runs of Soar.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// gets an SMem variable from the database
inline bool smem_variable_get( agent *my_agent, smem_variable_key variable_id, intptr_t *variable_value )
{
	soar_module::exec_result status;
	soar_module::sqlite_statement *var_get = my_agent->smem_stmts->var_get;

	var_get->bind_int( 1, variable_id );
	status = var_get->execute();

	if ( status == soar_module::row )
	{
		(*variable_value) = var_get->column_int( 0 );
	}

	var_get->reinitialize();

	return ( status == soar_module::row );
}

// sets an existing SMem variable in the database
inline void smem_variable_set( agent *my_agent, smem_variable_key variable_id, intptr_t variable_value )
{
	soar_module::sqlite_statement *var_set = my_agent->smem_stmts->var_set;
	
	var_set->bind_int( 1, variable_value );
	var_set->bind_int( 2, variable_id );

	var_set->execute( soar_module::op_reinit );
}

// creates a new SMem variable in the database
inline void smem_variable_create( agent *my_agent, smem_variable_key variable_id, intptr_t variable_value )
{
	soar_module::sqlite_statement *var_create = my_agent->smem_stmts->var_create;
	
	var_create->bind_int( 1, variable_id );
	var_create->bind_int( 2, variable_value );	

	var_create->execute( soar_module::op_reinit );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Temporal Hash Functions (smem::hash)
//
// The rete has symbol hashing, but the values are
// reliable only for the lifetime of a symbol.  This
// isn't good for SMem.  Hence, we implement a simple
// lookup table.
//
// Note the hashing functions for the symbol types are
// very similar, but with enough differences that I
// separated them out for clarity.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline smem_hash_id smem_temporal_hash_add( agent* my_agent, byte sym_type )
{
	my_agent->smem_stmts->hash_add_type->bind_int( 1, sym_type );
	my_agent->smem_stmts->hash_add_type->execute( soar_module::op_reinit );
	return static_cast<smem_hash_id>( my_agent->smem_db->last_insert_rowid() );
}

inline smem_hash_id smem_temporal_hash_int( agent *my_agent, intptr_t val, bool add_on_fail = true )
{
	smem_hash_id return_val = NIL;
	
	// search first
	my_agent->smem_stmts->hash_get_int->bind_int( 1, val );
	if ( my_agent->smem_stmts->hash_get_int->execute() == soar_module::row )
	{
		return_val = static_cast<smem_hash_id>( my_agent->smem_stmts->hash_get_int->column_int( 0 ) );
	}
	my_agent->smem_stmts->hash_get_int->reinitialize();

	// if fail and supposed to add
	if ( !return_val && add_on_fail )
	{
		// type first		
		return_val = smem_temporal_hash_add( my_agent, INT_CONSTANT_SYMBOL_TYPE );

		// then content
		my_agent->smem_stmts->hash_add_int->bind_int( 1, return_val );
		my_agent->smem_stmts->hash_add_int->bind_int( 2, val );
		my_agent->smem_stmts->hash_add_int->execute( soar_module::op_reinit );
	}

	return return_val;
}

inline smem_hash_id smem_temporal_hash_float( agent *my_agent, double val, bool add_on_fail = true )
{
	smem_hash_id return_val = NIL;
	
	// search first
	my_agent->smem_stmts->hash_get_float->bind_double( 1, val );
	if ( my_agent->smem_stmts->hash_get_float->execute() == soar_module::row )
	{
		return_val = static_cast<smem_hash_id>( my_agent->smem_stmts->hash_get_float->column_int( 0 ) );
	}
	my_agent->smem_stmts->hash_get_float->reinitialize();

	// if fail and supposed to add
	if ( !return_val && add_on_fail )
	{
		// type first		
		return_val = smem_temporal_hash_add( my_agent, FLOAT_CONSTANT_SYMBOL_TYPE );

		// then content
		my_agent->smem_stmts->hash_add_float->bind_int( 1, return_val );
		my_agent->smem_stmts->hash_add_float->bind_double( 2, val );
		my_agent->smem_stmts->hash_add_float->execute( soar_module::op_reinit );
	}

	return return_val;
}

inline smem_hash_id smem_temporal_hash_str( agent *my_agent, char* val, bool add_on_fail = true )
{
	smem_hash_id return_val = NIL;
	
	// search first
	my_agent->smem_stmts->hash_get_str->bind_text( 1, static_cast<const char *>( val ) );
	if ( my_agent->smem_stmts->hash_get_str->execute() == soar_module::row )
	{
		return_val = static_cast<smem_hash_id>( my_agent->smem_stmts->hash_get_str->column_int( 0 ) );
	}
	my_agent->smem_stmts->hash_get_str->reinitialize();

	// if fail and supposed to add
	if ( !return_val && add_on_fail )
	{
		// type first		
		return_val = smem_temporal_hash_add( my_agent, SYM_CONSTANT_SYMBOL_TYPE );

		// then content
		my_agent->smem_stmts->hash_add_str->bind_int( 1, return_val );
		my_agent->smem_stmts->hash_add_str->bind_text( 2, static_cast<const char*>( val ) );
		my_agent->smem_stmts->hash_add_str->execute( soar_module::op_reinit );
	}

	return return_val;
}

// returns a temporally unique integer representing a symbol constant
smem_hash_id smem_temporal_hash( agent *my_agent, Symbol *sym, bool add_on_fail = true )
{
	smem_hash_id return_val = NIL;

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->hash->start();
	////////////////////////////////////////////////////////////////////////////

	if ( smem_symbol_is_constant( sym ) )
	{
		if ( ( !sym->common.smem_hash ) || ( sym->common.smem_valid != my_agent->smem_validation ) )
		{
			sym->common.smem_hash = NIL;
			sym->common.smem_valid = my_agent->smem_validation;

			switch ( sym->common.symbol_type )
			{
				case SYM_CONSTANT_SYMBOL_TYPE:
					return_val = smem_temporal_hash_str( my_agent, sym->sc.name, add_on_fail );
					break;

				case INT_CONSTANT_SYMBOL_TYPE:
					return_val = smem_temporal_hash_int( my_agent, sym->ic.value, add_on_fail );
					break;

				case FLOAT_CONSTANT_SYMBOL_TYPE:
					return_val = smem_temporal_hash_float( my_agent, sym->fc.value, add_on_fail );
					break;
			}

			// cache results for later re-use
			sym->common.smem_hash = return_val;
			sym->common.smem_valid = my_agent->smem_validation;
		}

		return_val = sym->common.smem_hash;
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->hash->stop();
	////////////////////////////////////////////////////////////////////////////

	return return_val;
}

inline intptr_t smem_reverse_hash_int( agent* my_agent, smem_hash_id hash_value )
{
	intptr_t return_val = NIL;
	
	my_agent->smem_stmts->hash_rev_int->bind_int( 1, hash_value );
	soar_module::exec_result res = my_agent->smem_stmts->hash_rev_int->execute();
	assert( res == soar_module::row );
	return_val = my_agent->smem_stmts->hash_rev_int->column_int(0);
	my_agent->smem_stmts->hash_rev_int->reinitialize();

	return return_val;
}

inline double smem_reverse_hash_float( agent* my_agent, smem_hash_id hash_value )
{
	double return_val = NIL;
	
	my_agent->smem_stmts->hash_rev_float->bind_int( 1, hash_value );
	soar_module::exec_result res = my_agent->smem_stmts->hash_rev_float->execute();
	assert( res == soar_module::row );
	return_val = my_agent->smem_stmts->hash_rev_float->column_double(0);
	my_agent->smem_stmts->hash_rev_float->reinitialize();

	return return_val;
}

inline void smem_reverse_hash_str( agent* my_agent, smem_hash_id hash_value, std::string& dest )
{
	my_agent->smem_stmts->hash_rev_str->bind_int( 1, hash_value );
	soar_module::exec_result res = my_agent->smem_stmts->hash_rev_str->execute();
	assert( res == soar_module::row );
	dest.assign( my_agent->smem_stmts->hash_rev_str->column_text(0) );
	my_agent->smem_stmts->hash_rev_str->reinitialize();
}

inline Symbol* smem_reverse_hash( agent* my_agent, byte sym_type, smem_hash_id hash_value )
{
	Symbol *return_val = NULL;
	std::string dest;

	switch ( sym_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:			
			smem_reverse_hash_str( my_agent, hash_value, dest );
			return_val = make_sym_constant( my_agent, const_cast<char *>( dest.c_str() ) );
			break;

		case INT_CONSTANT_SYMBOL_TYPE:
			return_val = make_int_constant( my_agent, static_cast<long>( smem_reverse_hash_int( my_agent, hash_value ) ) );
			break;

		case FLOAT_CONSTANT_SYMBOL_TYPE:
			return_val = make_float_constant( my_agent, smem_reverse_hash_float( my_agent, hash_value ) );
			break;

		default:
			return_val = NULL;
			break;
	}

	return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Long-Term Identifier Functions (smem::lti)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// copied primarily from add_bound_variables_in_test
void _smem_lti_from_test( test t, std::set<Symbol *> *valid_ltis )
{
	if ( test_is_blank_test(t) ) return;
	
	if ( test_is_blank_or_equality_test(t) )
	{
		Symbol *referent = referent_of_equality_test(t);
		if ( ( referent->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) && ( referent->id.smem_lti != NIL ) )
		{
			valid_ltis->insert( referent );
		}
      
		return;
	}

	{		
		complex_test *ct = complex_test_from_test(t);

		if ( ct->type==CONJUNCTIVE_TEST ) 
		{
			for ( cons *c=ct->data.conjunct_list; c!=NIL; c=c->rest )
			{
				_smem_lti_from_test( static_cast<test>( c->first ), valid_ltis );
			}				
		}
	}
}

// copied primarily from add_all_variables_in_rhs_value
void _smem_lti_from_rhs_value( rhs_value rv, std::set<Symbol *> *valid_ltis )
{
	if ( rhs_value_is_symbol( rv ) )
	{
		Symbol *sym = rhs_value_to_symbol( rv );
		if ( ( sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) && ( sym->id.smem_lti != NIL ) )
		{
			valid_ltis->insert( sym );
		}
	}
	else
	{
		list *fl = rhs_value_to_funcall_list( rv );
		for ( cons *c=fl->rest; c!=NIL; c=c->rest )
		{
			_smem_lti_from_rhs_value( static_cast<rhs_value>( c->first ), valid_ltis );
		}
	}
}

// make sure ltis in actions are grounded
bool smem_valid_production( condition *lhs_top, action *rhs_top )
{
	bool return_val = true;
	
	std::set<Symbol *> valid_ltis;
	std::set<Symbol *>::iterator lti_p;
	
	// collect valid ltis
	for ( condition *c=lhs_top; c!=NIL; c=c->next )
	{
		if ( c->type == POSITIVE_CONDITION )
		{
			_smem_lti_from_test( c->data.tests.attr_test, &valid_ltis );
			_smem_lti_from_test( c->data.tests.value_test, &valid_ltis );
		}
	}

	// validate ltis in actions
	// copied primarily from add_all_variables_in_action
	{
		Symbol *id;
		action *a;		
		int action_counter = 0;

		for ( a=rhs_top; a!=NIL; a=a->next )
		{		
			a->already_in_tc = false;
			action_counter++;
		}

		// good_pass detects infinite loops
		bool good_pass = true;
		bool good_action = true;
		while ( good_pass && action_counter )
		{
			good_pass = false;
			
			for ( a=rhs_top; a!=NIL; a=a->next )
			{
				if ( !a->already_in_tc )
				{
					good_action = false;
					
					if ( a->type == MAKE_ACTION )
					{
						id = rhs_value_to_symbol( a->id );

						// non-identifiers are ok
						if ( id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{
							good_action = true;
						}
						// short-term identifiers are ok
						else if ( id->id.smem_lti == NIL )
						{
							good_action = true;
						}
						// valid long-term identifiers are ok
						else if ( valid_ltis.find( id ) != valid_ltis.end() )
						{
							good_action = true;
						}
					}
					else
					{						
						good_action = true;
					}

					// we've found a new good action
					// mark as good, collect all goodies
					if ( good_action )
					{
						a->already_in_tc = true;

						// everyone has values
						_smem_lti_from_rhs_value( a->value, &valid_ltis );
						
						// function calls don't have attributes
						if ( a->type == MAKE_ACTION )
						{
							_smem_lti_from_rhs_value( a->attr, &valid_ltis );
						}

						// note that we've dealt with another action
						action_counter--;
						good_pass = true;
					}
				}
			}
		};

		return_val = ( action_counter == 0 );
	}

	return return_val;
}

// instance of hash_table_callback_fn2
Bool smem_count_ltis( agent * /*my_agent*/, void *item, void *userdata )
{
	Symbol *id = static_cast<symbol_union *>(item);

	if ( id->id.smem_lti != NIL )
	{
		unsigned long *counter = reinterpret_cast<unsigned long *>( userdata );
		(*counter)++;
	}

	return false;
}


// activates a new or existing long-term identifier
inline void smem_lti_activate( agent *my_agent, smem_lti_id lti )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->act->start();
	////////////////////////////////////////////////////////////////////////////

	my_agent->smem_stmts->act_lti_child_ct_get->bind_int( 1, lti );
	my_agent->smem_stmts->act_lti_child_ct_get->execute();

	if ( my_agent->smem_stmts->act_lti_child_ct_get->column_int( 0 ) >= my_agent->smem_params->thresh->get_value() )
	{
		// cycle=? WHERE lti=?
		my_agent->smem_stmts->act_lti_set->bind_int( 1, ( my_agent->smem_max_cycle++ ) );
		my_agent->smem_stmts->act_lti_set->bind_int( 2, lti );
		my_agent->smem_stmts->act_lti_set->execute( soar_module::op_reinit );
	}
	else
	{
		// cycle=? WHERE lti=?
		my_agent->smem_stmts->act_set->bind_int( 1, ( my_agent->smem_max_cycle++ ) );
		my_agent->smem_stmts->act_set->bind_int( 2, lti );
		my_agent->smem_stmts->act_set->execute( soar_module::op_reinit );
	}

	my_agent->smem_stmts->act_lti_child_ct_get->reinitialize();

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->act->stop();
	////////////////////////////////////////////////////////////////////////////
}

// gets the lti id for an existing lti letter/number pair (or NIL if failure)
smem_lti_id smem_lti_get_id( agent *my_agent, char name_letter, uint64_t name_number )
{
	smem_lti_id return_val = NIL;

	// getting lti ids requires an open semantic database
	smem_attach( my_agent );
	
	// letter=? AND number=?
	my_agent->smem_stmts->lti_get->bind_int( 1, static_cast<uintptr_t>( name_letter ) );
	my_agent->smem_stmts->lti_get->bind_int( 2, static_cast<uintptr_t>( name_number ) );

	if ( my_agent->smem_stmts->lti_get->execute() == soar_module::row )
	{
		return_val = my_agent->smem_stmts->lti_get->column_int( 0 );
	}

	my_agent->smem_stmts->lti_get->reinitialize();

	return return_val;
}

// adds a new lti id for a letter/number pair
inline smem_lti_id smem_lti_add_id( agent *my_agent, char name_letter, uint64_t name_number )
{
	smem_lti_id return_val;

	// create lti: letter, number
	my_agent->smem_stmts->lti_add->bind_int( 1, static_cast<uintptr_t>( name_letter ) );
	my_agent->smem_stmts->lti_add->bind_int( 2, static_cast<uintptr_t>( name_number ) );
	my_agent->smem_stmts->lti_add->bind_int( 3, static_cast<uintptr_t>( 0 ) );
	my_agent->smem_stmts->lti_add->bind_int( 4, static_cast<uintptr_t>( 0 ) );
	my_agent->smem_stmts->lti_add->execute( soar_module::op_reinit );

	return_val = static_cast<smem_lti_id>( my_agent->smem_db->last_insert_rowid() );

	// increment stat
	my_agent->smem_stats->chunks->set_value( my_agent->smem_stats->chunks->get_value() + 1 );

	return return_val;
}

// makes a non-long-term identifier into a long-term identifier
inline smem_lti_id smem_lti_soar_add( agent *my_agent, Symbol *id )
{
	if ( ( id->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
		 ( id->id.smem_lti == NIL ) )
	{
		// try to find existing lti
		id->id.smem_lti = smem_lti_get_id( my_agent, id->id.name_letter, id->id.name_number );

		// if doesn't exist, add
		if ( id->id.smem_lti == NIL )
		{
			id->id.smem_lti = smem_lti_add_id( my_agent, id->id.name_letter, id->id.name_number );

			id->id.smem_time_id = my_agent->epmem_stats->time->get_value();
			id->id.smem_valid = my_agent->epmem_validation;
		}
	}

	return id->id.smem_lti;
}

// returns a reference to an lti
Symbol *smem_lti_soar_make( agent *my_agent, smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level )
{
	Symbol *return_val;

	// try to find existing
	return_val = find_identifier( my_agent, name_letter, name_number );

	// otherwise create
	if ( return_val == NIL )
	{
		return_val = make_new_identifier( my_agent, name_letter, level, name_number );
	}
	else
	{
		symbol_add_ref( return_val );

		if ( ( return_val->id.level == SMEM_LTI_UNKNOWN_LEVEL ) && ( level != SMEM_LTI_UNKNOWN_LEVEL ) )
		{
			return_val->id.level = level;
			return_val->id.promotion_level = level;
		}
	}

	// set lti field irrespective
	return_val->id.smem_lti = lti;

	return return_val;
}

void smem_reset_id_counters( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() == soar_module::connected )
	{
		// letter, max
		while ( my_agent->smem_stmts->lti_max->execute() == soar_module::row )
		{
			uint64_t name_letter = static_cast<uint64_t>( my_agent->smem_stmts->lti_max->column_int( 0 ) );
			uint64_t letter_max = static_cast<uint64_t>( my_agent->smem_stmts->lti_max->column_int( 1 ) );

			// shift to alphabet
			name_letter -= static_cast<uint64_t>( 'A' );

			// get count
			uint64_t *letter_ct =& my_agent->id_counter[ name_letter ];

			// adjust if necessary
			if ( (*letter_ct) <= letter_max )
			{
				(*letter_ct) = ( letter_max + 1 );
			}
		}

		my_agent->smem_stmts->lti_max->reinitialize();
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Storage Functions (smem::storage)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline smem_slot *smem_make_slot( smem_slot_map *slots, Symbol *attr )
{
	smem_slot **s =& (*slots)[ attr ];

	if ( !(*s) )
	{
		(*s) = new smem_slot;
	}

	return (*s);
}

void smem_disconnect_chunk( agent *my_agent, smem_lti_id parent_id )
{
	// adjust attribute counts
	{
		intptr_t counter = 0;
		
		// get all old counts
		my_agent->smem_stmts->web_attr_ct->bind_int( 1, parent_id );
		while ( my_agent->smem_stmts->web_attr_ct->execute() == soar_module::row )
		{
			counter += my_agent->smem_stmts->web_attr_ct->column_int( 1 );
			
			// adjust in opposite direction ( adjust, attribute )
			my_agent->smem_stmts->ct_attr_update->bind_int( 1, -( my_agent->smem_stmts->web_attr_ct->column_int( 1 ) ) );
			my_agent->smem_stmts->ct_attr_update->bind_int( 2, my_agent->smem_stmts->web_attr_ct->column_int( 0 ) );
			my_agent->smem_stmts->ct_attr_update->execute( soar_module::op_reinit );
		}
		my_agent->smem_stmts->web_attr_ct->reinitialize();

		my_agent->smem_stats->slots->set_value( my_agent->smem_stats->slots->get_value() - counter );
	}

	// adjust const counts
	{
		// get all old counts
		my_agent->smem_stmts->web_const_ct->bind_int( 1, parent_id );
		while ( my_agent->smem_stmts->web_const_ct->execute() == soar_module::row )
		{
			// adjust in opposite direction ( adjust, attribute, const )
			my_agent->smem_stmts->ct_const_update->bind_int( 1, -( my_agent->smem_stmts->web_const_ct->column_int( 2 ) ) );
			my_agent->smem_stmts->ct_const_update->bind_int( 2, my_agent->smem_stmts->web_const_ct->column_int( 0 ) );
			my_agent->smem_stmts->ct_const_update->bind_int( 3, my_agent->smem_stmts->web_const_ct->column_int( 1 ) );
			my_agent->smem_stmts->ct_const_update->execute( soar_module::op_reinit );
		}
		my_agent->smem_stmts->web_const_ct->reinitialize();
	}

	// adjust lti counts
	{
		// get all old counts
		my_agent->smem_stmts->web_lti_ct->bind_int( 1, parent_id );
		while ( my_agent->smem_stmts->web_lti_ct->execute() == soar_module::row )
		{
			// adjust in opposite direction ( adjust, attribute, lti )
			my_agent->smem_stmts->ct_lti_update->bind_int( 1, -( my_agent->smem_stmts->web_lti_ct->column_int( 2 ) ) );
			my_agent->smem_stmts->ct_lti_update->bind_int( 2, my_agent->smem_stmts->web_lti_ct->column_int( 0 ) );
			my_agent->smem_stmts->ct_lti_update->bind_int( 3, my_agent->smem_stmts->web_lti_ct->column_int( 1 ) );
			my_agent->smem_stmts->ct_lti_update->execute( soar_module::op_reinit );
		}
		my_agent->smem_stmts->web_lti_ct->reinitialize();
	}

	// disconnect
	{
		my_agent->smem_stmts->web_truncate->bind_int( 1, parent_id );
		my_agent->smem_stmts->web_truncate->execute( soar_module::op_reinit );
	}
}

void smem_store_chunk( agent *my_agent, smem_lti_id parent_id, smem_slot_map *children, bool remove_old_children = true )
{
	smem_slot_map::iterator s;
	smem_slot::iterator v;

	smem_hash_id attr_hash = 0;
	smem_hash_id value_hash = 0;
	smem_lti_id value_lti = 0;

	std::map<smem_hash_id, unsigned long> attr_ct_adjust;
	std::map<smem_hash_id, std::map<smem_hash_id, unsigned long> > const_ct_adjust;
	std::map<smem_hash_id, std::map<smem_lti_id, unsigned long> > lti_ct_adjust;
	intptr_t stat_adjust = 0;

	intptr_t next_act_cycle = ( my_agent->smem_max_cycle++ );
	
	// clear web, adjust counts
	unsigned long child_ct = 0;
	if ( remove_old_children )
	{
		smem_disconnect_chunk( my_agent, parent_id );
	}
	else
	{
		my_agent->smem_stmts->act_lti_child_ct_get->bind_int( 1, parent_id );
		my_agent->smem_stmts->act_lti_child_ct_get->execute();

		child_ct = static_cast<unsigned long>( my_agent->smem_stmts->act_lti_child_ct_get->column_int(0) );

		my_agent->smem_stmts->act_lti_child_ct_get->reinitialize();
	}

	// already above threshold?
	unsigned long thresh = static_cast<unsigned long>( my_agent->smem_params->thresh->get_value() );
	bool before_above = ( child_ct >= thresh );

	// get final count
	{
		for ( s=children->begin(); s!=children->end(); s++ )
		{
			for ( v=s->second->begin(); v!=s->second->end(); v++ )
			{
				child_ct++;
			}
		}
	}

	// above threshold now?
	bool after_above = ( child_ct >= thresh );
	intptr_t web_act_cycle = ( ( after_above )?( SMEM_ACT_MAX ):( next_act_cycle ) );

	// if didn't clear and wasn't already above, need to update kids
	if ( ( !remove_old_children ) && ( !before_above ) )
	{
		my_agent->smem_stmts->act_set->bind_int( 1, web_act_cycle );
		my_agent->smem_stmts->act_set->bind_int( 2, parent_id );
		my_agent->smem_stmts->act_set->execute( soar_module::op_reinit );
	}

	// if above threshold, update parent activation
	if ( after_above )
	{
		my_agent->smem_stmts->act_lti_set->bind_int( 1, next_act_cycle );
		my_agent->smem_stmts->act_lti_set->bind_int( 2, parent_id );
		my_agent->smem_stmts->act_lti_set->execute( soar_module::op_reinit );
	}

	// for all slots
	for ( s=children->begin(); s!=children->end(); s++ )
	{
		// get attribute hash and contribute to count adjustment
		attr_hash = smem_temporal_hash( my_agent, s->first );
		attr_ct_adjust[ attr_hash ]++;
		stat_adjust++;

		// for all values in the slot
		for ( v=s->second->begin(); v!=s->second->end(); v++ )
		{			
			// most handling is specific to constant vs. identifier
			if ( (*v)->val_const.val_type == value_const_t )
			{
				value_hash = smem_temporal_hash( my_agent, (*v)->val_const.val_value );

				// parent_id, attr, val_const, val_lti, act_cycle
				my_agent->smem_stmts->web_add->bind_int( 1, parent_id );
				my_agent->smem_stmts->web_add->bind_int( 2, attr_hash );
				my_agent->smem_stmts->web_add->bind_int( 3, value_hash );
				my_agent->smem_stmts->web_add->bind_null( 4 );
				my_agent->smem_stmts->web_add->bind_int( 5, web_act_cycle );
				my_agent->smem_stmts->web_add->execute( soar_module::op_reinit );

				const_ct_adjust[ attr_hash ][ value_hash ]++;
			}
			else
			{
				value_lti = (*v)->val_lti.val_value->lti_id;
				if ( value_lti == NIL )
				{
					value_lti = smem_lti_add_id( my_agent, (*v)->val_lti.val_value->lti_letter, (*v)->val_lti.val_value->lti_number );
					(*v)->val_lti.val_value->lti_id = value_lti;

					if ( (*v)->val_lti.val_value->soar_id != NIL )
					{
						(*v)->val_lti.val_value->soar_id->id.smem_lti = value_lti;

						(*v)->val_lti.val_value->soar_id->id.smem_time_id = my_agent->epmem_stats->time->get_value();
						(*v)->val_lti.val_value->soar_id->id.smem_valid = my_agent->epmem_validation;
					}
				}

				// parent_id, attr, val_const, val_lti, act_cycle
				my_agent->smem_stmts->web_add->bind_int( 1, parent_id );
				my_agent->smem_stmts->web_add->bind_int( 2, attr_hash );
				my_agent->smem_stmts->web_add->bind_null( 3 );
				my_agent->smem_stmts->web_add->bind_int( 4, value_lti );
				my_agent->smem_stmts->web_add->bind_int( 5, web_act_cycle );
				my_agent->smem_stmts->web_add->execute( soar_module::op_reinit );

				// add to counts
				lti_ct_adjust[ attr_hash ][ value_lti ]++;
			}
		}
	}

	// update stat
	{
		my_agent->smem_stats->slots->set_value( my_agent->smem_stats->slots->get_value() + stat_adjust );
	}

	// update attribute counts
	{
		std::map<smem_hash_id, unsigned long>::iterator p;

		for ( p=attr_ct_adjust.begin(); p!= attr_ct_adjust.end(); p++ )
		{
			// check if counter exists (and add if does not): attr
			my_agent->smem_stmts->ct_attr_check->bind_int( 1, p->first );
			if ( my_agent->smem_stmts->ct_attr_check->execute( soar_module::op_reinit ) != soar_module::row )
			{
				my_agent->smem_stmts->ct_attr_add->bind_int( 1, p->first );
				my_agent->smem_stmts->ct_attr_add->execute( soar_module::op_reinit );
			}

			// adjust count (adjustment, attr)
			my_agent->smem_stmts->ct_attr_update->bind_int( 1, p->second );
			my_agent->smem_stmts->ct_attr_update->bind_int( 2, p->first );
			my_agent->smem_stmts->ct_attr_update->execute( soar_module::op_reinit );
		}
	}

	// update constant counts
	{
		std::map<smem_hash_id, std::map<smem_hash_id, unsigned long> >::iterator p1;
		std::map<smem_hash_id, unsigned long>::iterator p2;

		for ( p1=const_ct_adjust.begin(); p1!=const_ct_adjust.end(); p1++ )
		{
			for ( p2=(p1->second).begin(); p2!=(p1->second).end(); p2++ )
			{
				// check if counter exists (and add if does not): attr, val
				my_agent->smem_stmts->ct_const_check->bind_int( 1, p1->first );
				my_agent->smem_stmts->ct_const_check->bind_int( 2, p2->first );
				if ( my_agent->smem_stmts->ct_const_check->execute( soar_module::op_reinit ) != soar_module::row )
				{
					my_agent->smem_stmts->ct_const_add->bind_int( 1, p1->first );
					my_agent->smem_stmts->ct_const_add->bind_int( 2, p2->first );
					my_agent->smem_stmts->ct_const_add->execute( soar_module::op_reinit );
				}

				// adjust count (adjustment, attr, val)
				my_agent->smem_stmts->ct_const_update->bind_int( 1, p2->second );
				my_agent->smem_stmts->ct_const_update->bind_int( 2, p1->first );
				my_agent->smem_stmts->ct_const_update->bind_int( 3, p2->first );
				my_agent->smem_stmts->ct_const_update->execute( soar_module::op_reinit );
			}
		}
	}

	// update lti counts
	{
		std::map<smem_hash_id, std::map<smem_lti_id, unsigned long> >::iterator p1;
		std::map<smem_lti_id, unsigned long>::iterator p2;

		for ( p1=lti_ct_adjust.begin(); p1!=lti_ct_adjust.end(); p1++ )
		{
			for ( p2=(p1->second).begin(); p2!=(p1->second).end(); p2++ )
			{
				// check if counter exists (and add if does not): attr, val
				my_agent->smem_stmts->ct_lti_check->bind_int( 1, p1->first );
				my_agent->smem_stmts->ct_lti_check->bind_int( 2, p2->first );
				if ( my_agent->smem_stmts->ct_lti_check->execute( soar_module::op_reinit ) != soar_module::row )
				{
					my_agent->smem_stmts->ct_lti_add->bind_int( 1, p1->first );
					my_agent->smem_stmts->ct_lti_add->bind_int( 2, p2->first );
					my_agent->smem_stmts->ct_lti_add->execute( soar_module::op_reinit );
				}

				// adjust count (adjustment, attr, lti)
				my_agent->smem_stmts->ct_lti_update->bind_int( 1, p2->second );
				my_agent->smem_stmts->ct_lti_update->bind_int( 2, p1->first );
				my_agent->smem_stmts->ct_lti_update->bind_int( 3, p2->first );
				my_agent->smem_stmts->ct_lti_update->execute( soar_module::op_reinit );
			}
		}
	}

	// update child count
	{
		my_agent->smem_stmts->act_lti_child_ct_set->bind_int( 1, child_ct );
		my_agent->smem_stmts->act_lti_child_ct_set->bind_int( 2, parent_id );
		my_agent->smem_stmts->act_lti_child_ct_set->execute( soar_module::op_reinit );
	}
}

void smem_soar_store( agent *my_agent, Symbol *id, smem_storage_type store_type = store_level, tc_number tc = NIL )
{
	// transitive closure only matters for recursive storage
	if ( ( store_type == store_recursive ) && ( tc == NIL ) )
	{
		tc = get_new_tc_number( my_agent );
	}

	// get level
	smem_wme_list *children = smem_get_direct_augs_of_id( id, tc );
	smem_wme_list::iterator w;

	// encode this level
	{
		smem_sym_to_chunk_map sym_to_chunk;
		smem_sym_to_chunk_map::iterator c_p;
		smem_chunk **c;

		smem_slot_map slots;
		smem_slot_map::iterator s_p;
		smem_slot::iterator v_p;
		smem_slot *s;
		smem_chunk_value *v;

		for ( w=children->begin(); w!=children->end(); w++ )
		{
			// get slot
			s = smem_make_slot( &( slots ), (*w)->attr );

			// create value, per type
			v = new smem_chunk_value;
			if ( smem_symbol_is_constant( (*w)->value ) )
			{
				v->val_const.val_type = value_const_t;
				v->val_const.val_value = (*w)->value;
			}
			else
			{
				v->val_lti.val_type = value_lti_t;

				// try to find existing chunk
				c =& sym_to_chunk[ (*w)->value ];

				// if doesn't exist, add; else use existing
				if ( !(*c) )
				{
					(*c) = new smem_chunk;
					(*c)->lti_id = (*w)->value->id.smem_lti;
					(*c)->lti_letter = (*w)->value->id.name_letter;
					(*c)->lti_number = (*w)->value->id.name_number;
					(*c)->slots = NULL;
					(*c)->soar_id = (*w)->value;
				}

				v->val_lti.val_value = (*c);
			}

			// add value to slot
			s->push_back( v );
		}

		smem_store_chunk( my_agent, smem_lti_soar_add( my_agent, id ), &( slots ) );

		// clean up
		{
			// de-allocate slots
			for ( s_p=slots.begin(); s_p!=slots.end(); s_p++ )
			{
				for ( v_p=s_p->second->begin(); v_p!=s_p->second->end(); v_p++ )
				{
					delete (*v_p);
				}

				delete s_p->second;
			}

			// de-allocate chunks
			for ( c_p=sym_to_chunk.begin(); c_p!=sym_to_chunk.end(); c_p++ )
			{
				delete c_p->second;
			}
		}
	}

	// recurse as necessary
	if ( store_type == store_recursive )
	{
		for ( w=children->begin(); w!=children->end(); w++ )
		{
			if ( !smem_symbol_is_constant( (*w)->value ) )
			{
				smem_soar_store( my_agent, (*w)->value, store_type, tc );
			}
		}
	}

	// clean up child wme list
	delete children;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Non-Cue-Based Retrieval Functions (smem::ncb)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_install_memory( agent *my_agent, Symbol *state, smem_lti_id parent_id, Symbol *lti = NIL )
{
	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->ncb_retrieval->start();
	////////////////////////////////////////////////////////////////////////////

	// get the ^result header for this state
	Symbol *result_header = state->id.smem_result_header;

	// get identifier if not known
	bool lti_created_here = false;
	if ( lti == NIL )
	{
		soar_module::sqlite_statement *q = my_agent->smem_stmts->lti_letter_num;

		q->bind_int( 1, parent_id );
		q->execute();

		lti = smem_lti_soar_make( my_agent, parent_id, static_cast<char>( q->column_int( 0 ) ), static_cast<unsigned long>( q->column_int( 1 ) ), result_header->id.level );

		q->reinitialize();

		lti_created_here = true;
	}

	// activate lti
	smem_lti_activate( my_agent, parent_id );

	// point retrieved to lti
	smem_add_meta_wme( my_agent, state, result_header, my_agent->smem_sym_retrieved, lti );
	if ( lti_created_here )
	{
		// if the identifier was created above we need to
		// remove a single ref count AFTER the wme
		// is added (such as to not deallocate the symbol
		// prematurely)
		symbol_remove_ref( my_agent, lti );
	}	

	// if no children, then retrieve children
	if ( ( lti->id.impasse_wmes == NIL ) &&
		 ( lti->id.input_wmes == NIL ) &&
		 ( lti->id.slots == NIL ) )
	{
		soar_module::sqlite_statement *expand_q = my_agent->smem_stmts->web_expand;
		Symbol *attr_sym;
		Symbol *value_sym;

		// get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
		expand_q->bind_int( 1, parent_id );
		while ( expand_q->execute() == soar_module::row )
		{
			// make the identifier symbol irrespective of value type
			attr_sym = smem_reverse_hash( my_agent, static_cast<byte>( expand_q->column_int(0) ), static_cast<smem_hash_id>( expand_q->column_int(1) ) );

			// identifier vs. constant
			if ( expand_q->column_type( 6 ) != soar_module::null_t )
			{
				value_sym = smem_lti_soar_make( my_agent, static_cast<smem_lti_id>( expand_q->column_int( 6 ) ), static_cast<char>( expand_q->column_int( 4 ) ), static_cast<unsigned long>( expand_q->column_int( 5 ) ), lti->id.level );
			}
			else
			{
				value_sym = smem_reverse_hash( my_agent, static_cast<byte>( expand_q->column_int(2) ), static_cast<smem_hash_id>( expand_q->column_int(3) ) );
			}

			// add wme
			smem_add_retrieved_wme( my_agent, state, lti, attr_sym, value_sym );

			// deal with ref counts - attribute/values are always created in this function
			// (thus an extra ref count is set before adding a wme)
			symbol_remove_ref( my_agent, attr_sym );
			symbol_remove_ref( my_agent, value_sym );
		}
		expand_q->reinitialize();
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->ncb_retrieval->stop();
	////////////////////////////////////////////////////////////////////////////
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Cue-Based Retrieval Functions (smem::cbr)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_lti_id smem_process_query( agent *my_agent, Symbol *state, Symbol *query, smem_lti_set *prohibit, smem_query_levels query_level = qry_full )
{	
	smem_weighted_cue_list weighted_cue;	
	bool good_cue = true;

	soar_module::sqlite_statement *q = NULL;

	smem_lti_id king_id = NIL;

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->query->start();
	////////////////////////////////////////////////////////////////////////////

	// prepare query stats
	{
		smem_prioritized_weighted_cue weighted_pq;
		smem_weighted_cue_element *new_cue_element;
		
		smem_wme_list *cue = smem_get_direct_augs_of_id( query );
		smem_wme_list::iterator cue_p;

		smem_hash_id attr_hash;
		smem_hash_id value_hash;
		smem_lti_id value_lti;
		smem_cue_element_type element_type = attr_t;

		wme *w;

		for ( cue_p=cue->begin(); cue_p!=cue->end(); cue_p++ )
		{
			w = (*cue_p);

			state->id.smem_info->cue_wmes->insert( w );

			if ( good_cue )
			{
				// we only have to do hard work if
				attr_hash = smem_temporal_hash( my_agent, w->attr, false );
				if ( attr_hash != NIL )
				{
					if ( smem_symbol_is_constant( w->value ) )
					{
						value_lti = NIL;
						value_hash = smem_temporal_hash( my_agent, w->value, false );

						if ( value_hash != NIL )
						{
							q = my_agent->smem_stmts->ct_const_get;
							q->bind_int( 1, attr_hash );
							q->bind_int( 2, value_hash );

							element_type = value_const_t;
						}
						else
						{
							good_cue = false;
						}
					}
					else
					{
						value_lti = w->value->id.smem_lti;
						value_hash = NIL;

						if ( value_lti == NIL )
						{
							q = my_agent->smem_stmts->ct_attr_get;
							q->bind_int( 1, attr_hash );

							element_type = attr_t;
						}
						else
						{
							q = my_agent->smem_stmts->ct_lti_get;
							q->bind_int( 1, attr_hash );
							q->bind_int( 2, value_lti );

							element_type = value_lti_t;
						}
					}

					if ( good_cue )
					{
						if ( q->execute() == soar_module::row )
						{
							new_cue_element = new smem_weighted_cue_element;

							new_cue_element->weight = q->column_int( 0 );
							new_cue_element->attr_hash = attr_hash;
							new_cue_element->value_hash = value_hash;
							new_cue_element->value_lti = value_lti;
							new_cue_element->cue_element = w;

							new_cue_element->element_type = element_type;

							weighted_pq.push( new_cue_element );
							new_cue_element = NULL;
						}
						else
						{
							good_cue = false;
						}

						q->reinitialize();
					}
				}
				else
				{
					good_cue = false;
				}
			}
		}

		// if valid cue, transfer priority queue to list
		if ( good_cue )
		{
			while ( !weighted_pq.empty() )
			{
				weighted_cue.push_back( weighted_pq.top() );
				weighted_pq.pop();
			}
		}
		// else deallocate priority queue contents
		else
		{
			while ( !weighted_pq.empty() )
			{
				delete weighted_pq.top();
				weighted_pq.pop();
			}
		}

		// clean cue irrespective of validity
		delete cue;
	}

	// only search if the cue was valid
	if ( good_cue && !weighted_cue.empty() )
	{
		smem_weighted_cue_list::iterator first_element = weighted_cue.begin();
		smem_weighted_cue_list::iterator next_element;
		smem_weighted_cue_list::iterator second_element = first_element;		
		second_element++;

		soar_module::sqlite_statement *q2 = NULL;
		smem_lti_set::iterator prohibit_p;

		smem_lti_id cand;
		bool good_cand;
		
		// setup first query, which is sorted on activation already
		{
			if ( (*first_element)->element_type == attr_t )
			{
				// attr=?
				q = my_agent->smem_stmts->web_attr_all;
			}
			else if ( (*first_element)->element_type == value_const_t )
			{
				// attr=? AND val_const=?
				q = my_agent->smem_stmts->web_const_all;
				q->bind_int( 2, (*first_element)->value_hash );
			}
			else if ( (*first_element)->element_type == value_lti_t )
			{
				// attr=? AND val_lti=?
				q = my_agent->smem_stmts->web_lti_all;
				q->bind_int( 2, (*first_element)->value_lti );
			}

			// all require hash as first parameter
			q->bind_int( 1, (*first_element)->attr_hash );
		}

		if ( q->execute() == soar_module::row )
		{
			smem_prioritized_activated_lti_queue plentiful_parents;
			bool more_rows = true;
			bool use_db = false;

			while ( more_rows && ( q->column_int( 1 ) == SMEM_ACT_MAX ) )
			{
				my_agent->smem_stmts->act_lti_get->bind_int( 1, q->column_int( 0 ) );
				my_agent->smem_stmts->act_lti_get->execute();				
				plentiful_parents.push( std::make_pair< intptr_t, smem_lti_id >( my_agent->smem_stmts->act_lti_get->column_int( 0 ), q->column_int( 0 ) ) );
				my_agent->smem_stmts->act_lti_get->reinitialize();

				more_rows = ( q->execute() == soar_module::row );
			}

			while ( ( king_id == NIL ) && ( ( more_rows ) || ( !plentiful_parents.empty() ) ) )
			{
				// choose next candidate (db vs. priority queue)
				{				
					use_db = false;
					
					if ( !more_rows )
					{
						use_db = false;
					}
					else if ( plentiful_parents.empty() )
					{
						use_db = true;
					}
					else
					{
						use_db = ( q->column_int( 1 ) >  plentiful_parents.top().first );						
					}

					if ( use_db )
					{
						cand = q->column_int( 0 );
						more_rows = ( q->execute() == soar_module::row );
					}
					else
					{
						cand = plentiful_parents.top().second;
						plentiful_parents.pop();
					}
				}

				// if not prohibited, submit to the remaining cue elements
				prohibit_p = prohibit->find( cand );
				if ( prohibit_p == prohibit->end() )
				{
					good_cand = true;

					for ( next_element=second_element; ( ( good_cand ) && ( next_element!=weighted_cue.end() ) ); next_element++ )
					{
						if ( (*next_element)->element_type == attr_t )
						{
							// parent=? AND attr=?
							q2 = my_agent->smem_stmts->web_attr_child;
						}
						else if ( (*next_element)->element_type == value_const_t )
						{
							// parent=? AND attr=? AND val_const=?
							q2 = my_agent->smem_stmts->web_const_child;
							q2->bind_int( 3, (*next_element)->value_hash );
						}
						else if ( (*next_element)->element_type == value_lti_t )
						{
							// parent=? AND attr=? AND val_lti=?
							q2 = my_agent->smem_stmts->web_lti_child;
							q2->bind_int( 3, (*next_element)->value_lti );
						}

						// all require own id, attribute
						q2->bind_int( 1, cand );
						q2->bind_int( 2, (*next_element)->attr_hash );

						good_cand = ( q2->execute( soar_module::op_reinit ) == soar_module::row );
					}

					if ( good_cand )
					{
						king_id = cand;
					}
				}
			}
		}
		q->reinitialize();		

		// clean weighted cue
		for ( next_element=weighted_cue.begin(); next_element!=weighted_cue.end(); next_element++ )
		{
			delete (*next_element);
		}
	}

	// reconstruction depends upon level
	if ( query_level == qry_full )
	{
		// produce results
		if ( king_id != NIL )
		{
			// success!
			smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_success, query );

			////////////////////////////////////////////////////////////////////////////
			my_agent->smem_timers->query->stop();
			////////////////////////////////////////////////////////////////////////////

			smem_install_memory( my_agent, state, king_id );
		}
		else
		{
			smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_failure, query );

			////////////////////////////////////////////////////////////////////////////
			my_agent->smem_timers->query->stop();
			////////////////////////////////////////////////////////////////////////////
		}
	}
	else
	{
		////////////////////////////////////////////////////////////////////////////
		my_agent->smem_timers->query->stop();
		////////////////////////////////////////////////////////////////////////////
	}

	return king_id;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization (smem::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_clear_result( agent *my_agent, Symbol *state )
{
	preference *pref;

	while ( !state->id.smem_info->smem_wmes->empty() )
	{
		pref = state->id.smem_info->smem_wmes->top();
		state->id.smem_info->smem_wmes->pop();

		if ( pref->in_tm )
		{
			remove_preference_from_tm( my_agent, pref );
		}
	}
}

// performs cleanup when a state is removed
void smem_reset( agent *my_agent, Symbol *state )
{
	if ( state == NULL )
	{
		state = my_agent->top_goal;
	}

	while( state )
	{
		smem_data *data = state->id.smem_info;
		
		data->last_cmd_time[0] = 0;
		data->last_cmd_time[1] = 0;
		data->last_cmd_count[0] = 0;
		data->last_cmd_count[1] = 0;

		data->cue_wmes->clear();
		
		// this will be called after prefs from goal are already removed,
		// so just clear out result stack
		while ( !data->smem_wmes->empty() )
		{
			data->smem_wmes->pop();
		}		

		state = state->id.lower_goal;
	}
}

// opens the SQLite database and performs all initialization required for the current mode
void smem_init_db( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() != soar_module::disconnected )
	{
		return;
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->init->start();
	////////////////////////////////////////////////////////////////////////////

	const char *db_path;
	if ( my_agent->smem_params->database->get_value() == smem_param_container::memory )
	{
		db_path = ":memory:";
	}
	else
	{
		db_path = my_agent->smem_params->path->get_value();
	}

	// attempt connection
	my_agent->smem_db->connect( db_path );

	if ( my_agent->smem_db->get_status() == soar_module::problem )
	{
		char buf[256];
		SNPRINTF( buf, 254, "DB ERROR: %s", my_agent->smem_db->get_errmsg() );

		print( my_agent, buf );
		xml_generate_warning( my_agent, buf );
	}
	else
	{
		// temporary queries for one-time init actions
		soar_module::sqlite_statement *temp_q = NULL;

		// apply performance options
		{
			// cache
			{
				switch ( my_agent->smem_params->cache->get_value() )
				{
					// 5MB cache
					case ( smem_param_container::cache_S ):
						temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA cache_size = 5000" );
						break;

					// 20MB cache
					case ( smem_param_container::cache_M ):
						temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA cache_size = 20000" );
						break;

					// 100MB cache
					case ( smem_param_container::cache_L ):
						temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA cache_size = 100000" );
						break;
				}

				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}

			// optimization
			if ( my_agent->smem_params->opt->get_value() == smem_param_container::opt_speed )
			{
				// synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
				temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA synchronous = OFF" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;

				// journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
				temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA journal_mode = OFF" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
				
				// locking_mode - no one else can view the database after our first write
				temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "PRAGMA locking_mode = EXCLUSIVE" );
				temp_q->prepare();
				temp_q->execute();
				delete temp_q;
				temp_q = NULL;
			}
		}

		// update validation count
		my_agent->smem_validation++;

		// setup common structures/queries
		my_agent->smem_stmts = new smem_statement_container( my_agent );

		// setup initial structures (if necessary)
		bool tabula_rasa;
		{
			// create structures if database does not contain signature table
			// which we can detect by trying to create it
			// note: this only could have been done with an open database (hence in initialization)

			temp_q = new soar_module::sqlite_statement( my_agent->smem_db, "CREATE TABLE " SMEM_SIGNATURE " (uid INTEGER)" );

			temp_q->prepare();
			tabula_rasa = ( temp_q->get_status() == soar_module::ready );

			if ( tabula_rasa )
			{
				// if was possible to prepare, the table doesn't exist so we create it
				temp_q->execute();

				// and all other structures
				my_agent->smem_stmts->structure();
			}

			delete temp_q;
			temp_q = NULL;
		}

		// initialize queries given database structure
		my_agent->smem_stmts->prepare();

		// initialize persistent variables
		if ( tabula_rasa )
		{
			my_agent->smem_stmts->begin->execute( soar_module::op_reinit );
			{
				// max cycle
				my_agent->smem_max_cycle = 1;
				smem_variable_create( my_agent, var_max_cycle, 1 );

				// number of nodes
				my_agent->smem_stats->chunks->set_value( 0 );
				smem_variable_create( my_agent, var_num_nodes, 0 );

				// number of edges
				my_agent->smem_stats->slots->set_value( 0 );
				smem_variable_create( my_agent, var_num_edges, 0 );

				// threshold (from user parameter value)
				smem_variable_create( my_agent, var_act_thresh, static_cast<intptr_t>( my_agent->smem_params->thresh->get_value() ) );
			}
			my_agent->smem_stmts->commit->execute( soar_module::op_reinit );
		}
		else
		{
			intptr_t temp;

			// max cycle
			smem_variable_get( my_agent, var_max_cycle, &( my_agent->smem_max_cycle ) );

			// number of nodes
			smem_variable_get( my_agent, var_num_nodes, &( temp ) );
			my_agent->smem_stats->chunks->set_value( temp );

			// number of edges
			smem_variable_get( my_agent, var_num_edges, &( temp ) );
			my_agent->smem_stats->slots->set_value( temp );

			// threshold
			smem_variable_get( my_agent, var_act_thresh, &( temp ) );
			my_agent->smem_params->thresh->set_value( static_cast<long>( temp ) );
		}

		// reset identifier counters
		smem_reset_id_counters( my_agent );

		// if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
		if ( my_agent->smem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->smem_stmts->begin->execute( soar_module::op_reinit );
		}
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->init->stop();
	////////////////////////////////////////////////////////////////////////////
}

void smem_attach( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() == soar_module::disconnected )
	{
		smem_init_db( my_agent );
	}
}

// performs cleanup operations when the database needs to be closed (end soar, manual close, etc)
void smem_close( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() == soar_module::connected )
	{
		// store max cycle for future use of the smem database
		smem_variable_set( my_agent, var_max_cycle, my_agent->smem_max_cycle );

		// store num nodes/edges for future use of the smem database
		smem_variable_set( my_agent, var_num_nodes, my_agent->smem_stats->chunks->get_value() );
		smem_variable_set( my_agent, var_num_edges, my_agent->smem_stats->slots->get_value() );

		// if lazy, commit
		if ( my_agent->smem_params->lazy_commit->get_value() == soar_module::on )
		{
			my_agent->smem_stmts->commit->execute( soar_module::op_reinit );
		}

		// de-allocate common statements
		delete my_agent->smem_stmts;

		// close the database
		my_agent->smem_db->disconnect();
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parsing (smem::parse)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_deallocate_chunk( agent *my_agent, smem_chunk *chunk, bool free_chunk = true )
{
	if ( chunk )
	{
		// proceed to slots
		if ( chunk->slots )
		{
			smem_slot_map::iterator s;
			smem_slot::iterator v;

			// iterate over slots
			while ( !chunk->slots->empty() )
			{
				s = chunk->slots->begin();

				// proceed to slot contents
				if ( s->second )
				{
					// iterate over each value
					for ( v=s->second->begin(); v!=s->second->end(); v=s->second->erase(v) )
					{
						// de-allocation of value is dependent upon type
						if ( (*v)->val_const.val_type == value_const_t )
						{
							symbol_remove_ref( my_agent, (*v)->val_const.val_value );
						}
						else
						{
							// we never deallocate the lti chunk, as we assume
							// it will exist elsewhere for deallocation
							// delete (*s)->val_lti.val_value;
						}

						delete (*v);
					}

					delete s->second;
				}

				// deallocate attribute for each corresponding value
				symbol_remove_ref( my_agent, s->first );

				chunk->slots->erase( s );
			}

			// remove slots
			delete chunk->slots;
			chunk->slots = NULL;
		}

		// remove chunk itself
		if ( free_chunk )
		{
			delete chunk;
			chunk = NULL;
		}
	}
}

inline std::string *smem_parse_lti_name( struct lexeme_info *lexeme, char *id_letter, unsigned long *id_number )
{
	std::string *return_val = new std::string;

	if ( (*lexeme).type == IDENTIFIER_LEXEME )
	{
		std::string num;
		to_string( (*lexeme).id_number, num );

		return_val->append( 1, (*lexeme).id_letter );
		return_val->append( num );

		(*id_letter) = (*lexeme).id_letter;
		(*id_number) = (*lexeme).id_number;
	}
	else
	{
		return_val->assign( (*lexeme).string );

		(*id_letter) = static_cast<char>( static_cast<int>( toupper( (*lexeme).string[1] ) ) );
		(*id_number) = NIL;
	}

	return return_val;
}

inline Symbol *smem_parse_constant_attr( agent *my_agent, struct lexeme_info *lexeme )
{
	Symbol *return_val = NIL;

	if ( ( (*lexeme).type == SYM_CONSTANT_LEXEME ) )
	{
		return_val = make_sym_constant( my_agent, static_cast<const char *>( (*lexeme).string ) );
	}
	else if ( (*lexeme).type == INT_CONSTANT_LEXEME )
	{
		return_val = make_int_constant( my_agent, (*lexeme).int_val );
	}
	else if ( (*lexeme).type == FLOAT_CONSTANT_LEXEME )
	{
		return_val = make_float_constant( my_agent, (*lexeme).float_val );
	}

	return return_val;
}

bool smem_parse_chunk( agent *my_agent, smem_str_to_chunk_map *chunks, smem_chunk_set *newbies )
{
	bool return_val = false;

	smem_chunk *new_chunk = new smem_chunk;
	new_chunk->slots = NULL;

	std::string *chunk_name = NULL;

	char temp_letter;
	unsigned long temp_number;

	bool good_at;

	//

	// consume left paren
	get_lexeme( my_agent );

	if ( ( my_agent->lexeme.type == AT_LEXEME ) || ( my_agent->lexeme.type == IDENTIFIER_LEXEME ) || ( my_agent->lexeme.type == VARIABLE_LEXEME ) )
	{		
		good_at = true;
		
		if ( my_agent->lexeme.type == AT_LEXEME )
		{
			get_lexeme( my_agent );

			good_at = ( my_agent->lexeme.type == IDENTIFIER_LEXEME );
		}
		
		if ( good_at )
		{
			// save identifier
			chunk_name = smem_parse_lti_name( &( my_agent->lexeme ), &( temp_letter ), &( temp_number ) );
			new_chunk->lti_letter = temp_letter;
			new_chunk->lti_number = temp_number;
			new_chunk->lti_id = NIL;
			new_chunk->soar_id = NIL;
			new_chunk->slots = new smem_slot_map;

			// consume id
			get_lexeme( my_agent );

			//

			unsigned long intermediate_counter = 1;
			smem_chunk *intermediate_parent;
			smem_chunk *temp_chunk;
			std::string temp_key;
			std::string *temp_key2;
			Symbol *chunk_attr;
			smem_chunk_value *chunk_value;
			smem_slot *s;

			// populate slots
			while ( my_agent->lexeme.type == UP_ARROW_LEXEME )
			{
				intermediate_parent = new_chunk;

				// go on to attribute
				get_lexeme( my_agent );

				// get the appropriate constant type
				chunk_attr = smem_parse_constant_attr( my_agent, &( my_agent->lexeme ) );

				// if constant attribute, proceed to value
				if ( chunk_attr != NIL )
				{
					// consume attribute
					get_lexeme( my_agent );

					// support for dot notation:
					// when we encounter a dot, instantiate
					// the previous attribute as a temporary
					// identifier and use that as the parent
					while ( my_agent->lexeme.type == PERIOD_LEXEME )
					{
						// create a new chunk
						temp_chunk = new smem_chunk;
						temp_chunk->lti_letter = ( ( chunk_attr->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE )?( static_cast<char>( static_cast<int>( chunk_attr->sc.name[0] ) ) ):( 'X' ) );
						temp_chunk->lti_number = ( intermediate_counter++ );
						temp_chunk->lti_id = NIL;
						temp_chunk->slots = new smem_slot_map;
						temp_chunk->soar_id = NIL;

						// add it as a child to the current parent
						chunk_value = new smem_chunk_value;
						chunk_value->val_lti.val_type = value_lti_t;
						chunk_value->val_lti.val_value = temp_chunk;
						s = smem_make_slot( intermediate_parent->slots, chunk_attr );
						s->push_back( chunk_value );

						// create a key guaranteed to be unique
						std::string temp_key3;
						to_string( temp_chunk->lti_number, temp_key3 );
						temp_key.assign( "<" );
						temp_key.append( 1, temp_chunk->lti_letter );
						temp_key.append( "#" );
						temp_key.append( temp_key3 );
						temp_key.append( ">" );

						// insert the new chunk
						(*chunks)[ temp_key ] = temp_chunk;

						// definitely a new chunk
						newbies->insert( temp_chunk );

						// the new chunk is our parent for this set of values (or further dots)
						intermediate_parent = temp_chunk;
						temp_chunk = NULL;

						// get the next attribute
						get_lexeme( my_agent );
						chunk_attr = smem_parse_constant_attr( my_agent, &( my_agent->lexeme ) );

						// consume attribute
						get_lexeme( my_agent );
					}

					if ( chunk_attr != NIL )
					{
						do
						{
							// value by type
							chunk_value = NIL;
							if ( ( my_agent->lexeme.type == SYM_CONSTANT_LEXEME ) )
							{
								chunk_value = new smem_chunk_value;
								chunk_value->val_const.val_type = value_const_t;
								chunk_value->val_const.val_value = make_sym_constant( my_agent, static_cast<const char *>( my_agent->lexeme.string ) );
							}
							else if ( ( my_agent->lexeme.type == INT_CONSTANT_LEXEME ) )
							{
								chunk_value = new smem_chunk_value;
								chunk_value->val_const.val_type = value_const_t;
								chunk_value->val_const.val_value = make_int_constant( my_agent, my_agent->lexeme.int_val );
							}
							else if ( ( my_agent->lexeme.type == FLOAT_CONSTANT_LEXEME ) )
							{
								chunk_value = new smem_chunk_value;
								chunk_value->val_const.val_type = value_const_t;
								chunk_value->val_const.val_value = make_float_constant( my_agent, my_agent->lexeme.float_val );
							}
							else if ( ( my_agent->lexeme.type == AT_LEXEME ) || ( my_agent->lexeme.type == IDENTIFIER_LEXEME ) || ( my_agent->lexeme.type == VARIABLE_LEXEME ) )
							{
								good_at = true;
								
								if ( my_agent->lexeme.type == AT_LEXEME )
								{
									get_lexeme( my_agent );

									good_at = ( my_agent->lexeme.type == IDENTIFIER_LEXEME );
								}

								if ( good_at )
								{								
									// create new value
									chunk_value = new smem_chunk_value;
									chunk_value->val_lti.val_type = value_lti_t;

									// get key
									temp_key2 = smem_parse_lti_name( &( my_agent->lexeme ), &( temp_letter ), &( temp_number ) );

									// search for an existing chunk
									smem_str_to_chunk_map::iterator p = chunks->find( (*temp_key2) );

									// if exists, point; else create new
									if ( p != chunks->end() )
									{
										chunk_value->val_lti.val_value = p->second;
									}
									else
									{
										// create new chunk
										temp_chunk = new smem_chunk;
										temp_chunk->lti_id = NIL;
										temp_chunk->lti_letter = temp_letter;
										temp_chunk->lti_number = temp_number;
										temp_chunk->lti_id = NIL;
										temp_chunk->slots = NULL;

										// associate with value
										chunk_value->val_lti.val_value = temp_chunk;

										// add to chunks
										(*chunks)[ (*temp_key2) ] = temp_chunk;

										// possibly a newbie (could be a self-loop)
										newbies->insert( temp_chunk );
									}

									delete temp_key2;
								}
							}

							if ( chunk_value != NIL )
							{
								// consume
								get_lexeme( my_agent );

								// add to appropriate slot
								s = smem_make_slot( intermediate_parent->slots, chunk_attr );
								s->push_back( chunk_value );

								// if this was the last attribute
								if ( my_agent->lexeme.type == R_PAREN_LEXEME )
								{
									return_val = true;
									get_lexeme( my_agent );
									chunk_value = NIL;
								}
							}
						} while ( chunk_value != NIL );
					}
				}
			}
		}
		else
		{
			delete new_chunk;
		}
	}
	else
	{
		delete new_chunk;
	}

	if ( return_val )
	{
		// search for an existing chunk (occurs if value comes before id)
		smem_chunk **p =& (*chunks)[ (*chunk_name ) ];

		if ( !(*p) )
		{
			(*p) = new_chunk;

			// a newbie!
			newbies->insert( new_chunk );
		}
		else
		{
			// transfer slots
			if ( !(*p)->slots )
			{
				// if none previously, can just use
				(*p)->slots = new_chunk->slots;
				new_chunk->slots = NULL;
			}
			else
			{
				// otherwise, copy

				smem_slot_map::iterator ss_p;
				smem_slot::iterator s_p;

				smem_slot *source_slot;
				smem_slot *target_slot;

				// for all slots
				for ( ss_p=new_chunk->slots->begin(); ss_p!=new_chunk->slots->end(); ss_p++ )
				{
					target_slot = smem_make_slot( (*p)->slots, ss_p->first );
					source_slot = ss_p->second;

					// for all values in the slot
					for ( s_p=source_slot->begin(); s_p!=source_slot->end(); s_p++ )
					{
						// copy each value
						target_slot->push_back( (*s_p) );
					}

					// once copied, we no longer need the slot
					delete source_slot;
				}

				// we no longer need the slots
				delete new_chunk->slots;
				new_chunk->slots = NULL;
			}

			// contents are new
			newbies->insert( (*p) );

			// deallocate
			smem_deallocate_chunk( my_agent, new_chunk );
		}
	}
	else
	{
		newbies->clear();
	}

	// de-allocate id name
	if ( chunk_name )
	{
		delete chunk_name;
	}

	return return_val;
}

bool smem_parse_chunks( agent *my_agent, const char *chunks, std::string **err_msg )
{
	bool return_val = false;
	unsigned long clause_count = 0;

	// parsing chunks requires an open semantic database
	smem_attach( my_agent );

	// copied primarily from cli_sp
	my_agent->alternate_input_string = chunks;
	my_agent->alternate_input_suffix = const_cast<char *>( ") " );
	my_agent->current_char = ' ';
	my_agent->alternate_input_exit = true;
	set_lexer_allow_ids( my_agent, true );
	get_lexeme( my_agent );

	if ( my_agent->lexeme.type == L_BRACE_LEXEME )
	{
		bool good_chunk = true;
		
		smem_str_to_chunk_map chunks;
		smem_str_to_chunk_map::iterator c_old;
		
		smem_chunk_set newbies;
		smem_chunk_set::iterator c_new;		

		// consume next token
		get_lexeme( my_agent );

		// while there are chunks to consume
		while ( ( my_agent->lexeme.type == L_PAREN_LEXEME ) && ( good_chunk ) )
		{
			good_chunk = smem_parse_chunk( my_agent, &( chunks ), &( newbies ) );

			if ( good_chunk )
			{
				// add all newbie lti's as appropriate
				for ( c_new=newbies.begin(); c_new!=newbies.end(); c_new++ )
				{
					if ( (*c_new)->lti_id == NIL )
					{					
						// deal differently with variable vs. lti
						if ( (*c_new)->lti_number == NIL )
						{
							// add a new lti id (we have a guarantee this won't be in Soar's WM)
							(*c_new)->lti_number = ( my_agent->id_counter[ (*c_new)->lti_letter - static_cast<unsigned long>('A') ]++ );
							(*c_new)->lti_id = smem_lti_add_id( my_agent, (*c_new)->lti_letter, (*c_new)->lti_number );
						}
						else
						{
							// should ALWAYS be the case (it's a newbie and we've initialized lti_id to NIL)
							if ( (*c_new)->lti_id == NIL )
							{
								// get existing
								(*c_new)->lti_id = smem_lti_get_id( my_agent, (*c_new)->lti_letter, (*c_new)->lti_number );

								// if doesn't exist, add it
								if ( (*c_new)->lti_id == NIL )
								{
									(*c_new)->lti_id = smem_lti_add_id( my_agent, (*c_new)->lti_letter, (*c_new)->lti_number );

									// this could affect an existing identifier in Soar's WM
									Symbol *id_parent = find_identifier( my_agent, (*c_new)->lti_letter, (*c_new)->lti_number );
									if ( id_parent != NIL )
									{
										// if so we make it an lti manually
										id_parent->id.smem_lti = (*c_new)->lti_id;

										id_parent->id.smem_time_id = my_agent->epmem_stats->time->get_value();
										id_parent->id.smem_valid = my_agent->epmem_validation;
									}
								}
							}
						}
					}
				}

				// add all newbie contents (append, as opposed to replace, children)
				for ( c_new=newbies.begin(); c_new!=newbies.end(); c_new++ )
				{
					if ( (*c_new)->slots != NIL )
					{
						smem_store_chunk( my_agent, (*c_new)->lti_id, (*c_new)->slots, false );
					}
				}

				// deallocate *contents* of all newbies (need to keep around name->id association for future chunks)
				for ( c_new=newbies.begin(); c_new!=newbies.end(); c_new++ )
				{
					smem_deallocate_chunk( my_agent, (*c_new), false );
				}

				// increment clause counter
				clause_count++;

				// clear newbie list
				newbies.clear();
			}
		};

		if ( good_chunk && ( my_agent->lexeme.type == R_BRACE_LEXEME ) )
		{
			// consume right brace
			get_lexeme( my_agent );

			// confirm (but don't consume) suffix
			return_val = ( my_agent->lexeme.type == R_PAREN_LEXEME );		
		}

		// deallocate all chunks
		{
			for ( c_old=chunks.begin(); c_old!=chunks.end(); c_old++ )
			{
				smem_deallocate_chunk( my_agent, c_old->second, true );
			}
		}
	}

	// produce error message on failure
	if ( !return_val )
	{
		std::string num;
		to_string( clause_count, num );

		(*err_msg) = new std::string( "Error parsing clause #" );
		(*err_msg)->append( num );
	}

	return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Implementation (smem::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_respond_to_cmd( agent *my_agent, bool store_only )
{
	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	smem_wme_list *wmes;
	smem_wme_list *cmds;
	smem_wme_list::iterator w_p;

	Symbol *query;
	Symbol *retrieve;
	smem_sym_list prohibit;
	smem_sym_list store;

	enum path_type { blank_slate, cmd_bad, cmd_retrieve, cmd_query, cmd_store } path;

	unsigned int time_slot = ( ( store_only )?(1):(0) );
	unsigned long wme_count;
	bool new_cue;

	int tc;	

	Symbol *parent_sym;
	std::queue<Symbol *> syms;

	int parent_level;
	std::queue<int> levels;	

	while ( state != NULL )
	{
		////////////////////////////////////////////////////////////////////////////
		my_agent->smem_timers->api->start();
		////////////////////////////////////////////////////////////////////////////

		// make sure this state has had some sort of change to the cmd
		// NOTE: we only care one-level deep!
		new_cue = false;
		wme_count = 0;
		cmds = NIL;
		{
			tc = get_new_tc_number( my_agent );

			// initialize BFS at command
			syms.push( state->id.smem_cmd_header );
			levels.push( 0 );			

			while ( !syms.empty() )
			{
				// get state
				parent_sym = syms.front();
				syms.pop();

				parent_level = levels.front();
				levels.pop();

				// get children of the current identifier
				wmes = smem_get_direct_augs_of_id( parent_sym, tc );
				{
					for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
					{
						if ( ( ( store_only ) && ( ( parent_level != 0 ) || ( ( (*w_p)->attr != my_agent->smem_sym_query ) && ( (*w_p)->attr != my_agent->smem_sym_retrieve ) ) ) ) || 
							 ( ( !store_only ) && ( ( parent_level != 0 ) || ( (*w_p)->attr != my_agent->smem_sym_store ) ) ) )
						{						
							wme_count++;

							if ( (*w_p)->timetag > state->id.smem_info->last_cmd_time[ time_slot ] )
							{
								new_cue = true;
								state->id.smem_info->last_cmd_time[ time_slot ] = (*w_p)->timetag;
							}

							if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
								 ( parent_level == 0 ) &&
								 ( ( (*w_p)->attr == my_agent->smem_sym_query ) || ( (*w_p)->attr == my_agent->smem_sym_store ) ) )								 
							{
								syms.push( (*w_p)->value );
								levels.push( parent_level + 1 );
							}
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
			if ( state->id.smem_info->last_cmd_count[ time_slot ] != wme_count )
			{
				new_cue = true;
				state->id.smem_info->last_cmd_count[ time_slot ] = wme_count;
			}
			

			if ( new_cue )
			{
				// clear old cue
				state->id.smem_info->cue_wmes->clear();

				// clear old results
				smem_clear_result( my_agent, state );

				// change is afoot!
				my_agent->smem_made_changes = true;
			}
		}		

		// a command is issued if the cue is new
		// and there is something on the cue
		if ( new_cue && wme_count )
		{
			// initialize command vars
			retrieve = NIL;
			query = NIL;
			store.clear();
			prohibit.clear();
			path = blank_slate;

			// process top-level symbols
			for ( w_p=cmds->begin(); w_p!=cmds->end(); w_p++ )
			{
				state->id.smem_info->cue_wmes->insert( (*w_p) );

				if ( path != cmd_bad )
				{
					// collect information about known commands
					if ( (*w_p)->attr == my_agent->smem_sym_retrieve )
					{
						if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( path == blank_slate ) )
						{
							retrieve = (*w_p)->value;
							path = cmd_retrieve;
						}
						else
						{
							path = cmd_bad;
						}
					}
					else if ( (*w_p)->attr == my_agent->smem_sym_query )
					{
						if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == blank_slate ) || ( path == cmd_query ) ) &&
							 ( query == NIL ) )

						{
							query = (*w_p)->value;
							path = cmd_query;
						}
						else
						{
							path = cmd_bad;
						}
					}
					else if ( (*w_p)->attr == my_agent->smem_sym_prohibit )
					{
						if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == blank_slate ) || ( path == cmd_query ) ) &&
							 ( (*w_p)->value->id.smem_lti != NIL ) )
						{
							prohibit.push_back( (*w_p)->value );
							path = cmd_query;
						}
						else
						{
							path = cmd_bad;
						}
					}
					else if ( (*w_p)->attr == my_agent->smem_sym_store )
					{
						if ( ( (*w_p)->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
							 ( ( path == blank_slate ) || ( path == cmd_store ) ) )
						{
							store.push_back( (*w_p)->value );
							path = cmd_store;
						}
						else
						{
							path = cmd_bad;
						}
					}
					else
					{
						path = cmd_bad;
					}
				}
			}

			// if on path 3 must have query/neg-query
			if ( ( path == cmd_query ) && ( query == NULL ) )
			{
				path = cmd_bad;
			}

			// must be on a path
			if ( path == blank_slate )
			{
				path = cmd_bad;
			}

			////////////////////////////////////////////////////////////////////////////
			my_agent->smem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////

			// process command
			if ( path != cmd_bad )
			{
				// performing any command requires an initialized database
				smem_attach( my_agent );

				// retrieve
				if ( path == cmd_retrieve )
				{
					if ( retrieve->id.smem_lti == NIL )
					{
						// retrieve is not pointing to an lti!
						smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_failure, retrieve );
					}
					else
					{
						// status: success
						smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_success, retrieve );

						// install memory directly onto the retrieve identifier
						smem_install_memory( my_agent, state, retrieve->id.smem_lti, retrieve );

						// add one to the expansions stat
						my_agent->smem_stats->expansions->set_value( my_agent->smem_stats->expansions->get_value() + 1 );
					}
				}
				// query
				else if ( path == cmd_query )
				{
					smem_lti_set prohibit_lti;
					smem_sym_list::iterator sym_p;

					for ( sym_p=prohibit.begin(); sym_p!=prohibit.end(); sym_p++ )
					{
						prohibit_lti.insert( (*sym_p)->id.smem_lti );
					}

					smem_process_query( my_agent, state, query, &( prohibit_lti ) );

					// add one to the cbr stat
					my_agent->smem_stats->cbr->set_value( my_agent->smem_stats->cbr->get_value() + 1 );
				}
				else if ( path == cmd_store )
				{
					smem_sym_list::iterator sym_p;

					////////////////////////////////////////////////////////////////////////////
					my_agent->smem_timers->storage->start();
					////////////////////////////////////////////////////////////////////////////

					// start transaction (if not lazy)
					if ( my_agent->smem_params->lazy_commit->get_value() == soar_module::off )
					{
						my_agent->smem_stmts->begin->execute( soar_module::op_reinit );
					}

					for ( sym_p=store.begin(); sym_p!=store.end(); sym_p++ )
					{
						smem_soar_store( my_agent, (*sym_p) );

						// status: success
						smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_success, (*sym_p) );

						// add one to the store stat
						my_agent->smem_stats->stores->set_value( my_agent->smem_stats->stores->get_value() + 1 );
					}

					// commit transaction (if not lazy)
					if ( my_agent->smem_params->lazy_commit->get_value() == soar_module::off )
					{
						my_agent->smem_stmts->commit->execute( soar_module::op_reinit );
					}

					////////////////////////////////////////////////////////////////////////////
					my_agent->smem_timers->storage->stop();
					////////////////////////////////////////////////////////////////////////////
				}
			}
			else
			{
				smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_bad_cmd, state->id.smem_cmd_header );
			}
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////
			my_agent->smem_timers->api->stop();
			////////////////////////////////////////////////////////////////////////////
		}

		// free space from aug list
		delete cmds;

		state = state->id.higher_goal;
	}
}

void smem_go( agent *my_agent, bool store_only )
{
	// after we are done we will perform a wm phase
	// if any adds/removes
	my_agent->smem_made_changes = false;
	
	my_agent->smem_timers->total->start();

#ifndef SMEM_EXPERIMENT

	smem_respond_to_cmd( my_agent, store_only );

#else // SMEM_EXPERIMENT

#endif // SMEM_EXPERIMENT

	my_agent->smem_timers->total->stop();

	if ( my_agent->smem_made_changes )
	{
		do_working_memory_phase( my_agent );
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Visualization (smem::viz)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_visualize_store( agent *my_agent, std::string *return_val )
{
	// vizualizing the store requires an open semantic database
	smem_attach( my_agent );

	// header
	return_val->append( "digraph smem {" );
	return_val->append( "\n" );

	// LTIs
	return_val->append( "node [ shape = doublecircle ];" );
	return_val->append( "\n" );

	std::map< smem_lti_id, std::string > lti_names;
	std::map< smem_lti_id, std::string >::iterator n_p;
	{
		soar_module::sqlite_statement *q;

		smem_lti_id lti_id;
		char lti_letter;
		unsigned long lti_number;

		std::string *lti_name;
		std::string temp_str;
		intptr_t temp_int;
		double temp_double;

		// id, letter, number
		q = my_agent->smem_stmts->vis_lti;
		while ( q->execute() == soar_module::row )
		{
			lti_id = q->column_int( 0 );
			lti_letter = static_cast<char>( q->column_int( 1 ) );
			lti_number = static_cast<unsigned long>( q->column_int( 2 ) );

			lti_name =& lti_names[ lti_id ];
			lti_name->push_back( lti_letter );

			to_string( lti_number, temp_str );
			lti_name->append( temp_str );

			return_val->append( (*lti_name) );
			return_val->append( " " );
		}
		q->reinitialize();

		if ( !lti_names.empty() )
		{
			// terminal nodes first
			{
				std::map< smem_lti_id, std::list<std::string> > lti_terminals;
				std::map< smem_lti_id, std::list<std::string> >::iterator t_p;
				std::list<std::string>::iterator a_p;

				std::list<std::string> *my_terminals;
				std::list<std::string>::size_type terminal_num;

				return_val->append( ";" );
				return_val->append( "\n" );

				// proceed to terminal nodes
				return_val->append( "node [ shape = plaintext ];" );
				return_val->append( "\n" );
				
				// parent_id, attr_type, attr_hash, val_type, val_hash
				q = my_agent->smem_stmts->vis_value_const;
				while ( q->execute() == soar_module::row )
				{
					lti_id = q->column_int( 0 );
					my_terminals =& lti_terminals[ lti_id ];
					lti_name =& lti_names[ lti_id ];

					// parent prefix
					return_val->append( (*lti_name) );
					return_val->append( "_" );

					// terminal count
					terminal_num = my_terminals->size();
					to_string( terminal_num, temp_str );
					return_val->append( temp_str );

					// prepare for value
					return_val->append( " [ label = \"" );

					// output value
					{
						switch ( q->column_int( 3 ) )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								smem_reverse_hash_str( my_agent, q->column_int(4), temp_str );								
								break;

							case INT_CONSTANT_SYMBOL_TYPE:
								temp_int = smem_reverse_hash_int( my_agent, q->column_int(4) );
								to_string( temp_int, temp_str );
								break;

							case FLOAT_CONSTANT_SYMBOL_TYPE:
								temp_double = smem_reverse_hash_float( my_agent, q->column_int(4) );
								to_string( temp_double, temp_str );
								break;

							default:
								temp_str.clear();
								break;
						}

						return_val->append( temp_str );
					}

					// store terminal (attribute for edge label)
					{
						switch ( q->column_int(1) )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								smem_reverse_hash_str( my_agent, q->column_int(2), temp_str );								
								break;

							case INT_CONSTANT_SYMBOL_TYPE:
								temp_int = smem_reverse_hash_int( my_agent, q->column_int(2) );
								to_string( temp_int, temp_str );
								break;

							case FLOAT_CONSTANT_SYMBOL_TYPE:
								temp_double = smem_reverse_hash_float( my_agent, q->column_int(2) );
								to_string( temp_double, temp_str );
								break;

							default:
								temp_str.clear();
								break;
						}

						my_terminals->push_back( temp_str );
					}

					// footer
					return_val->append( "\" ];" );
					return_val->append( "\n" );
				}
				q->reinitialize();

				// output edges
				{
					unsigned int terminal_counter;

					for ( n_p=lti_names.begin(); n_p!=lti_names.end(); n_p++ )
					{
						t_p = lti_terminals.find( n_p->first );

						if ( t_p != lti_terminals.end() )
						{
							terminal_counter = 0;

							for ( a_p=t_p->second.begin(); a_p!=t_p->second.end(); a_p++ )
							{
								return_val->append( n_p->second );
								return_val ->append( " -> " );
								return_val->append( n_p->second );
								return_val->append( "_" );

								to_string( terminal_counter, temp_str );
								return_val->append( temp_str );
								return_val->append( " [ label=\"" );

								return_val->append( (*a_p) );

								return_val->append( "\" ];" );
								return_val->append( "\n" );

								terminal_counter++;
							}
						}
					}
				}
			}

			// then links to other LTIs
			{
				// parent_id, attr_type, attr_hash, val_lti
				q = my_agent->smem_stmts->vis_value_lti;
				while ( q->execute() == soar_module::row )
				{
					// source
					lti_id = q->column_int( 0 );
					lti_name =& lti_names[ lti_id ];
					return_val->append( (*lti_name) );
					return_val->append( " -> " );

					// destination
					lti_id = q->column_int( 3 );
					lti_name =& lti_names[ lti_id ];
					return_val->append( (*lti_name) );
					return_val->append( " [ label =\"" );

					// output attribute
					{
						switch ( q->column_int(1) )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								smem_reverse_hash_str( my_agent, q->column_int(2), temp_str );								
								break;

							case INT_CONSTANT_SYMBOL_TYPE:
								temp_int = smem_reverse_hash_int( my_agent, q->column_int(2) );
								to_string( temp_int, temp_str );
								break;

							case FLOAT_CONSTANT_SYMBOL_TYPE:
								temp_double = smem_reverse_hash_float( my_agent, q->column_int(2) );
								to_string( temp_double, temp_str );
								break;

							default:
								temp_str.clear();
								break;
						}

						return_val->append( temp_str );
					}

					// footer
					return_val->append( "\" ];" );
					return_val->append( "\n" );
				}
				q->reinitialize();
			}
		}
	}

	// footer
	return_val->append( "}" );
	return_val->append( "\n" );
}

void smem_visualize_lti( agent *my_agent, smem_lti_id lti_id, unsigned long depth, std::string *return_val )
{
	soar_module::sqlite_statement* expand_q = my_agent->smem_stmts->web_expand;

	unsigned long child_counter;

	std::string temp_str;
	std::string temp_str2;
	intptr_t temp_int;
	double temp_double;

	std::queue<smem_vis_lti *> bfs;
	smem_vis_lti *new_lti;
	smem_vis_lti *parent_lti;

	std::set<smem_lti_id> close_list;
	std::set<smem_lti_id>::iterator cl_p;

	// header
	return_val->append( "digraph smem_lti {" );
	return_val->append( "\n" );

	// root
	{
		new_lti = new smem_vis_lti;
		new_lti->lti_id = lti_id;
		new_lti->level = 0;

		// fake former linkage
		{
			soar_module::sqlite_statement *lti_q = my_agent->smem_stmts->lti_letter_num;

			// get just this lti
			lti_q->bind_int( 1, lti_id );
			lti_q->execute();

			// letter
			new_lti->lti_name.push_back( static_cast<char>( lti_q->column_int( 0 ) ) );

			// number
			temp_int = lti_q->column_int( 1 );
			to_string( temp_int, temp_str );
			new_lti->lti_name.append( temp_str );

			// done with lookup
			lti_q->reinitialize();

			// output without linkage
			return_val->append( "node [ shape = doublecircle ];" );
			return_val->append( "\n" );

			return_val->append( new_lti->lti_name );
			return_val->append( ";" );
			return_val->append( "\n" );
		}

		bfs.push( new_lti );
		new_lti = NULL;

		close_list.insert( lti_id );
	}

	// optionally depth-limited breadth-first-search of children
	while ( !bfs.empty() )
	{
		parent_lti = bfs.front();
		bfs.pop();

		child_counter = 0;
		
		// get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
		expand_q->bind_int( 1, parent_lti->lti_id );
		while ( expand_q->execute() == soar_module::row )
		{
			// identifier vs. constant
			if ( expand_q->column_type( 6 ) != soar_module::null_t )
			{
				new_lti = new smem_vis_lti;
				new_lti->lti_id = expand_q->column_int( 6 );
				new_lti->level = ( parent_lti->level + 1 );

				// add node
				{
					// letter
					new_lti->lti_name.push_back( static_cast<char>( expand_q->column_int( 4 ) ) );

					// number
					temp_int = expand_q->column_int( 5 );
					to_string( temp_int, temp_str );
					new_lti->lti_name.append( temp_str );

					// output node
					return_val->append( "node [ shape = doublecircle ];" );
					return_val->append( "\n" );

					return_val->append( new_lti->lti_name );
					return_val->append( ";" );
					return_val->append( "\n" );
				}


				// add linkage
				{
					// get attribute
					switch ( expand_q->column_int(0) )
					{
						case SYM_CONSTANT_SYMBOL_TYPE:
							smem_reverse_hash_str( my_agent, expand_q->column_int(1), temp_str );							
							break;

						case INT_CONSTANT_SYMBOL_TYPE:
							temp_int = smem_reverse_hash_int( my_agent, expand_q->column_int(1) );
							to_string( temp_int, temp_str );
							break;

						case FLOAT_CONSTANT_SYMBOL_TYPE:
							temp_double = smem_reverse_hash_float( my_agent, expand_q->column_int(1) );
							to_string( temp_double, temp_str );
							break;

						default:
							temp_str.clear();
							break;
					}

					// output linkage
					return_val->append( parent_lti->lti_name );
					return_val->append( " -> " );
					return_val->append( new_lti->lti_name );
					return_val->append( " [ label = \"" );
					return_val->append( temp_str );
					return_val->append( "\" ];" );
					return_val->append( "\n" );
				}

				// add to bfs (if still in depth limit)
				if ( ( depth == 0 ) || ( new_lti->level < depth ) )
				{
					// prevent looping
					cl_p = close_list.find( new_lti->lti_id );
					if ( cl_p == close_list.end() )
					{
						close_list.insert( new_lti->lti_id );						
						bfs.push( new_lti );
					}
					else
					{
						delete new_lti;
					}				
				}
				else
				{
					delete new_lti;
				}

				new_lti = NULL;
			}
			else
			{
				// add value node
				{
					// get node name
					{
						temp_str2.assign( parent_lti->lti_name );
						temp_str2.append( "_" );

						to_string( child_counter, temp_str );
						temp_str2.append( temp_str );
					}

					// get value
					switch ( expand_q->column_int(2) )
					{
						case SYM_CONSTANT_SYMBOL_TYPE:
							smem_reverse_hash_str( my_agent, expand_q->column_int(3), temp_str );							
							break;

						case INT_CONSTANT_SYMBOL_TYPE:
							temp_int = smem_reverse_hash_int( my_agent, expand_q->column_int(3) );
							to_string( temp_int, temp_str );
							break;

						case FLOAT_CONSTANT_SYMBOL_TYPE:
							temp_double = smem_reverse_hash_float( my_agent, expand_q->column_int(3) );
							to_string( temp_double, temp_str );
							break;

						default:
							temp_str.clear();
							break;
					}

					// output node
					return_val->append( "node [ shape = plaintext ];" );
					return_val->append( "\n" );
					return_val->append( temp_str2 );
					return_val->append( " [ label=\"" );
					return_val->append( temp_str );
					return_val->append( "\" ];" );
					return_val->append( "\n" );
				}

				// add linkage
				{
					// get attribute
					switch ( expand_q->column_int(0) )
					{
						case SYM_CONSTANT_SYMBOL_TYPE:
							smem_reverse_hash_str( my_agent, expand_q->column_int(1), temp_str );
							break;

						case INT_CONSTANT_SYMBOL_TYPE:
							temp_int = smem_reverse_hash_int( my_agent, expand_q->column_int(1) );
							to_string( temp_int, temp_str );
							break;

						case FLOAT_CONSTANT_SYMBOL_TYPE:
							temp_double = smem_reverse_hash_float( my_agent, expand_q->column_int(1) );
							to_string( temp_double, temp_str );
							break;

						default:
							temp_str.clear();
							break;
					}

					// output linkage
					return_val->append( parent_lti->lti_name );
					return_val->append( " -> " );
					return_val->append( temp_str2 );
					return_val->append( " [ label = \"" );
					return_val->append( temp_str );
					return_val->append( "\" ];" );
					return_val->append( "\n" );
				}

				child_counter++;
			}
		}
		expand_q->reinitialize();

		delete parent_lti;
	}

	// footer
	return_val->append( "}" );
	return_val->append( "\n" );
}

