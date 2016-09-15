/////////////////////////////////////////////////////////////////
// gds-print command file.
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
#include "sml_AgentSML.h"
#include "print.h"
#include "agent.h"
#include "working_memory.h"
#include "decide.h"
#include "symbol.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoGDSPrint()
{
    wme* w;
    Symbol* goal;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    
    
    thisAgent->outputManager->printa_sf(thisAgent,  "********************* Current GDS **************************\n");
    thisAgent->outputManager->printa_sf(thisAgent,  "stepping thru all wmes in rete, looking for any that are in a gds...\n");
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        if (w->gds)
        {
            if (w->gds->goal)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "  For Goal  %y  ", w->gds->goal);
            }
            else
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "  Old GDS value ");
            }
            thisAgent->outputManager->printa_sf(thisAgent,  "(%u: ", w->timetag);
            thisAgent->outputManager->printa_sf(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
            if (w->acceptable)
            {
                thisAgent->outputManager->printa(thisAgent, " +");
            }
            thisAgent->outputManager->printa(thisAgent, ")");
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        }
    }
    thisAgent->outputManager->printa_sf(thisAgent,  "************************************************************\n");
    for (goal = thisAgent->top_goal; goal != NIL; goal = goal->id->lower_goal)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "  For Goal  %y  ", goal);
        if (goal->id->gds)
        {
            /* Loop over all the WMEs in the GDS */
            thisAgent->outputManager->printa_sf(thisAgent,  "\n");
            for (w = goal->id->gds->wmes_in_gds; w != NIL; w = w->gds_next)
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "                (%u: ", w->timetag);
                thisAgent->outputManager->printa_sf(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
                if (w->acceptable)
                {
                    thisAgent->outputManager->printa(thisAgent, " +");
                }
                thisAgent->outputManager->printa(thisAgent, ")");
                thisAgent->outputManager->printa_sf(thisAgent,  "\n");
            }
            
        }
        else
        {
            thisAgent->outputManager->printa_sf(thisAgent,  ": No GDS for this goal.\n");
        }
    }
    
    thisAgent->outputManager->printa_sf(thisAgent,  "************************************************************\n");
    return true;
}

