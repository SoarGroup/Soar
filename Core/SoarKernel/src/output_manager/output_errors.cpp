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

void Output_Manager::display_reorder_error(agent* thisAgent, ProdReorderFailureType pErrorType, const char* pString1, const char* pString2)
{
    if (!thisAgent->outputManager->settings[OM_WARNINGS]) return;
    switch (pErrorType)
    {
        case reorder_failed_reordering_rhs:
        {
            printa_sf(thisAgent,  "%eAttempted to add rule with ungrounded action(s).\n"
                                  "The following RHS actions contain variables that are not tested\n"
                                  "in a positive condition on the LHS: \n\n"
                                  "%s\n", pString2);
            break;
        }
        case reorder_failed_unconnected_conditions:
        {
            printa_sf(thisAgent,"%eConditions on the LHS contain tests that are not connected \n"
                                "to a goal: %s\n\n", pString2);
            break;
        }
        case reorder_failed_no_roots:
        {
            printa_sf(thisAgent, "Error: production %s has no positive conditions that reference a goal state.\n"\
                "Did you forget to add \"^type state\" or \"^superstate nil\"?\n",
                thisAgent->name_of_production_being_reordered);
            break;
        }
        case reorder_failed_negative_relational_test_bindings:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Attempted to add an invalid rule:");
            printa(thisAgent,  "   Unbound relational test in negative condition of rule \n");
            break;
        }
        default:
        {
            thisAgent->explanationBasedChunker->print_current_built_rule("Attempted to add an invalid rule:");
            printa(thisAgent, "\nUnspecified reordering/validation failure. That's weird. Should report.\n\n");
            printa_sf(thisAgent, "        %s\n", pString1);
        }
    }
}

void Output_Manager::display_soar_feedback(agent* thisAgent, SoarCannedMessageType pMessageType, bool shouldPrint)
{
    if ( !shouldPrint)
    {
        return;
    }

    switch (pMessageType)
    {
        case ebc_error_max_chunks:
        {
            printa_sf(thisAgent, "%eWarning: Maximum number of chunks reached.  Skipping opportunity to learn new rule.\n");
            break;
        }
        case ebc_error_max_dupes:
        {
            printa_sf(thisAgent, "%eWarning: Rule has produced maximum number of duplicate chunks this decision cycle.  Skipping opportunity to learn new rule.\n");
            break;
        }
        case ebc_error_invalid_chunk:
        {
            printa(thisAgent, "...repair failed.\n");
            break;
        }
        case ebc_error_invalid_justification:
        {
            printa_sf(thisAgent, "%eWarning:  Chunking produced an invalid justification.  Ignoring.\n");
            break;
        }
        case ebc_progress_validating:
        {
            printa(thisAgent, "Validating repaired rule.\n");
            break;
        }
        case ebc_progress_repairing:
        {
            printa(thisAgent, "Attempting to repair rule.\n");
            break;
        }
        case ebc_progress_repaired:
        {
            printa(thisAgent, "...repair succeeded.\n");
            break;
        }
        case ebc_error_no_conditions:
        {
            printa(thisAgent, "\nWarning: Soar has created a chunk or justification with no conditions.  Ignoring.\n\n"
                                "         Any results created will lose support when the sub-state disappears.\n"
                                "         To avoid this issue, the problem-solving in the sub-state must\n"
                                "         positively test at least one item in the super-state.\n");
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
            thisAgent->outputManager->printa_sf(thisAgent, ", or %s?\n", last_p.c_str());
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, " %s", last_p.c_str());
        }
    }
}

