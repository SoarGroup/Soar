/*
 * visualize.cpp
 *
 *  Created on: Apr 23, 2016
 *      Author: mazzin
 */
#include "visualize.h"

#include "action_record.h"
#include "condition_record.h"
#include "identity_record.h"
#include "instantiation_record.h"
#include "production_record.h"

#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "ebc.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"
#include "working_memory.h"

#include <regex>

ID_Augmentation_Map::ID_Augmentation_Map(agent* myAgent)
{
    thisAgent = myAgent;
    id_augmentations = new sym_to_aug_map();
}

ID_Augmentation_Map::~ID_Augmentation_Map()
{
    reset();
    delete id_augmentations;
}

void ID_Augmentation_Map::add_triple(Symbol* id, Symbol* attr, Symbol* value)
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

void ID_Augmentation_Map::add_current_wm()
{
    wme* w;
    for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        add_triple(w->id, w->attr, w->value);
    }
}

void ID_Augmentation_Map::reset()
{
    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        delete it->second;
    }
    id_augmentations->clear();
}
void ID_Augmentation_Map::visualize_wm()
{
    reset();
    add_current_wm();

    augmentation_set* lAugSet;
    Symbol* lIDSym;
    augmentation* lAug;

    std::string graphviz_connections;


    for (auto it = id_augmentations->begin(); it != id_augmentations->end(); it++)
    {
        thisAgent->visualizer->viz_rule_start(it->first, 0, viz_id_and_augs);
        lIDSym = it->first;
        lAugSet = it->second;
        for (auto it2 = lAugSet->begin(); it2 != lAugSet->end(); ++it2)
        {
            lAug = (*it2);
            if (lAug->value->is_identifier() && (lAug->attr != thisAgent->superstate_symbol))
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, graphviz_connections, "\"%y\" -#@ \"%y\" [label = \"%y\"]\n", lIDSym, lAug->value, lAug->attr);

            } else {
                thisAgent->visualizer->viz_record_start();
                thisAgent->visualizer->viz_table_element_start();
                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizer->graphviz_output, "%y", lAug->attr);
                thisAgent->visualizer->viz_table_element_end();
                thisAgent->visualizer->viz_table_element_start();
                thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizer->graphviz_output, "%y", lAug->value);
                thisAgent->visualizer->viz_table_element_end();
                thisAgent->visualizer->viz_record_end();
                thisAgent->visualizer->viz_endl();
            }
        }
        thisAgent->visualizer->viz_rule_end(viz_id_and_augs);
        thisAgent->visualizer->viz_endl();
    }
    thisAgent->visualizer->graphviz_output += graphviz_connections;
}

GraphViz_Visualizer::GraphViz_Visualizer(agent* myAgent)
{
    thisAgent = myAgent;
    outputManager = thisAgent->outputManager;
    m_viz_print = false;
    m_viz_launch_image = true;
    m_viz_launch_gv = false;
    m_simple_inst = true;
    m_include_chunk = false;
}

GraphViz_Visualizer::~GraphViz_Visualizer()
{
}

void GraphViz_Visualizer::visualize_wm()
{
    graphviz_output.clear();

    ID_Augmentation_Map* wme_map = new ID_Augmentation_Map(thisAgent);
    viz_graph_start();
    wme_map->visualize_wm();
    viz_graph_end();
    delete wme_map;
}

void GraphViz_Visualizer::viz_graph_start()
{
    graphviz_output.clear();

    graphviz_output +=  "digraph g {\n"
        "   graph [ rankdir = \"LR\" splines = \"spline\"];\n"
        "   node [fontsize = \"16\"];\n"
        "   edge [];\n";
}
void GraphViz_Visualizer::viz_graph_end()
{
    graphviz_output += "}\n";

    escape_graphviz_chars();
}

void GraphViz_Visualizer::viz_rule_start(Symbol* pName, uint64_t node_id, visualizationObjectType objectType)
{
    switch (objectType)
    {
        case viz_inst_record:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   rule%u [\n"
                "      penwidth = \"0\"\n"
                "      label = @#", node_id);
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                @#TR#@ @#TD COLSPAN=\"3\"#@Instantiation (i %u) of rule@#BR/#@%y@#/TD#@ @#/TR#@\n", node_id, pName);
            break;

        case viz_chunk_record:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   chunk%u [\n"
                "      shape = \"box\"  style = \"dashed, bold, rounded\"\n"
                "      label = @#", node_id);
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                @#TR#@ @#TD COLSPAN=\"3\"#@%y (i %u)@#/TD#@ @#/TR#@\n", pName, node_id);
            break;
        case viz_id_and_augs:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   \"%y\" [\n"
                "      penwidth = \"0\"\n"
                "      label = @#", pName);
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                @#TR#@ @#TD COLSPAN=\"3\"#@%y@#/TD#@ @#/TR#@\n", pName);
            break;
        case viz_simple_inst:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   rule%u [\n"
                "      shape = \"box\" style = \"rounded\"\n"
                "      label = \"%y (i %u)", node_id, pName, node_id);
            break;
        case viz_wme:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   rule%u [\n"
                "      shape = \"box\" style = \"rounded\"\n"
                "      label = \"%y (i %u)", node_id, pName, node_id);
            break;

        default:
            assert(false);
            break;
    }

}

void GraphViz_Visualizer::viz_rule_end(visualizationObjectType objectType)
{
    switch (objectType)
    {
        case viz_inst_record:
        case viz_chunk_record:
        case viz_id_and_augs:
            viz_table_end();
            graphviz_output += "              #@\n   ];\n\n";
            break;

        case viz_simple_inst:
        case viz_wme:
            graphviz_output += "\"\n   ];\n\n";
            break;

        default:
            assert(false);
            break;
    }
}

void GraphViz_Visualizer::viz_record_start()
{
    graphviz_output += "                @#TR#@ ";
}

void GraphViz_Visualizer::viz_record_end(bool pLeftJustify)
{
    if (pLeftJustify)
        graphviz_output +=  " @#/TR#@";
    else
        graphviz_output +=  " @#/TR#@";
}
void GraphViz_Visualizer::viz_table_start()
{
    outputManager->sprinta_sf(thisAgent, graphviz_output,
        "@#TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\"#@\n");
}

void GraphViz_Visualizer::viz_table_end()
{
    graphviz_output += "              @#/TABLE#@\n";
}

void GraphViz_Visualizer::viz_table_element_conj_start(uint64_t pNodeID, char pTypeChar, bool pIsLeftPort)
{
    if (pNodeID)
    {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\n                @#TD PORT=\"%c_%u%cs\"#@ ",
            pTypeChar, pNodeID, (pIsLeftPort ? "_l" : "_r"));
    } else {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\n                @#TD#@ ");
    }
    viz_table_start();
}

void GraphViz_Visualizer::viz_table_element_start(uint64_t pNodeID, char pTypeChar, bool pIsLeftPort)
{
    if (pNodeID)
    {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "@#TD PORT=\"%c_%u%s\"#@ ",
            pTypeChar, pNodeID, (pIsLeftPort ? "_l" : "_r"));
    } else {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "@#TD#@ ");
    }
}

void GraphViz_Visualizer::viz_table_element_end()
{
    outputManager->sprinta_sf(thisAgent, graphviz_output, "@#/TD#@");
}

void GraphViz_Visualizer::viz_text_record(const char* pMsg)
{
    outputManager->sprinta_sf(thisAgent, graphviz_output, "@#TD#@ %s @#/TD#@", pMsg);
}

void GraphViz_Visualizer::viz_NCC_start()
{
    graphviz_output +=  "                @#TR#@ @#TD COLSPAN=\"3\" ALIGN=\"LEFT\"#@ -\$ @#/TD#@ @#/TR#@\n";
}

void GraphViz_Visualizer::viz_NCC_end()
{
    graphviz_output +=  "                @#TR#@ @#TD COLSPAN=\"3\" ALIGN=\"LEFT\"#@ \% @#/TD#@ @#/TR#@\n";
}

void GraphViz_Visualizer::viz_seperator()
{
    graphviz_output +=  "                @#TR#@ @#TD COLSPAN=\"3\"#@ ----> @#/TD#@ @#/TR#@";
}

void GraphViz_Visualizer::viz_endl()
{
    graphviz_output +=  "\n";
}

void GraphViz_Visualizer::escape_graphviz_chars()
{
    /* Note that we temporarily use #@ and @# for graphviz < > that don't need to be escaped */
    /* MToDo | Should find a better way.  */
    graphviz_output = std::regex_replace(graphviz_output, std::regex("<"), "&lt;");
    graphviz_output = std::regex_replace(graphviz_output, std::regex(">"), "&gt;");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("@#"), "<");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("#@"), ">");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("\\$"), "\{");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("\\%"), "\}");
}
void GraphViz_Visualizer::clear_visualization()
{
        graphviz_output.clear();
}
void GraphViz_Visualizer::viz_connect_action_to_cond(uint64_t pSrcRuleID, uint64_t pSrcActionID, uint64_t pTargetRuleID, uint64_t pTargetCondID)
{
    graphviz_output += "   rule";
    graphviz_output += std::to_string(pSrcRuleID);
    graphviz_output += ":a_";
    graphviz_output += std::to_string(pSrcActionID);
    graphviz_output += "_r -#@ rule";
    graphviz_output += std::to_string(pTargetRuleID);
    graphviz_output += ":c_";
    graphviz_output += std::to_string(pTargetCondID);
    graphviz_output += "_l\n";
}

void GraphViz_Visualizer::viz_connect_inst_to_chunk(uint64_t pSrcRuleID, uint64_t pTargetRuleID, uint64_t pTargetCondID)
{
    graphviz_output += "   rule";
    graphviz_output += std::to_string(pSrcRuleID);
    graphviz_output += " -#@ chunk";
    graphviz_output += std::to_string(pTargetRuleID);
    graphviz_output += ":c_";
    graphviz_output += std::to_string(pTargetCondID);
    graphviz_output += "_l [style = \"dashed\"  penwidth = \"2\"]\n";
}

