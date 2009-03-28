#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.cpp
 *
 * =======================================================================
 * Description  :  Useful functions for Soar modules
 * =======================================================================
 */

#include "soar_module.h"
#include "tempmem.h"
#include "gdatastructs.h"
#include "mem.h"
#include "print.h"
#include "decide.h"
#include "agent.h"
#include "xml.h"
#include "soar_TraceNames.h"

wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, Bool acceptable);
typedef struct agent_struct agent;

namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value )
	{
		slot *my_slot = make_slot( my_agent, id, attr );
		wme *w = make_wme( my_agent, id, attr, value, false );

		insert_at_head_of_dll( my_slot->wmes, w, next, prev );
		add_wme_to_wm( my_agent, w );

		return w;
	}

	void remove_module_wme( agent *my_agent, wme *w )
	{
		slot *my_slot = find_slot( w->id, w->attr );

		if ( my_slot )
		{
			remove_from_dll( my_slot->wmes, w, next, prev );

			if ( my_agent->operand2_mode )
			{
				if ( w->gds ) 
				{
					if ( w->gds->goal != NIL )
					{	             
						if ( my_agent->soar_verbose_flag || my_agent->sysparams[TRACE_WM_CHANGES_SYSPARAM] )
						{
							char buf[256];
							SNPRINTF( buf, 254, "remove_module_wme: Removing state S%d because element in GDS changed.", w->gds->goal->id.level );
							print( my_agent, buf );
							print( my_agent, " WME: " ); 
							
							xml_begin_tag( my_agent, soar_TraceNames::kTagVerbose );
							xml_att_val( my_agent, soar_TraceNames::kTypeString, buf );
							print_wme( my_agent, w );
							xml_end_tag( my_agent, soar_TraceNames::kTagVerbose );
						}
						
						gds_invalid_so_remove_goal( my_agent, w );
						
						/* NOTE: the call to remove_wme_from_wm will take care of checking if GDS should be removed */
					}
				}
			}

			remove_wme_from_wm( my_agent, w );
		}
	}
}

