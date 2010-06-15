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

bool CommandLineInterface::ParseGDSPrint(std::vector<std::string>&) {
	return DoGDSPrint();
}

bool CommandLineInterface::DoGDSPrint() {
	wme *w;
	Symbol *goal;


	print(m_pAgentSoar, "********************* Current GDS **************************\n");
	print(m_pAgentSoar, "stepping thru all wmes in rete, looking for any that are in a gds...\n");
	for (w=m_pAgentSoar->all_wmes_in_rete; w!=NIL; w=w->rete_next) 
	{
		if (w->gds)
		{
			if (w->gds->goal) 
			{
				print_with_symbols (m_pAgentSoar, "  For Goal  %y  ", w->gds->goal);
			} 
			else 
			{
				print(m_pAgentSoar, "  Old GDS value ");
			}
			print (m_pAgentSoar, "(%lu: ", w->timetag);
			print_with_symbols (m_pAgentSoar, "%y ^%y %y", w->id, w->attr, w->value);
			if (w->acceptable) 
			{
				print_string (m_pAgentSoar, " +");
			}
			print_string (m_pAgentSoar, ")");
			print (m_pAgentSoar, "\n");
		}
	}
	print(m_pAgentSoar, "************************************************************\n");
	for (goal=m_pAgentSoar->top_goal; goal!=NIL; goal=goal->id.lower_goal)
	{
		print_with_symbols (m_pAgentSoar, "  For Goal  %y  ", goal);
		if (goal->id.gds){
			/* Loop over all the WMEs in the GDS */
			print (m_pAgentSoar, "\n");
			for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next)
			{
				print (m_pAgentSoar, "                (%lu: ", w->timetag);
				print_with_symbols (m_pAgentSoar, "%y ^%y %y", w->id, w->attr, w->value);
				if (w->acceptable) 
				{
					print_string (m_pAgentSoar, " +");
				}
				print_string (m_pAgentSoar, ")");
				print (m_pAgentSoar, "\n");
			}

		} 
		else 
		{
			print(m_pAgentSoar, ": No GDS for this goal.\n");
		}
	}

	print(m_pAgentSoar, "************************************************************\n");
	return true;
}

