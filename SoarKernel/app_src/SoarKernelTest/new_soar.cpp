typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/************************************************************************/
/* new_soar.h

   This file contains additional functionality that
   was added to Soar in the 8.4 version and is
   needed by some functions in the test interface.

*/


#include "sk.h"
#include "soarapi.h"
#include "new_soar.h"
#include "definitions.h"

#include "wmem.h"
#include "print.h"
#include "agent.h"
#include "symtab.h"
#include "parser.h"
#include "rhsfun.h"
#include "init_soar.h"
#include "production.h"
#include "gdatastructs.h"
#include "kernel_struct.h"

extern Kernel * SKT_kernel;

void run_all_agents (Kernel* thisKernel, 
		long go_number,
		enum go_type_enum go_type,
		Symbol * go_slot_attr,
		goal_stack_level go_slot_level ) 
{
  Bool agent_to_run = TRUE;
  cons  * c; 
  agent * the_agent; 
  agent * prev_agent;
  long cycle_count = 0;


  
  prev_agent = glbAgent;
  c = SKT_kernel->all_soar_agents;
  if ( !c->rest ) {

    run_current_agent( go_number, go_type, go_slot_attr, go_slot_level );
  }

  else {
  
    for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
      the_agent = (agent *) c->first;
      the_agent->stop_soar = FALSE;
    }
    
    while (TRUE) {
      
      if (cycle_count == go_number) break;
      cycle_count++;
      
      agent_to_run = FALSE;
      for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
	glbAgent = (agent *) c->first;
	if ( !(glbAgent->stop_soar) ) {
	  agent_to_run = TRUE;
	}
      }
      
      if (!agent_to_run) break;
      
      for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
	glbAgent = (agent *) c->first;
	
	
	/*
	 *  I have put a callback in to do just this.  But I haven't
	 *  really figured out what functionality it provides....
	 *  This statement was only defined with the USE_TCL flag
	 *  082099 SW
	 */
	/*
	  Soar_SelectGlobalInterpByInterp(glbAgent->interpreter);
	*/
	
	
	
	if (!current_agent(stop_soar)) {
	  
	  run_current_agent( 1, go_type, go_slot_attr, go_slot_level );
	  
	} /* if */
      } /* for */
      
      
    } /* while */
    
    glbAgent = prev_agent;
  
  
    /* 
     *  There is no analog for this, however, I don't really see
     *  what this does either... (possible bug)
     *  This block was only defined in with USE_TCL flag
     *  082099 SW
     *  SWBUG possible bug for when using Tcl
     */
    /*  Soar_SelectGlobalInterpByInterp(glbAgent->interpreter); */
  }


}

void run_current_agent( long go_number, enum go_type_enum go_type,
			Symbol * go_slot_attr, 
			goal_stack_level go_slot_level) {

  soar_invoke_callbacks(glbAgent, glbAgent, 
			BEFORE_SCHEDULE_CYCLE_CALLBACK,
			(soar_call_data) TRUE);
  

    
    switch (go_type) {
    case GO_PHASE:
      run_for_n_phases (glbAgent, go_number);
      break;
    case GO_ELABORATION:
      run_for_n_elaboration_cycles (glbAgent, go_number);
      break;
    case GO_DECISION:
      run_for_n_decision_cycles (glbAgent, go_number);
      break;
    case GO_STATE:
      run_for_n_selections_of_slot (glbAgent, go_number, glbAgent->state_symbol);
      break;
    case GO_OPERATOR:
      run_for_n_selections_of_slot (glbAgent, go_number, glbAgent->operator_symbol);
      break;
    case GO_SLOT:
      run_for_n_selections_of_slot_at_level (glbAgent, go_number, go_slot_attr,
					     go_slot_level);
      break;
    case GO_OUTPUT:
      run_for_n_modifications_of_output (glbAgent, go_number);
      break;
    }

  
    soar_invoke_callbacks(glbAgent, glbAgent, 
			  AFTER_SCHEDULE_CYCLE_CALLBACK,
			  (soar_call_data) TRUE);
    
}

int parse_filter_type(char *s, Bool *forAdds, Bool *forRemoves) {
  if        (!strcmp(s,"-adds")) {
    *forAdds = TRUE;
    *forRemoves = FALSE;
    return SOAR_OK;
  } else if (!strcmp(s,"-removes")) {
    *forAdds = FALSE;
    *forRemoves = TRUE;
    return SOAR_OK;
  } else if (!strcmp(s,"-both")) {
    *forAdds = TRUE;
    *forRemoves = TRUE;
    return SOAR_OK;
  }
  return SOAR_ERROR;
}

int read_wme_filter_component(agent* thisAgent, char *s, Symbol **sym) {
  get_lexeme_from_string(s);
  if(current_agent(lexeme).type == IDENTIFIER_LEXEME) {

    if ((*sym = find_identifier(thisAgent, current_agent(lexeme).id_letter, 
			      current_agent(lexeme).id_number)) == NIL) {
      return -1; /* Identifier does not exist */
    }
  } else
    *sym = make_symbol_for_current_lexeme(thisAgent);
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * get_lexeme_from_string --
 *
 *	A hack to get the Soar lexer to take a string
 *      as a lexeme and setup the agent lexeme structure.  It
 *      is assumed that the lexeme is composed of Soar
 *      "constituent" characters -- i.e., does not contain any
 *      Soar special characters.  
 *
 *      See lexer.c for more information.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	String copied to lexeme structure,  string length
 *      computed, and lexeme type determined.
 *      Overwrites previous lexeme contents. 
 *
 *----------------------------------------------------------------------
 */

Bool get_lexeme_from_string (char * the_lexeme)
{
	int i;
	char * c;
	Bool sym_constant_start_found = FALSE;
	Bool sym_constant_end_found = FALSE;
	
	for (c = the_lexeme, i = 0; *c; c++, i++)
    {
		if (*c == '|')
		{
			if (!sym_constant_start_found)
			{
				i--;
				sym_constant_start_found = TRUE;
			}
			else
			{
				i--;
				sym_constant_end_found = TRUE;
			}
		}
		else
		{
			current_agent(lexeme).string[i] = *c;
		}
    }
	
	current_agent(lexeme).string[i] = '\0'; /* Null terminate lexeme string */
	
	current_agent(lexeme).length = i;
	
	if (sym_constant_end_found)
    {
		current_agent(lexeme).type = SYM_CONSTANT_LEXEME;
    }
	else 
    {
		if (!determine_type_of_constituent_string(glbAgent))
			return FALSE;
	}

	return TRUE;
}

/* Soar-Bugs #54 TMH */
/*
 *----------------------------------------------------------------------
 *
 * soar_alternate_input --
 *
 *	This procedure initializes alternate input buffers for a
 *      soar agent.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	The soar agents alternate input values are updated and its
 *      current character is reset to a whitespace value.
 *
 *----------------------------------------------------------------------
 */

void soar_alternate_input(agent *ai_agent,
                     char  *ai_string, 
                     char  *ai_suffix, 
                     Bool   ai_exit   )
{
  ai_agent->alternate_input_string = ai_string;
  ai_agent->alternate_input_suffix = ai_suffix;
  ai_agent->current_char = ' ';
  ai_agent->alternate_input_exit = ai_exit;
  return;
}

/*
 *  This function should be replaced by the one above and added to the
 *  Soar kernel. 
 */

Symbol *read_identifier_or_context_variable (void) {
  Symbol *id;
  Symbol *g, *attr, *value;

  if (current_agent(lexeme).type==IDENTIFIER_LEXEME) {
    id = find_identifier (glbAgent, current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
    if (!id) {
      print (glbAgent, "There is no identifier %c%lu.\n", current_agent(lexeme).id_letter,
             current_agent(lexeme).id_number);
      print_location_of_most_recent_lexeme(glbAgent);
      return NIL;
    }
    return id;
  }
  if (current_agent(lexeme).type==VARIABLE_LEXEME) {
    get_context_var_info (&g, &attr, &value);
    if (!attr) {
      print (glbAgent, "Expected identifier (or context variable)\n");
      print_location_of_most_recent_lexeme(glbAgent);
      return NIL;
    }
    if (!value) {
      print (glbAgent, "There is no current %s.\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme(glbAgent);
      return NIL;
    }
    if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
      print (glbAgent, "The current %s ", current_agent(lexeme).string);
      print_with_symbols (glbAgent, "(%y) is not an identifier.\n", value);
      print_location_of_most_recent_lexeme(glbAgent);
      return NIL;
    }
    return value;
  }
  print (glbAgent, "Expected identifier (or context variable)\n");
  print_location_of_most_recent_lexeme(glbAgent);
  return NIL;
}

void do_print_for_identifier (Symbol *id, int depth, Bool internal) {
  tc_number tc;

  tc = get_new_tc_number(glbAgent);
  print_augs_of_id (id, depth, internal, 0, tc);
}

void get_context_var_info (Symbol **dest_goal,
                           Symbol **dest_attr_of_slot,
                           Symbol **dest_current_value) {
 
  get_context_var_info_from_string ( current_agent(lexeme).string,
				     dest_goal,
				     dest_attr_of_slot,
				     dest_current_value );
}

void get_context_var_info_from_string ( char *str,
					Symbol ** dest_goal,
					Symbol ** dest_attr_of_slot,
					Symbol ** dest_current_value ) {

  Symbol *v, *g;
  int levels_up;
  wme *w;

  v = find_variable ( glbAgent, str );
  if (v==current_agent(s_context_variable)) {
    levels_up = 0;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(o_context_variable)) {
    levels_up = 0;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(ss_context_variable)) {
    levels_up = 1;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(so_context_variable)) {
    levels_up = 1;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(sss_context_variable)) {
    levels_up = 2;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(sso_context_variable)) {
    levels_up = 2;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else if (v==current_agent(ts_context_variable)) {
    levels_up = current_agent(top_goal) ? current_agent(bottom_goal)->id.level-current_agent(top_goal)->id.level : 0;
    *dest_attr_of_slot = current_agent(state_symbol);
  } else if (v==current_agent(to_context_variable)) {
    levels_up = current_agent(top_goal) ? current_agent(bottom_goal)->id.level-current_agent(top_goal)->id.level : 0;
    *dest_attr_of_slot = current_agent(operator_symbol);
  } else {
    *dest_goal = NIL;
    *dest_attr_of_slot = NIL;
    *dest_current_value = NIL;
    return;
  }

  g = current_agent(bottom_goal);
  while (g && levels_up) {
    g = g->id.higher_goal;
    levels_up--;
  }
  *dest_goal = g;

  if (!g) {
    *dest_current_value = NIL;
    return;
  }
  
   if (*dest_attr_of_slot==current_agent(state_symbol)) {
     *dest_current_value = g;
   } else {
      w = g->id.operator_slot->wmes;
    *dest_current_value = w ? w->value : NIL;
  }
}

void print_augs_of_id (Symbol *id, int depth, Bool internal,
                       int indent, tc_number tc) {
 slot *s;
  wme *w;

  wme **list;    /* array of WME pointers, AGR 652 */
  int num_attr;  /* number of attributes, AGR 652 */
  int attr;      /* attribute index, AGR 652 */
 
  
  list = get_augs_of_id( id, tc, &num_attr );
  if ( !list ) return;


  /* --- finally, print the sorted wmes and deallocate the array --- */
  if (internal) {
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      print_spaces (glbAgent, indent);
      print_wme (glbAgent, w);
    }
  } else {
    print_spaces (glbAgent, indent);
    print_with_symbols (glbAgent, "(%y", id);
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      neatly_print_wme_augmentation_of_id (w, indent);
    }
    print (glbAgent, ")\n");
  }
  free_memory(glbAgent, list, MISCELLANEOUS_MEM_USAGE);
/* AGR 652 end */

  /* --- if depth<=1, we're done --- */
  if (depth<=1) return;

  /* --- call this routine recursively --- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next) {
    print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
    print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
  }
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next) {
    print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
    print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
  }
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next) {
      print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
      print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
    }
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) {
      print_augs_of_id (w->attr, depth-1, internal, indent+2, tc);
      print_augs_of_id (w->value, depth-1, internal, indent+2, tc);
    }
  }
}

wme ** get_augs_of_id( Symbol *id, tc_number tc, int *num_attr ) {
 slot *s;
  wme *w;

  wme **list;    /* array of WME pointers, AGR 652 */
  int attr;      /* attribute index, AGR 652 */
  int n;

/* AGR 652  The plan is to go through the list of WMEs and find out how
   many there are.  Then we malloc an array of that many pointers.
   Then we go through the list again and copy all the pointers to that array.
   Then we qsort the array and print it out.  94.12.13 */

  if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return NULL;
  if (id->id.tc_num==tc) return NULL;
  id->id.tc_num = tc;

  /* --- first, count all direct augmentations of this id --- */
  n= 0;
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next) n++;
  for (w=id->id.input_wmes; w!=NIL; w=w->next) n++;
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next) n++;
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) n++;
  }

  /* --- next, construct the array of wme pointers and sort them --- */
  list = static_cast<wme **>(allocate_memory(glbAgent, n*sizeof(wme *), MISCELLANEOUS_MEM_USAGE));
  attr = 0;
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
    list[attr++] = w;
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    list[attr++] = w;
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next)
      list[attr++] = w;
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
      list[attr++] = w;
  }
  qsort (list, n, sizeof (wme *), compare_attr); 

  *num_attr = n;
  return list;

}

/* This should probably be in the Soar kernel interface. */
void neatly_print_wme_augmentation_of_id (wme *w, int indentation) {
  char buf[10000], *ch;

  strcpy (buf, " ^");
  ch = buf;
  while (*ch) ch++;
  symbol_to_string (glbAgent, w->attr, TRUE, ch); while (*ch) ch++;
  *(ch++) = ' ';
  symbol_to_string (glbAgent, w->value, TRUE, ch); while (*ch) ch++;
  if (w->acceptable) { strcpy (ch, " +"); while (*ch) ch++; }

  if (get_printer_output_column(glbAgent) + (ch - buf) >= 80) {
    print (glbAgent, "\n");
    print_spaces (glbAgent, indentation+6);
  }
  print_string (glbAgent, buf);
}

/* The header for compare_attr needs to be defined in this way because
   otherwise we get compiler warnings at the qsort lines about the 4th
   argument being an incompatible pointer type.  */

int compare_attr (const void * e1, const void * e2)
{
  wme **p1, **p2;

  char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

  p1 = (wme **) e1;
  p2 = (wme **) e2;

  symbol_to_string (glbAgent, (*p1)->attr, TRUE, s1);
  symbol_to_string (glbAgent, (*p2)->attr, TRUE, s2);

  return (strcmp (s1, s2));
}

Bool soar_exists_global_callback( SOAR_GLOBAL_CALLBACK_TYPE callback_type )
{
  list * cb_cons;

  cb_cons = soar_global_callbacks[callback_type];

  if (cb_cons == NULL)
    {
      return FALSE;
    }

  return TRUE;
}

void soar_invoke_global_callbacks ( soar_callback_agent a, 
            			    SOAR_CALLBACK_TYPE callback_type, 
							soar_call_data call_data)
{
  cons * c;

  /* So far, there's no need to mess with the timers, becuase
   * we have only implemented function which get called at 
   * agent creation and destruction
   */ 

  for (c = soar_global_callbacks[callback_type];
       c != NIL;
       c = c->rest)
    {
      soar_callback * cb;

      cb = (soar_callback *) c->first;
      cb->function( a, cb->data, call_data);
    }


}

void soar_default_destroy_agent_procedure (Kernel* thisKernel, psoar_agent delete_agent)
{
   cons  * c;
   cons  * prev = NULL;   /* Initialized to placate gcc -Wall */
   agent * the_agent;
   Bool  already_deleted;
   
   remove_built_in_rhs_functions(glbAgent);
   
   already_deleted = TRUE;
   /* Splice agent structure out of global list of agents. */
   for (c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {  
      the_agent = (agent *) c->first;
      if (the_agent == delete_agent) {
         if (c == thisKernel->all_soar_agents) {
            thisKernel->all_soar_agents = c->rest;
         } else {
            prev->rest = c->rest;
         }
         already_deleted = FALSE;
         break;
      }
      prev = c;
   }
   
   if ( already_deleted ) {
      char msg[128];
      sprintf( msg, "Tried to delete invalid agent (%p).\n", delete_agent );
      abort_with_fatal_error( glbAgent, msg );
   }
   
   /* If we're deleting the current agent we need a new current agent! */
   if ( delete_agent == glbAgent )
      glbAgent = reinterpret_cast<agent *>(c->rest); ////JNW:  HUH?   
   
   /* Free agent id */
   ////soar_agent_ids[((agent *)delete_agent)->id] = TOUCHED;

   /* Free structures stored in agent structure */
   free(((agent *)delete_agent)->name);
   
   /* KNOWN MEMORY LEAK! Need to track down and free ALL structures */
   /* pointed to be fields in the agent structure.                  */
   
   /* Free soar agent structure */
   free((void *) delete_agent);
   
   thisKernel->agent_count--;
   
}

/*
 *----------------------------------------------------------------------
 *
 * read_id_or_context_var_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for an identifier or context variable.
 * 
 *      Many interface routines take identifiers as arguments.  
 *      These ids can be given as normal ids, or as special variables 
 *      such as <s> for the current state, etc.  This routine reads 
 *      (without consuming it) an identifier or context variable, 
 *      and returns a pointer (Symbol *) to the id.  (In the case of 
 *      context variables, the instantiated variable is returned.  If 
 *      any error occurs (e.g., no such id, no instantiation of the 
 *      variable), an error message is printed and NIL is returned.
 *
 * Results:
 *	Pointer to a symbol for the variable or NIL.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_id_or_context_var_from_string (char * the_lexeme,
					Symbol * * result_id) 
{
  Symbol *id;
  Symbol *g, *attr, *value;

  get_lexeme_from_string(the_lexeme);

  if (current_agent(lexeme).type == IDENTIFIER_LEXEME) 
    {
      id = find_identifier (glbAgent, current_agent(lexeme).id_letter, 
			    current_agent(lexeme).id_number);
      if (!id) 
	{
	  return SOAR_ERROR;
	}
      else
	{
	  *result_id = id;
	  return SOAR_OK;
	}
    }

  if (current_agent(lexeme).type==VARIABLE_LEXEME) 
    {
      get_context_var_info (&g, &attr, &value);

      if ((!attr) || (!value))
	{
	  return SOAR_ERROR;
	}

      if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
	{
	  return SOAR_ERROR;
	}

      *result_id = value;
      return SOAR_OK;
    }

  return SOAR_ERROR;
}
