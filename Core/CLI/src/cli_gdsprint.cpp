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
#include "wmem.h"
#include "decide.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoGDSPrint()
{
    wme* w;
    Symbol* goal;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    
    
    print(thisAgent,  "********************* Current GDS **************************\n");
    print(thisAgent,  "stepping thru all wmes in rete, looking for any that are in a gds...\n");
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        if (w->gds)
        {
            if (w->gds->goal)
            {
                print_with_symbols(thisAgent, "  For Goal  %y  ", w->gds->goal);
            }
            else
            {
                print(thisAgent,  "  Old GDS value ");
            }
            print(thisAgent,  "(%lu: ", w->timetag);
            print_with_symbols(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
            if (w->acceptable)
            {
                print(thisAgent,  " +");
            }
            print(thisAgent,  ")");
            print(thisAgent,  "\n");
        }
    }
    print(thisAgent,  "************************************************************\n");
    for (goal = thisAgent->top_goal; goal != NIL; goal = goal->id->lower_goal)
    {
        print_with_symbols(thisAgent, "  For Goal  %y  ", goal);
        if (goal->id->gds)
        {
            /* Loop over all the WMEs in the GDS */
            print(thisAgent,  "\n");
            for (w = goal->id->gds->wmes_in_gds; w != NIL; w = w->gds_next)
            {
                print(thisAgent,  "                (%lu: ", w->timetag);
                print_with_symbols(thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
                if (w->acceptable)
                {
                    print(thisAgent,  " +");
                }
                print(thisAgent,  ")");
                print(thisAgent,  "\n");
            }
            
        }
        else
        {
            print(thisAgent,  ": No GDS for this goal.\n");
        }
    }
    
    print(thisAgent,  "************************************************************\n");
    return true;
}

