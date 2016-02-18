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

void Output_Manager::display_soar_error(agent* thisAgent, SoarError pErrorType, int64_t pSysParam)
{
    if ( (pSysParam > 0) && !thisAgent->sysparams[pSysParam])
    {
        printa(thisAgent, "Warning: Setting off.\n");
        xml_generate_warning(thisAgent, "Warning: Setting off.");
        return;
    }

    switch (pErrorType)
    {
        case ebc_error_max_chunks:
        {
            printa(thisAgent, "Warning: We've formed the maximum number of chunks.  Skipping opportunity to learn new rule.\n");
            xml_generate_warning(thisAgent, "Warning: We've formed the maximum number of chunks.  Skipping opportunity to learn new rule.");
            break;
        }
        case ebc_error_invalid_chunk:
        {
            printa(thisAgent, "\nCould not create production for variablized rule. Creating justification instead.\n");
            printa(thisAgent, "\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            printa(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            printa(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            printa(thisAgent, "through the local state.\n");
            xml_generate_warning(thisAgent, "\nCould not create production for variablized rule. Creating justification instead.\n");
            xml_generate_warning(thisAgent, "\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            xml_generate_warning(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            xml_generate_warning(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            xml_generate_warning(thisAgent, "through the local state.\n\n");
            break;
        }
        case ebc_error_invalid_justification:
        {
            printa(thisAgent, "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            printa(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            printa(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            printa(thisAgent, "through the local state.\n");
            xml_generate_warning(thisAgent, "\nnUnable to reorder this chunk.\n");
            xml_generate_warning(thisAgent, "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            xml_generate_warning(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            xml_generate_warning(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            xml_generate_warning(thisAgent, "through the local state.\n");
            break;
        }
        case ebc_error_no_conditions:
        {
            printa(thisAgent, "Warning: Chunk/justification has no conditions.  Soar cannot learn a rule for\n");
            printa(thisAgent, "         the results created from this substate. To avoid this issue, the agent\n");
            printa(thisAgent, "         needs to positively test at least one item in the superstate.\n");
            xml_generate_warning(thisAgent, "Warning: Chunk/justification has no conditions.  Soar cannot learn a rule for");
            xml_generate_warning(thisAgent, "         the results created by this substate. To avoid this issue, the agent");
            xml_generate_warning(thisAgent, "         needs to positively test at least one item in the superstate.");
            break;
        }
        default:
        {
            printa(thisAgent, "Warning: Unspecified error. That's weird.  Should report.\n");
            xml_generate_warning(thisAgent, "Warning: Unspecified error. That's weird.  Should report.");
        }
    }
}
