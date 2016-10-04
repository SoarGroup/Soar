/*
 * cli_Cli_enums.h
 *
 *  Created on: Oct 3, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_CLI_ENUMS_H_
#define CORE_CLI_SRC_CLI_CLI_ENUMS_H_

#include <bitset>

namespace cli
{
    enum eCaptureInputMode
    {
        CAPTURE_INPUT_OPEN,
        CAPTURE_INPUT_QUERY,
        CAPTURE_INPUT_CLOSE,
    };

    enum eLogMode
    {
        LOG_QUERY,
        LOG_NEW,
        LOG_NEWAPPEND,
        LOG_CLOSE,
        LOG_ADD,
    };

    enum eExciseOptions
    {
        EXCISE_ALL,
        EXCISE_CHUNKS,
        EXCISE_DEFAULT,
        EXCISE_RL,
        EXCISE_TASK,
        EXCISE_TEMPLATE,
        EXCISE_USER,
        EXCISE_NEVER_FIRED,
        EXCISE_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<EXCISE_NUM_OPTIONS> ExciseBitset;

    enum ePrintOptions
    {
        PRINT_ALL,
        PRINT_CHUNKS,
        PRINT_DEPTH,
        PRINT_DEFAULTS,
        PRINT_FULL,
        PRINT_FILENAME,
        PRINT_GDS,
        PRINT_INTERNAL,
        PRINT_TREE,
        PRINT_JUSTIFICATIONS,
        PRINT_NAME,
        PRINT_OPERATORS,
        PRINT_RL,
        PRINT_STACK,
        PRINT_STATES,
        PRINT_TEMPLATE,
        PRINT_USER,
        PRINT_VARPRINT,
        PRINT_EXACT,
        PRINT_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<PRINT_NUM_OPTIONS> PrintBitset;

    enum eLearnOptions
    {
        LEARN_ALL_LEVELS,
        LEARN_BOTTOM_UP,
        LEARN_DISABLE,
        LEARN_ENABLE,
        LEARN_EXCEPT,
        LEARN_LIST,
        LEARN_ONLY,
        LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS,
        LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS,
        LEARN_ENABLE_THROUGH_EVALUATION_RULES,
        LEARN_DISABLE_THROUGH_EVALUATION_RULES,
        LEARN_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<LEARN_NUM_OPTIONS> LearnBitset;

    enum eMatchesMode
    {
        MATCHES_PRODUCTION,
        MATCHES_ASSERTIONS,
        MATCHES_RETRACTIONS,
        MATCHES_ASSERTIONS_RETRACTIONS,
    };

    enum eWMEDetail
    {
        WME_DETAIL_NONE,
        WME_DETAIL_TIMETAG,
        WME_DETAIL_FULL,
    };

    enum eMemoriesOptions
    {
        MEMORIES_CHUNKS,
        MEMORIES_DEFAULT,
        MEMORIES_JUSTIFICATIONS,
        MEMORIES_TEMPLATES,
        MEMORIES_USER,
        MEMORIES_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<MEMORIES_NUM_OPTIONS> MemoriesBitset;

    enum ePreferencesDetail
    {
        PREFERENCES_ONLY,
        PREFERENCES_NAMES,
        PREFERENCES_TIMETAGS,
        PREFERENCES_WMES,
    };
    enum eProductionFindOptions
    {
        PRODUCTION_FIND_INCLUDE_LHS,
        PRODUCTION_FIND_INCLUDE_RHS,
        PRODUCTION_FIND_ONLY_CHUNKS,
        PRODUCTION_FIND_NO_CHUNKS,
        PRODUCTION_FIND_SHOWBINDINGS,
        PRODUCTION_FIND_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<PRODUCTION_FIND_NUM_OPTIONS> ProductionFindBitset;

    enum eReplayInputMode
    {
        REPLAY_INPUT_OPEN,
        REPLAY_INPUT_QUERY,
        REPLAY_INPUT_CLOSE,
    };

    enum eRunOptions
    {
        RUN_DECISION,
        RUN_ELABORATION,
        RUN_FOREVER,
        RUN_INTERLEAVE,
        RUN_OUTPUT,
        RUN_PHASE,
        RUN_SELF,
        RUN_UPDATE,
        RUN_NO_UPDATE,
        RUN_GOAL,
        RUN_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<RUN_NUM_OPTIONS> RunBitset;

    enum eRunInterleaveMode
    {
        RUN_INTERLEAVE_DEFAULT,
        RUN_INTERLEAVE_ELABORATION,
        RUN_INTERLEAVE_PHASE,
        RUN_INTERLEAVE_DECISION,
        RUN_INTERLEAVE_OUTPUT,
    };
    enum eSourceOptions
    {
        SOURCE_ALL,
        SOURCE_DISABLE,
        SOURCE_VERBOSE,
        SOURCE_NUM_OPTIONS,    // must be last
    };
    typedef std::bitset<SOURCE_NUM_OPTIONS> SourceBitset;

    enum eStatsOptions
    {
        STATS_MEMORY,
        STATS_RETE,
        STATS_SYSTEM,
        STATS_MAX,
        STATS_RESET,
        STATS_TRACK,
        STATS_CYCLE,
        STATS_CSV,
        STATS_STOP_TRACK,
        STATS_DECISION,
        STATS_AGENT,
        STATS_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<STATS_NUM_OPTIONS> StatsBitset;

    enum eWatchOptions
    {
        WATCH_DECISIONS,
        WATCH_PHASES,
        WATCH_DEFAULT,
        WATCH_USER,
        WATCH_CHUNKS,
        WATCH_JUSTIFICATIONS,
        WATCH_TEMPLATES,
        WATCH_WMES,
        WATCH_PREFERENCES,
        WATCH_WME_DETAIL,
        WATCH_LEARNING,
        WATCH_BACKTRACING,
        WATCH_INDIFFERENT,
        WATCH_RL,
        WATCH_WATERFALL,
        WATCH_EPMEM,
        WATCH_SMEM,
        WATCH_WMA,
        WATCH_GDS_WMES,
        WATCH_GDS_STATE_REMOVAL,
        WATCH_NUM_OPTIONS, // must be last
    };

    typedef std::bitset<WATCH_NUM_OPTIONS> WatchBitset;
    enum eWatchWMEsMode
    {
        WATCH_WMES_ADD,
        WATCH_WMES_REMOVE,
        WATCH_WMES_LIST,
        WATCH_WMES_RESET,
    };

    enum eWatchWMEsOptions
    {
        WATCH_WMES_TYPE_ADDS,
        WATCH_WMES_TYPE_REMOVES,
        WATCH_WMES_TYPE_NUM_OPTIONS, // must be last
    };
    typedef std::bitset<WATCH_WMES_TYPE_NUM_OPTIONS> WatchWMEsTypeBitset;
}
#endif /* CORE_CLI_SRC_CLI_CLI_ENUMS_H_ */
