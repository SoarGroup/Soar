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
#include <queue>

#include "soar_module.h"
#include "soar_db.h"

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

class smem_param_container: public soar_module::param_container
{
	public:
		enum db_choices { memory, file };		

		soar_module::boolean_param *learning;
		soar_module::constant_param<db_choices> *database;
		smem_path_param *path;
		soar_module::boolean_param *lazy_commit;
		
		soar_module::constant_param<soar_module::timer::timer_level> *timers;

		smem_param_container( agent *new_agent );
};

class smem_path_param: public soar_module::string_param
{
	protected:
		agent *my_agent;

	public:
		smem_path_param( const char *new_name, const char *new_value, soar_module::predicate<const char *> *new_val_pred, soar_module::predicate<const char *> *new_prot_pred, agent *new_agent );
		virtual void set_value( const char *new_value );
};

template <typename T>
class smem_db_predicate: public soar_module::agent_predicate<T>
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

class smem_stat_container: public soar_module::stat_container
{
	public:
		smem_mem_usage_stat *mem_usage;
		smem_mem_high_stat *mem_high;

		smem_stat_container( agent *my_agent );
};

class smem_mem_usage_stat: public soar_module::integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_usage_stat( agent *new_agent, const char *new_name, long new_value, soar_module::predicate<long> *new_prot_pred );
		long get_value();
};

//

class smem_mem_high_stat: public soar_module::integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_high_stat( agent *new_agent, const char *new_name, long new_value, soar_module::predicate<long> *new_prot_pred );
		long get_value();
};


//////////////////////////////////////////////////////////
// SMem Timers
//////////////////////////////////////////////////////////

class smem_timer_container: public soar_module::timer_container
{
	public:
		soar_module::timer *total;
		soar_module::timer *storage;
		soar_module::timer *ncb_retrieval;
		soar_module::timer *query;
		soar_module::timer *api;		
		soar_module::timer *init;		
		soar_module::timer *hash;

		smem_timer_container( agent *my_agent );
};

class smem_timer_level_predicate: public soar_module::agent_predicate<soar_module::timer::timer_level>
{
	public:
		smem_timer_level_predicate( agent *new_agent );
		bool operator() ( soar_module::timer::timer_level val );
};

class smem_timer: public soar_module::timer
{
	public:
		smem_timer( const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level );
};


//////////////////////////////////////////////////////////
// EpMem Statements
//////////////////////////////////////////////////////////

class smem_statement_container: public soar_module::sqlite_statement_container
{
	public:
		soar_module::sqlite_statement *begin;
		soar_module::sqlite_statement *commit;
		soar_module::sqlite_statement *rollback;

		soar_module::sqlite_statement *var_get;
		soar_module::sqlite_statement *var_set;

		soar_module::sqlite_statement *hash_get;
		soar_module::sqlite_statement *hash_add;

		soar_module::sqlite_statement *lti_add;
		soar_module::sqlite_statement *lti_get;
		soar_module::sqlite_statement *lti_letter_num;
		soar_module::sqlite_statement *lti_max;

		soar_module::sqlite_statement *web_add;
		soar_module::sqlite_statement *web_truncate;
		soar_module::sqlite_statement *web_expand;

		soar_module::sqlite_statement *web_attr_ct;
		soar_module::sqlite_statement *web_const_ct;
		soar_module::sqlite_statement *web_lti_ct;

		soar_module::sqlite_statement *web_attr_all;
		soar_module::sqlite_statement *web_const_all;
		soar_module::sqlite_statement *web_lti_all;

		soar_module::sqlite_statement *web_attr_child;
		soar_module::sqlite_statement *web_const_child;
		soar_module::sqlite_statement *web_lti_child;

		soar_module::sqlite_statement *ct_attr_add;
		soar_module::sqlite_statement *ct_const_add;
		soar_module::sqlite_statement *ct_lti_add;

		soar_module::sqlite_statement *ct_attr_update;
		soar_module::sqlite_statement *ct_const_update;
		soar_module::sqlite_statement *ct_lti_update;

		soar_module::sqlite_statement *ct_attr_get;
		soar_module::sqlite_statement *ct_const_get;
		soar_module::sqlite_statement *ct_lti_get;

		soar_module::sqlite_statement *act_set;

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

// represents a collection of long-term identifiers
typedef std::list<smem_lti_id> smem_lti_list;
typedef std::set<smem_lti_id> smem_lti_set;

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

//

enum smem_cue_element_type { attr_t, value_const_t, value_lti_t };

typedef struct smem_weighted_cue_element_struct
{
	unsigned long weight;
	
	struct wme_struct *cue_element;
	long attr_hash;
	long value_hash;
	smem_lti_id value_lti;

	smem_cue_element_type element_type;

} smem_weighted_cue_element;

struct smem_compare_weighted_cue_elements
{
	bool operator() ( const smem_weighted_cue_element *a, const smem_weighted_cue_element *b ) const
	{
		return ( a->weight > b->weight );
	}
};

typedef std::priority_queue<smem_weighted_cue_element *, std::vector<smem_weighted_cue_element *>, smem_compare_weighted_cue_elements> smem_weighted_cue;


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

extern inline bool smem_enabled( agent *my_agent );

extern void smem_reset_id_counters( agent *my_agent );
extern void smem_init_db( agent *my_agent, bool readonly = false );
extern void smem_close( agent *my_agent );

// perform smem actions
extern void smem_go( agent *my_agent );

#endif
