/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.h
 *
 * =======================================================================
 */

#ifndef SEMANTIC_MEMORY_H
#define SEMANTIC_MEMORY_H

#include <portability.h>

#include <stack>
#include <set>
#include <list>
#include <vector>

#include "soar_module.h"
#include "soar_db.h"

using namespace soar_module;


//////////////////////////////////////////////////////////
// SMem Experimentation
//
// If defined, we hijack the main SMem function
// for tight-loop experimentation/timing.
//
//////////////////////////////////////////////////////////

//#define SMEM_EXPERIMENT


//////////////////////////////////////////////////////////
// SMem Parameters
//////////////////////////////////////////////////////////

class smem_path_param;

class smem_param_container: public param_container
{
	public:
		enum db_choices { memory, file };		

		boolean_param *learning;
		constant_param<db_choices> *database;
		smem_path_param *path;		
		
		constant_param<timer::timer_level> *timers;

		smem_param_container( agent *new_agent );
};

class smem_path_param: public string_param
{
	protected:
		agent *my_agent;

	public:
		smem_path_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred, agent *new_agent );
		virtual void set_value( const char *new_value );
};

template <typename T>
class smem_db_predicate: public agent_predicate<T>
{
	public:
		smem_db_predicate( agent *new_agent );
		bool operator() ( T val );
};


//////////////////////////////////////////////////////////
// SMem Statistics
//////////////////////////////////////////////////////////

class smem_mem_usage_stat;
class smem_mem_high_stat;

class smem_stat_container: public stat_container
{
	public:
		smem_mem_usage_stat *mem_usage;
		smem_mem_high_stat *mem_high;

		smem_stat_container( agent *my_agent );
};

class smem_mem_usage_stat: public integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_usage_stat( agent *new_agent, const char *new_name, long new_value, predicate<long> *new_prot_pred );
		long get_value();
};

//

class smem_mem_high_stat: public integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_high_stat( agent *new_agent, const char *new_name, long new_value, predicate<long> *new_prot_pred );
		long get_value();
};


//////////////////////////////////////////////////////////
// SMem Timers
//////////////////////////////////////////////////////////

class smem_timer_container: public timer_container
{
	public:
		timer *total;
		timer *storage;
		timer *ncb_retrieval;
		timer *query;
		timer *api;		
		timer *init;		
		timer *hash;

		smem_timer_container( agent *my_agent );
};

class smem_timer_level_predicate: public agent_predicate<timer::timer_level>
{
	public:
		smem_timer_level_predicate( agent *new_agent );
		bool operator() ( timer::timer_level val );
};

class smem_timer: public timer
{
	public:
		smem_timer( const char *new_name, agent *new_agent, timer_level new_level );
};


//////////////////////////////////////////////////////////
// EpMem Statements
//////////////////////////////////////////////////////////

class smem_statement_container: public sqlite_statement_container
{
	public:
		sqlite_statement *begin;
		sqlite_statement *commit;
		sqlite_statement *rollback;

		sqlite_statement *var_get;
		sqlite_statement *var_set;

		sqlite_statement *hash_get;
		sqlite_statement *hash_add;

		sqlite_statement *lti_add;
		sqlite_statement *lti_get;

		sqlite_statement *web_add;
		sqlite_statement *web_truncate;
		sqlite_statement *web_expand;

		sqlite_statement *ct_attr_add;
		sqlite_statement *ct_const_add;
		sqlite_statement *ct_lti_add;

		sqlite_statement *ct_attr_update;
		sqlite_statement *ct_const_update;
		sqlite_statement *ct_lti_update;

		sqlite_statement *ct_attr_get;
		sqlite_statement *ct_const_get;
		sqlite_statement *ct_lti_get;

		sqlite_statement *act_set;

		smem_statement_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// Soar Constants
//////////////////////////////////////////////////////////

enum smem_variable_key
{
	var_stuff
};


//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// represents the unique identification of a
// long-term identifier
typedef unsigned long smem_lti_id;

// represents an activation cycle
typedef unsigned long smem_activation_cycle;

// represents a vector of long-term identifiers
typedef std::vector<smem_lti_id> smem_lti_list;

// a list of symbols
typedef std::list<Symbol *> smem_sym_list;

// ways to store an identifier
enum smem_storage_type { store_level, store_recursive };

// represents a list of wmes
typedef std::list<wme *> smem_wme_list;

// data associated with each state
typedef struct smem_data_struct
{
	unsigned long last_cmd_time;	// last update to smem.command
	unsigned long last_cmd_count;	// last update to smem.command

	std::set<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *smem_wmes;	// wmes in last smem
} smem_data;


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

inline extern bool smem_enabled( agent *my_agent );

extern void smem_init_db( agent *my_agent, bool readonly = false );
extern void smem_close( agent *my_agent );

// perform smem actions
extern void smem_go( agent *my_agent );

#endif
