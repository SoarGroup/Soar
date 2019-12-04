#include "episodic_memory.h"

#include "agent.h"
#include "decide.h"
#include "ebc.h"
#include "instantiation.h"
#include "preference.h"
#include "semantic_memory.h"
#include "slot.h"
#include "symbol.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <set>
#include <climits>


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters                   epmem::param
// stats                        epmem::stats
// timers                       epmem::timers
// statements                   epmem::statements

// wme-related                  epmem::wmes

// variable abstraction         epmem::var

// relational interval tree     epmem::rit

// cleaning up                  epmem::clean
// initialization               epmem::init

// temporal hash                epmem::hash

// storing new episodes         epmem::storage
// non-cue-based queries        epmem::ncb
// cue-based queries            epmem::cbr

// vizualization                epmem::viz

// high-level api               epmem::api


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (epmem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_param_container::epmem_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    // learning
    learning = new soar_module::boolean_param("learning", off, new soar_module::f_predicate<boolean>());
    add(learning);

    ////////////////////
    // Encoding
    ////////////////////

    // phase
    phase = new soar_module::constant_param<phase_choices>("phase", phase_output, new soar_module::f_predicate<phase_choices>());
    phase->add_mapping(phase_output, "output");
    phase->add_mapping(phase_selection, "selection");
    add(phase);

    // trigger
    trigger = new soar_module::constant_param<trigger_choices>("trigger", dc, new soar_module::f_predicate<trigger_choices>());
    trigger->add_mapping(none, "none");
    trigger->add_mapping(output, "output");
    trigger->add_mapping(dc, "dc");
    add(trigger);

    // force
    force = new soar_module::constant_param<force_choices>("force", force_off, new soar_module::f_predicate<force_choices>());
    force->add_mapping(remember, "remember");
    force->add_mapping(ignore, "ignore");
    force->add_mapping(force_off, "off");
    add(force);

    // exclusions - this is initialized with "epmem" directly after hash tables
    exclusions = new soar_module::sym_set_param("exclusions", new soar_module::f_predicate<const char*>, thisAgent);
    add(exclusions);


    ////////////////////
    // Storage
    ////////////////////

    // database
    database = new soar_module::constant_param<db_choices>("database", memory, new soar_module::f_predicate<db_choices>());
    database->add_mapping(memory, "memory");
    database->add_mapping(file, "file");
    add(database);

    // append database or dump data on init
    append_db = new soar_module::boolean_param("append", off, new soar_module::f_predicate<boolean>());
    add(append_db);

    // path
    path = new epmem_path_param("path", "", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>(), thisAgent);
    add(path);

    // auto-commit
    lazy_commit = new soar_module::boolean_param("lazy-commit", on, new epmem_db_predicate<boolean>(thisAgent));
    add(lazy_commit);

    ////////////////////
    // Retrieval
    ////////////////////

    // graph-match
    graph_match = new soar_module::boolean_param("graph-match", on, new soar_module::f_predicate<boolean>());
    add(graph_match);

    // balance
    balance = new soar_module::decimal_param("balance", 1, new soar_module::btw_predicate<double>(0, 1, true), new soar_module::f_predicate<double>());
    add(balance);


    ////////////////////
    // Performance
    ////////////////////

    // timers
    timers = new soar_module::constant_param<soar_module::timer::timer_level>("timers", soar_module::timer::zero, new soar_module::f_predicate<soar_module::timer::timer_level>());
    timers->add_mapping(soar_module::timer::zero, "off");
    timers->add_mapping(soar_module::timer::one, "one");
    timers->add_mapping(soar_module::timer::two, "two");
    timers->add_mapping(soar_module::timer::three, "three");
    add(timers);

    // page_size
    page_size = new soar_module::constant_param<page_choices>("page-size", page_8k, new epmem_db_predicate<page_choices>(thisAgent));
    page_size->add_mapping(epmem_param_container::page_1k, "1k");
    page_size->add_mapping(epmem_param_container::page_2k, "2k");
    page_size->add_mapping(epmem_param_container::page_4k, "4k");
    page_size->add_mapping(epmem_param_container::page_8k, "8k");
    page_size->add_mapping(epmem_param_container::page_16k, "16k");
    page_size->add_mapping(epmem_param_container::page_32k, "32k");
    page_size->add_mapping(epmem_param_container::page_64k, "64k");
    add(page_size);

    // cache_size
    cache_size = new soar_module::integer_param("cache-size", 10000, new soar_module::gt_predicate<int64_t>(1, true), new epmem_db_predicate<int64_t>(thisAgent));
    add(cache_size);

    // opt
    opt = new soar_module::constant_param<opt_choices>("optimization", epmem_param_container::opt_speed, new epmem_db_predicate<opt_choices>(thisAgent));
    opt->add_mapping(epmem_param_container::opt_safety, "safety");
    opt->add_mapping(epmem_param_container::opt_speed, "performance");
    add(opt);


    ////////////////////
    // Experimental
    ////////////////////

    gm_ordering = new soar_module::constant_param<gm_ordering_choices>("graph-match-ordering", gm_order_undefined, new soar_module::f_predicate<gm_ordering_choices>());
    gm_ordering->add_mapping(gm_order_undefined, "undefined");
    gm_ordering->add_mapping(gm_order_dfs, "dfs");
    gm_ordering->add_mapping(gm_order_mcv, "mcv");
    add(gm_ordering);

    // merge
    merge = new soar_module::constant_param<merge_choices>("merge", merge_none, new soar_module::f_predicate<merge_choices>());
    merge->add_mapping(merge_none, "none");
    merge->add_mapping(merge_add, "add");
    add(merge);
}

//

epmem_path_param::epmem_path_param(const char* new_name, const char* new_value, soar_module::predicate<const char*>* new_val_pred, soar_module::predicate<const char*>* new_prot_pred, agent* new_agent): soar_module::string_param(new_name, new_value, new_val_pred, new_prot_pred), thisAgent(new_agent) {}

void epmem_path_param::set_value(const char* new_value)
{
    /* Removed automatic switching to disk database mode when first setting path.  Now
       that switching databases and database modes on the fly seems to work, there's
       no need to attach special significance to the first time the path is set.
       MMA 2013 */

    value->assign(new_value);
}

//

template <typename T>
epmem_db_predicate<T>::epmem_db_predicate(agent* new_agent): soar_module::agent_predicate<T>(new_agent) {}

template <typename T>
bool epmem_db_predicate<T>::operator()(T /*val*/)
{
    return (this->thisAgent->EpMem->epmem_db->get_status() == soar_module::connected);
}


/***************************************************************************
 * Function     : epmem_enabled
 * Author       : Nate Derbinsky
 * Notes        : Shortcut function to system parameter
 **************************************************************************/
bool epmem_enabled(agent* thisAgent)
{
    return (thisAgent->EpMem->epmem_params->learning->get_value() == on);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Temporal Hash Functions (epmem::hash)
//
// The rete has symbol hashing, but the values are
// reliable only for the lifetime of a symbol.  This
// isn't good for epmem.  Hence, we implement a simple
// lookup table.
//
// Note the hashing functions for the symbol types are
// very similar, but with enough differences that I
// separated them out for clarity.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline epmem_hash_id epmem_temporal_hash_add_type(agent* thisAgent, byte sym_type)
{
    thisAgent->EpMem->epmem_stmts_common->hash_add_type->bind_int(1, sym_type);
    thisAgent->EpMem->epmem_stmts_common->hash_add_type->execute(soar_module::op_reinit);
    return static_cast<epmem_hash_id>(thisAgent->EpMem->epmem_db->last_insert_rowid());
}

inline epmem_hash_id epmem_temporal_hash_int(agent* thisAgent, int64_t val, bool add_on_fail = true)
{
    epmem_hash_id return_val = NIL;

    // search first
    thisAgent->EpMem->epmem_stmts_common->hash_get_int->bind_int(1, val);
    if (thisAgent->EpMem->epmem_stmts_common->hash_get_int->execute() == soar_module::row)
    {
        return_val = static_cast<epmem_hash_id>(thisAgent->EpMem->epmem_stmts_common->hash_get_int->column_int(0));
    }
    thisAgent->EpMem->epmem_stmts_common->hash_get_int->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = epmem_temporal_hash_add_type(thisAgent, INT_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->EpMem->epmem_stmts_common->hash_add_int->bind_int(1, return_val);
        thisAgent->EpMem->epmem_stmts_common->hash_add_int->bind_int(2, val);
        thisAgent->EpMem->epmem_stmts_common->hash_add_int->execute(soar_module::op_reinit);
    }

    return return_val;
}

inline epmem_hash_id epmem_temporal_hash_float(agent* thisAgent, double val, bool add_on_fail = true)
{
    epmem_hash_id return_val = NIL;

    // search first
    thisAgent->EpMem->epmem_stmts_common->hash_get_float->bind_double(1, val);
    if (thisAgent->EpMem->epmem_stmts_common->hash_get_float->execute() == soar_module::row)
    {
        return_val = static_cast<epmem_hash_id>(thisAgent->EpMem->epmem_stmts_common->hash_get_float->column_int(0));
    }
    thisAgent->EpMem->epmem_stmts_common->hash_get_float->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = epmem_temporal_hash_add_type(thisAgent, FLOAT_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->EpMem->epmem_stmts_common->hash_add_float->bind_int(1, return_val);
        thisAgent->EpMem->epmem_stmts_common->hash_add_float->bind_double(2, val);
        thisAgent->EpMem->epmem_stmts_common->hash_add_float->execute(soar_module::op_reinit);
    }

    return return_val;
}

inline epmem_hash_id epmem_temporal_hash_str(agent* thisAgent, char* val, bool add_on_fail = true)
{
    epmem_hash_id return_val = NIL;

    // search first
    thisAgent->EpMem->epmem_stmts_common->hash_get_str->bind_text(1, static_cast<const char*>(val));
    if (thisAgent->EpMem->epmem_stmts_common->hash_get_str->execute() == soar_module::row)
    {
        return_val = static_cast<epmem_hash_id>(thisAgent->EpMem->epmem_stmts_common->hash_get_str->column_int(0));
    }
    thisAgent->EpMem->epmem_stmts_common->hash_get_str->reinitialize();

    // if fail and supposed to add
    if (!return_val && add_on_fail)
    {
        // type first
        return_val = epmem_temporal_hash_add_type(thisAgent, STR_CONSTANT_SYMBOL_TYPE);

        // then content
        thisAgent->EpMem->epmem_stmts_common->hash_add_str->bind_int(1, return_val);
        thisAgent->EpMem->epmem_stmts_common->hash_add_str->bind_text(2, static_cast<const char*>(val));
        thisAgent->EpMem->epmem_stmts_common->hash_add_str->execute(soar_module::op_reinit);
    }

    return return_val;
}


inline int64_t epmem_reverse_hash_int(agent* thisAgent, epmem_hash_id s_id_lookup)
{
    int64_t return_val = NIL;

    thisAgent->EpMem->epmem_stmts_common->hash_rev_int->bind_int(1, s_id_lookup);
    soar_module::exec_result res = thisAgent->EpMem->epmem_stmts_common->hash_rev_int->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = thisAgent->EpMem->epmem_stmts_common->hash_rev_int->column_int(0);
    thisAgent->EpMem->epmem_stmts_common->hash_rev_int->reinitialize();

    return return_val;
}

inline double epmem_reverse_hash_float(agent* thisAgent, epmem_hash_id s_id_lookup)
{
    double return_val = NIL;

    thisAgent->EpMem->epmem_stmts_common->hash_rev_float->bind_int(1, s_id_lookup);
    soar_module::exec_result res = thisAgent->EpMem->epmem_stmts_common->hash_rev_float->execute();
    (void)res; // quells compiler warning
    assert(res == soar_module::row);
    return_val = thisAgent->EpMem->epmem_stmts_common->hash_rev_float->column_double(0);
    thisAgent->EpMem->epmem_stmts_common->hash_rev_float->reinitialize();

    return return_val;
}

inline void epmem_reverse_hash_str(agent* thisAgent, epmem_hash_id s_id_lookup, std::string& dest)
{
    soar_module::exec_result res;
    soar_module::sqlite_statement* sql_hash_rev_str = thisAgent->EpMem->epmem_stmts_common->hash_rev_str;

    sql_hash_rev_str->bind_int(1, s_id_lookup);
    res = sql_hash_rev_str->execute();
    (void)res; // quells compiler warning
    if (res != soar_module::row)
    {
        epmem_close(thisAgent);
    }
    assert(res == soar_module::row);
    dest.assign(sql_hash_rev_str->column_text(0));
    sql_hash_rev_str->reinitialize();
}

/* **************************************************************************

                         epmem_reverse_hash

  This function will take an s_id and return a symbol whose contents match those
  stored in the epmem database.  If no sym_type is passed in, this function will
  look up the type in the symbol type database.

  How type id is handled is changed somewhat  from how smem does it and epmem
  previously did it;  that code retrieves the symbol types within the original
  large query, while this one does another retrieve as needed. Hopefully, we'll
  gain more from removing a join from the big, more computationally intensive
  query than we'll lose from the overhead of a second query.  This leverages
  that we always know the symbol type for id's and attributes and don't even
  need to join with the type table for those.

  Will want to verify later.  If confirmed, we should check if we could do it
  for smem too.  We could also remove the LTI join from the big query too and
  do those retrieves as needed.

************************************************************************** */

inline Symbol* epmem_reverse_hash(agent* thisAgent, epmem_hash_id s_id_lookup, byte sym_type = 255)
{
    Symbol* return_val = NULL;
    std::string dest;

    if (sym_type == 255)
    {
        thisAgent->EpMem->epmem_stmts_common->hash_get_type->bind_int(1, s_id_lookup);
        soar_module::exec_result res = thisAgent->EpMem->epmem_stmts_common->hash_get_type->execute();
        (void)res; // quells compiler warning
        assert(res == soar_module::row);
        sym_type = static_cast<byte>(thisAgent->EpMem->epmem_stmts_common->hash_get_type->column_int(0));
        thisAgent->EpMem->epmem_stmts_common->hash_get_type->reinitialize();
    }

    switch (sym_type)
    {
        case STR_CONSTANT_SYMBOL_TYPE:
            epmem_reverse_hash_str(thisAgent, s_id_lookup, dest);
            return_val = thisAgent->symbolManager->make_str_constant(const_cast<char*>(dest.c_str()));
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_int_constant(epmem_reverse_hash_int(thisAgent, s_id_lookup));
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return_val = thisAgent->symbolManager->make_float_constant(epmem_reverse_hash_float(thisAgent, s_id_lookup));
            break;

        default:
            return_val = NULL;
            break;
    }

    return return_val;
}

/* **************************************************************************

                         epmem_reverse_hash_print

  This function will take an s_id and stores a printable string version of the
  content of that symbol stored in the epmem database into the dest parameter.
  If no sym_type is passed in, this function will look up the type in the
  symbol type database.


************************************************************************** */

inline void epmem_reverse_hash_print(agent* thisAgent, epmem_hash_id s_id_lookup, std::string& dest, byte sym_type = 255)
{
    Symbol* return_val = NULL;

    // This may be faster than including type lookup in edges?  Might want to check later.

    if (sym_type == 255)
    {
        thisAgent->EpMem->epmem_stmts_common->hash_get_type->bind_int(1, s_id_lookup);
        soar_module::exec_result res = thisAgent->EpMem->epmem_stmts_common->hash_get_type->execute();
        (void)res; // quells compiler warning
        assert(res == soar_module::row);
        // check if should be column_int
        sym_type = static_cast<byte>(thisAgent->EpMem->epmem_stmts_common->hash_get_type->column_int(0));
        thisAgent->EpMem->epmem_stmts_common->hash_get_type->reinitialize();
    }

    switch (sym_type)
    {
        case STR_CONSTANT_SYMBOL_TYPE:
            epmem_reverse_hash_str(thisAgent, s_id_lookup, dest);
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            to_string(epmem_reverse_hash_int(thisAgent, s_id_lookup), dest);
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            to_string(epmem_reverse_hash_float(thisAgent, s_id_lookup), dest);
            break;

        default:
            return_val = NULL;
            break;
    }
}

/* **************************************************************************

                         epmem_temporal_hash

    This function returns an s_id (symbol id) representing a symbol constant
    stored in the epmem database. The individual hash functions will first
    check if there already exists an identical entry in the epmem database
    and return it if found.  If not, it will add new entries to the both the
    type table and one of the typed constant tables.

************************************************************************** */

epmem_hash_id epmem_temporal_hash(agent* thisAgent, Symbol* sym, bool add_on_fail)
{
    epmem_hash_id return_val = NIL;

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->hash->start();
    ////////////////////////////////////////////////////////////////////////////

    if (sym->is_constant())
    {
        if ((!sym->epmem_hash) || (sym->epmem_valid != thisAgent->EpMem->epmem_validation))
        {
            sym->epmem_hash = NIL;
            sym->epmem_valid = thisAgent->EpMem->epmem_validation;

            switch (sym->symbol_type)
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                    return_val = epmem_temporal_hash_str(thisAgent, sym->sc->name, add_on_fail);
                    break;

                case INT_CONSTANT_SYMBOL_TYPE:
                    return_val = epmem_temporal_hash_int(thisAgent, sym->ic->value, add_on_fail);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return_val = epmem_temporal_hash_float(thisAgent, sym->fc->value, add_on_fail);
                    break;
            }

            // cache results for later re-use
            sym->epmem_hash = return_val;
            sym->epmem_valid = thisAgent->EpMem->epmem_validation;
        }

        return_val = sym->epmem_hash;
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->hash->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (epmem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_stat_container::epmem_stat_container(agent* new_agent): soar_module::stat_container(new_agent)
{
    // time
    time = new epmem_time_id_stat("time", 0, new epmem_db_predicate<epmem_time_id>(thisAgent));
    add(time);

    // db-lib-version
    db_lib_version = new epmem_db_lib_version_stat(thisAgent, "db-lib-version", NULL, new soar_module::predicate< const char* >());
    add(db_lib_version);

    // mem-usage
    mem_usage = new epmem_mem_usage_stat(thisAgent, "mem-usage", 0, new soar_module::predicate<int64_t>());
    add(mem_usage);

    // mem-high
    mem_high = new epmem_mem_high_stat(thisAgent, "mem-high", 0, new soar_module::predicate<int64_t>());
    add(mem_high);

    // non-cue-based-retrievals
    ncbr = new soar_module::integer_stat("retrievals", 0, new soar_module::f_predicate<int64_t>());
    add(ncbr);

    // cue-based-retrievals
    cbr = new soar_module::integer_stat("queries", 0, new soar_module::f_predicate<int64_t>());
    add(cbr);

    // nexts
    nexts = new soar_module::integer_stat("nexts", 0, new soar_module::f_predicate<int64_t>());
    add(nexts);

    // prev's
    prevs = new soar_module::integer_stat("prevs", 0, new soar_module::f_predicate<int64_t>());
    add(prevs);

    // ncb-wmes
    ncb_wmes = new soar_module::integer_stat("ncb-wmes", 0, new soar_module::f_predicate<int64_t>());
    add(ncb_wmes);

    // qry-pos
    qry_pos = new soar_module::integer_stat("qry-pos", 0, new soar_module::f_predicate<int64_t>());
    add(qry_pos);

    // qry-neg
    qry_neg = new soar_module::integer_stat("qry-neg", 0, new soar_module::f_predicate<int64_t>());
    add(qry_neg);

    // qry-ret
    qry_ret = new epmem_time_id_stat("qry-ret", 0, new soar_module::f_predicate<epmem_time_id>());
    add(qry_ret);

    // qry-card
    qry_card = new soar_module::integer_stat("qry-card", 0, new soar_module::f_predicate<int64_t>());
    add(qry_card);

    // qry-lits
    qry_lits = new soar_module::integer_stat("qry-lits", 0, new soar_module::f_predicate<int64_t>());
    add(qry_lits);

    // next-id
    next_id = new epmem_node_id_stat("next-id", 0, new epmem_db_predicate<epmem_node_id>(thisAgent));
    add(next_id);

    // rit-offset-1
    rit_offset_1 = new soar_module::integer_stat("rit-offset-1", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_offset_1);

    // rit-left-root-1
    rit_left_root_1 = new soar_module::integer_stat("rit-left-root-1", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_left_root_1);

    // rit-right-root-1
    rit_right_root_1 = new soar_module::integer_stat("rit-right-root-1", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_right_root_1);

    // rit-min-step-1
    rit_min_step_1 = new soar_module::integer_stat("rit-min-step-1", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_min_step_1);

    // rit-offset-2
    rit_offset_2 = new soar_module::integer_stat("rit-offset-2", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_offset_2);

    // rit-left-root-2
    rit_left_root_2 = new soar_module::integer_stat("rit-left-root-2", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_left_root_2);

    // rit-right-root-2
    rit_right_root_2 = new soar_module::integer_stat("rit-right-root-2", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_right_root_2);

    // rit-min-step-2
    rit_min_step_2 = new soar_module::integer_stat("rit-min-step-2", 0, new epmem_db_predicate<int64_t>(thisAgent));
    add(rit_min_step_2);


    /////////////////////////////
    // connect to rit state
    /////////////////////////////

    // graph
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.stat = rit_offset_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].offset.var_key = var_rit_offset_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.stat = rit_left_root_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].leftroot.var_key = var_rit_leftroot_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.stat = rit_right_root_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].rightroot.var_key = var_rit_rightroot_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.stat = rit_min_step_1;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].minstep.var_key = var_rit_minstep_1;

    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.stat = rit_offset_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].offset.var_key = var_rit_offset_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.stat = rit_left_root_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].leftroot.var_key = var_rit_leftroot_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.stat = rit_right_root_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].rightroot.var_key = var_rit_rightroot_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.stat = rit_min_step_2;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].minstep.var_key = var_rit_minstep_2;
}

//

epmem_db_lib_version_stat::epmem_db_lib_version_stat(agent* new_agent, const char* new_name, const char* new_value, soar_module::predicate< const char* >* new_prot_pred): soar_module::primitive_stat< const char* >(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

const char* epmem_db_lib_version_stat::get_value()
{
    return thisAgent->EpMem->epmem_db->lib_version();
}

//

epmem_mem_usage_stat::epmem_mem_usage_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t epmem_mem_usage_stat::get_value()
{
    return thisAgent->EpMem->epmem_db->memory_usage();
}

//

epmem_mem_high_stat::epmem_mem_high_stat(agent* new_agent, const char* new_name, int64_t new_value, soar_module::predicate<int64_t>* new_prot_pred): soar_module::integer_stat(new_name, new_value, new_prot_pred), thisAgent(new_agent) {}

int64_t epmem_mem_high_stat::get_value()
{
    return thisAgent->EpMem->epmem_db->memory_highwater();
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (epmem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

epmem_timer_container::epmem_timer_container(agent* new_agent): soar_module::timer_container(new_agent)
{
    // one

    total = new epmem_timer("_total", thisAgent, soar_module::timer::one);
    add(total);

    // two

    storage = new epmem_timer("epmem_storage", thisAgent, soar_module::timer::two);
    add(storage);

    ncb_retrieval = new epmem_timer("epmem_ncb_retrieval", thisAgent, soar_module::timer::two);
    add(ncb_retrieval);

    query = new epmem_timer("epmem_query", thisAgent, soar_module::timer::two);
    add(query);

    api = new epmem_timer("epmem_api", thisAgent, soar_module::timer::two);
    add(api);

    trigger = new epmem_timer("epmem_trigger", thisAgent, soar_module::timer::two);
    add(trigger);

    init = new epmem_timer("epmem_init", thisAgent, soar_module::timer::two);
    add(init);

    next = new epmem_timer("epmem_next", thisAgent, soar_module::timer::two);
    add(next);

    prev = new epmem_timer("epmem_prev", thisAgent, soar_module::timer::two);
    add(prev);

    hash = new epmem_timer("epmem_hash", thisAgent, soar_module::timer::two);
    add(hash);

    wm_phase = new epmem_timer("epmem_wm_phase", thisAgent, soar_module::timer::two);
    add(wm_phase);

    // three

    ncb_edge = new epmem_timer("ncb_edge", thisAgent, soar_module::timer::three);
    add(ncb_edge);

    ncb_edge_rit = new epmem_timer("ncb_edge_rit", thisAgent, soar_module::timer::three);
    add(ncb_edge_rit);

    ncb_node = new epmem_timer("ncb_node", thisAgent, soar_module::timer::three);
    add(ncb_node);

    ncb_node_rit = new epmem_timer("ncb_node_rit", thisAgent, soar_module::timer::three);
    add(ncb_node_rit);

    query_dnf = new epmem_timer("query_dnf", thisAgent, soar_module::timer::three);
    add(query_dnf);

    query_walk = new epmem_timer("query_walk", thisAgent, soar_module::timer::three);
    add(query_walk);

    query_walk_edge = new epmem_timer("query_walk_edge", thisAgent, soar_module::timer::three);
    add(query_walk_edge);

    query_walk_interval = new epmem_timer("query_walk_interval", thisAgent, soar_module::timer::three);
    add(query_walk_interval);

    query_graph_match = new epmem_timer("query_graph_match", thisAgent, soar_module::timer::three);
    add(query_graph_match);

    query_result = new epmem_timer("query_result", thisAgent, soar_module::timer::three);
    add(query_result);

    query_cleanup = new epmem_timer("query_cleanup", thisAgent, soar_module::timer::three);
    add(query_cleanup);

    query_sql_edge = new epmem_timer("query_sql_edge", thisAgent, soar_module::timer::three);
    add(query_sql_edge);

    query_sql_start_ep = new epmem_timer("query_sql_start_ep", thisAgent, soar_module::timer::three);
    add(query_sql_start_ep);

    query_sql_start_now = new epmem_timer("query_sql_start_now", thisAgent, soar_module::timer::three);
    add(query_sql_start_now);

    query_sql_start_point = new epmem_timer("query_sql_start_point", thisAgent, soar_module::timer::three);
    add(query_sql_start_point);

    query_sql_end_ep = new epmem_timer("query_sql_end_ep", thisAgent, soar_module::timer::three);
    add(query_sql_end_ep);

    query_sql_end_now = new epmem_timer("query_sql_end_now", thisAgent, soar_module::timer::three);
    add(query_sql_end_now);

    query_sql_end_point = new epmem_timer("query_sql_end_point", thisAgent, soar_module::timer::three);
    add(query_sql_end_point);

    /////////////////////////////
    // connect to rit state
    /////////////////////////////

    // graph
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].timer = ncb_node_rit;
    thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].timer = ncb_edge_rit;
}

//

epmem_timer_level_predicate::epmem_timer_level_predicate(agent* new_agent): soar_module::agent_predicate<soar_module::timer::timer_level>(new_agent) {}

bool epmem_timer_level_predicate::operator()(soar_module::timer::timer_level val)
{
    return (thisAgent->EpMem->epmem_params->timers->get_value() >= val);
}

//

epmem_timer::epmem_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level): soar_module::timer(new_name, new_agent, new_level, new epmem_timer_level_predicate(new_agent)) {}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statement Functions (epmem::statements)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void epmem_common_statement_container::create_graph_tables()
{

    add_structure("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_persistent_variables (variable_id INTEGER PRIMARY KEY,variable_value NONE)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_rit_left_nodes (rit_min INTEGER, rit_max INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_rit_right_nodes (rit_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_symbols_type (s_id INTEGER PRIMARY KEY, symbol_type INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_symbols_integer (s_id INTEGER PRIMARY KEY, symbol_value INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_symbols_float (s_id INTEGER PRIMARY KEY, symbol_value REAL)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_symbols_string (s_id INTEGER PRIMARY KEY, symbol_value TEXT)");

}

void epmem_common_statement_container::create_graph_indices()
{

    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS symbols_int_const ON epmem_symbols_integer (symbol_value)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS symbols_float_const ON epmem_symbols_float (symbol_value)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS symbols_str_const ON epmem_symbols_string (symbol_value)");

}

void epmem_common_statement_container::drop_graph_tables()
{

    // Note: We don't want to dump versions database because it might also contain other version information
    // if we ever combine epmem and smem into one database, which is something that has been discussed

    add_structure("DROP TABLE IF EXISTS epmem_persistent_variables");
    add_structure("DROP TABLE IF EXISTS epmem_rit_left_nodes");
    add_structure("DROP TABLE IF EXISTS epmem_rit_right_nodes");
    add_structure("DROP TABLE IF EXISTS epmem_symbols_type");
    add_structure("DROP TABLE IF EXISTS epmem_symbols_integer");
    add_structure("DROP TABLE IF EXISTS epmem_symbols_float");
    add_structure("DROP TABLE IF EXISTS epmem_symbols_string");

}

epmem_common_statement_container::epmem_common_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->EpMem->epmem_db)
{
    soar_module::sqlite_database* new_db = new_agent->EpMem->epmem_db;

    // Drop tables in the database if append setting is off.  (Tried DELETE before, but it had problems.)
    if ((new_agent->EpMem->epmem_params->database->get_value() != epmem_param_container::memory) &&
            (new_agent->EpMem->epmem_params->append_db->get_value() == off))
    {
        drop_graph_tables();
    }

    create_graph_tables();
    create_graph_indices();

    // Update the schema version number
    add_structure("INSERT OR REPLACE INTO versions (system, version_number) VALUES ('epmem_schema'," EPMEM_SCHEMA_VERSION ")");

    // Add symbol lookups for special cases

    // Root node of tree
    // Note:  I don't think root node string is ever actually looked up.  Set to root instead of
    //        previous NULL for compatibility with other db systems.
    add_structure("INSERT OR IGNORE INTO epmem_symbols_type (s_id,symbol_type) VALUES (0,2)");
    add_structure("INSERT OR IGNORE INTO epmem_symbols_string (s_id,symbol_value) VALUES (0,'root')");

    // Acceptable preference wmes: id 1 = "operator+"
    add_structure("INSERT OR IGNORE INTO epmem_symbols_type (s_id,symbol_type) VALUES (1,2)");
    add_structure("INSERT OR IGNORE INTO epmem_symbols_string (s_id,symbol_value) VALUES (1,'operator*')");

    //

    begin = new soar_module::sqlite_statement(new_db, "BEGIN");
    add(begin);

    commit = new soar_module::sqlite_statement(new_db, "COMMIT");
    add(commit);

    rollback = new soar_module::sqlite_statement(new_db, "ROLLBACK");
    add(rollback);

    //

    var_get = new soar_module::sqlite_statement(new_db, "SELECT variable_value FROM epmem_persistent_variables WHERE variable_id=?");
    add(var_get);

    var_set = new soar_module::sqlite_statement(new_db, "REPLACE INTO epmem_persistent_variables (variable_id,variable_value) VALUES (?,?)");
    add(var_set);

    //

    rit_add_left = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_rit_left_nodes (rit_min,rit_max) VALUES (?,?)");
    add(rit_add_left);

    rit_truncate_left = new soar_module::sqlite_statement(new_db, "DELETE FROM epmem_rit_left_nodes");
    add(rit_truncate_left);

    rit_add_right = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_rit_right_nodes (rit_id) VALUES (?)");
    add(rit_add_right);

    rit_truncate_right = new soar_module::sqlite_statement(new_db, "DELETE FROM epmem_rit_right_nodes");
    add(rit_truncate_right);

    //

    hash_rev_int = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM epmem_symbols_integer WHERE s_id=?");
    add(hash_rev_int);

    hash_rev_float = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM epmem_symbols_float WHERE s_id=?");
    add(hash_rev_float);

    hash_rev_str = new soar_module::sqlite_statement(new_db, "SELECT symbol_value FROM epmem_symbols_string WHERE s_id=?");
    add(hash_rev_str);

    hash_get_int = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM epmem_symbols_integer WHERE symbol_value=?");
    add(hash_get_int);

    hash_get_float = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM epmem_symbols_float WHERE symbol_value=?");
    add(hash_get_float);

    hash_get_str = new soar_module::sqlite_statement(new_db, "SELECT s_id FROM epmem_symbols_string WHERE symbol_value=?");
    add(hash_get_str);

    hash_get_type = new soar_module::sqlite_statement(new_db, "SELECT symbol_type FROM epmem_symbols_type WHERE s_id=?");
    add(hash_get_type);

    hash_add_type = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_symbols_type (symbol_type) VALUES (?)");
    add(hash_add_type);

    hash_add_int = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_symbols_integer (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_int);

    hash_add_float = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_symbols_float (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_float);

    hash_add_str = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_symbols_string (s_id,symbol_value) VALUES (?,?)");
    add(hash_add_str);

}

void epmem_graph_statement_container::create_graph_tables()
{

    add_structure("CREATE TABLE IF NOT EXISTS epmem_nodes (n_id INTEGER, lti_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_episodes (episode_id INTEGER PRIMARY KEY)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_constant_now (wc_id INTEGER,start_episode_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_identifier_now (wi_id INTEGER,start_episode_id INTEGER, lti_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_constant_point (wc_id INTEGER,episode_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_identifier_point (wi_id INTEGER,episode_id INTEGER, lti_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_constant_range (rit_id INTEGER,start_episode_id INTEGER,end_episode_id INTEGER,wc_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_identifier_range (rit_id INTEGER,start_episode_id INTEGER,end_episode_id INTEGER,wi_id INTEGER, lti_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_constant (wc_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_n_id INTEGER,attribute_s_id INTEGER, value_s_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_wmes_identifier (wi_id INTEGER PRIMARY KEY AUTOINCREMENT,parent_n_id INTEGER,attribute_s_id INTEGER,child_n_id INTEGER, last_episode_id INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS epmem_ascii (ascii_num INTEGER PRIMARY KEY, ascii_chr TEXT)");
}

void epmem_graph_statement_container::create_graph_indices()
{
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_node_lti ON epmem_nodes (n_id, lti_id)");
    add_structure("CREATE INDEX IF NOT EXISTS epmem_lti ON epmem_nodes (lti_id)");

    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_constant_now_start ON epmem_wmes_constant_now (start_episode_id)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_constant_now_id_start ON epmem_wmes_constant_now (wc_id,start_episode_id DESC)");

    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_identifier_now_start ON epmem_wmes_identifier_now (start_episode_id)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_identifier_now_id_start ON epmem_wmes_identifier_now (wi_id,start_episode_id DESC)");

    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_constant_point_id_start ON epmem_wmes_constant_point (wc_id,episode_id DESC)");
    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_constant_point_start ON epmem_wmes_constant_point (episode_id)");

    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_identifier_point_id_start ON epmem_wmes_identifier_point (wi_id,episode_id DESC)");
    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_identifier_point_start ON epmem_wmes_identifier_point (episode_id)");

    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_constant_range_lower ON epmem_wmes_constant_range (rit_id,start_episode_id)");
    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_constant_range_upper ON epmem_wmes_constant_range (rit_id,end_episode_id)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_constant_range_id_start ON epmem_wmes_constant_range (wc_id,start_episode_id DESC)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_constant_range_id_end_start ON epmem_wmes_constant_range (wc_id,end_episode_id DESC,start_episode_id)");

    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_identifier_range_lower ON epmem_wmes_identifier_range (rit_id,start_episode_id)");
    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_identifier_range_upper ON epmem_wmes_identifier_range (rit_id,end_episode_id)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_identifier_range_id_start ON epmem_wmes_identifier_range (wi_id,start_episode_id DESC)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_identifier_range_id_end_start ON epmem_wmes_identifier_range (wi_id,end_episode_id DESC,start_episode_id)");

    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_constant_parent_attribute_value ON epmem_wmes_constant (parent_n_id,attribute_s_id,value_s_id)");

    add_structure("CREATE INDEX IF NOT EXISTS epmem_wmes_identifier_parent_attribute_last ON epmem_wmes_identifier (parent_n_id,attribute_s_id,last_episode_id)");
    add_structure("CREATE UNIQUE INDEX IF NOT EXISTS epmem_wmes_identifier_parent_attribute_child ON epmem_wmes_identifier (parent_n_id,attribute_s_id,child_n_id)");

}

void epmem_graph_statement_container::drop_graph_tables()
{
    add_structure("DROP TABLE IF EXISTS epmem_nodes");
    add_structure("DROP TABLE IF EXISTS epmem_episodes");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_constant_now");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_identifier_now");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_constant_point");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_identifier_point");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_constant_range");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_identifier_range");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_constant");
    add_structure("DROP TABLE IF EXISTS epmem_wmes_identifier");
}

epmem_graph_statement_container::epmem_graph_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->EpMem->epmem_db)
{
    soar_module::sqlite_database* new_db = new_agent->EpMem->epmem_db;

    // Delete all entries from the tables in the database if append setting is off
    if (new_agent->EpMem->epmem_params->append_db->get_value() == off)
    {
        print_sysparam_trace(new_agent, 0, "Erasing contents of episodic memory database. (append = off)\n");
        drop_graph_tables();
    }

    create_graph_tables();
    create_graph_indices();

    // workaround for tree: type 1 = IDENTIFIER_SYMBOL_TYPE
    add_structure("INSERT OR IGNORE INTO epmem_nodes (n_id) VALUES (0)");
    {
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (65,'A')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (66,'B')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (67,'C')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (68,'D')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (69,'E')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (70,'F')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (71,'G')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (72,'H')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (73,'I')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (74,'J')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (75,'K')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (76,'L')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (77,'M')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (78,'N')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (79,'O')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (80,'P')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (81,'Q')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (82,'R')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (83,'S')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (84,'T')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (85,'U')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (86,'V')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (87,'W')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (88,'X')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (89,'Y')");
        add_structure("INSERT OR IGNORE INTO epmem_ascii (ascii_num, ascii_chr) VALUES (90,'Z')");
    }

    //

    add_time = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_episodes (episode_id) VALUES (?)");
    add(add_time);

    add_node = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_nodes (n_id,lti_id) VALUES (?,?)");
    add(add_node);

    update_node = new soar_module::sqlite_statement(new_db, "INSERT OR REPLACE INTO epmem_nodes (n_id, lti_id) VALUES (?,?)");//"UPDATE epmem_nodes SET lti_id=? where n_id=?");
    add(update_node);


    add_epmem_wmes_constant_now = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_constant_now (wc_id,start_episode_id) VALUES (?,?)");
    add(add_epmem_wmes_constant_now);

    delete_epmem_wmes_constant_now = new soar_module::sqlite_statement(new_db, "DELETE FROM epmem_wmes_constant_now WHERE wc_id=?");
    add(delete_epmem_wmes_constant_now);

    add_epmem_wmes_constant_point = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_constant_point (wc_id,episode_id) VALUES (?,?)");
    add(add_epmem_wmes_constant_point);

    add_epmem_wmes_constant_range = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_constant_range (rit_id,start_episode_id,end_episode_id,wc_id) VALUES (?,?,?,?)");
    add(add_epmem_wmes_constant_range);

    add_epmem_wmes_constant = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_constant (parent_n_id,attribute_s_id,value_s_id) VALUES (?,?,?)");
    add(add_epmem_wmes_constant);

    find_epmem_wmes_constant = new soar_module::sqlite_statement(new_db, "SELECT wc_id FROM epmem_wmes_constant WHERE parent_n_id=? AND attribute_s_id=? AND value_s_id=?");
    add(find_epmem_wmes_constant);

    add_epmem_wmes_identifier_now = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_identifier_now (wi_id,start_episode_id,lti_id) VALUES (?,?,?)");
    add(add_epmem_wmes_identifier_now);

    delete_epmem_wmes_identifier_now = new soar_module::sqlite_statement(new_db, "DELETE FROM epmem_wmes_identifier_now WHERE wi_id=?");
    add(delete_epmem_wmes_identifier_now);

    add_epmem_wmes_identifier_point = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_identifier_point (wi_id,episode_id,lti_id) VALUES (?,?,?)");
    add(add_epmem_wmes_identifier_point);

    add_epmem_wmes_identifier_range = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_identifier_range (rit_id,start_episode_id,end_episode_id,wi_id,lti_id) VALUES (?,?,?,?,?)");
    add(add_epmem_wmes_identifier_range);

    add_epmem_wmes_identifier = new soar_module::sqlite_statement(new_db, "INSERT INTO epmem_wmes_identifier (parent_n_id,attribute_s_id,child_n_id,last_episode_id) VALUES (?,?,?,?)");
    add(add_epmem_wmes_identifier);

    find_epmem_wmes_identifier = new soar_module::sqlite_statement(new_db, "SELECT wi_id, child_n_id FROM epmem_wmes_identifier WHERE parent_n_id=? AND attribute_s_id=?");
    add(find_epmem_wmes_identifier);

    find_epmem_wmes_identifier_shared = new soar_module::sqlite_statement(new_db, "SELECT wi_id, child_n_id FROM epmem_wmes_identifier WHERE parent_n_id=? AND attribute_s_id=? AND child_n_id=?");
    add(find_epmem_wmes_identifier_shared);

    valid_episode = new soar_module::sqlite_statement(new_db, "SELECT COUNT(*) AS ct FROM epmem_episodes WHERE episode_id=?");
    add(valid_episode);

    next_episode = new soar_module::sqlite_statement(new_db, "SELECT episode_id FROM epmem_episodes WHERE episode_id>? ORDER BY episode_id ASC LIMIT 1");
    add(next_episode);

    prev_episode = new soar_module::sqlite_statement(new_db, "SELECT episode_id FROM epmem_episodes WHERE episode_id<? ORDER BY episode_id DESC LIMIT 1");
    add(prev_episode);

    get_wmes_with_constant_values = new soar_module::sqlite_statement(new_db,
            "SELECT f.wc_id, f.parent_n_id, f.attribute_s_id, f.value_s_id "
            "FROM epmem_wmes_constant f "
            "WHERE f.wc_id IN "
            "(SELECT n.wc_id FROM epmem_wmes_constant_now n WHERE n.start_episode_id<= ? UNION ALL "
            "SELECT p.wc_id FROM epmem_wmes_constant_point p WHERE p.episode_id=? UNION ALL "
            "SELECT e1.wc_id FROM epmem_wmes_constant_range e1, epmem_rit_left_nodes lt WHERE e1.rit_id=lt.rit_min AND e1.end_episode_id >= ? UNION ALL "
            "SELECT e2.wc_id FROM epmem_wmes_constant_range e2, epmem_rit_right_nodes rt WHERE e2.rit_id = rt.rit_id AND e2.start_episode_id <= ?) "
            "ORDER BY f.wc_id ASC", new_agent->EpMem->epmem_timers->ncb_node);
    add(get_wmes_with_constant_values);

    get_wmes_with_identifier_values = new soar_module::sqlite_statement(new_db,
            "WITH timetables AS ( "
            "SELECT n.wi_id, n.lti_id FROM epmem_wmes_identifier_now n WHERE n.start_episode_id<= ? UNION ALL "
            "SELECT p.wi_id, p.lti_id FROM epmem_wmes_identifier_point p WHERE p.episode_id = ? UNION ALL "
            "SELECT e1.wi_id, e1.lti_id FROM epmem_wmes_identifier_range e1, epmem_rit_left_nodes lt WHERE e1.rit_id=lt.rit_min AND e1.end_episode_id >= ? UNION ALL "
            "SELECT e2.wi_id, e2.lti_id FROM epmem_wmes_identifier_range e2, epmem_rit_right_nodes rt WHERE e2.rit_id = rt.rit_id AND e2.start_episode_id <= ?) "
            "SELECT f.parent_n_id, f.attribute_s_id, f.child_n_id, n.lti_id FROM epmem_wmes_identifier f, timetables n WHERE f.wi_id=n.wi_id "
            "ORDER BY f.parent_n_id ASC, f.child_n_id ASC", new_agent->EpMem->epmem_timers->ncb_edge);
            /*"SELECT f.parent_n_id, f.attribute_s_id, f.child_n_id, n.lti_id "
            "FROM epmem_wmes_identifier f, epmem_nodes n "
            "WHERE (f.child_n_id=n.n_id) AND f.wi_id IN "
            "(SELECT n.wi_id FROM epmem_wmes_identifier_now n WHERE n.start_episode_id<= ? UNION ALL "
            "SELECT p.wi_id FROM epmem_wmes_identifier_point p WHERE p.episode_id = ? UNION ALL "
            "SELECT e1.wi_id FROM epmem_wmes_identifier_range e1, epmem_rit_left_nodes lt WHERE e1.rit_id=lt.rit_min AND e1.end_episode_id >= ? UNION ALL "
            "SELECT e2.wi_id FROM epmem_wmes_identifier_range e2, epmem_rit_right_nodes rt WHERE e2.rit_id = rt.rit_id AND e2.start_episode_id <= ?) "
            "ORDER BY f.parent_n_id ASC, f.child_n_id ASC", new_agent->EpMem->epmem_timers->ncb_edge);*/
    add(get_wmes_with_identifier_values);

    update_epmem_wmes_identifier_last_episode_id = new soar_module::sqlite_statement(new_db, "UPDATE epmem_wmes_identifier SET last_episode_id=? WHERE wi_id=?");
    add(update_epmem_wmes_identifier_last_episode_id);

    // init statement pools
    {
        int j, k, m;

        const char* epmem_find_edge_queries[2][2] =
        {
            {
                "SELECT wc_id, value_s_id, ? FROM epmem_wmes_constant WHERE parent_n_id=? AND attribute_s_id=?",
                "SELECT wc_id, value_s_id, ? FROM epmem_wmes_constant WHERE parent_n_id=? AND attribute_s_id=? AND value_s_id=?"
            },
            {
                "SELECT wi_id, child_n_id, last_episode_id FROM epmem_wmes_identifier WHERE parent_n_id=? AND attribute_s_id=? AND ?<last_episode_id ORDER BY last_episode_id DESC",
                "SELECT wi_id, child_n_id, last_episode_id FROM epmem_wmes_identifier WHERE parent_n_id=? AND attribute_s_id=? AND child_n_id=? AND ?<last_episode_id"
            }
        };

        for (j = EPMEM_RIT_STATE_NODE; j <= EPMEM_RIT_STATE_EDGE; j++)
        {
            for (k = 0; k <= 1; k++)
            {
                pool_find_edge_queries[ j ][ k ] = new soar_module::sqlite_statement_pool(new_agent, new_db, epmem_find_edge_queries[ j ][ k ]);
            }
        }

        //

        // Because the DB records when things are /inserted/, we need to offset
        // the start by 1 to /remove/ them at the right time. Ditto to even
        // include those intervals correctly
        const char* epmem_find_interval_queries[2][2][3] =
        {
            {
                {
                    "SELECT (e.start_episode_id - 1) AS start FROM epmem_wmes_constant_range e WHERE e.wc_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT (e.start_episode_id - 1) AS start FROM epmem_wmes_constant_now e WHERE e.wc_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT (e.episode_id - 1) AS start FROM epmem_wmes_constant_point e WHERE e.wc_id=? AND e.episode_id<=? ORDER BY e.episode_id DESC"
                },
                {
                    "SELECT e.end_episode_id AS end FROM epmem_wmes_constant_range e WHERE e.wc_id=? AND e.end_episode_id>0 AND e.start_episode_id<=? ORDER BY e.end_episode_id DESC",
                    "SELECT ? AS end FROM epmem_wmes_constant_now e WHERE e.wc_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT e.episode_id AS end FROM epmem_wmes_constant_point e WHERE e.wc_id=? AND e.episode_id<=? ORDER BY e.episode_id DESC"
                }
            },
            {
                {
                    "SELECT (e.start_episode_id - 1) AS start FROM epmem_wmes_identifier_range e WHERE e.wi_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT (e.start_episode_id - 1) AS start FROM epmem_wmes_identifier_now e WHERE e.wi_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT (e.episode_id - 1) AS start FROM epmem_wmes_identifier_point e WHERE e.wi_id=? AND e.episode_id<=? ORDER BY e.episode_id DESC"
                },
                {
                    "SELECT e.end_episode_id AS end FROM epmem_wmes_identifier_range e WHERE e.wi_id=? AND e.end_episode_id>0 AND e.start_episode_id<=? ORDER BY e.end_episode_id DESC",
                    "SELECT ? AS end FROM epmem_wmes_identifier_now e WHERE e.wi_id=? AND e.start_episode_id<=? ORDER BY e.start_episode_id DESC",
                    "SELECT e.episode_id AS end FROM epmem_wmes_identifier_point e WHERE e.wi_id=? AND e.episode_id<=? ORDER BY e.episode_id DESC"
                }
            },
        };

        for (j = EPMEM_RIT_STATE_NODE; j <= EPMEM_RIT_STATE_EDGE; j++)
        {
            for (k = EPMEM_RANGE_START; k <= EPMEM_RANGE_END; k++)
            {
                for (m = EPMEM_RANGE_EP; m <= EPMEM_RANGE_POINT; m++)
                {
                    pool_find_interval_queries[ j ][ k ][ m ] = new soar_module::sqlite_statement_pool(new_agent, new_db, epmem_find_interval_queries[ j ][ k ][ m ]);
                }
            }
        }

        //

        pool_dummy = new soar_module::sqlite_statement_pool(new_agent, new_db, "SELECT ? as start");
    }
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (epmem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_augs_of_id
 * Author       : Nate Derbinsky
 * Notes        : This routine gets all wmes rooted at an id.
 **************************************************************************/
epmem_wme_list* epmem_get_augs_of_id(Symbol* id, tc_number tc)
{
    slot* s;
    wme* w;
    epmem_wme_list* return_val = new epmem_wme_list;

    // augs only exist for identifiers
    if ((id->is_sti()) &&
            (id->tc_num != tc))
    {
        id->tc_num = tc;

        // impasse wmes
        for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        {
            return_val->push_back(w);
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
                return_val->push_back(w);
            }

            for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            {
                return_val->push_back(w);
            }
        }
    }

    return return_val;
}

inline void _epmem_process_buffered_wme_list(agent* thisAgent, Symbol* state, wme_set& cue_wmes, symbol_triple_list& my_list, preference_list* epmem_wmes)
{
    if (my_list.empty())
    {
        return;
    }

    instantiation* inst = make_architectural_instantiation_for_memory_system(thisAgent, state, &cue_wmes, &my_list, false);

    for (preference* pref = inst->preferences_generated; pref;)
    {
        // add the preference to temporary memory
        if (add_preference_to_tm(thisAgent, pref))
        {
            // add to the list of preferences to be removed
            // when the goal is removed
            insert_at_head_of_dll(state->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
            pref->on_goal_list = true;

            if (epmem_wmes)
            {
                // if this is a meta wme, then it is completely local
                // to the state and thus we will manually remove it
                // (via preference removal) when the time comes
                epmem_wmes->push_back(pref);
            }
        }
        else
        {
			if (pref->reference_count == 0)
			{
				preference* previous = pref;
				pref = pref->inst_next;
				possibly_deallocate_preference_and_clones(thisAgent, previous, true);
				continue;
			}
        }

		pref = pref->inst_next;
    }

    if (!epmem_wmes)
    {
        instantiation* my_justification_list = NIL;
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
                    if (add_preference_to_tm(thisAgent, just_pref))
                    {
                        if (wma_enabled(thisAgent))
                        {
                            wma_activate_wmes_in_pref(thisAgent, just_pref);
                        }
                    }
                    else
                    {
                        preference_add_ref(just_pref);
                        preference_remove_ref(thisAgent, just_pref);
                    }
                }
            }
        }
    }
}

inline void epmem_process_buffered_wmes(agent* thisAgent, Symbol* state, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes)
{
    _epmem_process_buffered_wme_list(thisAgent, state, cue_wmes, meta_wmes, state->id->epmem_info->epmem_wmes);
    _epmem_process_buffered_wme_list(thisAgent, state, cue_wmes, retrieval_wmes, NULL);
}

inline void epmem_buffer_add_wme(agent* thisAgent, symbol_triple_list& my_list, Symbol* id, Symbol* attr, Symbol* value)
{
    my_list.push_back(new symbol_triple(id, attr, value));

    thisAgent->symbolManager->symbol_add_ref(id);
    thisAgent->symbolManager->symbol_add_ref(attr);
    thisAgent->symbolManager->symbol_add_ref(value);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Variable Functions (epmem::var)
//
// Variables are key-value pairs stored in the database
// that are necessary to maintain a store between
// multiple runs of Soar.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_get_variable
 * Author       : Nate Derbinsky
 * Notes        : Gets an EpMem variable from the database
 **************************************************************************/
bool epmem_get_variable(agent* thisAgent, epmem_variable_key variable_id, int64_t* variable_value)
{
    soar_module::exec_result status;
    soar_module::sqlite_statement* var_get = thisAgent->EpMem->epmem_stmts_common->var_get;

    var_get->bind_int(1, variable_id);
    status = var_get->execute();

    if (status == soar_module::row)
    {
        (*variable_value) = var_get->column_int(0);
    }

    var_get->reinitialize();

    return (status == soar_module::row);
}

/***************************************************************************
 * Function     : epmem_set_variable
 * Author       : Nate Derbinsky
 * Notes        : Sets an EpMem variable in the database
 **************************************************************************/
void epmem_set_variable(agent* thisAgent, epmem_variable_key variable_id, int64_t variable_value)
{
    soar_module::sqlite_statement* var_set = thisAgent->EpMem->epmem_stmts_common->var_set;

    var_set->bind_int(1, variable_id);
    var_set->bind_int(2, variable_value);

    var_set->execute(soar_module::op_reinit);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// RIT Functions (epmem::rit)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_rit_fork_node
 * Author       : Nate Derbinsky
 * Notes        : Implements the forkNode function of RIT
 **************************************************************************/
int64_t epmem_rit_fork_node(int64_t lower, int64_t upper, bool /*bounds_offset*/, int64_t* step_return, epmem_rit_state* rit_state)
{
    // never called
    /*if ( !bounds_offset )
      {
      int64_t offset = rit_state->offset.stat->get_value();

      lower = ( lower - offset );
      upper = ( upper - offset );
      }*/

    // descend the tree down to the fork node
    int64_t node = EPMEM_RIT_ROOT;
    if (upper < EPMEM_RIT_ROOT)
    {
        node = rit_state->leftroot.stat->get_value();
    }
    else if (lower > EPMEM_RIT_ROOT)
    {
        node = rit_state->rightroot.stat->get_value();
    }

    int64_t step;
    for (step = (((node >= 0) ? (node) : (-1 * node)) / 2); step >= 1; step /= 2)
    {
        if (upper < node)
        {
            node -= step;
        }
        else if (node < lower)
        {
            node += step;
        }
        else
        {
            break;
        }
    }

    // never used
    // if ( step_return != NULL )
    {
        (*step_return) = step;
    }

    return node;
}

/***************************************************************************
 * Function     : epmem_rit_clear_left_right
 * Author       : Nate Derbinsky
 * Notes        : Clears the left/right relations populated during prep
 **************************************************************************/
void epmem_rit_clear_left_right(agent* thisAgent)
{
    thisAgent->EpMem->epmem_stmts_common->rit_truncate_left->execute(soar_module::op_reinit);
    thisAgent->EpMem->epmem_stmts_common->rit_truncate_right->execute(soar_module::op_reinit);
}

/***************************************************************************
 * Function     : epmem_rit_add_left
 * Author       : Nate Derbinsky
 * Notes        : Adds a range to the left relation
 **************************************************************************/
void epmem_rit_add_left(agent* thisAgent, epmem_time_id min, epmem_time_id max)
{
    thisAgent->EpMem->epmem_stmts_common->rit_add_left->bind_int(1, min);
    thisAgent->EpMem->epmem_stmts_common->rit_add_left->bind_int(2, max);
    thisAgent->EpMem->epmem_stmts_common->rit_add_left->execute(soar_module::op_reinit);
}

/***************************************************************************
 * Function     : epmem_rit_add_right
 * Author       : Nate Derbinsky
 * Notes        : Adds a node to the to the right relation
 **************************************************************************/
void epmem_rit_add_right(agent* thisAgent, epmem_time_id id)
{
    thisAgent->EpMem->epmem_stmts_common->rit_add_right->bind_int(1, id);
    thisAgent->EpMem->epmem_stmts_common->rit_add_right->execute(soar_module::op_reinit);
}

/***************************************************************************
 * Function     : epmem_rit_prep_left_right
 * Author       : Nate Derbinsky
 * Notes        : Implements the computational components of the RIT
 *                query algorithm
 **************************************************************************/
void epmem_rit_prep_left_right(agent* thisAgent, int64_t lower, int64_t upper, epmem_rit_state* rit_state)
{
    ////////////////////////////////////////////////////////////////////////////
    rit_state->timer->start();
    ////////////////////////////////////////////////////////////////////////////

    int64_t offset = rit_state->offset.stat->get_value();
    int64_t node, step;
    int64_t left_node, left_step;
    int64_t right_node, right_step;

    lower = (lower - offset);
    upper = (upper - offset);

    // auto add good range
    epmem_rit_add_left(thisAgent, lower, upper);

    // go to fork
    node = EPMEM_RIT_ROOT;
    step = 0;
    if ((lower > node) || (upper < node))
    {
        if (lower > node)
        {
            node = rit_state->rightroot.stat->get_value();
            epmem_rit_add_left(thisAgent, EPMEM_RIT_ROOT, EPMEM_RIT_ROOT);
        }
        else
        {
            node = rit_state->leftroot.stat->get_value();
            epmem_rit_add_right(thisAgent, EPMEM_RIT_ROOT);
        }

        for (step = (((node >= 0) ? (node) : (-1 * node)) / 2); step >= 1; step /= 2)
        {
            if (lower > node)
            {
                epmem_rit_add_left(thisAgent, node, node);
                node += step;
            }
            else if (upper < node)
            {
                epmem_rit_add_right(thisAgent, node);
                node -= step;
            }
            else
            {
                break;
            }
        }
    }

    // go left
    left_node = node - step;
    for (left_step = (step / 2); left_step >= 1; left_step /= 2)
    {
        if (lower == left_node)
        {
            break;
        }
        else if (lower > left_node)
        {
            epmem_rit_add_left(thisAgent, left_node, left_node);
            left_node += left_step;
        }
        else
        {
            left_node -= left_step;
        }
    }

    // go right
    right_node = node + step;
    for (right_step = (step / 2); right_step >= 1; right_step /= 2)
    {
        if (upper == right_node)
        {
            break;
        }
        else if (upper < right_node)
        {
            epmem_rit_add_right(thisAgent, right_node);
            right_node -= right_step;
        }
        else
        {
            right_node += right_step;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    rit_state->timer->stop();
    ////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : epmem_rit_insert_interval
 * Author       : Nate Derbinsky
 * Notes        : Inserts an interval in the RIT
 **************************************************************************/
void epmem_rit_insert_interval(agent* thisAgent, int64_t lower, int64_t upper, epmem_node_id id, epmem_rit_state* rit_state, int64_t lti_id = 0)
{
    // initialize offset
    int64_t offset = rit_state->offset.stat->get_value();
    if (offset == EPMEM_RIT_OFFSET_INIT)
    {
        offset = lower;

        // update database
        epmem_set_variable(thisAgent, rit_state->offset.var_key, offset);

        // update stat
        rit_state->offset.stat->set_value(offset);
    }

    // get node
    int64_t node;
    {
        int64_t left_root = rit_state->leftroot.stat->get_value();
        int64_t right_root = rit_state->rightroot.stat->get_value();
        int64_t min_step = rit_state->minstep.stat->get_value();

        // shift interval
        int64_t l = (lower - offset);
        int64_t u = (upper - offset);

        // update left_root
        if ((u < EPMEM_RIT_ROOT) && (l <= (2 * left_root)))
        {
            left_root = static_cast<int64_t>(pow(-2.0, floor(log(static_cast<double>(-l)) / EPMEM_LN_2)));

            // update database
            epmem_set_variable(thisAgent, rit_state->leftroot.var_key, left_root);

            // update stat
            rit_state->leftroot.stat->set_value(left_root);
        }

        // update right_root
        if ((l > EPMEM_RIT_ROOT) && (u >= (2 * right_root)))
        {
            right_root = static_cast<int64_t>(pow(2.0, floor(log(static_cast<double>(u)) / EPMEM_LN_2)));

            // update database
            epmem_set_variable(thisAgent, rit_state->rightroot.var_key, right_root);

            // update stat
            rit_state->rightroot.stat->set_value(right_root);
        }

        // update min_step
        int64_t step;
        node = epmem_rit_fork_node(l, u, true, &step, rit_state);

        if ((node != EPMEM_RIT_ROOT) && (step < min_step))
        {
            min_step = step;

            // update database
            epmem_set_variable(thisAgent, rit_state->minstep.var_key, min_step);

            // update stat
            rit_state->minstep.stat->set_value(min_step);
        }
    }

    // perform insert

    /*std::ostringstream temp;
    temp << "\nInserting element with id: ";
    temp << id;
    temp << ", and lti_id: ";
    temp << static_cast<int64_t>(lti_id);//lti_id;
    temp << ".\n";
    std::string temp2 = temp.str();
    thisAgent->outputManager->print(temp2.c_str());*/

    rit_state->add_query->bind_int(1, node);
    rit_state->add_query->bind_int(2, lower);
    rit_state->add_query->bind_int(3, upper);
    rit_state->add_query->bind_int(4, id);
    //if (lti_id)
    //A horrible error was occuring where instead of defaulting to null in the absense of a given
    //lti_id, another value was used, which ended up effectively assigning lti status to absurd things
    //when they were recorded into the history for episodic memory.
    {
        rit_state->add_query->bind_int(5, (lti_id));
    }
    rit_state->add_query->execute(soar_module::op_reinit);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Clean-Up Functions (epmem::clean)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void epmem_clear_transient_structures(agent* thisAgent)
{
    epmem_parent_id_pool::iterator p;
    epmem_hashed_id_pool::iterator p_p;

    // de-allocate statement pools
    {
        int j, k, m;

        for (j = EPMEM_RIT_STATE_NODE; j <= EPMEM_RIT_STATE_EDGE; j++)
        {
            for (k = 0; k <= 1; k++)
            {
                delete thisAgent->EpMem->epmem_stmts_graph->pool_find_edge_queries[ j ][ k ];
            }
        }

        for (j = EPMEM_RIT_STATE_NODE; j <= EPMEM_RIT_STATE_EDGE; j++)
        {
            for (k = EPMEM_RANGE_START; k <= EPMEM_RANGE_END; k++)
            {
                for (m = EPMEM_RANGE_EP; m <= EPMEM_RANGE_POINT; m++)
                {
                    delete thisAgent->EpMem->epmem_stmts_graph->pool_find_interval_queries[ j ][ k ][ m ];
                }
            }
        }

        delete thisAgent->EpMem->epmem_stmts_graph->pool_dummy;
    }

    // de-allocate statements
    delete thisAgent->EpMem->epmem_stmts_common;
    delete thisAgent->EpMem->epmem_stmts_graph;

    // de-allocate id repository
    for (p = thisAgent->EpMem->epmem_id_repository->begin(); p != thisAgent->EpMem->epmem_id_repository->end(); p++)
    {
        for (p_p = p->second->begin(); p_p != p->second->end(); p_p++)
        {
            delete p_p->second;
        }

        delete p->second;
    }
    thisAgent->EpMem->epmem_id_repository->clear();
    thisAgent->EpMem->epmem_id_replacement->clear();

    // de-allocate id ref counts
    for (epmem_id_ref_counter::iterator rf_it = thisAgent->EpMem->epmem_id_ref_counts->begin(); rf_it != thisAgent->EpMem->epmem_id_ref_counts->end(); rf_it++)
    {
        delete rf_it->second;
    }
    thisAgent->EpMem->epmem_id_ref_counts->clear();
    thisAgent->EpMem->epmem_wme_adds->clear();

}

/***************************************************************************
 * Function     : epmem_close
 * Author       : Nate Derbinsky
 * Notes        : Performs cleanup operations when the database needs
 *                to be closed (end soar, manual close, etc)
 **************************************************************************/
void epmem_close(agent* thisAgent)
{
    if (thisAgent->EpMem->epmem_db->get_status() == soar_module::connected)
    {
        print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "Closing episodic memory database %s.\n", thisAgent->EpMem->epmem_params->path->get_value());
        // if lazy, commit
        if (thisAgent->EpMem->epmem_params->lazy_commit->get_value() == on)
        {
            thisAgent->EpMem->epmem_stmts_common->commit->execute(soar_module::op_reinit);
        }

        epmem_clear_transient_structures(thisAgent);

        // close the database
        thisAgent->EpMem->epmem_db->disconnect();
    }
}

void epmem_attach(agent* thisAgent)
{
    if (thisAgent->EpMem->epmem_db->get_status() == soar_module::disconnected)
    {
        epmem_init_db(thisAgent);
    }
}

/**
 * @name    epmem_reinit
 * @param   thisAgent
 * @brief   The function closes and then intializes the episodic memory
 *          database.  All data structures should be cleaned up and
 *          re-initialized properly, so this can be used for other database
 *          setting changes
 */
void epmem_reinit_cmd(agent* thisAgent)
{
    epmem_close(thisAgent);
    epmem_init_db(thisAgent);
}


void epmem_reinit(agent* thisAgent)
{
    if (thisAgent->EpMem->epmem_db->get_status() == soar_module::connected)
    {
        if ((thisAgent->EpMem->epmem_params->database->get_value() == epmem_param_container::memory))
        {
            if (thisAgent->EpMem->epmem_params->append_db->get_value() == off)
            {
                print_sysparam_trace(thisAgent, 0, "Episodic memory re-initializing.\n");
            }
            else
            {
                print_sysparam_trace(thisAgent, 0, "Note: Episodic memory can currently only append to an an on-disk database.  Ignoring append = on.\n");
                print_sysparam_trace(thisAgent, 0, "Episodic memory re-initializing.\n");
            }
        }
        else
        {
            print_sysparam_trace(thisAgent, 0, "Episodic memory re-initializing.\n");
        }
        epmem_close(thisAgent);
    }
}
/***************************************************************************
 * Function     : epmem_clear_result
 * Author       : Nate Derbinsky
 * Notes        : Removes any WMEs produced by EpMem resulting from
 *                a command
 **************************************************************************/
void epmem_clear_result(agent* thisAgent, Symbol* state)
{
    preference* pref;

    while (!state->id->epmem_info->epmem_wmes->empty())
    {
        pref = state->id->epmem_info->epmem_wmes->back();
        state->id->epmem_info->epmem_wmes->pop_back();

        if (pref->in_tm)
        {
            remove_preference_from_tm(thisAgent, pref);
        }
    }
}

/***************************************************************************
 * Function     : epmem_reset
 * Author       : Nate Derbinsky
 * Notes        : Performs cleanup when a state is removed
 **************************************************************************/
void epmem_reset(agent* thisAgent, Symbol* state)
{
    if (state == NULL)
    {
        state = thisAgent->top_goal;
    }

    while (state)
    {
        epmem_data* data = state->id->epmem_info;

        data->last_ol_time = 0;

        data->last_cmd_time = 0;
        data->last_cmd_count = 0;

        data->last_memory = EPMEM_MEMID_NONE;

        // this will be called after prefs from goal are already removed,
        // so just clear out result stack
        data->epmem_wmes->clear();

        state = state->id->lower_goal;
    }
}

void epmem_switch_db_mode(agent* thisAgent, std::string& buf, bool readonly)
{
    print_sysparam_trace(thisAgent, 0, buf.c_str());
    thisAgent->EpMem->epmem_db->disconnect();
    thisAgent->EpMem->epmem_params->database->set_value(epmem_param_container::memory);
    epmem_init_db(thisAgent, readonly);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Initialization Functions (epmem::init)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_init_db
 * Author       : Nate Derbinsky
 * Notes        : Opens the SQLite database and performs all
 *                initialization required for the current mode
 *
 *                The readonly param should only be used in
 *                experimentation where you don't want to alter
 *                previous database state.
 **************************************************************************/

void epmem_init_db(agent* thisAgent, bool readonly)
{
    if (thisAgent->EpMem->epmem_db->get_status() != soar_module::disconnected)
    {
        print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "Cannot initialize episodic memory database.  It is already connected!");
        return;
    }


    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->init->start();
    ////////////////////////////////////////////////////////////////////////////

    const char* db_path;
    if (thisAgent->EpMem->epmem_params->database->get_value() == epmem_param_container::memory)
    {
        db_path = ":memory:";
        print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "Initializing episodic memory database in cpu memory.\n");
    }
    else
    {
        db_path = thisAgent->EpMem->epmem_params->path->get_value();
        print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "Initializing episodic memory database at %s\n", db_path);
    }

    // attempt connection
    thisAgent->EpMem->epmem_db->connect(db_path);

    if (thisAgent->EpMem->epmem_db->get_status() == soar_module::problem)
    {
        print_sysparam_trace(thisAgent, 0, "Episodic memory database error: %s\n", thisAgent->EpMem->epmem_db->get_errmsg());
    }
    else
    {
        epmem_time_id time_max;
        soar_module::sqlite_statement* temp_q = NULL;
        soar_module::sqlite_statement* temp_q2 = NULL;

        // If the database is on file, make sure the database contents use the current schema
        // If it does not, switch to memory-based database

        if (strcmp(db_path, ":memory:")) // Only worry about database version if writing to disk
        {
            bool switch_to_memory, sql_is_new;
            std::string schema_version, version_error_message;

            switch_to_memory = true;

            if (thisAgent->EpMem->epmem_db->sql_is_new_db(sql_is_new))
            {
                if (sql_is_new)
                {
                    switch_to_memory = false;
                    print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "...episodic memory database is new.\n");
                }
                else
                {
                    // Check if table exists already
                    temp_q = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (thisAgent->EpMem->epmem_db->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'epmem_schema'", schema_version))
                        {
                            if (schema_version != EPMEM_SCHEMA_VERSION)   // Incompatible version
                            {
                                version_error_message.assign("...Error:  Cannot load episodic memory database with schema version ");
                                version_error_message.append(schema_version.c_str());
                                version_error_message.append(".\n...Please convert old database or start a new database by "
                                                             "setting a new database file path.\n...Switching to memory-based database.\n");
                            }
                            else     // Version is OK
                            {
                                print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "...version of episodic memory database ok.\n");
                                switch_to_memory = false;
                            }

                        }
                        else     // Some sort of error reading version info from version database
                        {
                            version_error_message.assign("...Error:  Cannot read version number from file-based episodic memory database.\n"
                                                         "...Switching to memory-based database.\n");
                        }
                    }
                    else     // Non-empty database exists with no version table.  Probably schema 1.0
                    {
                        version_error_message.assign("...Error:  Cannot load an episodic memory database with an old schema version.\n...Please convert "
                                                     "old database or start a new database by setting a new database file path.\n...Switching "
                                                     "to memory-based database.\n");
                    }
                    delete temp_q;
                    temp_q = NULL;
                }
            }
            else
            {
                version_error_message.assign("...Error:  Cannot read database meta info from file-based episodic memory database.\n"
                                             "...Switching to memory-based database.\n");
            }
            if (switch_to_memory)
            {
                // Memory mode will be set on, database will be disconnected to and then init_db
                // will be called again to reinitialize database.
                epmem_switch_db_mode(thisAgent, version_error_message, readonly);
                return;
            }
        }
        // apply performance options
        {
            // page_size
            {
                switch (thisAgent->EpMem->epmem_params->page_size->get_value())
                {
                    case (epmem_param_container::page_1k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 1024");
                        break;

                    case (epmem_param_container::page_2k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 2048");
                        break;

                    case (epmem_param_container::page_4k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 4096");
                        break;

                    case (epmem_param_container::page_8k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 8192");
                        break;

                    case (epmem_param_container::page_16k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 16384");
                        break;

                    case (epmem_param_container::page_32k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 32768");
                        break;

                    case (epmem_param_container::page_64k):
                        thisAgent->EpMem->epmem_db->sql_execute("PRAGMA page_size = 65536");
                        break;
                }
            }

            // cache_size
            {
                std::string cache_sql("PRAGMA cache_size = ");
                char* str = thisAgent->EpMem->epmem_params->cache_size->get_cstring();
                cache_sql.append(str);
                free(str);
                str = NULL;

                thisAgent->EpMem->epmem_db->sql_execute(cache_sql.c_str());
            }

            // optimization
            if (thisAgent->EpMem->epmem_params->opt->get_value() == epmem_param_container::opt_speed)
            {
                // synchronous - don't wait for writes to complete (can corrupt the db in case unexpected crash during transaction)
                thisAgent->EpMem->epmem_db->sql_execute("PRAGMA synchronous = OFF");

                // journal_mode - no atomic transactions (can result in database corruption if crash during transaction)
                thisAgent->EpMem->epmem_db->sql_execute("PRAGMA journal_mode = OFF");

                // locking_mode - no one else can view the database after our first write
                thisAgent->EpMem->epmem_db->sql_execute("PRAGMA locking_mode = EXCLUSIVE");
            }
        }

        // point stuff
        epmem_time_id range_start;
        epmem_time_id time_last;

        // update validation count
        thisAgent->EpMem->epmem_validation++;

        // setup common structures/queries
        thisAgent->EpMem->epmem_stmts_common = new epmem_common_statement_container(thisAgent);
        thisAgent->EpMem->epmem_stmts_common->structure();
        thisAgent->EpMem->epmem_stmts_common->prepare();

        {
            // setup graph structures/queries
            thisAgent->EpMem->epmem_stmts_graph = new epmem_graph_statement_container(thisAgent);
            thisAgent->EpMem->epmem_stmts_graph->structure();
            thisAgent->EpMem->epmem_stmts_graph->prepare();

            // initialize range tracking
            thisAgent->EpMem->epmem_node_mins->clear();
            thisAgent->EpMem->epmem_node_maxes->clear();
            thisAgent->EpMem->epmem_node_removals->clear();

            thisAgent->EpMem->epmem_edge_mins->clear();
            thisAgent->EpMem->epmem_edge_maxes->clear();
            thisAgent->EpMem->epmem_edge_removals->clear();

            (*thisAgent->EpMem->epmem_id_repository)[ EPMEM_NODEID_ROOT ] = new epmem_hashed_id_pool;
            {
#ifdef USE_MEM_POOL_ALLOCATORS
                epmem_wme_set* wms_temp = new epmem_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >());
#else
                epmem_wme_set* wms_temp = new epmem_wme_set();
#endif

                // voigtjr: Cast to wme* is necessary for compilation in VS10
                // Without it, it picks insert(int) instead of insert(wme*) and does not compile.
                wms_temp->insert(static_cast<wme*>(NULL));

                (*thisAgent->EpMem->epmem_id_ref_counts)[ EPMEM_NODEID_ROOT ] = wms_temp;
            }

            // initialize time
            thisAgent->EpMem->epmem_stats->time->set_value(1);

            // initialize next_id
            thisAgent->EpMem->epmem_stats->next_id->set_value(1);
            {
                int64_t stored_id = NIL;
                if (epmem_get_variable(thisAgent, var_next_id, &stored_id))
                {
                    thisAgent->EpMem->epmem_stats->next_id->set_value(stored_id);
                }
                else
                {
                    epmem_set_variable(thisAgent, var_next_id, thisAgent->EpMem->epmem_stats->next_id->get_value());
                }
            }

            // initialize rit state
            for (int i = EPMEM_RIT_STATE_NODE; i <= EPMEM_RIT_STATE_EDGE; i++)
            {
                thisAgent->EpMem->epmem_rit_state_graph[ i ].offset.stat->set_value(EPMEM_RIT_OFFSET_INIT);
                thisAgent->EpMem->epmem_rit_state_graph[ i ].leftroot.stat->set_value(0);
                thisAgent->EpMem->epmem_rit_state_graph[ i ].rightroot.stat->set_value(1);
                thisAgent->EpMem->epmem_rit_state_graph[ i ].minstep.stat->set_value(LONG_MAX);
            }
            thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ].add_query = thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_range;
            thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ].add_query = thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_range;

            ////

            // get/set RIT variables
            {
                int64_t var_val = NIL;

                for (int i = EPMEM_RIT_STATE_NODE; i <= EPMEM_RIT_STATE_EDGE; i++)
                {
                    // offset
                    if (epmem_get_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].offset.var_key, &var_val))
                    {
                        thisAgent->EpMem->epmem_rit_state_graph[ i ].offset.stat->set_value(var_val);
                    }
                    else
                    {
                        epmem_set_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].offset.var_key, thisAgent->EpMem->epmem_rit_state_graph[ i ].offset.stat->get_value());
                    }

                    // leftroot
                    if (epmem_get_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].leftroot.var_key, &var_val))
                    {
                        thisAgent->EpMem->epmem_rit_state_graph[ i ].leftroot.stat->set_value(var_val);
                    }
                    else
                    {
                        epmem_set_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].leftroot.var_key, thisAgent->EpMem->epmem_rit_state_graph[ i ].leftroot.stat->get_value());
                    }

                    // rightroot
                    if (epmem_get_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].rightroot.var_key, &var_val))
                    {
                        thisAgent->EpMem->epmem_rit_state_graph[ i ].rightroot.stat->set_value(var_val);
                    }
                    else
                    {
                        epmem_set_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].rightroot.var_key, thisAgent->EpMem->epmem_rit_state_graph[ i ].rightroot.stat->get_value());
                    }

                    // minstep
                    if (epmem_get_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].minstep.var_key, &var_val))
                    {
                        thisAgent->EpMem->epmem_rit_state_graph[ i ].minstep.stat->set_value(var_val);
                    }
                    else
                    {
                        epmem_set_variable(thisAgent, thisAgent->EpMem->epmem_rit_state_graph[ i ].minstep.var_key, thisAgent->EpMem->epmem_rit_state_graph[ i ].minstep.stat->get_value());
                    }
                }
            }

            ////

            // get max time
            {
                temp_q = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, "SELECT MAX(episode_id) FROM epmem_episodes");
                temp_q->prepare();
                if (temp_q->execute() == soar_module::row)
                {
                    thisAgent->EpMem->epmem_stats->time->set_value(temp_q->column_int(0) + 1);
                }

                delete temp_q;
                temp_q = NULL;
            }
            time_max = thisAgent->EpMem->epmem_stats->time->get_value();

            // insert non-NOW intervals for all current NOW's
            // remove NOW's
            if (!readonly)
            {
                time_last = (time_max - 1);

                const char* now_select[] = { "SELECT wc_id,start_episode_id FROM epmem_wmes_constant_now", "SELECT wi_id,start_episode_id,lti_id FROM epmem_wmes_identifier_now" };
                soar_module::sqlite_statement* now_add[] = { thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_point, thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_point };
                const char* now_delete[] = { "DELETE FROM epmem_wmes_constant_now", "DELETE FROM epmem_wmes_identifier_now" };

                for (int i = EPMEM_RIT_STATE_NODE; i <= EPMEM_RIT_STATE_EDGE; i++)
                {
                    temp_q = now_add[i];
                    temp_q->bind_int(2, time_last);

                    temp_q2 = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, now_select[i]);
                    temp_q2->prepare();
                    while (temp_q2->execute() == soar_module::row)
                    {
                        range_start = temp_q2->column_int(1);

                        // point
                        if (range_start == time_last)
                        {
                            temp_q->bind_int(1, temp_q2->column_int(0));
                            if (i == EPMEM_RIT_STATE_EDGE)
                            {
                                temp_q->bind_int(3, temp_q2->column_int(2));
                            }
                            temp_q->execute(soar_module::op_reinit);
                        }
                        else
                        {
                            epmem_rit_insert_interval(thisAgent, range_start, time_last, temp_q2->column_int(0), &(thisAgent->EpMem->epmem_rit_state_graph[i]), temp_q2->column_int(2));
                        }

                        if (i == EPMEM_RIT_STATE_EDGE)
                        {
                            thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(1, time_last);
                            thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(2, temp_q2->column_int(0));
                            thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->execute(soar_module::op_reinit);
                        }
                    }
                    delete temp_q2;
                    temp_q2 = NULL;
                    temp_q = NULL;


                    // remove all NOW intervals
                    temp_q = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, now_delete[i]);
                    temp_q->prepare();
                    temp_q->execute();
                    delete temp_q;
                    temp_q = NULL;
                }
            }

            // get max id + max list
            {
                const char* minmax_select[] = { "SELECT MAX(wc_id) FROM epmem_wmes_constant", "SELECT MAX(wi_id) FROM epmem_wmes_identifier" };
                std::vector<bool>* minmax_max[] = { thisAgent->EpMem->epmem_node_maxes, thisAgent->EpMem->epmem_edge_maxes };
                std::vector<epmem_time_id>* minmax_min[] = { thisAgent->EpMem->epmem_node_mins, thisAgent->EpMem->epmem_edge_mins };

                for (int i = EPMEM_RIT_STATE_NODE; i <= EPMEM_RIT_STATE_EDGE; i++)
                {
                    temp_q = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, minmax_select[i]);
                    temp_q->prepare();
                    temp_q->execute();
                    if (temp_q->column_type(0) != soar_module::null_t)
                    {
                        std::vector<bool>::size_type num_ids = static_cast<size_t>(temp_q->column_int(0));

                        minmax_max[i]->resize(num_ids, true);
                        minmax_min[i]->resize(num_ids, time_max);
                    }

                    delete temp_q;
                    temp_q = NULL;
                }
            }

            // get id pools
            {
                epmem_node_id parent_n_id;
                int64_t attribute_s_id;
                epmem_node_id child_n_id;
                epmem_node_id wi_id;

                epmem_hashed_id_pool** hp;
                epmem_id_pool** ip;

                temp_q = new soar_module::sqlite_statement(thisAgent->EpMem->epmem_db, "SELECT parent_n_id, attribute_s_id, child_n_id, wi_id FROM epmem_wmes_identifier");
                temp_q->prepare();

                while (temp_q->execute() == soar_module::row)
                {
                    parent_n_id = temp_q->column_int(0);
                    attribute_s_id = temp_q->column_int(1);
                    child_n_id = temp_q->column_int(2);
                    wi_id = temp_q->column_int(3);

                    hp = & (*thisAgent->EpMem->epmem_id_repository)[ parent_n_id ];
                    if (!(*hp))
                    {
                        (*hp) = new epmem_hashed_id_pool;
                    }

                    ip = & (*(*hp))[ attribute_s_id ];
                    if (!(*ip))
                    {
                        (*ip) = new epmem_id_pool;
                    }

                    (*ip)->push_front(std::make_pair(child_n_id, wi_id));

                    hp = & (*thisAgent->EpMem->epmem_id_repository)[ child_n_id ];
                    if (!(*hp))
                    {
                        (*hp) = new epmem_hashed_id_pool;
                    }
                }

                delete temp_q;
                temp_q = NULL;
            }

            // at init, top-state is considered the only known identifier
            thisAgent->top_goal->id->epmem_id = EPMEM_NODEID_ROOT;
            thisAgent->top_goal->id->epmem_valid = thisAgent->EpMem->epmem_validation;

            // capture augmentations of top-state as the sole set of adds,
            // which catches up to what would have been incremental encoding
            // to this point
            {
                thisAgent->EpMem->epmem_wme_adds->insert(thisAgent->top_state);
            }
        }

        // if lazy commit, then we encapsulate the entire lifetime of the agent in a single transaction
        if (thisAgent->EpMem->epmem_params->lazy_commit->get_value() == on)
        {
            thisAgent->EpMem->epmem_stmts_common->begin->execute(soar_module::op_reinit);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->init->stop();
    ////////////////////////////////////////////////////////////////////////////
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Storage Functions (epmem::storage)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/* **************************************************************************

                         _epmem_store_level

   This function process the addition of "one level" of wme's rooted at
   the n_id specified by parent_id into the working memory graph.  It does
   not process any temporal information.

   - w_b is the list of wme's it will process (until it hits w_e)

   - can add syms and ids of augmentations to parent_syms and parent_ids
   queues so that they can be processed during the next "level"

   - The identifier symbol of each wme isn't set and is ignored. parent_id
     is used to specify the root.


     three cases for sharing ids amongst identifiers in two passes:
     1. value known in phase one (try reservation)
     2. value unknown in phase one, but known at phase two (try assignment adhering to constraint)
     3. value unknown in phase one/two (if anything is left, unconstrained assignment)

************************************************************************** */


inline void _epmem_store_level(agent* thisAgent,
                               std::queue< Symbol* >& parent_syms,
                               std::queue< epmem_node_id >& parent_ids,
                               tc_number tc,
                               epmem_wme_list::iterator w_b,
                               epmem_wme_list::iterator w_e,
                               epmem_node_id parent_id,
                               epmem_time_id time_counter,
                               std::map< wme*, epmem_id_reservation* >& id_reservations,
                               std::set< Symbol* >& new_identifiers,
                               std::queue< epmem_node_id >& epmem_node,
                               std::queue<std::pair<epmem_node_id,int64_t>>& epmem_edge)
{
    epmem_wme_list::iterator w_p;
    bool value_known_apriori = false;

    // temporal hash
    epmem_hash_id my_hash;  // attribute
    epmem_hash_id my_hash2; // value

    // id repository
    epmem_id_pool** my_id_repo;
    epmem_id_pool* my_id_repo2;
    epmem_id_pool::iterator pool_p;
    std::map<wme*, epmem_id_reservation*>::iterator r_p;
    epmem_id_reservation* new_id_reservation;

    // identifier recursion
    epmem_wme_list::iterator w_p2;

#ifdef DEBUG_EPMEM_WME_ADD
    fprintf(stderr, "==================================================\nDEBUG _epmem_store_level called for parent_id %d\n==================================================\n", (unsigned int) parent_id);
#endif

    // find WME ID for WMEs whose value is an identifier and has a known epmem id (prevents ordering issues with unknown children)
    for (w_p = w_b; w_p != w_e; w_p++)
    {
//      #ifdef DEBUG_EPMEM_WME_ADD
//      fprintf(stderr, "DEBUG epmem.2132: _epmem_store_level processing wme (types: %d %d %d)\n",
//              (*w_p)->id->symbol_type,  (*w_p)->attr->var->symbol_type,  (*w_p)->value->symbol_type);
//      #endif
        // skip over WMEs already in the system
        if (((*w_p)->epmem_id != EPMEM_NODEID_BAD) && ((*w_p)->epmem_valid == thisAgent->EpMem->epmem_validation))
        {
            continue;
        }
        /* Not sure why this is excluding LTIs or whether that makes sense any more. */
        if (((*w_p)->value->symbol_type     == IDENTIFIER_SYMBOL_TYPE) &&
           (((*w_p)->value->id->epmem_id    != EPMEM_NODEID_BAD) &&
            ((*w_p)->value->id->epmem_valid == thisAgent->EpMem->epmem_validation)))
        {
            // prevent exclusions from being recorded
            if (thisAgent->EpMem->epmem_params->exclusions->in_set((*w_p)->attr))
            {
                continue;
            }

#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "--------------------------------------------\nReserving WME: %d ^%s %s\n",
                    (unsigned int) parent_id, symbol_to_string(thisAgent, (*w_p)->attr, true, NIL, 0), symbol_to_string(thisAgent, (*w_p)->value, true, NIL, 0));
#endif

            // if still here, create reservation (case 1)
#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   wme is known.  creating reservation.\n");
#endif
            new_id_reservation = new epmem_id_reservation;
            new_id_reservation->my_id = EPMEM_NODEID_BAD;
            new_id_reservation->my_pool = NULL;

            if ((*w_p)->acceptable)
            {
                new_id_reservation->my_hash = EPMEM_HASH_ACCEPTABLE;
            }
            else
            {
                new_id_reservation->my_hash = epmem_temporal_hash(thisAgent, (*w_p)->attr);
            }

            // try to find appropriate reservation
            my_id_repo = & (*(*thisAgent->EpMem->epmem_id_repository)[ parent_id ])[ new_id_reservation->my_hash ];
            if ((*my_id_repo))
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   id repository exists.  reserving id\n");
#endif
                for (pool_p = (*my_id_repo)->begin(); pool_p != (*my_id_repo)->end(); pool_p++)
                {
                    if (pool_p->first == (*w_p)->value->id->epmem_id)
                    {
#ifdef DEBUG_EPMEM_WME_ADD
                        fprintf(stderr, "   reserved id %d\n", (unsigned int) pool_p->second);
#endif
                        new_id_reservation->my_id = pool_p->second;
                        (*my_id_repo)->erase(pool_p);
                        break;
                    }
                }
            }
            else
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   no id repository found.  creating new id repository.\n");
#endif
                // add repository
                (*my_id_repo) = new epmem_id_pool;
            }

            new_id_reservation->my_pool = (*my_id_repo);
            id_reservations[(*w_p) ] = new_id_reservation;
            new_id_reservation = NULL;
        }
    }

    for (w_p = w_b; w_p != w_e; w_p++)
    {
#ifdef DEBUG_EPMEM_WME_ADD
        fprintf(stderr, "--------------------------------------------\nProcessing WME: %d ^%s %s\n",
                (unsigned int) parent_id, symbol_to_string(thisAgent, (*w_p)->attr, true, NIL, 0), symbol_to_string(thisAgent, (*w_p)->value, true, NIL, 0));
#endif
        // skip over WMEs already in the system
        if (((*w_p)->epmem_id != EPMEM_NODEID_BAD) && ((*w_p)->epmem_valid == thisAgent->EpMem->epmem_validation))
        {
#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   WME already in system with id %d.\n", (unsigned int)(*w_p)->epmem_id);
#endif
            continue;
        }

        // prevent exclusions from being recorded
        if (thisAgent->EpMem->epmem_params->exclusions->in_set((*w_p)->attr))
        {
#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   WME excluded.  Skipping.\n");
#endif
            continue;
        }

        if ((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   WME value is IDENTIFIER.\n");
#endif
            (*w_p)->epmem_valid = thisAgent->EpMem->epmem_validation;
            (*w_p)->epmem_id = EPMEM_NODEID_BAD;

            my_hash = NIL;
            my_id_repo2 = NIL;

            // if the value already has an epmem_id, the WME ID would have already been assigned above (ie. the epmem_id of the VALUE is KNOWN APRIORI [sic])
            // however, it's also possible that the value is known but no WME ID is given (eg. (<s> ^foo <a> ^bar <a>)); this is case 2
            value_known_apriori = (((*w_p)->value->id->epmem_id != EPMEM_NODEID_BAD) && ((*w_p)->value->id->epmem_valid == thisAgent->EpMem->epmem_validation));

            // if long-term identifier as value, special processing
            if ((*w_p)->value->id->LTI_ID && ((*w_p)->value->id->LTI_epmem_valid != thisAgent->EpMem->epmem_validation) && ((*w_p)->value->id->epmem_id != EPMEM_NODEID_BAD))
            {
                // Update the node database with the new lti_id
                thisAgent->EpMem->epmem_stmts_graph->update_node->bind_int(2, (*w_p)->value->id->LTI_ID);
                thisAgent->EpMem->epmem_stmts_graph->update_node->bind_int(1, (*w_p)->value->id->epmem_id);
                thisAgent->EpMem->epmem_stmts_graph->update_node->execute(soar_module::op_reinit);
                (*w_p)->value->id->LTI_epmem_valid = thisAgent->EpMem->epmem_validation;
            }// Steven - This is an important point - It's where we know we're adding an lti for which there wasn't already a known epmem id correspondence assigned to it.
            {
                // in the case of a known value, we already have a reservation (case 1)
                if (value_known_apriori)
                {
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   WME is known.  Looking for reservation.\n");
#endif
                    r_p = id_reservations.find((*w_p));

                    if (r_p != id_reservations.end())
                    {
#ifdef DEBUG_EPMEM_WME_ADD
                        fprintf(stderr, "   Found existing reservation.\n");
#endif
                        // restore reservation info
                        my_hash = r_p->second->my_hash;
                        my_id_repo2 = r_p->second->my_pool;

                        if (r_p->second->my_id != EPMEM_NODEID_BAD)
                        {
                            (*w_p)->epmem_id = r_p->second->my_id;
                            (*thisAgent->EpMem->epmem_id_replacement)[(*w_p)->epmem_id ] = my_id_repo2;
#ifdef DEBUG_EPMEM_WME_ADD
                            fprintf(stderr, "   Assigning id from existing pool: %d\n", (unsigned int)(*w_p)->epmem_id);
#endif
                        }

                        // delete reservation and map entry
                        delete r_p->second;
                        id_reservations.erase(r_p);
                    }
                    // OR a shared identifier at the same level, in which case we need an exact match (case 2)
                    else
                    {
#ifdef DEBUG_EPMEM_WME_ADD
                        fprintf(stderr, "   No reservation found.  Looking for shared identifier at same level.\n");
#endif
                        // get temporal hash
                        if ((*w_p)->acceptable)
                        {
                            my_hash = EPMEM_HASH_ACCEPTABLE;
                        }
                        else
                        {
                            my_hash = epmem_temporal_hash(thisAgent, (*w_p)->attr);
                        }

                        // try to get an id that matches new information
                        my_id_repo = & (*(*thisAgent->EpMem->epmem_id_repository)[ parent_id ])[ my_hash ];
                        if ((*my_id_repo))
                        {
                            if (!(*my_id_repo)->empty())
                            {
                                for (pool_p = (*my_id_repo)->begin(); pool_p != (*my_id_repo)->end(); pool_p++)
                                {
                                    if (pool_p->first == (*w_p)->value->id->epmem_id)
                                    {
                                        (*w_p)->epmem_id = pool_p->second;
                                        (*my_id_repo)->erase(pool_p);
                                        (*thisAgent->EpMem->epmem_id_replacement)[(*w_p)->epmem_id ] = (*my_id_repo);
#ifdef DEBUG_EPMEM_WME_ADD
                                        fprintf(stderr, "   Assigning id from existing pool: %d\n", (unsigned int)(*w_p)->epmem_id);
#endif
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
#ifdef DEBUG_EPMEM_WME_ADD
                            fprintf(stderr, "   No pool.  Creating a new one.\n");
#endif
                            // add repository
                            (*my_id_repo) = new epmem_id_pool;
                        }

                        // keep the address for later (used if w->epmem_id was not assigned)
                        my_id_repo2 = (*my_id_repo);
                    }
                }
                // case 3
                else
                {
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   WME is unknown.  Looking for id in repo pool.\n");
#endif
                    // UNKNOWN identifier
                    new_identifiers.insert((*w_p)->value);

                    // get temporal hash
                    if ((*w_p)->acceptable)
                    {
                        my_hash = EPMEM_HASH_ACCEPTABLE;
                    }
                    else
                    {
                        my_hash = epmem_temporal_hash(thisAgent, (*w_p)->attr);
                    }

                    // try to find node
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   Trying to find node with parent=%d and attr=%d\n", (unsigned int) parent_id, (unsigned int) my_hash);
#endif
                    my_id_repo = & (*(*thisAgent->EpMem->epmem_id_repository)[ parent_id ])[ my_hash ];
                    if ((*my_id_repo))
                    {
                        // if something leftover, try to use it
                        if (!(*my_id_repo)->empty())
                        {
                            for (pool_p = (*my_id_repo)->begin(); pool_p != (*my_id_repo)->end(); pool_p++)
                            {
                                // the ref set for this epmem_id may not be there if the pools were regenerated from a previous DB
                                // a non-existant ref set is the equivalent of a ref count of 0 (ie. an empty ref set)
                                // so we allow the identifier from the pool to be used
                                if ((thisAgent->EpMem->epmem_id_ref_counts->count(pool_p->first) == 0) ||
                                        ((*thisAgent->EpMem->epmem_id_ref_counts)[ pool_p->first ]->empty()))

                                {
                                    (*w_p)->epmem_id = pool_p->second;
                                    (*w_p)->value->id->epmem_id = pool_p->first;
#ifdef DEBUG_EPMEM_WME_ADD
                                    fprintf(stderr, "   Found unused id. Setting wme id for VALUE to %d\n", (unsigned int)(*w_p)->value->id->epmem_id);
#endif
                                    (*w_p)->value->id->epmem_valid = thisAgent->EpMem->epmem_validation;
                                    (*my_id_repo)->erase(pool_p);
                                    (*thisAgent->EpMem->epmem_id_replacement)[(*w_p)->epmem_id ] = (*my_id_repo);

#ifdef DEBUG_EPMEM_WME_ADD
                                    fprintf(stderr, "   Assigning id from existing pool %d.\n", (unsigned int)(*w_p)->epmem_id);
#endif
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef DEBUG_EPMEM_WME_ADD
                        fprintf(stderr, "   No pool.  Creating a new one.\n");
#endif
                        // add repository
                        (*my_id_repo) = new epmem_id_pool;
                    }

                    // keep the address for later (used if w->epmem_id was not assgined)
                    my_id_repo2 = (*my_id_repo);
                }
            }

            // add wme if no success above
            if ((*w_p)->epmem_id == EPMEM_NODEID_BAD)
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   No success, adding wme to database.");
#endif
                // can't use value_known_apriori, since value may have been assigned (lti, id repository via case 3)
                if (((*w_p)->value->id->epmem_id == EPMEM_NODEID_BAD) || ((*w_p)->value->id->epmem_valid != thisAgent->EpMem->epmem_validation))
                {
                    // update next id
                    (*w_p)->value->id->epmem_id = thisAgent->EpMem->epmem_stats->next_id->get_value();
                    (*w_p)->value->id->epmem_valid = thisAgent->EpMem->epmem_validation;
                    thisAgent->EpMem->epmem_stats->next_id->set_value((*w_p)->value->id->epmem_id + 1);
                    epmem_set_variable(thisAgent, var_next_id, (*w_p)->value->id->epmem_id + 1);

#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   Adding new n_id and setting wme id for VALUE to %d\n", (unsigned int)(*w_p)->value->id->epmem_id);
#endif
                    // Update the node database with the new n_id
                    thisAgent->EpMem->epmem_stmts_graph->update_node->bind_int(1, (*w_p)->value->id->epmem_id);
                    if (!(*w_p)->value->id->LTI_ID)
                    {
                        thisAgent->EpMem->epmem_stmts_graph->update_node->bind_int(2, 0);
                    } else {
                        thisAgent->EpMem->epmem_stmts_graph->update_node->bind_int(2, (*w_p)->value->id->LTI_ID);
                        (*w_p)->value->id->LTI_epmem_valid = thisAgent->EpMem->epmem_validation;
                    }
                    thisAgent->EpMem->epmem_stmts_graph->update_node->execute(soar_module::op_reinit);

                    // add repository for possible future children
                    (*thisAgent->EpMem->epmem_id_repository)[(*w_p)->value->id->epmem_id ] = new epmem_hashed_id_pool;

                    // add ref set
#ifdef USE_MEM_POOL_ALLOCATORS
                    (*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ] = new epmem_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >());
#else
                    (*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ] = new epmem_wme_set();
#endif
                }

                // insert (parent_n_id,attribute_s_id,child_n_id)
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   Performing database insertion: %d %d %d\n",
                        (unsigned int) parent_id, (unsigned int) my_hash, (unsigned int)(*w_p)->value->id->epmem_id);

                fprintf(stderr, "   Adding wme to epmem_wmes_identifier table.\n");
#endif
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier->bind_int(1, parent_id);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier->bind_int(2, my_hash);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier->bind_int(3, (*w_p)->value->id->epmem_id);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier->bind_int(4, LLONG_MAX);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier->execute(soar_module::op_reinit);

                (*w_p)->epmem_id = static_cast<epmem_node_id>(thisAgent->EpMem->epmem_db->last_insert_rowid());
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   Incrementing and setting wme id to %d\n", (unsigned int)(*w_p)->epmem_id);
#endif
                // replace the epmem_id and wme id in the right place
                (*thisAgent->EpMem->epmem_id_replacement)[(*w_p)->epmem_id ] = my_id_repo2;

                // new nodes definitely start
                epmem_edge.emplace((*w_p)->epmem_id,static_cast<int64_t>((*w_p)->value->id->is_lti() ? (*w_p)->value->id->LTI_ID : 0));
                thisAgent->EpMem->epmem_edge_mins->push_back(time_counter);
                thisAgent->EpMem->epmem_edge_maxes->push_back(false);
            }
            else
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   No success but already has id, so don't remove.\n");
#endif
                // definitely don't remove
                (*thisAgent->EpMem->epmem_edge_removals)[std::make_pair((*w_p)->epmem_id, static_cast<int64_t>((*w_p)->value->id->is_lti() ? (*w_p)->value->id->LTI_ID : 0)) ] = false;

                // we add ONLY if the last thing we did was remove
                if ((*thisAgent->EpMem->epmem_edge_maxes)[static_cast<size_t>((*w_p)->epmem_id - 1)])
                {
                    epmem_edge.emplace((*w_p)->epmem_id,static_cast<int64_t>((*w_p)->value->id->is_lti() ? (*w_p)->value->id->LTI_ID : 0));
                    (*thisAgent->EpMem->epmem_edge_maxes)[static_cast<size_t>((*w_p)->epmem_id - 1)] = false;
                }
            }

            // at this point we have successfully added a new wme
            // whose value is an identifier.  If the value was
            // unknown at the beginning of this episode, then we need
            // to update its ref count for each WME added (thereby catching
            // up with ref counts that would have been accumulated via wme adds)
            if (new_identifiers.find((*w_p)->value) != new_identifiers.end())
            {
                // because we could have bypassed the ref set before, we need to create it here
                if (thisAgent->EpMem->epmem_id_ref_counts->count((*w_p)->value->id->epmem_id) == 0)
                {
#ifdef USE_MEM_POOL_ALLOCATORS
                    (*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ] = new epmem_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >(thisAgent));
#else
                    (*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ] = new epmem_wme_set;
#endif
                }
                (*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ]->insert((*w_p));
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   increasing ref_count of value in %d %d %d; new ref_count is %d\n",
                        (unsigned int)(*w_p)->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, (*w_p)->attr), (unsigned int)(*w_p)->value->id->epmem_id, (unsigned int)(*thisAgent->EpMem->epmem_id_ref_counts)[(*w_p)->value->id->epmem_id ]->size());
#endif
            }

            // if the value has not been iterated over, continue to augmentations
            if ((*w_p)->value->tc_num != tc)
            {
                parent_syms.push((*w_p)->value);
                parent_ids.push((*w_p)->value->id->epmem_id);
            }
        }
        else
        {
#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   WME value is a CONSTANT.\n");
#endif

            // have we seen this node in this database?
            if (((*w_p)->epmem_id == EPMEM_NODEID_BAD) || ((*w_p)->epmem_valid != thisAgent->EpMem->epmem_validation))
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   This is a new wme.\n");
#endif

                (*w_p)->epmem_id = EPMEM_NODEID_BAD;
                (*w_p)->epmem_valid = thisAgent->EpMem->epmem_validation;

                my_hash = epmem_temporal_hash(thisAgent, (*w_p)->attr);
                my_hash2 = epmem_temporal_hash(thisAgent, (*w_p)->value);

                // try to get node id
                {
                    // parent_n_id=? AND attribute_s_id=? AND value_s_id=?
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   Looking for id of a duplicate entry in epmem_wmes_constant.\n");
#endif
                    thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->bind_int(1, parent_id);
                    thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->bind_int(2, my_hash);
                    thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->bind_int(3, my_hash2);

                    if (thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->execute() == soar_module::row)
                    {
                        (*w_p)->epmem_id = thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->column_int(0);
                    }

                    thisAgent->EpMem->epmem_stmts_graph->find_epmem_wmes_constant->reinitialize();
                }

                // act depending on new/existing feature
                if ((*w_p)->epmem_id == EPMEM_NODEID_BAD)
                {
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   No duplicate wme found in epmem_wmes_constant.  Adding wme to table!!!!\n");
                    fprintf(stderr, "   Performing database insertion: %d %d %d\n",
                            (unsigned int) parent_id, (unsigned int) my_hash, (unsigned int) my_hash2);
#endif
                    // insert (parent_n_id, attribute_s_id, value_s_id)
                    thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant->bind_int(1, parent_id);
                    thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant->bind_int(2, my_hash);
                    thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant->bind_int(3, my_hash2);
                    thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant->execute(soar_module::op_reinit);

                    (*w_p)->epmem_id = (epmem_node_id) thisAgent->EpMem->epmem_db->last_insert_rowid();
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   Setting wme id from last row to %d\n", (unsigned int)(*w_p)->epmem_id);
#endif
                    // new nodes definitely start
                    epmem_node.push((*w_p)->epmem_id);
                    thisAgent->EpMem->epmem_node_mins->push_back(time_counter);
                    thisAgent->EpMem->epmem_node_maxes->push_back(false);
                }
                else
                {
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   Node found in database, definitely don't remove.\n");
                    fprintf(stderr, "   Setting wme id from existing node to %d\n", (unsigned int)(*w_p)->epmem_id);
#endif
                    // definitely don't remove
                    (*thisAgent->EpMem->epmem_node_removals)[(*w_p)->epmem_id ] = false;

                    // add ONLY if the last thing we did was add
                    if ((*thisAgent->EpMem->epmem_node_maxes)[static_cast<size_t>((*w_p)->epmem_id - 1)])
                    {
                        epmem_node.push((*w_p)->epmem_id);
                        (*thisAgent->EpMem->epmem_node_maxes)[static_cast<size_t>((*w_p)->epmem_id - 1)] = false;
                    }
                }
            }
        }
    }
}

void epmem_new_episode(agent* thisAgent)
{

    epmem_attach(thisAgent);

    // add the episode only if db is properly initialized
    if (thisAgent->EpMem->epmem_db->get_status() != soar_module::connected)
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->storage->start();
    ////////////////////////////////////////////////////////////////////////////

    epmem_time_id time_counter = thisAgent->EpMem->epmem_stats->time->get_value();

    // provide trace output
    print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM,  "New episodic memory recorded for time %u.\n", static_cast<long int>(time_counter));

    // perform storage
    {
        // seen nodes (non-identifiers) and edges (identifiers)
        std::queue<epmem_node_id> epmem_node;
        std::queue<std::pair<epmem_node_id,int64_t>> epmem_edge;//epmem_edge now needs to keep track of the lti status/identity of the wmenode/epmemedge

        // walk appropriate levels
        {
            // prevents infinite loops
            tc_number tc = get_new_tc_number(thisAgent);

            // children of the current identifier
            epmem_wme_list* wmes = NULL;

            // breadth first search state
            std::queue< Symbol* > parent_syms;
            Symbol* parent_sym = NULL;
            std::queue< epmem_node_id > parent_ids;
            epmem_node_id parent_id;

            // cross-level information
            std::map< wme*, epmem_id_reservation* > id_reservations;
            std::set< Symbol* > new_identifiers;

            // start with new WMEs attached to known identifiers
            for (epmem_symbol_set::iterator id_p = thisAgent->EpMem->epmem_wme_adds->begin(); id_p != thisAgent->EpMem->epmem_wme_adds->end(); id_p++)
            {
                // make sure the WME is valid
                // it can be invalid if a child WME was added, but then the parent was removed, setting the epmem_id to EPMEM_NODEID_BAD
                if ((*id_p)->id->epmem_id != EPMEM_NODEID_BAD)
                {
                    parent_syms.push((*id_p));
                    parent_ids.push((*id_p)->id->epmem_id);
                    while (!parent_syms.empty())
                    {
                        parent_sym = parent_syms.front();
                        parent_syms.pop();
                        parent_id = parent_ids.front();
                        parent_ids.pop();
                        wmes = epmem_get_augs_of_id(parent_sym, tc);
                        if (! wmes->empty())
                        {
                            _epmem_store_level(thisAgent, parent_syms, parent_ids, tc, wmes->begin(), wmes->end(), parent_id, time_counter, id_reservations, new_identifiers, epmem_node, epmem_edge);
                        }
                        delete wmes;
                    }
                }
            }
        }

        // all inserts
        {
            epmem_node_id* temp_node;
            int64_t* lti_id;

            // nodes
            while (!epmem_node.empty())
            {
                temp_node = & epmem_node.front();

                // add NOW entry
                // id = ?, start_episode_id = ?
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_now->bind_int(1, (*temp_node));
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_now->bind_int(2, time_counter);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_now->execute(soar_module::op_reinit);

                // update min
                (*thisAgent->EpMem->epmem_node_mins)[static_cast<size_t>((*temp_node) - 1)] = time_counter;

                epmem_node.pop();
            }

            // edges
            while (!epmem_edge.empty())
            {//For the identifiers that are lti instances and for which they previously were not stored with the lti metadata they currently possess,
             //we need to make a new interval. The lti instance metadata will be stored as a field on the interval. The most recent lti will be
             //treated as the "default" and splitting of intervals will happen only if a change from the most recent is detected.
             //It will function similarly to a removal and replacement with respect to the relational interval tree.
                temp_node = & epmem_edge.front().first;
                lti_id = & epmem_edge.front().second;

                // add NOW entry
                // id = ?, start_episode_id = ?
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_now->bind_int(1, (*temp_node));
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_now->bind_int(2, time_counter);
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_now->bind_int(3, (*lti_id));
                thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_now->execute(soar_module::op_reinit);

                // update min
                (*thisAgent->EpMem->epmem_edge_mins)[static_cast<size_t>((*temp_node) - 1)] = time_counter;

                thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(1, LLONG_MAX);
                thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(2, *temp_node);
                thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->execute(soar_module::op_reinit);

                epmem_edge.pop();
            }
        }

        // all removals
        {

            epmem_time_id range_start;
            epmem_time_id range_end;

            // wme's with constant values
            {
                epmem_id_removal_map::iterator r;
                r = thisAgent->EpMem->epmem_node_removals->begin();
                while (r != thisAgent->EpMem->epmem_node_removals->end())
                {
                    if (r->second)
                    {

                        // remove NOW entry
                        // id = ?
                        thisAgent->EpMem->epmem_stmts_graph->delete_epmem_wmes_constant_now->bind_int(1, r->first);
                        thisAgent->EpMem->epmem_stmts_graph->delete_epmem_wmes_constant_now->execute(soar_module::op_reinit);

                        range_start = (*thisAgent->EpMem->epmem_node_mins)[static_cast<size_t>(r->first - 1)];
                        range_end = (time_counter - 1);

                        // point (id, start_episode_id)
                        if (range_start == range_end)
                        {
                            thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_point->bind_int(1, r->first);
                            thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_point->bind_int(2, range_start);
                            thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_constant_point->execute(soar_module::op_reinit);
                        }
                        // node
                        else
                        {
                            epmem_rit_insert_interval(thisAgent, range_start, range_end, r->first, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ]));
                        }

                        // update max
                        (*thisAgent->EpMem->epmem_node_maxes)[static_cast<size_t>(r->first - 1)] = true;
                    }

                    r++;
                }
                thisAgent->EpMem->epmem_node_removals->clear();
            }

            // wme's with identifier values
            epmem_edge_removal_map::iterator r = thisAgent->EpMem->epmem_edge_removals->begin();
            while (r != thisAgent->EpMem->epmem_edge_removals->end())
            {
                if (r->second)
                {
                    // remove NOW entry
                    // id = ?
                    thisAgent->EpMem->epmem_stmts_graph->delete_epmem_wmes_identifier_now->bind_int(1, r->first.first);
                    thisAgent->EpMem->epmem_stmts_graph->delete_epmem_wmes_identifier_now->execute(soar_module::op_reinit);

                    range_start = (*thisAgent->EpMem->epmem_edge_mins)[static_cast<size_t>(r->first.first - 1)];
                    range_end = (time_counter - 1);

                    thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(1, range_end);
                    thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->bind_int(2, r->first.first);
                    thisAgent->EpMem->epmem_stmts_graph->update_epmem_wmes_identifier_last_episode_id->execute(soar_module::op_reinit);
                    // point (id, start_episode_id)
                    if (range_start == range_end)
                    {
                        thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_point->bind_int(1, r->first.first);
                        thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_point->bind_int(2, range_start);
                        thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_point->bind_int(3, r->first.second);
                        thisAgent->EpMem->epmem_stmts_graph->add_epmem_wmes_identifier_point->execute(soar_module::op_reinit);
                    }
                    // node
                    else
                    {
                        epmem_rit_insert_interval(thisAgent, range_start, range_end, r->first.first, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ]), r->first.second);
                    }

                    // update max
                    (*thisAgent->EpMem->epmem_edge_maxes)[static_cast<size_t>(r->first.first - 1)] = true;
                }

                r++;
            }
            thisAgent->EpMem->epmem_edge_removals->clear();
        }

        // add the time id to the epmem_episodes table
        thisAgent->EpMem->epmem_stmts_graph->add_time->bind_int(1, time_counter);
        thisAgent->EpMem->epmem_stmts_graph->add_time->execute(soar_module::op_reinit);

        thisAgent->EpMem->epmem_stats->time->set_value(time_counter + 1);

        // update time wme on all states
        {
            Symbol* state = thisAgent->bottom_goal;
            Symbol* my_time_sym = thisAgent->symbolManager->make_int_constant(time_counter + 1);

            while (state != NULL)
            {
                if (state->id->epmem_info->epmem_time_wme != NIL)
                {
                    soar_module::remove_module_wme(thisAgent, state->id->epmem_info->epmem_time_wme);
                }

                state->id->epmem_info->epmem_time_wme = soar_module::add_module_wme(thisAgent, state->id->epmem_info->epmem_link_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_present_id, my_time_sym);

                state = state->id->higher_goal;
            }

            thisAgent->symbolManager->symbol_remove_ref(&my_time_sym);
        }

        // clear add/remove maps
        {
            thisAgent->EpMem->epmem_wme_adds->clear();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->storage->stop();
    ////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Non-Cue-Based Retrieval Functions (epmem::ncb)
//
// NCB retrievals occur when you know the episode you
// want to retrieve.  It is the process of converting
// the database representation to WMEs in working
// memory.
//
// This occurs at the end of a cue-based query, or
// in response to a retrieve/next/previous command.
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_valid_episode
 * Author       : Nate Derbinsky
 * Notes        : Returns true if the temporal id is valid
 **************************************************************************/
bool epmem_valid_episode(agent* thisAgent, epmem_time_id memory_id)
{
    bool return_val = false;

    {
        soar_module::sqlite_statement* my_q = thisAgent->EpMem->epmem_stmts_graph->valid_episode;

        my_q->bind_int(1, memory_id);
        my_q->execute();
        return_val = (my_q->column_int(0) > 0);
        my_q->reinitialize();
    }

    return return_val;
}

inline void _epmem_install_id_wme(agent* thisAgent, Symbol* parent, Symbol* attr, std::map< epmem_node_id, std::pair< Symbol*, bool > >* ids, epmem_node_id child_n_id, uint64_t val_num, epmem_id_mapping* id_record, symbol_triple_list& retrieval_wmes)
{
    std::map< epmem_node_id, std::pair< Symbol*, bool > >::iterator id_p = ids->find(child_n_id);
    bool existing_identifier = (id_p != ids->end());

        if (!existing_identifier)
        {
            if (val_num)
            {
                id_p = ids->insert(std::make_pair(child_n_id, std::make_pair(thisAgent->symbolManager->make_new_identifier(((attr->symbol_type == STR_CONSTANT_SYMBOL_TYPE) ? (attr->sc->name[0]) : ('L')), parent->id->level), true))).first;
                if (thisAgent->SMem->lti_exists(val_num))
                {
                    id_p->second.first->id->LTI_ID = val_num;
                    id_p->second.first->update_cached_lti_print_str();
                    id_p->second.first->id->LTI_epmem_valid = thisAgent->EpMem->epmem_validation;
                }
            } else {
                id_p = ids->insert(std::make_pair(child_n_id, std::make_pair(thisAgent->symbolManager->make_new_identifier(((attr->symbol_type == STR_CONSTANT_SYMBOL_TYPE) ? (attr->sc->name[0]) : ('E')), parent->id->level), true))).first;
            }
            if (id_record)
            {
                epmem_id_mapping::iterator rec_p = id_record->find(child_n_id);
                if (rec_p != id_record->end())
                {
                    rec_p->second = id_p->second.first;
                }
            }
        }

        epmem_buffer_add_wme(thisAgent, retrieval_wmes, parent, attr, id_p->second.first);

        if (!existing_identifier)
        {
            thisAgent->symbolManager->symbol_remove_ref(&id_p->second.first);
        }
}

/***************************************************************************
 * Function     : epmem_install_memory
 * Author       : Nate Derbinsky
 * Notes        : Reconstructs an episode in working memory.
 *
 *                Use RIT to collect appropriate ranges.  Then
 *                combine with NOW and POINT.  Merge with unique
 *                to get all the data necessary to create WMEs.
 *
 *                The id_record parameter is only used in the case
 *                that the graph-match has a match and creates
 *                a mapping of identifiers that should be recorded
 *                during reconstruction.
 **************************************************************************/
void epmem_install_memory(agent* thisAgent, Symbol* state, epmem_time_id memory_id, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, epmem_id_mapping* id_record = NULL)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->ncb_retrieval->start();
    ////////////////////////////////////////////////////////////////////////////

    // get the ^result header for this state
    Symbol* result_header = state->id->epmem_info->result_wme->value;

    // initialize stat
    int64_t num_wmes = 0;
    thisAgent->EpMem->epmem_stats->ncb_wmes->set_value(num_wmes);

    // if no memory, say so
    if ((memory_id == EPMEM_MEMID_NONE) ||
            !epmem_valid_episode(thisAgent, memory_id))
    {
        epmem_buffer_add_wme(thisAgent, meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.epmem_sym_retrieved, thisAgent->symbolManager->soarSymbols.epmem_sym_no_memory);
        state->id->epmem_info->last_memory = EPMEM_MEMID_NONE;

        ////////////////////////////////////////////////////////////////////////////
        thisAgent->EpMem->epmem_timers->ncb_retrieval->stop();
        ////////////////////////////////////////////////////////////////////////////

        return;
    }

    // remember this as the last memory installed
    state->id->epmem_info->last_memory = memory_id;

    // create a new ^retrieved header for this result
    Symbol* retrieved_header;
    retrieved_header = thisAgent->symbolManager->make_new_identifier('R', result_header->id->level);
    if (id_record)
    {
        (*id_record)[ EPMEM_NODEID_ROOT ] = retrieved_header;
    }

    epmem_buffer_add_wme(thisAgent, meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.epmem_sym_retrieved, retrieved_header);
    thisAgent->symbolManager->symbol_remove_ref(&retrieved_header);

    // add *-id wme's
    {
        Symbol* my_meta;

        my_meta = thisAgent->symbolManager->make_int_constant(static_cast<int64_t>(memory_id));
        epmem_buffer_add_wme(thisAgent, meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.epmem_sym_memory_id, my_meta);
        thisAgent->symbolManager->symbol_remove_ref(&my_meta);

        my_meta = thisAgent->symbolManager->make_int_constant(static_cast<int64_t>(thisAgent->EpMem->epmem_stats->time->get_value()));
        epmem_buffer_add_wme(thisAgent, meta_wmes, result_header, thisAgent->symbolManager->soarSymbols.epmem_sym_present_id, my_meta);
        thisAgent->symbolManager->symbol_remove_ref(&my_meta);
    }

    // install memory
    {
        // Big picture: create identifier skeleton, then hang non-identifers
        //
        // Because of shared WMEs at different levels of the storage breadth-first search,
        // there is the possibility that the unique database id of an identifier can be
        // greater than that of its parent.  Because the retrieval query sorts by
        // unique id ascending, it is thus possible to have an "orphan" - a child with
        // no current parent.  We keep track of orphans and add them later, hoping their
        // parents have shown up.  I *suppose* there could be a really evil case in which
        // the ordering of the unique ids is exactly opposite of their insertion order.
        // I just hope this isn't a common case...

        // shared identifier lookup table
        std::map< epmem_node_id, std::pair< Symbol*, bool > > ids;
        bool dont_abide_by_ids_second = (thisAgent->EpMem->epmem_params->merge->get_value() == epmem_param_container::merge_add);

        // symbols used to create WMEs
        Symbol* attr = NULL;

        // lookup query
        soar_module::sqlite_statement* my_q;

        // initialize the lookup table
        ids[ EPMEM_NODEID_ROOT ] = std::make_pair(retrieved_header, true);

        // first identifiers (i.e. reconstruct)
        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_identifier_values;
        {
            // relates to finite automata: child_n_id = d(parent_n_id, attribute_s_id)
            epmem_node_id parent_n_id; // id
            epmem_node_id child_n_id; // attribute

            uint64_t val_lti_id = NIL;

            // used to lookup shared identifiers
            // the bool in the pair refers to if children are allowed on this id (re: lti)
            std::map< epmem_node_id, std::pair< Symbol*, bool> >::iterator id_p;

            // orphaned children
            std::queue< epmem_edge* > orphans;
            epmem_edge* orphan;

            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ]));

            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            my_q->bind_int(5, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                // parent_n_id, attribute_s_id, child_n_id, epmem_node.lti_id
                parent_n_id = my_q->column_int(0);
                child_n_id = my_q->column_int(2);
                val_lti_id = (my_q->column_type(3) == soar_module::null_t ? 0 : static_cast<uint64_t>(my_q->column_int(3)));
                attr = epmem_reverse_hash(thisAgent, my_q->column_int(1));

                // get a reference to the parent
                id_p = ids.find(parent_n_id);
                if (id_p != ids.end())
                {
                    // if existing lti with kids don't touch
                    if (dont_abide_by_ids_second || id_p->second.second)
                    {
                        _epmem_install_id_wme(thisAgent, id_p->second.first, attr, &(ids), child_n_id, val_lti_id, id_record, retrieval_wmes);
                        num_wmes++;
                    }

                    thisAgent->symbolManager->symbol_remove_ref(&attr);
                }
                else
                {
                    // out of order
                    orphan = new epmem_edge;
                    orphan->parent_n_id = parent_n_id;
                    orphan->attribute = attr;
                    orphan->child_n_id = child_n_id;
                    orphan->child_lti_id = val_lti_id;

                    orphans.push(orphan);
                }
            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);

            // take care of any orphans
            if (!orphans.empty())
            {
                std::queue<epmem_edge*>::size_type orphans_left;
                std::queue<epmem_edge*> still_orphans;

                do
                {
                    orphans_left = orphans.size();

                    while (!orphans.empty())
                    {
                        orphan = orphans.front();
                        orphans.pop();

                        // get a reference to the parent
                        id_p = ids.find(orphan->parent_n_id);
                        if (id_p != ids.end())
                        {
                            if (dont_abide_by_ids_second || id_p->second.second)
                            {
                                _epmem_install_id_wme(thisAgent, id_p->second.first, orphan->attribute, &(ids), orphan->child_n_id, orphan->child_lti_id, id_record, retrieval_wmes);
                                num_wmes++;
                            }

                            thisAgent->symbolManager->symbol_remove_ref(&orphan->attribute);

                            delete orphan;
                        }
                        else
                        {
                            still_orphans.push(orphan);
                        }
                    }

                    orphans = still_orphans;
                    while (!still_orphans.empty())
                    {
                        still_orphans.pop();
                    }

                }
                while ((!orphans.empty()) && (orphans_left != orphans.size()));

                while (!orphans.empty())
                {
                    orphan = orphans.front();
                    orphans.pop();

                    thisAgent->symbolManager->symbol_remove_ref(&orphan->attribute);

                    delete orphan;
                }
            }
        }

        // then epmem_wmes_constant
        // f.wc_id, f.parent_n_id, f.attribute_s_id, f.value_s_id
        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_constant_values;
        {
            epmem_node_id parent_n_id;
            std::pair< Symbol*, bool > parent;
            Symbol* value = NULL;

            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ]));

            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                parent_n_id = my_q->column_int(1);

                // get a reference to the parent
                parent = ids[ parent_n_id ];

                if (dont_abide_by_ids_second || parent.second)
                {
                    // make a symbol to represent the attribute
                    attr = epmem_reverse_hash(thisAgent, my_q->column_int(2));

                    // make a symbol to represent the value
                    value = epmem_reverse_hash(thisAgent, my_q->column_int(3));

                    epmem_buffer_add_wme(thisAgent, retrieval_wmes, parent.first, attr, value);
                    num_wmes++;

                    thisAgent->symbolManager->symbol_remove_ref(&attr);
                    thisAgent->symbolManager->symbol_remove_ref(&value);
                }
            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);
        }
    }

    // adjust stat
    thisAgent->EpMem->epmem_stats->ncb_wmes->set_value(num_wmes);

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->ncb_retrieval->stop();
    ////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************
 * Function     : epmem_next_episode
 * Author       : Nate Derbinsky
 * Notes        : Returns the next valid temporal id.  This is really
 *                only an issue if you implement episode dynamics like
 *                forgetting.
 **************************************************************************/
epmem_time_id epmem_next_episode(agent* thisAgent, epmem_time_id memory_id)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->next->start();
    ////////////////////////////////////////////////////////////////////////////

    epmem_time_id return_val = EPMEM_MEMID_NONE;

    if (memory_id != EPMEM_MEMID_NONE)
    {
        soar_module::sqlite_statement* my_q = thisAgent->EpMem->epmem_stmts_graph->next_episode;
        my_q->bind_int(1, memory_id);
        if (my_q->execute() == soar_module::row)
        {
            return_val = (epmem_time_id) my_q->column_int(0);
        }

        my_q->reinitialize();
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->next->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}

/***************************************************************************
 * Function     : epmem_previous_episode
 * Author       : Nate Derbinsky
 * Notes        : Returns the last valid temporal id.  This is really
 *                only an issue if you implement episode dynamics like
 *                forgetting.
 **************************************************************************/
epmem_time_id epmem_previous_episode(agent* thisAgent, epmem_time_id memory_id)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->prev->start();
    ////////////////////////////////////////////////////////////////////////////

    epmem_time_id return_val = EPMEM_MEMID_NONE;

    if (memory_id != EPMEM_MEMID_NONE)
    {
        soar_module::sqlite_statement* my_q = thisAgent->EpMem->epmem_stmts_graph->prev_episode;
        my_q->bind_int(1, memory_id);
        if (my_q->execute() == soar_module::row)
        {
            return_val = (epmem_time_id) my_q->column_int(0);
        }

        my_q->reinitialize();
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->prev->stop();
    ////////////////////////////////////////////////////////////////////////////

    return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Cue-Based Retrieval (epmem::cbr)
//
// Cue-based retrievals are searches in response to
// queries and/or neg-queries.
//
// All functions below implement John/Andy's range search
// algorithm (see Andy's thesis).  The primary insight
// is to only deal with changes.  In this case, change
// occurs at the end points of ranges of node occurrence.
//
// The implementations below share a common theme:
// 1) identify wmes in the cue
// 2) get pointers to ALL b-tree leaves
//    associated with sorted occurrence-endpoints
//    of these wmes (ranges, points, now)
// 3) step through the leaves according to the range
//    search algorithm
//
// In the Working Memory Tree, the occurrence of a leaf
// node is tantamount to the occurrence of the path to
// the leaf node (since there are no shared identifiers).
// However, when we add graph functionality, path is
// important.  Moreover, identifiers that "blink" have
// ambiguous identities over time.  Thus I introduced
// the Disjunctive Normal Form (DNF) graph.
//
// The primary insight of the DNF graph is that paths to
// leaf nodes can be written as the disjunction of the
// conjunction of paths.
//
// Metaphor: consider that a human child's lineage is
// in question (perhaps for purposes of estate).  We
// are unsure as to the child's grandfather.  The grand-
// father can be either gA or gB.  If it is gA, then the
// father is absolutely fA (otherwise fB).  So the child
// could exist as (where cX is child with lineage X):
//
// (gA ^ fA ^ cA) \/ (gB ^ fB ^ cB)
//
// Note that due to family... irregularities
// (i.e. men sleeping around), a parent might contribute
// to multiple family lines.  Thus gX could exist in
// multiple clauses.  However, due to well-enforced
// incest laws (i.e. we only support acyclic graph cues),
// an individual can only occur once within a lineage/clause.
//
// We have a "match" (i.e. identify the child's lineage)
// only if all literals are "on" in a path of
// lineage.  Thus, our task is to efficiently track DNF
// satisfaction while flipping on/off a single literal
// (which may exist in multiple clauses).
//
// The DNF graph implements this intuition efficiently by
// (say it with me) only processing changes!  First we
// construct the graph by creating "literals" (gA, fA, etc)
// and maintaining parent-child relationships (gA connects
// to fA which connects to cA).  Leaf literals have no
// children, but are associated with a "match."  Each match
// has a cardinality count (positive/negative 1 depending on
// query vs. neg-query) and a WMA value (weighting).
//
// We then connect the b-tree pointers from above with
// each of the literals.  It is possible that a query
// can serve multiple literals, so we gain from sharing.
//
// Nodes within the DNF Graph need only save a "count":
// zero means neither it nor its lineage to date is
// satisfied, one means either its lineage or it is
// satisfied, two means it and its lineage is satisfied.
//
// The range search algorithm is simply walking (in-order)
// the parallel b-tree pointers.  When we get to an endpoint,
// we appropriately turn on/off all directly associated
// literals.  Based upon the current state of the literals,
// we may need to propagate changes to children.
//
// If propogation reaches and changes a match, we alter the
// current episode's cardinality/score.  Thus we achieve
// the Soar mantra of only processing changes!
//
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Justin's Stuff
//////////////////////////////////////////////////////////

#define QUERY_DEBUG 0

void epmem_print_retrieval_state(epmem_wme_literal_map& literals, epmem_triple_pedge_map pedge_caches[], epmem_triple_uedge_map uedge_caches[])
{
    //std::map<epmem_node_id, std::string> tsh;
    std::cout << std::endl;
    std::cout << "digraph {" << std::endl;
    std::cout << "node [style=\"filled\"];" << std::endl;
    // LITERALS
    std::cout << "subgraph cluster_literals {" << std::endl;
    std::cout << "node [fillcolor=\"#0084D1\"];" << std::endl;
    for (epmem_wme_literal_map::iterator lit_iter = literals.begin(); lit_iter != literals.end(); lit_iter++)
    {
        epmem_literal* literal = (*lit_iter).second;
        if (literal->id_sym)
        {
            std::cout << "\"" << literal->value_sym << "\" [";
            if (literal->child_n_id == EPMEM_NODEID_BAD)
            {
                std::cout << "label=\"" << literal->value_sym << "\"";
            }
            else
            {
                std::cout << "label=\"" << literal->child_n_id << "\"";
            }
            if (!literal->value_is_id)
            {
                std::cout << ", shape=\"rect\"";
            }
            if (literal->matches.size() == 0)
            {
                std::cout << ", penwidth=\"2.0\"";
            }
            if (literal->is_neg_q)
            {
                std::cout << ", fillcolor=\"#C5000B\"";
            }
            std::cout << "];" << std::endl;
            std::cout << "\"" << literal->id_sym << "\" -> \"" << literal->value_sym << "\" [label=\"";
            if (literal->attribute_s_id == EPMEM_NODEID_BAD)
            {
                std::cout << "?";
            }
            else
            {
                std::cout << literal->attribute_s_id;
            }
            std::cout << "\\n" << literal << "\"];" << std::endl;
        }
    }
    std::cout << "};" << std::endl;
    // NODES / NODE->NODE
    std::cout << "subgraph cluster_uedges{" << std::endl;
    std::cout << "node [fillcolor=\"#FFD320\"];" << std::endl;
    for (int type = EPMEM_RIT_STATE_NODE; type <= EPMEM_RIT_STATE_EDGE; type++)
    {
        epmem_triple_uedge_map* uedge_cache = &uedge_caches[type];
        for (epmem_triple_uedge_map::iterator uedge_iter = uedge_cache->begin(); uedge_iter != uedge_cache->end(); uedge_iter++)
        {
            epmem_triple triple = (*uedge_iter).first;
            if (triple.child_n_id != EPMEM_NODEID_ROOT)
            {
                if (type == EPMEM_RIT_STATE_NODE)
                {
                    std::cout << "\"n" << triple.child_n_id << "\" [shape=\"rect\"];" << std::endl;
                }
                std::cout << "\"e" << triple.parent_n_id << "\" -> \"" << (type == EPMEM_RIT_STATE_NODE ? "n" : "e") << triple.child_n_id << "\" [label=\"" << triple.attribute_s_id << "\"];" << std::endl;
            }
        }
    }
    std::cout << "};" << std::endl;
    // PEDGES / LITERAL->PEDGE
    std::cout << "subgraph cluster_pedges {" << std::endl;
    std::cout << "node [fillcolor=\"#008000\"];" << std::endl;
    std::multimap<epmem_node_id, epmem_pedge*> parent_pedge_map;
    for (int type = EPMEM_RIT_STATE_NODE; type <= EPMEM_RIT_STATE_EDGE; type++)
    {
        for (epmem_triple_pedge_map::iterator pedge_iter = pedge_caches[type].begin(); pedge_iter != pedge_caches[type].end(); pedge_iter++)
        {
            epmem_triple triple = (*pedge_iter).first;
            epmem_pedge* pedge = (*pedge_iter).second;
            if (triple.attribute_s_id != EPMEM_NODEID_BAD)
            {
                std::cout << "\"" << pedge << "\" [label=\"" << pedge << "\\n(" << triple.parent_n_id << ", " << triple.attribute_s_id << ", ";
                if (triple.child_n_id == EPMEM_NODEID_BAD)
                {
                    std::cout << "?";
                }
                else
                {
                    std::cout << triple.child_n_id;
                }
                std::cout << ")\"";
                if (!pedge->value_is_id)
                {
                    std::cout << ", shape=\"rect\"";
                }
                std::cout << "];" << std::endl;
                for (epmem_literal_set::iterator lit_iter = pedge->literals.begin(); lit_iter != pedge->literals.end(); lit_iter++)
                {
                    epmem_literal* literal = *lit_iter;
                    std::cout << "\"" << literal->value_sym << "\" -> \"" << pedge << "\";" << std::endl;
                }
                parent_pedge_map.insert(std::make_pair(triple.parent_n_id, pedge));
            }
        }
    }
    std::cout << "};" << std::endl;
    // PEDGE->PEDGE / PEDGE->NODE
    std::set<std::pair<epmem_pedge*, epmem_node_id> > drawn;
    for (int type = EPMEM_RIT_STATE_NODE; type <= EPMEM_RIT_STATE_EDGE; type++)
    {
        epmem_triple_uedge_map* uedge_cache = &uedge_caches[type];
        for (epmem_triple_uedge_map::iterator uedge_iter = uedge_cache->begin(); uedge_iter != uedge_cache->end(); uedge_iter++)
        {
            epmem_triple triple = (*uedge_iter).first;
            epmem_uedge* uedge = (*uedge_iter).second;
            if (triple.attribute_s_id != EPMEM_NODEID_BAD)
            {
                for (epmem_pedge_set::iterator pedge_iter = uedge->pedges.begin(); pedge_iter != uedge->pedges.end(); pedge_iter++)
                {
                    epmem_pedge* pedge = *pedge_iter;
                    std::pair<epmem_pedge*, epmem_node_id> pair = std::make_pair(pedge, triple.parent_n_id);
                    if (!drawn.count(pair))
                    {
                        drawn.insert(pair);
                        std::cout << "\"" << pedge << "\" -> \"e" << triple.parent_n_id << "\";" << std::endl;
                    }
                    std::cout << "\"" << pedge << "\" -> \"" << (pedge->value_is_id ? "e" : "n") << triple.child_n_id << "\" [style=\"dashed\"];" << std::endl;
                    std::pair<std::multimap<epmem_node_id, epmem_pedge*>::iterator, std::multimap<epmem_node_id, epmem_pedge*>::iterator> pedge_iters = parent_pedge_map.equal_range(triple.child_n_id);
                    for (std::multimap<epmem_node_id, epmem_pedge*>::iterator pedge_iter = pedge_iters.first; pedge_iter != pedge_iters.second; pedge_iter++)
                    {
                        std::cout << "\"" << pedge << "\" -> \"" << (*pedge_iter).second << "\";" << std::endl;
                    }
                }
            }
        }
    }
    std::cout << "}" << std::endl;
}

bool epmem_gm_mcv_comparator(const epmem_literal* a, const epmem_literal* b)
{
    return (a->matches.size() < b->matches.size());
}

epmem_literal* epmem_build_dnf(wme* cue_wme, epmem_wme_literal_map& literal_cache, epmem_literal_set& leaf_literals, epmem_symbol_int_map& symbol_num_incoming, epmem_literal_deque& gm_ordering, int query_type, std::set<Symbol*>& visiting, wme_set& cue_wmes, agent* thisAgent)
{
    // if the value is being visited, this is part of a loop; return NULL
    // remove this check (and in fact, the entire visiting parameter) if cyclic cues are allowed
    if (visiting.count(cue_wme->value))
    {
        return NULL;
    }
    // if the value is an identifier and we've been here before, we can return the previous literal
    if (literal_cache.count(cue_wme))
    {
        return literal_cache[cue_wme];
    }

    cue_wmes.insert(cue_wme);
    Symbol* value = cue_wme->value;
    epmem_literal* literal;
    thisAgent->memoryManager->allocate_with_pool(MP_epmem_literal, &literal);
    new(&(literal->parents)) epmem_literal_set();
    new(&(literal->children)) epmem_literal_set();

    if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE)   // WME is a value
    {
        literal->value_is_id = EPMEM_RIT_STATE_NODE;
        literal->is_leaf = true;
        literal->child_n_id = epmem_temporal_hash(thisAgent, value);
        leaf_literals.insert(literal);
    }
    //else if (value->id->is_lti() && value->id->LTI_ID)
    //{
        // This is an attempt to reintegrate matching of ltis into epmem queries despite the
        // change to make ltis into instances. The idea is that ltis require both a structure match
        // like normal identifiers, and also a direct match of the lti in question.
        // Treating ltis purely like normal identifiers (lacking this "else", essentially)
        // is done with an additional command when the query is issued to epmem.
        // TODO: Actually implement that extra (agent-initiated epmem-link) command.
        /*
         * scijones - May 2 2017 My first try at implementing this is just to copy the old code
         * that was here in the first place before we changed ltis to be instance-based.
         */
        // The first step is to find the LTI at all in the first place. If it's not present,
        // we can just return failure.

    //}
    else     // WME is a normal identifier
    {
        // we determine whether it is a leaf by checking for children
        epmem_wme_list* children = epmem_get_augs_of_id(value, get_new_tc_number(thisAgent));
        literal->value_is_id = EPMEM_RIT_STATE_EDGE;
        literal->child_n_id = EPMEM_NODEID_BAD;

        // if the WME has no children, then it's a leaf
        // otherwise, we recurse for all children
        if (children->empty())
        {
            literal->is_leaf = true;
            leaf_literals.insert(literal);
            delete children;
        }
        else
        {
            bool cycle = false;
            visiting.insert(cue_wme->value);
            for (epmem_wme_list::iterator wme_iter = children->begin(); wme_iter != children->end(); wme_iter++)
            {
                // check to see if this child forms a cycle
                // if it does, we skip over it
                epmem_literal* child = epmem_build_dnf(*wme_iter, literal_cache, leaf_literals, symbol_num_incoming, gm_ordering, query_type, visiting, cue_wmes, thisAgent);
                if (child)
                {
                    child->parents.insert(literal);
                    literal->children.insert(child);
                }
                else
                {
                    cycle = true;
                }
            }
            delete children;
            visiting.erase(cue_wme->value);
            // if all children of this WME lead to cycles, then we don't need to walk this path
            // in essence, this forces the DNF graph to be acyclic
            // this results in savings in not walking edges and intervals
            if (cycle && literal->children.empty())
            {
                literal->parents.~epmem_literal_set();
                literal->children.~epmem_literal_set();
                thisAgent->memoryManager->free_with_pool(MP_epmem_literal, literal);
                return NULL;
            }
            literal->is_leaf = false;
            epmem_symbol_int_map::iterator rem_iter = symbol_num_incoming.find(value);
            if (rem_iter == symbol_num_incoming.end())
            {
                symbol_num_incoming[value] = 1;
            }
            else
            {
                (*rem_iter).second++;
            }
        }
    }

    if (query_type == EPMEM_NODE_POS)
    {
        gm_ordering.push_front(literal);
        thisAgent->EpMem->epmem_stats->qry_pos->set_value(thisAgent->EpMem->epmem_stats->qry_pos->get_value() + 1);
    }
    else
    {
        thisAgent->EpMem->epmem_stats->qry_neg->set_value(thisAgent->EpMem->epmem_stats->qry_neg->get_value() + 1);
    }

    literal->id_sym = cue_wme->id;
    literal->value_sym = cue_wme->value;
    literal->attribute_s_id = epmem_temporal_hash(thisAgent, cue_wme->attr);
    literal->is_neg_q = query_type;
    literal->weight = (literal->is_neg_q ? -1 : 1) * (thisAgent->EpMem->epmem_params->balance->get_value() >= 1.0 - 1.0e-8 ? 1.0 : wma_get_wme_activation(thisAgent, cue_wme, true));
#ifdef USE_MEM_POOL_ALLOCATORS
    new(&(literal->matches)) epmem_node_pair_set(std::less<epmem_node_pair>(), soar_module::soar_memory_pool_allocator<epmem_node_pair>());
#else
    new(&(literal->matches)) epmem_node_pair_set();
#endif
    new(&(literal->values)) epmem_node_int_map();

    literal_cache[cue_wme] = literal;
    return literal;
}

bool epmem_register_pedges(epmem_node_id parent, epmem_literal* literal, epmem_pedge_pq& pedge_pq, epmem_time_id after, epmem_triple_pedge_map pedge_caches[], epmem_triple_uedge_map uedge_caches[], agent* thisAgent)
{
    // we don't need to keep track of visited literals/nodes because the literals are guaranteed to be acyclic
    // that is, the expansion to the literal's children will eventually bottom out
    // select the query
    epmem_triple triple = {parent, literal->attribute_s_id, literal->child_n_id};
    int is_edge = literal->value_is_id;
    if (QUERY_DEBUG >= 1)
    {
        std::cout << "		RECURSING ON " << parent << " " << literal << std::endl;
    }
    // if the unique edge does not exist, create a new unique edge query
    // otherwse, if the pedge has not been registered with this literal
    epmem_triple_pedge_map* pedge_cache = &(pedge_caches[is_edge]);
    epmem_triple_pedge_map::iterator pedge_iter = pedge_cache->find(triple);
    epmem_pedge* child_pedge = NULL;
    if (pedge_iter == pedge_cache->end() || (*pedge_iter).second == NULL)
    {
        int has_value = (literal->child_n_id != EPMEM_NODEID_BAD ? 1 : 0);
        soar_module::pooled_sqlite_statement* pedge_sql = thisAgent->EpMem->epmem_stmts_graph->pool_find_edge_queries[is_edge][has_value]->request(thisAgent->EpMem->epmem_timers->query_sql_edge);
        int bind_pos = 1;
        if (!is_edge)
        {
            pedge_sql->bind_int(bind_pos++, LLONG_MAX);
        }
        pedge_sql->bind_int(bind_pos++, triple.parent_n_id);
        pedge_sql->bind_int(bind_pos++, triple.attribute_s_id);
        if (has_value)
        {
            pedge_sql->bind_int(bind_pos++, triple.child_n_id);
        }
        if (is_edge)
        {
            pedge_sql->bind_int(bind_pos++, after);
        }
        if (pedge_sql->execute() == soar_module::row)
        {
            thisAgent->memoryManager->allocate_with_pool(MP_epmem_pedge, &child_pedge);
            child_pedge->triple = triple;
            child_pedge->value_is_id = literal->value_is_id;
            child_pedge->sql = pedge_sql;
            new(&(child_pedge->literals)) epmem_literal_set();
            child_pedge->literals.insert(literal);
            child_pedge->time = child_pedge->sql->column_int(2);
            pedge_pq.push(child_pedge);
            (*pedge_cache)[triple] = child_pedge;
            return true;
        }
        else
        {
            pedge_sql->get_pool()->release(pedge_sql);
            return false;
        }
    }
    else
    {
        child_pedge = (*pedge_iter).second;
        if (!child_pedge->literals.count(literal))
        {
            child_pedge->literals.insert(literal);
            // if the literal is an edge with no specified value, add the literal to all potential pedges
            if (!literal->is_leaf && literal->child_n_id == EPMEM_NODEID_BAD)
            {
                bool created = false;
                epmem_triple_uedge_map* uedge_cache = &uedge_caches[is_edge];
                for (epmem_triple_uedge_map::iterator uedge_iter = uedge_cache->lower_bound(triple); uedge_iter != uedge_cache->end(); uedge_iter++)
                {
                    epmem_triple child_triple = (*uedge_iter).first;
                    // make sure we're still looking at the right edge(s)
                    if (child_triple.parent_n_id != triple.parent_n_id || child_triple.attribute_s_id != triple.attribute_s_id)
                    {
                        break;
                    }
                    epmem_uedge* child_uedge = (*uedge_iter).second;
                    if (child_triple.child_n_id != EPMEM_NODEID_BAD && child_uedge->value_is_id)
                    {
                        for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++)
                        {
                            created |= epmem_register_pedges(child_triple.child_n_id, *child_iter, pedge_pq, after, pedge_caches, uedge_caches, thisAgent);
                        }
                    }
                }
                return created;
            }
        }
    }
    return true;
}

bool epmem_satisfy_literal(epmem_literal* literal, epmem_node_id parent, epmem_node_id child, double& current_score, long int& current_cardinality, epmem_symbol_node_pair_int_map& symbol_node_count, epmem_triple_uedge_map uedge_caches[], epmem_symbol_int_map& symbol_num_incoming)
{
    epmem_symbol_node_pair_int_map::iterator match_iter;
    if (QUERY_DEBUG >= 1)
    {
        std::cout << "		RECURSING ON " << parent << " " << child << " " << literal << std::endl;
    }
    // check if the ancestors of this literal are satisfied
    bool parents_satisfied = (literal->id_sym == NULL);
    if (!parents_satisfied)
    {
        // ancestors are satisfied if:
        // 1. all incoming literals are satisfied
        // 2. all incoming literals have this particular node satisfying it
        int num_incoming = symbol_num_incoming[literal->id_sym];
        match_iter = symbol_node_count.find(std::make_pair(literal->id_sym, parent));
        // since, by definition, if a node satisfies all incoming literals, all incoming literals are satisfied
        parents_satisfied = (match_iter != symbol_node_count.end()) && ((*match_iter).second == num_incoming);
    }
    // if yes
    if (parents_satisfied)
    {
        // add the edge as a match
        literal->matches.insert(std::make_pair(parent, child));
        epmem_node_int_map::iterator values_iter = literal->values.find(child);
        if (values_iter == literal->values.end())
        {
            literal->values[child] = 1;
            if (QUERY_DEBUG >= 1)
            {
                std::cout << "		LITERAL SATISFIED: " << literal << std::endl;
            }
            if (literal->is_leaf)
            {
                if (literal->matches.size() == 1)
                {
                    current_score += literal->weight;
                    current_cardinality += (literal->is_neg_q ? -1 : 1);
                    if (QUERY_DEBUG >= 1)
                    {
                        std::cout << "			NEW SCORE: " << current_score << ", " << current_cardinality << std::endl;
                    }
                    return true;
                }
            }
            else
            {
                bool changed_score = false;
                // change bookkeeping information about ancestry
                epmem_symbol_node_pair match = std::make_pair(literal->value_sym, child);
                match_iter = symbol_node_count.find(match);
                if (match_iter == symbol_node_count.end())
                {
                    symbol_node_count[match] = 1;
                }
                else
                {
                    symbol_node_count[match]++;
                }
                // recurse over child literals
                for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++)
                {
                    epmem_literal* child_lit = *child_iter;
                    epmem_triple_uedge_map* uedge_cache = &uedge_caches[child_lit->value_is_id];
                    epmem_triple child_triple = {child, child_lit->attribute_s_id, child_lit->child_n_id};
                    epmem_triple_uedge_map::iterator uedge_iter;
                    epmem_uedge* child_uedge = NULL;
                    if (child_lit->child_n_id == EPMEM_NODEID_BAD)
                    {
                        uedge_iter = uedge_cache->lower_bound(child_triple);
                        while (uedge_iter != uedge_cache->end())
                        {
                            child_triple = (*uedge_iter).first;
                            child_uedge = (*uedge_iter).second;
                            if (child_triple.parent_n_id != child || child_triple.attribute_s_id != child_lit->attribute_s_id)
                            {
                                break;
                            }
                            if (child_uedge->activated && child_uedge->activation_count == 1)
                            {
                                changed_score |= epmem_satisfy_literal(child_lit, child_triple.parent_n_id, child_triple.child_n_id, current_score, current_cardinality, symbol_node_count, uedge_caches, symbol_num_incoming);
                            }
                            uedge_iter++;
                        }
                    }
                    else
                    {
                        uedge_iter = uedge_cache->find(child_triple);
                        if (uedge_iter != uedge_cache->end() && (*uedge_iter).second != NULL)
                        {
                            child_uedge = (*uedge_iter).second;
                            if (child_uedge->activated && child_uedge->activation_count == 1)
                            {
                                changed_score |= epmem_satisfy_literal(child_lit, child_triple.parent_n_id, child_triple.child_n_id, current_score, current_cardinality, symbol_node_count, uedge_caches, symbol_num_incoming);
                            }
                        }
                    }
                }
                return changed_score;
            }
        }
        else
        {
            (*values_iter).second++;
        }
    }
    return false;
}

bool epmem_unsatisfy_literal(epmem_literal* literal, epmem_node_id parent, epmem_node_id child, double& current_score, long int& current_cardinality, epmem_symbol_node_pair_int_map& symbol_node_count)
{
    epmem_symbol_int_map::iterator count_iter;
    if (literal->matches.size() == 0)
    {
        return false;
    }
    if (QUERY_DEBUG >= 1)
    {
        std::cout << "		RECURSING ON " << parent << " " << child << " " << literal << std::endl;
    }
    // we only need things if this parent-child pair is matching the literal
    epmem_node_pair_set::iterator lit_match_iter = literal->matches.find(std::make_pair(parent, child));
    if (lit_match_iter != literal->matches.end())
    {
        // erase the edge from this literal's matches
        literal->matches.erase(lit_match_iter);
        epmem_node_int_map::iterator values_iter = literal->values.find(child);
        (*values_iter).second--;
        if ((*values_iter).second == 0)
        {
            literal->values.erase(values_iter);
            if (QUERY_DEBUG >= 1)
            {
                std::cout << "		LITERAL UNSATISFIED: " << literal << std::endl;
            }
            if (literal->is_leaf)
            {
                if (literal->matches.size() == 0)
                {
                    current_score -= literal->weight;
                    current_cardinality -= (literal->is_neg_q ? -1 : 1);
                    if (QUERY_DEBUG >= 1)
                    {
                        std::cout << "			NEW SCORE: " << current_score << ", " << current_cardinality << std::endl;
                    }
                    return true;
                }
            }
            else
            {
                bool changed_score = false;
                epmem_symbol_node_pair match = std::make_pair(literal->value_sym, child);
                epmem_symbol_node_pair_int_map::iterator match_iter = symbol_node_count.find(match);
                (*match_iter).second--;
                if ((*match_iter).second == 0)
                {
                    symbol_node_count.erase(match_iter);
                }
                // if this literal is no longer satisfied, recurse on all children
                // if this literal is still satisfied, recurse on children who is matching on descendants of this edge
                if (literal->matches.size() == 0)
                {
                    for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++)
                    {
                        epmem_literal* child_lit = *child_iter;
                        epmem_node_pair_set::iterator node_iter = child_lit->matches.begin();
                        while (node_iter != child_lit->matches.end())
                        {
                            changed_score |= epmem_unsatisfy_literal(child_lit, (*node_iter).first, (*node_iter).second, current_score, current_cardinality, symbol_node_count);
                            if (child_lit->matches.empty())
                            {
                                break;
                            }
                            else
                            {
                                node_iter++;
                            }
                        }
                    }
                }
                else
                {
                    epmem_node_pair node_pair = std::make_pair(child, EPMEM_NODEID_BAD);
                    for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++)
                    {
                        epmem_literal* child_lit = *child_iter;
                        epmem_node_pair_set::iterator node_iter = child_lit->matches.lower_bound(node_pair);
                        if (node_iter != child_lit->matches.end() && (*node_iter).first == child)
                        {
                            changed_score |= epmem_unsatisfy_literal(child_lit, (*node_iter).first, (*node_iter).second, current_score, current_cardinality, symbol_node_count);
                        }
                    }
                }
                return changed_score;
            }
        }
    }
    return false;
}

bool epmem_graph_match(epmem_literal_deque::iterator& dnf_iter, epmem_literal_deque::iterator& iter_end, epmem_literal_node_pair_map& bindings, epmem_node_symbol_map bound_nodes[], agent* thisAgent, int depth = 0)
{
    if (dnf_iter == iter_end)
    {
        return true;
    }
    epmem_literal* literal = *dnf_iter;
    if (bindings.count(literal))
    {
        return false;
    }
    epmem_literal_deque::iterator next_iter = dnf_iter;
    next_iter++;
#ifdef USE_MEM_POOL_ALLOCATORS
    epmem_node_set failed_parents = epmem_node_set(std::less<epmem_node_id>(), soar_module::soar_memory_pool_allocator<epmem_node_id>());
    epmem_node_set failed_children = epmem_node_set(std::less<epmem_node_id>(), soar_module::soar_memory_pool_allocator<epmem_node_id>());
#else
    epmem_node_set failed_parents;
    epmem_node_set failed_children;
#endif
    // go through the list of matches, binding each one to this literal in turn
    for (epmem_node_pair_set::iterator match_iter = literal->matches.begin(); match_iter != literal->matches.end(); match_iter++)
    {
        epmem_node_id parent_n_id = (*match_iter).first;
        epmem_node_id child_n_id = (*match_iter).second;
        if (failed_parents.count(parent_n_id))
        {
            continue;
        }
        if (QUERY_DEBUG >= 2)
        {
            for (int i = 0; i < depth; i++)
            {
                std::cout << "\t";
            }
            std::cout << "TRYING " << literal << " " << parent_n_id << std::endl;
        }
        bool relations_okay = true;
        // for all parents
        for (epmem_literal_set::iterator parent_iter = literal->parents.begin(); relations_okay && parent_iter != literal->parents.end(); parent_iter++)
        {
            epmem_literal* parent = *parent_iter;
            epmem_literal_node_pair_map::iterator bind_iter = bindings.find(parent);
            if (bind_iter != bindings.end() && (*bind_iter).second.second != parent_n_id)
            {
                relations_okay = false;
            }
        }
        if (!relations_okay)
        {
            if (QUERY_DEBUG >= 2)
            {
                for (int i = 0; i < depth; i++)
                {
                    std::cout << "\t";
                }
                std::cout << "PARENT CONSTRAINT FAIL" << std::endl;
            }
            failed_parents.insert(parent_n_id);
            continue;
        }
        // if the node has already been bound, make sure it's bound to the same thing
        epmem_node_symbol_map::iterator binder = bound_nodes[literal->value_is_id].find(child_n_id);
        if (binder != bound_nodes[literal->value_is_id].end() && (*binder).second != literal->value_sym)
        {
            failed_children.insert(child_n_id);
            continue;
        }
        if (QUERY_DEBUG >= 2)
        {
            for (int i = 0; i < depth; i++)
            {
                std::cout << "\t";
            }
            std::cout << "TRYING " << literal << " " << parent_n_id << " " << child_n_id << std::endl;
        }
        if (literal->child_n_id != EPMEM_NODEID_BAD && literal->child_n_id != child_n_id)
        {
            relations_okay = false;
        }
        // for all children
        for (epmem_literal_set::iterator child_iter = literal->children.begin(); relations_okay && child_iter != literal->children.end(); child_iter++)
        {
            epmem_literal* child = *child_iter;
            epmem_literal_node_pair_map::iterator bind_iter = bindings.find(child);
            if (bind_iter != bindings.end() && (*bind_iter).second.first != child_n_id)
            {
                relations_okay = false;
            }
        }
        if (!relations_okay)
        {
            if (QUERY_DEBUG >= 2)
            {
                for (int i = 0; i < depth; i++)
                {
                    std::cout << "\t";
                }
                std::cout << "CHILD CONSTRAINT FAIL" << std::endl;
            }
            failed_children.insert(child_n_id);
            continue;
        }
        if (QUERY_DEBUG >= 2)
        {
            for (int i = 0; i < depth; i++)
            {
                std::cout << "\t";
            }
            std::cout << literal << " " << parent_n_id << " " << child_n_id << std::endl;
        }
        // temporarily modify the bindings and bound nodes
        bindings[literal] = std::make_pair(parent_n_id, child_n_id);
        bound_nodes[literal->value_is_id][child_n_id] = literal->value_sym;
        // recurse on the rest of the list
        bool list_satisfied = epmem_graph_match(next_iter, iter_end, bindings, bound_nodes, thisAgent, depth + 1);
        // if the rest of the list matched, we've succeeded
        // otherwise, undo the temporarily modifications and try again
        if (list_satisfied)
        {
            return true;
        }
        else
        {
            bindings.erase(literal);
            bound_nodes[literal->value_is_id].erase(child_n_id);
        }
    }
    // this means we've tried everything and this whole exercise was a waste of time
    // EPIC FAIL
    if (QUERY_DEBUG >= 2)
    {
        for (int i = 0; i < depth; i++)
        {
            std::cout << "\t";
        }
        std::cout << "EPIC FAIL" << std::endl;
    }
    return false;
}

void epmem_process_query(agent* thisAgent, Symbol* state, Symbol* pos_query, Symbol* neg_query, epmem_time_list& prohibits, epmem_time_id before, epmem_time_id after, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, int level = 3)
{
    // a query must contain a positive cue
    if (pos_query == NULL)
    {
        epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_status, thisAgent->symbolManager->soarSymbols.epmem_sym_bad_cmd);
        return;
    }

    // before and after, if specified, must be valid relative to each other
    if (before != EPMEM_MEMID_NONE && after != EPMEM_MEMID_NONE && before <= after)
    {
        epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_status, thisAgent->symbolManager->soarSymbols.epmem_sym_bad_cmd);
        return;
    }

    if (QUERY_DEBUG >= 1)
    {
        std::cout << std::endl << "==========================" << std::endl << std::endl;
    }

    thisAgent->EpMem->epmem_timers->query->start();

    // sort probibit's
    if (!prohibits.empty())
    {
        std::sort(prohibits.begin(), prohibits.end());
    }

    // epmem options
    bool do_graph_match = (thisAgent->EpMem->epmem_params->graph_match->get_value() == on);
    epmem_param_container::gm_ordering_choices gm_order = thisAgent->EpMem->epmem_params->gm_ordering->get_value();

    // variables needed for cleanup
    epmem_wme_literal_map literal_cache;
    epmem_triple_pedge_map pedge_caches[2];
#ifdef USE_MEM_POOL_ALLOCATORS
    epmem_triple_uedge_map uedge_caches[2] =
    {
        epmem_triple_uedge_map(std::less<epmem_triple>(), soar_module::soar_memory_pool_allocator<std::pair<const epmem_triple, epmem_uedge*> >()),
        epmem_triple_uedge_map(std::less<epmem_triple>(), soar_module::soar_memory_pool_allocator<std::pair<const epmem_triple, epmem_uedge*> >())
    };
    epmem_interval_set interval_cleanup = epmem_interval_set(std::less<epmem_interval*>(), soar_module::soar_memory_pool_allocator<epmem_interval*>());
#else
    epmem_triple_uedge_map uedge_caches[2] = {epmem_triple_uedge_map(), epmem_triple_uedge_map()};
    epmem_interval_set interval_cleanup = epmem_interval_set();
#endif

    // TODO JUSTIN additional indices

    // variables needed for building the DNF
    epmem_literal* root_literal;
    thisAgent->memoryManager->allocate_with_pool(MP_epmem_literal, &root_literal);
    epmem_literal_set leaf_literals;

    // priority queues for interval walk
    epmem_pedge_pq pedge_pq;
    epmem_interval_pq interval_pq;

    // variables needed to track satisfiability
    epmem_symbol_int_map symbol_num_incoming;                 // number of literals with a certain symbol as its value
    epmem_symbol_node_pair_int_map symbol_node_count;         // number of times a symbol is matched by a node

    // various things about the current and the best episodes
    epmem_time_id best_episode = EPMEM_MEMID_NONE;
    double best_score = 0;
    bool best_graph_matched = false;
    long int best_cardinality = 0;
    epmem_literal_node_pair_map best_bindings;
    double current_score = 0;
    long int current_cardinality = 0;

    // variables needed for graphmatch
    epmem_literal_deque gm_ordering;

    if (level > 1)
    {
        // build the DNF graph while checking for leaf WMEs
        {
            thisAgent->EpMem->epmem_stats->qry_pos->set_value(0);
            thisAgent->EpMem->epmem_stats->qry_neg->set_value(0);
            thisAgent->EpMem->epmem_timers->query_dnf->start();
            root_literal->id_sym = NULL;
            root_literal->value_sym = pos_query;
            root_literal->is_neg_q = EPMEM_NODE_POS;
            root_literal->value_is_id = EPMEM_RIT_STATE_EDGE;
            root_literal->is_leaf = false;
            root_literal->attribute_s_id = EPMEM_NODEID_BAD;
            root_literal->child_n_id = EPMEM_NODEID_ROOT;
            root_literal->weight = 0.0;
            new(&(root_literal->parents)) epmem_literal_set();
            new(&(root_literal->children)) epmem_literal_set();
#ifdef USE_MEM_POOL_ALLOCATORS
            new(&(root_literal->matches)) epmem_node_pair_set(std::less<epmem_node_pair>(), soar_module::soar_memory_pool_allocator<epmem_node_pair>(thisAgent));
#else
            new(&(root_literal->matches)) epmem_node_pair_set();
#endif
            new(&(root_literal->values)) epmem_node_int_map();
            symbol_num_incoming[pos_query] = 1;
            literal_cache[NULL] = root_literal;

            std::set<Symbol*> visiting;
            visiting.insert(pos_query);
            visiting.insert(neg_query);
            for (int query_type = EPMEM_NODE_POS; query_type <= EPMEM_NODE_NEG; query_type++)
            {
                Symbol* query_root = NULL;
                switch (query_type)
                {
                    case EPMEM_NODE_POS:
                        query_root = pos_query;
                        break;
                    case EPMEM_NODE_NEG:
                        query_root = neg_query;
                        break;
                }
                if (!query_root)
                {
                    continue;
                }
                epmem_wme_list* children = epmem_get_augs_of_id(query_root, get_new_tc_number(thisAgent));
                // for each first level WME, build up a DNF
                for (epmem_wme_list::iterator wme_iter = children->begin(); wme_iter != children->end(); wme_iter++)
                {
                    epmem_literal* child = epmem_build_dnf(*wme_iter, literal_cache, leaf_literals, symbol_num_incoming, gm_ordering, query_type, visiting, cue_wmes, thisAgent);
                    if (child)
                    {
                        // force all first level literals to have the same id symbol
                        child->id_sym = pos_query;
                        child->parents.insert(root_literal);
                        root_literal->children.insert(child);
                    }
                }
                delete children;
            }
            thisAgent->EpMem->epmem_timers->query_dnf->stop();
            thisAgent->EpMem->epmem_stats->qry_lits->set_value(thisAgent->EpMem->epmem_stats->qry_pos->get_value() + thisAgent->EpMem->epmem_stats->qry_neg->get_value());
        }

        // calculate the highest possible score and cardinality score
        double perfect_score = 0;
        int perfect_cardinality = 0;
        for (epmem_literal_set::iterator iter = leaf_literals.begin(); iter != leaf_literals.end(); iter++)
        {
            if (!(*iter)->is_neg_q)
            {
                perfect_score += (*iter)->weight;
                perfect_cardinality++;
            }
        }

        // set default values for before and after
        if (before == EPMEM_MEMID_NONE)
        {
            before = thisAgent->EpMem->epmem_stats->time->get_value() - 1;
        }
        else
        {
            before = before - 1; // since before's are strict
        }
        if (after == EPMEM_MEMID_NONE)
        {
            after = EPMEM_MEMID_NONE;
        }
        epmem_time_id current_episode = before;
        epmem_time_id next_episode;

        // create dummy edges and intervals
        {
            // insert dummy unique edge and interval end point queries for DNF root
            // we make an SQL statement just so we don't have to do anything special at cleanup
            epmem_triple triple = {EPMEM_NODEID_BAD, EPMEM_NODEID_BAD, EPMEM_NODEID_ROOT};
            epmem_pedge* root_pedge;
            thisAgent->memoryManager->allocate_with_pool(MP_epmem_pedge, &root_pedge);
            root_pedge->triple = triple;
            root_pedge->value_is_id = EPMEM_RIT_STATE_EDGE;
            new(&(root_pedge->literals)) epmem_literal_set();
            root_pedge->literals.insert(root_literal);
            root_pedge->sql = thisAgent->EpMem->epmem_stmts_graph->pool_dummy->request();
            root_pedge->sql->prepare();
            root_pedge->sql->bind_int(1, LLONG_MAX);
            root_pedge->sql->execute(soar_module::op_reinit);
            root_pedge->time = LLONG_MAX;
            pedge_pq.push(root_pedge);
            pedge_caches[EPMEM_RIT_STATE_EDGE][triple] = root_pedge;

            epmem_uedge* root_uedge;
            thisAgent->memoryManager->allocate_with_pool(MP_epmem_uedge, &root_uedge);
            root_uedge->triple = triple;
            root_uedge->value_is_id = EPMEM_RIT_STATE_EDGE;
            root_uedge->activation_count = 0;
            new(&(root_uedge->pedges)) epmem_pedge_set();
            root_uedge->intervals = 1;
            root_uedge->activated = false;
            uedge_caches[EPMEM_RIT_STATE_EDGE][triple] = root_uedge;

            epmem_interval* root_interval;
            thisAgent->memoryManager->allocate_with_pool(MP_epmem_interval, &root_interval);
            root_interval->uedge = root_uedge;
            root_interval->is_end_point = true;
            root_interval->sql = thisAgent->EpMem->epmem_stmts_graph->pool_dummy->request();
            root_interval->sql->prepare();
            root_interval->sql->bind_int(1, before);
            root_interval->sql->execute(soar_module::op_reinit);
            root_interval->time = before;
            interval_pq.push(root_interval);
            interval_cleanup.insert(root_interval);
        }

        if (QUERY_DEBUG >= 1)
        {
            epmem_print_retrieval_state(literal_cache, pedge_caches, uedge_caches);
        }

        // main loop of interval walk
        thisAgent->EpMem->epmem_timers->query_walk->start();
        while (pedge_pq.size() && current_episode > after)
        {
            epmem_time_id next_edge;
            epmem_time_id next_interval;

            bool changed_score = false;

            thisAgent->EpMem->epmem_timers->query_walk_edge->start();
            next_edge = pedge_pq.top()->time;

            // process all edges which were last used at this time point
            while (pedge_pq.size() && (pedge_pq.top()->time == next_edge || pedge_pq.top()->time >= current_episode))
            {
                epmem_pedge* pedge = pedge_pq.top();
                pedge_pq.pop();
                epmem_triple triple = pedge->triple;
                triple.child_n_id = pedge->sql->column_int(1);

                if (QUERY_DEBUG >= 1)
                {
                    std::cout << "	EDGE " << triple.parent_n_id << "-" << triple.attribute_s_id << "-" << triple.child_n_id << std::endl;
                }

                // create queries for the unique edge children of this partial edge
                if (pedge->value_is_id)
                {
                    bool created = false;
                    for (epmem_literal_set::iterator literal_iter = pedge->literals.begin(); literal_iter != pedge->literals.end(); literal_iter++)
                    {
                        epmem_literal* literal = *literal_iter;
                        for (epmem_literal_set::iterator child_iter = literal->children.begin(); child_iter != literal->children.end(); child_iter++)
                        {
                            created |= epmem_register_pedges(triple.child_n_id, *child_iter, pedge_pq, after, pedge_caches, uedge_caches, thisAgent);
                        }
                    }
                }
                // TODO JUSTIN what I want to do here is, if there is no children which leads to a leaf, retract everything
                // I'm not sure how to properly test for this though

                // look for uedge with triple; if none exist, create one
                // otherwise, link up the uedge with the pedge and consider score changes
                epmem_triple_uedge_map* uedge_cache = &uedge_caches[pedge->value_is_id];
                epmem_triple_uedge_map::iterator uedge_iter = uedge_cache->find(triple);
                if (uedge_iter == uedge_cache->end())
                {
                    // create a uedge for this
                    epmem_uedge* uedge;
                    thisAgent->memoryManager->allocate_with_pool(MP_epmem_uedge, &uedge);
                    uedge->triple = triple;
                    uedge->value_is_id = pedge->value_is_id;
                    uedge->activation_count = 0;
                    new(&(uedge->pedges)) epmem_pedge_set();
                    uedge->intervals = 0;
                    uedge->activated = false;
                    // create interval queries for this partial edge
                    bool created = false;
                    int64_t edge_id = pedge->sql->column_int(0);
                    for (int interval_type = EPMEM_RANGE_EP; interval_type <= EPMEM_RANGE_POINT; interval_type++)
                    {
                        for (int point_type = EPMEM_RANGE_START; point_type <= EPMEM_RANGE_END; point_type++)
                        {
                            // pick a timer (any timer)
                            soar_module::timer* sql_timer = NULL;
                            switch (interval_type)
                            {
                                case EPMEM_RANGE_EP:
                                    if (point_type == EPMEM_RANGE_START)
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_start_ep;
                                    }
                                    else
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_end_ep;
                                    }
                                    break;
                                case EPMEM_RANGE_NOW:
                                    if (point_type == EPMEM_RANGE_START)
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_start_now;
                                    }
                                    else
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_end_now;
                                    }
                                    break;
                                case EPMEM_RANGE_POINT:
                                    if (point_type == EPMEM_RANGE_START)
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_start_point;
                                    }
                                    else
                                    {
                                        sql_timer = thisAgent->EpMem->epmem_timers->query_sql_end_point;
                                    }
                                    break;
                            }
                            // create the SQL query and bind it
                            // try to find an existing query first; if none exist, allocate a new one from the memory pools
                            soar_module::pooled_sqlite_statement* interval_sql = NULL;
                            interval_sql = thisAgent->EpMem->epmem_stmts_graph->pool_find_interval_queries[pedge->value_is_id][point_type][interval_type]->request(sql_timer);
                            int bind_pos = 1;
                            if (point_type == EPMEM_RANGE_END && interval_type == EPMEM_RANGE_NOW)
                            {
                                interval_sql->bind_int(bind_pos++, current_episode);
                            }
                            interval_sql->bind_int(bind_pos++, edge_id);
                            interval_sql->bind_int(bind_pos++, current_episode);
                            if (interval_sql->execute() == soar_module::row)
                            {
                                epmem_interval* interval;
                                thisAgent->memoryManager->allocate_with_pool(MP_epmem_interval, &interval);
                                interval->is_end_point = point_type;
                                interval->uedge = uedge;
                                // If it's an start point of a range (ie. not a point) and it's before the promo time
                                // (this is possible if a the promotion is in the middle of a range)
                                // trim it to the promo time.
                                // This will only happen if the LTI is promoted in the last interval it appeared in
                                // (since otherwise the start point would not be before its promotion).
                                // We don't care about the remaining results of the query
                                interval->time = interval_sql->column_int(0);
                                interval->sql = interval_sql;
                                interval_pq.push(interval);
                                interval_cleanup.insert(interval);
                                uedge->intervals++;
                                created = true;
                            }
                            else
                            {
                                interval_sql->get_pool()->release(interval_sql);
                            }
                        }
                    }
                    if (created)
                    {
                        uedge->pedges.insert(pedge);
                        uedge_cache->insert(std::make_pair(triple, uedge));
                    }
                    else
                    {
                        uedge->pedges.~epmem_pedge_set();
                        thisAgent->memoryManager->free_with_pool(MP_epmem_uedge, uedge);
                    }
                }
                else
                {
                    epmem_uedge* uedge = (*uedge_iter).second;
                    uedge->pedges.insert(pedge);
                    if (uedge->activated && uedge->activation_count == 1)
                    {
                        for (epmem_literal_set::iterator lit_iter = pedge->literals.begin(); lit_iter != pedge->literals.end(); lit_iter++)
                        {
                            epmem_literal* literal = (*lit_iter);
                            changed_score |= epmem_satisfy_literal(literal, triple.parent_n_id, triple.child_n_id, current_score, current_cardinality, symbol_node_count, uedge_caches, symbol_num_incoming);
                        }
                    }
                }

                // put the partial edge query back into the queue if there's more
                // otherwise, reinitialize the query and put it in a pool
                if (pedge->sql && pedge->sql->execute() == soar_module::row)
                {
                    pedge->time = pedge->sql->column_int(2);
                    pedge_pq.push(pedge);
                }
                else if (pedge->sql)
                {
                    pedge->sql->get_pool()->release(pedge->sql);
                    pedge->sql = NULL;
                }
            }
            next_edge = (pedge_pq.empty() ? after : pedge_pq.top()->time);
            thisAgent->EpMem->epmem_timers->query_walk_edge->stop();

            // process all intervals before the next edge arrives
            thisAgent->EpMem->epmem_timers->query_walk_interval->start();
            while (interval_pq.size() && interval_pq.top()->time > next_edge && current_episode > after)
            {
                if (QUERY_DEBUG >= 1)
                {
                    std::cout << "EPISODE " << current_episode << std::endl;
                }
                // process all interval endpoints at this time step
                while (interval_pq.size() && interval_pq.top()->time >= current_episode)
                {
                    epmem_interval* interval = interval_pq.top();
                    interval_pq.pop();
                    epmem_uedge* uedge = interval->uedge;
                    epmem_triple triple = uedge->triple;
                    if (QUERY_DEBUG >= 1)
                    {
                        std::cout << "	INTERVAL (" << (interval->is_end_point ? "end" : "start") << " at time " << interval->time << "): " << triple.parent_n_id << "-" << triple.attribute_s_id << "-" << triple.child_n_id << std::endl;
                    }
                    if (interval->is_end_point)
                    {
                        uedge->activated = true;
                        uedge->activation_count++;
                        if (uedge->activation_count == 1)
                        {
                            for (epmem_pedge_set::iterator pedge_iter = uedge->pedges.begin(); pedge_iter != uedge->pedges.end(); pedge_iter++)
                            {
                                epmem_pedge* pedge = *pedge_iter;
                                for (epmem_literal_set::iterator lit_iter = pedge->literals.begin(); lit_iter != pedge->literals.end(); lit_iter++)
                                {
                                    epmem_literal* literal = *lit_iter;
                                    changed_score |= epmem_satisfy_literal(literal, triple.parent_n_id, triple.child_n_id, current_score, current_cardinality, symbol_node_count, uedge_caches, symbol_num_incoming);
                                }
                            }
                        }
                    }
                    else
                    {
                        uedge->activated = false;
                        uedge->activation_count--;
                        for (epmem_pedge_set::iterator pedge_iter = uedge->pedges.begin(); pedge_iter != uedge->pedges.end(); pedge_iter++)
                        {
                            epmem_pedge* pedge = *pedge_iter;
                            for (epmem_literal_set::iterator lit_iter = pedge->literals.begin(); lit_iter != pedge->literals.end(); lit_iter++)
                            {
                                changed_score |= epmem_unsatisfy_literal(*lit_iter, triple.parent_n_id, triple.child_n_id, current_score, current_cardinality, symbol_node_count);
                            }
                        }
                    }
                    // put the interval query back into the queue if there's more and some literal cares
                    // otherwise, reinitialize the query and put it in a pool
                    if (interval->sql && interval->sql->execute() == soar_module::row)
                    {
                        interval->time = interval->sql->column_int(0);
                        interval_pq.push(interval);
                    }
                    else if (interval->sql)
                    {
                        interval->sql->get_pool()->release(interval->sql);
                        interval->sql = NULL;
                        uedge->intervals--;
                        if (uedge->intervals)
                        {
                            interval_cleanup.erase(interval);
                            thisAgent->memoryManager->free_with_pool(MP_epmem_interval, interval);
                        }
                        else
                        {
                            // TODO JUSTIN retract intervals
                        }
                    }
                }
                next_interval = (interval_pq.empty() ? after : interval_pq.top()->time);
                next_episode = (next_edge > next_interval ? next_edge : next_interval);

                // update the prohibits list to catch up
                while (prohibits.size() && prohibits.back() > current_episode)
                {
                    prohibits.pop_back();
                }
                // ignore the episode if it is prohibited
                while (prohibits.size() && current_episode > next_episode && current_episode == prohibits.back())
                {
                    current_episode--;
                    prohibits.pop_back();
                }

                if (QUERY_DEBUG >= 2)
                {
                    epmem_print_retrieval_state(literal_cache, pedge_caches, uedge_caches);
                }

                print_sysparam_trace(thisAgent, TRACE_EPMEM_SYSPARAM, "Considering episode (time, cardinality, score) (%u, %u, %f)\n", static_cast<long long int>(current_episode), current_cardinality, current_score);

                // if
                // * the current time is still before any new intervals
                // * and the score was changed in this period
                // * and the new score is higher than the best score
                // then save the current time as the best one
                if (current_episode > next_episode && changed_score && (best_episode == EPMEM_MEMID_NONE || current_score > best_score || (do_graph_match && current_score == best_score && !best_graph_matched)))
                {
                    bool new_king = false;
                    if (best_episode == EPMEM_MEMID_NONE || current_score > best_score)
                    {
                        best_episode = current_episode;
                        best_score = current_score;
                        best_cardinality = current_cardinality;
                        new_king = true;
                    }
                    // we should graph match if the option is set and all leaf literals are satisfied
                    if (current_cardinality == perfect_cardinality)
                    {
                        bool graph_matched = false;
                        if (do_graph_match)
                        {
                            if (gm_order == epmem_param_container::gm_order_undefined)
                            {
                                std::sort(gm_ordering.begin(), gm_ordering.end());
                            }
                            else if (gm_order == epmem_param_container::gm_order_mcv)
                            {
                                std::sort(gm_ordering.begin(), gm_ordering.end(), epmem_gm_mcv_comparator);
                            }
                            epmem_literal_deque::iterator begin = gm_ordering.begin();
                            epmem_literal_deque::iterator end = gm_ordering.end();
                            best_bindings.clear();
                            epmem_node_symbol_map bound_nodes[2];
                            if (QUERY_DEBUG >= 1)
                            {
                                std::cout << "	GRAPH MATCH" << std::endl;
                                epmem_print_retrieval_state(literal_cache, pedge_caches, uedge_caches);
                            }
                            thisAgent->EpMem->epmem_timers->query_graph_match->start();
                            graph_matched = epmem_graph_match(begin, end, best_bindings, bound_nodes, thisAgent, 2);
                            thisAgent->EpMem->epmem_timers->query_graph_match->stop();
                        }
                        if (!do_graph_match || graph_matched)
                        {
                            best_episode = current_episode;
                            best_graph_matched = true;
                            current_episode = EPMEM_MEMID_NONE;
                            new_king = true;
                        }
                    }
                    if (new_king && thisAgent->trace_settings[TRACE_EPMEM_SYSPARAM])
                    {
                        char buf[256];
                        SNPRINTF(buf, 254, "NEW KING (perfect, graph-match): (%s, %s)\n", (current_cardinality == perfect_cardinality ? "true" : "false"), (best_graph_matched ? "true" : "false"));
                        thisAgent->outputManager->printa_sf(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }

                if (current_episode == EPMEM_MEMID_NONE)
                {
                    break;
                }
                else
                {
                    current_episode = next_episode;
                }
            }
            thisAgent->EpMem->epmem_timers->query_walk_interval->stop();
        }
        thisAgent->EpMem->epmem_timers->query_walk->stop();

        // if the best episode is the default, fail
        // otherwise, put the episode in working memory
        if (best_episode == EPMEM_MEMID_NONE)
        {
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_failure, pos_query);
            if (neg_query)
            {
                epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_failure, neg_query);
            }
        }
        else
        {
            thisAgent->EpMem->epmem_stats->qry_ret->set_value(best_episode);
            thisAgent->EpMem->epmem_stats->qry_card->set_value(best_cardinality);
            thisAgent->EpMem->epmem_timers->query_result->start();
            Symbol* temp_sym;
            epmem_id_mapping node_map_map;
            epmem_id_mapping node_mem_map;
            // cue size
            temp_sym = thisAgent->symbolManager->make_int_constant(leaf_literals.size());
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_cue_size, temp_sym);
            thisAgent->symbolManager->symbol_remove_ref(&temp_sym);
            // match cardinality
            temp_sym = thisAgent->symbolManager->make_int_constant(best_cardinality);
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_match_cardinality, temp_sym);
            thisAgent->symbolManager->symbol_remove_ref(&temp_sym);
            // match score
            temp_sym = thisAgent->symbolManager->make_float_constant(best_score);
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_match_score, temp_sym);
            thisAgent->symbolManager->symbol_remove_ref(&temp_sym);
            // normalized match score
            temp_sym = thisAgent->symbolManager->make_float_constant(best_score / perfect_score);
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_normalized_match_score, temp_sym);
            thisAgent->symbolManager->symbol_remove_ref(&temp_sym);
            // status
            epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_success, pos_query);
            if (neg_query)
            {
                epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_success, neg_query);
            }
            // give more metadata if graph match is turned on
            if (do_graph_match)
            {
                // graph match
                temp_sym = thisAgent->symbolManager->make_int_constant((best_graph_matched ? 1 : 0));
                epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_graph_match, temp_sym);
                thisAgent->symbolManager->symbol_remove_ref(&temp_sym);

                // mapping
                if (best_graph_matched)
                {
                    goal_stack_level level = state->id->epmem_info->result_wme->value->id->level;
                    // mapping identifier
                    Symbol* mapping = thisAgent->symbolManager->make_new_identifier('M', level);
                    epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_graph_match_mapping, mapping);
                    thisAgent->symbolManager->symbol_remove_ref(&mapping);

                    for (epmem_literal_node_pair_map::iterator iter = best_bindings.begin(); iter != best_bindings.end(); iter++)
                    {
                        if ((*iter).first->value_is_id)
                        {
                            // create the node
                            temp_sym = thisAgent->symbolManager->make_new_identifier('N', level);
                            epmem_buffer_add_wme(thisAgent, meta_wmes, mapping, thisAgent->symbolManager->soarSymbols.epmem_sym_graph_match_mapping_node, temp_sym);
                            thisAgent->symbolManager->symbol_remove_ref(&temp_sym);
                            // point to the cue identifier
                            epmem_buffer_add_wme(thisAgent, meta_wmes, temp_sym, thisAgent->symbolManager->soarSymbols.epmem_sym_graph_match_mapping_cue, (*iter).first->value_sym);
                            // save the mapping point for the episode
                            node_map_map[(*iter).second.second] = temp_sym;
                            node_mem_map[(*iter).second.second] = NULL;
                        }
                    }
                }
            }
            // reconstruct the actual episode
            if (level > 2)
            {
                epmem_install_memory(thisAgent, state, best_episode, meta_wmes, retrieval_wmes, &node_mem_map);
            }
            if (best_graph_matched)
            {
                for (epmem_id_mapping::iterator iter = node_mem_map.begin(); iter != node_mem_map.end(); iter++)
                {
                    epmem_id_mapping::iterator map_iter = node_map_map.find((*iter).first);
                    if (map_iter != node_map_map.end() && (*iter).second)
                    {
                        epmem_buffer_add_wme(thisAgent, meta_wmes, (*map_iter).second, thisAgent->symbolManager->soarSymbols.epmem_sym_retrieved, (*iter).second);
                    }
                }
            }
            thisAgent->EpMem->epmem_timers->query_result->stop();
        }
    }

    // cleanup
    thisAgent->EpMem->epmem_timers->query_cleanup->start();
    for (epmem_interval_set::iterator iter = interval_cleanup.begin(); iter != interval_cleanup.end(); iter++)
    {
        epmem_interval* interval = *iter;
        if (interval->sql)
        {
            interval->sql->get_pool()->release(interval->sql);
        }
        thisAgent->memoryManager->free_with_pool(MP_epmem_interval, interval);
    }
    for (int type = EPMEM_RIT_STATE_NODE; type <= EPMEM_RIT_STATE_EDGE; type++)
    {
        for (epmem_triple_pedge_map::iterator iter = pedge_caches[type].begin(); iter != pedge_caches[type].end(); iter++)
        {
            epmem_pedge* pedge = (*iter).second;
            if (pedge->sql)
            {
                pedge->sql->get_pool()->release(pedge->sql);
            }
            pedge->literals.~epmem_literal_set();
            thisAgent->memoryManager->free_with_pool(MP_epmem_pedge, pedge);
        }
        for (epmem_triple_uedge_map::iterator iter = uedge_caches[type].begin(); iter != uedge_caches[type].end(); iter++)
        {
            epmem_uedge* uedge = (*iter).second;
            uedge->pedges.~epmem_pedge_set();
            thisAgent->memoryManager->free_with_pool(MP_epmem_uedge, uedge);
        }
    }
    for (epmem_wme_literal_map::iterator iter = literal_cache.begin(); iter != literal_cache.end(); iter++)
    {
        epmem_literal* literal = (*iter).second;
        literal->parents.~epmem_literal_set();
        literal->children.~epmem_literal_set();
        literal->matches.~epmem_node_pair_set();
        literal->values.~epmem_node_int_map();
        thisAgent->memoryManager->free_with_pool(MP_epmem_literal, literal);
    }
    thisAgent->EpMem->epmem_timers->query_cleanup->stop();

    thisAgent->EpMem->epmem_timers->query->stop();
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Visualization (epmem::viz)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

inline std::string _epmem_print_sti(epmem_node_id id)
{
    std::string t1, t2;

    t1.assign("<id");

    to_string(id, t2);
    t1.append(t2);
    t1.append(">");

    return t1;
}

void epmem_print_episode(agent* thisAgent, epmem_time_id memory_id, std::string* buf)
{
    epmem_attach(thisAgent);

    // if bad memory, bail
    buf->clear();
    if ((memory_id == EPMEM_MEMID_NONE) ||
            !epmem_valid_episode(thisAgent, memory_id))
    {
        return;
    }

    // fill episode map
    std::map< epmem_node_id, std::string > ltis;
    std::map< epmem_node_id, std::map< std::string, std::list< std::string > > > ep;
    {
        soar_module::sqlite_statement* my_q;
        std::string temp_s, temp_s2, temp_s3;
        int64_t temp_i;

        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_identifier_values;
        {
            epmem_node_id parent_n_id;
            epmem_node_id child_n_id;
            bool val_is_short_term;

            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ]));

            // query for edges
            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            my_q->bind_int(5, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                // parent_n_id, attribute_s_id, child_n_id, epmem_node.lti_id
                parent_n_id = my_q->column_int(0);
                child_n_id = my_q->column_int(2);

                epmem_reverse_hash_print(thisAgent, my_q->column_int(1), temp_s);

                val_is_short_term = (my_q->column_type(3) == soar_module::null_t || my_q->column_int(3) == 0);
                if (val_is_short_term)
                {
                    temp_s2 = _epmem_print_sti(child_n_id);
                }
                else
                {
                    temp_s2.assign("@");
                    temp_i = static_cast< uint64_t >(my_q->column_int(3));
                    to_string(temp_i, temp_s3);
                    temp_s2.append(temp_s3);

                    ltis[ child_n_id ] = temp_s2;
                }

                ep[ parent_n_id ][ temp_s ].push_back(temp_s2);
            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);
        }

        // f.wc_id, f.parent_n_id, f.attribute_s_id, f.value_s_id
        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_constant_values;
        {
            epmem_node_id parent_n_id;

            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ]));

            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                parent_n_id = my_q->column_int(1);
//              fprintf(stderr, "PRINTING %d %d %d\n", (unsigned int) parent_n_id, (unsigned int) my_q->column_int( 2 ), (unsigned int) my_q->column_int( 3 ));
                epmem_reverse_hash_print(thisAgent, my_q->column_int(2), temp_s);
//              fprintf(stderr, "  - Attribute is %s\n", temp_s.data());
                epmem_reverse_hash_print(thisAgent, my_q->column_int(3), temp_s2);
//              fprintf(stderr, "  - Value is %s\n", temp_s2.data());

                ep[ parent_n_id ][ temp_s ].push_back(temp_s2);
            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);
        }
    }

    // output
    {
        std::map< epmem_node_id, std::string >::iterator lti_it;
        std::map< epmem_node_id, std::map< std::string, std::list< std::string > > >::iterator ep_it;
        std::map< std::string, std::list< std::string > >::iterator slot_it;
        std::list< std::string >::iterator val_it;

        for (ep_it = ep.begin(); ep_it != ep.end(); ep_it++)
        {
            buf->append("(");

            // id
            lti_it = ltis.find(ep_it->first);
            if (lti_it == ltis.end())
            {
                buf->append(_epmem_print_sti(ep_it->first));
            }
            else
            {
                buf->append(lti_it->second);
            }

            // attr
            for (slot_it = ep_it->second.begin(); slot_it != ep_it->second.end(); slot_it++)
            {
                buf->append(" ^");
                buf->append(slot_it->first);

                for (val_it = slot_it->second.begin(); val_it != slot_it->second.end(); val_it++)
                {
                    buf->append(" ");
                    buf->append(*val_it);
                }
            }

            buf->append(")\n");
        }
    }
}

void epmem_visualize_episode(agent* thisAgent, epmem_time_id memory_id, std::string* buf)
{
    epmem_attach(thisAgent);

    // if bad memory, bail
    buf->clear();
    if ((memory_id == EPMEM_MEMID_NONE) ||
            !epmem_valid_episode(thisAgent, memory_id))
    {
        return;
    }

    // init
    {
        buf->append("digraph epmem {\n");
    }

    // taken heavily from install
    {
        soar_module::sqlite_statement* my_q;

        // first identifiers (i.e. reconstruct)
        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_identifier_values;
        {
            // for printing
            std::map< epmem_node_id, std::string > stis;
            std::map< epmem_node_id, std::pair< std::string, std::string > > ltis;
            std::list< std::string > edges;
            std::map< epmem_node_id, std::string >::iterator sti_p;
            std::map< epmem_node_id, std::pair< std::string, std::string > >::iterator lti_p;

            // relates to finite automata: child_n_id = d(parent_n_id, w)
            epmem_node_id parent_n_id; // id
            epmem_node_id child_n_id; // attribute
            std::string temp, temp2, temp3, temp4;

            bool val_is_short_term;
            uint64_t val_num;

            // 0 is magic
            temp.assign("ID_0");
            stis.insert(std::make_pair(epmem_node_id(0), temp));

            // prep rit
            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_EDGE ]));

            // query for edges
            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            my_q->bind_int(5, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                // parent_n_id, attribute_s_id, child_n_id, epmem_node.lti_id
                parent_n_id = my_q->column_int(0);
                child_n_id = my_q->column_int(2);

                // "ID_parent_n_id"
                temp.assign("ID_");
                to_string(parent_n_id, temp2);
                temp.append(temp2);

                // "ID_child_n_id"
                temp3.assign("ID_");
                to_string(child_n_id, temp2);
                temp3.append(temp2);

                val_is_short_term = (my_q->column_type(3) == soar_module::null_t || my_q->column_int(3) == 0);
                if (val_is_short_term)
                {
                    sti_p = stis.find(child_n_id);
                    if (sti_p == stis.end())
                    {
                        stis.insert(std::make_pair(child_n_id, temp3));
                    }
                }
                else
                {
                    lti_p = ltis.find(child_n_id);

                    if (lti_p == ltis.end())
                    {
                        // "L#"
                        temp4 = "@";
                        val_num = static_cast<uint64_t>(my_q->column_int(3));
                        to_string(val_num, temp2);
                        temp4.append(temp2);

                        ltis.insert(std::make_pair(child_n_id, std::make_pair(temp3, temp4)));
                    }
                }

                // " -> ID_child_n_id"
                temp.append(" -> ");
                temp.append(temp3);

                // " [ label="attribute_s_id" ];\n"
                temp.append(" [ label=\"");
                epmem_reverse_hash_print(thisAgent, my_q->column_int(1), temp2);
                temp.append(temp2);
                temp.append("\" ];\n");

                edges.push_back(temp);
            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);

            // identifiers
            {
                // short-term
                {
                    buf->append("node [ shape = circle ];\n");

                    for (sti_p = stis.begin(); sti_p != stis.end(); sti_p++)
                    {
                        buf->append(sti_p->second);
                        buf->append(" ");
                    }

                    buf->append(";\n");
                }

                // long-term
                {
                    buf->append("node [ shape = doublecircle ];\n");

                    for (lti_p = ltis.begin(); lti_p != ltis.end(); lti_p++)
                    {
                        buf->append(lti_p->second.first);
                        buf->append(" [ label=\"");
                        buf->append(lti_p->second.second);
                        buf->append("\" ];\n");
                    }

                    buf->append("\n");
                }
            }

            // edges
            {
                std::list< std::string >::iterator e_p;

                for (e_p = edges.begin(); e_p != edges.end(); e_p++)
                {
                    buf->append((*e_p));
                }
            }
        }

        // then epmem_wmes_constant

        my_q = thisAgent->EpMem->epmem_stmts_graph->get_wmes_with_constant_values;
        {
            epmem_node_id wc_id;
            epmem_node_id parent_n_id;

            std::list< std::string > edges;
            std::list< std::string > consts;

            std::string temp, temp2;

            epmem_rit_prep_left_right(thisAgent, memory_id, memory_id, &(thisAgent->EpMem->epmem_rit_state_graph[ EPMEM_RIT_STATE_NODE ]));

            my_q->bind_int(1, memory_id);
            my_q->bind_int(2, memory_id);
            my_q->bind_int(3, memory_id);
            my_q->bind_int(4, memory_id);
            while (my_q->execute() == soar_module::row)
            {
                // f.wc_id, f.parent_n_id, f.attribute_s_id, f.value_s_id
                wc_id = my_q->column_int(0);
                parent_n_id = my_q->column_int(1);

                temp.assign("ID_");
                to_string(parent_n_id, temp2);
                temp.append(temp2);
                temp.append(" -> C_");
                to_string(wc_id, temp2);
                temp.append(temp2);
                temp.append(" [ label=\"");
                epmem_reverse_hash_print(thisAgent, my_q->column_int(2), temp2);
                temp.append(temp2);
                temp.append("\" ];\n");
                edges.push_back(temp);

                temp.assign("C_");
                to_string(wc_id, temp2);
                temp.append(temp2);
                temp.append(" [ label=\"");
                epmem_reverse_hash_print(thisAgent, my_q->column_int(3), temp2);
                temp.append(temp2);
                temp.append("\" ];\n");

                consts.push_back(temp);

            }
            my_q->reinitialize();
            epmem_rit_clear_left_right(thisAgent);

            // constant nodes
            {
                std::list< std::string >::iterator e_p;

                buf->append("node [ shape = plaintext ];\n");

                for (e_p = consts.begin(); e_p != consts.end(); e_p++)
                {
                    buf->append((*e_p));
                }
            }

            // edges
            {
                std::list< std::string >::iterator e_p;

                for (e_p = edges.begin(); e_p != edges.end(); e_p++)
                {
                    buf->append((*e_p));
                }
            }
        }
    }

    // close
    {
        buf->append("\n}\n");
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// API Implementation (epmem::api)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : epmem_consider_new_episode
 * Author       : Nate Derbinsky
 * Notes        : Based upon trigger/force parameter settings, potentially
 *                records a new episode
 **************************************************************************/
bool epmem_consider_new_episode(agent* thisAgent)
{
    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->trigger->start();
    ////////////////////////////////////////////////////////////////////////////

    const int64_t force = thisAgent->EpMem->epmem_params->force->get_value();
    bool new_memory = false;

    if (force == epmem_param_container::force_off)
    {
        const int64_t trigger = thisAgent->EpMem->epmem_params->trigger->get_value();

        if (trigger == epmem_param_container::output)
        {
            slot* s;
            wme* w;
            Symbol* ol = thisAgent->io_header_output;

            // examine all commands on the output-link for any
            // that appeared since last memory was recorded
            for (s = ol->id->slots; s != NIL; s = s->next)
            {
                for (w = s->wmes; w != NIL; w = w->next)
                {
                    if (w->timetag > thisAgent->top_goal->id->epmem_info->last_ol_time)
                    {
                        new_memory = true;
                        thisAgent->top_goal->id->epmem_info->last_ol_time = w->timetag;
                    }
                }
            }
        }
        else if (trigger == epmem_param_container::dc)
        {
            new_memory = true;
        }
        else if (trigger == epmem_param_container::none)
        {
            new_memory = false;
        }
    }
    else
    {
        new_memory = (force == epmem_param_container::remember);

        thisAgent->EpMem->epmem_params->force->set_value(epmem_param_container::force_off);
    }

    ////////////////////////////////////////////////////////////////////////////
    thisAgent->EpMem->epmem_timers->trigger->stop();
    ////////////////////////////////////////////////////////////////////////////

    if (new_memory)
    {
        epmem_new_episode(thisAgent);
    }

    return new_memory;
}

void inline _epmem_respond_to_cmd_parse(agent* thisAgent, epmem_wme_list* cmds, bool& good_cue, int& path, epmem_time_id& retrieve, Symbol*& next, Symbol*& previous, Symbol*& query, Symbol*& neg_query, epmem_time_list& prohibit, epmem_time_id& before, epmem_time_id& after, wme_set& cue_wmes)
{
    cue_wmes.clear();

    retrieve = EPMEM_MEMID_NONE;
    next = NULL;
    previous = NULL;
    query = NULL;
    neg_query = NULL;
    prohibit.clear();
    before = EPMEM_MEMID_NONE;
    after = EPMEM_MEMID_NONE;
    good_cue = true;
    path = 0;

    for (epmem_wme_list::iterator w_p = cmds->begin(); w_p != cmds->end(); w_p++)
    {
        cue_wmes.insert((*w_p));

        if (good_cue)
        {
            // collect information about known commands
            if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_retrieve)
            {
                if (((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE) &&
                        (path == 0) &&
                        ((*w_p)->value->ic->value > 0))
                {
                    retrieve = (*w_p)->value->ic->value;
                    path = 1;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_next)
            {
                if (((*w_p)->value->is_sti()) &&
                        (path == 0))
                {
                    next = (*w_p)->value;
                    path = 2;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_prev)
            {
                if (((*w_p)->value->is_sti()) &&
                        (path == 0))
                {
                    previous = (*w_p)->value;
                    path = 2;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_query)
            {
                if (((*w_p)->value->is_sti()) &&
                        ((path == 0) || (path == 3)) &&
                        (query == NULL))

                {
                    query = (*w_p)->value;
                    path = 3;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_negquery)
            {
                if (((*w_p)->value->is_sti()) &&
                        ((path == 0) || (path == 3)) &&
                        (neg_query == NULL))

                {
                    neg_query = (*w_p)->value;
                    path = 3;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_before)
            {
                if (((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE) &&
                        ((path == 0) || (path == 3)))
                {
                    if ((before == EPMEM_MEMID_NONE) || (static_cast<epmem_time_id>((*w_p)->value->ic->value) < before))
                    {
                        before = (*w_p)->value->ic->value;
                    }
                    path = 3;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_after)
            {
                if (((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE) &&
                        ((path == 0) || (path == 3)))
                {
                    if (after < static_cast<epmem_time_id>((*w_p)->value->ic->value))
                    {
                        after = (*w_p)->value->ic->value;
                    }
                    path = 3;
                }
                else
                {
                    good_cue = false;
                }
            }
            else if ((*w_p)->attr == thisAgent->symbolManager->soarSymbols.epmem_sym_prohibit)
            {
                if (((*w_p)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE) &&
                        ((path == 0) || (path == 3)))
                {
                    prohibit.push_back((*w_p)->value->ic->value);
                    path = 3;
                }
                else
                {
                    good_cue = false;
                }
            }
            else
            {
                good_cue = false;
            }
        }
    }

    // if on path 3 must have query
    if ((path == 3) && (query == NULL))
    {
        good_cue = false;
    }

    // must be on a path
    if (path == 0)
    {
        good_cue = false;
    }
}

/***************************************************************************
 * Function     : epmem_respond_to_cmd
 * Author       : Nate Derbinsky
 * Notes        : Implements the Soar-EpMem API
 **************************************************************************/
void epmem_respond_to_cmd(agent* thisAgent)
{
    epmem_attach(thisAgent);

    // respond to query only if db is properly initialized
    if (thisAgent->EpMem->epmem_db->get_status() != soar_module::connected)
    {
        return;
    }

    // start at the bottom and work our way up
    // (could go in the opposite direction as well)
    Symbol* state = thisAgent->bottom_goal;

    epmem_wme_list* wmes;
    epmem_wme_list* cmds;
    epmem_wme_list::iterator w_p;

    wme_set cue_wmes;
    symbol_triple_list meta_wmes;
    symbol_triple_list retrieval_wmes;

    epmem_time_id retrieve;
    Symbol* next;
    Symbol* previous;
    Symbol* query;
    Symbol* neg_query;
    epmem_time_list prohibit;
    epmem_time_id before, after;
    bool good_cue;
    int path;

    uint64_t wme_count;
    bool new_cue;

    bool do_wm_phase = false;

    while (state != NULL)
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->EpMem->epmem_timers->api->start();
        ////////////////////////////////////////////////////////////////////////////
        // make sure this state has had some sort of change to the cmd
        new_cue = false;
        wme_count = 0;
        cmds = NULL;
        {
            tc_number tc = get_new_tc_number(thisAgent);
            std::queue<Symbol*> syms;
            Symbol* parent_sym;

            // initialize BFS at command
            syms.push(state->id->epmem_info->cmd_wme->value);

            while (!syms.empty())
            {
                // get state
                parent_sym = syms.front();
                syms.pop();

                // get children of the current identifier
                wmes = epmem_get_augs_of_id(parent_sym, tc);
                {
                    for (w_p = wmes->begin(); w_p != wmes->end(); w_p++)
                    {
                        wme_count++;

                        if ((*w_p)->timetag > state->id->epmem_info->last_cmd_time)
                        {
                            new_cue = true;
                            state->id->epmem_info->last_cmd_time = (*w_p)->timetag;
                        }

                        if ((*w_p)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                        {
                            syms.push((*w_p)->value);
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
            if (state->id->epmem_info->last_cmd_count != wme_count)
            {
                new_cue = true;
                state->id->epmem_info->last_cmd_count = wme_count;
            }

            if (new_cue)
            {
                // clear old results
                epmem_clear_result(thisAgent, state);

                do_wm_phase = true;
            }
        }

        // a command is issued if the cue is new
        // and there is something on the cue
        if (new_cue && wme_count)
        {
            _epmem_respond_to_cmd_parse(thisAgent, cmds, good_cue, path, retrieve, next, previous, query, neg_query, prohibit, before, after, cue_wmes);

            ////////////////////////////////////////////////////////////////////////////
            thisAgent->EpMem->epmem_timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////

            retrieval_wmes.clear();
            meta_wmes.clear();

            // process command
            if (good_cue)
            {
                thisAgent->explanationBasedChunker->clear_symbol_identity_map();

                // retrieve
                if (path == 1)
                {
                    epmem_install_memory(thisAgent, state, retrieve, meta_wmes, retrieval_wmes);

                    // add one to the ncbr stat
                    thisAgent->EpMem->epmem_stats->ncbr->set_value(thisAgent->EpMem->epmem_stats->ncbr->get_value() + 1);
                }
                // previous or next
                else if (path == 2)
                {
                    if (next)
                    {
                        epmem_install_memory(thisAgent, state, epmem_next_episode(thisAgent, state->id->epmem_info->last_memory), meta_wmes, retrieval_wmes);

                        // add one to the next stat
                        thisAgent->EpMem->epmem_stats->nexts->set_value(thisAgent->EpMem->epmem_stats->nexts->get_value() + 1);
                    }
                    else
                    {
                        epmem_install_memory(thisAgent, state, epmem_previous_episode(thisAgent, state->id->epmem_info->last_memory), meta_wmes, retrieval_wmes);

                        // add one to the prev stat
                        thisAgent->EpMem->epmem_stats->prevs->set_value(thisAgent->EpMem->epmem_stats->prevs->get_value() + 1);
                    }

                    if (state->id->epmem_info->last_memory == EPMEM_MEMID_NONE)
                    {
                        epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_failure, ((next) ? (next) : (previous)));
                    }
                    else
                    {
                        epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_success, ((next) ? (next) : (previous)));
                    }
                }
                // query
                else if (path == 3)
                {
                    epmem_process_query(thisAgent, state, query, neg_query, prohibit, before, after, cue_wmes, meta_wmes, retrieval_wmes);

                    // add one to the cbr stat
                    thisAgent->EpMem->epmem_stats->cbr->set_value(thisAgent->EpMem->epmem_stats->cbr->get_value() + 1);
                }
            }
            else
            {
                epmem_buffer_add_wme(thisAgent, meta_wmes, state->id->epmem_info->result_wme->value, thisAgent->symbolManager->soarSymbols.epmem_sym_status, thisAgent->symbolManager->soarSymbols.epmem_sym_bad_cmd);
            }

            // clear prohibit list
            prohibit.clear();

            if (!retrieval_wmes.empty() || !meta_wmes.empty())
            {
                // process preference assertion en masse
                epmem_process_buffered_wmes(thisAgent, state, cue_wmes, meta_wmes, retrieval_wmes);

                // clear cache
                {
                    symbol_triple_list::iterator mw_it;

                    for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);

                        delete(*mw_it);
                    }
                    retrieval_wmes.clear();

                    for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
                    {
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                        thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);

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
            thisAgent->EpMem->epmem_timers->api->stop();
            ////////////////////////////////////////////////////////////////////////////
        }

        // free space from command aug list
        if (cmds)
        {
            delete cmds;
        }

        state = state->id->higher_goal;
    }

    if (do_wm_phase)
    {
        ////////////////////////////////////////////////////////////////////////////
        thisAgent->EpMem->epmem_timers->wm_phase->start();
        ////////////////////////////////////////////////////////////////////////////

        do_working_memory_phase(thisAgent);

        ////////////////////////////////////////////////////////////////////////////
        thisAgent->EpMem->epmem_timers->wm_phase->stop();
        ////////////////////////////////////////////////////////////////////////////
    }
}

/***************************************************************************
 * Function     : epmem_go
 * Author       : Nate Derbinsky
 * Notes        : The kernel calls this function to implement Soar-EpMem:
 *                consider new storage and respond to any commands
 **************************************************************************/
void epmem_go(agent* thisAgent, bool allow_store)
{

    thisAgent->EpMem->epmem_timers->total->start();

    if (allow_store)
    {
        epmem_consider_new_episode(thisAgent);
    }
    epmem_respond_to_cmd(thisAgent);


    thisAgent->EpMem->epmem_timers->total->stop();

}

bool epmem_backup_db(agent* thisAgent, const char* file_name, std::string* err)
{
    bool return_val = false;

    if (thisAgent->EpMem->epmem_db->get_status() == soar_module::connected)
    {
        if (thisAgent->EpMem->epmem_params->lazy_commit->get_value() == on)
        {
            thisAgent->EpMem->epmem_stmts_common->commit->execute(soar_module::op_reinit);
        }

        return_val = thisAgent->EpMem->epmem_db->backup(file_name, err);

        if (thisAgent->EpMem->epmem_params->lazy_commit->get_value() == on)
        {
            thisAgent->EpMem->epmem_stmts_common->begin->execute(soar_module::op_reinit);
        }
    }
    else
    {
        err->assign("Episodic database is not currently connected.");
    }

    return return_val;
}

EpMem_Manager::EpMem_Manager(agent* myAgent)
{
    thisAgent = myAgent;

    thisAgent->EpMem = this;

    // epmem initialization
     epmem_params = new epmem_param_container(thisAgent);
     epmem_stats = new epmem_stat_container(thisAgent);
     epmem_timers = new epmem_timer_container(thisAgent);

     epmem_db = new soar_module::sqlite_database();
     epmem_stmts_common = NULL;
     epmem_stmts_graph = NULL;

     epmem_node_mins = new std::vector<epmem_time_id>();
     epmem_node_maxes = new std::vector<bool>();

     epmem_edge_mins = new std::vector<epmem_time_id>();
     epmem_edge_maxes = new std::vector<bool>();
     epmem_id_repository = new epmem_parent_id_pool();
     epmem_id_replacement = new epmem_return_id_pool();
     epmem_id_ref_counts = new epmem_id_ref_counter();

 #ifdef USE_MEM_POOL_ALLOCATORS
     epmem_node_removals = new epmem_id_removal_map(std::less< epmem_node_id >(), soar_module::soar_memory_pool_allocator< std::pair< std::pair<epmem_node_id const,int64_t> const, bool > >(thisAgent));
     epmem_edge_removals = new epmem_edge_removal_map(std::less< std::pair<epmem_node_id const,int64_t> >(), soar_module::soar_memory_pool_allocator< std::pair< epmem_node_id const, bool > >(thisAgent));
     epmem_wme_adds = new epmem_symbol_set(std::less< Symbol* >(), soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
     epmem_id_removes = new epmem_symbol_stack(soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
 #else
     epmem_node_removals = new epmem_id_removal_map();
     epmem_edge_removals = new epmem_edge_removal_map();
     epmem_wme_adds = new epmem_symbol_set();
     epmem_id_removes = new epmem_symbol_stack();
 #endif

     epmem_validation = 0;

};

void EpMem_Manager::clean_up_for_agent_deletion()
{
    /* This is not in destructor because it may be called before other
     * deletion code that may need params, stats or timers to exist */
    // cleanup exploration

    epmem_close(thisAgent);
    delete epmem_params;
    delete epmem_stats;
    delete epmem_timers;

    delete epmem_node_removals;
    delete epmem_node_mins;
    delete epmem_node_maxes;
    delete epmem_edge_removals;
    delete epmem_edge_mins;
    delete epmem_edge_maxes;
    delete epmem_id_repository;
    delete epmem_id_replacement;
    delete epmem_id_ref_counts;
    delete epmem_id_removes;

    delete epmem_wme_adds;

    delete epmem_db;
}
void epmem_param_container::print_settings(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

//    outputManager->reset_column_indents();
//    outputManager->set_column_indent(0, 25);
//    outputManager->set_column_indent(1, 58);
//    outputManager->printa(thisAgent, "=======================================================\n");
//    outputManager->printa(thisAgent, "-      Episodic Memory Sub-Commands and Options       -\n");
//    outputManager->printa(thisAgent, "=======================================================\n");
//    outputManager->printa_sf(thisAgent, "%s   %-\n", concatJustified("enabled",learning->get_string(), 55).c_str());
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("database", database->get_string(), 55).c_str(), "Whether to store database in memory or file");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("append", append_db->get_string(), 55).c_str(), "Whether to append or overwrite database");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("path", path->get_string(), 55).c_str(), "Path of database file");
//    outputManager->printa(thisAgent, "-------------------------------------------------------\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem", "[? | help]", 55).c_str(), "Print this help screen");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem", "[--enable | --disable ]", 55).c_str(), "Enable/disable semantic memory");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem [--get | --set] ","<option> [<value>]", 55).c_str(), "Print or set value of an SMem parameter");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --add","{ (id ^attr value)* }", 55).c_str(), "Add concepts to semantic memory");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --backup","<filename>", 55).c_str(), "Creates a backup of the semantic database on disk");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --init ","", 55).c_str(), "Reinitialize ALL memories");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --query ","{(<cue>*} [<num>]}", 55).c_str(), "Query for concepts in semantic store matching some cue");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --remove","{ (id [^attr [value]])* }", 55).c_str(), "Remove concepts from semantic memory");
//    outputManager->printa(thisAgent, "------------------------ Printing ---------------------\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("print","@", 55).c_str(), "Print semantic memory store");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("print","<LTI>", 55).c_str(), "Print specific semantic memory");
//    outputManager->printa(thisAgent, "---------------------- Activation --------------------\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --history","<LTI>", 55).c_str(), "Print activation history for some LTM");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("activation-mode", activation_mode->get_string(), 55).c_str(), "recency, frequency, base-level");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("activate-on-query", activate_on_query->get_string(), 55).c_str(), "on, off");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-decay", base_decay->get_string(), 55).c_str(), "Decay parameter for base-level activation computation");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-update-policy", base_update->get_string(), 55).c_str(), "stable, naive, incremental");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("base-incremental-threshes", base_incremental_threshes->get_string(), 55).c_str(), "integer > 0");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("thresh", thresh->get_string(), 55).c_str(), "integer >= 0");
//    outputManager->printa(thisAgent, "------------- Database Optimization Settings ----------\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("lazy-commit", lazy_commit->get_string(), 55).c_str(), "Delay writing semantic store until exit");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("optimization", opt->get_string(), 55).c_str(), "safety, performance");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("cache-size", cache_size->get_string(), 55).c_str(), "Number of memory pages used for SQLite cache");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("page-size", page_size->get_string(), 55).c_str(), "Size of each memory page used");
//    outputManager->printa(thisAgent, "----------------- Timers and Statistics ---------------\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("timers", timers->get_string(), 55).c_str(), "Timer granularity (off, one, two, three)");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --timers ","[<timer>]", 55).c_str(), "Print timer summary or specific statistic:");
//    outputManager->printa_sf(thisAgent, "%-%- (_total, smem_api, smem_hash, smem_init, smem_query)\n");
//    outputManager->printa_sf(thisAgent, "%-%- (smem_ncb_retrieval, smem_storage, three_activation)\n");
//    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("smem --stats","[<stat>]", 55).c_str(), "Print statistic summary or specific statistic:");
//    outputManager->printa_sf(thisAgent, "%-%- (act_updates, db-lib-version, edges, mem-usage)\n");
//    outputManager->printa_sf(thisAgent, "%-%- (mem-high, nodes, queries, retrieves, stores)\n");
//    outputManager->printa(thisAgent, "-------------------------------------------------------\n\n");
//    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%- help output\n");

    // Print Epmem Settings
//    PrintCLIMessage_Header("Episodic Memory Settings", 40);
//    PrintCLIMessage_Item("learning:", thisAgent->EpMem->epmem_params->learning, 40);
//    PrintCLIMessage_Section("Encoding", 40);
//    PrintCLIMessage_Item("phase:", thisAgent->EpMem->epmem_params->phase, 40);
//    PrintCLIMessage_Item("trigger:", thisAgent->EpMem->epmem_params->trigger, 40);
//    PrintCLIMessage_Item("force:", thisAgent->EpMem->epmem_params->force, 40);
//    PrintCLIMessage_Item("exclusions:", thisAgent->EpMem->epmem_params->exclusions, 40);
//    PrintCLIMessage_Section("Storage", 40);
//    PrintCLIMessage_Item("database:", thisAgent->EpMem->epmem_params->database, 40);
//    PrintCLIMessage_Item("append:", thisAgent->EpMem->epmem_params->append_db, 40);
//    PrintCLIMessage_Item("path:", thisAgent->EpMem->epmem_params->path, 40);
//    PrintCLIMessage_Item("lazy-commit:", thisAgent->EpMem->epmem_params->lazy_commit, 40);
//    PrintCLIMessage_Section("Retrieval", 40);
//    PrintCLIMessage_Item("balance:", thisAgent->EpMem->epmem_params->balance, 40);
//    PrintCLIMessage_Item("graph-match:", thisAgent->EpMem->epmem_params->graph_match, 40);
//    PrintCLIMessage_Item("graph-match-ordering:", thisAgent->EpMem->epmem_params->gm_ordering, 40);
//    PrintCLIMessage_Section("Performance", 40);
//    PrintCLIMessage_Item("page-size:", thisAgent->EpMem->epmem_params->page_size, 40);
//    PrintCLIMessage_Item("cache-size:", thisAgent->EpMem->epmem_params->cache_size, 40);
//    PrintCLIMessage_Item("optimization:", thisAgent->EpMem->epmem_params->opt, 40);
//    PrintCLIMessage_Item("timers:", thisAgent->EpMem->epmem_params->timers, 40);
//    PrintCLIMessage_Section("Experimental", 40);
//    PrintCLIMessage_Item("merge:", thisAgent->EpMem->epmem_params->merge, 40);
//    PrintCLIMessage("");
}

void epmem_param_container::print_summary(agent* thisAgent)
{
    std::string tempString;
    Output_Manager* outputManager = thisAgent->outputManager;

    std::string lStr("Episodic memory is ");
    lStr.append(thisAgent->EpMem->epmem_params->learning->get_value() ? "enabled." : "not enabled.");
//    PrintCLIMessage(lStr.c_str());
//    PrintCLIMessage("Use 'epmem ?' to see EpMem setting and 'help epmem' to learn more about the epmem command.");
}
