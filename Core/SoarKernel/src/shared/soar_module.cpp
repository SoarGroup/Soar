/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.cpp
 *
 * =======================================================================
 * Description  :  Useful functions for Soar modules
 * =======================================================================
 */

#include "soar_module.h"

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "ebc.h"
#include "instantiation.h"
#include "slot.h"
#include "mem.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "soar_TraceNames.h"
#include "test.h"
#include "xml.h"
#include "working_memory_activation.h"
#include "working_memory.h"
#include "dprint.h"

namespace soar_module
{
    timer::timer(const char* new_name, agent* new_agent, timer_level new_level, predicate<timer_level>* new_pred, bool soar_control): named_object(new_name), thisAgent(new_agent), level(new_level), pred(new_pred)
    {
        stopwatch.set_enabled(soar_control ? &(new_agent->timers_enabled) : (NULL));
        reset();
    }

    /////////////////////////////////////////////////////////////
    // Utility functions
    /////////////////////////////////////////////////////////////

    wme* add_module_wme(agent* thisAgent, Symbol* id, Symbol* attr, Symbol* value, bool isSingleton)
    {
        slot* my_slot = make_slot(thisAgent, id, attr);
        wme* w = make_wme(thisAgent, id, attr, value, false);

        insert_at_head_of_dll(my_slot->wmes, w, next, prev);
        add_wme_to_wm(thisAgent, w);
        if (isSingleton)
        {
            thisAgent->explanationBasedChunker->add_to_singletons(w);
        }
//        /*MToDo | Adding a refcount here.  Not sure if needed, but seems like it should and currently has issues with certain links being deallocated */
        wme_add_ref(w, true);
        return w;
    }

    void remove_module_wme(agent* thisAgent, wme* w)
    {
        slot* my_slot = find_slot(w->id, w->attr);

        if (my_slot)
        {
            remove_from_dll(my_slot->wmes, w, next, prev);

            if (w->gds)
            {
                if (w->gds->goal != NIL)
                {
                    gds_invalid_so_remove_goal(thisAgent, w);

                    /* NOTE: the call to remove_wme_from_wm will take care of checking if GDS should be removed */
                }
            }

            remove_wme_from_wm(thisAgent, w);
        }
        wme_remove_ref(thisAgent, w, true);
    }

    void print_ambiguous_commands(agent* thisAgent, const std::string badCommand, const std::list<std::string> matched_name_list)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%?  Did you mean", badCommand.c_str());
        thisAgent->outputManager->display_ambiguous_command_error(thisAgent, matched_name_list);
    }

}

