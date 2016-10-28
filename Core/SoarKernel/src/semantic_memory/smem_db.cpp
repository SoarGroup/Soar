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
#include "guard.hpp"
#include "VariadicBind.h"

#include "agent.h"
#include "print.h"

namespace SMemExperimental
{

void smem_statement_container::create_tables()
{
    add_structure("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    add_structure("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    add_structure("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY, symbol_type INTEGER)");
    add_structure("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY, symbol_value INTEGER)");
    add_structure("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY, symbol_value REAL)");
    add_structure("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY, symbol_value TEXT)");
    add_structure("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY, total_augmentations INTEGER, activation_value REAL, activations_total INTEGER, activations_last INTEGER, activations_first INTEGER)");
    add_structure("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY, t1 INTEGER, t2 INTEGER, t3 INTEGER, t4 INTEGER, t5 INTEGER, t6 INTEGER, t7 INTEGER, t8 INTEGER, t9 INTEGER, t10 INTEGER)");
    add_structure("CREATE TABLE smem_augmentations (lti_id INTEGER, attribute_s_id INTEGER, value_constant_s_id INTEGER, value_lti_id INTEGER, activation_value REAL)");
    add_structure("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER, value_constant_s_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER, value_lti_id INTEGER, edge_frequency INTEGER)");
    add_structure("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)");

    // adding an ascii table just to make lti queries easier when inspecting database
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

void smem_statement_container::create_indices()
{
    add_structure("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    add_structure("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    add_structure("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    add_structure("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    add_structure("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id)");
    add_structure("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    add_structure("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    add_structure("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    add_structure("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
}

void smem_statement_container::drop_tables()
{
    DB.exec("DROP TABLE IF EXISTS smem_persistent_variables");
    DB.exec("DROP TABLE IF EXISTS smem_symbols_type");
    DB.exec("DROP TABLE IF EXISTS smem_symbols_integer");
    DB.exec("DROP TABLE IF EXISTS smem_symbols_float");
    DB.exec("DROP TABLE IF EXISTS smem_symbols_string");
    DB.exec("DROP TABLE IF EXISTS smem_lti");
    DB.exec("DROP TABLE IF EXISTS smem_activation_history");
    DB.exec("DROP TABLE IF EXISTS smem_augmentations");
    DB.exec("DROP TABLE IF EXISTS smem_attribute_frequency");
    DB.exec("DROP TABLE IF EXISTS smem_wmes_constant_frequency");
    DB.exec("DROP TABLE IF EXISTS smem_wmes_lti_frequency");
    DB.exec("DROP TABLE IF EXISTS smem_ascii");
}

smem_statement_container::smem_statement_container(SMem_Manager* SMem)
: statement_container(SMem->DB, [this,SMem]() {
    // Delete all entries from the tables in the database if append setting is off
    if (SMem->settings->append_db->get_value() == off)
    {
        print_sysparam_trace(SMem->thisAgent, 0, "Erasing contents of semantic memory database. (append = off)\n");
        drop_tables();
    }

    create_tables();
    create_indices();

    // Update the version number
    add_structure("REPLACE INTO versions (system, version_number) VALUES ('smem_schema'," SMEM_SCHEMA_VERSION ")");

    if (!DB.containsData())
        createStructure();
}),

begin(DB, "BEGIN"),
commit(DB, "COMMIT"),

rollback(DB, "ROLLBACK"),

var_get(DB, "SELECT variable_value FROM smem_persistent_variables WHERE variable_id=?"),
var_set(DB, "UPDATE smem_persistent_variables SET variable_value=? WHERE variable_id=?"),
var_create(DB, "INSERT INTO smem_persistent_variables (variable_id,variable_value) VALUES (?,?)"),


hash_rev_int(DB, "SELECT symbol_value FROM smem_symbols_integer WHERE s_id=?"),
hash_rev_float(DB, "SELECT symbol_value FROM smem_symbols_float WHERE s_id=?"),
hash_rev_str(DB, "SELECT symbol_value FROM smem_symbols_string WHERE s_id=?"),
hash_rev_type(DB, "SELECT symbol_type FROM smem_symbols_type WHERE s_id=?"),

hash_get_int(DB, "SELECT s_id FROM smem_symbols_integer WHERE symbol_value=?"),
hash_get_float(DB, "SELECT s_id FROM smem_symbols_float WHERE symbol_value=?"),
hash_get_str(DB, "SELECT s_id FROM smem_symbols_string WHERE symbol_value=?"),

hash_add_type(DB, "INSERT INTO smem_symbols_type (symbol_type) VALUES (?)"),
hash_add_int(DB, "INSERT INTO smem_symbols_integer (s_id,symbol_value) VALUES (?,?)"),
hash_add_float(DB, "INSERT INTO smem_symbols_float (s_id,symbol_value) VALUES (?,?)"),
hash_add_str(DB, "INSERT INTO smem_symbols_string (s_id,symbol_value) VALUES (?,?)"),

lti_id_exists(DB, "SELECT lti_id FROM smem_lti WHERE lti_id=?"),
lti_id_max(DB, "SELECT MAX(lti_id) FROM smem_lti"),
lti_add(DB, "INSERT INTO smem_lti (lti_id, total_augmentations,activation_value,activations_total,activations_last,activations_first) VALUES (?,?,?,?,?,?)"),
lti_access_get(DB, "SELECT activations_total, activations_last, activations_first FROM smem_lti WHERE lti_id=?"),
lti_access_set(DB, "UPDATE smem_lti SET activations_total=?, activations_last=?, activations_first=? WHERE lti_id=?"),
lti_get_t(DB, "SELECT lti_id FROM smem_lti WHERE activations_last=?"),

web_add(DB, "INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) VALUES (?,?,?,?,?)"),
web_truncate(DB, "DELETE FROM smem_augmentations WHERE lti_id=?"),
web_expand(DB, "SELECT tsh_a.symbol_type AS attr_type, tsh_a.s_id AS attr_hash, vcl.symbol_type AS value_type, vcl.s_id AS value_hash, vcl.value_lti_id AS value_lti FROM ((smem_augmentations w LEFT JOIN smem_symbols_type tsh_v ON w.value_constant_s_id=tsh_v.s_id) vc LEFT JOIN smem_lti AS lti ON vc.value_lti_id=lti.lti_id) vcl INNER JOIN smem_symbols_type tsh_a ON vcl.attribute_s_id=tsh_a.s_id WHERE vcl.lti_id=?"),

web_all(DB, "SELECT attribute_s_id, value_constant_s_id, value_lti_id FROM smem_augmentations WHERE lti_id=?"),

web_attr_all(DB, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? ORDER BY activation_value DESC"),
web_const_all(DB, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=? AND value_lti_id=" SMEM_AUGMENTATIONS_NULL_STR " ORDER BY activation_value DESC"),
web_lti_all(DB, "SELECT lti_id, activation_value FROM smem_augmentations w WHERE attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=? ORDER BY activation_value DESC"),

web_attr_child(DB, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=?"),
web_const_child(DB, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=?"),
web_lti_child(DB, "SELECT lti_id, value_constant_s_id FROM smem_augmentations WHERE lti_id=? AND attribute_s_id=? AND value_constant_s_id=" SMEM_AUGMENTATIONS_NULL_STR " AND value_lti_id=?"),

attribute_frequency_check(DB, "SELECT edge_frequency FROM smem_attribute_frequency WHERE attribute_s_id=?"),
wmes_constant_frequency_check(DB, "SELECT edge_frequency FROM smem_wmes_constant_frequency WHERE attribute_s_id=? AND value_constant_s_id=?"),
wmes_lti_frequency_check(DB, "SELECT edge_frequency FROM smem_wmes_lti_frequency WHERE attribute_s_id=? AND value_lti_id=?"),

attribute_frequency_add(DB, "INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) VALUES (?,1)"),
wmes_constant_frequency_add(DB, "INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) VALUES (?,?,1)"),
wmes_lti_frequency_add(DB, "INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) VALUES (?,?,1)"),

attribute_frequency_update(DB, "UPDATE smem_attribute_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=?"),
wmes_constant_frequency_update(DB, "UPDATE smem_wmes_constant_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=? AND value_constant_s_id=?"),
wmes_lti_frequency_update(DB, "UPDATE smem_wmes_lti_frequency SET edge_frequency = edge_frequency + ? WHERE attribute_s_id=? AND value_lti_id=?"),

attribute_frequency_get(DB, "SELECT edge_frequency FROM smem_attribute_frequency WHERE attribute_s_id=?"),
wmes_constant_frequency_get(DB, "SELECT edge_frequency FROM smem_wmes_constant_frequency WHERE attribute_s_id=? AND value_constant_s_id=?"),
wmes_lti_frequency_get(DB, "SELECT edge_frequency FROM smem_wmes_lti_frequency WHERE attribute_s_id=? AND value_lti_id=?"),

act_set(DB, "UPDATE smem_augmentations SET activation_value=? WHERE lti_id=?"),
act_lti_child_ct_get(DB, "SELECT total_augmentations FROM smem_lti WHERE lti_id=?"),
act_lti_child_ct_set(DB, "UPDATE smem_lti SET total_augmentations=? WHERE lti_id=?"),
act_lti_set(DB, "UPDATE smem_lti SET activation_value=? WHERE lti_id=?"),
act_lti_get(DB, "SELECT activation_value FROM smem_lti WHERE lti_id=?"),

history_get(DB, "SELECT t1,t2,t3,t4,t5,t6,t7,t8,t9,t10 FROM smem_activation_history WHERE lti_id=?"),
history_push(DB, "UPDATE smem_activation_history SET t10=t9,t9=t8,t8=t7,t8=t7,t7=t6,t6=t5,t5=t4,t4=t3,t3=t2,t2=t1,t1=? WHERE lti_id=?"),
history_add(DB, "INSERT INTO smem_activation_history (lti_id,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10) VALUES (?,?,0,0,0,0,0,0,0,0,0)"),

vis_lti(DB, "SELECT lti_id, activation_value FROM smem_lti ORDER BY lti_id ASC"),
vis_lti_act(DB, "SELECT activation_value FROM smem_lti WHERE lti_id=?"),
vis_value_const(DB, "SELECT lti_id, tsh1.symbol_type AS attr_type, tsh1.s_id AS attr_hash, tsh2.symbol_type AS val_type, tsh2.s_id AS val_hash FROM smem_augmentations w, smem_symbols_type tsh1, smem_symbols_type tsh2 WHERE (w.attribute_s_id=tsh1.s_id) AND (w.value_constant_s_id=tsh2.s_id)"),
vis_value_lti(DB, "SELECT lti_id, tsh.symbol_type AS attr_type, tsh.s_id AS attr_hash, value_lti_id FROM smem_augmentations w, smem_symbols_type tsh WHERE (w.attribute_s_id=tsh.s_id) AND (value_lti_id<>" SMEM_AUGMENTATIONS_NULL_STR ")")
{}

smem_statement_container::smem_statement_container(smem_statement_container&& other)
: statement_container(other.DB),
begin(std::move(other.begin)),
commit(std::move(other.commit)),
rollback(std::move(other.rollback)),

var_get(std::move(other.var_get)),
var_set(std::move(other.var_set)),
var_create(std::move(other.var_create)),

hash_rev_int(std::move(other.hash_rev_int)),
hash_rev_float(std::move(other.hash_rev_float)),
hash_rev_str(std::move(other.hash_rev_str)),
hash_rev_type(std::move(other.hash_rev_type)),
hash_get_int(std::move(other.hash_get_int)),
hash_get_float(std::move(other.hash_get_float)),
hash_get_str(std::move(other.hash_get_str)),
hash_add_type(std::move(other.hash_add_type)),
hash_add_int(std::move(other.hash_add_int)),
hash_add_float(std::move(other.hash_add_float)),
hash_add_str(std::move(other.hash_add_str)),

lti_id_exists(std::move(other.lti_id_exists)),
lti_id_max(std::move(other.lti_id_max)),
lti_add(std::move(other.lti_add)),
lti_access_get(std::move(other.lti_access_get)),
lti_access_set(std::move(other.lti_access_set)),
lti_get_t(std::move(other.lti_get_t)),

web_add(std::move(other.web_add)),
web_truncate(std::move(other.web_truncate)),
web_expand(std::move(other.web_expand)),

web_all(std::move(other.web_all)),

web_attr_all(std::move(other.web_attr_all)),
web_const_all(std::move(other.web_const_all)),
web_lti_all(std::move(other.web_lti_all)),

web_attr_child(std::move(other.web_attr_child)),
web_const_child(std::move(other.web_const_child)),
web_lti_child(std::move(other.web_lti_child)),

attribute_frequency_check(std::move(other.attribute_frequency_check)),
wmes_constant_frequency_check(std::move(other.wmes_constant_frequency_check)),
wmes_lti_frequency_check(std::move(other.wmes_lti_frequency_check)),

attribute_frequency_add(std::move(other.attribute_frequency_add)),
wmes_constant_frequency_add(std::move(other.wmes_constant_frequency_add)),
wmes_lti_frequency_add(std::move(other.wmes_lti_frequency_add)),

attribute_frequency_update(std::move(other.attribute_frequency_update)),
wmes_constant_frequency_update(std::move(other.wmes_constant_frequency_update)),
wmes_lti_frequency_update(std::move(other.wmes_lti_frequency_update)),

attribute_frequency_get(std::move(other.attribute_frequency_get)),
wmes_constant_frequency_get(std::move(other.wmes_constant_frequency_get)),
wmes_lti_frequency_get(std::move(other.wmes_lti_frequency_get)),

act_set(std::move(other.act_set)),
act_lti_child_ct_set(std::move(other.act_lti_child_ct_set)),
act_lti_child_ct_get(std::move(other.act_lti_child_ct_get)),
act_lti_set(std::move(other.act_lti_set)),
act_lti_get(std::move(other.act_lti_get)),

history_get(std::move(other.history_get)),
history_push(std::move(other.history_push)),
history_add(std::move(other.history_add)),

vis_lti(std::move(other.vis_lti)),
vis_lti_act(std::move(other.vis_lti_act)),
vis_value_const(std::move(other.vis_value_const)),
vis_value_lti(std::move(other.vis_value_lti))
{}

smem_statement_container& smem_statement_container::operator=(smem_statement_container&& other)
{
    structure = std::move(other.structure);
    DB = std::move(other.DB);

    begin = std::move(other.begin);
    commit = std::move(other.commit);
    rollback = std::move(other.rollback);

    var_get = std::move(other.var_get);
    var_set = std::move(other.var_set);
    var_create = std::move(other.var_create);

    hash_rev_int = std::move(other.hash_rev_int);
    hash_rev_float = std::move(other.hash_rev_float);
    hash_rev_str = std::move(other.hash_rev_str);
    hash_rev_type = std::move(other.hash_rev_type);
    hash_get_int = std::move(other.hash_get_int);
    hash_get_float = std::move(other.hash_get_float);
    hash_get_str = std::move(other.hash_get_str);
    hash_add_type = std::move(other.hash_add_type);
    hash_add_int = std::move(other.hash_add_int);
    hash_add_float = std::move(other.hash_add_float);
    hash_add_str = std::move(other.hash_add_str);

    lti_id_exists = std::move(other.lti_id_exists);
    lti_id_max = std::move(other.lti_id_max);
    lti_add = std::move(other.lti_add);
    lti_access_get = std::move(other.lti_access_get);
    lti_access_set = std::move(other.lti_access_set);
    lti_get_t = std::move(other.lti_get_t);

    web_add = std::move(other.web_add);
    web_truncate = std::move(other.web_truncate);
    web_expand = std::move(other.web_expand);

    web_all = std::move(other.web_all);

    web_attr_all = std::move(other.web_attr_all);
    web_const_all = std::move(other.web_const_all);
    web_lti_all = std::move(other.web_lti_all);

    web_attr_child = std::move(other.web_attr_child);
    web_const_child = std::move(other.web_const_child);
    web_lti_child = std::move(other.web_lti_child);

    attribute_frequency_check = std::move(other.attribute_frequency_check);
    wmes_constant_frequency_check = std::move(other.wmes_constant_frequency_check);
    wmes_lti_frequency_check = std::move(other.wmes_lti_frequency_check);

    attribute_frequency_add = std::move(other.attribute_frequency_add);
    wmes_constant_frequency_add = std::move(other.wmes_constant_frequency_add);
    wmes_lti_frequency_add = std::move(other.wmes_lti_frequency_add);

    attribute_frequency_update = std::move(other.attribute_frequency_update);
    wmes_constant_frequency_update = std::move(other.wmes_constant_frequency_update);
    wmes_lti_frequency_update = std::move(other.wmes_lti_frequency_update);

    attribute_frequency_get = std::move(other.attribute_frequency_get);
    wmes_constant_frequency_get = std::move(other.wmes_constant_frequency_get);
    wmes_lti_frequency_get = std::move(other.wmes_lti_frequency_get);

    act_set = std::move(other.act_set);
    act_lti_child_ct_set = std::move(other.act_lti_child_ct_set);
    act_lti_child_ct_get = std::move(other.act_lti_child_ct_get);
    act_lti_set = std::move(other.act_lti_set);
    act_lti_get = std::move(other.act_lti_get);

    history_get = std::move(other.history_get);
    history_push = std::move(other.history_push);
    history_add = std::move(other.history_add);
    
    vis_lti = std::move(other.vis_lti);
    vis_lti_act = std::move(other.vis_lti_act);
    vis_value_const = std::move(other.vis_value_const);
    vis_value_lti = std::move(other.vis_value_lti);

    return *this;
}

};

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
    smem_hash_id rowID = 0;

    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_add_type);

        SQLite::bind(*sql, symbol_type);
        sql->exec();

        rowID = JobQueue.db.getLastInsertRowid();
    })->wait();

    return rowID;
}

smem_hash_id SMem_Manager::hash_int(int64_t val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_get_int);

        SQLite::bind(*sql, val);
        if (sql->executeStep())
            return_val = sql->getColumn(0).getUInt64();

        // See DID: SQLITE_LOCKED (search for 'DID: SQLITE_LOCKED')
        sql->reset();

        if (!return_val && add_on_fail)
        {
            JobQueue.post([&]() mutable {
                // type first
                return_val = hash_add_type(INT_CONSTANT_SYMBOL_TYPE);

                // then content
                auto sql = sqlite_thread_guard(SQL.hash_add_int);

                SQLite::bind(*sql, return_val, val);
                sql->exec();
            })->wait();
        }
    })->wait();

    return return_val;
}

smem_hash_id SMem_Manager::hash_float(double val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_get_float);

        SQLite::bind(*sql, val);
        if (sql->executeStep())
            return_val = sql->getColumn(0).getUInt64();

        // See DID: SQLITE_LOCKED (search for 'DID: SQLITE_LOCKED')
        sql->reset();

        if (!return_val && add_on_fail)
        {
            JobQueue.post([&]() mutable {
                // type first
                return_val = hash_add_type(FLOAT_CONSTANT_SYMBOL_TYPE);

                // then content
                auto sql = sqlite_thread_guard(SQL.hash_add_float);

                SQLite::bind(*sql, return_val, val);
                sql->exec();
            })->wait();
        }
    })->wait();

    return return_val;
}

smem_hash_id SMem_Manager::hash_str(char* val, bool add_on_fail)
{
    smem_hash_id return_val = NIL;

    // search first
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_get_str);

        SQLite::bind(*sql, val);
        if (sql->executeStep())
            return_val = sql->getColumn(0).getUInt64();

        // See DID: SQLITE_LOCKED (search for 'DID: SQLITE_LOCKED')
        sql->reset();

        if (!return_val && add_on_fail)
        {
            JobQueue.post([&]() mutable {
                // type first
                return_val = hash_add_type(STR_CONSTANT_SYMBOL_TYPE);

                // then content
                auto sql = sqlite_thread_guard(SQL.hash_add_str);

                SQLite::bind(*sql, return_val, val);
                sql->exec();
            })->wait();
        }
    })->wait();

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

    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_rev_int);

        SQLite::bind(*sql, hash_value);
        assert(sql->executeStep());
        return_val = sql->getColumn(0).getInt64();
    })->wait();

    return return_val;
}

double SMem_Manager::rhash__float(smem_hash_id hash_value)
{
    double return_val = NIL;

    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_rev_float);

        SQLite::bind(*sql, hash_value);
        assert(sql->executeStep());
        return_val = sql->getColumn(0).getDouble();
    })->wait();

    return return_val;
}

void SMem_Manager::rhash__str(smem_hash_id hash_value, std::string& dest)
{
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.hash_rev_str);

        SQLite::bind(*sql, hash_value);
        assert(sql->executeStep());
        dest = sql->getColumn(0).getString();
    })->wait();
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

    std::string db_path;
    bool tabula_rasa = false;

    if (settings->database->get_value() == smem_param_container::memory)
    {
        db_path = SMem_Manager::memoryDatabasePath;
        tabula_rasa = true;
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory database in memory.\n");
    }
    else
    {
        db_path = settings->path->get_value();
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory memory database at %s\n", db_path.c_str());
    }

    // attempt connection
    recreateDB(db_path);

    // If the database is on file, make sure the database contents use the current schema
    // If it does not, switch to memory-based database

    if (db_path == SMem_Manager::memoryDatabasePath) // Check if database mode is to a file
    {
        bool switch_to_memory;
        std::string schema_version, version_error_message;

        /* -- Set switch_to_memory true in case we have any errors with the database -- */
        switch_to_memory = true;

        if (!DB.containsData())
        {
            print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "...semantic memory database is new.\n");
            switch_to_memory = false;
            tabula_rasa = true;
        }
        else
        {
            // Check if table exists already
            if (DB.tableExists("versions"))
            {
                schema_version = DB.execAndGet("SELECT version_number FROM versions WHERE system = 'smem_schema'").getString();

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
                version_error_message.assign("...Error: Cannot load a semantic memory database with an old schema version.\n...Please convert "
                                             "old semantic memory database or start a new database by setting a new database file path.\n...Switching "
                                             "to memory-based database.\n");
            }
        }

        if (switch_to_memory)
        {
            // Memory mode will be set on, database will be disconnected to and then init_db
            // will be called again to reinitialize database.
            switch_to_memory_db(version_error_message);
            return;
        }
    }

    // apply performance options
    {
        // page_size
        {
            switch (settings->page_size->get_value())
            {
                case (smem_param_container::page_1k):
                    DB.exec("PRAGMA page_size = 1024");
                    break;

                case (smem_param_container::page_2k):
                    DB.exec("PRAGMA page_size = 2048");
                    break;

                case (smem_param_container::page_4k):
                    DB.exec("PRAGMA page_size = 4096");
                    break;

                case (smem_param_container::page_8k):
                    DB.exec("PRAGMA page_size = 8192");
                    break;

                case (smem_param_container::page_16k):
                    DB.exec("PRAGMA page_size = 16384");
                    break;

                case (smem_param_container::page_32k):
                    DB.exec("PRAGMA page_size = 32768");
                    break;

                case (smem_param_container::page_64k):
                    DB.exec("PRAGMA page_size = 65536");
                    break;
            }
        }

        // cache_size
        {
            std::string cache_sql("PRAGMA cache_size = ");
            char* str = settings->cache_size->get_string();
            cache_sql.append(str);
            free(str);
            str = NULL;
            DB.exec(cache_sql.c_str());
        }

        // optimization
        if (settings->opt->get_value() == smem_param_container::opt_speed)
        {
            // synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
            DB.exec("PRAGMA synchronous = OFF");

            // journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
            DB.exec("PRAGMA journal_mode = OFF");

            // locking_mode - no one else can view the database after our first write
            DB.exec("PRAGMA locking_mode = EXCLUSIVE");
        }
    }

    // update validation count
    smem_validation++;

    // setup common structures/queries
    SQL = SMemExperimental::smem_statement_container(this);

    // initialize persistent variables
    if (tabula_rasa || (settings->append_db->get_value() == off))
    {
//        SQL.begin.exec();
//        SQL.begin.reset();

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

//        SQL.commit.exec();
//        SQL.commit.reset();
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
//        SQL.begin.exec();
//        SQL.begin.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->init->stop();
    ////////////////////////////////////////////////////////////////////////////
}

// gets an SMem variable from the database
bool SMem_Manager::variable_get(smem_variable_key variable_id, int64_t* variable_value)
{
    bool result = false;

    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.var_get);

        SQLite::bind(*sql, variable_id);

        if (sql->executeStep())
        {
            *variable_value = sql->getColumn(0).getInt64();
            result = true;
        }
    })->wait();

    return result;
}

// sets an existing SMem variable in the database
void SMem_Manager::variable_set(smem_variable_key variable_id, int64_t variable_value)
{
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.var_set);

        SQLite::bind(*sql, variable_value, variable_id);
        sql->exec();
    })->wait();
}

// creates a new SMem variable in the database
void SMem_Manager::variable_create(smem_variable_key variable_id, int64_t variable_value)
{
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.var_create);

        SQLite::bind(*sql, variable_id, variable_value);
        sql->exec();
    })->wait();
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
//            SQL.commit.exec();
//            SQL.commit.reset();
        }

        // de-allocate common statements
        delete thisAgent->lastCue;
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
//            SQL.commit.exec();
//            SQL.commit.reset();
        }

        try {
            DB.backup(file_name);
            return_val = true;
        }
        catch (SQLite::Exception& e) {
            *err = e.getErrorStr();
        }

        if (settings->lazy_commit->get_value() == on)
        {
//            SQL.begin.exec();
//            SQL.begin.reset();
        }
    }
    else
        *err = "Semantic database is not currently connected.";

    return return_val;
}

void SMem_Manager::switch_to_memory_db(std::string& buf)
{
    print_sysparam_trace(thisAgent, 0, buf.c_str());
    settings->database->set_value(smem_param_container::memory);
    init_db();
}

void SMem_Manager::update_schema_one_to_two()
{
    DB.exec("BEGIN TRANSACTION");
    DB.exec("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY,symbol_type INTEGER)");
    DB.exec("INSERT INTO smem_symbols_type (s_id, symbol_type) SELECT id, sym_type FROM smem7_symbols_type");
    DB.exec("DROP TABLE smem7_symbols_type");

    DB.exec("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY,symbol_value TEXT)");
    DB.exec("INSERT INTO smem_symbols_string (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_str");
    DB.exec("DROP TABLE smem7_symbols_str");

    DB.exec("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY,symbol_value INTEGER)");
    DB.exec("INSERT INTO smem_symbols_integer (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_int");
    DB.exec("DROP TABLE smem7_symbols_int");

    DB.exec("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY,ascii_chr TEXT)");
    DB.exec("INSERT INTO smem_ascii (ascii_num, ascii_chr) SELECT ascii_num, ascii_num FROM smem7_ascii");
    DB.exec("DROP TABLE smem7_ascii");

    DB.exec("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY,symbol_value REAL)");
    DB.exec("INSERT INTO smem_symbols_float (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_float");
    DB.exec("DROP TABLE smem7_symbols_float");

    DB.exec("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY,total_augmentations INTEGER,activation_value REAL,activations_total INTEGER,activations_last INTEGER,activations_first INTEGER)");
    DB.exec("INSERT INTO smem_lti (lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first) SELECT id, child_ct, act_value, access_n, access_t, access_1 FROM smem7_lti");
    DB.exec("DROP TABLE smem7_lti");

    DB.exec("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY,t1 INTEGER,t2 INTEGER,t3 INTEGER,t4 INTEGER,t5 INTEGER,t6 INTEGER,t7 INTEGER,t8 INTEGER,t9 INTEGER,t10 INTEGER)");
    DB.exec("INSERT INTO smem_activation_history (lti_id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) SELECT id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 FROM smem7_history");
    DB.exec("DROP TABLE smem7_history");

    DB.exec("CREATE TABLE smem_augmentations (lti_id INTEGER,attribute_s_id INTEGER,value_constant_s_id INTEGER,value_lti_id INTEGER,activation_value REAL)");
    DB.exec("INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) SELECT parent_id, attr, val_const, val_lti, act_value FROM smem7_web");
    DB.exec("DROP TABLE smem7_web");

    DB.exec("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY,edge_frequency INTEGER)");
    DB.exec("INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) SELECT attr, ct FROM smem7_ct_attr");
    DB.exec("DROP TABLE smem7_ct_attr");

    DB.exec("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER,value_constant_s_id INTEGER,edge_frequency INTEGER)");
    DB.exec("INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) SELECT attr, val_const, ct FROM smem7_ct_const");
    DB.exec("DROP TABLE smem7_ct_const");

    DB.exec("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER,value_lti_id INTEGER,edge_frequency INTEGER)");
    DB.exec("INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) SELECT attr, val_lti, ct FROM smem7_ct_lti");
    DB.exec("DROP TABLE smem7_ct_lti");

    DB.exec("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    DB.exec("INSERT INTO smem_persistent_variables (variable_id, variable_value) SELECT id, value FROM smem7_vars");
    DB.exec("DROP TABLE smem7_vars");

    DB.exec("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    DB.exec("INSERT INTO versions (system, version_number) VALUES ('smem_schema','2.0')");
    DB.exec("DROP TABLE smem7_signature");

    DB.exec("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    DB.exec("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
    DB.exec("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    DB.exec("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    DB.exec("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    DB.exec("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id,value_lti_id)");
    DB.exec("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    DB.exec("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    DB.exec("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    DB.exec("COMMIT");
}

uint64_t SMem_Manager::lti_exists(uint64_t pLTI_ID)
{
    uint64_t return_val = NIL;

    if (connected())
    {
        JobQueue.post([&]() mutable {
            auto sql = sqlite_thread_guard(SQL.lti_id_exists);

            SQLite::bind(*sql, pLTI_ID);

            if (sql->executeStep())
                return_val = sql->getColumn(0).getUInt64();
        })->wait();
    }

    return return_val;
}

uint64_t SMem_Manager::get_max_lti_id()
{
    uint64_t return_val = 0;

    if (connected())
    {
        JobQueue.post([&]() mutable {
            auto sql = sqlite_thread_guard(SQL.lti_id_max);

            if (sql->executeStep())
                return_val = sql->getColumn(0).getUInt64();
        })->wait();
    }

    return return_val;
}

uint64_t SMem_Manager::add_new_LTI()
{
    uint64_t lti_id = ++lti_id_counter;
    while (lti_exists(lti_id))
    {
        lti_id = ++lti_id_counter;
    }

    // add lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first
    JobQueue.post([=]() mutable {
        auto sql = sqlite_thread_guard(SQL.lti_add);

        SQLite::bind(*sql, lti_id, 0, 0, 0, 0, 0);
        sql->exec();

    //    assert(lti_id_counter == smem_db->last_insert_rowid());
    })->wait();

    statistics->nodes->set_value(statistics->nodes->get_value() + 1);

    return lti_id_counter;
}

uint64_t SMem_Manager::add_specific_LTI(uint64_t lti_id)
{
    // add lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first

    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.lti_add);

        SQLite::bind(*sql, lti_id, 0, 0, 0, 0, 0);
        sql->exec();

        //    assert(lti_id_counter == smem_db->last_insert_rowid());
    })->wait();

    statistics->nodes->set_value(statistics->nodes->get_value() + 1);

    return lti_id;
}
