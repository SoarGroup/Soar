typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * soar_agent;
/************************************************************************/
/*************************************************************************
 *
 *  file:  main.cpp
 *
 * =======================================================================
 *  This file is obsolete, having been replaced by the Tcl interface.
 *  Scott Wallace updated when testing Soar 8 without Tcl.
 * =======================================================================
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 * =======================================================================
 */
/* ===================================================================
                       Main file for Soar 6
   =================================================================== */

#include "kernel.h"
#include "init_soar.h"
#include "interface.h"

/* ===================================================================
   
                           Main Function

=================================================================== */

int main ()
{
#ifdef THINK_C
	/* Increase the application stack by 16K.
	 * This is done by decreasing the heap.
	 */
	SetApplLimit(GetApplLimit() - 16384);
	MaxApplZone();
#endif /* THINK_C */

  //init_soar();
  //repeatedly_read_and_dispatch_commands (soar_agent);
  return terminate_soar();
}

