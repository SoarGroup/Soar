#ifndef _SOAR_CORE_UTILS_H_     /* ExcludeFromBuildInfo */
#define _SOAR_CORE_UTILS_H_

#include "soarkernel.h"
#include "soar_core_api.h"

#ifdef USE_CAPTURE_REPLAY
#include "soar_ecore_utils.h"
#endif

extern Symbol *read_identifier_or_context_variable(void);

extern int read_id_or_context_var_from_string(const char *the_lexeme, Symbol * *result_id);

extern production *name_to_production(const char *string_to_test);

extern void do_print_for_identifier(Symbol * id, int depth, bool internal);

extern void get_lexeme_from_string(const char *the_lexeme);

extern void get_context_var_info(Symbol ** dest_goal, Symbol ** dest_attr_of_slot, Symbol ** dest_current_value);

extern void get_context_var_info_from_string(char *str, Symbol ** dest_goal,
                                             Symbol ** dest_attr_of_slot, Symbol ** dest_current_value);

extern void print_augs_of_id(Symbol * id, int depth, bool internal, int indent, tc_number tc);

extern int compare_attr(const void *e1, const void *e2);

extern void neatly_print_wme_augmentation_of_id(wme * w, int indentation);

extern bool string_match(const char *string1, const char *string2);
extern bool string_match_up_to(const char *string1, const char *string2, int positions);

extern void excise_all_productions_of_type(byte type);

extern void soar_default_create_agent_procedure(const char *agent_name);
extern void soar_default_destroy_agent_procedure(psoar_agent delete_agent);

#endif
