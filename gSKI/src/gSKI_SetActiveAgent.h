/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_SetActiveAgent.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_SETACTIVEAGENT_H
#define GSKI_SETACTIVEAGENT_H

//typedef struct agent_struct agent;
//extern agent* soar_agent;

namespace gSKI {

#define SetActiveAgent(newAgent) soar_agent = newAgent

#define NI() MegaAssert(false, "NOT IMPLEMENTED YET!");

}
#endif
