#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* utilities.cpp */

#include "agent.h"
#include "misc.h"
#include "stats.h"

#include "wmem.h"
#include "print.h"
#include "xml.h"

#include <time.h>

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
void init_real_time(agent* thisAgent)
{
    thisAgent->real_time_tracker =
        (struct timeval*) malloc(sizeof(struct timeval));
    timerclear(thisAgent->real_time_tracker);
    thisAgent->real_time_idling = false;
    current_real_time =
        (struct timeval*) malloc(sizeof(struct timeval));
}
void test_for_input_delay(agent* thisAgent)
{
    /* RMJ; For real-time behavior, don't start any new decision phase
     * until the specified "artificial" time step has passed
     */
    start_timer(thisAgent, current_real_time);
    if (timercmp(current_real_time, thisAgent->real_time_tracker, <))
    {
        if (!(thisAgent->real_time_idling))
        {
            thisAgent->real_time_idling = true;
            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- Real-time Idle Phase ---\n");
            }
        }
        break;
    }
    /* Artificial time delay has passed.
     * Reset new delay and start the decision phase with input
    */
    thisAgent->real_time_tracker->tv_sec = current_real_time->tv_sec;
    thisAgent->real_time_tracker->tv_usec =
        current_real_time->tv_usec +
        1000 * thisAgent->sysparams[REAL_TIME_SYSPARAM];
    if (thisAgent->real_time_tracker->tv_usec >= 1000000)
    {
        thisAgent->real_time_tracker->tv_sec +=
            thisAgent->real_time_tracker->tv_usec / 1000000;
        thisAgent->real_time_tracker->tv_usec %= 1000000;
    }
    thisAgent->real_time_idling = false;
}
#endif // REAL_TIME_BEHAVIOR

#ifdef ATTENTION_LAPSE
/* RMJ */
int64_t init_lapse_duration(struct timeval* tv)
{
    return 0;
}
void init_attention_lapse(void)
{
    thisAgent->attention_lapse_tracker =
        (struct timeval*) malloc(sizeof(struct timeval));
    wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
    current_real_time =
        (struct timeval*) malloc(sizeof(struct timeval));
#endif // REAL_TIME_BEHAVIOR
}
void start_attention_lapse(int64_t duration)
{
    /* Set tracker to time we should wake up */
    start_timer(thisAgent->attention_lapse_tracker);
    thisAgent->attention_lapse_tracker->tv_usec += 1000 * duration;
    if (thisAgent->attention_lapse_tracker->tv_usec >= 1000000)
    {
        thisAgent->attention_lapse_tracker->tv_sec +=
            thisAgent->attention_lapse_tracker->tv_usec / 1000000;
        thisAgent->attention_lapse_tracker->tv_usec %= 1000000;
    }
    thisAgent->attention_lapsing = true;
}
void wake_from_attention_lapse(void)
{
    /* Set tracker to last time we woke up */
    start_timer(thisAgent->attention_lapse_tracker);
    thisAgent->attention_lapsing = false;
}
void determine_lapsing(agent* thisAgent)
{
    /* RMJ; decide whether to start or finish an attentional lapse */
    if (thisAgent->sysparams[ATTENTION_LAPSE_ON_SYSPARAM])
    {
        if (thisAgent->attention_lapsing)
        {
            /* If lapsing, is it time to stop? */
            start_timer(thisAgent, current_real_time);
            if (cmp(current_real_time, thisAgent->attention_lapse_tracker, >))
            {
                wake_from_attention_lapse();
            }
        }
        else
        {
            /* If not lapsing, should we start? */
            lapse_duration = init_lapse_duration(thisAgent->attention_lapse_tracker);
            if (lapse_duration > 0)
            {
                start_attention_lapse(lapse_duration);
            }
        }
    }
}
/* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.  But
   we need to put a placeholder function here just to be safe.
*/

#endif // ATTENTION_LAPSE

void stats_init_db(agent* thisAgent)
{
    if (thisAgent->stats_db->get_status() != soar_module::disconnected)
    {
        return;
    }
    
    const char* db_path = ":memory:";
    //const char *db_path = "C:\\Users\\voigtjr\\Desktop\\stats_debug.db";
    
    // attempt connection
    thisAgent->stats_db->connect(db_path);
    
    if (thisAgent->stats_db->get_status() == soar_module::problem)
    {
        char buf[256];
        SNPRINTF(buf, 254, "DB ERROR: %s", thisAgent->stats_db->get_errmsg());
        
        print(thisAgent,  buf);
        xml_generate_warning(thisAgent, buf);
    }
    else
    {
        // setup common structures/queries
        thisAgent->stats_stmts = new stats_statement_container(thisAgent);
        thisAgent->stats_stmts->structure();
        thisAgent->stats_stmts->prepare();
    }
}


void stats_db_store(agent* thisAgent, const uint64_t& dc_time, const uint64_t& dc_wm_changes, const uint64_t& dc_firing_counts)
{
    if (thisAgent->stats_db->get_status() == soar_module::disconnected)
    {
        stats_init_db(thisAgent);
    }
    
    thisAgent->stats_stmts->insert->bind_int(1, thisAgent->d_cycle_count);
    thisAgent->stats_stmts->insert->bind_int(2, dc_time);
    thisAgent->stats_stmts->insert->bind_int(3, dc_wm_changes);
    thisAgent->stats_stmts->insert->bind_int(4, dc_firing_counts);
    
    thisAgent->stats_stmts->insert->execute(soar_module::op_reinit);   // makes it ready for next execution
}

stats_statement_container::stats_statement_container(agent* new_agent): soar_module::sqlite_statement_container(new_agent->stats_db)
{
    soar_module::sqlite_database* new_db = new_agent->stats_db;
    
    //
    
    add_structure("CREATE TABLE IF NOT EXISTS stats (dc INTEGER PRIMARY KEY, time INTEGER, wm_changes INTEGER, firing_count INTEGER)");
    add_structure("CREATE INDEX IF NOT EXISTS stats_time ON stats (time)");
    add_structure("CREATE INDEX IF NOT EXISTS stats_wm_changes ON stats (wm_changes)");
    add_structure("CREATE INDEX IF NOT EXISTS stats_firing_count ON stats (firing_count)");
    
    //
    
    insert = new soar_module::sqlite_statement(new_db, "INSERT INTO stats (dc, time, wm_changes, firing_count) VALUES (?,?,?,?)");
    add(insert);
    
    cache5 = new soar_module::sqlite_statement(new_db, "PRAGMA cache_size = 5000");
    add(cache5);
    
    cache20 = new soar_module::sqlite_statement(new_db, "PRAGMA cache_size = 20000");
    add(cache20);
    
    cache100 = new soar_module::sqlite_statement(new_db, "PRAGMA cache_size = 100000");
    add(cache100);
    
    sel_dc_inc = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY dc");
    add(sel_dc_inc);
    
    sel_dc_dec = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY dc DESC");
    add(sel_dc_dec);
    
    sel_time_inc = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY time");
    add(sel_time_inc);
    
    sel_time_dec = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY time DESC");
    add(sel_time_dec);
    
    sel_wm_changes_inc = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY wm_changes");
    add(sel_wm_changes_inc);
    
    sel_wm_changes_dec = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY wm_changes DESC");
    add(sel_wm_changes_dec);
    
    sel_firing_count_inc = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY firing_count");
    add(sel_firing_count_inc);
    
    sel_firing_count_dec = new soar_module::sqlite_statement(new_db, "SELECT * FROM stats ORDER BY firing_count DESC");
    add(sel_firing_count_dec);
}

void stats_close(agent* thisAgent)
{
    if (thisAgent->stats_db->get_status() == soar_module::connected)
    {
        // de-allocate common statements
        delete thisAgent->stats_stmts;
        thisAgent->stats_stmts = 0;
        
        // close the database
        thisAgent->stats_db->disconnect();
    }
}

uint64_t get_derived_kernel_time_usec(agent* thisAgent)
{
#ifndef NO_TIMING_STUFF
    return thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[PREFERENCE_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[WM_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_usec()
           + thisAgent->timers_decision_cycle_phase[DECISION_PHASE].get_usec();
#else
    return 0;
#endif
}





