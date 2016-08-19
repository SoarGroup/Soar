/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             errors.cpp

   @brief This file contains functions that print error messages in
   the various parts of the kernel.  Moving it here allows the logic
   to be cleaner and easier to read.

------------------------------------------------------------------ */

#include "output_manager.h"
#include "agent.h"
#include "ebc.h"
#include "xml.h"

void Output_Manager::display_ebc_error(agent* thisAgent, EBCFailureType pErrorType, const char* pString1, const char* pString2)
{
    if (!thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) return;
    switch (pErrorType)
    {
        case ebc_failed_reordering_rhs:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Chunking has created an invalid rule:");

            printa_sf(thisAgent,  "   Reason:  RHS actions contain the following variables/long-term \n"
                                  "   identifiers that are not tested in a positive condition \n\n"
                                  "   on the LHS: %s\n\n", pString2);
            break;
        }
        case ebc_failed_unconnected_conditions:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Chunking has created an invalid rule:");

            printa_sf(thisAgent,"   Reason: Conditions on the LHS test the following identifiers that are not connected \n"
                                "   to a goal: %s\n\n", pString2);
            printa(thisAgent,   "   This is likely caused by a condition that tested either\n"
                                "      (a) a semantic memory retrieved in the sub-state that already \n"
                                "          exists in a super-state \n"
                                "      (b) a working memory element that was created in the sub-state \n"
                                "          and then connected to the super-state during problem-solving.\n\n");
            break;
        }
        case ebc_failed_no_roots:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Chunking has created an invalid rule:");
//            printa_sf(thisAgent,"\nChunking has created an invalid rule: %s\n\n", pString1);
            printa(   thisAgent,  "   None of the conditions reference a goal state.\n\n");
            break;
        }
        case ebc_failed_negative_relational_test_bindings:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Chunking has created an invalid rule:");
//            printa_sf(thisAgent,"\nChunking has created an invalid rule: %s\n\n", pString1);
            printa(thisAgent,  "   Unbound relational test in negative condition of rule \n\n");
            break;
        }
        default:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Chunking has created an invalid rule:");
            printa(thisAgent, "\nUnspecified chunking failure. That's weird.  Should report.\n\n");
            printa_sf(thisAgent, "        %s\n\n", pString1);
        }
    }
}

void Output_Manager::display_soar_feedback(agent* thisAgent, SoarCannedMessageType pMessageType, int64_t pSysParam)
{
    if ( (pSysParam > 0) && !thisAgent->sysparams[pSysParam])
    {
        return;
    }

    switch (pMessageType)
    {
        case ebc_error_max_chunks:
        {
            printa(thisAgent, "Warning: Maximum number of chunks reached.  Skipping opportunity to learn new rule.\n");
            break;
        }
        case ebc_error_max_dupes:
        {
            printa(thisAgent, "Warning: Rule has produced maximum number of duplicate chunks this decision cycle.  Skipping opportunity to learn new rule.\n");
            break;
        }
        case ebc_error_invalid_chunk:
        {
            printa(thisAgent, "...repair failed...\n");
            break;
        }
        case ebc_error_invalid_justification:
        {
            printa(thisAgent, "Warning:  Chunking produced an invalid justification.  Ignoring.\n");
            break;
        }
        case ebc_progress_repairing:
        {
            printa(thisAgent, "...attempting to repair rule.\n");
            break;
        }
        case ebc_progress_repaired:
        {
            printa(thisAgent, "...repair succeeded.\n");
            break;
        }
        case ebc_error_no_conditions:
        {
            printa(thisAgent, "Warning: Chunking has produced a rule with no conditions.  Ignoring.\n");
            printa(thisAgent, "         To avoid this issue, the problem-solving in the substate must\n");
            printa(thisAgent, "         positively test at least one item in the superstate.\n");
            break;
        }
        default:
        {
            printa(thisAgent, "Warning: Unspecified soar error. That's weird.  Should report.\n");
        }
    }
}

void Output_Manager::display_ambiguous_command_error(agent* thisAgent, const std::list< std::string > matched_objects_str)
{
    std::string last_p;
    for (auto p = matched_objects_str.begin(); p != matched_objects_str.end();)
    {
        last_p = (*p++);

        if (p == matched_objects_str.end())
        {
            thisAgent->outputManager->printa_sf(thisAgent, ", or %s?", last_p.c_str());
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, " %s", last_p.c_str());
        }
    }
}

