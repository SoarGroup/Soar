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
#include <set>

#include "symtab.h"
#include "io_soar.h"
#include "wmem.h"

#include "xmlTraceNames.h"
#include "gski_event_system_functions.h"
#include "print.h"

#include "episodic_memory.h"

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
bool epmem_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
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
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->epmem_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const long param, const char *new_val )
{
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
	return ( ( new_val > 0 ) && ( new_val <= EPMEM_TRIGGER_OUTPUT ) );
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
	}
	
	return return_val;
}

const long epmem_convert_trigger( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "output" ) )
		return_val = EPMEM_TRIGGER_OUTPUT;	
	
	return return_val;
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

	return my_agent->epmem_stats[ stat ]->value;
}

double epmem_get_stat( agent *my_agent, const long stat )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
		return 0;

	return my_agent->epmem_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : epmem_set_stat
 **************************************************************************/
bool epmem_set_stat( agent *my_agent, const char *name, double new_val )
{
	const long stat = epmem_convert_stat( my_agent, name );
	if ( stat == EPMEM_STATS )
		return false;
	
	my_agent->epmem_stats[ stat ]->value = new_val;
	
	return true;
}

bool epmem_set_stat( agent *my_agent, const long stat, double new_val )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
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
	
	// Generate a hash value for the WME's attr and value
	hash_value = hash_string( w->attr->sc.name );
	
	switch( w->value->common.symbol_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
			hash_value += hash_string( w->value->sc.name );
			break;
            
		case INT_CONSTANT_SYMBOL_TYPE:
			hash_value += w->value->ic.value;
			break;
		
		case FLOAT_CONSTANT_SYMBOL_TYPE:
			hash_value += ( unsigned long )w->value->fc.value;
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
		int rc;
					
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
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT e.id, i.parent_id, i.name, i.value FROM episodes e INNER JOIN ids i ON e.id=i.child_id WHERE e.time=? ORDER BY e.id ASC", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_GET_EPISODE ] ), &tail );

				// get max time
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(time) FROM episodes", -1, &create, &tail );
				if ( sqlite3_step( create ) == SQLITE_ROW )						
					my_agent->epmem_time_counter = ( sqlite3_column_int( create, 0 ) + 1 );
				sqlite3_finalize( create );
				
				break;

			case EPMEM_INDEXING_BIGTREE_RANGE:
				// episodes table
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS episodes (id INTEGER,start INTEGER,end INTEGER)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );			
				
				// end_id index (for updates)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE UNIQUE INDEX IF NOT EXISTS episode_end_id ON episodes (end,id)", -1, &create, &tail );
				sqlite3_step( create );					
				sqlite3_finalize( create );

				// start/end index (for retrieval)
				sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS episode_start_end ON episodes (start,end)", -1, &create, &tail );
				sqlite3_step( create );
				sqlite3_finalize( create );			

				// custom statement for updating episodes
				sqlite3_prepare_v2( my_agent->epmem_db, "UPDATE episodes SET end=? WHERE id=? AND end=?", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] ), &tail );

				// custom statement for inserting episodes
				sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO episodes (id,start,end) VALUES (?,?,?)", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] ), &tail );			
				
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
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT e.id, i.parent_id, i.name, i.value FROM episodes e INNER JOIN ids i ON e.id=i.child_id WHERE ? BETWEEN e.start AND e.end ORDER BY e.id ASC", -1, &( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ] ), &tail );

				// get max time
				sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(end) FROM episodes", -1, &create, &tail );
				if ( sqlite3_step( create ) == SQLITE_ROW )						
					my_agent->epmem_time_counter = ( sqlite3_column_int( create, 0 ) + 1 );
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
				
		data->last_tag = 0;
		
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
			
		// examine all commands on the output-link for any
		// that appeared since last memory was recorded
		for ( s = ol->id.slots; s != NIL; s = s->next )
		{
			for ( w = s->wmes; w != NIL; w = w->next )
			{
				if ( w->timetag > my_agent->bottom_goal->id.epmem_info->last_tag )
				{
					new_memory = true;
					my_agent->bottom_goal->id.epmem_info->last_tag = w->timetag; 
				}
			}
		}
	}
	
	if ( new_memory )
		epmem_new_episode( my_agent );
}

/***************************************************************************
 * Function     : epmem_new_episode
 **************************************************************************/
void epmem_new_episode( agent *my_agent )
{	
	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%d)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number );
		
		print( my_agent, buf );
		
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
	}
	
	// if this is the first episode, initialize db components	
	if ( my_agent->epmem_db_status == -1 )
		epmem_init_db( my_agent );
	
	// add the episode only if db is properly initialized
	if ( my_agent->epmem_db_status != SQLITE_OK )
		return;
	
	if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		wme **wmes = NULL;
		int len = 0;
		
		vector<Symbol *> syms;
		vector<int> ids;
		unsigned int pos = 0;

		int parent_id;
		int child_id;
		map<int, double *> epmem;

		unsigned long my_hash;
		int tc = my_agent->top_goal->id.tc_num + 3;

		int i;	

		syms.push_back( my_agent->top_goal );
		ids.push_back( 0 );	
		
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		while ( pos != syms.size() )
		{		
			parent_sym = syms[ pos ];
			parent_id = ids[ pos ];
			pos++;				
			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				for ( i=0; i<len; i++ )
				{
					// find wme id
					child_id = -1;
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
							child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ], 0 );
						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID ] );
					}
					else
					{
						// hash=? AND parent_id=? AND name=? AND value IS NULL
						
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 1, my_hash );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 2, parent_id );
						sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );

						if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] ) == SQLITE_ROW )
							child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ], 0 );
						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_FIND_ID_NULL ] );
					}
					
					// insert on no id
					if ( child_id == -1 )
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

						child_id = sqlite3_last_insert_rowid( my_agent->epmem_db );
					}
					
					// keep track of identifiers (for further study)
					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						syms.push_back( wmes[i]->value );
						ids.push_back( child_id );

						epmem[ child_id ] = NULL;
					}
					else
					{
						// for clarity:
						// map initializes a new element.
						// I want to point to that address and change it if necessary.
						double **p =& epmem[ child_id ];
						
						// replace 1 here with actual weight
						if ( *p == NULL )
							*p = new double(1);
						else if ( 1 > **p )
							**p = 1;					
					}
				}

				// free space from aug list
				free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
			}
		}

		// all inserts at once (provides unique)
		map<int, double *>::iterator e = epmem.begin();
		while ( e != epmem.end() )
		{
			// add nodes to the episodic store
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 1, e->first );
			sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_I_ADD_EPISODE ], 2, my_agent->epmem_time_counter );

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
		my_agent->epmem_time_counter++;
	}
	else if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_RANGE )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		wme **wmes = NULL;
		int len = 0;
		
		vector<Symbol *> syms;
		vector<int> ids;
		unsigned int pos = 0;

		int parent_id;
		int child_id;
		bool newbie;
		map<int, bool> epmem;

		unsigned long my_hash;
		int tc = my_agent->top_goal->id.tc_num + 3;

		int i;	

		syms.push_back( my_agent->top_goal );
		ids.push_back( 0 );	
		
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BEGIN ] );
		while ( pos != syms.size() )
		{		
			parent_sym = syms[ pos ];
			parent_id = ids[ pos ];
			pos++;				
			wmes = epmem_get_augs_of_id( my_agent, parent_sym, tc, &len );

			if ( wmes != NULL )
			{
				for ( i=0; i<len; i++ )
				{
					// find wme id
					child_id = -1;
					newbie = false;
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
							child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ], 0 );
						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID ] );
					}
					else
					{
						// hash=? AND parent_id=? AND name=? AND value IS NULL
						
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 1, my_hash );
						sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 2, parent_id );
						sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 3, (const char *) wmes[i]->attr->sc.name, -1, SQLITE_STATIC );

						if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] ) == SQLITE_ROW )
							child_id = sqlite3_column_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ], 0 );
						
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_FIND_ID_NULL ] );
					}
					
					// insert on no id
					if ( child_id == -1 )
					{						
						newbie = true;
						
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

						child_id = sqlite3_last_insert_rowid( my_agent->epmem_db );
					}
					
					// keep track of identifiers (for further study)
					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						syms.push_back( wmes[i]->value );
						ids.push_back( child_id );
					}
					
					// record for insertion
					if ( !epmem[ child_id ] && newbie )
						epmem[ child_id ] = true;					
				}

				// free space from aug list
				free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );
			}
		}

		// all inserts at once (provides unique)
		map<int, bool>::iterator e = epmem.begin();
		int updated;
		while ( e != epmem.end() )
		{
			// add nodes to the episodic store
			updated = 0;
			if ( !e->second )
			{
				// SET end=? WHERE id=? AND end=?
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ], 1, my_agent->epmem_time_counter );
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ], 2, e->first );
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ], 3, ( my_agent->epmem_time_counter - 1 ) );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] );
				
				updated = sqlite3_changes( my_agent->epmem_db );
				
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE ] );
			}
			if ( !updated )
			{
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ], 1, e->first );
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ], 2, my_agent->epmem_time_counter );
				sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ], 3, my_agent->epmem_time_counter );
				sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] );
				sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_ADD_EPISODE ] );
			}

			e++;
		}

		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_COMMIT ] );	
		my_agent->epmem_time_counter++;
	}

	if ( my_agent->epmem_time_counter == 2 )
		epmem_install_memory( my_agent, my_agent->top_goal, 1 );
}

void epmem_install_memory( agent *my_agent, Symbol *state, int memory_id )
{
	// get the ^result header for this state
	Symbol *result_header = state->id.epmem_result_header;

	// if intentionally no memory, say so
	if ( memory_id == EPMEM_MEMID_NONE )
	{
		add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, my_agent->epmem_no_memory_symbol );
		return;
	}

	// create a new ^retrieved header for this result
	Symbol *retrieved_header = make_new_identifier( my_agent, 'R', result_header->id.level );
	add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, retrieved_header );

	// add *-id wme's
	add_input_wme( my_agent, result_header, my_agent->epmem_memory_id_symbol, make_int_constant( my_agent, memory_id ) );
	add_input_wme( my_agent, result_header, my_agent->epmem_present_id_symbol, make_int_constant( my_agent, my_agent->epmem_time_counter ) );

	if ( epmem_get_parameter( my_agent, EPMEM_PARAM_INDEXING, EPMEM_RETURN_LONG ) == EPMEM_INDEXING_BIGTREE_INSTANCE )
	{
		map<int, Symbol *> ids;
		int child_id;
		int parent_id;
		const char *name;
		int type_code;
		Symbol *attr;
		Symbol *value;
		Symbol *parent;

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
		map<int, Symbol *> ids;
		int child_id;
		int parent_id;
		const char *name;
		int type_code;
		Symbol *attr;
		Symbol *value;
		Symbol *parent;

		ids[ 0 ] = retrieved_header;

		sqlite3_bind_int( my_agent->epmem_statements[ EPMEM_STMT_BIGTREE_R_GET_EPISODE ], 1, memory_id );
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
