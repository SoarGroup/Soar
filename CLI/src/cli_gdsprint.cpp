/////////////////////////////////////////////////////////////////
// gds-print command file.
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

#include "sml_KernelSML.h"
#include "print.h"
#include "agent.h"
#include "wmem.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoGDSPrint() {
	wme *w;
	Symbol *goal;
    agent* agnt = m_pAgentSML->GetSoarAgent();


	print(agnt, "********************* Current GDS **************************\n");
	print(agnt, "stepping thru all wmes in rete, looking for any that are in a gds...\n");
	for (w=agnt->all_wmes_in_rete; w!=NIL; w=w->rete_next) 
	{
		if (w->gds)
		{
			if (w->gds->goal) 
			{
				print_with_symbols (agnt, "  For Goal  %y  ", w->gds->goal);
			} 
			else 
			{
				print(agnt, "  Old GDS value ");
			}
			print (agnt, "(%lu: ", w->timetag);
			print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
			if (w->acceptable) 
			{
				print_string (agnt, " +");
			}
			print_string (agnt, ")");
			print (agnt, "\n");
		}
	}
	print(agnt, "************************************************************\n");
	for (goal=agnt->top_goal; goal!=NIL; goal=goal->id.lower_goal)
	{
		print_with_symbols (agnt, "  For Goal  %y  ", goal);
		if (goal->id.gds){
			/* Loop over all the WMEs in the GDS */
			print (agnt, "\n");
			for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next)
			{
				print (agnt, "                (%lu: ", w->timetag);
				print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
				if (w->acceptable) 
				{
					print_string (agnt, " +");
				}
				print_string (agnt, ")");
				print (agnt, "\n");
			}

		} 
		else 
		{
			print(agnt, ": No GDS for this goal.\n");
		}
	}

	print(agnt, "************************************************************\n");
	return true;
}

