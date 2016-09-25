/*
 * visualize.h
 *
 *  Created on: Apr 23, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_H_
#define CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_H_

#include "kernel.h"

#include "visualize_settings.h"

#include <string>

class GraphViz_Visualizer
{
    public:

        GraphViz_Visualizer(agent* myAgent);
        ~GraphViz_Visualizer();

        /* A string buffer for the visualization command */
        std::string         graphviz_output;
        Viz_Parameters*     settings;

        void visualize_wm();
        void visualize_smem(uint64_t lti_id = 0, int depth = 1);

        const std::string get_next_filename() {
            if (!thisAgent->visualizationManager->settings->use_same_file->get_value())
            {
                std::string lFileName = thisAgent->visualizationManager->settings->file_name->get_value();
                ++m_file_count;
                lFileName.append(std::to_string(m_file_count));
                /* MToDO | Check if another file exists and keep trying new names */
                return lFileName;
            }
            return thisAgent->visualizationManager->settings->file_name->get_value();
        }

        /* Utility graphviz printing functions */
        void viz_graph_start(bool pLeftRight = true);
        void viz_graph_end();
        void viz_object_start(Symbol* pName, uint64_t node_id, visObjectType objectType, std::string* pMakeUnique = NULL);
        void viz_object_start_string(std::string &pName, uint64_t node_id, visObjectType objectType, std::string* pMakeUnique = NULL);
        void viz_object_end(visObjectType objectType);
        void viz_table_start();
        void viz_table_end();
        void viz_NCC_start();
        void viz_NCC_end();
        void viz_seperator();
        void viz_record_start();
        void viz_record_end(bool pLeftJustify = true);
        void viz_endl();
        void viz_text_record(const char* pMsg);
        void viz_table_element_start(uint64_t pNodeID = 0, char pTypeChar = ' ', bool pIsLeftPort = true);
        void viz_table_element_end();
        void viz_table_element_conj_start(uint64_t pNodeID = 0, char pTypeChar = ' ', bool pIsLeftPort = true);

        void escape_graphviz_chars();
        void clear_visualization();

        void viz_connect_action_to_cond(uint64_t pSrcRuleID, uint64_t pSrcActionID, uint64_t pTargetRuleID, uint64_t pTargetCondID);
        void viz_connect_inst_to_chunk(uint64_t pSrcRuleID, uint64_t pTargetRuleID, uint64_t pTargetCondID);

    private:

        agent*              thisAgent;
        Output_Manager*     outputManager;

        uint64_t            m_file_count;
        uint64_t            m_unique_counter;
};

#endif /* CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_H_ */
