typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
#define dealloc_and_return(x,y) { deallocate_test(glbAgent, x) ; return (y) ; }
/************************************************************************/
/* soarapi.cpp

   This file contains various api functions that were included in version
   8.4 of the kernel. For the purpose of testing version 8.3, they have
   been included here as well
*/

#include "soarapi.h"
#include "new_soar.h"
#include "utilities.h"
#include "soar_core_api.h"
#include "soar_ecore_api.h"

#include "explain.h"

#include "wmem.h"
#include "rete.h"
#include "agent.h"
#include "trace.h"
#include "print.h"
#include "parser.h"
#include "rhsfun.h"
#include "symtab.h"
#include "tempmem.h"
#include "init_soar.h"
#include "production.h"
#include "gdatastructs.h"
#include "gsysparam.h"

extern Kernel* SKT_kernel;

/*
char * preference_name[] = {
  "acceptable",
  "require",
  "reject",
  "prohibit",
  "reconsider",
  "unary indifferent",
  "unary parallel",
  "best",
  "worst",
  "binary indifferent",
  "binary parallel",
  "better",
  "worse"
};
*/
void add_multi_attribute_or_change_value(char *sym, long val) {
  multi_attribute *m = current_agent(multi_attributes);
  Symbol *s = make_sym_constant(glbAgent, sym);

  while(m) {
    if(m->symbol == s) {
      m->value = val;
      symbol_remove_ref(glbAgent, s);
      return;
    }
    m = m->next;
  }
  /* sym wasn't in the table if we get here, so add it */
  m = (multi_attribute *)allocate_memory(glbAgent, sizeof(multi_attribute),
                                         MISCELLANEOUS_MEM_USAGE);
  m->value = val;
  m->symbol = s;
  m->next = current_agent(multi_attributes);
  current_agent(multi_attributes) = m;
}

/*
*----------------------------------------------------------------------
*
* soar_PWatch --
*
*----------------------------------------------------------------------
*/

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

int soar_PWatch (int argc, char *argv[], soarResult *res)
{
   Bool trace_productions = TRUE;
   int next_arg = 1;
   
   clearSoarResultResult( res );
   if (argc == 1)
   {
      cons *c;
      
      for (c=current_agent(productions_being_traced); c!=NIL; c=c->rest)
         print_with_symbols (glbAgent, " %y\n", ((production *)(c->first))->name);  
      
      return SOAR_OK;
   }
   
   /* Check for optional argument */
   
   if (string_match_up_to(argv[1], "-on", 3))
   {
      trace_productions = TRUE;
      next_arg++;
   }
   if (string_match_up_to(argv[1], "-off", 3))
   {
      trace_productions = FALSE;
      next_arg++;
   }
   
   if ((argc == 2) && (next_arg != 1))
   {
      /* Turn on/off all productions */
      
      if (trace_productions)
      {
         /* List all productions that are currently being traced */
         soar_ecPrintProductionsBeingTraced();
         return SOAR_OK;
      }
      else
      {
         /* Stop tracing all productions */
         soar_ecStopAllProductionTracing();
         return SOAR_OK;
      }
   }
   
   /* Otherwise, we have a list of productions to process */
   {
      
      if ( trace_productions ) {
         if (soar_ecBeginTracingProductions((argc-next_arg), &argv[next_arg] )) {
            setSoarResultResult( res, "Could not begin tracing" );
            return SOAR_ERROR;
         }
         
      }
      else {
         if (soar_ecStopTracingProductions( (argc-next_arg), &argv[next_arg] )) {
            setSoarResultResult( res, "Could not stop tracing" );
            return SOAR_ERROR;
         }
      }
      
      return SOAR_OK;
   }
   
}

#endif

/*
*----------------------------------------------------------------------
*
* soar_Watch --
*
*----------------------------------------------------------------------
*/

int soar_Watch (int argc, char *argv[], soarResult *res)
{
   
   clearSoarResultResult( res );
   if (argc == 1)
   {
      print_current_watch_settings();
   }
   
   {
      int i;
      
      for (i = 1; i < argc; i++)
      {
         if ( (string_match("0",argv[i])) || (string_match("none", argv[i])) )
         {
            if ( soar_ecWatchLevel( 0 ) )
            {
               setSoarResultResultStdError( res );
               return SOAR_ERROR;
            }
         }
         else if (string_match("1", argv[i]))
         {
            if (soar_ecWatchLevel( 1 ))
            {
               return SOAR_ERROR;
            }
         }
         else if (string_match("decisions", argv[i]))
         {
            /* check if -on|-off|-inc follows */
            if ( (i+1) < argc )
            {
               if ((string_match("-on",argv[i+1])) ||
                  (string_match("-off",argv[i+1])) ||
                  (string_match_up_to("-inclusive",argv[i+1],3)))
               {
                  if (set_watch_setting (TRACE_CONTEXT_DECISIONS_SYSPARAM,
                     argv[i],
                     argv[++i], 
                     res) 
                     != SOAR_OK)
                  {
                     return SOAR_ERROR;
                  }
               }
               else /* something else follows setting, so it's inclusive */
               {
                  if (soar_ecWatchLevel( 1 ))
                  {
                     return SOAR_ERROR;
                  }
               }
            }
            else /* nothing else on cmd line */
            {
               if (soar_ecWatchLevel( 1 ))
               {
                  return SOAR_ERROR;
               }
            }
         }
         else if (string_match("2", argv[i]))
         {
            if (soar_ecWatchLevel( 2 ))
            {
               return SOAR_ERROR;
            }
         }
         else if (string_match("phases", argv[i]))
         {
            /* check if -on|-off|-inc follows */
            if ( (i+1) < argc )
            {
               if ((string_match("-on",argv[i+1])) ||
                  (string_match("-off",argv[i+1])) ||
                  (string_match_up_to("-inclusive",argv[i+1],3)))
               {
                  if (set_watch_setting (TRACE_PHASES_SYSPARAM,
                     argv[i],
                     argv[++i], 
                     res) 
                     != SOAR_OK)
                  {
                     return SOAR_ERROR;
                  }
               }
               else /* something else follows setting, so it's inclusive */
               {
                  if (soar_ecWatchLevel( 2 ))
                  {
                     return SOAR_ERROR;
                  }
               }
            }
            else /* nothing else on cmd line */
            {
               if (soar_ecWatchLevel( 2 ))
               {
                  return SOAR_ERROR;
               }
            }
         }
         else if (string_match("3", argv[i]))
         {
            if (soar_ecWatchLevel( 3 ))
            {
               return SOAR_ERROR;
            }
         }
         else if (string_match("productions", argv[i]))
         {
            int t;
            
            /* productions is a different beast, since there are four
            separate categories and more flags possible */
            /* check if -on|-off|-inc follows */
            if ( (i+1) < argc )
            {
               if (string_match("-on",argv[i+1]))
               {
                  for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
                     set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+t,
                     TRUE);
                  i++;
               }
               else if (string_match("-off",argv[i+1]))
               {
                  for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
                     set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+t,
                     FALSE);
                  i++;
               }
               else if (string_match_up_to("-inclusive",argv[i+1],3))
               {
                  if (soar_ecWatchLevel( 3 ))
                  {
                     return SOAR_ERROR;
                  }
                  i++;
               }
               
               /* check for specific production types */
               
               else if ((string_match("-all", argv[i+1])) || (string_match("-a",argv[i+1]))) {
                  i++;
                  set_watch_prod_group_setting( 0,argv[i], argv[++i], res);
               }
               else if ((string_match("-chunks", argv[i+1])) ||
                  (string_match("-c",argv[i+1])))  {
                  i++;
                  set_watch_prod_group_setting( 1, argv[i], argv[++i], res);
               }
               else if ((string_match("-defaults", argv[i+1])) ||
                  (string_match("-d",argv[i+1])))  {
                  i++;
                  set_watch_prod_group_setting(2, argv[i], argv[++i], res);
               }
               else if ((string_match("-justifications", argv[i+1])) ||
                  (string_match("-j",argv[i+1])))  {
                  i++;
                  set_watch_prod_group_setting( 3, argv[i], argv[++i], res);
               }
               else if ((string_match("-user", argv[i+1])) ||
                  (string_match("-u",argv[i+1])))  {
                  i++;
                  set_watch_prod_group_setting( 4, argv[i], argv[++i], res);
               }
               
               else /* something else follows setting, so it's inclusive */
               {
                  if (soar_ecWatchLevel( 3 ))
                  {
                     return SOAR_ERROR;
                  }
               }
            }
            else /* nothing else on cmd line */
            {
               if (soar_ecWatchLevel( 3 ))
               {
                  return SOAR_ERROR;
               }
            }
         }
         else if (string_match("4", argv[i]))
         {
            print( glbAgent, "Receive Watch 4\n" );
            
            if  (soar_ecWatchLevel( 4 ) )
            {
               
               return SOAR_ERROR;
            }
         }
         else if (string_match("wmes", argv[i]))
         {
            /* kjh(CUSP-B2) begin */
            char *wmes_option_syntax_msg = "\
                                              watch wmes syntax:\n\
                                              wmes [ -on |\n\
                                              -off |\n\
                                              -inc[lusive] |\n\
                                              {-add-filter    type filter} |\n\
                                              {-remove-filter type filter} |\n\
                                              {-reset-filter  type} |\n\
                                              {-list-filter   type} ]\n\
                                              where\n\
                                              type   = -adds|-removes|-both\n\
                                              filter = {id|*} {attribute|*} {value|*}";
            
            if (i+1 >= argc) { /* nothing else on cmd line, so it's inclusive */
               if (soar_ecWatchLevel( 4 ))
               {
                  return SOAR_ERROR;
               }
            } else if ((string_match(argv[i+1],"-on")) || 
               (string_match(argv[i+1],"-off")) || 
               (string_match_up_to("-inclusive",argv[i+1],3))) {
               if (set_watch_setting (TRACE_WM_CHANGES_SYSPARAM,
                  argv[i],
                  argv[i+1],
                  res) 
                  != SOAR_OK)
                  return SOAR_ERROR;
               else
                  i += 1;
            } else if (i+2 >= argc) {
               setSoarResultResult( res,  wmes_option_syntax_msg);
               return SOAR_ERROR;
            } else if (string_match(argv[i+1],"-add-filter")) {
               Bool forAdds, forRemoves;
               if ( (i+5 >= argc)
                  || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == SOAR_ERROR)) {
                  appendSoarResultResult( res, wmes_option_syntax_msg );
                  return SOAR_ERROR;
               } else {
                  if (soar_ecAddWmeFilter(argv[i+3],argv[i+4],argv[i+5],
                     forAdds,forRemoves) != 0 ) {
                     setSoarResultResult(res, "Error: Filter not added.");
                     return SOAR_ERROR;
                  } else {
                     setSoarResultResult(res, "Filter added.");
                  }
               }
               i += 5;
            } else if (string_match(argv[i+1],"-remove-filter")) {
               Bool forAdds, forRemoves;
               if ( (i+5 >= argc)
                  || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == SOAR_ERROR)) {
                  appendSoarResultResult(res, wmes_option_syntax_msg);
                  return SOAR_ERROR;
               } else {
                  if (soar_ecRemoveWmeFilter(argv[i+3],argv[i+4],argv[i+5],
                     forAdds,forRemoves) != 0 ) {
                     setSoarResultResult(res, 
                        "Error: Bad args or filter not found");
                     return SOAR_ERROR;
                  } else {
                     appendSoarResultResult(res, "Filter removed.");
                  }
               }
               i += 5;
            } else if (string_match(argv[i+1],"-reset-filter")) {
               Bool forAdds, forRemoves;
               if ( (i+2 >= argc)
                  || (parse_filter_type(argv[i+2],&forAdds,&forRemoves) == SOAR_ERROR)) {
                  appendSoarResultResult(res,  wmes_option_syntax_msg);
                  return SOAR_ERROR;
               } else {
                  if (soar_ecResetWmeFilters(forAdds,forRemoves) != 0) {
                     appendSoarResultResult( res, "No filters were removed.");
                     return SOAR_ERROR;
                  }
               }
               i += 2;
            } else if (string_match(argv[i+1],"-list-filter")) {
               Bool forAdds, forRemoves;
               if ( (i+2 >= argc) || 
                  (parse_filter_type( argv[i+2], &forAdds, 
                  &forRemoves)  == SOAR_ERROR) ) {
                  
                  appendSoarResultResult( res, wmes_option_syntax_msg );
                  return SOAR_ERROR;
               } 
               else {
                  soar_ecListWmeFilters(forAdds,forRemoves);
               }
               i += 2;
            }
         }
         /* kjh(CUSP-B2) end */
         /*  kjc note:  not sure CUSP-B2 solution accounts for other
         *  non-wme args following "wmes" which should make it -inc 
         */
         
         else if (string_match("5", argv[i]))
         {
            if (soar_ecWatchLevel( 5 ))
            {
               return SOAR_ERROR;
            }
         }
         else if (string_match("preferences", argv[i]))
         {
            /* check if -on|-off|-inc follows */
            if ( (i+1) < argc )
            {
               if ((string_match("-on",argv[i+1])) ||
                  (string_match("-off",argv[i+1])) ||
                  (string_match_up_to("-inclusive",argv[i+1],3)))
               {
                  if (set_watch_setting (TRACE_FIRINGS_PREFERENCES_SYSPARAM,
                     argv[i],
                     argv[++i],
                     res) 
                     != SOAR_OK)
                  {
                     return SOAR_ERROR;
                  }
               }
               else /* something else follows setting, so it's inclusive */
               {
                  if (soar_ecWatchLevel( 5 ))
                  {
                     return SOAR_ERROR;
                  }
               }
            }
            else /* nothing else on cmd line */
            {
               if (soar_ecWatchLevel( 5 ))
               {
                  return SOAR_ERROR;
               }
            }
         } 
         else if ((string_match("-all", argv[i])) || (string_match("-a",argv[i])))
         {
            set_watch_prod_group_setting(0, argv[i], argv[++i], res);
         }
         else if ((string_match("-chunks", argv[i])) ||
            (string_match("-c",argv[i])))
         {
            set_watch_prod_group_setting(1, argv[i], argv[++i], res);
         }
         else if ((string_match("-defaults", argv[i])) ||
            (string_match("-d",argv[i])))
         {
            set_watch_prod_group_setting( 2, argv[i], argv[++i], res);
         }
         else if ((string_match("-justifications", argv[i])) ||
            (string_match("-j",argv[i])))
         {
            set_watch_prod_group_setting( 3, argv[i], argv[++i], res);
         }
         else if ((string_match("-user", argv[i])) ||
            (string_match("-u",argv[i])))
         {
            set_watch_prod_group_setting( 4, argv[i], argv[++i], res);
         }
         else if (string_match_up_to("-nowmes", argv[i],4))
         {
            set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
               NONE_WME_TRACE);
         }
         else if (string_match_up_to("-timetags", argv[i],3))
         {
            set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
               TIMETAG_WME_TRACE);
         }
         else if (string_match_up_to("-fullwmes", argv[i],6))
         {
            set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
               FULL_WME_TRACE);
         }
         else if (string_match_up_to("-prefs", argv[i],4))
         {
            set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,TRUE);
         }
         else if (string_match_up_to("-noprefs", argv[i],6))
         {
            set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,FALSE);
         }
         /* REW: begin 10.22.97 */
         else if (string_match_up_to("-soar8", argv[i],6))
         {
            set_sysparam(glbAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM,TRUE);
         }
         else if (string_match_up_to("-nosoar8", argv[i],6))
         {
            set_sysparam(glbAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM,FALSE);
         }
         /* REW: end   10.22.97 */
         else if (string_match_up_to("learning", argv[i],2))
         {
            /* check if -print|-noprint|-fullprint follows */
            if ( (i+1) < argc )
            {
               if (string_match("-print",argv[i+1]))
               {
                  set_sysparam(glbAgent, TRACE_CHUNK_NAMES_SYSPARAM,TRUE);
                  set_sysparam(glbAgent, TRACE_CHUNKS_SYSPARAM,     FALSE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM,TRUE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATIONS_SYSPARAM,     FALSE);
                  i++;
               }
               else if  (string_match("-noprint",argv[i+1]))
               {
                  set_sysparam(glbAgent, TRACE_CHUNK_NAMES_SYSPARAM,FALSE);
                  set_sysparam(glbAgent, TRACE_CHUNKS_SYSPARAM,     FALSE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM,FALSE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATIONS_SYSPARAM,     FALSE);
                  i++;
               }
               else if (string_match_up_to("-fullprint",argv[i+1],3))
               {
                  set_sysparam(glbAgent, TRACE_CHUNK_NAMES_SYSPARAM,TRUE);
                  set_sysparam(glbAgent, TRACE_CHUNKS_SYSPARAM,     TRUE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM,TRUE);
                  set_sysparam(glbAgent, TRACE_JUSTIFICATIONS_SYSPARAM,     TRUE);
                  i++;
               }
               else
               { /* error: no arg for learning */
                  setSoarResultResult( res, 
                     "Missing setting for watch learning, should be -noprint|-print|-fullprint");
                  return SOAR_ERROR;
               }
            }
            else
            { /* error: no arg for learning */
               setSoarResultResult( res, 
                  "Missing setting for watch learning, should be -noprint|-print|-fullprint");
               return SOAR_ERROR;
            }
         }
         else if (string_match("backtracing", argv[i]))
         {
            if (set_watch_setting (TRACE_BACKTRACING_SYSPARAM,
               argv[i],
               argv[++i],
               res) 
               != SOAR_OK)
            {
               return SOAR_ERROR;
            }
         }
         else if (string_match("loading", argv[i]))
         {
            if (set_watch_setting (TRACE_LOADING_SYSPARAM,
               argv[i],
               argv[++i],
               res) 
               != SOAR_OK)
            {
               return SOAR_ERROR;
            }
         }
         /* Pushed to interface  081699 
         else if (string_match("aliases", argv[i]))
         {
         if (argv[i+1] == NULL)
         {
         setSoarResultResult( res, 
         "Missing setting for watch alias, should be -on|-off");
         return SOAR_ERROR;
         }
         else if (string_match("-on",argv[i+1]))
         {
         Interface_SetVar(glbAgent, "print_alias_switch","on");
         i++;
         }
         else if (string_match("-off",argv[i+1]))
         {
         Interface_SetVar(glbAgent,"print_alias_switch","off");
         i++;
         }
         else
         {
         setSoarResultResult( res, 
         "Unrecognized argument to watch alias : %s",
         argv[i+1]);
         return SOAR_ERROR;
         }
         }
         */
         else
         {
            setSoarResultResult( res, 
               "Unrecognized argument to watch command: %s",
               argv[i]);
            return SOAR_ERROR;
         }
      }
  }
  
  return SOAR_OK;
}

void print_memories (int num_to_print, int to_print[]) {
  int i, num_prods;
  production_memory_use *temp, *first, *tempnext;
  production *prod;

  print(glbAgent, "\nMemory use for productions:\n\n");

  /* Start by doing ALL of them. */
  first = NULL;
  num_prods = 0;

  for (i=0; i < NUM_PRODUCTION_TYPES; i++)
    if (to_print[i]) 
      for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL; prod=prod->next) {
	 temp = static_cast<production_memory_use *>(allocate_memory (glbAgent, 
		 sizeof (production_memory_use), MISCELLANEOUS_MEM_USAGE));
	 temp->next = NULL;
	 temp->name = prod->name;

         temp->mem = count_rete_tokens_for_production (glbAgent, prod);

	 first = print_memories_insert_in_list(temp,first);
	 num_prods++;
        }

  i = 0;
  if (num_to_print < 0) num_to_print = num_prods;

  for (temp = first; ((temp != NULL) && (i < num_to_print)); temp = tempnext) {
    print_with_symbols(glbAgent, "%y: ", temp->name);
    print(glbAgent, "%d\n",temp->mem);
    tempnext = temp->next;
    free_memory(glbAgent, temp,MISCELLANEOUS_MEM_USAGE);
    i++;
   }
}

production_memory_use *print_memories_insert_in_list(production_memory_use *New,
						     production_memory_use *list) {
  production_memory_use *ctr, *prev;

  /* Add to beginning. */
  if ((list == NULL) || (New->mem >= list->mem)) {
    New->next = list;
    return New;}

  /* Add to middle. */
  prev = list;
  for (ctr = list->next; ctr != NULL; ctr = ctr->next) {
    if (New->mem >= ctr->mem) {
      prev->next = New;
      New->next = ctr;
      return list;
    }
    prev = ctr;
  }

  /* Add to end. */
  prev->next = New;
  New->next = NULL;
  return list;
}

/*
 *----------------------------------------------------------------------
 *
 * print_preference_and_source --
 *
 *	This procedure prints a preference and the production
 *      which is the source of the preference.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Prints the preference and its source production.
 *
 *----------------------------------------------------------------------
 */

void print_preference_and_source (preference *pref,
                                  Bool print_source,
                                  wme_trace_type wtt) 
{
  print_string (glbAgent, "  ");
  print_object_trace (glbAgent, pref->value);
  print(glbAgent, " %c", preference_type_indicator (glbAgent, pref->type));
  if (preference_is_binary(pref->type)) print_object_trace (glbAgent, pref->referent);
  if (pref->o_supported) print(glbAgent, " :O ");
  print(glbAgent, "\n");
  if (print_source) {
    print(glbAgent, "    From ");
    print_instantiation_with_wmes (glbAgent, pref->inst, wtt);
    print(glbAgent, "\n");
  }
}

/*
 *----------------------------------------------------------------------
 *
 * print_current_learn_settings --
 *
 *	This procedure prints the current learn settings.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current settings.
 *
 *----------------------------------------------------------------------
 */

void print_current_learn_settings(void)
{
  print(glbAgent, "Current learn settings:\n");
/* AGR MVL1 begin */
    if ((! current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) &&
	(! current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM]))
      print(glbAgent, "   %s\n", current_agent(sysparams)[LEARNING_ON_SYSPARAM] ? "-on" : "-off");
    else
      print(glbAgent, "   %s\n", current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] ? "-only" : "-except");

/* AGR MVL1 end */
  print(glbAgent, "   %s\n", current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] ? "-all-levels" : "-bottom-up");
  
}

void print_current_watch_settings (void)
{
/* Added this to avoid segfault on Solaris when attempt to print NULL */
  /*char *a = NULL; */

  print(glbAgent, "Current watch settings:\n");
  print(glbAgent, "  Decisions:  %s\n",
	 current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
  print(glbAgent, "  Phases:  %s\n",
	 current_agent(sysparams)[TRACE_PHASES_SYSPARAM] ? "on" : "off");
  print(glbAgent, "  Production firings/retractions\n");
  print(glbAgent, "    default productions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
  print(glbAgent, "    user productions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
  print(glbAgent, "    chunks:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
  print(glbAgent, "    justifications:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
  print(glbAgent, "    WME detail level:  %d\n",
	 current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
  print(glbAgent, "  Working memory changes:  %s\n",
	 current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
  print(glbAgent, "  Preferences generated by firings/retractions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
  /*  don't print these individually...see chunk-creation
   *  print(glbAgent, "  Chunk names:  %s\n",
   *      current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "on" : "off");
   *  print(glbAgent, "  Justification names:  %s\n",
   *      current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] ? "on" : "off");
   *  print(glbAgent, "  Chunks:  %s\n",
   *      current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] ? "on" : "off");
   *  print(glbAgent, "  Justifications:  %s\n",
   *      current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
   */
  print(glbAgent, "\n");
  if (current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM]) {
    print(glbAgent, "  Learning:  -fullprint  (watch creation of chunks/just.)\n");
  } else {
    print(glbAgent, "  Learning:  %s  (watch creation of chunks/just.)\n",
	   current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "-print" : "-noprint");
  }
  print(glbAgent, "  Backtracing:  %s\n",
	 current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");

  print(glbAgent, "  Loading:  %s\n",
	 current_agent(sysparams)[TRACE_LOADING_SYSPARAM] ? "on" : "off");
 
  print(glbAgent, "\n" );
}

/*
 *----------------------------------------------------------------------
 *
 * print_multi_attribute_symbols --
 *
 *	This procedure prints a list of the symbols that are
 *      declared to have multi_attributes.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the list of symbols and the value of multi_attribute.
 *
 *----------------------------------------------------------------------
 */
void print_multi_attribute_symbols(void)
{
  multi_attribute *m = current_agent(multi_attributes);

  print(glbAgent, "\n");
  if (!m) {
    print(glbAgent, "No multi-attributes declared for this agent.\n");
  } else {
    print(glbAgent, "Value\tSymbol\n");
    while(m) {
      print(glbAgent, "%ld\t%s\n", m->value, symbol_to_string(glbAgent, m->symbol, TRUE, NIL, 0));
      m = m->next;
    }
  }
  print(glbAgent, "\n");
}

/*
 *----------------------------------------------------------------------
 *
 * set_watch_setting --
 *
 *	This procedure parses an argument to a watch variable to
 *      determine if "on" or "off" has been selected.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Sets the desired paramater to the indicated setting.
 *
 *----------------------------------------------------------------------
 */

int set_watch_setting ( int dest_sysparam_number, char * param, 
			char * arg, soarResult *res) 
{
  if (arg == NULL)
    {
      setSoarResultResult( res, 
	      "Missing setting for watch parameter %s",
	      param);
      return SOAR_ERROR;
    }

  if (!strcmp("-on", arg))
    {
      set_sysparam (glbAgent, dest_sysparam_number, TRUE);
    }
  else if (!strcmp("-off", arg))
    {
      set_sysparam (glbAgent, dest_sysparam_number, FALSE);
    }
  else if (!strncmp("-inclusive",arg,3))
    {
      soar_ecWatchLevel(dest_sysparam_number);
    }
  else
    {
      setSoarResultResult( res, 
	      "Unrecognized setting for watch parameter %s: %s",
	      param, arg);
      return SOAR_ERROR;
    }
  return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * set_watch_prod_group_setting --
 *
 *	This procedure parses an argument to a watch production-type to
 *      determine if "print" or "noprint" has been selected.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Sets the desired prod-group to the indicated setting.
 *
 *----------------------------------------------------------------------
 */

int set_watch_prod_group_setting (int  prodgroup,
				  char * prodtype, char * arg,
				  soarResult *res) 
{
  if (arg == NULL)
    {
      setSoarResultResult( res, 
	      "Missing setting for watch %s",
	      prodtype);
      return SOAR_ERROR;
    }

  if ( !strcmp("-print", arg))
    {
      switch (prodgroup) {
      case 0:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
	break;
      case 1:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	break;
      case 2:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
	break;
      case 3:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
	break;
      case 4:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
	break;
      }
    }
  else if ( !strcmp("-noprint", arg))
    {
      switch (prodgroup) {
      case 0:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
	break;
      case 1:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
	break;
      case 2:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
	break;
      case 3:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
	break;
      case 4:
	set_sysparam (glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
	break;
      }
    }
  else if ( !strcmp("-fullprint", arg))
    {
      setSoarResultResult( res, 
	      "Sorry, -fullprint not yet implemented for watch productions");
      return SOAR_ERROR;
    }
  else
    {
      setSoarResultResult( res, 
	      "Unrecognized setting for watch %s: %s.  Use -print|-noprint|-fullprint",
	      prodtype, arg);
      return SOAR_ERROR;
    }
  return SOAR_OK;
}

#ifdef USE_STDARGS
void setSoarResultResult ( soarResult *res, const char *format, ...) {
  va_list args;

  va_start (args, format);

#else
void setSoarResultResult (va_list va_alist) {
  va_list args;
  char *format;
  soarResult *res;

  va_start (args);
  res = va_arg(args, soarResult *);
  format = va_arg(args, char *);
#endif
  
  vsprintf( res->result, format, args );
}

#ifdef USE_STDARGS
void appendSoarResultResult ( soarResult *res, const char *format, ...) {
  va_list args;
  int i;

  va_start (args, format);
#else
void appendSoarResultResult (va_alist) va_dcl {
  va_list args;
  char *format;
  soarResult *res;
  int i;

  va_start (args);
  res = va_arg(args, soarResult *);
  format = va_arg(args, char *);
#endif
  
  i = 0;
  while( res->result[i] ) i++;
  

  vsprintf( &res->result[i], format, args );
}

/*
*----------------------------------------------------------------------
*
* soar_Stats --
*
*----------------------------------------------------------------------
*/

int soar_Stats ( int argc, char *argv[], soarResult *res)
{
   
   if (   (argc == 1) 
      || string_match_up_to("-system", argv[1], 2))
   {
      return parse_system_stats( argc, argv, res);
   }
   else if (string_match_up_to("-memory", argv[1], 2))
   {
      return parse_memory_stats( argc, argv, res);
   }
   else if (string_match_up_to("-rete", argv[1], 2))
   {
      return parse_rete_stats( argc, argv, res);
   }
#ifdef DC_HISTOGRAM
   else if (string_match_up_to("-dc_histogram", argv[1], 2))
   {
      return soar_ecPrintDCHistogram();
   }
#endif
#ifdef KT_HISTOGRAM
   else if (string_match_up_to("-kt_histogram", argv[1], 2 ))
   {
      return soar_ecPrintKTHistogram();
   }
#endif
#ifndef NO_TIMING_STUFF
   else if (string_match_up_to("-timers", argv[1], 2 ))
   {
      return printTimingInfo();
   }
#endif
   else
   {
      setSoarResultResult( res, 
         "Unrecognized argument to stats: %s",
         argv[1]);
      return SOAR_ERROR;
   }
}

/*
 *----------------------------------------------------------------------
 *
 * parse_system_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -system <stype>
 *         <stype> ::= -default-production-count
 *                     -user-production-count
 *                     -chunk-count
 *                     -justification-count
 *                     -all-productions-count
 *                     -dc-count
 *                     -ec-count
 *                     -ecs/dc
 *                     -firings-count
 *                     -firings/ec
 *                     -wme-change-count
 *                     -wme-addition-count
 *                     -wme-removal-count
 *                     -wme-count
 *                     -wme-avg-count
 *                     -wme-max-count
 *                     -total-time             |T
 *                     -ms/dc                  |T
 *                     -ms/ec                  |T
 *                     -ms/firing              |T
 *                     -ms/wme-change          |T
 *                     -match-time             |D
 *                     -ownership-time         |D
 *                     -chunking-time          |D
 *
 *         The items marked |T are available when Soar has been
 *         compiled with the NO_TIMING_STUFF flag NOT SET and 
 *         the items marked |D are available when Soar has been
 *         compiled with the DETAILED_TIMING_STATS flag SET.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_system_stats (int argc, char * argv[], soarResult *res)
{
  #ifndef NO_TIMING_STUFF
  double total_kernel_time, total_kernel_msec;

#ifdef DETAILED_TIMING_STATS
  double time;
#endif
#endif

  
  if (argc > 3)
    {
      setSoarResultResult( res, 
         "Too many arguments, should be: stats -system [<type>]");
      return SOAR_ERROR;
    }

  total_kernel_time = timer_value( &current_agent(total_kernel_time) );
  total_kernel_msec = total_kernel_time * 1000.0;

  if (argc <= 2) /* Invoked as stats or stats -system */
    {
      soar_ecPrintSystemStatistics();
    }
  else
    {
      if (!strcmp("-default-production-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]);
	}
      else if (!strcmp("-user-production-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]);

	}
      else if (!strcmp("-chunk-count", argv[2]))
	{
	  setSoarResultResult( res, "%lu", 
		  current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
	}
      else if (!strcmp("-justification-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
	}
      else if (!strcmp("-all-productions-count", argv[2]))
	{
	  setSoarResultResult( res, "%lu", 
		  current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]
		  + current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]
		  + current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);

	}
      else if (!strcmp("-dc-count", argv[2]))
	{
	  setSoarResultResult( res, "%lu", current_agent(d_cycle_count));
	}      
      else if (!strcmp("-ec-count", argv[2]))
	{
	  setSoarResultResult( res, "%lu", current_agent(e_cycle_count));
	}      
      else if (!strcmp("-ecs/dc", argv[2]))
	{
	  setSoarResultResult( res, "%.3f", 
		  (current_agent(d_cycle_count) 
		   ? ((double) current_agent(e_cycle_count)
		               / current_agent(d_cycle_count))
		   : 0.0));
	}      
      else if (!strcmp("-firings-count", argv[2]))
	{
	  setSoarResultResult( res, "%lu", 
		  current_agent(production_firing_count));
	}      
      else if (!strcmp("-firings/ec", argv[2]))
	{
	  setSoarResultResult( res, "%.3f", 
		  (current_agent(e_cycle_count) 
		   ? ((double) current_agent(production_firing_count)
		               / current_agent(e_cycle_count))
		   : 0.0));
	}      
      else if (!strcmp("-wme-change-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(wme_addition_count)
		  + current_agent(wme_removal_count));
	}      
      else if (!strcmp("-wme-addition-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(wme_addition_count));
	}      
      else if (!strcmp("-wme-removal-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(wme_removal_count));
	}
      else if (!strcmp("-wme-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(num_wmes_in_rete));
	}      
      else if (!strcmp("-wme-avg-count", argv[2]))
	{
	  setSoarResultResult( res,  "%.3f", 
		  (current_agent(num_wm_sizes_accumulated) 
		   ? (current_agent(cumulative_wm_size) 
		      / current_agent(num_wm_sizes_accumulated)) 
		   : 0.0));
	}      
      else if (!strcmp("-wme-max-count", argv[2]))
	{
	  setSoarResultResult( res,  "%lu", 
		  current_agent(max_wm_size));
	}     
#ifndef NO_TIMING_STUFF
      else if (!strcmp("-total-time", argv[2]))
	{
	  setSoarResultResult( res,  "%.3f", total_kernel_time);
	}
      else if (!strcmp("-ms/dc", argv[2]))
	{
	  setSoarResultResult( res,  "%.3f", 
		  (current_agent(d_cycle_count) 
		   ? total_kernel_msec/current_agent(d_cycle_count) 
		   : 0.0));
	}      
      else if (!strcmp("-ms/ec", argv[2]))
	{
	  setSoarResultResult( res, "%.3f", 
		  (current_agent(e_cycle_count) 
		   ? total_kernel_msec/current_agent(e_cycle_count) 
		   : 0.0));
	}      
      else if (!strcmp("-ms/firing", argv[2]))
	{
	  setSoarResultResult( res, "%.3f", 
		  (current_agent(production_firing_count)
		   ? total_kernel_msec/current_agent(production_firing_count)
		   : 0.0));
	}      
#endif /* NO_TIMING_STUFF */
#ifdef DETAILED_TIMING_STATS
      else if (!strcmp("-ms/wme-change", argv[2]))
	{
	  long wme_changes;
	  time = timer_value (&current_agent(match_cpu_time[INPUT_PHASE])) 
	    + timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[WM_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[DECISION_PHASE]));

	  time *= 1000; /* convert to msec */

	  wme_changes = current_agent(wme_addition_count)
	                + current_agent(wme_removal_count);

	  setSoarResultResult( res,  "%.3f", 
		  (wme_changes ? time/wme_changes : 0.0));
	}      
      else if (!strcmp("-match-time", argv[2]))
	{

	  time = timer_value (&current_agent(match_cpu_time[INPUT_PHASE])) 
	    + timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[WM_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])) 
	    + timer_value (&current_agent(match_cpu_time[DECISION_PHASE]));
  
	  setSoarResultResult( res,  "%.3f", time);

	}
      else if (!strcmp("-ownership-time", argv[2]))
	{
	  time = timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])) 
	    + timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])) 
	    + timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])) 
	    + timer_value (&current_agent(ownership_cpu_time[WM_PHASE])) 
	    + timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])) 
	    + timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]));

	  setSoarResultResult( res,  "%.3f", time);

	}
      else if (!strcmp("-chunking-time", argv[2]))
	{
	  time = timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])) 
	    + timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])) 
	    + timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])) 
	    + timer_value (&current_agent(chunking_cpu_time[WM_PHASE])) 
	    + timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])) 
	    + timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));

	  setSoarResultResult( res,  "%.3f",  time);
 
	}
#endif /* DETAILED_TIMING_STATS */
      else
	{
	  setSoarResultResult( res, 
		  "Unrecognized argument to stats: -system %s",
		  argv[2]);
	}
    }

  return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_memory_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -rete <mtype>
 *         <mtype> ::= -total
 *                     -overhead
 *                     -strings
 *                     -hash-table
 *                     -pool [-total | pool-name [<aspect>]]
 *                     -misc
 *        <aspect> ::= -used                   |M
 *                     -free                   |M
 *                     -item-size
 *                     -total-bytes
 *
 *          The items marked |M are available only when Soar has
 *          been compiled with the MEMORY_POOL_STATS option.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_memory_stats (int argc, char * argv[], soarResult *res)
{
  if (argc == 2)
    {
      soar_ecPrintMemoryStatistics ();
      soar_ecPrintMemoryPoolStatistics ();  

      return SOAR_OK;
    }

  if (!strcmp("-total", argv[2]))
    {
      unsigned long total;
      int i;

      total = 0;
      for (i=0; i<NUM_MEM_USAGE_CODES; i++) 
	total += current_agent(memory_for_usage)[i];
  
      setSoarResultResult( res, "%lu", total);  
    }
  else if (!strcmp("-overhead", argv[2]))
    {
      setSoarResultResult( res, "%lu",
	       current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
    }
  else if (!strcmp("-strings", argv[2]))
    {
      setSoarResultResult( res, "%lu",
	       current_agent(memory_for_usage)[STRING_MEM_USAGE]);
    }
  else if (!strcmp("-hash-table", argv[2]))
    {
      setSoarResultResult( res, "%lu",
	       current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
    }
  else if (!strcmp("-pool", argv[2]))
    {                                          /* Handle pool stats */
      if (argc == 3)
	{
	  soar_ecPrintMemoryPoolStatistics();
	}
      else if (!strcmp("-total", argv[3]))
	{
	  setSoarResultResult( res,  "%lu",
		   current_agent(memory_for_usage)[POOL_MEM_USAGE]);
	}
      else 
	{                         /* Match pool name or invalid item */
	  memory_pool *p;
	  memory_pool *pool_found = NIL;

	  for (p=current_agent(memory_pools_in_use); p!=NIL; p=p->next) 
	    {
	      if (!strcmp (argv[3], p->name))
		{
		  pool_found = p;
		  break;
		}
	    }

	  if (!pool_found)
	    {
	      setSoarResultResult( res, 
		      "Unrecognized pool name: stats -memory -pool %s",
		      argv[4]);
	      return SOAR_ERROR;
	    }

	  if (argc == 4)
	    {
#ifdef MEMORY_POOL_STATS
	      long total_items;
#endif
	      print(glbAgent, "Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
	      print(glbAgent, "Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
	      print(glbAgent, "---------------  ----------  ----------  ---------  -----------\n");
#else
	      print(glbAgent, "Pool Name        Item Size  Total Bytes\n");
	      print(glbAgent, "---------------  ---------  -----------\n");
#endif

	      print_string (glbAgent, pool_found->name);
	      print_spaces (glbAgent, MAX_POOL_NAME_LENGTH - strlen(pool_found->name));
#ifdef MEMORY_POOL_STATS
	      print(glbAgent, "  %10lu", pool_found->used_count);
	      total_items = pool_found->num_blocks 
		            * pool_found->items_per_block;
	      print(glbAgent, "  %10lu", total_items - pool_found->used_count);
#endif
	      print(glbAgent, "  %9lu", pool_found->item_size);
	      print(glbAgent, "  %11lu\n", pool_found->num_blocks 
		                  * pool_found->items_per_block 
		                  * pool_found->item_size);
	    }
	  else if (argc == 5)
	    {                                /* get pool attribute */
	      long total_items;     

	      total_items = pool_found->num_blocks 
		            * pool_found->items_per_block;

	      if (!strcmp("-item-size", argv[4]))
		{
		  setSoarResultResult( res, "%lu", pool_found->item_size);
		}
#ifdef MEMORY_POOL_STATS		
	      else if (!strcmp("-used", argv[4]))
		{
		  setSoarResultResult( res,  "%lu", pool_found->used_count);
		}
	      else if (!strcmp("-free", argv[4]))
		{
		  setSoarResultResult( res,  "%lu", 
			   total_items - pool_found->used_count);
		}
#endif
	      else if (!strcmp("-total-bytes", argv[4]))
		{
		  setSoarResultResult( res,  "%lu", 
			   pool_found->num_blocks 
			   * pool_found->items_per_block 
			   * pool_found->item_size);
		}
	      else
		{
		  setSoarResultResult( res, 
			  "Unrecognized argument to stats: -memory -pool %s %s",
			  argv[3], argv[4]);
		  return SOAR_ERROR;
		}
	    }
	  else
	    {
	      setSoarResultResult( res, "Too many arguments, should be: stats -memory -pool [-total | pool-name [<aspect>]]");
	      return SOAR_ERROR;
	    }
	}
    }
  else if (!strcmp("-misc", argv[2]))
    {
      setSoarResultResult( res,  "%lu",
	       current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
    }
  else
    {
      setSoarResultResult( res, 
	      "Unrecognized argument to stats: -memory %s",
	      argv[2]);
      return SOAR_ERROR;
    }

  return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_rete_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *          stats -rete <rtype> <qualifier>
 *          <rtype> ::= unhashed memory
 *                      memory
 *                      unhashed mem-pos
 *                      mem-pos
 *                      unhashed negative
 *                      negative
 *                      unhashed positive
 *                      positive
 *                      dummy top
 *                      dummy matches
 *                      nconj. neg.
 *                      conj. neg. partner
 *                      production
 *                      total
 *          <qualifier> ::= -actual | -if-no-merging | -if-no-sharing
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_rete_stats ( int argc, char * argv[], soarResult *res)
{
  unsigned long data;

  if (argc == 2)
    {
      soar_ecPrintReteStatistics ();
      return SOAR_OK;
    }

  if (argc == 3)
    {
      setSoarResultResult( res,  
	   "Too few arguments, should be: stats -rete [type qualifier]");
      return SOAR_ERROR;
    }

  if (argc > 4)
    {
      setSoarResultResult( res, 
	   "Too many arguments, should be: stats -rete [type qualifier]" );
      return SOAR_ERROR;
    }

  if (get_node_count_statistic(glbAgent, argv[2], (char *) argv[3]+1, &data))
    {
      setSoarResultResult( res, "%lu", data);
    }
  else
    {
      setSoarResultResult( res, 
	      "Unrecognized argument to stats: -rete %s %s",
	      argv[2], argv[3]);
      return SOAR_ERROR;
    }
  return SOAR_OK;
}

int printTimingInfo () {

#ifdef PII_TIMERS  
  unsigned long long int start, stop;
#else
  struct timeval start, stop;
#endif

  double min, max, min_nz;

  min_nz = soar_cDetermineTimerResolution( &min, &max );
  
#ifdef PII_TIMERS
  print(glbAgent, "Using Pentium II Time Stamp -- Assuming %d MHZ Processor...\n", MHZ);
#else
  print(glbAgent, "Using system timers...\n" );
#endif

  print(glbAgent, "---------------------------------------------------------------\n"); 
  print(glbAgent, "A timing loop is used to obtain the following values.\n" );
  print(glbAgent, "At least one additional instruction takes place between\n" );
  print(glbAgent, "starting and stopping the timers, thus a perfectly accurate\n" );
  print(glbAgent, "timer which costs nothing to invoke would still accumulate\n" );
  print(glbAgent, "non-zero value. The loop runs for a total of ~2 seconds as \n" );
  print(glbAgent, "a result, the Maximum timer value is likely to be relatively \n" );
  print(glbAgent, "large, but should be < 2 seconds\n" );
  print(glbAgent, "** NOTE: If the code was optimized, the timing loop may yield\n" );
  print(glbAgent, "         unanticipated results.  If both minimum values are\n" );
  print(glbAgent, "         zero, it is unclear what the timer resolution is...\n" );
  print(glbAgent, "---------------------------------------------------------------\n"); 
  print(glbAgent, "Minimum (Non-Zero) Timer Value: %11.5e sec\n", min_nz );
  print(glbAgent, "Minimum Timer Value           : %11.5e sec\n", min );
  print(glbAgent, "Maximum Timer Value           : %11.5e sec\n\n", max );
  

  print(glbAgent, "---------------------------------------------------------------\n"); 
  print(glbAgent, "A short delay will be issued using the sleep() command, and \n" );
  print(glbAgent, "timed using Soar's timers....\n" );
  reset_timer( &stop );
  start_timer( &start );
  sys_sleep( 3 );
  stop_timer( &start, &stop );
  print(glbAgent, "Sleep interval  -->   3 seconds\n" );
  print(glbAgent, "Timers report   -->  %8.5f seconds\n", timer_value( &stop ) );
  print(glbAgent, "---------------------------------------------------------------\n"); 

  return 1;

}

/*
*----------------------------------------------------------------------
*
* soar_ExplainBacktraces --
*
*----------------------------------------------------------------------
*/

int soar_ExplainBacktraces ( int argc, char *argv[], soarResult *res)
{
   
   if (argc == 1)
   {
      explain_list_chunks(glbAgent);
      clearSoarResultResult( res );
      return SOAR_OK;
   }
   
   {
      int cond_num;
      
      get_lexeme_from_string(argv[1]);
      
      if (current_agent(lexeme).type==SYM_CONSTANT_LEXEME) 
      {
         if (argc > 2) 
         {
            if ( string_match("-full", argv[2]))
            {
               /* handle the 'explain name -full' case */
               
               soar_ecExplainChunkTrace(current_agent(lexeme.string));
            }
            else if ( getInt( argv[2], &cond_num) == SOAR_OK)
            {
               /* handle the 'explain name <cond-num>' case */
               
               soar_ecExplainChunkCondition(current_agent(lexeme.string),
                  cond_num);
            }
            else
            {
               setSoarResultResult( res, 
                  "Unexpected argument to %s %s: %s.  Should be -full or integer.",
                  argv[0], argv[1], argv[2]);
               return SOAR_ERROR;
            }      
         }
         else
         {
            /* handle the 'explain name' case */
            
            soar_ecExplainChunkConditionList(current_agent(lexeme.string));
         }
      }
      else
      {
         setSoarResultResult( res,
            "Unexpected argument to explain-backtraces: %s.  Should be symbolic constant or integer.",
            argv[1]);
         return SOAR_ERROR;
      }
   }
   clearSoarResultResult( res );
   return SOAR_OK;
}



/*
*----------------------------------------------------------------------
*
* soar_Sp --
*
*----------------------------------------------------------------------
*/

int soar_Sp (int argc, char *argv[], soarResult *res)
{
   
   
   if (argc == 1)
   {
      setSoarResultResult(res, 
         "Too few arguments.\nUsage: sp {rule} sourceFile.");
      return SOAR_ERROR;
   }
   
   
   if (argc > 3)
   {
      setSoarResultResult(res, 
         "Too many arguments.\nUsage: sp {rule} sourceFile.");
      return SOAR_ERROR;
   }
   
   if ( argc == 2 )
      soar_ecSp( argv[1], NULL );
   else 
      soar_ecSp( argv[1], argv[2] );
   
   /* Errors are non-fatal, return SOAR_OK */
   clearSoarResultResult( res );
   return SOAR_OK;
}

/*
*----------------------------------------------------------------------
*
* soar_Print --
*
*----------------------------------------------------------------------
*/

int soar_Print (int argc, char *argv[], soarResult *res)
{
   static char * too_few_args  = "Too few arguments.\nUsage: print [-depth n] [-internal] arg*";
   
   Bool internal;
   Bool name_only, full_prod;
   Bool output_arg; /* Soar-Bugs #161 */
   Bool print_filename;  /* CUSP (B11) kjh */
   
   int depth;
   Symbol *id;
   wme *w;
   list *wmes;
   cons *c;
   int i,next_arg;
   
   internal = FALSE;
   depth = current_agent(default_wme_depth);   /* AGR 646 */
   name_only = FALSE;
   full_prod = FALSE;
   print_filename = FALSE;  /* kjh CUSP(B11) */
   output_arg = FALSE; /* Soar-Bugs #161 */
   
   
   
   if (argc == 1)
   {
      setSoarResultResult( res, too_few_args );
      return SOAR_ERROR;
   }
   
   clearSoarResultResult( res );
   
   
   next_arg = 1;
   
   /* --- see if we have the -stack option --- */
   if (string_match_up_to("-stack", argv[next_arg],4)) 
   {
      Bool print_states;
      Bool print_operators;
      
      /* Determine context items to print */
      
      if (argc == 2)                    /* No options given */
      {
         print_states    = TRUE;
         print_operators = TRUE;
      }
      else
      {
         int i;
         
         print_states    = FALSE;
         print_operators = FALSE;
         
         next_arg++;
         for (i = next_arg; i < argc; i++)
         {
            if (string_match_up_to(argv[i], "-states", 2))
               print_states = TRUE;
            else if (string_match_up_to(argv[i], "-operators", 2))
               print_operators      = TRUE;
            else
            {
               setSoarResultResult( res, 
                  "Unknown option passed to print -stack: (%s).\nUsage print -stack [-state | -operator]*",
                  argv[next_arg]);
               return SOAR_ERROR;
            }
         }
      }
      
      {
         Symbol *g;
         
         for (g = current_agent(top_goal); g != NIL; g = g->id.lower_goal) 
         {
            if (print_states)
            {
               print_stack_trace (glbAgent, g, g, FOR_STATES_TF, FALSE);
               print(glbAgent, "\n");
            }
            if (print_operators && g->id.operator_slot->wmes) 
            {
               print_stack_trace (glbAgent, g->id.operator_slot->wmes->value,
                  g, FOR_OPERATORS_TF, FALSE);
               print(glbAgent, "\n");
            }
         }
      }
      
      return SOAR_OK;      
   } /* End of string_match "-stack" */
   
   
   /* --- repeat: read one arg and print it --- */
   while (next_arg < argc)
   {
      
      /* --- read optional -depth flag --- */
      if (string_match_up_to("-depth", argv[next_arg],4))
      {
         if ((argc - next_arg) <= 1)
         {
            setSoarResultResult( res, too_few_args );
            return SOAR_ERROR;
         }
         else
         {
            if (getInt( argv[++next_arg], &depth) != SOAR_OK)
            {
               setSoarResultResult( res, 
                  "Integer expected after %s, not %s.",
                  argv[next_arg-1], argv[next_arg]);
               return SOAR_ERROR;
            }
         }
      }
      
      /* --- read optional -internal flag --- */
      else if (string_match_up_to("-internal", argv[next_arg],2))
      {
         internal = TRUE;
      }
      
      /* kjh CUSP(B11) begin */
      /* --- read optional -filename flag --- */
      
      else if (string_match_up_to("-filename", argv[next_arg],2)) {
         print_filename = TRUE;
      }
      /* kjh CUSP(B11) end */
      
      /* --- read optional -name flag --- */
      else if (string_match_up_to("-name", argv[next_arg],2))
      {
         name_only = TRUE;
         full_prod = FALSE; /* Soar-Bugs #161 */
      }
      /* --- read optional -full flag --- */
      else if (string_match_up_to("-full", argv[next_arg],2))
      {
         full_prod = TRUE;
         name_only = FALSE; /* Soar-Bugs #161 */
      }
      else if (string_match_up_to("-all", argv[next_arg],2))
      {
         output_arg = TRUE; /* Soar-Bugs #161 */
         for (i=0; i < NUM_PRODUCTION_TYPES; i++) {
            soar_ecPrintAllProductionsOfType( i, internal, 
               print_filename, full_prod );
         }
      }
      else if (string_match_up_to("-defaults", argv[next_arg],4))
      {
         output_arg = TRUE; /* Soar-Bugs #161 */
                            /* SW 042000: Using this new API function, all prods are
                            * printed in the reverse order they were loaded...
         */
         soar_ecPrintAllProductionsOfType( DEFAULT_PRODUCTION_TYPE,
            internal, print_filename, 
            full_prod );
         
      }
      else if (string_match_up_to("-user", argv[next_arg],2))
      {
         output_arg = TRUE; /* TEST for Soar-Bugs #161 */
         soar_ecPrintAllProductionsOfType( USER_PRODUCTION_TYPE,
            internal, print_filename,
            full_prod );
      }
      else if (string_match_up_to("-chunks", argv[next_arg],2))
      {
         output_arg = TRUE; /* Soar-Bugs #161 */
         soar_ecPrintAllProductionsOfType( CHUNK_PRODUCTION_TYPE,
            internal, print_filename,
            full_prod );
      }
      else if (string_match_up_to("-justifications", argv[next_arg],2))
      {
         output_arg = TRUE; /* Soar-Bugs #161 */
         soar_ecPrintAllProductionsOfType( JUSTIFICATION_PRODUCTION_TYPE,
            internal, print_filename,
            full_prod );
      }
      else if (string_match_up_to("-",argv[next_arg],1))
      {
         setSoarResultResult( res, 
            "Unrecognized option to print command: %s",
            argv[next_arg]);
         return SOAR_ERROR;
      }
      else { /* check for production name or wme */
         
         get_lexeme_from_string(argv[next_arg]);
         
         switch (current_agent(lexeme).type) {
         case SYM_CONSTANT_LEXEME:
            output_arg = TRUE; /* Soar-Bugs #161 */
            if (!name_only || print_filename) {
               /* kjh CUSP(B11) begin */
               do_print_for_production_name (argv[next_arg], internal, 
                  print_filename, full_prod);
            } else {
               print(glbAgent, "%s\n",argv[next_arg]);
            }
            break;
            
         case INT_CONSTANT_LEXEME:
            output_arg = TRUE; /* Soar-Bugs #161 */
            for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
               if (w->timetag == current_agent(lexeme).int_val) break;
               if (w) {
                  do_print_for_wme (w, depth, internal);
               } else {
                  setSoarResultResult( res, 
                     "No wme %ld in working memory", 
                     current_agent(lexeme).int_val);
                  return SOAR_ERROR;
               }
               break;
               
         case IDENTIFIER_LEXEME:
         case VARIABLE_LEXEME:
            
            output_arg = TRUE; /* Soar-Bugs #161 */
            id = read_identifier_or_context_variable();
            if (id) do_print_for_identifier (id, depth, internal);
            break;
            
         case QUOTED_STRING_LEXEME:
            output_arg = TRUE; /* Soar-Bugs #161 */
            /* Soar-Bugs #54 TMH */
            soar_alternate_input(glbAgent, argv[next_arg], ") ",TRUE);
            /*  print( "Set alternate Input to '%s'\n", argv[next_arg] ); */
            /* ((agent *)clientData)->alternate_input_string = argv[next_arg];
            * ((agent *)clientData)->alternate_input_suffix = ") ";
            */
            get_lexeme(glbAgent);
            wmes = read_pattern_and_get_matching_wmes ();
            soar_alternate_input(glbAgent, NIL, NIL, FALSE); 
            current_agent(current_char) = ' ';
            for (c=wmes; c!=NIL; c=c->rest)
               do_print_for_wme (static_cast<wme *>(c->first), depth, internal);
            free_list (glbAgent, wmes);
            break;
            
         default:
            setSoarResultResult( res, 
               "Unrecognized argument to print command: %s", argv[next_arg]);
            return SOAR_ERROR;
         } /* end of switch statement */
         output_arg = TRUE;  /* Soar-bugs #161 */
      } /* end of if-else stmt */
      next_arg++;
  } /* end of while loop */
  
  /* Soar-Bugs #161 */
  if (!output_arg) {
     setSoarResultResult( res,  too_few_args );
     return SOAR_ERROR;
  } else
     return SOAR_OK;
  
}

/*
*----------------------------------------------------------------------
*
* soar_Excise --
*
*----------------------------------------------------------------------
*/

int soar_Excise ( int argc, char *argv[], soarResult *res)
{
   int i;
   
   if (argc == 1)
   {
      setSoarResultResult( res, "No arguments given.\nUsage: excise production-name | -chunks | -default | -task | -user | -all");
      return SOAR_ERROR;
   }
   
   for (i = 1; i < argc; i++)
   {      
      if ( string_match_up_to(argv[i], "-chunks", 2)) {
         soar_cExciseAllProductionsOfType( CHUNK_PRODUCTION_TYPE );
         soar_cExciseAllProductionsOfType( JUSTIFICATION_PRODUCTION_TYPE );
      }
      else if ( string_match_up_to(argv[i], "-default", 2))
         soar_cExciseAllProductionsOfType( DEFAULT_PRODUCTION_TYPE );
      else if ( string_match_up_to(argv[i], "-task", 2))
         soar_cExciseAllTaskProductions(  );
      else if (string_match_up_to(argv[i], "-user", 2))
         soar_cExciseAllProductionsOfType( USER_PRODUCTION_TYPE );
      else if (string_match_up_to(argv[i], "-all", 2))
         soar_cExciseAllProductions( );
      else if ( soar_cExciseProductionByName( argv[i] ) ) 
      {
         setSoarResultResult( res, "Unrecognized Production name or argument: %s",
            argv[i] );
         return SOAR_ERROR;
      }
   }
   clearSoarResultResult( res );
   return SOAR_OK;
}

void do_print_for_production_name (char *prod_name, Bool internal,
				   Bool print_filename, Bool full_prod) 
{
  Symbol *sym;
  
  sym = find_sym_constant (glbAgent, current_agent(lexeme).string);
  if (sym && sym->sc.production) {
    /* kjh CUSP(B11) begin */  /* also Soar-Bugs #161 */
    if (print_filename) {
      if (full_prod) print_string(glbAgent, "# sourcefile : ");
      print(glbAgent, "%s\n", sym->sc.production->filename);
    }
    /* KJC added so get at least some output for any named productions */
    if ((full_prod) || (!print_filename)) {
      print_production (glbAgent, sym->sc.production, internal);
      print(glbAgent, "\n");
    } /* end CUSP B11 kjh */
  } else {
    print(glbAgent, "No production named %s\n", prod_name);
    print_location_of_most_recent_lexeme(glbAgent);
  }
}

void do_print_for_wme (wme *w, int depth, Bool internal) {
  tc_number tc;
  
  if (internal && (depth==0)) {
    print_wme (glbAgent, w);
    /* print(glbAgent, "\n"); */
  } else {
    tc = get_new_tc_number(glbAgent);
    print_augs_of_id (w->id, depth, internal, 0, tc);
  }
}

/*
 *----------------------------------------------------------------------
 *
 * read_attribute_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for an existing attribute.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_attribute_from_string (Symbol *id, char * the_lexeme, Symbol * * attr)
{
  Symbol *attr_tmp;
  slot *s;

  /* skip optional '^' if present.  KJC added to Ken's code */
  if (*the_lexeme == '^')
    {
      the_lexeme++;
    }
  
  get_lexeme_from_string(the_lexeme);

  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    attr_tmp = find_sym_constant (glbAgent, current_agent(lexeme).string);
    break;
  case INT_CONSTANT_LEXEME:
    attr_tmp = find_int_constant (glbAgent, current_agent(lexeme).int_val);
    break;
  case FLOAT_CONSTANT_LEXEME:
    attr_tmp = find_float_constant (glbAgent, current_agent(lexeme).float_val);
    break;
  case IDENTIFIER_LEXEME:
    attr_tmp = find_identifier (glbAgent, current_agent(lexeme).id_letter,
                                current_agent(lexeme).id_number);
    break;
  case VARIABLE_LEXEME:
    attr_tmp = read_identifier_or_context_variable();
    if (!attr_tmp)
      return FALSE;
    break;
  default:
    return FALSE;
  }
  s = find_slot (id, attr_tmp);
  if (s) {
    *attr = attr_tmp;
    return TRUE;
  } else
    return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * read_pref_detail_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for the detail level indicator for the 'preferences'
 *      command.  If so, it sets the_lexeme and wme_trace_type accordingly
 *      and returns TRUE; otherwise, it leaves those parameters untouched
 *      and returns FALSE.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_pref_detail_from_string (char *the_lexeme,
                                  Bool *print_productions,
                                  wme_trace_type *wtt)
{
  if (          string_match_up_to(the_lexeme, "-none", 3)
             || string_match(the_lexeme, "0")) {
    *print_productions = FALSE;
    *wtt               = NONE_WME_TRACE;
  } else if (   string_match_up_to(the_lexeme, "-names", 3)
	         || string_match(the_lexeme, "1")) {
    *print_productions = TRUE;
	*wtt               = NONE_WME_TRACE;
  } else if (   string_match_up_to(the_lexeme, "-timetags", 2)
	         || string_match(the_lexeme, "2")) {
    *print_productions = TRUE;
    *wtt               = TIMETAG_WME_TRACE;
  } else if (   string_match_up_to(the_lexeme, "-wmes", 2)
             || string_match(the_lexeme, "3")) {
    *print_productions = TRUE;
    *wtt               = FULL_WME_TRACE;
  } else {
    return FALSE;
  }
  return TRUE;
}

/* Routine for LHS. */
void read_pattern_and_get_matching_productions (list **current_pf_list, Bool show_bindings,
                                                Bool just_chunks,Bool no_chunks) {
  condition *c, *clist, *top, *bottom, *pc;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  Bool match, match_this_c;


  bindings = NIL;
  current_binding_point = NIL;

/*  print(glbAgent, "Parsing as a lhs...\n"); */
  clist = (condition *) parse_lhs(glbAgent);
  if (!clist) {
    print(glbAgent, "Error: not a valid condition list.\n");
    current_pf_list = NIL;
    return;
  }
/*
  print(glbAgent, "Valid condition list:\n");
  print_condition_list(clist,0,FALSE);
  print(glbAgent, "\nMatches:\n");
*/

  /* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
   */

  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
        (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
     for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL; prod=prod->next) {

      /* Now the complicated part. */
      /* This is basically a full graph-match.  Like the rete.  Yikes! */
      /* Actually it's worse, because there are so many different KINDS of
         conditions (negated, >/<, acc prefs, ...) */
      /* Is there some way I could *USE* the rete for this?  -- for lhs
         positive conditions, possibly.  Put some fake stuff into WM
         (i.e. with make-wme's), see what matches all of them, and then
         yank out the fake stuff.  But that won't work for RHS or for
         negateds.       */

      /* Also note that we need bindings for every production.  Very expensive
         (but don't necc. need to save them -- maybe can just print them as we go). */

      match = TRUE;
      p_node_to_conditions_and_nots (glbAgent, prod->p_node, NIL, NIL, &top, &bottom,
                                     NIL, NIL);

      free_binding_list(bindings);
      bindings = NIL;

      for (c=clist;c!=NIL;c=c->next) {
        match_this_c= FALSE;
        current_binding_point = bindings;

        for (pc = top; pc != NIL; pc=pc->next) {
          if (conditions_are_equal_with_bindings(c,pc,&bindings)) {
            match_this_c = TRUE;
            break;}
          else {
            /* Remove new, incorrect bindings. */
            reset_old_binding_point(&bindings,&current_binding_point);
            bindings= current_binding_point;
	  }
	}
        if (!match_this_c) {match = FALSE; break;}
      }
      deallocate_condition_list (glbAgent, top); /* DJP 4/3/96 -- Never dealloced */
      if (match) {
        push(glbAgent, prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols(glbAgent, "%y, with bindings:\n",prod->name);
          print_binding_list(bindings);}
        else
          print_with_symbols(glbAgent, "%y\n",prod->name);
      }
     }
  if (bindings) free_binding_list(bindings); /* DJP 4/3/96 -- To catch the last production */
}

/* Routine for RHS. */
void read_rhs_pattern_and_get_matching_productions (list **current_pf_list, Bool show_bindings,
                                                    Bool just_chunks, Bool no_chunks) {
  action *a, *alist, *pa;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  Bool match, match_this_a, parsed_ok;
  action *rhs; 
  condition *top_cond, *bottom_cond;

  bindings = NIL;
  current_binding_point = NIL;

/*  print(glbAgent, "Parsing as a rhs...\n"); */
  parsed_ok=parse_rhs(glbAgent, &alist);
  if (!parsed_ok) {
    print(glbAgent, "Error: not a valid rhs.\n");
    current_pf_list = NIL;
    return;
  }

/*
  print(glbAgent, "Valid RHS:\n");
  print_action_list(glbAgent, alist,0,FALSE);
  print(glbAgent, "\nMatches:\n");
*/

  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
        (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
     for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL;
          prod=prod->next) {
      match = TRUE;

      free_binding_list(bindings);
      bindings = NIL;

      p_node_to_conditions_and_nots (glbAgent, prod->p_node, NIL, NIL, &top_cond,
                                     &bottom_cond, NIL, &rhs);
      deallocate_condition_list (glbAgent, top_cond);
      for (a=alist;a!=NIL;a=a->next) {
        match_this_a= FALSE;
        current_binding_point = bindings;

        for (pa = rhs; pa != NIL; pa=pa->next) {
          if (actions_are_equal_with_bindings(a,pa,&bindings)) {
            match_this_a = TRUE;
            break;}
          else {
            /* Remove new, incorrect bindings. */
            reset_old_binding_point(&bindings,&current_binding_point);
            bindings= current_binding_point;
          }
        }
        if (!match_this_a) {match = FALSE; break;}
      }
      deallocate_action_list (glbAgent, rhs);
      if (match) {
        push(glbAgent, prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols(glbAgent, "%y, with bindings:\n",prod->name);
          print_binding_list(bindings);}
        else
          print_with_symbols(glbAgent, "%y\n",prod->name);
      }
    }
  if (bindings) free_binding_list(bindings); /* DJP 4/3/96 -- To catch the last production */
}

list *read_pattern_and_get_matching_wmes (void) {
  int parentheses_level;
  list *wmes;
  wme *w;
  Symbol *id, *attr, *value;
  int id_result, attr_result, value_result;
  Bool acceptable;
  
  if (current_agent(lexeme).type!=L_PAREN_LEXEME) {
    print(glbAgent, "Expected '(' to begin wme pattern not string '%s' or char '%c'\n", 
		   current_agent(lexeme).string, current_agent(current_char));
    print_location_of_most_recent_lexeme(glbAgent);
    return NIL;
  }
  parentheses_level = current_lexer_parentheses_level(glbAgent);

  get_lexeme(glbAgent);
  id_result = read_pattern_component (&id);
  if (! id_result) {
    skip_ahead_to_balanced_parentheses (glbAgent, parentheses_level-1);
    return NIL;
  }
  get_lexeme(glbAgent);
  if (current_agent(lexeme).type!=UP_ARROW_LEXEME) {
    print(glbAgent, "Expected ^ in wme pattern\n");
    print_location_of_most_recent_lexeme(glbAgent);
    skip_ahead_to_balanced_parentheses (glbAgent, parentheses_level-1);
    return NIL;
  }
  get_lexeme(glbAgent);
  attr_result = read_pattern_component (&attr);
  if (! attr_result) {
    skip_ahead_to_balanced_parentheses (glbAgent, parentheses_level-1);
    return NIL;
  }
  get_lexeme(glbAgent);
  value_result = read_pattern_component (&value);
  if (! value_result) {
    skip_ahead_to_balanced_parentheses (glbAgent, parentheses_level-1);
    return NIL;
  }
  get_lexeme(glbAgent);
  if (current_agent(lexeme).type==PLUS_LEXEME) {
    acceptable = TRUE;
    get_lexeme(glbAgent);
  } else {
    acceptable = FALSE;
  }
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print(glbAgent, "Expected ')' to end wme pattern\n");
    print_location_of_most_recent_lexeme(glbAgent);
    skip_ahead_to_balanced_parentheses (glbAgent, parentheses_level-1);
    return NIL;
  }
  {
	int i  = 0;
  wmes = NIL;
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next) {
    if ((id_result==1) || (id==w->id))
      if ((attr_result==1) || (attr==w->attr))
        if ((value_result==1) || (value==w->value))
          if (acceptable==w->acceptable) {
            push (glbAgent, w, wmes);
			i++;
		  }
  
  }
  /* printf( "--- FOUND %d matching wmes\n", i ); */
  }
  return wmes;  
}

/* This should be added to the Soar kernel.  It must be re-written to
 * take a string to parse.
 */

int read_pattern_component (Symbol **dest_sym) {
  /* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
     otherwise return 2 and set dest_sym to find_symbol() result. --- */
  if (strcmp(current_agent(lexeme).string,"*") == 0) return 1;
  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    *dest_sym = find_sym_constant (glbAgent, current_agent(lexeme).string); return 2;
  case INT_CONSTANT_LEXEME:
    *dest_sym = find_int_constant (glbAgent, current_agent(lexeme).int_val); return 2;
  case FLOAT_CONSTANT_LEXEME:
    *dest_sym = find_float_constant (glbAgent, current_agent(lexeme).float_val); return 2;
  case IDENTIFIER_LEXEME:
    *dest_sym = find_identifier (glbAgent, current_agent(lexeme).id_letter, current_agent(lexeme).id_number); return 2;
  case VARIABLE_LEXEME:
    *dest_sym = read_identifier_or_context_variable();
    if (*dest_sym) return 2;
    return 0;
  default:
    print(glbAgent, "Expected identifier or constant in wme pattern\n");
    print_location_of_most_recent_lexeme(glbAgent);
    return 0;
  }
}

/*
*----------------------------------------------------------------------
*
* soar_OSupportMode --
*
*----------------------------------------------------------------------
*/

int soar_OSupportMode (int argc, char *argv[], soarResult *res)
{
   
   if (argc > 2)
   {
      setSoarResultResult( res, 
         "Too many arguments.\nUsage: o-support-mode 0|1|2|3");
      return SOAR_ERROR;
   }
   
   if (argc == 2) {
      if (! strcmp(argv[1], "0")) {
         current_agent(o_support_calculation_type) = 0;
      } else if (! strcmp(argv[1], "1")) {
         current_agent(o_support_calculation_type) = 1;
      } else if (! strcmp(argv[1], "2")) {
         current_agent(o_support_calculation_type) = 2;
      } else if (! strcmp(argv[1], "3")) {
         /* Note: Eventually, this 3rd option should be removed, as it is not
		    supported in Soar 8.3. */
		 current_agent(o_support_calculation_type) = 3; 
      } else {
         setSoarResultResult( res, 
            "Unrecognized argument to %s: %s.  Integer 0, 1, 2, or 3 expected.",
            argv[0], argv[1]);
         return SOAR_ERROR;
      }
   }
   
   setSoarResultResult( res, 
      "%d", current_agent(o_support_calculation_type));
   return SOAR_OK;
}

#ifdef NULL_ACTIVATION_STATS

void null_activation_stats_for_right_activation (rete_node *node, rete_node *node_to_ignore_for_null_activation_stats)
{
  if (node==node_to_ignore_for_activation_stats) return;
  switch (node->node_type) {
  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    current_agent(num_right_activations)++;
    if (! node->parent->a.np.tokens)
      current_agent(num_null_right_activations)++;
    break;
  case MP_BNODE:
  case UNHASHED_MP_BNODE:
    current_agent(num_right_activations)++;
    if (! node->a.np.tokens) current_agent(num_null_right_activations)++;
    break;
  }
}

void null_activation_stats_for_left_activation (rete_node *node) {
  switch (node->node_type) {
  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    current_agent(num_left_activations)++;
    if (node->b.posneg.alpha_mem->right_mems==NIL)
      current_agent(num_null_left_activations)++;
    break;
  case MP_BNODE:
  case UNHASHED_MP_BNODE:
    if (mp_bnode_is_left_unlinked(node)) return;
    current_agent(num_left_activations)++;
    if (node->b.posneg.alpha_mem->right_mems==NIL)
      current_agent(num_null_left_activations)++;
    break;
  }
}

void print_null_activation_stats () {
  print(glbAgent, "\nActivations: %lu right (%lu null), %lu left (%lu null)\n",
         current_agent(num_right_activations),
         current_agent(num_null_right_activations),
         current_agent(num_left_activations),
         current_agent(num_null_left_activations));
}

#endif

#ifdef __cplusplus
extern "C"
{
#endif

//void Soar_Print (agent * the_agent, char * str)
//{
//  soar_invoke_first_callback(glbAgent, the_agent, PRINT_CALLBACK, /*(ClientData)*/ (void *) str);
//}
//
//void Soar_Log (agent * the_agent, char * str)
//{
//  soar_invoke_first_callback(glbAgent, the_agent, LOG_CALLBACK, /*(ClientData)*/ (void *) str);
//} 

//void Soar_LogAndPrint (agent * the_agent, char * str)
//{
//  Soar_Log(the_agent, str);
//  Soar_Print(the_agent, str);
//}
//
//Bool wme_filter_component_match(Symbol *filterComponent, Symbol *wmeComponent) {
//  if ((filterComponent->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
//      (!strcmp(filterComponent->sc.name,"*"))) 
//    return TRUE;
//  else
//    return(filterComponent == wmeComponent);
//}
//
//Bool passes_wme_filtering(agent* thisAgent, wme *w, Bool isAdd) {
//  cons *c;
//  wme_filter *wf;
//
//  /*  print(glbAgent, "testing wme for filtering: ");  print_wme(w); */
//  
//  if (!current_agent(wme_filter_list))
//    return TRUE; /* no filters defined -> everything passes */
//  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
//    wf = (wme_filter *) c->first;
//    /*     print_with_symbols("  trying filter: %y ^%y %y\n",wf->id,wf->attr,wf->value); */
//    if (   ((isAdd && wf->adds) || ((!isAdd) && wf->removes))
//        && wme_filter_component_match(wf->id,w->id)
//        && wme_filter_component_match(wf->attr,w->attr)
//        && wme_filter_component_match(wf->value,w->value))
//      return TRUE;
//  }
//  return FALSE; /* no defined filters match -> w passes */
//}

int GDS_PrintCmd (/****ClientData****/ int clientData, 
                  /****Tcl_Interp****/ void * interp,
                  int argc, char *argv[])
{
   return false;
}

#ifdef __cplusplus
}
#endif

#ifdef ATTENTION_LAPSE

/* RMJ */
/*
 *----------------------------------------------------------------------
 *
 * print_current_attention_lapse_settings --
 *
 *	This procedure prints the current attention_lapse setting.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current setting.
 *
 *----------------------------------------------------------------------
 */

void print_current_attention_lapse_settings(void)
{
  print(glbAgent, "Current attention-lapse setting:\n");
  print(glbAgent, "   %s\n", current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] ? "-on" : "-off");
 
}

#endif /* ATTENTION_LAPSE */

/*
 *----------------------------------------------------------------------
 *
 * name_to_production --
 *
 *	This procedure determines if a string matches an existing
 *      production name.  If so, the production is returned.
 *
 * Results:
 *	Returns the production if a match is found, NIL otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

production * name_to_production (char * string_to_test)
{
  Symbol * sym;

  sym = find_sym_constant(glbAgent, string_to_test);
  
  if (sym && sym->sc.production)
    return sym->sc.production;
  else
    return NIL;
}

Bool funcalls_match(list *fc1, list *fc2) {
  /* I have no idea how to do this. */
  return FALSE;
}

Symbol *get_binding (Symbol *f, list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest) {
    if (((Binding *) c->first)->from == f)
      return ((Binding *) c->first)->to;
  }
  return NIL;
}

void reset_old_binding_point(list **bindings, list **current_binding_point) {
  cons *c,*c_next;

  c = *bindings;
  while (c != *current_binding_point) {
    c_next = c->rest;
    free_memory(glbAgent, c->first,MISCELLANEOUS_MEM_USAGE);
    free_cons (glbAgent, c);
    c = c_next;
  }

  bindings = current_binding_point;
}

void free_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    free_memory(glbAgent, c->first,MISCELLANEOUS_MEM_USAGE);
  free_list(glbAgent, bindings);
}

void print_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    print_with_symbols (glbAgent, "   (%y -> %y)\n",((Binding *) c->first)->from,((Binding *) c->first)->to);
}

Bool symbols_are_equal_with_bindings (Symbol *s1, Symbol *s2, list **bindings) {
  Binding *b;
  Symbol *bvar;

  /* SBH/MVP 7-5-94 */
  if ((s1 == s2) && (s1->common.symbol_type != VARIABLE_SYMBOL_TYPE))
    return TRUE;

  /* "*" matches everything. */
  if ((s1->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
      (!strcmp(s1->sc.name,"*"))) return TRUE;
  if ((s2->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
      (!strcmp(s2->sc.name,"*"))) return TRUE;


  if ((s1->common.symbol_type != VARIABLE_SYMBOL_TYPE) ||
      (s2->common.symbol_type != VARIABLE_SYMBOL_TYPE))
    return FALSE;
  /* Both are variables */
  bvar = get_binding(s1,*bindings);
  if (bvar == NIL) {
    b = (Binding *) allocate_memory(glbAgent, sizeof(Binding),MISCELLANEOUS_MEM_USAGE);
    b->from = s1;
    b->to = s2;
    push(glbAgent, b,*bindings);
    return TRUE;
  }
  else if (bvar == s2) {
    return TRUE;
  }
  else return FALSE;
}

/* ----------------------------------------------------------------
   Returns TRUE iff the two tests are identical given a list of bindings.
   Augments the bindings list if necessary./
---------------------------------------------------------------- */

/* DJP 4/3/96 -- changed t2 to test2 in declaration */
Bool tests_are_equal_with_bindings (test t1, test test2, list **bindings) {
  cons *c1, *c2;
  complex_test *ct1, *ct2;
  Bool goal_test,impasse_test;

  /* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
  /*               and sometimes it wasn't.  If it was copied, the copy was never */
  /*               deallocated.  There's a few choices about how to fix this.  I  */
  /*               decided to just create a copy always and then always           */
  /*               deallocate it before returning.  Added a macro to do that.     */

  test t2;

  /* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
  if (test_is_blank_test(t1)) return(test_is_blank_test(test2));

  /* If the pattern doesn't include "(state", but the test from the
     production does, strip it out of the production's. */
  if ((!test_includes_goal_or_impasse_id_test(t1,TRUE,FALSE)) &&
      test_includes_goal_or_impasse_id_test(test2,TRUE,FALSE)) {
    goal_test = FALSE;
    impasse_test = FALSE;
    t2 = copy_test_removing_goal_impasse_tests(glbAgent, test2, &goal_test, &impasse_test);
  }
  else
    t2 = copy_test(glbAgent, test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */

  if (test_is_blank_or_equality_test(t1)) {
    if (!(test_is_blank_or_equality_test(t2) && !(test_is_blank_test(t2)))) dealloc_and_return(t2,FALSE)
    else {
      if (symbols_are_equal_with_bindings(referent_of_equality_test(t1),
					  referent_of_equality_test(t2),
					  bindings))
	dealloc_and_return(t2,TRUE)
      else
	dealloc_and_return(t2,FALSE)
    }
  }

  ct1 = complex_test_from_test(t1);
  ct2 = complex_test_from_test(t2);

  if (ct1->type != ct2->type) dealloc_and_return(t2,FALSE)

  switch(ct1->type) {
  case GOAL_ID_TEST: dealloc_and_return(t2,TRUE)
  case IMPASSE_ID_TEST: dealloc_and_return(t2,TRUE)

  case DISJUNCTION_TEST:
    for (c1=ct1->data.disjunction_list, c2=ct2->data.disjunction_list;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->rest, c2=c2->rest)
      if (c1->first != c2->first) dealloc_and_return(t2,FALSE)
    if (c1==c2) dealloc_and_return(t2,TRUE)  /* make sure they both hit end-of-list */
    dealloc_and_return(t2,FALSE)

  case CONJUNCTIVE_TEST:
    for (c1=ct1->data.conjunct_list, c2=ct2->data.conjunct_list;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->rest, c2=c2->rest)
      if (! tests_are_equal_with_bindings(static_cast<char *>(c1->first),static_cast<char *>(c2->first),bindings)) dealloc_and_return(t2,FALSE)
    if (c1==c2) dealloc_and_return(t2,TRUE)  /* make sure they both hit end-of-list */
    dealloc_and_return(t2,FALSE)

  default:  /* relational tests other than equality */
    if (symbols_are_equal_with_bindings(ct1->data.referent,ct2->data.referent,bindings)) dealloc_and_return(t2,TRUE)
    dealloc_and_return(t2,FALSE)
  }
}

Bool conditions_are_equal_with_bindings (condition *c1, condition *c2, list **bindings) {
  if (c1->type != c2->type) return FALSE;
  switch (c1->type) {
  case POSITIVE_CONDITION:
  case NEGATIVE_CONDITION:
    if (! tests_are_equal_with_bindings (c1->data.tests.id_test,
                           c2->data.tests.id_test,bindings))
      return FALSE;
    if (! tests_are_equal_with_bindings (c1->data.tests.attr_test,
                           c2->data.tests.attr_test,bindings))

      return FALSE;
    if (! tests_are_equal_with_bindings (c1->data.tests.value_test,
                           c2->data.tests.value_test,bindings))
      return FALSE;
    if (c1->test_for_acceptable_preference !=
        c2->test_for_acceptable_preference)
      return FALSE;
    return TRUE;

  case CONJUNCTIVE_NEGATION_CONDITION:
    for (c1=c1->data.ncc.top, c2=c2->data.ncc.top;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->next, c2=c2->next)
      if (! conditions_are_equal_with_bindings (c1,c2,bindings)) return FALSE;
    if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
    return FALSE;
  }
  return FALSE; /* unreachable, but without it, gcc -Wall warns here */
}

Bool actions_are_equal_with_bindings (action *a1, action *a2, list **bindings) {
  if (a1->type == FUNCALL_ACTION) {
    if ((a2->type == FUNCALL_ACTION)) {
      if (funcalls_match(rhs_value_to_funcall_list(a1->value),
                         rhs_value_to_funcall_list(a2->value))) {
        return TRUE;}
      else return FALSE;
    }
    else return FALSE;
  }
  if (a2->type == FUNCALL_ACTION) return FALSE;

  /* Both are make_actions. */

  if (a1->preference_type != a2->preference_type) return FALSE;

  if (!symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->id),
                                       rhs_value_to_symbol(a2->id),
                                       bindings)) return FALSE;

  if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr))) {
    if (!symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->attr),
					     rhs_value_to_symbol(a2->attr),
					     bindings)) return FALSE;
  } else {
    if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr))) {
      if (!funcalls_match(rhs_value_to_funcall_list(a1->attr),
                              rhs_value_to_funcall_list(a2->attr)))
        return FALSE;
    }
  }

  /* Values are different. They are rhs_value's. */

  if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value))) {
    if (symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->value),
                                         rhs_value_to_symbol(a2->value),
                                         bindings)) return TRUE;
    else return FALSE;
  }
  if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value))) {
    if (funcalls_match(rhs_value_to_funcall_list(a1->value),
                       rhs_value_to_funcall_list(a2->value)))
      return TRUE;
    else return FALSE;}
  return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * print_production_firings --
 *
 *	This procedure prints a list of the top production firings.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Prints the productions and their firing counts.
 *
 *----------------------------------------------------------------------
 */

int print_production_firings (int num_requested)
{
  int  i;
  long num_prods;
  production *((*all_prods)[]), **ap_item, *p;
  
  num_prods = current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE];

  if (num_prods == 0) 
    {
      printf("\nNo productions defined.\n");
      return TRUE;                     /* so we don't barf on zero later */
    }

  /* --- make an array of pointers to all the productions --- */
  all_prods = static_cast<production *(*)[]>(allocate_memory (glbAgent, num_prods * sizeof (production *),
                               MISCELLANEOUS_MEM_USAGE));

  /* MVP - 6-8-94 where is it freed ? */

  ap_item = &((*all_prods)[0]);
  for (p=current_agent(all_productions_of_type)[DEFAULT_PRODUCTION_TYPE]; 
       p!=NIL; 
       p=p->next)
    *(ap_item++) = p;
  for (p=current_agent(all_productions_of_type)[USER_PRODUCTION_TYPE]; 
       p!=NIL; 
       p=p->next)
    *(ap_item++) = p;
  for (p=current_agent(all_productions_of_type)[CHUNK_PRODUCTION_TYPE]; 
       p!=NIL; 
       p=p->next)
    *(ap_item++) = p;
  

  /* --- now print out the results --- */
  if (num_requested == 0)
    {
      /* print only the productions that have never fired. */
      ap_item = &((*all_prods)[0]);
      for (i = 0; i < num_prods; i++)
	{
	  if ((*ap_item)->firing_count == 0)
	    {
	      print_with_symbols (glbAgent, "%y\n", (*ap_item)->name);
	    }
	  ap_item++;
	}
    }
  else
    {
      /* --- sort array according to firing counts --- */
      qsort (all_prods, num_prods, sizeof (production *), 
	     compare_firing_counts);

      if ((num_requested < 0) || (num_requested > num_prods))
	{
	  num_requested = num_prods;
	}
      
      ap_item = &((*all_prods)[num_prods-1]);
      while (num_requested) {
	print(glbAgent, "%6lu:  ", (*ap_item)->firing_count);
	print_with_symbols (glbAgent, "%y\n", (*ap_item)->name);
	ap_item--;
	num_requested--;
      }
    }

  /* MVP 6-8-94 also try this to plug memory leak */
  free_memory(glbAgent, all_prods, MISCELLANEOUS_MEM_USAGE);

  return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * compare_firing_counts --
 *
 *	This procedure compares two productions to determine
 *      how they compare vis-a-vis firing counts.
 *
 * Results:
 *	Returns -1 if the first production has fired less
 *               0 if they two productions have fired the same
 *               1 if the first production has fired more
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int compare_firing_counts (const void * e1, const void * e2)
{
  production *p1, *p2;
  unsigned long count1, count2;

  p1 = *((production **)e1);
  p2 = *((production **)e2);

  count1 = p1->firing_count;
  count2 = p2->firing_count;

  return (count1<count2) ? -1 : (count1>count2) ? 1 : 0;
}
