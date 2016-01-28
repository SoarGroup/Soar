/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* -- stats.h
 *
 *    This file used to be utilities.h, but most stuff seemed to fit better
 *    elsewhere.  There's code for two legacy experimental modes still in
 *    this file.  They probably don't work any more, but we want to leave
 *    them in for now in case they can be re-used in the future.
 *
 */

#ifndef STATS_H
#define STATS_H

#include <list>
#include <map>

#include "soar_db.h"

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
extern void init_real_time(agent* thisAgent);
extern struct timeval* current_real_time;
#endif // REAL_TIME_BEHAVIOR

//////////////////////////////////////////////////////////
// Statistics database
//////////////////////////////////////////////////////////

class stats_statement_container: public soar_module::sqlite_statement_container
{
    public:
        soar_module::sqlite_statement* insert;
        
        soar_module::sqlite_statement* cache5;
        soar_module::sqlite_statement* cache20;
        soar_module::sqlite_statement* cache100;
        
        soar_module::sqlite_statement* sel_dc_inc;
        soar_module::sqlite_statement* sel_dc_dec;
        soar_module::sqlite_statement* sel_time_inc;
        soar_module::sqlite_statement* sel_time_dec;
        soar_module::sqlite_statement* sel_wm_changes_inc;
        soar_module::sqlite_statement* sel_wm_changes_dec;
        soar_module::sqlite_statement* sel_firing_count_inc;
        soar_module::sqlite_statement* sel_firing_count_dec;
        
        stats_statement_container(agent* new_agent);
};

// Store statistics in to database
extern void stats_db_store(agent* thisAgent, const uint64_t& dc_time, const uint64_t& dc_wm_changes, const uint64_t& dc_firing_counts);
extern void stats_close(agent* thisAgent);

/* derived_kernel_time := Total of the time spent in the phases of the decision cycle,
excluding Input Function, Output function, and pre-defined callbacks.
This computed time should be roughly equal to total_kernel_time,
as determined above. */
uint64_t get_derived_kernel_time_usec(agent* thisAgent);

#endif //STATS_H
