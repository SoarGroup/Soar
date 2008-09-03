/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  decision_manipulation.h
 *
 * =======================================================================
 */

#ifndef DECISION_MANIPULATION_H
#define DECISION_MANIPULATION_H

#include <string>

#include "agent.h"
#include "gdatastructs.h"

//////////////////////////////////////////////////////////
// select types
//////////////////////////////////////////////////////////

typedef struct select_info_struct
{
	bool select_enabled;
	std::string select_operator;
} select_info;

//////////////////////////////////////////////////////////
// select functions
//////////////////////////////////////////////////////////

// initialization of select per agent
extern void init_select( agent *my_agent );

// make selection, does not validate operator
extern void select_next_operator( agent *my_agent, const char *operator_id );

// get current select, NULL on none
extern const char *get_selected_operator( agent *my_agent );

// force selection, NULL on invalid selection choice
extern preference *force_selection( agent *my_agent, preference *candidates, bool reinit = true );

//////////////////////////////////////////////////////////
// predict functions
//////////////////////////////////////////////////////////

// initialization of predict per agent
extern void init_predict( agent *my_agent );

// establishes and stores a known srand state
extern void srand_store_snapshot( agent *my_agent );

// restores a previously stored srand state, optionally clearing the old state
extern void srand_restore_snapshot( agent *my_agent, bool clear_snapshot = true );

// sets the prediction
extern void set_prediction( agent *my_agent, const char *prediction);

// gets a new prediction
extern const char *get_prediction( agent *my_agent );

#endif

