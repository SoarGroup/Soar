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
 * Description  :  Various functions for EpMem
 * =======================================================================
 */

#include <stdlib.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>

#include "symtab.h"
#include "io_soar.h"
#include "wmem.h"
#include "soar_rand.h"
#include "production.h"

#include "xmlTraceNames.h"
#include "gski_event_system_functions.h"
#include "print.h"

#include "episodic_memory.h"
#include "misc.h"

#include "sqlite3.h"

using namespace std;

// defined in symtab.cpp but not in symtab.h
extern unsigned long compress( unsigned long h, short num_bits );
extern unsigned long hash_string( const char *s );

/***************************************************************************
 * Function     : epmem_clean_parameters
 **************************************************************************/
void epmem_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<EPMEM_PARAMS; i++ )
	{
		if ( my_agent->epmem_params[ i ]->type == epmem_param_string )
			delete my_agent->epmem_params[ i ]->param->string_param.value;
			
		delete my_agent->epmem_params[ i ];
	}
}

/***************************************************************************
 * Function     : epmem_add_parameter
 **************************************************************************/
epmem_parameter *epmem_add_parameter( const char *name, double value, bool (*val_func)( double ) )
{
	// new parameter entry
	epmem_parameter *newbie = new epmem_parameter;
	newbie->param = new epmem_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = epmem_param_number;
	newbie->name = name;
	
	return newbie;
}

epmem_parameter *epmem_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	// new parameter entry
	epmem_parameter *newbie = new epmem_parameter;
	newbie->param = new epmem_parameter_union;
	newbie->param->constant_param.val_func = val_func;
	newbie->param->constant_param.to_str = to_str;
	newbie->param->constant_param.from_str = from_str;
	newbie->param->constant_param.value = value;
	newbie->type = epmem_param_constant;
	newbie->name = name;
	
	return newbie;
}

epmem_parameter *epmem_add_parameter( const char *name, const char *value, bool (*val_func)( const char * ) )
{
	// new parameter entry
	epmem_parameter *newbie = new epmem_parameter;
	newbie->param = new epmem_parameter_union;
	newbie->param->string_param.value = new string( value );
	newbie->param->string_param.val_func = val_func;
	newbie->type = epmem_param_string;
	newbie->name = name;
	
	return newbie;
}

/***************************************************************************
 * Function     : epmem_convert_parameter
 **************************************************************************/
const char *epmem_convert_parameter( agent *my_agent, const long param )
{
	if ( ( param < 0 ) || ( param >= EPMEM_PARAMS ) )
		return NULL;

	return my_agent->epmem_params[ param ]->name;
}

const long epmem_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<EPMEM_PARAMS; i++ )
		if ( !strcmp( name, my_agent->epmem_params[ i ]->name ) )
			return i;

	return EPMEM_PARAMS;
}

/***************************************************************************
 * Function     : epmem_valid_parameter
 **************************************************************************/
bool epmem_valid_parameter( agent *my_agent, const char *name )
{
	return ( epmem_convert_parameter( my_agent, name ) != EPMEM_PARAMS );
}

bool epmem_valid_parameter( agent *my_agent, const long param )
{
	return ( epmem_convert_parameter( my_agent, param ) != NULL );
}

/***************************************************************************
 * Function     : epmem_get_parameter_type
 **************************************************************************/
epmem_param_type epmem_get_parameter_type( agent *my_agent, const char *name )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return epmem_param_invalid;
	
	return my_agent->epmem_params[ param ]->type;
}

epmem_param_type epmem_get_parameter_type( agent *my_agent, const long param )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return epmem_param_invalid;

	return my_agent->epmem_params[ param ]->type;
}

/***************************************************************************
 * Function     : epmem_get_parameter
 **************************************************************************/
const long epmem_get_parameter( agent *my_agent, const char *name, const double test )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->constant_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const char *name, const char *test )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
		return my_agent->epmem_params[ param ]->param->string_param.value->c_str();
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->constant_param.to_str( my_agent->epmem_params[ param ]->param->constant_param.value );
}

double epmem_get_parameter( agent *my_agent, const char *name )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_number )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->number_param.value;
}

//

const long epmem_get_parameter( agent *my_agent, const long param, const double test )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return NULL;

	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->constant_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const long param, const char *test )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
		return my_agent->epmem_params[ param ]->param->string_param.value->c_str();
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->constant_param.to_str( my_agent->epmem_params[ param ]->param->constant_param.value );
}

double epmem_get_parameter( agent *my_agent, const long param )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_number )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->number_param.value;
}

/***************************************************************************
 * Function     : epmem_valid_parameter_value
 **************************************************************************/
bool epmem_valid_parameter_value( agent *my_agent, const char *name, double new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_number )
		return false;
	
	return my_agent->epmem_params[ param ]->param->number_param.val_func( new_val );
}

bool epmem_valid_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
		return my_agent->epmem_params[ param ]->param->string_param.val_func( new_val );
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return false;
	
	return my_agent->epmem_params[ param ]->param->constant_param.val_func( my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool epmem_valid_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return false;
	
	return my_agent->epmem_params[ param ]->param->constant_param.val_func( new_val );
}

//

bool epmem_valid_parameter_value( agent *my_agent, const long param, double new_val )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_number )
		return false;
	
	return my_agent->epmem_params[ param ]->param->number_param.val_func( new_val );
}

bool epmem_valid_parameter_value( agent *my_agent, const long param, const char *new_val )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
		return my_agent->epmem_params[ param ]->param->string_param.val_func( new_val );
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return false;
	
	return my_agent->epmem_params[ param ]->param->constant_param.val_func( my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool epmem_valid_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return false;
	
	return my_agent->epmem_params[ param ]->param->constant_param.val_func( new_val );
}

/***************************************************************************
 * Function     : epmem_set_parameter
 **************************************************************************/
bool epmem_parameter_protected( agent *my_agent, const long param )
{
	return ( ( my_agent->epmem_db_status != -1 ) && ( param >= EPMEM_PARAM_DB ) && ( param <= EPMEM_PARAM_PROVENANCE ) );
}

bool epmem_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;

	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->epmem_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const char *name, const char *new_val )
{	
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;

	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
	{
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}
	
	const long converted_val = my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	
	my_agent->epmem_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;

	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, new_val );
	
	my_agent->epmem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//

bool epmem_set_parameter( agent *my_agent, const long param, double new_val )
{
	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->epmem_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const long param, const char *new_val )
{
	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
	{
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}
	
	const long converted_val = my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	
	my_agent->epmem_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( epmem_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, new_val );
	
	my_agent->epmem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_learning
 **************************************************************************/
bool epmem_validate_learning( const long new_val )
{
	return ( ( new_val == EPMEM_LEARNING_ON ) || ( new_val == EPMEM_LEARNING_OFF ) );
}

/***************************************************************************
 * Function     : epmem_convert_learning
 **************************************************************************/
const char *epmem_convert_learning( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case EPMEM_LEARNING_ON:
			return_val = "on";
			break;
			
		case EPMEM_LEARNING_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long epmem_convert_learning( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = EPMEM_LEARNING_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = EPMEM_LEARNING_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// database
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_database
 **************************************************************************/
bool epmem_validate_database( const long new_val )
{
	return ( ( new_val == EPMEM_DB_MEM ) || ( new_val == EPMEM_DB_FILE ) );
}

/***************************************************************************
 * Function     : epmem_convert_database
 **************************************************************************/
const char *epmem_convert_database( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case EPMEM_DB_MEM:
			return_val = "memory";
			break;
			
		case EPMEM_DB_FILE:
			return_val = "file";
			break;
	}
	
	return return_val;
}

const long epmem_convert_database( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "memory" ) )
		return_val = EPMEM_DB_MEM;
	else if ( !strcmp( val, "file" ) )
		return_val = EPMEM_DB_FILE;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// path
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_path
 **************************************************************************/
bool epmem_validate_path( const char *new_val )
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// indexing
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_indexing
 **************************************************************************/
bool epmem_validate_indexing( const long new_val )
{
	return ( ( new_val > 0 ) && ( new_val <= EPMEM_INDEXING_BIGTREE_RANGE ) );
}

/***************************************************************************
 * Function     : epmem_convert_indexing
 **************************************************************************/
const char *epmem_convert_indexing( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case EPMEM_INDEXING_BIGTREE_INSTANCE:
			return_val = "bigtree_instance";
			break;

		case EPMEM_INDEXING_BIGTREE_RANGE:
			return_val = "bigtree_range";
			break;
	}
	
	return return_val;
}

const long epmem_convert_indexing( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "bigtree_instance" ) )
		return_val = EPMEM_INDEXING_BIGTREE_INSTANCE;

	if ( !strcmp( val, "bigtree_range" ) )
		return_val = EPMEM_INDEXING_BIGTREE_RANGE;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// provenance
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_provenance
 **************************************************************************/
bool epmem_validate_provenance( const long new_val )
{
	return ( ( new_val == EPMEM_PROVENANCE_ON ) || ( new_val == EPMEM_PROVENANCE_OFF ) );
}

/***************************************************************************
 * Function     : epmem_convert_provenance
 **************************************************************************/
const char *epmem_convert_provenance( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case EPMEM_PROVENANCE_ON:
			return_val = "on";
			break;
			
		case EPMEM_PROVENANCE_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long epmem_convert_provenance( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = EPMEM_PROVENANCE_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = EPMEM_PROVENANCE_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// trigger
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_trigger
 **************************************************************************/
bool epmem_validate_trigger( const long new_val )
{
	return ( ( new_val > 0 ) && ( new_val <= EPMEM_TRIGGER_DC ) );
}

/***************************************************************************
 * Function     : epmem_convert_trigger
 **************************************************************************/
const char *epmem_convert_trigger( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case EPMEM_TRIGGER_OUTPUT:
			return_val = "output";
			break;
			
		case EPMEM_TRIGGER_DC:
			return_val = "dc";
			break;
	}
	
	return return_val;
}

const long epmem_convert_trigger( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "output" ) )
		return_val = EPMEM_TRIGGER_OUTPUT;
	
	if ( !strcmp( val, "dc" ) )
		return_val = EPMEM_TRIGGER_DC;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// balance
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_validate_balance
 **************************************************************************/
bool epmem_validate_balance( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_enabled
 **************************************************************************/
bool epmem_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ EPMEM_ENABLED ] == EPMEM_LEARNING_ON );
}

/***************************************************************************
 * Function     : epmem_clean_stats
 **************************************************************************/
void epmem_clean_stats( agent *my_agent )
{
	for ( int i=0; i<EPMEM_STATS; i++ )
	  delete my_agent->epmem_stats[ i ];
}

/***************************************************************************
 * Function     : epmem_reset_stats
 **************************************************************************/
void epmem_reset_stats( agent *my_agent )
{
	for ( int i=0; i<EPMEM_STATS; i++ )
		my_agent->epmem_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : epmem_add_stat
 **************************************************************************/
epmem_stat *epmem_add_stat( const char *name )
{
	// new stat entry
	epmem_stat *newbie = new epmem_stat;
	newbie->name = name;
	newbie->value = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : epmem_convert_stat
 **************************************************************************/
const long epmem_convert_stat( agent *my_agent, const char *name )
{
	for ( int i=0; i<EPMEM_STATS; i++ )
		if ( !strcmp( name, my_agent->epmem_stats[ i ]->name ) )
			return i;

	return EPMEM_STATS;
}

const char *epmem_convert_stat( agent *my_agent, const long stat )
{
	if ( ( stat < 0 ) || ( stat >= EPMEM_STATS ) )
		return NULL;

	return my_agent->epmem_stats[ stat ]->name;
}

/***************************************************************************
 * Function     : epmem_valid_stat
 **************************************************************************/
bool epmem_valid_stat( agent *my_agent, const char *name )
{
	return ( epmem_convert_stat( my_agent, name ) != EPMEM_STATS );
}

bool epmem_valid_stat( agent *my_agent, const long stat )
{
	return ( epmem_convert_stat( my_agent, stat ) != NULL );
}

/***************************************************************************
 * Function     : epmem_get_stat
 **************************************************************************/
double epmem_get_stat( agent *my_agent, const char *name )
{
	const long stat = epmem_convert_stat( my_agent, name );
	if ( stat == EPMEM_STATS )
		return 0;
	
	if ( stat == EPMEM_STAT_MEM_USAGE )
		return sqlite3_memory_used();
	if ( stat == EPMEM_STAT_MEM_HIGH )
		return sqlite3_memory_highwater( false );

	return my_agent->epmem_stats[ stat ]->value;
}

double epmem_get_stat( agent *my_agent, const long stat )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
		return 0;
	
	if ( stat == EPMEM_STAT_MEM_USAGE )
		return sqlite3_memory_used();
	if ( stat == EPMEM_STAT_MEM_HIGH )
		return sqlite3_memory_highwater( false );

	return my_agent->epmem_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : epmem_set_stat
 **************************************************************************/
bool epmem_set_stat( agent *my_agent, const char *name, double new_val )
{
	const long stat = epmem_convert_stat( my_agent, name );
	if ( ( stat == EPMEM_STATS ) ||
		 ( stat == EPMEM_STAT_MEM_USAGE ) ||
		 ( stat == EPMEM_STAT_MEM_HIGH ) )
		return false;
	
	my_agent->epmem_stats[ stat ]->value = new_val;
	
	return true;
}

bool epmem_set_stat( agent *my_agent, const long stat, double new_val )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
		return false;
	
	if ( ( stat == EPMEM_STAT_MEM_USAGE ) ||
		 ( stat == EPMEM_STAT_MEM_HIGH ) )
		return false;
	
	my_agent->epmem_stats[ stat ]->value = new_val;
	
	return true;
}

//

/***************************************************************************
 * Function     : epmem_get_augs_of_id
 * Author		: Andy Nuxoll
 * Notes		: This routine works just like the one defined in utilities.h.
 *				  Except this one does not use C++ templates because I have an
 *				  irrational dislike for them borne from the years when the STL
 *				  highly un-portable.  I'm told this is no longer true but I'm still
 *				  bitter.
 **************************************************************************/
wme **epmem_get_augs_of_id( agent* my_agent, Symbol * id, tc_number tc, int *num_attr )
{
	slot *s;
	wme *w;
	wme **list;
	int list_position;
	int n = 0;

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
 * Function     : epmem_hash_wme
 * Author		: Andy Nuxoll
 * Notes		: Creates a hash value for a WME.  This is used to find the
 *				  corresponding wmetree node in a hash table.
 **************************************************************************/
unsigned long epmem_hash_wme( wme *w )
{
	unsigned long hash_value;
	string *temp;
	
	// Generate a hash value for the WME's attr and value
	hash_value = hash_string( w->attr->sc.name );
	
	switch( w->value->common.symbol_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
			hash_value += hash_string( w->value->sc.name );
			break;
            
		case INT_CONSTANT_SYMBOL_TYPE:
			temp = to_string( w->value->ic.value );
			hash_value += hash_string( temp->c_str() );
			delete temp;
			break;
		
		case FLOAT_CONSTANT_SYMBOL_TYPE:
			temp = to_string( w->value->fc.value );
			hash_value += hash_string( temp->c_str() );
			delete temp;			
			break;
	}
	
	return hash_value;
}

/***************************************************************************
 * Function     : epmem_init_db
 **************************************************************************/
void epmem_init_db( agent *my_agent )
{
	if ( my_agent->epmem_db_status != -1 )
		return;
	
	const char *db_path;
	if ( epmem_get_parameter( my_agent, EPMEM_PARAM_DB, EPMEM_RETURN_LONG ) == EPMEM_DB_MEM )
		db_path = ":memory:";
	else
		db_path = epmem_get_parameter( my_agent, EPMEM_PARAM_PATH, EPMEM_RETURN_STRING );
	
	// attempt connection
	my_agent->epmem_db_status = sqlite3_open_v2( db_path, &(my_agent->epmem_db), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
	if ( my_agent->epmem_db_status )
	{
		char buf[256];
		SNPRINTF( buf, 254, "DB ERROR: %s", sqlite3_errmsg( my_agent->epmem_db ) );
		
		print( my_agent, buf );
				
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
	}
	else
	{
		const char *tail;
		sqlite3_stmt *create;
		long time_max;
					
		// create vars table (needed before var queries)
		sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS vars (id INTEGER PRIMARY KEY,value NONE)", -1, &create, &tail );
		sqlite3_step( create );
		sqlite3_finalize( create );
		
		// common queries
		sqlite3_prepare_v2( my_agent->epmem_db, "BEGIN", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] ), &tail );
		sqlite3_prepare_v2( my_agent->epmem_db, "COMMIT", -1, &( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] ), &tail );
		sqlite3_prepare_v2( my_agent->epmem_db, "ROLLBACK", -1, &( my_agent->epmem_statements[ EPMEM_STMT_ROLLBACK ] ), &tail );			
		sqlite3_prepare_v2( my_agent->epmem_db, "SELECT value FROM vars WHERE id=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_VAR_GET ] ), &tail );
		sqlite3_prepare_v2( my_agent->epmem_db, "REPLACE INTO vars (id,value) VALUES (?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_VAR_SET ] ), &tail );
		
		// further statement preparation depends upon representation options
		const long indexing = epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG );
		const long provenance = epmem_get_parameter( my_agent, EPMEM_PARAM_PROVENANCE, EPMEM_RETURN_LONG );
					
		// at this point initialize the database for receipt of episodes
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		switch ( indexing )
		{
			case EPMEM_INDEXING_BIGTREE_INSTANCE:
				// episodes table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS episodes (id INTEGER,time INTEGER,weight REAL)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );				
				
				// weight index (for sorting)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS episode_weight ON episodes (weight)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );
				
				// time index (for next)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS episode_time ON episodes (time)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// custom statement for inserting episodes
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO episodes (id,time,weight) VALUES (?,?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ] ), &tail );
				
				// ids table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS ids (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,name TEXT,value NONE,hash INTEGER)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );			

				// hash index for searching
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS id_hash_parent ON ids (hash,parent_id)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// custom statement for inserting ids
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO ids (parent_id,name,value,hash) VALUES (?,?,?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ] ), &tail );

				// custom statement for finding non-identifier id's
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM ids WHERE hash=? AND parent_id=? AND name=? AND value=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] ), &tail );

				// custom statement for finding identifier id's
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM ids WHERE hash=? AND parent_id=? AND name=? AND value IS NULL", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] ), &tail );
							
				// custom statement for retrieving an episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT i.child_id, i.parent_id, i.name, i.value FROM ids i INNER JOIN episodes e ON i.child_id=e.id WHERE e.time=? ORDER BY i.child_id ASC", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ] ), &tail );

				// custom statement for validating an episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT COUNT(*) AS ct FROM episodes WHERE time=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_VALID_EPISODE ] ), &tail );

				// custom statement for finding the next episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT time FROM episodes WHERE time>? ORDER BY time ASC LIMIT 1", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_NEXT_EPISODE ] ), &tail );

				// custom statement for finding the previous episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT time FROM episodes WHERE time<? ORDER BY time DESC LIMIT 1", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_PREV_EPISODE ] ), &tail );

				// get max time
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(time) FROM episodes", -1, &create, &tail );
				if ( sqlite3_step( create ) == SQLITE_ROW )						
					epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, ( sqlite3_column_int( create, 0 ) + 1 ) );
				sqlite3_finalize( create );
				
				break;

			case EPMEM_INDEXING_BIGTREE_RANGE:				
				// times table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS times (id INTEGER PRIMARY KEY)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// custom statement for inserting times
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO times (id) VALUES (?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_TIME ] ), &tail );
				
				////

				// episodes table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS episodes (id INTEGER,start INTEGER,end INTEGER)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );			
				
				// end_id index (for updates)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS episode_id_end ON episodes (id,end)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// start/end index (for retrieval)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS episode_start_end ON episodes (start,end)", -1, &create, &tail );
				sqlite3_step( create );
				sqlite3_finalize( create );

				// custom statement for updating episodes
				sqlite3_prepare_v2( my_agent->epmem_db, "UPDATE episodes SET end=? WHERE id=? AND end IS NULL", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] ), &tail );

				// custom statement for inserting episodes
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO episodes (id,start,end) VALUES (?,?,NULL)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] ), &tail );

				////
				
				// ids table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS ids (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,name TEXT,value NONE,hash INTEGER)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );			

				// hash index for searching
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS id_hash_parent ON ids (hash,parent_id)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// custom statement for inserting ids
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO ids (parent_id,name,value,hash) VALUES (?,?,?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ] ), &tail );

				// custom statement for finding non-identifier id's
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM ids WHERE hash=? AND parent_id=? AND name=? AND value=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] ), &tail );

				// custom statement for finding identifier id's
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM ids WHERE hash=? AND parent_id=? AND name=? AND value IS NULL", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] ), &tail );

				// custom statement for retrieving an episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT i.child_id, i.parent_id, i.name, i.value FROM episodes e INNER JOIN ids i ON e.id=i.child_id WHERE e.start<=? AND (e.end>=? OR e.end IS NULL) ORDER BY i.child_id ASC", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ] ), &tail );

				// custom statement for validating an episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT COUNT(*) AS ct FROM times WHERE id=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_VALID_EPISODE ] ), &tail );

				// custom statement for finding the next episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id>? ORDER BY id ASC LIMIT 1", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NEXT_EPISODE ] ), &tail );

				// custom statement for finding the prev episode
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT id FROM times WHERE id<? ORDER BY id DESC LIMIT 1", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_PREV_EPISODE ] ), &tail );

				////

				// weights table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS weights (id INTEGER PRIMARY KEY, weight REAL)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// custom statement for adding a weight
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO weights (id,weight) VALUES (?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_WEIGHT ] ), &tail );

				// custom statement for removing all weights
				sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM weights", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_WEIGHTS ] ), &tail );
				
				////

				// ranges table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS ranges (start INTEGER, end INTEGER, weight REAL, ct INTEGER)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// end index
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS ranges_end ON ranges (end)", -1, &create, &tail );
				sqlite3_step( create );
				sqlite3_finalize( create );

				// custom statements for creating/dropping the start index
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS ranges_start ON ranges (start)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1 ] ), &tail );
				sqlite3_prepare_v2( my_agent->epmem_db, "DROP INDEX ranges_start", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1R ] ), &tail );

				// custom statements for creating/dropping the start/end index
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS ranges_start_end ON ranges (start,end)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2 ] ), &tail );
				sqlite3_prepare_v2( my_agent->epmem_db, "DROP INDEX ranges_start_end", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2R ] ), &tail );				

				// custom statement for replacing null ends
				sqlite3_prepare_v2( my_agent->epmem_db, "UPDATE ranges SET end=? WHERE end IS NULL", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NULL_RANGES ] ), &tail );

				// custom statement for deleting contained prohibited ranges
				sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM ranges WHERE start<? AND end>?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB ] ), &tail );

				// custom statement for updating lower boundary
				sqlite3_prepare_v2( my_agent->epmem_db, "UPDATE ranges SET start=? WHERE start BETWEEN ? AND ?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ] ), &tail );

				// custom statement for updating upper boundary
				sqlite3_prepare_v2( my_agent->epmem_db, "UPDATE ranges SET end=? WHERE end BETWEEN ? AND ?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ] ), &tail );

				// custom statement for inserting non-containing
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO ranges (start,end,weight,ct) SELECT start,?,weight,ct FROM ranges WHERE start<? AND end>? UNION ALL SELECT ?,end,weight,ct FROM ranges WHERE start<? AND end>?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ] ), &tail );

				// custom statement for getting the low list
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT start, SUM(ct) AS cnt, SUM(weight) AS v FROM ranges GROUP BY start ORDER BY start", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ] ), &tail );

				// custom statement for getting the high list
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT end, SUM(ct) AS cnt, SUM(weight) AS v FROM ranges GROUP BY end ORDER BY end", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ] ), &tail );

				// custom statement for removing all ranges
				sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM ranges", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_RANGES ] ), &tail );

				// get max time
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(id) FROM times", -1, &create, &tail );
				if ( sqlite3_step( create ) == SQLITE_ROW )						
					epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, ( sqlite3_column_int( create, 0 ) + 1 ) );
				sqlite3_finalize( create );

				// get max id + max list
				time_max = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME );
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(child_id) FROM ids", -1, &create, &tail );
				sqlite3_step( create );
				if ( sqlite3_column_type( create, 0 ) != SQLITE_NULL )
				{
					my_agent->epmem_range_maxes->resize( sqlite3_column_int( create, 0 ), time_max - 1 );

					sqlite3_stmt *create2;
					sqlite3_prepare_v2( my_agent->epmem_db, "SELECT e.id, MAX(e.end) FROM episodes e WHERE e.id NOT IN (SELECT id FROM episodes WHERE end IS NULL) GROUP BY e.id", -1, &create2, &tail );
					while ( sqlite3_step( create2 ) == SQLITE_ROW )
						(*my_agent->epmem_range_maxes)[ sqlite3_column_int( create2, 0 ) - 1 ] = sqlite3_column_int( create2, 1 );

					sqlite3_finalize( create2 );
				}				
				sqlite3_finalize( create );
							
				break;
		}
		
		switch ( provenance )
		{
			case EPMEM_PROVENANCE_ON:
				break;
				
			case EPMEM_PROVENANCE_OFF:
				break;
		}
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
	}
}

/***************************************************************************
 * Function     : epmem_reset
 **************************************************************************/
void epmem_reset( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;
	while( goal )
	{
		epmem_data *data = goal->id.epmem_info;
				
		data->last_ol_time = 0;
		data->last_ol_count = 0;

		data->last_cmd_time = 0;
		data->last_cmd_count = 0;

		data->last_memory = EPMEM_MEMID_NONE;
		
		goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : epmem_consider_new_episode
 **************************************************************************/
void epmem_consider_new_episode( agent *my_agent )
{
	const long trigger = epmem_get_parameter( my_agent, EPMEM_PARAM_TRIGGER, EPMEM_RETURN_LONG );
	bool new_memory = false;
	
	if ( trigger == EPMEM_TRIGGER_OUTPUT )
	{
		slot *s;
		wme *w;
		Symbol *ol = my_agent->io_header_output;
		int wme_count = 0;
			
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

		if ( my_agent->bottom_goal->id.epmem_info->last_ol_count != wme_count )
		{
			new_memory = true;
			my_agent->bottom_goal->id.epmem_info->last_ol_count = wme_count;
		}
	}
	else if ( trigger == EPMEM_TRIGGER_DC )
	{
		new_memory = true;
	}
	
	if ( new_memory )
		epmem_new_episode( my_agent );
}

/***************************************************************************
 * Function     : epmem_new_episode
 **************************************************************************/
void epmem_new_episode( agent *my_agent )
{		
	// if this is the first episode, initialize db components	
	if ( my_agent->epmem_db_status == -1 )
		epmem_init_db( my_agent );
	
	// add the episode only if db is properly initialized
	if ( my_agent->epmem_db_status != SQLITE_OK )
		return;

	const long time_counter = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME );

	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%d, %d)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number, time_counter );
		
		print( my_agent, buf );
		
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
	}
	
	if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		wme **wmes = NULL;
		int len = 0;
		
		queue<Symbol *> syms;
		queue<unsigned long> ids;		

		unsigned long parent_id;		
		map<unsigned long, double *> epmem;

		unsigned long my_hash;
		int tc = get_new_tc_number( my_agent );

		int i;	

		syms.push( my_agent->top_goal );
		ids.push( EPMEM_MEMID_ROOT );	
		
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		while ( !syms.empty() )
		{		
			parent_sym = syms.front();
			syms.pop();

			parent_id = ids.front();
			ids.pop();

			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				for ( i=0; i<len; i++ )
				{					
					//if ( ( parent_id == 13 ) && ( !strcmp("random",wmes[i]->attr->sc.name) || !strcmp("clock",wmes[i]->attr->sc.name) ) )
					//	continue;
					
					if ( wmes[i]->epmem_id == NULL )
					{					
						// find wme id					
						my_hash = epmem_hash_wme( wmes[i] );
						if ( wmes[i]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{					
							// hash=? AND parent_id=? AND name=? AND value=?
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 1, my_hash );
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 2, parent_id );
							sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );
							switch( wmes[i]->value->common.symbol_type )
							{
								case SYM_CONSTANT_SYMBOL_TYPE:
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, (const char *) wmes[i]->value->sc.name, -1, SQLITE_STATIC );
									break;
						            
								case INT_CONSTANT_SYMBOL_TYPE:
			        				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, wmes[i]->value->ic.value );
									break;
					
								case FLOAT_CONSTANT_SYMBOL_TYPE:
			        				sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, wmes[i]->value->fc.value );
									break;
							}
							
							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 0 );
							
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] );
						}
						else
						{
							// hash=? AND parent_id=? AND name=? AND value IS NULL						
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 1, my_hash );
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 2, parent_id );
							sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 0 );
							
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] );
						}
					}
					
					// insert on no id
					if ( wmes[i]->epmem_id == NULL )
					{						
						// insert (parent_id,name,value,hash)						
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 1, parent_id );
						sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 2, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );				
						switch ( wmes[i]->value->common.symbol_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 3, (const char *) wmes[i]->value->sc.name, -1, SQLITE_STATIC );
								break;
								
							case INT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 3, wmes[i]->value->ic.value );
								break;
								
							case FLOAT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 3, wmes[i]->value->fc.value );
								break;
								
							case IDENTIFIER_SYMBOL_TYPE:
								sqlite3_bind_null( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 3 );
								break;
						}
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ], 4, my_hash );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_ID ] );					

						wmes[i]->epmem_id = sqlite3_last_insert_rowid( my_agent->epmem_db );
					}
					
					// keep track of identifiers (for further study)
					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						syms.push( wmes[i]->value );
						ids.push( wmes[i]->epmem_id );

						epmem[ wmes[i]->epmem_id ] = NULL;
					}
					else
					{
						// for clarity:
						// map initializes a new element.
						// I want to point to that address and change it if necessary.
						double **p =& epmem[ wmes[i]->epmem_id ];
						
						// replace rand here with actual weight
						double my_val = SoarRand();
						if ( *p == NULL )
							*p = new double( my_val );
						else if ( my_val > **p )
							**p = my_val;					
					}
				}

				// free space from aug list
				free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
			}
		}

		// all inserts at once (provides unique)
		map<unsigned long, double *>::iterator e = epmem.begin();
		while ( e != epmem.end() )
		{
			// add nodes to the episodic store
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 1, e->first );
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 2, time_counter );

			if ( e->second == NULL )
				sqlite3_bind_null( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 3 );
			else
			{
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 3, (*e->second) );
				delete e->second;
			}

			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ] );

			e++;
		}

		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, time_counter + 1 );
	}
	else if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		wme **wmes = NULL;
		int len = 0;
		
		queue<Symbol *> syms;
		queue<unsigned long> ids;		

		unsigned long parent_id;
		map<unsigned long, bool> epmem;

		unsigned long my_hash;
		int tc = get_new_tc_number( my_agent );

		int i;	

		syms.push( my_agent->top_goal );
		ids.push( EPMEM_MEMID_ROOT );
		
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		while ( !syms.empty() )
		{		
			parent_sym = syms.front();
			syms.pop();

			parent_id = ids.front();
			ids.pop();

			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				for ( i=0; i<len; i++ )
				{
					//if ( ( parent_id == 13 ) && ( !strcmp("random",wmes[i]->attr->sc.name) || !strcmp("clock",wmes[i]->attr->sc.name) ) )
					//	continue;
					
					if ( wmes[i]->epmem_id == NULL )
					{						
						my_hash = epmem_hash_wme( wmes[i] );
						if ( wmes[i]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{					
							// hash=? AND parent_id=? AND name=? AND value=?
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 1, my_hash );
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 2, parent_id );
							sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );
							switch( wmes[i]->value->common.symbol_type )
							{
								case SYM_CONSTANT_SYMBOL_TYPE:
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, (const char *) wmes[i]->value->sc.name, -1, SQLITE_STATIC );
									break;
						            
								case INT_CONSTANT_SYMBOL_TYPE:
			        				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, wmes[i]->value->ic.value );
									break;
					
								case FLOAT_CONSTANT_SYMBOL_TYPE:
			        				sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, wmes[i]->value->fc.value );
									break;
							}
							
							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 0 );
							
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] );
						}
						else
						{
							// hash=? AND parent_id=? AND name=? AND value IS NULL							
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 1, my_hash );
							sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 2, parent_id );
							sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 0 );
							
							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] );
						}
					}					
										
					// insert on no id
					if ( wmes[i]->epmem_id == NULL )
					{						
						// insert (parent_id,name,value,hash)						
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 1, parent_id );
						sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 2, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );				
						switch ( wmes[i]->value->common.symbol_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 3, (const char *) wmes[i]->value->sc.name, -1, SQLITE_STATIC );
								break;
								
							case INT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 3, wmes[i]->value->ic.value );
								break;
								
							case FLOAT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 3, wmes[i]->value->fc.value );
								break;
								
							case IDENTIFIER_SYMBOL_TYPE:
								sqlite3_bind_null( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 3 );
								break;
						}
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ], 4, my_hash );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_ID ] );					

						wmes[i]->epmem_id = sqlite3_last_insert_rowid( my_agent->epmem_db );

						// new nodes definitely start
						epmem[ wmes[i]->epmem_id ] = true;
						my_agent->epmem_range_maxes->push_back( time_counter );
					}
					else
					{
						// definitely don't update/delete
						(*my_agent->epmem_range_removals)[ wmes[i]->epmem_id ] = false;

						// we insert if current time is > 1+ max
						if ( (*my_agent->epmem_range_maxes)[ wmes[i]->epmem_id - 1 ] < ( time_counter - 1 ) )
							epmem[ wmes[i]->epmem_id ] = true;

						// update max irrespectively
						(*my_agent->epmem_range_maxes)[ wmes[i]->epmem_id - 1 ] = time_counter;
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
		map<unsigned long, bool>::iterator e = epmem.begin();
		while ( e != epmem.end() )
		{	
			// INSERT (id, start, NULL)
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ], 1, e->first );
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ], 2, time_counter );			
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] );

			e++;
		}

		// all removals at once
		std::map<unsigned long, bool>::iterator r = my_agent->epmem_range_removals->begin();
		while ( r != my_agent->epmem_range_removals->end() )
		{
			if ( r->second )
			{			
				// UPDATE set end=? WHERE id=? AND end IS NULL
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ], 1, ( time_counter - 1 ) );
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ], 2, r->first );			
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] );
			}
			
			r++;
		}
		my_agent->epmem_range_removals->clear();

		// add the time id to the times table
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_TIME ], 1, time_counter );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_TIME ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_TIME ] );

		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, time_counter + 1 );
	}
}

/***************************************************************************
 * Function     : epmem_valid_episode
 **************************************************************************/
bool epmem_valid_episode( agent *my_agent, long memory_id )
{
	const long indexing = epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG );
	bool return_val = false;

	if ( indexing == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_VALID_EPISODE ] );
		return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_VALID_EPISODE ] );
	}
	else if ( indexing == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_VALID_EPISODE ] );
		return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_VALID_EPISODE ] );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_next_episode
 **************************************************************************/
long epmem_next_episode( agent *my_agent, long memory_id )
{
	const long indexing = epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG );
	long return_val = EPMEM_MEMID_NONE;

	if ( indexing == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_NEXT_EPISODE ] );
	}
	else if ( indexing == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NEXT_EPISODE ] );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_previous_episode
 **************************************************************************/
long epmem_previous_episode( agent *my_agent, long memory_id )
{
	const long indexing = epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG );
	long return_val = EPMEM_MEMID_NONE;

	if ( indexing == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_PREV_EPISODE ] );
	}
	else if ( indexing == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_PREV_EPISODE ] );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_respond_to_cmd
 **************************************************************************/
void epmem_respond_to_cmd( agent *my_agent )
{
	// if this is before the first episode, initialize db components	
	if ( my_agent->epmem_db_status == -1 )
		epmem_init_db( my_agent );
	
	// respond to query only if db is properly initialized
	if ( my_agent->epmem_db_status != SQLITE_OK )
		return;
	
	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	wme **wmes;	
	const char *attr_name;
	int len;
	int i;

	long retrieve;
	bool next, previous;
	Symbol *query;
	Symbol *neg_query;
	vector<long> *prohibit = new vector<long>();
	long before, after;
	bool good_cue;
	int path;

	slot *s;
	wme *w;
	Symbol *epmem_cmd;
	int wme_count;
	bool new_cue;

	while ( state != NULL )
	{
		// make sure this state has had some sort of change to the cmd
		new_cue = false;
		wme_count = 0;
		{
			epmem_cmd = state->id.epmem_cmd_header;
				
			// examine all entries on the cmd header
			// that appeared since last cue was encountered
			for ( s = epmem_cmd->id.slots; s != NIL; s = s->next )
			{
				for ( w = s->wmes; w != NIL; w = w->next )
				{
					wme_count++;
					
					if ( w->timetag > state->id.epmem_info->last_cmd_time )
					{
						new_cue = true;
						state->id.epmem_info->last_cmd_time = w->timetag; 
					}
				}
			}

			if ( state->id.epmem_info->last_cmd_count != wme_count )
			{				
				state->id.epmem_info->last_cmd_count = wme_count;

				if ( wme_count != 0 )
					new_cue = true;
				else
					epmem_clear_result( my_agent, state );
			}
		}
		
		if ( new_cue )
		{		
			// initialize command vars
			retrieve = EPMEM_MEMID_NONE;
			next = false;
			previous = false;
			query = NULL;
			neg_query = NULL;
			prohibit->clear();
			before = EPMEM_MEMID_NONE;
			after = EPMEM_MEMID_NONE;
			good_cue = true;
			path = 0;
			
			// get all top-level symbols
			wmes = epmem_get_augs_of_id( my_agent, state->id.epmem_cmd_header, get_new_tc_number( my_agent ), &len );

			// process top-level symbols
			for ( i=0; i<len; i++ )
			{
				if ( good_cue )
				{
					// get attribute name
					attr_name = (const char *) wmes[ i ]->attr->sc.name;

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

			if ( good_cue )
			{
				// retrieve
				if ( path == 1 )
				{
					if ( retrieve != state->id.epmem_info->last_memory )
					{
						epmem_clear_result( my_agent, state );
						epmem_install_memory( my_agent, state, retrieve );
					}
				}
				// previous or next
				else if ( path == 2 )
				{
					epmem_clear_result( my_agent, state );
					epmem_install_memory( my_agent, state, ( ( next )?( epmem_next_episode( my_agent, state->id.epmem_info->last_memory ) ):( epmem_previous_episode( my_agent, state->id.epmem_info->last_memory ) ) ) );
				}
				// query
				else if ( path == 3 )
				{
					epmem_clear_result( my_agent, state );				
					epmem_process_query( my_agent, state, query, neg_query, prohibit, before, after );
				}
			}
			else
			{
				epmem_clear_result( my_agent, state );
				add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );
			}

			// free space from aug list
			free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
		}

		state = state->id.higher_goal;
	}
}

/***************************************************************************
 * Function     : epmem_clear_result
 **************************************************************************/
void epmem_clear_result( agent *my_agent, Symbol *state )
{	
	int len;
	wme **wmes = epmem_get_augs_of_id( my_agent, state->id.epmem_result_header, get_new_tc_number( my_agent ), &len );

	for ( int i=0; i<len; i++ )
		remove_input_wme( my_agent, wmes[ i ] );
	
	free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
}

/***************************************************************************
 * Function     : epmem_process_query
 **************************************************************************/
void epmem_process_query( agent *my_agent, Symbol *state, Symbol *query, Symbol *neg_query, vector<long> *prohibit, long before, long after )
{
	int len_query = 0, len_neg_query = 0;
	wme **wmes_query = NULL;
	if ( query != NULL )
		wmes_query = epmem_get_augs_of_id( my_agent, query, get_new_tc_number( my_agent ), &len_query );

	wme **wmes_neg_query = NULL;
	if ( neg_query != NULL )
		wmes_neg_query = epmem_get_augs_of_id( my_agent, neg_query, get_new_tc_number( my_agent ), &len_neg_query );

	if ( ( len_query != 0 ) || ( len_neg_query != 0 ) )
	{
		const long indexing = epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG );

		if ( indexing == EPMEM_INDEXING_BIGTREE_INSTANCE )
		{
			// initialize pos/neg lists
			std::list<unsigned long> leaf_ids[2];
			std::list<unsigned long>::iterator leaf_p;
			vector<long>::iterator prohibit_p;
			{
				wme ***wmes;
				int len;				
				
				queue<Symbol *> parent_syms;
				queue<unsigned long> parent_ids;			
				int tc = get_new_tc_number( my_agent );

				Symbol *parent_sym;
				unsigned long parent_id;
				unsigned long my_hash;

				int i, j;
				bool just_started;
				
				for ( i=0; i<2; i++ )
				{			
					switch ( i )
					{
						case 0:
							wmes = &wmes_query;
							len = len_query;					
							parent_syms.push( query );
							parent_ids.push( EPMEM_MEMID_ROOT );
							just_started = true;
							break;

						case 1:
							wmes = &wmes_neg_query;
							len = len_neg_query;						
							parent_syms.push( neg_query );
							parent_ids.push( EPMEM_MEMID_ROOT );
							just_started = true;
							break;
					}
					
					while ( !parent_syms.empty() )
					{
						parent_sym = parent_syms.front();
						parent_syms.pop();

						parent_id = parent_ids.front();					
						parent_ids.pop();

						if ( !just_started )
							(*wmes) = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );
						else
							just_started = false;

						if ( (*wmes) != NULL )
						{
							for ( j=0; j<len; j++ )
							{
								// find wme id							
								my_hash = epmem_hash_wme( (*wmes)[j] );
								if ( (*wmes)[j]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
								{
									// hash=? AND parent_id=? AND name=? AND value=?
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 1, my_hash );
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 2, parent_id );
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 3, (const char *) (*wmes)[j]->attr->sc.name, -1, SQLITE_STATIC );
									switch( (*wmes)[j]->value->common.symbol_type )
									{
										case SYM_CONSTANT_SYMBOL_TYPE:
											sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, (const char *) (*wmes)[j]->value->sc.name, -1, SQLITE_STATIC );
											break;
								            
										case INT_CONSTANT_SYMBOL_TYPE:
			        						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, (*wmes)[j]->value->ic.value );
											break;
							
										case FLOAT_CONSTANT_SYMBOL_TYPE:
			        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 4, (*wmes)[j]->value->fc.value );
											break;
									}
									
									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] ) == SQLITE_ROW )
										leaf_ids[i].push_back( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 0 ) );
									
									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] );
								}
								else
								{
									// hash=? AND parent_id=? AND name=? AND value IS NULL						
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 1, my_hash );
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 2, parent_id );
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 3, (const char *) (*wmes)[j]->attr->sc.name, -1, SQLITE_STATIC );

									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] ) == SQLITE_ROW )
									{
										parent_syms.push( (*wmes)[j]->value );
										parent_ids.push( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 0 ) );
									}

									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] );
								}
							}

							// free space from aug list
							free_memory( my_agent, (*wmes), MISCELLANEOUS_MEM_USAGE );
						}
					}
				}
			}

			// at this point leaf_ids has ids of interest for
			// query and neg-query, now proceed
			if ( leaf_ids[0].empty() && leaf_ids[1].empty() )
			{
				add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
			}
			else
			{				
				sqlite3_stmt *search;
				int cue_size;
				int result_time;
				int result_cardinality;
				double result_features;
				double result_match_score;
				
				if ( leaf_ids[0].empty() || leaf_ids[1].empty() )
				{
					int index = ( ( leaf_ids[0].empty() )?( 1 ):( 0 ) );
					const char *sort_dir = ( ( leaf_ids[0].empty() )?( "ASC" ):( "DESC" ) );
					double balance = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_BALANCE );
				
					// construct basic sql					
					string search_sql = "SELECT a.time, a.cardinality, a.features, ((a.cardinality * ?) + (a.features * ?)) AS match_score FROM (SELECT time, COUNT(time) AS cardinality, SUM(weight) AS features FROM episodes WHERE weight IS NOT NULL AND id IN (";
					{
						string *qs = string_multi_copy( "?,", leaf_ids[ index ].size() - 1 );												
						search_sql.append( *qs );
						delete qs;
					}
					search_sql += "?) ";
					
					// prohibit
					if ( !prohibit->empty() )
					{
						string *qs = string_multi_copy( "?,", prohibit->size() - 1 );					

						search_sql += "AND time NOT IN (";
						search_sql.append( *qs );
						search_sql += "?) ";

						delete qs;
					}

					// before/after
					if ( before != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time < ? ";
					}
					if ( after != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time > ? ";
					}

					// finish up
					search_sql += "GROUP BY time) a ORDER BY match_score ";
					search_sql += sort_dir;
					search_sql += ", a.time DESC LIMIT 1";

					// prepare query
					const char *tail;
					sqlite3_prepare_v2( my_agent->epmem_db, search_sql.c_str(), -1, &search, &tail );

					// bind variables
					int var = 1;
					sqlite3_bind_double( search, var++, balance );
					sqlite3_bind_double( search, var++, 1 - balance );
					leaf_p = leaf_ids[ index ].begin();
					while ( leaf_p != leaf_ids[ index ].end() )
					{
						sqlite3_bind_int( search, var++, (*leaf_p) );
						leaf_p++;
					}						
					if ( !prohibit->empty() )
					{
						prohibit_p = prohibit->begin();
						while ( prohibit_p != prohibit->end() )
						{
							sqlite3_bind_int( search, var++, (*prohibit_p) );
							prohibit_p++;
						}							
					}
					if ( before != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, before );
					if ( after != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, after );

					// meta data
					cue_size = leaf_ids[ index ].size();
				}
				else
				{
					double balance = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_BALANCE );
					
					// start sql
					string search_sql = "SELECT b.time, b.cardinality, b.features, ((b.cardinality * ?) + (b.features * ?)) AS match_score FROM (SELECT a.t AS time, SUM(a.c) AS cardinality, SUM(a.f) AS features FROM (SELECT time AS t, COUNT(time) AS c, SUM(weight) AS f FROM episodes WHERE weight IS NOT NULL AND id IN (";

					// add positives
					{
						string *qs = string_multi_copy( "?,", leaf_ids[0].size() - 1 );
						search_sql.append( *qs );
						delete qs;
					}
					search_sql += "?) ";

					// positive prohibit
					if ( !prohibit->empty() )
					{
						string *qs = string_multi_copy( "?,", prohibit->size() - 1 );					
						
						search_sql += "AND time NOT IN (";
						search_sql.append( *qs );
						search_sql += "?) ";

						delete qs;
					}

					// positive before/after
					if ( before != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time < ? ";
					}
					if ( after != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time > ? ";
					}

					// proceed to neg
					search_sql += "GROUP BY time UNION ALL \
								   SELECT time AS t, -COUNT(time) AS c, -SUM(weight) AS f FROM episodes WHERE weight IS NOT NULL AND id IN (";

					// add negatives
					{
						string *qs = string_multi_copy( "?,", leaf_ids[1].size() - 1 );					
						search_sql.append( *qs );
						delete qs;
					}					
					search_sql += "?) ";

					// neg prohibit
					if ( !prohibit->empty() )
					{
						string *qs = string_multi_copy( "?,", prohibit->size() - 1 );						
						
						search_sql += "AND time NOT IN (";
						search_sql.append( *qs );
						search_sql += "?) ";

						delete qs;
					}

					// neg before/after
					if ( before != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time < ? ";
					}
					if ( after != EPMEM_MEMID_NONE )
					{
						search_sql += "AND time > ? ";
					}

					// finish
					search_sql += "GROUP BY time) a \
								   GROUP BY a.t) b \
								   ORDER BY match_score DESC, b.time DESC LIMIT 1";

					// prepare query
					const char *tail;
					sqlite3_prepare_v2( my_agent->epmem_db, search_sql.c_str(), -1, &search, &tail );

					// bind variables
					int var = 1;
					sqlite3_bind_double( search, var++, balance );
					sqlite3_bind_double( search, var++, 1 - balance );
					// positive
					leaf_p = leaf_ids[0].begin();
					while ( leaf_p != leaf_ids[0].end() )
					{
						sqlite3_bind_int( search, var++, (*leaf_p) );
						leaf_p++;
					}
					
					if ( !prohibit->empty() )
					{
						prohibit_p = prohibit->begin();
						while ( prohibit_p != prohibit->end() )
						{
							sqlite3_bind_int( search, var++, (*prohibit_p) );
							prohibit_p++;
						}
					}
					if ( before != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, before );
					if ( after != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, after );
					// neg
					leaf_p = leaf_ids[1].begin();
					while ( leaf_p != leaf_ids[1].end() )
					{
						sqlite3_bind_int( search, var++, (*leaf_p) );
						leaf_p++;
					}					
					if ( !prohibit->empty() )
					{
						prohibit_p = prohibit->begin();
						while ( prohibit_p != prohibit->end() )
						{
							sqlite3_bind_int( search, var++, (*prohibit_p) );
							prohibit_p++;
						}
					}
					if ( before != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, before );
					if ( after != EPMEM_MEMID_NONE )
						sqlite3_bind_int( search, var++, after );

					// meta data
					cue_size = ( leaf_ids[0].size() + leaf_ids[1].size() );
				}

				// attempt to execute SQL
				if ( sqlite3_step( search ) == SQLITE_ROW )
				{
					result_time = sqlite3_column_int( search, 0 );
					result_cardinality = sqlite3_column_int( search, 1 );
					result_features = sqlite3_column_double( search, 2 );
					result_match_score = sqlite3_column_double( search, 3 );
					
					// status
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );

					// match score
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, make_float_constant( my_agent, result_match_score ) );

					// cue-size
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, make_int_constant( my_agent, cue_size ) );

					// normalized-match-score
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, make_float_constant( my_agent, ( result_match_score / cue_size ) ) );

					// match-cardinality
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, make_int_constant( my_agent, result_cardinality ) );

					// actual memory
					epmem_install_memory( my_agent, state, result_time );
				}
				else
				{
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
				}

				// cleanup
				sqlite3_finalize( search );
			}
		}
		else if ( indexing == EPMEM_INDEXING_BIGTREE_RANGE )
		{
			// start transaction: performance measure to keep weights/ranges non-permanent (i.e. in memory)
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );

			// get the leaf id's
			std::list<unsigned long> leaf_ids[2];
			std::list<unsigned long>::iterator leaf_p;
			vector<long>::iterator prohibit_p;
			{
				wme ***wmes;
				int len;
				
				queue<Symbol *> parent_syms;
				queue<unsigned long> parent_ids;			
				int tc = get_new_tc_number( my_agent );

				Symbol *parent_sym;
				unsigned long parent_id;
				unsigned long my_hash;

				int i, j;
				bool just_started;

				// initialize pos/neg lists
				for ( i=0; i<2; i++ )
				{			
					switch ( i )
					{
						case 0:
							wmes = &wmes_query;
							len = len_query;					
							parent_syms.push( query );
							parent_ids.push( EPMEM_MEMID_ROOT );
							just_started = true;
							break;

						case 1:
							wmes = &wmes_neg_query;
							len = len_neg_query;						
							parent_syms.push( neg_query );
							parent_ids.push( EPMEM_MEMID_ROOT );
							just_started = true;
							break;
					}
					
					while ( !parent_syms.empty() )
					{
						parent_sym = parent_syms.front();
						parent_syms.pop();

						parent_id = parent_ids.front();					
						parent_ids.pop();

						if ( !just_started )
							(*wmes) = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );
						else
							just_started = false;

						if ( (*wmes) != NULL )
						{
							for ( j=0; j<len; j++ )
							{
								// find wme id							
								my_hash = epmem_hash_wme( (*wmes)[j] );
								if ( (*wmes)[j]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
								{
									// hash=? AND parent_id=? AND name=? AND value=?
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 1, my_hash );
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 2, parent_id );
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 3, (const char *) (*wmes)[j]->attr->sc.name, -1, SQLITE_STATIC );
									switch( (*wmes)[j]->value->common.symbol_type )
									{
										case SYM_CONSTANT_SYMBOL_TYPE:
											sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, (const char *) (*wmes)[j]->value->sc.name, -1, SQLITE_STATIC );
											break;
								            
										case INT_CONSTANT_SYMBOL_TYPE:
			        						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, (*wmes)[j]->value->ic.value );
											break;
							
										case FLOAT_CONSTANT_SYMBOL_TYPE:
			        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 4, (*wmes)[j]->value->fc.value );
											break;
									}
									
									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] ) == SQLITE_ROW )
										leaf_ids[i].push_back( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 0 ) );
									
									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] );
								}
								else
								{
									// hash=? AND parent_id=? AND name=? AND value IS NULL						
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 1, my_hash );
									sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 2, parent_id );
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 3, (const char *) (*wmes)[j]->attr->sc.name, -1, SQLITE_STATIC );

									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] ) == SQLITE_ROW )
									{
										parent_syms.push( (*wmes)[j]->value );
										parent_ids.push( sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 0 ) );
									}

									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] );
								}
							}

							// free space from aug list
							free_memory( my_agent, (*wmes), MISCELLANEOUS_MEM_USAGE );
						}
					}
				}
			}
			int cue_size = ( leaf_ids[0].size() + leaf_ids[1].size() );

			// set weights for all leaf id's
			{			
				for ( int i=0; i<2; i++ )
				{				
					leaf_p = leaf_ids[i].begin();
					while ( leaf_p != leaf_ids[i].end() )
					{						
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_WEIGHT ], 1, (*leaf_p) );
						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_WEIGHT ], 2, SoarRand() );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_WEIGHT ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_WEIGHT ] );
						
						leaf_p++;
					}					
				}
			}

			// create initial set of ranges
			// cross-reference episodes and weights, insert into ranges
			{				
				const char *tail;
				
				if ( !leaf_ids[0].empty() )
				{
					string insert_sql_pos = "INSERT INTO ranges (start,end,weight,ct) SELECT e.start, e.end, w.weight, ? FROM episodes e INNER JOIN weights w ON e.id=w.id WHERE e.id IN (";

					// add positives
					{
						string *qs = string_multi_copy( "?,", leaf_ids[0].size() - 1 );					
						insert_sql_pos.append( *qs );
						delete qs;
					}					
					insert_sql_pos += "?)";

					// optimize for set before
					if ( before != EPMEM_MEMID_NONE )
					{
						insert_sql_pos += " AND e.start<?";
					}

					// prep statement
					sqlite3_stmt *insert;
					int pos = 1;
					sqlite3_prepare_v2( my_agent->epmem_db, insert_sql_pos.c_str(), -1, &insert, &tail );

					// static bindings
					sqlite3_bind_int( insert, pos++, 1 );

					// positive bindings
					leaf_p = leaf_ids[0].begin();
					while ( leaf_p != leaf_ids[0].end() )
					{
						sqlite3_bind_int( insert, pos++, (*leaf_p) );
						leaf_p++;
					}

					// optimization binding
					// optimize for set before
					if ( before != EPMEM_MEMID_NONE )
					{
						sqlite3_bind_int( insert, pos++, before );
					}

					// perform insertion
					sqlite3_step( insert );
					sqlite3_finalize( insert );
				}
				
				if ( !leaf_ids[1].empty() )
				{
					string insert_sql_neg = "INSERT INTO ranges (start,end,weight,ct) SELECT e.start, e.end, ?*w.weight, ? FROM episodes e INNER JOIN weights w ON e.id=w.id WHERE e.id IN (";

					// add negatives
					{
						string *qs = string_multi_copy( "?,", leaf_ids[1].size() - 1 );
						insert_sql_neg.append( *qs );
						delete qs;
					}					
					insert_sql_neg += "?)";

					// optimize for set before
					if ( before != EPMEM_MEMID_NONE )
					{
						insert_sql_neg += " AND e.start<?";
					}

					// prep statement
					sqlite3_stmt *insert;
					int pos = 1;
					sqlite3_prepare_v2( my_agent->epmem_db, insert_sql_neg.c_str(), -1, &insert, &tail );

					// static bindings
					sqlite3_bind_int( insert, pos++, -1 );
					sqlite3_bind_int( insert, pos++, -1 );

					// negative bindings
					leaf_p = leaf_ids[1].begin();
					while ( leaf_p != leaf_ids[1].end() )
					{
						sqlite3_bind_int( insert, pos++, (*leaf_p) );
						leaf_p++;
					}

					// optimize bindings
					if ( before != EPMEM_MEMID_NONE )
					{
						sqlite3_bind_int( insert, pos++, before );
					}

					// perform insertion
					sqlite3_step( insert );
					sqlite3_finalize( insert );
				}
			}

			// take care of NULL ends, add indexes
			{
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NULL_RANGES ], 1, epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) - 1 );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NULL_RANGES ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_NULL_RANGES ] );

				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1 ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1 ] );

				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2 ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2 ] );
			}
			
			// modify ranges to accomodate prohibition			
			if ( !prohibit->empty() )
			{			
				vector<long *> prohibit_ranges;
				vector<long *>::iterator pr_p;

				long start = EPMEM_MEMID_NONE;
				long prev = EPMEM_MEMID_NONE;

				// sort
				std::sort( prohibit->begin(), prohibit->end() );

				// convert to ranges
				prohibit_p = prohibit->begin();
				while ( prohibit_p != prohibit->end() )
				{
					if ( ( prev + 1 ) == (*prohibit_p) )
					{
						prev = (*prohibit_p);
					}
					else
					{
						if ( start != EPMEM_MEMID_NONE )
						{
							long *range = new long[2];
							range[0] = start;
							range[1] = prev;

							prohibit_ranges.push_back( range );
						}

						start = (*prohibit_p);
						prev = start;
					}
					
					prohibit_p++;
				}
				{
					long *range = new long[2];
					range[0] = start;
					range[1] = prev;

					prohibit_ranges.push_back( range );
				}

				// remove ranges that are entirely contained within prohibit ranges
				{
					// dynamic sql
					string del_sql = "DELETE FROM ranges WHERE ";					
					string *clauses = string_multi_copy( "(start>=? AND end<=?) OR ", prohibit_ranges.size() - 1 );
					
					del_sql.append( *clauses );
					del_sql += "(start>=? AND end<=?)";
					delete clauses;

					// prep statement
					sqlite3_stmt *del;
					const char *tail;
					sqlite3_prepare_v2( my_agent->epmem_db, del_sql.c_str(), -1, &del, &tail );

					// dynamic bindings
					int pos = 1;
					pr_p = prohibit_ranges.begin();
					while ( pr_p != prohibit_ranges.end() )
					{
						sqlite3_bind_int( del, pos++, (*pr_p)[0] );
						sqlite3_bind_int( del, pos++, (*pr_p)[1] );
						
						pr_p++;
					}

					// execute
					sqlite3_step( del );
					sqlite3_finalize( del );
				}

				// update ranges that are on the fringes of prohibition
				{
					pr_p = prohibit_ranges.begin();
					while ( pr_p != prohibit_ranges.end() )
					{
						// start=? WHERE start BETWEEN ? AND ?
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ], 1, (*pr_p)[1] + 1 );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ], 2, (*pr_p)[0] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ], 3, (*pr_p)[1] );

						// end=? WHERE end BETWEEN ? AND ?
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ], 1, (*pr_p)[0] - 1 );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ], 2, (*pr_p)[0] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ], 3, (*pr_p)[1] );					

						// execute+reset
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ] );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ] );						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH ] );						
						
						pr_p++;
					}
				}

				// update ranges that contain prohibition
				{
					pr_p = prohibit_ranges.begin();
					while ( pr_p != prohibit_ranges.end() )
					{						
						// -1 start, start, end, +1 end, start, end
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 1, (*pr_p)[0] - 1 );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 2, (*pr_p)[0] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 3, (*pr_p)[1] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 4, (*pr_p)[1] + 1 );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 5, (*pr_p)[0] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ], 6, (*pr_p)[1] );

						// start<? AND end>?
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB ], 1, (*pr_p)[0] );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB ], 2, (*pr_p)[1] );

						// execute+reset						
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ] );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB ] );						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_DEL_PROHIB ] );
						
						pr_p++;
					}
				}
			}

			// play king of the mountain
			{
				long king_id = EPMEM_MEMID_NONE;
				double king_score = -1;
				double king_cardinality = 0;

				double balance = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_BALANCE );
				double balance_inv = 1 - balance;
				
				// dynamic programming stuff
				int sum_ct = 0;
				double sum_v = 0;

				// current pointer
				int current_list = EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES;
				long current_id = EPMEM_MEMID_NONE;
				int current_ct = 0;
				double current_v = 0;
				long current_end;
				long current_valid_end;
				double current_score;

				// next pointers
				long low_id = EPMEM_MEMID_NONE;
				long high_id = EPMEM_MEMID_NONE;
				long *next_id;
				int next_list;				
				
				// completion (allows for smart cut-offs later
				bool done = false;				

				// initialize current as first low
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ] );				
				current_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ], 0 );
				current_ct = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ], 1 );
				current_v = sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ], 2 );				
				
				// initialize next low				
				if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ] ) == SQLITE_ROW )
					low_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ], 0 );
				else
					low_id = EPMEM_MEMID_NONE;

				// initialize next high
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ] );
				high_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ], 0 );			

				do
				{
					// if both lists are finished, we are done
					if ( ( low_id == EPMEM_MEMID_NONE ) && ( high_id == EPMEM_MEMID_NONE ) )
					{
						done = true;
					}
					// if we are beyond a specified before, we are done
					else if ( ( before != EPMEM_MEMID_NONE ) && ( current_id >= before ) )
					{
						done = true;
					}
					// if one list is finished, we go to the other
					else if ( ( low_id == EPMEM_MEMID_NONE ) || ( high_id == EPMEM_MEMID_NONE ) )
					{
						next_list = ( ( low_id == EPMEM_MEMID_NONE )?( EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ):( EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ) );						
					}
					// if neither list is finished, we prefer the lower id (low in case of tie)
					else
					{
						next_list = ( ( low_id <= high_id )?( EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ):( EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ) );
					}

					// if we choose a list, update variables
					if ( !done )
					{
						// update sums
						sum_ct += current_ct;
						sum_v += current_v;

						// update end range
						current_end = ( ( next_list == EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES )?( high_id ):( low_id - 1 ) );
						if ( before == EPMEM_MEMID_NONE )
							current_valid_end = current_end;
						else
							current_valid_end = ( ( current_end < before )?( current_end ):( before - 1 ) );
						
						// if we are beyond after AND
						// we have cardinality, compute score
						// for possible new king
						if ( ( current_valid_end > after ) && ( sum_ct > 0 ) )
						{
							current_score = ( balance * current_ct ) + ( balance_inv * current_v );
							
							// we prefer more recent in case of tie
							if ( current_score >= king_score )
							{
								king_id = current_valid_end;
								king_score = current_score;
								king_cardinality = sum_ct;
							}
						}

						// based upon choice, update variables						
						current_list = next_list;
						current_id = current_end + 1;
						current_ct = ( ( next_list == EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES )?( 1 ):( -1 ) ) * sqlite3_column_int( my_agent->epmem_statements[ next_list ], 1 );
						current_v = ( ( next_list == EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES )?( 1 ):( -1 ) ) * sqlite3_column_double( my_agent->epmem_statements[ next_list ], 2 );

						next_id = ( ( next_list == EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES )?( &low_id ):( &high_id ) );
						if ( sqlite3_step( my_agent->epmem_statements[ next_list ] ) == SQLITE_ROW )
							( *next_id ) = sqlite3_column_int( my_agent->epmem_statements[ next_list ], 0 );
						else
							( *next_id ) = EPMEM_MEMID_NONE;
					}
				} while ( !done );

				
				// reset list queries
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES ] );
				
				if ( king_id != EPMEM_MEMID_NONE )
				{
					// status
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );

					// match score
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, make_float_constant( my_agent, king_score ) );

					// cue-size
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, make_int_constant( my_agent, cue_size ) );

					// normalized-match-score
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, make_float_constant( my_agent, ( king_score / cue_size ) ) );

					// match-cardinality
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, make_int_constant( my_agent, king_cardinality ) );

					// actual memory
					epmem_install_memory( my_agent, state, king_id );
				}
				else
				{
					add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
				}
			}

			// remove indexes
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1R ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1R ] );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2R ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2R ] );

			// delete all ranges and weights
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_WEIGHTS ] );
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_RANGES ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_WEIGHTS ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_TRUNCATE_RANGES ] );

			// finish transaction
			sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		}
	}
	else
	{
		add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );
		free_memory( my_agent, wmes_query, MISCELLANEOUS_MEM_USAGE );
		free_memory( my_agent, wmes_neg_query, MISCELLANEOUS_MEM_USAGE );
	}
}

/***************************************************************************
 * Function     : epmem_install_memory
 **************************************************************************/
void epmem_install_memory( agent *my_agent, Symbol *state, long memory_id )
{
	// get the ^result header for this state
	Symbol *result_header = state->id.epmem_result_header;

	// if no memory, say so
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, my_agent->epmem_no_memory_symbol );
		state->id.epmem_info->last_memory = EPMEM_MEMID_NONE;
		return;
	}

	// remember this as the last memory installed
	state->id.epmem_info->last_memory = memory_id;

	// create a new ^retrieved header for this result
	Symbol *retrieved_header = make_new_identifier( my_agent, 'R', result_header->id.level );
	add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, retrieved_header );

	// add *-id wme's
	add_input_wme( my_agent, result_header, my_agent->epmem_memory_id_symbol, make_int_constant( my_agent, memory_id ) );
	add_input_wme( my_agent, result_header, my_agent->epmem_present_id_symbol, make_int_constant( my_agent, epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) ) );

	if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		map<unsigned long, Symbol *> ids;
		unsigned long child_id;
		unsigned long parent_id;
		const char *name;
		int type_code;
		Symbol *attr = NULL;
		Symbol *value = NULL;
		Symbol *parent = NULL;

		ids[ 0 ] = retrieved_header;

		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 1, memory_id );
		while ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ] ) == SQLITE_ROW )
		{
			// e.id, i.parent_id, i.name, i.value
			child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 0 );
			parent_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 1 );
			name = (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 2 );
			type_code = sqlite3_column_type( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 3 );
			
			// make a symbol to represent the attribute name		
			attr = make_sym_constant( my_agent, const_cast<char *>( name ) );

			// get a reference to the parent
			parent = ids[ parent_id ];

			// identifier = NULL, else attr->val
			if ( type_code == SQLITE_NULL )
			{
				value = make_new_identifier( my_agent, name[0], parent->id.level );
				add_input_wme( my_agent, parent, attr, value );

				ids[ child_id ] = value;
			}
			else
			{
				switch ( type_code )
				{
					case SQLITE_INTEGER:
						value = make_int_constant( my_agent, sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 3 ) );
						break;

					case SQLITE_FLOAT:
						value = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 3 ) );
						break;

					case SQLITE_TEXT:						
						value = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ], 3 ) ) );
						break;
				}

				add_input_wme( my_agent, parent, attr, value );
			}
		}
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ] );
	}
	else if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		map<unsigned long, Symbol *> ids;
		unsigned long child_id;
		unsigned long parent_id;
		const char *name;
		int type_code;
		Symbol *attr = NULL;
		Symbol *value = NULL;
		Symbol *parent = NULL;

		ids[ 0 ] = retrieved_header;

		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 1, memory_id );
		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 2, memory_id );
		while ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ] ) == SQLITE_ROW )
		{
			// e.id, i.parent_id, i.name, i.value
			child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 0 );
			parent_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 1 );
			name = (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 2 );
			type_code = sqlite3_column_type( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 3 );
			
			// make a symbol to represent the attribute name		
			attr = make_sym_constant( my_agent, const_cast<char *>( name ) );

			// get a reference to the parent
			parent = ids[ parent_id ];

			// identifier = NULL, else attr->val
			if ( type_code == SQLITE_NULL )
			{
				value = make_new_identifier( my_agent, name[0], parent->id.level );
				add_input_wme( my_agent, parent, attr, value );

				ids[ child_id ] = value;
			}
			else
			{
				switch ( type_code )
				{
					case SQLITE_INTEGER:
						value = make_int_constant( my_agent, sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 3 ) );
						break;

					case SQLITE_FLOAT:
						value = make_float_constant( my_agent, sqlite3_column_double( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 3 ) );
						break;

					case SQLITE_TEXT:						
						value = make_sym_constant( my_agent, const_cast<char *>( (const char *) sqlite3_column_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 3 ) ) );
						break;
				}

				add_input_wme( my_agent, parent, attr, value );
			}
		}
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ] );
	}
}
