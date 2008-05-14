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
 * Function     : epmem_init
 **************************************************************************/
void epmem_init( agent *my_agent )
{
	my_agent->epmem_header->last_tag = 0;
}

/***************************************************************************
 * Function     : epmem_update
 **************************************************************************/
void epmem_update( agent *my_agent )
{	
	return;
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
			if ( w->timetag > my_agent->epmem_header->last_tag )
			{
				new_memory = true;
				my_agent->epmem_header->last_tag = w->timetag; 
			}
		}
	}
	
	if ( new_memory )
	{
		std::cerr << "NEW EPISODE!!" << std::endl;
	}
}

/***************************************************************************
 * Function     : epmem_create_buffer
 **************************************************************************/
void epmem_create_buffer( agent *my_agent, Symbol *s )
{	
	// allocate a new epmem_data header
	epmem_data *d = (epmem_data *)allocate_memory( my_agent, sizeof( epmem_data ), MISCELLANEOUS_MEM_USAGE );
	
	// assign values
	d->state = s;
	
	// create epmem wme symbols
	d->id_epmem = make_new_identifier( my_agent, 'E', s->id.level );
	d->id_cmd = make_new_identifier( my_agent, 'C', s->id.level );
	d->id_result = make_new_identifier( my_agent, 'R', s->id.level );
	
	// create epmem wme's	
	d->wme_epmem = add_input_wme( my_agent, s, make_sym_constant( my_agent, "epmem" ), d->id_epmem );
	wme_add_ref( d->wme_epmem );
	d->wme_cmd = add_input_wme( my_agent, d->id_epmem, make_sym_constant( my_agent, "command" ), d->id_cmd );
	wme_add_ref( d->wme_cmd );
	d->wme_result = add_input_wme( my_agent, d->id_epmem, make_sym_constant( my_agent, "result" ), d->id_result );
	wme_add_ref( d->wme_result );
	
	// save on top-state
	my_agent->epmem_header = d;
}

/***************************************************************************
 * Function     : epmem_clean_agent
 **************************************************************************/
void epmem_clean_agent( agent *my_agent )
{	
	free_memory( my_agent, my_agent->epmem_header, MISCELLANEOUS_MEM_USAGE );
}
