/* new_soar.h

   This file contains additional functionality that
   was added to Soar in the 8.4 version and is
   needed by some functions in the test interface.

*/

#ifndef NEW_SOAR_H
#define NEW_SOAR_H

#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif
#include "agent.h"


#include "definitions.h"

typedef void * psoar_agent;
typedef signed short goal_stack_level;
typedef union symbol_union Symbol;

void run_all_agents (Kernel* thisKernel, 
		long go_number,
		enum go_type_enum go_type,
		Symbol * go_slot_attr,
		goal_stack_level go_slot_level );
void run_current_agent( long go_number, enum go_type_enum go_type,
			Symbol * go_slot_attr, 
			goal_stack_level go_slot_level);
int parse_filter_type(char *s, Bool *forAdds, Bool *forRemoves);
int read_wme_filter_component(agent* thisAgent, char *s, Symbol **sym);
Bool get_lexeme_from_string (char * the_lexeme);
void soar_alternate_input(agent *ai_agent,
                     char  *ai_string, 
                     char  *ai_suffix, 
                     Bool   ai_exit   );
Symbol *read_identifier_or_context_variable (void);
void do_print_for_identifier (Symbol *id, int depth, Bool internal);
void get_context_var_info (Symbol **dest_goal,
                           Symbol **dest_attr_of_slot,
                           Symbol **dest_current_value);
void get_context_var_info_from_string ( char *str,
					Symbol ** dest_goal,
					Symbol ** dest_attr_of_slot,
					Symbol ** dest_current_value );
void print_augs_of_id (Symbol *id, int depth, Bool internal,
                       int indent, tc_number tc);
wme ** get_augs_of_id( Symbol *id, tc_number tc, int *num_attr );
int compare_attr (const void * e1, const void * e2);
Bool soar_exists_global_callback( SOAR_GLOBAL_CALLBACK_TYPE callback_type );
void soar_invoke_global_callbacks ( soar_callback_agent a, 
            		SOAR_CALLBACK_TYPE callback_type, 
				    soar_call_data call_data);
void soar_default_destroy_agent_procedure (Kernel* thisKernel, psoar_agent delete_agent);
production * name_to_production (char * string_to_test);
int read_id_or_context_var_from_string (char * the_lexeme,
					Symbol * * result_id);
void soar_default_create_agent_procedure (char * agent_name);

#endif
