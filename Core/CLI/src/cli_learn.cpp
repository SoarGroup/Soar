/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"

using namespace cli;
using namespace sml;

void GetForceLearnStates(agent* agnt, std::stringstream& res) {
    cons *c;
    char buff[1024];

    for (c = agnt->chunky_problem_spaces; c != NIL; c = c->rest) {
        symbol_to_string(agnt, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
        res << buff;
    }
}

void GetDontLearnStates(agent* agnt, std::stringstream& res) {
    cons *c;
    char buff[1024];

    for (c = agnt->chunk_free_problem_spaces; c != NIL; c = c->rest) {
        symbol_to_string(agnt, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
        res << buff;
    }
}

bool CommandLineInterface::DoLearn(const LearnBitset& options) {
    // No options means print current settings
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (options.none() || options.test(LEARN_LIST)) {

        if (m_RawOutput) {
            if (agnt->sysparams[LEARNING_ON_SYSPARAM]) {
                m_Result << "Learning is enabled.";
                if (agnt->sysparams[LEARNING_ONLY_SYSPARAM]) m_Result << " (only)";
                if (agnt->sysparams[LEARNING_EXCEPT_SYSPARAM]) m_Result << " (except)";
                if (agnt->sysparams[LEARNING_ALL_GOALS_SYSPARAM]) m_Result << " (all-levels)";
                if (agnt->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM]) {
                    m_Result << " (through-local-negations)";
                } else {
                    m_Result << " (not through-local-negations)";
                }

            } else {
                m_Result << "Learning is disabled.";
            }
        } else {
            AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, agnt->sysparams[LEARNING_ON_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, agnt->sysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, agnt->sysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, agnt->sysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
        }

        if (options.test(LEARN_LIST)) {
            std::stringstream output;
            if (m_RawOutput) {
                m_Result << "\nforce-learn states (when learn 'only'):";
                GetForceLearnStates(agnt, output);
                if (output.str().size()) m_Result << '\n' << output.str();

                m_Result << "\ndont-learn states (when learn 'except'):";
                GetDontLearnStates(agnt, output);
                if (output.str().size()) m_Result << '\n' << output.str();

            } else {
                GetForceLearnStates(agnt, output);
                AppendArgTagFast(sml_Names::kParamLearnForceLearnStates, sml_Names::kTypeString, output.str());
                GetDontLearnStates(agnt, output);
                AppendArgTagFast(sml_Names::kParamLearnDontLearnStates, sml_Names::kTypeString, output.str());
            }
        }
        return true;
    }

    if (options.test(LEARN_ONLY)) {
        set_sysparam(agnt, LEARNING_ON_SYSPARAM, true);
        set_sysparam(agnt, LEARNING_ONLY_SYSPARAM, true);
        set_sysparam(agnt, LEARNING_EXCEPT_SYSPARAM, false);
    }

    if (options.test(LEARN_EXCEPT)) {
        set_sysparam(agnt, LEARNING_ON_SYSPARAM, true);
        set_sysparam(agnt, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(agnt, LEARNING_EXCEPT_SYSPARAM, true);
    }

    if (options.test(LEARN_ENABLE)) {
        set_sysparam(agnt, LEARNING_ON_SYSPARAM, true);
        set_sysparam(agnt, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(agnt, LEARNING_EXCEPT_SYSPARAM, false);
    }

    if (options.test(LEARN_DISABLE)) {
        set_sysparam(agnt, LEARNING_ON_SYSPARAM, false);
        set_sysparam(agnt, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(agnt, LEARNING_EXCEPT_SYSPARAM, false);
    }

    if (options.test(LEARN_ALL_LEVELS)) {
        set_sysparam(agnt, LEARNING_ALL_GOALS_SYSPARAM, true);
    }

    if (options.test(LEARN_BOTTOM_UP)) {
        set_sysparam(agnt, LEARNING_ALL_GOALS_SYSPARAM, false);
    }

    if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS)) {
        set_sysparam(agnt, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, true);
    }

    if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS)) {
        set_sysparam(agnt, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, false);
    }

    return true;
}

