/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  exploration.h
 *
 * =======================================================================
 */

#ifndef EXPLORATION_H
#define EXPLORATION_H

#include <map>
#include <sstream>

//////////////////////////////////////////////////////////
// Exploration constants
//////////////////////////////////////////////////////////
#define EXPLORATION_EXPONENTIAL 1
#define EXPLORATION_LINEAR 2

//////////////////////////////////////////////////////////
// Exploration Types
//////////////////////////////////////////////////////////
typedef struct exploration_parameter_struct  
{
	double value;
	long reduction_policy;
	std::map<long, double> rates;
} exploration_parameter;

typedef std::map<const char *, long>::iterator reduction_policy_iterator;

//////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////

// Conversion of value to string
template<class T> const char * to_string( T &x )
{
	// instantiate stream
	std::ostringstream o;
	
	// get value into stream
	o << x;
	
	// spit value back as string
	return o.str().c_str();
}

//////////////////////////////////////////////////////////
// Exploration Policies
//////////////////////////////////////////////////////////

// policy <=> name conversion
extern const long convert_exploration_policy( const char *policy_name );
extern const char *convert_exploration_policy( const long policy );

// sets exploration policy name
extern bool set_exploration_policy( agent *my_agent, const char *policy_name );
extern bool set_exploration_policy( agent *my_agent, const long policy );

// get exploration policy
extern const long get_exploration_policy( agent *my_agent );

//////////////////////////////////////////////////////////
// Exploration Policy Parameters
//////////////////////////////////////////////////////////

// add parameter
extern exploration_parameter *add_exploration_parameter( double value );

// validate parameter name
extern bool valid_parameter( agent *my_agent, const char *name );

// get parameter value
extern double get_parameter_value( agent *my_agent, const char *parameter );

#endif