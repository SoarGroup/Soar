/*
 * =======================================================================
 *  File:  soarCommandUtils.c
 *
 * This file includes the utility routines for supporting the
 * Soar Command set, which is found in soarCommands.c.  
 *
 * =======================================================================
 *
 *
 * Copyright 1995-2003 Carnegie Mellon University,
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


#include <ctype.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "soar.h"
#include "soarCommandUtils.h"

#define MAXPATHLEN 1024

/* DJP 4/3/96 */
#define dealloc_and_return(x,y) { deallocate_test(x) ; return (y) ; }

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

void add_multi_attribute_or_change_value(char *sym, long val) {
  multi_attribute *m = current_agent(multi_attributes);
  Symbol *s = make_sym_constant(sym);

  while(m) {
    if(m->symbol == s) {
      m->value = val;
      symbol_remove_ref(s);
      return;
    }
    m = m->next;
  }
  /* sym wasn't in the table if we get here, so add it */
  m = (multi_attribute *)allocate_memory(sizeof(multi_attribute),
                                         MISCELLANEOUS_MEM_USAGE);
  m->value = val;
  m->symbol = s;
  m->next = current_agent(multi_attributes);
  current_agent(multi_attributes) = m;
}

void capture_input_wme(wme *w, 
					   char *id, char *attr, char *value, 
					   char *flag) {
	FILE *f;
 
	f= current_agent(capture_fileID);

	if (strcmp(flag,"add") == 0) {
 		/* add the wme to the captured input file */
		fprintf(f,"%d : %d : add-wme %s %s %s\n",current_agent(d_cycle_count),
			    w->timetag, id, attr, value);  
	} else if (strcmp(flag,"remove") == 0) {
		/* write  the timetag; written twice so parsing easier...blech */
		fprintf(f,"%d : %d : remove-wme %d\n",current_agent(d_cycle_count), 
			    w->timetag, w->timetag);
		/* might be better to save id/attr/val and search, just in case
		wme exists with different timetag in later run...we'll cheat for now. */
	}
}

void replay_input_wme (agent * agent, int dummy, int mode) {
	/* this routine is registered as a callback when user issues a
	 * "replay-input -open fname" command in Soar.  The file is
	 * opened in the interface routine ReplayInputCmd.
	 */

	FILE *f;
	bool eof, readnext, wme_added;
  	int numargs;
        unsigned long new_timetag, old_timetag, cycle; 
	long loc;

 	/* char id[256], attr[256], value[256]; */
	char input[1024], cmd[1024], result[1024];

	/* print("\nin replay_input_wme\n"); */
	f= current_agent(replay_fileID);
	eof = FALSE;
	readnext = TRUE;
	loc = ftell(f);
	/* really should switch on input-link mode...ignore for now */
	while ((!feof(f)) && (readnext)) { 
		numargs = fscanf(f,"%i : %i :", &cycle, &old_timetag);
		/* print("\n\t numargs = %d \t cycle = %d \t timetag = %d\n",
		         numargs,cycle,old_timetag); */
                if (cycle <= current_agent(d_cycle_count)) {
			fgets(input,1024,f);
			/* if this is a remove-wme, we need to use the "old" timetag */
			if (strstr(input,"remove-wme")) {
				sprintf(cmd,"remove-wme %d\n",
					current_agent(replay_timetags)[old_timetag]);
			} else {
				if (strstr(input,"add-wme")) wme_added = TRUE;
				/* add-wme or other can just be copied to be eval'd */
				sprintf(cmd,"%s\n",input);
			}
			Tcl_Eval((Tcl_Interp *)current_agent(interpreter),cmd);
			strcpy(result,((Tcl_Interp *)current_agent(interpreter))->result);
			print("\nfrom replay file: %s\n", input);
			/* if cmd was add-wme, need to compare and index the timetags */
			if (wme_added) {
				numargs = sscanf(result,"%d: ", &new_timetag);
				print("wme_added: numargs = %d\n",numargs);
				current_agent(replay_timetags)[old_timetag] = new_timetag;
				if (old_timetag != new_timetag) 
					print("\n\nWARNING: timetags from replay file differ from current run!!");
				print("          result: %s\n", result);
			}
			loc = ftell(f);
		} else {
			fseek(f,loc,SEEK_SET);
			readnext = FALSE;
		}
	} 
	if (feof(f)) {
		/* warn user; turn off replay;  close file */
		strcpy(cmd,"echo WARNING!!!  Replay end of file reached.  Closing file.\n");
		Tcl_Eval((Tcl_Interp *)current_agent(interpreter),cmd);
		strcpy(cmd,"replay-input -close\n");
		Tcl_Eval((Tcl_Interp *)current_agent(interpreter),cmd);
	}
}

Symbol * space_to_remove_from_cfps;

bool cfps_removal_test_function (cons *c) {
  return (c->first == space_to_remove_from_cfps);
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


void do_print_for_identifier (Symbol *id, int depth, bool internal) {
  tc_number tc;

  tc = get_new_tc_number();
  print_augs_of_id (id, depth, internal, 0, tc);
}


/* CUSP B11 - Soar should know what file each production comes from */
void do_print_for_production (production *prod, bool internal, 
			      bool print_filename, bool full_prod) 
{
  if (!full_prod) {
    print_with_symbols("%y  ",prod->name);
  }
  if (print_filename) {
    print_string("# sourcefile : ");
    if (prod->filename) {
      print_string(prod->filename);
    } else {
      print_string(" _unknown_ ");
    }
  }
  print("\n");
  if (full_prod) {
    print_production (prod, internal);
    print("\n");
  }
}

void do_print_for_production_name (char *prod_name, bool internal,
				   bool print_filename, bool full_prod) 
{
  Symbol *sym;
  
  sym = find_sym_constant (current_agent(lexeme).string);
  if (sym && sym->sc.production) {
    /* kjh CUSP(B11) begin */  /* also Soar-Bugs #161 */
    if (print_filename) {
      if (full_prod) print_string("# sourcefile : ");
      print ("%s\n", sym->sc.production->filename);
    }
    /* KJC added so get at least some output for any named productions */
    if ((full_prod) || (!print_filename)) {
      print_production (sym->sc.production, internal);
      print ("\n");
    } /* end CUSP B11 kjh */
  } else {
    print ("No production named %s\n", prod_name);
    print_location_of_most_recent_lexeme();
  }
}

void do_print_for_wme (wme *w, int depth, bool internal) {
  tc_number tc;
  
  if (internal && (depth==0)) {
    print_wme (w);
    print ("\n");
  } else {
    tc = get_new_tc_number();
    print_augs_of_id (w->id, depth, internal, 0, tc);
  }
}

/* This should be added to the Soar kernel. */
void excise_all_productions_of_type (byte type) {
  while (current_agent(all_productions_of_type)[type])
    excise_production (current_agent(all_productions_of_type)[type],
		       (bool)(TRUE&&current_agent(sysparams)[TRACE_LOADING_SYSPARAM])); 
}

void execute_go_selection (agent * the_agent,
			   long go_number, 
			   enum go_type_enum go_type,
			   Symbol * go_slot_attr, 
			   goal_stack_level go_slot_level)
{
  switch (go_type) 
    {
    case GO_PHASE:
      run_for_n_phases (go_number);
      break;
    case GO_ELABORATION:
      run_for_n_elaboration_cycles (go_number);
      break;
    case GO_DECISION:
      run_for_n_decision_cycles (go_number);
      break;
    case GO_STATE:
      run_for_n_selections_of_slot (go_number, the_agent->state_symbol);
      break;
    case GO_OPERATOR:
      run_for_n_selections_of_slot (go_number, the_agent->operator_symbol);
      break;
    case GO_SLOT:
      run_for_n_selections_of_slot_at_level (go_number, go_slot_attr,
					     go_slot_level);
      break;
    case GO_OUTPUT:
      run_for_n_modifications_of_output (go_number);
      break;
    }
}

/* ===================================================================
                       Get Context Var Info

   This utility routine is used by interface routines that take context
   variable arguments (e.g., <s> for the current state).  It looks at
   the current lexeme (which must be of type VARIABLE_LEXEME), and
   checks to see if it's a context variable.  Returns:

    if lexeme is not a context variable, dest_attr_of_slot=NIL; else
      dest_attr_of_slot = {goal_symbol, problem_space_symbol, etc.}.
      dest_goal = goal identifier for the given slot (NIL if no such goal)
      dest_current_value = currently installed value (goal id itself for goals,
                           NIL if no installed value)
=================================================================== */

void get_context_var_info (Symbol **dest_goal,
                           Symbol **dest_attr_of_slot,
                           Symbol **dest_current_value) {
  Symbol *v, *g;
  int levels_up;
  wme *w;

  v = find_variable (current_agent(lexeme).string);
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

/* This routine was moved to command_utils.c for Tcl.  However, it cannot
 * reside outside the Soar kernel since it references lexeme.  This should
 * be rewritten to take a string argument and added to the Soar kernel
 * interface file (whatever that will be).
 */

#ifndef USE_TCL
void get_context_var_info (Symbol **dest_goal,
                           Symbol **dest_attr_of_slot,
                           Symbol **dest_current_value) {
  Symbol *v, *g;
  int levels_up;
  wme *w;
  
  v = find_variable (current_agent(lexeme).string);
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
#endif


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

void get_lexeme_from_string (char * the_lexeme)
{
  int i;
  char * c;
  bool sym_constant_start_found = FALSE;
  bool sym_constant_end_found = FALSE;

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
      determine_type_of_constituent_string();
    }
}

/*
 *----------------------------------------------------------------------
 *
 * install_tcl_soar_cmd --
 *
 *	This procedure installs a new Soar command in the Tcl
 *      interpreter.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	Command is installed in the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

void
install_tcl_soar_cmd (agent * the_agent, 
		      char * cmd_name, 
		      Tcl_CmdProc * cmd_proc)
{
  Tcl_CreateCommand(the_agent->interpreter, 
		    cmd_name, cmd_proc, 
		    (ClientData) the_agent, NULL);
}


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

  sym = find_sym_constant(string_to_test);
  
  if (sym && sym->sc.production)
    return sym->sc.production;
  else
    return NIL;
}

/* This should probably be in the Soar kernel interface. */
void neatly_print_wme_augmentation_of_id (wme *w, int indentation) {
  char buf[10000], *ch;

  strcpy (buf, " ^");
  ch = buf;
  while (*ch) ch++;
  symbol_to_string (w->attr, TRUE, ch); while (*ch) ch++;
  *(ch++) = ' ';
  symbol_to_string (w->value, TRUE, ch); while (*ch) ch++;
  if (w->acceptable) { strcpy (ch, " +"); while (*ch) ch++; }

  if (get_printer_output_column() + (ch - buf) >= 80) {
    print ("\n");
    print_spaces (indentation+6);
  }
  print_string (buf);
}

int parse_go_command (Tcl_Interp * interp, 
		      int argc, char * argv[],
		      long * go_number, 
		      enum go_type_enum * go_type,
		      Symbol * * go_slot_attr, 
		      goal_stack_level * go_slot_level) 
{
  Symbol *g, *attr, *value;
  int i;

  for (i = 1; i < argc; i++)
    {
      get_lexeme_from_string(argv[i]);

      if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) 
	{
	  *go_number = current_agent(lexeme).int_val;
	}
      else if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) 
	{
	  if (string_match("forever", argv[i]))
	    {
	      *go_number = -1;
	    }
	  else if (string_match("p", argv[i]))
	    {
	      *go_type = GO_PHASE;
	    }
	  else if (string_match("e", argv[i]))
	    {
	      *go_type = GO_ELABORATION;
	    }
	  else if (string_match("d", argv[i]))
	    {
	      *go_type = GO_DECISION;
	    }
	  else if (string_match("s", argv[i]))
	    {
	      *go_type = GO_STATE;
	    }
	  else if (string_match("o", argv[i]))
	    {
	      *go_type = GO_OPERATOR;
	    }
          else
	    {
              sprintf(interp->result,
		      "Unrecognized argument to go command: %s",
		      argv[i]);
              return TCL_ERROR;
            }
	}
      else if (current_agent(lexeme).type == VARIABLE_LEXEME) 
	{
	  get_context_var_info (&g, &attr, &value);
	  if (!attr) 
	    {
	      sprintf(interp->result,
		      "Expected a context variable in go command, not %s",
		      argv[i]);
	      return TCL_ERROR;
	    }

	  if (!g) 
	    {
	      sprintf(interp->result,
		      "Goal stack level %s does not exist",
		      argv[i]);
	      return TCL_ERROR;
	    }

	  *go_type = GO_SLOT;
	  *go_slot_level = g->id.level;
	  *go_slot_attr = attr;
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to go command: %s",
		  argv[i]);
	  return TCL_ERROR;	  
	}
    }

  return TCL_OK;
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

int parse_memory_stats (Tcl_Interp * interp, int argc, char * argv[])
{
  if (argc == 2)
    {
      print_memory_statistics ();
      print_memory_pool_statistics ();  

      return TCL_OK;
    }

  if (string_match("-total", argv[2]))
    {
      unsigned long total;
      int i;

      total = 0;
      for (i=0; i<NUM_MEM_USAGE_CODES; i++) 
	total += current_agent(memory_for_usage)[i];
  
      sprintf (interp->result, "%lu", total);  
    }
  else if (string_match("-overhead", argv[2]))
    {
      sprintf (interp->result, "%lu",
	       current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
    }
  else if (string_match("-strings", argv[2]))
    {
      sprintf (interp->result, "%lu",
	       current_agent(memory_for_usage)[STRING_MEM_USAGE]);
    }
  else if (string_match("-hash-table", argv[2]))
    {
      sprintf (interp->result, "%lu",
	       current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
    }
  else if (string_match("-pool", argv[2]))
    {                                          /* Handle pool stats */
      if (argc == 3)
	{
	  print_memory_pool_statistics ();  
	}
      else if (string_match("-total", argv[3]))
	{
	  sprintf (interp->result, "%lu",
		   current_agent(memory_for_usage)[POOL_MEM_USAGE]);
	}
      else 
	{                         /* Match pool name or invalid item */
	  memory_pool *p;
	  memory_pool *pool_found = NIL;

	  for (p=current_agent(memory_pools_in_use); p!=NIL; p=p->next) 
	    {
	      if (string_match (argv[3], p->name))
		{
		  pool_found = p;
		  break;
		}
	    }

	  if (!pool_found)
	    {
	      sprintf(interp->result,
		      "Unrecognized pool name: stats -memory -pool %s",
		      argv[4]);
	      return TCL_ERROR;
	    }

	  if (argc == 4)
	    {

	      print ("Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
	      print ("Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
	      print ("---------------  ----------  ----------  ---------  -----------\n");
#else
	      print ("Pool Name        Item Size  Total Bytes\n");
	      print ("---------------  ---------  -----------\n");
#endif

	      print_string (pool_found->name);
	      print_spaces (MAX_POOL_NAME_LENGTH - strlen(pool_found->name));
#ifdef MEMORY_POOL_STATS
	      print ("  %10lu", pool_found->used_count);
	      total_items = pool_found->num_blocks 
		            * pool_found->items_per_block;
	      print ("  %10lu", total_items - pool_found->used_count);
#endif
	      print ("  %9lu", pool_found->item_size);
	      print ("  %11lu\n", pool_found->num_blocks 
		                  * pool_found->items_per_block 
		                  * pool_found->item_size);
	    }
	  else if (argc == 5)
	    {                                /* get pool attribute */
	      long total_items;

	      total_items = pool_found->num_blocks 
		            * pool_found->items_per_block;

	      if (string_match("-item-size", argv[4]))
		{
		  sprintf (interp->result, "%lu", pool_found->item_size);
		}
#ifdef MEMORY_POOL_STATS		
	      else if (string_match("-used", argv[4]))
		{
		  sprintf (interp->result, "%lu", pool_found->used_count);
		}
	      else if (string_match("-free", argv[4]))
		{
		  sprintf (interp->result, "%lu", 
			   total_items - pool_found->used_count);
		}
#endif
	      else if (string_match("-total-bytes", argv[4]))
		{
		  sprintf (interp->result, "%lu", 
			   pool_found->num_blocks 
			   * pool_found->items_per_block 
			   * pool_found->item_size);
		}
	      else
		{
		  sprintf(interp->result,
			  "Unrecognized argument to stats: -memory -pool %s %s",
			  argv[3], argv[4]);
		  return TCL_ERROR;
		}
	    }
	  else
	    {
	      interp->result = "Too many arguments, should be: stats -memory -pool [-total | pool-name [<aspect>]]";
	      return TCL_ERROR;
	    }
	}
    }
  else if (string_match("-misc", argv[2]))
    {
      sprintf (interp->result, "%lu",
	       current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to stats: -memory %s",
	      argv[2]);
      return TCL_ERROR;
    }

  return TCL_OK;
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

int parse_rete_stats (Tcl_Interp * interp, int argc, char * argv[])
{
  unsigned long data;

  if (argc == 2)
    {
      print_rete_statistics ();
      return TCL_OK;
    }

  if (argc == 3)
    {
      interp->result = "Too few arguments, should be: stats -rete [type qualifier]";
      return TCL_ERROR;
    }

  if (argc > 4)
    {
      interp->result = "Too many arguments, should be: stats -rete [type qualifier]";
      return TCL_ERROR;
    }

  if (get_node_count_statistic(argv[2], (char *) argv[3]+1, &data))
    {
      sprintf(interp->result, "%lu", data);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to stats: -rete %s %s",
	      argv[2], argv[3]);
      return TCL_ERROR;
    }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_run_command--
 *
 *	This procedure parses an argv array and sets the agent parameters.
 *
 *      The syntax being parsed is:
 *          run [n] [units] [-self]
 *
 *              [n] is an integer, which defaults to 1 if a unit is 
 *                  specified; if no units, [n] defaults to forever.
 *              [units]:
 *                 d - decision cycles
 *                 e - elaboration cycles
 *                 p - phases
 *                 s - state
 *                 o - operator selection
 *               out - run by decision cycles until output generated
 *                <s>- current level of subgoaling
 *               <ss>- superstate's level of subgoaling
 *              <sss>- supersuperstate's level of subgoaling
 *                <o>- operator selection AT THIS LEVEL of subgoaling
 *                        or current level of subgoaling terminated
 *               <so>- superoperator selection (or THAT subgoaling terminated)
 *              <sso>- supersuperoperator selection (or subgoaling terminated)
 * 
 *              [-self] to run only the current agent interp.
 *                      (flag only returned, RunCmd processes it)
 *
 * Results:
 *	Returns the agent parameters and the Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int parse_run_command (Tcl_Interp * interp, 
		       int argc, char * argv[],
		       long * go_number, 
		       enum go_type_enum * go_type,
		       Symbol * * go_slot_attr, 
		       goal_stack_level * go_slot_level,
		       bool * self_only)
{
  Symbol *g, *attr, *value;
  int i;
  bool  no_number = TRUE;


  for (i = 1; i < argc; i++)
    {
      get_lexeme_from_string(argv[i]);

      if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) 
	{
	  *go_number = current_agent(lexeme).int_val;
	  no_number  = FALSE;
	}
      else if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) 
	{
	  if (string_match("forever", argv[i]))
	    {
	      *go_number = -1;
	    }
	  else if (string_match("p", argv[i]))
	    {
	      *go_type = GO_PHASE;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("e", argv[i]))
	    {
	      *go_type = GO_ELABORATION;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("d", argv[i]))
	    {
	      *go_type = GO_DECISION;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("s", argv[i]))
	    {
	      *go_type = GO_STATE;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("o", argv[i]))
	    {
	      *go_type = GO_OPERATOR;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("out", argv[i]))
	    {
	      *go_type = GO_OUTPUT;
	      if (no_number) {
		*go_number = 1;
		 no_number = FALSE;
	      }
	    }
	  else if (string_match("-self", argv[i]))
	    {
	      *self_only = TRUE;
	    }
          else
	    {
              sprintf(interp->result,
		      "Unrecognized argument to run command: %s",
		      argv[i]);
              return TCL_ERROR;
            }
	}
      else if (current_agent(lexeme).type == VARIABLE_LEXEME) 
	{
	  get_context_var_info (&g, &attr, &value);
	  if (!attr) 
	    {
	      sprintf(interp->result,
		      "Expected <s>, <o>, <ss>, <so>, <sss>, or <sso> as variable in run command, not %s",
		      argv[i]);
	      return TCL_ERROR;
	    }

	  if (!g) 
	    {
	      sprintf(interp->result,
		      "There is either no superstate, or no supersuperstate (whichever you specified, %s) of the current level",
		      argv[i]);
	      return TCL_ERROR;
	    }
	  *go_type = GO_SLOT;
	  *go_slot_level = g->id.level;
	  *go_slot_attr = attr;
	  if (no_number) {
	    *go_number = 1;
	    no_number = FALSE;
	  }
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to run command: %s",
		  argv[i]);
	  return TCL_ERROR;	  
	}
    }

  /* if there were no units and no number given as args to run, 
   * then run forever.
   */
  if (no_number) {
    *go_number = -1;
  }
  /* 
  printf("parsing run cmd: ");
  for (i = 0; i < argc; i++) printf(" %s ",argv[i]);
  printf("\n \tnumber = %ld \n\ttype = %d\n",*go_number,*go_type);
  */
  return TCL_OK;
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

#ifndef gethostname
#endif

#ifndef time
#endif

int parse_system_stats (Tcl_Interp * interp, int argc, char * argv[])
{
  unsigned long wme_changes;

  /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  double total_kernel_time, total_kernel_msec, derived_kernel_time, monitors_sum,
         input_function_time,  input_phase_total_time,
         output_function_time, output_phase_total_time,
         determine_level_phase_total_time,  /* REW: end   05.05.97 */
         preference_phase_total_time, wm_phase_total_time, decision_phase_total_time,
         derived_total_cpu_time;


 #ifdef DETAILED_TIMING_STATS
  double match_time, match_msec;
  double ownership_time, chunking_time;
  double other_phase_kernel_time[6], other_total_kernel_time;
 #endif
#endif
  /* REW: end 28.07.96 */

  /* MVP 6-8-94 */
  char hostname[MAX_LEXEME_LENGTH+1];
  long current_time;  

#if defined (THINK_C) || defined(_WINDOWS) || defined (__SC__) || defined(WIN32) || defined(MACINTOSH)
  strcpy (hostname, "[host name unknown]");
#else
  if (gethostname (hostname, MAX_LEXEME_LENGTH)) 
    strcpy (hostname, "[host name unknown]");
#endif
  current_time = time(NULL);

  if (argc > 3)
    {
      interp->result = "Too many arguments, should be: stats -system [<type>]";
      return TCL_ERROR;
    }

/* REW: begin 28.07.96 */   /* See note in soarkernel.h for a description of the timers */
#ifndef NO_TIMING_STUFF
      total_kernel_time = timer_value (&current_agent(total_kernel_time));
      total_kernel_msec = total_kernel_time * 1000.0;
 
      /* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
         excluding Input Function, Output function, and pre-defined callbacks. 
         This computed time should be roughly equal to total_kernel_time, 
         as determined above.*/
       
      derived_kernel_time = timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
                    + timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

      input_function_time  = timer_value (&current_agent(input_function_cpu_time));

      output_function_time = timer_value (&current_agent(output_function_cpu_time));


      /* Total of the time spent in callback routines. */
      monitors_sum =  timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));

      derived_total_cpu_time  = derived_kernel_time + monitors_sum + input_function_time 
                                  + output_function_time;

      /* Total time spent in the input phase */
      input_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
               + timer_value (&current_agent(input_function_cpu_time));

      /* REW: begin 10.30.97 */
      determine_level_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
      /* REW: end   10.30.97 */      

      /* Total time spent in the preference phase */
      preference_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));

      /* Total time spent in the working memory phase */
      wm_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[WM_PHASE]));

      /* Total time spent in the output phase */
      output_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
               + timer_value (&current_agent(output_function_cpu_time));

      /* Total time spent in the decision phase */
      decision_phase_total_time = 
                 timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])) 
               + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));

      /* The sum of these six phase timers is exactly equal to the derived_total_cpu_time */

 #ifdef DETAILED_TIMING_STATS

      match_time = timer_value (&current_agent(match_cpu_time[INPUT_PHASE])) 
	         + timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[WM_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])) 
                 + timer_value (&current_agent(match_cpu_time[DECISION_PHASE]));
   
      match_msec = 1000 * match_time; 

     ownership_time = timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])) 
	            + timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]));

      chunking_time = timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])) 
	            + timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));

      /* O-support time should go to 0 with o-support-mode 2 */
      /* o_support_time = timer_value (&current_agent(o_support_cpu_time[INPUT_PHASE])) 
	            + timer_value (&current_agent(o_support_cpu_time[DETERMINE_LEVEL_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[PREFERENCE_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[WM_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[OUTPUT_PHASE])) 
                    + timer_value (&current_agent(o_support_cpu_time[DECISION_PHASE])); */

      other_phase_kernel_time[INPUT_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[INPUT_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE]));

      other_phase_kernel_time[DETERMINE_LEVEL_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));


     other_phase_kernel_time[PREFERENCE_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));

      other_phase_kernel_time[WM_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[WM_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[WM_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[WM_PHASE]));

      other_phase_kernel_time[OUTPUT_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE]));

      other_phase_kernel_time[DECISION_PHASE] = 
                       timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
                    -  timer_value (&current_agent(match_cpu_time[DECISION_PHASE]))
                    -  timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]))
                    -  timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));

      other_total_kernel_time = other_phase_kernel_time[INPUT_PHASE] 
                              + other_phase_kernel_time[DETERMINE_LEVEL_PHASE]
                              + other_phase_kernel_time[PREFERENCE_PHASE]
                              + other_phase_kernel_time[WM_PHASE]
                              + other_phase_kernel_time[OUTPUT_PHASE]
                              + other_phase_kernel_time[DECISION_PHASE];

 #endif
#endif
/* REW: end 28.07.96 */      


  if (argc <= 2) /* Invoked as stats or stats -system */
    {
      print ("Soar %s on %s at %s\n", soar_version_string,
	     hostname, ctime((const time_t *)&current_time));

      print ("%lu productions (%lu default, %lu user, %lu chunks)\n",
	     current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
	     current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
	     current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE],
	     current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
	     current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
	     current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
      print ("   + %lu justifications\n",
	     current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);

/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
      /* The fields for the timers are 8.3, providing an upper limit of approximately
         2.5 hours the printing of the run time calculations.  Obviously, these will
         need to be increased if you plan on needing run-time data for a process that
         you expect to take longer than 2 hours. :) */


      print ("                                                                |    Derived\n");
      print ("Phases:      Input      DLP     Pref      W/M   Output Decision |     Totals\n");
      print ("================================================================|===========\n");

      
      print ("Kernel:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n", timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])), derived_kernel_time);


#ifdef DETAILED_TIMING_STATS

      print ("====================  Detailed Timing Statistics  ==============|===========\n");


      print ("   Match: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n", 
                 timer_value (&current_agent(match_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])),
                 timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(match_cpu_time[DECISION_PHASE])) , 
                 match_time);

      print ("Own'ship: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
                 timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])),
                 timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE])), 
                 ownership_time);

      print ("Chunking: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
                 timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])),
                 timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE])), 
                 chunking_time);

      print ("   Other: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
                 other_phase_kernel_time[INPUT_PHASE],
                 other_phase_kernel_time[DETERMINE_LEVEL_PHASE],
                 other_phase_kernel_time[PREFERENCE_PHASE],
                 other_phase_kernel_time[WM_PHASE],
                 other_phase_kernel_time[OUTPUT_PHASE],
                 other_phase_kernel_time[DECISION_PHASE],
                 other_total_kernel_time);


  /* REW: begin 11.25.96 */ 
      print ("     GDS: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
                 timer_value (&current_agent(gds_cpu_time[INPUT_PHASE])), 
	         timer_value (&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])), 
                 timer_value (&current_agent(gds_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(gds_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(gds_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(gds_cpu_time[DECISION_PHASE])),
                 timer_value (&current_agent(gds_cpu_time[INPUT_PHASE])) + 
                 timer_value (&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])) + 
                 timer_value (&current_agent(gds_cpu_time[PREFERENCE_PHASE])) +
                 timer_value (&current_agent(gds_cpu_time[WM_PHASE])) +
                 timer_value (&current_agent(gds_cpu_time[OUTPUT_PHASE])) +
                 timer_value (&current_agent(gds_cpu_time[DECISION_PHASE]))); 

  /* REW: end   11.25.96 */ 


#endif

      
      print ("================================================================|===========\n");
      print ("Input fn: %8.3f                                              | %10.3f\n",  
              input_function_time, input_function_time); 

      print ("================================================================|===========\n");
      print ("Outpt fn:                                     %8.3f          | %10.3f\n",  
              output_function_time, output_function_time);

      print ("================================================================|===========\n");
      print ("Callbcks: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
                 timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[WM_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])), 
                 timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE])), 
                 monitors_sum);

      print ("================================================================|===========\n");
      print ("Derived---------------------------------------------------------+-----------\n");
      print ("Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n\n",
                        input_phase_total_time,
	                determine_level_phase_total_time, 
                        preference_phase_total_time,
                        wm_phase_total_time,
                        output_phase_total_time, 
                        decision_phase_total_time,
                        derived_total_cpu_time);

	  if (!current_agent(stop_soar)) {
		  /* Soar is still running, so this must have been invoked
		   * from the RHS, therefore these timers need to be updated. */
		  stop_timer(&current_agent(start_total_tv),&current_agent(total_cpu_time));
		  stop_timer(&current_agent(start_kernel_tv),&current_agent(total_kernel_time));
		  start_timer(&current_agent(start_total_tv));
		  start_timer(&current_agent(start_kernel_tv));
	  }
      print ("Values from single timers:\n");
      print (" Kernel CPU Time: %11.3f sec. \n", total_kernel_time);     
      print (" Total  CPU Time: %11.3f sec.\n\n", timer_value (&current_agent(total_cpu_time)));

#endif

#if !defined(NO_TIMING_STUFF)
      print ("%lu decision cycles (%.3f msec/dc)\n",
	     current_agent(d_cycle_count),
	     current_agent(d_cycle_count) ? total_kernel_msec/current_agent(d_cycle_count) : 0.0);
      print ("%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
	     current_agent(e_cycle_count),
	     current_agent(d_cycle_count) ? (double)current_agent(e_cycle_count)/current_agent(d_cycle_count) : 0,
	     current_agent(e_cycle_count) ? total_kernel_msec/current_agent(e_cycle_count) : 0);
      /* REW: begin 09.15.96 */
      if (current_agent(operand2_mode))
          print ("%lu p-elaboration cycles (%.3f pe's per dc, %.3f msec/pe)\n",
	     current_agent(pe_cycle_count),
	     current_agent(d_cycle_count) ? (double)current_agent(pe_cycle_count)/current_agent(d_cycle_count) : 0,
	     current_agent(pe_cycle_count) ? total_kernel_msec/current_agent(pe_cycle_count) : 0);
      /* REW: end 09.15.96 */
      print ("%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
	     current_agent(production_firing_count),
	     current_agent(e_cycle_count) ? (double)current_agent(production_firing_count)/current_agent(e_cycle_count) : 0.0,
	     current_agent(production_firing_count) ? total_kernel_msec/current_agent(production_firing_count) : 0.0);

#else
      print ("%lu decision cycles\n", current_agent(d_cycle_count));
      print ("%lu elaboration cycles \n", current_agent(e_cycle_count));
      print ("%lu production firings \n", current_agent(production_firing_count));
#endif /* !NO_TIMING_STUFF */      

      
      wme_changes = current_agent(wme_addition_count) + current_agent(wme_removal_count);
      print ("%lu wme changes (%lu additions, %lu removals)\n",
	     wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
#ifdef DETAILED_TIMING_STATS
  print ("    match time: %.3f msec/wm change\n",
         wme_changes ? match_msec/wme_changes : 0.0);
#endif

      print ("WM size: %lu current, %.3f mean, %lu maximum\n",
	     current_agent(num_wmes_in_rete),
	     (current_agent(num_wm_sizes_accumulated) ?
	      (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
	      0.0),
	     current_agent(max_wm_size));

#ifndef NO_TIMING_STUFF
     print ("\n");
     print ("    *** Time/<x> statistics use the total kernel time from a ***\n");
     print ("    *** single kernel timer.  Differences between this value ***\n");
     print ("    *** and the derived total kernel time  are expected. See ***\n");
     print ("    *** help  for the  stats command  for more  information. ***\n");   
#endif
     /* REW: end 28.07.96 */
    }
  else
    {
      if (string_match("-default-production-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]);
	}
      else if (string_match("-user-production-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]);

	}
      else if (string_match("-chunk-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
	}
      else if (string_match("-justification-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
	}
      else if (string_match("-all-productions-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]
		  + current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]
		  + current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);

	}
      else if (string_match("-dc-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", current_agent(d_cycle_count));
	}      
      else if (string_match("-ec-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", current_agent(e_cycle_count));
	}      
      else if (string_match("-ecs/dc", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(d_cycle_count) 
		   ? ((double) current_agent(e_cycle_count)
		               / current_agent(d_cycle_count))
		   : 0.0));
	}      
      else if (string_match("-firings-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(production_firing_count));
	}      
      else if (string_match("-firings/ec", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(e_cycle_count) 
		   ? ((double) current_agent(production_firing_count)
		               / current_agent(e_cycle_count))
		   : 0.0));
	}      
      else if (string_match("-wme-change-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(wme_addition_count)
		  + current_agent(wme_removal_count));
	}      
      else if (string_match("-wme-addition-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(wme_addition_count));
	}      
      else if (string_match("-wme-removal-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(wme_removal_count));
	}
      else if (string_match("-wme-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(num_wmes_in_rete));
	}      
      else if (string_match("-wme-avg-count", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(num_wm_sizes_accumulated) 
		   ? (current_agent(cumulative_wm_size) 
		      / current_agent(num_wm_sizes_accumulated)) 
		   : 0.0));
	}      
      else if (string_match("-wme-max-count", argv[2]))
	{
	  sprintf(interp->result, "%lu", 
		  current_agent(max_wm_size));
	}     
#ifndef NO_TIMING_STUFF
      else if (string_match("-total-time", argv[2]))
	{
	  sprintf(interp->result, "%.3f", total_kernel_time);
	}
      else if (string_match("-ms/dc", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(d_cycle_count) 
		   ? total_kernel_msec/current_agent(d_cycle_count) 
		   : 0.0));
	}      
      else if (string_match("-ms/ec", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(e_cycle_count) 
		   ? total_kernel_msec/current_agent(e_cycle_count) 
		   : 0.0));
	}      
      else if (string_match("-ms/firing", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  (current_agent(production_firing_count)
		   ? total_kernel_msec/current_agent(production_firing_count)
		   : 0.0));
	}      
#endif /* NO_TIMING_STUFF */
#ifdef DETAILED_TIMING_STATS
      else if (string_match("-ms/wme-change", argv[2]))
	{
	  long wme_changes;

	  wme_changes = current_agent(wme_addition_count)
	                + current_agent(wme_removal_count);

	  sprintf(interp->result, "%.3f", 
		  (wme_changes ? match_msec/wme_changes : 0.0));
	}      
      else if (string_match("-match-time", argv[2]))
	{
	  sprintf(interp->result, "%.3f", match_time);
	}
      else if (string_match("-ownership-time", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  ownership_time);
	}
      else if (string_match("-chunking-time", argv[2]))
	{
	  sprintf(interp->result, "%.3f", 
		  chunking_time);
	}
#endif /* DETAILED_TIMING_STATS */
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to stats: -system %s",
		  argv[2]);
	}
    }

  return TCL_OK;
}

/* AGR 652 begin */
/* Comment from Bob:  
   Also, if speed becomes an issue, you might put in a special case check
   for the common case where both p1 and p2 are SYM_CONSTANT's, in which
   case you could avoid the symbol_to_string() calls entirely and just
   call strcmp() directly on the attribute names.  (Maybe just leave this
   as a comment for now, just in case somebody complains about print speed
   some day.)  */

/* The header for compare_attr needs to be defined in this way because
   otherwise we get compiler warnings at the qsort lines about the 4th
   argument being an incompatible pointer type.  */

int compare_attr (const void * e1, const void * e2)
{
  wme **p1, **p2;

  char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

  p1 = (wme **) e1;
  p2 = (wme **) e2;

  symbol_to_string ((*p1)->attr, TRUE, s1);
  symbol_to_string ((*p2)->attr, TRUE, s2);

  return (strcmp (s1, s2));
}

void print_augs_of_id (Symbol *id, int depth, bool internal,
                       int indent, tc_number tc) {
  slot *s;
  wme *w;

  wme **list;    /* array of WME pointers, AGR 652 */
  int num_attr;  /* number of attributes, AGR 652 */
  int attr;      /* attribute index, AGR 652 */

/* AGR 652  The plan is to go through the list of WMEs and find out how
   many there are.  Then we malloc an array of that many pointers.
   Then we go through the list again and copy all the pointers to that array.
   Then we qsort the array and print it out.  94.12.13 */

  if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return;
  if (id->id.tc_num==tc) return;
  id->id.tc_num = tc;

  /* --- first, count all direct augmentations of this id --- */
  num_attr = 0;
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next) num_attr++;
  for (w=id->id.input_wmes; w!=NIL; w=w->next) num_attr++;
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (w=s->wmes; w!=NIL; w=w->next) num_attr++;
    for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) num_attr++;
  }

  /* --- next, construct the array of wme pointers and sort them --- */
  list = allocate_memory(num_attr*sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
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
  qsort (list, num_attr, sizeof (wme *), compare_attr); 

  /* --- finally, print the sorted wmes and deallocate the array --- */
  if (internal) {
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      print_spaces (indent);
      print_wme (w);
    }
  } else {
    print_spaces (indent);
    print_with_symbols ("(%y", id);
    for (attr=0; attr < num_attr; attr++) {
      w = list[attr];
      neatly_print_wme_augmentation_of_id (w, indent);
    }
    print (")\n");
  }
  free_memory(list, MISCELLANEOUS_MEM_USAGE);
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
  print ("Current attention-lapse setting:\n");
  print ("   %s\n", current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] ? "-on" : "-off");
  
}

#endif /* ATTENTION_LAPSE */

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
  print ("Current learn settings:\n");
/* AGR MVL1 begin */
    if ((! current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) &&
	(! current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM]))
      print ("   %s\n", current_agent(sysparams)[LEARNING_ON_SYSPARAM] ? "-on" : "-off");
    else
      print ("   %s\n", current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] ? "-only" : "-except");

/* AGR MVL1 end */
  print ("   %s\n", current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] ? "-all-levels" : "-bottom-up");
  
}

/*
 *----------------------------------------------------------------------
 *
 * print_current_watch_settings --
 *
 *	This procedure prints the current watch settings.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current watch settings.
 *
 *----------------------------------------------------------------------
 */

void print_current_watch_settings (Tcl_Interp * interp)
{
/* Added this to avoid segfault on Solaris when attempt to print NULL */
char *a = NULL;

  print ("Current watch settings:\n");
  print ("  Decisions:  %s\n",
	 current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
  print ("  Phases:  %s\n",
	 current_agent(sysparams)[TRACE_PHASES_SYSPARAM] ? "on" : "off");
  print ("  Production firings/retractions\n");
  print ("    default productions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
  print ("    user productions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
  print ("    chunks:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
  print ("    justifications:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
  print ("    WME detail level:  %d\n",
	 current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
  print ("  Working memory changes:  %s\n",
	 current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
  print ("  Preferences generated by firings/retractions:  %s\n",
	 current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
  /*  don't print these individually...see chunk-creation
   *  print ("  Chunk names:  %s\n",
   *      current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "on" : "off");
   *  print ("  Justification names:  %s\n",
   *      current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] ? "on" : "off");
   *  print ("  Chunks:  %s\n",
   *      current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] ? "on" : "off");
   *  print ("  Justifications:  %s\n",
   *      current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
   */
  print ("\n");
  if (current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM]) {
    print ("  Learning:  -fullprint  (watch creation of chunks/just.)\n");
  } else {
    print ("  Learning:  %s  (watch creation of chunks/just.)\n",
	   current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "-print" : "-noprint");
  }
  print ("  Backtracing:  %s\n",
	 current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");

  /* To make sure that the program does not segfault
      it is necessary to check if the pointer returned from
      Tcl_GetVar is NULL because the solaris version of vsprintf
      doesn't like NULL pointers. */
  print ("  Alias printing:  %s\n",
	 (a=Tcl_GetVar(interp,"print_alias_switch",0)) ? a : "(null)");
  print ("  Loading:  %s\n",
	 current_agent(sysparams)[TRACE_LOADING_SYSPARAM] ? "on" : "off");
  print ("\n");
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

  print("\n");
  if (!m) {
    print("No multi-attributes declared for this agent.\n");
  } else {
    print("Value\tSymbol\n");
    while(m) {
      print("%ld\t%s\n", m->value, symbol_to_string(m->symbol, TRUE, NIL));
      m = m->next;
    }
  }
  print("\n");
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
                                  bool print_source,
                                  wme_trace_type wtt) 
{
  print_string ("  ");
  print_object_trace (pref->value);
  print (" %c", preference_type_indicator (pref->type));
  if (preference_is_binary(pref->type)) print_object_trace (pref->referent);
  if (pref->o_supported) print (" :O ");
  print ("\n");
  if (print_source) {
    print ("    From ");
    print_instantiation_with_wmes (pref->inst, wtt);
    print ("\n");
  }
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

int print_production_firings (Tcl_Interp * interp, int num_requested)
{
  int  i;
  long num_prods;
  production *((*all_prods)[]), **ap_item, *p;
  
  num_prods = current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
              current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE];

  if (num_prods == 0) 
    {
      interp->result = "No productions defined.";
      return TCL_OK;                     /* so we don't barf on zero later */
    }

  /* --- make an array of pointers to all the productions --- */
  all_prods = allocate_memory (num_prods * sizeof (production *),
                               MISCELLANEOUS_MEM_USAGE);

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
	      print_with_symbols ("%y\n", (*ap_item)->name);
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
	print ("%6lu:  ", (*ap_item)->firing_count);
	print_with_symbols ("%y\n", (*ap_item)->name);
	ap_item--;
	num_requested--;
      }
    }

  /* MVP 6-8-94 also try this to plug memory leak */
  free_memory(all_prods, MISCELLANEOUS_MEM_USAGE);

  return TCL_OK;
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
      id = find_identifier (current_agent(lexeme).id_letter, 
			    current_agent(lexeme).id_number);
      if (!id) 
	{
	  return TCL_ERROR;
	}
      else
	{
	  *result_id = id;
	  return TCL_OK;
	}
    }

  if (current_agent(lexeme).type==VARIABLE_LEXEME) 
    {
      get_context_var_info (&g, &attr, &value);

      if ((!attr) || (!value))
	{
	  return TCL_ERROR;
	}

      if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
	{
	  return TCL_ERROR;
	}

      *result_id = value;
      return TCL_OK;
    }

  return TCL_ERROR;
}


/*
 *  This function should be replaced by the one above and added to the
 *  Soar kernel. 
 */

Symbol *read_identifier_or_context_variable (void) {
  Symbol *id;
  Symbol *g, *attr, *value;

  if (current_agent(lexeme).type==IDENTIFIER_LEXEME) {
    id = find_identifier (current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
    if (!id) {
      print ("There is no identifier %c%lu.\n", current_agent(lexeme).id_letter,
             current_agent(lexeme).id_number);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    return id;
  }
  if (current_agent(lexeme).type==VARIABLE_LEXEME) {
    get_context_var_info (&g, &attr, &value);
    if (!attr) {
      print ("Expected identifier (or context variable)\n");
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    if (!value) {
      print ("There is no current %s.\n", current_agent(lexeme).string);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
      print ("The current %s ", current_agent(lexeme).string);
      print_with_symbols ("(%y) is not an identifier.\n", value);
      print_location_of_most_recent_lexeme();
      return NIL;
    }
    return value;
  }
  print ("Expected identifier (or context variable)\n");
  print_location_of_most_recent_lexeme();
  return NIL;
}

/* kjh (CUSP-B7):  Add to soarCommandUtils.c after procedure
   "read_id_or_context_var_from_string" */
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
    attr_tmp = find_sym_constant (current_agent(lexeme).string);
    break;
  case INT_CONSTANT_LEXEME:
    attr_tmp = find_int_constant (current_agent(lexeme).int_val);
    break;
  case FLOAT_CONSTANT_LEXEME:
    attr_tmp = find_float_constant (current_agent(lexeme).float_val);
    break;
  case IDENTIFIER_LEXEME:
    attr_tmp = find_identifier (current_agent(lexeme).id_letter,
                                current_agent(lexeme).id_number);
    break;
  case VARIABLE_LEXEME:
    attr_tmp = read_identifier_or_context_variable();
    if (!attr_tmp)
      return TCL_ERROR;
    break;
  default:
    return TCL_ERROR;
  }
  s = find_slot (id, attr_tmp);
  if (s) {
    *attr = attr_tmp;
    return TCL_OK;
  } else
    return TCL_ERROR;
}


/* kjh (CUSP-B7):  Add to soarCommandUtils.c after procedure
   "read_id_or_context_var_from_string" */
/*
 *----------------------------------------------------------------------
 *
 * read_pref_detail_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for the detail level indicator for the 'preferences'
 *      command.  If so, it sets the_lexeme and wme_trace_type accordingly
 *      and returns TCL_OK; otherwise, it leaves those parameters untouched
 *      and returns TCL_ERROR.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_pref_detail_from_string (char *the_lexeme,
                                  bool *print_productions,
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
    return TCL_ERROR;
  }
  return TCL_OK;
}


/* ===================================================================
                Read Pattern And Get Matching Wmes

   This routine reads a pattern and returns a list of all wmes that
   match it.  At entry, the current lexeme should be the "("; at exit,
   the current lexeme will be the ")".  If any error occurs or if no
   wmes match the pattern, the function returns NIL.

   pattern ::= ( {identifier | '*'} ^ { attribute | '*'} { value | '*' } [+])

=================================================================== */

/* This should be added to the Soar kernel.  It must be re-written to
 * take a string to parse.
 */

int read_pattern_component (Symbol **dest_sym) {
  /* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
     otherwise return 2 and set dest_sym to find_symbol() result. --- */
  if (strcmp(current_agent(lexeme).string,"*") == 0) return 1;
  switch (current_agent(lexeme).type) {
  case SYM_CONSTANT_LEXEME:
    *dest_sym = find_sym_constant (current_agent(lexeme).string); return 2;
  case INT_CONSTANT_LEXEME:
    *dest_sym = find_int_constant (current_agent(lexeme).int_val); return 2;
  case FLOAT_CONSTANT_LEXEME:
    *dest_sym = find_float_constant (current_agent(lexeme).float_val); return 2;
  case IDENTIFIER_LEXEME:
    *dest_sym = find_identifier (current_agent(lexeme).id_letter, current_agent(lexeme).id_number); return 2;
  case VARIABLE_LEXEME:
    *dest_sym = read_identifier_or_context_variable();
    if (*dest_sym) return 2;
    return 0;
  default:
    print ("Expected identifier or constant in wme pattern\n");
    print_location_of_most_recent_lexeme();
    return 0;
  }
}

list *read_pattern_and_get_matching_wmes (void) {
  int parentheses_level;
  list *wmes;
  wme *w;
  Symbol *id, *attr, *value;
  int id_result, attr_result, value_result;
  bool acceptable;
  
  if (current_agent(lexeme).type!=L_PAREN_LEXEME) {
    print ("Expected '(' to begin wme pattern not string '%s' or char '%c'\n", 
	   current_agent(lexeme).string, current_agent(current_char));
    print_location_of_most_recent_lexeme();
    return NIL;
  }
  parentheses_level = current_lexer_parentheses_level();

  get_lexeme();
  id_result = read_pattern_component (&id);
  if (! id_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  if (current_agent(lexeme).type!=UP_ARROW_LEXEME) {
    print ("Expected ^ in wme pattern\n");
    print_location_of_most_recent_lexeme();
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  attr_result = read_pattern_component (&attr);
  if (! attr_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  value_result = read_pattern_component (&value);
  if (! value_result) {
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }
  get_lexeme();
  if (current_agent(lexeme).type==PLUS_LEXEME) {
    acceptable = TRUE;
    get_lexeme();
  } else {
    acceptable = FALSE;
  }
  if (current_agent(lexeme).type!=R_PAREN_LEXEME) {
    print ("Expected ')' to end wme pattern\n");
    print_location_of_most_recent_lexeme();
    skip_ahead_to_balanced_parentheses (parentheses_level-1);
    return NIL;
  }

  wmes = NIL;
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next) {
    if ((id_result==1) || (id==w->id))
      if ((attr_result==1) || (attr==w->attr))
        if ((value_result==1) || (value==w->value))
          if (acceptable==w->acceptable)
            push (w, wmes);
  }
  return wmes;  
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

int set_watch_setting (Tcl_Interp * interp, 
		       int dest_sysparam_number, char * param, char * arg) 
{
  if (arg == NULL)
    {
      sprintf(interp->result,
	      "Missing setting for watch parameter %s",
	      param);
      return TCL_ERROR;
    }

  if (string_match("-on", arg))
    {
      set_sysparam (dest_sysparam_number, TRUE);
    }
  else if (string_match("-off", arg))
    {
      set_sysparam (dest_sysparam_number, FALSE);
    }
  else if (string_match_up_to("-inclusive",arg,3))
    {
      set_watch_level_inc (interp, dest_sysparam_number);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized setting for watch parameter %s: %s",
	      param, arg);
      return TCL_ERROR;
    }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * set_watch_level_inc --
 *
 *	This procedure sets the parameters for the inclusive watch
 *      level given.  It can only be called when watch arg is either
 *      an integer (0-5), or a named setting that is inclusive.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Sets the desired paramaters to the appropriate setting.
 *
 *----------------------------------------------------------------------
 */

int set_watch_level_inc (Tcl_Interp * interp, int level)

{
  /* printf("set_watch_level_inc:  level = %d\n",level); */
  
  /* intially turn off all parameters, (this may not be correct thing?
     what if going from a lower level to a higher level-> don't reset */

  set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM,         FALSE);
  set_sysparam(TRACE_PHASES_SYSPARAM,                    FALSE);
  set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM,  FALSE);
  set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,     FALSE);
  set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM,         FALSE);
  set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
  set_sysparam(TRACE_WM_CHANGES_SYSPARAM,                FALSE);
  set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,       FALSE);
  set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM,         FALSE);
  
  switch (level) {
  case 0:
    /* make sure everything is off */
    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
    set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM,            FALSE);
    set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM,    FALSE);
    set_sysparam(TRACE_CHUNKS_SYSPARAM,                 FALSE);
    set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM,         FALSE);
    set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM,      FALSE);
    break;

  case TRACE_FIRINGS_PREFERENCES_SYSPARAM:
    
    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,      TRUE);

  case TRACE_WM_CHANGES_SYSPARAM:

    set_sysparam(TRACE_WM_CHANGES_SYSPARAM,               TRUE);
    /* set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   FULL_WME_TRACE);*/

  case TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM:

    set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM,  TRUE);
    set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,     TRUE);
    set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM,         TRUE);
    set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);

  case TRACE_PHASES_SYSPARAM:

    set_sysparam(TRACE_PHASES_SYSPARAM,                   TRUE);

  case TRACE_CONTEXT_DECISIONS_SYSPARAM:

    set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM,        TRUE);
  }
  
  return TCL_OK;
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

int set_watch_prod_group_setting (Tcl_Interp * interp,
				  int  prodgroup,
				  char * prodtype, char * arg) 
{
  if (arg == NULL)
    {
      sprintf(interp->result,
	      "Missing setting for watch %s",
	      prodtype);
      return TCL_ERROR;
    }

  if (string_match("-print", arg))
    {
      switch (prodgroup) {
      case 0:
	set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
	set_sysparam (TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	set_sysparam (TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
	break;
      case 1:
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
	break;
      case 2:
	set_sysparam (TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
	break;
      case 3:
	set_sysparam (TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
	break;
      case 4:
	set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
	break;
      }
    }
  else if (string_match("-noprint", arg))
    {
      switch (prodgroup) {
      case 0:
	set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
	set_sysparam (TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
	break;
      case 1:
	set_sysparam (TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
	break;
      case 2:
	set_sysparam (TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
	break;
      case 3:
	set_sysparam (TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
	break;
      case 4:
	set_sysparam (TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
	break;
      }
    }
  else if (string_match("-fullprint", arg))
    {
      sprintf(interp->result,
	      "Sorry, -fullprint not yet implemented for watch productions");
      return TCL_ERROR;
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized setting for watch %s: %s.  Use -print|-noprint|-fullprint",
	      prodtype, arg);
      return TCL_ERROR;
    }
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_agent_already_defined --
 *
 *	This procedure determines if a Soar agent name is
 *      already in use.
 *
 * Results:
 *	Returns a boolean indication of whether the agent
 *      name is already defined or not.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool
soar_agent_already_defined (char * name)
{
  cons *  c;                             /* Cons cell index            */
  agent * the_agent;                     /* Agent index                */
                                
  for (c = all_soar_agents; c != NIL; c = c->rest) 
    {
      the_agent = (agent *) c->first;
      if (!strcmp(the_agent->name, name)) 
	{
	  return TRUE;
	}
    }

  return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_callback_to_tcl (soar_callback_agent the_agent, 
		      soar_callback_data data,
		      soar_call_data call_data)
{
  int code;

  code = Tcl_GlobalEval(((agent *)the_agent)->interpreter, 
			(char *) data);
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) data);
      print("Reason: %s\n", ((Tcl_Interp*) ((agent *)the_agent)->interpreter)->result);
      control_c_handler(0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_input_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar input callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_input_callback_to_tcl (soar_callback_agent the_agent, 
			    soar_callback_data data,
			    soar_call_data call_data)
{
  Tcl_DString command;
  int mode;
  char * mode_name;
  int code;

  mode = (int) call_data;
  if (mode == TOP_STATE_JUST_CREATED)
    {
      mode_name = "top-state-just-created";
    }
  else if (mode == NORMAL_INPUT_CYCLE)
    {
      mode_name = "normal-input-cycle";
    }
  else if (mode == TOP_STATE_JUST_REMOVED)
    {
      mode_name = "top-state-just-removed";
    }
  else 
    {
      print("Error calling Tcl input procedure callback function -- unrecognized mode: %d", mode);
      return;
    }

  Tcl_DStringInit(&command);
  Tcl_DStringAppend(&command, data, strlen(data));
  Tcl_DStringAppendElement(&command, mode_name);

  code = Tcl_GlobalEval(((agent *)the_agent)->interpreter, 
			(char *) Tcl_DStringValue(&command));
 
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) Tcl_DStringValue(&command));
      print("Reason: %s\n", ((Tcl_Interp*) ((agent *)the_agent)->interpreter)->result);
      Tcl_DStringFree(&command);
      control_c_handler(0);
    }

 Tcl_DStringFree(&command);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_output_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar output callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_output_callback_to_tcl (soar_callback_agent the_agent, 
			     soar_callback_data data,
			     soar_call_data call_data)
{
  Tcl_DString command;
  output_call_info * output_info;
  int mode;
  io_wme * outputs, * wme;
  char * mode_name;
  int code;

  output_info = (output_call_info *) call_data;
  mode = output_info->mode;
  outputs = output_info->outputs;

  if (mode == ADDED_OUTPUT_COMMAND)
    {
      mode_name = "added-output-command";
    }
  else if (mode == MODIFIED_OUTPUT_COMMAND)
    {
      mode_name = "modified-output-command";
    }
  else if (mode == REMOVED_OUTPUT_COMMAND)
    {
      mode_name = "removed-output-command";
    }
  else 
    {
      print("Error calling Tcl output procedure callback function -- unrecognized mode: %d", mode);
      return;
    }

  Tcl_DStringInit(&command);
  Tcl_DStringAppend(&command, data, strlen(data));
  Tcl_DStringAppendElement(&command, mode_name);
  Tcl_DStringAppend(&command, " { ", 3);

  /* Build output list */
  for (wme = outputs; wme != NIL; wme = wme->next)
    {
      char wme_string[1000];
      char obj_string[1000];
      char attr_string[1000];
      char value_string[1000];

      symbol_to_string(wme->id,    FALSE, obj_string);
      symbol_to_string(wme->attr,  FALSE, attr_string);
      symbol_to_string(wme->value, FALSE, value_string);

      sprintf(wme_string, "%s %s %s", obj_string, attr_string, value_string);

      Tcl_DStringAppendElement(&command, wme_string);
    }

  Tcl_DStringAppend(&command, " }", 2);

  code = Tcl_GlobalEval(((agent *)the_agent)->interpreter, 
			(char *) Tcl_DStringValue(&command));
 
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) Tcl_DStringValue(&command));
      print("Reason: %s\n", ((Tcl_Interp*) ((agent *)the_agent)->interpreter)->result);
      Tcl_DStringFree(&command);
      control_c_handler(0);
    }

 Tcl_DStringFree(&command);
}

/*
 *----------------------------------------------------------------------
 *
 * string_match --
 *
 *	This procedure compares two strings to see if there 
 *      are equal.
 *
 * Results:
 *	TRUE if the strings match, FALSE otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool
string_match (char * string1, char * string2)
{
  if ((string1 == NULL) && (string2 == NULL))
    return TRUE;

  if (   (string1 != NULL) 
      && (string2 != NULL) 
      && !(strcmp(string1, string2)))
    return TRUE;
  else
    return FALSE;
}


/*
 *----------------------------------------------------------------------
 *
 * string_match_up_to --
 *
 *	This procedure compares two strings to see if there 
 *      are equal up to the indicated number of positions.
 *
 * Results:
 *	TRUE if the strings match over the given number of
 *      characters, FALSE otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool
string_match_up_to (char * string1, char * string2, int positions)
{
    unsigned int i,num;

  /*  what we really want is to require a match over the length of
      the shorter of the two strings, with positions being a minimum */

  num = strlen(string1);
  if (num > strlen(string2)) num = strlen(string2);
  if ((unsigned int)positions < num)  positions = num;  
   
  for (i = 0; i < (unsigned int)positions; i++)  
    {
      if (string1[i] != string2[i])
	return FALSE;
    }

  return TRUE;      
}

typedef struct production_memory_use_struct {
  Symbol *name;
  int mem;
  struct production_memory_use_struct *next;
} production_memory_use;

void print_memories (int num_to_print, int to_print[]) {
  int i, num_prods;
  production_memory_use *temp, *first, *tempnext;
  production *prod;
  production_memory_use *print_memories_insert_in_list();

  print("\nMemory use for productions:\n\n");

  /* Start by doing ALL of them. */
  first = NULL;
  num_prods = 0;

  for (i=0; i < NUM_PRODUCTION_TYPES; i++)
    if (to_print[i]) 
      for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL; prod=prod->next) {
	 temp = allocate_memory (sizeof (production_memory_use),
				 MISCELLANEOUS_MEM_USAGE);
	 temp->next = NULL;
	 temp->name = prod->name;

         temp->mem = count_rete_tokens_for_production (prod);

	 first = print_memories_insert_in_list(temp,first);
	 num_prods++;
        }

  i = 0;
  if (num_to_print < 0) num_to_print = num_prods;

  for (temp = first; ((temp != NULL) && (i < num_to_print)); temp = tempnext) {
    print_with_symbols("%y: ", temp->name);
    print("%d\n",temp->mem);
    tempnext = temp->next;
    free_memory(temp,MISCELLANEOUS_MEM_USAGE);
    i++;
   }
}

production_memory_use *print_memories_insert_in_list(production_memory_use *new,
						     production_memory_use *list) {
  production_memory_use *ctr, *prev;

  /* Add to beginning. */
  if ((list == NULL) || (new->mem >= list->mem)) {
    new->next = list;
    return new;}

  /* Add to middle. */
  prev = list;
  for (ctr = list->next; ctr != NULL; ctr = ctr->next) {
    if (new->mem >= ctr->mem) {
      prev->next = new;
      new->next = ctr;
      return list;
    }
    prev = ctr;
  }

  /* Add to end. */
  prev->next = new;
  new->next = NULL;
  return list;
}


typedef struct binding_structure {
  Symbol *from, *to;
} Binding;

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
    free_memory(c->first,MISCELLANEOUS_MEM_USAGE);
    free_cons (c);
    c = c_next;
  }

  bindings = current_binding_point;
}



void free_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    free_memory(c->first,MISCELLANEOUS_MEM_USAGE);
  free_list(bindings);
}

void print_binding_list (list *bindings) {
  cons *c;

  for (c=bindings;c!=NIL;c=c->rest)
    print_with_symbols ("   (%y -> %y)\n",((Binding *) c->first)->from,((Binding *) c->first)->to);
}


bool symbols_are_equal_with_bindings (Symbol *s1, Symbol *s2, list **bindings) {
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
    b = (Binding *) allocate_memory(sizeof(Binding),MISCELLANEOUS_MEM_USAGE);
    b->from = s1;
    b->to = s2;
    push(b,*bindings);
    return TRUE;
  }
  else if (bvar == s2) {
    return TRUE;
  }
  else return FALSE;
}

/* ----------------------------------------------------------------
   Returns TRUE iff the two tests are identical given a list of bindings.
   Augments the bindings list if necessary.
---------------------------------------------------------------- */

/* DJP 4/3/96 -- changed t2 to test2 in declaration */
bool tests_are_equal_with_bindings (test t1, test test2, list **bindings) {
  cons *c1, *c2;
  complex_test *ct1, *ct2;
  bool goal_test,impasse_test;

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
    t2 = copy_test_removing_goal_impasse_tests(test2, &goal_test, &impasse_test);
  }
  else
    t2 = copy_test(test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */

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
      if (! tests_are_equal_with_bindings(c1->first,c2->first,bindings)) dealloc_and_return(t2,FALSE)
    if (c1==c2) dealloc_and_return(t2,TRUE)  /* make sure they both hit end-of-list */
    dealloc_and_return(t2,FALSE)

  default:  /* relational tests other than equality */
    if (symbols_are_equal_with_bindings(ct1->data.referent,ct2->data.referent,bindings)) dealloc_and_return(t2,TRUE)
    dealloc_and_return(t2,FALSE)
  }
}

bool conditions_are_equal_with_bindings (condition *c1, condition *c2, list **bindings) {
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

/* Routine for LHS. */
void read_pattern_and_get_matching_productions (list **current_pf_list, bool show_bindings,
                                                bool just_chunks,bool no_chunks) {
  condition *c, *clist, *top, *bottom, *pc;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  bool match, match_this_c;


  bindings = NIL;
  current_binding_point = NIL;

/*  print("Parsing as a lhs...\n"); */
  clist = (condition *) parse_lhs();
  if (!clist) {
    print("Error: not a valid condition list.\n");
    current_pf_list = NIL;
    return;
  }
/*
  print("Valid condition list:\n");
  print_condition_list(clist,0,FALSE);
  print("\nMatches:\n");
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
      p_node_to_conditions_and_nots (prod->p_node, NIL, NIL, &top, &bottom,
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
      deallocate_condition_list (top); /* DJP 4/3/96 -- Never dealloced */
      if (match) {
        push(prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols("%y, with bindings:\n",prod->name);
          print_binding_list(bindings);}
        else
          print_with_symbols("%y\n",prod->name);
      }
     }
  if (bindings) free_binding_list(bindings); /* DJP 4/3/96 -- To catch the last production */
}


bool funcalls_match(list *fc1, list *fc2) {
  /* I have no idea how to do this. */
  return FALSE;
}

bool actions_are_equal_with_bindings (action *a1, action *a2, list **bindings) {
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

/* Routine for RHS. */
void read_rhs_pattern_and_get_matching_productions (list **current_pf_list, bool show_bindings,
                                                    bool just_chunks, bool no_chunks) {
  action *a, *alist, *pa;
  int i;
  production *prod;
  list *bindings, *current_binding_point;
  bool match, match_this_a, parsed_ok;
  action *rhs; 
  condition *top_cond, *bottom_cond;

  bindings = NIL;
  current_binding_point = NIL;

/*  print("Parsing as a rhs...\n"); */
  parsed_ok=parse_rhs(&alist);
  if (!parsed_ok) {
    print("Error: not a valid rhs.\n");
    current_pf_list = NIL;
    return;
  }

/*
  print("Valid RHS:\n");
  print_action_list(alist,0,FALSE);
  print("\nMatches:\n");
*/

  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
        (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
     for (prod=current_agent(all_productions_of_type)[i]; prod!=NIL;
          prod=prod->next) {
      match = TRUE;

      free_binding_list(bindings);
      bindings = NIL;

      p_node_to_conditions_and_nots (prod->p_node, NIL, NIL, &top_cond,
                                     &bottom_cond, NIL, &rhs);
      deallocate_condition_list (top_cond);
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
      deallocate_action_list (rhs);
      if (match) {
        push(prod,(*current_pf_list));
        if (show_bindings) {
          print_with_symbols("%y, with bindings:\n",prod->name);
          print_binding_list(bindings);}
        else
          print_with_symbols("%y\n",prod->name);
      }
    }
  if (bindings) free_binding_list(bindings); /* DJP 4/3/96 -- To catch the last production */
}


Soar_TextWidgetPrintData *
Soar_MakeTextWidgetPrintData (Tcl_Interp * interp, char * widget_name)
{
  Soar_TextWidgetPrintData * data;

  data = (Soar_TextWidgetPrintData *) 
         malloc (sizeof (Soar_TextWidgetPrintData));
  data->interp = interp;
  data->text_widget = savestring(widget_name);

  return data;
}

extern void Soar_FreeTextWidgetPrintData (Soar_TextWidgetPrintData * data)
{
  free((void *) data->text_widget);
  free((void *) data);
}

/* kjh(CUSP-B2) begin */
extern Symbol *make_symbol_for_current_lexeme (void);

typedef struct wme_filter_struct {
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  bool   adds;
  bool   removes;
} wme_filter;

int
wmes_filter_add(Tcl_Interp *interp, char *idStr, char *attrStr, char *valueStr, bool adds, bool removes) {
  Symbol *id,*attr,*value;
  wme_filter *wf, *existing_wf;
  cons *c;
  
  id = NIL;
  attr = NIL;
  value = NIL;
  
  if ( (read_wme_filter_component(interp,idStr,&id) == TCL_ERROR)
    || (read_wme_filter_component(interp,attrStr,&attr) == TCL_ERROR)
    || (read_wme_filter_component(interp,valueStr,&value) == TCL_ERROR))
    goto error_out;

  if (id && attr && value) {
    /* check to see if such a filter has already been added: */
    for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
      existing_wf = (wme_filter *) c->first;
      if (  (existing_wf->adds == adds) && (existing_wf->removes == removes)
         && (existing_wf->id == id) && (existing_wf->attr == attr) && (existing_wf->value == value)) {
         print("Filter already exists.\n");
         goto error_out;	/* already exists */
      }  
    }
    
    wf = allocate_memory (sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE);
    wf->id = id;
    wf->attr = attr;
    wf->value = value;
    wf->adds = adds;
    wf->removes  = removes;
    
    /* Rather than add refs for the new filter symbols and then remove refs 
     * for the identical symbols created from the string parameters, skip
     * the two nullifying steps altogether and just return immediately
     * after pushing the new filter:
     */
    push(wf,current_agent(wme_filter_list));	/* note: nested macro */
    return TCL_OK;
  }
error_out:
  /* clean up symbols created from string parameters */
  if (id) symbol_remove_ref(id);
  if (attr) symbol_remove_ref(attr);
  if (value) symbol_remove_ref(value);
  return TCL_ERROR;
}

int
read_wme_filter_component(Tcl_Interp *interp, char *s, Symbol **sym) {
  get_lexeme_from_string(s);
  if(current_agent(lexeme).type == IDENTIFIER_LEXEME) {
    if ((*sym = find_identifier(current_agent(lexeme).id_letter, 
			      current_agent(lexeme).id_number)) == NIL) {
      sprintf(interp->result, "Error: Only constants or pre-exiting identifiers are allowed.\nThe identifier %s does not exist.\n", s);
      return TCL_ERROR;
    }
  } else
    *sym = make_symbol_for_current_lexeme();
  return TCL_OK;
}

int
wmes_filter_remove(Tcl_Interp *interp, char *idStr, char *attrStr, char *valueStr, bool adds, bool removes) {
  Symbol *id,*attr,*value;
  wme_filter *wf;
  int ret = TCL_ERROR;
  cons *c;
  cons **prev_cons_rest;
  
  id = NIL;
  attr = NIL;
  value = NIL;

  if ( (read_wme_filter_component(interp,idStr,&id) == TCL_ERROR)
    || (read_wme_filter_component(interp,attrStr,&attr) == TCL_ERROR)
    || (read_wme_filter_component(interp,valueStr,&value) == TCL_ERROR))
    goto clean_up;

  if (id && attr && value) {
    prev_cons_rest = &current_agent(wme_filter_list);
    for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
      wf = (wme_filter *) c->first;
      if (  ((adds && wf->adds) || ((removes) && wf->removes))
         && (wf->id == id) && (wf->attr == attr) && (wf->value == value)) {
        *prev_cons_rest = c->rest;
        symbol_remove_ref(id);
        symbol_remove_ref(attr);
        symbol_remove_ref(value);
        free_memory (wf, MISCELLANEOUS_MEM_USAGE);
        free_cons(c);
        break;	/* assume that wmes_filter_add did not add duplicates */
      }
      prev_cons_rest = &(c->rest);
    }
    if (c != NIL)
      ret = TCL_OK; /* filter was sucessfully removed */
  }
clean_up:
  /* clean up symbols created from string parameters */
  if (id) symbol_remove_ref(id);
  if (attr) symbol_remove_ref(attr);
  if (value) symbol_remove_ref(value);
  return ret;
}

int
wmes_filter_reset(Tcl_Interp *interp, bool adds, bool removes) {
  wme_filter *wf;
  cons *c;
  cons **prev_cons_rest;
  bool didRemoveSome;
  
  didRemoveSome = FALSE;
  prev_cons_rest = &current_agent(wme_filter_list);
  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
    wf = (wme_filter *) c->first;
    if ((adds && wf->adds) || (removes && wf->removes)) {
      *prev_cons_rest = c->rest;
      print_with_symbols("Removed: \(%y ^%y %y\) ",wf->id,wf->attr,wf->value);
      print("%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
      symbol_remove_ref(wf->id);
      symbol_remove_ref(wf->attr);
      symbol_remove_ref(wf->value);
      free_memory (wf, MISCELLANEOUS_MEM_USAGE);
      free_cons(c);
      didRemoveSome = TRUE;
    }
    prev_cons_rest = &(c->rest);
  }
  if (didRemoveSome)
    return TCL_OK;
  else
    return TCL_ERROR;
}
int
wmes_filter_list(Tcl_Interp *interp, bool adds, bool removes) {
  wme_filter *wf;
  cons *c;
  
  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
    wf = (wme_filter *) c->first;
    if ((adds && wf->adds) || (removes && wf->removes)) {
      print_with_symbols("wme filter: \(%y ^%y %y\) ",wf->id,wf->attr,wf->value);
      print("%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
    }
  }
  return TCL_OK;
}

bool
wme_filter_component_match(Symbol *filterComponent, Symbol *wmeComponent) {
  if ((filterComponent->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
      (!strcmp(filterComponent->sc.name,"*"))) 
    return TRUE;
  else
    return(filterComponent == wmeComponent);
}

bool
passes_wme_filtering(wme *w, bool isAdd) {
  cons *c;
  wme_filter *wf;

  /*  print ("testing wme for filtering: ");  print_wme(w); */
  
  if (!current_agent(wme_filter_list))
    return TRUE; /* no filters defined -> everything passes */
  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
    wf = (wme_filter *) c->first;
    /*  print_with_symbols("  trying filter: %y ^%y %y\n",wf->id,wf->attr,wf->value); */
    if (   ((isAdd && wf->adds) || ((!isAdd) && wf->removes))
        && wme_filter_component_match(wf->id,w->id)
        && wme_filter_component_match(wf->attr,w->attr)
        && wme_filter_component_match(wf->value,w->value))
      return TRUE;
  }
  return FALSE; /* no defined filters match -> w passes */
}

int
parse_filter_type(char *s, bool *forAdds, bool *forRemoves) {
  if        (string_match(s,"-adds")) {
    *forAdds = TRUE;
    *forRemoves = FALSE;
    return TCL_OK;
  } else if (string_match(s,"-removes")) {
    *forAdds = FALSE;
    *forRemoves = TRUE;
    return TCL_OK;
  } else if (string_match(s,"-both")) {
    *forAdds = TRUE;
    *forRemoves = TRUE;
    return TCL_OK;
  }
  return TCL_ERROR;
}

/* kjh(CUSP-B2) end */


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

void
soar_alternate_input(agent *ai_agent,
                     char  *ai_string, 
                     char  *ai_suffix, 
                     bool   ai_exit   )
{
  ai_agent->alternate_input_string = ai_string;
  ai_agent->alternate_input_suffix = ai_suffix;
  ai_agent->current_char = ' ';
  ai_agent->alternate_input_exit = ai_exit;
  return;
}

