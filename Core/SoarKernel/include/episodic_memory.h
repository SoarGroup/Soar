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
	unsigned long last_tag;		// last update to output-link
} epmem_data;

//////////////////////////////////////////////////////////
// Core Functions
//////////////////////////////////////////////////////////

// init
extern void epmem_reset( agent *my_agent );

// Grand Central Station of EpMem
extern void epmem_update( agent *my_agent );

#endif
