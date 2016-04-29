/*
 * visualize.h
 *
 *  Created on: Apr 23, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_VISUALIZE_H_
#define CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_VISUALIZE_H_

#include "kernel.h"
#include "stl_typedefs.h"

typedef struct aug_struct
{
        Symbol* attr;
        Symbol* value;
} augmentation;

class WM_Visualization_Map
{
    public:

        WM_Visualization_Map(agent* myAgent);
        ~WM_Visualization_Map();

        void reset();
        void add_triple(Symbol* id, Symbol* attr, Symbol* value);
        void add_current_wm();
        void visualize_wm_as_linked_records();
        void visualize_wm_as_graph();

    private:

        agent*             thisAgent;
        sym_to_aug_map*    id_augmentations;
};

class GraphViz_Visualizer
{
    public:

        GraphViz_Visualizer(agent* myAgent);
        ~GraphViz_Visualizer();

        /* A string buffer for the visualization command */
        std::string         graphviz_output;

        void visualize_wm();

        bool is_viz_print_enabled() { return m_viz_print; }
        bool is_viz_launch_img_enabled() { return m_viz_launch_image; }
        bool is_viz_launch_gv_enabled() { return m_viz_launch_gv; }
        bool is_simple_inst_enabled() { return m_simple_inst; }
        bool is_include_chunk_enabled() { return m_include_chunk; }
        void toggle_viz_print_enabled() { if (m_viz_print) m_viz_print = false; else m_viz_print = true; }
        void toggle_viz_launch_img_enabled() { if (m_viz_launch_image) m_viz_launch_image = false; else m_viz_launch_image = true; }
        void toggle_viz_launch_gv_enabled() { if (m_viz_launch_gv) m_viz_launch_gv = false; else m_viz_launch_gv = true; }
        void toggle_simple_inst_enabled() { if (m_simple_inst) m_simple_inst = false; else m_simple_inst = true; }
        void toggle_include_chunk_enabled() { if (m_include_chunk) m_include_chunk = false; else m_include_chunk = true; }

        /* Utility graphviz printing functions */
        void                viz_graph_start(bool pLeftRight = true);
        void                viz_graph_end();
        void                viz_object_start(Symbol* pName, uint64_t node_id, visualizationObjectType objectType);
        void                viz_object_end(visualizationObjectType objectType);
        void                viz_table_start();
        void                viz_table_end();
        void                viz_NCC_start();
        void                viz_NCC_end();
        void                viz_seperator();
        void                viz_record_start();
        void                viz_record_end(bool pLeftJustify = true);
        void                viz_endl();
        void                viz_text_record(const char* pMsg);
        void                viz_table_element_start(uint64_t pNodeID = 0, char pTypeChar = ' ', bool pIsLeftPort = true);
        void                viz_table_element_end();
        void                viz_table_element_conj_start(uint64_t pNodeID = 0, char pTypeChar = ' ', bool pIsLeftPort = true);

        void                escape_graphviz_chars();
        void                clear_visualization();

        void                viz_connect_action_to_cond(uint64_t pSrcRuleID, uint64_t pSrcActionID, uint64_t pTargetRuleID, uint64_t pTargetCondID);
        void                viz_connect_inst_to_chunk(uint64_t pSrcRuleID, uint64_t pTargetRuleID, uint64_t pTargetCondID);

    private:

        agent*              thisAgent;
        Output_Manager*     outputManager;

        bool m_viz_print, m_viz_launch_image, m_viz_launch_gv, m_simple_inst, m_include_chunk;


};

#endif /* CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_VISUALIZE_H_ */
