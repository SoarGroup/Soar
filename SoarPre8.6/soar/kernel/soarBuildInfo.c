#include "soarkernel.h"
#include "soar_ecore_api.h"

void soar_ecBuildInfo()
{

    print("\nSoar v.");
    print(soar_version_string);
    print(" Build Information.\n");

#ifdef NUMERIC_INDIFFERENCE
    print(" - NUMERIC_INDIFFERENCE\n");
#endif
#ifdef MATCHTIME_INTERRUPT
    print(" - MATCHTIME_INTERRUPT\n");
#endif
#ifdef O_REJECTS_FIRST
    print(" - O_REJECTS_FIRST\n");
#endif
#ifdef AGRESSIVE_ONC
    print(" - AGRESSIVE_ONC\n");
#endif
#ifdef ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS
    print(" - ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS\n");
#endif

#ifdef ATTENTION_LAPSE
    print(" - ATTENTION_LAPSE\n");
#endif

#ifdef COUNT_KERNEL_TIMER_STOPS
    print(" - COUNT_KERNEL_TIMER_STOPS\n");
#endif

#ifdef DC_HISTOGRAM
    print(" - DC_HISTOGRAM\n");
#endif

#ifdef DEBUG_CHUNK_NAMES
    print(" - DEBUG_CHUNK_NAMES\n");
#endif

#ifdef DEBUG_CONSISTENCY_CHECK
    print(" - DEBUG_CONSISTENCY_CHECK\n");
#endif

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print(" - DEBUG_DETERMINE_LEVEL_PHASE\n");
#endif

#ifdef DEBUG_FIND_SLOT
    print(" - DEBUG_FIND_SLOT\n");
#endif

#ifdef DEBUG_GDS
    print(" - DEBUG_GDS\n");
#endif

#ifdef DEBUG_GDS_HIGH
    print(" - DEBUG_GDS_HIGH\n");
#endif

#ifdef DEBUG_INSTANTIATIONS
    print(" - DEBUG_INSTANTIATIONS\n");
#endif

#ifdef DEBUG_LINKS
    print(" - DEBUG_LINKS\n");
#endif

#ifdef DEBUG_MEMORY
    print(" - DEBUG_MEMORY\n");
#endif

#ifdef DEBUG_NO_TOP_LEVEL_REFS
    print(" - DEBUG_NO_TOP_LEVEL_REFS\n");
#endif

#ifdef DEBUG_PREFS
    print(" - DEBUG_PREFS\n");
#endif

#ifdef DEBUG_RETE_PNODES
    print(" - DEBUG_RETE_PNODES\n");
#endif

#ifdef DEBUG_SLOTS
    print(" - DEBUG_SLOTS\n");
#endif

#ifdef DEBUG_SYMBOLS
    print(" - DEBUG_SYMBOLS\n");
#endif

#ifdef DEBUG_WATERFALL
    print(" - DEBUG_WATERFALL\n");
#endif

#ifdef DEBUG_WMES
    print(" - DEBUG_WMES\n");
#endif

#ifdef DETAILED_TIMING_STATS
    print(" - DETAILED_TIMING_STATS\n");
#endif

#ifdef DONT_ALLOW_VARIABLIZATION
    print(" - DONT_ALLOW_VARIABLIZATION\n");
#endif

#ifdef DONT_CALC_GDS_OR_BT
    print(" - DONT_CALC_GDS_OR_BT\n");
#endif

#ifdef DONT_DO_IO_CYCLES
    print(" - DONT_DO_IO_CYCLES\n");
#endif

#ifdef DO_ACTIVATION_STATS_ON_REMOVALS
    print(" - DO_ACTIVATION_STATS_ON_REMOVALS\n");
#endif

#ifdef DO_COMPILE_TIME_O_SUPPORT_CALCS
    print(" - DO_COMPILE_TIME_O_SUPPORT_CALCS\n");
#endif

#ifdef FEW_CALLBACKS
    print(" - FEW_CALLBACKS\n");
#endif

#ifdef GOAL_SANITY_CHECK
    print(" - GOAL_SANITY_CHECK\n");
#endif

#ifdef HAVE_BOOL
    print(" - HAVE_BOOL\n");
#endif

#ifdef HEAVY
    print(" - HEAVY\n");
#endif

#ifdef KERNEL_TIME_ONLY
    print(" - KERNEL_TIME_ONLY\n");
#endif

#ifdef KT_HISTOGRAM
    print(" - KT_HISTOGRAM\n");
#endif

#ifdef LIST_COMPILE_TIME_O_SUPPORT_FAILURES
    print(" - LIST_COMPILE_TIME_O_SUPPORT_FAILURES\n");
#endif

#ifdef LITE
    print(" - LITE\n");
#endif

#ifdef MAKE_PRODUCTION_FOR_THIN_JUSTS
    print(" - MAKE_PRODUCTION_FOR_THIN_JUSTS\n");
#endif

#ifdef MAX_SIMULTANEOUS_AGENTS
    print(" - MAX_SIMULTANEOUS_AGENTS %d\n", MAX_SIMULTANEOUS_AGENTS);
#endif

#ifdef MEMORY_POOL_STATS
    print(" - MEMORY_POOL_STATS\n");
#endif

#ifdef MHZ
    print(" - MHZ %d\n", MHZ);
#endif

#ifdef NO_ADC_CALLBACK
    print(" - NO_ADC_CALLBACK\n");
#endif

#ifdef NO_ADP_CALLBACK
    print(" - NO_ADP_CALLBACK\n");
#endif

#ifdef NO_BACKTRACING
    print(" - NO_BACKTRACING\n");
#endif

#ifdef NO_IO_CALLBACKS
    print(" - NO_IO_CALLBACKS\n");
#endif

#ifdef NO_JUSTS_BELOW_OSUPPORT
    print(" - NO_JUSTS_BELOW_OSUPPORT\n");
#endif

#ifdef NO_TIMING_STUFF
    print(" - NO_TIMING_STUFF\n");
#endif

#ifdef NO_TOP_JUST
    print(" - NO_TOP_JUST\n");
#endif

#ifdef NO_TOP_LEVEL_REFS
    print(" - NO_TOP_LEVEL_REFS\n");
#endif

#ifdef NULL_ACTIVATION_STATS
    print(" - NULL_ACTIVATION_STATS\n");
#endif

#ifdef OPTIMIZE_TOP_LEVEL_RESULTS
    print(" - OPTIMIZE_TOP_LEVEL_RESULTS\n");
#endif

#ifdef PII_TIMERS
    print(" - PII_TIMERS\n");
#endif

#ifdef REAL_TIME_BEHAVIOR
    print(" - REAL_TIME_BEHAVIOR\n");
#endif

#ifdef REMOVE_INSTS_WITH_O_PREFS
    print(" - REMOVE_INSTS_WITH_O_PREFS\n");
#endif

#ifdef SHARING_FACTORS
    print(" - SHARING_FACTORS\n");
#endif

#ifdef SINGLE_THIN_JUSTIFICATION
    print(" - SINGLE_THIN_JUSTIFICATION\n");
#endif

#ifdef SOAR_8_ONLY
    print(" - SOAR_8_ONLY\n");
#endif

#ifdef STD
    print(" - STD\n");
#endif

#ifdef THINK_C
    print(" - THINK_C\n");
#endif

#ifdef THIN_JUSTIFICATIONS
    print(" - THIN_JUSTIFICATIONS\n");
#endif

#ifdef TOKEN_SHARING_STATS
    print(" - TOKEN_SHARING_STATS\n");
#endif

#ifdef TRACE_CONTEXT_DECISIONS_ONLY
    print(" - TRACE_CONTEXT_DECISIONS_ONLY\n");
#endif

#ifdef TRACE_MEMORY_USAGE
    print(" - TRACE_MEMORY_USAGE\n");
#endif

#ifdef TRACK_MEMORY_USAGE
    print(" - TRACK_MEMORY_USAGE\n");
#endif

#ifdef TRY_SSCI_PROD_NIL
    print(" - TRY_SSCI_PROD_NIL\n");
#endif

#ifdef USE_AGENT_DBG_FILE
    print(" - USE_AGENT_DBG_FILE\n");
#endif

#ifdef USE_CAPTURE_REPLAY
    print(" - USE_CAPTURE_REPLAY\n");
#endif

#ifdef USE_DEBUG_UTILS
    print(" - USE_DEBUG_UTILS\n");
#endif

#ifdef USE_STDARGS
    print(" - USE_STDARGS\n");
#endif

#ifdef WARN_IF_RESULT_IS_I_SUPPORTED
    print(" - WARN_IF_RESULT_IS_I_SUPPORTED\n");
#endif

#ifdef WARN_IF_TIMERS_REPORT_ZERO
    print(" - WARN_IF_TIMERS_REPORT_ZERO\n");
#endif

#ifdef WATCH_CHUNK_INST
    print(" - WATCH_CHUNK_INST\n");
#endif

#ifdef WATCH_INSTS_WITH_O_PREFS
    print(" - WATCH_INSTS_WITH_O_PREFS\n");
#endif

#ifdef WATCH_INST_CONDS
    print(" - WATCH_INST_CONDS\n");
#endif

#ifdef WATCH_PREFS_GENERATED
    print(" - WATCH_PREFS_GENERATED\n");
#endif

#ifdef WATCH_PRODUCTIONS
    print(" - WATCH_PRODUCTIONS\n");
#endif

#ifdef WATCH_RESULTS
    print(" - WATCH_RESULTS\n");
#endif

#ifdef WATCH_SSCI_CONDS
    print(" - WATCH_SSCI_CONDS\n");
#endif

#ifdef WATCH_SSCI_INSTS
    print(" - WATCH_SSCI_INSTS\n");
#endif

#ifdef WIN32
    print(" - WIN32\n");
#endif

#ifdef _WINDOWS
    print(" - _WINDOWS\n");
#endif

#ifdef __SC__
    print(" - __SC__\n");
#endif

#ifdef __ultrix
    print(" - __ultrix\n");
#endif

#ifdef tolower
    print(" - tolower\n");
#endif

}

void soar_ecExcludedBuildInfo()
{

    print("\nSoar v. ");
    print(soar_version_string);
    print(" Excluded Build Information.\n");

}
