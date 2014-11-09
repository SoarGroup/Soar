#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-SMem
 * =======================================================================
 */

#include "semantic_memory.h"
#include "agent.h"
#include "prefmem.h"
#include "symtab.h"
#include "wmem.h"
#include "print.h"
#include "xml.h"
#include "lexer.h"
#include "instantiations.h"
#include "rhs.h"
#include "decide.h"
#include "test.h"
#include "tempmem.h"
#include "variablization_manager.h"

#include <list>
#include <map>
#include <queue>
#include <utility>
#include <ctype.h>
#include <fstream>
#include <algorithm>

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters                   smem::param
// stats                        smem::stats
// timers                       smem::timers
// statements                   smem::statements

// wmes                         smem::wmes

// variables                    smem::var
// temporal hash                smem::hash
// activation                   smem::act
// long-term identifiers        smem::lti

// storage                      smem::storage
// non-cue-based retrieval      smem::ncb
// cue-based retrieval          smem::cbr

// initialization               smem::init
// parsing                      smem::parse
// api                          smem::api

// visualization                smem::viz


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (smem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_param_container::smem_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // learning
    learning = new soar_module::boolean_param("learning", off, new soar_module::f_predicate<boolean>());
    add(learning);

    // database
    database = new soar_module::constant_param<db_choices>("database", memory, new soar_module::f_predicate<db_choices>());
    database->add_mapping(memory, "memory");
    database->add_mapping(file, "file");
    add(database);

    // append database or dump data on init
    append_db = new soar_module::boolean_param("append", on, new soar_module::f_predicate<boolean>());
    add(append_db);

    // path
    path = new smem_path_param("path", "", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>(), thisAgent);
    add(path);

    // auto-commit
    lazy_commit = new soar_module::boolean_param("lazy-commit", on, new smem_db_predicate<boolean>(thisAgent));
    add(lazy_commit);

    // timers
    timers = new soar_module::constant_param<soar_module::timer::timer_level>("timers", soar_module::timer::zero, new soar_module::f_predicate<soar_module::timer::timer_level>());
    timers->add_mapping(soar_module::timer::zero, "off");
    timers->add_mapping(soar_module::timer::one, "one");
    timers->add_mapping(soar_module::timer::two, "two");
    timers->add_mapping(soar_module::timer::three, "three");
    add(timers);

    // page_size
    page_size = new soar_module::constant_param<page_choices>("page-size", page_8k, new smem_db_predicate<page_choices>(thisAgent));
    page_size->add_mapping(page_1k, "1k");
    page_size->add_mapping(page_2k, "2k");
    page_size->add_mapping(page_4k, "4k");
    page_size->add_mapping(page_8k, "8k");
    page_size->add_mapping(page_16k, "16k");
    page_size->add_mapping(page_32k, "32k");
    page_size->add_mapping(page_64k, "64k");
    add(page_size);

    // cache_size
    cache_size = new soar_module::integer_param("cache-size", 10000, new soar_module::gt_predicate<int64_t>(1, true), new smem_db_predicate<int64_t>(thisAgent));
    add(cache_size);

    // opt
    opt = new soar_module::constant_param<opt_choices>("optimization", opt_speed, new smem_db_predicate<opt_choices>(thisAgent));
    opt->add_mapping(opt_safety, "safety");
    opt->add_mapping(opt_speed, "performance");
    add(opt);

    // thresh
    thresh = new soar_module::integer_param("thresh", 100, new soar_module::predicate<int64_t>(), new smem_db_predicate<int64_t>(thisAgent));
    add(thresh);

    // merge
    merge = new soar_module::constant_param<merge_choices>("merge", merge_add, new soar_module::f_predicate<merge_choices>());
    merge->add_mapping(merge_none, "none");
    merge->add_mapping(merge_add, "add");
    add(merge);

    // activate_on_query
    activate_on_query = new soar_module::boolean_param("activate-on-query", on, new soar_module::f_predicate<boolean>());
    add(activate_on_query);

    // activation_mode
    activation_mode = new soar_module::constant_param<act_choices>("activation-mode", act_recency, new soar_module::f_predicate<act_choices>());
    activation_mode->add_mapping(act_recency, "recency");
    activation_mode->add_mapping(act_frequency, "frequency");
    activation_mode->add_mapping(act_base, "base-level");
    add(activation_mode);

    // base_decay
    base_decay = new soar_module::decimal_param("base-decay", 0.5, new soar_module::gt_predicate<double>(0, false), new soar_module::f_predicate<double>());
    add(base_decay);

    // base_update_policy
    base_update = new soar_module::constant_param<base_update_choices>("base-update-policy", bupt_stable, new soar_module::f_predicate<base_update_choices>());
    base_update->add_mapping(bupt_stable, "stable");
    base_update->add_mapping(bupt_naive, "naive");
    base_update->add_mapping(bupt_incremental, "incremental");
    add(base_update);

    // incremental update thresholds
    base_incremental_threshes = new soar_module::int_set_param("base-incremental-threshes", new soar_module::f_predicate< int64_t >());
    add(base_incremental_threshes);

    // mirroring
    mirroring = new soar_module::boolean_param("mirroring", off, new smem_db_predicate< boolean >(thisAgent));
    add(mirroring);
}

//

/* This is a test of whether or not the SMEM database with no version number is the one
that smem_update_schema_one_to_two can convert.  It tests for the existence of a table name to determine if this is the old version. */
inline bool smem_version_one(agent* thisAgent)
{
    double check_num_tables;
    thisAgent->smem_db->sql_simple_get_float("SELECT count(type) FROM sqlite_master WHERE type='table' AND name='smem7_signature'", check_num_tables);
    if (check_num_tables == 0)
    {
        return false;
    }
    return true;
}

smem_path_param::smem_path_param(const char* new_name, const char* new_value, soar_module::predicate<const char*>* new_val_pred, soar_module::predicate<const char*>* new_prot_pred, agent* new_agent): soar_module::string_param(new_name, new_value, new_val_pred, new_prot_pred), thisAgent(new_agent) {}

void smem_path_param::set_value(const char* new_value)
{
    /* The first time path is set, we check that the the database is the right version,
     * so you can warn someone before they try to use it that conversion will take some
     * time. That way, they can then switch to another before dedicating that time. */
    value->assign(new_value);

    const char* db_path;
    db_path = thisAgent->smem_params->path->get_value();
    bool attempt_connection_here = thisAgent->smem_db->get_status() == soar_module::disconnected;
    if (attempt_connection_here)
    {
        thisAgent->smem_db->connect(db_path);
    }

    if (thisAgent->smem_db->get_status() == soar_module::problem)
    {
        print_sysparam_trace(thisAgent, 0, "Semantic memory database Error: %s\n", thisAgent->smem_db->get_errmsg());
    }
    else
    {
        // temporary queries for one-time init actions
        soar_module::sqlite_statement* temp_q = NULL;

        // If the database is on file, make sure the database contents use the current schema
        // If it does not, switch to memory-based database

        if (strcmp(db_path, ":memory:")) // Only worry about database version if writing to disk
        {
            bool sql_is_new;
            std::string schema_version;
            if (thisAgent->smem_db->sql_is_new_db(sql_is_new))
            {
                if (!sql_is_new)
                {
                    // Check if table exists already
                    temp_q = new soar_module::sqlite_statement(thisAgent->smem_db, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (!thisAgent->smem_db->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'smem_schema'", schema_version))
                        {
                            if (smem_version_one(thisAgent))
                            {
                                print(thisAgent, "...You have selected a database with an old version.\n"
                                      "...If you proceed, the database will be converted to a\n"
                                      "...new version when the database is initialized.\n"
                                      "...Conversion can take a large amount of time with large databases.\n");
                            }
                        }
                    }
                }
            }
        }

        delete temp_q;
        temp_q = NULL;
    }
    if (attempt_connection_here)
    {
        thisAgent->smem_db->disconnect();
    }
}

//

template <typename T>
smem_db_predicate<T>::smem_db_predicate(agent* new_agent): soar_module::agent_predicate<T>(new_agent) {}

template <typename T>
bool smem_db_predicate<T>::operator()(T /*val*/)
{
    return (this->thisAgent->smem_db->get_status() == soar_module::connected);
}


bool smem_enabled(agent* thisAgent)
{
    return (thisAgent->smem_params->learning->get_value() == on);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (smem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_stat_container::smem_stat_container(agent* new_agent): soar_module::stat_container(new_agent)
{
    // db-lib-version
    db_lib_version = new smem_db_lib_version_stat(thisAgent, "db-lib-version", NULL, new soar_module::predicate< const char* >());
    add(db_lib_version);

    // mem-usage
    mem_usage = new smem_mem_usage_stat(thisAgent, "mem-usage", 0, new soar_module::predicate<int64_t>());
    add(mem_usage);

    // mem-high
    mem_high = new smem_mem_high_stat(thisAgent, "mem-high", 0, new soar_module::predicate<int64_t>());
    add(mem_high);

    //

    // expansions
    expansions = new soar_module::integer_stat("retrieves", 0, new soar_module::f_predicate<int64_t>());
    add(expansions);

    // cue-based-retrievals
    cbr = new soar_module::integer_stat("queries", 0, new soar_module::f_predicate<int64_t>());
    add(cbr);

    // stores
    stores = new soar_module::integer_stat("stores", 0, new soar_module::f_predicate<int64_t>());
    add(stores);

    // activations
    act_updates = new soar_module::integer_stat("act_updates", 0, new soar_module::f_predicate<int64_t>());
    add(act_updates);

    // mirrors
    mirrors = new soar_module::integer_stat("mirrors", 0, new soar_module::f_predicate<int64_t>());
    add(mirrors);

    //

    // chunks
    chunks = new soar_module::integer_stat("nodes", 0, new smem_db_predicate< int64_t >(thisAgent));
    add(chunks);

    // slots
    slots = new soar_module::integer_stat("edges", 0, new smem_db_predicate< int64_t >(thisAgent));
    add(slots);
}

//

smem_db_lib_version_stat::smem_db_lib_version_stat(agent* new_agent, const char* new_name, const char* new_value, soar_module::predicate< const char* >* new_prot_pred): soar_module::primitive_stat< const char* >(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

const char* smem_db_lib_version_stat::get_value()
{
    return thisAgent->smem_db->lib_version();
}

//

smem_mem_usage_stat::smem_mem_usage_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t smem_mem_usage_stat::get_value()
{
    return thisAgent->smem_db->memory_usage();
}

//

smem_mem_high_stat::smem_mem_high_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t smem_mem_high_stat::get_value()
{
    return thisAgent->smem_db->memory_highwater();
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (smem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_timer_container::smem_timer_container(agent* new_agent): soar_module::timer_container(new_agent)
{
    // one

    total = new smem_timer("_total", thisAgent, soar_module::timer::one);
    add(total);

    // two

    storage = new smem_timer("smem_storage", thisAgent, soar_module::timer::two);
    add(storage);

    ncb_retrieval = new smem_timer("smem_ncb_retrieval", thisAgent, soar_module::timer::two);
    add(ncb_retrieval);

    query = new smem_timer("smem_query", thisAgent, soar_module::timer::two);
    add(query);

    api = new smem_timer("smem_api", thisAgent, soar_module::timer::two);
    add(api);

    init = new smem_timer("smem_init", thisAgent, soar_module::timer::two);
    add(init);

    hash = new smem_timer("smem_hash", thisAgent, soar_module::timer::two);
    add(hash);

    act = new smem_timer("three_activation", thisAgent, soar_module::timer::three);
    add(act);
}

//

smem_timer_level_predicate::smem_timer_level_predicate(agent* new_agent): soar_module::agent_predicate<soar_module::timer::timer_level>(new_agent) {}

bool smem_timer_level_predicate::operator()(soar_module::timer::timer_level val)
{
    return (thisAgent->smem_params->timers->get_value() >= val);
}

//

smem_timer::smem_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level): soar_module::timer(new_name, new_agent, new_level, new smem_timer_level_predicate(new_agent)) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statement Functions (smem::statements)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
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
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_persistent_variables");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_type");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_integer");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_float");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_symbols_string");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_lti");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_activation_history");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_augmentations");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_attribute_frequency");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_wmes_constant_frequency");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_wmes_lti_frequency");
    new_agent->smem_db->sql_execute("DROP TABLE IF EXISTS smem_ascii");
}

smem_statement_container::smem_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->smem_db)
{
    soar_module::sqlite_database* new_db = new_agent->smem_db;

    // Delete all entries from the tables in the database if append setting is off
    if (new_agent->smem_params->append_db->get_value() == off)
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

    web_expand = new soar_module::sqlite_statement(new_db, "SELECT tsh_a.symbol_type AS attr_type, tsh_a.s_id AS attr_hash, vcl.symbol_type AS value_type, vcl.s_id AS value_hash, vcl.soar_letter AS value_letter, vcl.soar_number AS value_num, vcl.value_lti_id AS value_lti FROM ((smem_augmentations w LEFT JOIN smem_symbols_type tsh_v ON w.value_constant_s_id=tsh_v.s_id) vc LEFT JOIN smem_lti AS lti ON vc.value_lti_id=lti.lti_id) vcl INNER JOIN smem_symbols_type tsh_a ON vcl.attribute_s_id=tsh_a.s_id WHERE lti_id=?");
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
// WME Functions (smem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

smem_wme_list* smem_get_direct_augs_of_id(Symbol* id, tc_number tc = NIL)
{
    slot* s;
    wme* w;
    smem_wme_list* return_val = new smem_wme_list;

    // augs only exist for identifiers
    if (id->is_identifier())
    {
        if (tc != NIL)
        {
            if (tc == id->tc_num)
            {
                return return_val;
            }
            else
            {
                id->tc_num = tc;
            }
        }

        // impasse wmes
        for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        {
            if (!w->acceptable)
            {
                return_val->push_back(w);
            }
        }

        // input wmes
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            return_val->push_back(w);
        }

        // regular wmes
        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (!w->acceptable)
                {
                    return_val->push_back(w);
                }
            }
        }
    }

    return return_val;
}

inline void _smem_process_buffered_wme_list(agent* thisAgent, Symbol* state, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& my_list, bool meta)
{
    if (my_list.empty())
    {
        return;
    }

    instantiation* inst = soar_module::make_fake_instantiation(thisAgent, state, &cue_wmes, &my_list);
    for (preference* pref = inst->preferences_generated; pref; pref = pref->inst_next)
    {
        // add the preference to temporary memory
        add_preference_to_tm(thisAgent, pref);
            // and add it to the list of preferences to be removed
            // when the goal is removed
            insert_at_head_of_dll(state->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
            pref->on_goal_list = true;

            if (meta)
            {
                // if this is a meta wme, then it is completely local
                // to the state and thus we will manually remove it
                // (via preference removal) when the time comes
                state->id->smem_info->smem_wmes->push_back(pref);
            }
        }

    if (!meta)
    {
        // otherwise, we submit the fake instantiation to backtracing
        // such as to potentially produce justifications that can follow
        // it to future adventures (potentially on new states)
        instantiation* my_justification_list = NIL;
        dprint(DT_FUNC_PRODUCTIONS, "Calling chunk instantiation from _smem_process_buffered_wme_list...\n");
//        thisAgent->variablizationManager->add_ltis_to_dnvl_for_conditions(inst->top_of_instantiated_conditions);
        thisAgent->variablizationManager->add_ltis_to_dnvl_for_prefs(inst->preferences_generated);
        chunk_instantiation(thisAgent, inst, false, &my_justification_list, true);
        thisAgent->variablizationManager->clear_dnvl();

        // if any justifications are created, assert their preferences manually
        // (copied mainly from assert_new_preferences with respect to our circumstances)
        if (my_justification_list != NIL)
        {
            preference* just_pref = NIL;
            instantiation* next_justification = NIL;

            for (instantiation* my_justification = my_justification_list;
                    my_justification != NIL;
                    my_justification = next_justification)
            {
                next_justification = my_justification->next;

                if (my_justification->in_ms)
                {
                    insert_at_head_of_dll(my_justification->prod->instantiations, my_justification, next, prev);
                }

                for (just_pref = my_justification->preferences_generated; just_pref != NIL; just_pref = just_pref->inst_next)
                {
                    add_preference_to_tm(thisAgent, just_pref);
                        if (wma_enabled(thisAgent))
                        {
                            wma_activate_wmes_in_pref(thisAgent, just_pref);
                        }
                    }
                }
            }
        }
}

inline void smem_process_buffered_wmes(agent* thisAgent, Symbol* state, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes)
{
    _smem_process_buffered_wme_list(thisAgent, state, cue_wmes, meta_wmes, true);
    _smem_process_buffered_wme_list(thisAgent, state, cue_wmes, retrieval_wmes, false);
}

inline void smem_buffer_add_wme(agent* thisAgent, soar_module::symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value)
{
    my_list.push_back(new soar_module::symbol_triple(id, attr, value));

    symbol_add_ref(thisAgent, id);
    symbol_add_ref(thisAgent, attr);
    symbol_add_ref(thisAgent, value);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Variable Functions (smem::var)
//
// Variables are key-value pairs stored in the database
// that are necessary to maintain a store between
// multiple runs of Soar.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// gets an SMem variable from the database
inline bool smem_variable_get(agent* thisAgent, smem_variable_key variable_id, int64_t* variable_value)
{
    soar_module::exec_result status;
    soar_module::sqlite_statement* var_get = thisAgent->smem_stmts->var_get;

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
inline void smem_variable_set(agent* thisAgent, smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_set = thisAgent->smem_stmts->var_set;

    var_set->bind_int(1, variable_value);
    var_set->bind_int(2, variable_id);

    var_set->execute(soar_module::op_reinit);
}

// creates a new SMem variable in the database
inline void smem_variable_create(agent* thisAgent, smem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_create = thisAgent->smem_stmts->var_create;

    var_create->bind_int(1, variable_id);
    var_create->bind_int(2, variable_value);

    var_create->execute(soar_module::op_reinit);
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

inline smem_hash_id smem_temporal_hash_add_type(agent* thisAgent, byte symbol_type)
{
    thisAgent->smem_stmts->hash_add_type->bind_int(1, symbol_type);
    thisAgent->smem_stmts->hash_add_type->execute(soar_module::op_reinit);
    return static_cast<smem_hash_id>(thisAgent->smem_db->last_insert_rowid());
}

inline smem_hash_id smem_temporal_hash_int(agent* thisAgent, int64_t val, bool add_on_fail = true)
{
    smem_hash_id return_val = NIL;

    // search first
    thisAgent->smem_stmts->hash_get_int->bind_int(1, val);
    if (thisAgent->smem_stmts->hash_get_int->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(thisAgent->smem_stmts->hash_get_int->column_int(0));
    }
    thisAgent->smem_stmts->hash_get_int->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(thisAgent, INT_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->smem_stmts->hash_add_int->bind_int(1, return_val);
        thisAgent->smem_stmts->hash_add_int->bind_int(2, val);
        thisAgent->smem_stmts->hash_add_int->execute(soar_module::op_reinit);
    }

    return return_val;
}

inline smem_hash_id smem_temporal_hash_float(agent* thisAgent, double val, bool add_on_fail = true)
{
    smem_hash_id return_val = NIL;

    // search first
    thisAgent->smem_stmts->hash_get_float->bind_double(1, val);
    if (thisAgent->smem_stmts->hash_get_float->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(thisAgent->smem_stmts->hash_get_float->column_int(0));
    }
    thisAgent->smem_stmts->hash_get_float->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(thisAgent, FLOAT_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->smem_stmts->hash_add_float->bind_int(1, return_val);
        thisAgent->smem_stmts->hash_add_float->bind_double(2, val);
        thisAgent->smem_stmts->hash_add_float->execute(soar_module::op_reinit);
    }

    return return_val;
}

inline smem_hash_id smem_temporal_hash_str(agent* thisAgent, char* val, bool add_on_fail = true)
{
    smem_hash_id return_val = NIL;

    // search first
    thisAgent->smem_stmts->hash_get_str->bind_text(1, static_cast<const char*>(val));
    if (thisAgent->smem_stmts->hash_get_str->execute() == soar_module::row)
    {
        return_val = static_cast<smem_hash_id>(thisAgent->smem_stmts->hash_get_str->column_int(0));
    }
    thisAgent->smem_stmts->hash_get_str->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = smem_temporal_hash_add_type(thisAgent, STR_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->smem_stmts->hash_add_str->bind_int(1, return_val);
        thisAgent->smem_stmts->hash_add_str->bind_text(2, static_cast<const char*>(val));
        thisAgent->smem_stmts->hash_add_str->execute(soar_module::op_reinit);
    }

    return return_val;
}

// returns a temporally unique integer representing a symbol constant
smem_hash_id smem_temporal_hash(agent* thisAgent, Symbol* sym, bool add_on_fail = true)
{
    smem_hash_id return_val = NIL;

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->hash->start();
    ////////////////////////////////////////////////////////////////////////////

    if (sym->is_constant())
    {
        if ((!sym->smem_hash) || (sym->smem_valid != thisAgent->smem_validation))
        {
            sym->smem_hash = NIL;
            sym->smem_valid = thisAgent->smem_validation;

            switch (sym->symbol_type)
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                    return_val = smem_temporal_hash_str(thisAgent, sym->sc->name, add_on_fail);
                    break;

                case INT_CONSTANT_SYMBOL_TYPE:
                    return_val = smem_temporal_hash_int(thisAgent, sym->ic->value, add_on_fail);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return_val = smem_temporal_hash_float(thisAgent, sym->fc->value, add_on_fail);
                    break;
            }

            // cache results for later re-use
            sym->smem_hash = return_val;
            sym->smem_valid = thisAgent->smem_validation;
        }

        return_val = sym->smem_hash;
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->hash->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}

inline int64_t smem_reverse_hash_int(agent* thisAgent, smem_hash_id hash_value)
{
    int64_t return_val = NIL;

    thisAgent->smem_stmts->hash_rev_int->bind_int(1, hash_value);
    soar_module::exec_result res = thisAgent->smem_stmts->hash_rev_int->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = thisAgent->smem_stmts->hash_rev_int->column_int(0);
    thisAgent->smem_stmts->hash_rev_int->reinitialize();

    return return_val;
}

inline double smem_reverse_hash_float(agent* thisAgent, smem_hash_id hash_value)
{
    double return_val = NIL;

    thisAgent->smem_stmts->hash_rev_float->bind_int(1, hash_value);
    soar_module::exec_result res = thisAgent->smem_stmts->hash_rev_float->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = thisAgent->smem_stmts->hash_rev_float->column_double(0);
    thisAgent->smem_stmts->hash_rev_float->reinitialize();

    return return_val;
}

inline void smem_reverse_hash_str(agent* thisAgent, smem_hash_id hash_value, std::string& dest)
{
    thisAgent->smem_stmts->hash_rev_str->bind_int(1, hash_value);
    soar_module::exec_result res = thisAgent->smem_stmts->hash_rev_str->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    dest.assign(thisAgent->smem_stmts->hash_rev_str->column_text(0));
    thisAgent->smem_stmts->hash_rev_str->reinitialize();
}

inline Symbol* smem_reverse_hash(agent* thisAgent, byte symbol_type, smem_hash_id hash_value)
{
    Symbol* return_val = NULL;
    std::string dest;

    switch (symbol_type)
    {
        case STR_CONSTANT_SYMBOL_TYPE:
            smem_reverse_hash_str(thisAgent, hash_value, dest);
            return_val = make_str_constant(thisAgent, const_cast<char*>(dest.c_str()));
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            return_val = make_int_constant(thisAgent, smem_reverse_hash_int(thisAgent, hash_value));
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return_val = make_float_constant(thisAgent, smem_reverse_hash_float(thisAgent, hash_value));
            break;

        default:
            return_val = NULL;
            break;
    }

    return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Activation Functions (smem::act)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline double smem_lti_calc_base(agent* thisAgent, smem_lti_id lti, int64_t time_now, uint64_t n = 0, uint64_t activations_first = 0)
{
    double sum = 0.0;
    double d = thisAgent->smem_params->base_decay->get_value();
    uint64_t t_k;
    uint64_t t_n = (time_now - activations_first);

    if (n == 0)
    {
        thisAgent->smem_stmts->lti_access_get->bind_int(1, lti);
        thisAgent->smem_stmts->lti_access_get->execute();

        n = thisAgent->smem_stmts->lti_access_get->column_int(0);
        activations_first = thisAgent->smem_stmts->lti_access_get->column_int(2);

        thisAgent->smem_stmts->lti_access_get->reinitialize();
    }

    // get all history
    thisAgent->smem_stmts->history_get->bind_int(1, lti);
    thisAgent->smem_stmts->history_get->execute();
    {
        int available_history = static_cast<int>((SMEM_ACT_HISTORY_ENTRIES < n) ? (SMEM_ACT_HISTORY_ENTRIES) : (n));
        t_k = static_cast<uint64_t>(time_now - thisAgent->smem_stmts->history_get->column_int(available_history - 1));

        for (int i = 0; i < available_history; i++)
        {
            sum += pow(static_cast<double>(time_now - thisAgent->smem_stmts->history_get->column_int(i)),
                       static_cast<double>(-d));
        }
    }
    thisAgent->smem_stmts->history_get->reinitialize();

    // if available history was insufficient, approximate rest
    if (n > SMEM_ACT_HISTORY_ENTRIES)
    {
        double apx_numerator = (static_cast<double>(n - SMEM_ACT_HISTORY_ENTRIES) * (pow(static_cast<double>(t_n), 1.0 - d) - pow(static_cast<double>(t_k), 1.0 - d)));
        double apx_denominator = ((1.0 - d) * static_cast<double>(t_n - t_k));

        sum += (apx_numerator / apx_denominator);
    }

    return ((sum > 0) ? (log(sum)) : (SMEM_ACT_LOW));
}

// activates a new or existing long-term identifier
// note: optional num_edges parameter saves us a lookup
//       just when storing a new chunk (default is a
//       big number that should never come up naturally
//       and if it does, satisfies thresholding behavior).
inline double smem_lti_activate(agent* thisAgent, smem_lti_id lti, bool add_access, uint64_t num_edges = SMEM_ACT_MAX)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->act->start();
    ////////////////////////////////////////////////////////////////////////////

    int64_t time_now;
    if (add_access)
    {
        time_now = thisAgent->smem_max_cycle++;

        if ((thisAgent->smem_params->activation_mode->get_value() == smem_param_container::act_base) &&
                (thisAgent->smem_params->base_update->get_value() == smem_param_container::bupt_incremental))
        {
            int64_t time_diff;

            for (std::set< int64_t >::iterator b = thisAgent->smem_params->base_incremental_threshes->set_begin(); b != thisAgent->smem_params->base_incremental_threshes->set_end(); b++)
            {
                if (*b > 0)
                {
                    time_diff = (time_now - *b);

                    if (time_diff > 0)
                    {
                        std::list< smem_lti_id > to_update;

                        thisAgent->smem_stmts->lti_get_t->bind_int(1, time_diff);
                        while (thisAgent->smem_stmts->lti_get_t->execute() == soar_module::row)
                        {
                            to_update.push_back(static_cast< smem_lti_id >(thisAgent->smem_stmts->lti_get_t->column_int(0)));
                        }
                        thisAgent->smem_stmts->lti_get_t->reinitialize();

                        for (std::list< smem_lti_id >::iterator it = to_update.begin(); it != to_update.end(); it++)
                        {
                            smem_lti_activate(thisAgent, (*it), false);
                        }
                    }
                }
            }
        }
    }
    else
    {
        time_now = thisAgent->smem_max_cycle;

        thisAgent->smem_stats->act_updates->set_value(thisAgent->smem_stats->act_updates->get_value() + 1);
    }

    // access information
    uint64_t prev_access_n = 0;
    uint64_t prev_access_t = 0;
    uint64_t prev_access_1 = 0;
    {
        // get old (potentially useful below)
        {
            thisAgent->smem_stmts->lti_access_get->bind_int(1, lti);
            thisAgent->smem_stmts->lti_access_get->execute();

            prev_access_n = thisAgent->smem_stmts->lti_access_get->column_int(0);
            prev_access_t = thisAgent->smem_stmts->lti_access_get->column_int(1);
            prev_access_1 = thisAgent->smem_stmts->lti_access_get->column_int(2);

            thisAgent->smem_stmts->lti_access_get->reinitialize();
        }

        // set new
        if (add_access)
        {
            thisAgent->smem_stmts->lti_access_set->bind_int(1, (prev_access_n + 1));
            thisAgent->smem_stmts->lti_access_set->bind_int(2, time_now);
            thisAgent->smem_stmts->lti_access_set->bind_int(3, ((prev_access_n == 0) ? (time_now) : (prev_access_1)));
            thisAgent->smem_stmts->lti_access_set->bind_int(4, lti);
            thisAgent->smem_stmts->lti_access_set->execute(soar_module::op_reinit);
        }
    }

    // get new activation value (depends upon bias)
    double new_activation = 0.0;
    smem_param_container::act_choices act_mode = thisAgent->smem_params->activation_mode->get_value();
    if (act_mode == smem_param_container::act_recency)
    {
        new_activation = static_cast<double>(time_now);
    }
    else if (act_mode == smem_param_container::act_frequency)
    {
        new_activation = static_cast<double>(prev_access_n + ((add_access) ? (1) : (0)));
    }
    else if (act_mode == smem_param_container::act_base)
    {
        if (prev_access_n == 0)
        {
            if (add_access)
            {
                thisAgent->smem_stmts->history_add->bind_int(1, lti);
                thisAgent->smem_stmts->history_add->bind_int(2, time_now);
                thisAgent->smem_stmts->history_add->execute(soar_module::op_reinit);
            }

            new_activation = 0;
        }
        else
        {
            if (add_access)
            {
                thisAgent->smem_stmts->history_push->bind_int(1, time_now);
                thisAgent->smem_stmts->history_push->bind_int(2, lti);
                thisAgent->smem_stmts->history_push->execute(soar_module::op_reinit);
            }

            new_activation = smem_lti_calc_base(thisAgent, lti, time_now + ((add_access) ? (1) : (0)), prev_access_n + ((add_access) ? (1) : (0)), prev_access_1);
        }
    }

    // get number of augmentations (if not supplied)
    if (num_edges == SMEM_ACT_MAX)
    {
        thisAgent->smem_stmts->act_lti_child_ct_get->bind_int(1, lti);
        thisAgent->smem_stmts->act_lti_child_ct_get->execute();

        num_edges = thisAgent->smem_stmts->act_lti_child_ct_get->column_int(0);

        thisAgent->smem_stmts->act_lti_child_ct_get->reinitialize();
    }

    // only if augmentation count is less than threshold do we associate with edges
    if (num_edges < static_cast<uint64_t>(thisAgent->smem_params->thresh->get_value()))
    {
        // activation_value=? WHERE lti=?
        thisAgent->smem_stmts->act_set->bind_double(1, new_activation);
        thisAgent->smem_stmts->act_set->bind_int(2, lti);
        thisAgent->smem_stmts->act_set->execute(soar_module::op_reinit);
    }

    // always associate activation with lti
    {
        // activation_value=? WHERE lti=?
        thisAgent->smem_stmts->act_lti_set->bind_double(1, new_activation);
        thisAgent->smem_stmts->act_lti_set->bind_int(2, lti);
        thisAgent->smem_stmts->act_lti_set->execute(soar_module::op_reinit);
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->act->stop();
    ////////////////////////////////////////////////////////////////////////////

    return new_activation;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Long-Term Identifier Functions (smem::lti)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void _smem_lti_from_test(test t, std::set<Symbol*>* valid_ltis)
{
    if (test_is_blank(t))
    {
        return;
    }

    if (t->type == EQUALITY_TEST)
    {
        if ((t->data.referent->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (t->data.referent->id->smem_lti != NIL))
        {
            valid_ltis->insert(t->data.referent);
        }

        return;
    }

    {
        if (t->type == CONJUNCTIVE_TEST)
        {
            for (cons* c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                _smem_lti_from_test(static_cast<test>(c->first), valid_ltis);
            }
        }
    }
}

// copied primarily from add_all_variables_in_rhs_value
void _smem_lti_from_rhs_value(rhs_value rv, std::set<Symbol*>* valid_ltis)
{
    if (rhs_value_is_symbol(rv))
    {
        Symbol* sym = rhs_value_to_symbol(rv);
        if ((sym->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (sym->id->smem_lti != NIL))
        {
            valid_ltis->insert(sym);
        }
    }
    else
    {
        list* fl = rhs_value_to_funcall_list(rv);
        for (cons* c = fl->rest; c != NIL; c = c->rest)
        {
            _smem_lti_from_rhs_value(static_cast<rhs_value>(c->first), valid_ltis);
        }
    }
}

// make sure ltis in actions are grounded
bool smem_valid_production(condition* lhs_top, action* rhs_top)
{
    bool return_val = true;

    std::set<Symbol*> valid_ltis;
    std::set<Symbol*>::iterator lti_p;

    // collect valid ltis
    for (condition* c = lhs_top; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            _smem_lti_from_test(c->data.tests.attr_test, &valid_ltis);
            _smem_lti_from_test(c->data.tests.value_test, &valid_ltis);
        }
    }

    // validate ltis in actions
    // copied primarily from add_all_variables_in_action
    {
        Symbol* id;
        action* a;
        int action_counter = 0;

        for (a = rhs_top; a != NIL; a = a->next)
        {
            a->already_in_tc = false;
            action_counter++;
        }

        // good_pass detects infinite loops
        bool good_pass = true;
        bool good_action = true;
        while (good_pass && action_counter)
        {
            good_pass = false;

            for (a = rhs_top; a != NIL; a = a->next)
            {
                if (!a->already_in_tc)
                {
                    good_action = false;

                    if (a->type == MAKE_ACTION)
                    {
                        id = rhs_value_to_symbol(a->id);

                        // non-identifiers are ok
                        if (!id->is_identifier())
                        {
                            good_action = true;
                        }
                        // short-term identifiers are ok
                        else if (id->id->smem_lti == NIL)
                        {
                            good_action = true;
                        }
                        // valid long-term identifiers are ok
                        else if (valid_ltis.find(id) != valid_ltis.end())
                        {
                            good_action = true;
                        }
                    }
                    else
                    {
                        good_action = true;
                    }

                    // we've found a new good action
                    // mark as good, collect all goodies
                    if (good_action)
                    {
                        a->already_in_tc = true;

                        // everyone has values
                        _smem_lti_from_rhs_value(a->value, &valid_ltis);

                        // function calls don't have attributes
                        if (a->type == MAKE_ACTION)
                        {
                            _smem_lti_from_rhs_value(a->attr, &valid_ltis);
                        }

                        // note that we've dealt with another action
                        action_counter--;
                        good_pass = true;
                    }
                }
            }
        };

        return_val = (action_counter == 0);
    }

    return return_val;
}

// instance of hash_table_callback_fn2
bool smem_count_ltis(agent* /*thisAgent*/, void* item, void* userdata)
{
    Symbol* id = static_cast<symbol_struct*>(item);

    if (id->id->smem_lti != NIL)
    {
        uint64_t* counter = reinterpret_cast<uint64_t*>(userdata);
        (*counter)++;
    }

    return false;
}

// gets the lti id for an existing lti soar_letter/number pair (or NIL if failure)
smem_lti_id smem_lti_get_id(agent* thisAgent, char name_letter, uint64_t name_number)
{
    smem_lti_id return_val = NIL;

    // getting lti ids requires an open semantic database
    smem_attach(thisAgent);

    // soar_letter=? AND number=?
    thisAgent->smem_stmts->lti_get->bind_int(1, static_cast<uint64_t>(name_letter));
    thisAgent->smem_stmts->lti_get->bind_int(2, static_cast<uint64_t>(name_number));

    if (thisAgent->smem_stmts->lti_get->execute() == soar_module::row)
    {
        return_val = thisAgent->smem_stmts->lti_get->column_int(0);
    }

    thisAgent->smem_stmts->lti_get->reinitialize();

    return return_val;
}

// adds a new lti id for a soar_letter/number pair
inline smem_lti_id smem_lti_add_id(agent* thisAgent, char name_letter, uint64_t name_number)
{
    smem_lti_id return_val;

    // create lti: soar_letter, number, total_augmentations, activation_value, activations_total, activations_last, activations_first
    thisAgent->smem_stmts->lti_add->bind_int(1, static_cast<uint64_t>(name_letter));
    thisAgent->smem_stmts->lti_add->bind_int(2, static_cast<uint64_t>(name_number));
    thisAgent->smem_stmts->lti_add->bind_int(3, static_cast<uint64_t>(0));
    thisAgent->smem_stmts->lti_add->bind_double(4, static_cast<double>(0));
    thisAgent->smem_stmts->lti_add->bind_int(5, static_cast<uint64_t>(0));
    thisAgent->smem_stmts->lti_add->bind_int(6, static_cast<uint64_t>(0));
    thisAgent->smem_stmts->lti_add->bind_int(7, static_cast<uint64_t>(0));
    thisAgent->smem_stmts->lti_add->execute(soar_module::op_reinit);

    return_val = static_cast<smem_lti_id>(thisAgent->smem_db->last_insert_rowid());

    // increment stat
    thisAgent->smem_stats->chunks->set_value(thisAgent->smem_stats->chunks->get_value() + 1);

    return return_val;
}

// makes a non-long-term identifier into a long-term identifier
inline smem_lti_id smem_lti_soar_add(agent* thisAgent, Symbol* id)
{
    if ((id->is_identifier()) &&
            (id->id->smem_lti == NIL))
    {
        // try to find existing lti
        id->id->smem_lti = smem_lti_get_id(thisAgent, id->id->name_letter, id->id->name_number);

        // if doesn't exist, add
        if (id->id->smem_lti == NIL)
        {
            id->id->smem_lti = smem_lti_add_id(thisAgent, id->id->name_letter, id->id->name_number);

            id->id->smem_time_id = thisAgent->epmem_stats->time->get_value();
            id->id->smem_valid = thisAgent->epmem_validation;
            epmem_schedule_promotion(thisAgent, id);
        }
    }

    return id->id->smem_lti;
}

// returns a reference to an lti
Symbol* smem_lti_soar_make(agent* thisAgent, smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level)
{
    Symbol* return_val;

    // try to find existing
    return_val = find_identifier(thisAgent, name_letter, name_number);

    // otherwise create
    if (return_val == NIL)
    {
        return_val = make_new_identifier(thisAgent, name_letter, level, name_number);
    }
    else
    {
        symbol_add_ref(thisAgent, return_val);

        if ((return_val->id->level == SMEM_LTI_UNKNOWN_LEVEL) && (level != SMEM_LTI_UNKNOWN_LEVEL))
        {
            return_val->id->level = level;
            return_val->id->promotion_level = level;
        }
    }

    // set lti field irrespective
    return_val->id->smem_lti = lti;

    return return_val;
}

void smem_reset_id_counters(agent* thisAgent)
{
    if (thisAgent->smem_db->get_status() == soar_module::connected)
    {
        // soar_letter, max
        while (thisAgent->smem_stmts->lti_max->execute() == soar_module::row)
        {
            uint64_t name_letter = static_cast<uint64_t>(thisAgent->smem_stmts->lti_max->column_int(0));
            uint64_t letter_max = static_cast<uint64_t>(thisAgent->smem_stmts->lti_max->column_int(1));

            // shift to alphabet
            name_letter -= static_cast<uint64_t>('A');

            // get count
            uint64_t* letter_ct = & thisAgent->id_counter[ name_letter ];

            // adjust if necessary
            if ((*letter_ct) <= letter_max)
            {
                (*letter_ct) = (letter_max + 1);
            }
        }

        thisAgent->smem_stmts->lti_max->reinitialize();
    }
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Storage Functions (smem::storage)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline smem_slot* smem_make_slot(smem_slot_map* slots, Symbol* attr)
{
    smem_slot** s = & (*slots)[ attr ];

    if (!(*s))
    {
        (*s) = new smem_slot;
    }

    return (*s);
}

void smem_disconnect_chunk(agent* thisAgent, smem_lti_id lti_id)
{
    // adjust attr, attr/value counts
    {
        uint64_t pair_count = 0;

        smem_lti_id child_attr = 0;
        std::set<smem_lti_id> distinct_attr;

        // pairs first, accumulate distinct attributes and pair count
        thisAgent->smem_stmts->web_all->bind_int(1, lti_id);
        while (thisAgent->smem_stmts->web_all->execute() == soar_module::row)
        {
            pair_count++;

            child_attr = thisAgent->smem_stmts->web_all->column_int(0);
            distinct_attr.insert(child_attr);

            // null -> attr/lti
            if (thisAgent->smem_stmts->web_all->column_int(1) != SMEM_AUGMENTATIONS_NULL)
            {
                // adjust in opposite direction ( adjust, attribute, const )
                thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(1, -1);
                thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(2, child_attr);
                thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(3, thisAgent->smem_stmts->web_all->column_int(1));
                thisAgent->smem_stmts->wmes_constant_frequency_update->execute(soar_module::op_reinit);
            }
            else
            {
                // adjust in opposite direction ( adjust, attribute, lti )
                thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(1, -1);
                thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(2, child_attr);
                thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(3, thisAgent->smem_stmts->web_all->column_int(2));
                thisAgent->smem_stmts->wmes_lti_frequency_update->execute(soar_module::op_reinit);
            }
        }
        thisAgent->smem_stmts->web_all->reinitialize();

        // now attributes
        for (std::set<smem_lti_id>::iterator a = distinct_attr.begin(); a != distinct_attr.end(); a++)
        {
            // adjust in opposite direction ( adjust, attribute )
            thisAgent->smem_stmts->attribute_frequency_update->bind_int(1, -1);
            thisAgent->smem_stmts->attribute_frequency_update->bind_int(2, *a);
            thisAgent->smem_stmts->attribute_frequency_update->execute(soar_module::op_reinit);
        }

        // update local statistic
        thisAgent->smem_stats->slots->set_value(thisAgent->smem_stats->slots->get_value() - pair_count);
    }

    // disconnect
    {
        thisAgent->smem_stmts->web_truncate->bind_int(1, lti_id);
        thisAgent->smem_stmts->web_truncate->execute(soar_module::op_reinit);
    }
}

void smem_store_chunk(agent* thisAgent, smem_lti_id lti_id, smem_slot_map* children, bool remove_old_children = true, Symbol* print_id = NULL, bool activate = true)
{
    // if remove children, disconnect chunk -> no existing edges
    // else, need to query number of existing edges
    uint64_t existing_edges = 0;
    if (remove_old_children)
    {
        smem_disconnect_chunk(thisAgent, lti_id);

        // provide trace output
        if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
        {
            char buf[256];

            snprintf_with_symbols(thisAgent, buf, 256, "<=SMEM: (%y ^* *)\n", print_id);

            print(thisAgent,  buf);
            xml_generate_warning(thisAgent, buf);
        }
    }
    else
    {
        thisAgent->smem_stmts->act_lti_child_ct_get->bind_int(1, lti_id);
        thisAgent->smem_stmts->act_lti_child_ct_get->execute();

        existing_edges = static_cast<uint64_t>(thisAgent->smem_stmts->act_lti_child_ct_get->column_int(0));

        thisAgent->smem_stmts->act_lti_child_ct_get->reinitialize();
    }

    // get new edges
    // if didn't disconnect, entails lookups in existing edges
    std::set<smem_hash_id> attr_new;
    std::set< std::pair<smem_hash_id, smem_hash_id> > const_new;
    std::set< std::pair<smem_hash_id, smem_lti_id> > lti_new;
    {
        smem_slot_map::iterator s;
        smem_slot::iterator v;

        smem_hash_id attr_hash = 0;
        smem_hash_id value_hash = 0;
        smem_lti_id value_lti = 0;

        for (s = children->begin(); s != children->end(); s++)
        {
            attr_hash = smem_temporal_hash(thisAgent, s->first);
            if (remove_old_children)
            {
                attr_new.insert(attr_hash);
            }
            else
            {
                // lti_id, attribute_s_id
                thisAgent->smem_stmts->web_attr_child->bind_int(1, lti_id);
                thisAgent->smem_stmts->web_attr_child->bind_int(2, attr_hash);
                if (thisAgent->smem_stmts->web_attr_child->execute(soar_module::op_reinit) != soar_module::row)
                {
                    attr_new.insert(attr_hash);
                }
            }

            for (v = s->second->begin(); v != s->second->end(); v++)
            {
                if ((*v)->val_const.val_type == value_const_t)
                {
                    value_hash = smem_temporal_hash(thisAgent, (*v)->val_const.val_value);

                    if (remove_old_children)
                    {
                        const_new.insert(std::make_pair(attr_hash, value_hash));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_const
                        thisAgent->smem_stmts->web_const_child->bind_int(1, lti_id);
                        thisAgent->smem_stmts->web_const_child->bind_int(2, attr_hash);
                        thisAgent->smem_stmts->web_const_child->bind_int(3, value_hash);
                        if (thisAgent->smem_stmts->web_const_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            const_new.insert(std::make_pair(attr_hash, value_hash));
                        }
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
                    {
                        char buf[256];

                        snprintf_with_symbols(thisAgent, buf, 256, "=>SMEM: (%y ^%y %y)\n", print_id, s->first, (*v)->val_const.val_value);

                        print(thisAgent,  buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
                else
                {
                    value_lti = (*v)->val_lti.val_value->lti_id;
                    if (value_lti == NIL)
                    {
                        value_lti = smem_lti_add_id(thisAgent, (*v)->val_lti.val_value->lti_letter, (*v)->val_lti.val_value->lti_number);
                        (*v)->val_lti.val_value->lti_id = value_lti;

                        if ((*v)->val_lti.val_value->soar_id != NIL)
                        {
                            (*v)->val_lti.val_value->soar_id->id->smem_lti = value_lti;

                            (*v)->val_lti.val_value->soar_id->id->smem_time_id = thisAgent->epmem_stats->time->get_value();
                            (*v)->val_lti.val_value->soar_id->id->smem_valid = thisAgent->epmem_validation;
                            epmem_schedule_promotion(thisAgent, (*v)->val_lti.val_value->soar_id);
                        }
                    }

                    if (remove_old_children)
                    {
                        lti_new.insert(std::make_pair(attr_hash, value_lti));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_lti
                        thisAgent->smem_stmts->web_lti_child->bind_int(1, lti_id);
                        thisAgent->smem_stmts->web_lti_child->bind_int(2, attr_hash);
                        thisAgent->smem_stmts->web_lti_child->bind_int(3, value_lti);
                        if (thisAgent->smem_stmts->web_lti_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            lti_new.insert(std::make_pair(attr_hash, value_lti));
                        }
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
                    {
                        char buf[256];

                        snprintf_with_symbols(thisAgent, buf, 256, "=>SMEM: (%y ^%y %y)\n", print_id, s->first, (*v)->val_lti.val_value->soar_id);

                        print(thisAgent,  buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
            }
        }
    }

    // activation function assumes proper thresholding state
    // thus, consider four cases of augmentation counts (w.r.t. thresh)
    // 1. before=below, after=below: good (activation will update smem_augmentations)
    // 2. before=below, after=above: need to update smem_augmentations->inf
    // 3. before=after, after=below: good (activation will update smem_augmentations, free transition)
    // 4. before=after, after=after: good (activation won't touch smem_augmentations)
    //
    // hence, we detect + handle case #2 here
    uint64_t new_edges = (existing_edges + const_new.size() + lti_new.size());
    bool after_above;
    double web_act = static_cast<double>(SMEM_ACT_MAX);
    {
        uint64_t thresh = static_cast<uint64_t>(thisAgent->smem_params->thresh->get_value());
        after_above = (new_edges >= thresh);

        // if before below
        if (existing_edges < thresh)
        {
            if (after_above)
            {
                // update smem_augmentations to inf
                thisAgent->smem_stmts->act_set->bind_double(1, web_act);
                thisAgent->smem_stmts->act_set->bind_int(2, lti_id);
                thisAgent->smem_stmts->act_set->execute(soar_module::op_reinit);
            }
        }
    }

    // update edge counter
    {
        thisAgent->smem_stmts->act_lti_child_ct_set->bind_int(1, new_edges);
        thisAgent->smem_stmts->act_lti_child_ct_set->bind_int(2, lti_id);
        thisAgent->smem_stmts->act_lti_child_ct_set->execute(soar_module::op_reinit);
    }

    // now we can safely activate the lti
    if (activate)
    {
        double lti_act = smem_lti_activate(thisAgent, lti_id, true, new_edges);

        if (!after_above)
        {
            web_act = lti_act;
        }
    }

    // insert new edges, update counters
    {
        // attr/const pairs
        {
            for (std::set< std::pair< smem_hash_id, smem_hash_id > >::iterator p = const_new.begin(); p != const_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    thisAgent->smem_stmts->web_add->bind_int(1, lti_id);
                    thisAgent->smem_stmts->web_add->bind_int(2, p->first);
                    thisAgent->smem_stmts->web_add->bind_int(3, p->second);
                    thisAgent->smem_stmts->web_add->bind_int(4, SMEM_AUGMENTATIONS_NULL);
                    thisAgent->smem_stmts->web_add->bind_double(5, web_act);
                    thisAgent->smem_stmts->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    thisAgent->smem_stmts->wmes_constant_frequency_check->bind_int(1, p->first);
                    thisAgent->smem_stmts->wmes_constant_frequency_check->bind_int(2, p->second);
                    if (thisAgent->smem_stmts->wmes_constant_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        thisAgent->smem_stmts->wmes_constant_frequency_add->bind_int(1, p->first);
                        thisAgent->smem_stmts->wmes_constant_frequency_add->bind_int(2, p->second);
                        thisAgent->smem_stmts->wmes_constant_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, val)
                        thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(1, 1);
                        thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(2, p->first);
                        thisAgent->smem_stmts->wmes_constant_frequency_update->bind_int(3, p->second);
                        thisAgent->smem_stmts->wmes_constant_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // attr/lti pairs
        {
            for (std::set< std::pair< smem_hash_id, smem_lti_id > >::iterator p = lti_new.begin(); p != lti_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    thisAgent->smem_stmts->web_add->bind_int(1, lti_id);
                    thisAgent->smem_stmts->web_add->bind_int(2, p->first);
                    thisAgent->smem_stmts->web_add->bind_int(3, SMEM_AUGMENTATIONS_NULL);
                    thisAgent->smem_stmts->web_add->bind_int(4, p->second);
                    thisAgent->smem_stmts->web_add->bind_double(5, web_act);
                    thisAgent->smem_stmts->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    thisAgent->smem_stmts->wmes_lti_frequency_check->bind_int(1, p->first);
                    thisAgent->smem_stmts->wmes_lti_frequency_check->bind_int(2, p->second);
                    if (thisAgent->smem_stmts->wmes_lti_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        thisAgent->smem_stmts->wmes_lti_frequency_add->bind_int(1, p->first);
                        thisAgent->smem_stmts->wmes_lti_frequency_add->bind_int(2, p->second);
                        thisAgent->smem_stmts->wmes_lti_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, lti)
                        thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(1, 1);
                        thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(2, p->first);
                        thisAgent->smem_stmts->wmes_lti_frequency_update->bind_int(3, p->second);
                        thisAgent->smem_stmts->wmes_lti_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // update attribute count
        {
            for (std::set< smem_hash_id >::iterator a = attr_new.begin(); a != attr_new.end(); a++)
            {
                // check if counter exists (and add if does not): attribute_s_id
                thisAgent->smem_stmts->attribute_frequency_check->bind_int(1, *a);
                if (thisAgent->smem_stmts->attribute_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                {
                    thisAgent->smem_stmts->attribute_frequency_add->bind_int(1, *a);
                    thisAgent->smem_stmts->attribute_frequency_add->execute(soar_module::op_reinit);
                }
                else
                {
                    // adjust count (adjustment, attribute_s_id)
                    thisAgent->smem_stmts->attribute_frequency_update->bind_int(1, 1);
                    thisAgent->smem_stmts->attribute_frequency_update->bind_int(2, *a);
                    thisAgent->smem_stmts->attribute_frequency_update->execute(soar_module::op_reinit);
                }
            }
        }

        // update local edge count
        {
            thisAgent->smem_stats->slots->set_value(thisAgent->smem_stats->slots->get_value() + (const_new.size() + lti_new.size()));
        }
    }
}

void smem_soar_store(agent* thisAgent, Symbol* id, smem_storage_type store_type = store_level, tc_number tc = NIL)
{
    // transitive closure only matters for recursive storage
    if ((store_type == store_recursive) && (tc == NIL))
    {
        tc = get_new_tc_number(thisAgent);
    }
    smem_sym_list shorties;

    // get level
    smem_wme_list* children = smem_get_direct_augs_of_id(id, tc);
    smem_wme_list::iterator w;

    // make the target an lti, so intermediary data structure has lti_id
    // (takes care of short-term id self-referencing)
    smem_lti_soar_add(thisAgent, id);

    // encode this level
    {
        smem_sym_to_chunk_map sym_to_chunk;
        smem_sym_to_chunk_map::iterator c_p;
        smem_chunk** c;

        smem_slot_map slots;
        smem_slot_map::iterator s_p;
        smem_slot::iterator v_p;
        smem_slot* s;
        smem_chunk_value* v;

        for (w = children->begin(); w != children->end(); w++)
        {
            // get slot
            s = smem_make_slot(&(slots), (*w)->attr);

            // create value, per type
            v = new smem_chunk_value;
            if ((*w)->value->is_constant())
            {
                v->val_const.val_type = value_const_t;
                v->val_const.val_value = (*w)->value;
            }
            else
            {
                v->val_lti.val_type = value_lti_t;

                // try to find existing chunk
                c = & sym_to_chunk[(*w)->value ];

                // if doesn't exist, add; else use existing
                if (!(*c))
                {
                    (*c) = new smem_chunk;
                    (*c)->lti_id = (*w)->value->id->smem_lti;
                    (*c)->lti_letter = (*w)->value->id->name_letter;
                    (*c)->lti_number = (*w)->value->id->name_number;
                    (*c)->slots = NULL;
                    (*c)->soar_id = (*w)->value;

                    // only traverse to short-term identifiers
                    if ((store_type == store_recursive) && ((*c)->lti_id == NIL))
                    {
                        shorties.push_back((*c)->soar_id);
                    }
                }

                v->val_lti.val_value = (*c);
            }

            // add value to slot
            s->push_back(v);
        }

        smem_store_chunk(thisAgent, id->id->smem_lti, &(slots), true, id);

        // clean up
        {
            // de-allocate slots
            for (s_p = slots.begin(); s_p != slots.end(); s_p++)
            {
                for (v_p = s_p->second->begin(); v_p != s_p->second->end(); v_p++)
                {
                    delete(*v_p);
                }

                delete s_p->second;
            }

            // de-allocate chunks
            for (c_p = sym_to_chunk.begin(); c_p != sym_to_chunk.end(); c_p++)
            {
                delete c_p->second;
            }

            delete children;
        }
    }

    // recurse as necessary
    for (smem_sym_list::iterator shorty = shorties.begin(); shorty != shorties.end(); shorty++)
    {
        smem_soar_store(thisAgent, (*shorty), store_recursive, tc);
    }
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Non-Cue-Based Retrieval Functions (smem::ncb)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_install_memory(agent* thisAgent, Symbol* state, smem_lti_id lti_id, Symbol* lti, bool activate_lti, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes, smem_install_type install_type = wm_install, uint64_t depth = 1)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->ncb_retrieval->start();
    ////////////////////////////////////////////////////////////////////////////

    // get the ^result header for this state
    Symbol* result_header;
    if (install_type == wm_install)
    {
        result_header = state->id->smem_result_header;
    }

    // get identifier if not known
    bool lti_created_here = false;
    if (lti == NIL && install_type == wm_install)
    {
        soar_module::sqlite_statement* q = thisAgent->smem_stmts->lti_letter_num;

        q->bind_int(1, lti_id);
        q->execute();

        lti = smem_lti_soar_make(thisAgent, lti_id, static_cast<char>(q->column_int(0)), static_cast<uint64_t>(q->column_int(1)), result_header->id->level);

        q->reinitialize();

        lti_created_here = true;
    }

    // activate lti
    if (activate_lti)
    {
        smem_lti_activate(thisAgent, lti_id, true);
    }

    // point retrieved to lti
    if (install_type == wm_install)
    {
    smem_buffer_add_wme(thisAgent, meta_wmes, result_header, thisAgent->smem_sym_retrieved, lti);
    }
    if (lti_created_here)
    {
        // if the identifier was created above we need to
        // remove a single ref count AFTER the wme
        // is added (such as to not deallocate the symbol
        // prematurely)
        symbol_remove_ref(thisAgent, lti);
    }

    // if no children, then retrieve children
    // merge may override this behavior
    if (((thisAgent->smem_params->merge->get_value() == smem_param_container::merge_add) ||
            ((lti->id->impasse_wmes == NIL) &&
             (lti->id->input_wmes == NIL) &&
             (lti->id->slots == NIL)))
            || (install_type == fake_install)) //(The final bit is if this is being called by the remove command.)
    {
        soar_module::sqlite_statement* expand_q = thisAgent->smem_stmts->web_expand;
        Symbol* attr_sym;
        Symbol* value_sym;

        // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
        expand_q->bind_int(1, lti_id);

        std::list<Symbol*> children;

        while (expand_q->execute() == soar_module::row)
        {
            // make the identifier symbol irrespective of value type
            attr_sym = smem_reverse_hash(thisAgent, static_cast<byte>(expand_q->column_int(0)), static_cast<smem_hash_id>(expand_q->column_int(1)));

            // identifier vs. constant
            if (expand_q->column_int(6) != SMEM_AUGMENTATIONS_NULL)
            {
                value_sym = smem_lti_soar_make(thisAgent, static_cast<smem_lti_id>(expand_q->column_int(6)), static_cast<char>(expand_q->column_int(4)), static_cast<uint64_t>(expand_q->column_int(5)), lti->id->level);
                if (depth > 1)
                {
                    children.push_back(value_sym);
            }
            }
            else
            {
                value_sym = smem_reverse_hash(thisAgent, static_cast<byte>(expand_q->column_int(2)), static_cast<smem_hash_id>(expand_q->column_int(3)));
            }

            // add wme
            smem_buffer_add_wme(thisAgent, retrieval_wmes, lti, attr_sym, value_sym);

            // deal with ref counts - attribute/values are always created in this function
            // (thus an extra ref count is set before adding a wme)
            symbol_remove_ref(thisAgent, attr_sym);
            symbol_remove_ref(thisAgent, value_sym);
        }
        expand_q->reinitialize();

        //Attempt to find children for the case of depth.
        std::list<Symbol*>::iterator iterator;
        std::list<Symbol*>::iterator end = children.end();
        for (iterator = children.begin(); iterator != end; ++iterator)
        {
            smem_install_memory(thisAgent, state, (*iterator)->id->smem_lti, (*iterator), (thisAgent->smem_params->activate_on_query->get_value() == on), meta_wmes, retrieval_wmes, wm_install, depth - 1);
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->ncb_retrieval->stop();
    ////////////////////////////////////////////////////////////////////////////
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Cue-Based Retrieval Functions (smem::cbr)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline soar_module::sqlite_statement* smem_setup_web_crawl(agent* thisAgent, smem_weighted_cue_element* el)
{
    soar_module::sqlite_statement* q = NULL;

    // first, point to correct query and setup
    // query-specific parameters
    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        q = thisAgent->smem_stmts->web_attr_all;
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        q = thisAgent->smem_stmts->web_const_all;
        q->bind_int(2, el->value_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        q = thisAgent->smem_stmts->web_lti_all;
        q->bind_int(2, el->value_lti);
    }

    // all require hash as first parameter
    q->bind_int(1, el->attr_hash);

    return q;
}

inline bool _smem_process_cue_wme(agent* thisAgent, wme* w, bool pos_cue, smem_prioritized_weighted_cue& weighted_pq, MathQuery* mathQuery)
{
    bool good_wme = true;
    smem_weighted_cue_element* new_cue_element;

    smem_hash_id attr_hash;
    smem_hash_id value_hash;
    smem_lti_id value_lti;
    smem_cue_element_type element_type;

    soar_module::sqlite_statement* q = NULL;

    {
        // we only have to do hard work if
        attr_hash = smem_temporal_hash(thisAgent, w->attr, false);
        if (attr_hash != NIL)
        {
            if (w->value->is_constant() && mathQuery == NIL)
            {
                value_lti = NIL;
                value_hash = smem_temporal_hash(thisAgent, w->value, false);
                element_type = value_const_t;

                if (value_hash != NIL)
                {
                    q = thisAgent->smem_stmts->wmes_constant_frequency_get;
                    q->bind_int(1, attr_hash);
                    q->bind_int(2, value_hash);
                }
                else if (pos_cue)
                {
                        good_wme = false;
                    }
                else
                {
                    //This would be a negative query that smem has no hash for.  This means that
                    //there is no way it could be in any of the results, and we don't
                    //need to continue processing it, let alone use it in the search.  --ACN
                    return true;
                }
            }
            else
            {
                //If we get here on a math query, the value may not be an identifier
                if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                {
                value_lti = w->value->id->smem_lti;
                }
                else
                {
                    value_lti = 0;
                }
                value_hash = NIL;

                if (value_lti == NIL)
                {
                    q = thisAgent->smem_stmts->attribute_frequency_get;
                    q->bind_int(1, attr_hash);

                    element_type = attr_t;
                }
                else
                {
                    q = thisAgent->smem_stmts->wmes_lti_frequency_get;
                    q->bind_int(1, attr_hash);
                    q->bind_int(2, value_lti);

                    element_type = value_lti_t;
                }
            }

            if (good_wme)
            {
                if (q->execute() == soar_module::row)
                {
                    new_cue_element = new smem_weighted_cue_element;

                    new_cue_element->weight = q->column_int(0);
                    new_cue_element->attr_hash = attr_hash;
                    new_cue_element->value_hash = value_hash;
                    new_cue_element->value_lti = value_lti;
                    new_cue_element->cue_element = w;

                    new_cue_element->element_type = element_type;
                    new_cue_element->pos_element = pos_cue;
                    new_cue_element->mathElement = mathQuery;

                    weighted_pq.push(new_cue_element);
                    new_cue_element = NULL;
                }
                else
                {
                    if (pos_cue)
                    {
                        good_wme = false;
                    }
                }

                q->reinitialize();
            }
        }
        else
        {
            if (pos_cue)
            {
                good_wme = false;
            }
        }
    }
    //If we brought in a math query and didn't use it
    if (!good_wme && mathQuery != NIL)
    {
        delete mathQuery;
    }
    return good_wme;
}

//this returns a pair with <needFullSearch, goodCue>
std::pair<bool, bool>* processMathQuery(agent* thisAgent, Symbol* mathQuery, smem_prioritized_weighted_cue* weighted_pq)
{
    bool needFullSearch = false;
    //Use this set to track when certain elements have been added, so we don't add them twice
    std::set<Symbol*> uniqueMathQueryElements;
    std::pair<bool, bool>* result = new std::pair<bool, bool>(true, true);

    smem_wme_list* cue = smem_get_direct_augs_of_id(mathQuery);
    for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
    {

        smem_wme_list* cueTypes = smem_get_direct_augs_of_id((*cue_p)->value);
        if (cueTypes->empty())
        {
            //This would be an attribute without a query type attached
            result->first = false;
            result->second = false;
            break;
        }
        else
        {
            for (smem_wme_list::iterator cueType = cueTypes->begin(); cueType != cueTypes->end(); cueType++)
            {
                if ((*cueType)->attr == thisAgent->smem_sym_math_query_less)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryLess((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryLess((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->smem_sym_math_query_greater)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryGreater((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryGreater((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->smem_sym_math_query_less_or_equal)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryLessOrEqual((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryLessOrEqual((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->smem_sym_math_query_greater_or_equal)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryGreaterOrEqual((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                        _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryGreaterOrEqual((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->smem_sym_math_query_max)
                {
                    if (uniqueMathQueryElements.find(thisAgent->smem_sym_math_query_max) != uniqueMathQueryElements.end())
                    {
                        //Only one max at a time
                        result->first = false;
                        result->second = false;
                        break;
                    }
                    else
                    {
                        uniqueMathQueryElements.insert(thisAgent->smem_sym_math_query_max);
                    }
                    needFullSearch = true;
                    _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryMax());
                }
                else if ((*cueType)->attr == thisAgent->smem_sym_math_query_min)
                {
                    if (uniqueMathQueryElements.find(thisAgent->smem_sym_math_query_min) != uniqueMathQueryElements.end())
                    {
                        //Only one min at a time
                        result->first = false;
                        result->second = false;
                        break;
                    }
                    else
                    {
                        uniqueMathQueryElements.insert(thisAgent->smem_sym_math_query_min);
                    }
                    needFullSearch = true;
                    _smem_process_cue_wme(thisAgent, (*cue_p), true, *weighted_pq, new MathQueryMin());
                }
            }
        }
        delete cueTypes;
    }
    delete cue;
    if (result->second)
    {
        result->first = needFullSearch;
        return result;
    }
    return result;
}

smem_lti_id smem_process_query(agent* thisAgent, Symbol* state, Symbol* query, Symbol* negquery, Symbol* mathQuery, smem_lti_set* prohibit, soar_module::wme_set& cue_wmes, soar_module::symbol_triple_list& meta_wmes, soar_module::symbol_triple_list& retrieval_wmes, smem_query_levels query_level = qry_full, int number_to_retrieve = 1, std::list<smem_lti_id>* match_ids = NIL)
{
    smem_weighted_cue_list weighted_cue;
    bool good_cue = true;

    //This is used when doing math queries that need to look at more that just the first valid element
    bool needFullSearch = false;

    soar_module::sqlite_statement* q = NULL;

    std::list<smem_lti_id> temp_list;
    if (query_level == qry_full)
    {
        match_ids = &(temp_list);
    }

    smem_lti_id king_id = NIL;

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->query->start();
    ////////////////////////////////////////////////////////////////////////////

    // prepare query stats
    {
        smem_prioritized_weighted_cue weighted_pq;

        // positive cue - always
        {
            smem_wme_list* cue = smem_get_direct_augs_of_id(query);
            if (cue->empty())
            {
                good_cue = false;
            }

            for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
            {
                cue_wmes.insert((*cue_p));

                if (good_cue)
                {
                    good_cue = _smem_process_cue_wme(thisAgent, (*cue_p), true, weighted_pq, NIL);
                }
            }

            delete cue;
        }

        //Look through while were here, so that we can make sure the attributes we need are in the results
        if (mathQuery != NIL && good_cue)
        {
            std::pair<bool, bool>* mpr = processMathQuery(thisAgent, mathQuery, &weighted_pq);
            needFullSearch = mpr->first;
            good_cue = mpr->second;
            delete mpr;
        }

        // negative cue - if present
        if (negquery)
        {
            smem_wme_list* cue = smem_get_direct_augs_of_id(negquery);

            for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
            {
                cue_wmes.insert((*cue_p));

                if (good_cue)
                {
                    good_cue = _smem_process_cue_wme(thisAgent, (*cue_p), false, weighted_pq, NIL);
                }
            }

            delete cue;
        }

        // if valid cue, transfer priority queue to list
        if (good_cue)
        {
            while (!weighted_pq.empty())
            {
                weighted_cue.push_back(weighted_pq.top());
                weighted_pq.pop();
            }
        }
        // else deallocate priority queue contents
        else
        {
            while (!weighted_pq.empty())
            {
                smem_prioritized_weighted_cue::value_type top = weighted_pq.top();
                weighted_pq.pop();
                if (top->mathElement != NIL)
                {
                    delete top->mathElement;
                }
                delete top;
                /*if(weighted_pq.top()->mathElement != NIL){
                    delete weighted_pq.top()->mathElement;
                }
                delete weighted_pq.top();
                weighted_pq.pop();*/
            }
        }
    }

    // only search if the cue was valid
    if (good_cue && !weighted_cue.empty())
    {
        // by definition, the first positive-cue element dictates the candidate set
        smem_weighted_cue_list::iterator cand_set;
        smem_weighted_cue_list::iterator next_element;
        for (next_element = weighted_cue.begin(); next_element != weighted_cue.end(); next_element++)
        {
            if ((*next_element)->pos_element)
            {
                cand_set = next_element;
                break;
            }
        }

        soar_module::sqlite_statement* q2 = NULL;
        smem_lti_set::iterator prohibit_p;

        smem_lti_id cand;
        bool good_cand;

        if (thisAgent->smem_params->activation_mode->get_value() == smem_param_container::act_base)
        {
            // naive base-level updates means update activation of
            // every candidate in the minimal list before the
            // confirmation walk
            if (thisAgent->smem_params->base_update->get_value() == smem_param_container::bupt_naive)
            {
                q = smem_setup_web_crawl(thisAgent, (*cand_set));

                // queue up distinct lti's to update
                // - set because queries could contain wilds
                // - not in loop because the effects of activation may actually
                //   alter the resultset of the query (isolation???)
                std::set< smem_lti_id > to_update;
                while (q->execute() == soar_module::row)
                {
                    to_update.insert(q->column_int(0));
                }

                for (std::set< smem_lti_id >::iterator it = to_update.begin(); it != to_update.end(); it++)
                {
                    smem_lti_activate(thisAgent, (*it), false);
                }

                q->reinitialize();
            }
        }

        // setup first query, which is sorted on activation already
        q = smem_setup_web_crawl(thisAgent, (*cand_set));

        // this becomes the minimal set to walk (till match or fail)
        if (q->execute() == soar_module::row)
        {
            smem_prioritized_activated_lti_queue plentiful_parents;
            bool more_rows = true;
            bool use_db = false;
            bool has_feature = false;

            while (more_rows && (q->column_double(1) == static_cast<double>(SMEM_ACT_MAX)))
            {
                thisAgent->smem_stmts->act_lti_get->bind_int(1, q->column_int(0));
                thisAgent->smem_stmts->act_lti_get->execute();
                plentiful_parents.push(std::make_pair< double, smem_lti_id >(thisAgent->smem_stmts->act_lti_get->column_double(0), q->column_int(0)));
                thisAgent->smem_stmts->act_lti_get->reinitialize();

                more_rows = (q->execute() == soar_module::row);
            }
            bool first_element = false;
            while (((match_ids->size() < number_to_retrieve) || (needFullSearch)) && ((more_rows) || (!plentiful_parents.empty())))
            {
                // choose next candidate (db vs. priority queue)
                {
                    use_db = false;

                    if (!more_rows)
                    {
                        use_db = false;
                    }
                    else if (plentiful_parents.empty())
                    {
                        use_db = true;
                    }
                    else
                    {
                        use_db = (q->column_double(1) >  plentiful_parents.top().first);
                    }

                    if (use_db)
                    {
                        cand = q->column_int(0);
                        more_rows = (q->execute() == soar_module::row);
                    }
                    else
                    {
                        cand = plentiful_parents.top().second;
                        plentiful_parents.pop();
                    }
                }

                // if not prohibited, submit to the remaining cue elements
                prohibit_p = prohibit->find(cand);
                if (prohibit_p == prohibit->end())
                {
                    good_cand = true;

                    for (next_element = weighted_cue.begin(); next_element != weighted_cue.end() && good_cand; next_element++)
                    {
                        // don't need to check the generating list
                        //If the cand_set is a math query, we care about more than its existence
                        if ((*next_element) == (*cand_set) && (*next_element)->mathElement == NIL)
                        {
                            continue;
                        }

                        if ((*next_element)->element_type == attr_t)
                        {
                            // parent=? AND attribute_s_id=?
                            q2 = thisAgent->smem_stmts->web_attr_child;
                        }
                        else if ((*next_element)->element_type == value_const_t)
                        {
                            // parent=? AND attribute_s_id=? AND value_constant_s_id=?
                            q2 = thisAgent->smem_stmts->web_const_child;
                            q2->bind_int(3, (*next_element)->value_hash);
                        }
                        else if ((*next_element)->element_type == value_lti_t)
                        {
                            // parent=? AND attribute_s_id=? AND value_lti_id=?
                            q2 = thisAgent->smem_stmts->web_lti_child;
                            q2->bind_int(3, (*next_element)->value_lti);
                        }

                        // all require own id, attribute
                        q2->bind_int(1, cand);
                        q2->bind_int(2, (*next_element)->attr_hash);

                        has_feature = (q2->execute() == soar_module::row);
                        bool mathQueryMet = false;
                        if ((*next_element)->mathElement != NIL && has_feature)
                        {
                            do
                            {
                                smem_hash_id valueHash = q2->column_int(2 - 1);
                                thisAgent->smem_stmts->hash_rev_type->bind_int(1, valueHash);

                                if (thisAgent->smem_stmts->hash_rev_type->execute() != soar_module::row)
                                {
                                    good_cand = false;
                                }
                                else
                                {
                                    switch (thisAgent->smem_stmts->hash_rev_type->column_int(1 - 1))
                                    {
                                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                                            mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(smem_reverse_hash_float(thisAgent, valueHash));
                                            break;
                                        case INT_CONSTANT_SYMBOL_TYPE:
                                            mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(smem_reverse_hash_int(thisAgent, valueHash));
                                            break;
                                    }
                                }
                                thisAgent->smem_stmts->hash_rev_type->reinitialize();
                            }
                            while (q2->execute() == soar_module::row);
                            good_cand = mathQueryMet;
                        }
                        else
                        {
                        good_cand = (((*next_element)->pos_element) ? (has_feature) : (!has_feature));
                        }
                        //In CSoar this needs to happen before the break, or the query might not be ready next time
                        q2->reinitialize();
                        if (!good_cand)
                        {
                            break;
                        }
                    }

                    if (good_cand)
                    {
                        king_id = cand;
                        first_element = true;
                        match_ids->push_back(cand);
                        prohibit->insert(cand);
                    }
                    if (good_cand && first_element)
                    {
                        for (smem_weighted_cue_list::iterator wce = weighted_cue.begin(); wce != weighted_cue.end(); wce++)
                        {
                            if ((*wce)->mathElement != NIL)
                            {
                                (*wce)->mathElement->commit();
                }
            }
        }
                    else if (first_element)
                    {
                        for (smem_weighted_cue_list::iterator wce = weighted_cue.begin(); wce != weighted_cue.end(); wce++)
                        {
                            if ((*wce)->mathElement != NIL)
                            {
                                (*wce)->mathElement->rollback();
                            }
                        }
                    }
                }
            }
//            if (!match_ids->empty())
//            {
//                king_id = match_ids->front();
//            }
        }
        q->reinitialize();

        // clean weighted cue
        for (next_element = weighted_cue.begin(); next_element != weighted_cue.end(); next_element++)
        {
            if ((*next_element)->mathElement != NIL)
            {
                delete(*next_element)->mathElement;
            }
            delete(*next_element);
        }
    }

    // reconstruction depends upon level
    if (query_level == qry_full)
    {
        // produce results
        if (king_id != NIL)
        {
            // success!
            smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_success, query);
            if (negquery)
            {
                smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_success, negquery);
            }

            ////////////////////////////////////////////////////////////////////////////
            thisAgent->smem_timers->query->stop();
            ////////////////////////////////////////////////////////////////////////////

            smem_install_memory(thisAgent, state, king_id, NIL, (thisAgent->smem_params->activate_on_query->get_value() == on), meta_wmes, retrieval_wmes);
        }
        else
        {
            smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_failure, query);
            if (negquery)
            {
                smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_failure, negquery);
            }

            ////////////////////////////////////////////////////////////////////////////
            thisAgent->smem_timers->query->stop();
            ////////////////////////////////////////////////////////////////////////////
        }
    }
    else
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->smem_timers->query->stop();
        ////////////////////////////////////////////////////////////////////////////
    }

    return king_id;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization (smem::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_clear_result(agent* thisAgent, Symbol* state)
{
    preference* pref;

    while (!state->id->smem_info->smem_wmes->empty())
    {
        pref = state->id->smem_info->smem_wmes->back();
        state->id->smem_info->smem_wmes->pop_back();

        if (pref->in_tm)
        {
            remove_preference_from_tm(thisAgent, pref);
        }
    }
}

// performs cleanup when a state is removed
void smem_reset(agent* thisAgent, Symbol* state)
{
    if (state == NULL)
    {
        state = thisAgent->top_goal;
    }

    while (state)
    {
        smem_data* data = state->id->smem_info;

        data->last_cmd_time[0] = 0;
        data->last_cmd_time[1] = 0;
        data->last_cmd_count[0] = 0;
        data->last_cmd_count[1] = 0;

        // this will be called after prefs from goal are already removed,
        // so just clear out result stack
        data->smem_wmes->clear();

        state = state->id->lower_goal;
    }
}

void smem_switch_to_memory_db(agent* thisAgent, std::string& buf)
{
    print_sysparam_trace(thisAgent, 0, buf.c_str());
    thisAgent->smem_db->disconnect();
    thisAgent->smem_params->database->set_value(smem_param_container::memory);
    smem_init_db(thisAgent);
}

inline void smem_update_schema_one_to_two(agent* thisAgent)
{
    thisAgent->smem_db->sql_execute("BEGIN TRANSACTION");
    thisAgent->smem_db->sql_execute("CREATE TABLE smem_symbols_type (s_id INTEGER PRIMARY KEY,symbol_type INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_symbols_type (s_id, symbol_type) SELECT id, sym_type FROM smem7_symbols_type");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_symbols_type");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_symbols_string (s_id INTEGER PRIMARY KEY,symbol_value TEXT)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_symbols_string (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_str");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_symbols_str");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_symbols_integer (s_id INTEGER PRIMARY KEY,symbol_value INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_symbols_integer (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_int");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_symbols_int");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_ascii (ascii_num INTEGER PRIMARY KEY,ascii_chr TEXT)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_ascii (ascii_num, ascii_chr) SELECT ascii_num, ascii_num FROM smem7_ascii");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_ascii");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_symbols_float (s_id INTEGER PRIMARY KEY,symbol_value REAL)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_symbols_float (s_id, symbol_value) SELECT id, sym_const FROM smem7_symbols_float");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_symbols_float");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_lti (lti_id INTEGER PRIMARY KEY,soar_letter INTEGER,soar_number INTEGER,total_augmentations INTEGER,activation_value REAL,activations_total INTEGER,activations_last INTEGER,activations_first INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_lti (lti_id, soar_letter, soar_number, total_augmentations, activation_value, activations_total, activations_last, activations_first) SELECT id, letter, num, child_ct, act_value, access_n, access_t, access_1 FROM smem7_lti");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_lti");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_activation_history (lti_id INTEGER PRIMARY KEY,t1 INTEGER,t2 INTEGER,t3 INTEGER,t4 INTEGER,t5 INTEGER,t6 INTEGER,t7 INTEGER,t8 INTEGER,t9 INTEGER,t10 INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_activation_history (lti_id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) SELECT id, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 FROM smem7_history");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_history");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_augmentations (lti_id INTEGER,attribute_s_id INTEGER,value_constant_s_id INTEGER,value_lti_id INTEGER,activation_value REAL)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_augmentations (lti_id, attribute_s_id, value_constant_s_id, value_lti_id, activation_value) SELECT parent_id, attr, val_const, val_lti, act_value FROM smem7_web");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_web");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_attribute_frequency (attribute_s_id INTEGER PRIMARY KEY,edge_frequency INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_attribute_frequency (attribute_s_id, edge_frequency) SELECT attr, ct FROM smem7_ct_attr");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_ct_attr");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_wmes_constant_frequency (attribute_s_id INTEGER,value_constant_s_id INTEGER,edge_frequency INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id, edge_frequency) SELECT attr, val_const, ct FROM smem7_ct_const");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_ct_const");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_wmes_lti_frequency (attribute_s_id INTEGER,value_lti_id INTEGER,edge_frequency INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_wmes_lti_frequency (attribute_s_id, value_lti_id, edge_frequency) SELECT attr, val_lti, ct FROM smem7_ct_lti");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_ct_lti");

    thisAgent->smem_db->sql_execute("CREATE TABLE smem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value INTEGER)");
    thisAgent->smem_db->sql_execute("INSERT INTO smem_persistent_variables (variable_id, variable_value) SELECT id, value FROM smem7_vars");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_vars");

    thisAgent->smem_db->sql_execute("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    thisAgent->smem_db->sql_execute("INSERT INTO versions (system, version_number) VALUES ('smem_schema','2.0')");
    thisAgent->smem_db->sql_execute("DROP TABLE smem7_signature");

    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_int_const ON smem_symbols_integer (symbol_value)");
    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_ct_lti_attr_val ON smem_wmes_lti_frequency (attribute_s_id, value_lti_id)");
    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_float_const ON smem_symbols_float (symbol_value)");
    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_symbols_str_const ON smem_symbols_string (symbol_value)");
    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_lti_letter_num ON smem_lti (soar_letter,soar_number)");
    thisAgent->smem_db->sql_execute("CREATE INDEX smem_lti_t ON smem_lti (activations_last)");
    thisAgent->smem_db->sql_execute("CREATE INDEX smem_augmentations_parent_attr_val_lti ON smem_augmentations (lti_id, attribute_s_id, value_constant_s_id,value_lti_id)");
    thisAgent->smem_db->sql_execute("CREATE INDEX smem_augmentations_attr_val_lti_cycle ON smem_augmentations (attribute_s_id, value_constant_s_id, value_lti_id, activation_value)");
    thisAgent->smem_db->sql_execute("CREATE INDEX smem_augmentations_attr_cycle ON smem_augmentations (attribute_s_id, activation_value)");
    thisAgent->smem_db->sql_execute("CREATE UNIQUE INDEX smem_wmes_constant_frequency_attr_val ON smem_wmes_constant_frequency (attribute_s_id, value_constant_s_id)");
    thisAgent->smem_db->sql_execute("COMMIT");
}

// opens the SQLite database and performs all initialization required for the current mode
void smem_init_db(agent* thisAgent)
{
    if (thisAgent->smem_db->get_status() != soar_module::disconnected)
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->init->start();
    ////////////////////////////////////////////////////////////////////////////

    const char* db_path;
    bool tabula_rasa;

    if (thisAgent->smem_params->database->get_value() == smem_param_container::memory)
    {
        db_path = ":memory:";
        tabula_rasa = true;
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory database in cpu memory.\n");
    }
    else
    {
        db_path = thisAgent->smem_params->path->get_value();
        print_sysparam_trace(thisAgent, TRACE_SMEM_SYSPARAM, "Initializing semantic memory memory database at %s\n", db_path);
    }

    // attempt connection
    thisAgent->smem_db->connect(db_path);

    if (thisAgent->smem_db->get_status() == soar_module::problem)
    {
        print_sysparam_trace(thisAgent, 0, "Semantic memory database Error: %s\n", thisAgent->smem_db->get_errmsg());
    }
    else
    {
        // temporary queries for one-time init actions
        soar_module::sqlite_statement* temp_q = NULL;

        // If the database is on file, make sure the database contents use the current schema
        // If it does not, switch to memory-based database

        if (strcmp(db_path, ":memory:")) // Check if database mode is to a file
        {
            bool switch_to_memory, versions_exists, sql_is_new;
            std::string schema_version, version_error_message;

            /* -- Set switch_to_memory true in case we have any errors with the database -- */
            switch_to_memory = true;

            if (thisAgent->smem_db->sql_is_new_db(sql_is_new))
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
                    temp_q = new soar_module::sqlite_statement(thisAgent->smem_db, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (thisAgent->smem_db->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'smem_schema'", schema_version))
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
                            if (smem_version_one(thisAgent))
                            {
                                version_error_message.assign("...Version of semantic memory database is old.\n"
                                                             "...Converting to version 2.0.\n");
                                smem_update_schema_one_to_two(thisAgent);
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
                        if (smem_version_one(thisAgent))
                        {
                            version_error_message.assign("...Version of semantic memory database is old.\n"
                                                         "...Converting to version 2.0.\n");
                            smem_update_schema_one_to_two(thisAgent);
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
                smem_switch_to_memory_db(thisAgent, version_error_message);
                return;
            }
        }

        // apply performance options
        {
            // page_size
            {
                switch (thisAgent->smem_params->page_size->get_value())
                {
                    case (smem_param_container::page_1k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 1024");
                        break;

                    case (smem_param_container::page_2k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 2048");
                        break;

                    case (smem_param_container::page_4k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 4096");
                        break;

                    case (smem_param_container::page_8k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 8192");
                        break;

                    case (smem_param_container::page_16k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 16384");
                        break;

                    case (smem_param_container::page_32k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 32768");
                        break;

                    case (smem_param_container::page_64k):
                        thisAgent->smem_db->sql_execute("PRAGMA page_size = 65536");
                        break;
                }
            }

            // cache_size
            {
                std::string cache_sql("PRAGMA cache_size = ");
                char* str = thisAgent->smem_params->cache_size->get_string();
                cache_sql.append(str);
                free(str);
                str = NULL;
                thisAgent->smem_db->sql_execute(cache_sql.c_str());
            }

            // optimization
            if (thisAgent->smem_params->opt->get_value() == smem_param_container::opt_speed)
            {
                // synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
                thisAgent->smem_db->sql_execute("PRAGMA synchronous = OFF");

                // journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
                thisAgent->smem_db->sql_execute("PRAGMA journal_mode = OFF");

                // locking_mode - no one else can view the database after our first write
                thisAgent->smem_db->sql_execute("PRAGMA locking_mode = EXCLUSIVE");
            }
        }

        // update validation count
        thisAgent->smem_validation++;

        // setup common structures/queries
        thisAgent->smem_stmts = new smem_statement_container(thisAgent);

        if (tabula_rasa || (thisAgent->smem_params->append_db->get_value() == off))
        {
            thisAgent->smem_stmts->structure();
        }

        // initialize queries given database structure
        thisAgent->smem_stmts->prepare();

        // initialize persistent variables
        if (tabula_rasa || (thisAgent->smem_params->append_db->get_value() == off))
        {
            thisAgent->smem_stmts->begin->execute(soar_module::op_reinit);
            {
                // max cycle
                thisAgent->smem_max_cycle = static_cast<int64_t>(1);
                smem_variable_create(thisAgent, var_max_cycle, 1);

                // number of nodes
                thisAgent->smem_stats->chunks->set_value(0);
                smem_variable_create(thisAgent, var_num_nodes, 0);

                // number of edges
                thisAgent->smem_stats->slots->set_value(0);
                smem_variable_create(thisAgent, var_num_edges, 0);

                // threshold (from user parameter value)
                smem_variable_create(thisAgent, var_act_thresh, static_cast<int64_t>(thisAgent->smem_params->thresh->get_value()));

                // activation mode (from user parameter value)
                smem_variable_create(thisAgent, var_act_mode, static_cast<int64_t>(thisAgent->smem_params->activation_mode->get_value()));
            }
            thisAgent->smem_stmts->commit->execute(soar_module::op_reinit);
        }
        else
        {
            int64_t temp;

            // max cycle
            smem_variable_get(thisAgent, var_max_cycle, &(thisAgent->smem_max_cycle));

            // number of nodes
            smem_variable_get(thisAgent, var_num_nodes, &(temp));
            thisAgent->smem_stats->chunks->set_value(temp);

            // number of edges
            smem_variable_get(thisAgent, var_num_edges, &(temp));
            thisAgent->smem_stats->slots->set_value(temp);

            // threshold
            smem_variable_get(thisAgent, var_act_thresh, &(temp));
            thisAgent->smem_params->thresh->set_value(temp);

            // activation mode
            smem_variable_get(thisAgent, var_act_mode, &(temp));
            thisAgent->smem_params->activation_mode->set_value(static_cast< smem_param_container::act_choices >(temp));
        }

        // reset identifier counters
        smem_reset_id_counters(thisAgent);

        // if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
        if (thisAgent->smem_params->lazy_commit->get_value() == on)
        {
            thisAgent->smem_stmts->begin->execute(soar_module::op_reinit);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->smem_timers->init->stop();
    ////////////////////////////////////////////////////////////////////////////
}

void smem_attach(agent* thisAgent)
{
    if (thisAgent->smem_db->get_status() == soar_module::disconnected)
    {
        smem_init_db(thisAgent);
    }
}

inline void _smem_close_vars(agent* thisAgent)
{
    // store max cycle for future use of the smem database
    smem_variable_set(thisAgent, var_max_cycle, thisAgent->smem_max_cycle);

    // store num nodes/edges for future use of the smem database
    smem_variable_set(thisAgent, var_num_nodes, thisAgent->smem_stats->chunks->get_value());
    smem_variable_set(thisAgent, var_num_edges, thisAgent->smem_stats->slots->get_value());
}

// performs cleanup operations when the database needs to be closed (end soar, manual close, etc)
void smem_close(agent* thisAgent)
{
    if (thisAgent->smem_db->get_status() == soar_module::connected)
    {
        _smem_close_vars(thisAgent);

        // if lazy, commit
        if (thisAgent->smem_params->lazy_commit->get_value() == on)
        {
            thisAgent->smem_stmts->commit->execute(soar_module::op_reinit);
        }

        // de-allocate common statements
        delete thisAgent->smem_stmts;

        // close the database
        thisAgent->smem_db->disconnect();
    }
}

void smem_reinit_cmd(agent* thisAgent)
{
    smem_close(thisAgent);
//    smem_init_db(thisAgent);
}

void smem_reinit(agent* thisAgent)
{
    if (thisAgent->smem_db->get_status() == soar_module::connected)
    {
        if (thisAgent->smem_params->append_db->get_value() == off)
        {
            smem_close(thisAgent);
            smem_init_db(thisAgent);
        }
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parsing (smem::parse)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_deallocate_chunk(agent* thisAgent, smem_chunk* chunk, bool free_chunk = true)
{
    if (chunk)
    {
        // proceed to slots
        if (chunk->slots)
        {
            smem_slot_map::iterator s;
            smem_slot::iterator v;

            // iterate over slots
            while (!chunk->slots->empty())
            {
                s = chunk->slots->begin();

                // proceed to slot contents
                if (s->second)
                {
                    // iterate over each value
                    for (v = s->second->begin(); v != s->second->end(); v = s->second->erase(v))
                    {
                        // de-allocation of value is dependent upon type
                        if ((*v)->val_const.val_type == value_const_t)
                        {
                            symbol_remove_ref(thisAgent, (*v)->val_const.val_value);
                        }
                        else
                        {
                            // we never deallocate the lti chunk, as we assume
                            // it will exist elsewhere for deallocation
                            // delete (*s)->val_lti.val_value;
                        }

                        delete(*v);
                    }

                    delete s->second;
                }

                // deallocate attribute for each corresponding value
                symbol_remove_ref(thisAgent, s->first);

                chunk->slots->erase(s);
            }

            // remove slots
            delete chunk->slots;
            chunk->slots = NULL;
        }

        // remove chunk itself
        if (free_chunk)
        {
            delete chunk;
            chunk = NULL;
        }
    }
}

inline std::string* smem_parse_lti_name(struct lexeme_info* lexeme, char* id_letter, uint64_t* id_number)
{
    std::string* return_val = new std::string;

    if ((*lexeme).type == IDENTIFIER_LEXEME)
    {
        std::string soar_number;
        to_string((*lexeme).id_number, soar_number);

        return_val->append(1, (*lexeme).id_letter);
        return_val->append(soar_number);

        (*id_letter) = (*lexeme).id_letter;
        (*id_number) = (*lexeme).id_number;
    }
    else
    {
        return_val->assign((*lexeme).string);

        (*id_letter) = static_cast<char>(toupper((*lexeme).string[1]));
        (*id_number) = 0;
    }

    return return_val;
}

inline Symbol* smem_parse_constant_attr(agent* thisAgent, struct lexeme_info* lexeme)
{
    Symbol* return_val = NIL;

    if ((*lexeme).type == STR_CONSTANT_LEXEME)
    {
        return_val = make_str_constant(thisAgent, static_cast<const char*>((*lexeme).string));
    }
    else if ((*lexeme).type == INT_CONSTANT_LEXEME)
    {
        return_val = make_int_constant(thisAgent, (*lexeme).int_val);
    }
    else if ((*lexeme).type == FLOAT_CONSTANT_LEXEME)
    {
        return_val = make_float_constant(thisAgent, (*lexeme).float_val);
    }

    return return_val;
}

bool smem_parse_chunk(agent* thisAgent, smem_str_to_chunk_map* chunks, smem_chunk_set* newbies)
{
    bool return_val = false;

    smem_chunk* new_chunk = new smem_chunk;
    new_chunk->slots = NULL;

    std::string* chunk_name = NULL;

    char temp_letter;
    uint64_t temp_number;

    bool good_at;

    //

    // consume left paren
    get_lexeme(thisAgent);

    if ((thisAgent->lexeme.type == AT_LEXEME) || (thisAgent->lexeme.type == IDENTIFIER_LEXEME) || (thisAgent->lexeme.type == VARIABLE_LEXEME))
    {
        good_at = true;

        if (thisAgent->lexeme.type == AT_LEXEME)
        {
            get_lexeme(thisAgent);

            good_at = (thisAgent->lexeme.type == IDENTIFIER_LEXEME);
        }

        if (good_at)
        {
            // save identifier
            chunk_name = smem_parse_lti_name(&(thisAgent->lexeme), &(temp_letter), &(temp_number));
            new_chunk->lti_letter = temp_letter;
            new_chunk->lti_number = temp_number;
            new_chunk->lti_id = NIL;
            new_chunk->soar_id = NIL;
            new_chunk->slots = new smem_slot_map;

            // consume id
            get_lexeme(thisAgent);

            //

            uint64_t intermediate_counter = 1;
            smem_chunk* intermediate_parent;
            smem_chunk* temp_chunk;
            std::string temp_key;
            std::string* temp_key2;
            Symbol* chunk_attr;
            smem_chunk_value* chunk_value;
            smem_slot* s;

            // populate slots
            while (thisAgent->lexeme.type == UP_ARROW_LEXEME)
            {
                intermediate_parent = new_chunk;

                // go on to attribute
                get_lexeme(thisAgent);

                // get the appropriate constant type
                chunk_attr = smem_parse_constant_attr(thisAgent, &(thisAgent->lexeme));

                // if constant attribute, proceed to value
                if (chunk_attr != NIL)
                {
                    // consume attribute
                    get_lexeme(thisAgent);

                    // support for dot notation:
                    // when we encounter a dot, instantiate
                    // the previous attribute as a temporary
                    // identifier and use that as the parent
                    while (thisAgent->lexeme.type == PERIOD_LEXEME)
                    {
                        // create a new chunk
                        temp_chunk = new smem_chunk;
                        temp_chunk->lti_letter = ((chunk_attr->symbol_type == STR_CONSTANT_SYMBOL_TYPE) ? (static_cast<char>(static_cast<int>(chunk_attr->sc->name[0]))) : ('X'));
                        temp_chunk->lti_number = (intermediate_counter++);
                        temp_chunk->lti_id = NIL;
                        temp_chunk->slots = new smem_slot_map;
                        temp_chunk->soar_id = NIL;

                        // add it as a child to the current parent
                        chunk_value = new smem_chunk_value;
                        chunk_value->val_lti.val_type = value_lti_t;
                        chunk_value->val_lti.val_value = temp_chunk;
                        s = smem_make_slot(intermediate_parent->slots, chunk_attr);
                        s->push_back(chunk_value);

                        // create a key guaranteed to be unique
                        std::string temp_key3;
                        to_string(temp_chunk->lti_number, temp_key3);
                        temp_key.assign("<");
                        temp_key.append(1, temp_chunk->lti_letter);
                        temp_key.append("#");
                        temp_key.append(temp_key3);
                        temp_key.append(">");

                        // insert the new chunk
                        (*chunks)[ temp_key ] = temp_chunk;

                        // definitely a new chunk
                        newbies->insert(temp_chunk);

                        // the new chunk is our parent for this set of values (or further dots)
                        intermediate_parent = temp_chunk;
                        temp_chunk = NULL;

                        // get the next attribute
                        get_lexeme(thisAgent);
                        chunk_attr = smem_parse_constant_attr(thisAgent, &(thisAgent->lexeme));

                        // consume attribute
                        get_lexeme(thisAgent);
                    }

                    if (chunk_attr != NIL)
                    {
                        bool first_value = true;

                        do
                        {
                            // value by type
                            chunk_value = NIL;
                            if (thisAgent->lexeme.type == STR_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = make_str_constant(thisAgent, static_cast<const char*>(thisAgent->lexeme.string));
                            }
                            else if (thisAgent->lexeme.type == INT_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = make_int_constant(thisAgent, thisAgent->lexeme.int_val);
                            }
                            else if (thisAgent->lexeme.type == FLOAT_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = make_float_constant(thisAgent, thisAgent->lexeme.float_val);
                            }
                            else if ((thisAgent->lexeme.type == AT_LEXEME) || (thisAgent->lexeme.type == IDENTIFIER_LEXEME) || (thisAgent->lexeme.type == VARIABLE_LEXEME))
                            {
                                good_at = true;

                                if (thisAgent->lexeme.type == AT_LEXEME)
                                {
                                    get_lexeme(thisAgent);

                                    good_at = (thisAgent->lexeme.type == IDENTIFIER_LEXEME);
                                }

                                if (good_at)
                                {
                                    // create new value
                                    chunk_value = new smem_chunk_value;
                                    chunk_value->val_lti.val_type = value_lti_t;

                                    // get key
                                    temp_key2 = smem_parse_lti_name(&(thisAgent->lexeme), &(temp_letter), &(temp_number));

                                    // search for an existing chunk
                                    smem_str_to_chunk_map::iterator p = chunks->find((*temp_key2));

                                    // if exists, point; else create new
                                    if (p != chunks->end())
                                    {
                                        chunk_value->val_lti.val_value = p->second;
                                    }
                                    else
                                    {
                                        // create new chunk
                                        temp_chunk = new smem_chunk;
                                        temp_chunk->lti_id = NIL;
                                        temp_chunk->lti_letter = temp_letter;
                                        temp_chunk->lti_number = temp_number;
                                        temp_chunk->lti_id = NIL;
                                        temp_chunk->slots = NIL;
                                        temp_chunk->soar_id = NIL;

                                        // associate with value
                                        chunk_value->val_lti.val_value = temp_chunk;

                                        // add to chunks
                                        (*chunks)[(*temp_key2) ] = temp_chunk;

                                        // possibly a newbie (could be a self-loop)
                                        newbies->insert(temp_chunk);
                                    }

                                    delete temp_key2;
                                }
                            }

                            if (chunk_value != NIL)
                            {
                                // consume
                                get_lexeme(thisAgent);

                                // add to appropriate slot
                                s = smem_make_slot(intermediate_parent->slots, chunk_attr);
                                if (first_value && !s->empty())
                                {
                                    // in the case of a repeated attribute, remove ref here to avoid leak
                                    symbol_remove_ref(thisAgent, chunk_attr);
                                }
                                s->push_back(chunk_value);

                                // if this was the last attribute
                                if (thisAgent->lexeme.type == R_PAREN_LEXEME)
                                {
                                    return_val = true;
                                    get_lexeme(thisAgent);
                                    chunk_value = NIL;
                                }

                                first_value = false;
                            }
                        }
                        while (chunk_value != NIL);
                    }
                }
            }
        }
        else
        {
            delete new_chunk;
        }
    }
    else
    {
        delete new_chunk;
    }

    if (return_val)
    {
        // search for an existing chunk (occurs if value comes before id)
        smem_chunk** p = & (*chunks)[(*chunk_name) ];

        if (!(*p))
        {
            (*p) = new_chunk;

            // a newbie!
            newbies->insert(new_chunk);
        }
        else
        {
            // transfer slots
            if (!(*p)->slots)
            {
                // if none previously, can just use
                (*p)->slots = new_chunk->slots;
                new_chunk->slots = NULL;
            }
            else
            {
                // otherwise, copy

                smem_slot_map::iterator ss_p;
                smem_slot::iterator s_p;

                smem_slot* source_slot;
                smem_slot* target_slot;

                // for all slots
                for (ss_p = new_chunk->slots->begin(); ss_p != new_chunk->slots->end(); ss_p++)
                {
                    target_slot = smem_make_slot((*p)->slots, ss_p->first);
                    source_slot = ss_p->second;

                    // for all values in the slot
                    for (s_p = source_slot->begin(); s_p != source_slot->end(); s_p++)
                    {
                        // copy each value
                        target_slot->push_back((*s_p));
                    }

                    // once copied, we no longer need the slot
                    delete source_slot;
                }

                // we no longer need the slots
                delete new_chunk->slots;
                new_chunk->slots = NULL;
            }

            // contents are new
            newbies->insert((*p));

            // deallocate
            smem_deallocate_chunk(thisAgent, new_chunk);
        }
    }
    else
    {
        newbies->clear();
    }

    // de-allocate id name
    if (chunk_name)
    {
        delete chunk_name;
    }

    return return_val;
}

bool smem_parse_chunks(agent* thisAgent, const char* chunks_str, std::string** err_msg)
{
    bool return_val = false;
    uint64_t clause_count = 0;

    // parsing chunks requires an open semantic database
    smem_attach(thisAgent);

    // copied primarily from cli_sp
    thisAgent->alternate_input_string = chunks_str;
    thisAgent->alternate_input_suffix = const_cast<char*>(") ");
    thisAgent->current_char = ' ';
    thisAgent->alternate_input_exit = true;
    set_lexer_allow_ids(thisAgent, true);

    bool good_chunk = true;

    smem_str_to_chunk_map chunks;
    smem_str_to_chunk_map::iterator c_old;

    smem_chunk_set newbies;
    smem_chunk_set::iterator c_new;

    // consume next token
    get_lexeme(thisAgent);

    if (thisAgent->lexeme.type != L_PAREN_LEXEME)
    {
        good_chunk = false;
    }

    // while there are chunks to consume
    while ((thisAgent->lexeme.type == L_PAREN_LEXEME) && (good_chunk))
    {
        good_chunk = smem_parse_chunk(thisAgent, &(chunks), &(newbies));

        if (good_chunk)
        {
            // add all newbie lti's as appropriate
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->lti_id == NIL)
                {
                    // deal differently with variable vs. lti
                    if ((*c_new)->lti_number == NIL)
                    {
                        // add a new lti id (we have a guarantee this won't be in Soar's WM)
                        (*c_new)->lti_number = (thisAgent->id_counter[(*c_new)->lti_letter - static_cast<char>('A') ]++);
                        (*c_new)->lti_id = smem_lti_add_id(thisAgent, (*c_new)->lti_letter, (*c_new)->lti_number);
                    }
                    else
                    {
                        // should ALWAYS be the case (it's a newbie and we've initialized lti_id to NIL)
                        if ((*c_new)->lti_id == NIL)
                        {
                            // get existing
                            (*c_new)->lti_id = smem_lti_get_id(thisAgent, (*c_new)->lti_letter, (*c_new)->lti_number);

                            // if doesn't exist, add it
                            if ((*c_new)->lti_id == NIL)
                            {
                                (*c_new)->lti_id = smem_lti_add_id(thisAgent, (*c_new)->lti_letter, (*c_new)->lti_number);

                                // this could affect an existing identifier in Soar's WM
                                Symbol* id_parent = find_identifier(thisAgent, (*c_new)->lti_letter, (*c_new)->lti_number);
                                if (id_parent != NIL)
                                {
                                    // if so we make it an lti manually
                                    id_parent->id->smem_lti = (*c_new)->lti_id;

                                    id_parent->id->smem_time_id = thisAgent->epmem_stats->time->get_value();
                                    id_parent->id->smem_valid = thisAgent->epmem_validation;
                                    epmem_schedule_promotion(thisAgent, id_parent);
                                }
                            }
                        }
                    }
                }
            }

            // add all newbie contents (append, as opposed to replace, children)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->slots != NIL)
                {
                    smem_store_chunk(thisAgent, (*c_new)->lti_id, (*c_new)->slots, false);
                }
            }

            // deallocate *contents* of all newbies (need to keep around name->id association for future chunks)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                smem_deallocate_chunk(thisAgent, (*c_new), false);
            }

            // increment clause counter
            clause_count++;

            // clear newbie list
            newbies.clear();
        }
    };

    return_val = good_chunk;

    // deallocate all chunks
    {
        for (c_old = chunks.begin(); c_old != chunks.end(); c_old++)
        {
            smem_deallocate_chunk(thisAgent, c_old->second, true);
        }
    }

    // produce error message on failure
    if (!return_val)
    {
        std::string num;
        to_string(clause_count, num);

        (*err_msg)->append("Error parsing clause #");
        (*err_msg)->append(num);
    }

    return return_val;
}

/* The following function is supposed to read in the lexemes
 * and turn them into the cue wme for a call to smem_process_query.
 * This is intended to be run from the command line and does not yet have
 * full functionality. It doesn't work with mathqueries, for example.
 * This is for debugging purposes.
 * -Steven 23-7-2014
 */

bool smem_parse_cues(agent* thisAgent, const char* chunks_str, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve)
{
    uint64_t clause_count = 0;  // This is counting up the number of parsed clauses
    // so that there is a pointer to a failure location.

    //Parsing requires an open semantic database.
    smem_attach(thisAgent);

    //Next 5 lines copied as in smem_parse_chunks.
    thisAgent->alternate_input_string = chunks_str;
    thisAgent->alternate_input_suffix = const_cast<char*>(")");
    thisAgent->current_char = ' ';
    thisAgent->alternate_input_exit = true;
    set_lexer_allow_ids(thisAgent, true);

    bool good_cue = true;   // This is a success or failure flag that will be checked periodically
    // and indicates whether or not we can call smem_process_query.

    std::map<std::string, Symbol*> cue_ids; //I want to keep track of previous references when adding a new element to the cue.

    //consume next token.
    get_lexeme(thisAgent);

    good_cue = thisAgent->lexeme.type == L_PAREN_LEXEME;

    Symbol* root_cue_id = NIL;    //This is the id that gets passed to smem_process_query.
    //It's main purpose is to contain augmentations
    Symbol* negative_cues;  //This is supposed to contain the negative augmentations.

    bool trigger_first = true; //Just for managing my loop.
    bool minus_ever = false; //Did a negative cue ever show up?
    bool first_attribute = true; //Want to make sure there is a positive attribute to begin with.

    // While there is parsing to be done:
    while ((thisAgent->lexeme.type == L_PAREN_LEXEME) && good_cue)
    {
        //First, consume the left paren.
        get_lexeme(thisAgent);

        if (trigger_first)
        {
            good_cue = thisAgent->lexeme.type == VARIABLE_LEXEME;

            if (good_cue)
            {
                root_cue_id = make_new_identifier(thisAgent, (char) thisAgent->lexeme.string[1], 1);
                cue_ids[thisAgent->lexeme.string] = root_cue_id;
                negative_cues = make_new_identifier(thisAgent, (char) thisAgent->lexeme.string[1], 1);
            }
            else
            {
                (*err_msg)->append("Error: The cue must be a variable.\n");//Spit out that the cue must be a variable.
                break;
            }

            trigger_first = false;
        }
        else
        {
            //If this isn't the first time around, then this better be the same as the root_cue_id variable.
            good_cue = cue_ids[thisAgent->lexeme.string] == root_cue_id;
            if (!good_cue)
            {
                (*err_msg)->append("Error: Additional clauses must share same variable.\n");//Spit out that additional clauses must share the same variable as the original cue variable.
                break;
            }
        }

        if (good_cue)
        {
            //Consume the root_cue_id
            get_lexeme(thisAgent);

            Symbol* attribute;
            slot* temp_slot;

            //Now, we process the attributes of the cue id contained in this clause.
            bool minus = false;

            //Loop as long as positive or negative cues keep popping up.
            while (good_cue && (thisAgent->lexeme.type == UP_ARROW_LEXEME || thisAgent->lexeme.type == MINUS_LEXEME))
            {
                if (thisAgent->lexeme.type == MINUS_LEXEME)
                {
                    minus_ever = true;
                    if (first_attribute)
                    {
                        good_cue = false;
                        break;
                    }
                    get_lexeme(thisAgent);
                    good_cue = thisAgent->lexeme.type == UP_ARROW_LEXEME;
                    minus = true;
                }
                else
                {
                    minus = false;
                }

                get_lexeme(thisAgent);//Consume the up arrow and move on to the attribute.

                if (thisAgent->lexeme.type == VARIABLE_LEXEME)
                {
                    //SMem doesn't suppose variable attributes ... YET.
                    good_cue = false;
                    break;
                }

                // TODO: test to make sure this is good. Previously there was no test
                // for the type of the lexeme so passing a "(" caused a segfault when making the slot.
                attribute = smem_parse_constant_attr(thisAgent, &(thisAgent->lexeme));
                if (attribute == NIL)
                {
                    good_cue = false;
                    break;
                }

                Symbol* value;
                wme* temp_wme;

                if (minus)
                {
                    temp_slot = make_slot(thisAgent, negative_cues, attribute);
                }
                else
                {
                    temp_slot = make_slot(thisAgent, root_cue_id, attribute); //Make a slot for this attribute, or return slot it already has.
                }

                //consume the attribute.
                get_lexeme(thisAgent);
                bool hasAddedValue = false;

                do //Add value by type
                {
                    value = NIL;
                    if (thisAgent->lexeme.type == STR_CONSTANT_LEXEME)
                    {
                        value = make_str_constant(thisAgent, static_cast<const char*>(thisAgent->lexeme.string));
                        get_lexeme(thisAgent);
                    }
                    else if (thisAgent->lexeme.type == INT_CONSTANT_LEXEME)
                    {
                        value = make_int_constant(thisAgent, thisAgent->lexeme.int_val);
                        get_lexeme(thisAgent);
                    }
                    else if (thisAgent->lexeme.type == FLOAT_CONSTANT_LEXEME)
                    {
                        value = make_float_constant(thisAgent, thisAgent->lexeme.float_val);
                        get_lexeme(thisAgent);
                    }
                    else if (thisAgent->lexeme.type == AT_LEXEME)
                    {
                        //If the LTI isn't recognized, then it cannot be a good cue.
                        get_lexeme(thisAgent);
                        smem_lti_id value_id = smem_lti_get_id(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
                        if (value_id == NIL)
                        {
                            good_cue = false;
                            (*err_msg)->append("Error: LTI was not found.\n");
                            break;
                        }
                        else
                        {
                            value = smem_lti_soar_make(thisAgent, value_id, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number, SMEM_LTI_UNKNOWN_LEVEL);
                        }
                        get_lexeme(thisAgent);
                    }
                    else if (thisAgent->lexeme.type == VARIABLE_LEXEME || thisAgent->lexeme.type == IDENTIFIER_LEXEME)
                    {
                        std::map<std::basic_string<char>, Symbol*>::iterator value_iterator;
                        value_iterator = cue_ids.find(thisAgent->lexeme.string);

                        if (value_iterator == cue_ids.end())
                        {
                            value = make_new_identifier(thisAgent, (char) thisAgent->lexeme.string[0], 1);
                            cue_ids[thisAgent->lexeme.string] = value; //Keep track of created symbols for deletion later.
                        }
                        get_lexeme(thisAgent);
                    }
                    else
                    {
                        if (((thisAgent->lexeme.type == R_PAREN_LEXEME || thisAgent->lexeme.type == UP_ARROW_LEXEME) || thisAgent->lexeme.type == MINUS_LEXEME) && hasAddedValue)
                        {
                            //good_cue = true;
                            break;
                        }
                        else
                        {
                            good_cue = false;
                            break;
                        }
                    }

                    if (value != NIL && good_cue)
                    {
                        //Value might be nil, but R_paren or next attribute could make it a good cue.
                        hasAddedValue = true;
                        if (minus)
                        {
                            temp_wme = make_wme(thisAgent, negative_cues, attribute, value, false);
                        }
                        else
                        {
                            temp_wme = make_wme(thisAgent, root_cue_id, attribute, value, false);
                        }
                        insert_at_head_of_dll(temp_slot->wmes, temp_wme, next, prev); //Put the wme in the slow for the attribute.
                    }
                }
                while (value != NIL);  //Loop until there are no more value lexemes to add to that attribute.

                first_attribute = false;

            }
        }
        else
        {
            break;
        }

        while (thisAgent->lexeme.type == R_PAREN_LEXEME)
        {
            get_lexeme(thisAgent);
        }

        clause_count++;
        trigger_first = false; //It is no longer the first clause.

    }
    if (!good_cue)
    {
        std::string num;
        to_string(clause_count, num);

        (*err_msg)->append("Error parsing clause #");
        (*err_msg)->append(num + ".");
    }
    else
    {
        smem_lti_set* prohibit = new smem_lti_set;
        soar_module::wme_set cue_wmes;
        soar_module::symbol_triple_list meta_wmes;
        soar_module::symbol_triple_list retrieval_wmes;
        (*result_message) = new std::string();

        std::list<smem_lti_id> match_ids;

        smem_process_query(thisAgent, NIL, root_cue_id, minus_ever ? negative_cues : NIL, NIL, prohibit, cue_wmes, meta_wmes, retrieval_wmes, qry_search, number_to_retrieve, &(match_ids));

        if (!match_ids.empty())
        {
            for (std::list<smem_lti_id>::const_iterator id = match_ids.begin(), end = match_ids.end(); id != end; ++id)
            {
                smem_print_lti(thisAgent, (*id), 1, *result_message); //"1" is the depth.
            }
        }
        else
        {
            (*result_message)->append("SMem| No results for query.");
        }
        delete prohibit;

    }

    /*
     * Below is the clean-up
     */
    if (root_cue_id != NIL)
    {
        slot* s;

        for (s = root_cue_id->id->slots; s != NIL; s = s->next)
        {
            //Remove all wme's from the slot.
            wme* delete_wme;
            for (delete_wme = s->wmes; delete_wme != NIL; delete_wme = delete_wme->next)
            {
                symbol_remove_ref(thisAgent, delete_wme->value);
                deallocate_wme(thisAgent, delete_wme);
            }

            s->wmes = NIL;
            symbol_remove_ref(thisAgent, s->attr);
            mark_slot_for_possible_removal(thisAgent, s);
        }//End of for-slots loop.
        root_cue_id->id->slots = NIL;

        for (s = negative_cues->id->slots; s != NIL; s = s->next)
        {
            //Remove all wme's from the slot.
            wme* delete_wme;
            for (delete_wme = s->wmes; delete_wme != NIL; delete_wme = delete_wme->next)
            {
                symbol_remove_ref(thisAgent, delete_wme->value);
                deallocate_wme(thisAgent, delete_wme);
            }

            s->wmes = NIL;
            symbol_remove_ref(thisAgent, s->attr);
            mark_slot_for_possible_removal(thisAgent, s);
        }//End of for-slots loop.
        negative_cues->id->slots = NIL;

        symbol_remove_ref(thisAgent, root_cue_id);//gets rid of cue id.
        symbol_remove_ref(thisAgent, negative_cues);//gets rid of negative cues id.
    }

    return good_cue;
}

/*
 * This is intended to allow the user to remove part or all of information stored on a LTI.
 * (All attributes, selected attributes, or just values from particular attributes.)
 */
bool smem_parse_remove(agent* thisAgent, const char* chunks_str, std::string** err_msg, std::string** result_message, bool force)
{
    //TODO: need to fix so that err_msg and result_message are actually used or not passed.
    bool good_command = true;

    //parsing chunks requires an open semantic database
    smem_attach(thisAgent);

    //copied primarily from cli_sp
    thisAgent->alternate_input_string = chunks_str;
    thisAgent->alternate_input_suffix = const_cast<char*>(") ");
    thisAgent->current_char = ' ';
    thisAgent->alternate_input_exit = true;
    set_lexer_allow_ids(thisAgent, true);//This is the end of the incantation.

    get_lexeme(thisAgent);

    if (thisAgent->lexeme.type == L_PAREN_LEXEME)
    {
        get_lexeme(thisAgent);//Consumes the left paren
    }

    if (thisAgent->lexeme.type == AT_LEXEME && good_command)
    {
        get_lexeme(thisAgent);
    }

    good_command = thisAgent->lexeme.type == IDENTIFIER_LEXEME;

    smem_lti_id lti_id;

    if (good_command)
    {
        lti_id = smem_lti_get_id(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
    }
    else
    {
        (*err_msg)->append("Error: No LTI found for that letter and number.\n");
    }

    soar_module::symbol_triple_list retrieval_wmes;
    soar_module::symbol_triple_list meta_wmes;

    if (good_command && lti_id != NIL)
    {
        Symbol* lti = smem_lti_soar_make(thisAgent, lti_id, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number, SMEM_LTI_UNKNOWN_LEVEL);

        get_lexeme(thisAgent);//Consume the identifier.

        smem_slot_map children;

        if (thisAgent->lexeme.type == UP_ARROW_LEXEME)
        {
            //Now that we know we have a good lti, we can do a NCBR so that we know what attributes and values we can delete.
            //"--force" will ignore attempts to delete that which isn't there, while the default will be to stop and report back.
            smem_install_memory(thisAgent, NIL, lti_id, lti, false, meta_wmes, retrieval_wmes, fake_install);

            //First, we'll create the slot_map according to retrieval_wmes, then we'll remove what we encounter during parsing.
            soar_module::symbol_triple_list::iterator triple_ptr_iter;
            smem_slot* temp_slot;
            for (triple_ptr_iter = retrieval_wmes.begin(); triple_ptr_iter != retrieval_wmes.end(); triple_ptr_iter++)
            {
                if (children.count((*triple_ptr_iter)->attr)) //If the attribute is already in the map.
                {
                    temp_slot = (children.find((*triple_ptr_iter)->attr)->second);
                    smem_chunk_value* temp_val = new smem_chunk_value;
                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                    {
                        //If the chunk was retrieved and it is an identifier it is lti.
                        smem_chunk_value_lti temp_lti;
                        smem_chunk_value_constant temp_const;
                        temp_val->val_const = temp_const;
                        temp_val->val_const.val_type = value_lti_t;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_lti_t;
                        smem_chunk* temp_chunk = new smem_chunk;
                        temp_chunk->lti_id = (*triple_ptr_iter)->value->id->smem_lti;
                        temp_chunk->lti_letter = (*triple_ptr_iter)->value->id->name_letter;
                        temp_chunk->lti_number = (*triple_ptr_iter)->value->id->name_number;
                        temp_chunk->soar_id = (*triple_ptr_iter)->value;
                        temp_val->val_lti.val_value = temp_chunk;
                    }
                    else //If the value is not an identifier, then it is a "constant".
                    {
                        smem_chunk_value_constant temp_const;
                        smem_chunk_value_lti temp_lti;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_const_t;
                        temp_val->val_const.val_type = value_const_t;
                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
                    }
                    (*temp_slot).push_back(temp_val);
                }
                else //If the attribute is not in the map and we need to make a slot.
                {
                    temp_slot = new smem_slot;
                    smem_chunk_value* temp_val = new smem_chunk_value;
                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                    {
                        //If the chunk was retrieved and it is an identifier it is lti.
                        smem_chunk_value_lti temp_lti;
                        smem_chunk_value_constant temp_const;
                        temp_val->val_const = temp_const;
                        temp_val->val_const.val_type = value_lti_t;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_lti_t;
                        smem_chunk* temp_chunk = new smem_chunk;
                        temp_chunk->lti_id = (*triple_ptr_iter)->value->id->smem_lti;
                        temp_chunk->lti_letter = (*triple_ptr_iter)->value->id->name_letter;
                        temp_chunk->lti_number = (*triple_ptr_iter)->value->id->name_number;
                        temp_chunk->soar_id = (*triple_ptr_iter)->value;
                        temp_val->val_lti.val_value = temp_chunk;
                    }
                    else //If the value is nt an identifier, then it is a "constant".
                    {
                        smem_chunk_value_constant temp_const;
                        smem_chunk_value_lti temp_lti;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_const_t;
                        temp_val->val_const.val_type = value_const_t;
                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
                    }
                    (*temp_slot).push_back(temp_val);
                    children[(*triple_ptr_iter)->attr] = temp_slot;
                }
            }

            //Now we process attributes one at a time.
            while (thisAgent->lexeme.type == UP_ARROW_LEXEME && (good_command || force))
            {
                get_lexeme(thisAgent);// Consume the up arrow.

                Symbol* attribute = NIL;

                if (thisAgent->lexeme.type == STR_CONSTANT_LEXEME)
                {
                    attribute = find_str_constant(thisAgent, static_cast<const char*>(thisAgent->lexeme.string));
                }
                else if (thisAgent->lexeme.type == INT_CONSTANT_LEXEME)
                {
                    attribute = find_int_constant(thisAgent, thisAgent->lexeme.int_val);
                }
                else if (thisAgent->lexeme.type == FLOAT_CONSTANT_LEXEME)
                {
                    attribute = find_float_constant(thisAgent, thisAgent->lexeme.float_val);
                }

                if (attribute == NIL)
                {
                    good_command = false;
                    (*err_msg)->append("Error: Attribute was not found.\n");
                }
                else
                {
                    get_lexeme(thisAgent);//Consume the attribute.
                    good_command = true;
                }

                if (good_command && (thisAgent->lexeme.type != UP_ARROW_LEXEME && thisAgent->lexeme.type != R_PAREN_LEXEME)) //If there are values.
                {
                    Symbol* value;
                    do //Add value by type
                    {
                        value = NIL;
                        if (thisAgent->lexeme.type == STR_CONSTANT_LEXEME)
                        {
                            value = find_str_constant(thisAgent, static_cast<const char*>(thisAgent->lexeme.string));
                            get_lexeme(thisAgent);
                        }
                        else if (thisAgent->lexeme.type == INT_CONSTANT_LEXEME)
                        {
                            value = find_int_constant(thisAgent, thisAgent->lexeme.int_val);
                            get_lexeme(thisAgent);
                        }
                        else if (thisAgent->lexeme.type == FLOAT_CONSTANT_LEXEME)
                        {
                            value = find_float_constant(thisAgent, thisAgent->lexeme.float_val);
                            get_lexeme(thisAgent);
                        }
                        else if (thisAgent->lexeme.type == AT_LEXEME)
                        {
                            get_lexeme(thisAgent);
                            if (thisAgent->lexeme.type == IDENTIFIER_LEXEME)
                            {
                                value = find_identifier(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
                                get_lexeme(thisAgent);
                            }
                            else
                            {
                                (*err_msg)->append("Error: '@' should be followed by an identifier.\n");
                                good_command = false;
                                break;
                            }
                        }
                        else
                        {
                            good_command = (thisAgent->lexeme.type == R_PAREN_LEXEME || thisAgent->lexeme.type == UP_ARROW_LEXEME);
                            if (!good_command)
                            {
                                (*err_msg)->append("Error: Expected ')' or '^'.\n... The value was likely not found.\n");
                            }
                        }

                        if (value != NIL && good_command) //Value might be nil, but that can be just fine.
                        {
                            //Given a value for this attribute, we have a symbol triple to remove.
                            smem_slot::iterator values;
                            for (values = (children.find(attribute))->second->begin(); values != (children.find(attribute))->second->end(); values++)
                            {
                                if (value->symbol_type == IDENTIFIER_SYMBOL_TYPE && (*values)->val_lti.val_type == value_lti_t)
                                {
                                    if ((*values)->val_lti.val_value->soar_id == value)
                                    {
                                        delete(*values)->val_lti.val_value;
                                        delete *values;
                                        (*(children.find(attribute))).second->erase(values);
                                        break;
                                    }
                                }
                                else if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE && (*values)->val_const.val_type == value_const_t)
                                {
                                    if ((*values)->val_const.val_value == value)
                                    {
                                        delete *values;
                                        (*(children.find(attribute))).second->erase(values);
                                        break;
                                    }
                                }
                            }
                            if (values == (children.find(attribute))->second->end())
                            {
                                (*err_msg)->append("Error: Value does not exist on attribute.\n");
                            }
                        }
                        else
                        {
                            if ((good_command && !force) && (thisAgent->lexeme.type != R_PAREN_LEXEME && thisAgent->lexeme.type != UP_ARROW_LEXEME))
                            {
                                (*err_msg)->append("Error: Attribute contained a value that could not be found.\n");
                                break;
                            }
                        }
                    }
                    while (good_command && (value != NIL || !(thisAgent->lexeme.type == R_PAREN_LEXEME || thisAgent->lexeme.type == UP_ARROW_LEXEME)));
                }
                else if (good_command && children.find(attribute) != children.end()) //If we didn't have any values, then we just get rid of everything on the attribute.
                {
                    smem_slot* result = (children.find(attribute))->second;
                    smem_slot::iterator values, end = result->end();
                    for (values = (children.find(attribute))->second->begin(); values != end; values++)
                    {
                        delete *values;
                    }
                    children.erase(attribute);
                }
                if (force)
                {
                    while ((thisAgent->lexeme.type != EOF_LEXEME && thisAgent->lexeme.type != UP_ARROW_LEXEME) && thisAgent->lexeme.type != R_PAREN_LEXEME) //Loop until the lexeme is EOF, another ^, or ")".
                    {
                        get_lexeme(thisAgent);
                    }
                }
            }
        }
        if (good_command && thisAgent->lexeme.type == R_PAREN_LEXEME)
        {
            smem_store_chunk(thisAgent, lti_id, &(children), true, NULL, false);
        }
        else if (good_command)
        {
            (*err_msg)->append("Error: Expected a ')'.\n");
        }

        //Clean up.
        smem_slot_map::iterator attributes, end = children.end();
        for (attributes = children.begin(); attributes != end; attributes++)
        {
            smem_slot* result = (children.find(attributes->first))->second;
            smem_slot::iterator values, end = result->end();
            for (values = result->begin(); values != end; values++)
            {
                if ((*values)->val_lti.val_type == value_lti_t)
                {
                    delete(*values)->val_lti.val_value;
                }
                delete *values;
            }
            delete attributes->second;
        }

        soar_module::symbol_triple_list::iterator triple_iterator, end2 = retrieval_wmes.end();
        for (triple_iterator = retrieval_wmes.begin(); triple_iterator != end2; triple_iterator++)
        {
            symbol_remove_ref(thisAgent, (*triple_iterator)->id);
            symbol_remove_ref(thisAgent, (*triple_iterator)->attr);
            symbol_remove_ref(thisAgent, (*triple_iterator)->value);
            delete *triple_iterator;
        }
        symbol_remove_ref(thisAgent, lti);
    }
    return good_command;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Implementation (smem::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_respond_to_cmd(agent* thisAgent, bool store_only)
{

    smem_attach(thisAgent);

    // start at the bottom and work our way up
    // (could go in the opposite direction as well)
    Symbol* state = thisAgent->bottom_goal;

    smem_wme_list* wmes;
    smem_wme_list* cmds;
    smem_wme_list::iterator w_p;

    soar_module::symbol_triple_list meta_wmes;
    soar_module::symbol_triple_list retrieval_wmes;
    soar_module::wme_set cue_wmes;

    Symbol* query;
    Symbol* negquery;
    Symbol* retrieve;
    Symbol* math;
    uint64_t depth;
    smem_sym_list prohibit;
    smem_sym_list store;

    enum path_type { blank_slate, cmd_bad, cmd_retrieve, cmd_query, cmd_store } path;

    unsigned int time_slot = ((store_only) ? (1) : (0));
    uint64_t wme_count;
    bool new_cue;

    tc_number tc;

    Symbol* parent_sym;
    std::queue<Symbol*> syms;

    int parent_level;
    std::queue<int> levels;

    bool do_wm_phase = false;
    bool mirroring_on = (thisAgent->smem_params->mirroring->get_value() == on);

    //

    while (state != NULL)
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->smem_timers->api->start();
        ////////////////////////////////////////////////////////////////////////////

        // make sure this state has had some sort of change to the cmd
        // NOTE: we only care one-level deep!
        new_cue = false;
        wme_count = 0;
        cmds = NIL;
        {
            tc = get_new_tc_number(thisAgent);

            // initialize BFS at command
            syms.push(state->id->smem_cmd_header);
            levels.push(0);

            while (!syms.empty())
            {
                // get state
                parent_sym = syms.front();
                syms.pop();

                parent_level = levels.front();
                levels.pop();

                // get children of the current identifier
                wmes = smem_get_direct_augs_of_id(parent_sym, tc);
                {
                    for (w_p = wmes->begin(); w_p != wmes->end(); w_p++)
                    {
                        if (((store_only) && ((parent_level != 0) || ((*w_p)->attr == thisAgent->smem_sym_store))) ||
                                ((!store_only) && ((parent_level != 0) || ((*w_p)->attr != thisAgent->smem_sym_store))))
                        {
                            wme_count++;

                            if ((*w_p)->timetag > state->id->smem_info->last_cmd_time[ time_slot ])
                            {
                                new_cue = true;
                                state->id->smem_info->last_cmd_time[ time_slot ] = (*w_p)->timetag;
                            }

                            if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                    (parent_level == 0) &&
                                    (((*w_p)->attr == thisAgent->smem_sym_query) || ((*w_p)->attr == thisAgent->smem_sym_store)))
                            {
                                syms.push((*w_p)->value);
                                levels.push(parent_level + 1);
                            }
                        }
                    }

                    // free space from aug list
                    if (cmds == NIL)
                    {
                        cmds = wmes;
                    }
                    else
                    {
                        delete wmes;
                    }
                }
            }

            // see if any WMEs were removed
            if (state->id->smem_info->last_cmd_count[ time_slot ] != wme_count)
            {
                new_cue = true;
                state->id->smem_info->last_cmd_count[ time_slot ] = wme_count;
            }


            if (new_cue)
            {
                // clear old results
                smem_clear_result(thisAgent, state);

                do_wm_phase = true;
            }
        }

        // a command is issued if the cue is new
        // and there is something on the cue
        if (new_cue && wme_count)
        {
            cue_wmes.clear();
            meta_wmes.clear();
            retrieval_wmes.clear();

            // initialize command vars
            retrieve = NIL;
            query = NIL;
            negquery = NIL;
            math = NIL;
            store.clear();
            prohibit.clear();
            path = blank_slate;
            depth = 1;

            // process top-level symbols
            for (w_p = cmds->begin(); w_p != cmds->end(); w_p++)
            {
                cue_wmes.insert((*w_p));

                if (path != cmd_bad)
                {
                    // collect information about known commands
                    if ((*w_p)->attr == thisAgent->smem_sym_retrieve)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                (path == blank_slate))
                        {
                            retrieve = (*w_p)->value;
                            path = cmd_retrieve;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_query)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
                                (query == NIL))

                        {
                            query = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_negquery)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
                                (negquery == NIL))

                        {
                            negquery = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_prohibit)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
                                ((*w_p)->value->id->smem_lti != NIL))
                        {
                            prohibit.push_back((*w_p)->value);
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_math_query)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_query)) &&
                                (math == NIL))
                        {
                            math = (*w_p)->value;
                            path = cmd_query;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_store)
                    {
                        if (((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                                ((path == blank_slate) || (path == cmd_store)))
                        {
                            store.push_back((*w_p)->value);
                            path = cmd_store;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else if ((*w_p)->attr == thisAgent->smem_sym_depth)
                    {
                        if ((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                        {
                            depth = ((*w_p)->value->ic->value > 0) ? (*w_p)->value->ic->value : 1;
                        }
                        else
                        {
                            path = cmd_bad;
                        }
                    }
                    else
                    {
                        path = cmd_bad;
                    }
                }
            }

            // if on path 3 must have query/neg-query
            if ((path == cmd_query) && (query == NULL))
            {
                path = cmd_bad;
            }

            // must be on a path
            if (path == blank_slate)
            {
                path = cmd_bad;
            }

            ////////////////////////////////////////////////////////////////////////////
            thisAgent->smem_timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////

            // process command
            if (path != cmd_bad)
            {
                // performing any command requires an initialized database
                smem_attach(thisAgent);

                // retrieve
                if (path == cmd_retrieve)
                {
                    if (retrieve->id->smem_lti == NIL)
                    {
                        // retrieve is not pointing to an lti!
                        smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_failure, retrieve);
                    }
                    else
                    {
                        // status: success
                        smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_success, retrieve);

                        // install memory directly onto the retrieve identifier
                        smem_install_memory(thisAgent, state, retrieve->id->smem_lti, retrieve, true, meta_wmes, retrieval_wmes, wm_install, depth);

                        // add one to the expansions stat
                        thisAgent->smem_stats->expansions->set_value(thisAgent->smem_stats->expansions->get_value() + 1);
                    }
                }
                // query
                else if (path == cmd_query)
                {
                    smem_lti_set prohibit_lti;
                    smem_sym_list::iterator sym_p;

                    for (sym_p = prohibit.begin(); sym_p != prohibit.end(); sym_p++)
                    {
                        prohibit_lti.insert((*sym_p)->id->smem_lti);
                    }

                    smem_process_query(thisAgent, state, query, negquery, math, &(prohibit_lti), cue_wmes, meta_wmes, retrieval_wmes);

                    // add one to the cbr stat
                    thisAgent->smem_stats->cbr->set_value(thisAgent->smem_stats->cbr->get_value() + 1);
                }
                else if (path == cmd_store)
                {
                    smem_sym_list::iterator sym_p;

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->smem_timers->storage->start();
                    ////////////////////////////////////////////////////////////////////////////

                    // start transaction (if not lazy)
                    if (thisAgent->smem_params->lazy_commit->get_value() == off)
                    {
                        thisAgent->smem_stmts->begin->execute(soar_module::op_reinit);
                    }

                    for (sym_p = store.begin(); sym_p != store.end(); sym_p++)
                    {
                        smem_soar_store(thisAgent, (*sym_p), ((mirroring_on) ? (store_recursive) : (store_level)));

                        // status: success
                        smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_success, (*sym_p));

                        // add one to the store stat
                        thisAgent->smem_stats->stores->set_value(thisAgent->smem_stats->stores->get_value() + 1);
                    }

                    // commit transaction (if not lazy)
                    if (thisAgent->smem_params->lazy_commit->get_value() == off)
                    {
                        thisAgent->smem_stmts->commit->execute(soar_module::op_reinit);
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    thisAgent->smem_timers->storage->stop();
                    ////////////////////////////////////////////////////////////////////////////
                }
            }
            else
            {
                smem_buffer_add_wme(thisAgent, meta_wmes, state->id->smem_result_header, thisAgent->smem_sym_bad_cmd, state->id->smem_cmd_header);
            }

            if (!meta_wmes.empty() || !retrieval_wmes.empty())
            {
                // process preference assertion en masse
                smem_process_buffered_wmes(thisAgent, state, cue_wmes, meta_wmes, retrieval_wmes);

                // clear cache
                {
                    soar_module::symbol_triple_list::iterator mw_it;

                    for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
                    {
                        symbol_remove_ref(thisAgent, (*mw_it)->id);
                        symbol_remove_ref(thisAgent, (*mw_it)->attr);
                        symbol_remove_ref(thisAgent, (*mw_it)->value);

                        delete(*mw_it);
                    }
                    retrieval_wmes.clear();

                    for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
                    {
                        symbol_remove_ref(thisAgent, (*mw_it)->id);
                        symbol_remove_ref(thisAgent, (*mw_it)->attr);
                        symbol_remove_ref(thisAgent, (*mw_it)->value);

                        delete(*mw_it);
                    }
                    meta_wmes.clear();
                }

                // process wm changes on this state
                do_wm_phase = true;
            }

            // clear cue wmes
            cue_wmes.clear();
        }
        else
        {
            ////////////////////////////////////////////////////////////////////////////
            thisAgent->smem_timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////
        }

        // free space from aug list
        delete cmds;

        state = state->id->higher_goal;
    }

    if (store_only && mirroring_on && (!thisAgent->smem_changed_ids->empty()))
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->smem_timers->storage->start();
        ////////////////////////////////////////////////////////////////////////////

        // start transaction (if not lazy)
        if (thisAgent->smem_params->lazy_commit->get_value() == off)
        {
            thisAgent->smem_stmts->begin->execute(soar_module::op_reinit);
        }

        for (smem_pooled_symbol_set::iterator it = thisAgent->smem_changed_ids->begin(); it != thisAgent->smem_changed_ids->end(); it++)
        {
            // require that the lti has at least one augmentation
            if ((*it)->id->slots)
            {
                smem_soar_store(thisAgent, (*it), store_recursive);

                // add one to the mirrors stat
                thisAgent->smem_stats->mirrors->set_value(thisAgent->smem_stats->mirrors->get_value() + 1);
            }

            symbol_remove_ref(thisAgent, (*it));
        }

        // commit transaction (if not lazy)
        if (thisAgent->smem_params->lazy_commit->get_value() == off)
        {
            thisAgent->smem_stmts->commit->execute(soar_module::op_reinit);
        }

        // clear symbol set
        thisAgent->smem_changed_ids->clear();

        ////////////////////////////////////////////////////////////////////////////
        thisAgent->smem_timers->storage->stop();
        ////////////////////////////////////////////////////////////////////////////
    }

    if (do_wm_phase)
    {
        thisAgent->smem_ignore_changes = true;

        do_working_memory_phase(thisAgent);

        thisAgent->smem_ignore_changes = false;
    }
}

void smem_go(agent* thisAgent, bool store_only)
{
    thisAgent->smem_timers->total->start();

#ifndef SMEM_EXPERIMENT

    smem_respond_to_cmd(thisAgent, store_only);

#else // SMEM_EXPERIMENT

#endif // SMEM_EXPERIMENT

    thisAgent->smem_timers->total->stop();
}

bool smem_backup_db(agent* thisAgent, const char* file_name, std::string* err)
{
    bool return_val = false;

    if (thisAgent->smem_db->get_status() == soar_module::connected)
    {
        _smem_close_vars(thisAgent);

        if (thisAgent->smem_params->lazy_commit->get_value() == on)
        {
            thisAgent->smem_stmts->commit->execute(soar_module::op_reinit);
        }

        return_val = thisAgent->smem_db->backup(file_name, err);

        if (thisAgent->smem_params->lazy_commit->get_value() == on)
        {
            thisAgent->smem_stmts->begin->execute(soar_module::op_reinit);
        }
    }
    else
    {
        err->assign("Semantic database is not currently connected.");
    }

    return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Visualization (smem::viz)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void smem_visualize_store(agent* thisAgent, std::string* return_val)
{
    // header
    return_val->append("digraph smem {");
    return_val->append("\n");

    // LTIs
    return_val->append("node [ shape = doublecircle ];");
    return_val->append("\n");

    std::map< smem_lti_id, std::string > lti_names;
    std::map< smem_lti_id, std::string >::iterator n_p;
    {
        soar_module::sqlite_statement* q;

        smem_lti_id lti_id;
        char lti_letter;
        uint64_t lti_number;

        std::string* lti_name;
        std::string temp_str;
        int64_t temp_int;
        double temp_double;

        // id, soar_letter, number
        q = thisAgent->smem_stmts->vis_lti;
        while (q->execute() == soar_module::row)
        {
            lti_id = q->column_int(0);
            lti_letter = static_cast<char>(q->column_int(1));
            lti_number = static_cast<uint64_t>(q->column_int(2));

            lti_name = & lti_names[ lti_id ];
            lti_name->push_back(lti_letter);

            to_string(lti_number, temp_str);
            lti_name->append(temp_str);

            return_val->append((*lti_name));
            return_val->append(" [ label=\"");
            return_val->append((*lti_name));
            return_val->append("\\n[");

            temp_double = q->column_double(3);
            to_string(temp_double, temp_str, 3, true);
            if (temp_double >= 0)
            {
                return_val->append("+");
            }
            return_val->append(temp_str);

            return_val->append("]\"");
            return_val->append(" ];");
            return_val->append("\n");
        }
        q->reinitialize();

        if (!lti_names.empty())
        {
            // terminal nodes first
            {
                std::map< smem_lti_id, std::list<std::string> > lti_terminals;
                std::map< smem_lti_id, std::list<std::string> >::iterator t_p;
                std::list<std::string>::iterator a_p;

                std::list<std::string>* my_terminals;
                std::list<std::string>::size_type terminal_num;

                return_val->append("\n");

                // proceed to terminal nodes
                return_val->append("node [ shape = plaintext ];");
                return_val->append("\n");

                // lti_id, attr_type, attr_hash, val_type, val_hash
                q = thisAgent->smem_stmts->vis_value_const;
                while (q->execute() == soar_module::row)
                {
                    lti_id = q->column_int(0);
                    my_terminals = & lti_terminals[ lti_id ];
                    lti_name = & lti_names[ lti_id ];

                    // parent prefix
                    return_val->append((*lti_name));
                    return_val->append("_");

                    // terminal count
                    terminal_num = my_terminals->size();
                    to_string(terminal_num, temp_str);
                    return_val->append(temp_str);

                    // prepare for value
                    return_val->append(" [ label = \"");

                    // output value
                    {
                        switch (q->column_int(3))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                smem_reverse_hash_str(thisAgent, q->column_int(4), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = smem_reverse_hash_int(thisAgent, q->column_int(4));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = smem_reverse_hash_float(thisAgent, q->column_int(4));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        return_val->append(temp_str);
                    }

                    // store terminal (attribute for edge label)
                    {
                        switch (q->column_int(1))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                smem_reverse_hash_str(thisAgent, q->column_int(2), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = smem_reverse_hash_int(thisAgent, q->column_int(2));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = smem_reverse_hash_float(thisAgent, q->column_int(2));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        my_terminals->push_back(temp_str);
                    }

                    // footer
                    return_val->append("\" ];");
                    return_val->append("\n");
                }
                q->reinitialize();

                // output edges
                {
                    unsigned int terminal_counter;

                    for (n_p = lti_names.begin(); n_p != lti_names.end(); n_p++)
                    {
                        t_p = lti_terminals.find(n_p->first);

                        if (t_p != lti_terminals.end())
                        {
                            terminal_counter = 0;

                            for (a_p = t_p->second.begin(); a_p != t_p->second.end(); a_p++)
                            {
                                return_val->append(n_p->second);
                                return_val ->append(" -> ");
                                return_val->append(n_p->second);
                                return_val->append("_");

                                to_string(terminal_counter, temp_str);
                                return_val->append(temp_str);
                                return_val->append(" [ label=\"");

                                return_val->append((*a_p));

                                return_val->append("\" ];");
                                return_val->append("\n");

                                terminal_counter++;
                            }
                        }
                    }
                }
            }

            // then links to other LTIs
            {
                // lti_id, attr_type, attr_hash, value_lti_id
                q = thisAgent->smem_stmts->vis_value_lti;
                while (q->execute() == soar_module::row)
                {
                    // source
                    lti_id = q->column_int(0);
                    lti_name = & lti_names[ lti_id ];
                    return_val->append((*lti_name));
                    return_val->append(" -> ");

                    // destination
                    lti_id = q->column_int(3);
                    lti_name = & lti_names[ lti_id ];
                    return_val->append((*lti_name));
                    return_val->append(" [ label =\"");

                    // output attribute
                    {
                        switch (q->column_int(1))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                smem_reverse_hash_str(thisAgent, q->column_int(2), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = smem_reverse_hash_int(thisAgent, q->column_int(2));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = smem_reverse_hash_float(thisAgent, q->column_int(2));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        return_val->append(temp_str);
                    }

                    // footer
                    return_val->append("\" ];");
                    return_val->append("\n");
                }
                q->reinitialize();
            }
        }
    }

    // footer
    return_val->append("}");
    return_val->append("\n");
}

void smem_visualize_lti(agent* thisAgent, smem_lti_id lti_id, unsigned int depth, std::string* return_val)
{
    // buffer
    std::string return_val2;

    soar_module::sqlite_statement* expand_q = thisAgent->smem_stmts->web_expand;

    uint64_t child_counter;

    std::string temp_str;
    std::string temp_str2;
    int64_t temp_int;
    double temp_double;

    std::queue<smem_vis_lti*> bfs;
    smem_vis_lti* new_lti;
    smem_vis_lti* parent_lti;

    std::map< smem_lti_id, smem_vis_lti* > close_list;
    std::map< smem_lti_id, smem_vis_lti* >::iterator cl_p;

    // header
    return_val->append("digraph smem_lti {");
    return_val->append("\n");

    // root
    {
        new_lti = new smem_vis_lti;
        new_lti->lti_id = lti_id;
        new_lti->level = 0;

        // fake former linkage
        {
            soar_module::sqlite_statement* lti_q = thisAgent->smem_stmts->lti_letter_num;

            // get just this lti
            lti_q->bind_int(1, lti_id);
            lti_q->execute();

            // soar_letter
            new_lti->lti_name.push_back(static_cast<char>(lti_q->column_int(0)));

            // number
            temp_int = lti_q->column_int(1);
            to_string(temp_int, temp_str);
            new_lti->lti_name.append(temp_str);

            // done with lookup
            lti_q->reinitialize();
        }

        bfs.push(new_lti);
        close_list.insert(std::make_pair(lti_id, new_lti));

        new_lti = NULL;
    }

    // optionally depth-limited breadth-first-search of children
    while (!bfs.empty())
    {
        parent_lti = bfs.front();
        bfs.pop();

        child_counter = 0;

        // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
        expand_q->bind_int(1, parent_lti->lti_id);
        while (expand_q->execute() == soar_module::row)
        {
            // identifier vs. constant
            if (expand_q->column_int(6) != SMEM_AUGMENTATIONS_NULL)
            {
                new_lti = new smem_vis_lti;
                new_lti->lti_id = expand_q->column_int(6);
                new_lti->level = (parent_lti->level + 1);

                // add node
                {
                    // soar_letter
                    new_lti->lti_name.push_back(static_cast<char>(expand_q->column_int(4)));

                    // number
                    temp_int = expand_q->column_int(5);
                    to_string(temp_int, temp_str);
                    new_lti->lti_name.append(temp_str);
                }


                // add linkage
                {
                    // get attribute
                    switch (expand_q->column_int(0))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            smem_reverse_hash_str(thisAgent, expand_q->column_int(1), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = smem_reverse_hash_int(thisAgent, expand_q->column_int(1));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = smem_reverse_hash_float(thisAgent, expand_q->column_int(1));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output linkage
                    return_val2.append(parent_lti->lti_name);
                    return_val2.append(" -> ");
                    return_val2.append(new_lti->lti_name);
                    return_val2.append(" [ label = \"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                // prevent looping
                {
                    cl_p = close_list.find(new_lti->lti_id);
                    if (cl_p == close_list.end())
                    {
                        close_list.insert(std::make_pair(new_lti->lti_id, new_lti));

                        if ((depth == 0) || (new_lti->level < depth))
                        {
                            bfs.push(new_lti);
                        }
                    }
                    else
                    {
                        delete new_lti;
                    }
                }

                new_lti = NULL;
            }
            else
            {
                // add value node
                {
                    // get node name
                    {
                        temp_str2.assign(parent_lti->lti_name);
                        temp_str2.append("_");

                        to_string(child_counter, temp_str);
                        temp_str2.append(temp_str);
                    }

                    // get value
                    switch (expand_q->column_int(2))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            smem_reverse_hash_str(thisAgent, expand_q->column_int(3), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = smem_reverse_hash_int(thisAgent, expand_q->column_int(3));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = smem_reverse_hash_float(thisAgent, expand_q->column_int(3));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output node
                    return_val2.append("node [ shape = plaintext ];");
                    return_val2.append("\n");
                    return_val2.append(temp_str2);
                    return_val2.append(" [ label=\"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                // add linkage
                {
                    // get attribute
                    switch (expand_q->column_int(0))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            smem_reverse_hash_str(thisAgent, expand_q->column_int(1), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = smem_reverse_hash_int(thisAgent, expand_q->column_int(1));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = smem_reverse_hash_float(thisAgent, expand_q->column_int(1));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output linkage
                    return_val2.append(parent_lti->lti_name);
                    return_val2.append(" -> ");
                    return_val2.append(temp_str2);
                    return_val2.append(" [ label = \"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                child_counter++;
            }
        }
        expand_q->reinitialize();
    }

    // footer
    return_val2.append("}");
    return_val2.append("\n");

    // handle lti nodes at once
    {
        soar_module::sqlite_statement* act_q = thisAgent->smem_stmts->vis_lti_act;

        return_val->append("node [ shape = doublecircle ];");
        return_val->append("\n");

        for (cl_p = close_list.begin(); cl_p != close_list.end(); cl_p++)
        {
            return_val->append(cl_p->second->lti_name);
            return_val->append(" [ label=\"");
            return_val->append(cl_p->second->lti_name);
            return_val->append("\\n[");

            act_q->bind_int(1, cl_p->first);
            if (act_q->execute() == soar_module::row)
            {
                temp_double = act_q->column_double(0);
                to_string(temp_double, temp_str, 3, true);
                if (temp_double >= 0)
                {
                    return_val->append("+");
                }
                return_val->append(temp_str);
            }
            act_q->reinitialize();

            return_val->append("]\"");
            return_val->append(" ];");
            return_val->append("\n");

            delete cl_p->second;
        }
    }

    // transfer buffer after nodes
    return_val->append(return_val2);
}

inline std::set< smem_lti_id > _smem_print_lti(agent* thisAgent, smem_lti_id lti_id, char lti_letter, uint64_t lti_number, double lti_act, std::string* return_val, std::list<uint64_t>* history = NIL)
{
    std::set< smem_lti_id > next;

    std::string temp_str, temp_str2, temp_str3;
    int64_t temp_int;
    double temp_double;

    std::map< std::string, std::list< std::string > > augmentations;
    std::map< std::string, std::list< std::string > >::iterator lti_slot;
    std::list< std::string >::iterator slot_val;

    smem_attach(thisAgent);

    soar_module::sqlite_statement* expand_q = thisAgent->smem_stmts->web_expand;

    return_val->append("(@");
    return_val->push_back(lti_letter);
    to_string(lti_number, temp_str);
    return_val->append(temp_str);

    bool possible_id, possible_ic, possible_fc, possible_sc, possible_var, is_rereadable;

    // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
    expand_q->bind_int(1, lti_id);
    while (expand_q->execute() == soar_module::row)
    {
        // get attribute
        switch (expand_q->column_int(0))
        {
            case STR_CONSTANT_SYMBOL_TYPE:
            {
                smem_reverse_hash_str(thisAgent, expand_q->column_int(1), temp_str);

                if (count(temp_str.begin(), temp_str.end(), ' ') > 0)
                {
                    temp_str.insert(0, "|");
                    temp_str += '|';
                break;
                }

                determine_possible_symbol_types_for_string(temp_str.c_str(),
                        strlen(temp_str.c_str()),
                        &possible_id,
                        &possible_var,
                        &possible_sc,
                        &possible_ic,
                        &possible_fc,
                        &is_rereadable);

                bool has_angle_bracket = temp_str[0] == '<' || temp_str[temp_str.length() - 1] == '>';

                if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                        (!is_rereadable) ||
                        has_angle_bracket)
                {
                    /* BUGBUG if in context where id's could occur, should check
                     possible_id flag here also */
                    temp_str.insert(0, "|");
                    temp_str += '|';
                }
                break;
            }
            case INT_CONSTANT_SYMBOL_TYPE:
                temp_int = smem_reverse_hash_int(thisAgent, expand_q->column_int(1));
                to_string(temp_int, temp_str);
                break;

            case FLOAT_CONSTANT_SYMBOL_TYPE:
                temp_double = smem_reverse_hash_float(thisAgent, expand_q->column_int(1));
                to_string(temp_double, temp_str);
                break;

            default:
                temp_str.clear();
                break;
        }

        // identifier vs. constant
        if (expand_q->column_int(6) != SMEM_AUGMENTATIONS_NULL)
        {
            temp_str2.clear();
            temp_str2.push_back('@');

            // soar_letter
            temp_str2.push_back(static_cast<char>(expand_q->column_int(4)));

            // number
            temp_int = expand_q->column_int(5);
            to_string(temp_int, temp_str3);
            temp_str2.append(temp_str3);

            // add to next
            next.insert(static_cast< smem_lti_id >(expand_q->column_int(6)));
        }
        else
        {
            switch (expand_q->column_int(2))
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                {
                    smem_reverse_hash_str(thisAgent, expand_q->column_int(3), temp_str2);

                    if (count(temp_str2.begin(), temp_str2.end(), ' ') > 0)
                    {
                        temp_str2.insert(0, "|");
                        temp_str2 += '|';
                    break;
                    }

                    determine_possible_symbol_types_for_string(temp_str2.c_str(),
                            temp_str2.length(),
                            &possible_id,
                            &possible_var,
                            &possible_sc,
                            &possible_ic,
                            &possible_fc,
                            &is_rereadable);

                    bool has_angle_bracket = temp_str2[0] == '<' || temp_str2[temp_str2.length() - 1] == '>';

                    if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                            (!is_rereadable) ||
                            has_angle_bracket)
                    {
                        /* BUGBUG if in context where id's could occur, should check
                         possible_id flag here also */
                        temp_str2.insert(0, "|");
                        temp_str2 += '|';
                    }
                    break;
                }
                case INT_CONSTANT_SYMBOL_TYPE:
                    temp_int = smem_reverse_hash_int(thisAgent, expand_q->column_int(3));
                    to_string(temp_int, temp_str2);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    temp_double = smem_reverse_hash_float(thisAgent, expand_q->column_int(3));
                    to_string(temp_double, temp_str2);
                    break;

                default:
                    temp_str2.clear();
                    break;
            }
        }

        augmentations[ temp_str ].push_back(temp_str2);
    }
    expand_q->reinitialize();

    // output augmentations nicely
    {
        for (lti_slot = augmentations.begin(); lti_slot != augmentations.end(); lti_slot++)
        {
            return_val->append(" ^");
            return_val->append(lti_slot->first);

            for (slot_val = lti_slot->second.begin(); slot_val != lti_slot->second.end(); slot_val++)
            {
                return_val->append(" ");
                return_val->append((*slot_val));
            }
        }
    }
    augmentations.clear();

    return_val->append(" [");
    to_string(lti_act, temp_str, 3, true);
    if (lti_act >= 0)
    {
        return_val->append("+");
    }
    return_val->append(temp_str);
    return_val->append("]");
    return_val->append(")\n");

    if (history != NIL)
    {
        std::ostringstream temp_string;
        return_val->append("SMem Access Cycle History\n");
        return_val->append("[-");
        for (std::list<uint64_t>::iterator history_item = (*history).begin(); history_item != (*history).end(); ++history_item)
        {
            if (history_item != (*history).begin())
            {
                return_val->append(", -");
            }
            temp_string << ((int64_t)thisAgent->smem_max_cycle - (int64_t)*history_item);
            return_val->append(temp_string.str());
            temp_string.str("");
        }
        return_val->append("]\n");
    }

    return next;
}

void smem_print_store(agent* thisAgent, std::string* return_val)
{
    // id, soar_letter, number
    soar_module::sqlite_statement* q = thisAgent->smem_stmts->vis_lti;
    while (q->execute() == soar_module::row)
    {
        _smem_print_lti(thisAgent, q->column_int(0), static_cast<char>(q->column_int(1)), static_cast<uint64_t>(q->column_int(2)), q->column_double(3), return_val);
    }
    q->reinitialize();
}

void smem_print_lti(agent* thisAgent, smem_lti_id lti_id, unsigned int depth, std::string* return_val, bool history)
{
    std::set< smem_lti_id > visited;
    std::pair< std::set< smem_lti_id >::iterator, bool > visited_ins_result;

    std::queue< std::pair< smem_lti_id, unsigned int > > to_visit;
    std::pair< smem_lti_id, unsigned int > c;

    std::set< smem_lti_id > next;
    std::set< smem_lti_id >::iterator next_it;

    soar_module::sqlite_statement* lti_q = thisAgent->smem_stmts->lti_letter_num;
    soar_module::sqlite_statement* act_q = thisAgent->smem_stmts->vis_lti_act;
    soar_module::sqlite_statement* hist_q = thisAgent->smem_stmts->history_get;
    soar_module::sqlite_statement* lti_access_q = thisAgent->smem_stmts->lti_access_get;
    unsigned int i;


    // initialize queue/set
    to_visit.push(std::make_pair(lti_id, 1u));
    visited.insert(lti_id);

    while (!to_visit.empty())
    {
        c = to_visit.front();
        to_visit.pop();

        // output leading spaces ala depth
        for (i = 1; i < c.second; i++)
        {
            return_val->append("  ");
        }

        // get lti info
        {
            lti_q->bind_int(1, c.first);
            lti_q->execute();

            act_q->bind_int(1, c.first);
            act_q->execute();

            //Look up activation history.
            std::list<uint64_t> access_history;
            if (history)
            {
                lti_access_q->bind_int(1, c.first);
                lti_access_q->execute();
                uint64_t n = lti_access_q->column_int(0);
                lti_access_q->reinitialize();
                hist_q->bind_int(1, c.first);
                hist_q->execute();
                for (int i = 0; i < n && i < 10; ++i) //10 because of the length of the history record kept for smem.
                {
                    if (thisAgent->smem_stmts->history_get->column_int(i) != 0)
                    {
                        access_history.push_back(hist_q->column_int(i));
                    }
                }
                hist_q->reinitialize();
            }

            if (history && !access_history.empty())
            {
                next = _smem_print_lti(thisAgent, c.first, static_cast<char>(lti_q->column_int(0)), static_cast<uint64_t>(lti_q->column_int(1)), act_q->column_double(0), return_val, &(access_history));
            }
            else
            {
                next = _smem_print_lti(thisAgent, c.first, static_cast<char>(lti_q->column_int(0)), static_cast<uint64_t>(lti_q->column_int(1)), act_q->column_double(0), return_val);
            }

            // done with lookup
            lti_q->reinitialize();
            act_q->reinitialize();

            // consider further depth
            if (c.second < depth)
            {
                for (next_it = next.begin(); next_it != next.end(); next_it++)
                {
                    visited_ins_result = visited.insert((*next_it));
                    if (visited_ins_result.second)
                    {
                        to_visit.push(std::make_pair((*next_it), c.second + 1u));
                    }
                }
            }
        }
    }
}
