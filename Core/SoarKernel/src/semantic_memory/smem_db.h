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
#include "soar_db.h"

class smem_statement_container: public soar_module::sqlite_statement_container
{
    public:
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

        soar_module::sqlite_statement* lti_add;
        soar_module::sqlite_statement* lti_get;
        soar_module::sqlite_statement* lti_letter_num;
        soar_module::sqlite_statement* lti_max;
        soar_module::sqlite_statement* lti_access_get;
        soar_module::sqlite_statement* lti_access_set;
        soar_module::sqlite_statement* lti_get_t;

        soar_module::sqlite_statement* web_add;
        soar_module::sqlite_statement* web_truncate;
        soar_module::sqlite_statement* web_expand;

        soar_module::sqlite_statement* web_all;

        soar_module::sqlite_statement* web_attr_all;
        soar_module::sqlite_statement* web_const_all;
        soar_module::sqlite_statement* web_lti_all;

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
        soar_module::sqlite_statement* act_lti_set;
        soar_module::sqlite_statement* act_lti_get;

        soar_module::sqlite_statement* history_get;
        soar_module::sqlite_statement* history_push;
        soar_module::sqlite_statement* history_add;

        soar_module::sqlite_statement* vis_lti;
        soar_module::sqlite_statement* vis_lti_act;
        soar_module::sqlite_statement* vis_value_const;
        soar_module::sqlite_statement* vis_value_lti;

        smem_statement_container(agent* new_agent);

    private:

        void create_tables();
        void create_indices();
        void drop_tables(agent* new_agent);
};

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_DB_H_ */
