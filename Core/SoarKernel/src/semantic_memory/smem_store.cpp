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
#include "VariadicBind.h"

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

void SMem_Manager::disconnect_ltm(uint64_t pLTI_ID)
{
    // adjust attr, attr/value counts
    // pairs first, accumulate distinct attributes and pair count
    uint64_t pair_count = 0;

    JobQueue->post([&]() mutable {
        uint64_t child_attr = 0;
        std::set<uint64_t> distinct_attr;

        auto sql = sqlite_thread_guard(SQL->web_all);
        SQLite::bind(*sql, pLTI_ID);
        while (sql->executeStep())
        {
            pair_count++;

            child_attr = sql->getColumn(0).getInt();
            distinct_attr.insert(child_attr);

            // null -> attr/lti
            if (sql->getColumn(1).getInt() != SMEM_AUGMENTATIONS_NULL)
            {
                // adjust in opposite direction ( adjust, attribute, const )
                auto wlfu = sqlite_thread_guard(SQL->wmes_lti_frequency_update);
                SQLite::bind(*wlfu, -1, child_attr, sql->getColumn(1).getInt());
                wlfu->exec();
            }
            else
            {
                // adjust in opposite direction ( adjust, attribute, lti )
                auto wlfu = sqlite_thread_guard(SQL->wmes_lti_frequency_update);
                SQLite::bind(*wlfu, -1, child_attr, sql->getColumn(2).getInt());
                wlfu->exec();
            }
        }

        // now attributes
        for (std::set<uint64_t>::iterator a = distinct_attr.begin(); a != distinct_attr.end(); a++)
        {
            // adjust in opposite direction ( adjust, attribute )
            auto afu = sqlite_thread_guard(SQL->attribute_frequency_update);
            SQLite::bind(*afu, -1, *a);
            afu->exec();
        }
    })->wait();


    // update local statistic
    statistics->edges->set_value(statistics->edges->get_value() - pair_count);

    // disconnect
    JobQueue->post([=]() {
        auto wt = sqlite_thread_guard(SQL->web_truncate);
        SQLite::bind(*wt, pLTI_ID);
        wt->exec();
    })->wait();
}

/* This function now requires that all LTI IDs are set up beforehand */

void SMem_Manager::LTM_to_DB(uint64_t pLTI_ID, ltm_slot_map* children, bool remove_old_children, bool activate)
{
    assert(pLTI_ID);
    // if remove children, disconnect ltm -> no existing edges
    // else, need to query number of existing edges
    uint64_t existing_edges = 0;
    if (remove_old_children)
    {
        disconnect_ltm(pLTI_ID);

        // provide trace output
        if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ])
        {
            char buf[256];

            thisAgent->outputManager->sprinta_sf_cstr(thisAgent, buf, 256, "<=SMEM: (@%u ^* *)\n", pLTI_ID);
            thisAgent->outputManager->printa(thisAgent, buf);
            xml_generate_warning(thisAgent, buf);
        }
    }
    else
    {
        JobQueue->post([=,&existing_edges]() mutable {
            auto sql = sqlite_thread_guard(SQL->act_lti_child_ct_get);

            SQLite::bind(*sql, pLTI_ID);

            if (!sql->executeStep())
                throw SoarAssertionException("Failed to retrieve column", __FILE__, __LINE__);

            existing_edges = static_cast<uint64_t>(sql->getColumn(0).getUInt64());
        })->wait();
    }

    // get new edges
    // if didn't disconnect, entails lookups in existing edges
    std::set<smem_hash_id> attr_new;
    std::set< std::pair<smem_hash_id, smem_hash_id> > const_new;
    std::set< std::pair<smem_hash_id, uint64_t> > lti_new;
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
                JobQueue->post([&]() mutable {
                    auto sql = sqlite_thread_guard(SQL->web_attr_child);
                    SQLite::bind(*sql, pLTI_ID, attr_hash);

                    if (!sql->executeStep())
                        attr_new.insert(attr_hash);
                })->wait();
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

                        JobQueue->post([&]() mutable {
                            auto sql = sqlite_thread_guard(SQL->web_const_child);
                            SQLite::bind(*sql, pLTI_ID, attr_hash, value_hash);

                            if (!sql->executeStep())
                                const_new.insert(std::make_pair(attr_hash, value_hash));
                        })->wait();
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ])
                    {
                        char buf[256];

                        thisAgent->outputManager->sprinta_sf_cstr(thisAgent, buf, 256, "=>SMEM: (@%u ^%y %y)\n", pLTI_ID, s->first, (*v)->val_const.val_value);

                        thisAgent->outputManager->printa(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
                else
                {
                    value_lti = (*v)->val_lti.val_value->lti_id;
                    assert(value_lti);

                    if (remove_old_children)
                    {
                        lti_new.insert(std::make_pair(attr_hash, value_lti));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_lti
                        assert(pLTI_ID && attr_hash && value_lti);
                        JobQueue->post([&]() mutable {
                            auto sql = sqlite_thread_guard(SQL->web_lti_child);
                            SQLite::bind(*sql, pLTI_ID, attr_hash, value_lti);

                            if (!sql->executeStep())
                                lti_new.insert(std::make_pair(attr_hash, value_lti));
                        })->wait();
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ])
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

    // activation function assumes proper thresholding state
    // thus, consider four cases of augmentation counts (w.r.t. thresh)
    // 1. before=below, after=below: good (activation will update smem_augmentations)
    // 2. before=below, after=above: need to update smem_augmentations->inf
    // 3. before=after, after=below: good (activation will update smem_augmentations, free transition)
    // 4. before=after, after=after: good (activation won't touch smem_augmentations)
    //
    // hence, we detect + handle case #2 here
    uint64_t new_edges = (existing_edges + const_new.size() + lti_new.size());
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
                JobQueue->post([&]() {
                    auto sql = sqlite_thread_guard(SQL->act_set);
                    SQLite::bind(*sql, web_act, pLTI_ID);
                    sql->exec();
                })->wait();
            }
        }
    }

    // update edge counter
    JobQueue->post([&]() {
        auto sql = sqlite_thread_guard(SQL->act_lti_child_ct_set);
        SQLite::bind(*sql, new_edges, pLTI_ID);
        sql->exec();
    })->wait();

    // now we can safely activate the lti
    if (activate)
    {
        double lti_act = lti_activate(pLTI_ID, true, new_edges);

        if (!after_above)
        {
            web_act = lti_act;
        }
    }

    // insert new edges, update counters
    // attr/const pairs
    for (std::set< std::pair< smem_hash_id, smem_hash_id > >::iterator p = const_new.begin(); p != const_new.end(); p++)
    {
        // insert
        JobQueue->post([=]() {
            // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
            auto sql = sqlite_thread_guard(SQL->web_add);
            SQLite::bind(*sql, pLTI_ID, p->first, p->second, SMEM_AUGMENTATIONS_NULL, web_act);
            sql->exec();
        })->wait();

        // update counter
        // check if counter exists (and add if does not): attribute_s_id, val
        JobQueue->post([=]() {
            auto sql = sqlite_thread_guard(SQL->wmes_constant_frequency_check);
            SQLite::bind(*sql, p->first, p->second);

            if (!sql->executeStep())
            {
                auto sql = sqlite_thread_guard(SQL->wmes_constant_frequency_add);
                SQLite::bind(*sql, p->first, p->second);
                sql->exec();
            }
            else
            {
                // adjust count (adjustment, attribute_s_id, val)
                auto sql = sqlite_thread_guard(SQL->wmes_constant_frequency_update);
                SQLite::bind(*sql, 1, p->first, p->second);
                sql->exec();
            }
        })->wait();
    }

    // attr/lti pairs
    for (std::set< std::pair< smem_hash_id, uint64_t > >::iterator p = lti_new.begin(); p != lti_new.end(); p++)
    {
        // insert
        JobQueue->post([=]() {
            // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
            auto sql = sqlite_thread_guard(SQL->web_add);
            SQLite::bind(*sql, pLTI_ID, p->first, SMEM_AUGMENTATIONS_NULL, p->second, web_act);
            sql->exec();
        })->wait();

        // update counter
        JobQueue->post([=]() {
            // check if counter exists (and add if does not): attribute_s_id, val
            auto sql = sqlite_thread_guard(SQL->wmes_lti_frequency_check);
            SQLite::bind(*sql, p->first, p->second);
            if (!sql->executeStep())
            {
                auto sql = sqlite_thread_guard(SQL->wmes_lti_frequency_add);
                SQLite::bind(*sql, p->first, p->second);
                sql->exec();
            }
            else
            {
                // adjust count (adjustment, attribute_s_id, lti)
                auto sql = sqlite_thread_guard(SQL->wmes_lti_frequency_update);
                SQLite::bind(*sql, 1, p->first, p->second);
                sql->exec();
            }
        })->wait();
    }

    // update attribute count
    for (std::set< smem_hash_id >::iterator a = attr_new.begin(); a != attr_new.end(); a++)
    {
        // check if counter exists (and add if does not): attribute_s_id
        JobQueue->post([=]() {
            auto sql = sqlite_thread_guard(SQL->attribute_frequency_check);
            SQLite::bind(*sql, *a);
            if (!sql->executeStep())
            {
                auto sql = sqlite_thread_guard(SQL->attribute_frequency_add);
                SQLite::bind(*sql, *a);
                sql->exec();
            }
            else
            {
                // adjust count (adjustment, attribute_s_id)
                auto sql = sqlite_thread_guard(SQL->attribute_frequency_update);
                SQLite::bind(*sql, 1, *a);
                sql->exec();
            }
        })->wait();
    }

    // update local edge count
    statistics->edges->set_value(statistics->edges->get_value() + (const_new.size() + lti_new.size()));
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

    dprint(DT_SMEM_INSTANCE, "STM_to_LTM adding %y (%u), %s and %soverwriting old LTI links if they exist\n", pSTI, pSTI->id->LTI_ID, use_lookup ? "using look-up table" : "using existing LTI IDs", pOverwriteOldLinkToLTM ? " " : "not ");
    // transitive closure only matters for recursive storage
    if ((store_type == store_recursive) && (tc == NIL))
    {
        tc = get_new_tc_number(thisAgent);
    }
    symbol_list shorties;

    wme_list* children = get_direct_augs_of_id(pSTI, tc);
    wme_list::iterator w;

    uint64_t l_val_ID, l_LTM_ID = get_current_LTI_for_iSTI(pSTI, use_lookup, pOverwriteOldLinkToLTM);

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
