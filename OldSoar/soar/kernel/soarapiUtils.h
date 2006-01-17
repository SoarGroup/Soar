#ifndef _SOAR_API_UTILS_H_      /* excludeFromBuildInfo */
#define _SOAR_API_UTILS_H_

#include "soarapi.h"
#include "soarapi_datatypes.h"
#include "soar_ecore_utils.h"

void cb_appendToSoarResultResult(agent * the_agent, soar_callback_data data, soar_call_data call_data);

extern int getInt(const char *string, int *i);
extern bool cfps_removal_test_function(cons * c);

extern void do_print_for_production(production * prod, bool internal, bool print_filename, bool full_prod);

extern void do_print_for_production_name(const char *prod_name, bool internal, bool print_filename, bool full_prod);
extern void do_print_for_wme(wme * w, int depth, bool internal);

extern void execute_go_selection(agent * the_agent,
                                 long go_number,
                                 enum go_type_enum go_type, Symbol * go_slot_attr, goal_stack_level go_slot_level);

extern void print_current_learn_settings(void);
extern void print_multi_attribute_symbols(void);

extern int read_pref_detail_from_string(const char *the_lexeme, bool * print_productions, wme_trace_type * wtt);
extern int read_pattern_component(Symbol ** dest_sym);
extern list *read_pattern_and_get_matching_wmes(void);

extern bool soar_agent_already_defined(char *name);

extern Symbol *get_binding(Symbol * f, list * bindings);
extern void reset_old_binding_point(list ** bindings, list ** current_binding_point);
extern void free_binding_list(list * bindings);
extern void print_binding_list(list * bindings);
extern bool symbols_are_equal_with_bindings(Symbol * s1, Symbol * s2, list ** bindings);
extern bool tests_are_equal_with_bindings(test t1, test test2, list ** bindings);

extern bool conditions_are_equal_with_bindings(condition * c1, condition * c2, list ** bindings);
extern void read_pattern_and_get_matching_productions(list ** current_pf_list, bool show_bindings,
                                                      bool just_chunks, bool no_chunks);
extern bool funcalls_match(list * fc1, list * fc2);
extern bool actions_are_equal_with_bindings(action * a1, action * a2, list ** bindings);
extern void read_rhs_pattern_and_get_matching_productions(list ** current_pf_list, bool show_bindings,
                                                          bool just_chunks, bool no_chunks);
extern bool wme_filter_component_match(Symbol * filterComponent, Symbol * wmeComponent);
extern bool passes_wme_filtering(wme * w, bool isAdd);
extern int parse_filter_type(const char *s, bool * forAdds, bool * forRemoves);

extern void print_current_watch_settings(void);
extern int set_watch_setting(int dest_sysparam_number, const char *param, const char *arg, soarResult * res);
extern int set_watch_level_inc(int level);
extern int set_watch_prod_group_setting(int prodgroup, const char *prodtype, const char *arg, soarResult * res);

extern int parse_run_command(int argc, const char *argv[],
                             long *go_number,
                             enum go_type_enum *go_type,
                             Symbol * *go_slot_attr,
                             goal_stack_level * go_slot_level, bool * self_only, soarResult * res);

extern int parse_go_command(int argc, char *argv[],
                            long *go_number,
                            enum go_type_enum *go_type,
                            Symbol * *go_slot_attr, goal_stack_level * go_slot_level, soarResult * res);
extern int parse_memory_stats(int argc, const char *argv[], soarResult * res);

extern int parse_rete_stats(int argc, const char *argv[], soarResult * res);
extern int parse_system_stats(int argc, const char *argv[], soarResult * res);

extern int printTimingInfo();

#endif
