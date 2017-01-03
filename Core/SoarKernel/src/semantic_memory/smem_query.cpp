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

#include "dprint.h"
#include "symbol.h"
#include "working_memory.h"

#include "VariadicBind.h"

#include <thread>

std::shared_ptr<sqlite_thread_guard> SMem_Manager::setup_web_crawl(smem_weighted_cue_element* el)
{
    // first, point to correct query and setup
    // query-specific parameters
    std::shared_ptr<sqlite_thread_guard> sql;

    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_attr_all);
        (*sql)->bind(1, el->attr_hash);
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_const_all);
        (*sql)->bind(1, el->attr_hash);
        (*sql)->bind(2, el->value_hash);
    }
    else //if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        sql = std::make_shared<sqlite_thread_guard>(SQL->web_lti_all);
        (*sql)->bind(1, el->attr_hash);
        (*sql)->bind(2, el->value_lti);
    }

    return sql;
}

std::shared_ptr<sqlite_thread_guard> SMem_Manager::setup_web_crawl_spread(smem_weighted_cue_element* el)
{
    std::shared_ptr<sqlite_thread_guard> q;

    // first, point to correct query and setup
    // query-specific parameters
    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_attr_all_spread);
        (*q)->bind(1, el->attr_hash);
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_const_all_spread);
        (*q)->bind(2, el->value_hash);
        (*q)->bind(1, el->attr_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_lti_all_spread);
        (*q)->bind(2, el->value_lti);
        (*q)->bind(1, el->attr_hash);
    }

    // all require hash as first parameter
    //q->bind(1, el->attr_hash);

    return q;
}

std::shared_ptr<sqlite_thread_guard> SMem_Manager::setup_cheap_web_crawl(smem_weighted_cue_element* el)
{
    std::shared_ptr<sqlite_thread_guard> q;

    // first, point to correct query and setup
    // query-specific parameters
    if (el->element_type == attr_t)
    {
        // attribute_s_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_attr_all_cheap);
        (*q)->bind(1, el->attr_hash);
    }
    else if (el->element_type == value_const_t)
    {
        // attribute_s_id=? AND value_constant_s_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_const_all_cheap);
        (*q)->bind(2, el->value_hash);
        (*q)->bind(1, el->attr_hash);
    }
    else if (el->element_type == value_lti_t)
    {
        // attribute_s_id=? AND value_lti_id=?
        q = std::make_shared<sqlite_thread_guard>(SQL->web_lti_all_cheap);
        (*q)->bind(2, el->value_lti);
        (*q)->bind(1, el->attr_hash);
    }

    // all require hash as first parameter
    //q->bind(1, el->attr_hash);

    return q;
}

bool SMem_Manager::process_cue_wme(wme* w, bool pos_cue, smem_prioritized_weighted_cue& weighted_pq, MathQuery* mathQuery)
{
    bool good_wme = true;
    smem_weighted_cue_element* new_cue_element;

    smem_hash_id attr_hash;
    smem_hash_id value_hash;
    uint64_t value_lti;
    smem_cue_element_type element_type;

    sqlite_thread_guard* q = nullptr;

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
                q = new sqlite_thread_guard(SQL->wmes_constant_frequency_get);
                (*q)->bind(1, attr_hash);
                (*q)->bind(2, value_hash);
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
                value_lti = w->value->id->LTI_ID;
            }
            else
            {
                value_lti = 0;
            }
            value_hash = NIL;

            if (value_lti == NIL)
            {
                q = new sqlite_thread_guard(SQL->attribute_frequency_get);
                (*q)->bind(1, attr_hash);

                element_type = attr_t;
            }
            else
            {
                q = new sqlite_thread_guard(SQL->wmes_lti_frequency_get);
                (*q)->bind(1, attr_hash);
                (*q)->bind(2, value_lti);

                element_type = value_lti_t;
            }
        }

        if (good_wme)
        {
            if ((*q)->executeStep())
            {
                new_cue_element = new smem_weighted_cue_element;

                new_cue_element->weight = (*q)->getColumn(0).getInt64();
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
        }
    }
    else
    {
        if (pos_cue)
        {
            good_wme = false;
        }
    }

    delete q;

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

    wme_list* cue = get_direct_augs_of_id(mathQuery);
    for (wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
    {

        wme_list* cueTypes =get_direct_augs_of_id((*cue_p)->value);
        if (cueTypes->empty())
        {
            //This would be an attribute without a query type attached
            result->first = false;
            result->second = false;
            break;
        }
        else
        {
            for (wme_list::iterator cueType = cueTypes->begin(); cueType != cueTypes->end(); cueType++)
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

void SMem_Manager::process_query_SQL(smem_weighted_cue_list weighted_cue, bool needFullSearch, const id_set& prohibit_lti, Symbol* state, Symbol* query, Symbol* negquery, std::list<uint64_t>* match_ids, uint64_t number_to_retrieve, uint64_t depth)
{
    id_set prohibit;
    prohibit.insert(prohibit_lti.begin(), prohibit_lti.end());
    //Under the philosophy that activation only matters in the service of a query, we defer processing prohibits until now..
    id_set::iterator prohibited_lti_p;
    for (prohibited_lti_p = prohibit.begin(); prohibited_lti_p != prohibit.end(); ++prohibited_lti_p)
    {
        std::packaged_task<bool()> pt_ProhibitCheck([this, prohibited_lti_p] {
            auto sql = sqlite_thread_guard(SQL->prohibit_check);

            sql->bind(1, *prohibited_lti_p);

            return sql->executeStep();
        });
        bool prohibited = JobQueue->post(pt_ProhibitCheck).get();
        if (prohibited)
        {
            std::packaged_task<void()> pt_ProhibitSet([this, prohibited_lti_p]{
                auto sql = sqlite_thread_guard(SQL->prohibit_set);

                sql->bind(1, *prohibited_lti_p);

                sql->exec();
            });

            JobQueue->post(pt_ProhibitSet).wait();
        }
    }

    //smem_weighted_cue_list weighted_cue;
    bool good_cue = true;

    uint64_t king_id = NIL;

    std::list<uint64_t> temp;
    if (match_ids == nullptr)
        match_ids = &temp;

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

    timers->query->stop();

    //If spreading is on, and depending on the candidate set, we do spreading processing.
    if (settings->spreading->get_value() == on)
    {
        timers->spreading->start();
        auto q = setup_cheap_web_crawl(*cand_set);
        std::set<uint64_t> to_update;
        int num_answers = 0;
        while ((*q)->executeStep() && num_answers < 400)
        {//TODO: The 400 there should actually reflect the size of the context's recipients.
            num_answers++;
            to_update.insert((*q)->getColumn(0).getInt64());
        }
        timers->spreading->stop();
        if (num_answers >= 400)
        {
            calc_spread(&to_update, true, &cand_set);
        }
        else if (num_answers > 1)
        {
            calc_spread(&to_update, false);
        }
    }

    timers->query->start();

    id_set::iterator prohibit_p;

    uint64_t cand;
    bool good_cand;

    if (settings->activation_mode->get_value() == smem_param_container::act_base)
    {
        // naive base-level updates means update activation of
        // every candidate in the minimal list before the
        // confirmation walk
        if (settings->base_update->get_value() == smem_param_container::bupt_naive)
        {
            auto q = setup_web_crawl((*cand_set));

            // queue up distinct lti's to update
            // - set because queries could contain wilds
            // - not in loop because the effects of activation may actually
            //   alter the resultset of the query (isolation???)
            std::set< uint64_t > to_update;
            while ((*q)->executeStep())
            {
                to_update.insert((*q)->getColumn(0).getInt64());
            }

            for (std::set< uint64_t >::iterator it = to_update.begin(); it != to_update.end(); it++)
            {
                lti_activate((*it), false);
            }
        }
    }

    // setup first query, which is sorted on activation already
    auto q = setup_web_crawl((*cand_set));
    thisAgent->lastCue = new agent::BasicWeightedCue((*cand_set)->cue_element, (*cand_set)->weight);

    // this becomes the minimal set to walk (till match or fail)
    if ((*q)->executeStep())
    {
        smem_prioritized_activated_lti_queue plentiful_parents;
        bool more_rows = true;
        bool use_db = false;
        bool has_feature = false;

        while (more_rows && ((*q)->getColumn(1).getDouble() == static_cast<double>(SMEM_ACT_MAX)))
        {
            auto sql = sqlite_thread_guard(SQL->act_lti_get);

            sql->bind(1, (*q)->getColumn(0).getInt64());

            if (!sql->executeStep())
                throw SoarAssertionException("Failed to retrieve column", __FILE__, __LINE__);

            plentiful_parents.push(std::make_pair<double, uint64_t>(sql->getColumn(0).getDouble(), (*q)->getColumn(0).getInt64()));

            more_rows = (*q)->executeStep();
        }
        auto spread_q = setup_web_crawl_spread(*cand_set);
        //uint64_t highest_so_far = 0;
        while ((*spread_q)->executeStep())
        {
            plentiful_parents.push(std::make_pair<double, uint64_t>((*spread_q)->getColumn(1).getDouble(), (*spread_q)->getColumn(0).getInt64()));
        }
        //spread_q->reinitialize();
        bool first_element = false;
        while ((match_ids->size() < number_to_retrieve || needFullSearch) && ((more_rows) || (!plentiful_parents.empty())))
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
                    use_db = (*q)->getColumn(1).getDouble() >  plentiful_parents.top().first;
                }

                if (use_db)
                {
                    cand = (*q)->getColumn(0).getInt64();
                    more_rows = (*q)->executeStep();
                }
                else
                {
                    cand = plentiful_parents.top().second;
                    plentiful_parents.pop();
                }
            }

            // if not prohibited, submit to the remaining cue elements
            prohibit_p = prohibit.find(cand);
            if (prohibit_p == prohibit.end())
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

                    sqlite_thread_guard* q2 = nullptr;
                    if ((*next_element)->element_type == attr_t)
                    {
                        // parent=? AND attribute_s_id=?
                        q2 = new sqlite_thread_guard(SQL->web_attr_child);
                    }
                    else if ((*next_element)->element_type == value_const_t)
                    {
                        // parent=? AND attribute_s_id=? AND value_constant_s_id=?
                        q2 = new sqlite_thread_guard(SQL->web_const_child);
                        (*q2)->bind(3, (*next_element)->value_hash);
                    }
                    else if ((*next_element)->element_type == value_lti_t)
                    {
                        // parent=? AND attribute_s_id=? AND value_lti_id=?
                        q2 = new sqlite_thread_guard(SQL->web_lti_child);
                        (*q2)->bind(3, (*next_element)->value_lti);
                    }

                    // all require own id, attribute
                    (*q2)->bind(1, cand);
                    (*q2)->bind(2, (*next_element)->attr_hash);

                    has_feature = (*q2)->executeStep();
                    bool mathQueryMet = false;
                    if ((*next_element)->mathElement != NIL && has_feature)
                    {
                        do
                        {
                            smem_hash_id valueHash = (*q2)->getColumn(2 - 1).getInt64();
                            auto sql = sqlite_thread_guard(SQL->hash_rev_type);
                            sql->bind(1, valueHash);

                            if (!sql->executeStep())
                            {
                                good_cand = false;
                            }
                            else
                            {
                                switch (sql->getColumn(1 - 1).getInt64())
                                {
                                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                                        mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(rhash__float(valueHash));
                                        break;
                                    case INT_CONSTANT_SYMBOL_TYPE:
                                        mathQueryMet |= (*next_element)->mathElement->valueIsAcceptable(rhash__int(valueHash));
                                        break;
                                }
                            }
                        }
                        while ((*q2)->executeStep());
                        good_cand = mathQueryMet;
                    }
                    else
                    {
                        good_cand = (((*next_element)->pos_element) ? (has_feature) : (!has_feature));
                    }

                    delete q2;

                    //In CSoar this needs to happen before the break, or the query might not be ready next time
                    if (!good_cand)
                    {
                        break;
                    }
                }

                if (good_cand)
                {
                    king_id = cand;
                    first_element = true;
                    prohibit.insert(cand);
                    match_ids->push_back(cand);
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
    }

    // clean weighted cue
    for (next_element = weighted_cue.begin(); next_element != weighted_cue.end(); next_element++)
    {
        if ((*next_element)->mathElement != NIL)
        {
            delete(*next_element)->mathElement;
        }
        delete(*next_element);
    }

    std::lock_guard<std::mutex> lock(agent_jobqueue_boundary_mutex);
    query_results.push_back({king_id, depth, state, query, negquery});
}

void SMem_Manager::process_query(Symbol* state, Symbol* query, Symbol* negquery, Symbol* mathQuery, const id_set& prohibit, wme_set& cue_wmes, symbol_triple_list& meta_wmes, symbol_triple_list& retrieval_wmes, smem_query_levels query_level, std::list<uint64_t> *match_ids, uint64_t number_to_retrieve, uint64_t depth, smem_install_type install_type, bool synchronous)
{
    smem_weighted_cue_list weighted_cue;
    bool good_cue = true;

    dprint(DT_SMEM_INSTANCE, "process_query called with %y %y %y %y\n", state, query, negquery, mathQuery);
    //This is used when doing math queries that need to look at more that just the first valid element
    bool needFullSearch = false;

    ////////////////////////////////////////////////////////////////////////////
    timers->query->start();
    ////////////////////////////////////////////////////////////////////////////

    // prepare query stats
    {
        smem_prioritized_weighted_cue weighted_pq;

        // positive cue - always
        {
            wme_list* cue = get_direct_augs_of_id(query);
            if (cue->empty())
            {
                good_cue = false;
            }

            for (wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
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
            wme_list* cue = get_direct_augs_of_id(negquery);

            for (wme_list::iterator cue_p = cue->begin(); cue_p != cue->end(); cue_p++)
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

    double cand_act = 0.0;
    // only search if the cue was valid
    if (good_cue && !weighted_cue.empty())
    {
        std::packaged_task<void()> processQuery([=] {
            process_query_SQL(weighted_cue, needFullSearch, prohibit, state, query, negquery, match_ids, number_to_retrieve, depth);
        });

        auto task = JobQueue->post(processQuery);
        if (synchronous)
            task.wait();
    }

    ////////////////////////////////////////////////////////////////////////////
    timers->query->stop();
    ////////////////////////////////////////////////////////////////////////////
}



