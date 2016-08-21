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

#include "kernel.h"

#include "stl_typedefs.h"

#include <string>

//#define SMEM_EXPERIMENT  // hijack the main SMem function for tight-loop experimentation/timing

//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

extern bool smem_enabled(agent* thisAgent);
extern void smem_attach(agent* thisAgent);

extern bool smem_parse_chunks(agent* thisAgent, const char* chunks, std::string** err_msg);
extern bool smem_parse_cues(agent* thisAgent, const char* chunks, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve);
extern bool smem_parse_remove(agent* thisAgent, const char* chunks, std::string** err_msg, std::string** result_message, bool force = false);

extern void smem_visualize_store(agent* thisAgent, std::string* return_val);
extern void smem_visualize_lti(agent* thisAgent, smem_lti_id lti_id, unsigned int depth, std::string* return_val);
extern void smem_print_store(agent* thisAgent, std::string* return_val);
extern void smem_print_lti(agent* thisAgent, smem_lti_id lti_id, uint64_t depth, std::string* return_val, bool history = false);

extern bool smem_valid_production(condition* lhs_top, action* rhs_top);

extern smem_lti_id smem_lti_get_id(agent* thisAgent, char name_letter, uint64_t name_number);
extern Symbol* smem_lti_soar_make(agent* thisAgent, smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level);
extern void smem_lti_soar_promote_STI(agent* thisAgent, Symbol* id);

extern void smem_reset(agent* thisAgent, Symbol* state);
extern void smem_reset_id_counters(agent* thisAgent);
extern void smem_close(agent* thisAgent);
extern void smem_reinit(agent* thisAgent);
extern void smem_reinit_cmd(agent* thisAgent);

// perform smem actions
extern void smem_go(agent* thisAgent, bool store_only);
extern bool smem_backup_db(agent* thisAgent, const char* file_name, std::string* err);

void smem_init_db(agent* thisAgent);

class SMem_Manager
{
    public:
        SMem_Manager(agent* myAgent);
        ~SMem_Manager() {};

        void clean_up_for_agent_deletion();

        smem_param_container*           smem_params;
        smem_stat_container*            smem_stats;
        smem_timer_container*           smem_timers;
        soar_module::sqlite_database*   smem_db;
        smem_statement_container*       smem_stmts;

        uint64_t                        smem_validation;
        int64_t                         smem_max_cycle;
        symbol_set*                     smem_changed_ids;
        bool                            smem_ignore_changes;

    private:

        agent*                          thisAgent;

};

#endif
