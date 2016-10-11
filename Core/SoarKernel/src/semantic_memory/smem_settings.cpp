/*
 * smem_param.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_settings.h"
#include "smem_stats.h"

#include "output_manager.h"

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

    /* Moved from init_agent */
    base_incremental_threshes->set_string("10");

}

//

/* This is a test of whether or not the SMEM database with no version number is the one
that smem_update_schema_one_to_two can convert.  It tests for the existence of a table name to determine if this is the old version. */
bool SMem_Manager::is_version_one_db()
{
    return DB.execAndGet("SELECT count(type) FROM sqlite_master WHERE type='table' AND name='smem7_signature'").getInt64() != 0;
}

smem_path_param::smem_path_param(const char* new_name, const char* new_value, soar_module::predicate<const char*>* new_val_pred, soar_module::predicate<const char*>* new_prot_pred, agent* new_agent): soar_module::string_param(new_name, new_value, new_val_pred, new_prot_pred), thisAgent(new_agent) {}

void smem_path_param::set_value(const char* new_value)
{
    /* The first time path is set, we check that the the database is the right version,
     * so you can warn someone before they try to use it that conversion will take some
     * time. That way, they can then switch to another before dedicating that time. */
    value->assign(new_value);

    const char* db_path;
    db_path = thisAgent->SMem->settings->path->get_value();
    bool attempt_connection_here = !thisAgent->SMem->connected();
    if (attempt_connection_here)
    {
        try
        {
            thisAgent->SMem->recreateDB(db_path);
        }
        catch (SQLite::Exception& e)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Semantic memory database error: %s\n", e.getErrorStr());
        }
    }

    // If the database is on file, make sure the database contents use the current schema
    // If it does not, switch to memory-based database

    if (strcmp(db_path, ":memory:")) // Only worry about database version if writing to disk
    {
        std::string schema_version;
        if (thisAgent->SMem->DB.containsData())
        {
            // Check if table exists already
            thisAgent->SMem->DB.exec("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
            if (thisAgent->SMem->is_version_one_db())
            {
                thisAgent->outputManager->printa(thisAgent, "...You have selected a database with an old version.\n"
                                                 "...If you proceed, the database will be converted to a\n"
                                                 "...new version when the database is initialized.\n"
                                                 "...Conversion can take a large amount of time with large databases.\n");
            }
        }
    }
}

//

template <typename T>
smem_db_predicate<T>::smem_db_predicate(agent* new_agent): soar_module::agent_predicate<T>(new_agent) {}

template <typename T>
bool smem_db_predicate<T>::operator()(T /*val*/)
{
    return (this->thisAgent->SMem->connected());
}


smem_stat_container::smem_stat_container(agent* new_agent): soar_module::stat_container(new_agent)
{
    db_lib_version = new smem_db_lib_version_stat(thisAgent, "db-lib-version", NULL, new soar_module::predicate< const char* >());
    add(db_lib_version);

    mem_usage = new smem_mem_usage_stat(thisAgent, "mem-usage", 0, new soar_module::predicate<int64_t>());
    add(mem_usage);

    mem_high = new smem_mem_high_stat(thisAgent, "mem-high", 0, new soar_module::predicate<int64_t>());
    add(mem_high);

    retrievals = new soar_module::integer_stat("retrieves", 0, new soar_module::f_predicate<int64_t>());
    add(retrievals);

    queries = new soar_module::integer_stat("queries", 0, new soar_module::f_predicate<int64_t>());
    add(queries);

    stores = new soar_module::integer_stat("stores", 0, new soar_module::f_predicate<int64_t>());
    add(stores);

    act_updates = new soar_module::integer_stat("act_updates", 0, new soar_module::f_predicate<int64_t>());
    add(act_updates);

    nodes = new soar_module::integer_stat("nodes", 0, new smem_db_predicate< int64_t >(thisAgent));
    add(nodes);

    edges = new soar_module::integer_stat("edges", 0, new smem_db_predicate< int64_t >(thisAgent));
    add(edges);
}

//

smem_db_lib_version_stat::smem_db_lib_version_stat(agent* new_agent, const char* new_name, const char* new_value, soar_module::predicate< const char* >* new_prot_pred): soar_module::primitive_stat< const char* >(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

const char* smem_db_lib_version_stat::get_value()
{
    return SQLite::getLibVersion();
}

smem_mem_usage_stat::smem_mem_usage_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t smem_mem_usage_stat::get_value()
{
    return thisAgent->SMem->DB.getMemoryUsage();
}

smem_mem_high_stat::smem_mem_high_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t smem_mem_high_stat::get_value()
{
    return thisAgent->SMem->DB.getMemoryHighwater();
}

bool SMem_Manager::enabled()
{
    return (settings->learning->get_value() == on);
}

bool SMem_Manager::connected()
{
    return true;
}
