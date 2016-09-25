/*
 * visualize_settings.h
 *
 *  Created on: Sep 25, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_SETTINGS_H_

#include "kernel.h"
#include "soar_module.h"

class Viz_Parameters: public soar_module::param_container
{
    public:

        Viz_Parameters(agent* new_agent);


        soar_module::constant_param<visMemoryFormat>* memory_format;
        soar_module::constant_param<visRuleFormat>* rule_format;
        soar_module::string_param* line_style;
        soar_module::boolean_param* separate_states;
        soar_module::boolean_param* architectural_wmes;

        soar_module::string_param* file_name;
        soar_module::boolean_param* use_same_file;
        soar_module::boolean_param* gen_image;
        soar_module::string_param* image_type;

        soar_module::boolean_param* launch_viewer;
        soar_module::boolean_param* launch_editor;
        soar_module::boolean_param* print_gv;

        soar_module::boolean_param* viz_wm;
        soar_module::boolean_param* viz_smem;
        soar_module::boolean_param* viz_epmem;
        soar_module::boolean_param* viz_last;
        soar_module::boolean_param* viz_instantiations;
        soar_module::boolean_param* viz_contributors;
        soar_module::boolean_param* help_cmd;
        soar_module::boolean_param* qhelp_cmd;

        void print_visualization_settings(agent* thisAgent);

};

#endif /* CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_SETTINGS_H_ */
