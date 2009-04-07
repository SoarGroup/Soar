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

#include "soar_module.h"
#include "soar_db.h"

using namespace soar_module;

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
		integer_param *commit;
		
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
		integer_stat *next_id;

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
		timer *next;
		timer *prev;
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

		smem_statement_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// data associated with each state
typedef struct smem_data_struct
{
	unsigned long last_cmd_time;	// last update to smem.command
	unsigned long last_cmd_count;	// last update to smem.command

	std::set<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *smem_wmes;	// wmes in last smem
} smem_data;


#endif
