/*
 * smem_db.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_timers.h"
#include "smem_settings.h"
#include "smem_stats.h"

#include "agent.h"
#include "print.h"

void smem_statement_container::create_tables()
{
    add_structure("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    add_structure("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    add_structure("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY, symbol_type INTEGER)");
    add_structure("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY, symbol_value INTEGER)");
    add_structure("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY, symbol_value REAL)");
    add_structure("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY, symbol_value TEXT)");
    add_structure("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY, total_augmentations INTEGER, activation_base_level REAL, activations_total REAL, activations_last INTEGER, activations_first INTEGER, activation_spread REAL, activation_value REAL, lti_augmentations INTEGER)");
    add_structure("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY, t1 INTEGER, t2 INTEGER, t3 INTEGER, t4 INTEGER, t5 INTEGER, t6 INTEGER, t7 INTEGER, t8 INTEGER, t9 INTEGER, t10 INTEGER, touch1 REAL, touch2 REAL, touch3 REAL, touch4 REAL, touch5 REAL, touch6 REAL, touch7 REAL, touch8 REAL, touch9 REAL, touch10 REAL)");
    add_structure("CREATE TABLE smem_augmentations (lti_id INTEGER, attribute_s_id INTEGER, value_constant_s_id INTEGER, value_lti_id INTEGER, activation_value REAL, edge_weight REAL)");
    add_structure("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER, value_constant_s_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER, value_lti_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)");
    add_structure("CREATE TABLE smem_prohibited (lti_id INTEGER PRIMARY KEY, prohibited INTEGER, dirty INTEGER)");
    /* just for spreading: */
    add_structure("CREATE TABLE smem_constants_store (smem_act_max REAL, smem_act_low REAL)");
    add_structure("INSERT OR IGNORE INTO smem_constants_store (smem_act_max, smem_act_low) VALUES (9223372036854775807, -1000000000)");
    add_structure("INSERT OR IGNORE INTO smem_constants_store (smem_act_max, smem_act_low) VALUES (9223372036854775808, -1000000000)");
    add_structure("CREATE TABLE smem_likelihood_trajectories (lti_id INTEGER, lti1 INTEGER, lti2 INTEGER, lti3 INTEGER, lti4 INTEGER, lti5 INTEGER, lti6 INTEGER, lti7 INTEGER, lti8 INTEGER, lti9 INTEGER, lti10 INTEGER, valid_bit INTEGER)");
    add_structure("CREATE TABLE smem_likelihoods (lti_j INTEGER, lti_i INTEGER, num_appearances_i_j REAL, PRIMARY KEY (lti_j, lti_i)) WITHOUT ROWID");
    add_structure("CREATE TABLE smem_trajectory_num (lti_id INTEGER PRIMARY KEY, num_appearances REAL)");
    add_structure("CREATE TABLE smem_current_spread (lti_id INTEGER, num_appearances_i_j REAL, num_appearances REAL, lti_source INTEGER, sign INTEGER, PRIMARY KEY (lti_source, lti_id)) WITHOUT ROWID");
    add_structure("CREATE TABLE smem_uncommitted_spread (lti_id INTEGER, num_appearances_i_j REAL, num_appearances REAL, lti_source INTEGER, sign INTEGER, PRIMARY KEY (lti_source, lti_id)) WITHOUT ROWID");
    add_structure("CREATE TABLE smem_committed_spread (lti_id INTEGER, num_appearances_i_j REAL, num_appearances REAL, lti_source INTEGER, PRIMARY KEY (lti_source, lti_id)) WITHOUT ROWID");
    add_structure("CREATE TABLE smem_current_spread_activations (lti_id INTEGER PRIMARY KEY, activation_base_level REAL, activation_spread REAL, activation_value REAL)");
    add_structure("CREATE TABLE smem_to_delete (lti_id INTEGER PRIMARY KEY)");
    add_structure("CREATE TABLE smem_invalid_parents (lti_id INTEGER PRIMARY KEY) WITHOUT ROWID");
    // adding an ascii table just to make lti queries easier when inspecting database
    {
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (65,'A')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (66,'B')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (67,'C')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (68,'D')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (69,'E')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (70,'F')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (71,'G')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (72,'H')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (73,'I')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (74,'J')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (75,'K')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (76,'L')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (77,'M')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (78,'N')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (79,'O')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (80,'P')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (81,'Q')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (82,'R')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (83,'S')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (84,'T')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (85,'U')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (86,'V')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (87,'W')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (88,'X')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (89,'Y')");
        add_structure("INSERT OR IGNORE INTO smem_ascii (ascii_num, ascii_chr) VALUES (90,'Z')");
    }
}

void smem_statement_container::create_indices()
{
    add_structure("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    add_structure("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    add_structure("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    add_structure("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    add_structure("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, edge_weight)");
    add_structure("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    add_structure("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    add_structure("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    add_structure("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
    /* The indexes below are for spreading. */
    //the index below is used for forward spread.
    add_structure("CREATE INDEX smem_augmentations_parent_val_lti ON smem_augmentations (lti_id, value_constant_s_id, value_lti_id, edge_weight)");
    //scijones - I'm not sure, but the below index is for backwards spread, which is no longer supported.
    //add_structure("CREATE INDEX smem_augmentations_backlink ON smem_augmentations (value_lti_id, value_constant_s_id, lti_id)");
    add_structure("CREATE INDEX trajectory_lti ON smem_likelihood_trajectories (lti_id, valid_bit)");
    add_structure("CREATE INDEX lti_t1 ON smem_likelihood_trajectories (lti_id, lti1) WHERE lti1 != 0");
    add_structure("CREATE INDEX lti_t2 ON smem_likelihood_trajectories (lti1, lti2) WHERE lti2 != 0");
    add_structure("CREATE INDEX lti_t3 ON smem_likelihood_trajectories (lti2, lti3) WHERE lti3 != 0");
    add_structure("CREATE INDEX lti_t4 ON smem_likelihood_trajectories (lti3, lti4) WHERE lti4 != 0");
    add_structure("CREATE INDEX lti_t5 ON smem_likelihood_trajectories (lti4, lti5) WHERE lti5 != 0");
    add_structure("CREATE INDEX lti_t6 ON smem_likelihood_trajectories (lti5, lti6) WHERE lti6 != 0");
    add_structure("CREATE INDEX lti_t7 ON smem_likelihood_trajectories (lti6, lti7) WHERE lti7 != 0");
    add_structure("CREATE INDEX lti_t8 ON smem_likelihood_trajectories (lti7, lti8) WHERE lti8 != 0");
    add_structure("CREATE INDEX lti_t9 ON smem_likelihood_trajectories (lti8, lti9) WHERE lti9 != 0");
    add_structure("CREATE INDEX lti_t10 ON smem_likelihood_trajectories (lti9, lti10) WHERE lti10 != 0");
    add_structure("CREATE INDEX lti_t12 ON smem_likelihood_trajectories (lti_id, lti1) WHERE lti2=0");
    add_structure("CREATE INDEX lti_t23 ON smem_likelihood_trajectories (lti_id, lti2) WHERE lti3=0");
    add_structure("CREATE INDEX lti_t34 ON smem_likelihood_trajectories (lti_id, lti3) WHERE lti4=0");
    add_structure("CREATE INDEX lti_t45 ON smem_likelihood_trajectories (lti_id, lti4) WHERE lti5=0");
    add_structure("CREATE INDEX lti_t56 ON smem_likelihood_trajectories (lti_id, lti5) WHERE lti6=0");
    add_structure("CREATE INDEX lti_t67 ON smem_likelihood_trajectories (lti_id, lti6) WHERE lti7=0");
    add_structure("CREATE INDEX lti_t78 ON smem_likelihood_trajectories (lti_id, lti7) WHERE lti8=0");
    add_structure("CREATE INDEX lti_t89 ON smem_likelihood_trajectories (lti_id, lti8) WHERE lti9=0");
    add_structure("CREATE INDEX lti_t910 ON smem_likelihood_trajectories (lti_id, lti9) WHERE lti10=0");
    add_structure("CREATE INDEX lti_tid10 ON smem_likelihood_trajectories (lti_id, lti10) WHERE lti10!=0");
    add_structure("CREATE INDEX lti_source ON smem_uncommitted_spread (lti_source)");
}

void smem_statement_container::drop_tables(agent* new_agent)
{
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_persistent_variables");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_symbols_type");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_symbols_integer");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_symbols_float");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_symbols_string");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_lti");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_activation_history");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_augmentations");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_attribute_frequency");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_wmes_constant_frequency");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_wmes_lti_frequency");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_ascii");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_prohibited");

    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_likelihood_trajectories");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_likelihoods");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_current_spread");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_trajectory_num");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_current_context");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_committed_spread");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_uncommitted_spread");
    new_agent->SMem->DB->sql_execute("DROP TABLE IF EXISTS smem_current_spread_activations");
}

smem_statement_container::smem_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->SMem->DB)
{
    soar_module::sqlite_database* new_db = new_agent->SMem->DB;

    // Delete all entries from the tables in the database if append setting is off
    if (new_agent->SMem->settings->append_db->get_value() == off)
    {
        print_sysparam_trace(new_agent, 0, "Erasing contents of semantic memory database. (append = off)\n");
        drop_tables(new_agent);
    }

    create_tables();
    create_indices();

    // Update the version number
    add_structure("REPLACE INTO versions (system, version_number) VALUES ('smem_schema'," SMEM_SCHEMA_VERSION ")");

    begin = new soar_module::sqlite_statement(new_db, "BEGIN");
    add(begin);

    commit = new soar_module::sqlite_statement(new_db, "COMMIT");
    add(commit);

    rollback = new soar_module::sqlite_statement(new_db, "ROLLBACK");
    add(rollback);

    //

    var_get = new soar_module::sqlite_statement(new_db, "SELECT variable_value FROM smem_persistent_variables WHERE variable_id=?");
    add(var_get);

    var_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_persistent_variables SET variable_value=? WHERE variable_id=?");
    add(var_set);

    var_create = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_persistent_variables (variable_id,variable_value) VALUES (?,?)");
    add(var_create);

    //

    hash_rev_int = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM smem_symbols_integer WHERE s_id=?");
    add(hash_rev_int);

    hash_rev_float = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM smem_symbols_float WHERE s_id=?");
    add(hash_rev_float);

    hash_rev_str = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM smem_symbols_string WHERE s_id=?");
    add(hash_rev_str);

    hash_rev_type = new soar_module::sqlite_statement(new_db, "SELECT symbol_type FROM smem_symbols_type WHERE s_id=?");
    add(hash_rev_type);

    hash_get_int = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM smem_symbols_integer WHERE symbol_value=?");
    add(hash_get_int);

    hash_get_float = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM smem_symbols_float WHERE symbol_value=?");
    add(hash_get_float);

    hash_get_str = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM smem_symbols_string WHERE symbol_value=?");
    add(hash_get_str);

    hash_add_type = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_symbols_type (symbol_type) VALUES (?)");
    add(hash_add_type);

    hash_add_int = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_symbols_integer (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_int);

    hash_add_float = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_symbols_float (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_float);

    hash_add_str = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_symbols_string (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_str);

    //

    lti_id_exists = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_lti WHERE lti_id=?");
    add(lti_id_exists);

    lti_id_max = new soar_module::sqlite_statement(new_db, "SELECT MAX(lti_id) FROM smem_lti");
    add(lti_id_max);

    lti_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_lti (lti_id, total_augmentations,activation_base_level,activations_total,activations_last,activations_first,activation_spread,activation_value,lti_augmentations) VALUES (?,?,?,?,?,?,?,?,?)");
    add(lti_add);

    lti_access_get = new soar_module::sqlite_statement(new_db, "SELECT activations_total, activations_last, activations_first FROM smem_lti WHERE lti_id=?");
    add(lti_access_get);

    lti_access_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET activations_total=?, activations_last=?, activations_first=? WHERE lti_id=?");
    add(lti_access_set);

    lti_get_t = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_lti WHERE activations_last=?");
    add(lti_get_t);

    //

    web_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value, edge_weight) VALUES (?,?,?,?,?,?)");
    add(web_add);

    web_truncate = new soar_module::sqlite_statement(new_db, "DELETE FROM smem_augmentations WHERE lti_id=?");
    add(web_truncate);

    web_expand = new soar_module::sqlite_statement(new_db, "SELECT tsh_a.symbol_type AS attr_type, tsh_a.s_id AS attr_hash, vcl.symbol_type AS value_type, vcl.s_id AS value_hash, vcl.value_lti_id AS value_lti FROM ((smem_augmentations w LEFT JOIN smem_symbols_type tsh_v ON w.value_constant_s_id=tsh_v.s_id) vc LEFT JOIN smem_lti AS lti ON vc.value_lti_id=lti.lti_id) vcl INNER JOIN smem_symbols_type tsh_a ON vcl.attribute_s_id=tsh_a.s_id WHERE vcl.lti_id=?");
    add(web_expand);

    //

    web_all = new soar_module::sqlite_statement(new_db, "SELECT attribute_s_id, value_constant_s_id, value_lti_id FROM smem_augmentations WHERE lti_id=?");
    add(web_all);

    web_edge = new soar_module::sqlite_statement(new_db, "SELECT value_lti_id, edge_weight FROM smem_augmentations WHERE lti_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR "");
    add(web_edge);

    // /* The following indexes can be postpended with ", lti_id ASC" (without quotes) for debugging. */

    web_attr_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? ORDER BY activation_value DESC");
    add(web_attr_all);

    web_const_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=? AND value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR " ORDER BY activation_value DESC");
    add(web_const_all);

    web_lti_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=? ORDER BY activation_value DESC");
    add(web_lti_all);

    //

    web_attr_all_no_spread = new soar_module::sqlite_statement(new_db, "SELECT w.lti_id, w.activation_value FROM smem_augmentations w LEFT OUTER JOIN smem_current_spread_activations h ON w.lti_id = h.lti_id WHERE h.lti_id IS NULL AND w.attribute_s_id=? ORDER BY w.activation_value DESC");
    add(web_attr_all_no_spread);

    web_const_all_no_spread = new soar_module::sqlite_statement(new_db, "SELECT w.lti_id, w.activation_value FROM smem_augmentations w LEFT OUTER JOIN smem_current_spread_activations h ON w.lti_id = h.lti_id WHERE h.lti_id IS NULL AND w.attribute_s_id=? AND w.value_constant_s_id=? AND w.value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR " ORDER BY w.activation_value DESC");
    add(web_const_all_no_spread);

    web_lti_all_no_spread = new soar_module::sqlite_statement(new_db, "SELECT w.lti_id, w.activation_value FROM smem_augmentations w LEFT OUTER JOIN smem_current_spread_activations h ON w.lti_id = h.lti_id  WHERE h.lti_id IS NULL AND w.attribute_s_id=? AND w.value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND w.value_lti_id=? ORDER BY w.activation_value DESC");
    add(web_lti_all_no_spread);

    //

    web_attr_all_spread = new soar_module::sqlite_statement(new_db, "SELECT smem_augmentations.lti_id, smem_current_spread_activations.activation_value FROM smem_augmentations INNER JOIN smem_current_spread_activations ON smem_augmentations.lti_id = smem_current_spread_activations.lti_id WHERE attribute_s_id=?");
    add(web_attr_all_spread);

    web_const_all_spread = new soar_module::sqlite_statement(new_db, "SELECT smem_augmentations.lti_id, smem_current_spread_activations.activation_value FROM smem_augmentations INNER JOIN smem_current_spread_activations ON smem_augmentations.lti_id = smem_current_spread_activations.lti_id WHERE attribute_s_id=? AND value_constant_s_id=? AND value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR "");
    add(web_const_all_spread);

    web_lti_all_spread = new soar_module::sqlite_statement(new_db, "SELECT smem_augmentations.lti_id, smem_current_spread_activations.activation_value FROM smem_augmentations INNER JOIN smem_current_spread_activations ON smem_augmentations.lti_id = smem_current_spread_activations.lti_id WHERE attribute_s_id=? AND value_constant_s_id = " SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id = ?");
    add(web_lti_all_spread);

    web_attr_all_cheap = new soar_module::sqlite_statement(new_db, "SELECT lti_id AS lti_id1, activation_value AS activation_value_aug FROM smem_augmentations WHERE attribute_s_id=? ORDER BY activation_value_aug DESC");
    add(web_attr_all_cheap);

    web_const_all_cheap = new soar_module::sqlite_statement(new_db, "SELECT lti_id AS lti_id1, activation_value AS activation_value_aug FROM smem_augmentations WHERE attribute_s_id=? AND value_constant_s_id=? AND value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR " ORDER BY activation_value_aug DESC");
    add(web_const_all_cheap);

    web_lti_all_cheap = new soar_module::sqlite_statement(new_db, "SELECT lti_id AS lti_id1, activation_value AS activation_value_aug FROM smem_augmentations WHERE attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id = ? ORDER BY activation_value_aug DESC");
    add(web_lti_all_cheap);

    web_attr_all_manual = new soar_module::sqlite_statement(new_db, "SELECT 1 FROM smem_augmentations WHERE attribute_s_id=? AND lti_id=?");
    add(web_attr_all_manual);

    web_const_all_manual = new soar_module::sqlite_statement(new_db, "SELECT 1 FROM smem_augmentations WHERE attribute_s_id=? AND lti_id=? AND value_constant_s_id=? AND value_lti_id = " SMEM_AUGMENTATIONS_NULL_STR "");
    add(web_const_all_manual);

    web_lti_all_manual = new soar_module::sqlite_statement(new_db, "SELECT 1 FROM smem_augmentations WHERE attribute_s_id=? AND lti_id=? AND value_constant_s_id = " SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=?");
    add(web_lti_all_manual);

    //

    web_attr_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=?");
    add(web_attr_child);

    web_const_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=?");
    add(web_const_child);

    web_lti_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=?");
    add(web_lti_child);

    //

    web_val_child = new soar_module::sqlite_statement(new_db, "SELECT value_lti_id, edge_weight FROM smem_augmentations WHERE lti_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR "");
    add(web_val_child);

    //

    web_update_child_edge = new soar_module::sqlite_statement(new_db, "UPDATE smem_augmentations SET edge_weight = ? WHERE lti_id = ? AND value_constant_s_id = " SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id = ?");
    add(web_update_child_edge);

    web_update_all_lti_child_edges = new soar_module::sqlite_statement(new_db, "UPDATE smem_augmentations SET edge_weight = ? WHERE lti_id = ? AND value_constant_s_id = " SMEM_AUGMENTATIONS_NULL_STR "");
    add(web_update_all_lti_child_edges);

    //

    attribute_frequency_check = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_attribute_frequency WHERE attribute_s_id=?");
    add(attribute_frequency_check);

    wmes_constant_frequency_check = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_wmes_constant_frequency WHERE attribute_s_id=? AND value_constant_s_id=?");
    add(wmes_constant_frequency_check);

    wmes_lti_frequency_check = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_wmes_lti_frequency WHERE attribute_s_id=? AND value_lti_id=?");
    add(wmes_lti_frequency_check);

    //

    attribute_frequency_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) VALUES (?,1)");
    add(attribute_frequency_add);

    wmes_constant_frequency_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) VALUES (?,?,1)");
    add(wmes_constant_frequency_add);

    wmes_lti_frequency_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) VALUES (?,?,1)");
    add(wmes_lti_frequency_add);

    //

    attribute_frequency_update = new soar_module::sqlite_statement(new_db, "UPDATE smem_attribute_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=?");
    add(attribute_frequency_update);

    wmes_constant_frequency_update = new soar_module::sqlite_statement(new_db, "UPDATE smem_wmes_constant_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=? AND value_constant_s_id=?");
    add(wmes_constant_frequency_update);

    wmes_lti_frequency_update = new soar_module::sqlite_statement(new_db, "UPDATE smem_wmes_lti_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=? AND value_lti_id=?");
    add(wmes_lti_frequency_update);

    //

    attribute_frequency_get = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_attribute_frequency WHERE attribute_s_id=?");
    add(attribute_frequency_get);

    wmes_constant_frequency_get = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_wmes_constant_frequency WHERE attribute_s_id=? AND value_constant_s_id=?");
    add(wmes_constant_frequency_get);

    wmes_lti_frequency_get = new soar_module::sqlite_statement(new_db, "SELECT edge_frequency FROM smem_wmes_lti_frequency WHERE attribute_s_id=? AND value_lti_id=?");
    add(wmes_lti_frequency_get);

    //

    act_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_augmentations SET activation_value=? WHERE lti_id=?");
    add(act_set);

    act_lti_child_ct_get = new soar_module::sqlite_statement(new_db, "SELECT total_augmentations FROM smem_lti WHERE lti_id=?");
    add(act_lti_child_ct_get);

    act_lti_child_ct_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET total_augmentations=? WHERE lti_id=?");
    add(act_lti_child_ct_set);

    act_lti_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET activation_base_level = ?, activation_spread = ?, activation_value=? WHERE lti_id=?");
    add(act_lti_set);

    act_lti_get = new soar_module::sqlite_statement(new_db, "SELECT activation_base_level, activation_spread, activation_value FROM smem_lti WHERE lti_id=?");
    add(act_lti_get);

    history_get = new soar_module::sqlite_statement(new_db, "SELECT t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,touch1,touch2,touch3,touch4,touch5,touch6,touch7,touch8,touch9,touch10 FROM smem_activation_history WHERE lti_id=?");
    add(history_get);

    history_push = new soar_module::sqlite_statement(new_db, "UPDATE smem_activation_history SET t10=t9,t9=t8,t8=t7,t8=t7,t7=t6,t6=t5,t5=t4,t4=t3,t3=t2,t2=t1,t1=?,touch10=touch9,touch9=touch8,touch8=touch7,touch7=touch6,touch6=touch5,touch5=touch4,touch4=touch3,touch3=touch2,touch2=touch1,touch1=? WHERE lti_id=?");
    add(history_push);

    history_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_activation_history (lti_id,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,touch1,touch2,touch3,touch4,touch5,touch6,touch7,touch8,touch9,touch10) VALUES (?,?,0,0,0,0,0,0,0,0,0,?,0,0,0,0,0,0,0,0,0)");
    add(history_add);

    // Adding statements needed to support prohibits.

    prohibit_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_prohibited SET prohibited=1,dirty=1 WHERE lti_id=?");
    add(prohibit_set);

    prohibit_add = new soar_module::sqlite_statement(new_db, "INSERT OR IGNORE INTO smem_prohibited (lti_id,prohibited,dirty) VALUES (?,0,0)");
    add(prohibit_add);

    prohibit_check = new soar_module::sqlite_statement(new_db, "SELECT lti_id,dirty FROM smem_prohibited WHERE lti_id=? AND prohibited=1");
    add(prohibit_check);

    prohibit_reset = new soar_module::sqlite_statement(new_db, "UPDATE smem_prohibited SET prohibited=0,dirty=0 WHERE lti_id=?");
    add(prohibit_reset);

    prohibit_clean = new soar_module::sqlite_statement(new_db, "UPDATE smem_prohibited SET prohibited=1,dirty=0 WHERE lti_id=?");
    add(prohibit_clean);

    prohibit_remove = new soar_module::sqlite_statement(new_db, "DELETE FROM smem_prohibited WHERE lti_id=?");
    add(prohibit_remove);

    history_remove = new soar_module::sqlite_statement(new_db, "UPDATE smem_activation_history SET t1=t2,t2=t3,t3=t4,t4=t5,t5=t6,t6=t7,t7=t8,t8=t9,t9=t10,t10=0,touch1=touch2,touch2=touch3,touch3=touch4,touch4=touch5,touch5=touch6,touch6=touch7,touch7=touch8,touch8=touch9,touch9=touch10,touch10=0 WHERE lti_id=?"); //add something like "only use 9/10 when prohibited"
    add(history_remove);

    //

    act_lti_child_lti_ct_get = new soar_module::sqlite_statement(new_db, "SELECT lti_augmentations FROM smem_ltI WHERE lti_id = ?");
    add(act_lti_child_lti_ct_get);

    act_lti_child_lti_ct_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET lti_augmentations = ? WHERE lti_id = ?");
    add(act_lti_child_lti_ct_set);

    act_lti_fake_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_current_spread_activations SET activation_base_level = ?, activation_spread = ?, activation_value = ? WHERE lti_id = ?");
    add(act_lti_fake_set);

    act_lti_fake_insert = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_current_spread_activations (lti_id, activation_base_level, activation_spread, activation_value) VALUES (?,?,?,?)");
    add(act_lti_fake_insert);

    act_lti_fake_delete = new soar_module::sqlite_statement(new_db, "DELETE FROM smem_current_spread_activations WHERE lti_id = ?");
    add(act_lti_fake_delete);

    act_lti_fake_get = new soar_module::sqlite_statement(new_db, "SELECT activation_base_level, activation_spread, activation_value FROM smem_current_spread_activations WHERE lti_id = ?");
    add(act_lti_fake_get);

    //

    vis_lti = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_lti ORDER BY lti_id ASC");
    add(vis_lti);

    vis_lti_act = new soar_module::sqlite_statement(new_db, "SELECT activation_value FROM smem_lti WHERE lti_id=?");
    add(vis_lti_act);

    vis_value_const = new soar_module::sqlite_statement(new_db, "SELECT lti_id, tsh1.symbol_type AS attr_type, tsh1.s_id AS attr_hash, tsh2.symbol_type AS val_type, tsh2.s_id AS val_hash FROM smem_augmentations w, smem_symbols_type tsh1, smem_symbols_type tsh2 WHERE (w.attribute_s_id=tsh1.s_id) AND (w.value_constant_s_id=tsh2.s_id)");
    add(vis_value_const);

    vis_value_lti = new soar_module::sqlite_statement(new_db, "SELECT lti_id, tsh.symbol_type AS attr_type, tsh.s_id AS attr_hash, value_lti_id FROM smem_augmentations w, smem_symbols_type tsh WHERE (w.attribute_s_id=tsh.s_id) AND (value_lti_id<>" SMEM_AUGMENTATIONS_NULL_STR ")");
    add(vis_value_lti);

    //This was for spreading (batch processing/initialization), but it just iterates over all ltis.
    // I should perhaps change to iterate based on the ordering in smem_augmentations, but if it isn't broke...
    lti_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_lti");
    add(lti_all);

        //adding trajectory into fingerprint. Assume we do not insert invalid trajectories.
    trajectory_add = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_likelihood_trajectories (lti_id, lti1, lti2, lti3, lti4, lti5, lti6, lti7, lti8, lti9, lti10, valid_bit) VALUES (?,?,?,?,?,?,?,?,?,?,?,1)");
    add(trajectory_add);

    //Removing trajectories for a particular lti. Assume we do not remove valid trajectories.
    trajectory_remove = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_likelihood_trajectories WHERE lti_id=? AND valid_bit=0");
    add(trajectory_remove);

    //Removing trajectories for a particular lti.
    trajectory_remove_lti = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_likelihood_trajectories WHERE lti_id=?");
    add(trajectory_remove_lti);

    //like trajectory_get, but with invalid instead of valid.
    trajectory_check_invalid = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_likelihood_trajectories WHERE lti_id=? AND valid_bit=0");
    add(trajectory_check_invalid);

    //Removing all invalid trajectories.
    trajectory_remove_invalid = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_likelihood_trajectories WHERE valid_bit=0");
    add(trajectory_remove_invalid);

    //Removing all trajectories from ltis with invalid trajectories.
    trajectory_remove_all = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_likelihood_trajectories WHERE lti_id IN (SELECT lti_id FROM smem_likelihood_trajectories WHERE valid_bit=0)");
    add(trajectory_remove_all);//"DELETE a.* FROM smem_likelihood_trajectories AS a INNER JOIN smem_likelihood_trajectories AS b on a.lti_id = b.lti_id WHERE b.valid_bit=0"
//"DELETE FROM smem_likelihood_trajectories WHERE lti_id IN (SELECT DISTINCT lti_id FROM smem_likelihood_trajectories WHERE valid_bit=0)"
    //"DELETE FROM smem_likelihood_trajectories WHERE EXISTS (SELECT * FROM smem_likelihood_trajectories AS b WHERE b.lti_id = smem_likelihood_trajectories.lti_id AND b.valid_bit = 0)"

    //Find all of the ltis with invalid trajectories and find how many new ones they need.
    trajectory_find_invalid = new soar_module::sqlite_statement(new_db, "SELECT lti_id, COUNT(*) FROM smem_likelihood_trajectories WHERE valid_bit=0 GROUP BY lti_id");
    add(trajectory_find_invalid);

    //getting trajectory from fingerprint. Only retrieves ones with valid bit of 1.
    trajectory_get = new soar_module::sqlite_statement(new_db, "SELECT lti1, lti2, lti3, lti4, lti5, lti6, lti7, lti8, lti9, lti10 FROM smem_likelihood_trajectories WHERE lti_id=? AND valid_bit=1");
    add(trajectory_get);

    //invalidating trajectories containing some lti and don't have null afterwards
    trajectory_invalidate_from_lti = new soar_module::sqlite_statement(new_db,"UPDATE smem_likelihood_trajectories SET valid_bit=0 WHERE (lti_id=? AND lti1!=0) OR (lti1=? AND lti2!=0) OR (lti2=? AND lti3!=0) OR (lti3=? AND lti4!=0) OR (lti4=? AND lti5!=0) OR (lti5=? AND lti6!=0) OR (lti6=? AND lti7!=0) OR (lti7=? AND lti8!=0) OR (lti8=? AND lti9!=0) OR (lti9=? AND lti10!=0)");
    add(trajectory_invalidate_from_lti);

    trajectory_invalidate_from_lti_add = new soar_module::sqlite_statement(new_db,"INSERT OR IGNORE INTO smem_invalid_parents (lti_id) VALUES (?)");
    add(trajectory_invalidate_from_lti_add);

    trajectory_invalidation_check_for_rows = new soar_module::sqlite_statement(new_db,"SELECT lti_id FROM smem_invalid_parents");
    add(trajectory_invalidation_check_for_rows);

    trajectory_invalidate_from_lti_clear = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_invalid_parents");
    add(trajectory_invalidate_from_lti_clear);

    trajectory_invalidate_from_lti_table = new soar_module::sqlite_statement(new_db,"UPDATE smem_likelihood_trajectories SET valid_bit=0 WHERE rowid in (SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON smem_likelihood_trajectories.lti_id=smem_invalid_parents.lti_id WHERE lti1!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti1=smem_invalid_parents.lti_id WHERE lti2!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti2=smem_invalid_parents.lti_id WHERE lti3!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti3=smem_invalid_parents.lti_id WHERE lti4!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti4=smem_invalid_parents.lti_id WHERE lti5!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti5=smem_invalid_parents.lti_id WHERE lti6!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti6=smem_invalid_parents.lti_id WHERE lti7!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti7=smem_invalid_parents.lti_id WHERE lti8!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti8=smem_invalid_parents.lti_id WHERE lti9!=0 UNION SELECT smem_likelihood_trajectories.rowid FROM smem_likelihood_trajectories INNER JOIN smem_invalid_parents ON lti9=smem_invalid_parents.lti_id WHERE lti10!=0)");
    add(trajectory_invalidate_from_lti_table);

    //invalidating trajectories containing some lti followed by a particular different lti
    trajectory_invalidate_edge = new soar_module::sqlite_statement(new_db,"UPDATE smem_likelihood_trajectories SET valid_bit=0 WHERE (lti_id=? AND lti1=? AND lti1!=0) OR (lti1=? AND lti2=? AND lti2!=0) OR (lti2=? AND lti3=? AND lti3!=0) OR (lti3=? AND lti4=? AND lti4!=0) OR (lti4=? AND lti5=? AND lti5!=0) OR (lti5=? AND lti6=? AND lti6!=0) OR (lti6=? AND lti7=? AND lti7!=0) OR (lti7=? AND lti8=? AND lti8!=0) OR (lti8=? AND lti9=? AND lti9!=0) OR (lti9=? AND lti10=? AND lti10!=0)");
    add(trajectory_invalidate_edge);

    //gets the size of the current fingerprint table.
    trajectory_size_debug_cmd = new soar_module::sqlite_statement(new_db,"SELECT COUNT(*) FROM smem_likelihood_trajectories WHERE lti1!=0");
    add(trajectory_size_debug_cmd);


    //

    //take away spread precalculated values for some lti
    likelihood_cond_count_remove = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_likelihoods WHERE lti_j=?");
    add(likelihood_cond_count_remove);

    //take away other spread precalculated values for some lti
    lti_count_num_appearances_remove = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_trajectory_num WHERE lti_id=?");
    add(lti_count_num_appearances_remove);

    //add spread precalculated values for some lti
    //likelihood_cond_count_insert = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_likelihoods (lti_j, lti_i, num_appearances_i_j) SELECT parent, lti, SUM(count) FROM (SELECT lti_id AS parent, lti1 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti1 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti2 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti2 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti3 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti3 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti4 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti4 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti5 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti5 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti6 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti6 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti7 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti7 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti8 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti8 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti9 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti9 !=0 AND lti_id=? GROUP BY lti, parent UNION ALL SELECT lti_id AS parent, lti10 AS lti,COUNT(*) AS count FROM smem_likelihood_trajectories WHERE lti10 !=0 AND lti_id=? GROUP BY lti, parent) GROUP BY parent, lti");
    //add(likelihood_cond_count_insert);

    likelihood_cond_count_find = new soar_module::sqlite_statement(new_db,"SELECT lti_id AS parent, lti1 AS lti,1 AS depth FROM smem_likelihood_trajectories WHERE lti1 !=0 AND lti2 = 0 AND lti_id=? UNION ALL SELECT lti1 AS parent, lti2 AS lti,2 AS depth FROM smem_likelihood_trajectories WHERE lti2 !=0 AND lti3 = 0 AND lti_id=? UNION ALL SELECT lti2 AS parent, lti3 AS lti,3 AS depth FROM smem_likelihood_trajectories WHERE lti3 !=0 AND lti4 = 0 AND lti_id=? UNION ALL SELECT lti3 AS parent, lti4 AS lti,4 AS depth FROM smem_likelihood_trajectories WHERE lti4 !=0 AND lti5 = 0 AND lti_id=? UNION ALL SELECT lti4 AS parent, lti5 AS lti,5 AS depth FROM smem_likelihood_trajectories WHERE lti5 !=0 AND lti6 = 0 AND lti_id=? UNION ALL SELECT lti5 AS parent, lti6 AS lti,6 AS depth FROM smem_likelihood_trajectories WHERE lti6 !=0 AND lti7 = 0 AND lti_id=? UNION ALL SELECT lti6 AS parent, lti7 AS lti,7 AS depth FROM smem_likelihood_trajectories WHERE lti7 !=0 AND lti8 = 0 AND lti_id=? UNION ALL SELECT lti7 AS parent, lti8 AS lti,8 AS depth FROM smem_likelihood_trajectories WHERE lti8 !=0 AND lti9 = 0 AND lti_id=? UNION ALL SELECT lti8 AS parent, lti9 AS lti,9 AS depth FROM smem_likelihood_trajectories WHERE lti9 !=0 AND lti10 = 0 AND lti_id=? UNION ALL SELECT lti9 AS parent, lti10 AS lti,10 AS depth FROM smem_likelihood_trajectories WHERE lti10 !=0 AND lti_id=? ORDER BY 3 ASC, lti DESC");
    add(likelihood_cond_count_find);

    likelihood_cond_count_insert = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_likelihoods (lti_j,lti_i,num_appearances_i_j) VALUES (?,?,?)");
    add(likelihood_cond_count_insert);

    //add other spread precalculated values for some lti
    lti_count_num_appearances_insert = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_trajectory_num (lti_id, num_appearances) SELECT lti_j, SUM(num_appearances_i_j) FROM smem_likelihoods WHERE lti_j=? GROUP BY lti_j");
    add(lti_count_num_appearances_insert);

    //gets the relevant info from currently relevant ltis
    //calc_spread = new soar_module::sqlite_statement(new_db,"SELECT lti_id,num_appearances,num_appearances_i_j FROM smem_current_spread WHERE lti_source = ?");
    //add(calc_spread);

    //gets the relevant info from currently relevant ltis
    calc_uncommitted_spread = new soar_module::sqlite_statement(new_db,"SELECT lti_id,num_appearances,num_appearances_i_j,sign,lti_source FROM smem_uncommitted_spread WHERE lti_id = ?");
    add(calc_uncommitted_spread);

    calc_current_spread = new soar_module::sqlite_statement(new_db,"SELECT lti_id,num_appearances,num_appearances_i_j,sign,lti_source FROM smem_current_spread WHERE lti_id = ?");
    add(calc_current_spread);

    list_uncommitted_spread = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_uncommitted_spread");
    add(list_uncommitted_spread);

    //gets the size of the current spread table.
    calc_spread_size_debug_cmd = new soar_module::sqlite_statement(new_db,"SELECT COUNT(*) FROM smem_committed_spread");
    add(calc_spread_size_debug_cmd);

    //delete lti from context table
    //delete_old_context = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_current_context WHERE lti_id=?");
    //add(delete_old_context);

    //delete lti's info from current spread table
    delete_old_spread = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_current_spread WHERE lti_source=?");
    add(delete_old_spread);

    list_current_spread = new soar_module::sqlite_statement(new_db,"SELECT lti_id from smem_current_spread");
    add(list_current_spread);

    //When spread is still uncommitted, just remove. when it is committed, mark row as negative.
    //This should be called alongside reverse_old_committed_spread
    delete_old_uncommitted_spread = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_uncommitted_spread WHERE lti_source=? AND lti_id NOT IN (SELECT lti_id FROM smem_committed_spread WHERE lti_source=?)");
    add(delete_old_uncommitted_spread);

    //When spread is committed but needs removal, add a negative row for later processing.
    //This needs to be called before delete_old_spread and for the same value as delete_old_spread's delete.
    reverse_old_committed_spread = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_uncommitted_spread(lti_id,num_appearances_i_j,num_appearances,lti_source,sign) SELECT lti_id,num_appearances_i_j,num_appearances,lti_source,0 FROM smem_committed_spread WHERE lti_source=?");//
    add(reverse_old_committed_spread);

    //add lti to the context table
    //add_new_context = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_current_context (lti_id) VALUES (?)");
    //add(add_new_context);

    //add a fingerprint's information to the current spread table.
    select_fingerprint = new soar_module::sqlite_statement(new_db,"SELECT lti_i,num_appearances_i_j,num_appearances,1,lti_j FROM smem_likelihoods INNER JOIN smem_trajectory_num ON lti_id=lti_j WHERE lti_j=?");
    add(select_fingerprint);
    add_fingerprint = new soar_module::sqlite_statement(new_db,"INSERT or ignore INTO smem_current_spread(lti_id,num_appearances_i_j,num_appearances,sign,lti_source) VALUES (?,?,?,?,?)");
    add(add_fingerprint);

    //add a fingerprint's information to the current uncommitted spread table. should happen after add_fingerprint
    add_uncommitted_fingerprint = new soar_module::sqlite_statement(new_db,"INSERT OR IGNORE INTO smem_uncommitted_spread SELECT lti_id,num_appearances_i_j,num_appearances,lti_source,1 FROM smem_current_spread WHERE lti_source=?");
//    add_uncommitted_fingerprint = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_uncommitted_spread(lti_id,num_appearances_i_j,num_appearances,lti_source,sign) SELECT lti_i,num_appearances_i_j,num_appearances,lti_j,1 FROM (SELECT * FROM smem_likelihoods WHERE lti_j=?) INNER JOIN smem_trajectory_num ON lti_id=lti_j");
    add(add_uncommitted_fingerprint);

    remove_fingerprint_reversal = new soar_module::sqlite_statement(new_db, "DELETE FROM smem_uncommitted_spread WHERE lti_source=? AND lti_id IN (SELECT lti_id FROM smem_committed_spread WHERE lti_source=?)");
    add(remove_fingerprint_reversal);

    prepare_delete_committed_fingerprint = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_to_delete (lti_id) VALUES (?)");
    add(prepare_delete_committed_fingerprint);

    //remove a fingerprint's information from the current uncommitted spread table.(has been processed)
    delete_committed_fingerprint = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_uncommitted_spread WHERE lti_id IN (SELECT lti_id FROM smem_to_delete)");
    add(delete_committed_fingerprint);
    delete_committed_fingerprint_2 = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_to_delete");
    add(delete_committed_fingerprint_2);

    delete_commit_of_negative_fingerprint = new soar_module::sqlite_statement(new_db,"DELETE FROM smem_committed_spread WHERE lti_id=? AND lti_source=?");
    add(delete_commit_of_negative_fingerprint);

    add_committed_fingerprint = new soar_module::sqlite_statement(new_db,"INSERT INTO smem_committed_spread (lti_id,num_appearances_i_j,num_appearances,lti_source) VALUES (?,?,?,?)");
    add(add_committed_fingerprint);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Temporal Hash Functions (smem::hash)
//
// The rete has symbol hashing, but the values are
// reliable only for the lifetime of a symbol.  This
// isn't good for SMem.  Hence, we implement a simple
// lookup table.
//
// Note the hashing functions for the symbol types are
// very similar, but with enough differences that I
// separated them out for clarity.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_hash_id SMem_Manager::hash_add_type(byte symbol_type)
{
    SQL->hash_add_type->bind_int(1, symbol_type);
    SQL->hash_add_type->execute(soar_module::op_reinit);
    return static_cast<smem_hash_id>(DB->last_insert_rowid());
}

smem_hash_id SMem_Manager::hash_int(int64_t val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    SQL->hash_get_int->bind_int(1, val);
    if (SQL->hash_get_int->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(SQL->hash_get_int->column_int(0));
    }
    SQL->hash_get_int->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = hash_add_type(INT_CONSTANT_SYMBOL_TYPE);

        // then content
        SQL->hash_add_int->bind_int(1, return_val);
        SQL->hash_add_int->bind_int(2, val);
        SQL->hash_add_int->execute(soar_module::op_reinit);
    }

    return return_val;
}

smem_hash_id SMem_Manager::hash_float(double val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    SQL->hash_get_float->bind_double(1, val);
    if (SQL->hash_get_float->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(SQL->hash_get_float->column_int(0));
    }
    SQL->hash_get_float->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = hash_add_type(FLOAT_CONSTANT_SYMBOL_TYPE);

        // then content
        SQL->hash_add_float->bind_int(1, return_val);
        SQL->hash_add_float->bind_double(2, val);
        SQL->hash_add_float->execute(soar_module::op_reinit);
    }

    return return_val;
}

smem_hash_id SMem_Manager::hash_str(char* val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    SQL->hash_get_str->bind_text(1, static_cast<const char*>(val));
    if (SQL->hash_get_str->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(SQL->hash_get_str->column_int(0));
    }
    SQL->hash_get_str->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = hash_add_type(STR_CONSTANT_SYMBOL_TYPE);

        // then content
        SQL->hash_add_str->bind_int(1, return_val);
        SQL->hash_add_str->bind_text(2, static_cast<const char*>(val));
        SQL->hash_add_str->execute(soar_module::op_reinit);
    }

    return return_val;
}

// returns a temporally unique integer representing a symbol constant
smem_hash_id SMem_Manager::hash(Symbol* sym, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    ////////////////////////////////////////////////////////////////////////////
    timers->hash->start();
    ////////////////////////////////////////////////////////////////////////////

    if (sym->is_constant())
    {
        if ((!sym->smem_hash) || (sym->smem_valid != smem_validation))
        {
            sym->smem_hash = NIL;
            sym->smem_valid = smem_validation;

            switch (sym->symbol_type)
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                    return_val = hash_str(sym->sc->name, add_on_fail);
                    break;

                case INT_CONSTANT_SYMBOL_TYPE:
                    return_val = hash_int(sym->ic->value, add_on_fail);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return_val = hash_float(sym->fc->value, add_on_fail);
                    break;
            }

            // cache results for later re-use
            sym->smem_hash = return_val;
            sym->smem_valid = smem_validation;
        }

        return_val = sym->smem_hash;
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->hash->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}

int64_t SMem_Manager::rhash__int(smem_hash_id hash_value)
{
    int64_t return_val = NIL;

    SQL->hash_rev_int->bind_int(1, hash_value);
    soar_module::exec_result res = SQL->hash_rev_int->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = SQL->hash_rev_int->column_int(0);
    SQL->hash_rev_int->reinitialize();

    return return_val;
}

double SMem_Manager::rhash__float(smem_hash_id hash_value)
{
    double return_val = NIL;

    SQL->hash_rev_float->bind_int(1, hash_value);
    soar_module::exec_result res = SQL->hash_rev_float->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = SQL->hash_rev_float->column_double(0);
    SQL->hash_rev_float->reinitialize();

    return return_val;
}

void SMem_Manager::rhash__str(smem_hash_id hash_value, std::string& dest)
{
    SQL->hash_rev_str->bind_int(1, hash_value);
    soar_module::exec_result res = SQL->hash_rev_str->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    dest.assign(SQL->hash_rev_str->column_text(0));
    SQL->hash_rev_str->reinitialize();
}

 Symbol* SMem_Manager::rhash_(byte symbol_type, smem_hash_id hash_value)
{
    Symbol* return_val = NULL;
    std::string dest;

    switch (symbol_type)
    {
        case STR_CONSTANT_SYMBOL_TYPE:
            rhash__str(hash_value, dest);
            return_val = thisAgent->symbolManager->make_str_constant(const_cast<char*>(dest.c_str()));
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_int_constant(rhash__int(hash_value));
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_float_constant(rhash__float(hash_value));
            break;

        default:
            return_val = NULL;
            break;
    }

    return return_val;
}

// opens the SQLite database and performs all initialization required for the current mode
void SMem_Manager::init_db()
{
    if (connected()) return;

    ////////////////////////////////////////////////////////////////////////////
    timers->init->start();
    ////////////////////////////////////////////////////////////////////////////

    const char* db_path;
    bool tabula_rasa = false;

    if (settings->database->get_value() == smem_param_container::memory)
    {
        db_path = ":memory:";
        tabula_rasa = true;
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory database in cpu memory.\n");
    }
    else
    {
        db_path = settings->path->get_value();
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory memory database at %s\n", db_path);
    }

    // attempt connection
    DB->connect(db_path);

    if (DB->get_status() == soar_module::problem)
    {
        print_sysparam_trace(thisAgent, 0, "Semantic memory database Error: %s\n", DB->get_errmsg());
    }
    else
    {
        // temporary queries for one-time init actions
        soar_module::sqlite_statement* temp_q = NULL;

        // If the database is on file, make sure the database contents use the current schema
        // If it does not, switch to memory-based database

        if (strcmp(db_path, ":memory:")) // Check if database mode is to a file
        {
            bool switch_to_memory, sql_is_new;
            std::string schema_version, version_error_message;

            /* -- Set switch_to_memory true in case we have any errors with the database -- */
            switch_to_memory = true;

            if (DB->sql_is_new_db(sql_is_new))
            {
                if (sql_is_new)
                {
                    print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "...semantic memory database is new.\n");
                    switch_to_memory = false;
                    tabula_rasa = true;
                }
                else
                {
                    // Check if table exists already
                    temp_q = new soar_module::sqlite_statement(DB, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (DB->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'smem_schema'", schema_version))
                        {
                            if (schema_version != SMEM_SCHEMA_VERSION)
                            {
                                version_error_message.assign("...Error: Cannot load semantic memory database with schema version ");
                                version_error_message.append(schema_version.c_str());
                                version_error_message.append(".\n...Please convert old semantic memory database or start a new database by "
                                                             "setting a new database file path.\n...Switching to memory-based database.\n");
                            }
                            else
                            {
                                print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "...version of semantic memory database ok.\n");
                                switch_to_memory = false;
                                tabula_rasa = false;
                            }

                        }
                        else
                        {
                            version_error_message.assign("...Error: Cannot read version number from file-based semantic memory database.\n");
                            if (is_version_one_db())
                            {
                                version_error_message.assign("...Version of semantic memory database is old.\n"
                                                             "...Converting to version 2.0.\n");
                                update_schema_one_to_two();
                                switch_to_memory = false;
                                tabula_rasa = false;
                                delete temp_q;
                                temp_q = NULL;
                            }
                            else
                            {
                                version_error_message.assign("...Switching to memory-based database.\n");
                            }
                        }
                    }
                    else     // Non-empty database exists with no version table.  Probably schema 1.0
                    {
                        if (is_version_one_db())
                        {
                            version_error_message.assign("...Version of semantic memory database is old.\n"
                                                         "...Converting to version 2.0.\n");
                            update_schema_one_to_two();
                            switch_to_memory = false;
                            tabula_rasa = false;
                            delete temp_q;
                            temp_q = NULL;
                        }
                        else
                        {
                            version_error_message.assign("...Error: Cannot load a semantic memory database with an old schema version.\n...Please convert "
                                                         "old semantic memory database or start a new database by setting a new database file path.\n...Switching "
                                                         "to memory-based database.\n");
                        }
                    }
                    delete temp_q;
                    temp_q = NULL;
                }
            }
            else
            {
                version_error_message.assign("...Error:  Cannot read database meta info from file-based semantic memory database.\n"
                                             "...Switching to memory-based database.\n");
            }
            if (switch_to_memory)
            {
                // Memory mode will be set on, database will be disconnected to and then init_db
                // will be called again to reinitialize database.
                switch_to_memory_db(version_error_message);
                return;
            }
            else
            {
                // Spreading activation violates an assumption that the database be somewhat stateless
                // for the sake of backup. TODO: Make the state-dependent parts of spreading in-memory.
                // In the meantime, the solution is simply to clear out those tables which contain state
                // when restoring from a backup.
                soar_module::sqlite_statement* temp_spread = new soar_module::sqlite_statement(DB, "SELECT name FROM sqlite_master WHERE type='table' AND name='smem_current_spread';");
                temp_spread->prepare();
                if (temp_spread->get_status() == soar_module::ready)
                {
                    if (temp_spread->execute() == soar_module::row)
                    {
                        soar_module::sqlite_statement* temp_spread_q_1 = new soar_module::sqlite_statement(DB, "DELETE FROM smem_current_spread");
                        temp_spread_q_1->prepare();
                        temp_spread_q_1->execute(soar_module::op_reinit);
                        soar_module::sqlite_statement* temp_spread_q_2 = new soar_module::sqlite_statement(DB, "DELETE FROM smem_current_spread_activations");
                        temp_spread_q_2->prepare();
                        temp_spread_q_2->execute(soar_module::op_reinit);
                        delete temp_spread_q_1;
                        delete temp_spread_q_2;
                    }
                }
                temp_spread->reinitialize();
                delete temp_spread;
            }
        }

        // apply performance options
        {
            // page_size
            {
                switch (settings->page_size->get_value())
                {
                    case (smem_param_container::page_1k):
                        DB->sql_execute("PRAGMA page_size = 1024");
                        break;

                    case (smem_param_container::page_2k):
                        DB->sql_execute("PRAGMA page_size = 2048");
                        break;

                    case (smem_param_container::page_4k):
                        DB->sql_execute("PRAGMA page_size = 4096");
                        break;

                    case (smem_param_container::page_8k):
                        DB->sql_execute("PRAGMA page_size = 8192");
                        break;

                    case (smem_param_container::page_16k):
                        DB->sql_execute("PRAGMA page_size = 16384");
                        break;

                    case (smem_param_container::page_32k):
                        DB->sql_execute("PRAGMA page_size = 32768");
                        break;

                    case (smem_param_container::page_64k):
                        DB->sql_execute("PRAGMA page_size = 65536");
                        break;
                }
            }

            // cache_size
            {
                std::string cache_sql("PRAGMA cache_size = ");
                char* str = settings->cache_size->get_cstring();
                cache_sql.append(str);
                free(str);
                str = NULL;
                DB->sql_execute(cache_sql.c_str());
            }

            // optimization
            if (settings->opt->get_value() == smem_param_container::opt_speed)
            {
                // synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
                DB->sql_execute("PRAGMA synchronous = OFF");

                // journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
                DB->sql_execute("PRAGMA journal_mode = OFF");

                // locking_mode - no one else can view the database after our first write
                DB->sql_execute("PRAGMA locking_mode = EXCLUSIVE");
            }
        }

        // update validation count
        smem_validation++;

        // setup common structures/queries
        SQL = new smem_statement_container(thisAgent);

        if (tabula_rasa || (settings->append_db->get_value() == off))
        {
            SQL->structure();
        }

        // initialize queries given database structure
        SQL->prepare();

        // initialize persistent variables
        if (tabula_rasa || (settings->append_db->get_value() == off))
        {
            SQL->begin->execute(soar_module::op_reinit);
            {
                // max cycle
                smem_max_cycle = static_cast<int64_t>(1);
                variable_create(var_max_cycle, 1);

                // number of nodes
                statistics->nodes->set_value(0);
                variable_create(var_num_nodes, 0);

                // number of edges
                statistics->edges->set_value(0);
                variable_create(var_num_edges, 0);

                // threshold (from user parameter value)
                variable_create(var_act_thresh, static_cast<int64_t>(settings->thresh->get_value()));

                // activation mode (from user parameter value)
                variable_create(var_act_mode, static_cast<int64_t>(settings->activation_mode->get_value()));
            }
            SQL->commit->execute(soar_module::op_reinit);
        }
        else
        {
            int64_t temp;

            // max cycle
            variable_get(var_max_cycle, &(smem_max_cycle));

            // number of nodes
            variable_get(var_num_nodes, &(temp));
            statistics->nodes->set_value(temp);

            // number of edges
            variable_get(var_num_edges, &(temp));
            statistics->edges->set_value(temp);

            // threshold
            variable_get(var_act_thresh, &(temp));
            settings->thresh->set_value(temp);

            // activation mode
            variable_get(var_act_mode, &(temp));
            settings->activation_mode->set_value(static_cast< smem_param_container::act_choices >(temp));
        }

        reset_id_counters();

        // if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
        if (settings->lazy_commit->get_value() == on)
        {
            SQL->begin->execute(soar_module::op_reinit);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->init->stop();
    ////////////////////////////////////////////////////////////////////////////
}

// gets an SMem variable from the database
bool SMem_Manager::variable_get(smem_variable_key variable_id, int64_t* variable_value)
{
    soar_module::exec_result status;
    soar_module::sqlite_statement* var_get = SQL->var_get;

    var_get->bind_int(1, variable_id);
    status = var_get->execute();

    if (status == soar_module::row)
    {
        (*variable_value) = var_get->column_int(0);
    }

    var_get->reinitialize();

    return (status == soar_module::row);
}

// sets an existing SMem variable in the database
void SMem_Manager::variable_set(smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_set = SQL->var_set;

    var_set->bind_int(1, variable_value);
    var_set->bind_int(2, variable_id);

    var_set->execute(soar_module::op_reinit);
}

// creates a new SMem variable in the database
void SMem_Manager::variable_create(smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_create = SQL->var_create;

    var_create->bind_int(1, variable_id);
    var_create->bind_int(2, variable_value);

    var_create->execute(soar_module::op_reinit);
}

void SMem_Manager::store_globals_in_db()
{
    // store max cycle for future use of the smem database
    variable_set(var_max_cycle, smem_max_cycle);

    // store num nodes/edges for future use of the smem database
    variable_set(var_num_nodes, statistics->nodes->get_value());
    variable_set(var_num_edges, statistics->edges->get_value());
}

// performs cleanup operations when the database needs to be closed (end soar, manual close, etc)
void SMem_Manager::close()
{
    if (connected())
    {
        store_globals_in_db();

        // if lazy, commit
        if (settings->lazy_commit->get_value() == on)
        {
            SQL->commit->execute(soar_module::op_reinit);
        }

        // de-allocate common statements
        delete SQL;
        delete thisAgent->lastCue;
        thisAgent->lastCue = NULL;

        // close the database
        DB->disconnect();
    }
}

void SMem_Manager::attach()
{
    if (!connected())
    {
        init_db();
    }
}

bool SMem_Manager::backup_db(const char* file_name, std::string* err)
{
    bool return_val = false;

    if (connected())
    {
        store_globals_in_db();

        if (settings->lazy_commit->get_value() == on)
        {
            SQL->commit->execute(soar_module::op_reinit);
        }

        return_val = DB->backup(file_name, err);

        if (settings->lazy_commit->get_value() == on)
        {
            SQL->begin->execute(soar_module::op_reinit);
        }
    }
    else
    {
        err->assign("Semantic database is not currently connected.");
    }

    return return_val;
}

void SMem_Manager::switch_to_memory_db(std::string& buf)
{
    print_sysparam_trace(thisAgent, 0, buf.c_str());
    DB->disconnect();
    settings->database->set_value(smem_param_container::memory);
    init_db();
}

void SMem_Manager::update_schema_one_to_two()
{
    DB->sql_execute("BEGIN TRANSACTION");
    DB->sql_execute("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY,symbol_type INTEGER)");
    DB->sql_execute("INSERT INTO smem_symbols_type (s_id, symbol_type) SELECT id, sym_type FROM smem7_symbols_type");
    DB->sql_execute("DROP TABLE smem7_symbols_type");

    DB->sql_execute("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY,symbol_value TEXT)");
    DB->sql_execute("INSERT INTO smem_symbols_string (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_str");
    DB->sql_execute("DROP TABLE smem7_symbols_str");

    DB->sql_execute("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY,symbol_value INTEGER)");
    DB->sql_execute("INSERT INTO smem_symbols_integer (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_int");
    DB->sql_execute("DROP TABLE smem7_symbols_int");

    DB->sql_execute("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY,ascii_chr TEXT)");
    DB->sql_execute("INSERT INTO smem_ascii (ascii_num, ascii_chr) SELECT ascii_num, ascii_num FROM smem7_ascii");
    DB->sql_execute("DROP TABLE smem7_ascii");

    DB->sql_execute("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY,symbol_value REAL)");
    DB->sql_execute("INSERT INTO smem_symbols_float (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_float");
    DB->sql_execute("DROP TABLE smem7_symbols_float");

    DB->sql_execute("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY,total_augmentations INTEGER,activation_value REAL,activations_total INTEGER,activations_last INTEGER,activations_first INTEGER)");
    DB->sql_execute("INSERT INTO smem_lti (lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first) SELECT id, child_ct, act_value, access_n, access_t, access_1 FROM smem7_lti");
    DB->sql_execute("DROP TABLE smem7_lti");

    DB->sql_execute("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY,t1 INTEGER,t2 INTEGER,t3 INTEGER,t4 INTEGER,t5 INTEGER,t6 INTEGER,t7 INTEGER,t8 INTEGER,t9 INTEGER,t10 INTEGER)");
    DB->sql_execute("INSERT INTO smem_activation_history (lti_id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) SELECT id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 FROM smem7_history");
    DB->sql_execute("DROP TABLE smem7_history");

    DB->sql_execute("CREATE TABLE smem_augmentations (lti_id INTEGER,attribute_s_id INTEGER,value_constant_s_id INTEGER,value_lti_id INTEGER,activation_value REAL)");
    DB->sql_execute("INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) SELECT parent_id, attr, val_const, val_lti, act_value FROM smem7_web");
    DB->sql_execute("DROP TABLE smem7_web");

    DB->sql_execute("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY,edge_frequency INTEGER)");
    DB->sql_execute("INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) SELECT attr, ct FROM smem7_ct_attr");
    DB->sql_execute("DROP TABLE smem7_ct_attr");

    DB->sql_execute("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER,value_constant_s_id INTEGER,edge_frequency INTEGER)");
    DB->sql_execute("INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) SELECT attr, val_const, ct FROM smem7_ct_const");
    DB->sql_execute("DROP TABLE smem7_ct_const");

    DB->sql_execute("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER,value_lti_id INTEGER,edge_frequency INTEGER)");
    DB->sql_execute("INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) SELECT attr, val_lti, ct FROM smem7_ct_lti");
    DB->sql_execute("DROP TABLE smem7_ct_lti");

    DB->sql_execute("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    DB->sql_execute("INSERT INTO smem_persistent_variables (variable_id, variable_value) SELECT id, value FROM smem7_vars");
    DB->sql_execute("DROP TABLE smem7_vars");

    DB->sql_execute("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    DB->sql_execute("INSERT INTO versions (system, version_number) VALUES ('smem_schema','2.0')");
    DB->sql_execute("DROP TABLE smem7_signature");

    DB->sql_execute("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    DB->sql_execute("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
    DB->sql_execute("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    DB->sql_execute("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    DB->sql_execute("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    DB->sql_execute("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id,value_lti_id)");
    DB->sql_execute("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    DB->sql_execute("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    DB->sql_execute("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    DB->sql_execute("COMMIT");
}

uint64_t SMem_Manager::lti_exists(uint64_t pLTI_ID)
{
    uint64_t return_val = NIL;

    if (connected())
    {
        SQL->lti_id_exists->bind_int(1, static_cast<uint64_t>(pLTI_ID));

        if (SQL->lti_id_exists->execute() == soar_module::row)
        {
            return_val = SQL->lti_id_exists->column_int(0);
        }

        SQL->lti_id_exists->reinitialize();
    }
    return return_val;
}

uint64_t SMem_Manager::get_max_lti_id()
{
    uint64_t return_val = 0;

    if (connected())
    {
        if (SQL->lti_id_max->execute() == soar_module::row)
        {
            return_val = SQL->lti_id_max->column_int(0);
        }

        SQL->lti_id_max->reinitialize();
    }
    return return_val;
}

void SMem_Manager::set_id_counter(uint64_t counter_value)
{
    lti_id_counter = counter_value-1;
}

uint64_t SMem_Manager::add_new_LTI()
{
    uint64_t lti_id = ++lti_id_counter;
//    if (lti_exists(lti_id))
//    {/* The previous solution would be horrible in adversarial cases. It incremented,
//      * then checked for existing id in a loop. If ids were manually assigned from
//      * 2 to 1,000,000, this loop would have ~1,000,000 iterations. Now, instead, if
//      * someone manually inserts an element with id 2 and id 1,000,000, on the
//      * collision with 2, we'll skip straight to 1,000,001 instead of checking for
//      * empty slots between. These numbers shouldn't be assumed to be meaningful
//      * outside of self-consistency, so I think this is better.
//      *
//      * - scijones 2016
//      */
//        /*
//         * As it turns out, the above solution can be bad too, so I've commented it out.
//         * Basically, just setting one id to max int ruins things with the "new" solution.
//         */
//        uint64_t test_lti_id = get_max_lti_id();
//        if (test_lti_id == 0)
//        {//If for some reason the get_max_lti_id doesn't work, we use the old behavior.
//            while (lti_exists(lti_id))
//            {
//                lti_id = ++lti_id_counter;
//            }
//        }
//        else
//        {
//            lti_id_counter = test_lti_id + 1;
//            lti_id = lti_id_counter;
//        }
//    }
    while (lti_exists(lti_id))
    {
        lti_id = ++lti_id_counter;
    }
    // add lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first
    SQL->lti_add->bind_int(1, static_cast<uint64_t>(lti_id));
    SQL->lti_add->bind_int(2, static_cast<uint64_t>(0));
    SQL->lti_add->bind_double(3, static_cast<double>(0));
    SQL->lti_add->bind_int(4, static_cast<uint64_t>(0));
    SQL->lti_add->bind_int(5, static_cast<uint64_t>(0));
    SQL->lti_add->bind_int(6, static_cast<uint64_t>(0));
    SQL->lti_add->execute(soar_module::op_reinit);

//    assert(lti_id_counter == smem_db->last_insert_rowid());

    statistics->nodes->set_value(statistics->nodes->get_value() + 1);

    return lti_id_counter;
}

uint64_t SMem_Manager::add_specific_LTI(uint64_t lti_id)
{
    // add lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first
    SQL->lti_add->bind_int(1, static_cast<uint64_t>(lti_id));
    SQL->lti_add->bind_int(2, static_cast<uint64_t>(0));
    SQL->lti_add->bind_double(3, static_cast<double>(0));
    SQL->lti_add->bind_int(4, static_cast<uint64_t>(0));
    SQL->lti_add->bind_int(5, static_cast<uint64_t>(0));
    SQL->lti_add->bind_int(6, static_cast<uint64_t>(0));
    SQL->lti_add->execute(soar_module::op_reinit);

//    assert(lti_id_counter == smem_db->last_insert_rowid());

    statistics->nodes->set_value(statistics->nodes->get_value() + 1);

    return lti_id;
}
