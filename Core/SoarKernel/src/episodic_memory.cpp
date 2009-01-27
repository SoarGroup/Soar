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

// defined in symtab.cpp but not in symtab.h
extern unsigned long hash_string( const char *s );


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				epmem::param
// stats 						epmem::stats
// timers 						epmem::timers

// wme-related					epmem::wmes
// preference-related 			epmem::prefs

// sqlite query					epmemepmem::query
// sqlite transactions			epmem::transaction
// variable abstraction			epmem::var

// relational interval tree		epmem::rit

// cleaning up					epmem::clean
// initialization				epmem::init

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
const long epmem_rit_state_one[6] = { EPMEM_VAR_RIT_OFFSET_1, EPMEM_VAR_RIT_LEFTROOT_1, EPMEM_VAR_RIT_RIGHTROOT_1, EPMEM_VAR_RIT_MINSTEP_1, EPMEM_STMT_ONE_ADD_NODE_RANGE, EPMEM_TIMER_NCB_NODE_RIT };

// first array adheres to the EPMEM_RIT_STATE_NODE/EDGE
// predefines
const long epmem_rit_state_three[2][6] =
{
	{ EPMEM_VAR_RIT_OFFSET_1, EPMEM_VAR_RIT_LEFTROOT_1, EPMEM_VAR_RIT_RIGHTROOT_1, EPMEM_VAR_RIT_MINSTEP_1, EPMEM_STMT_THREE_ADD_NODE_RANGE, EPMEM_TIMER_NCB_NODE_RIT },
	{ EPMEM_VAR_RIT_OFFSET_2, EPMEM_VAR_RIT_LEFTROOT_2, EPMEM_VAR_RIT_RIGHTROOT_2, EPMEM_VAR_RIT_MINSTEP_2, EPMEM_STMT_THREE_ADD_EDGE_RANGE, EPMEM_TIMER_NCB_EDGE_RIT }
};

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (epmem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_clean_parameters
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for parameters
 **************************************************************************/
void epmem_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<EPMEM_PARAMS; i++ )
	{
		if ( my_agent->epmem_params[ i ]->type == epmem_param_string )
			delete my_agent->epmem_params[ i ]->param->string_param.value;

		delete my_agent->epmem_params[ i ]->param;
		delete my_agent->epmem_params[ i ];
	}
}

/***************************************************************************
 * Function     : epmem_new_parameter
 * Author		: Nate Derbinsky
 * Notes		: Creates a new parameter (of desired type)
 **************************************************************************/
epmem_parameter *epmem_new_parameter( const char *name, double value, bool (*val_func)( double ) )
{
	epmem_parameter *newbie = new epmem_parameter;
	newbie->param = new epmem_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = epmem_param_number;
	newbie->name = name;

	return newbie;
}

epmem_parameter *epmem_new_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
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

epmem_parameter *epmem_new_parameter( const char *name, const char *value, bool (*val_func)( const char * ) )
{
	epmem_parameter *newbie = new epmem_parameter;
	newbie->param = new epmem_parameter_union;
	newbie->param->string_param.value = new std::string( value );
	newbie->param->string_param.val_func = val_func;
	newbie->type = epmem_param_string;
	newbie->name = name;

	return newbie;
}

/***************************************************************************
 * Function     : epmem_convert_parameter
 * Author		: Nate Derbinsky
 * Notes		: Convert parameter name <=> constant
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
 * Author		: Nate Derbinsky
 * Notes		: Determines if a parameter name/number is valid
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
 * Author		: Nate Derbinsky
 * Notes		: Returns the parameter type
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
 * Author		: Nate Derbinsky
 * Notes		: Get the parameter value
 **************************************************************************/
const long epmem_get_parameter( agent *my_agent, const char *name, const double /*test*/ )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return NULL;

	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;

	return my_agent->epmem_params[ param ]->param->constant_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const char *name, const char * /*test*/ )
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

const long epmem_get_parameter( agent *my_agent, const long param, const double /*test*/ )
{
	if ( !epmem_valid_parameter( my_agent, param ) )
		return NULL;

	if ( epmem_get_parameter_type( my_agent, param ) != epmem_param_constant )
		return NULL;

	return my_agent->epmem_params[ param ]->param->constant_param.value;
}

const char *epmem_get_parameter( agent *my_agent, const long param, const char * /*test*/ )
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
 * Function     : epmem_enabled
 * Author		: Nate Derbinsky
 * Notes		: Shortcut function to system parameter
 **************************************************************************/
bool epmem_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ EPMEM_ENABLED ] == EPMEM_LEARNING_ON );
}

/***************************************************************************
 * Function     : epmem_valid_parameter_value
 * Author		: Nate Derbinsky
 * Notes		: Returns if a value is valid for the parameter
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
 * Function     : epmem_parameter_protected
 * Author		: Nate Derbinsky
 * Notes		: Returns true if a parameter is currently protected
 * 				  from modification
 **************************************************************************/
bool epmem_parameter_protected( agent *my_agent, const long param )
{
	return ( ( my_agent->epmem_db_status != EPMEM_DB_CLOSED ) && ( param >= EPMEM_PARAM_DB ) && ( param <= EPMEM_PARAM_MODE ) );
}

/***************************************************************************
 * Function     : epmem_set_parameter
 * Author		: Nate Derbinsky
 * Notes		: Set parameter value
 * 				  Special considerations for commit, exclusions
 * 				  learning, graph-match
 **************************************************************************/
bool epmem_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = epmem_convert_parameter( my_agent, name );
	if ( param == EPMEM_PARAMS )
		return false;

	if ( epmem_parameter_protected( my_agent, param ) )
		return false;

	// special case of commit needing conversion to int
	if ( param == EPMEM_PARAM_COMMIT )
		new_val = floor( new_val );

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

	// exclusions special case
	if ( param == EPMEM_PARAM_EXCLUSIONS )
	{
		std::string new_implode;

		// search through exclusions
		std::list<const char *>::iterator e_p = my_agent->epmem_exclusions->begin();
		while ( e_p != my_agent->epmem_exclusions->end() )
		{
			if ( strcmp( new_val, (*e_p) ) == 0 )
				break;

			e_p++;
		}

		// remove if found
		if ( e_p != my_agent->epmem_exclusions->end() )
		{
			delete (*e_p);
			my_agent->epmem_exclusions->erase( e_p );

			// get new list
			e_p = my_agent->epmem_exclusions->begin();
			while ( e_p != my_agent->epmem_exclusions->end() )
			{
				new_implode.append( (*e_p) );

				e_p++;

				if ( e_p != my_agent->epmem_exclusions->end() )
					new_implode.append( ", " );
			}
		}
		// otherwise it's new
		else
		{
			char *newbie = new char[ strlen( new_val ) + 1 ];
			strcpy( newbie, new_val );
			my_agent->epmem_exclusions->push_back( newbie );

			new_implode  = (*my_agent->epmem_params[ param ]->param->string_param.value);
			if ( !new_implode.empty() )
				new_implode.append( ", " );

			new_implode.append( new_val );
		}

		// keep comma-separated list around
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_implode;
		return true;
	}

	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
	{
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}

	const long converted_val = my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	}
	// graph match headaches
	else if ( param == EPMEM_PARAM_GRAPH_MATCH )
	{
		if ( ( converted_val != EPMEM_GRAPH_MATCH_OFF ) && ( my_agent->epmem_params[ EPMEM_PARAM_MODE ]->param->constant_param.value != EPMEM_MODE_THREE ) )
			return false;
	}
	else if ( param == EPMEM_PARAM_MODE )
	{
		if ( converted_val != EPMEM_MODE_THREE )
			my_agent->epmem_params[ EPMEM_PARAM_GRAPH_MATCH ]->param->constant_param.value = EPMEM_GRAPH_MATCH_OFF;
	}

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
	{
		set_sysparam( my_agent, EPMEM_ENABLED, new_val );
	}
	// graph match headaches
	else if ( param == EPMEM_PARAM_GRAPH_MATCH )
	{
		if ( ( new_val != EPMEM_GRAPH_MATCH_OFF ) && ( my_agent->epmem_params[ EPMEM_PARAM_MODE ]->param->constant_param.value != EPMEM_MODE_THREE ) )
			return false;
	}
	else if ( param == EPMEM_PARAM_MODE )
	{
		if ( new_val != EPMEM_MODE_THREE )
			my_agent->epmem_params[ EPMEM_PARAM_GRAPH_MATCH ]->param->constant_param.value = EPMEM_GRAPH_MATCH_OFF;
	}

	my_agent->epmem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//

bool epmem_set_parameter( agent *my_agent, const long param, double new_val )
{
	if ( epmem_parameter_protected( my_agent, param ) )
		return false;

	// special case of commit needing conversion to int
	if ( param == EPMEM_PARAM_COMMIT )
		new_val = floor( new_val );

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

	// exclusions special case
	if ( param == EPMEM_PARAM_EXCLUSIONS )
	{
		std::string new_implode;

		// search through exclusions
		std::list<const char *>::iterator e_p = my_agent->epmem_exclusions->begin();
		while ( e_p != my_agent->epmem_exclusions->end() )
		{
			if ( strcmp( new_val, (*e_p) ) == 0 )
				break;

			e_p++;
		}

		// remove if found
		if ( e_p != my_agent->epmem_exclusions->end() )
		{
			delete (*e_p);
			my_agent->epmem_exclusions->erase( e_p );

			// get new list
			e_p = my_agent->epmem_exclusions->begin();
			while ( e_p != my_agent->epmem_exclusions->end() )
			{
				new_implode.append( (*e_p) );

				e_p++;

				if ( e_p != my_agent->epmem_exclusions->end() )
					new_implode.append( ", " );
			}
		}
		// otherwise it's new
		else
		{
			char *newbie = new char[ strlen( new_val ) + 1 ];
			strcpy( newbie, new_val );
			my_agent->epmem_exclusions->push_back( newbie );

			new_implode = (*my_agent->epmem_params[ param ]->param->string_param.value);
			if ( !new_implode.empty() )
				new_implode.append( ", " );

			new_implode.append( new_val );
		}

		// keep comma-separated list around
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_implode;
		return true;
	}

	if ( epmem_get_parameter_type( my_agent, param ) == epmem_param_string )
	{
		(*my_agent->epmem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}

	const long converted_val = my_agent->epmem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == EPMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, EPMEM_ENABLED, converted_val );
	}
	// graph match headaches
	if ( ( converted_val != EPMEM_GRAPH_MATCH_OFF ) && ( my_agent->epmem_params[ EPMEM_PARAM_MODE ]->param->constant_param.value != EPMEM_MODE_THREE ) )
	{
		if ( my_agent->epmem_params[ EPMEM_PARAM_MODE ]->param->constant_param.value != EPMEM_MODE_THREE )
			return false;
	}
	else if ( param == EPMEM_PARAM_MODE )
	{
		if ( converted_val != EPMEM_MODE_THREE )
			my_agent->epmem_params[ EPMEM_PARAM_GRAPH_MATCH ]->param->constant_param.value = EPMEM_GRAPH_MATCH_OFF;
	}

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
	{
		set_sysparam( my_agent, EPMEM_ENABLED, new_val );
	}
	// graph match headaches
	else if ( param == EPMEM_PARAM_GRAPH_MATCH )
	{
		if ( ( new_val != EPMEM_GRAPH_MATCH_OFF ) && ( my_agent->epmem_params[ EPMEM_PARAM_MODE ]->param->constant_param.value != EPMEM_MODE_THREE ) )
			return false;
	}
	else if ( param == EPMEM_PARAM_MODE )
	{
		if ( new_val != EPMEM_MODE_THREE )
			my_agent->epmem_params[ EPMEM_PARAM_GRAPH_MATCH ]->param->constant_param.value = EPMEM_GRAPH_MATCH_OFF;
	}

	my_agent->epmem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//
// validation/conversion functions for parameters
//

// learning parameter
bool epmem_validate_learning( const long new_val )
{
	return ( ( new_val == EPMEM_LEARNING_ON ) || ( new_val == EPMEM_LEARNING_OFF ) );
}

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

// database parameter
bool epmem_validate_database( const long new_val )
{
	return ( ( new_val == EPMEM_DB_MEM ) || ( new_val == EPMEM_DB_FILE ) );
}

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

// path parameter
bool epmem_validate_path( const char * /*new_val*/ )
{
	return true;
}

// mode parameter
bool epmem_validate_mode( const long new_val )
{
	return ( ( new_val == EPMEM_MODE_ONE ) || ( new_val == EPMEM_MODE_THREE ) );
}

const char *epmem_convert_mode( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case EPMEM_MODE_ONE:
			return_val = "tree";
			break;

		case EPMEM_MODE_THREE:
			return_val = "graph";
			break;
	}

	return return_val;
}

const long epmem_convert_mode( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "tree" ) )
		return_val = EPMEM_MODE_ONE;
	else if ( !strcmp( val, "graph" ) )
		return_val = EPMEM_MODE_THREE;

	return return_val;
}

// graph match parameter
bool epmem_validate_graph_match( const long new_val )
{
	return ( ( new_val >= EPMEM_GRAPH_MATCH_OFF ) && ( new_val <= EPMEM_GRAPH_MATCH_WMES ) );
}

const char *epmem_convert_graph_match( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case EPMEM_GRAPH_MATCH_OFF:
			return_val = "off";
			break;

		case EPMEM_GRAPH_MATCH_PATHS:
			return_val = "paths";
			break;

		case EPMEM_GRAPH_MATCH_WMES:
			return_val = "full";
			break;
	}

	return return_val;
}

const long epmem_convert_graph_match( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "off" ) )
		return_val = EPMEM_GRAPH_MATCH_OFF;
	else if ( !strcmp( val, "paths" ) )
		return_val = EPMEM_GRAPH_MATCH_PATHS;
	else if ( !strcmp( val, "full" ) )
		return_val = EPMEM_GRAPH_MATCH_WMES;

	return return_val;
}

// trigger parameter
bool epmem_validate_trigger( const long new_val )
{
	return ( ( new_val > 0 ) && ( new_val <= EPMEM_TRIGGER_DC ) );
}

const char *epmem_convert_trigger( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case EPMEM_TRIGGER_NONE:
			return_val = "none";
			break;

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

	if ( !strcmp( val, "none" ) )
		return_val = EPMEM_TRIGGER_NONE;

	if ( !strcmp( val, "output" ) )
		return_val = EPMEM_TRIGGER_OUTPUT;

	if ( !strcmp( val, "dc" ) )
		return_val = EPMEM_TRIGGER_DC;

	return return_val;
}

// force parameter
bool epmem_validate_force( const long new_val )
{
	return ( ( new_val >= EPMEM_FORCE_REMEMBER ) && ( new_val <= EPMEM_FORCE_OFF ) );
}

const char *epmem_convert_force( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case EPMEM_FORCE_REMEMBER:
			return_val = "remember";
			break;

		case EPMEM_FORCE_IGNORE:
			return_val = "ignore";
			break;

		case EPMEM_FORCE_OFF:
			return_val = "off";
			break;
	}

	return return_val;
}

const long epmem_convert_force( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "remember" ) )
		return_val = EPMEM_FORCE_REMEMBER;
	else if ( !strcmp( val, "ignore" ) )
		return_val = EPMEM_FORCE_IGNORE;
	else if ( !strcmp( val, "off" ) )
		return_val = EPMEM_FORCE_OFF;

	return return_val;
}

// balance parameter
bool epmem_validate_balance( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

// exclusions parameter
bool epmem_validate_exclusions( const char * /*new_val*/ )
{
	return true;
}

// commit parameter
bool epmem_validate_commit( const double new_val )
{
	return ( new_val > 0 );
}

// timers parameter
bool epmem_validate_ext_timers( const long new_val )
{
	return ( ( new_val >= EPMEM_TIMERS_OFF ) && ( new_val <= EPMEM_TIMERS_THREE ) );
}

const char *epmem_convert_ext_timers( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case EPMEM_TIMERS_OFF:
			return_val = "off";
			break;

		case EPMEM_TIMERS_ONE:
			return_val = "one";
			break;

		case EPMEM_TIMERS_TWO:
			return_val = "two";
			break;

		case EPMEM_TIMERS_THREE:
			return_val = "three";
			break;
	}

	return return_val;
}

const long epmem_convert_ext_timers( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "off" ) )
		return_val = EPMEM_TIMERS_OFF;
	else if ( !strcmp( val, "one" ) )
		return_val = EPMEM_TIMERS_ONE;
	else if ( !strcmp( val, "two" ) )
		return_val = EPMEM_TIMERS_TWO;
	else if ( !strcmp( val, "three" ) )
		return_val = EPMEM_TIMERS_THREE;

	return return_val;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (epmem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_clean_stats
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for statistics
 **************************************************************************/
void epmem_clean_stats( agent *my_agent )
{
	for ( int i=0; i<EPMEM_STATS; i++ )
	  delete my_agent->epmem_stats[ i ];
}

/***************************************************************************
 * Function     : epmem_stat_protected
 * Author		: Nate Derbinsky
 * Notes		: Determines if a statistic is protected from resetting
 **************************************************************************/
bool epmem_stat_protected( agent *my_agent, const long stat )
{
	return ( ( my_agent->epmem_db_status != EPMEM_DB_CLOSED ) &&
		     ( ( stat >= EPMEM_STAT_RIT_OFFSET_1 ) && ( stat <= EPMEM_STAT_RIT_MINSTEP_1 ) ) ||
			 ( ( stat >= EPMEM_STAT_RIT_OFFSET_2 ) && ( stat <= EPMEM_STAT_RIT_MINSTEP_2 ) ) ||
			 ( stat == EPMEM_STAT_TIME ) );
}

/***************************************************************************
 * Function     : epmem_reset_stats
 * Author		: Nate Derbinsky
 * Notes		: Reesets unprotected statistics
 **************************************************************************/
void epmem_reset_stats( agent *my_agent )
{
	for ( int i=0; i<EPMEM_STATS; i++ )
		if ( !epmem_stat_protected( my_agent, i ) )
			my_agent->epmem_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : epmem_new_stat
 * Author		: Nate Derbinsky
 * Notes		: Creates a new statistic
 **************************************************************************/
epmem_stat *epmem_new_stat( const char *name )
{
	epmem_stat *newbie = new epmem_stat;
	newbie->name = name;
	newbie->value = 0;

	return newbie;
}

/***************************************************************************
 * Function     : epmem_convert_stat
 * Author		: Nate Derbinsky
 * Notes		: Convert between statistic name<=>number
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
 * Author		: Nate Derbinsky
 * Notes		: Return if a statistic name/number is valid
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
 * Author		: Nate Derbinsky
 * Notes		: Get a statistic value
 * 				  Special consideration for sqlite info
 **************************************************************************/
EPMEM_TYPE_INT epmem_get_stat( agent *my_agent, const char *name )
{
	const long stat = epmem_convert_stat( my_agent, name );
	if ( stat == EPMEM_STATS )
		return 0;

	if ( stat == EPMEM_STAT_MEM_USAGE )
		return (EPMEM_TYPE_INT) sqlite3_memory_used();
	if ( stat == EPMEM_STAT_MEM_HIGH )
		return (EPMEM_TYPE_INT) sqlite3_memory_highwater( false );

	return my_agent->epmem_stats[ stat ]->value;
}

EPMEM_TYPE_INT epmem_get_stat( agent *my_agent, const long stat )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
		return 0;

	if ( stat == EPMEM_STAT_MEM_USAGE )
		return (EPMEM_TYPE_INT) sqlite3_memory_used();
	if ( stat == EPMEM_STAT_MEM_HIGH )
		return (EPMEM_TYPE_INT) sqlite3_memory_highwater( false );

	return my_agent->epmem_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : epmem_set_stat
 * Author		: Nate Derbinsky
 * Notes		: Set a statistic value
 * 				  Special consideration for sqlite info
 **************************************************************************/
bool epmem_set_stat( agent *my_agent, const char *name, EPMEM_TYPE_INT new_val )
{
	const long stat = epmem_convert_stat( my_agent, name );
	if ( ( stat == EPMEM_STATS ) ||
		 ( stat == EPMEM_STAT_MEM_USAGE ) ||
		 ( stat == EPMEM_STAT_MEM_HIGH ) )
		return false;

	my_agent->epmem_stats[ stat ]->value = new_val;

	return true;
}

bool epmem_set_stat( agent *my_agent, const long stat, EPMEM_TYPE_INT new_val )
{
	if ( !epmem_valid_stat( my_agent, stat ) )
		return false;

	if ( ( stat == EPMEM_STAT_MEM_USAGE ) ||
		 ( stat == EPMEM_STAT_MEM_HIGH ) )
		return false;

	my_agent->epmem_stats[ stat ]->value = new_val;

	return true;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (epmem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_clean_timers
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for timers
 **************************************************************************/
void epmem_clean_timers( agent *my_agent )
{
	for ( int i=0; i<EPMEM_TIMERS; i++ )
		delete my_agent->epmem_timers[ i ];
}

/***************************************************************************
 * Function     : epmem_reset_timers
 * Author		: Nate Derbinsky
 * Notes		: Resets all timers
 **************************************************************************/
void epmem_reset_timers( agent *my_agent )
{
	for ( int i=0; i<EPMEM_TIMERS; i++ )
	{
		reset_timer( &my_agent->epmem_timers[ i ]->start_timer );
		reset_timer( &my_agent->epmem_timers[ i ]->total_timer );
	}
}

/***************************************************************************
 * Function     : epmem_new_timer
 * Author		: Nate Derbinsky
 * Notes		: Creates a new initialized timer
 **************************************************************************/
epmem_timer *epmem_new_timer( const char *name, long level )
{
	// new timer entry
	epmem_timer *newbie = new epmem_timer;
	newbie->name = name;
	newbie->level = level;

	reset_timer( &newbie->start_timer );
	reset_timer( &newbie->total_timer );

	return newbie;
}

/***************************************************************************
 * Function     : epmem_convert_timer
 * Author		: Nate Derbinsky
 * Notes		: Convert a timer name<=>number
 **************************************************************************/
const long epmem_convert_timer( agent *my_agent, const char *name )
{
	for ( int i=0; i<EPMEM_TIMERS; i++ )
		if ( !strcmp( name, my_agent->epmem_timers[ i ]->name ) )
			return i;

	return EPMEM_TIMERS;
}

const char *epmem_convert_timer( agent *my_agent, const long timer )
{
	if ( ( timer < 0 ) || ( timer >= EPMEM_TIMERS ) )
		return NULL;

	return my_agent->epmem_timers[ timer ]->name;
}

/***************************************************************************
 * Function     : epmem_valid_timer
 * Author		: Nate Derbinsky
 * Notes		: Determines if the timer name/number is valid
 **************************************************************************/
bool epmem_valid_timer( agent *my_agent, const char *name )
{
	return ( epmem_convert_timer( my_agent, name ) != EPMEM_TIMERS );
}

bool epmem_valid_timer( agent *my_agent, const long timer )
{
	return ( epmem_convert_timer( my_agent, timer ) != NULL );
}

/***************************************************************************
 * Function     : epmem_get_timer_value
 * Author		: Nate Derbinsky
 * Notes		: Returns the current value of the timer
 **************************************************************************/
double epmem_get_timer_value( agent *my_agent, const char *name )
{
	const long timer = epmem_convert_timer( my_agent, name );
	if ( timer == EPMEM_TIMERS )
		return 0.0;

	return timer_value( &my_agent->epmem_timers[ timer ]->total_timer );
}

double epmem_get_timer_value( agent *my_agent, const long timer )
{
	if ( !epmem_valid_timer( my_agent, timer ) )
		return 0.0;

	return timer_value( &my_agent->epmem_timers[ timer ]->total_timer );
}

/***************************************************************************
 * Function     : epmem_get_timer_name
 * Author		: Nate Derbinsky
 * Notes		: Returns the timer name
 **************************************************************************/
const char *epmem_get_timer_name( agent *my_agent, const char *name )
{
	const long timer = epmem_convert_timer( my_agent, name );
	if ( timer == EPMEM_TIMERS )
		return 0;

	return my_agent->epmem_timers[ timer ]->name;
}

const char *epmem_get_timer_name( agent *my_agent, const long timer )
{
	if ( !epmem_valid_timer( my_agent, timer ) )
		return 0;

	return my_agent->epmem_timers[ timer ]->name;
}

/***************************************************************************
 * Function     : epmem_start_timer
 * Author		: Nate Derbinsky
 * Notes		: Starts a timer
 **************************************************************************/
void epmem_start_timer( agent *my_agent, const long timer )
{
	if ( epmem_valid_timer( my_agent, timer ) && ( epmem_get_parameter( my_agent, EPMEM_PARAM_TIMERS, EPMEM_RETURN_LONG ) >= my_agent->epmem_timers[ timer ]->level ) )
	{
		start_timer( my_agent, &my_agent->epmem_timers[ timer ]->start_timer );
	}
}

/***************************************************************************
 * Function     : epmem_stop_timer
 * Author		: Nate Derbinsky
 * Notes		: Stops a timer
 **************************************************************************/
void epmem_stop_timer( agent *my_agent, const long timer )
{
	if ( epmem_valid_timer( my_agent, timer ) && ( epmem_get_parameter( my_agent, EPMEM_PARAM_TIMERS, EPMEM_RETURN_LONG ) >= my_agent->epmem_timers[ timer ]->level ) )
	{
		stop_timer( my_agent, &my_agent->epmem_timers[ timer ]->start_timer, &my_agent->epmem_timers[ timer ]->total_timer );
	}
}


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
wme **epmem_get_augs_of_id( agent* my_agent, Symbol * id, tc_number tc, unsigned EPMEM_TYPE_INT *num_attr )
{
	slot *s;
	wme *w;
	wme **list;
	unsigned EPMEM_TYPE_INT list_position;
	unsigned EPMEM_TYPE_INT n = 0;

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

	unsigned EPMEM_TYPE_INT len = 0;
	unsigned EPMEM_TYPE_INT i;

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

/***************************************************************************
 * Function     : epmem_hash_wme
 * Author		: Nate Derbinsky
 * Notes		: Calculates the hash of a wme as the string hash of
 * 				  the attribute added to the string hash of the value
 **************************************************************************/
unsigned long epmem_hash_wme( agent *my_agent, wme *w )
{
	return ( hash_string( epmem_symbol_to_string( my_agent, w->attr ) ) + hash_string( epmem_symbol_to_string( my_agent, w->value ) ) );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Preference Functions (epmem::prefs)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_make_fake_preference
 * Author		: Andy Nuxoll/Nate Derbinsky
 * Notes		: This function adds a fake preference to a WME so that
 *                it will not be added to the goal dependency set of the
 *                state it is attached to.  This is used to prevents the
 *                GDS from removing a state whenever a epmem is retrieved
 *                that is attached to it.
 *
 *                The conditions of the preference refer to the WMEs
 *                in the current cue.
 *
 *                (The bulk of the content of this function is taken from
 *                 make_fake_preference_for_goal_item() in decide.c)
 **************************************************************************/
preference *epmem_make_fake_preference( agent *my_agent, Symbol *state, wme *w )
{
	// if we are on the top state, don't make the preference
	if ( state->id.epmem_info->ss_wme == NULL )
		return NIL;

	// make fake preference
	preference *pref = make_preference( my_agent, ACCEPTABLE_PREFERENCE_TYPE, w->id, w->attr, w->value, NIL );
	pref->o_supported = TRUE;
	symbol_add_ref( pref->id );
	symbol_add_ref( pref->attr );
	symbol_add_ref( pref->value );

	// add preference to goal list
	insert_at_head_of_dll( state->id.preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev );
	pref->on_goal_list = TRUE;

	// add reference
	preference_add_ref( pref );

	// make fake instantiation
	instantiation *inst;
	allocate_with_pool( my_agent, &( my_agent->instantiation_pool ), &inst );
	pref->inst = inst;
	pref->inst_next = pref->inst_prev = NULL;
	inst->preferences_generated = pref;
	inst->prod = NULL;
	inst->next = inst->prev = NULL;
	inst->rete_token = NULL;
	inst->rete_wme = NULL;
	inst->match_goal = state;
	inst->match_goal_level = state->id.level;
	inst->okay_to_variablize = TRUE;
	inst->backtrace_number = 0;
	inst->in_ms = FALSE;

	// create a condition for each cue WME (superstate if no cue)
	bool no_cue = state->id.epmem_info->cue_wmes->empty();
	condition *cond = NULL;
	condition *prev_cond = NULL;
	if ( no_cue )
		state->id.epmem_info->cue_wmes->insert( state->id.epmem_info->ss_wme );
	{
		std::set<wme *>::iterator p = state->id.epmem_info->cue_wmes->begin();

		while ( p != state->id.epmem_info->cue_wmes->end() )
		{
			// construct the condition
			allocate_with_pool( my_agent, &( my_agent->condition_pool ), &cond );
			cond->type = POSITIVE_CONDITION;
			cond->prev = prev_cond;
			cond->next = NULL;
			if ( prev_cond != NULL )
			{
				prev_cond->next = cond;
			}
			else
			{
				inst->top_of_instantiated_conditions = cond;
				inst->bottom_of_instantiated_conditions = cond;
				inst->nots = NULL;
			}
			cond->data.tests.id_test = make_equality_test( (*p)->id );
			cond->data.tests.attr_test = make_equality_test( (*p)->attr );
			cond->data.tests.value_test = make_equality_test( (*p)->value );
			cond->test_for_acceptable_preference = TRUE;
			cond->bt.wme_ = (*p);
			wme_add_ref( (*p) );
			cond->bt.level = (*p)->id->id.level;
			cond->bt.trace = (*p)->preference;
			if ( cond->bt.trace )
				preference_add_ref( cond->bt.trace );
			cond->bt.prohibits = NULL;

			prev_cond = cond;

			p++;
		}
	}
	if ( no_cue )
		state->id.epmem_info->cue_wmes->clear();

    return pref;
}

/***************************************************************************
 * Function     : epmem_remove_fake_preference
 * Author		: Andy Nuxoll
 * Notes		: This function removes a fake preference on a WME
 *                created by epmem_make_fake_preference().  While it's
 *                a one-line function I thought it was important to
 *                create so it would be clear what's going on in this
 *                case.
 **************************************************************************/
void epmem_remove_fake_preference( agent *my_agent, wme *w )
{
	if ( w->preference )
		preference_remove_ref( my_agent, w->preference );
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
int epmem_exec_query( agent *my_agent, sqlite3_stmt *stmt, const long timer )
{
	int return_val;

	epmem_start_timer( my_agent, timer );
	return_val = sqlite3_step( stmt );
	epmem_stop_timer( my_agent, timer );

	return return_val;
}

/***************************************************************************
 * Function     : epmem_exec_range_query
 * Author		: Nate Derbinsky
 * Notes		: Easy timer function for range query structs
 **************************************************************************/
int epmem_exec_range_query( agent *my_agent, epmem_range_query *stmt )
{
	return epmem_exec_query( my_agent, stmt->stmt, stmt->timer );
}

/***************************************************************************
 * Function     : epmem_exec_shared_query
 * Author		: Nate Derbinsky
 * Notes		: Easy timer function for shared query structs
 **************************************************************************/
int epmem_exec_shared_query( agent *my_agent, epmem_shared_query *stmt )
{
	return epmem_exec_query( my_agent, stmt->stmt, stmt->timer );
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

	return ( (EPMEM_TYPE_INT) epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) % (EPMEM_TYPE_INT) epmem_get_parameter( my_agent, EPMEM_PARAM_COMMIT ) );
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
bool epmem_get_variable( agent *my_agent, EPMEM_TYPE_INT variable_id, EPMEM_TYPE_INT *variable_value )
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
void epmem_set_variable( agent *my_agent, EPMEM_TYPE_INT variable_id, EPMEM_TYPE_INT variable_value )
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
EPMEM_TYPE_INT epmem_rit_fork_node( agent *my_agent, epmem_time_id lower, epmem_time_id upper, bool bounds_offset, EPMEM_TYPE_INT *step_return, const long *rit_state )
{
	if ( !bounds_offset )
	{
		EPMEM_TYPE_INT offset = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_OFFSET ] );

		lower = ( lower - offset );
		upper = ( upper - offset );
	}

	// descend the tree down to the fork node
	EPMEM_TYPE_INT node = EPMEM_RIT_ROOT;
	if ( upper < EPMEM_RIT_ROOT )
		node = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_LEFTROOT ] );
	else if ( lower > EPMEM_RIT_ROOT )
		node = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_RIGHTROOT ] );

	EPMEM_TYPE_INT step;
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
void epmem_rit_prep_left_right( agent *my_agent, epmem_time_id lower, epmem_time_id upper, const long *rit_state )
{
	////////////////////////////////////////////////////////////////////////////
	epmem_start_timer( my_agent, rit_state[ EPMEM_RIT_STATE_TIMER ] );
	////////////////////////////////////////////////////////////////////////////

	EPMEM_TYPE_INT offset = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_OFFSET ] );
	EPMEM_TYPE_INT node, step;
	EPMEM_TYPE_INT left_node, left_step;
	EPMEM_TYPE_INT right_node, right_step;

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
			node = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_RIGHTROOT ] );
			epmem_rit_add_left( my_agent, EPMEM_RIT_ROOT, EPMEM_RIT_ROOT );
		}
		else
		{
			node = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_LEFTROOT ] );
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
	epmem_stop_timer( my_agent, rit_state[ EPMEM_RIT_STATE_TIMER ] );
	////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : epmem_rit_prep_left_right
 * Author		: Nate Derbinsky
 * Notes		: Inserts an interval in the RIT
 **************************************************************************/
void epmem_rit_insert_interval( agent *my_agent, epmem_time_id lower, epmem_time_id upper, epmem_node_id id, const long *rit_state )
{
	// initialize offset
	EPMEM_TYPE_INT offset = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_OFFSET ] );
	if ( offset == EPMEM_RIT_OFFSET_INIT )
	{
		offset = lower;

		// update database
		epmem_set_variable( my_agent, rit_state[ EPMEM_RIT_STATE_OFFSET ], offset );

		// update stat
		epmem_set_stat( my_agent, rit_state[ EPMEM_RIT_STATE_OFFSET ], offset );
	}

	// get node
	EPMEM_TYPE_INT node;
	{
		EPMEM_TYPE_INT left_root = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_LEFTROOT ] );
		EPMEM_TYPE_INT right_root = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_RIGHTROOT ] );
		EPMEM_TYPE_INT min_step = epmem_get_stat( my_agent, rit_state[ EPMEM_RIT_STATE_MINSTEP ] );

		// shift interval
		epmem_time_id l = ( lower - offset );
		epmem_time_id u = ( upper - offset );

		// update left_root
		if ( ( u < EPMEM_RIT_ROOT ) && ( l <= ( 2 * left_root ) ) )
		{
			left_root = (EPMEM_TYPE_INT) pow( -2, floor( log( (double) -l ) / EPMEM_LN_2 ) );

			// update database
			epmem_set_variable( my_agent, rit_state[ EPMEM_RIT_STATE_LEFTROOT ], left_root );

			// update stat
			epmem_set_stat( my_agent, rit_state[ EPMEM_RIT_STATE_LEFTROOT ], left_root );
		}

		// update right_root
		if ( ( l > EPMEM_RIT_ROOT ) && ( u >= ( 2 * right_root ) ) )
		{
			right_root = (EPMEM_TYPE_INT) pow( 2, floor( log( (double) u ) / EPMEM_LN_2 ) );

			// update database
			epmem_set_variable( my_agent, rit_state[ EPMEM_RIT_STATE_RIGHTROOT ], right_root );

			// update stat
			epmem_set_stat( my_agent, rit_state[ EPMEM_RIT_STATE_RIGHTROOT ], right_root );
		}

		// update min_step
		EPMEM_TYPE_INT step;
		node = epmem_rit_fork_node( my_agent, l, u, true, &step, rit_state );

		if ( ( node != EPMEM_RIT_ROOT ) && ( step < min_step ) )
		{
			min_step = step;

			// update database
			epmem_set_variable( my_agent, rit_state[ EPMEM_RIT_STATE_MINSTEP ], min_step );

			// update stat
			epmem_set_stat( my_agent, rit_state[ EPMEM_RIT_STATE_MINSTEP ], min_step );
		}
	}

	// perform insert
	// ( node, start, end, id )
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ], 1, node );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ], 2, lower );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ], 3, upper );
	EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ], 4, id );
	sqlite3_step( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ] );
	sqlite3_reset( my_agent->epmem_statements[ rit_state[ EPMEM_RIT_STATE_ADD ] ] );
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
		// note: not being used, so commented out to avoid compiler warning
		/*
		const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );
		{
		}
		*/

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
	while ( !state->id.epmem_info->epmem_wmes->empty() )
	{
		epmem_remove_fake_preference( my_agent, state->id.epmem_info->epmem_wmes->top() );
		remove_input_wme( my_agent, state->id.epmem_info->epmem_wmes->top() );
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

		// remove fake preferences
		epmem_remove_fake_preference( my_agent, state->id.epmem_wme );
		epmem_remove_fake_preference( my_agent, state->id.epmem_cmd_wme );
		epmem_remove_fake_preference( my_agent, state->id.epmem_result_wme );

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
 **************************************************************************/
void epmem_init_db( agent *my_agent )
{
	if ( my_agent->epmem_db_status != EPMEM_DB_CLOSED )
		return;

	////////////////////////////////////////////////////////////////////////////
	epmem_start_timer( my_agent, EPMEM_TIMER_INIT );
	////////////////////////////////////////////////////////////////////////////

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

		// mode - read if existing
		long mode;
		{
			EPMEM_TYPE_INT stored_mode = NULL;
			if ( epmem_get_variable( my_agent, EPMEM_VAR_MODE, &stored_mode ) )
			{
				epmem_set_parameter( my_agent, (const long) EPMEM_PARAM_MODE, (const long) stored_mode );
				mode = (long) stored_mode;
			}
			else
			{
				mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );
				epmem_set_variable( my_agent, EPMEM_VAR_MODE, (EPMEM_TYPE_INT) mode );
			}
		}

		// at this point initialize the database for receipt of episodes
		epmem_transaction_begin( my_agent );

		if ( mode == EPMEM_MODE_ONE )
		{
			// variable initialization
			epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, 1 );
			epmem_set_stat( my_agent, (const long) epmem_rit_state_one[ EPMEM_RIT_STATE_OFFSET ], EPMEM_RIT_OFFSET_INIT );
			epmem_set_stat( my_agent, (const long) epmem_rit_state_one[ EPMEM_RIT_STATE_LEFTROOT ], 0 );
			epmem_set_stat( my_agent, (const long) epmem_rit_state_one[ EPMEM_RIT_STATE_RIGHTROOT ], 0 );
			epmem_set_stat( my_agent, (const long) epmem_rit_state_one[ EPMEM_RIT_STATE_MINSTEP ], LONG_MAX );
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
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,name NONE,value NONE,hash INTEGER,attr_type INTEGER,value_type INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// hash index for searching
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_unique_hash_parent ON node_unique (hash,parent_id)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for inserting ids
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_unique (parent_id,name,value,hash,attr_type,value_type) VALUES (?,?,?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding non-identifier id's
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE hash=? AND parent_id=? AND name=? AND value=? AND attr_type=? AND value_type=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for finding identifier id's
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE hash=? AND parent_id=? AND name=? AND value IS NULL AND attr_type=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] ), &tail ) == SQLITE_OK );
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
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT i.child_id, i.parent_id, i.name, i.value, i.attr_type, i.value_type FROM node_unique i WHERE i.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY i.child_id ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// get/set RIT variables
			{
				EPMEM_TYPE_INT var_val;

				for ( int i=epmem_rit_state_one[ EPMEM_RIT_STATE_OFFSET ]; i<=epmem_rit_state_one[ EPMEM_RIT_STATE_MINSTEP ]; i++ )
				{
					if ( epmem_get_variable( my_agent, i, &var_val ) )
						epmem_set_stat( my_agent, i, var_val );
					else
						epmem_set_variable( my_agent, i, epmem_get_stat( my_agent, i ) );
				}
			}

			// get max time
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(id) FROM times", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			if ( sqlite3_step( create ) == SQLITE_ROW )
				epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, ( EPMEM_SQLITE_COLUMN_INT( create, 0 ) + 1 ) );
			sqlite3_finalize( create );
			time_max = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME );

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
					epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), epmem_rit_state_one );
			}
			sqlite3_finalize( create );

			// remove all NOW intervals
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "DELETE FROM node_now", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// get max id + max list
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(child_id) FROM node_unique", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			if ( sqlite3_column_type( create, 0 ) != SQLITE_NULL )
			{
				unsigned EPMEM_TYPE_INT num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

				my_agent->epmem_node_maxes->resize( num_ids, EPMEM_MEMID_NONE );
				my_agent->epmem_node_mins->resize( num_ids, time_max );
			}
			sqlite3_finalize( create );
		}
		else if ( mode == EPMEM_MODE_THREE )
		{
			// initialize range tracking
			my_agent->epmem_node_mins->clear();
			my_agent->epmem_node_maxes->clear();
			my_agent->epmem_node_removals->clear();

			my_agent->epmem_edge_mins->clear();
			my_agent->epmem_edge_maxes->clear();
			my_agent->epmem_edge_removals->clear();

			// initialize time
			epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, 1 );

			// initialize next_id
			epmem_set_stat( my_agent, (const long) EPMEM_STAT_NEXT_ID, 1 );
			{
				EPMEM_TYPE_INT stored_id = NULL;
				if ( epmem_get_variable( my_agent, EPMEM_VAR_NEXT_ID, &stored_id ) )
					epmem_set_stat( my_agent, (const long) EPMEM_STAT_NEXT_ID, stored_id );
				else
					epmem_set_variable( my_agent, EPMEM_VAR_NEXT_ID, epmem_get_stat( my_agent, EPMEM_STAT_NEXT_ID ) );
			}

			// initialize rit state
			for ( int i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
			{
				epmem_set_stat( my_agent, (const long) epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_OFFSET ], EPMEM_RIT_OFFSET_INIT );
				epmem_set_stat( my_agent, (const long) epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_LEFTROOT ], 0 );
				epmem_set_stat( my_agent, (const long) epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_RIGHTROOT ], 0 );
				epmem_set_stat( my_agent, (const long) epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_MINSTEP ], LONG_MAX );
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
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS node_unique (child_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_id INTEGER,name NONE,value NONE,hash INTEGER,attr_type INTEGER,value_type INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// hash index for identification
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS node_unique_parent_hash ON node_unique (parent_id,hash)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for finding
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT child_id FROM node_unique WHERE parent_id=? AND hash=? AND name=? AND value=? AND attr_type=? AND value_type=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO node_unique (parent_id,name,value,hash,attr_type,value_type) VALUES (?,?,?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// edge_unique table
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE TABLE IF NOT EXISTS edge_unique (parent_id INTEGER PRIMARY KEY AUTOINCREMENT,q0 INTEGER,w NONE,q1 INTEGER,w_type INTEGER)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// index for identification
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "CREATE INDEX IF NOT EXISTS edge_unique_q0_w_q1 ON edge_unique (q0,w,q1)", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			sqlite3_step( create );
			sqlite3_finalize( create );

			// custom statement for finding
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT parent_id, q1 FROM edge_unique WHERE q0=? AND w=? AND w_type=?", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// custom statement for inserting
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "INSERT INTO edge_unique (q0,w,q1,w_type) VALUES (?,?,?,?)", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] ), &tail ) == SQLITE_OK );
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
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT f.child_id, f.parent_id, f.name, f.value, f.attr_type, f.value_type FROM node_unique f WHERE f.child_id IN (SELECT n.id FROM node_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM node_point p WHERE p.start=? UNION ALL SELECT e1.id FROM node_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM node_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY f.child_id ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			// range intersection query: edge
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT f.q0, f.w, f.q1, f.w_type FROM edge_unique f WHERE f.parent_id IN (SELECT n.id FROM edge_now n WHERE n.start<= ? UNION ALL SELECT p.id FROM edge_point p WHERE p.start=? UNION ALL SELECT e1.id FROM edge_range e1, rit_left_nodes lt WHERE e1.rit_node=lt.min AND e1.end >= ? UNION ALL SELECT e2.id FROM edge_range e2, rit_right_nodes rt WHERE e2.rit_node = rt.node AND e2.start <= ?) ORDER BY f.q0 ASC, f.q1 ASC", EPMEM_DB_PREP_STR_MAX, &( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ] ), &tail ) == SQLITE_OK );
			assert( my_assert );

			////

			// get/set RIT variables
			{
				EPMEM_TYPE_INT var_val;
				int i, j;

				for ( i=EPMEM_RIT_STATE_NODE; i<=EPMEM_RIT_STATE_EDGE; i++ )
				{
					for ( j=epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_OFFSET ]; j<=epmem_rit_state_three[ i ][ EPMEM_RIT_STATE_MINSTEP ]; j++ )
					{
						if ( epmem_get_variable( my_agent, j, &var_val ) )
							epmem_set_stat( my_agent, j, var_val );
						else
							epmem_set_variable( my_agent, j, epmem_get_stat( my_agent, j ) );
					}
				}
			}

			////

			// get max time
			my_assert = ( sqlite3_prepare_v2( my_agent->epmem_db, "SELECT MAX(id) FROM times", EPMEM_DB_PREP_STR_MAX, &create, &tail ) == SQLITE_OK );
			assert( my_assert );
			if ( sqlite3_step( create ) == SQLITE_ROW )
				epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, ( EPMEM_SQLITE_COLUMN_INT( create, 0 ) + 1 ) );
			sqlite3_finalize( create );
			time_max = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME );

			// insert non-NOW intervals for all current NOW's
			// remove NOW's
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
							epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), epmem_rit_state_three[ EPMEM_RIT_STATE_NODE ] );
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
							epmem_rit_insert_interval( my_agent, range_start, time_last, EPMEM_SQLITE_COLUMN_INT( create, 0 ), epmem_rit_state_three[ EPMEM_RIT_STATE_EDGE ] );
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
						unsigned EPMEM_TYPE_INT num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

						my_agent->epmem_node_maxes->resize( num_ids, EPMEM_MEMID_NONE );
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
						unsigned EPMEM_TYPE_INT num_ids = EPMEM_SQLITE_COLUMN_INT( create, 0 );

						my_agent->epmem_edge_maxes->resize( num_ids, EPMEM_MEMID_NONE );
						my_agent->epmem_edge_mins->resize( num_ids, time_max );
					}
					sqlite3_finalize( create );
				}
			}
		}

		epmem_transaction_end( my_agent, true );
		epmem_transaction_begin( my_agent );
	}

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_INIT );
	////////////////////////////////////////////////////////////////////////////
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Storage Functions (epmem::storage)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// adds a new episode to the store
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
	epmem_start_timer( my_agent, EPMEM_TIMER_STORAGE );
	////////////////////////////////////////////////////////////////////////////

	epmem_time_id time_counter = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME );

	// provide trace output
	if ( my_agent->sysparams[ TRACE_EPMEM_SYSPARAM ] )
	{
		char buf[256];
		SNPRINTF( buf, 254, "NEW EPISODE: (%c%d, %d)", my_agent->bottom_goal->id.name_letter, my_agent->bottom_goal->id.name_number, time_counter );

		print( my_agent, buf );

		xml_generate_warning( my_agent, buf );
	}

	const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );

	if ( mode == EPMEM_MODE_ONE )
	{
		// for now we are only recording episodes at the top state
		Symbol *parent_sym;

		// keeps children of the identifier of interest
		wme **wmes = NULL;
		unsigned EPMEM_TYPE_INT len = 0;
		unsigned EPMEM_TYPE_INT i;

		// future states of breadth-first search
		std::queue<Symbol *> syms;
		std::queue<epmem_node_id> ids;

		// current state
		epmem_node_id parent_id;

		// nodes to be recorded (implements tree flattening)
		std::map<epmem_node_id, bool> epmem;

		// wme hashing improves search speed
		unsigned long my_hash;

		// prevents infinite loops
		int tc = get_new_tc_number( my_agent );

		// used to implement exclusions
		const char *attr_name;

		// prevent recording exclusions
		std::list<const char *>::iterator exclusion;
		bool should_exclude;

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
					// prevent exclusions from being recorded
					should_exclude = false;
					attr_name = epmem_symbol_to_string( my_agent, wmes[i]->attr );
					for ( exclusion=my_agent->epmem_exclusions->begin();
						  ( ( !should_exclude ) && ( exclusion!=my_agent->epmem_exclusions->end() ) );
						  exclusion++ )
						if ( strcmp( attr_name, (*exclusion) ) == 0 )
							should_exclude = true;
					if ( should_exclude )
						continue;

					// if we haven't seen this WME before
					// or we haven't seen it within this database...
					// look it up in the database
					if ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) )
					{
						wmes[i]->epmem_id = NULL;
						wmes[i]->epmem_valid = my_agent->epmem_validation;

						my_hash = epmem_hash_wme( my_agent, wmes[i] );
						if ( wmes[i]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
						{
							// hash=? AND parent_id=? AND name=? AND value=? AND attr_type=? AND value_type=?
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 1, my_hash );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 2, parent_id );

							switch( wmes[i]->attr->common.symbol_type )
							{
								case SYM_CONSTANT_SYMBOL_TYPE:
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
									break;

								case INT_CONSTANT_SYMBOL_TYPE:
		        					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, wmes[i]->attr->ic.value );
									break;

								case FLOAT_CONSTANT_SYMBOL_TYPE:
		        					sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, wmes[i]->attr->fc.value );
									break;
							}

							switch( wmes[i]->value->common.symbol_type )
							{
								case SYM_CONSTANT_SYMBOL_TYPE:
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, (const char *) wmes[i]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
									break;

								case INT_CONSTANT_SYMBOL_TYPE:
		        					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, wmes[i]->value->ic.value );
									break;

								case FLOAT_CONSTANT_SYMBOL_TYPE:
		        					sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, wmes[i]->value->fc.value );
									break;
							}
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 5, wmes[i]->attr->common.symbol_type );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 6, wmes[i]->value->common.symbol_type );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 0 );

							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] );
						}
						else
						{
							// hash=? AND parent_id=? AND name=? AND value IS NULL AND attr_type=?
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 1, my_hash );
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 2, parent_id );

							switch( wmes[i]->attr->common.symbol_type )
							{
								case SYM_CONSTANT_SYMBOL_TYPE:
									sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
									break;

								case INT_CONSTANT_SYMBOL_TYPE:
		        					EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, wmes[i]->attr->ic.value );
									break;

								case FLOAT_CONSTANT_SYMBOL_TYPE:
		        					sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, wmes[i]->attr->fc.value );
									break;
							}
							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 4, wmes[i]->attr->common.symbol_type );

							if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] ) == SQLITE_ROW )
								wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 0 );

							sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ] );
						}
					}

					// insert on no id
					if ( wmes[i]->epmem_id == NULL )
					{
						EPMEM_TYPE_INT attr_type = wmes[i]->attr->common.symbol_type;
						EPMEM_TYPE_INT value_type = wmes[i]->value->common.symbol_type;

						// insert (parent_id,name,value,hash,attr_type,value_type)
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 1, parent_id );

						switch ( attr_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 2, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
								break;

							case INT_CONSTANT_SYMBOL_TYPE:
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 2, wmes[i]->attr->ic.value );
								break;

							case FLOAT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 2, wmes[i]->attr->fc.value );
								break;
						}

						switch ( value_type )
						{
							case SYM_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3, (const char *) wmes[i]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
								break;

							case INT_CONSTANT_SYMBOL_TYPE:
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3, wmes[i]->value->ic.value );
								break;

							case FLOAT_CONSTANT_SYMBOL_TYPE:
								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3, wmes[i]->value->fc.value );
								break;

							case IDENTIFIER_SYMBOL_TYPE:
								sqlite3_bind_null( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 3 );
								break;
						}
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 4, my_hash );
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 5, attr_type );
						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ], 6, value_type );
						sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] );
						sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_NODE_UNIQUE ] );

						wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );

						// new nodes definitely start
						epmem[ wmes[i]->epmem_id ] = true;
						my_agent->epmem_node_mins->push_back( time_counter );
						my_agent->epmem_node_maxes->push_back( time_counter );
					}
					else
					{
						// definitely don't update/delete
						(*my_agent->epmem_node_removals)[ wmes[i]->epmem_id ] = false;

						// we insert if current time is > 1+ max
						if ( (*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] < ( time_counter - 1 ) )
							epmem[ wmes[i]->epmem_id ] = true;

						// update max irrespectively
						(*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] = time_counter;
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
					epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, epmem_rit_state_one );
			}

			r++;
		}
		my_agent->epmem_node_removals->clear();

		// add the time id to the times table
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ], 1, time_counter );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_ADD_TIME ] );

		epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, time_counter + 1 );
	}
	else if ( mode == EPMEM_MODE_THREE )
	{
		// prevents infinite loops
		int tc = get_new_tc_number( my_agent );
		std::map<epmem_node_id, bool> seen_ids;
		std::map<epmem_node_id, bool>::iterator seen_p;

		// depth first search state
		std::queue<Symbol *> parent_syms;
		Symbol *parent_sym;
		std::queue<epmem_node_id> parent_ids;
		epmem_node_id parent_id;

		// seen nodes (non-identifiers) and edges (identifiers)
		std::queue<epmem_node_id> epmem_node;
		std::queue<epmem_node_id> epmem_edge;

		// hashing improves search speed
		unsigned long my_hash;

		// used to implement attribute label exclusions
		const char *attr_name;

		// children of the current identifier
		wme **wmes = NULL;
		unsigned EPMEM_TYPE_INT len = 0;
		unsigned EPMEM_TYPE_INT i;

		// prevent recording exclusions
		std::list<const char *>::iterator exclusion;
		bool should_exclude;

		// initialize BFS
		parent_syms.push( my_agent->top_goal );
		parent_ids.push( EPMEM_NODEID_ROOT );

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
				for ( i=0; i<len; i++ )
				{
					// prevent exclusions from being recorded
					should_exclude = false;
					attr_name = epmem_symbol_to_string( my_agent, wmes[i]->attr );
					for ( exclusion=my_agent->epmem_exclusions->begin();
						  ( ( !should_exclude ) && ( exclusion!=my_agent->epmem_exclusions->end() ) );
						  exclusion++ )
						if ( strcmp( attr_name, (*exclusion) ) == 0 )
							should_exclude = true;
					if ( should_exclude )
						continue;

					if ( wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					{
						// have we seen this WME during this database?
						if ( ( wmes[i]->epmem_id == NULL ) || ( wmes[i]->epmem_valid != my_agent->epmem_validation ) )
						{
							wmes[i]->epmem_valid = my_agent->epmem_validation;

							// add path
							{
								if ( wmes[i]->value->id.epmem_id == NULL )
								{
									// update next id
									wmes[i]->value->id.epmem_id = epmem_get_stat( my_agent, EPMEM_STAT_NEXT_ID );
									epmem_set_stat( my_agent, EPMEM_STAT_NEXT_ID, wmes[i]->value->id.epmem_id + 1 );
									epmem_set_variable( my_agent, EPMEM_VAR_NEXT_ID, wmes[i]->value->id.epmem_id + 1);
								}

								// insert (q0,w,q1,w_type)
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 1, parent_id );

								switch( wmes[i]->attr->common.symbol_type )
								{
									case SYM_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 2, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
										break;

									case INT_CONSTANT_SYMBOL_TYPE:
		        						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 2, wmes[i]->attr->ic.value );
										break;

									case FLOAT_CONSTANT_SYMBOL_TYPE:
		        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 2, wmes[i]->attr->fc.value );
										break;
								}

								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 3, wmes[i]->value->id.epmem_id );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ], 4, wmes[i]->attr->common.symbol_type );
								sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] );
								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_EDGE_UNIQUE ] );

								wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );

								// new nodes definitely start
								epmem_edge.push( wmes[i]->epmem_id );
								my_agent->epmem_edge_mins->push_back( time_counter );
								my_agent->epmem_edge_maxes->push_back( time_counter );
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

							my_hash = epmem_hash_wme( my_agent, wmes[i] );

							// try to get node id
							{
								// parent_id=? AND hash=? AND name=? AND value=? AND attr_type=? AND value_type=?
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 1, parent_id );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 2, my_hash );

								switch( wmes[i]->attr->common.symbol_type )
								{
									case SYM_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
										break;

									case INT_CONSTANT_SYMBOL_TYPE:
		        						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, wmes[i]->attr->ic.value );
										break;

									case FLOAT_CONSTANT_SYMBOL_TYPE:
		        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, wmes[i]->attr->fc.value );
										break;
								}

								switch( wmes[i]->value->common.symbol_type )
								{
									case SYM_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, (const char *) wmes[i]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
										break;

									case INT_CONSTANT_SYMBOL_TYPE:
		        						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, wmes[i]->value->ic.value );
										break;

									case FLOAT_CONSTANT_SYMBOL_TYPE:
		        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, wmes[i]->value->fc.value );
										break;
								}
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 5, wmes[i]->attr->common.symbol_type );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 6, wmes[i]->value->common.symbol_type );

								if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
									wmes[i]->epmem_id = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 0 );

								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] );
							}

							// act depending on new/existing feature
							if ( wmes[i]->epmem_id == NULL )
							{
								EPMEM_TYPE_INT attr_type = wmes[i]->attr->common.symbol_type;
								EPMEM_TYPE_INT value_type = wmes[i]->value->common.symbol_type;

								// insert (parent_id,name,value,hash,attr_type,value_type)
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 1, parent_id );

								switch( attr_type )
								{
									case SYM_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 2, (const char *) wmes[i]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
										break;

									case INT_CONSTANT_SYMBOL_TYPE:
		        						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 2, wmes[i]->attr->ic.value );
										break;

									case FLOAT_CONSTANT_SYMBOL_TYPE:
		        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 2, wmes[i]->attr->fc.value );
										break;
								}

								switch ( value_type )
								{
									case SYM_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 3, (const char *) wmes[i]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
										break;

									case INT_CONSTANT_SYMBOL_TYPE:
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 3, wmes[i]->value->ic.value );
										break;

									case FLOAT_CONSTANT_SYMBOL_TYPE:
										sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 3, wmes[i]->value->fc.value );
										break;
								}
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 4, my_hash );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 5, attr_type );
								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ], 6, value_type );
								sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] );
								sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_NODE_UNIQUE ] );

								wmes[i]->epmem_id = (epmem_node_id) sqlite3_last_insert_rowid( my_agent->epmem_db );

								// new nodes definitely start
								epmem_node.push( wmes[i]->epmem_id );
								my_agent->epmem_node_mins->push_back( time_counter );
								my_agent->epmem_node_maxes->push_back( time_counter );
							}
							else
							{
								// definitely don't update/delete
								(*my_agent->epmem_node_removals)[ wmes[i]->epmem_id ] = false;

								// we insert if current time is > 1+ max
								if ( (*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] < ( time_counter - 1 ) )
									epmem_node.push( wmes[i]->epmem_id );

								// update max irrespectively
								(*my_agent->epmem_node_maxes)[ wmes[i]->epmem_id - 1 ] = time_counter;
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
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, epmem_rit_state_three[ EPMEM_RIT_STATE_NODE ] );
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
						epmem_rit_insert_interval( my_agent, range_start, range_end, r->first, epmem_rit_state_three[ EPMEM_RIT_STATE_EDGE ] );
				}

				r++;
			}
			my_agent->epmem_edge_removals->clear();
		}

		// add the time id to the times table
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ], 1, time_counter );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ] );
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_ADD_TIME ] );

		epmem_set_stat( my_agent, (const long) EPMEM_STAT_TIME, time_counter + 1 );
	}

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_STORAGE );
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
	const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );
	bool return_val = false;

	if ( mode == EPMEM_MODE_ONE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ] );
		return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_VALID_EPISODE ] );
	}
	else if ( mode == EPMEM_MODE_THREE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ], 1, memory_id );
		sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ] );
		return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ], 0 ) > 0 );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_VALID_EPISODE ] );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_valid_episode
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
	wme *new_wme;

	////////////////////////////////////////////////////////////////////////////
	epmem_start_timer( my_agent, EPMEM_TIMER_NCB_RETRIEVAL );
	////////////////////////////////////////////////////////////////////////////

	// get the ^result header for this state
	Symbol *result_header = state->id.epmem_result_header;

	// initialize stat
	EPMEM_TYPE_INT num_wmes = 0;
	epmem_set_stat( my_agent, EPMEM_STAT_NCB_WMES, num_wmes );

	// if no memory, say so
	if ( ( memory_id == EPMEM_MEMID_NONE ) ||
		 !epmem_valid_episode( my_agent, memory_id ) )
	{
		new_wme = add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, my_agent->epmem_no_memory_symbol );
		new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
		state->id.epmem_info->epmem_wmes->push( new_wme );

		state->id.epmem_info->last_memory = EPMEM_MEMID_NONE;

		epmem_stop_timer( my_agent, EPMEM_TIMER_NCB_RETRIEVAL );
		return;
	}

	// remember this as the last memory installed
	state->id.epmem_info->last_memory = memory_id;

	// create a new ^retrieved header for this result
	Symbol *retrieved_header = make_new_identifier( my_agent, 'R', result_header->id.level );
	new_wme = add_input_wme( my_agent, result_header, my_agent->epmem_retrieved_symbol, retrieved_header );
	new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
	state->id.epmem_info->epmem_wmes->push( new_wme );
	symbol_remove_ref( my_agent, retrieved_header );

	// add *-id wme's
	{
		Symbol *my_meta;

		my_meta = make_int_constant( my_agent, memory_id );
		new_wme = add_input_wme( my_agent, result_header, my_agent->epmem_memory_id_symbol, my_meta );
		new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
		state->id.epmem_info->epmem_wmes->push( new_wme );
		symbol_remove_ref( my_agent, my_meta );

		my_meta = make_int_constant( my_agent, epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) );
		new_wme = add_input_wme( my_agent, result_header, my_agent->epmem_present_id_symbol, my_meta );
		new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
		state->id.epmem_info->epmem_wmes->push( new_wme );
		symbol_remove_ref( my_agent, my_meta );
	}

	const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );

	if ( mode == EPMEM_MODE_ONE )
	{
		epmem_id_mapping ids;
		epmem_node_id child_id;
		epmem_node_id parent_id;
		EPMEM_TYPE_INT attr_type;
		EPMEM_TYPE_INT value_type;
		Symbol *attr = NULL;
		Symbol *value = NULL;
		Symbol *parent = NULL;

		ids[ 0 ] = retrieved_header;

		epmem_rit_prep_left_right( my_agent, memory_id, memory_id, epmem_rit_state_one );

		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 1, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 2, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 3, memory_id );
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], 4, memory_id );
		while ( epmem_exec_query( my_agent, my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ], EPMEM_TIMER_NCB_NODE ) == SQLITE_ROW )
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

				new_wme = add_input_wme( my_agent, parent, attr, value );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );

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

				new_wme = add_input_wme( my_agent, parent, attr, value );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				num_wmes++;
			}

			symbol_remove_ref( my_agent, attr );
			symbol_remove_ref( my_agent, value );
		}
		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_GET_EPISODE ] );

		epmem_rit_clear_left_right( my_agent );
	}
	else if ( mode == EPMEM_MODE_THREE )
	{
		// Big picture: create identifier skeleton, then hang non-identifers
		//
		// Because of shared WMEs at different levels of the storage depth-first search,
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
			EPMEM_TYPE_INT w_type; // we support any constant attribute symbol

			// used to lookup shared identifiers
			epmem_id_mapping::iterator id_p;

			// orphaned children
			std::queue<epmem_edge *> orphans;
			epmem_edge *orphan;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, epmem_rit_state_three[ EPMEM_RIT_STATE_EDGE ] );

			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 1, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 2, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 3, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], 4, memory_id );
			while ( epmem_exec_query( my_agent, my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_EDGES ], EPMEM_TIMER_NCB_EDGE ) == SQLITE_ROW )
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

					new_wme = add_input_wme( my_agent, parent, attr, (*value) );
					new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
					state->id.epmem_info->epmem_wmes->push( new_wme );
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

					new_wme = add_input_wme( my_agent, parent, orphan->w, (*value) );
					new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
					state->id.epmem_info->epmem_wmes->push( new_wme );
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
			EPMEM_TYPE_INT attr_type;
			EPMEM_TYPE_INT value_type;

			Symbol *value = NULL;

			epmem_rit_prep_left_right( my_agent, memory_id, memory_id, epmem_rit_state_three[ EPMEM_RIT_STATE_NODE ] );

			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 1, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 2, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 3, memory_id );
			EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], 4, memory_id );
			while ( epmem_exec_query( my_agent, my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ], EPMEM_TIMER_NCB_NODE ) == SQLITE_ROW )
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

				new_wme = add_input_wme( my_agent, parent, attr, value );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				num_wmes++;

				symbol_remove_ref( my_agent, attr );
				symbol_remove_ref( my_agent, value );
			}

			sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_GET_NODES ] );
			epmem_rit_clear_left_right( my_agent );
		}
	}

	// adjust stat
	epmem_set_stat( my_agent, EPMEM_STAT_NCB_WMES, num_wmes );

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_NCB_RETRIEVAL );
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
	epmem_start_timer( my_agent, EPMEM_TIMER_NEXT );
	////////////////////////////////////////////////////////////////////////////

	const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );
	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( mode == EPMEM_MODE_ONE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_NEXT_EPISODE ] );
	}
	else if ( mode == EPMEM_MODE_THREE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_NEXT_EPISODE ] );
	}

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_NEXT );
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
	epmem_start_timer( my_agent, EPMEM_TIMER_PREV );
	////////////////////////////////////////////////////////////////////////////

	const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );
	epmem_time_id return_val = EPMEM_MEMID_NONE;

	if ( mode == EPMEM_MODE_ONE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_PREV_EPISODE ] );
	}
	else if ( mode == EPMEM_MODE_THREE )
	{
		EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ], 1, memory_id );
		if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ] ) == SQLITE_ROW )
			return_val = ( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ], 0 ) );

		sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_PREV_EPISODE ] );
	}

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_PREV );
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
// the Disjunctive Normal Form (DNF) tree.
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
void epmem_incremental_row( agent *my_agent, epmem_range_query_list *queries, epmem_time_id &id, EPMEM_TYPE_INT &ct, double &v, EPMEM_TYPE_INT &updown, const unsigned int list )
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
		more_data = ( epmem_exec_range_query( my_agent, temp_query ) == SQLITE_ROW );
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
void epmem_shared_flip( epmem_shared_literal *flip, const unsigned int list, EPMEM_TYPE_INT &ct, double &v, EPMEM_TYPE_INT &updown )
{
	if ( list == EPMEM_RANGE_START )
	{
		if ( ( --flip->ct ) == ( EPMEM_DNF - 1 ) )
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
		if ( ( ++flip->ct ) == EPMEM_DNF )
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
void epmem_shared_increment( agent *my_agent, epmem_shared_query_list *queries, epmem_time_id &id, EPMEM_TYPE_INT &ct, double &v, EPMEM_TYPE_INT &updown, const unsigned int list )
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
		more_data = ( epmem_exec_shared_query( my_agent, temp_query ) == SQLITE_ROW );
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
 * Function     : epmem_graph_match_paths
 * Author		: Nate Derbinsky
 * Notes		: Performs DFS to determine the maximum number of
 * 				  leaf WMEs that can be satisfied by holding constant
 * 				  the identity of a single literal per clause.
 *
 * 				  In other words, from the given list of literals,
 * 				  if we only choose one literal for each WME, how many
 * 				  satisfied matches can we reach?  Equivalent to a full
 * 				  graph-match over a tree cue.
 *
 * 				  Notes:
 * 				    - takes advantage of the ordering of literals (literals
 * 				      of the same WME are grouped together)
 * 					- this is not true graph-match because it does
 * 				      not respect constraints of shared identifiers.
 **************************************************************************/
unsigned EPMEM_TYPE_INT epmem_graph_match_paths( epmem_shared_literal_group *literals )
{
	// number of satisfied matches reached
	unsigned EPMEM_TYPE_INT return_val = 0;

	if ( !literals->literals->empty() )
	{
		// keeps track of the current literal
		epmem_shared_literal_list::size_type l_c = 0;
		epmem_shared_literal *l_p = literals->literals->front();

		// keeps track of the current WME
		epmem_shared_wme_list::iterator w_p = literals->wmes->begin();
		literals->c_wme = l_p->wme;

		bool good_literal = false;
		bool done = false;

		// proceed through all the wmes in the group
		do
		{
			// try the literal
			good_literal = false;
			if ( l_p->ct == EPMEM_DNF )
			{
				// if the associated wme has children
				if ( l_p->wme_kids )
				{
					// can we reach the appropriate number
					// of satisfied matches?
					if ( ( l_p->children ) && ( epmem_graph_match_paths( l_p->children ) == l_p->wme_kids ) )
					{
						good_literal = true;
					}
				}
				else
				{
					// is the associated match satisfied?
					if ( l_p->match->ct )
					{
						good_literal = true;
					}
				}
			}

			if ( good_literal )
			{
				// successful at this wme
				return_val++;

				// proceed to the next wme
				w_p++;
				if ( w_p != literals->wmes->end() )
				{
					l_c = (*w_p);
					l_p = (*literals->literals)[ l_c ];
					literals->c_wme = l_p->wme;
				}
				else
				{
					done = true;
				}
			}
			else
			{
				// try next literal
				l_c++;
				if ( l_c < literals->literals->size() )
				{
					l_p = (*literals->literals)[ l_c ];
					if ( literals->c_wme != l_p->wme )
					{
						done = true;
					}
				}
				else
				{
					done = true;
				}
			}
		} while ( !done );
	}

	return return_val;
}

/***************************************************************************
 * Function     : epmem_graph_match_wmes
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
unsigned EPMEM_TYPE_INT epmem_graph_match_wmes( epmem_shared_literal_group *literals, epmem_constraint_list *constraints )
{
	// number of satisfied leaf WMEs in this list
	unsigned EPMEM_TYPE_INT return_val = 0;

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
				if ( c_l->ct == EPMEM_DNF )
				{
					// cue identifier
					if ( c_l->wme_kids )
					{
						// check if unconstrained
						if ( c_id == EPMEM_NODEID_ROOT )
						{
							// copy constraints
							n_c = new epmem_constraint_list( *c_c );

							// try DFS
							if ( ( c_l->children ) && ( epmem_graph_match_wmes( c_l->children, n_c ) == c_l->wme_kids ) )
							{
								// on success, keep new constraints
								good_literal = true;
								(*c_c) = (*n_c);

								// update constraints with this literal
								(*c_c)[ c_l->wme->value ] = c_l->shared_id;
							}

							delete n_c;
						}
						else
						{
							// if shared identifier, we don't need to perform recursion
							// (we rely upon previous results)
							good_literal = ( c_id == c_l->shared_id );
						}
					}
					// leaf node
					else
					{
						good_literal = ( c_l->match->ct );
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
 **************************************************************************/
void epmem_process_query( agent *my_agent, Symbol *state, Symbol *query, Symbol *neg_query, epmem_time_list *prohibit, epmem_time_id before, epmem_time_id after )
{
	////////////////////////////////////////////////////////////////////////////
	epmem_start_timer( my_agent, EPMEM_TIMER_QUERY );
	////////////////////////////////////////////////////////////////////////////

	unsigned EPMEM_TYPE_INT len_query = 0, len_neg_query = 0;
	wme **wmes_query = NULL;
	if ( query != NULL )
		wmes_query = epmem_get_augs_of_id( my_agent, query, get_new_tc_number( my_agent ), &len_query );

	wme **wmes_neg_query = NULL;
	if ( neg_query != NULL )
		wmes_neg_query = epmem_get_augs_of_id( my_agent, neg_query, get_new_tc_number( my_agent ), &len_neg_query );

	// only perform a query if there potentially valid cue(s)
	if ( ( len_query != 0 ) || ( len_neg_query != 0 ) )
	{
		const long mode = epmem_get_parameter( my_agent, EPMEM_PARAM_MODE, EPMEM_RETURN_LONG );

		if ( !prohibit->empty() )
			std::sort( prohibit->begin(), prohibit->end() );

		if ( mode == EPMEM_MODE_ONE )
		{
			wme *new_wme;

			// BFS to get the leaf id's
			std::list<epmem_leaf_node *> leaf_ids[2];
			std::list<epmem_leaf_node *>::iterator leaf_p;
			epmem_time_list::iterator prohibit_p;
			{
				wme ***wmes;
				unsigned EPMEM_TYPE_INT len;

				std::queue<Symbol *> parent_syms;
				std::queue<epmem_node_id> parent_ids;
				std::queue<wme *> parent_wmes;
				int tc = get_new_tc_number( my_agent );

				Symbol *parent_sym;
				epmem_node_id parent_id;
				wme *parent_wme;
				unsigned long my_hash;

				int i;
				unsigned EPMEM_TYPE_INT j;
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
									my_hash = epmem_hash_wme( my_agent, (*wmes)[j] );
									if ( (*wmes)[j]->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
									{
										// hash=? AND parent_id=? AND name=? AND value=? AND attr_type=? AND value_type=?
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 1, my_hash );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 2, parent_id );

										switch( (*wmes)[j]->attr->common.symbol_type )
										{
											case SYM_CONSTANT_SYMBOL_TYPE:
												sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, (const char *) (*wmes)[j]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
												break;

											case INT_CONSTANT_SYMBOL_TYPE:
			        							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, (*wmes)[j]->attr->ic.value );
												break;

											case FLOAT_CONSTANT_SYMBOL_TYPE:
			        							sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 3, (*wmes)[j]->attr->fc.value );
												break;
										}

										switch( (*wmes)[j]->value->common.symbol_type )
										{
											case SYM_CONSTANT_SYMBOL_TYPE:
												sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, (const char *) (*wmes)[j]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
												break;

											case INT_CONSTANT_SYMBOL_TYPE:
			        							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, (*wmes)[j]->value->ic.value );
												break;

											case FLOAT_CONSTANT_SYMBOL_TYPE:
			        							sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 4, (*wmes)[j]->value->fc.value );
												break;
										}

										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 5, (*wmes)[j]->attr->common.symbol_type );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 6, (*wmes)[j]->value->common.symbol_type );

										if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
											leaf_ids[i].push_back( epmem_create_leaf_node( EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ], 0 ), wma_get_wme_activation( my_agent, (*wmes)[j] ) ) );

										sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_NODE_UNIQUE ] );
									}
									else
									{
										// hash=? AND parent_id=? AND name=? AND value IS NULL AND attr_type=?
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 1, my_hash );
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 2, parent_id );

										switch( (*wmes)[j]->attr->common.symbol_type )
										{
											case SYM_CONSTANT_SYMBOL_TYPE:
												sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, (const char *) (*wmes)[j]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
												break;

											case INT_CONSTANT_SYMBOL_TYPE:
			        							EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, (*wmes)[j]->attr->ic.value );
												break;

											case FLOAT_CONSTANT_SYMBOL_TYPE:
			        							sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 3, (*wmes)[j]->attr->fc.value );
												break;
										}
										EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_ONE_FIND_IDENTIFIER ], 4, (*wmes)[j]->attr->common.symbol_type );

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

			epmem_set_stat( my_agent, EPMEM_STAT_QRY_POS, leaf_ids[ EPMEM_NODE_POS ].size() );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_NEG, leaf_ids[ EPMEM_NODE_NEG ].size() );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_RET, 0 );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_CARD, 0 );

			// useful statistics
			int cue_sizes[2] = { leaf_ids[ EPMEM_NODE_POS ].size(), leaf_ids[ EPMEM_NODE_NEG ].size() };
			int cue_size = ( cue_sizes[ EPMEM_NODE_POS ] + cue_sizes[ EPMEM_NODE_NEG ] );
			unsigned EPMEM_TYPE_INT perfect_match = leaf_ids[ EPMEM_NODE_POS ].size();

			// only perform search if necessary
			if ( cue_size )
			{
				// perform incremental, integrated range search
				{
					// variables to populate
					epmem_time_id king_id = EPMEM_MEMID_NONE;
					double king_score = -1000;
					unsigned EPMEM_TYPE_INT king_cardinality = 0;

					// prepare queries
					epmem_range_query_list *queries = new epmem_range_query_list[2];
					int i, j, k;
					{
						const char *tail;
						epmem_time_id time_now = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) - 1;
						int position;

						epmem_range_query *new_query = NULL;
						sqlite3_stmt *new_stmt = NULL;
						long new_timer = NULL;

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
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_EP ):( EPMEM_TIMER_QUERY_POS_END_EP ) ):( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_EP ):( EPMEM_TIMER_QUERY_NEG_END_EP ) ) );
													break;

												case EPMEM_RANGE_NOW:
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_NOW ):( EPMEM_TIMER_QUERY_POS_END_NOW ) ):( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_NOW ):( EPMEM_TIMER_QUERY_NEG_END_NOW ) ) );
													break;

												case EPMEM_RANGE_POINT:
													new_timer = ( ( i == EPMEM_NODE_POS )?( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_POINT ):( EPMEM_TIMER_QUERY_POS_END_POINT ) ):( ( j == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_POINT ):( EPMEM_TIMER_QUERY_NEG_END_POINT ) ) );
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

											if ( epmem_exec_query( my_agent, new_stmt, new_timer ) == SQLITE_ROW )
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
						double balance = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_BALANCE );
						double balance_inv = 1 - balance;

						// dynamic programming stuff
						EPMEM_TYPE_INT sum_ct = 0;
						double sum_v = 0;
						EPMEM_TYPE_INT sum_updown = 0;

						// current pointer
						epmem_time_id current_id = EPMEM_MEMID_NONE;
						EPMEM_TYPE_INT current_ct = 0;
						double current_v = 0;
						EPMEM_TYPE_INT current_updown = 0;
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
						epmem_incremental_row( my_agent, queries, current_id, current_ct, current_v, current_updown, EPMEM_RANGE_END );
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

									// new king if no old king OR better score
									if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score > king_score ) )
									{
										king_id = current_valid_end;
										king_score = current_score;
										king_cardinality = sum_ct;

										if ( king_cardinality == perfect_match )
											done = true;
									}
								}

								if ( !done )
								{
									// based upon choice, update variables
									epmem_incremental_row( my_agent, queries, current_id, current_ct, current_v, current_updown, next_list );
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

						epmem_set_stat( my_agent, EPMEM_STAT_QRY_RET, king_id );
						epmem_set_stat( my_agent, EPMEM_STAT_QRY_CARD, king_cardinality );

						// status
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );

						// match score
						my_meta = make_float_constant( my_agent, king_score );
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, my_meta );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );
						symbol_remove_ref( my_agent, my_meta );

						// cue-size
						my_meta = make_int_constant( my_agent, cue_size );
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, my_meta );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );
						symbol_remove_ref( my_agent, my_meta );

						// normalized-match-score
						my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, my_meta );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );
						symbol_remove_ref( my_agent, my_meta );

						// match-cardinality
						my_meta = make_int_constant( my_agent, king_cardinality );
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, my_meta );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );
						symbol_remove_ref( my_agent, my_meta );

						////////////////////////////////////////////////////////////////////////////
						epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
						////////////////////////////////////////////////////////////////////////////

						// actual memory
						epmem_install_memory( my_agent, state, king_id );
					}
					else
					{
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );

						////////////////////////////////////////////////////////////////////////////
						epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
						////////////////////////////////////////////////////////////////////////////
					}
				}
			}
			else
			{
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );

				////////////////////////////////////////////////////////////////////////////
				epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
				////////////////////////////////////////////////////////////////////////////
			}
		}
		else if ( mode == EPMEM_MODE_THREE )
		{
			// queries
			epmem_shared_query_list *queries = new epmem_shared_query_list[2];
			std::list<epmem_shared_literal_list *> trigger_lists;

			// match counters
			std::list<epmem_shared_match *> matches;

			// literals
			std::list<epmem_shared_literal *> literals;

			// graph match
			const long graph_match = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_GRAPH_MATCH, EPMEM_RETURN_LONG );
			epmem_shared_literal_group *graph_match_roots;
			if ( graph_match != EPMEM_GRAPH_MATCH_OFF )
			{
				graph_match_roots = new epmem_shared_literal_group();

				graph_match_roots->literals = new epmem_shared_literal_list();
				graph_match_roots->wmes = new epmem_shared_wme_list();
				graph_match_roots->c_wme = NULL;
			}

			unsigned EPMEM_TYPE_INT leaf_ids[2] = { 0, 0 };

			wme *new_wme;
			epmem_time_id time_now = epmem_get_stat( my_agent, (const long) EPMEM_STAT_TIME ) - 1;

			// simultaneously process cue, construct DNF graph, and add queries to priority cue
			// (i.e. prep for range search query)
			{
				// wme cache
				int tc = get_new_tc_number( my_agent );
				std::map<Symbol *, epmem_wme_cache_element *> wme_cache;
				epmem_wme_cache_element *current_cache_element;
				epmem_wme_cache_element **cache_hit;

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
				unsigned EPMEM_TYPE_INT j;

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
				epmem_shared_wme_list *new_wme_list = NULL;
				epmem_shared_literal_group *new_literal_group = NULL;
				long new_timer = NULL;
				sqlite3_stmt *new_stmt = NULL;

				// identity (i.e. database id)
				epmem_node_id unique_identity;
				epmem_node_id shared_identity;

				// initialize wme cache
				{
					// query
					new_cache_element = new epmem_wme_cache_element;
					new_cache_element->len = len_query;
					new_cache_element->wmes = wmes_query;
					wme_cache[ query ] = new_cache_element;
					new_cache_element = NULL;

					// negative query
					new_cache_element = new epmem_wme_cache_element;
					new_cache_element->len = len_neg_query;
					new_cache_element->wmes = wmes_neg_query;
					wme_cache[ neg_query ] = new_cache_element;
					new_cache_element = NULL;
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
									// q0=? AND w=? AND w_type=?
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 1, parent_id );
									switch( current_cache_element->wmes[j]->attr->common.symbol_type )
									{
										case SYM_CONSTANT_SYMBOL_TYPE:
											sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 2, (const char *) current_cache_element->wmes[j]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
											break;

										case INT_CONSTANT_SYMBOL_TYPE:
			        						EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 2, current_cache_element->wmes[j]->attr->ic.value );
											break;

										case FLOAT_CONSTANT_SYMBOL_TYPE:
			        						sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 2, current_cache_element->wmes[j]->attr->fc.value );
											break;
									}
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 3, current_cache_element->wmes[j]->attr->common.symbol_type );

									while ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] ) == SQLITE_ROW )
									{
										// get identity
										unique_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 0 );
										shared_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ], 1 );

										// create new literal
										new_literal = new epmem_shared_literal;
										new_literal->shared_id = shared_identity;
										new_literal->wme = current_cache_element->wmes[j];
										new_literal->children = NULL;
										new_literal->match = NULL;
										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											// root is always on and satisfies one parental branch
											new_literal->ct = 1;

											// keep track of root literals for graph-match
											if ( ( i == EPMEM_NODE_POS ) && ( graph_match != EPMEM_GRAPH_MATCH_OFF ) )
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
										literals.push_back( new_literal );

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
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_EP ):( EPMEM_TIMER_QUERY_POS_END_EP ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_EP ):( EPMEM_TIMER_QUERY_NEG_END_EP ) ) );
															break;

														case EPMEM_RANGE_NOW:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_NOW ):( EPMEM_TIMER_QUERY_POS_END_NOW ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_NOW ):( EPMEM_TIMER_QUERY_NEG_END_NOW ) ) );
															break;

														case EPMEM_RANGE_POINT:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_POINT ):( EPMEM_TIMER_QUERY_POS_END_POINT ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_POINT ):( EPMEM_TIMER_QUERY_NEG_END_POINT ) ) );
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
													if ( epmem_exec_query( my_agent, new_stmt, new_timer ) == SQLITE_ROW )
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

										// check for children
										cache_hit =& wme_cache[ new_literal->wme->value ];
										if ( !(*cache_hit ) )
										{
											new_cache_element = new epmem_wme_cache_element;
											new_cache_element->wmes = epmem_get_augs_of_id( my_agent, new_literal->wme->value, tc, &( new_cache_element->len ) );

											(*cache_hit) = new_cache_element;
											new_cache_element = NULL;
										}
										new_literal->wme_kids = (*cache_hit)->len;

										if ( (*cache_hit)->len )
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

										new_literal = NULL;
									}
									sqlite3_reset( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_EDGE_UNIQUE ] );
								}
								else
								{
									// parent_id=? AND hash=? AND name=? AND value=? AND attr_type=? AND value_type=?
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 1, parent_id );
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 2, epmem_hash_wme( my_agent, current_cache_element->wmes[j] ) );

									switch( current_cache_element->wmes[j]->attr->common.symbol_type )
									{
										case SYM_CONSTANT_SYMBOL_TYPE:
											sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, (const char *) current_cache_element->wmes[j]->attr->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
											break;

										case INT_CONSTANT_SYMBOL_TYPE:
	        								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, current_cache_element->wmes[j]->attr->ic.value );
											break;

										case FLOAT_CONSTANT_SYMBOL_TYPE:
	        								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 3, current_cache_element->wmes[j]->attr->fc.value );
											break;
									}

									switch( current_cache_element->wmes[j]->value->common.symbol_type )
									{
										case SYM_CONSTANT_SYMBOL_TYPE:
											sqlite3_bind_text( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, (const char *) current_cache_element->wmes[j]->value->sc.name, EPMEM_DB_PREP_STR_MAX, SQLITE_STATIC );
											break;

										case INT_CONSTANT_SYMBOL_TYPE:
	        								EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, current_cache_element->wmes[j]->value->ic.value );
											break;

										case FLOAT_CONSTANT_SYMBOL_TYPE:
	        								sqlite3_bind_double( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 4, current_cache_element->wmes[j]->value->fc.value );
											break;
									}
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 5, current_cache_element->wmes[j]->attr->common.symbol_type );
									EPMEM_SQLITE_BIND_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 6, current_cache_element->wmes[j]->value->common.symbol_type );

									if ( sqlite3_step( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ] ) == SQLITE_ROW )
									{
										// get identity
										unique_identity = EPMEM_SQLITE_COLUMN_INT( my_agent->epmem_statements[ EPMEM_STMT_THREE_FIND_NODE_UNIQUE ], 0 );

										// create new literal
										new_literal = new epmem_shared_literal;
										new_literal->shared_id = EPMEM_NODEID_ROOT;
										new_literal->wme_kids = 0;
										new_literal->wme = current_cache_element->wmes[j];
										new_literal->children = NULL;
										if ( parent_id == EPMEM_NODEID_ROOT )
										{
											new_literal->ct = 1;

											if ( ( i == EPMEM_NODE_POS ) && ( graph_match != EPMEM_GRAPH_MATCH_OFF ) )
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
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_EP ):( EPMEM_TIMER_QUERY_POS_END_EP ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_EP ):( EPMEM_TIMER_QUERY_NEG_END_EP ) ) );
															break;

														case EPMEM_RANGE_NOW:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_NOW ):( EPMEM_TIMER_QUERY_POS_END_NOW ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_NOW ):( EPMEM_TIMER_QUERY_NEG_END_NOW ) ) );
															break;

														case EPMEM_RANGE_POINT:
															new_timer = ( ( i == EPMEM_NODE_POS )?( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_POS_START_POINT ):( EPMEM_TIMER_QUERY_POS_END_POINT ) ):( ( k == EPMEM_RANGE_START )?( EPMEM_TIMER_QUERY_NEG_START_POINT ):( EPMEM_TIMER_QUERY_NEG_END_POINT ) ) );
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
													if ( epmem_exec_query( my_agent, new_stmt, new_timer ) == SQLITE_ROW )
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
				std::map<Symbol *, epmem_wme_cache_element *>::iterator cache_p;
				for ( cache_p=wme_cache.begin(); cache_p!=wme_cache.end(); cache_p++ )
				{
					if ( cache_p->second->wmes )
						free_memory( my_agent, cache_p->second->wmes, MISCELLANEOUS_MEM_USAGE );

					delete cache_p->second;
				}
			}

			epmem_set_stat( my_agent, EPMEM_STAT_QRY_POS, leaf_ids[ EPMEM_NODE_POS ] );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_NEG, leaf_ids[ EPMEM_NODE_NEG ] );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_RET, 0 );
			epmem_set_stat( my_agent, EPMEM_STAT_QRY_CARD, 0 );

			// useful statistics
			int cue_size = ( leaf_ids[ EPMEM_NODE_POS ] + leaf_ids[ EPMEM_NODE_NEG ] );
			unsigned EPMEM_TYPE_INT perfect_match = leaf_ids[ EPMEM_NODE_POS ];

			// vars to set in range search
			epmem_time_id king_id = EPMEM_MEMID_NONE;
			double king_score = -1000;
			unsigned EPMEM_TYPE_INT king_cardinality = 0;
			unsigned EPMEM_TYPE_INT king_graph_match = 0;
			epmem_constraint_list king_constraints;

			// perform range search if any leaf wmes
			if ( cue_size )
			{
				double balance = epmem_get_parameter( my_agent, (const long) EPMEM_PARAM_BALANCE );
				double balance_inv = 1 - balance;

				// dynamic programming stuff
				EPMEM_TYPE_INT sum_ct = 0;
				double sum_v = 0;
				EPMEM_TYPE_INT sum_updown = 0;

				// current pointer
				epmem_time_id current_id = EPMEM_MEMID_NONE;
				EPMEM_TYPE_INT current_ct = 0;
				double current_v = 0;
				EPMEM_TYPE_INT current_updown = 0;
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
				unsigned EPMEM_TYPE_INT current_graph_match_counter = 0;
				epmem_constraint_list current_constraints;

				// initialize current as last end
				// initialize next end
				epmem_shared_increment( my_agent, queries, current_id, current_ct, current_v, current_updown, EPMEM_RANGE_END );
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

							if ( graph_match != EPMEM_GRAPH_MATCH_OFF )
							{
								// policy:
								// - king candidate MUST have AT LEAST king score
								// - perform graph match ONLY if cardinality is perfect
								// - ONLY stop if graph match is perfect

								if ( ( king_id == EPMEM_MEMID_NONE ) || ( current_score >= king_score ) )
								{
									if ( sum_ct == (EPMEM_TYPE_INT) perfect_match )
									{
										if ( graph_match == EPMEM_GRAPH_MATCH_PATHS )
										{
											////////////////////////////////////////////////////////////////////////////
											epmem_start_timer( my_agent, EPMEM_TIMER_QUERY_GRAPH_MATCH );
											////////////////////////////////////////////////////////////////////////////

											current_graph_match_counter = epmem_graph_match_paths( graph_match_roots );

											////////////////////////////////////////////////////////////////////////////
											epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY_GRAPH_MATCH );
											////////////////////////////////////////////////////////////////////////////
										}
										else
										{
											current_constraints.clear();

											////////////////////////////////////////////////////////////////////////////
											epmem_start_timer( my_agent, EPMEM_TIMER_QUERY_GRAPH_MATCH );
											////////////////////////////////////////////////////////////////////////////

											current_graph_match_counter = epmem_graph_match_wmes( graph_match_roots, &current_constraints );

											////////////////////////////////////////////////////////////////////////////
											epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY_GRAPH_MATCH );
											////////////////////////////////////////////////////////////////////////////
										}

										if ( ( king_id == EPMEM_MEMID_NONE ) ||
											 ( current_score > king_score ) ||
											 ( current_graph_match_counter == len_query ) )
										{
											king_id = current_valid_end;
											king_score = current_score;
											king_cardinality = sum_ct;
											king_graph_match = current_graph_match_counter;
											king_constraints = current_constraints;

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

									if ( king_cardinality == perfect_match )
										done = true;
								}
							}
						}

						if ( !done )
						{
							// based upon choice, update variables
							epmem_shared_increment( my_agent, queries, current_id, current_ct, current_v, current_updown, next_list );
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
				if ( graph_match != EPMEM_GRAPH_MATCH_OFF )
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

				epmem_set_stat( my_agent, EPMEM_STAT_QRY_RET, king_id );
				epmem_set_stat( my_agent, EPMEM_STAT_QRY_CARD, king_cardinality );

				// status
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_success_symbol );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );

				// match score
				my_meta = make_float_constant( my_agent, king_score );
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_score_symbol, my_meta );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				symbol_remove_ref( my_agent, my_meta );

				// cue-size
				my_meta = make_int_constant( my_agent, cue_size );
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_cue_size_symbol, my_meta );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				symbol_remove_ref( my_agent, my_meta );

				// normalized-match-score
				my_meta = make_float_constant( my_agent, ( king_score / perfect_match ) );
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_normalized_match_score_symbol, my_meta );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				symbol_remove_ref( my_agent, my_meta );

				// match-cardinality
				my_meta = make_int_constant( my_agent, king_cardinality );
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_match_cardinality_symbol, my_meta );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
				symbol_remove_ref( my_agent, my_meta );

				// graph match
				if ( graph_match != EPMEM_GRAPH_MATCH_OFF )
				{
					// graph-match 0/1
					my_meta = make_int_constant( my_agent, ( ( king_graph_match == len_query )?(1):(0) ) );
					new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_graph_match_symbol, my_meta );
					new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
					state->id.epmem_info->epmem_wmes->push( new_wme );
					symbol_remove_ref( my_agent, my_meta );

					// full mapping if appropriate
					if ( ( graph_match == EPMEM_GRAPH_MATCH_WMES) && ( king_graph_match == len_query ) )
					{
						Symbol *my_meta2;
						Symbol *my_meta3;

						my_meta = make_new_identifier( my_agent, 'M', state->id.epmem_result_header->id.level );
						new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_graph_match_mapping_symbol, my_meta );
						new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
						state->id.epmem_info->epmem_wmes->push( new_wme );
						symbol_remove_ref( my_agent, my_meta );

						my_mapping = new epmem_id_mapping();
						for ( epmem_constraint_list::iterator c_p=king_constraints.begin(); c_p!=king_constraints.end(); c_p++ )
						{
							// create the node
							my_meta2 = make_new_identifier( my_agent, 'N', my_meta->id.level );
							new_wme = add_input_wme( my_agent, my_meta, my_agent->epmem_graph_match_mapping_node_symbol, my_meta2 );
							new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
							state->id.epmem_info->epmem_wmes->push( new_wme );
							symbol_remove_ref( my_agent, my_meta2 );

							// point to the cue identifier
							new_wme = add_input_wme( my_agent, my_meta2, my_agent->epmem_graph_match_mapping_cue_symbol, c_p->first );
							new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
							state->id.epmem_info->epmem_wmes->push( new_wme );

							// create and store away the [yet-to-be-retrieved] identifier
							my_meta3 = make_new_identifier( my_agent, c_p->first->id.name_letter, my_meta2->id.level );
							new_wme = add_input_wme( my_agent, my_meta2, my_agent->epmem_retrieved_symbol, my_meta3 );
							new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
							state->id.epmem_info->epmem_wmes->push( new_wme );
							symbol_remove_ref( my_agent, my_meta3 );
							(*my_mapping)[ c_p->second ] = my_meta3;
						}
					}
				}

				////////////////////////////////////////////////////////////////////////////
				epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
				////////////////////////////////////////////////////////////////////////////

				// actual memory
				epmem_install_memory( my_agent, state, king_id, my_mapping );

				if ( my_mapping )
					delete my_mapping;
			}
			else
			{
				new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_failure_symbol );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );

				////////////////////////////////////////////////////////////////////////////
				epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
				////////////////////////////////////////////////////////////////////////////
			}
		}
	}
	else
	{
		wme *new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );
		new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
		state->id.epmem_info->epmem_wmes->push( new_wme );

		free_memory( my_agent, wmes_query, MISCELLANEOUS_MEM_USAGE );
		free_memory( my_agent, wmes_neg_query, MISCELLANEOUS_MEM_USAGE );

		////////////////////////////////////////////////////////////////////////////
		epmem_stop_timer( my_agent, EPMEM_TIMER_QUERY );
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
	epmem_start_timer( my_agent, EPMEM_TIMER_TRIGGER );
	////////////////////////////////////////////////////////////////////////////

	const long force = epmem_get_parameter( my_agent, EPMEM_PARAM_FORCE, EPMEM_RETURN_LONG );
	bool new_memory = false;

	if ( force == EPMEM_FORCE_OFF )
	{
		const long trigger = epmem_get_parameter( my_agent, EPMEM_PARAM_TRIGGER, EPMEM_RETURN_LONG );

		if ( trigger == EPMEM_TRIGGER_OUTPUT )
		{
			slot *s;
			wme *w;
			Symbol *ol = my_agent->io_header_output;
			unsigned EPMEM_TYPE_INT wme_count = 0;

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
				new_memory = true;
				my_agent->bottom_goal->id.epmem_info->last_ol_count = wme_count;
			}
		}
		else if ( trigger == EPMEM_TRIGGER_DC )
		{
			new_memory = true;
		}
		else if ( trigger == EPMEM_TRIGGER_NONE )
		{
			new_memory = false;
		}
	}
	else
	{
		new_memory = ( force == EPMEM_FORCE_REMEMBER );

		epmem_set_parameter( my_agent, (const long) EPMEM_PARAM_FORCE, (const long) EPMEM_FORCE_OFF );
	}

	////////////////////////////////////////////////////////////////////////////
	epmem_stop_timer( my_agent, EPMEM_TIMER_TRIGGER );
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
	epmem_start_timer( my_agent, EPMEM_TIMER_API );
	////////////////////////////////////////////////////////////////////////////

	// start at the bottom and work our way up
	// (could go in the opposite direction as well)
	Symbol *state = my_agent->bottom_goal;

	wme **wmes;
	const char *attr_name;
	unsigned EPMEM_TYPE_INT len;
	unsigned EPMEM_TYPE_INT i;

	epmem_time_id retrieve;
	bool next, previous;
	Symbol *query;
	Symbol *neg_query;
	epmem_time_list *prohibit;
	epmem_time_id before, after;
	bool good_cue;
	int path;

	slot *s;
	wme *w;
	Symbol *epmem_cmd;
	unsigned EPMEM_TYPE_INT wme_count;
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
			epmem_stop_timer( my_agent, EPMEM_TIMER_API );
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
				wme *new_wme = add_input_wme( my_agent, state->id.epmem_result_header, my_agent->epmem_status_symbol, my_agent->epmem_bad_cmd_symbol );
				new_wme->preference = epmem_make_fake_preference( my_agent, state, new_wme );
				state->id.epmem_info->epmem_wmes->push( new_wme );
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
	epmem_start_timer( my_agent, EPMEM_TIMER_TOTAL );

	if ( !epmem_in_transaction( my_agent ) )
		epmem_transaction_begin( my_agent );

	epmem_consider_new_episode( my_agent );
	epmem_respond_to_cmd( my_agent );

	if ( !epmem_in_transaction( my_agent ) )
		epmem_transaction_end( my_agent, true );

	epmem_stop_timer( my_agent, EPMEM_TIMER_TOTAL );
}
