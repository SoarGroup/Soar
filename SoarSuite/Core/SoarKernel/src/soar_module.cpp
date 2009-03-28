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

wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, Bool acceptable);

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

		remove_from_dll( my_slot->wmes, w, next, prev );
		remove_wme_from_wm( my_agent, w );
	}

}

