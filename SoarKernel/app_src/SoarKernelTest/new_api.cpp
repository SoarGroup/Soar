typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/************************************************************************/
/*

  This file contains various api functions that mimic the behavior of Soar
  commands. Each of these is a modified version of a function from the Soar 
  8.4 interface that ends with the letters "Cmd" (for "Command"). For instance,
  "MemoriesCmd" becomes soar_Memories. In addition, the following changes have
  been made to these functions to make them independent of TCL:

    - Their parameters have been reduced to argv and argc. (The original
	  functions received a pointer to the TCL interpreter and a ClientData
	  struct in addition to these.)
    
	- Of course, all references to the TCL interpreter and the ClientData
	  struct have been removed.

	- All output to TCL has been redirected to stdout using printf.
      These are for testing purposes only.

    - Some commands read numerical arguments from the parser. Any TCL
	  functions used toward this end have been removed in favor of the atoi
	  function.
*/

#include "new_api.h"
#include "new_soar.h"
#include "soarapi.h"
#include "definitions.h"
#include "utilities.h"

#include "rete.h"
#include "wmem.h"
#include "print.h"
#include "trace.h"
#include "tempmem.h"
#include "gsysparam.h"
#include "gdatastructs.h"
#include "production.h"

#include <string.h>
#include <ctype.h>

extern Kernel * SKT_kernel;

int soar_Memories(char ** argv, int argc)
{
	int i;
	int num;
	int num_items = -1;
	int mems_to_print[NUM_PRODUCTION_TYPES];
	Bool set_mems = FALSE;
	production * prod;
	
	for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
		mems_to_print[i] = FALSE;
	
	for (i = 1; i < argc; i++)
    {
		if (string_match_up_to(argv[i], "-chunks", 2))
		{
			mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
			set_mems = TRUE;
		}
		else if (string_match_up_to(argv[i], "-user", 2))
		{
			mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
			set_mems = TRUE;
		}
		else if (string_match_up_to(argv[i], "-defaults", 2))
		{
			mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
			set_mems = TRUE;
		}
		else if (string_match_up_to(argv[i], "-justifications", 2))
		{
			mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
			set_mems = TRUE;
		}
		else if ((prod = name_to_production(argv[i])))
		{
			print(glbAgent, "\n Memory use for %s: %ld\n\n", argv[i], 
				count_rete_tokens_for_production(glbAgent, prod));
			set_mems = TRUE;
			return SOAR_OK;
		}
		else if (isdigit(argv[i][0]))
		{
			num = atoi(argv[i]);
			if (num <= 0)
			{
				printf("Count argument to memories must be a positive integer, not: %s", argv[i]);
				return FALSE;
				break;
			}
			else
			{
				num_items = num;
			}
		}
		else
		{
			printf("\nUnrecognized argument to memories: %s\n", argv[i]);
			return FALSE;
		}
    }
	
	if (!set_mems) {
		mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
		mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
		mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
		mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
	}
	/*printf("chunkflag = %d\nuserflag = %d\ndefflag = %d\njustflag = %d\n",
	*     mems_to_print[CHUNK_PRODUCTION_TYPE],
	*     mems_to_print[USER_PRODUCTION_TYPE],
	*	 mems_to_print[DEFAULT_PRODUCTION_TYPE],
	*	 mems_to_print[JUSTIFICATION_PRODUCTION_TYPE]);
	*/ 
	print_memories(num_items, mems_to_print);
	
	return SOAR_OK;
}

int soar_Preferences(char ** argv, int argc)
{
	static char * too_many_args = "Too many arguments.\nUsage: preferences [id] [attribute] [detail]";
	static char * wrong_args = "Usage: preferences [id] [attribute] [detail]";
	
	Symbol *id, *id_tmp, *attr, *attr_tmp;
	Bool print_productions;
	wme_trace_type wtt;
	slot *s;
	preference *p;
	int i;
	
	/* Establish argument defaults: */
	id                = current_agent(bottom_goal);
	id_tmp            = NIL;
	attr              = current_agent(operator_symbol);
	attr_tmp          = NIL;
	print_productions = FALSE;
	wtt               = NONE_WME_TRACE;
	
	/* Parse provided arguments: */
	switch (argc) {
	case 1:
		/* No arguments; defaults suffice. */
		break;
    case 2:
		/* One argument; replace one of the defaults: */
		if (  (read_id_or_context_var_from_string(argv[1], &id_tmp)
			== FALSE)
			&& (read_attribute_from_string(id, argv[1], &attr_tmp)
			== FALSE)
			&& (read_pref_detail_from_string(argv[1], &print_productions, &wtt)
			== FALSE))  {             
			printf("\n%s\n", wrong_args);
			return FALSE;
		}
		break;
    case 3:
		/* Two arguments; replace two of the defaults: */
		if (read_id_or_context_var_from_string(argv[1], &id_tmp) == FALSE) {
			id_tmp = id;
			if (read_attribute_from_string(id,argv[1], &attr_tmp) == FALSE) {
				printf("\n%s\n", wrong_args);
				return FALSE;
			}
		}
		if (  (read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
			== FALSE)
			&& (read_pref_detail_from_string(argv[2], &print_productions, &wtt)
			== FALSE))  {             
			printf("\n%s\n", wrong_args);
			return FALSE;
		}
		break;
    case 4:
		/* Three arguments; replace (all) three of the defaults: */
		if (  (read_id_or_context_var_from_string(argv[1], &id_tmp)
			== FALSE)
			|| (read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
			== FALSE)
			|| (read_pref_detail_from_string(argv[3], &print_productions, &wtt)
			== FALSE))  {             
			printf("\n%s\n", wrong_args);
			return FALSE;
		}
		break;
    default:
		/* Too many arguments; complain: */
		printf("\n%s\n", too_many_args);
		return FALSE;
		break;
	}
	
	/* kjh (CUSP-B7) end */
	
	/* --- print the preferences --- */
	if (id_tmp != NIL)
		id = id_tmp;
	if (attr_tmp != NIL)
		attr = attr_tmp;
	
	if (id == NIL)
		return(SOAR_OK);
	
	s = find_slot (id, attr);
	if (!s)
    {
		printf("There are no preferences for %s ^%s.", argv[1], argv[2]);
		return FALSE;
    }
	
	print_with_symbols (glbAgent, "Preferences for %y ^%y:\n", id, attr);
	
	for (i = 0; i < NUM_PREFERENCE_TYPES; i++)
    {
		if (s->preferences[i])
        {
			print (glbAgent, "\n%ss:\n", preference_name[i]);
			for (p = s->preferences[i]; p; p = p->next)
            {
				print_preference_and_source (p, print_productions, wtt);
            }
        }
    }
	
	return SOAR_OK;
}

int soar_AttributePreferencesMode(char ** argv, int argc)
{
	if (argc > 2)
    {
		printf("Too many arguments.\nUsage: attribute-preferences-mode 0|1|2\n");
		return FALSE;
	}
	
	if (argc == 2) {
		if (current_agent(operand2_mode) && (strcmp(argv[1],"2"))) {
			/* we're in Soar8 mode, but tried setting mode != 2 */
			printf("\nUnallowed argument to attribute-preferences-mode: %s.\nIn soar8 mode, attribute-preferences-mode is obsolete;\nthe code automatically operates as if attr-pref-mode == 2.\n",
				argv[1]);
			return FALSE;
		}
		if (! strcmp(argv[1], "0")) {
			current_agent(attribute_preferences_mode) = 0;
		} else if (! strcmp(argv[1], "1")) {
			current_agent(attribute_preferences_mode) = 1;
		} else if (! strcmp(argv[1], "2")) {
			current_agent(attribute_preferences_mode) = 2;
		} else {
			printf("\nUnrecognized argument to %s: %s.  Integer 0, 1, or 2 expected.\n", argv[0], argv[1]);
			return FALSE;
		}
	}
	
	printf("\n%d\n", current_agent(attribute_preferences_mode));
	return SOAR_OK;
}

int soar_MultiAttributes(char ** argv, int argc)
{
	int num;
	
	if (argc == 1)
    {
		print_multi_attribute_symbols();
		return TRUE;
    }
	
	if (argc > 3)
    {
		printf("\nToo many arguments, should be: multi-attribute [symbol] [value]\n");
		return FALSE;
    }
	
	get_lexeme_from_string(argv[1]);
	
	if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME)
    {
		printf("\nExpected symbolic constant for symbol but got: %s\nUsage: multi-attributes [symbol] [value]\n",	argv[1]);      
		return FALSE;
    }
	
	if (argc == 3)
    {
		if (!isdigit(argv[2][0]))
		{
			printf("\nNon-integer given for attribute count: %s\nUsage: multi-attributes [symbol] [value]\n", argv[2]);      
			return FALSE;
		}
		else
		{
			num = atoi(argv[2]);
			if (num > 1)
			{
				add_multi_attribute_or_change_value(argv[1], num);
			} 
			else 
			{
				printf("Integer must be greater than 1 but was %s", argv[2]);
				return FALSE;
			}
		}
    }
	else
    {
		add_multi_attribute_or_change_value(argv[1], 10);
    }
	
	return TRUE;
}

int soar_Matches(char ** argv, int argc)
{
  production * prod = NULL;
  production * prod_named;
  wme_trace_type wtt = NONE_WME_TRACE;
  ms_trace_type  mst = MS_ASSERT_RETRACT;
  int curr_arg = 1;

  while (curr_arg < argc)
    {
      if (string_match_up_to("-assertions", argv[curr_arg], 2))
	{
	  mst = MS_ASSERT;
	}
      else if (string_match_up_to("-retractions", argv[curr_arg], 2))
	{
	  mst = MS_RETRACT;
	}
      else if (   string_match_up_to("-names", argv[curr_arg], 2)
	       || string_match_up_to("-count", argv[curr_arg], 2)
	       || string_match("0", argv[curr_arg]))
	{
	  wtt = NONE_WME_TRACE;
	}
      else if (   string_match_up_to("-timetags", argv[curr_arg], 2)
	       || string_match("1", argv[curr_arg]))
	{
	  wtt = TIMETAG_WME_TRACE;
	}
      else if (   string_match_up_to("-wmes", argv[curr_arg], 2)
	       || string_match("2", argv[curr_arg]))
	{
	  wtt = FULL_WME_TRACE;
	}
      else if ((prod_named = name_to_production(argv[curr_arg])))
	{
	  prod = prod_named;
	}
      else
	{
	  printf("\nUnrecognized option to matches command: %s\n", argv[curr_arg]);
	  return FALSE;
	}

      curr_arg++;
    }

  if (prod != NULL) 
    {
      print_partial_match_information (glbAgent, prod->p_node, wtt);
    }
  else 
    {
      print_match_set (glbAgent, wtt, mst);
    }

  return TRUE;
}

int soar_Warnings(char ** argv, int argc)
{
  if (argc == 1)
    {
      printf("\n%s\n", current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM] ? "on" : "off");
      return TRUE;
    }

  if (argc > 2)
    {
      printf("\nToo many arguments, should be: warnings [-on | -off]\n");
      return FALSE;
    }

  if (string_match_up_to(argv[1], "-on", 3))
    {
      set_sysparam (glbAgent, PRINT_WARNINGS_SYSPARAM, TRUE);
    }
  else if (string_match_up_to(argv[1], "-off", 3))
    {
    set_sysparam (glbAgent, PRINT_WARNINGS_SYSPARAM, FALSE);
    }
  else
    {
      printf("\nUnrecognized option to warnings command: %s\n", argv[1]);
      return FALSE;
    }

  return TRUE;
}

int soar_Learn(char ** argv, int argc)
{
	if (argc == 1)
    {
		print_current_learn_settings();
		return TRUE;
    }
	
	int i;
	
	for (i = 1; i < argc; i++)
	{
		if (string_match("-on", argv[i]))
		{
			set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
			set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
			set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
		}
		else if (string_match_up_to("-only", argv[i], 3))
		{
			set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
			set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, TRUE);
			set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
		}
		else if (string_match_up_to("-except", argv[i], 2))
		{
			set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
			set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
			set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, TRUE);
		}
		else if (string_match_up_to("-off", argv[i], 3))
		{
			set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, FALSE); 
			set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
			set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
		}
		else if (string_match_up_to("-all-levels", argv[i], 2)) 
		{
			set_sysparam (glbAgent, LEARNING_ALL_GOALS_SYSPARAM, TRUE);
		}
		else if (string_match_up_to("-bottom-up", argv[i], 2))
		{
			set_sysparam (glbAgent, LEARNING_ALL_GOALS_SYSPARAM, FALSE);
		}
		else if (string_match_up_to("-list", argv[i], 2))
		{
			cons * c;
			char buff[1024];
			
			print_current_learn_settings();
			printf("force-learn states (when learn = -only):\n", (char *) NULL);
			for (c = current_agent(chunky_problem_spaces); 
			c != NIL;
			c = c->rest)
			{
				symbol_to_string(glbAgent, (Symbol *) (c->first), TRUE, buff, 1024);
				printf("\n%s\n", buff);
			}
			printf("\ndont-learn states (when learn = -except):\n", (char *) NULL);
			for (c = current_agent(chunk_free_problem_spaces); 
			c != NIL; 
			c = c->rest)
			{
				symbol_to_string(glbAgent, (Symbol *) (c->first), TRUE, buff, 1024);
				printf("\n%s\n", buff);
			}
			return TRUE;
		}
		else
		{
			printf("\nUnrecognized argument to learn command: %s\n", argv[i]);
			return FALSE;
		}
	}
	
	return TRUE;
}

int soar_Echo(char ** argv, int argc)
{
	int i;
	Bool newline = TRUE;
	
	for (i = 1; i < argc; i++)
    {
		if (string_match_up_to("-nonewline", argv[i], 2))
		{
			newline = FALSE;
		}
		else 
		{
			//Soar_LogAndPrint((agent *) clientData, " ");
			Soar_LogAndPrint(glbAgent, glbAgent, argv[i]);
			if ((i + 1) < argc) 
			{
				//Soar_LogAndPrint((agent *) clientData, " ");
				Soar_LogAndPrint(glbAgent, glbAgent, " ");
			}
		}
    }
	
	if (newline)
    {
		//Soar_LogAndPrint((agent *) clientData, "\n");
		Soar_LogAndPrint(glbAgent, glbAgent, "\n");
    }
	
	return TRUE;
}

int soar_ProductionFind(char ** argv, int argc)
{  
	int i;
	
	list *current_pf_list = NIL;
	
	Bool lhs = TRUE;
	Bool rhs = FALSE;
	Bool show_bindings = FALSE;
	Bool just_chunks = FALSE;
	Bool no_chunks = FALSE;
	Bool clause_found = FALSE;
	
	if (argc == 1)
    {
		printf("\nNo arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}\n");
		return FALSE;
    }
	
	/* the args parsing should really be done in a while loop, where we
	have better control over the loop vars, which we'll use later.
	But using clause_found will do for now...  */
	
	for (i = 1; i < argc; i++)
    {
		if (string_match_up_to(argv[i], "-lhs", 2))
		{
			lhs = TRUE;
		}
		else if (string_match_up_to(argv[i], "-rhs", 2))
		{
			rhs = TRUE;
			lhs = FALSE;
		}
		else if (string_match_up_to(argv[i], "-show-bindings", 2))
		{
			show_bindings = TRUE;
		}
		else if (string_match_up_to(argv[i], "-chunks", 2))
		{
			just_chunks = TRUE ;
		}
		else if (string_match_up_to(argv[i], "-nochunks", 2))
		{
			no_chunks = TRUE ;
		}
		else if (strchr(argv[i], '(') != 0)
			/* strchr allows for leading whitespace */
		{
			/* it's the clause */
			clause_found = TRUE ;
			break;
		}
		else
		{
			printf("\nUnrecognized argument to %s command: %s\n", argv[0], argv[i]);
			return FALSE;
		}
    }
	
	if (!clause_found)
    {
		printf("\nNo clause found.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}\n");
		return FALSE;
    }
	
	if ((*argv[i] == '-') || (strchr(argv[i],'(') != 0))
    {
		if (argc > i + 1) 
		{
			printf("\nToo many arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}\n");
			return FALSE;
		}
		if ((*argv[i] == '-') && (argc < i + 1))
		{
			printf("\nToo few arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}\n");
			return FALSE;
		}
		if (lhs)
		{
			/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
			/* Soar-Bugs #54 TMH */
			/*  soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE); */
			//((agent *)clientData)->alternate_input_string = argv[i];
			//((agent *)clientData)->alternate_input_suffix = ") ";
			(glbAgent)->alternate_input_string = argv[i];
			(glbAgent)->alternate_input_suffix = ") ";
			
			get_lexeme(glbAgent);
			read_pattern_and_get_matching_productions (&current_pf_list,
				show_bindings,
				just_chunks, no_chunks);
			/* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE);  */
			current_agent(current_char) = ' ';
		}
		if (rhs)
		{
			/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
			/* Soar-Bugs #54 TMH */
			/* soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE);  */
			//((agent *)clientData)->alternate_input_string = argv[i];
			//((agent *)clientData)->alternate_input_suffix = ") ";
			(glbAgent)->alternate_input_string = argv[i];
			(glbAgent)->alternate_input_suffix = ") ";
			
			get_lexeme(glbAgent);
			read_rhs_pattern_and_get_matching_productions (&current_pf_list,
				show_bindings,
				just_chunks, 
				no_chunks);
			/* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE); */
			current_agent(current_char) = ' ';
		}
		if (current_pf_list == NIL) 
		{
			print(glbAgent, "No matches.\n");
		}
		
		free_list(glbAgent, current_pf_list);
    }
	else
    {
		printf("Unknown argument to %s command: %s", argv[0], argv[i]);
		return FALSE;
    }
	
	return TRUE;
}

int soar_FiringCounts(char ** argv, int argc)
{
	int num_requested;
	
	if (argc > 1)
    {
		if (isdigit(argv[1][0]))
		{
			num_requested = atoi(argv[1]);
		}
		else
		{
			int i;
			
			for(i = 1; i < argc; i++)
			{
				production * p;
				
				p = name_to_production(argv[i]);
				if (p)
				{
					print (glbAgent, "%6lu:  %s\n", p->firing_count, argv[i]);
				}
				else
				{
					printf("\nNo production named %s\n", argv[i]);
					return FALSE;
				}
			}
			return TRUE;
		}
    }
	
	print_production_firings(((argc <= 1) ? 20 : num_requested));
	
	return TRUE;
}

int soar_FormatWatch(char ** argv, int argc)
{
  static char * too_few_args  = "Too few arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";
  static char * too_many_args = "Too many arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";

  Bool stack_trace;
  int type_restriction;
  Symbol *name_restriction = NIL;
  Bool remove;
  int format_arg = 0;  /* Initialized to placate gcc -Wall */

  if (argc == 1)
    {
      printf("\n%s\n", too_few_args);
      return FALSE;
    }

  /* --- set stack_trace depending on which option was given --- */  
  if (string_match("-stack", argv[1]))
    {
      stack_trace = TRUE;
    }
  else if (string_match("-object", argv[1]))
    {
      stack_trace = FALSE;
    }
  else
    {
      printf("\nUnrecognized option to format-watch : %s\n",
	      argv[1]);
      return FALSE;
    }

  /* --- if no further args, print all trace formats of that type --- */
  
  if (argc == 2)
    {
      //print_all_trace_formats_tcl (stack_trace);
      return TRUE;
    }

  /* --- next argument must be either -add or -remove --- */
  if (string_match("-add", argv[2]))
    {
      remove = FALSE;
    }
  else if (string_match("-remove", argv[2]))
    {
      remove = TRUE;
    }
  else 
    {
      printf("\nUnrecognized option to format-watch %s: %s\n", argv[1], argv[2]);
      return FALSE;
    }

  if (argc == 3)
    {
      printf("\n%s\n", too_few_args);
      return FALSE;
    }

  /* --- read context item argument: s, o, or '*' --- */
  
  if (string_match("s", argv[3]))
      {
	type_restriction = FOR_STATES_TF;
      }
  else if (string_match("o", argv[3]))
      {
	type_restriction = FOR_OPERATORS_TF;
      }
  else if (string_match("*", argv[3]))
      {
	type_restriction = FOR_ANYTHING_TF;
      }
  else 
      {
	printf("\nUnrecognized option to %s %s %s: %s\n", argv[0], argv[1], argv[2], argv[3]);
	return FALSE;	
      }

  if (argc > 4)
    {
      get_lexeme_from_string(argv[4]);

      /* --- read optional name restriction --- */
      if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) 
	{
	  name_restriction = make_sym_constant (glbAgent, argv[4]);
	  format_arg = 5;
        }
      else
	{
	  format_arg = 4;
	}

      if (   (remove  && (argc > format_arg))
	  || (!remove && (argc > (format_arg + 1))))
	{
	  printf("\n%s\n", too_many_args);
	  return FALSE;
	}
    }

  /* --- finally, execute the command --- */
  if (remove) 
    {
      remove_trace_format (glbAgent, stack_trace, type_restriction, name_restriction);
    } 
  else 
    {
      if (argc == (format_arg + 1))
	{
	  add_trace_format (glbAgent, 
				stack_trace, 
			    type_restriction, 
			    name_restriction,
			    argv[format_arg]);
	}
      else
	{
	  if (name_restriction)
	    {
	      symbol_remove_ref (glbAgent, name_restriction);
	    }
	  
	  printf("Missing format string");
	  return FALSE;
	}
    }

  if (name_restriction) 
    {
      symbol_remove_ref (glbAgent, name_restriction);
    }

  return TRUE;
}

int soar_MaxChunks(char ** argv, int argc)
{
	int num;
	
	if (argc == 1)
    {
		printf("\n%ld\n", current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
		return TRUE;
    }
	
	if (argc > 2)
    {
		printf("\nToo many arguments, should be: max-chunks [integer]\n");
		return FALSE;
    }
	
	if (isdigit(argv[1][0]))
    {
		num = atoi(argv[1]);
		set_sysparam (glbAgent, MAX_CHUNKS_SYSPARAM, num);
    }
	else
    {
		printf("\nExpected integer for new maximum chunks count: %s\n", argv[1]);
		return TRUE;
    }
	
	return TRUE;
}

int soar_MaxElaborations(char ** argv, int argc)
{
  int num;

  if (argc == 1)    {
      printf("\n%ld\n", current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM]);
      return TRUE;
  }

  if (argc > 2)    {
      printf("\nToo many arguments, should be: max-elaborations [integer]\n");
      return FALSE;
  }

  if (isdigit(argv[1][0]))    {
      num = atoi(argv[1]);
	  set_sysparam (glbAgent, MAX_ELABORATIONS_SYSPARAM, num);
  } else  {
      printf("\nExpected integer for new maximum elaborations count: %s\n", argv[1]);
      return TRUE;
  }

  return TRUE;
}

int soar_MaxNilOutputCycles(char ** argv, int argc)
{
  int num;

  if (argc == 1)    {
      printf("\n%ld\n", current_agent(sysparams)[MAX_NIL_OUTPUT_CYCLES_SYSPARAM]);
      return TRUE;
  }

  if (argc > 2)    {
      printf("\nToo many arguments, should be: max-nil-output-cycles [integer]\n");
      return FALSE;
  }

  if (isdigit(argv[1][0]))    {
      num = atoi(argv[1]);
	  set_sysparam (glbAgent, MAX_NIL_OUTPUT_CYCLES_SYSPARAM, num);
  } else  {
      printf("\nExpected integer for new maximum output cycle count: %s\n", argv[1]);
      return TRUE;
  }

  return TRUE;
}

int soar_ChunkNameFormat(char ** argv, int argc)
{
  unsigned long tmp_chunk_count;
  int i;
  Bool seen_long_or_short;

  if (argc == 1) {
    printf("\nNo arguments given.\nUsage: chunk-name-format [-short|-long] [-prefix [<prefix>]] [-count [<start-chunk-number>]]\n");
    return FALSE;
  }

  seen_long_or_short = FALSE;
  for (i = 1; i < argc; i++) {
    if        (string_match_up_to(argv[i], "-short",  2)) {
      if (seen_long_or_short) {
        printf("\n-long and -short are exclusive options\n", argv[i]);
        return FALSE;
      } else {
        seen_long_or_short = TRUE;
        set_sysparam(glbAgent, USE_LONG_CHUNK_NAMES, FALSE);
      }
    } else if (string_match_up_to(argv[i], "-long",   2)) {
      if (seen_long_or_short) {
        printf("\n-long and -short are exclusive options\n", argv[i]);
        return FALSE;
      } else {
        seen_long_or_short = TRUE;
        set_sysparam(glbAgent, USE_LONG_CHUNK_NAMES, TRUE);
      }
    } else if (string_match_up_to(argv[i], "-prefix", 2)) {
      if ((i+1 >= argc) || (*argv[i+1] == '-'))
        print(glbAgent, "%s\n",current_agent(chunk_name_prefix));
      else {
        if (strchr(argv[i+1],'*')) {
          printf("\nPrefix-string may not contain a '*' character.\n");
          return FALSE;
        } else
          strcpy(current_agent(chunk_name_prefix),argv[++i]);
      }
    } else if (string_match_up_to(argv[i], "-count",  2)) {
      if ((i+1 >= argc) || (*argv[i+1] == '-'))
        print(glbAgent, "%lu\n",current_agent(chunk_count));
      else if (sscanf(argv[i+1],"%lu",&tmp_chunk_count) == 1) {
        i++;
        if        (tmp_chunk_count < 0) {
          printf("\nchunk-name-format: start-chunk-number must be > 0\n");
          return FALSE;
        } else if (tmp_chunk_count >= current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]) {
          printf("\nchunk-name-format: start-chunk-number must be < max chunk system parameter (%ld)\n", 
             current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
          return FALSE;
        } else if (tmp_chunk_count < current_agent(chunk_count)) {
          printf("\nchunk-name-format: start-chunk-number cannot be less than current chunk count (%ld)\n", 
             current_agent(chunk_count));
          return FALSE;
        } else {
          current_agent(chunk_count) = tmp_chunk_count;
        }
      } else {
        printf("\nchunk-name-format: expected number after -count; got \"%s\"\n", argv[i]);
        return FALSE;
      }
    } else {
      printf("\nUnrecognized argument to chunk-name-format: %s\n", argv[i]);
      return FALSE;
    }
  }
  return TRUE;
}

int soar_DefWmeDepth(char ** argv, int argc)
{
	int depth;

	if (argc == 1)
    {
		printf("\n%d\n", current_agent(default_wme_depth));
		return TRUE;
    }

	if (argc > 2)
    {
		printf("\nToo many arguments, should be: default-wme-depth [integer]\n");
		return FALSE;
    }

	if (isdigit(argv[1][0]))
    {
		depth = atoi(argv[1]);		
		current_agent(default_wme_depth) = depth;
    }
	else
    {
		printf("\nExpected integer for new default print depth: %s\n", argv[1]);
		return TRUE;
    }

	return TRUE;
}

int soar_Version(char ** argv, int argc)
{
	if (argc > 1)
    {
		printf("\nToo many arguments, should be: version\n");
		return FALSE;
    }
	
	printf("\n%s\n", soar_version_string); 
	return TRUE;
}

int soar_Soarnews(char ** argv, int argc)
{
	/* BUGBUG update soarnews printout on successive versions */
	printf("\nNews for Soar version %s:\n%s\n", soar_version_string, soar_news_string);
	
	return TRUE;
}

int soar_InternalSymbols(char ** argv, int argc)
{
	if (argc > 1)
    {
		printf("\nToo many arguments, should be: internal-symbols\n");
		return FALSE;
    }
	
	print_internal_symbols(glbAgent);
	
	return TRUE;
}

int soar_InputPeriod(char ** argv, int argc)
{
	int period;
	
	if (argc == 1)
    {
		printf("\n%d\n", current_agent(input_period));
		return TRUE;
    }
	
	if (argc > 2)
    {
		printf("\nToo many arguments, should be: input-period [integer]\n");
		return FALSE;
    }
	
	if (isdigit(argv[1][0]))
    {
		period = atoi(argv[1]);
		if (period >= 0)
		{
			current_agent(input_period) = period;
		}
		else
		{
			printf("\nInteger for new input period must be >= 0, not %s\n", argv[1]);
			return FALSE;
		}
    }
	else
    {
		printf("\nExpected integer for new input period: %s\n", argv[1]);
		return FALSE;
    }
	
	return TRUE;
}

int soar_GDS_Print(char ** argv, int argc)
{
	wme *w;
	Symbol *goal;
	
	print(glbAgent, "********************* Current GDS **************************\n");
	print(glbAgent, "stepping thru all wmes in rete, looking for any that are in a gds...\n");
	for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next) {
		if (w->gds){
			if (w->gds->goal) {
				print_with_symbols (glbAgent, "  For Goal  %y  ", w->gds->goal);
			} else {
				print(glbAgent, "  Old GDS value ");
			}
			print (glbAgent, "(%lu: ", w->timetag);
			print_with_symbols (glbAgent, "%y ^%y %y", w->id, w->attr, w->value);
			if (w->acceptable) print_string (glbAgent, " +");
			print_string (glbAgent, ")");
			print (glbAgent, "\n");
		}
	}
	print(glbAgent, "************************************************************\n");
	for (goal=current_agent(top_goal); goal!=NIL; goal=goal->id.lower_goal){
		print_with_symbols (glbAgent, "  For Goal  %y  ", goal);
		if (goal->id.gds){
			/* Loop over all the WMEs in the GDS */
			print(glbAgent, "\n");
			for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next){
				print(glbAgent, "                (%lu: ", w->timetag);
				print_with_symbols (glbAgent, "%y ^%y %y", w->id, w->attr, w->value);
				if (w->acceptable) print_string (glbAgent, " +");
				print_string (glbAgent, ")");
				print(glbAgent, "\n");
			}
			
		} else print(glbAgent, ": No GDS for this goal.\n");
	}
    
	print(glbAgent, "************************************************************\n");
	return TRUE;
}

int soar_IndifferentSelection(char ** argv, int argc)
{
  if (argc == 1)
    {
      switch (current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]) 
	{
	case USER_SELECT_FIRST:  printf("\n-first\n");  break;
	case USER_SELECT_LAST:   printf("\n-last\n");   break;
	case USER_SELECT_ASK:    printf("\n-ask\n");    break;
	case USER_SELECT_RANDOM: printf("\n-random\n"); break;
	}
      return TRUE;
    }

  if (argc > 2)
    {
      printf("\nToo many arguments, should be: user-select [-first | -last | -ask | -random ]\n");
      return FALSE;
    }

  if (string_match_up_to(argv[1], "-ask", 2))
    {
      set_sysparam (glbAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_ASK);
    }
  else if (string_match_up_to(argv[1], "-first", 2))
    {
      set_sysparam (glbAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
    }
  else if (string_match_up_to(argv[1], "-last", 2))
    {
      set_sysparam (glbAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
    }
  else if (string_match_up_to(argv[1], "-random", 2))
    {
      set_sysparam (glbAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
    }
  else
    {
      printf("\nUnrecognized argument to indifferent-selection: %s\n", argv[1]);
      return FALSE;
    }

  return TRUE;
}
