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
    add_structure("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY, soar_letter INTEGER, soar_number INTEGER, total_augmentations INTEGER, activation_value REAL, activations_total INTEGER, activations_last INTEGER, activations_first INTEGER)");
    add_structure("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY, t1 INTEGER, t2 INTEGER, t3 INTEGER, t4 INTEGER, t5 INTEGER, t6 INTEGER, t7 INTEGER, t8 INTEGER, t9 INTEGER, t10 INTEGER)");
    add_structure("CREATE TABLE smem_augmentations (lti_id INTEGER, attribute_s_id INTEGER, value_constant_s_id INTEGER, value_lti_id INTEGER, activation_value REAL)");
    add_structure("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER, value_constant_s_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER, value_lti_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)");
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
    add_structure("CREATE UNIQUE INDEX smem_lti_letter_num ON smem_lti (soar_letter, soar_number)");
    add_structure("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    add_structure("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id)");
    add_structure("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    add_structure("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    add_structure("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    add_structure("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
}

void smem_statement_container::drop_tables(agent* new_agent)
{
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_persistent_variables");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_type");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_integer");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_float");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_string");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_lti");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_activation_history");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_augmentations");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_attribute_frequency");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_wmes_constant_frequency");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_wmes_lti_frequency");
    new_agent->SMem->smem_db->sql_execute("DROP TABLE IF EXISTS smem_ascii");
}

smem_statement_container::smem_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->SMem->smem_db)
{
    soar_module::sqlite_database* new_db = new_agent->SMem->smem_db;

    // Delete all entries from the tables in the database if append setting is off
    if (new_agent->SMem->smem_params->append_db->get_value() == off)
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

    lti_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_lti (soar_letter,soar_number,total_augmentations,activation_value,activations_total,activations_last,activations_first) VALUES (?,?,?,?,?,?,?)");
    add(lti_add);

    lti_get = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_lti WHERE soar_letter=? AND soar_number=?");
    add(lti_get);

    lti_letter_num = new soar_module::sqlite_statement(new_db, "SELECT soar_letter, soar_number FROM smem_lti WHERE lti_id=?");
    add(lti_letter_num);

    lti_max = new soar_module::sqlite_statement(new_db, "SELECT soar_letter, MAX(soar_number) FROM smem_lti GROUP BY soar_letter");
    add(lti_max);

    lti_access_get = new soar_module::sqlite_statement(new_db, "SELECT activations_total, activations_last, activations_first FROM smem_lti WHERE lti_id=?");
    add(lti_access_get);

    lti_access_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET activations_total=?, activations_last=?, activations_first=? WHERE lti_id=?");
    add(lti_access_set);

    lti_get_t = new soar_module::sqlite_statement(new_db, "SELECT lti_id FROM smem_lti WHERE activations_last=?");
    add(lti_get_t);

    //

    web_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) VALUES (?,?,?,?,?)");
    add(web_add);

    web_truncate = new soar_module::sqlite_statement(new_db, "DELETE FROM smem_augmentations WHERE lti_id=?");
    add(web_truncate);

    web_expand = new soar_module::sqlite_statement(new_db, "SELECT tsh_a.symbol_type AS attr_type, tsh_a.s_id AS attr_hash, vcl.symbol_type AS value_type, vcl.s_id AS value_hash, vcl.soar_letter AS value_letter, vcl.soar_number AS value_num, vcl.value_lti_id AS value_lti FROM ((smem_augmentations w LEFT JOIN smem_symbols_type tsh_v ON w.value_constant_s_id=tsh_v.s_id) vc LEFT JOIN smem_lti AS lti ON vc.value_lti_id=lti.lti_id) vcl INNER JOIN smem_symbols_type tsh_a ON vcl.attribute_s_id=tsh_a.s_id WHERE vcl.lti_id=?");
    add(web_expand);

    //

    web_all = new soar_module::sqlite_statement(new_db, "SELECT attribute_s_id, value_constant_s_id, value_lti_id FROM smem_augmentations WHERE lti_id=?");
    add(web_all);

    //

    web_attr_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? ORDER BY activation_value DESC");
    add(web_attr_all);

    web_const_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=? AND value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR " ORDER BY activation_value DESC");
    add(web_const_all);

    web_lti_all = new soar_module::sqlite_statement(new_db, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=? ORDER BY activation_value DESC");
    add(web_lti_all);

    //

    web_attr_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=?");
    add(web_attr_child);

    web_const_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=?");
    add(web_const_child);

    web_lti_child = new soar_module::sqlite_statement(new_db, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=?");
    add(web_lti_child);

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

    act_lti_set = new soar_module::sqlite_statement(new_db, "UPDATE smem_lti SET activation_value=? WHERE lti_id=?");
    add(act_lti_set);

    act_lti_get = new soar_module::sqlite_statement(new_db, "SELECT activation_value FROM smem_lti WHERE lti_id=?");
    add(act_lti_get);

    history_get = new soar_module::sqlite_statement(new_db, "SELECT t1,t2,t3,t4,t5,t6,t7,t8,t9,t10 FROM smem_activation_history WHERE lti_id=?");
    add(history_get);

    history_push = new soar_module::sqlite_statement(new_db, "UPDATE smem_activation_history SET t10=t9,t9=t8,t8=t7,t8=t7,t7=t6,t6=t5,t5=t4,t4=t3,t3=t2,t2=t1,t1=? WHERE lti_id=?");
    add(history_push);

    history_add = new soar_module::sqlite_statement(new_db, "INSERT INTO smem_activation_history (lti_id,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10) VALUES (?,?,0,0,0,0,0,0,0,0,0)");
    add(history_add);

    //

    vis_lti = new soar_module::sqlite_statement(new_db, "SELECT lti_id, soar_letter, soar_number, activation_value FROM smem_lti ORDER BY soar_letter ASC, soar_number ASC");
    add(vis_lti);

    vis_lti_act = new soar_module::sqlite_statement(new_db, "SELECT activation_value FROM smem_lti WHERE lti_id=?");
    add(vis_lti_act);

    vis_value_const = new soar_module::sqlite_statement(new_db, "SELECT lti_id, tsh1.symbol_type AS attr_type, tsh1.s_id AS attr_hash, tsh2.symbol_type AS val_type, tsh2.s_id AS val_hash FROM smem_augmentations w, smem_symbols_type tsh1, smem_symbols_type tsh2 WHERE (w.attribute_s_id=tsh1.s_id) AND (w.value_constant_s_id=tsh2.s_id)");
    add(vis_value_const);

    vis_value_lti = new soar_module::sqlite_statement(new_db, "SELECT lti_id, tsh.symbol_type AS attr_type, tsh.s_id AS attr_hash, value_lti_id FROM smem_augmentations w, smem_symbols_type tsh WHERE (w.attribute_s_id=tsh.s_id) AND (value_lti_id<>" SMEM_AUGMENTATIONS_NULL_STR ")");
    add(vis_value_lti);
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

smem_hash_id SMem_Manager::smem_temporal_hash_add_type(byte symbol_type)
{
    smem_stmts->hash_add_type->bind_int(1, symbol_type);
    smem_stmts->hash_add_type->execute(soar_module::op_reinit);
    return static_cast<smem_hash_id>(smem_db->last_insert_rowid());
}

smem_hash_id SMem_Manager::smem_temporal_hash_int(int64_t val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    smem_stmts->hash_get_int->bind_int(1, val);
    if (smem_stmts->hash_get_int->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(smem_stmts->hash_get_int->column_int(0));
    }
    smem_stmts->hash_get_int->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(INT_CONSTANT_SYMBOL_TYPE);

        // then content
        smem_stmts->hash_add_int->bind_int(1, return_val);
        smem_stmts->hash_add_int->bind_int(2, val);
        smem_stmts->hash_add_int->execute(soar_module::op_reinit);
    }

    return return_val;
}

smem_hash_id SMem_Manager::smem_temporal_hash_float(double val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    smem_stmts->hash_get_float->bind_double(1, val);
    if (smem_stmts->hash_get_float->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(smem_stmts->hash_get_float->column_int(0));
    }
    smem_stmts->hash_get_float->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(FLOAT_CONSTANT_SYMBOL_TYPE);

        // then content
        smem_stmts->hash_add_float->bind_int(1, return_val);
        smem_stmts->hash_add_float->bind_double(2, val);
        smem_stmts->hash_add_float->execute(soar_module::op_reinit);
    }

    return return_val;
}

smem_hash_id SMem_Manager::smem_temporal_hash_str(char* val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    smem_stmts->hash_get_str->bind_text(1, static_cast<const char*>(val));
    if (smem_stmts->hash_get_str->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(smem_stmts->hash_get_str->column_int(0));
    }
    smem_stmts->hash_get_str->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(STR_CONSTANT_SYMBOL_TYPE);

        // then content
        smem_stmts->hash_add_str->bind_int(1, return_val);
        smem_stmts->hash_add_str->bind_text(2, static_cast<const char*>(val));
        smem_stmts->hash_add_str->execute(soar_module::op_reinit);
    }

    return return_val;
}

// returns a temporally unique integer representing a symbol constant
smem_hash_id SMem_Manager::smem_temporal_hash(Symbol* sym, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->hash->start();
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
                    return_val = smem_temporal_hash_str(sym->sc->name, add_on_fail);
                    break;

                case INT_CONSTANT_SYMBOL_TYPE:
                    return_val = smem_temporal_hash_int(sym->ic->value, add_on_fail);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return_val = smem_temporal_hash_float(sym->fc->value, add_on_fail);
                    break;
            }

            // cache results for later re-use
            sym->smem_hash = return_val;
            sym->smem_valid = smem_validation;
        }

        return_val = sym->smem_hash;
    }

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->hash->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}

int64_t SMem_Manager::smem_reverse_hash_int(smem_hash_id hash_value)
{
    int64_t return_val = NIL;

    smem_stmts->hash_rev_int->bind_int(1, hash_value);
    soar_module::exec_result res = smem_stmts->hash_rev_int->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = smem_stmts->hash_rev_int->column_int(0);
    smem_stmts->hash_rev_int->reinitialize();

    return return_val;
}

double SMem_Manager::smem_reverse_hash_float(smem_hash_id hash_value)
{
    double return_val = NIL;

    smem_stmts->hash_rev_float->bind_int(1, hash_value);
    soar_module::exec_result res = smem_stmts->hash_rev_float->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = smem_stmts->hash_rev_float->column_double(0);
    smem_stmts->hash_rev_float->reinitialize();

    return return_val;
}

void SMem_Manager::smem_reverse_hash_str(smem_hash_id hash_value, std::string& dest)
{
    smem_stmts->hash_rev_str->bind_int(1, hash_value);
    soar_module::exec_result res = smem_stmts->hash_rev_str->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    dest.assign(smem_stmts->hash_rev_str->column_text(0));
    smem_stmts->hash_rev_str->reinitialize();
}

 Symbol* SMem_Manager::smem_reverse_hash(byte symbol_type, smem_hash_id hash_value)
{
    Symbol* return_val = NULL;
    std::string dest;

    switch (symbol_type)
    {
        case STR_CONSTANT_SYMBOL_TYPE:
            smem_reverse_hash_str(hash_value, dest);
            return_val = thisAgent->symbolManager->make_str_constant(const_cast<char*>(dest.c_str()));
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_int_constant(smem_reverse_hash_int(hash_value));
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_float_constant(smem_reverse_hash_float(hash_value));
            break;

        default:
            return_val = NULL;
            break;
    }

    return return_val;
}

// opens the SQLite database and performs all initialization required for the current mode
void SMem_Manager::smem_init_db()
{
    if (smem_db->get_status() != soar_module::disconnected)
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->init->start();
    ////////////////////////////////////////////////////////////////////////////

    const char* db_path;
    bool tabula_rasa = false;

    if (smem_params->database->get_value() == smem_param_container::memory)
    {
        db_path = ":memory:";
        tabula_rasa = true;
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory database in cpu memory.\n");
    }
    else
    {
        db_path = smem_params->path->get_value();
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory memory database at %s\n", db_path);
    }

    // attempt connection
    smem_db->connect(db_path);

    if (smem_db->get_status() == soar_module::problem)
    {
        print_sysparam_trace(thisAgent, 0, "Semantic memory database Error: %s\n", smem_db->get_errmsg());
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

            if (smem_db->sql_is_new_db(sql_is_new))
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
                    temp_q = new soar_module::sqlite_statement(smem_db, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (smem_db->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'smem_schema'", schema_version))
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
                            if (smem_version_one())
                            {
                                version_error_message.assign("...Version of semantic memory database is old.\n"
                                                             "...Converting to version 2.0.\n");
                                smem_update_schema_one_to_two();
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
                        if (smem_version_one())
                        {
                            version_error_message.assign("...Version of semantic memory database is old.\n"
                                                         "...Converting to version 2.0.\n");
                            smem_update_schema_one_to_two();
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
                smem_switch_to_memory_db(version_error_message);
                return;
            }
        }

        // apply performance options
        {
            // page_size
            {
                switch (smem_params->page_size->get_value())
                {
                    case (smem_param_container::page_1k):
                        smem_db->sql_execute("PRAGMA page_size = 1024");
                        break;

                    case (smem_param_container::page_2k):
                        smem_db->sql_execute("PRAGMA page_size = 2048");
                        break;

                    case (smem_param_container::page_4k):
                        smem_db->sql_execute("PRAGMA page_size = 4096");
                        break;

                    case (smem_param_container::page_8k):
                        smem_db->sql_execute("PRAGMA page_size = 8192");
                        break;

                    case (smem_param_container::page_16k):
                        smem_db->sql_execute("PRAGMA page_size = 16384");
                        break;

                    case (smem_param_container::page_32k):
                        smem_db->sql_execute("PRAGMA page_size = 32768");
                        break;

                    case (smem_param_container::page_64k):
                        smem_db->sql_execute("PRAGMA page_size = 65536");
                        break;
                }
            }

            // cache_size
            {
                std::string cache_sql("PRAGMA cache_size = ");
                char* str = smem_params->cache_size->get_string();
                cache_sql.append(str);
                free(str);
                str = NULL;
                smem_db->sql_execute(cache_sql.c_str());
            }

            // optimization
            if (smem_params->opt->get_value() == smem_param_container::opt_speed)
            {
                // synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
                smem_db->sql_execute("PRAGMA synchronous = OFF");

                // journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
                smem_db->sql_execute("PRAGMA journal_mode = OFF");

                // locking_mode - no one else can view the database after our first write
                smem_db->sql_execute("PRAGMA locking_mode = EXCLUSIVE");
            }
        }

        // update validation count
        smem_validation++;

        // setup common structures/queries
        smem_stmts = new smem_statement_container(thisAgent);

        if (tabula_rasa || (smem_params->append_db->get_value() == off))
        {
            smem_stmts->structure();
        }

        // initialize queries given database structure
        smem_stmts->prepare();

        // initialize persistent variables
        if (tabula_rasa || (smem_params->append_db->get_value() == off))
        {
            smem_stmts->begin->execute(soar_module::op_reinit);
            {
                // max cycle
                smem_max_cycle = static_cast<int64_t>(1);
                smem_variable_create(var_max_cycle, 1);

                // number of nodes
                smem_stats->chunks->set_value(0);
                smem_variable_create(var_num_nodes, 0);

                // number of edges
                smem_stats->slots->set_value(0);
                smem_variable_create(var_num_edges, 0);

                // threshold (from user parameter value)
                smem_variable_create(var_act_thresh, static_cast<int64_t>(smem_params->thresh->get_value()));

                // activation mode (from user parameter value)
                smem_variable_create(var_act_mode, static_cast<int64_t>(smem_params->activation_mode->get_value()));
            }
            smem_stmts->commit->execute(soar_module::op_reinit);
        }
        else
        {
            int64_t temp;

            // max cycle
            smem_variable_get(var_max_cycle, &(smem_max_cycle));

            // number of nodes
            smem_variable_get(var_num_nodes, &(temp));
            smem_stats->chunks->set_value(temp);

            // number of edges
            smem_variable_get(var_num_edges, &(temp));
            smem_stats->slots->set_value(temp);

            // threshold
            smem_variable_get(var_act_thresh, &(temp));
            smem_params->thresh->set_value(temp);

            // activation mode
            smem_variable_get(var_act_mode, &(temp));
            smem_params->activation_mode->set_value(static_cast< smem_param_container::act_choices >(temp));
        }

        // reset identifier counters
        smem_reset_id_counters();

        // if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
        if (smem_params->lazy_commit->get_value() == on)
        {
            smem_stmts->begin->execute(soar_module::op_reinit);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->init->stop();
    ////////////////////////////////////////////////////////////////////////////
}

// gets an SMem variable from the database
bool SMem_Manager::smem_variable_get(smem_variable_key variable_id, int64_t* variable_value)
{
    soar_module::exec_result status;
    soar_module::sqlite_statement* var_get = smem_stmts->var_get;

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
void SMem_Manager::smem_variable_set(smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_set = smem_stmts->var_set;

    var_set->bind_int(1, variable_value);
    var_set->bind_int(2, variable_id);

    var_set->execute(soar_module::op_reinit);
}

// creates a new SMem variable in the database
void SMem_Manager::smem_variable_create(smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_create = smem_stmts->var_create;

    var_create->bind_int(1, variable_id);
    var_create->bind_int(2, variable_value);

    var_create->execute(soar_module::op_reinit);
}

void SMem_Manager::store_globals_in_db()
{
    // store max cycle for future use of the smem database
    smem_variable_set(var_max_cycle, smem_max_cycle);

    // store num nodes/edges for future use of the smem database
    smem_variable_set(var_num_nodes, smem_stats->chunks->get_value());
    smem_variable_set(var_num_edges, smem_stats->slots->get_value());
}

// performs cleanup operations when the database needs to be closed (end soar, manual close, etc)
void SMem_Manager::smem_close()
{
    if (smem_db->get_status() == soar_module::connected)
    {
        store_globals_in_db();

        // if lazy, commit
        if (smem_params->lazy_commit->get_value() == on)
        {
            smem_stmts->commit->execute(soar_module::op_reinit);
        }

        // de-allocate common statements
        delete smem_stmts;
        delete thisAgent->lastCue;

        // close the database
        smem_db->disconnect();
    }
}

void SMem_Manager::smem_attach()
{
    if (smem_db->get_status() == soar_module::disconnected)
    {
        smem_init_db();
    }
}

bool SMem_Manager::smem_backup_db(const char* file_name, std::string* err)
{
    bool return_val = false;

    if (smem_db->get_status() == soar_module::connected)
    {
        store_globals_in_db();

        if (smem_params->lazy_commit->get_value() == on)
        {
            smem_stmts->commit->execute(soar_module::op_reinit);
        }

        return_val = smem_db->backup(file_name, err);

        if (smem_params->lazy_commit->get_value() == on)
        {
            smem_stmts->begin->execute(soar_module::op_reinit);
        }
    }
    else
    {
        err->assign("Semantic database is not currently connected.");
    }

    return return_val;
}

void SMem_Manager::smem_switch_to_memory_db(std::string& buf)
{
    print_sysparam_trace(thisAgent, 0, buf.c_str());
    smem_db->disconnect();
    smem_params->database->set_value(smem_param_container::memory);
    smem_init_db();
}

void SMem_Manager::smem_update_schema_one_to_two()
{
    smem_db->sql_execute("BEGIN TRANSACTION");
    smem_db->sql_execute("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY,symbol_type INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_symbols_type (s_id, symbol_type) SELECT id, sym_type FROM smem7_symbols_type");
    smem_db->sql_execute("DROP TABLE smem7_symbols_type");

    smem_db->sql_execute("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY,symbol_value TEXT)");
    smem_db->sql_execute("INSERT INTO smem_symbols_string (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_str");
    smem_db->sql_execute("DROP TABLE smem7_symbols_str");

    smem_db->sql_execute("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY,symbol_value INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_symbols_integer (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_int");
    smem_db->sql_execute("DROP TABLE smem7_symbols_int");

    smem_db->sql_execute("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY,ascii_chr TEXT)");
    smem_db->sql_execute("INSERT INTO smem_ascii (ascii_num, ascii_chr) SELECT ascii_num, ascii_num FROM smem7_ascii");
    smem_db->sql_execute("DROP TABLE smem7_ascii");

    smem_db->sql_execute("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY,symbol_value REAL)");
    smem_db->sql_execute("INSERT INTO smem_symbols_float (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_float");
    smem_db->sql_execute("DROP TABLE smem7_symbols_float");

    smem_db->sql_execute("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY,soar_letter INTEGER,soar_number INTEGER,total_augmentations INTEGER,activation_value REAL,activations_total INTEGER,activations_last INTEGER,activations_first INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_lti (lti_id, soar_letter, soar_number, total_augmentations, activation_value, activations_total, activations_last, activations_first) SELECT id, letter, num, child_ct, act_value, access_n, access_t, access_1 FROM smem7_lti");
    smem_db->sql_execute("DROP TABLE smem7_lti");

    smem_db->sql_execute("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY,t1 INTEGER,t2 INTEGER,t3 INTEGER,t4 INTEGER,t5 INTEGER,t6 INTEGER,t7 INTEGER,t8 INTEGER,t9 INTEGER,t10 INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_activation_history (lti_id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) SELECT id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 FROM smem7_history");
    smem_db->sql_execute("DROP TABLE smem7_history");

    smem_db->sql_execute("CREATE TABLE smem_augmentations (lti_id INTEGER,attribute_s_id INTEGER,value_constant_s_id INTEGER,value_lti_id INTEGER,activation_value REAL)");
    smem_db->sql_execute("INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) SELECT parent_id, attr, val_const, val_lti, act_value FROM smem7_web");
    smem_db->sql_execute("DROP TABLE smem7_web");

    smem_db->sql_execute("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY,edge_frequency INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) SELECT attr, ct FROM smem7_ct_attr");
    smem_db->sql_execute("DROP TABLE smem7_ct_attr");

    smem_db->sql_execute("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER,value_constant_s_id INTEGER,edge_frequency INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) SELECT attr, val_const, ct FROM smem7_ct_const");
    smem_db->sql_execute("DROP TABLE smem7_ct_const");

    smem_db->sql_execute("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER,value_lti_id INTEGER,edge_frequency INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) SELECT attr, val_lti, ct FROM smem7_ct_lti");
    smem_db->sql_execute("DROP TABLE smem7_ct_lti");

    smem_db->sql_execute("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    smem_db->sql_execute("INSERT INTO smem_persistent_variables (variable_id, variable_value) SELECT id, value FROM smem7_vars");
    smem_db->sql_execute("DROP TABLE smem7_vars");

    smem_db->sql_execute("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    smem_db->sql_execute("INSERT INTO versions (system, version_number) VALUES ('smem_schema','2.0')");
    smem_db->sql_execute("DROP TABLE smem7_signature");

    smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    smem_db->sql_execute("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
    smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    smem_db->sql_execute("CREATE UNIQUE INDEX smem_lti_letter_num ON smem_lti (soar_letter,soar_number)");
    smem_db->sql_execute("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    smem_db->sql_execute("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id,value_lti_id)");
    smem_db->sql_execute("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    smem_db->sql_execute("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    smem_db->sql_execute("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    smem_db->sql_execute("COMMIT");
}
