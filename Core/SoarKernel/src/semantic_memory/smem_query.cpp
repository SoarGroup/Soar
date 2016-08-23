/*
 *  * smem_query.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_math_query.h"
#include "smem_timers.h"
#include "smem_settings.h"

#include "symbol.h"
#include "working_memory.h"

soar_module::sqlite_statement* SMem_Manager::setup_web_crawl(smem_weighted_cue_element* el)
{
    soar_module::sqlite_statement* q = NULL;

    // first, point to correct query and setup
    // query-specific parameters
    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        q = smem_stmts->web_attr_all;
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        q = smem_stmts->web_const_all;
        q->bind_int(2, el->value_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        q = smem_stmts->web_lti_all;
        q->bind_int(2, el->value_lti);
    }

    // all require hash as first parameter
    q->bind_int(1, el->attr_hash);

    return q;
}

bool SMem_Manager::process_cue_wme(wme* w, bool pos_cue, smem_prioritized_weighted_cue& weighted_pq, MathQuery* mathQuery)
{
    bool good_wme = true;
    smem_weighted_cue_element* new_cue_element;

    smem_hash_id attr_hash;
    smem_hash_id value_hash;
    smem_lti_id value_lti;
    smem_cue_element_type element_type;

    soar_module::sqlite_statement* q = NULL;

    {
        // we only have to do hard work if
        attr_hash = hash(w->attr, false);
        if (attr_hash != NIL)
        {
            if (w->value->is_constant() && mathQuery == NIL)
            {
                value_lti = NIL;
                value_hash = hash(w->value, false);
                element_type = value_const_t;

                if (value_hash != NIL)
                {
                    q = smem_stmts->wmes_constant_frequency_get;
                    q->bind_int(1, attr_hash);
                    q->bind_int(2, value_hash);
                }
                else if (pos_cue)
                {
                    good_wme = false;
                }
                else
                {
                    //This would be a negative query that smem has no hash for.  This means that
                    //there is no way it could be in any of the results, and we don't
                    //need to continue processing it, let alone use it in the search.  --ACN
                    return true;
                }
            }
            else
            {
                //If we get here on a math query, the value may not be an identifier
                if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                {
                    value_lti = w->value->id->smem_lti;
                }
                else
                {
                    value_lti = 0;
                }
                value_hash = NIL;

                if (value_lti == NIL)
                {
                    q = smem_stmts->attribute_frequency_get;
                    q->bind_int(1, attr_hash);

                    element_type = attr_t;
                }
                else
                {
                    q = smem_stmts->wmes_lti_frequency_get;
                    q->bind_int(1, attr_hash);
                    q->bind_int(2, value_lti);

                    element_type = value_lti_t;
                }
            }

            if (good_wme)
            {
                if (q->execute() == soar_module::row)
                {
                    new_cue_element = new smem_weighted_cue_element;

                    new_cue_element->weight = q->column_int(0);
                    new_cue_element->attr_hash = attr_hash;
                    new_cue_element->value_hash = value_hash;
                    new_cue_element->value_lti = value_lti;
                    new_cue_element->cue_element = w;

                    new_cue_element->element_type = element_type;
                    new_cue_element->pos_element = pos_cue;
                    new_cue_element->mathElement = mathQuery;

                    weighted_pq.push(new_cue_element);
                    new_cue_element = NULL;
                }
                else
                {
                    if (pos_cue)
                    {
                        good_wme = false;
                    }
                }

                q->reinitialize();
            }
        }
        else
        {
            if (pos_cue)
            {
                good_wme = false;
            }
        }
    }
    //If we brought in a math query and didn't use it
    if (!good_wme && mathQuery != NIL)
    {
        delete mathQuery;
    }
    return good_wme;
}

//this returns a pair with <needFullSearch, goodCue>
std::pair<bool, bool>* SMem_Manager::processMathQuery(Symbol* mathQuery, smem_prioritized_weighted_cue* weighted_pq)
{
    bool needFullSearch = false;
    //Use this set to track when certain elements have been added, so we don't add them twice
    std::set<Symbol*> uniqueMathQueryElements;
    std::pair<bool, bool>* result = new std::pair<bool, bool>(true, true);

    smem_wme_list* cue = get_direct_augs_of_id(mathQuery);
    for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
    {

        smem_wme_list* cueTypes =get_direct_augs_of_id((*cue_p)->value);
        if (cueTypes->empty())
        {
            //This would be an attribute without a query type attached
            result->first = false;
            result->second = false;
            break;
        }
        else
        {
            for (smem_wme_list::iterator cueType = cueTypes->begin(); cueType != cueTypes->end(); cueType++)
            {
                if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_less)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryLess((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryLess((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_greater)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryGreater((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryGreater((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_less_or_equal)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryLessOrEqual((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryLessOrEqual((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_greater_or_equal)
                {
                    if ((*cueType)->value->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryGreaterOrEqual((*cueType)->value->fc->value));
                    }
                    else if ((*cueType)->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
                    {
                       process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryGreaterOrEqual((*cueType)->value->ic->value));
                    }
                    else
                    {
                        //There isn't a valid value to compare against
                        result->first = false;
                        result->second = false;
                        break;
                    }
                }
                else if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_max)
                {
                    if (uniqueMathQueryElements.find(thisAgent->symbolManager->soarSymbols.smem_sym_math_query_max) != uniqueMathQueryElements.end())
                    {
                        //Only one max at a time
                        result->first = false;
                        result->second = false;
                        break;
                    }
                    else
                    {
                        uniqueMathQueryElements.insert(thisAgent->symbolManager->soarSymbols.smem_sym_math_query_max);
                    }
                    needFullSearch = true;
                   process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryMax());
                }
                else if ((*cueType)->attr == thisAgent->symbolManager->soarSymbols.smem_sym_math_query_min)
                {
                    if (uniqueMathQueryElements.find(thisAgent->symbolManager->soarSymbols.smem_sym_math_query_min) != uniqueMathQueryElements.end())
                    {
                        //Only one min at a time
                        result->first = false;
                        result->second = false;
                        break;
                    }
                    else
                    {
                        uniqueMathQueryElements.insert(thisAgent->symbolManager->soarSymbols.smem_sym_math_query_min);
                    }
                    needFullSearch = true;
                   process_cue_wme((*cue_p), true, *weighted_pq, new MathQueryMin());
                }
            }
        }
        delete cueTypes;
    }
    delete cue;
    if (result->second)
    {
        result->first = needFullSearch;
        return result;
    }
    return result;
}

smem_lti_id SMem_Manager::process_query(Symbol* state, Symbol* query, Symbol* negquery, Symbol* mathQuery, smem_lti_set* prohibit, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_query_levels query_level, uint64_t number_to_retrieve , std::list<smem_lti_id>* match_ids, uint64_t depth, smem_install_type install_type)
{
    smem_weighted_cue_list weighted_cue;
    bool good_cue = true;

    //This is used when doing math queries that need to look at more that just the first valid element
    bool needFullSearch = false;

    soar_module::sqlite_statement* q = NULL;

    std::list<smem_lti_id> temp_list;
    if (query_level == qry_full)
    {
        match_ids = &(temp_list);
    }

    smem_lti_id king_id = NIL;

    ////////////////////////////////////////////////////////////////////////////
    smem_timers->query->start();
    ////////////////////////////////////////////////////////////////////////////

    // prepare query stats
    {
        smem_prioritized_weighted_cue weighted_pq;

        // positive cue - always
        {
            smem_wme_list* cue = get_direct_augs_of_id(query);
            if (cue->empty())
            {
                good_cue = false;
            }

            for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
            {
                cue_wmes.insert((*cue_p));

                if (good_cue)
                {
                    good_cue = process_cue_wme((*cue_p), true, weighted_pq, NIL);
                }
            }

            delete cue;
        }

        //Look through while were here, so that we can make sure the attributes we need are in the results
        if (mathQuery != NIL && good_cue)
        {
            std::pair<bool, bool>* mpr = processMathQuery(mathQuery, &weighted_pq);
            needFullSearch = mpr->first;
            good_cue = mpr->second;
            delete mpr;
        }

        // negative cue - if present
        if (negquery)
        {
            smem_wme_list* cue = get_direct_augs_of_id(negquery);

            for (smem_wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
            {
                cue_wmes.insert((*cue_p));

                if (good_cue)
                {
                    good_cue = process_cue_wme((*cue_p), false, weighted_pq, NIL);
                }
            }

            delete cue;
        }

        // if valid cue, transfer priority queue to list
        if (good_cue)
        {
            while (!weighted_pq.empty())
            {
                weighted_cue.push_back(weighted_pq.top());
                weighted_pq.pop();
            }
        }
        // else deallocate priority queue contents
        else
        {
            while (!weighted_pq.empty())
            {
                smem_prioritized_weighted_cue::value_type top = weighted_pq.top();
                weighted_pq.pop();
                if (top->mathElement != NIL)
                {
                    delete top->mathElement;
                }
                delete top;
                /*if(weighted_pq.top()->mathElement != NIL){
                    delete weighted_pq.top()->mathElement;
                }
                delete weighted_pq.top();
                weighted_pq.pop();*/
            }
        }
    }

    // only search if the cue was valid
    if (good_cue && !weighted_cue.empty())
    {
        // by definition, the first positive-cue element dictates the candidate set
        smem_weighted_cue_list::iterator cand_set;
        smem_weighted_cue_list::iterator next_element;
        for (next_element = weighted_cue.begin(); next_element != weighted_cue.end(); next_element++)
        {
            if ((*next_element)->pos_element)
            {
                cand_set = next_element;
                break;
            }
        }

        soar_module::sqlite_statement* q2 = NULL;
        smem_lti_set::iterator prohibit_p;

        smem_lti_id cand;
        bool good_cand;

        if (smem_params->activation_mode->get_value() == smem_param_container::act_base)
        {
            // naive base-level updates means update activation of
            // every candidate in the minimal list before the
            // confirmation walk
            if (smem_params->base_update->get_value() == smem_param_container::bupt_naive)
            {
                q =setup_web_crawl((*cand_set));

                // queue up distinct lti's to update
                // - set because queries could contain wilds
                // - not in loop because the effects of activation may actually
                //   alter the resultset of the query (isolation???)
                std::set< smem_lti_id > to_update;
                while (q->execute() == soar_module::row)
                {
                    to_update.insert(q->column_int(0));
                }

                for (std::set< smem_lti_id >::iterator it = to_update.begin(); it != to_update.end(); it++)
                {
                    lti_activate((*it), false);
                }

                q->reinitialize();
            }
        }

        // setup first query, which is sorted on activation already
        q =setup_web_crawl((*cand_set));
        thisAgent->lastCue = new agent::BasicWeightedCue((*cand_set)->cue_element, (*cand_set)->weight);

        // this becomes the minimal set to walk (till match or fail)
        if (q->execute() == soar_module::row)
        {
            smem_prioritized_activated_lti_queue plentiful_parents;
            bool more_rows = true;
            bool use_db = false;
            bool has_feature = false;

            while (more_rows && (q->column_double(1) == static_cast<double>(SMEM_ACT_MAX)))
            {
                smem_stmts->act_lti_get->bind_int(1, q->column_int(0));
                smem_stmts->act_lti_get->execute();
                plentiful_parents.push(std::make_pair< double, smem_lti_id >(smem_stmts->act_lti_get->column_double(0), q->column_int(0)));
                smem_stmts->act_lti_get->reinitialize();

                more_rows = (q->execute() == soar_module::row);
            }
            bool first_element = false;
            while (((match_ids->size() < number_to_retrieve) || (needFullSearch)) && ((more_rows) || (!plentiful_parents.empty())))
            {
                // choose next candidate (db vs. priority queue)
                {
                    use_db = false;

                    if (!more_rows)
                    {
                        use_db = false;
                    }
                    else if (plentiful_parents.empty())
                    {
                        use_db = true;
                    }
                    else
                    {
                        use_db = (q->column_double(1) >  plentiful_parents.top().first);
                    }

                    if (use_db)
                    {
                        cand = q->column_int(0);
                        more_rows = (q->execute() == soar_module::row);
                    }
                    else
                    {
                        cand = plentiful_parents.top().second;
                        plentiful_parents.pop();
                    }
                }

                // if not prohibited, submit to the remaining cue elements
                prohibit_p = prohibit->find(cand);
                if (prohibit_p == prohibit->end())
                {
                    good_cand = true;

                    for (next_element = weighted_cue.begin(); next_element != weighted_cue.end() && good_cand; next_element++)
                    {
                        // don't need to check the generating list
                        //If the cand_set is a math query, we care about more than its existence
                        if ((*next_element) == (*cand_set) && (*next_element)->mathElement == NIL)
                        {
                            continue;
                        }

                        if ((*next_element)->element_type == attr_t)
                        {
                            // parent=? AND attribute_s_id=?
                            q2 = smem_stmts->web_attr_child;
                        }
                        else if ((*next_element)->element_type == value_const_t)
                        {
                            // parent=? AND attribute_s_id=? AND value_constant_s_id=?
                            q2 = smem_stmts->web_const_child;
                            q2->bind_int(3, (*next_element)->value_hash);
                        }
                        else if ((*next_element)->element_type == value_lti_t)
                        {
                            // parent=? AND attribute_s_id=? AND value_lti_id=?
                            q2 = smem_stmts->web_lti_child;
                            q2->bind_int(3, (*next_element)->value_lti);
                        }

                        // all require own id, attribute
                        q2->bind_int(1, cand);
                        q2->bind_int(2, (*next_element)->attr_hash);

                        has_feature = (q2->execute() == soar_module::row);
                        bool mathQueryMet = false;
                        if ((*next_element)->mathElement != NIL && has_feature)
                        {
                            do
                            {
                                smem_hash_id valueHash = q2->column_int(2 - 1);
                                smem_stmts->hash_rev_type->bind_int(1, valueHash);

                                if (smem_stmts->hash_rev_type->execute() != soar_module::row)
                                {
                                    good_cand = false;
                                }
                                else
                                {
                                    switch (smem_stmts->hash_rev_type->column_int(1 - 1))
                                    {
                                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                                            mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(rhash__float(valueHash));
                                            break;
                                        case INT_CONSTANT_SYMBOL_TYPE:
                                            mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(rhash__int(valueHash));
                                            break;
                                    }
                                }
                                smem_stmts->hash_rev_type->reinitialize();
                            }
                            while (q2->execute() == soar_module::row);
                            good_cand = mathQueryMet;
                        }
                        else
                        {
                            good_cand = (((*next_element)->pos_element) ? (has_feature) : (!has_feature));
                        }
                        //In CSoar this needs to happen before the break, or the query might not be ready next time
                        q2->reinitialize();
                        if (!good_cand)
                        {
                            break;
                        }
                    }

                    if (good_cand)
                    {
                        king_id = cand;
                        first_element = true;
                        match_ids->push_back(cand);
                        prohibit->insert(cand);
                    }
                    if (good_cand && first_element)
                    {
                        for (smem_weighted_cue_list::iterator wce = weighted_cue.begin(); wce != weighted_cue.end(); wce++)
                        {
                            if ((*wce)->mathElement != NIL)
                            {
                                (*wce)->mathElement->commit();
                            }
                        }
                    }
                    else if (first_element)
                    {
                        for (smem_weighted_cue_list::iterator wce = weighted_cue.begin(); wce != weighted_cue.end(); wce++)
                        {
                            if ((*wce)->mathElement != NIL)
                            {
                                (*wce)->mathElement->rollback();
                            }
                        }
                    }
                }
            }
//            if (!match_ids->empty())
//            {
//                king_id = match_ids->front();
//            }
        }
        q->reinitialize();

        // clean weighted cue
        for (next_element = weighted_cue.begin(); next_element != weighted_cue.end(); next_element++)
        {
            if ((*next_element)->mathElement != NIL)
            {
                delete(*next_element)->mathElement;
            }
            delete(*next_element);
        }
    }

    // reconstruction depends upon level
    if (query_level == qry_full)
    {
        // produce results
        if (king_id != NIL)
        {
            // success!
            add_triple_to_recall_buffer(meta_wmes, state->id->smem_result_header, thisAgent->symbolManager->soarSymbols.smem_sym_success, query);
            if (negquery)
            {
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_result_header, thisAgent->symbolManager->soarSymbols.smem_sym_success, negquery);
            }

            ////////////////////////////////////////////////////////////////////////////
            smem_timers->query->stop();
            ////////////////////////////////////////////////////////////////////////////

            install_memory(state, king_id, NIL, (smem_params->activate_on_query->get_value() == on), meta_wmes, retrieval_wmes, install_type, depth);
        }
        else
        {
            add_triple_to_recall_buffer(meta_wmes, state->id->smem_result_header, thisAgent->symbolManager->soarSymbols.smem_sym_failure, query);
            if (negquery)
            {
                add_triple_to_recall_buffer(meta_wmes, state->id->smem_result_header, thisAgent->symbolManager->soarSymbols.smem_sym_failure, negquery);
            }

            ////////////////////////////////////////////////////////////////////////////
            smem_timers->query->stop();
            ////////////////////////////////////////////////////////////////////////////
        }
    }
    else
    {
        ////////////////////////////////////////////////////////////////////////////
        smem_timers->query->stop();
        ////////////////////////////////////////////////////////////////////////////
    }

    return king_id;
}



