/*
 * smem_db.h
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_DB_H_
#define CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_DB_H_

#include "kernel.h"

#include "smem_stats.h"
#include "soar_module.h"

#include "Database.h"
#include "Statement.h"

#include <memory>

namespace SMemExperimental
{

class statement_container
{
protected:
    std::list<std::string> structure;
    std::shared_ptr<SQLite::Database> DB;

public:
    statement_container(std::shared_ptr<SQLite::Database> database, std::function<void()> initializationCode = [](){})
    : DB(database)
    {
        initializationCode();
    }


    void add_structure(const std::string& new_structure)
    {
        structure.push_back(new_structure);
    }

    void createStructure()
    {
        for (auto& statement : structure)
        {
            auto s = SQLite::Statement(*DB, statement);
            s.exec();
        }
    }
};

class smem_statement_container : public SMemExperimental::statement_container
{
    public:
<<<<<<< HEAD
        SQLite::Statement begin;
        SQLite::Statement commit;
        SQLite::Statement rollback;

        SQLite::Statement var_get;
        SQLite::Statement var_set;
        SQLite::Statement var_create;

        SQLite::Statement hash_rev_int;
        SQLite::Statement hash_rev_float;
        SQLite::Statement hash_rev_str;
        SQLite::Statement hash_rev_type;
        SQLite::Statement hash_get_int;
        SQLite::Statement hash_get_float;
        SQLite::Statement hash_get_str;
        SQLite::Statement hash_add_type;
        SQLite::Statement hash_add_int;
        SQLite::Statement hash_add_float;
        SQLite::Statement hash_add_str;

        SQLite::Statement lti_id_exists;
        SQLite::Statement lti_id_max  ;
        SQLite::Statement lti_add;
        SQLite::Statement lti_access_get;
        SQLite::Statement lti_access_set;
        SQLite::Statement lti_get_t;

        SQLite::Statement web_add;
        SQLite::Statement web_truncate;
        SQLite::Statement web_expand;

        SQLite::Statement web_all;

        SQLite::Statement web_attr_all;
        SQLite::Statement web_const_all;
        SQLite::Statement web_lti_all;

        SQLite::Statement web_attr_child;
        SQLite::Statement web_const_child;
        SQLite::Statement web_lti_child;

        SQLite::Statement attribute_frequency_check;
        SQLite::Statement wmes_constant_frequency_check;
        SQLite::Statement wmes_lti_frequency_check;

        SQLite::Statement attribute_frequency_add;
        SQLite::Statement wmes_constant_frequency_add;
        SQLite::Statement wmes_lti_frequency_add;

        SQLite::Statement attribute_frequency_update;
        SQLite::Statement wmes_constant_frequency_update;
        SQLite::Statement wmes_lti_frequency_update;

        SQLite::Statement attribute_frequency_get;
        SQLite::Statement wmes_constant_frequency_get;
        SQLite::Statement wmes_lti_frequency_get;

        SQLite::Statement act_set;
        SQLite::Statement act_lti_child_ct_set;
        SQLite::Statement act_lti_child_ct_get;
        SQLite::Statement act_lti_set;
        SQLite::Statement act_lti_get;

        SQLite::Statement history_get;
        SQLite::Statement history_push;
        SQLite::Statement history_add;

        SQLite::Statement vis_lti;
        SQLite::Statement vis_lti_act;
        SQLite::Statement vis_value_const;
        SQLite::Statement vis_value_lti;

        smem_statement_container(SMem_Manager* SMem);

        smem_statement_container(smem_statement_container&& other);
        smem_statement_container& operator=(smem_statement_container&& other);
=======
        soar_module::sqlite_statement* begin;
        soar_module::sqlite_statement* commit;
        soar_module::sqlite_statement* rollback;

        soar_module::sqlite_statement* var_get;
        soar_module::sqlite_statement* var_set;
        soar_module::sqlite_statement* var_create;

        soar_module::sqlite_statement* hash_rev_int;
        soar_module::sqlite_statement* hash_rev_float;
        soar_module::sqlite_statement* hash_rev_str;
        soar_module::sqlite_statement* hash_rev_type;
        soar_module::sqlite_statement* hash_get_int;
        soar_module::sqlite_statement* hash_get_float;
        soar_module::sqlite_statement* hash_get_str;
        soar_module::sqlite_statement* hash_add_type;
        soar_module::sqlite_statement* hash_add_int;
        soar_module::sqlite_statement* hash_add_float;
        soar_module::sqlite_statement* hash_add_str;

        soar_module::sqlite_statement* lti_id_exists;
        soar_module::sqlite_statement* lti_id_max  ;
        soar_module::sqlite_statement* lti_add;
        soar_module::sqlite_statement* lti_access_get;
        soar_module::sqlite_statement* lti_access_set;
        soar_module::sqlite_statement* lti_get_t;

        soar_module::sqlite_statement* web_add;
        soar_module::sqlite_statement* web_truncate;
        soar_module::sqlite_statement* web_expand;

        soar_module::sqlite_statement* web_all;
        soar_module::sqlite_statement* web_edge;

        soar_module::sqlite_statement* web_attr_all;
        soar_module::sqlite_statement* web_const_all;
        soar_module::sqlite_statement* web_lti_all;

        soar_module::sqlite_statement* web_attr_all_spread;
        soar_module::sqlite_statement* web_const_all_spread;
        soar_module::sqlite_statement* web_lti_all_spread;

        soar_module::sqlite_statement* web_attr_all_cheap;
        soar_module::sqlite_statement* web_const_all_cheap;
        soar_module::sqlite_statement* web_lti_all_cheap;

        soar_module::sqlite_statement* web_attr_all_manual;
        soar_module::sqlite_statement* web_const_all_manual;
        soar_module::sqlite_statement* web_lti_all_manual;

        soar_module::sqlite_statement* web_attr_child;
        soar_module::sqlite_statement* web_const_child;
        soar_module::sqlite_statement* web_lti_child;

        soar_module::sqlite_statement* attribute_frequency_check;
        soar_module::sqlite_statement* wmes_constant_frequency_check;
        soar_module::sqlite_statement* wmes_lti_frequency_check;

        soar_module::sqlite_statement* attribute_frequency_add;
        soar_module::sqlite_statement* wmes_constant_frequency_add;
        soar_module::sqlite_statement* wmes_lti_frequency_add;

        soar_module::sqlite_statement* attribute_frequency_update;
        soar_module::sqlite_statement* wmes_constant_frequency_update;
        soar_module::sqlite_statement* wmes_lti_frequency_update;

        soar_module::sqlite_statement* attribute_frequency_get;
        soar_module::sqlite_statement* wmes_constant_frequency_get;
        soar_module::sqlite_statement* wmes_lti_frequency_get;

        soar_module::sqlite_statement* act_set;
        soar_module::sqlite_statement* act_lti_child_ct_set;
        soar_module::sqlite_statement* act_lti_child_ct_get;
        soar_module::sqlite_statement* act_lti_child_lti_ct_set;
        soar_module::sqlite_statement* act_lti_child_lti_ct_get;
        soar_module::sqlite_statement* act_lti_set;
        soar_module::sqlite_statement* act_lti_get;

        soar_module::sqlite_statement* act_lti_fake_set;
        soar_module::sqlite_statement* act_lti_fake_insert;
        soar_module::sqlite_statement* act_lti_fake_delete;
        soar_module::sqlite_statement* act_lti_fake_get;

        soar_module::sqlite_statement* history_get;
        soar_module::sqlite_statement* history_push;
        soar_module::sqlite_statement* history_add;
        soar_module::sqlite_statement* prohibit_set;
        soar_module::sqlite_statement* prohibit_add;
        soar_module::sqlite_statement* prohibit_check;
        soar_module::sqlite_statement* prohibit_reset;
        soar_module::sqlite_statement* prohibit_clean;
        soar_module::sqlite_statement* prohibit_remove;
        soar_module::sqlite_statement* history_remove;

        soar_module::sqlite_statement* vis_lti;
        soar_module::sqlite_statement* vis_lti_act;
        soar_module::sqlite_statement* vis_value_const;
        soar_module::sqlite_statement* vis_value_lti;

        //The below sqlite statements are for spreading:
        soar_module::sqlite_statement* web_val_child;
        soar_module::sqlite_statement* web_update_child_edge;
        soar_module::sqlite_statement* web_update_all_lti_child_edges;
        soar_module::sqlite_statement* lti_all;
        soar_module::sqlite_statement* trajectory_add;
        soar_module::sqlite_statement* trajectory_remove;
        soar_module::sqlite_statement* trajectory_remove_lti;
        soar_module::sqlite_statement* trajectory_check_invalid;
        soar_module::sqlite_statement* trajectory_remove_invalid;
        soar_module::sqlite_statement* trajectory_remove_all;
        soar_module::sqlite_statement* trajectory_find_invalid;
        soar_module::sqlite_statement* trajectory_get;
        soar_module::sqlite_statement* trajectory_invalidate_from_lti;
        soar_module::sqlite_statement* trajectory_invalidate_from_lti_add;
        soar_module::sqlite_statement* trajectory_invalidate_from_lti_clear;
        soar_module::sqlite_statement* trajectory_invalidate_from_lti_table;
        soar_module::sqlite_statement* trajectory_invalidate_edge;
        soar_module::sqlite_statement* trajectory_size_debug_cmd;
        soar_module::sqlite_statement* likelihood_cond_count_remove;
        soar_module::sqlite_statement* lti_count_num_appearances_remove;
        soar_module::sqlite_statement* likelihood_cond_count_find;
        soar_module::sqlite_statement* likelihood_cond_count_insert;
        soar_module::sqlite_statement* lti_count_num_appearances_insert;
        soar_module::sqlite_statement* calc_spread;
        soar_module::sqlite_statement* calc_spread_size_debug_cmd;
        soar_module::sqlite_statement* delete_old_context;
        soar_module::sqlite_statement* delete_old_spread;
        soar_module::sqlite_statement* add_new_context;
        soar_module::sqlite_statement* select_fingerprint;
        soar_module::sqlite_statement* add_fingerprint;
        soar_module::sqlite_statement* delete_old_uncommitted_spread;
        soar_module::sqlite_statement* reverse_old_committed_spread;
        soar_module::sqlite_statement* add_uncommitted_fingerprint;
        soar_module::sqlite_statement* remove_fingerprint_reversal;
        soar_module::sqlite_statement* prepare_delete_committed_fingerprint;
        soar_module::sqlite_statement* delete_committed_fingerprint;
        soar_module::sqlite_statement* delete_committed_fingerprint_2;
        soar_module::sqlite_statement* calc_uncommitted_spread;
        soar_module::sqlite_statement* list_uncommitted_spread;
        soar_module::sqlite_statement* delete_commit_of_negative_fingerprint;
        soar_module::sqlite_statement* add_committed_fingerprint;
        soar_module::sqlite_statement* list_current_spread;
        soar_module::sqlite_statement* calc_current_spread;

        smem_statement_container(agent* new_agent);
>>>>>>> origin/new_smem_with_edge_weight_spread

    private:
        void create_tables();
        void create_indices();
        void drop_tables();
};

} // End of namespace SMemExperimental

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_DB_H_ */
