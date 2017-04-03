/*
 * visualize.cpp
 *
 *  Created on: Apr 23, 2016
 *      Author: mazzin
 */
#include "visualize.h"

#include "agent.h"
#include "condition.h"
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
#include "dprint.h"
#include "action_record.h"
#include "condition_record.h"
#include "identity_record.h"
#include "instantiation_record.h"
#include "semantic_memory.h"
#include "production_record.h"
#include "visualize_wm.h"
#include "visualize_settings.h"

// Test if a path exists
bool fileExists(const char* path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributes(path);
    return a != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return (stat(path, &st) == 0);
#endif
}
const std::string GraphViz_Visualizer::get_next_filename() {
    if (!thisAgent->visualizationManager->settings->use_same_file->get_value())
    {
        std::string lFileNameBase = thisAgent->visualizationManager->settings->file_name->get_value();
        std::string lFileName;
        bool fileNameFound = false;
        while (!fileNameFound)
        {
            lFileName = lFileNameBase + std::to_string(++m_file_count) + std::string(".gv");
            fileNameFound = !fileExists(lFileName.c_str());
        }
        lFileName = lFileNameBase + std::to_string(m_file_count);
        return lFileName;
    }
    return thisAgent->visualizationManager->settings->file_name->get_value();
}

GraphViz_Visualizer::GraphViz_Visualizer(agent* myAgent)
{
    thisAgent = myAgent;
    outputManager = thisAgent->outputManager;

    settings = new Viz_Parameters(thisAgent);
    m_file_count = 0;
    m_next_color = 0;
    m_unique_counter = 0;
}

GraphViz_Visualizer::~GraphViz_Visualizer()
{
    delete settings;
}

void GraphViz_Visualizer::visualize_wm(Symbol* pSym, int pDepth)
{
    graphviz_output.clear();

    WM_Visualization_Map* wme_map = new WM_Visualization_Map(thisAgent);
    viz_graph_start(false);
    if (thisAgent->visualizationManager->settings->memory_format->get_value() == viz_node)
    {
        wme_map->visualize_wm_as_graph(pSym, pDepth);
    } else {
        wme_map->visualize_wm_as_linked_records(pSym, pDepth);
    }
    viz_graph_end();
    delete wme_map;
}

void GraphViz_Visualizer::visualize_smem(uint64_t lti_id, int pDepth)
{
    ltm_set store_set;
    std::string graphviz_connections, idStr;
    bool lRecordStarted ;

    if (!lti_id)
    {
        thisAgent->SMem->create_full_store_set(&store_set);
    } else {
        thisAgent->SMem->create_store_set(&store_set, lti_id, pDepth);
    }

    graphviz_output.clear();
    viz_graph_start(false);

    for (auto it = store_set.begin(); it != store_set.end(); ++it)
    {
        ltm_object* current_ltm = *it;
        idStr = "@";
        idStr.append(std::to_string(current_ltm->lti_id));

        thisAgent->visualizationManager->viz_object_start_string(idStr, current_ltm->lti_id, viz_id_and_augs);
        lRecordStarted = false;

        for (auto map_it = current_ltm->slots->begin(); map_it != current_ltm->slots->end(); ++map_it)
        {
            Symbol* attr = map_it->first;
            ltm_slot* current_slot  = map_it->second;

            for (auto slot_it = current_slot->begin(); slot_it != current_slot->end(); ++slot_it)
            {
                if ((*slot_it)->val_lti.val_type == value_lti_t)
                {
                    thisAgent->outputManager->sprinta_sf(thisAgent, graphviz_connections, "\"@%u\":s -\xF2 \"@%u\":n [label = \"%y\"]\n", current_ltm->lti_id, (*slot_it)->val_lti.val_value->lti_id, attr);
                }
                else
                {
                    if (!lRecordStarted)
                    {
                        thisAgent->visualizationManager->viz_record_start();
                        lRecordStarted = true;
                    }
                    thisAgent->visualizationManager->viz_table_element_start();
                    thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", attr);
                    thisAgent->visualizationManager->viz_table_element_end();
                    thisAgent->visualizationManager->viz_table_element_start();
                    thisAgent->outputManager->sprinta_sf(thisAgent, thisAgent->visualizationManager->graphviz_output, "%y", (*slot_it)->val_const.val_value);
                    thisAgent->visualizationManager->viz_table_element_end();
                }
                if (lRecordStarted)
                {
                    thisAgent->visualizationManager->viz_record_end();
                    thisAgent->visualizationManager->viz_endl();
                    lRecordStarted = false;
                }
            }
        }
        thisAgent->visualizationManager->viz_object_end(viz_id_and_augs);
        thisAgent->visualizationManager->viz_endl();
    }
    thisAgent->visualizationManager->graphviz_output += graphviz_connections;
    viz_graph_end();

    thisAgent->SMem->clear_store_set(&store_set);
}

void GraphViz_Visualizer::viz_graph_start(bool pLeftRight)
{
    graphviz_output.clear();

    graphviz_output +=  "digraph g {\n"
        "   node [shape = \"box\" fontsize = \"16\"];\n"
        "   edge [];\n";
    if (pLeftRight)
    {
        graphviz_output +=  "   graph [ rankdir = \"LR\" ";
    } else {
        graphviz_output +=  "   graph [ rankdir = \"TD\" ";
    }
    graphviz_output +=  "splines = \"";
    graphviz_output += thisAgent->visualizationManager->settings->line_style->get_value();
    graphviz_output +=  "\"];\n";
}

void GraphViz_Visualizer::viz_graph_end()
{
    graphviz_output += "}\n";

    escape_graphviz_chars();
}

void GraphViz_Visualizer::viz_object_start(Symbol* pName, uint64_t node_id, visObjectType objectType, std::string* pMakeUnique)
{

    std::string pNameString = pName->to_string();
    viz_object_start_string(pNameString, node_id, objectType, pMakeUnique);
}

void GraphViz_Visualizer::viz_object_start_string(std::string &pName, uint64_t node_id, visObjectType objectType, std::string* pMakeUnique)
{
    std::string nodeName(pName);
    if (pMakeUnique)
    {
        nodeName += std::to_string(++m_unique_counter);
    }
    switch (objectType)
    {
        case viz_inst_record:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   rule%u [\n"
                "      colorscheme=\"X11\"\n"
                "      penwidth = \"0\"\n"
                "      \n"
                "      label = \xF3", node_id);
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\" BGCOLOR=\"BLACK\"\xF2 \xF3\FONT COLOR=\"WHITE\"\xF2Instantiation (i %u) of rule\xF3\x42R/\xF2%s\xF3/FONT\xF2\xF3/TD\xF2\xF3/TR\xF2\n", node_id, pName.c_str());
            break;

        case viz_chunk_record:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   chunk%u [\n"
                "      style = \"dashed, bold, rounded\"\n"
                "      colorscheme=\"X11\"\n"
                "      label = \xF3", node_id);
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\" BGCOLOR=\"BLUE\"\xF2 \xF3\FONT COLOR=\"YELLOW\"\xF2%s (i %u)\xF3/FONT\xF2\xF3/TD\xF2 \xF3/TR\xF2\n", pName.c_str(), node_id);
            break;
        case viz_id_and_augs:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   \"%s\" [\n"
                "      colorscheme=\"X11\"\n"
                "      penwidth = \"0\"\n"
                "      label = \xF3", nodeName.c_str());
            viz_table_start();
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\"\xF2%s\xF3/TD\xF2 \xF3/TR\xF2\n", pName.c_str());
            break;
        case viz_simple_inst:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   rule%u [\n"
                "      colorscheme=\"X11\"\n"
                "      shape = \"box\" style = \"rounded\"\n"
                "      label = \"%s (i %u)", node_id, pName.c_str(), node_id);
            break;
        case viz_wme:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   \"%s\" [\n"
                "      colorscheme=\"X11\"\n"
                "      shape = \"circle\"\n"
                "      label = \"%s", nodeName.c_str(), pName.c_str());
            break;
        case viz_wme_terminal:
            outputManager->sprinta_sf(thisAgent, graphviz_output,
                "   \"%s\" [\n"
                "      colorscheme=\"X11\"\n"
                "      shape = \"box\"\n"
                "      label = \"%s", nodeName.c_str(), pName.c_str());
            break;

        default:
            assert(false);
            break;
    }
    if (pMakeUnique)
    {
        (*pMakeUnique) = nodeName;
    }
}

void GraphViz_Visualizer::viz_object_end(visObjectType objectType)
{
    switch (objectType)
    {
        case viz_inst_record:
        case viz_chunk_record:
        case viz_id_and_augs:
            viz_table_end();
            graphviz_output += "              \xF2\n   ];\n\n";
            break;

        case viz_simple_inst:
        case viz_wme:
        case viz_wme_terminal:
            graphviz_output += "\"\n   ];\n\n";
            break;

        default:
            assert(false);
            break;
    }
}

void GraphViz_Visualizer::viz_record_start()
{
    graphviz_output += "                \xF3TR\xF2 ";
}

void GraphViz_Visualizer::viz_record_end(bool pLeftJustify)
{
    if (pLeftJustify)
        graphviz_output +=  " \xF3/TR\xF2";
    else
        graphviz_output +=  " \xF3/TR\xF2";
}
void GraphViz_Visualizer::viz_table_start()
{
    outputManager->sprinta_sf(thisAgent, graphviz_output,
        "\xF3TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\"\xF2\n");
}

void GraphViz_Visualizer::viz_table_end()
{
    graphviz_output += "              \xF3/TABLE\xF2\n";
}

void GraphViz_Visualizer::viz_table_element_conj_start(uint64_t pNodeID, char pTypeChar, WME_Field pField, bool pRowBorder, const char* pModStr)
{
    const char* borderString;
    if (pRowBorder) borderString =  " BORDER=\"3\""; else borderString =  " ";
    if (pNodeID)
    {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\n                \xF3TD%s%sPORT=\"%c_%u%s\"\xF2 ",
            borderString, pModStr, pTypeChar, pNodeID, ((pField == ID_ELEMENT) ? "_l" : "_r"));
    } else {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\n                \xF3TD%s%s\xF2 ", borderString, pModStr);
    }
    viz_table_start();
}

void GraphViz_Visualizer::viz_table_element_start(uint64_t pNodeID, char pTypeChar, WME_Field pField, bool pRowBorder, const char* pModStr)
{
    const char* borderString;
    if (pRowBorder) borderString =  " BORDER=\"3\""; else borderString =  " ";

    if (pNodeID)
    {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\xF3TD%s%sPORT=\"%c_%u%s\"\xF2 ",
            borderString, pModStr, pTypeChar, pNodeID, ((pField == ID_ELEMENT) ? "_l" : "_r"));
    } else {
        outputManager->sprinta_sf(thisAgent, graphviz_output, "\xF3TD%s%s\xF2 ", borderString, pModStr);
    }
}

void GraphViz_Visualizer::viz_table_element_end()
{
    outputManager->sprinta_sf(thisAgent, graphviz_output, "\xF3/TD\xF2");
}

void GraphViz_Visualizer::viz_text_record(const char* pMsg)
{
    outputManager->sprinta_sf(thisAgent, graphviz_output, "\xF3TD\xF2 %s \xF3/TD\xF2", pMsg);
}

void GraphViz_Visualizer::viz_NCC_start()
{
    graphviz_output +=  "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\" ALIGN=\"LEFT\"\xF2 -\xE3 \xF3/TD\xF2 \xF3/TR\xF2\n";
}

void GraphViz_Visualizer::viz_NCC_end()
{
    graphviz_output +=  "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\" ALIGN=\"LEFT\"\xF2 \xE1 \xF3/TD\xF2 \xF3/TR\xF2\n";
}

void GraphViz_Visualizer::viz_seperator()
{
    graphviz_output +=  "                \xF3TR\xF2 \xF3TD COLSPAN=\"3\"\xF2 ----> \xF3/TD\xF2 \xF3/TR\xF2";
}

void GraphViz_Visualizer::viz_endl()
{
    graphviz_output +=  "\n";
}

void GraphViz_Visualizer::escape_graphviz_chars()
{

    if (graphviz_output.empty()) return;

    char last_char = 0;
    std::string finalString;

    for(char &c : graphviz_output)
    {
        switch (c)
        {
            case '<':
                finalString += "&lt;";
                break;
            case '>':
                finalString += "&gt;";
                break;
            case '\xF3':
                finalString += "<";
                break;
            case '\xF2':
                finalString += ">";
                break;
            case '\xE3':
                finalString += "{";
                break;
            case '\xE1':
                finalString += "}";
                break;
            default:
                if (last_char)
                {
                    finalString += last_char;
                    last_char = 0;
                }
                finalString += c;
                break;
        }
    }
    graphviz_output = finalString;
}

void GraphViz_Visualizer::clear_visualization()
{
        graphviz_output.clear();
}
void GraphViz_Visualizer::viz_connect_action_to_cond(uint64_t pSrcRuleID, uint64_t pSrcActionID, uint64_t pTargetRuleID, uint64_t pTargetCondID)
{
    graphviz_output += "   rule";
    graphviz_output += std::to_string(pSrcRuleID);
    if (thisAgent->visualizationManager->settings->rule_format->get_value() == viz_name)
    {
        graphviz_output += ":e";
    } else {
        graphviz_output += ":a_";
        graphviz_output += std::to_string(pSrcActionID);
        graphviz_output += "_r ";
    }
    graphviz_output += "-\xF2 rule";
    graphviz_output += std::to_string(pTargetRuleID);
    if (thisAgent->visualizationManager->settings->rule_format->get_value() == viz_name)
    {
        graphviz_output += ":w\n";
    } else {
        graphviz_output += ":c_";
        graphviz_output += std::to_string(pTargetCondID);
        graphviz_output += "_l\n";
    }
}

void GraphViz_Visualizer::viz_connect_inst_to_chunk(uint64_t pSrcRuleID, uint64_t pTargetRuleID)
{
    graphviz_output += "   rule";
    graphviz_output += std::to_string(pSrcRuleID);
    graphviz_output += " -\xF2 chunk";
    graphviz_output += std::to_string(pTargetRuleID);
    graphviz_output += "[style = \"dashed\"  penwidth = \"2\"]\n";
}

void GraphViz_Visualizer::viz_connect_identities(uint64_t pSrcRuleID, uint64_t pTargetRuleID)
{
    graphviz_output += "   Identity_";
    graphviz_output += std::to_string(pSrcRuleID);
    graphviz_output += " -\xF2 Identity_";
    graphviz_output += std::to_string(pTargetRuleID);
    graphviz_output += "[style = \"dashed\"  penwidth = \"2\"]\n";
}
