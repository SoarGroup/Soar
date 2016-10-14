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

namespace SMemExperimental
{

class statement_container
{
protected:
    std::list<std::string> structure;
    SQLite::Database& DB;

public:
    statement_container(SQLite::Database& database, std::function<void()> initializationCode = [](){})
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
            auto s = SQLite::Statement(DB, statement);
            s.exec();
        }
    }
};

class smem_statement_container : public SMemExperimental::statement_container
{
    public:
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

    private:
        void create_tables();
        void create_indices();
        void drop_tables();
};

} // End of namespace SMemExperimental

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_DB_H_ */
