/*************************************************************************
 *
 *  file:  explain.c
 *
 * =======================================================================
 * Description  :  To provide a function which at the least can do :
 *                  (explain chunk-1)
 *		  lists conditions -- given one can list the productions
 *		  which fired & caused it to be in the chunk.
 *
 *		  (It would only run AFTER backtracing).
 * =======================================================================
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

/*
  NOTES :
     1)  Explain search just finds ANY path--should be shortest.
*/

#include "soarkernel.h"         /* Condition type defs */
#include "soar_ecore_api.h"
#include "explain.h"

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

void init_explain(void)
{

/* "AGR 564" applies to this whole function */

    current_agent(explain_chunk_name[0]) = '\0';
    current_agent(explain_chunk_list) = NULL;
    current_agent(explain_backtrace_list) = NULL;
    /* added in this initialization, not sure why removed...  KJC 7/96 */
    /* current_agent(explain_flag) = FALSE;
     */
    /*  should we be re-initializing here??  */
    set_sysparam(EXPLAIN_SYSPARAM, FALSE);

/*
 * add_help("explain",help_on_explain);
 * add_command("explain",explain_interface_routine);
 *
 * explain_chunk_list = NULL;
 * explain_backtrace_list = NULL;
 * current_agent(explain_flag) = FALSE;
 */
}

/***************************************************************************
 * Function     : free_backtrace_list
 **************************************************************************/

void free_backtrace_list(backtrace_str * prod)
{

    backtrace_str *next_prod;

    while (prod != NULL) {
        next_prod = prod->next_backtrace;
        deallocate_condition_list(prod->trace_cond);
        deallocate_condition_list(prod->grounds);
        deallocate_condition_list(prod->potentials);
        deallocate_condition_list(prod->locals);
        deallocate_condition_list(prod->negated);
        free((void *) prod);
        prod = next_prod;
    }
}

/***************************************************************************
 * Function     : reset_backtrace_list
 **************************************************************************/

void reset_backtrace_list(void)
{

    free_backtrace_list(current_agent(explain_backtrace_list));
    current_agent(explain_backtrace_list) = NULL;
/* AGR 564  In both statements, the current_agent(...) was added.  2-May-94 */

}

/***************************************************************************
 * Function     : copy_cond_list
 **************************************************************************/

condition *copy_cond_list(condition * top_list)
{

    condition *new_top, *new_bottom;

    copy_condition_list(top_list, &new_top, &new_bottom);
    return (new_top);
}

/***************************************************************************
 * Function     : copy_conds_from_list
 **************************************************************************/

condition *copy_conds_from_list(cons * top_list)
{

    condition *top, *cond, *prev, *next;
    cons *cc;

    prev = next = top = NIL;

    for (cc = top_list; cc != NIL; cc = cc->rest) {
        cond = copy_condition(cc->first);
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
    (backtrace_str * temp, cons * grounds, cons * pots, cons * locals, cons * negateds) {

    backtrace_str *back;

    back = (backtrace_str *) malloc(sizeof(backtrace_str));
    back->result = temp->result;
    back->trace_cond = copy_condition(temp->trace_cond);
    if (back->trace_cond != NULL)
        back->trace_cond->next = NULL;
    strncpy(back->prod_name, temp->prod_name, PROD_NAME_SIZE);
    back->prod_name[PROD_NAME_SIZE - 1] = 0;

    back->grounds = copy_conds_from_list(grounds);
    back->potentials = copy_conds_from_list(pots);
    back->locals = copy_conds_from_list(locals);
    back->negated = copy_conds_from_list(negateds);

    back->next_backtrace = current_agent(explain_backtrace_list);
    current_agent(explain_backtrace_list) = back;
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

void explain_add_temp_to_chunk_list(explain_chunk_str * temp)
{

    explain_chunk_str *chunk;

    chunk = (explain_chunk_str *) malloc(sizeof(explain_chunk_str));
    chunk->conds = temp->conds;
    chunk->actions = temp->actions;
    strncpy(chunk->name, temp->name, PROD_NAME_SIZE);
    chunk->name[PROD_NAME_SIZE - 1] = 0;

    chunk->backtrace = current_agent(explain_backtrace_list);
    current_agent(explain_backtrace_list) = NULL;
/* AGR 564  In last 2 statements, current_agent(...) was added.  2-May-94 */

    chunk->all_grounds = copy_cond_list(temp->all_grounds);

    chunk->next_chunk = current_agent(explain_chunk_list);
    current_agent(explain_chunk_list) = chunk;
/* AGR 564  In last 2 statements, current_agent(...) was added.  2-May-94 */

}

/***************************************************************************
 * Function     : free_explain_chunk
 **************************************************************************/

/* Note - the calling procedure must ensure that the list which "chunk" is   
          a part of is correctly updated to allow for its removal.         */

void free_explain_chunk(explain_chunk_str * chunk)
{

    /* First free up all the traced productions */
    free_backtrace_list(chunk->backtrace);

    deallocate_condition_list(chunk->conds);
    deallocate_action_list(chunk->actions);
    deallocate_condition_list(chunk->all_grounds);

    /* Then free up this structure */
    free((void *) chunk);
}

/***************************************************************************
 * Function     : reset_explain
 **************************************************************************/

void reset_explain(void)
{

    explain_chunk_str *top, *chunk;

    top = current_agent(explain_chunk_list);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    while (top != NULL) {
        chunk = top;
        top = top->next_chunk;
        free_explain_chunk(chunk);
    }

    current_agent(explain_chunk_list) = NULL;
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    reset_backtrace_list();
}

/***************************************************************************
 * Function     : find_chunk
 * Description  : Find the data structure associated with an explain chunk by
 *                searching for its name.
 **************************************************************************/

explain_chunk_str *find_chunk(explain_chunk_str * chunk, char *name)
{

    while (chunk != NULL) {
        if (strcmp(chunk->name, name) == 0)
            return (chunk);
        chunk = chunk->next_chunk;
    }

    print("Could not find the chunk.  Maybe explain was not on when it was created.");
    /* BUGBUG *** shouldn't have user interface stuff in kernel!! */
    print("\nFor Soar 7: set save_backtraces 1 before the chunk is created.\n");

    return (NULL);
}

/***************************************************************************
 * Function     : find_ground
 * Description  : Find the numbered condition in the chunk.
 **************************************************************************/

condition *find_ground(explain_chunk_str * chunk, int number)
{

    condition *ground, *cond;

    ground = NIL;               /* unnecessary, but gcc -Wall warns without it */
    for (cond = chunk->all_grounds; cond != NIL; cond = cond->next) {
        number--;
        if (number == 0)
            ground = cond;
    }
    if (number > 0) {
        print("Could not find this condition.\n");
        return (NIL);
    }
    return (ground);
}

/***************************************************************************
 * Function     : explain_trace_chunk
 **************************************************************************/

void explain_trace_chunk(explain_chunk_str * chunk)
{

    backtrace_str *prod;

    print("Chunk : %s\n", chunk->name);
    prod = chunk->backtrace;
    while (prod != NULL) {
        print("Backtrace production : %s\n", prod->prod_name);
        print("Result : %d\n", prod->result);
        if (prod->trace_cond != NULL) {
            print("Trace condition : ");
            print_condition(prod->trace_cond);
        } else
            print("The result preference is not stored, sorry.\n");
        print_string("\nGrounds:\n");
        print_list_of_conditions(prod->grounds);
        print_string("\nPotentials:\n");
        print_list_of_conditions(prod->potentials);
        print_string("\nLocals:\n");
        print_list_of_conditions(prod->locals);
        print_string("\nNegateds:\n");
        print_list_of_conditions(prod->negated);
        prod = prod->next_backtrace;
        print("\n\n");
    }
}

/***************************************************************************
 * Function     : explain_find_cond
 * Description  : Return the matching condition from the list, NULL if no match.
 **************************************************************************/

condition *explain_find_cond(condition * target, condition * cond_list)
{

    condition *cond, *match;

    match = NULL;
    for (cond = cond_list; cond != NULL; cond = cond->next) {
        if (conditions_are_equal(target, cond))
            match = cond;
    }
    return (match);
}

/***************************************************************************
 * Function     : explain_trace
 * Description  : Search the backtrace structures to explain why the given
 *                condition appeared in the chunk.
 **************************************************************************/

void explain_trace(char *chunk_name, backtrace_str * prod_list, condition * ground)
{

    int count;
    condition *match, *target;
    backtrace_str *prod;

    /* Find which prod. inst. tested the ground originally to get   
       it included in the chunk.                                    
       Need to check potentials too, in case they got included      
       later on.                                                  */

    prod = prod_list;
    match = NULL;
    while (prod != NULL && match == NULL) {
        match = explain_find_cond(ground, prod->potentials);
        if (match == NULL)
            match = explain_find_cond(ground, prod->grounds);
        if (match == NULL)
            match = explain_find_cond(ground, prod->negated);
        if (match == NULL)
            prod = prod->next_backtrace;
    }

    if (match == NULL) {
        print("EXPLAIN: Error, couldn't find the ground condition\n");
        return;
    }

    print("Explanation of why condition ");
    print_condition(ground);
    print(" was included in %s\n\n", chunk_name);

    print("Production %s matched\n   ", prod->prod_name);
    print_condition(match);
    print(" which caused\n");

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
            match = explain_find_cond(target, prod->locals);
            /* Going to check all the other lists too just to be sure */
            if (match == NULL)
                match = explain_find_cond(target, prod->negated);
            if (match == NULL)
                match = explain_find_cond(target, prod->potentials);
            if (match == NULL)
                match = explain_find_cond(target, prod->grounds);
            if (match == NULL)
                prod = prod->next_backtrace;
        }

        if (match == NULL) {
            print("EXPLAIN : Unable to find which production matched condition ");
            print_condition(target);
            print("\nTo help understand what happened here and help debug this\n");
            print("here is all of the backtracing information stored for this chunk.\n");
            print("\n");
            soar_ecExplainChunkTrace(chunk_name);
        } else {
            print("production %s to match\n   ", prod->prod_name);
            print_condition(match);
            print(" which caused\n");
            target = prod->trace_cond;
        }
    }

    if (prod->result == TRUE)
        print("A result to be generated.\n");
    if (count >= 50)
        print("EXPLAIN: Exceeded 50 productions traced through, so terminating now.\n");
}

/***************************************************************************
 * Function     : explain_list_chunks
 **************************************************************************/

void explain_list_chunks(void)
{

    explain_chunk_str *chunk;

    chunk = current_agent(explain_chunk_list);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    if (!chunk)
        print("No chunks/justifications built yet!\n");
    else {
        print("List of all explained chunks/justifications:\n");
        while (chunk != NULL) {
            print("Have explanation for %s\n", chunk->name);
            chunk = chunk->next_chunk;
        }
    }
}

/***************************************************************************
 * Function     : explain_full_trace
 **************************************************************************/

void explain_full_trace(void)
{

    explain_chunk_str *chunk;

    chunk = current_agent(explain_chunk_list);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    while (chunk != NULL) {
        explain_trace_chunk(chunk);
        chunk = chunk->next_chunk;
    }
}
