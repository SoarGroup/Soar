#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  episodic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for EpMem
 * =======================================================================
 */

#include <stdlib.h>

#include <iostream>

#include "symtab.h"
#include "io_soar.h"
#include "wmem.h"

#include "episodic_memory.h"

/***************************************************************************
 * Function     : epmem_reset
 **************************************************************************/
void epmem_reset( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;
	while( goal )
	{
		epmem_data *data = goal->id.epmem_info;
				
		data->last_tag = 0;
		
		goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : epmem_update
 **************************************************************************/
void epmem_update( agent *my_agent )
{
	slot *s;
	wme *w;
	Symbol *ol = my_agent->io_header_output;
	bool new_memory = false;
		
	// examine all commands on the output-link for any
	// that appeared since last memory was recorded
	for ( s = ol->id.slots; s != NIL; s = s->next )
	{
		for ( w = s->wmes; w != NIL; w = w->next )
		{
			if ( w->timetag > my_agent->bottom_goal->id.epmem_info->last_tag )
			{
				new_memory = true;
				my_agent->bottom_goal->id.epmem_info->last_tag = w->timetag; 
			}
		}
	}
	
	if ( new_memory )
	{
		//std::cerr << "NEW EPISODE (" << my_agent->bottom_goal->id.name_letter << my_agent->bottom_goal->id.name_number << ")!!" << std::endl;
	}
}
