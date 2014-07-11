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

#include <string>

typedef struct agent_struct agent;
typedef struct preference_struct preference;

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
extern void select_init(agent* thisAgent);

// make selection, does not validate operator
extern void select_next_operator(agent* thisAgent, const char* operator_id);

// get current select, NULL on none
extern const char* select_get_operator(agent* thisAgent);

// force selection, NULL on invalid selection choice
extern preference* select_force(agent* thisAgent, preference* candidates, bool reinit = true);

//////////////////////////////////////////////////////////
// predict functions
//////////////////////////////////////////////////////////

// initialization of predict per agent
extern void predict_init(agent* thisAgent);

// establishes and stores a known srand state
extern void predict_srand_store_snapshot(agent* thisAgent);

// restores a previously stored srand state, optionally clearing the old state
extern void predict_srand_restore_snapshot(agent* thisAgent, bool clear_snapshot = true);

// sets the prediction
extern void predict_set(agent* thisAgent, const char* prediction);

// gets a new prediction
extern const char* predict_get(agent* thisAgent);

#endif

