/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                               epmem.h
======================================================================= */

#ifndef EPMEM_H
#define EPMEM_H

#include "symtab.h"

#ifdef __cplusplus
extern "C"
{
#endif

void init_epmem(agent *thisAgent);
void epmem_create_buffer(agent *thisAgent, Symbol *);
void epmem_update(agent *thisAgent);

#ifdef __cplusplus
}//extern "C"
#endif



#endif  //EPMEM_H
