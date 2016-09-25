/*
 * visualize_settings.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: mazzin
 */




/*
 * decider_settings.cpp
 *
 *  Created on: Sep 11, 2016
 *      Author: mazzin
 */

#include "visualize_settings.h"

//#include "agent.h"
//#include "output_manager.h"

Viz_Parameters::Viz_Parameters(agent* new_agent): soar_module::param_container(new_agent)
{
    memory_format = new soar_module::constant_param<visMemoryFormat>("memory-format", viz_record, new soar_module::f_predicate<visMemoryFormat>());
    memory_format->add_mapping(viz_node, "node");
    memory_format->add_mapping(viz_record, "record");
    rule_format = new soar_module::constant_param<visRuleFormat>("rule-format", viz_full, new soar_module::f_predicate<visRuleFormat>());
    rule_format->add_mapping(viz_name, "name");
    rule_format->add_mapping(viz_full, "full");

    launch_viewer = new soar_module::boolean_param("viewer-launch", on, new soar_module::f_predicate<boolean>());
    launch_editor = new soar_module::boolean_param("editor-launch", off, new soar_module::f_predicate<boolean>());
    print_gv = new soar_module::boolean_param("print", off, new soar_module::f_predicate<boolean>());
    gen_image = new soar_module::boolean_param("generate-image", on, new soar_module::f_predicate<boolean>());

    image_type = new soar_module::string_param("image-type", "svg", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    file_name = new soar_module::string_param("file-name", "soar_viz", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    line_style = new soar_module::string_param("line-style", "polyline", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    include_io_links = new soar_module::boolean_param("include-io-links", off, new soar_module::f_predicate<boolean>());


    viz_wm = new soar_module::boolean_param("wm", on, new soar_module::f_predicate<boolean>());
    viz_smem = new soar_module::boolean_param("smem", on, new soar_module::f_predicate<boolean>());
    viz_epmem = new soar_module::boolean_param("epmem", on, new soar_module::f_predicate<boolean>());
    viz_last = new soar_module::boolean_param("last", on, new soar_module::f_predicate<boolean>());
    viz_instantiations = new soar_module::boolean_param("instantiations", on, new soar_module::f_predicate<boolean>());
    viz_contributors = new soar_module::boolean_param("contributors", on, new soar_module::f_predicate<boolean>());

    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());

    add(memory_format);
    add(rule_format);
    add(launch_viewer);
    add(launch_editor);
    add(print_gv);
    add(gen_image);
    add(image_type);
    add(file_name);
    add(line_style);
    add(include_io_links);
    add(viz_wm);
    add(viz_smem);
    add(viz_epmem);
    add(viz_last);
    add(viz_instantiations);
    add(viz_contributors);
    add(help_cmd);
    add(qhelp_cmd);

}

//std::string concatJustified(const char* left_string, std::string right_string, int pWidth);

void Viz_Parameters::print_visualization_settings(agent* thisAgent)
{
}
