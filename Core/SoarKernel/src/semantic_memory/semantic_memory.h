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
#include "smem_settings.h"

#include <string>

//#define SMEM_EXPERIMENT  // hijack the main SMem function for tight-loop experimentation/timing
        class smem_statement_container;
        class smem_path_param;
        class smem_db_lib_version_stat;
        class smem_mem_usage_stat;
        class smem_mem_high_stat;
        class smem_timer_level_predicate;

class SMem_Manager
{
        friend cli::CommandLineInterface;
        friend smem_statement_container;
        friend smem_path_param;
        friend smem_db_lib_version_stat;
        friend smem_mem_usage_stat;
        friend smem_mem_high_stat;
        friend smem_timer_level_predicate;
        friend smem_db_predicate<int64_t>;
        friend smem_db_predicate<smem_param_container::page_choices>;
        friend smem_db_predicate<smem_param_container::opt_choices>;
        friend smem_db_predicate<boolean>;


    public:
        SMem_Manager(agent* myAgent);
        ~SMem_Manager() {};

        void clean_up_for_agent_deletion();
        bool enabled();
        bool connected();
        void attach();

        bool process_smem_add_object(const char* chunks, std::string** err_msg);
        bool parse_cues(const char* chunks, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve);
        bool process_smem_remove(const char* chunks, std::string** err_msg, std::string** result_message, bool force = false);

        void visualize_store(std::string* return_val);
        void visualize_lti(smem_lti_id lti_id, unsigned int depth, std::string* return_val);
        void print_store(std::string* return_val);
        void print_smem_object(smem_lti_id lti_id, uint64_t depth, std::string* return_val, bool history = false);

        smem_lti_id lti_exists(uint64_t pLTI_ID);
        smem_lti_id get_max_lti_id();

        void reset(Symbol* state);
        void reset_id_counters();
        void close();
        void reinit();
        void reinit_cmd();
        void reset_stats();

        // perform smem actions
        void go(bool store_only);
        bool backup_db(const char* file_name, std::string* err);

        bool is_version_one_db();

        /* The following remains public because used in run_soar.cpp */
        smem_timer_container*           smem_timers;

    private:

        agent*                          thisAgent;
        uint64_t                        lti_id_counter;

        uint64_t                        smem_validation;
        int64_t                         smem_max_cycle;

        smem_statement_container*       smem_stmts;
        smem_param_container*           smem_params;
        smem_stat_container*            smem_stats;
        soar_module::sqlite_database*   smem_db;

        void            store_globals_in_db();
        void            variable_create(smem_variable_key variable_id, int64_t variable_value);
        void            variable_set(smem_variable_key variable_id, int64_t variable_value);
        bool            variable_get(smem_variable_key variable_id, int64_t* variable_value);
        smem_hash_id    hash_add_type(byte symbol_type);
        smem_hash_id    hash_int(int64_t val, bool add_on_fail = true);
        smem_hash_id    hash_float(double val, bool add_on_fail = true);
        smem_hash_id    hash_str(char* val, bool add_on_fail = true);
        smem_hash_id    hash(Symbol* sym, bool add_on_fail = true);
        int64_t         rhash__int(smem_hash_id hash_value);
        double          rhash__float(smem_hash_id hash_value);
        void            rhash__str(smem_hash_id hash_value, std::string& dest);
        Symbol*         rhash_(byte symbol_type, smem_hash_id hash_value);

        void            update_schema_one_to_two();
        void            init_db();
        void            switch_to_memory_db(std::string& buf);

        void            get_lti_name(smem_lti_id lti_id, std::string &lti_name);
        smem_wme_list*  get_direct_augs_of_id(Symbol* id, tc_number tc = NIL);
        void            install_buffered_triple_list(Symbol* state, wme_set& cue_wmes, symbol_triple_list& my_list, bool meta);
        void            install_recall_buffer(Symbol* state, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes);
        void            add_triple_to_recall_buffer(symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value);
        double          lti_calc_base(smem_lti_id lti, int64_t time_now, uint64_t n = 0, uint64_t activations_first = 0);
        double          lti_activate(smem_lti_id lti, bool add_access, uint64_t num_edges = SMEM_ACT_MAX);
        void            lti_from_test(test t, std::set<Symbol*>* valid_ltis);
        void            lti_from_rhs_value(rhs_value rv, std::set<Symbol*>* valid_ltis);
        smem_lti_id     add_new_lti_id();
        void            link_sti_to_lti(Symbol* id);
        smem_slot*      make_smem_slot(smem_slot_map* slots, Symbol* attr);
        void            disconnect_chunk(smem_lti_id lti_id);
        void            add_semantic_object_to_smem(smem_lti_id lti_id, smem_slot_map* children, bool remove_old_children = true, Symbol* print_id = NULL, bool activate = true);
        void            store_in_smem(Symbol* id, smem_storage_type store_type = store_level, tc_number tc = NIL);
        void            install_memory(Symbol* state, smem_lti_id lti_id, Symbol* lti, bool activate_lti, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_install_type install_type = wm_install, uint64_t depth = 1, std::set<smem_lti_id>* visited = NULL);
        bool            process_cue_wme(wme* w, bool pos_cue, smem_prioritized_weighted_cue& weighted_pq, MathQuery* mathQuery);
        smem_lti_id     process_query(Symbol* state, Symbol* query, Symbol* negquery, Symbol* mathQuery, smem_lti_set* prohibit, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_query_levels query_level = qry_full, uint64_t number_to_retrieve = 1, std::list<smem_lti_id>* match_ids = NIL, uint64_t depth = 1, smem_install_type install_type = wm_install);
        void            clear_result(Symbol* state);
        void            deallocate_chunk(smem_chunk* chunk, bool free_chunk = true);
        std::string*    parse_lti_name(soar::Lexeme* lexeme, char* id_letter, uint64_t* id_number);
        Symbol*         parse_constant_attr(soar::Lexeme* lexeme);
        bool            process_smem_add(soar::Lexer* lexer, smem_str_to_chunk_map* chunks, smem_chunk_set* newbies);
        void            respond_to_cmd(bool store_only);

        soar_module::sqlite_statement*  setup_web_crawl(smem_weighted_cue_element* el);
        std::pair<bool, bool>*          processMathQuery(Symbol* mathQuery, smem_prioritized_weighted_cue* weighted_pq);
        std::set< smem_lti_id >         print_lti(smem_lti_id lti_id, double lti_act, std::string* return_val, std::list<uint64_t>* history = NIL);

        void            clear_instance_mappings();
        Symbol*         get_sti_for_lti(smem_lti_id pLTI_ID, goal_stack_level pLevel, char pChar = 'L');
        uint64_t        get_identity_for_recalled_sti(Symbol* pSTI, uint64_t pI_ID);

        id_to_sym_map  lti_to_sti_map;
        sym_to_id_map  sti_to_identity_map;
};

#endif
