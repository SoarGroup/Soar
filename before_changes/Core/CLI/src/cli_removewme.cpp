/////////////////////////////////////////////////////////////////
// remove-wme command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

#include "utilities.h"
#include "wmem.h"
#include "symtab.h"
#include "decide.h"
#include "gdatastructs.h"
#include "print.h"
#include "soar_TraceNames.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRemoveWME(std::vector<std::string>& argv) {
	// Exactly one argument
	if (argv.size() < 2) {
		SetErrorDetail("Please supply a timetag.");
		return SetError(CLIError::kTooFewArgs);
	}
	if (argv.size() > 2) {
		SetErrorDetail("Please supply only one timetag.");
		return SetError(CLIError::kTooManyArgs);
	}

	int timetag = atoi(argv[1].c_str());
	if (!timetag) return SetError(CLIError::kIntegerMustBePositive);

	return DoRemoveWME(timetag);
}

bool CommandLineInterface::DoRemoveWME(unsigned long timetag) {
	wme *pWme = 0;

	for ( pWme = m_pAgentSoar->all_wmes_in_rete; pWme != 0; pWme = pWme->rete_next )
	{
		if ( pWme->timetag == timetag )
		{
			break;
		}
	}

	if ( pWme )
	{
		Symbol* pId = pWme->id;

		// remove w from whatever list of wmes it's on
		for ( wme* pWme2 = pId->id.input_wmes; pWme2 != 0; pWme2 = pWme2->next)
		{
			if ( pWme == pWme2 )
			{
				remove_from_dll( pId->id.input_wmes, pWme, next, prev );
				break;
			}
		}

		for ( wme* pWme2 = pId->id.impasse_wmes; pWme2 != 0; pWme2 = pWme2->next )
		{
			if ( pWme == pWme2 )
			{
				remove_from_dll( pId->id.impasse_wmes, pWme, next, prev );
				break;
			}
		}

		for ( slot* s = pId->id.slots; s != 0; s = s->next ) 
		{

			for ( wme* pWme2 = s->wmes; pWme2 != 0; pWme2 = pWme2->next )
			{
				if ( pWme == pWme2 )
				{
					remove_from_dll( s->wmes, pWme, next, prev );
					break;
				}
			}

			for ( wme* pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next )
			{
				if ( pWme == pWme2 )
				{
					remove_from_dll( s->acceptable_preference_wmes, pWme, next, prev );
					break;
				}
			}
		}

		/* REW: begin 09.15.96 */
		if ( m_pAgentSoar->operand2_mode ) 
		{
			if ( pWme->gds ) 
			{
				if ( pWme->gds->goal != 0 ) 
				{
					if ( m_pAgentSoar->soar_verbose_flag || m_pAgentSoar->sysparams[TRACE_WM_CHANGES_SYSPARAM] )
					{
						std::stringstream removeMessage;
						removeMessage << "DoRemoveWME: Removing state S" << pWme->gds->goal->id.level << " because element in GDS changed.";
						print( m_pAgentSoar, "\n%s", removeMessage.str() );
						print( m_pAgentSoar, " WME: " ); 

						xml_begin_tag( m_pAgentSoar, soar_TraceNames::kTagVerbose );
						xml_att_val( m_pAgentSoar, soar_TraceNames::kTypeString, removeMessage.str().c_str() );
						print_wme( m_pAgentSoar, pWme);
						xml_end_tag( m_pAgentSoar, soar_TraceNames::kTagVerbose );
					}

					gds_invalid_so_remove_goal( m_pAgentSoar, pWme );
					/* NOTE: the call to remove_wme_from_wm will take care of checking if
					GDS should be removed */
				}
			}
		}
		/* REW: end   09.15.96 */

		// now remove w from working memory
		remove_wme_from_wm( m_pAgentSoar, pWme );

#ifndef NO_TOP_LEVEL_REFS
		do_buffered_wm_and_ownership_changes( m_pAgentSoar );
#endif // NO_TOP_LEVEL_REFS
	}

	return true;
}
