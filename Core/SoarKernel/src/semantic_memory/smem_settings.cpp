/*
 * smem_param.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "smem_settings.h"

#include "output_manager.h"
#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_stats.h"
#include "soar_module.h"

smem_param_container::smem_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // learning
    learning = new soar_module::boolean_param("learning", off, new soar_module::f_predicate<boolean>());
    add(learning);

    // spreading
    spreading = new soar_module::boolean_param("spreading", off, new soar_module::f_predicate<boolean>());
    add(spreading);

    // database
    database = new soar_module::constant_param<db_choices>("database", memory, new soar_module::f_predicate<db_choices>());
    database->add_mapping(memory, "memory");
    database->add_mapping(file, "file");
    add(database);

    // append database or dump data on init
    append_db = new soar_module::boolean_param("append", on, new soar_module::f_predicate<boolean>());
    add(append_db);

    synchronous_db = new soar_module::boolean_param("synchronous", on, new soar_module::f_predicate<boolean>());
    add(synchronous_db);

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

    // activate on add
    activate_on_add = new soar_module::boolean_param("activate-on-add", off, new soar_module::f_predicate<boolean>());
    add(activate_on_add);

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


    base_unused_age_offset = new soar_module::integer_param("base-unused-age-offset", 0, new soar_module::predicate<int64_t>(), new smem_db_predicate<int64_t>(thisAgent));
    add(base_unused_age_offset);

    // incremental update thresholds
    base_incremental_threshes = new soar_module::int_set_param("base-incremental-threshes", new soar_module::f_predicate< int64_t >());
    add(base_incremental_threshes);

    // initial variable id
    initial_variable_id = new soar_module::unsigned_integer_param("initial-variable-id", 1, new soar_module::predicate<uint64_t>(), new smem_db_predicate<uint64_t>(thisAgent));
    add(initial_variable_id);

    /* Moved from init_agent */
    base_incremental_threshes->set_string("10");

    /* The following are spreading activation settings */

    // spreading baseline - This sets the value at which we consider spread too small.
    spreading_baseline = new soar_module::decimal_param("spreading-baseline", 0.0001, new  soar_module::gt_predicate<double>(0, false), new soar_module::f_predicate<double>());
    add(spreading_baseline);

    // spreading continue probability - determines the decay over edge distances when edge weights are not present.
    spreading_continue_probability = new soar_module::decimal_param("spreading-continue-probability", 0.9, new soar_module::gt_predicate<double>(0, false), new soar_module::f_predicate<double>());
    add(spreading_continue_probability);

    // spreading limit - a limit to how many nodes a single source can impact
    spreading_limit = new soar_module::integer_param("spreading-limit", 300, new soar_module::predicate<int64_t>(), new smem_db_predicate<int64_t>(thisAgent));
    add(spreading_limit);

    // spreading depth limit - a limit to how many levels deep a source can spread
    spreading_depth_limit = new soar_module::integer_param("spreading-depth-limit", 10, new soar_module::predicate<int64_t>(), new smem_db_predicate<int64_t>(thisAgent));
    add(spreading_depth_limit);

    // spreading loop avoidance - whether or not loopy propagation is avoided
    spreading_loop_avoidance = new soar_module::boolean_param("spreading-loop-avoidance", off, new soar_module::f_predicate<boolean>());
    add(spreading_loop_avoidance);

    // inhibition for use with base-level activation. defaults to off.
    base_inhibition = new soar_module::boolean_param("base-inhibition", off, new soar_module::f_predicate<boolean>());
    add(base_inhibition);

    // using working memory activation for wmes that are LTI-to-LTI edges instanced in working memory to increase edge weight in SMEM
    spreading_edge_updating = new soar_module::boolean_param("spreading-edge-updating", off, new soar_module::f_predicate<boolean>());
    add(spreading_edge_updating);

    spreading_edge_update_factor = new soar_module::decimal_param("spreading-edge-update-factor", 0.99, new soar_module::gt_predicate<double>(0, false), new soar_module::f_predicate<double>());
    add(spreading_edge_update_factor);

    // using wma to supply the starting magnitude for a source of spread
    spreading_wma_source = new soar_module::boolean_param("spreading-wma-source", off, new soar_module::f_predicate<boolean>());
    add(spreading_wma_source);
}

//

/* This is a test of whether or not the SMEM database with no version number is the one
that smem_update_schema_one_to_two can convert.  It tests for the existence of a table name to determine if this is the old version. */
bool SMem_Manager::is_version_one_db()
{
    return DB->execAndGet("SELECT count(type) FROM sqlite_master WHERE type='table' AND name='smem7_signature'").getInt64() != 0;
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

    if (SMem_Manager::memoryDatabasePath == db_path) // Only worry about database version if writing to disk
    {
        std::string schema_version;
        if (thisAgent->SMem->DB->containsData())
        {
            // Check if table exists already
            thisAgent->SMem->DB->exec("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
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

    // A count of spread trajectories
    trajectories_total = new soar_module::integer_stat("trajectories_total", 0, new soar_module::f_predicate<int64_t>());
    add(trajectories_total);
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
    return thisAgent->SMem->DB->getMemoryUsage();
}

smem_mem_high_stat::smem_mem_high_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t smem_mem_high_stat::get_value()
{
    return thisAgent->SMem->DB->getMemoryHighwater();
}

bool SMem_Manager::enabled()
{
    return (settings->learning->get_value() == on);
}

bool SMem_Manager::connected()
{
    return DB != nullptr && SQL != nullptr && JobQueue != nullptr;
}

void smem_param_container::print_settings(agent* thisAgent)
{
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 25);
    outputManager->set_column_indent(1, 58);
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa(thisAgent, "-      Semantic Memory Sub-Commands and Options       -\n");
    outputManager->printa(thisAgent, "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("enabled",learning->get_string(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("database", database->get_string(), 55).c_str(), "Store database in memory or file");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("append", append_db->get_string(), 55).c_str(), "Append or overwrite after init");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("path", path->get_cstring(), 55).c_str(), "Path to database on disk");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem [? | help]", "", 55).c_str(), "Print this help screen");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem [--enable | --disable ]", "", 55).c_str(), "Enable/disable semantic memory");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem [--get | --set] ","<option> [<value>]", 55).c_str(), "Print or set value of an SMem parameter");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --add","{ (id ^attr value)* }", 55).c_str(), "Add concepts to semantic memory");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --backup","<filename>", 55).c_str(), "Saves a copy of database");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --clear","", 55).c_str(), "Deletes all semantic knowledge");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --export","<filename> [<LTI>]", 55).c_str(), "Export database to text file");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --init ","", 55).c_str(), "Reinitialize semantic memory store");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --query ","{(cue)* [<num>]}", 55).c_str(), "Query for concepts in semantic store matching cue");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --remove","{ (id [^attr [value]])* }", 55).c_str(), "Remove semantic memory structures");
    outputManager->printa(thisAgent, "------------------------ Printing ---------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("print","@", 55).c_str(), "Print all of semantic memory");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("print","<LTI>", 55).c_str(), "Print specific semantic memory");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --history","<LTI>", 55).c_str(), "Print activation history for some LTM");
    outputManager->printa(thisAgent, "---------------------- Activation --------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("activation-mode", activation_mode->get_string(), 55).c_str(), "recency, frequency, base-level");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("activate-on-query", activate_on_query->get_string(), 55).c_str(), "on, off");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-decay", base_decay->get_string(), 55).c_str(), "Decay parameter for base-level activation computation");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-update-policy", base_update->get_string(), 55).c_str(), "stable, naive, incremental");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-incremental-threshes", base_incremental_threshes->get_string(), 55).c_str(), "integer > 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("thresh", thresh->get_string(), 55).c_str(), "integer >= 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-inhibition", base_inhibition->get_string(), 55).c_str(), "on, off");
    outputManager->printa(thisAgent, "------------ Experimental Spreading Activation --------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading", spreading->get_string(), 55).c_str(), "on, off");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-limit", spreading_limit->get_string(), 55).c_str(), "integer > 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-depth-limit", spreading_depth_limit->get_string(), 55).c_str(), "integer > 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-baseline", spreading_baseline->get_string(), 55).c_str(), "1 > decimal > 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-continue-probability", spreading_continue_probability->get_string(), 55).c_str(), "1 > decimal > 0");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-loop-avoidance", spreading_loop_avoidance->get_string(), 55).c_str(), "on, off");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-edge-updating", spreading_edge_updating->get_string(), 55).c_str(), "on, off");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-wma-source", spreading_wma_source->get_string(), 55).c_str(), "on, off");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("spreading-edge-update-factor", spreading_edge_update_factor->get_string(), 55).c_str(), "1 > decimal > 0");
    outputManager->printa(thisAgent, "------------- Database Optimization Settings ----------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("lazy-commit", lazy_commit->get_string(), 55).c_str(), "Delay writing semantic store until exit");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("optimization", opt->get_string(), 55).c_str(), "safety, performance");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("cache-size", cache_size->get_string(), 55).c_str(), "Number of memory pages used for SQLite cache");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("page-size", page_size->get_string(), 55).c_str(), "Size of each memory page used");
    outputManager->printa(thisAgent, "----------------- Timers and Statistics ---------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("timers <detail>", timers->get_string(), 55).c_str(), "How detailed timers should be (use --set)");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --timers ","[<timer>]", 55).c_str(), "Print timer summary or specific statistic");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --stats","[<stat>]", 55).c_str(), "Print statistic summary or specific statistic");
    outputManager->printa(thisAgent, "                  ---------------------\n");
    outputManager->printa(thisAgent,
        "Detail: off, one, two, three\n"
        "Timers: smem_api, smem_hash, smem_init, smem_query,\n"
        "        smem_ncb_retrieval, three_activation\n"
        "        smem_storage, _total\n"
        "Stats:  act_updates, db-lib-version, edges, mem-usage,\n"
        "        mem-high, nodes, queries, retrieves, stores\n");
    outputManager->printa(thisAgent, "-------------------------------------------------------\n\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%- help smem\n");
}

void smem_param_container::print_summary(agent* thisAgent)
{
    std::string tempString, tempString2;
    Output_Manager* outputManager = &Output_Manager::Get_OM();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 51);

    outputManager->printa(thisAgent,    "====================================================\n");
    outputManager->printa_sf(thisAgent, "              Semantic Memory Summary\n");
    outputManager->printa(thisAgent,    "====================================================\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Enabled",learning->get_string(), 52).c_str());
    tempString = (database->get_value() == memory) ? "Memory" : "File";
    tempString2 = append_db->get_value() ? "(append after init)" : "(overwrite after init)";
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("Storage", tempString.c_str(), 52).c_str(), tempString2.c_str());
//    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("database", database->get_string(), 52).c_str());
//    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("append", append_db->get_string(), 52).c_str());
    if (strlen(path->get_value()) > 0)
        outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("path", path->get_cstring(), 52).c_str());
    outputManager->printa(thisAgent,    "----------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Nodes", std::to_string(thisAgent->SMem->statistics->nodes->get_value()), 52).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("Edges", std::to_string(thisAgent->SMem->statistics->edges->get_value()), 52).c_str());
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("Memory Usage", std::to_string(thisAgent->SMem->statistics->mem_usage->get_value()), 52).c_str(), "bytes");
    outputManager->printa(thisAgent,    "----------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "For a full list of smem's sub-commands and settings:  smem ?");
}
