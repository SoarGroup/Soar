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
        bool is_include_arch_enabled() { return m_include_arch; }
        bool is_use_same_file_enabled() { return m_use_same_file; }
        bool is_generate_img_enabled() { return m_generate_img; }
        const char* get_filename() { return m_filename_prefix.c_str(); }
        const char* get_line_style() { return m_line_style.c_str(); }
        const char* get_image_type() { return m_image_type.c_str(); }

        void set_viz_print_enabled(bool pOn) { m_viz_print = pOn; }
        void set_viz_launch_img_enabled(bool pOn) { m_viz_launch_image = pOn; }
        void set_viz_launch_gv_enabled(bool pOn) {m_viz_launch_gv = pOn; }
        void set_simple_inst_enabled(bool pOn) { m_simple_inst = pOn; }
        void set_generate_img_enabled(bool pOn) {  m_generate_img = pOn; }
        void set_include_arch_enabled(bool pOn) {  m_include_arch = pOn; }
        void set_use_same_file_enabled(bool pOn) { m_use_same_file = pOn; }
        void set_filename(const std::string& pString) { m_filename_prefix = pString; }
        void set_line_style(const std::string& pString) { m_line_style = pString; }
        void set_image_type(const std::string& pString) { m_image_type = pString; }

        /* Utility graphviz printing functions */
        void viz_graph_start(bool pLeftRight = true);
        void viz_graph_end();
        void viz_object_start(Symbol* pName, uint64_t node_id, visualizationObjectType objectType);
        void viz_object_end(visualizationObjectType objectType);
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

        bool                m_viz_print, m_viz_launch_image, m_viz_launch_gv, m_simple_inst;
        bool                m_generate_img, m_include_arch, m_use_same_file;
        std::string         m_line_style, m_filename_prefix, m_image_type;
};

#endif /* CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_VISUALIZE_H_ */
