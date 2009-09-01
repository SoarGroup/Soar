#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.cpp */

#include "stl_support.h"
#include "utilities.h"
#include "gdatastructs.h"
#include "wmem.h"
#include "print.h"

#include <time.h>

SoarSTLWMEPoolList* get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc)
{
	// notice how we give the constructor our custom SoarSTLWMEPoolList with the agent and memory type to use
	SoarSTLWMEPoolList* list = new SoarSTLWMEPoolList(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));

	slot *s;
    wme *w;

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return NULL;
    if (id->id.tc_num == tc)
        return NULL;
    id->id.tc_num = tc;

	// build list of wmes
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            list->push_back(w);
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            list->push_back(w);
    }

    return list;
}

bool read_id_or_context_var_from_string (agent* agnt, const char * the_lexeme,
	Symbol * * result_id) 
{
	Symbol *id;
	Symbol *g, *attr, *value;

	get_lexeme_from_string(agnt, the_lexeme);

	if (agnt->lexeme.type == IDENTIFIER_LEXEME) 
	{
		id = find_identifier(agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
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

	if (agnt->lexeme.type==VARIABLE_LEXEME) 
	{
		get_context_var_info (agnt, &g, &attr, &value);

		if ((!attr) || (!value))
		{
			return false;
		}

		if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
		{
			return false;
		}

		*result_id = value;
		return true;
	}

	return false;
}

void get_lexeme_from_string (agent* agnt, const char * the_lexeme)
{
	int i;
	const char * c;
	bool sym_constant_start_found = FALSE;
	bool sym_constant_end_found = FALSE;

	for (c = the_lexeme, i = 0; *c; c++, i++)
	{
		if (*c == '|')
		{
			if (!sym_constant_start_found)
			{
				i--;
				sym_constant_start_found = TRUE;
			}
			else
			{
				i--;
				sym_constant_end_found = TRUE;
			}
		}
		else
		{
			agnt->lexeme.string[i] = *c;
		} 
	}

	agnt->lexeme.string[i] = '\0'; /* Null terminate lexeme string */

	agnt->lexeme.length = i;

	if (sym_constant_end_found)
	{
		agnt->lexeme.type = SYM_CONSTANT_LEXEME;
	}
	else 
	{
		determine_type_of_constituent_string(agnt);
	}
}

void get_context_var_info ( agent* agnt, Symbol **dest_goal,
	Symbol **dest_attr_of_slot,
	Symbol **dest_current_value) 
{
	Symbol *v, *g;
	int levels_up;
	wme *w;

	v = find_variable (agnt, agnt->lexeme.string);
	if (v==agnt->s_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->o_context_variable) {
		levels_up = 0;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->ss_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->so_context_variable) {
		levels_up = 1;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->sss_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->sso_context_variable) {
		levels_up = 2;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else if (v==agnt->ts_context_variable) {
		levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
		*dest_attr_of_slot = agnt->state_symbol;
	} else if (v==agnt->to_context_variable) {
		levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
		*dest_attr_of_slot = agnt->operator_symbol;
	} else {
		*dest_goal = NIL;
		*dest_attr_of_slot = NIL;
		*dest_current_value = NIL;
		return;
	}

	g = agnt->bottom_goal;
	while (g && levels_up) {
		g = g->id.higher_goal;
		levels_up--;
	}
	*dest_goal = g;

	if (!g) {
		*dest_current_value = NIL;
		return;
	}

	if (*dest_attr_of_slot==agnt->state_symbol) {
		*dest_current_value = g;
	} else {
		w = g->id.operator_slot->wmes;
		*dest_current_value = w ? w->value : NIL;
	}
}

Symbol *read_identifier_or_context_variable (agent* agnt) 
{
	Symbol *id;
	Symbol *g, *attr, *value;

	if (agnt->lexeme.type==IDENTIFIER_LEXEME) {
		id = find_identifier (agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
		if (!id) {
			print (agnt, "There is no identifier %c%lu.\n", agnt->lexeme.id_letter,
				agnt->lexeme.id_number);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		return id;
	}
	if (agnt->lexeme.type==VARIABLE_LEXEME) 
	{
		get_context_var_info (agnt, &g, &attr, &value);
		if (!attr) {
			print (agnt, "Expected identifier (or context variable)\n");
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		if (!value) {
			print (agnt, "There is no current %s.\n", agnt->lexeme.string);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
			print (agnt, "The current %s ", agnt->lexeme.string);
			print_with_symbols (agnt, "(%y) is not an identifier.\n", value);
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}
		return value;
	}
	print (agnt, "Expected identifier (or context variable)\n");
	print_location_of_most_recent_lexeme(agnt);
	return NIL;
}		

/* ===================================================================

                       Timer Utility Routines

   These are utility routines for using timers.  We use (struct timeval)'s
   (defined in a system include file) for keeping track of the cumulative
   time spent in one part of the system or another.  Reset_timer()
   clears a timer to 0.  Start_timer() and stop_timer() are used for
   timing an interval of code--the usage is:
   
     start_timer (&timeval_to_record_the_start_time_in); 
     ... other code here ...
     stop_timer (&timeval_to_record_the_start_time_in,
                 &timeval_holding_accumulated_time_for_this_code);

   Finally, timer_value() returns the accumulated value of a timer
   (in seconds).
=================================================================== */
#define ONE_MILLION (1000000)

double timer_value (struct timeval *tv) {
  return tv->tv_sec + tv->tv_usec/static_cast<double>(ONE_MILLION);
}

void reset_timer (struct timeval *tv_to_reset) {
  tv_to_reset->tv_sec = 0;
  tv_to_reset->tv_usec = 0;
}

#ifndef NO_TIMING_STUFF

#ifdef WIN32

/* A fake implementation of rusage for WIN32. Taken from cygwin. */
#define RUSAGE_SELF 0
struct rusage {
   struct timeval ru_utime;
   struct timeval ru_stime;
};
#define NSPERSEC 10000000LL
#define FACTOR (0x19db1ded53e8000LL)

static uint64_t
__to_clock_t (FILETIME * src, int flag)
{
  uint64_t total = (static_cast<uint64_t>(src->dwHighDateTime) << 32) + static_cast<uint32_t>(src->dwLowDateTime);

  /* Convert into clock ticks - the total is in 10ths of a usec.  */
  if (flag)
    total -= FACTOR;
  
  total /= NSPERSEC / CLOCKS_PER_SEC;
  return total;
}
static void totimeval (struct timeval *dst, FILETIME *src, int sub, int flag)
{
  uint64_t x = __to_clock_t (src, flag);

  x *= ONE_MILLION / CLOCKS_PER_SEC; /* Turn x into usecs */
  x -= sub * ONE_MILLION;
  
  dst->tv_usec = static_cast<long>(x % ONE_MILLION); /* And split */
  dst->tv_sec = static_cast<long>(x / ONE_MILLION);
}

int getrusage(int /*who*/, struct rusage* r)
{
   FILETIME creation_time = {0,0};
   FILETIME exit_time = {0,0};
   FILETIME kernel_time = {0,0};
   FILETIME user_time = {0,0};

   memset (r, 0, sizeof (*r));
   GetProcessTimes (GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
   totimeval (&r->ru_stime, &kernel_time, 0, 0);
   totimeval (&r->ru_utime, &user_time, 0, 0);
   return 0;
}
#endif // WIN32

void get_cputime_from_rusage (struct rusage *r, struct timeval *dest_tv) {
  dest_tv->tv_sec = r->ru_utime.tv_sec + r->ru_stime.tv_sec;
  dest_tv->tv_usec = r->ru_utime.tv_usec + r->ru_stime.tv_usec;
  if (dest_tv->tv_usec >= ONE_MILLION) {
    dest_tv->tv_usec -= ONE_MILLION;
    dest_tv->tv_sec++;
  }
}

void start_timer (agent* thisAgent, struct timeval *tv_for_recording_start_time) {

    if(thisAgent && !thisAgent->sysparams[TIMERS_ENABLED]) {
        return;
    }
    struct rusage temp_rusage;
 
    getrusage (RUSAGE_SELF, &temp_rusage);
    get_cputime_from_rusage (&temp_rusage, tv_for_recording_start_time);
}

void stop_timer (agent* thisAgent,
struct timeval *tv_with_recorded_start_time,
struct timeval *tv_with_accumulated_time) {

    if(thisAgent && !thisAgent->sysparams[TIMERS_ENABLED]) {
        return;
    }

    struct rusage end_rusage;
    struct timeval end_tv;
    long delta_sec, delta_usec;
 
    getrusage (RUSAGE_SELF, &end_rusage);
    get_cputime_from_rusage (&end_rusage, &end_tv);

    delta_sec = end_tv.tv_sec - tv_with_recorded_start_time->tv_sec;
    delta_usec = end_tv.tv_usec - tv_with_recorded_start_time->tv_usec;
    if (delta_usec < 0) {
        delta_usec += ONE_MILLION;
        delta_sec--;
    }

    tv_with_accumulated_time->tv_sec += delta_sec;
    tv_with_accumulated_time->tv_usec += delta_usec;
    if (tv_with_accumulated_time->tv_usec >= ONE_MILLION) {
        tv_with_accumulated_time->tv_usec -= ONE_MILLION;
        tv_with_accumulated_time->tv_sec++;
    }
}
#endif // NO_TIMING_STUFF

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
void init_real_time (agent* thisAgent) {
   thisAgent->real_time_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   timerclear(thisAgent->real_time_tracker);
   thisAgent->real_time_idling = FALSE;
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
         thisAgent->real_time_idling = TRUE;
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
   thisAgent->real_time_idling = FALSE;
}
#endif // REAL_TIME_BEHAVIOR

#ifdef ATTENTION_LAPSE
/* RMJ */

void init_attention_lapse (void) {
   thisAgent->attention_lapse_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
#endif // REAL_TIME_BEHAVIOR
}
void start_attention_lapse (long duration) {
   /* Set tracker to time we should wake up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapse_tracker->tv_usec += 1000 * duration;
   if (thisAgent->attention_lapse_tracker->tv_usec >= 1000000) {
      thisAgent->attention_lapse_tracker->tv_sec +=
            thisAgent->attention_lapse_tracker->tv_usec / 1000000;
      thisAgent->attention_lapse_tracker->tv_usec %= 1000000;
   }
   thisAgent->attention_lapsing = TRUE;
}
void wake_from_attention_lapse (void) {
   /* Set tracker to last time we woke up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapsing = FALSE;
}
void determine_lapsing (agent* thisAgent) {
   /* RMJ; decide whether to start or finish an attentional lapse */
   if (thisAgent->sysparams[ATTENTION_LAPSE_ON_SYSPARAM]) {
      if (thisAgent->attention_lapsing) {
         /* If lapsing, is it time to stop? */
         start_timer (thisAgent, current_real_time);
         if (timercmp(current_real_time,
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
/* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.  But
   we need to put a placeholder function here just to be safe.
*/
long init_lapse_duration(struct timeval *tv) {
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

/***************************************************************************
 * Function     : get_number_from_symbol
 **************************************************************************/
double get_number_from_symbol( Symbol *sym )
{
	if ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
		return sym->fc.value;
	else if ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
		return static_cast<double>(sym->ic.value);
	
	return 0.0;
}
