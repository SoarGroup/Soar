#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.cpp */

#include "stl_support.h"
#include "utilities.h"
#include "gdatastructs.h"
#include "wmem.h"

SoarSTLWMEPoolList* get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc)
{
	// notice how we give the constructor our custom SoarSTLWMEPoolList with the agent and memory type to use
	SoarSTLWMEPoolList* list = new SoarSTLWMEPoolList(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));

	slot *s;
    wme *w;

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return NULL;
    if (id->id.tc_num == tc)
        return NULL;
    id->id.tc_num = tc;

	// build list of wmes
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            list->push_back(w);
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            list->push_back(w);
    }

    return list;
}
