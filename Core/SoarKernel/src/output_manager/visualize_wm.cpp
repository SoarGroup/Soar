/*
 * visualize_wm.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: mazzin
 */
#include "visualize.h"

#include "agent.h"
#include "output_manager.h"
#include "production.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"

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
    wme* w;
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        add_triple(w->id, w->attr, w->value);
    }
}

void WM_Visualization_Map::reset()
{
    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        delete it->second;
    }
    id_augmentations->clear();
}

void WM_Visualization_Map::visualize_wm_as_linked_records()
{
    augmentation_set* lAugSet;
    Symbol* lIDSym;
    augmentation* lAug;

    std::string graphviz_connections;

    reset();
    add_current_wm();

    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        thisAgent->visualizationManager->viz_object_start(it->first, 0, viz_id_and_augs);
        lIDSym = it->first;
        lAugSet = it->second;
        for (auto it2 = lAugSet->begin(); it2 != lAugSet->end(); ++it2)
        {
            lAug = (*it2);
            if (lAug->value->is_identifier() && (lAug->attr != thisAgent->symbolManager->soarSymbols.superstate_symbol))
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

void WM_Visualization_Map::visualize_wm_as_graph()
{
    reset();
    wme* w;
    tc_number tc = get_new_tc_number(thisAgent);

    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        if (!thisAgent->visualizationManager->is_include_arch_enabled() && !w->preference) continue;
        if (w->id->tc_num != tc)
        {
            thisAgent->visualizationManager->viz_object_start(w->id, 0, viz_wme);
            thisAgent->visualizationManager->viz_object_end(viz_wme);
            thisAgent->visualizationManager->viz_endl();
            w->id->tc_num = tc;
        }
        if (w->value->tc_num != tc)
        {
            thisAgent->visualizationManager->viz_object_start(w->value, 0, viz_wme);
            thisAgent->visualizationManager->viz_object_end(viz_wme);
            thisAgent->visualizationManager->viz_endl();
            w->value->tc_num = tc;
        }
        if (w->attr != thisAgent->symbolManager->soarSymbols.superstate_symbol)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "\"%y\":s -\xF2 \"%y\":n [label = \"%y\"]\n\n", w->id, w->value, w->attr);
        } else {
            thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "\"%y\":s -\xF2 \"State_%y\":n [label = \"%y\"]\n\n", w->id, w->value, w->attr);
        }
    }

}
