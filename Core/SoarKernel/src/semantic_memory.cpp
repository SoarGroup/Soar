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

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (smem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_param_container::smem_param_container( agent *new_agent ): param_container( new_agent )
{
	// learning
	learning = new boolean_param( "learning", on, new f_predicate<boolean>() );
	add( learning );

	// database
	database = new constant_param<db_choices>( "database", memory, new smem_db_predicate<db_choices>( my_agent ) );
	database->add_mapping( memory, "memory" );
	database->add_mapping( file, "file" );
	add( database );

	// path
	path = new smem_path_param( "path", "", new predicate<const char *>(), new smem_db_predicate<const char *>( my_agent ), my_agent );
	add( path );

	// commit
	commit = new integer_param( "commit", 1, new gt_predicate<long>( 1, true ), new f_predicate<long>() );
	add( commit );	

	// timers
	timers = new constant_param<soar_module::timer::timer_level>( "timers", soar_module::timer::zero, new f_predicate<soar_module::timer::timer_level>() );
	timers->add_mapping( soar_module::timer::zero, "off" );
	timers->add_mapping( soar_module::timer::one, "one" );
	timers->add_mapping( soar_module::timer::two, "two" );	
	add( timers );
}

//

smem_path_param::smem_path_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred, agent *new_agent ): string_param( new_name, new_value, new_val_pred, new_prot_pred ), my_agent( new_agent ) {}

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
smem_db_predicate<T>::smem_db_predicate( agent *new_agent ): agent_predicate<T>( new_agent ) {}

template <typename T>
bool smem_db_predicate<T>::operator() ( T /*val*/ ) { return ( this->my_agent->smem_db->get_status() == soar_module::connected ); }


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (smem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_stat_container::smem_stat_container( agent *new_agent ): stat_container( new_agent )
{
	// next-id
	next_id = new integer_stat( "next-id", 0, new smem_db_predicate<long>( my_agent ) );
	add( next_id );

	// mem-usage
	mem_usage = new smem_mem_usage_stat( my_agent, "mem-usage", 0, new predicate<long>() );
	add( mem_usage );

	// mem-high
	mem_high = new smem_mem_high_stat( my_agent, "mem-high", 0, new predicate<long>() );
	add( mem_high );
}

//

smem_mem_usage_stat::smem_mem_usage_stat( agent *new_agent, const char *new_name, long new_value, predicate<long> *new_prot_pred ): integer_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

long smem_mem_usage_stat::get_value()
{
	return my_agent->smem_db->memory_usage();
}

//

smem_mem_high_stat::smem_mem_high_stat( agent *new_agent, const char *new_name, long new_value, predicate<long> *new_prot_pred ): integer_stat( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

long smem_mem_high_stat::get_value()
{
	return my_agent->smem_db->memory_highwater();
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (smem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_timer_container::smem_timer_container( agent *new_agent ): timer_container( new_agent )
{
	// one
	
	total = new smem_timer( "_total", my_agent, timer::one );
	add( total );

	// two

	storage = new smem_timer( "smem_storage", my_agent, timer::two );
	add( storage );

	ncb_retrieval = new smem_timer( "smem_ncb_retrieval", my_agent, timer::two );
	add( ncb_retrieval );

	query = new smem_timer( "smem_query", my_agent, timer::two );
	add( query );

	api = new smem_timer( "smem_api", my_agent, timer::two );
	add( api );

	init = new smem_timer( "smem_init", my_agent, timer::two );
	add( init );

	next = new smem_timer( "smem_next", my_agent, timer::two );
	add( next );

	prev = new smem_timer( "smem_prev", my_agent, timer::two );
	add( prev );

	hash = new smem_timer( "smem_hash", my_agent, timer::two );
	add( hash );
}

//

smem_timer_level_predicate::smem_timer_level_predicate( agent *new_agent ): agent_predicate<soar_module::timer::timer_level>( new_agent ) {}

bool smem_timer_level_predicate::operator() ( soar_module::timer::timer_level val ) { return ( my_agent->smem_params->timers->get_value() >= val ); }

//

smem_timer::smem_timer(const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level): soar_module::timer( new_name, new_agent, new_level, new smem_timer_level_predicate( new_agent ) ) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statement Functions (smem::statements)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_statement_container::smem_statement_container( agent *new_agent ): sqlite_statement_container( new_agent->smem_db )
{
	sqlite_database *new_db = new_agent->smem_db;

	//

	add_structure( "CREATE TABLE IF NOT EXISTS vars (id INTEGER PRIMARY KEY,value NONE)" );	
	add_structure( "CREATE TABLE IF NOT EXISTS temporal_symbol_hash (id INTEGER PRIMARY KEY, sym_const NONE, sym_type INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS temporal_symbol_hash_const_type ON temporal_symbol_hash (sym_type,sym_const)" );

	//
	
	begin = new sqlite_statement( new_db, "BEGIN" );
	add( begin );

	commit = new sqlite_statement( new_db, "COMMIT" );
	add( commit );

	rollback = new sqlite_statement( new_db, "ROLLBACK" );
	add( rollback );

	//

	var_get = new sqlite_statement( new_db, "SELECT value FROM vars WHERE id=?" );
	add( var_get );

	var_set = new sqlite_statement( new_db, "REPLACE INTO vars (id,value) VALUES (?,?)" );
	add( var_set );

	//

	hash_get = new sqlite_statement( new_db, "SELECT id FROM temporal_symbol_hash WHERE sym_type=? AND sym_const=?" );
	add( hash_get );

	hash_add = new sqlite_statement( new_db, "INSERT INTO temporal_symbol_hash (sym_type,sym_const) VALUES (?,?)" );
	add( hash_add );
}

