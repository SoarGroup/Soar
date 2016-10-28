/*
 * smem_print.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"

#include "dprint.h"
#include "output_manager.h"
#include "lexer.h"

#include "VariadicBind.h"

#include <algorithm>

bool SMem_Manager::export_smem(uint64_t lti_id, std::string& result_text, std::string** err_msg)
{
    ltm_set store_set;

    if (!lti_id)
    {
        thisAgent->SMem->create_full_store_set(&store_set);
    } else {
        thisAgent->SMem->create_store_set(&store_set, lti_id, 0);
    }

    thisAgent->outputManager->sprinta_sf(thisAgent, result_text, "smem --add {\n");

    for (auto it = store_set.begin(); it != store_set.end(); ++it)
    {
        ltm_object* current_ltm = *it;

        /* Skip if LTI has no augmentations.  Will be added by other smem --add clause */
        if (current_ltm->slots->size() > 0)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, result_text, "(@%u", current_ltm->lti_id);

            for (auto map_it = current_ltm->slots->begin(); map_it != current_ltm->slots->end(); ++map_it)
            {
                Symbol* attr = map_it->first;
                ltm_slot* current_slot  = map_it->second;

                thisAgent->outputManager->sprinta_sf(thisAgent, result_text, " ^%y", attr);

                for (auto slot_it = current_slot->begin(); slot_it != current_slot->end(); ++slot_it)
                {
                    if ((*slot_it)->val_lti.val_type == value_lti_t)
                    {
                        thisAgent->outputManager->sprinta_sf(thisAgent, result_text, " @%u", (*slot_it)->val_lti.val_value->lti_id);
                    }
                    else
                    {
                        thisAgent->outputManager->sprinta_sf(thisAgent, result_text, " %y", (*slot_it)->val_const.val_value);
                    }
                }
            }
            thisAgent->outputManager->sprinta_sf(thisAgent, result_text, ")\n");
        }

    }
    thisAgent->outputManager->sprinta_sf(thisAgent, result_text, "}\n");

    thisAgent->SMem->clear_store_set(&store_set);
    return true;
}

void SMem_Manager::create_store_set(ltm_set* store_set, uint64_t lti_id, uint64_t depth)
{
    /*
     * This populates the input argument store_set (a ltm set) with a given lti_id's contents.
     * scijones Sept 9, 2016.
     */
    std::string attr_str;
    int64_t attr_int;
    double attr_double;
    std::string val_str;
    int64_t val_int;
    double val_double;

    smem_vis_lti* new_lti;
    smem_vis_lti* parent_lti;

    std::map< uint64_t, smem_vis_lti* > close_list;
    std::map< uint64_t, smem_vis_lti* >::iterator cl_p;

    std::queue<smem_vis_lti*> bfs;
    new_lti = new smem_vis_lti;
    new_lti->lti_id = lti_id;
    new_lti->level = 0;
    thisAgent->SMem->get_lti_name(new_lti->lti_id, new_lti->lti_name);
    bfs.push(new_lti);
    // optionally depth-limited breadth-first-search of children
    while (!bfs.empty())
    {
        parent_lti = bfs.front();
        bfs.pop();
        ltm_object* new_ltm = new ltm_object();
        new_ltm->lti_id = parent_lti->lti_id;
        new_ltm->slots = new ltm_slot_map();
        // get direct children: attr_type, attr_hash, value_type, value_hash, value_lti
        JobQueue.post([&]() mutable {
            auto sql = sqlite_thread_guard(SQL.web_expand);

            SQLite::bind(*sql, parent_lti->lti_id);
            while (sql->executeStep())
            {
                Symbol* attr = rhash_(static_cast<byte>(sql->getColumn(0).getInt64()), static_cast<smem_hash_id>(sql->getColumn(1).getUInt64()));
                //thisAgent->symbolManager->symbol_remove_ref(&attr);
                if (new_ltm->slots->find(attr) == new_ltm->slots->end())
                {
                    (*(new_ltm->slots))[attr] = new ltm_slot();
                }
                ltm_value* new_value = new ltm_value();

                if (sql->getColumn(4).getInt64() != SMEM_AUGMENTATIONS_NULL)
                {
                    new_lti = new smem_vis_lti;
                    new_lti->lti_id = sql->getColumn(4).getUInt64();
                    new_lti->level = (parent_lti->level + 1);
                    thisAgent->SMem->get_lti_name(new_lti->lti_id, new_lti->lti_name);
                    new_value->val_lti.val_type = value_lti_t;
                    new_value->val_const.val_type = value_lti_t;
                    new_value->val_lti.val_value = new ltm_object();
                    new_value->val_lti.val_value->lti_id = new_lti->lti_id;
                    // prevent looping
                    {
                        cl_p = close_list.find(new_lti->lti_id);
                        if (cl_p == close_list.end())
                        {
                            close_list.insert(std::make_pair(new_lti->lti_id, new_lti));

                            if ((depth == 0) || (new_lti->level < depth))
                            {
                                bfs.push(new_lti);
                            }
                        }
                        else
                        {
                            delete new_lti;
                            new_lti = NULL;
                        }
                    }
                }
                else
                {
                    new_value->val_const.val_type = value_const_t;
                    new_value->val_lti.val_type = value_const_t;
                    new_value->val_const.val_value  = rhash_(sql->getColumn(2).getInt64(),sql->getColumn(3).getInt64());
                    //thisAgent->symbolManager->symbol_remove_ref(&(new_value->val_const.val_value));
                }
                new_ltm->slots->at(attr)->push_back(new_value);
            }
            store_set->insert(new_ltm);
        })->wait();
    }
}

void SMem_Manager::create_full_store_set(ltm_set* store_set)
{
    //This makes a set that contains the entire contents of smem.
    JobQueue.post([&]() {
        auto sql = sqlite_thread_guard(SQL.vis_lti);

        while (sql->executeStep())
            create_store_set(store_set, sql->getColumn(0).getInt64(), 1);
    })->wait();
}

void SMem_Manager::clear_store_set(ltm_set* store_set)
{
    //this doesn't delete the set itself. This deletes the contents.
    ltm_set::iterator set_it;
    for (set_it = store_set->begin(); set_it != store_set->end(); ++set_it)
    {
        ltm_object* current_ltm = *set_it;
        ltm_slot_map* current_ltm_slot_map = current_ltm->slots;
        ltm_slot_map::iterator map_it;
        for (map_it = current_ltm_slot_map->begin(); map_it != current_ltm_slot_map->end(); ++map_it)
        {
            Symbol* attr = map_it->first;
            ltm_slot* current_slot  = map_it->second;
            ltm_slot::iterator slot_it;
            for (slot_it = current_slot->begin(); slot_it != current_slot->end(); ++slot_it)
            {
                if ((*slot_it)->val_lti.val_type == value_lti_t)
                {
                    delete ((*slot_it)->val_lti.val_value);
                }
                else
                {
                    thisAgent->symbolManager->symbol_remove_ref(&((*slot_it)->val_const.val_value));
                }
                delete (*slot_it);
            }
            delete current_slot;
            thisAgent->symbolManager->symbol_remove_ref(&(attr));
        }
        delete current_ltm_slot_map;
        delete current_ltm;
    }
}

id_set SMem_Manager::print_LTM(uint64_t pLTI_ID, double lti_act, std::string* return_val, std::list<uint64_t>* history)
{

    id_set next;

    std::string temp_str, temp_str2, temp_str3;
    int64_t temp_int;
    double temp_double;
    uint64_t temp_lti_id;

    std::map< std::string, std::list< std::string > > augmentations;
    std::map< std::string, std::list< std::string > >::iterator lti_slot;
    std::list< std::string >::iterator slot_val;

    attach();

    return_val->append("(");
    get_lti_name(pLTI_ID, *return_val);

    bool possible_id, possible_ic, possible_fc, possible_sc, possible_var, is_rereadable;

    // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.web_expand);

        SQLite::bind(*sql, pLTI_ID);
        while (sql->executeStep())
        {
            // get attribute
            switch (sql->getColumn(0).getInt64())
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                {
                    rhash__str(sql->getColumn(1).getInt64(), temp_str);

                    if (count(temp_str.begin(), temp_str.end(), ' ') > 0)
                    {
                        temp_str.insert(0, "|");
                        temp_str += '|';
                        break;
                    }

                    soar::Lexer::determine_possible_symbol_types_for_string(temp_str.c_str(),
                        strlen(temp_str.c_str()),
                        &possible_id,
                        &possible_var,
                        &possible_sc,
                        &possible_ic,
                        &possible_fc,
                        &is_rereadable);

                    bool has_angle_bracket = temp_str[0] == '<' || temp_str[temp_str.length() - 1] == '>';

                    if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                        (!is_rereadable) ||
                        has_angle_bracket)
                    {
                        /* BUGBUG if in context where id's could occur, should check
                         possible_id flag here also */
                        temp_str.insert(0, "|");
                        temp_str += '|';
                    }
                    break;
                }
                case INT_CONSTANT_SYMBOL_TYPE:
                    temp_int = rhash__int(sql->getColumn(1).getInt64());
                    to_string(temp_int, temp_str);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    temp_double = rhash__float(sql->getColumn(1).getInt64());
                    to_string(temp_double, temp_str);
                    break;

                default:
                    temp_str.clear();
                    break;
            }

            // identifier vs. constant
            if (sql->getColumn(4).getInt64() != SMEM_AUGMENTATIONS_NULL)
            {
                temp_lti_id = static_cast<uint64_t>(sql->getColumn(4).getInt64());
                temp_str2.clear();
                get_lti_name(temp_lti_id, temp_str2);

                /* The following line prints the children indented.  It seems redundant when printing the
                 * smem store, but perhaps it's useful for printing something rooted in an lti? */
                next.insert(temp_lti_id);
            }
            else
            {
                switch (sql->getColumn(2).getInt64())
                {
                    case STR_CONSTANT_SYMBOL_TYPE:
                    {
                        rhash__str(sql->getColumn(3).getInt64(), temp_str2);

                        if (count(temp_str2.begin(), temp_str2.end(), ' ') > 0)
                        {
                            temp_str2.insert(0, "|");
                            temp_str2 += '|';
                            break;
                        }

                        soar::Lexer::determine_possible_symbol_types_for_string(temp_str2.c_str(),
                            temp_str2.length(),
                            &possible_id,
                            &possible_var,
                            &possible_sc,
                            &possible_ic,
                            &possible_fc,
                            &is_rereadable);

                        bool has_angle_bracket = temp_str2[0] == '<' || temp_str2[temp_str2.length() - 1] == '>';

                        if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                            (!is_rereadable) ||
                            has_angle_bracket)
                        {
                            /* BUGBUG if in context where id's could occur, should check
                             possible_id flag here also */
                            temp_str2.insert(0, "|");
                            temp_str2 += '|';
                        }
                        break;
                    }
                    case INT_CONSTANT_SYMBOL_TYPE:
                        temp_int = rhash__int(sql->getColumn(3).getInt64());
                        to_string(temp_int, temp_str2);
                        break;

                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                        temp_double = rhash__float(sql->getColumn(3).getInt64());
                        to_string(temp_double, temp_str2);
                        break;

                    default:
                        temp_str2.clear();
                        break;
                }
            }

            augmentations[ temp_str ].push_back(temp_str2);
        }
    })->wait();

    // output augmentations nicely
    {
        for (lti_slot = augmentations.begin(); lti_slot != augmentations.end(); lti_slot++)
        {
            return_val->append(" ^");
            return_val->append(lti_slot->first);

            for (slot_val = lti_slot->second.begin(); slot_val != lti_slot->second.end(); slot_val++)
            {
                return_val->append(" ");
                return_val->append((*slot_val));
            }
        }
    }
    augmentations.clear();

    return_val->append(" [");
    to_string(lti_act, temp_str, 3, true);
    if (lti_act >= 0)
    {
        return_val->append("+");
    }
    return_val->append(temp_str);
    return_val->append("]");
    return_val->append(")\n");

    if (history != NIL)
    {
        std::ostringstream temp_string;
        return_val->append("SMem Access Cycle History\n");
        return_val->append("[-");
        for (std::list<uint64_t>::iterator history_item = (*history).begin(); history_item != (*history).end(); ++history_item)
        {
            if (history_item != (*history).begin())
            {
                return_val->append(", -");
            }
            temp_string << ((int64_t)thisAgent->SMem->smem_max_cycle - (int64_t)*history_item);
            return_val->append(temp_string.str());
            temp_string.str("");
        }
        return_val->append("]\n");
    }

    return next;
}

void SMem_Manager::print_store(std::string* return_val)
{
    JobQueue.post([&]() mutable {
        auto sql = sqlite_thread_guard(SQL.vis_lti);

        while (sql->executeStep())
            print_smem_object(sql->getColumn(0).getInt64(), 1, return_val);
    })->wait();
}

void SMem_Manager::print_smem_object(uint64_t pLTI_ID, uint64_t depth, std::string* return_val, bool history)
{
    id_set visited;
    std::pair< id_set::iterator, bool > visited_ins_result;

    std::queue< std::pair< uint64_t, unsigned int > > to_visit;
    std::pair< uint64_t, unsigned int > c;

    id_set next;
    id_set::iterator next_it;

    auto act_q = sqlite_thread_guard(SQL.vis_lti_act);
    auto hist_q = sqlite_thread_guard(SQL.history_get);
    auto lti_access_q = sqlite_thread_guard(SQL.lti_access_get);
    unsigned int i;


    // initialize queue/set
    to_visit.push(std::make_pair(pLTI_ID, 1u));
    visited.insert(pLTI_ID);

    while (!to_visit.empty())
    {
        c = to_visit.front();
        to_visit.pop();

        // output leading spaces ala depth
        for (i = 1; i < c.second; i++)
        {
            return_val->append("  ");
        }

        // get lti info
        {
            SQLite::bind(*act_q, c.first);
            assert(act_q->executeStep());

            //Look up activation history.
            std::list<uint64_t> access_history;
            if (history)
            {
                SQLite::bind(*lti_access_q, c.first);
                assert(lti_access_q->executeStep());
                uint64_t n = lti_access_q->getColumn(0).getInt64();
                lti_access_q->reset();
                SQLite::bind(*hist_q, c.first);
                assert(hist_q->executeStep());
                for (int i = 0; i < n && i < 10; ++i) //10 because of the length of the history record kept for smem.
                {
                    if (hist_q->getColumn(i).getInt64() != 0)
                    {
                        access_history.push_back(hist_q->getColumn(i).getInt64());
                    }
                }
                hist_q->reset();
            }

            if (history && !access_history.empty())
            {
                next = print_LTM(c.first, act_q->getColumn(0).getDouble(), return_val, &(access_history));
            }
            else
            {
                next = print_LTM(c.first, act_q->getColumn(0).getDouble(), return_val);
            }

            // done with lookup
            act_q->reset();

            // consider further depth
            if (c.second < depth)
            {
                for (next_it = next.begin(); next_it != next.end(); next_it++)
                {
                    visited_ins_result = visited.insert((*next_it));
                    if (visited_ins_result.second)
                    {
                        to_visit.push(std::make_pair((*next_it), c.second + 1u));
                    }
                }
            }
        }
    }
}
