/*
 * visualize_wm.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: mazzin
 */

#include "visualize_wm.h"

#include "agent.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "visualize.h"

WM_Visualization_Map::WM_Visualization_Map(agent* myAgent)
{
    thisAgent = myAgent;
    id_augmentations = new sym_to_aug_map();
}

WM_Visualization_Map::~WM_Visualization_Map()
{
    reset();
    delete id_augmentations;
}

void WM_Visualization_Map::add_triple(Symbol* id, Symbol* attr, Symbol* value)
{
    augmentation* lNewAugmentation = new augmentation();
    lNewAugmentation->attr = attr;
    lNewAugmentation->value = value;
    auto it = id_augmentations->find(id);
    if (it == id_augmentations->end())
    {
        (*id_augmentations)[id] = new augmentation_set();
    }
    augmentation_set* lNewAugSet = (*id_augmentations)[id];
    lNewAugSet->insert(lNewAugmentation);
}

void WM_Visualization_Map::add_current_wm()
{
}

void WM_Visualization_Map::reset()
{
    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        delete it->second;
    }
    id_augmentations->clear();
}

void WM_Visualization_Map::visualize_wm_as_linked_records(Symbol* pSym, int pDepth)
{
    augmentation_set* lAugSet;
    Symbol* lIDSym;
    augmentation* lAug;
    bool lSeparateStates = thisAgent->visualizationManager->settings->separate_states->get_value();
    std::string graphviz_connections;

    reset();
    get_wmes_for_symbol(pSym, pDepth);

    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        lIDSym = it->first;
        lAugSet = it->second;
        thisAgent->visualizationManager->viz_object_start(lIDSym, 0, viz_id_and_augs);
        for (auto it2 = lAugSet->begin(); it2 != lAugSet->end(); ++it2)
        {
            lAug = (*it2);
            if (lAug->value->is_sti() && ((!lAug->value->id->isa_goal && !lAug->value->id->isa_impasse) || !lSeparateStates))
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, graphviz_connections, "\"%y\":s -\xF2 \"%y\":n [label = \"%y\"]\n", lIDSym, lAug->value, lAug->attr);
            } else {
                thisAgent->visualizationManager->viz_record_start();
                thisAgent->visualizationManager->viz_table_element_start();
                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", lAug->attr);
                thisAgent->visualizationManager->viz_table_element_end();
                thisAgent->visualizationManager->viz_table_element_start();
                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", lAug->value);
                thisAgent->visualizationManager->viz_table_element_end();
                thisAgent->visualizationManager->viz_record_end();
                thisAgent->visualizationManager->viz_endl();
            }
        }
        thisAgent->visualizationManager->viz_object_end(viz_id_and_augs);
        thisAgent->visualizationManager->viz_endl();
    }
    thisAgent->visualizationManager->graphviz_output += graphviz_connections;
}

void WM_Visualization_Map::visualize_wm_as_graph(Symbol* pSym, int pDepth)
{
    augmentation_set* lAugSet;
    Symbol* lIDSym;
    augmentation* lAug;
    bool lSeparateStates = thisAgent->visualizationManager->settings->separate_states->get_value();

    reset();
    get_wmes_for_symbol(pSym, pDepth);

    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        lIDSym = it->first;
        lAugSet = it->second;
        thisAgent->visualizationManager->viz_object_start(lIDSym, 0, viz_wme);
        thisAgent->visualizationManager->viz_object_end(viz_wme);
        thisAgent->visualizationManager->viz_endl();
        for (auto it2 = lAugSet->begin(); it2 != lAugSet->end(); ++it2)
        {
            lAug = (*it2);
            //            if (lAug->value->is_sti() && ((!lAug->value->id->isa_goal && !lAug->value->id->isa_impasse) || !lSeparateStates))
            //            {
            //                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "\"%y\":s -\xF2 \"%s\":n [label = \"%y\"]\n\n", w->id, nodeName.c_str(), w->attr);
            //            } else {
            std::string nodeName;
            if (!lAug->value->is_sti())
            {
                thisAgent->visualizationManager->viz_object_start(lAug->value, 0, viz_wme_terminal, &nodeName);
                thisAgent->visualizationManager->viz_object_end(viz_wme_terminal);
                thisAgent->visualizationManager->viz_endl();
            } else {
                nodeName = lAug->value->to_string();
            }
            if (!lAug->value->is_sti() || ((!lAug->value->id->isa_goal && !lAug->value->id->isa_impasse) || !lSeparateStates))
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "\"%y\":s -\xF2 \"%s\":n [label = \"%y\"]\n\n", lIDSym, nodeName.c_str(), lAug->attr);
            }
        }
    }

}

int compare_attr2(const void* e1, const void* e2)
{
    wme** p1, **p2;

    char s1[output_string_size + 10], s2[output_string_size + 10];

    p1 = (wme**) e1;
    p2 = (wme**) e2;

    // passing null thisAgent is OK as long as dest is guaranteed != 0
    (*p1)->attr->to_string(true, s1, output_string_size + 10);
    (*p2)->attr->to_string(true, s2, output_string_size + 10);

    return strcmp(s1, s2);
}

/* The following was repurposed from print.h */
void WM_Visualization_Map::add_wmes_of_id(Symbol* id, int depth, int maxdepth, tc_number tc)
{
    slot* s;
    wme* w;

    wme** list;    /* array of WME pointers, AGR 652 */
    int num_attr;  /* number of attributes, AGR 652 */
    int attr;      /* attribute index, AGR 652 */

    if (!id->is_sti()) return;
    if (id->tc_num == tc) return;
    if (id->id->depth > depth) return;

    depth = id->id->depth;
    id->tc_num = tc;

    /* --- first, count all direct augmentations of this id --- */
    num_attr = 0;
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        num_attr++;
    }
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        num_attr++;
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            num_attr++;
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            num_attr++;
        }
    }

    /* --- next, construct the array of wme pointers and sort them --- */
    list = (wme**)thisAgent->memoryManager->allocate_memory(num_attr * sizeof(wme*), MISCELLANEOUS_MEM_USAGE);
    attr = 0;
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        list[attr++] = w;
    }
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        list[attr++] = w;
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            list[attr++] = w;
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            list[attr++] = w;
        }
    }
    qsort(list, num_attr, sizeof(wme*), compare_attr2);

    bool addArch = thisAgent->visualizationManager->settings->architectural_wmes->get_value();
    for (attr = 0; attr < num_attr; attr++)
    {
        w = list[attr];

        if (!addArch && (!w->preference || !w->preference->inst || !w->preference->inst->prod_name))
            continue;

        add_triple(w->id, w->attr, w->value);
    }

    if (depth > 1)
    {
        for (attr = 0; attr < num_attr; attr++)
        {
            w = list[attr];
            add_wmes_of_id(w->attr, depth - 1, maxdepth, tc);
            add_wmes_of_id(w->value, depth - 1, maxdepth, tc);
        }
    }
    thisAgent->memoryManager->free_memory(list, MISCELLANEOUS_MEM_USAGE);
}

void WM_Visualization_Map::mark_depths_augs_of_id(Symbol* id, int depth, tc_number tc)
{
    slot* s;
    wme* w;

    if (!id->is_sti()) return;
    if (id->tc_num == tc && id->id->depth >= depth) return;
    id->id->depth = depth;
    id->tc_num = tc;
    if (depth <= 1) return;

    /* --- call this routine recursively for children --- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        mark_depths_augs_of_id(w->attr, depth - 1, tc);
        mark_depths_augs_of_id(w->value, depth - 1, tc);
    }
    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
    {
        mark_depths_augs_of_id(w->attr, depth - 1, tc);
        mark_depths_augs_of_id(w->value, depth - 1, tc);
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            mark_depths_augs_of_id(w->attr, depth - 1, tc);
            mark_depths_augs_of_id(w->value, depth - 1, tc);
        }
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
        {
            mark_depths_augs_of_id(w->attr, depth - 1, tc);
            mark_depths_augs_of_id(w->value, depth - 1, tc);
        }
    }
}
void WM_Visualization_Map::get_wmes_for_symbol(Symbol* pSym, int pDepth)
{
    /* Add all goal states if no symbol is passed in */
    if (!pSym)
    {
        wme* w;
        bool getArchWMEs = thisAgent->visualizationManager->settings->architectural_wmes->get_value();

        for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
        {
            if (!getArchWMEs && (!w->preference || !w->preference->inst || !w->preference->inst->prod_name)) continue;
            add_triple(w->id, w->attr, w->value);
        }
    } else {
        tc_number seen_TC = get_new_tc_number(thisAgent);
        mark_depths_augs_of_id(pSym, pDepth, seen_TC);
        seen_TC = get_new_tc_number(thisAgent);
        mark_depths_augs_of_id(pSym, pDepth, seen_TC);
        seen_TC = get_new_tc_number(thisAgent);
        add_wmes_of_id(pSym, pDepth, pDepth, seen_TC);
    }
}
