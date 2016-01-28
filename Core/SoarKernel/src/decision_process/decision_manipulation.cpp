#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  decision_manipulation.cpp
 *
 * =======================================================================
 * Description  :  Predict/Select functionality
 * =======================================================================
 */

#include "decision_manipulation.h"

#include "agent.h"
#include "soar_rand.h"

#include "decide.h"
#include "misc.h"
#include "preference.h"

/***************************************************************************
 * Function     : select_init
 **************************************************************************/
void select_init(agent* thisAgent)
{
    thisAgent->select->select_enabled = false;
    thisAgent->select->select_operator.clear();
}

/***************************************************************************
 * Function     : select_next_operator
 **************************************************************************/
void select_next_operator(agent* thisAgent, const char* operator_id)
{
    select_init(thisAgent);
    std::string& op = thisAgent->select->select_operator;
    
    thisAgent->select->select_enabled = true;
    op.assign(operator_id);
    
    assert(!op.empty());
    
    // lazy users may use a lower-case letter
    std::string::iterator iter = op.begin();
    *iter = static_cast< char >(toupper(*iter));
}

/***************************************************************************
 * Function     : select_get_operator
 **************************************************************************/
const char* select_get_operator(agent* thisAgent)
{
    if (!thisAgent->select->select_enabled)
    {
        return NULL;
    }
    
    return thisAgent->select->select_operator.c_str();
}

/***************************************************************************
 * Function     : select_force
 **************************************************************************/
preference* select_force(agent* thisAgent, preference* candidates, bool reinit)
{
    preference* return_val = NULL;
    preference* cand = candidates;
    std::string temp;
    
    if (thisAgent->select->select_enabled)
    {
        // go through the list till we find a match or done
        while (cand && !return_val)
        {
            if (cand->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
            {
                // clear comparison string
                temp = "";
                
                // get first letter of comparison string
                temp += cand->value->id->name_letter;
                
                // get number of comparison string
                std::string temp2;
                to_string(cand->value->id->name_number, temp2);
                temp += temp2;
                
                if (!thisAgent->select->select_operator.compare(temp))
                {
                    return_val = cand;
                }
            }
            
            cand = cand->next;
        }
        
        if (return_val && reinit)
        {
            select_init(thisAgent);
        }
    }
    
    return return_val;
}

/***************************************************************************
 * Function     : predict_init
 **************************************************************************/
void predict_init(agent* thisAgent)
{
    thisAgent->predict_seed = 0;
    (*thisAgent->prediction) = "";
}

/***************************************************************************
 * Function     : predict_srand_store_snapshot
 **************************************************************************/
void predict_srand_store_snapshot(agent* thisAgent)
{
    uint32_t storage_val = 0;
    
    while (!storage_val)
    {
        storage_val = SoarRandInt();
    }
    
    thisAgent->predict_seed = storage_val;
}

/***************************************************************************
 * Function     : predict_srand_restore_snapshot
 **************************************************************************/
void predict_srand_restore_snapshot(agent* thisAgent, bool clear_snapshot)
{
    if (thisAgent->predict_seed)
    {
        SoarSeedRNG(thisAgent->predict_seed);
    }
    
    if (clear_snapshot)
    {
        predict_init(thisAgent);
    }
}

/***************************************************************************
 * Function     : predict_set
 **************************************************************************/
void predict_set(agent* thisAgent, const char* prediction)
{
    (*thisAgent->prediction) = prediction;
}

/***************************************************************************
 * Function     : predict_get
 **************************************************************************/
const char* predict_get(agent* thisAgent)
{
    predict_srand_store_snapshot(thisAgent);
    do_decision_phase(thisAgent, true);
    
    return thisAgent->prediction->c_str();
}
