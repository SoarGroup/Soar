/*
 * =======================================================================
 *  File:  soarCommandUtils.h
 *
 * This file includes the prototypes for the utility routines which are
 * used in Soar.
 *
 * =======================================================================
 *
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */

#include "soar.h"

/* Global vars */
extern Symbol *space_to_remove_from_cfps;

/* Function prototypes */
extern void add_multi_attribute_or_change_value(char *sym, long val);
extern bool cfps_removal_test_function(cons * c);
extern int compare_firing_counts(const void *e1, const void *e2);
extern unsigned long count_rete_tokens_for_production(production * prod);
extern void do_print_for_identifier(Symbol * id, int depth, bool internal);
extern void do_print_for_production(production * prod, bool internal, bool print_filename, bool full_prod);
extern void do_print_for_production_name(char *prod_name, bool internal, bool print_filename, bool full_prod);
extern void do_print_for_wme(wme * w, int depth, bool internal);
extern void excise_all_productions_of_type(byte type);
extern void execute_go_selection(agent * the_agent,
                                 long go_number,
                                 enum go_type_enum go_type, Symbol * go_slot_attr, goal_stack_level go_slot_level);
extern void get_context_var_info(Symbol ** dest_goal, Symbol ** dest_attr_of_slot, Symbol ** dest_current_value);
extern void get_lexeme_from_string(const char *the_lexeme);

extern void install_tcl_soar_cmd(agent *, char *, Tcl_ObjCmdProc *);
extern void install_tcl_soar_cmd_deprecated(agent *, char *, Tcl_CmdProc *);
extern void create_argv_from_objv(int objc, Tcl_Obj * const *, char ***);
extern void free_argv(int objc, char **);

extern bool is_production_name(char *);
extern production *name_to_production(const char *string_to_test);
extern void neatly_print_wme_augmentation_of_id(wme * w, int indentation);
extern int parse_go_command(Tcl_Interp * interp, int argc, char *argv[],
                            long *go_number, enum go_type_enum *go_type,
                            Symbol * *go_slot_attr, goal_stack_level * go_slot_level);
extern int parse_memory_stats(Tcl_Interp * interp, int argc, char *argv[]);
extern int parse_rete_stats(Tcl_Interp * interp, int argc, char *argv[]);
extern int parse_run_command(Tcl_Interp * interp, int argc, char *argv[],
                             long *go_number, enum go_type_enum *go_type,
                             Symbol * *go_slot_attr, goal_stack_level * go_slot_level, bool * self_only);
extern int parse_system_stats(Tcl_Interp * interp, int argc, char *argv[]);
extern void print_augs_of_id(Symbol * id, int depth, bool internal, int indent, tc_number tc);
extern void print_current_learn_settings(void);
extern void print_current_watch_settings(Tcl_Interp * interp);
extern void print_memories(int num_to_print, int to_print[]);
extern void print_multi_attribute_symbols(void);
extern void print_preference_and_source(preference * pref, bool print_source, wme_trace_type wtt);
extern int print_production_firings(Tcl_Interp * interp, int num_requested);
extern int read_id_or_context_var_from_string(const char *the_lexeme, Symbol **);
extern Symbol *read_identifier_or_context_variable(void);
extern int read_attribute_from_string(Symbol * id, const char *the_lexeme, Symbol * *attr);     /* kjh (CUSP-B7) */
extern int read_pref_detail_from_string(char *the_lexeme, bool * print_productions, wme_trace_type * wtt);      /* KJC added to .h */
extern void read_pattern_and_get_matching_productions(list ** current_pf_list,
                                                      bool show_bindings, bool just_chunks, bool no_chunks);
extern list *read_pattern_and_get_matching_wmes(void);
extern int read_pattern_component(Symbol ** dest_sym);
extern void read_rhs_pattern_and_get_matching_productions(list ** pf_list,
                                                          bool show_bindings, bool just_chunks, bool no_chunks);
extern int set_watch_setting(Tcl_Interp * interp, int dest_sysparam_number, char *param, char *arg);
extern int set_watch_prod_group_setting(Tcl_Interp * interp, int prodgroup, char *prodtype, char *arg);
extern int set_watch_level_inc(Tcl_Interp * interp, int level);
/* soar_alternate_input Soar-Bugs #54  TMH */
extern void soar_alternate_input(agent * ai_agent, const char *ai_string, const char *ai_suffix, bool ai_exit);
extern bool soar_agent_already_defined(char *);
extern void soar_callback_to_tcl(soar_callback_agent, soar_callback_data, soar_call_data);
extern bool string_match(const char *, const char *);
extern bool string_match_up_to(const char *, const char *, int);

extern void soar_input_callback_to_tcl(soar_callback_agent the_agent,
                                       soar_callback_data data, soar_call_data call_data);
extern void soar_output_callback_to_tcl(soar_callback_agent the_agent,
                                        soar_callback_data data, soar_call_data call_data);

extern void soar_ask_callback_to_tcl(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data);

/* kjh(CUSP-B2) begin */
int wmes_filter_add(Tcl_Interp * interp, char *idStr, char *attrStr, char *valueStr, bool adds, bool removes);
int wmes_filter_remove(Tcl_Interp * interp, char *idStr, char *attrStr, char *valueStr, bool adds, bool removes);
int wmes_filter_reset(Tcl_Interp * interp, bool adds, bool removes);
int wmes_filter_list(Tcl_Interp * interp, bool adds, bool removes);
bool wme_filter_component_match(Symbol * filterComponent, Symbol * wmeComponent);
bool passes_wme_filtering(wme * w, bool isAdd);
int parse_filter_type(char *s, bool * forAdds, bool * forRemoves);
/* kjh(CUSP-B2) end */

/* it would be better if the B2 routines were consistently
   named:  wme_filter_*  KJC  */
