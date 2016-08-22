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
#include "smem_structs.h"

#include <string>

//#define SMEM_EXPERIMENT  // hijack the main SMem function for tight-loop experimentation/timing

class SMem_Manager
{
    public:
        SMem_Manager(agent* myAgent);
        ~SMem_Manager() {};

        void clean_up_for_agent_deletion();
        bool smem_enabled();
        void smem_attach();

        bool smem_parse_chunks(const char* chunks, std::string** err_msg);
        bool smem_parse_cues(const char* chunks, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve);
        bool smem_parse_remove(const char* chunks, std::string** err_msg, std::string** result_message, bool force = false);

        void smem_visualize_store(std::string* return_val);
        void smem_visualize_lti(smem_lti_id lti_id, unsigned int depth, std::string* return_val);
        void smem_print_store(std::string* return_val);
        void smem_print_lti(smem_lti_id lti_id, uint64_t depth, std::string* return_val, bool history = false);

        smem_lti_id smem_lti_get_id(char name_letter, uint64_t name_number);
        Symbol* smem_lti_soar_make(smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level);
        void smem_lti_soar_promote_STI(Symbol* id);

        void smem_reset(Symbol* state);
        void smem_reset_id_counters();
        void smem_close();
        void smem_reinit();
        void smem_reinit_cmd();

        // perform smem actions
        void smem_go(bool store_only);
        bool smem_backup_db(const char* file_name, std::string* err);

        void smem_init_db();
        bool smem_version_one();

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

        void            store_globals_in_db();
        void            smem_variable_create(smem_variable_key variable_id, int64_t variable_value);
        void            smem_variable_set(smem_variable_key variable_id, int64_t variable_value);
        bool            smem_variable_get(smem_variable_key variable_id, int64_t* variable_value);
        smem_hash_id    smem_temporal_hash_add_type(byte symbol_type);
        smem_hash_id    smem_temporal_hash_int(int64_t val, bool add_on_fail = true);
        smem_hash_id    smem_temporal_hash_float(double val, bool add_on_fail = true);
        smem_hash_id    smem_temporal_hash_str(char* val, bool add_on_fail = true);
        smem_hash_id    smem_temporal_hash(Symbol* sym, bool add_on_fail = true);
        int64_t         smem_reverse_hash_int(smem_hash_id hash_value);
        double          smem_reverse_hash_float(smem_hash_id hash_value);
        void            smem_reverse_hash_str(smem_hash_id hash_value, std::string& dest);
        Symbol*         smem_reverse_hash(byte symbol_type, smem_hash_id hash_value);

        void            smem_update_schema_one_to_two();
        void            smem_switch_to_memory_db(std::string& buf);

        smem_wme_list*  smem_get_direct_augs_of_id(Symbol* id, tc_number tc = NIL);
        void            _smem_process_buffered_wme_list(Symbol* state, wme_set& cue_wmes, symbol_triple_list& my_list, bool meta);
        void            smem_process_buffered_wmes(Symbol* state, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes);
        void            smem_buffer_add_wme(symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value);
        double          smem_lti_calc_base(smem_lti_id lti, int64_t time_now, uint64_t n = 0, uint64_t activations_first = 0);
        double          smem_lti_activate(smem_lti_id lti, bool add_access, uint64_t num_edges = SMEM_ACT_MAX);
        void            _smem_lti_from_test(test t, std::set<Symbol*>* valid_ltis);
        void            _smem_lti_from_rhs_value(rhs_value rv, std::set<Symbol*>* valid_ltis);
        smem_lti_id     smem_lti_add_id(char name_letter, uint64_t name_number);
        void            smem_lti_soar_add(Symbol* id);
        smem_slot*      smem_make_slot(smem_slot_map* slots, Symbol* attr);
        void            smem_disconnect_chunk(smem_lti_id lti_id);
        void            smem_store_chunk(smem_lti_id lti_id, smem_slot_map* children, bool remove_old_children = true, Symbol* print_id = NULL, bool activate = true);
        void            smem_soar_store(Symbol* id, smem_storage_type store_type = store_level, tc_number tc = NIL);
        void            smem_install_memory(Symbol* state, smem_lti_id lti_id, Symbol* lti, bool activate_lti, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_install_type install_type = wm_install, uint64_t depth = 1, std::set<smem_lti_id>* visited = NULL);
        bool            _smem_process_cue_wme(wme* w, bool pos_cue, smem_prioritized_weighted_cue& weighted_pq, MathQuery* mathQuery);
        smem_lti_id     smem_process_query(Symbol* state, Symbol* query, Symbol* negquery, Symbol* mathQuery, smem_lti_set* prohibit, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_query_levels query_level = qry_full, uint64_t number_to_retrieve = 1, std::list<smem_lti_id>* match_ids = NIL, uint64_t depth = 1, smem_install_type install_type = wm_install);
        void            smem_clear_result(Symbol* state);
        void            smem_deallocate_chunk(smem_chunk* chunk, bool free_chunk = true);
        std::string*    smem_parse_lti_name(soar::Lexeme* lexeme, char* id_letter, uint64_t* id_number);
        Symbol*         smem_parse_constant_attr(soar::Lexeme* lexeme);
        bool            smem_parse_chunk(soar::Lexer* lexer, smem_str_to_chunk_map* chunks, smem_chunk_set* newbies);
        void            smem_respond_to_cmd(bool store_only);

        soar_module::sqlite_statement*  smem_setup_web_crawl(smem_weighted_cue_element* el);
        std::pair<bool, bool>*          processMathQuery(Symbol* mathQuery, smem_prioritized_weighted_cue* weighted_pq);
        std::set< smem_lti_id >         _smem_print_lti(smem_lti_id lti_id, char lti_letter, uint64_t lti_number, double lti_act, std::string* return_val, std::list<uint64_t>* history = NIL);

};

#endif
