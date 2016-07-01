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
#include "xml.h"

void Output_Manager::display_ebc_error(agent* thisAgent, EBCFailureType pErrorType, const char* pString1, const char* pString2)
{
    if (!thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) return;
    switch (pErrorType)
    {
        case ebc_failed_reordering_rhs:
        {
            printa(thisAgent,"\nChunking has created an invalid rule:\n");
            printa_sf(thisAgent,"   %s\n\n", pString1);
            printa(thisAgent,     "   RHS actions contain variables/long-term identifiers\n"
                                  "   that are not tested in a positive condition on the LHS.\n\n"
                                  "   Note:  Negative conditions don't count.  This error may also\n"
                                  "          occur if you pass an unbound variable as an argument \n"
                                  "          to a RHS function.\n");
            printa_sf(thisAgent,"\nProblem RHS Actions:\n%s\n", pString2);
            break;
        }
        case ebc_failed_unconnected_conditions:
        {
            printa(thisAgent,"\nChunking has created an invalid rule:\n");
            printa_sf(thisAgent,"   %s\n\n", pString1);
            printa(thisAgent,   "   A LHS condition is not connected to a goal.  This is likely caused \n"
                                "   by a condition in a matched rule that tested either\n"
                                "      (a) a long-term identifier retrieved in the sub-state that also \n"
                                "          exists in a super-state or "
                                "      (b) a working memory element that was created in the sub-state \n"
                                "          and then later linked to a WME in the super-state at some \n"
                                "          intermediate point during problem-solving.\n\n");
            printa_sf(thisAgent,"\nUnconnected identifiers: %s\n", pString2);
            break;
        }
        case ebc_failed_no_roots:
        {
            printa(thisAgent,"\nChunking has created an invalid rule:\n");
            printa_sf(thisAgent,"        %s\n\n", pString1);
            printa(   thisAgent,  "        None of the conditions reference a goal state.\n");
            break;
        }
        case ebc_failed_negative_relational_test_bindings:
        {
            printa(thisAgent,"\nChunking has created an invalid rule:\n");
            printa_sf(thisAgent,"        %s\n\n", pString1);
            printa(thisAgent,  "        Unbound relational test in negative condition of rule \n");
            break;
        }
        default:
        {
            printa(thisAgent, "\nUnspecified chunking failure. That's weird.  Should report.\n");
            printa_sf(thisAgent, "        %s\n\n", pString1);
        }
    }
}

void Output_Manager::display_soar_warning(agent* thisAgent, SoarError pErrorType, int64_t pSysParam)
{
    if ( (pSysParam > 0) && !thisAgent->sysparams[pSysParam])
    {
        return;
    }

    switch (pErrorType)
    {
        case ebc_error_max_chunks:
        {
            printa(thisAgent, "Warning: Maximum number of chunks reached.  Skipping opportunity to learn new rule.\n");
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
        case ebc_error_repairing:
        {
            printa(thisAgent, "...attempting to repair rule.\n");
            break;
        }
        case ebc_error_repaired:
        {
            printa(thisAgent, "...repair succeeded.\n");
            break;
        }
        case ebc_error_reverted_chunk:
        {
            printa(thisAgent, "\n...successfully generated justification for failed chunk.\n");
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
