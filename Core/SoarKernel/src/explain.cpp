#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  explain.cpp
 *
 * =======================================================================
 * Description  :  To provide a function which at the least can do :
 *                  (explain chunk-1)
 *		  lists conditions -- given one can list the productions
 *		  which fired & caused it to be in the chunk.
 *
 *		  (It would only run AFTER backtracing).
 * =======================================================================
 */


/*
  NOTES :
     1)  Explain search just finds ANY path--should be shortest.
*/

#include "explain.h"
#include "kernel.h"
#include "agent.h"
#include "backtrace.h"
#include "production.h"
#include "gdatastructs.h"
#include "print.h"

/* Define the "local" globals if that makes sense.   
   (Only accessed in this file) 
   The backtrace_list is built up until a call is made to create a new    
   entry in the chunk_list.  At that time the current backtrace list is   
   included in that structure and a new backtrace list begun.           */

/* static explain_chunk_str *explain_chunk_list;
   static backtrace_str     *explain_backtrace_list;
   static char explain_chunk_name[256] = { '\0' };
   AGR 564 */

/* AGR 564  This bug report came complete with fixes from Frank Koss.
   So, I just implemented the fixes.  The files with changes in them
   for this bug are explain.c, explain.h, 
   init_soar.c, interface.c, and soarkernel.h.  AGR 564 2-May-94  */

/***************************************************************************
 * Function     : init_explain
 **************************************************************************/

void init_explain (agent* thisAgent) {

/* "AGR 564" applies to this whole function */

  thisAgent->explain_chunk_name[0] = '\0';
  thisAgent->explain_chunk_list = NULL;
  thisAgent->explain_backtrace_list = NULL;
  /* added in this initialization, not sure why removed...  KJC 7/96 */
  /* thisAgent->explain_flag = FALSE;
   */
  /*  should we be re-initializing here??  */
  set_sysparam (thisAgent, EXPLAIN_SYSPARAM,FALSE);

/*
 * add_help("explain",help_on_explain);
 * add_command("explain",explain_interface_routine);
 *
 * explain_chunk_list = NULL;
 * explain_backtrace_list = NULL;
 * thisAgent->explain_flag = FALSE;
 */
}

/***************************************************************************
 * Function     : free_backtrace_list
 **************************************************************************/

void free_backtrace_list(agent* thisAgent, backtrace_str *prod) {

backtrace_str *next_prod;

  while (prod != NULL) {
    next_prod = prod->next_backtrace;
    deallocate_condition_list(thisAgent, prod->trace_cond);
    deallocate_condition_list(thisAgent, prod->grounds);
    deallocate_condition_list(thisAgent, prod->potentials);
    deallocate_condition_list(thisAgent, prod->locals);
    deallocate_condition_list(thisAgent, prod->negated);
    free((void *) prod);
    prod = next_prod;
  }
}

/***************************************************************************
 * Function     : reset_backtrace_list
 **************************************************************************/

void reset_backtrace_list (agent* thisAgent) {

  free_backtrace_list(thisAgent, thisAgent->explain_backtrace_list);
  thisAgent->explain_backtrace_list = NULL;
/* AGR 564  In both statements, the current_agent(...) was added.  2-May-94 */

}

/***************************************************************************
 * Function     : copy_cond_list
 **************************************************************************/

condition * copy_cond_list(agent* thisAgent, condition *top_list) {

condition *new_top, *new_bottom;

  copy_condition_list(thisAgent, top_list,&new_top,&new_bottom);
  return (new_top);
}

/***************************************************************************
 * Function     : copy_conds_from_list
 **************************************************************************/

condition *copy_conds_from_list(agent* thisAgent, cons *top_list) {

condition *top,*cond,*prev,*next;
cons *cc;

  prev = next = top = NIL;

  for (cc=top_list; cc!=NIL; cc=cc->rest) {
    cond = copy_condition(thisAgent, static_cast<condition_struct *>(cc->first));
    cond->prev = prev;
    cond->next = NIL;

    if (prev == NIL)
      top = cond;
    else
      prev->next = cond;

    prev = cond;
  }
  return (top);
}

/***************************************************************************
 * Function     : explain_add_temp_to_backtrace_list
 **************************************************************************/

void explain_add_temp_to_backtrace_list
     (agent* thisAgent, backtrace_str  *temp, cons *grounds, cons *pots, cons *locals, cons *negateds) {

backtrace_str *back;

  back = (backtrace_str *)malloc(sizeof (backtrace_str));
  back->result = temp->result;
  back->trace_cond = copy_condition(thisAgent, temp->trace_cond);
  if (back->trace_cond != NULL)
    back->trace_cond->next = NULL;
  strncpy(back->prod_name,temp->prod_name, BUFFER_PROD_NAME_SIZE);
  back->prod_name[BUFFER_PROD_NAME_SIZE - 1] = 0; /* ensure null termination */

  back->grounds    = copy_conds_from_list(thisAgent, grounds);
  back->potentials = copy_conds_from_list(thisAgent, pots);
  back->locals     = copy_conds_from_list(thisAgent, locals);
  back->negated    = copy_conds_from_list(thisAgent, negateds);

  back->next_backtrace = thisAgent->explain_backtrace_list;
  thisAgent->explain_backtrace_list = back;
/* AGR 564  In last 2 statements, current_agent(...) was added.  2-May-94 */

}

/***************************************************************************
* Function     : explain_add_temp_to_chunk_list
* Description  : Allocate a new chunk structure and copy the information in
*                the temp structure to it.  Also copy in the current
*                "explain_backtrace_list" and reset that list.
*                We want to copy all the information in the chunk/justification
*                in case it is excised or retracted later on and you still
*                want an explanation.  Therefore each item used is carefully
*                copied, rather than just keeping a pointer.
**************************************************************************/

void explain_add_temp_to_chunk_list(agent* thisAgent, explain_chunk_str *temp) {
   
explain_chunk_str *chunk;

  chunk = (explain_chunk_str *)malloc(sizeof (explain_chunk_str));
  chunk->conds   = temp->conds;
  chunk->actions = temp->actions;
  strncpy(chunk->name,temp->name,EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE);
  chunk->name[EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE - 1] = 0;

  chunk->backtrace = thisAgent->explain_backtrace_list;
  thisAgent->explain_backtrace_list = NULL;
/* AGR 564  In last 2 statements, current_agent(...) was added.  2-May-94 */

  chunk->all_grounds = copy_cond_list(thisAgent, temp->all_grounds);

  chunk->next_chunk  = thisAgent->explain_chunk_list;
  thisAgent->explain_chunk_list = chunk;
/* AGR 564  In last 2 statements, current_agent(...) was added.  2-May-94 */

}


/***************************************************************************
 * Function     : free_explain_chunk
 **************************************************************************/

/* Note - the calling procedure must ensure that the list which "chunk" is   
          a part of is correctly updated to allow for its removal.         */

void free_explain_chunk(agent* thisAgent, explain_chunk_str *chunk) {

  /* First free up all the traced productions */
  free_backtrace_list(thisAgent, chunk->backtrace);

  deallocate_condition_list(thisAgent, chunk->conds);
  deallocate_action_list(thisAgent, chunk->actions);
  deallocate_condition_list(thisAgent, chunk->all_grounds);

  /* Then free up this structure */
  free((void *) chunk);
}

/***************************************************************************
 * Function     : reset_explain
 **************************************************************************/

void reset_explain (agent* thisAgent) {
   
  explain_chunk_str *top, *chunk;

  top = thisAgent->explain_chunk_list;
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  while (top != NULL) {
    chunk = top;
    top = top->next_chunk;
    free_explain_chunk(thisAgent, chunk);
  }

  thisAgent->explain_chunk_list = NULL;
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  reset_backtrace_list(thisAgent);
}


/***************************************************************************
 * Function     : find_chunk
 * Description  : Find the data structure associated with an explain chunk by
 *                searching for its name.
 **************************************************************************/

explain_chunk_str *find_chunk(agent* thisAgent, explain_chunk_str *chunk, char *name) {

  while (chunk != NULL) {
    if (strcmp(chunk->name,name) == 0) 
       return(chunk);
    chunk = chunk->next_chunk;
  }

  print(thisAgent, "Could not find the chunk.  Maybe explain was not on when it was created.");
  /* BUGBUG: this doesn't belong here!  changed for bug 608 */
  print (thisAgent, "\nTo turn on explain: save-backtraces --enable before the chunk is created.\n");

  return (NULL);
}


/***************************************************************************
 * Function     : find_ground
 * Description  : Find the numbered condition in the chunk.
 **************************************************************************/

condition *find_ground(agent* thisAgent, explain_chunk_str *chunk, int number) {

  condition *ground, *cond;

  ground = NIL;  /* unnecessary, but gcc -Wall warns without it */
  for (cond = chunk->all_grounds; cond != NIL; cond = cond->next) {
    number--;
    if (number == 0) 
     ground = cond; 
  }
  if (number > 0) {
    print(thisAgent, "Could not find this condition.\n");
    return (NIL);
  }
  return (ground);
}

/***************************************************************************
 * Function     : explain_trace_chunk
 **************************************************************************/

void explain_trace_chunk(agent* thisAgent, explain_chunk_str *chunk) {

backtrace_str *prod;

  print(thisAgent, "Chunk : %s\n",chunk->name);
  prod = chunk->backtrace;
  while (prod != NULL) {
    print(thisAgent, "Backtrace production : %s\n",prod->prod_name);
    print(thisAgent, "Result : %d\n",prod->result);
    if (prod->trace_cond != NULL) {
      print(thisAgent, "Trace condition : ");
      print_condition(thisAgent, prod->trace_cond);
    }
    else
      print(thisAgent, "The result preference is not stored, sorry.\n");
    print_string (thisAgent, "\nGrounds:\n");
    print_list_of_conditions (thisAgent, prod->grounds);
    print_string (thisAgent, "\nPotentials:\n");
    print_list_of_conditions (thisAgent, prod->potentials);
    print_string (thisAgent, "\nLocals:\n");
    print_list_of_conditions (thisAgent, prod->locals);
    print_string (thisAgent, "\nNegateds:\n");
    print_list_of_conditions (thisAgent, prod->negated);
    prod = prod -> next_backtrace;
    print(thisAgent, "\n\n");
  }
}

/***************************************************************************
 * Function     : explain_trace_named_chunk
 **************************************************************************/

void explain_trace_named_chunk(agent* thisAgent, char *chunk_name) {

explain_chunk_str *chunk;

  chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list,chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  if (chunk)
    explain_trace_chunk(thisAgent, chunk);
}

/***************************************************************************
 * Function     : explain_find_cond
 * Description  : Return the matching condition from the list, NULL if no match.
 **************************************************************************/

condition *explain_find_cond(condition *target, condition *cond_list) {
   
condition *cond, *match;

  match = NULL;
  for (cond=cond_list; cond!=NULL; cond=cond->next) {
    if (conditions_are_equal (target,cond))
      match = cond;
  }
  return (match);
}

/***************************************************************************
 * Function     : explain_trace
 * Description  : Search the backtrace structures to explain why the given
 *                condition appeared in the chunk.
 **************************************************************************/

void explain_trace(agent* thisAgent, char *chunk_name, backtrace_str *prod_list, condition *ground) {
   
int count;
condition *match, *target;
backtrace_str *prod;

  /* Find which prod. inst. tested the ground originally to get   
  it included in the chunk.                                    
  Need to check potentials too, in case they got included      
  later on.                                                  */

  prod = prod_list; 
  match = NULL;
  while (prod != NULL && match == NULL)
  {
    match = explain_find_cond(ground,prod->potentials);
    if (match == NULL) match = explain_find_cond(ground,prod->grounds);
    if (match == NULL) match = explain_find_cond(ground,prod->negated);
    if (match == NULL) prod = prod->next_backtrace;
  }

  if (match == NULL) {
    print(thisAgent, "EXPLAIN: Error, couldn't find the ground condition\n");
    return;
  }

  print(thisAgent, "Explanation of why condition ");
  print_condition(thisAgent, ground);
  print(thisAgent, " was included in %s\n\n",chunk_name);

  print(thisAgent, "Production %s matched\n   ",prod->prod_name);
  print_condition(thisAgent, match);
  print(thisAgent, " which caused\n");

  /* Trace back the series of productions to find which one                   
  caused the matched condition to be created.                              
  Build in a safety limit of tracing 50 productions before cancelling.     
  This is in case there is a loop in the search procedure somehow or       
  a really long sequence of production firings.  Either way you probably   
  don't want to see more than 50 lines of junk....                       */

  target = prod->trace_cond; 
  count = 0;

  while (prod->result == FALSE && count < 50 && match != NULL) {
    prod = prod_list; 
    match = NULL; 
    count++;
    while (prod != NULL && match == NULL) {
       match = explain_find_cond(target,prod->locals);
       /* Going to check all the other lists too just to be sure */
       if (match == NULL) match = explain_find_cond(target,prod->negated);
       if (match == NULL) match = explain_find_cond(target,prod->potentials);
       if (match == NULL) match = explain_find_cond(target,prod->grounds);
       if (match == NULL) prod = prod->next_backtrace;
    }

    if (match == NULL) {
      print(thisAgent, "EXPLAIN : Unable to find which production matched condition ");
      print_condition(thisAgent, target);
      print(thisAgent, "\nTo help understand what happened here and help debug this\n");
      print(thisAgent, "here is all of the backtracing information stored for this chunk.\n");
      print(thisAgent, "\n");
       explain_trace_named_chunk(thisAgent, chunk_name);
    }
    else {
      print(thisAgent, "production %s to match\n   ",prod->prod_name);
      print_condition(thisAgent, match);
      print(thisAgent, " which caused\n");
      target = prod->trace_cond;
    }
  }

  if (prod->result == TRUE)
    print(thisAgent, "A result to be generated.\n");
  if (count >= 50)
    print(thisAgent, "EXPLAIN: Exceeded 50 productions traced through, so terminating now.\n");
}

/***************************************************************************
 * Function     : explain_chunk
 * Description  : Explain why the numbered condition appears in the given chunk.
 **************************************************************************/

void explain_chunk(agent* thisAgent, char *chunk_name, int cond_number) {

explain_chunk_str *chunk;
condition *ground;

  chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list,chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  if (chunk == NULL) return;

  ground = find_ground(thisAgent, chunk,cond_number);
  if (ground == NIL) return;

  explain_trace(thisAgent, chunk_name,chunk->backtrace,ground);
}

/***************************************************************************
 * Function     : explain_cond_list
 * Description  : List all of the conditions and number them for a named chunk.
 **************************************************************************/

void explain_cond_list(agent* thisAgent, char *chunk_name) {
   
explain_chunk_str *chunk;
condition *cond, *ground;
int i;

  chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list,chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  if (chunk == NULL) return;

  /* First print out the production in "normal" form */

  print (thisAgent, "(sp %s\n  ", chunk->name);
  print_condition_list (thisAgent, chunk->conds, 2, FALSE);
  print (thisAgent, "\n-->\n   ");
  print_action_list (thisAgent, chunk->actions, 3, FALSE);
  print(thisAgent, ")\n\n");

  /* Then list each condition and the associated "ground" WME */

  i = 0; 
  ground = chunk->all_grounds;

  for (cond = chunk->conds; cond != NIL; cond = cond->next) {
    i++; print(thisAgent, " %2d : ",i);
    print_condition(thisAgent, cond);
    while (get_printer_output_column(thisAgent) < COLUMNS_PER_LINE-40)
      print(thisAgent, " ");
	
    print(thisAgent, " Ground :");
    print_condition(thisAgent, ground);
    print(thisAgent, "\n");
    ground=ground->next;
  }
}


/***************************************************************************
 * Function     : explain_list_chunks
 **************************************************************************/

void explain_list_chunks (agent* thisAgent) {

explain_chunk_str *chunk;

  chunk = thisAgent->explain_chunk_list;
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  if (!chunk)
    print (thisAgent, "No chunks/justifications built yet!\n");
  else {
    print(thisAgent, "List of all explained chunks/justifications:\n");
    while (chunk != NULL) {
      print(thisAgent, "Have explanation for %s\n",chunk->name);
      chunk = chunk->next_chunk;
    }
  }
}

/***************************************************************************
 * Function     : explain_full_trace
 **************************************************************************/

void explain_full_trace (agent* thisAgent) {

explain_chunk_str *chunk;

  chunk = thisAgent->explain_chunk_list;
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

  while (chunk != NULL) {
    explain_trace_chunk(thisAgent, chunk);
    chunk = chunk->next_chunk; 
  }
}

