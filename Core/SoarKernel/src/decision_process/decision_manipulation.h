/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "kernel.h"

#include <string>

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
void select_init(agent* thisAgent);

// make selection, does not validate operator
void select_next_operator(agent* thisAgent, const char* operator_id);

// get current select, NULL on none
const char* select_get_operator(agent* thisAgent);

// force selection, NULL on invalid selection choice
preference* select_force(agent* thisAgent, preference* candidates, bool reinit = true);

//////////////////////////////////////////////////////////
// predict functions
//////////////////////////////////////////////////////////

// initialization of predict per agent
void predict_init(agent* thisAgent);

// establishes and stores a known srand state
void predict_srand_store_snapshot(agent* thisAgent);

// restores a previously stored srand state, optionally clearing the old state
void predict_srand_restore_snapshot(agent* thisAgent, bool clear_snapshot = true);

// sets the prediction
void predict_set(agent* thisAgent, const char* prediction);

// gets a new prediction
const char* predict_get(agent* thisAgent);

#endif

