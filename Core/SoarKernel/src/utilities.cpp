#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* utilities.cpp */

#include "agent.h"
#include "misc.h"
#include "utilities.h"
#include "gdatastructs.h"
#include "wmem.h"
#include "print.h"
#include "xml.h"

#include <time.h>

bool read_id_or_context_var_from_string (agent* thisAgent, const char * the_lexeme,
	Symbol * * result_id)
{
	Symbol *id;
	Symbol *g, *attr, *value;

	get_lexeme_from_string(thisAgent, the_lexeme);

	if (thisAgent->lexeme.type == IDENTIFIER_LEXEME)
	{
		id = find_identifier(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
		if (!id)
		{
			return false;
		}
		else
		{
			*result_id = id;
			return true;
		}
	}

	if (thisAgent->lexeme.type==VARIABLE_LEXEME)
	{
		get_context_var_info (thisAgent, &g, &attr, &value);

		if ((!attr) || (!value))
		{
			return false;
		}

		if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE)
		{
			return false;
		}

		*result_id = value;
		return true;
	}

	return false;
}

void get_lexeme_from_string (agent* thisAgent, const char * the_lexeme)
{
	int i;
	const char * c;
	bool sym_constant_start_found = false;
	bool sym_constant_end_found = false;

	for (c = the_lexeme, i = 0; *c; c++, i++)
	{
		if (*c == '|')
		{
			if (!sym_constant_start_found)
			{
				i--;
				sym_constant_start_found = true;
			}
			else
			{
				i--;
				sym_constant_end_found = true;
			}
		}
		else
		{
			thisAgent->lexeme.string[i] = *c;
		}
	}

	thisAgent->lexeme.string[i] = '\0'; /* Null terminate lexeme string */

	thisAgent->lexeme.length = i;

	if (sym_constant_end_found)
	{
		thisAgent->lexeme.type = SYM_CONSTANT_LEXEME;
	}
	else
	{
		determine_type_of_constituent_string(thisAgent);
	}
}

void get_context_var_info ( agent* thisAgent, Symbol **dest_goal,
	Symbol **dest_attr_of_slot,
	Symbol **dest_current_value)
{
	Symbol *v, *g;
	int levels_up;
	wme *w;

	v = find_variable (thisAgent, thisAgent->lexeme.string);
	if (v==thisAgent->s_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = thisAgent->state_symbol;
	} else if (v==thisAgent->o_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = thisAgent->operator_symbol;
	} else if (v==thisAgent->ss_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = thisAgent->state_symbol;
	} else if (v==thisAgent->so_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = thisAgent->operator_symbol;
	} else if (v==thisAgent->sss_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = thisAgent->state_symbol;
	} else if (v==thisAgent->sso_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = thisAgent->operator_symbol;
	} else if (v==thisAgent->ts_context_variable) {
		levels_up = thisAgent->top_goal ? thisAgent->bottom_goal->id->level-thisAgent->top_goal->id->level : 0;
		*dest_attr_of_slot = thisAgent->state_symbol;
	} else if (v==thisAgent->to_context_variable) {
		levels_up = thisAgent->top_goal ? thisAgent->bottom_goal->id->level-thisAgent->top_goal->id->level : 0;
		*dest_attr_of_slot = thisAgent->operator_symbol;
	} else {
		*dest_goal = NIL;
		*dest_attr_of_slot = NIL;
		*dest_current_value = NIL;
		return;
	}

	g = thisAgent->bottom_goal;
	while (g && levels_up) {
		g = g->id->higher_goal;
		levels_up--;
	}
	*dest_goal = g;

	if (!g) {
		*dest_current_value = NIL;
		return;
	}

	if (*dest_attr_of_slot==thisAgent->state_symbol) {
		*dest_current_value = g;
	} else {
		w = g->id->operator_slot->wmes;
		*dest_current_value = w ? w->value : NIL;
	}
}

Symbol *read_identifier_or_context_variable (agent* thisAgent)
{
	Symbol *id;
	Symbol *g, *attr, *value;

	if (thisAgent->lexeme.type==IDENTIFIER_LEXEME) {
		id = find_identifier (thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number);
		if (!id) {
			print (thisAgent, "There is no identifier %c%lu.\n", thisAgent->lexeme.id_letter,
				thisAgent->lexeme.id_number);
			print_location_of_most_recent_lexeme(thisAgent);
			return NIL;
		}
		return id;
	}
	if (thisAgent->lexeme.type==VARIABLE_LEXEME)
	{
		get_context_var_info (thisAgent, &g, &attr, &value);
		if (!attr) {
			print (thisAgent, "Expected identifier (or context variable)\n");
			print_location_of_most_recent_lexeme(thisAgent);
			return NIL;
		}
		if (!value) {
			print (thisAgent, "There is no current %s.\n", thisAgent->lexeme.string);
			print_location_of_most_recent_lexeme(thisAgent);
			return NIL;
		}
		if (value->symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
			print (thisAgent, "The current %s ", thisAgent->lexeme.string);
			print_with_symbols (thisAgent, "(%y) is not an identifier.\n", value);
			print_location_of_most_recent_lexeme(thisAgent);
			return NIL;
		}
		return value;
	}
	print (thisAgent, "Expected identifier (or context variable)\n");
	print_location_of_most_recent_lexeme(thisAgent);
	return NIL;
}

#ifdef REAL_TIME_BEHAVIOR
* RMJ */
void init_real_time (agent* thisAgent) {
   thisAgent->real_time_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   timerclear(thisAgent->real_time_tracker);
   thisAgent->real_time_idling = false;
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
}
void test_for_input_delay (agent* thisAgent) {
  /* RMJ; For real-time behavior, don't start any new decision phase
   * until the specified "artificial" time step has passed
   */
   start_timer (thisAgent, current_real_time);
   if (timercmp(current_real_time, thisAgent->real_time_tracker, <)) {
      if (!(thisAgent->real_time_idling)) {
         thisAgent->real_time_idling = true;
         if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
            print_phase (thisAgent, "\n--- Real-time Idle Phase ---\n");
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
   if (thisAgent->real_time_tracker->tv_usec >= 1000000) {
      thisAgent->real_time_tracker->tv_sec +=
            thisAgent->real_time_tracker->tv_usec / 1000000;
      thisAgent->real_time_tracker->tv_usec %= 1000000;
   }
   thisAgent->real_time_idling = false;
}
#endif // REAL_TIME_BEHAVIOR

#ifdef ATTENTION_LAPSE
* RMJ */

void init_attention_lapse (void) {
   thisAgent->attention_lapse_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
#endif // REAL_TIME_BEHAVIOR
}
void start_attention_lapse (int64_t duration) {
   /* Set tracker to time we should wake up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapse_tracker->tv_usec += 1000 * duration;
   if (thisAgent->attention_lapse_tracker->tv_usec >= 1000000) {
      thisAgent->attention_lapse_tracker->tv_sec +=
            thisAgent->attention_lapse_tracker->tv_usec / 1000000;
      thisAgent->attention_lapse_tracker->tv_usec %= 1000000;
   }
   thisAgent->attention_lapsing = true;
}
void wake_from_attention_lapse (void) {
   /* Set tracker to last time we woke up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapsing = false;
}
void determine_lapsing (agent* thisAgent) {
   /* RMJ; decide whether to start or finish an attentional lapse */
   if (thisAgent->sysparams[ATTENTION_LAPSE_ON_SYSPARAM]) {
      if (thisAgent->attention_lapsing) {
         /* If lapsing, is it time to stop? */
         start_timer (thisAgent, current_real_time);
         if (cmp(current_real_time,
                      thisAgent->attention_lapse_tracker, >)) {
            wake_from_attention_lapse();
         }
      } else {
         /* If not lapsing, should we start? */
         lapse_duration = init_lapse_duration(thisAgent->attention_lapse_tracker);
         if (lapse_duration > 0) {
            start_attention_lapse(lapse_duration);
         }
      }
   }
}
* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.  But
   we need to put a placeholder function here just to be safe.
*/
int64_t init_lapse_duration(struct timeval *tv) {
   return 0;
}
#endif // ATTENTION_LAPSE

// formerly in misc.cpp:
/***************************************************************************
 * Function     : is_natural_number
 **************************************************************************/
bool is_whole_number(const std::string &str)
{
	return is_whole_number(str.c_str());
}

bool is_whole_number(const char * str)
{
	if(!str || !*str)
		return false;

	do {
		if(isdigit(*str))
			++str;
		else
			return false;
	} while(*str);

	return true;
}


void stats_init_db( agent *thisAgent )
{
	if ( thisAgent->stats_db->get_status() != soar_module::disconnected )
		return;

	const char *db_path = ":memory:";
	//const char *db_path = "C:\\Users\\voigtjr\\Desktop\\stats_debug.db";

	// attempt connection
	thisAgent->stats_db->connect( db_path );

	if ( thisAgent->stats_db->get_status() == soar_module::problem )
	{
		char buf[256];
		SNPRINTF( buf, 254, "DB ERROR: %s", thisAgent->stats_db->get_errmsg() );

		print( thisAgent, buf );
		xml_generate_warning( thisAgent, buf );
	}
	else
	{
		// setup common structures/queries
		thisAgent->stats_stmts = new stats_statement_container( thisAgent );
		thisAgent->stats_stmts->structure();
		thisAgent->stats_stmts->prepare();
	}
}


void stats_db_store(agent* thisAgent, const uint64_t& dc_time, const uint64_t& dc_wm_changes, const uint64_t& dc_firing_counts)
{
	if ( thisAgent->stats_db->get_status() == soar_module::disconnected )
	{
		stats_init_db( thisAgent );
	}

	thisAgent->stats_stmts->insert->bind_int(1, thisAgent->d_cycle_count);
	thisAgent->stats_stmts->insert->bind_int(2, dc_time);
	thisAgent->stats_stmts->insert->bind_int(3, dc_wm_changes);
	thisAgent->stats_stmts->insert->bind_int(4, dc_firing_counts);

	thisAgent->stats_stmts->insert->execute( soar_module::op_reinit ); // makes it ready for next execution
}

stats_statement_container::stats_statement_container( agent *new_agent ): soar_module::sqlite_statement_container( new_agent->stats_db )
{
	soar_module::sqlite_database *new_db = new_agent->stats_db;

	//

	add_structure( "CREATE TABLE IF NOT EXISTS stats (dc INTEGER PRIMARY KEY, time INTEGER, wm_changes INTEGER, firing_count INTEGER)" );
	add_structure( "CREATE INDEX IF NOT EXISTS stats_time ON stats (time)" );
	add_structure( "CREATE INDEX IF NOT EXISTS stats_wm_changes ON stats (wm_changes)" );
	add_structure( "CREATE INDEX IF NOT EXISTS stats_firing_count ON stats (firing_count)" );

	//

	insert = new soar_module::sqlite_statement( new_db, "INSERT INTO stats (dc, time, wm_changes, firing_count) VALUES (?,?,?,?)" );
	add( insert );

	cache5 = new soar_module::sqlite_statement( new_db, "PRAGMA cache_size = 5000" );
	add( cache5 );

	cache20 = new soar_module::sqlite_statement( new_db, "PRAGMA cache_size = 20000" );
	add( cache20 );

	cache100 = new soar_module::sqlite_statement( new_db, "PRAGMA cache_size = 100000" );
	add( cache100 );

	sel_dc_inc = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY dc" );
	add( sel_dc_inc );

	sel_dc_dec = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY dc DESC" );
	add( sel_dc_dec );

	sel_time_inc = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY time" );
	add( sel_time_inc );

	sel_time_dec = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY time DESC" );
	add( sel_time_dec );

	sel_wm_changes_inc = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY wm_changes" );
	add( sel_wm_changes_inc );

	sel_wm_changes_dec = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY wm_changes DESC" );
	add( sel_wm_changes_dec );

	sel_firing_count_inc = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY firing_count" );
	add( sel_firing_count_inc );

	sel_firing_count_dec = new soar_module::sqlite_statement( new_db, "SELECT * FROM stats ORDER BY firing_count DESC" );
	add( sel_firing_count_dec );
}

void stats_close( agent *thisAgent )
{
	if ( thisAgent->stats_db->get_status() == soar_module::connected )
	{
		// de-allocate common statements
		delete thisAgent->stats_stmts;
		thisAgent->stats_stmts = 0;

		// close the database
		thisAgent->stats_db->disconnect();
	}
}

uint64_t get_derived_kernel_time_usec(agent* thisAgent) {
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

