#include "semantic_memory.h"

#include "smem_db.h"
#include "output_manager.h"
#include "lexer.h"

bool SMem_Manager::export_smem(uint64_t lti_id, std::string& result_text, std::string** err_msg)
{
    smem_chunk_set store_set;

    if (!lti_id)
    {
        thisAgent->SMem->create_full_store_set(&store_set);
    } else {
        thisAgent->SMem->create_store_set(&store_set, lti_id, 0);
    }

    thisAgent->outputManager->sprinta_sf(thisAgent, result_text, "smem --add {\n");

    for (auto it = store_set.begin(); it != store_set.end(); ++it)
    {
        smem_chunk* current_ltm = *it;

        /* Skip if LTI has no augmentations.  Will be added by other smem --add clause */
        if (current_ltm->slots->size() > 0)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, result_text, "(@%u", current_ltm->lti_id);

            for (auto map_it = current_ltm->slots->begin(); map_it != current_ltm->slots->end(); ++map_it)
            {
                Symbol* attr = map_it->first;
                smem_slot* current_slot  = map_it->second;

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

void SMem_Manager::create_store_set(smem_chunk_set* store_set, uint64_t lti_id, uint64_t depth)
{
    /*
     * This populates the input argument store_set (a ltm set) with a given lti_id's contents.
     * scijones Sept 9, 2016.
     */
    soar_module::sqlite_statement* expand_q = thisAgent->SMem->smem_stmts->web_expand;
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
//    thisAgent->SMem->get_lti_name(new_lti->lti_id, new_lti->lti_name);
    bfs.push(new_lti);
        // optionally depth-limited breadth-first-search of children
    while (!bfs.empty())
    {
        parent_lti = bfs.front();
        bfs.pop();
        smem_chunk* new_ltm = new smem_chunk();
        new_ltm->lti_id = parent_lti->lti_id;
        new_ltm->slots = new smem_slot_map();
        // get direct children: attr_type, attr_hash, value_type, value_hash, value_lti
        expand_q->bind_int(1, parent_lti->lti_id);
        while (expand_q->execute() == soar_module::row)
        {
            Symbol* attr = rhash_(static_cast<byte>(expand_q->column_int(0)), static_cast<smem_hash_id>(expand_q->column_int(1)));
            //thisAgent->symbolManager->symbol_remove_ref(&attr);
            if (new_ltm->slots->find(attr) == new_ltm->slots->end())
            {
                (*(new_ltm->slots))[attr] = new smem_slot();
            }
            smem_chunk_value* new_value = new smem_chunk_value();

            if (expand_q->column_int(6) != SMEM_AUGMENTATIONS_NULL)
            {
                new_lti = new smem_vis_lti;
                new_lti->lti_id = expand_q->column_int(6);
                new_lti->level = (parent_lti->level + 1);
//                thisAgent->SMem->get_lti_name(new_lti->lti_id, new_lti->lti_name);
                new_value->val_lti.val_type = value_lti_t;
                new_value->val_const.val_type = value_lti_t;
                new_value->val_lti.val_value = new smem_chunk();
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
                new_value->val_const.val_value  = rhash_(expand_q->column_int(2),expand_q->column_int(3));
                //thisAgent->symbolManager->symbol_remove_ref(&(new_value->val_const.val_value));
            }
            new_ltm->slots->at(attr)->push_back(new_value);
        }
        store_set->insert(new_ltm);
        expand_q->reinitialize();
    }
}

void SMem_Manager::create_full_store_set(smem_chunk_set* store_set)
{
    //This makes a set that contains the entire contents of smem.
    soar_module::sqlite_statement* q;
    q = thisAgent->SMem->smem_stmts->vis_lti;
    while (q->execute() == soar_module::row)
    {
        create_store_set(store_set, q->column_int(0), 1);
    }
    q->reinitialize();
}

void SMem_Manager::clear_store_set(smem_chunk_set* store_set)
{
    //this doesn't delete the set itself. This deletes the contents.
    smem_chunk_set::iterator set_it;
    for (set_it = store_set->begin(); set_it != store_set->end(); ++set_it)
    {
        smem_chunk* current_ltm = *set_it;
        smem_slot_map* current_smem_slot_map = current_ltm->slots;
        smem_slot_map::iterator map_it;
        for (map_it = current_smem_slot_map->begin(); map_it != current_smem_slot_map->end(); ++map_it)
        {
            Symbol* attr = map_it->first;
            smem_slot* current_slot  = map_it->second;
            smem_slot::iterator slot_it;
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
        delete current_smem_slot_map;
        delete current_ltm;
    }
}
