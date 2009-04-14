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

#include <list>
#include <map>
#include <queue>


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
// api							smem::api


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

	//

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


bool smem_enabled( agent *my_agent )
{
	return ( my_agent->smem_params->learning->get_value() == soar_module::on );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (smem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_stat_container::smem_stat_container( agent *new_agent ): stat_container( new_agent )
{
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

	add_structure( "CREATE TABLE IF NOT EXISTS lti (id INTEGER PRIMARY KEY, letter INTEGER, num INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS lti_letter_num ON lti (letter, num)" );

	add_structure( "CREATE TABLE IF NOT EXISTS web (parent_id INTEGER, attr INTEGER, val_const INTEGER, val_lti INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS web_parent_attr_val_lti ON web (parent_id, attr, val_const, val_lti)" );
	add_structure( "CREATE INDEX IF NOT EXISTS web_attr_val_lti ON web (attr, val_const, val_lti)" );

	add_structure( "CREATE TABLE IF NOT EXISTS ct_attr (attr INTEGER PRIMARY KEY, ct INTEGER)" );

	add_structure( "CREATE TABLE IF NOT EXISTS ct_const (attr INTEGER, val_const INTEGER, ct INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS ct_const_attr_val ON ct_const (attr, val_const)" );

	add_structure( "CREATE TABLE IF NOT EXISTS ct_lti (attr INTEGER, val_lti INTEGER, ct INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS ct_lti_attr_val ON ct_lti (attr, val_lti)" );

	add_structure( "CREATE TABLE IF NOT EXISTS activation (lti INTEGER PRIMARY KEY, cycle INTEGER)" );
	add_structure( "CREATE UNIQUE INDEX IF NOT EXISTS activation_cycle ON activation (cycle)" );
	add_structure( "INSERT OR IGNORE INTO activation (lti,cycle) VALUES (0,0)" );

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

	//

	lti_add = new sqlite_statement( new_db, "INSERT INTO lti (letter,num) VALUES (?,?)" );
	add( lti_add );

	lti_get = new sqlite_statement( new_db, "SELECT id FROM lti WHERE letter=? AND num=?" );
	add( lti_get );

	lti_letter_num = new sqlite_statement( new_db, "SELECT letter, num FROM lti WHERE id=?" );
	add( lti_letter_num );

	lti_max = new sqlite_statement( new_db, "SELECT letter, MAX(num) FROM lti GROUP BY letter" );
	add( lti_max );

	//

	web_add = new sqlite_statement( new_db, "INSERT INTO web (parent_id, attr, val_const, val_lti) VALUES (?,?,?,?)" );
	add( web_add );

	web_truncate = new sqlite_statement( new_db, "DELETE FROM web WHERE parent_id=?" );
	add( web_truncate );

	web_expand = new sqlite_statement( new_db, "SELECT tsh_a.sym_const AS attr_const, tsh_a.sym_type AS attr_type, vcl.sym_const AS value_const, vcl.sym_type AS value_type, vcl.letter AS value_letter, vcl.num AS value_num, vcl.val_lti AS value_lti FROM ((web w LEFT JOIN temporal_symbol_hash tsh_v ON w.val_const=tsh_v.id) vc LEFT JOIN lti ON vc.val_lti=lti.id) vcl INNER JOIN temporal_symbol_hash tsh_a ON vcl.attr=tsh_a.id WHERE parent_id=?" );
	add( web_expand );

	//

	web_attr_ct = new sqlite_statement( new_db, "SELECT attr, COUNT(*) AS ct FROM web WHERE parent_id=? GROUP BY attr" );
	add( web_attr_ct );

	web_const_ct = new sqlite_statement( new_db, "SELECT attr, val_const, COUNT(*) AS ct FROM web WHERE parent_id=? AND val_const IS NOT NULL GROUP BY attr, val_const" );
	add( web_const_ct );

	web_lti_ct = new sqlite_statement( new_db, "SELECT attr, val_lti, COUNT(*) AS ct FROM web WHERE parent_id=? AND val_const IS NULL GROUP BY attr, val_const, val_lti" );
	add( web_lti_ct );

	//

	web_attr_all = new sqlite_statement( new_db, "SELECT parent_id FROM web w INNER JOIN activation a ON w.parent_id=a.lti WHERE attr=? ORDER BY cycle DESC" );
	add( web_attr_all );

	web_const_all = new sqlite_statement( new_db, "SELECT parent_id FROM web w INNER JOIN activation a ON w.parent_id=a.lti WHERE attr=? AND val_const=? ORDER BY cycle DESC" );
	add( web_const_all );

	web_lti_all = new sqlite_statement( new_db, "SELECT parent_id FROM web w INNER JOIN activation a ON w.parent_id=a.lti WHERE attr=? AND val_const IS NULL AND val_lti=? ORDER BY cycle DESC" );
	add( web_lti_all );

	//

	web_attr_child = new sqlite_statement( new_db, "SELECT parent_id FROM web WHERE parent_id=? AND attr=?" );
	add( web_attr_child );

	web_const_child = new sqlite_statement( new_db, "SELECT parent_id FROM web WHERE parent_id=? AND attr=? AND val_const=?" );
	add( web_const_child );

	web_lti_child = new sqlite_statement( new_db, "SELECT parent_id FROM web WHERE parent_id=? AND attr=? AND val_const IS NULL AND val_lti=?" );
	add( web_lti_child );

	//

	ct_attr_add = new sqlite_statement( new_db, "INSERT OR IGNORE INTO ct_attr (attr, ct) VALUES (?,0)" );
	add( ct_attr_add );

	ct_const_add = new sqlite_statement( new_db, "INSERT OR IGNORE INTO ct_const (attr, val_const, ct) VALUES (?,?,0)" );
	add( ct_const_add );

	ct_lti_add = new sqlite_statement( new_db, "INSERT OR IGNORE INTO ct_lti (attr, val_lti, ct) VALUES (?,?,0)" );
	add( ct_lti_add );

	//

	ct_attr_update = new sqlite_statement( new_db, "UPDATE ct_attr SET ct = ct + ? WHERE attr=?" );
	add( ct_attr_update );

	ct_const_update = new sqlite_statement( new_db, "UPDATE ct_const SET ct = ct + ? WHERE attr=? AND val_const=?" );
	add( ct_const_update );

	ct_lti_update = new sqlite_statement( new_db, "UPDATE ct_lti SET ct = ct + ? WHERE attr=? AND val_lti=?" );
	add( ct_lti_update );

	//

	ct_attr_get = new sqlite_statement( new_db, "SELECT ct FROM ct_attr WHERE attr=?" );
	add( ct_attr_get );

	ct_const_get = new sqlite_statement( new_db, "SELECT ct FROM ct_const WHERE attr=? AND val_const=?" );
	add( ct_const_get );

	ct_lti_get = new sqlite_statement( new_db, "SELECT ct FROM ct_lti WHERE attr=? AND val_lti=?" );
	add( ct_lti_get );

	//

	act_set = new sqlite_statement( new_db, "REPLACE INTO activation (lti, cycle) VALUES (?,(SELECT MAX(cycle)+1 FROM activation))" );
	add( act_set );
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

bool smem_symbol_is_constant( Symbol *sym )
{
	return ( ( sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) ||
		     ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) ||
		     ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) );
}

//

inline void smem_symbol_to_bind( Symbol *sym, sqlite_statement *q, int type_field, int val_field )
{		
	q->bind_int( type_field, sym->common.symbol_type );
	switch ( sym->common.symbol_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
			q->bind_text( val_field, (const char *) sym->sc.name );
			break;

		case INT_CONSTANT_SYMBOL_TYPE:
			q->bind_int( val_field, sym->ic.value );
			break;

		case FLOAT_CONSTANT_SYMBOL_TYPE:
			q->bind_double( val_field, sym->fc.value );
			break;
	}
}

inline Symbol *smem_statement_to_symbol( agent *my_agent, sqlite_statement *q, int type_field, int val_field )
{
	Symbol *return_val = NULL;

	switch ( q->column_int( type_field ) )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
			return_val = make_sym_constant( my_agent, const_cast<char *>( (const char *) q->column_text( val_field ) ) );
			break;

		case INT_CONSTANT_SYMBOL_TYPE:
			return_val = make_int_constant( my_agent, q->column_int( val_field ) );
			break;

		case FLOAT_CONSTANT_SYMBOL_TYPE:
			return_val = make_float_constant( my_agent, q->column_double( val_field ) );
			break;

		default:
			return_val = NULL;
			break;
	}

	return return_val;
}

//

void _smem_add_wme( agent *my_agent, Symbol *state, Symbol *id, Symbol *attr, Symbol *value, bool meta )
{		
	wme *w = soar_module::add_module_wme( my_agent, id, attr, value );
	w->preference = soar_module::make_fake_preference( my_agent, state, w, state->id.smem_info->cue_wmes );

	if ( w->preference )
		add_preference_to_tm( my_agent, w->preference );

	if ( meta )
		state->id.smem_info->smem_wmes->push( w );
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

/***************************************************************************
 * Function     : smem_get_variable
 * Author		: Nate Derbinsky
 * Notes		: Gets an SMem variable from the database
 **************************************************************************/
bool smem_variable_get( agent *my_agent, smem_variable_key variable_id, long *variable_value )
{
	soar_module::exec_result status;
	soar_module::sqlite_statement *var_get = my_agent->smem_stmts->var_get;

	var_get->bind_int( 1, variable_id );
	status = var_get->execute();	

	if ( status == soar_module::row )
		(*variable_value) = var_get->column_int( 0 );

	var_get->reinitialize();

	return ( status == soar_module::row );
}

/***************************************************************************
 * Function     : smem_set_variable
 * Author		: Nate Derbinsky
 * Notes		: Sets an EpMem variable in the database
 **************************************************************************/
void smem_variable_set( agent *my_agent, smem_variable_key variable_id, long variable_value )
{
	soar_module::sqlite_statement *var_set = my_agent->smem_stmts->var_set;

	var_set->bind_int( 1, variable_id );
	var_set->bind_int( 2, variable_value );

	var_set->execute( soar_module::op_reinit );	
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Temporal Hash Functions (smem::hash)
//
// The rete has symbol hashing, but the values are
// reliable only for the lifetime of a symbol.  This
// isn't good for SMem.  Hence, we implement a simple
// lookup table, relying upon SQLite to deal with
// efficiency issues.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_temporal_hash
 * Author		: Nate Derbinsky
 * Notes		: Returns a temporally unique integer representing
 *                a symbol constant.
 **************************************************************************/
long smem_temporal_hash( agent *my_agent, Symbol *sym, bool add_on_fail = true )
{
	long return_val = NULL;

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->hash->start();
	////////////////////////////////////////////////////////////////////////////
	
	if ( smem_symbol_is_constant( sym ) )
	{
		if ( ( !sym->common.smem_hash ) || ( sym->common.smem_valid != my_agent->smem_validation ) )
		{		
			sym->common.smem_hash = NULL;
			sym->common.smem_valid = my_agent->smem_validation;
			
			// basic process:
			// - search
			// - if found, return
			// - else, add

			smem_symbol_to_bind( sym, my_agent->smem_stmts->hash_get, 1, 2 );
			if ( my_agent->smem_stmts->hash_get->execute() == soar_module::row )
			{
				return_val = my_agent->smem_stmts->hash_get->column_int( 0 );
			}
			my_agent->smem_stmts->hash_get->reinitialize();

			//

			if ( !return_val && add_on_fail )
			{
				smem_symbol_to_bind( sym, my_agent->smem_stmts->hash_add, 1, 2 );				
				my_agent->smem_stmts->hash_add->execute( soar_module::op_reinit );
				return_val = (long) my_agent->smem_db->last_insert_rowid();
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


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Long-Term Identifier Functions (smem::lti)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_lti_id smem_lti_add( agent *my_agent, Symbol *id )
{		
	if ( ( id->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) &&
		 ( id->id.smem_lti == NIL ) )
	{
		// try to find existing lti
		{
			// letter=? AND number=?
			my_agent->smem_stmts->lti_get->bind_int( 1, (unsigned long) id->id.name_letter );
			my_agent->smem_stmts->lti_get->bind_int( 2, (unsigned long) id->id.name_number );

			if ( my_agent->smem_stmts->lti_get->execute() == soar_module::row )
			{
				id->id.smem_lti = my_agent->smem_stmts->lti_get->column_int( 0 );
			}
			
			my_agent->smem_stmts->lti_get->reinitialize();
		}

		// if doesn't exist, add
		if ( id->id.smem_lti == NIL )
		{
			// create lti: letter, number
			my_agent->smem_stmts->lti_add->bind_int( 1, (unsigned long) id->id.name_letter );
			my_agent->smem_stmts->lti_add->bind_int( 2, (unsigned long) id->id.name_number );
			my_agent->smem_stmts->lti_add->execute( soar_module::op_reinit );

			id->id.smem_lti = (smem_lti_id) my_agent->smem_db->last_insert_rowid();
		}
	}

	return id->id.smem_lti;
}

void smem_lti_activate( agent *my_agent, smem_lti_id lti )
{	
	my_agent->smem_stmts->act_set->bind_int( 1, lti );
	my_agent->smem_stmts->act_set->execute( soar_module::op_reinit );
}

Symbol *smem_lti_make( agent *my_agent, char name_letter, unsigned long name_number, goal_stack_level level )
{
	Symbol *return_val;

	// try to find existing
	return_val = find_identifier( my_agent, name_letter, name_number );
	if ( return_val == NIL )
	{
		return_val = make_new_identifier( my_agent, name_letter, level, name_number );	
	}

	return return_val;
}

void smem_reset_id_counters( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() == soar_module::connected )
	{
		// letter, max
		while ( my_agent->smem_stmts->lti_max->execute() == soar_module::row )
		{
			unsigned long name_letter = (unsigned long) my_agent->smem_stmts->lti_max->column_int( 0 );
			unsigned long letter_max = (unsigned long) my_agent->smem_stmts->lti_max->column_int( 1 );

			// shift to alphabet
			name_letter -= (unsigned long) 'A';

			unsigned long *letter_ct =& my_agent->id_counter[ name_letter ];

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

void smem_disconnect_chunk( agent *my_agent, smem_lti_id parent_id )
{
	// adjust attribute counts
	{
		// get all old counts
		my_agent->smem_stmts->web_attr_ct->bind_int( 1, parent_id );
		while ( my_agent->smem_stmts->web_attr_ct->execute() == soar_module::row )
		{
			// adjust in opposite direction ( adjust, attribute )
			my_agent->smem_stmts->ct_attr_update->bind_int( 1, -( my_agent->smem_stmts->web_attr_ct->column_int( 1 ) ) );
			my_agent->smem_stmts->ct_attr_update->bind_int( 2, my_agent->smem_stmts->web_attr_ct->column_int( 0 ) );
			my_agent->smem_stmts->ct_attr_update->execute( soar_module::op_reinit );
		}
		my_agent->smem_stmts->web_attr_ct->reinitialize();
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

void smem_store_chunk( agent *my_agent, smem_lti_id parent_id, smem_wme_list *children )
{	
	smem_wme_list::iterator child_p;
	wme *child;

	long attr_hash = NULL;
	long value_hash = NULL;
	smem_lti_id value_lti = NULL;

	std::map<long, unsigned long> attr_ct_adjust;
	std::map<long, std::map<long, unsigned long>> const_ct_adjust;
	std::map<long, std::map<smem_lti_id, unsigned long>> lti_ct_adjust;
	
	// clear web, adjust counts
	smem_disconnect_chunk( my_agent, parent_id );

	for ( child_p=children->begin(); child_p!=children->end(); child_p++ )
	{
		// just a shortcut
		child = (*child_p);
		
		// get attribute hash and contribute to count adjustment
		attr_hash = smem_temporal_hash( my_agent, child->attr );
		attr_ct_adjust[ attr_hash ]++;

		// most handling is specific to constant vs. identifier
		if ( smem_symbol_is_constant( child->value ) )
		{
			value_hash = smem_temporal_hash( my_agent, child->value );

			// parent_id, attr, val_const, val_lti
			my_agent->smem_stmts->web_add->bind_int( 1, parent_id );
			my_agent->smem_stmts->web_add->bind_int( 2, attr_hash );			
			my_agent->smem_stmts->web_add->bind_int( 3, value_hash );
			my_agent->smem_stmts->web_add->bind_null( 4 );
			my_agent->smem_stmts->web_add->execute( soar_module::op_reinit );

			const_ct_adjust[ attr_hash ][ value_hash ]++;
		}
		else
		{
			value_lti = smem_lti_add( my_agent, child->value );

			// parent_id, attr, val_const, val_lti
			my_agent->smem_stmts->web_add->bind_int( 1, parent_id );
			my_agent->smem_stmts->web_add->bind_int( 2, attr_hash );
			my_agent->smem_stmts->web_add->bind_null( 3 );
			my_agent->smem_stmts->web_add->bind_int( 4, value_lti );
			my_agent->smem_stmts->web_add->execute( soar_module::op_reinit );

			// add to counts
			lti_ct_adjust[ attr_hash ][ value_lti ]++;
		}
	}

	// update attribute counts
	{
		std::map<long, unsigned long>::iterator p;

		for ( p=attr_ct_adjust.begin(); p!= attr_ct_adjust.end(); p++ )
		{
			// make sure counter exists (attr)
			my_agent->smem_stmts->ct_attr_add->bind_int( 1, p->first );
			my_agent->smem_stmts->ct_attr_add->execute( soar_module::op_reinit );

			// adjust count (adjustment, attr)
			my_agent->smem_stmts->ct_attr_update->bind_int( 1, p->second );
			my_agent->smem_stmts->ct_attr_update->bind_int( 2, p->first );
			my_agent->smem_stmts->ct_attr_update->execute( soar_module::op_reinit );
		}
	}

	// update constant counts
	{
		std::map<long, std::map<long, unsigned long>>::iterator p1;
		std::map<long, unsigned long>::iterator p2;

		for ( p1=const_ct_adjust.begin(); p1!=const_ct_adjust.end(); p1++ )
		{
			for ( p2=(p1->second).begin(); p2!=(p1->second).end(); p2++ )
			{
				// make sure counter exists (attr, val)
				my_agent->smem_stmts->ct_const_add->bind_int( 1, p1->first );
				my_agent->smem_stmts->ct_const_add->bind_int( 2, p2->first );
				my_agent->smem_stmts->ct_const_add->execute( soar_module::op_reinit );

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
		std::map<long, std::map<smem_lti_id, unsigned long>>::iterator p1;
		std::map<smem_lti_id, unsigned long>::iterator p2;

		for ( p1=lti_ct_adjust.begin(); p1!=lti_ct_adjust.end(); p1++ )
		{
			for ( p2=(p1->second).begin(); p2!=(p1->second).end(); p2++ )
			{
				// make sure counter exists (attr, lti)
				my_agent->smem_stmts->ct_lti_add->bind_int( 1, p1->first );
				my_agent->smem_stmts->ct_lti_add->bind_int( 2, p2->first );
				my_agent->smem_stmts->ct_lti_add->execute( soar_module::op_reinit );

				// adjust count (adjustment, attr, lti)
				my_agent->smem_stmts->ct_lti_update->bind_int( 1, p2->second );
				my_agent->smem_stmts->ct_lti_update->bind_int( 2, p1->first );
				my_agent->smem_stmts->ct_lti_update->bind_int( 3, p2->first );				
				my_agent->smem_stmts->ct_lti_update->execute( soar_module::op_reinit );
			}
		}
	}

	// activate parent
	smem_lti_activate( my_agent, parent_id );
}

void smem_store( agent *my_agent, Symbol *id, smem_storage_type store_type = store_level, tc_number tc = NIL )
{
	// transitive closure only matters for recursive storage
	if ( ( store_type == store_recursive ) && ( tc == NIL ) )
	{
		tc = get_new_tc_number( my_agent );
	}

	// get level
	smem_wme_list *children = smem_get_direct_augs_of_id( id, tc );	

	// encode this level
	smem_store_chunk( my_agent, smem_lti_add( my_agent, id ), children );

	// recurse as necessary
	if ( store_type == store_recursive )
	{
		smem_wme_list::iterator child_p;
		
		for ( child_p=children->begin(); child_p!=children->end(); child_p++ )
		{
			if ( !smem_symbol_is_constant( (*child_p)->value ) )
			{
				smem_store( my_agent, (*child_p)->value, store_type, tc );
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
	if ( lti == NIL )
	{
		sqlite_statement *q = my_agent->smem_stmts->lti_letter_num;
		
		q->bind_int( 1, parent_id );
		q->execute();

		lti = smem_lti_make( my_agent, (char) q->column_int( 0 ), (unsigned long) q->column_int( 1 ), result_header->id.level );

		q->reinitialize();
	}
	
	// activate lti
	smem_lti_activate( my_agent, parent_id );

	// point retrieved to lti
	smem_add_meta_wme( my_agent, state, result_header, my_agent->smem_sym_retrieved, lti );	

	// if no children, then retrieve children
	if ( ( lti->id.impasse_wmes == NIL ) && 
		 ( lti->id.input_wmes == NIL ) &&
		 ( lti->id.slots == NIL ) )
	{
		sqlite_statement *expand_q = my_agent->smem_stmts->web_expand;
		Symbol *attr_sym;
		Symbol *value_sym;
		
		// get direct children: attr_const, attr_type, value_const, value_type, value_letter, value_num, value_lti
		expand_q->bind_int( 1, parent_id );
		while ( expand_q->execute() == soar_module::row )
		{
			// make the identifier symbol irrespective of value type
			attr_sym = smem_statement_to_symbol( my_agent, expand_q, 1, 0 );				
			
			// identifier vs. constant
			if ( my_agent->smem_stmts->web_expand->column_type( 2 ) == soar_module::null_t )
			{
				value_sym = smem_lti_make( my_agent, (char) expand_q->column_int( 4 ), (unsigned long) expand_q->column_int( 5 ), lti->id.level );
				value_sym->id.smem_lti = expand_q->column_int( 6 );					
			}
			else
			{
				value_sym = smem_statement_to_symbol( my_agent, expand_q, 3, 2 );					
			}

			// add wme
			smem_add_retrieved_wme( my_agent, state, lti, attr_sym, value_sym );

			// deal with ref counts
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

void smem_process_query( agent *my_agent, Symbol *state, Symbol *query, smem_lti_set *prohibit )
{
	smem_wme_list *cue;
	smem_weighted_cue weighted_cue;
	smem_weighted_cue_element *new_cue_element;
	bool good_cue;

	sqlite_statement *q = NULL;

	smem_lti_id king_id = NIL;
	
	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->query->start();
	////////////////////////////////////////////////////////////////////////////

	cue = smem_get_direct_augs_of_id( query );
	good_cue = true;

	// prepare query stats	
	{
		smem_wme_list::iterator cue_p;

		long attr_hash;
		long value_hash;
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

					if ( q->execute() == soar_module::row )
					{
						new_cue_element = new smem_weighted_cue_element;
						
						new_cue_element->weight = q->column_int( 0 );
						new_cue_element->attr_hash = attr_hash;
						new_cue_element->value_hash = value_hash;
						new_cue_element->value_lti = value_lti;
						new_cue_element->cue_element = w;

						new_cue_element->element_type = element_type;

						weighted_cue.push( new_cue_element );
						new_cue_element = NULL;						
					}
					else
					{
						good_cue = false;
					}

					q->reinitialize();
				}
				else
				{
					good_cue = false;					
				}
			}
		}
	}

	// perform search only if necessary
	if ( good_cue && !weighted_cue.empty() )
	{		
		smem_lti_list candidates;		
		smem_lti_list::iterator cand_p;	

		// get initial candidate list (most restrictive, minus prohibitions)
		{		
			smem_lti_set::iterator prohibit_p;
			smem_lti_id cand;

			// get most restrictive cue element
			new_cue_element = weighted_cue.top();
			weighted_cue.pop();
			
			if ( new_cue_element->element_type == attr_t )
			{
				// attr=?
				q = my_agent->smem_stmts->web_attr_all;				
			}
			else if ( new_cue_element->element_type == value_const_t )
			{
				// attr=? AND val_const=?
				q = my_agent->smem_stmts->web_const_all;				
				q->bind_int( 2, new_cue_element->value_hash );
			}
			else if ( new_cue_element->element_type == value_lti_t )
			{
				// attr=? AND val_lti=?
				q = my_agent->smem_stmts->web_lti_all;
				q->bind_int( 1, new_cue_element->attr_hash );				
			}

			// all require hash as first parameter
			q->bind_int( 1, new_cue_element->attr_hash );
			while ( q->execute() == soar_module::row )
			{
				cand = q->column_int( 0 );

				prohibit_p = prohibit->find( cand );
				if ( prohibit_p == prohibit->end() )
				{
					candidates.push_back( cand );
				}
			}
			q->reinitialize();

			delete new_cue_element;
		}

		// proceed through remainder of cue
		while ( !candidates.empty() && !weighted_cue.empty() )
		{
			new_cue_element = weighted_cue.top();
			weighted_cue.pop();

			if ( new_cue_element->element_type == attr_t )
			{
				// parent=? AND attr=?
				q = my_agent->smem_stmts->web_attr_child;				
			}
			else if ( new_cue_element->element_type == value_const_t )
			{
				// parent=? AND attr=? AND val_const=?
				q = my_agent->smem_stmts->web_const_child;				
				q->bind_int( 3, new_cue_element->value_hash );
			}
			else if ( new_cue_element->element_type == value_lti_t )
			{
				// parent=? AND attr=? AND val_lti=?
				q = my_agent->smem_stmts->web_lti_child;				
				q->bind_int( 3, new_cue_element->value_lti );
			}

			// all require attribute
			q->bind_int( 2, new_cue_element->attr_hash );

			// iterate over remaining candidates, submit each to the cue element
			cand_p = candidates.begin();
			do
			{				
				// all require their own id for child search
				q->bind_int( 1, (*cand_p) );

				if ( q->execute( soar_module::op_reinit ) != soar_module::row )
				{
					cand_p = candidates.erase( cand_p );
				}
				else
				{
					cand_p++;
				}
			} while ( cand_p != candidates.end() );	
			
			// de-allocate cue element
			delete new_cue_element;
		}

		// if candidates left, front is winner
		if ( !candidates.empty() )
		{
			king_id = candidates.front();
		}
	}

	// clean cue
	delete cue;

	// clean weighted cue remnants
	while ( !weighted_cue.empty() )
	{
		new_cue_element = weighted_cue.top();
		weighted_cue.pop();

		delete new_cue_element;
	}

	// produce results
	if ( king_id != NIL )
	{
		// success!
		smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_status, my_agent->smem_sym_success );
		
		////////////////////////////////////////////////////////////////////////////
		my_agent->smem_timers->query->stop();
		////////////////////////////////////////////////////////////////////////////

		smem_install_memory( my_agent, state, king_id );
	}
	else
	{
		smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_status, my_agent->smem_sym_failure );

		////////////////////////////////////////////////////////////////////////////
		my_agent->smem_timers->query->stop();
		////////////////////////////////////////////////////////////////////////////
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization (smem::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_clear_result( agent *my_agent, Symbol *state )
{
	wme *w;
	
	while ( !state->id.smem_info->smem_wmes->empty() )
	{
		w = state->id.smem_info->smem_wmes->top();

		if ( w->preference )
			remove_preference_from_tm( my_agent, w->preference );

		soar_module::remove_module_wme( my_agent, w );

		state->id.smem_info->smem_wmes->pop();
	}
}

/***************************************************************************
 * Function     : smem_init_db
 * Author		: Nate Derbinsky
 * Notes		: Opens the SQLite database and performs all
 * 				  initialization required for the current mode
 *
 *                The readonly param should only be used in
 *                experimentation where you don't want to alter
 *                previous database state.
 **************************************************************************/
void smem_init_db( agent *my_agent, bool readonly )
{
	if ( my_agent->smem_db->get_status() != soar_module::disconnected )
		return;

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->init->start();	
	////////////////////////////////////////////////////////////////////////////

	const char *db_path;
	if ( my_agent->smem_params->database->get_value() == smem_param_container::memory )
		db_path = ":memory:";
	else
		db_path = my_agent->smem_params->path->get_value();

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
		soar_module::sqlite_statement *temp_q = NULL;
		//soar_module::sqlite_statement *temp_q2 = NULL;

		// update validation count
		my_agent->smem_validation++;		

		// setup common structures/queries
		my_agent->smem_stmts = new smem_statement_container( my_agent );
		my_agent->smem_stmts->structure();
		my_agent->smem_stmts->prepare();

		// reset identifier counters
		smem_reset_id_counters( my_agent );

		my_agent->smem_stmts->begin->execute( soar_module::op_reinit );

		if ( !readonly )
		{
			// reset lti activation
			{			
				// find lowest cycle
				temp_q = new sqlite_statement( my_agent->smem_db, "SELECT MIN(cycle) FROM activation WHERE cycle>0" );
				temp_q->prepare();
				temp_q->execute();
				if ( temp_q->column_type( 0 ) != soar_module::null_t )
				{					
					smem_activation_cycle cycle_min = (smem_activation_cycle) temp_q->column_int( 0 );
					delete temp_q;

					// shift all others down
					temp_q = new sqlite_statement( my_agent->smem_db, "UPDATE activation SET cycle=cycle - ? WHERE cycle>0" );
					temp_q->prepare();
					temp_q->bind_int( 1, ( cycle_min - 1 ) );
					temp_q->execute();
				}
				
				delete temp_q;
				temp_q = NULL;
			}
		}

		my_agent->smem_stmts->commit->execute( soar_module::op_reinit );
	}

	////////////////////////////////////////////////////////////////////////////
	my_agent->smem_timers->init->stop();	
	////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : smem_close
 * Author		: Nate Derbinsky
 * Notes		: Performs cleanup operations when the database needs
 * 				  to be closed (end soar, manual close, etc)
 **************************************************************************/
void smem_close( agent *my_agent )
{
	if ( my_agent->smem_db->get_status() == soar_module::connected )
	{
		// de-allocate common statements
		delete my_agent->smem_stmts;		

		// close the database
		my_agent->smem_db->disconnect();
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Implementation (smem::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_respond_to_cmd( agent *my_agent )
{	
	// respond to query only if db is properly initialized
	if ( my_agent->smem_db->get_status() != soar_module::connected )
		return;

	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	smem_wme_list *wmes;
	smem_wme_list *cmds;
	smem_wme_list::iterator w_p;	

	Symbol *query;
	Symbol *retrieve;	
	smem_lti_set *prohibit;
	smem_sym_list *store;
	
	enum path_type { blank_slate, cmd_bad, cmd_retrieve, cmd_query, cmd_store } path;
	
	unsigned long wme_count;
	bool new_cue;

	while ( state != NULL )
	{
		////////////////////////////////////////////////////////////////////////////
		my_agent->smem_timers->api->start();
		////////////////////////////////////////////////////////////////////////////
		
		// make sure this state has had some sort of change to the cmd
		new_cue = false;
		wme_count = 0;
		cmds = NIL;
		{			
			int tc = get_new_tc_number( my_agent );
			std::queue<Symbol *> syms;
			Symbol *parent_sym;			

			// initialize BFS at command
			syms.push( state->id.smem_cmd_header );

			while ( !syms.empty() )
			{
				// get state
				parent_sym = syms.front();
				syms.pop();
			
				// get children of the current identifier
				wmes = smem_get_direct_augs_of_id( parent_sym, tc );
				{
					for ( w_p=wmes->begin(); w_p!=wmes->end(); w_p++ )
					{
						wme_count++;

						if ( (*w_p)->timetag > state->id.smem_info->last_cmd_time )
						{
							new_cue = true;
							state->id.smem_info->last_cmd_time = (*w_p)->timetag;
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
			if ( state->id.smem_info->last_cmd_count != wme_count )
			{
				new_cue = true;
				state->id.smem_info->last_cmd_count = wme_count;
			}

			if ( new_cue )
			{
				// clear old cue
				state->id.smem_info->cue_wmes->clear();

				// clear old results
				smem_clear_result( my_agent, state );
			}
		}

		// a command is issued if the cue is new
		// and there is something on the cue
		if ( new_cue && wme_count )
		{
			// initialize command vars
			retrieve = NIL;			
			query = NIL;
			prohibit = new smem_lti_set;
			store = new smem_sym_list;
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
							prohibit->insert( (*w_p)->value->id.smem_lti );
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
							store->push_back( (*w_p)->value );
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
				// start transaction
				my_agent->smem_stmts->begin->execute( soar_module::op_reinit );

				{
				
					// retrieve
					if ( path == cmd_retrieve )
					{						
						if ( retrieve->id.smem_lti == NIL )
						{
							// retrieve is not pointing to an lti!
							smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_status, my_agent->smem_sym_failure );
						}
						else
						{
							// status: success
							smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_status, my_agent->smem_sym_success );
							
							// install memory directly onto the retrieve identifier
							smem_install_memory( my_agent, state, retrieve->id.smem_lti, retrieve );
						}					
					}				
					// query
					else if ( path == cmd_query )
					{
						smem_process_query( my_agent, state, query, prohibit );						
					}
					else if ( path == cmd_store )
					{
						smem_sym_list::iterator sym_p;

						////////////////////////////////////////////////////////////////////////////
						my_agent->smem_timers->storage->start();
						////////////////////////////////////////////////////////////////////////////
						
						for ( sym_p=store->begin(); sym_p!=store->end(); sym_p++ )
						{
							smem_store( my_agent, (*sym_p) );
						}

						////////////////////////////////////////////////////////////////////////////
						my_agent->smem_timers->storage->stop();
						////////////////////////////////////////////////////////////////////////////
					}
				}

				// commit transaction
				my_agent->smem_stmts->commit->execute( soar_module::op_reinit );
			}
			else
			{
				smem_add_meta_wme( my_agent, state, state->id.smem_result_header, my_agent->smem_sym_status, my_agent->smem_sym_bad_cmd );				
			}

			// free store list
			delete store;

			// free prohibit list
			delete prohibit;
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

void smem_go( agent *my_agent )
{
	my_agent->smem_timers->total->start();

#ifndef SMEM_EXPERIMENT
	
	smem_respond_to_cmd( my_agent );

#else // SMEM_EXPERIMENT

#endif // SMEM_EXPERIMENT

	my_agent->smem_timers->total->stop();	
}