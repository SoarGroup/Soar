/////////////////////////////////////////////////////////////////
// watch command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"

#include "misc.h"
#include "agent.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoTrace(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting, bool fromWatch)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.none())
    {
        // Print watch settings.
        int learning;
        if (!thisAgent->trace_settings[TRACE_CHUNK_NAMES_SYSPARAM]
                && !thisAgent->trace_settings[TRACE_CHUNKS_SYSPARAM]
                && !thisAgent->trace_settings[TRACE_JUSTIFICATION_NAMES_SYSPARAM]
                && !thisAgent->trace_settings[TRACE_JUSTIFICATIONS_SYSPARAM])
        {
            learning = 0;
        }
        else if (thisAgent->trace_settings[TRACE_CHUNK_NAMES_SYSPARAM]
                 && !thisAgent->trace_settings[TRACE_CHUNKS_SYSPARAM]
                 && thisAgent->trace_settings[TRACE_JUSTIFICATION_NAMES_SYSPARAM]
                 && !thisAgent->trace_settings[TRACE_JUSTIFICATIONS_SYSPARAM])
        {
            learning = 1;
        }
        else
        {
            learning = 2;
        }
        
        if (m_RawOutput)
        {
            PrintCLIMessage_Header("Soar Trace Messages", 60);
            PrintCLIMessage_Section("Level 1", 60);
            PrintCLIMessage_Justify("Operator decisions and states", (thisAgent->trace_settings[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off"), 60, "-d, --decisions");
            PrintCLIMessage_Section("Level 2", 60);
            PrintCLIMessage_Justify("Phases", (thisAgent->trace_settings[TRACE_PHASES_SYSPARAM] ? "on" : "off"), 60, "-p, --phases");
            PrintCLIMessage_Justify("State removals caused by GDS violation", (thisAgent->trace_settings[TRACE_GDS_STATE_REMOVAL_SYSPARAM] ? "on" : "off"), 60, "-g, --gds");
            PrintCLIMessage_Justify("Chunking warnings", (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM] ? "on" : "off"), 60, "-C, --chunk-warnings");
            PrintCLIMessage_Section("Level 3: Rule firings", 60);
            PrintCLIMessage_Justify("Default rules", (thisAgent->trace_settings[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off"), 60, "-D, --default");
            PrintCLIMessage_Justify("User rules", (thisAgent->trace_settings[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off"), 60, "-u, --user");
            PrintCLIMessage_Justify("Chunks", (thisAgent->trace_settings[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off"), 60, "-c, --chunks");
            PrintCLIMessage_Justify("Justifications", (thisAgent->trace_settings[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off"), 60, "-j, --justifications");
            PrintCLIMessage_Justify("Templates", (thisAgent->trace_settings[TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM] ? "on" : "off"), 60, "-T, --template");
            PrintCLIMessage_Justify("Firings inhibited by higher-level firings", (thisAgent->trace_settings[TRACE_WATERFALL_SYSPARAM] ? "on" : "off"), 60, "-W, --waterfall");
            PrintCLIMessage_Section("Level 4", 60);
            PrintCLIMessage_Justify("WME additions and removals", (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off"), 60, "-w, --wmes");
            PrintCLIMessage_Section("Level 5", 60);
            PrintCLIMessage_Justify("Preferences", (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off"), 60, "-r, --preferences");
            PrintCLIMessage_Section("Additional EBC Trace Messages", 60);
            PrintCLIMessage_Justify("Dependency analysis", (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off"), 60, "-b, --backtracing");
            PrintCLIMessage_Justify("Rules Learned Verbosity Level", ((learning == 0) ? "none (0)" : ((learning == 1) ? "rule name (1)" : "full rules (2)")), 60, "-L, --learning [0-2]");
            PrintCLIMessage_Section("Additional Soar Trace Messages", 60);
            PrintCLIMessage_Justify("Goal dependency set changes", (thisAgent->trace_settings[TRACE_GDS_WMES_SYSPARAM] ? "on" : "off"), 60, "-G, --gds-wmes");
            PrintCLIMessage_Justify("Numeric preference calculations", (thisAgent->trace_settings[TRACE_INDIFFERENT_SYSPARAM] ? "on" : "off"), 60, "-i, --indifferent-selection");
            PrintCLIMessage_Justify("Reinforcement learning value updates", (thisAgent->trace_settings[TRACE_RL_SYSPARAM] ? "on" : "off"), 60, "-R, --rl");
            PrintCLIMessage_Section("Additional Memory System Trace Messages", 60);
            PrintCLIMessage_Justify("Episodic memory recording and queries", (thisAgent->trace_settings[TRACE_EPMEM_SYSPARAM] ? "on" : "off"), 60, "-e, --epmem");
            PrintCLIMessage_Justify("Semantic memory additions", (thisAgent->trace_settings[TRACE_SMEM_SYSPARAM] ? "on" : "off"), 60, "-s, --smem");
            PrintCLIMessage_Justify("Working memory activation and forgetting", (thisAgent->trace_settings[TRACE_WMA_SYSPARAM] ? "on" : "off"), 60, "-a, --wma");
            PrintCLIMessage(" ");
            PrintCLIMessage_Justify("WME Detail Level", ((thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] == NONE_WME_TRACE) ? "none" :
                ((thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] == TIMETAG_WME_TRACE) ? "timetag" : "full detail")), 60, "--nowmes, --timetags, --fullwmes");
        }
        else
        {
            std::string temp;
            AppendArgTag(sml_Names::kParamWatchDecisions, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchPhases, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_PHASES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchProductionDefault, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchProductionUser, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchProductionChunks, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchProductionJustifications, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchProductionTemplates, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            // Subtract one here because the kernel constants (e.g. TIMETAG_WME_TRACE) are one plus the number we use
            AppendArgTag(sml_Names::kParamWatchWMEDetail, sml_Names::kTypeInt,
                         to_string(thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] - 1, temp));
                         
            AppendArgTag(sml_Names::kParamWatchWorkingMemoryChanges, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchPreferences, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchLearning, sml_Names::kTypeInt,
                         to_string(learning, temp));
                         
            AppendArgTag(sml_Names::kParamWatchBacktracing, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchIndifferentSelection, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_INDIFFERENT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchRL, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_RL_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchWaterfall, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_WATERFALL_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchEpMem, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_EPMEM_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchSMem, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_SMEM_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchWMA, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_WMA_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
                         
            AppendArgTag(sml_Names::kParamWatchGDS, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_GDS_WMES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

            AppendArgTag(sml_Names::kParamWatchGDSStateRemoval, sml_Names::kTypeBoolean,
                         thisAgent->trace_settings[TRACE_GDS_STATE_REMOVAL_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
        }
        
        return true;
    }
    
    std::string traceFeedback;

    // No watch level and no none flags, that means we have to do the rest
    if (options.test(WATCH_BACKTRACING))
    {
        set_trace_setting(thisAgent, TRACE_BACKTRACING_SYSPARAM, settings.test(WATCH_BACKTRACING));
        traceFeedback.append(settings.test(WATCH_BACKTRACING) ? "Now printing " : "Will not print ");
        traceFeedback.append("chunking's dependency analysis trace messages.\n");
    }
    
    if (options.test(WATCH_CHUNKS))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, settings.test(WATCH_CHUNKS));
        traceFeedback.append(settings.test(WATCH_CHUNKS) ? "Now printing " : "Will not print ");
        traceFeedback.append("when chunks fire.\n");
    }
    
    if (options.test(WATCH_CHUNK_WARNINGS))
    {
        set_trace_setting(thisAgent, TRACE_CHUNKS_WARNINGS_SYSPARAM, settings.test(WATCH_CHUNK_WARNINGS));
        traceFeedback.append(settings.test(WATCH_CHUNK_WARNINGS) ? "Now printing " : "Will not print ");
        traceFeedback.append("warnings when issues detected while learning rules.\n");
    }

    if (options.test(WATCH_DECISIONS))
    {
        set_trace_setting(thisAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, settings.test(WATCH_DECISIONS));
        traceFeedback.append(settings.test(WATCH_DECISIONS) ? "Now printing " : "Will not print ");
        traceFeedback.append("states created and operators selected.\n");
    }
    
    if (options.test(WATCH_DEFAULT))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, settings.test(WATCH_DEFAULT));
        traceFeedback.append(settings.test(WATCH_DEFAULT) ? "Now printing " : "Will not print ");
        traceFeedback.append("when rules marked as :default fire.\n");
    }
    
    if (options.test(WATCH_GDS_WMES))
    {
        set_trace_setting(thisAgent, TRACE_GDS_WMES_SYSPARAM, settings.test(WATCH_GDS_WMES));
        traceFeedback.append(settings.test(WATCH_GDS_WMES) ? "Now printing " : "Will not print ");
        traceFeedback.append("all WMEs added or removed from the Goal Dependency Set.\n");
    }
    
    if (options.test(WATCH_GDS_STATE_REMOVAL))
    {
        set_trace_setting(thisAgent, TRACE_GDS_STATE_REMOVAL_SYSPARAM, settings.test(WATCH_GDS_STATE_REMOVAL));
        traceFeedback.append(settings.test(WATCH_GDS_STATE_REMOVAL) ? "Now printing " : "Will not print ");
        traceFeedback.append("when a state is removed because of a GDS violation.\n");
    }

    if (options.test(WATCH_INDIFFERENT))
    {
        set_trace_setting(thisAgent, TRACE_INDIFFERENT_SYSPARAM, settings.test(WATCH_INDIFFERENT));
        traceFeedback.append(settings.test(WATCH_INDIFFERENT) ? "Now printing " : "Will not print ");
        traceFeedback.append("how Soar calculates and resolves numeric preferences.\n");
    }
    
    if (options.test(WATCH_RL))
    {
        set_trace_setting(thisAgent, TRACE_RL_SYSPARAM, settings.test(WATCH_RL));
        traceFeedback.append(settings.test(WATCH_RL) ? "Now printing " : "Will not print ");
        traceFeedback.append("reinforcement learning value updates and gap intervals.\n");
    }
    
    if (options.test(WATCH_EPMEM))
    {
        set_trace_setting(thisAgent, TRACE_EPMEM_SYSPARAM, settings.test(WATCH_EPMEM));
        traceFeedback.append(settings.test(WATCH_EPMEM) ? "Now printing " : "Will not print ");
        traceFeedback.append("when episodic memory records a new episode or considers an episode in a query.\n");
    }
    
    if (options.test(WATCH_JUSTIFICATIONS))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, settings.test(WATCH_JUSTIFICATIONS));
        traceFeedback.append(settings.test(WATCH_JUSTIFICATIONS) ? "Now printing " : "Will not print ");
        traceFeedback.append("when justifications fire.\n");
    }
    
    if (options.test(WATCH_TEMPLATES))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM, settings.test(WATCH_TEMPLATES));
        traceFeedback.append(settings.test(WATCH_TEMPLATES) ? "Now printing " : "Will not print ");
        traceFeedback.append("when templates match.\n");
    }
    
    if (options.test(WATCH_PHASES))
    {
        set_trace_setting(thisAgent, TRACE_PHASES_SYSPARAM, settings.test(WATCH_PHASES));
        traceFeedback.append(settings.test(WATCH_PHASES) ? "Now printing " : "Will not print ");
        traceFeedback.append("each individual phase.\n");
    }
    
    if (options.test(WATCH_PREFERENCES))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, settings.test(WATCH_PREFERENCES));
        traceFeedback.append(settings.test(WATCH_PREFERENCES) ? "Now printing " : "Will not print ");
        traceFeedback.append("preferences as they are created.\n");
    }
    
    if (options.test(WATCH_SMEM))
    {
        set_trace_setting(thisAgent, TRACE_SMEM_SYSPARAM, settings.test(WATCH_SMEM));
        traceFeedback.append(settings.test(WATCH_SMEM) ? "Now printing " : "Will not print ");
        traceFeedback.append("additions to semantic memory.\n");
    }
    
    if (options.test(WATCH_USER))
    {
        set_trace_setting(thisAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, settings.test(WATCH_USER));
        traceFeedback.append(settings.test(WATCH_USER) ? "Now printing " : "Will not print ");
        traceFeedback.append("when user rules fire.\n");
    }
    
    if (options.test(WATCH_WMES))
    {
        set_trace_setting(thisAgent, TRACE_WM_CHANGES_SYSPARAM, settings.test(WATCH_WMES));
        traceFeedback.append(settings.test(WATCH_WMES) ? "Now printing " : "Will not print ");
        traceFeedback.append("when working memory elements are added to or removed from memory.\n");
    }
    
    if (options.test(WATCH_WATERFALL))
    {
        set_trace_setting(thisAgent, TRACE_WATERFALL_SYSPARAM, settings.test(WATCH_WATERFALL));
        traceFeedback.append(settings.test(WATCH_WATERFALL) ? "Now printing " : "Will not print ");
        traceFeedback.append("when rules do not fire because a higher level rule matches and needs to fire first.\n");
    }
    
    if (options.test(WATCH_WMA))
    {
        set_trace_setting(thisAgent, TRACE_WMA_SYSPARAM, settings.test(WATCH_WMA));
        traceFeedback.append(settings.test(WATCH_WMA) ? "Now printing " : "Will not print ");
        traceFeedback.append("working memory activations, changed values, and removals caused by forgetting (if enabled).\n");
    }
    
    if (options.test(WATCH_LEARNING))
    {
        switch (learnSetting)
        {
            default:
            // falls through
            case 0:
                set_trace_setting(thisAgent, TRACE_CHUNK_NAMES_SYSPARAM, false);
                set_trace_setting(thisAgent, TRACE_CHUNKS_SYSPARAM, false);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, false);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
                traceFeedback.append("Will not print any information about chunks or justifications learned.\n");

                break;
            case 1:
                set_trace_setting(thisAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
                set_trace_setting(thisAgent, TRACE_CHUNKS_SYSPARAM, false);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
                traceFeedback.append("Now printing the names of chunks and justifications that are learned and any chunking issues detected.\n");
                break;
            case 2:
                set_trace_setting(thisAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
                set_trace_setting(thisAgent, TRACE_CHUNKS_SYSPARAM, true);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
                set_trace_setting(thisAgent, TRACE_JUSTIFICATIONS_SYSPARAM, true);
                traceFeedback.append("Now printing the full chunks and justifications that are learned and any chunking issues detected.\n");
                break;
        }
    }
    
    if (options.test(WATCH_WME_DETAIL))
    {
        switch (wmeSetting)
        {
            default:
            // falls through
            case 0:
                set_trace_setting(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
                traceFeedback.append("Will not print working memory element details.\n");
                break;
            case 1:
                set_trace_setting(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
                traceFeedback.append("Will only print working memory element timetags.\n");
                break;
            case 2:
                set_trace_setting(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
                traceFeedback.append("Will print the full working memory element.\n");
                break;
        }
    }
    if (!fromWatch)
    {
        PrintCLIMessage(traceFeedback.c_str());
    }
    return true;
}
