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
#include "output_manager.h"

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
    use_same_file = new soar_module::boolean_param("use-same-file", off, new soar_module::f_predicate<boolean>());
    line_style = new soar_module::string_param("line-style", "polyline", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    architectural_wmes = new soar_module::boolean_param("architectural-wmes", off, new soar_module::f_predicate<boolean>());
    separate_states = new soar_module::boolean_param("separate_states", on, new soar_module::f_predicate<boolean>());


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
    add(use_same_file);
    add(line_style);
    add(architectural_wmes);
    add(separate_states);
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
    std::string tempString;
    Output_Manager* outputManager = &Output_Manager::Get_OM();
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa_sf(thisAgent, "======= Visualization Commands and Settings =======\n");
    outputManager->printa_sf(thisAgent, "visualize ? %-%-%s\n", "Print this help listing");
    outputManager->printa_sf(thisAgent, "visualize [wm | smem | epmem] [id] [depth] %-%-%s\n", "Visualize from memory system");
    outputManager->printa_sf(thisAgent, "visualize [ last | instantiations | contributors] %-%-%s\n", "Visualize explainer analysis");
    outputManager->printa_sf(thisAgent, "------------------ Presentation -------------------\n");
    tempString = "[ ";
    tempString += (rule_format->get_value() == viz_name) ?  "NAME" : "name";
    tempString += " | ";
    tempString += (rule_format->get_value() == viz_full) ?  "FULL" : "full";
    tempString += "]";
    outputManager->printa_sf(thisAgent, "%s %-%s\n",
        concatJustified("rule-format", tempString, 51).c_str(),"Print all conditions and actions or just the rule name");
    tempString = "[ ";
    tempString += (memory_format->get_value() == viz_node) ?  "NODE" : "node";
    tempString += " | ";
    tempString += (memory_format->get_value() == viz_record) ?  "RECORD" : "record";
    tempString += "]";
    outputManager->printa_sf(thisAgent, "%s %-%s\n",
        concatJustified("memory-format", tempString, 51).c_str(),"Print memories as records or just simple nodes");

    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("line-style", line_style->get_value(), 51).c_str(), "GraphViz line style that will be used");
    outputManager->printa_sf(thisAgent, "separate-states                   %-%s%-%s\n", capitalizeOnOff(separate_states->get_value()), "Whether to create links between goal states");
    outputManager->printa_sf(thisAgent, "architectural-wmes                %-%s%-%s\n", capitalizeOnOff(architectural_wmes->get_value()), "Whether to include WMEs created by the Soar architecture");
    outputManager->printa_sf(thisAgent, "------------------ File Handling ------------------\n");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("file-name", file_name->get_value(), 51).c_str(), "");
    outputManager->printa_sf(thisAgent, "use-same-file                   %-%s%-%s\n", capitalizeOnOff(use_same_file->get_value()), "Whether to create new files each time");
    outputManager->printa_sf(thisAgent, "generate-image                  %-%s%-%s\n", capitalizeOnOff(gen_image->get_value()), "Whether an image should be created");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("image-type", image_type->get_value(), 51).c_str(), "Image type that will be generated");
    outputManager->printa_sf(thisAgent, "------------------ Post Actions -------------------\n");
    outputManager->printa_sf(thisAgent, "viewer-launch                   %-%s%-%s\n", capitalizeOnOff(launch_viewer->get_value()), "Launch image in viewer");
    outputManager->printa_sf(thisAgent, "editor-launch                   %-%s%-%s\n", capitalizeOnOff(launch_editor->get_value()), "Open data file in editor");
    outputManager->printa_sf(thisAgent, "print-debug                     %-%s%-%s\n", capitalizeOnOff(print_gv->get_value()), "Print data file to screen for debugging");

    outputManager->printa_sf(thisAgent, "\nTo change a setting: %-%- visualize <setting> [<value>]\n");
        outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%-help visualize\n");

}
