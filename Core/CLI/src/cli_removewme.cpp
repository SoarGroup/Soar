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

#include "sml_Names.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"

#include "agent.h"
#include "utilities.h"
#include "wmem.h"
#include "symtab.h"
#include "decide.h"
#include "gdatastructs.h"
#include "print.h"
#include "soar_TraceNames.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoRemoveWME(uint64_t timetag) {
    wme *pWme = 0;
    agent* agnt = m_pAgentSML->GetSoarAgent();

    for ( pWme = agnt->all_wmes_in_rete; pWme != 0; pWme = pWme->rete_next )
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
        if ( pWme->gds ) 
        {
            if ( pWme->gds->goal != 0 ) 
            {
                gds_invalid_so_remove_goal( agnt, pWme );
                /* NOTE: the call to remove_wme_from_wm will take care of checking if
                GDS should be removed */
            }
        }
        /* REW: end   09.15.96 */

        // now remove w from working memory
        remove_wme_from_wm( agnt, pWme );

#ifndef NO_TOP_LEVEL_REFS
        do_buffered_wm_and_ownership_changes( agnt );
#endif // NO_TOP_LEVEL_REFS
    }

    return true;
}
