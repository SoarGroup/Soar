/*
 * ebc_identity_sets.cpp
 *
 *  Created on: Feb 15, 2017
 *      Author: mazzin
 */

#include "ebc_identity_sets.h"
#include "dprint.h"

Identity_Sets::Identity_Sets(agent* myAgent)
{
    thisAgent = myAgent;
    id_set_counter = 0;

}

void Identity_Sets::reset()
{
    id_set_counter = 0;
    inst_id_to_id_set_map.clear();
    interm_id_to_id_set_map.clear();
    id_set_unifications.clear();
    operational_instantiations.clear();
    literalized_id_sets.clear();
}

uint64_t Identity_Sets::get_id_set(uint64_t pID)
{
    auto iter_sym = inst_id_to_id_set_map.find(pID);
    if (iter_sym != inst_id_to_id_set_map.end())
    {
        return iter_sym->second;
    }
    return NULL_IDENTITY_SET;
}

uint64_t Identity_Sets::get_or_create_id_set(uint64_t pID)
{
    auto iter_sym = inst_id_to_id_set_map.find(pID);
    if (iter_sym != inst_id_to_id_set_map.end())
    {
        return iter_sym->second;
    } else {
        increment_counter(id_set_counter);
//        dprint(DT_BACKTRACE1, "    ...created new identity set %u.\n", id_set_counter);
        inst_id_to_id_set_map[pID] = id_set_counter;
        return id_set_counter;
    }
}

void Identity_Sets::add_id_set_propagation(uint64_t pID, uint64_t pID_set)
{
    assert(inst_id_to_id_set_map.find(pID) == inst_id_to_id_set_map.end());
    inst_id_to_id_set_map[pID] = pID_set;
}

uint64_t Identity_Sets::get_intermediate_id_set(uint64_t pID)
{
    auto iter_sym = interm_id_to_id_set_map.find(pID);
    if (iter_sym != interm_id_to_id_set_map.end())
    {
        return iter_sym->second;
    } else {
        increment_counter(id_set_counter);
        interm_id_to_id_set_map[pID] = id_set_counter;
        return id_set_counter;
    }
}
