#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  print.cpp
 *
 * =======================================================================
 *  These are the routines that support printing Soar data structures.
 *  
 * obsolete comments deleted
 * =======================================================================
 */
/* =================================================================
                 Printing Utility Routines for Soar 6
   ================================================================= */

#include <stdlib.h>

#include "print.h"
#include "kernel.h"
#include "agent.h"
#include "symtab.h"
#include "init_soar.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "rete.h"
#include "rhsfun.h"
#include "production.h"
#include "instantiations.h"
#include "xml.h"
#include "soar_TraceNames.h"

#include <stdarg.h>

using namespace soar_TraceNames;

/* -------------------------------------------------------------------
   Print_string() and print_spaces() do the obvious things.
   Print() is exactly like printf() in C, except it prints to both
   the screen and log file (if there is one).  Print_with_symbols()
   is sort of like print, but only takes two kinds of escape sequences
   in the format string: 
       %y  -- print a symbol
       %%  -- print a "%" sign

   Sometimes we need to know the current output column so we can put
   a line break in the right place.  Get_printer_output_column() returns
   the current column number (1 means the start of the line). 
   Tell_printer_that_output_column_has_been_reset () is called from the
   lexer every time it reads a line from the keyboard--since after the
   user types a line (and hits return) the output column is reset.
------------------------------------------------------------------- */

int get_printer_output_column (agent* thisAgent) {
  return thisAgent->printer_output_column;
}

void tell_printer_that_output_column_has_been_reset (agent* thisAgent) {
  thisAgent->printer_output_column = 1;
}


/* -----------------------------------------------------------------------
                             Print_string

   This routine prints the given string, and updates printer_output_column.  
   (This routine is called from the other print(), etc. routines.)
----------------------------------------------------------------------- */



void print_string (agent* thisAgent, const char *s) {
	const char *ch;

	for (ch=s; *ch!=0; ch++) {
		if (*ch=='\n')
			thisAgent->printer_output_column = 1;
		else
			thisAgent->printer_output_column++;
	}

	soar_invoke_callbacks(thisAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char *>(s)));
}

/* ---------------------------------------------------------------
               Print, Print_with_symbols, Print_spaces
  
   These are the main printing routines.  (The code is ugly because
   it has to take a variable number of arguments, and there are two
   ways to do this, depending on whether we're using a fully ANSI
   compatible compiler or not.)
--------------------------------------------------------------- */

/* --- size of output buffer for a single call to one of these routines --- */
#define PRINT_BUFSIZE 5000   /* This better be large enough!! */

void print (agent* thisAgent, const char *format, ...) {
  va_list args;
  char buf[PRINT_BUFSIZE];

  va_start (args, format);
  vsprintf (buf, format, args);
  va_end (args);
  print_string (thisAgent, buf);
}

void print_with_symbols (agent* thisAgent, 
						 const char *format, ...) {
  va_list args;
  char buf[PRINT_BUFSIZE];
  char *ch;
  
  va_start (args, format);
  ch = buf;
  while (TRUE) {
    /* --- copy anything up to the first "%" --- */
    while ((*format != '%') && (*format != 0)) *(ch++) = *(format++);
    if (*format == 0) break;
    /* --- handle the %-thingy --- */
    if (*(format+1)=='y') {
		/* the size of the remaining buffer (after ch) is
			the difference between the address of ch and
			the address of the beginning of the buffer
			*/
      symbol_to_string (thisAgent, va_arg(args, Symbol *), TRUE, ch, PRINT_BUFSIZE - (ch - buf));
      while (*ch) ch++;
    } else {
      *(ch++) = '%';
    }
    format += 2;
  }
  va_end (args);

  *ch = 0;
  print_string (thisAgent, buf);
}

void print_spaces (agent* thisAgent, int n) {
  char *ch;
  char buf[PRINT_BUFSIZE];

  ch = buf;
  while (n) { *(ch++)=' '; n--; }
  *ch=0;
  print_string (thisAgent, buf);
}

/* ------------------------------------------------------------------------
                String to Escaped String Conversion
           {Symbol, Test, RHS Value} to String Conversion

   These routines produce strings.  Each takes an optional parameter "dest"
   which, if non-nil, points to the destination buffer for the result string.
   If dest is nil, these routines use a global buffer, and return a pointer
   to it.  (Otherwise "dest" itself is returned.)  Note that a single global
   buffer is shared by all three routines, so callers should assume the
   buffer will be destroyed by the next call to these routines with dest=NIL.

   String_to_escaped_string() takes a string and a first/last char,
   and produces an "escaped string" representation of the string; i.e.,
   a string that uses '\' escapes to include special characters.
   For example, input 'ab"c' with first/last character '"' yields
   '"ab\"c"'.  This is used for printing quoted strings and for printing
   symbols using |vbar| notation.
 
   Symbol_to_string() converts a symbol to a string.  The "rereadable"
   parameter indicates whether a rereadable representation is desired.
   Normally symbols are printed rereadably, but for (write) and Text I/O,
   we don't want this.

   Test_to_string() takes a test and produces a string representation.
   Rhs_value_to_string() takes an rhs_value and produces a string
   representation.  The rhs_value MUST NOT be a reteloc.
----------------------------------------------------------------------- */

char *string_to_escaped_string (agent* thisAgent, char *s, 
								char first_and_last_char, char *dest) {
  char *ch;

  if (!dest) dest = thisAgent->printed_output_string;
  ch = dest;
  *ch++ = first_and_last_char;
  while (*s) {
    if ((*s==first_and_last_char)||(*s=='\\')) *ch++ = '\\';
    *ch++ = *s++;
  }
  *ch++ = first_and_last_char;
  *ch = 0;
  return dest;
}


char const* symbol_to_typeString(agent* /*thisAgent*/, Symbol* sym)
{
  switch(sym->common.symbol_type) {
  case VARIABLE_SYMBOL_TYPE:
	  return kTypeVariable ;
  case IDENTIFIER_SYMBOL_TYPE:
	  return kTypeID ;
  case INT_CONSTANT_SYMBOL_TYPE:
	  return kTypeInt ;
  case FLOAT_CONSTANT_SYMBOL_TYPE:
	  return kTypeDouble ;
  case SYM_CONSTANT_SYMBOL_TYPE:
	  return kTypeString ;
  default:
	  return 0 ;
  }
}

char *symbol_to_string (agent* thisAgent, Symbol *sym, 
						Bool rereadable, char *dest, size_t dest_size) {
  Bool possible_id, possible_var, possible_ic, possible_fc;
  Bool is_rereadable;

  switch(sym->common.symbol_type) {
  case VARIABLE_SYMBOL_TYPE:
    if (!dest) return sym->var.name;
    strncpy (dest, sym->var.name, dest_size);
	dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;

  case IDENTIFIER_SYMBOL_TYPE:
	if (!dest) {
	  dest=thisAgent->printed_output_string;
	  dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
	}
	if (sym->id.smem_lti == NIL) {
		// NOT an lti (long term identifier), print like we always have 
	    SNPRINTF (dest, dest_size, "%c%llu", sym->id.name_letter, static_cast<unsigned long long>(sym->id.name_number));
	} 
	else {
		// IS an lti (long term identifier), prepend an @ symbol
	    SNPRINTF (dest, dest_size, "@%c%llu", sym->id.name_letter, static_cast<unsigned long long>(sym->id.name_number));
	}
	dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;

  case INT_CONSTANT_SYMBOL_TYPE:
	if (!dest) {
	  dest=thisAgent->printed_output_string;
	  dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
	}
    SNPRINTF (dest, dest_size, "%ld", sym->ic.value);
	dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
    
  case FLOAT_CONSTANT_SYMBOL_TYPE:
	if (!dest) {
	  dest=thisAgent->printed_output_string;
	  dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
	}
    SNPRINTF (dest, dest_size, "%#.16g", sym->fc.value);
	dest[dest_size - 1] = 0; /* ensure null termination */
    { /* --- strip off trailing zeros --- */
      char *start_of_exponent;
      char *end_of_mantissa;
      start_of_exponent = dest;
      while ((*start_of_exponent != 0) && (*start_of_exponent != 'e'))
        start_of_exponent++;
      end_of_mantissa = start_of_exponent - 1;
      while (*end_of_mantissa == '0') end_of_mantissa--;
      end_of_mantissa++;
      while (*start_of_exponent) *end_of_mantissa++ = *start_of_exponent++;
      *end_of_mantissa = 0;
    }
    return dest;
    
  case SYM_CONSTANT_SYMBOL_TYPE:
    if (!rereadable) {
      if (!dest) return sym->sc.name;
      strncpy (dest, sym->sc.name, dest_size);
      return dest;
    }
    determine_possible_symbol_types_for_string (sym->sc.name,
                                                strlen (sym->sc.name),
                                                &possible_id,
                                                &possible_var,
                                                &possible_ic,
                                                &possible_fc,
                                                &is_rereadable);

    if ( possible_var || possible_ic || possible_fc ||
        (!is_rereadable) || possible_id)
    {
      return string_to_escaped_string (thisAgent, sym->sc.name, '|', dest);
    }
    if (!dest) return sym->sc.name;
    strncpy (dest, sym->sc.name, dest_size);
    return dest;
    
  default:
    { 
	  char msg[BUFFER_MSG_SIZE];
      strncpy(msg, "Internal Soar Error:  symbol_to_string called on bad symbol\n", BUFFER_MSG_SIZE);
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
    }
  }
  return NIL; /* unreachable, but without it, gcc -Wall warns here */
}




char *test_to_string (agent* thisAgent, test t, char *dest, size_t dest_size) {
  cons *c;
  complex_test *ct;
  char *ch;

  if (test_is_blank_test(t)) {
    if (!dest) dest=thisAgent->printed_output_string;
    strncpy (dest, "[BLANK TEST]", dest_size);  /* this should never get executed */
	dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
  }

  if (test_is_blank_or_equality_test(t)) {
    return symbol_to_string (thisAgent, referent_of_equality_test(t), TRUE, dest, dest_size);
  }

  if (!dest) {
 	dest=thisAgent->printed_output_string;
	dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
  }
  ch = dest;
  ct = complex_test_from_test(t);
  
  switch (ct->type) {
  case NOT_EQUAL_TEST:
    strncpy (ch, "<> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) 
		ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case LESS_TEST:
    strncpy (ch, "< ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case GREATER_TEST:
    strncpy (ch, "> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case LESS_OR_EQUAL_TEST:
    strncpy (ch, "<= ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case GREATER_OR_EQUAL_TEST:
    strncpy (ch, ">= ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case SAME_TYPE_TEST:
    strncpy (ch, "<=> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case DISJUNCTION_TEST:
    strncpy (ch, "<< ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    for (c=ct->data.disjunction_list; c!=NIL; c=c->rest) {
      symbol_to_string (thisAgent, static_cast<symbol_union *>(c->first), TRUE, ch, dest_size - (ch - dest)); 
	  while (*ch) ch++;
      *(ch++) = ' ';
    }
    strncpy (ch, ">>", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case CONJUNCTIVE_TEST:
    strncpy (ch, "{ ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest) {
      test_to_string (thisAgent, static_cast<char *>(c->first), ch, dest_size - (ch - dest)); 
	  while (*ch) ch++;
      *(ch++) = ' ';
    }
    strncpy (ch, "}", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case GOAL_ID_TEST:
    strncpy (dest, "[GOAL ID TEST]", dest_size - (ch - dest)); /* this should never get executed */
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case IMPASSE_ID_TEST:
    strncpy (dest, "[IMPASSE ID TEST]", dest_size - (ch - dest)); /* this should never get executed */
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  }
  return dest;
}

char *rhs_value_to_string (agent* thisAgent, rhs_value rv, char *dest, size_t dest_size) {
  cons *c;
  list *fl;
  rhs_function *rf;  
  char *ch;

  if (rhs_value_is_reteloc(rv)) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "Internal error: rhs_value_to_string called on reteloc.\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
  
  if (rhs_value_is_symbol(rv)) {
    return symbol_to_string (thisAgent, rhs_value_to_symbol(rv), TRUE, dest, dest_size);
  }

  fl = rhs_value_to_funcall_list(rv);
  rf = static_cast<rhs_function_struct *>(fl->first);
  
  if (!dest) {
 	dest=thisAgent->printed_output_string;
	dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
  }
  ch = dest;

  strncpy (ch, "(", dest_size); 
  ch[dest_size - 1] = 0;
  while (*ch) ch++;

  if (!strcmp(rf->name->sc.name,"+")) {
	strncpy (ch, "+", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0;
  } else if (!strcmp(rf->name->sc.name,"-")) {
    strncpy (ch, "-", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0;
  } else {
	symbol_to_string (thisAgent, rf->name, TRUE, ch, dest_size - (ch - dest));
  }

  while (*ch) ch++;
  for (c=fl->rest; c!=NIL; c=c->rest) {
    strncpy (ch, " ", dest_size - (ch - dest)); 
	ch[dest_size - (ch - dest) - 1] = 0;
	while (*ch) 
		ch++;
    rhs_value_to_string (thisAgent, static_cast<char *>(c->first), ch, dest_size - (ch - dest)); 
	while (*ch) 
		ch++;
  }
  strncpy (ch, ")", dest_size - (ch - dest));
  ch[dest_size - (ch - dest) - 1] = 0;
  return dest;
}

/* ------------------------------------------------------------------
                        Print Condition List

   This prints a list of conditions.  The "indent" parameter tells
   how many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if TRUE, indicates that the condition list should be printed in
   internal format--one condition per line, without grouping all the
   conditions for the same id into one line.
------------------------------------------------------------------ */

Bool pick_conds_with_matching_id_test (dl_cons *dc, agent* thisAgent) {
  condition *cond;
  cond = static_cast<condition_struct *>(dc->item);
  if (cond->type==CONJUNCTIVE_NEGATION_CONDITION) return FALSE;
  return tests_are_equal (thisAgent->id_test_to_match, cond->data.tests.id_test, false);
}

/*
==============================

==============================
*/
#define PRINT_CONDITION_LIST_TEMP_SIZE 10000
void print_condition_list (agent* thisAgent, condition *conds, 
						   int indent, Bool internal) {
   dl_list *conds_not_yet_printed, *tail_of_conds_not_yet_printed;
   dl_list *conds_for_this_id;
   dl_cons *dc;
   condition *c;
   Bool removed_goal_test, removed_impasse_test;
   test id_test;  

   if (!conds) return;

   /* --- build dl_list of all the actions --- */
   conds_not_yet_printed = NIL;
   tail_of_conds_not_yet_printed = NIL;

   for (c=conds; c!=NIL; c=c->next) 
   {
      allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
      dc->item = c;
      if (conds_not_yet_printed) 
      {
         tail_of_conds_not_yet_printed->next = dc;
      }
      else 
      {
         conds_not_yet_printed = dc;
      }
      dc->prev = tail_of_conds_not_yet_printed;
      tail_of_conds_not_yet_printed = dc;
   }
   tail_of_conds_not_yet_printed->next = NIL;

   /* --- main loop: find all conds for first id, print them together --- */
   Bool did_one_line_already = FALSE;
   while (conds_not_yet_printed) 
   {
      if (did_one_line_already) 
      {
         print (thisAgent, "\n");
         print_spaces (thisAgent, indent);
      } 
      else 
      {
         did_one_line_already = TRUE;
      }

      dc = conds_not_yet_printed;
      remove_from_dll (conds_not_yet_printed, dc, next, prev);
      c = static_cast<condition_struct *>(dc->item);
      if (c->type==CONJUNCTIVE_NEGATION_CONDITION) 
      {
         free_with_pool (&thisAgent->dl_cons_pool, dc);
         print_string (thisAgent, "-{");
         xml_begin_tag(thisAgent, kTagConjunctive_Negation_Condition);
         print_condition_list (thisAgent, c->data.ncc.top, indent+2, internal);
         xml_end_tag(thisAgent, kTagConjunctive_Negation_Condition);
         print_string (thisAgent, "}");
         continue;
      }

      /* --- normal pos/neg conditions --- */
      removed_goal_test = removed_impasse_test = FALSE;
      id_test = copy_test_removing_goal_impasse_tests(thisAgent, c->data.tests.id_test, 
         &removed_goal_test, 
         &removed_impasse_test);
      thisAgent->id_test_to_match = copy_of_equality_test_found_in_test (thisAgent, id_test);

      /* --- collect all cond's whose id test matches this one --- */
      conds_for_this_id = dc;
      dc->prev = NIL;
      if (internal) 
      {
         dc->next = NIL;
      } 
      else 
      {
         dc->next = extract_dl_list_elements (thisAgent, &conds_not_yet_printed,
                                               pick_conds_with_matching_id_test);
      }

      /* --- print the collected cond's all together --- */
      print_string (thisAgent, " (");
      xml_begin_tag(thisAgent, kTagCondition);

      if (removed_goal_test) 
      {
         print_string (thisAgent, "state ");
         xml_att_val(thisAgent, kConditionTest, kConditionTestState);

      }

      if (removed_impasse_test) 
      {
         print_string (thisAgent, "impasse ");
         xml_att_val(thisAgent, kConditionTest, kConditionTestImpasse);
      }

      print_string (thisAgent, test_to_string (thisAgent, id_test, NULL, 0));
      xml_att_val(thisAgent, kConditionId, test_to_string (thisAgent, id_test, NULL, 0));
      deallocate_test (thisAgent, thisAgent->id_test_to_match);
      deallocate_test (thisAgent, id_test);

      growable_string gs = make_blank_growable_string(thisAgent);
      while (conds_for_this_id) 
      {
         dc = conds_for_this_id;
         conds_for_this_id = conds_for_this_id->next;
         c = static_cast<condition_struct *>(dc->item);
         free_with_pool (&thisAgent->dl_cons_pool, dc);

         { /* --- build and print attr/value test for condition c --- */
            char temp[PRINT_CONDITION_LIST_TEMP_SIZE], *ch;

            memset(temp,0,PRINT_CONDITION_LIST_TEMP_SIZE);
            ch = temp;
            strncpy (ch, " ", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
            if (c->type==NEGATIVE_CONDITION) 
            {
               strncat (ch, "-", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
            }
            while (*ch) ch++;

            strncpy (ch, "^", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
            while (*ch) ch++;
            test_to_string (thisAgent, c->data.tests.attr_test, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
            while (*ch) ch++;
            if (! test_is_blank_test(c->data.tests.value_test)) 
            {
               *(ch++) = ' ';
               test_to_string (thisAgent, c->data.tests.value_test, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
               while (*ch) ch++;
               if (c->test_for_acceptable_preference) 
               {
                  strncpy (ch, " +", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
                  while (*ch) ch++;
               }
            }
            *ch = 0;
            if (thisAgent->printer_output_column + (ch - temp) >= COLUMNS_PER_LINE) 
            {
               print_string (thisAgent, "\n");
               print_spaces (thisAgent, indent+6);
            }
            print_string (thisAgent, temp);
            add_to_growable_string(thisAgent, &gs, temp);
         }
      }
      xml_att_val(thisAgent, kCondition, text_of_growable_string(gs));
      free_growable_string(thisAgent, gs);
      print_string (thisAgent, ")");
      xml_end_tag(thisAgent, kTagCondition);
   } /* end of while (conds_not_yet_printed) */
}

/* ------------------------------------------------------------------
                        Print Action List

   This prints a list of actions.  The "indent" parameter tells how
   many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if TRUE, indicates that the action list should be printed in
   internal format--one action per line, without grouping all the
   actions for the same id into one line.
   Note:  the actions MUST NOT contain any reteloc's.
------------------------------------------------------------------ */

Bool pick_actions_with_matching_id (dl_cons *dc, agent* thisAgent) {
  action *a;
  a = static_cast<action_struct *>(dc->item);
  if (a->type!=MAKE_ACTION) return FALSE;
  return (rhs_value_to_symbol(a->id) == thisAgent->action_id_to_match);
}

#define PRINT_ACTION_LIST_TEMP_SIZE 10000
void print_action_list (agent* thisAgent, action *actions, 
						int indent, Bool internal) {
  Bool did_one_line_already;
  dl_list *actions_not_yet_printed, *tail_of_actions_not_yet_printed;
  dl_list *actions_for_this_id;
  dl_cons *dc;
  action *a;

  if (!actions) return;
  
  did_one_line_already = FALSE;

  /* --- build dl_list of all the actions --- */
  actions_not_yet_printed = NIL;
  tail_of_actions_not_yet_printed = NIL;
  for (a=actions; a!=NIL; a=a->next) {
    allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
    dc->item = a;
    if (actions_not_yet_printed) tail_of_actions_not_yet_printed->next = dc;
    else actions_not_yet_printed = dc;
    dc->prev = tail_of_actions_not_yet_printed;
    tail_of_actions_not_yet_printed = dc;
  }
  tail_of_actions_not_yet_printed->next = NIL;

  /* --- main loop: find all actions for first id, print them together --- */
  while (actions_not_yet_printed) {
    if (did_one_line_already) {
      print (thisAgent, "\n");
      print_spaces (thisAgent, indent);
    } else {
      did_one_line_already = TRUE;
    }
    dc = actions_not_yet_printed;
    remove_from_dll (actions_not_yet_printed, dc, next, prev);
    a = static_cast<action_struct *>(dc->item);
    if (a->type==FUNCALL_ACTION) {
      free_with_pool (&thisAgent->dl_cons_pool, dc);
      xml_begin_tag(thisAgent, kTagAction);
      print_string (thisAgent, rhs_value_to_string (thisAgent, a->value, NULL, 0));
      xml_att_val(thisAgent, kAction, rhs_value_to_string (thisAgent, a->value, NULL, 0));
      xml_end_tag(thisAgent, kTagAction);
      continue;
    }

    /* --- normal make actions --- */
    /* --- collect all actions whose id matches the first action's id --- */
    actions_for_this_id = dc;
    thisAgent->action_id_to_match = rhs_value_to_symbol(a->id);
    dc->prev = NIL;
    if (internal) {
      dc->next = NIL;
    } else {
      dc->next = extract_dl_list_elements (thisAgent, &actions_not_yet_printed,
                                           pick_actions_with_matching_id);
    }

    /* --- print the collected actions all together --- */
    print_with_symbols (thisAgent, "(%y", thisAgent->action_id_to_match);
    xml_begin_tag(thisAgent, kTagAction);
    xml_att_val(thisAgent, kActionId, thisAgent->action_id_to_match);
    growable_string gs = make_blank_growable_string(thisAgent);
    while (actions_for_this_id) {
      dc = actions_for_this_id;
      actions_for_this_id = actions_for_this_id->next;
      a = static_cast<action_struct *>(dc->item);
      free_with_pool (&thisAgent->dl_cons_pool, dc);

      { /* --- build and print attr/value test for action a --- */
        char temp[PRINT_ACTION_LIST_TEMP_SIZE], *ch;

        ch = temp;
        strncpy (ch, " ^", PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp)); while (*ch) ch++;
        rhs_value_to_string (thisAgent, a->attr, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp)); 
		while (*ch) ch++;
        *(ch++) = ' ';
        rhs_value_to_string (thisAgent, a->value, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp)); 
		while (*ch) ch++;
        *(ch++) = ' ';
        *(ch++) = preference_type_indicator (thisAgent, a->preference_type);
        if (preference_is_binary (a->preference_type)) {
          *(ch++) = ' ';
          rhs_value_to_string (thisAgent, a->referent, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp)); 
		  while (*ch) ch++;
        }
        *ch = 0;
        if (thisAgent->printer_output_column + (ch - temp) >=
            COLUMNS_PER_LINE) {
          print_string (thisAgent, "\n");
          print_spaces (thisAgent, indent+6);
        }
        print_string (thisAgent, temp);
        add_to_growable_string(thisAgent, &gs, temp);
      }
    }
    xml_att_val(thisAgent, kAction, text_of_growable_string(gs));
    free_growable_string(thisAgent, gs);
    print_string (thisAgent, ")");
    xml_end_tag(thisAgent, kTagAction);
  } /* end of while (actions_not_yet_printed) */
}

/* ------------------------------------------------------------------
                         Print Production

   This prints a production.  The "internal" parameter, if TRUE,
   indicates that the LHS and RHS should be printed in internal format.
------------------------------------------------------------------ */

void print_production (agent* thisAgent, production *p, Bool internal) {
  condition *top, *bottom;
  action *rhs;

  /* 
  --- print "sp" and production name --- 
  */
  print_with_symbols (thisAgent, "sp {%y\n", p->name);

  xml_begin_tag(thisAgent, kTagProduction);
  xml_att_val(thisAgent, kProduction_Name, p->name);
  
  /* 
  --- print optional documention string --- 
  */
  if (p->documentation) 
  {
    char temp[MAX_LEXEME_LENGTH*2+10];
    string_to_escaped_string (thisAgent, p->documentation, '"', temp);
    print (thisAgent, "    %s\n", temp);
	xml_att_val(thisAgent, kProductionDocumentation, temp);
  }
  
  /* 
  --- print any flags --- 
  */
  switch (p->type) 
  {
  case DEFAULT_PRODUCTION_TYPE: 
     print_string (thisAgent, "    :default\n");
     xml_att_val(thisAgent, kProductionType, kProductionTypeDefault);
     break;
  case USER_PRODUCTION_TYPE: 
     break;
  case CHUNK_PRODUCTION_TYPE: 
     print_string (thisAgent, "    :chunk\n"); 
     xml_att_val(thisAgent, kProductionType, kProductionTypeChunk);
     break;
  case JUSTIFICATION_PRODUCTION_TYPE:
    print_string (thisAgent, "    :justification ;# not reloadable\n");
    xml_att_val(thisAgent, kProductionType, kProductionTypeJustification);
    break;
  case TEMPLATE_PRODUCTION_TYPE:
	print_string (thisAgent, "   :template\n");
    xml_att_val(thisAgent, kProductionType, kProductionTypeTemplate);
	break;
  }

  if (p->declared_support==DECLARED_O_SUPPORT)
  {
    print_string (thisAgent, "    :o-support\n");
    xml_att_val(thisAgent, kProductionDeclaredSupport, kProductionDeclaredOSupport);
  }

  else if (p->declared_support==DECLARED_I_SUPPORT)
  {
    print_string (thisAgent, "    :i-support\n");
    xml_att_val(thisAgent, kProductionDeclaredSupport, kProductionDeclaredISupport);
  }

  if (p->interrupt)
    print_string(thisAgent, "    :interrupt\n");

  /* 
  --- print the LHS and RHS --- 
  */
  p_node_to_conditions_and_nots (thisAgent, p->p_node, NIL, NIL, 
								 &top, &bottom, NIL,&rhs);
  print_string (thisAgent, "   ");
  
  xml_begin_tag(thisAgent, kTagConditions);
  print_condition_list (thisAgent, top, 3, internal);
  xml_end_tag(thisAgent, kTagConditions);
  deallocate_condition_list (thisAgent, top);
  
  print_string (thisAgent, "\n    -->\n  ");
  print_string (thisAgent, "  ");
  xml_begin_tag(thisAgent, kTagActions);
  print_action_list (thisAgent, rhs, 4, internal);
  xml_end_tag(thisAgent, kTagActions);
  print_string (thisAgent, "\n}\n");
  xml_end_tag(thisAgent, kTagProduction);
  
  deallocate_action_list (thisAgent, rhs);
}

/* ------------------------------------------------------------------
                       Other Printing Utilities

   Print_condition() prints a single condition.  Print_action() prints
   a single action (which MUST NOT contain any reteloc's).
   Note that these routines work by calling print_condition_list() and
   print_action_list(), respectively, so they print a linefeed if the
   output would go past COLUMNS_PER_LINE.

   Preference_type_indicator() returns a character corresponding to
   a given preference type (byte)--for example, given BEST_PREFERENCE_TYPE,
   it returns '>'.

   Print_preference() prints a given preference.  Print_wme() prints a
   wme (including the timetag).  Print_instantiation_with_wmes() prints
   an instantiation's production name and the wmes it matched, using a
   given wme_trace_type (e.g., TIMETAG_WME_TRACE).
------------------------------------------------------------------ */

void print_condition (agent* thisAgent, condition *cond) {
  condition *old_next, *old_prev;

  old_next = cond->next;
  old_prev = cond->prev;
  cond->next = NIL;
  cond->prev = NIL;
  print_condition_list (thisAgent, cond, 0, TRUE);
  cond->next = old_next;
  cond->prev = old_prev;
}

void print_action (agent* thisAgent, action *a) {
  action *old_next;

  old_next = a->next;
  a->next = NIL;
  print_action_list (thisAgent, a, 0, TRUE);
  a->next = old_next;
}

char preference_type_indicator (agent* thisAgent, byte type) {
  switch (type) {
  case ACCEPTABLE_PREFERENCE_TYPE: return '+';
  case REQUIRE_PREFERENCE_TYPE: return '!';
  case REJECT_PREFERENCE_TYPE: return '-';
  case PROHIBIT_PREFERENCE_TYPE: return '~';
  case RECONSIDER_PREFERENCE_TYPE: return '@';
  case NUMERIC_INDIFFERENT_PREFERENCE_TYPE: return '=';
  case UNARY_INDIFFERENT_PREFERENCE_TYPE: return '=';
  case BINARY_INDIFFERENT_PREFERENCE_TYPE: return '=';
  case UNARY_PARALLEL_PREFERENCE_TYPE: return '&';
  case BINARY_PARALLEL_PREFERENCE_TYPE: return '&';
  case BEST_PREFERENCE_TYPE: return '>';
  case BETTER_PREFERENCE_TYPE: return '>';
  case WORST_PREFERENCE_TYPE: return '<';
  case WORSE_PREFERENCE_TYPE: return '<';
  default:
    { char msg[BUFFER_MSG_SIZE];
    strncpy(msg,
	   "print.c: Error: bad type passed to preference_type_indicator\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    }
  }
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

void print_preference (agent* thisAgent, preference *pref) {
  char pref_type = preference_type_indicator (thisAgent, pref->type);

  print_with_symbols (thisAgent, "(%y ^%y %y ", pref->id, pref->attr, pref->value);
  print (thisAgent, "%c", pref_type);
  if (preference_is_binary(pref->type)) {
    print_with_symbols (thisAgent, " %y", pref->referent);
  }
  if (pref->o_supported) print_string (thisAgent, "  :O ");
  print_string (thisAgent, ")");
  print (thisAgent, "\n");

  // <preference id="s1" attr="foo" value="123" pref_type=">"></preference>
  xml_begin_tag(thisAgent, kTagPreference);
  xml_att_val(thisAgent, kWME_Id, pref->id);
  xml_att_val(thisAgent, kWME_Attribute, pref->attr);
  xml_att_val(thisAgent, kWME_Value, pref->value);

  char buf[2];
  buf[0] = pref_type;
  buf[1] = 0;
  xml_att_val(thisAgent, kPreference_Type, buf);
  
  if (preference_is_binary(pref->type)) {
	  xml_att_val(thisAgent, kReferent, pref->referent);
  }
  if (pref->o_supported) {
	  xml_att_val(thisAgent, kOSupported, ":O");
  }
  xml_end_tag(thisAgent, kTagPreference);
			
}
//#ifdef USE_TCL

/* kjh(CUSP-B2) begin */

extern "C" Bool passes_wme_filtering(agent* thisAgent, wme *w, Bool isAdd);
void
filtered_print_wme_add(agent* thisAgent, wme *w) {
  if (passes_wme_filtering(thisAgent, w,TRUE)) 
  {
    print (thisAgent, "=>WM: ");
	xml_begin_tag(thisAgent, kTagWMEAdd);
    print_wme(thisAgent, w);
	xml_end_tag(thisAgent, kTagWMEAdd);
  }
}

void filtered_print_wme_remove(agent* thisAgent, wme *w) 
{
  if (passes_wme_filtering(thisAgent, w,FALSE)) 
  {
    print (thisAgent, "<=WM: ");
	xml_begin_tag(thisAgent, kTagWMERemove);
    print_wme(thisAgent, w);  /*  print_wme takes care of tagged output itself */
	xml_end_tag(thisAgent, kTagWMERemove);
  }
}
//#endif /* USE_TCL */

/* kjh(CUSP-B2) end */

void print_wme (agent* thisAgent, wme *w) {
  print (thisAgent, "(%lu: ", w->timetag);
  print_with_symbols (thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
  
  if (wma_enabled(thisAgent))
  {
    print (thisAgent, " [%0.2g]", wma_get_wme_activation(thisAgent, w));
  }
  
  if (w->acceptable) print_string (thisAgent, " +");
  print_string (thisAgent, ")");
  print (thisAgent, "\n");

  // <wme tag="123" id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>
  xml_object( thisAgent, w );
}

void print_wme_without_timetag (agent* thisAgent, wme *w) {
  print_with_symbols (thisAgent, "(%y ^%y %y", w->id, w->attr, w->value);
  if (w->acceptable) print_string (thisAgent, " +");
  print_string (thisAgent, ")");
  print (thisAgent, "\n");

  // <wme id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>
  xml_object( thisAgent, w, XML_WME_NO_TIMETAG );
}

//#ifdef USE_TCL
void print_wme_for_tcl (agent* thisAgent, wme *w) 
{
  print (thisAgent, "%lu: ", w->timetag);
  print_with_symbols (thisAgent, "%y ^%y %y", w->id, w->attr, w->value);
  if (w->acceptable) print_string (thisAgent, " +");
}
//#endif /* USE_TCL */

void print_instantiation_with_wmes (agent* thisAgent, instantiation *inst, 
									wme_trace_type wtt, int action) 
{
  int PRINTING = -1;
  int FIRING = 0;
  int RETRACTING = 1;
  condition *cond;


  if (action == PRINTING) {
	  xml_begin_tag(thisAgent, kTagProduction);
  } else if (action == FIRING) {
	  xml_begin_tag(thisAgent, kTagProduction_Firing);
	  xml_begin_tag(thisAgent, kTagProduction);
  } else if (action == RETRACTING) {
	  xml_begin_tag(thisAgent, kTagProduction_Retracting);
	  xml_begin_tag(thisAgent, kTagProduction);
  }

  if (inst->prod) {
      print_with_symbols  (thisAgent, "%y", inst->prod->name);
      xml_att_val(thisAgent, kProduction_Name, symbol_to_string (thisAgent, inst->prod->name, true, 0, 0));
  } else {
      print (thisAgent, "[dummy production]");
	  xml_att_val(thisAgent, kProduction_Name, "[dummy_production]");

  }

  print (thisAgent, "\n");

  if (wtt==NONE_WME_TRACE) {
	  if (action == PRINTING) {
		  xml_end_tag(thisAgent, kTagProduction);
	  } else if (action == FIRING) {
		  xml_end_tag(thisAgent, kTagProduction);
		  xml_end_tag(thisAgent, kTagProduction_Firing);
	  } else if (action == RETRACTING) {
		  xml_end_tag(thisAgent, kTagProduction);
		  xml_end_tag(thisAgent, kTagProduction_Retracting);
	  }
	  return;
  }

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION) {
      switch (wtt) {
      case TIMETAG_WME_TRACE:
        print (thisAgent, " %lu", cond->bt.wme_->timetag);

		xml_begin_tag(thisAgent, kTagWME);
		xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
		xml_end_tag(thisAgent, kTagWME);

        break;
      case FULL_WME_TRACE:	
		// Not all conds and wme_'s available when retracting, depending on DO_TOP_LEVEL_REF_CTS
		#ifdef DO_TOP_LEVEL_REF_CTS
		print (thisAgent, " ");
		print_wme (thisAgent, cond->bt.wme_);
		#else
		if (action != RETRACTING && cond->bt.level > TOP_GOAL_LEVEL) {
			print (thisAgent, " ");
			print_wme (thisAgent, cond->bt.wme_);
		} else {
			// Wmes that matched the LHS of a retraction may already be free'd; just print tt.
			print (thisAgent, " %lu", cond->bt.wme_->timetag);

			xml_begin_tag(thisAgent, kTagWME);
			xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
			xml_end_tag(thisAgent, kTagWME);
		}
		#endif		  
        break;
      }
    }
	
	if (action == PRINTING) {
		xml_end_tag(thisAgent, kTagProduction);
	} else if (action == FIRING) {
		xml_end_tag(thisAgent, kTagProduction);
		xml_end_tag(thisAgent, kTagProduction_Firing);
	} else if (action == RETRACTING) {
		xml_end_tag(thisAgent, kTagProduction);
		xml_end_tag(thisAgent, kTagProduction_Retracting);
	}
}

/***************************************************************************
* Function     : print_list_of_conditions
**************************************************************************/

void print_list_of_conditions(agent* thisAgent, condition *cond) {

  while (cond != NULL) {
    if (get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE-20)
      print (thisAgent, "\n      ");
    print_condition (thisAgent, cond);
    print (thisAgent, "\n");

    cond = cond->next;
  }
}

void print_phase (agent* thisAgent, const char * s, bool end_of_phase)
{
  // should be more consistent with creating string, but for now, for
  // consistency with previous versions, we'll let calling code set string.
  print (thisAgent, s);

  // the rest is all for tagged output events

  xml_begin_tag(thisAgent, kTagPhase);

  if (end_of_phase) {
	xml_att_val(thisAgent, kPhase_Status, kPhaseStatus_End);
  }

  switch (thisAgent->current_phase) {
  case INPUT_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Input);
	break;
  case PREFERENCE_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Pref);
    break;
  case WM_PHASE:
	  xml_att_val(thisAgent, kPhase_Name, kPhaseName_WM);
	  switch (thisAgent->FIRING_TYPE) {
	  case PE_PRODS:  /* no longer needed;  Soar8 has PROPOSE/APPLY */
		  xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_PE);
          break;
	  case IE_PRODS:
		  xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_IE);
          break;
	  }
	  break;
  case DECISION_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Decision);
    break;
  case OUTPUT_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Output);
    break;
  case PROPOSE_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Propose);
    break;
  case APPLY_PHASE:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Apply);
    break;
  default:
	xml_att_val(thisAgent, kPhase_Name, kPhaseName_Unknown);
    break;
  } // end switch

  xml_end_tag(thisAgent, kTagPhase);
  return;
}

/*
===========================

===========================
*/
Bool wme_filter_component_match(Symbol * /*filterComponent*/, Symbol * /*wmeComponent*/) {
//  if ((filterComponent->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
//      (!strcmp(filterComponent->sc.name,"*"))) 
//    return TRUE;
//  else
//    return(filterComponent == wmeComponent);
   return TRUE;
}

/*
===========================

===========================
*/
Bool passes_wme_filtering(agent* /*thisAgent*/, wme * /*w*/, Bool /*isAdd*/) {
//  cons *c;
//  wme_filter *wf;
//
//  /*  print ("testing wme for filtering: ");  print_wme(w); */
//  
//  if (!thisAgent->wme_filter_list)
//    return TRUE; /* no filters defined -> everything passes */
//  for (c=thisAgent->wme_filter_list; c!=NIL; c=c->rest) {
//    wf = (wme_filter *) c->first;
//    /*     print_with_symbols(thisAgent, "  trying filter: %y ^%y %y\n",wf->id,wf->attr,wf->value); */
//    if (   ((isAdd && wf->adds) || ((!isAdd) && wf->removes))
//        && wme_filter_component_match(wf->id,w->id)
//        && wme_filter_component_match(wf->attr,w->attr)
//        && wme_filter_component_match(wf->value,w->value))
//      return TRUE;
//  }
  return TRUE; /* no defined filters match -> w passes */
}

