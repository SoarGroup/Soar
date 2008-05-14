/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  episodic_memory.h
 *
 * =======================================================================
 */

#ifndef EPISODIC_MEMORY_H
#define EPISODIC_MEMORY_H

#include "symtab.h"
#include "wmem.h"

//////////////////////////////////////////////////////////
// EpMem Constants
//////////////////////////////////////////////////////////

//
// These must go below constants
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// EpMem Types
//////////////////////////////////////////////////////////
typedef struct epmem_data_struct 
{
	Symbol *state;				// top-state
	unsigned long last_tag;		// last update to output-link
	
	Symbol *id_epmem;
	Symbol *id_cmd;
	Symbol *id_result;			// id symbols for wme's
	
	wme *wme_epmem;
	wme *wme_cmd;
	wme *wme_result;			// wme references
} epmem_data;

//////////////////////////////////////////////////////////
// Core Functions
//////////////////////////////////////////////////////////

// init
extern void epmem_init( agent *my_agent );

// Grand Central Station of EpMem
extern void epmem_update( agent *my_agent );

// EpMem WM
extern void epmem_create_buffer( agent *my_agent, Symbol *s );

// Clean later
extern void epmem_clean_agent( agent *my_agent );

#endif
