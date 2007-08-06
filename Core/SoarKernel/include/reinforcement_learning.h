/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learing.h
 *
 * =======================================================================
 */

#ifndef REINFORCEMENT_LEARNING_H
#define REINFORCEMENT_LEARNING_H

#include <map>
#include <vector>

//////////////////////////////////////////////////////////
// RL Constants
//////////////////////////////////////////////////////////
#define RL_RETURN_LONG 0.1
#define RL_RETURN_STRING ""

#define RL_LEARNING_ON 1
#define RL_LEARNING_OFF 2

#define RL_ACCUMULATION_SUM 1
#define RL_ACCUMULATION_AVG 2

#define RL_DISCOUNT_EXPONENTIAL 1
#define RL_DISCOUNT_LINEAR 2

#define RL_LEARNING_SARSA 1
#define RL_LEARNING_Q 2

//////////////////////////////////////////////////////////
// RL Types
//////////////////////////////////////////////////////////
typedef struct rl_string_parameter_struct  
{
	long value;
	bool (*val_func)( const char * );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} rl_string_parameter;

typedef struct rl_number_parameter_struct  
{
	double value;
	bool (*val_func)( double );
} rl_number_parameter;

typedef union rl_parameter_union
{
	rl_string_parameter string_param;
	rl_number_parameter number_param;
} rl_parameter;

enum rl_param_type { rl_param_string = 1, rl_param_number = 2, rl_param_invalid = 3 };

typedef struct rl_parameter_tracking_struct
{
	const char *name;
	rl_param_type type;
} rl_parameter_tracking;

//////////////////////////////////////////////////////////
// Parameter Type Tracking
//////////////////////////////////////////////////////////

// add tracking info
extern rl_parameter_tracking *add_rl_tracking( const char *name, rl_param_type type );

//////////////////////////////////////////////////////////
// Parameter Get/Set/Validate
//////////////////////////////////////////////////////////

// add parameter
extern rl_parameter *add_rl_parameter( double value, bool (*val_func)( double ) );
extern rl_parameter *add_rl_parameter( const char *value, bool (*val_func)( const char * ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );

// validate parameter
extern bool valid_rl_parameter( agent *my_agent, const char *name );

// parameter type
extern rl_param_type get_rl_parameter_type( agent *my_agent, const char *name );

// get parameter
extern const long get_rl_parameter( agent *my_agent, const char *name, const double test );
extern const char *get_rl_parameter( agent *my_agent, const char *name, const char *test );
extern double get_rl_parameter( agent *my_agent, const char *name );

// validate parameter value
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, const long new_val );

// set parameter
extern bool set_rl_parameter( agent *my_agent, const char *name, double new_val );
extern bool set_rl_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool set_rl_parameter( agent *my_agent, const char *name, const long new_val );

// learning
extern bool validate_rl_learning( const char *new_val );
extern const char *convert_rl_learning( const long val );
extern const long convert_rl_learning( const char *val );

// accumulation mode
extern bool validate_rl_accumulation( const char *new_val );
extern const char *convert_rl_accumulation( const long val );
extern const long convert_rl_accumulation( const char *val );

// discount mode
extern bool validate_rl_discount( const char *new_val );
extern const char *convert_rl_discount( const long val );
extern const long convert_rl_discount( const char *val );

// exponential discount rate
extern bool validate_rl_exp_discount( double new_val );

// linear discount rate
extern bool validate_rl_lin_discount( double new_val );

// learning rate
extern bool validate_rl_learning_rate( double new_val );

// learning policy
extern bool validate_rl_learning_policy( const char *new_val );
extern const char *convert_rl_learning_policy( const long val );
extern const long convert_rl_learning_policy( const char *val );

// trace decay rate
extern bool validate_rl_decay_rate( double new_val );

// trace tolerance
extern bool validate_rl_trace_tolerance( double new_val );

//////////////////////////////////////////////////////////
// Stats
//////////////////////////////////////////////////////////

// valid stat
extern bool valid_rl_stat( agent *my_agent, const char *name );

// get stat
extern double get_rl_stat( agent *my_agent, const char *name );

// set stat
extern bool set_rl_stat( agent *my_agent, const char *name, double new_val );

#endif
