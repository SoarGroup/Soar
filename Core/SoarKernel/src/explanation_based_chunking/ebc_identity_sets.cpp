/*
 * ebc_identity_sets.cpp
 *
 *  Created on: Feb 15, 2017
 *      Author: mazzin
 */

#include "ebc_identity_sets.h"


Identity_Sets::Identity_Sets(agent* myAgent)
{
    thisAgent = myAgent;
    id_set_counter = 0;

}

void Identity_Sets::reset()
{
    id_set_counter = 0;
    base_id_to_id_set_map.clear();
    interm_id_to_id_set_map.clear();
    id_set_unifications.clear();
    operational_instantiations.clear();
    literalized_id_sets.clear();
}

uint64_t Identity_Sets::get_base_id_set(uint64_t pID)
{
    auto iter_sym = base_id_to_id_set_map.find(pID);
    if (iter_sym != base_id_to_id_set_map.end())
    {
        return iter_sym->second;
    } else {
        increment_counter(id_set_counter);
        base_id_to_id_set_map[pID] = id_set_counter;
        return id_set_counter;
    }
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
