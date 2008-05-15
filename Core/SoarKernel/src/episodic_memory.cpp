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

#include "symtab.h"
#include "io_soar.h"
#include "wmem.h"

#include "xmlTraceNames.h"
#include "gski_event_system_functions.h"
#include "print.h"

#include "episodic_memory.h"

/***************************************************************************
 * Function     : epmem_clean_parameters
 **************************************************************************/
void epmem_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<EPMEM_PARAMS; i++ )
	  delete my_agent->epmem_params[ i ];
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
	newbie->param->string_param.val_func = val_func;
	newbie->param->string_param.to_str = to_str;
	newbie->param->string_param.from_str = from_str;
	newbie->param->string_param.value = value;
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
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->string_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const char *name, const char *test )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->string_param.to_str( my_agent->epmem_params[ param ]->param->string_param.value );
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

	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->string_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const long param, const char *test )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return NULL;
	
	return my_agent->epmem_params[ param ]->param->string_param.to_str( my_agent->epmem_params[ param ]->param->string_param.value );
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
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return false;
	
	return my_agent->epmem_params[ param ]->param->string_param.val_func( my_agent->epmem_params[ param ]->param->string_param.from_str( new_val ) );
}

bool epmem_valid_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return false;
	
	return my_agent->epmem_params[ param ]->param->string_param.val_func( new_val );
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
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return false;
	
	return my_agent->epmem_params[ param ]->param->string_param.val_func( my_agent->epmem_params[ param ]->param->string_param.from_str( new_val ) );
}

bool epmem_valid_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return false;
	
	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_string )
		return false;
	
	return my_agent->epmem_params[ param ]->param->string_param.val_func( new_val );
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

	const long converted_val = my_agent->epmem_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	
	my_agent->epmem_params[ param ]->param->string_param.value = converted_val;

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
	
	my_agent->epmem_params[ param ]->param->string_param.value = new_val;

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

	const long converted_val = my_agent->epmem_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	
	my_agent->epmem_params[ param ]->param->string_param.value = converted_val;

	return true;
}

bool epmem_set_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( !epmem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
		set_sysparam( my_agent, EPMEM_ENABLED, new_val );
	
	my_agent->epmem_params[ param ]->param->string_param.value = new_val;

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
 * Function     : epmem_update
 **************************************************************************/
void epmem_update( agent *my_agent )
{
	slot *s;
	wme *w;
	Symbol *ol = my_agent->io_header_output;
	bool new_memory = false;
		
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
	
	if ( new_memory )
	{				
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%d)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number );
		
		print( my_agent, buf );
		
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
		gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
	}
}
