/*
 * smem_store.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_stats.h"
#include "smem_settings.h"

#include "agent.h"
#include "episodic_memory.h"
#include "lexer.h"
#include "mem.h"
#include "misc.h"
#include "output_manager.h"
#include "production.h"
#include "slot.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "xml.h"

void SMem_Manager::deallocate_ltm(ltm_object* pLTM, bool free_ltm )
{
    if (pLTM)
    {
        // proceed to slots
        if (pLTM->slots)
        {
            ltm_slot_map::iterator s;
            ltm_slot::iterator v;
            Symbol* lSym;
            // iterate over slots
            while (!pLTM->slots->empty())
            {
                s = pLTM->slots->begin();

                // proceed to slot contents
                if (s->second)
                {
                    // iterate over each value
                    for (v = s->second->begin(); v != s->second->end(); v = s->second->erase(v))
                    {
                        // de-allocation of value is dependent upon type
                        if ((*v)->val_const.val_type == value_const_t)
                        {
                            thisAgent->symbolManager->symbol_remove_ref(&(*v)->val_const.val_value);
                        }

                        delete(*v);
                    }

                    delete s->second;
                }

                // deallocate attribute for each corresponding value
                lSym = s->first;
                thisAgent->symbolManager->symbol_remove_ref(&lSym);

                pLTM->slots->erase(s);
            }

            // remove slots
            delete pLTM->slots;
            pLTM->slots = NULL;
        }

        // remove ltm itself
        if (free_ltm)
        {
            delete pLTM;
            pLTM = NULL;
            }
            }
}

/*
 * This is intended to allow the user to remove part or all of information stored on a LTI.
 * (All attributes, selected attributes, or just values from particular attributes.)
 */
ltm_slot* SMem_Manager::make_ltm_slot(ltm_slot_map* slots, Symbol* attr)
{
    ltm_slot** s = & (*slots)[ attr ];

    if (!(*s))
    {
        (*s) = new ltm_slot;
    }

    return (*s);
}

inline void SMem_Manager::count_child_connection(std::map<uint64_t,int64_t>* children, uint64_t child_lti_id)
{
    std::map<uint64_t, int64_t>::iterator child_location = children->find(child_lti_id);
    if (child_location != children->end())
    {
        (*children)[child_lti_id] = child_location->second + 1;
    }
    else
    {
        (*children)[child_lti_id] = 1;
    }
}

inline void SMem_Manager::count_child_connection(std::map<uint64_t,uint64_t>* children, uint64_t child_lti_id)
{
    std::map<uint64_t, uint64_t>::iterator child_location = children->find(child_lti_id);
    if (child_location != children->end())
    {
        (*children)[child_lti_id] = child_location->second + 1;
    }
    else
    {
        (*children)[child_lti_id] = 1;
    }
}

void SMem_Manager::disconnect_ltm(uint64_t pLTI_ID, std::map<uint64_t, uint64_t>* old_children = NULL)
{
    // adjust attr, attr/value counts
    {
        uint64_t pair_count = 0;

        uint64_t child_attr = 0;
        std::set<uint64_t> distinct_attr;

        // pairs first, accumulate distinct attributes and pair count
        SQL->web_all->bind_int(1, pLTI_ID);
        while (SQL->web_all->execute() == soar_module::row)
        {
            pair_count++;

            child_attr = SQL->web_all->column_int(0);
            distinct_attr.insert(child_attr);

            // null -> attr/lti
            if (SQL->web_all->column_int(1) != SMEM_AUGMENTATIONS_NULL)
            {
                // adjust in opposite direction ( adjust, attribute, const )
                SQL->wmes_constant_frequency_update->bind_int(1, -1);
                SQL->wmes_constant_frequency_update->bind_int(2, child_attr);
                SQL->wmes_constant_frequency_update->bind_int(3, SQL->web_all->column_int(1));
                SQL->wmes_constant_frequency_update->execute(soar_module::op_reinit);
            }
            else
            {
                if (old_children != NULL)
                {
                    count_child_connection(old_children, SQL->web_all->column_int(2));
                }
                // adjust in opposite direction ( adjust, attribute, lti )
                SQL->wmes_lti_frequency_update->bind_int(1, -1);
                SQL->wmes_lti_frequency_update->bind_int(2, child_attr);
                SQL->wmes_lti_frequency_update->bind_int(3, SQL->web_all->column_int(2));
                SQL->wmes_lti_frequency_update->execute(soar_module::op_reinit);
            }
        }
        SQL->web_all->reinitialize();

        // now attributes
        for (std::set<uint64_t>::iterator a = distinct_attr.begin(); a != distinct_attr.end(); a++)
        {
            // adjust in opposite direction ( adjust, attribute )
            SQL->attribute_frequency_update->bind_int(1, -1);
            SQL->attribute_frequency_update->bind_int(2, *a);
            SQL->attribute_frequency_update->execute(soar_module::op_reinit);
        }

        // update local statistic
        statistics->edges->set_value(statistics->edges->get_value() - pair_count);
    }

    // disconnect
    {
        SQL->web_truncate->bind_int(1, pLTI_ID);
        SQL->web_truncate->execute(soar_module::op_reinit);
    }
}

/* This function now requires that all LTI IDs are set up beforehand */

void SMem_Manager::LTM_to_DB(uint64_t pLTI_ID, ltm_slot_map* children, bool remove_old_children, bool activate, smem_storage_type store_type)
{
    assert(pLTI_ID);
    std::map<uint64_t, uint64_t>* old_children = NULL;
    std::map<uint64_t, int64_t>* new_children = NULL;
    if (settings->spreading->get_value() == on)
    {
        new_children = new std::map<uint64_t, int64_t>();
    }
    // if remove children, disconnect ltm -> no existing edges
    // else, need to query number of existing edges
    uint64_t existing_edges = 0;
    uint64_t existing_lti_edges = 0;
    if (remove_old_children)
    {
        if (settings->spreading->get_value() == on)
        {
            old_children = new std::map<uint64_t, uint64_t>();
        }
        disconnect_ltm(pLTI_ID);

        // provide trace output
        if (thisAgent->trace_settings[ TRACE_SMEM_SYSPARAM ])
        {
            char buf[256];

            thisAgent->outputManager->sprinta_sf_cstr(thisAgent, buf, 256, "<=SMEM: (@%u ^* *)\n", pLTI_ID);
            thisAgent->outputManager->printa(thisAgent, buf);
            xml_generate_warning(thisAgent, buf);
        }
    }
    else
    {
        SQL->act_lti_child_ct_get->bind_int(1, pLTI_ID);
        SQL->act_lti_child_ct_get->execute();

        existing_edges = static_cast<uint64_t>(SQL->act_lti_child_ct_get->column_int(0));

        SQL->act_lti_child_ct_get->reinitialize();

        //

        SQL->act_lti_child_lti_ct_get->bind_int(1,pLTI_ID);
        SQL->act_lti_child_lti_ct_get->execute();

        existing_lti_edges = static_cast<uint64_t>(SQL->act_lti_child_lti_ct_get->column_int(0));

        SQL->act_lti_child_lti_ct_get->reinitialize();
    }

    // get new edges
    // if didn't disconnect, entails lookups in existing edges
    std::set<smem_hash_id> attr_new;
    std::set< std::pair<smem_hash_id, smem_hash_id> > const_new;
    std::set< std::pair<smem_hash_id, uint64_t> > lti_new;
    bool ever_updated_edge_weight = false;
    bool added_edges = false;
    std::unordered_map<uint64_t, double> edge_weights;
    {
        ltm_slot_map::iterator s;
        ltm_slot::iterator v;

        smem_hash_id attr_hash = 0;
        smem_hash_id value_hash = 0;
        uint64_t value_lti = 0;

        for (s = children->begin(); s != children->end(); s++)
        {
            attr_hash = hash(s->first);
            if (remove_old_children)
            {
                attr_new.insert(attr_hash);
            }
            else
            {
                // lti_id, attribute_s_id
                assert(attr_hash);
                SQL->web_attr_child->bind_int(1, pLTI_ID);
                SQL->web_attr_child->bind_int(2, attr_hash);
                if (SQL->web_attr_child->execute(soar_module::op_reinit) != soar_module::row)
                {
                    attr_new.insert(attr_hash);
                }
            }

            for (v = s->second->begin(); v != s->second->end(); v++)
            {
                if ((*v)->val_const.val_type == value_const_t)
                {
                    value_hash = hash((*v)->val_const.val_value);

                    if (remove_old_children)
                    {
                        const_new.insert(std::make_pair(attr_hash, value_hash));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_const
                        assert(pLTI_ID && attr_hash && value_hash);
                        SQL->web_const_child->bind_int(1, pLTI_ID);
                        SQL->web_const_child->bind_int(2, attr_hash);
                        SQL->web_const_child->bind_int(3, value_hash);
                        if (SQL->web_const_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            const_new.insert(std::make_pair(attr_hash, value_hash));
                        }
                    }

                    // provide trace output
                    if (thisAgent->trace_settings[ TRACE_SMEM_SYSPARAM ])
                    {
                        char buf[256];

                        thisAgent->outputManager->sprinta_sf_cstr(thisAgent, buf, 256, "=>SMEM: (@%u ^%y %y)\n", pLTI_ID, s->first, (*v)->val_const.val_value);

                        thisAgent->outputManager->printa(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
                else
                {
                    added_edges = true;
                    value_lti = (*v)->val_lti.val_value->lti_id;
                    assert(value_lti);
                    double edge_weight = (*v)->val_lti.edge_weight;

                    if (remove_old_children)
                    {
                        lti_new.insert(std::make_pair(attr_hash, value_lti));
                        if (new_children != NULL)
                        {
                            count_child_connection(new_children, value_lti);
                        }
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_lti
                        assert(pLTI_ID && attr_hash && value_lti);
                        SQL->web_lti_child->bind_int(1, pLTI_ID);
                        SQL->web_lti_child->bind_int(2, attr_hash);
                        SQL->web_lti_child->bind_int(3, value_lti);
                        if (SQL->web_lti_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            lti_new.insert(std::make_pair(attr_hash, value_lti));
                            if (new_children != NULL)
                            {
                                count_child_connection(new_children, value_lti);
                            }
                        }
                    }

                    //We have an edge_weight and a parent and a child. This is where we set the edge_weight to the nonfan value.
                    if (edge_weight != 0.0)
                    {
                        /*SQL->web_update_child_edge->bind_double(1,edge_weight);
                        SQL->web_update_child_edge->bind_int(2,pLTI_ID);
                        SQL->web_update_child_edge->bind_int(3,value_lti);
                        SQL->web_update_child_edge->execute(soar_module::op_reinit);*/
                        edge_weights[value_lti] = edge_weight;
                        ever_updated_edge_weight = true;
                    }
                    // provide trace output
                    if (thisAgent->trace_settings[ TRACE_SMEM_SYSPARAM ])
                    {
                        char buf[256];

                        thisAgent->outputManager->sprinta_sf_cstr(thisAgent, buf, 256, "=>SMEM: (%u ^%y %u)\n", pLTI_ID, s->first, (*v)->val_lti.val_value->lti_id);
                        thisAgent->outputManager->printa(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
            }
        }
    }

    /*
     * Here, the delta between what the children of the lti used to be and
     * what they are now is calculated and used to determine what spreading
     * likelihoods need to be recalculated (since the network structure that
     * generated them is no longer valid).
     * */
    if (new_children != NULL)
    {
        if (remove_old_children)
        {//This is when the delta needs to be calculated.
            /*
             * Delta: Loop over the new children.
             * Check if they are also old children.
             * If so, calculate the delta and store that into ne children as the new value.
             * At the same time, erase the old children if it showed up (after calculating the delta)
             * then, loop through the remaining old children and just add those values as negative.
             */
            assert(old_children != NULL);

            std::map<uint64_t, int64_t>::iterator new_child;
            for (new_child = new_children->begin(); new_child != new_children->end(); ++new_child)
            {
                if (old_children->find(new_child->first) != old_children->end())
                {
                    (*new_children)[new_child->first] = (*new_children)[new_child->first] - (*old_children)[new_child->first];
                    old_children->erase(new_child->first);
                }
            }
            std::map<uint64_t, uint64_t>::iterator old_child;
            for (old_child = old_children->begin(); old_child != old_children->end(); ++old_child)
            {
                (*new_children)[old_child->first] = old_child->second;
            }
        }
        // new_children contains the set of changes to memory. We use those to invalidate spreading trajectories.
        invalidate_trajectories(pLTI_ID, new_children);
    }

    // activation function assumes proper thresholding state
    // thus, consider four cases of augmentation counts (w.r.t. thresh)
    // 1. before=below, after=below: good (activation will update smem_augmentations)
    // 2. before=below, after=above: need to update smem_augmentations->inf
    // 3. before=after, after=below: good (activation will update smem_augmentations, free transition)
    // 4. before=after, after=after: good (activation won't touch smem_augmentations)
    //
    // hence, we detect + handle case #2 here
    uint64_t new_edges = (existing_edges + const_new.size() + lti_new.size());
    uint64_t new_lti_edges = existing_lti_edges + lti_new.size();
    bool after_above;
    double web_act = static_cast<double>(SMEM_ACT_LOW);
    {
        uint64_t thresh = static_cast<uint64_t>(settings->thresh->get_value());
        after_above = (new_edges >= thresh);

        // if before below
        if (existing_edges < thresh)
        {
            if (after_above)
            {
                // update smem_augmentations to inf
                SQL->act_set->bind_double(1, web_act);
                SQL->act_set->bind_int(2, pLTI_ID);
                SQL->act_set->execute(soar_module::op_reinit);
            }
        }
    }

    // update edge counter
    {
        SQL->act_lti_child_ct_set->bind_int(1, new_edges);
        SQL->act_lti_child_ct_set->bind_int(2, pLTI_ID);
        SQL->act_lti_child_ct_set->execute(soar_module::op_reinit);
    }
    {
        SQL->act_lti_child_lti_ct_set->bind_int(1, new_lti_edges);
        SQL->act_lti_child_lti_ct_set->bind_int(2, pLTI_ID);
        SQL->act_lti_child_lti_ct_set->execute(soar_module::op_reinit);
    }



    //Put the initialization of the entry in the prohibit table here.
    //This doesn't create a prohibt. It creates an entry in the prohibit tracking table.
    {
        SQL->prohibit_add->bind_int(1,pLTI_ID);
        SQL->prohibit_add->execute(soar_module::op_reinit);
    }

    // now we can safely activate the lti
    if (activate)
    {
        bool activate_on_add = (settings->activate_on_add->get_value() == on);
        double lti_act = lti_activate(pLTI_ID, activate_on_add, new_edges);

        if (!after_above)
        {
            web_act = lti_act;
        }
    }

    // insert new edges, update counters
    {
        // attr/const pairs
        {
            for (std::set< std::pair< smem_hash_id, smem_hash_id > >::iterator p = const_new.begin(); p != const_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    SQL->web_add->bind_int(1, pLTI_ID);
                    SQL->web_add->bind_int(2, p->first);
                    SQL->web_add->bind_int(3, p->second);
                    SQL->web_add->bind_int(4, SMEM_AUGMENTATIONS_NULL);
                    SQL->web_add->bind_double(5, web_act);
                    SQL->web_add->bind_double(6, 0.0);
                    SQL->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    SQL->wmes_constant_frequency_check->bind_int(1, p->first);
                    SQL->wmes_constant_frequency_check->bind_int(2, p->second);
                    if (SQL->wmes_constant_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        SQL->wmes_constant_frequency_add->bind_int(1, p->first);
                        SQL->wmes_constant_frequency_add->bind_int(2, p->second);
                        SQL->wmes_constant_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, val)
                        SQL->wmes_constant_frequency_update->bind_int(1, 1);
                        SQL->wmes_constant_frequency_update->bind_int(2, p->first);
                        SQL->wmes_constant_frequency_update->bind_int(3, p->second);
                        SQL->wmes_constant_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // attr/lti pairs
        {
            for (std::set< std::pair< smem_hash_id, uint64_t > >::iterator p = lti_new.begin(); p != lti_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    SQL->web_add->bind_int(1, pLTI_ID);
                    SQL->web_add->bind_int(2, p->first);
                    SQL->web_add->bind_int(3, SMEM_AUGMENTATIONS_NULL);
                    SQL->web_add->bind_int(4, p->second);
                    SQL->web_add->bind_double(5, web_act);
                    if (ever_updated_edge_weight && edge_weights.find(p->second) != edge_weights.end())
                    {
                        /*
                         * This will currently silently fail if the user doesn't supply all of the edge weights
                         *  that they should. Some will be initialized to fan and others will not.
                         *  The first round of normalization will "fix" this, in that things won't "break",
                         *  but the values will be different than what the user presumably intended.
                         */
                        SQL->web_add->bind_double(6, edge_weights[p->second]);
                    }
                    else
                    {
                        SQL->web_add->bind_double(6, 1.0/((double)new_lti_edges));
                    }
                    SQL->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    SQL->wmes_lti_frequency_check->bind_int(1, p->first);
                    SQL->wmes_lti_frequency_check->bind_int(2, p->second);
                    if (SQL->wmes_lti_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        SQL->wmes_lti_frequency_add->bind_int(1, p->first);
                        SQL->wmes_lti_frequency_add->bind_int(2, p->second);
                        SQL->wmes_lti_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, lti)
                        SQL->wmes_lti_frequency_update->bind_int(1, 1);
                        SQL->wmes_lti_frequency_update->bind_int(2, p->first);
                        SQL->wmes_lti_frequency_update->bind_int(3, p->second);
                        SQL->wmes_lti_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // update attribute count
        {
            for (std::set< smem_hash_id >::iterator a = attr_new.begin(); a != attr_new.end(); a++)
            {
                // check if counter exists (and add if does not): attribute_s_id
                SQL->attribute_frequency_check->bind_int(1, *a);
                if (SQL->attribute_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                {
                    SQL->attribute_frequency_add->bind_int(1, *a);
                    SQL->attribute_frequency_add->execute(soar_module::op_reinit);
                }
                else
                {
                    // adjust count (adjustment, attribute_s_id)
                    SQL->attribute_frequency_update->bind_int(1, 1);
                    SQL->attribute_frequency_update->bind_int(2, *a);
                    SQL->attribute_frequency_update->execute(soar_module::op_reinit);
                }
            }
        }

        // update local edge count
        {
            statistics->edges->set_value(statistics->edges->get_value() + (const_new.size() + lti_new.size()));
        }
    }
    //For now, on a change to the network for an lti, I reset the edge weights.
    if (added_edges && !ever_updated_edge_weight)
    {
        double fan = 1.0/((double)new_lti_edges);
        SQL->web_update_all_lti_child_edges->bind_double(1,fan);
        SQL->web_update_all_lti_child_edges->bind_int(2,pLTI_ID);
        SQL->web_update_all_lti_child_edges->execute(soar_module::op_reinit);
    }
    if (old_children != NULL)
    {
        delete old_children;
    }
    if (new_children != NULL)
    {
        delete new_children;
    }
}

void SMem_Manager::store_new(Symbol* pSTI, smem_storage_type store_type, bool pOverwriteOldLinkToLTM, tc_number tc)
{
    /* We only need to use lookup (3rd arg), if we're storing new and can't overwrite the old lti_value */
    STM_to_LTM(pSTI, store_type, !pOverwriteOldLinkToLTM, pOverwriteOldLinkToLTM, tc);
}

void SMem_Manager::update(Symbol* pSTI, smem_storage_type store_type, tc_number tc)
{
    STM_to_LTM(pSTI, store_type, false, false, tc);
}

void SMem_Manager::STM_to_LTM(Symbol* pSTI, smem_storage_type store_type, bool use_lookup, bool pOverwriteOldLinkToLTM, tc_number tc)
{
    // transitive closure only matters for recursive storage
    if ((store_type == store_recursive) && (tc == NIL))
    {
        tc = get_new_tc_number(thisAgent);
    }
    symbol_list shorties;

    wme_list* children = get_direct_augs_of_id(pSTI, tc);
    wme_list::iterator w;

    uint64_t l_LTM_ID = get_current_LTI_for_iSTI(pSTI, use_lookup, pOverwriteOldLinkToLTM);

    // encode this level
    {
        sym_to_ltm_map sym_to_ltm;
        sym_to_ltm_map::iterator c_p;
        ltm_object** c;

        ltm_slot_map slots;
        ltm_slot_map::iterator s_p;
        ltm_slot::iterator v_p;
        ltm_slot* s;
        ltm_value* v;

        for (w = children->begin(); w != children->end(); w++)
        {
            // get slot
            s = make_ltm_slot(&(slots), (*w)->attr);

            // create value, per type
            v = new ltm_value;
            if ((*w)->value->is_constant())
            {
                v->val_const.val_type = value_const_t;
                v->val_const.val_value = (*w)->value;
            }
            else
            {
                v->val_lti.val_type = value_lti_t;

                /* This seems like funky map usage.  Following line will create entry in map if it doesn't
                 * exist.  This works because following code will use created entry anyway.  Should use
                 * iterator and find, then create if necessary. */
                c = & sym_to_ltm[(*w)->value ];

                // if doesn't exist, add; else use existing
                if (!(*c))
                {
                    (*c) = new ltm_object;
                    (*c)->slots = NULL;
                    (*c)->lti_id = get_current_LTI_for_iSTI((*w)->value, use_lookup, pOverwriteOldLinkToLTM);
                    if (store_type == store_recursive)
                    {
                        shorties.push_back((*w)->value);
                    }
                }

                v->val_lti.val_value = (*c);
                v->val_lti.edge_weight = 0.0;
            }

            // add value to slot
            s->push_back(v);
        }

        LTM_to_DB(l_LTM_ID, &(slots), true, true);

        // clean up
        {
            // de-allocate slots
            for (s_p = slots.begin(); s_p != slots.end(); s_p++)
            {
                for (v_p = s_p->second->begin(); v_p != s_p->second->end(); v_p++)
                {
                    delete(*v_p);
                }

                delete s_p->second;
            }

            // de-allocate ltms
            for (c_p = sym_to_ltm.begin(); c_p != sym_to_ltm.end(); c_p++)
            {
                delete c_p->second;
            }

            delete children;
        }
    }

    // recurse as necessary
    for (symbol_list::iterator shorty = shorties.begin(); shorty != shorties.end(); shorty++)
    {
        STM_to_LTM((*shorty), store_recursive, use_lookup, pOverwriteOldLinkToLTM, tc);
    }
}
