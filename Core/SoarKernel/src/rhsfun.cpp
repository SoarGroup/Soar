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
 *  file:  rhsfun.cpp
 *
 * =======================================================================
 *                   RHS Function Management for Soar 6
 *
 * The system maintains a list of available RHS functions.  Functions
 * can appear on the RHS of productions either as values (in make actions
 * or as arguments to other function calls) or as stand-alone actions
 * (e.g., "write" and "halt").  When a function is executed, its C code
 * is called with one parameter--a (consed) list of the arguments (symbols).
 * The C function should return either a symbol (if all goes well) or NIL
 * (if an error occurred, or if the function is a stand-alone action).
 *
 * All available RHS functions should be setup at system startup time via
 * calls to add_rhs_function().  It takes as arguments the name of the
 * function (a symbol), a pointer to the corresponding C function, the
 * number of arguments the function expects (-1 if the function can take
 * any number of arguments), and flags indicating whether the function can
 * be a RHS value or a stand-alone action.
 *
 * Lookup_rhs_function() takes a symbol and returns the corresponding
 * rhs_function structure (or NIL if there is no such function).
 *
 * Init_built_in_rhs_functions() should be called at system startup time
 * to setup all the built-in functions.
 * =======================================================================
 */

#include "rhsfun.h"
#include "kernel.h"
#include "print.h"
#include "mem.h"
#include "symtab.h"
#include "init_soar.h"
#include "gsysparam.h"
#include "agent.h"
#include "production.h"
#include "rhsfun_math.h"
#include "io.h"
#include "kernel_struct.h"
#include "recmem.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "gski_event_system_functions.h" // support for triggering XML events

#include <map>

using namespace xmlTraceNames;

void add_rhs_function (agent* thisAgent, 
                       Symbol *name,
                       rhs_function_routine f,
                       int num_args_expected,
                       Bool can_be_rhs_value,
                       Bool can_be_stand_alone_action,
                       void* user_data) {
  rhs_function *rf;

  if ((!can_be_rhs_value) && (!can_be_stand_alone_action)) 
  {
    print (thisAgent, "Internal error: attempt to add_rhs_function that can't appear anywhere\n");
    return;
  }

  for (rf=thisAgent->rhs_functions; rf!=NIL; rf=rf->next)
  {
     if (rf->name==name) 
     {
        print_with_symbols (thisAgent, "Internal error: attempt to add_rhs_function that already exists: %y\n", name);
        return;
     }
  }
  
  rf = static_cast<rhs_function_struct *>(allocate_memory (thisAgent, sizeof(rhs_function), MISCELLANEOUS_MEM_USAGE));

  /* Insertion into the list */
  rf->next = thisAgent->rhs_functions;
  thisAgent->rhs_functions = rf;
  
  /* Rest of the stuff */
  rf->name = name;
  rf->f = f;
  rf->num_args_expected = num_args_expected;
  rf->can_be_rhs_value = can_be_rhs_value;
  rf->can_be_stand_alone_action = can_be_stand_alone_action;
  rf->user_data = user_data;
}

rhs_function *lookup_rhs_function (agent* thisAgent, Symbol *name) 
{
  rhs_function *rf;

  for (rf=thisAgent->rhs_functions; rf!=NIL; rf=rf->next)
  {
     if (rf->name==name) 
        return rf;
  }
  return NIL;
}

void remove_rhs_function(agent* thisAgent, Symbol *name) {  /* code from Koss 8/00 */

  rhs_function *rf = NIL, *prev;

  /* Find registered function by name */
  for (prev = NIL, rf = thisAgent->rhs_functions;
       rf != NIL;
       prev = rf, rf = rf->next)
  {
    if (rf->name == name) 
       break;
  }

  /* If not found, there's a problem */
  if (rf == NIL) {
    fprintf(stderr, "Internal error: attempt to remove_rhs_function that does not exist.\n");
    print_with_symbols(thisAgent, "Internal error: attempt to remove_rhs_function that does not exist: %y\n", name);
  }

  /* Else, remove it */
  else 
  {

    /* Head of list special */
    if (prev == NIL)
      thisAgent->rhs_functions = rf->next;
    else
      prev->next = rf->next;

    free_memory(thisAgent, rf, MISCELLANEOUS_MEM_USAGE);
  }

   // DJP-FREE: The name reference needs to be released now the function is gone
   symbol_remove_ref(thisAgent,name);
}


/* ====================================================================

               Code for Executing Built-In RHS Functions

====================================================================  */

/* MVP 6-8-94 */
/* --------------------------------------------------------------------
			       User-Select
-------------------------------------------------------------------- */
Symbol *user_select_rhsfun (agent* thisAgent, list *args, void* user_data) {
  Symbol *uselect;

  uselect = static_cast<symbol_union *>(args->first);

  if (!strcmp(uselect->sc.name,"first")) {
    set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
    return NIL;
  }
/* AGR 615 begin */
  if (!strcmp(uselect->sc.name,"last")) {
    set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
    return NIL;
  }
/* AGR 615 end */
  if ((!strcmp(uselect->sc.name,"ask")) ||
      (!strcmp(uselect->sc.name,"t"))) {
    set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_ASK);
    return NIL;
  }
  if ((!strcmp(uselect->sc.name,"random")) ||
      (!strcmp(uselect->sc.name,"nil"))) {
    set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
    return NIL;
  }
  print (thisAgent, "Expected first, ask, or random for new value of user-select.\n");

  return NIL;
}

/* --------------------------------------------------------------------
                                Write

   Takes any number of arguments, and prints each one.
-------------------------------------------------------------------- */

Symbol *write_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg;
  char *string;
  growable_string gs = make_blank_growable_string(thisAgent); // for XML generation

  for ( ; args!=NIL; args=args->rest) {
    arg = static_cast<symbol_union *>(args->first);
    /* --- Note use of FALSE here--print the symbol itself, not a rereadable
       version of it --- */
    string = symbol_to_string (thisAgent, arg, FALSE, NIL, 0);
    add_to_growable_string(thisAgent, &gs, string); // for XML generation
    print_string (thisAgent, string);
  }

  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagRHS_write);
  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kRHS_String, text_of_growable_string(gs));
  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagRHS_write);

  free_growable_string(thisAgent, gs);
  
  return NIL;
}

/* --------------------------------------------------------------------
                                Crlf

   Just returns a sym_constant whose print name is a line feed.
-------------------------------------------------------------------- */

Symbol *crlf_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  return make_sym_constant (thisAgent, "\n");
}

/* --------------------------------------------------------------------
                                Halt

   Just sets a flag indicating that the system has halted.
-------------------------------------------------------------------- */

Symbol *halt_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  thisAgent->system_halted = TRUE;
  return NIL;
}

/* --------------------------------------------------------------------
                              Interrupt

   This causes an interrupt at the end of the current preference phase.
   It sets stop_soar to TRUE, and reason_for_stopping to an appropriate
   string.
-------------------------------------------------------------------- */

// MVP 6-27-94 */
//
char * RHS_interrupt_msg = "*** RHS Function Interrupt ***";
//
/*
  Replacing this with a new style rhs function through gSKI.

Symbol *interrupt_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
char *ch;

cons * c;
agent * the_agent;

for(c = thisAgent->kernel->all_soar_agents; c != NIL; c = c->rest) {
the_agent = ((agent *) c->first);
the_agent->stop_soar = TRUE;
the_agent->reason_for_stopping =  RHS_interrupt_msg;
}

strcpy (thisAgent->interrupt_source, "*** Interrupt from production ");
ch = thisAgent->interrupt_source;
while (*ch) ch++;
symbol_to_string (thisAgent, thisAgent->production_being_fired->name, TRUE, ch); 
while (*ch) ch++;
strcpy (ch, " ***");
thisAgent->reason_for_stopping = thisAgent->interrupt_source;
return NIL;
}
*/

/* --------------------------------------------------------------------
                         Make-constant-symbol

   Returns a newly generated sym_constant.  If no arguments are given,
   the constant will start with "constant".  If one or more arguments
   are given, the constant will start with a string equal to the
   concatenation of those arguments.
-------------------------------------------------------------------- */

Symbol *make_constant_symbol_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
#define MAKE_CONSTANT_SYMBOL_RHS_BUFFER_SIZE 1000 /* that ought to be long enough */ /* sigh -voigtjr */
  char buf[MAKE_CONSTANT_SYMBOL_RHS_BUFFER_SIZE];
  char *string;
  cons *c;

  if (!args) {
    strncpy (buf, "constant", MAKE_CONSTANT_SYMBOL_RHS_BUFFER_SIZE);
  } else {
    buf[0] = 0;
    for (c=args; c!=NIL; c=c->rest) {
      string = symbol_to_string (thisAgent, static_cast<symbol_union *>(c->first), FALSE, NIL, 0);
      strncat (buf, string, MAKE_CONSTANT_SYMBOL_RHS_BUFFER_SIZE);
	  buf[MAKE_CONSTANT_SYMBOL_RHS_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }
  }
  if ((!args) && (!find_sym_constant (thisAgent, buf))) return make_sym_constant (thisAgent, buf);
  return generate_new_sym_constant (thisAgent, buf, &thisAgent->mcs_counter);
}


/* --------------------------------------------------------------------
                               Timestamp

   Returns a newly generated sym_constant whose name is a representation
   of the current local time.
-------------------------------------------------------------------- */

Symbol *timestamp_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  time_t now;
  struct tm *temp;
#define TIMESTAMP_BUFFER_SIZE 100
  char buf[TIMESTAMP_BUFFER_SIZE];

  now = time(NULL);
#ifdef THINK_C
  temp = localtime ((const time_t *)&now);
#else
#ifdef __SC__
  temp = localtime ((const time_t *)&now);
#else
#ifdef __ultrix
  temp = localtime ((const time_t *)&now);
#else
#ifdef MACINTOSH
  temp = localtime ((const time_t *) &now);
#else
  temp = localtime (&now);
#endif
#endif
#endif
#endif
  snprintf (buf,TIMESTAMP_BUFFER_SIZE, "%d/%d/%d-%02d:%02d:%02d",
           temp->tm_mon + 1, temp->tm_mday, temp->tm_year,
           temp->tm_hour, temp->tm_min, temp->tm_sec);
  buf[TIMESTAMP_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  return make_sym_constant (thisAgent, buf);
}

/* --------------------------------------------------------------------
                              Accept

   Waits for the user to type a line of input; then returns the first
   symbol from that line.
-------------------------------------------------------------------- */

Symbol *accept_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  char buf[2000], *s;
  Symbol *sym;

  while (TRUE) {
    s = fgets (buf, 2000, stdin);
    //    s = Soar_Read(thisAgent, buf, 2000); /* kjh(CUSP-B10) */
    if (!s) {
      /* s==NIL means immediate eof encountered or read error occurred */
      return NIL;
    }
    s = buf;
    sym = get_next_io_symbol_from_text_input_line (thisAgent, &s);
    if (sym) break;
  }
  symbol_add_ref (sym);
  release_io_symbol (thisAgent, sym); /* because it was obtained using get_io_... */
  return sym;
}


/* ---------------------------------------------------------------------
  Capitalize a Symbol
------------------------------------------------------------------------ */

Symbol * 
capitalize_symbol_rhs_function_code (agent* thisAgent, list *args, void* user_data) 
{
  char * symbol_to_capitalize;
  Symbol * sym;

  if (!args) {
    print (thisAgent, "Error: 'capitalize-symbol' function called with no arguments.\n");
    return NIL;
  }

  sym = (Symbol *) args->first;
  if (sym->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-symbol (%y) passed to capitalize-symbol function.\n", sym);
    return NIL;
  }

  if (args->rest) {
    print (thisAgent, "Error: 'capitalize-symbol' takes exactly 1 argument.\n");
    return NIL;
  }

  symbol_to_capitalize = symbol_to_string(thisAgent, sym, FALSE, NIL, 0);
  symbol_to_capitalize = savestring(symbol_to_capitalize);
  *symbol_to_capitalize = toupper(*symbol_to_capitalize);
  return make_sym_constant(thisAgent, symbol_to_capitalize);
}

/* AGR 520 begin     6-May-94 */
/* ------------------------------------------------------------
   2 general purpose rhs functions by Gary.
------------------------------------------------------------

They are invoked in the following manner, and I use them
to produce nice traces.

   (ifeq <a> <b> abc def)
and 
   (strlen <a>)

ifeq -- checks if the first argument is "eq" to the second argument
        if it is then it returns the third argument else the fourth.
        It is useful in similar situations to the "?" notation in C.
        Contrary to earlier belief, all 4 arguments are required.

        examples:
           (sp trace-juggling
            (goal <g> ^state.ball.position <pos>)
           -->
            (write (ifeq <pos> at-top        |    o    | ||) (crlf)
                   (ifeq <pos> left-middle   | o       | ||)
                   (ifeq <pos> right-middle  |       o | ||) (crlf)
                   (ifeq <pos> left-hand     |o        | ||)
                   (ifeq <pos> right-hand    |        o| ||) (crlf)
                                             |V_______V|    (crlf))
            )

            This outputs with a single production one of the following
            pictures depending on the ball's position (providing the ball
            is not dropped of course. Then it outputs empty hands. :-)

                                         o
                         o                              o
           o                                                           o
           V-------V    V-------V    V-------V    V-------V    V-------V
                     or           or           or           or


           for a ball that takes this path.

               o
            o     o
           o       o
           V-------V

           Basically this is useful when you don't want the trace to
           match the internal working memory structure.

strlen <val> - returns the string length of the output string so that
               one can get the output to line up nicely. This is useful
               along with ifeq when the output string varies in length.

           example:

              (strlen |abc|) returns 3

              (write (ifeq (strlen <foo>) 3 | | ||)
                     (ifeq (strlen <foo>) 2 |  | ||)
                     (ifeq (strlen <foo>) 1 |   | ||) <foo>)

                  writes foo padding on the left with enough blanks so that
                  the length of the output is always at least 4 characters.

------------------------------------------------------------ */

Symbol *ifeq_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg1, *arg2;
  cons *c;

  if (!args) {
    print (thisAgent, "Error: 'ifeq' function called with no arguments\n");
    return NIL;
  }

  /* --- two or more arguments --- */
  arg1 = static_cast<symbol_union *>(args->first);
  c=args->rest;
  arg2 = static_cast<symbol_union *>(c->first);
  c=c->rest;

  if (arg1 == arg2)
    {
      symbol_add_ref((Symbol *)(c->first));
      return static_cast<symbol_union *>(c->first);
    }
  else if (c->rest)
    {
      symbol_add_ref((Symbol *)(c->rest->first));
      return static_cast<symbol_union *>(c->rest->first);
    }
  else return NIL;
}


Symbol *strlen_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg;
  char *string;

  arg = static_cast<symbol_union *>(args->first);

  /* --- Note use of FALSE here--print the symbol itself, not a rereadable
     version of it --- */
  string = symbol_to_string (thisAgent, arg, FALSE, NIL, 0);

  return make_int_constant (thisAgent, strlen(string));
}
/* AGR 520     end */

/* --------------------------------------------------------------------
                              dont_learn

Hack for learning.  Allow user to denote states in which learning
shouldn't occur when "learning" is set to "except".
-------------------------------------------------------------------- */

Symbol *dont_learn_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *state;

  if (!args) {
    print (thisAgent, "Error: 'dont-learn' function called with no arg.\n");
    return NIL;
  }

  state = (Symbol *) args->first;
  if (state->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-identifier (%y) passed to dont-learn function.\n", state);
    return NIL;
  } else if (! state->id.isa_goal) {
      print_with_symbols(thisAgent, "Error: identifier passed to dont-learn is not a state: %y.\n",state);
  }
  
  if (args->rest) {
    print (thisAgent, "Error: 'dont-learn' takes exactly 1 argument.\n");
    return NIL;
  }

  if (! member_of_list (state, thisAgent->chunk_free_problem_spaces)) {
    push(thisAgent, state, thisAgent->chunk_free_problem_spaces);
    /* print_with_symbols("State  %y  added to chunk_free_list.\n",state); */
  } 
  return NIL;

}

/* --------------------------------------------------------------------
                              force_learn

Hack for learning.  Allow user to denote states in which learning
should occur when "learning" is set to "only".
-------------------------------------------------------------------- */

Symbol *force_learn_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *state;

  if (!args) {
    print (thisAgent, "Error: 'force-learn' function called with no arg.\n");
    return NIL;
  }

  state = (Symbol *) args->first;
  if (state->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-identifier (%y) passed to force-learn function.\n", state);
    return NIL;
  } else if (! state->id.isa_goal) {
      print_with_symbols(thisAgent, "Error: identifier passed to force-learn is not a state: %y.\n",state);
  }

  
  if (args->rest) {
    print (thisAgent, "Error: 'force-learn' takes exactly 1 argument.\n");
    return NIL;
  }

  if (! member_of_list (state, thisAgent->chunky_problem_spaces)) {
    push(thisAgent, state, thisAgent->chunky_problem_spaces);
    /* print_with_symbols("State  %y  added to chunky_list.\n",state); */
  } 
  return NIL;

}

/* ====================================================================
                  RHS Deep copy recursive helper functions
====================================================================  */
void recursive_deep_copy_helper(agent* thisAgent,
                                Symbol* id_to_process,
                                Symbol* parent_id,
                                std::map<Symbol*,Symbol*>& processedSymbols);

void recursive_wme_copy(agent* thisAgent,
                        Symbol* parent_id,
                        wme* curwme,
                        std::map<Symbol*,Symbol*>& processedSymbols) {
   
   bool made_new_attr_symbol = false;
   bool made_new_value_symbol = false;

   Symbol* new_id = parent_id;
   Symbol* new_attr = curwme->attr;
   Symbol* new_value = curwme->value;
   
   /* Handling the case where the attribute is an id symbol */
   if ( curwme->attr->common.symbol_type == 1 ) {
      /* Have I already made a new identifier for this identifier */
      std::map<Symbol*,Symbol*>::iterator it = processedSymbols.find(curwme->attr);
      if ( it != processedSymbols.end() ) {
         /* Retrieve the previously created id symbol */
         new_attr = it->second;
      } else {
         /* Make a new id symbol */
         new_attr = make_new_identifier(thisAgent,
                                        curwme->attr->id.name_letter,
                                        1);
         made_new_attr_symbol = true;
      }

      recursive_deep_copy_helper(thisAgent,
                                 curwme->attr,
                                 new_attr,
                                 processedSymbols);
   }

   /* Handling the case where the value is an id symbol */
   if ( curwme->value->common.symbol_type == 1 ) {
      /* Have I already made a new identifier for this identifier */
      std::map<Symbol*,Symbol*>::iterator it = processedSymbols.find(curwme->value);
      if ( it != processedSymbols.end() ) {
         /* Retrieve the previously created id symbol */
         new_value = it->second;
      } else {
         /* Make a new id symbol */
         new_value = make_new_identifier(thisAgent,
                                         curwme->value->id.name_letter,
                                         1);
         made_new_value_symbol = true;
      }

      recursive_deep_copy_helper(thisAgent,
                                 curwme->value,
                                 new_value,
                                 processedSymbols);
   }

   /* Making the new wme (Note just reusing the wme data structure, these
      wme's actually get converted into preferences later).*/
   wme* oldGlobalWme = glbDeepCopyWMEs;

   /* TODO: We need a serious reference counting audit of the kernel But I think
      this mirrors what happens in the instantiate rhs value and execute action
      functions. */
   symbol_add_ref(new_id);
   if ( !made_new_attr_symbol ) symbol_add_ref(new_attr);
   if ( !made_new_value_symbol) symbol_add_ref(new_value);

   glbDeepCopyWMEs = make_wme(thisAgent, new_id, new_attr, new_value, true);
   glbDeepCopyWMEs->next = oldGlobalWme;
                        
}

void recursive_deep_copy_helper(agent* thisAgent,
                                Symbol* id_to_process,
                                Symbol* parent_id,
                                std::map<Symbol*,Symbol*>& processedSymbols)
{
   /* If this symbol has already been processed then ignore it and return */
   if ( processedSymbols.find(id_to_process) != processedSymbols.end() ) {
      return;
   }
   processedSymbols.insert(std::pair<Symbol*,Symbol*>(id_to_process,parent_id));

   /* Iterating over the normal slot wmes */
   for (slot* curslot = id_to_process->id.slots;
        curslot != 0;
        curslot = curslot->next) {
      
      /* Iterating over the wmes in this slot */
      for (wme* curwme = curslot->wmes;
           curwme != 0;
           curwme = curwme->next) {
         
         recursive_wme_copy(thisAgent,
                            parent_id,
                            curwme,
                            processedSymbols);
         
      }
      
   }
   
   /* Iterating over input wmes */
   for (wme* curwme = id_to_process->id.input_wmes;
        curwme != 0;
        curwme = curwme->next) {
      
      recursive_wme_copy(thisAgent,
                         parent_id,
                         curwme,
                         processedSymbols);

   }

}
/* ====================================================================
                  RHS Deep copy function
====================================================================  */
Symbol* deep_copy_rhs_function_code(agent* thisAgent, list *args, void* user_data) {

   /* Getting the argument symbol */
   Symbol* baseid = (Symbol*) args->first;
   if ( baseid->common.symbol_type != 1 ) {
      return make_sym_constant(thisAgent,"*symbol not id*");
   }

   /* Making the new root identifier symbol */
   Symbol* retval = make_new_identifier(thisAgent, 'D', 1);

   /* Now processing the wme's associated with the passed in symbol */
   std::map<Symbol*,Symbol*> processedSymbols;
   recursive_deep_copy_helper(thisAgent,
                              baseid,
                              retval,
                              processedSymbols);
                              

   return retval;;
}

/* ====================================================================

                  Initialize the Built-In RHS Functions

====================================================================  */

void init_built_in_rhs_functions (agent* thisAgent) {
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "write"), write_rhs_function_code,
                    -1, FALSE, TRUE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "crlf"), crlf_rhs_function_code,
                    0, TRUE, FALSE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "halt"), halt_rhs_function_code,
                    0, FALSE, TRUE, 0);
  /*
    Replaced with a gSKI rhs function
    add_rhs_function (thisAgent, make_sym_constant (thisAgent, "interrupt"),
    interrupt_rhs_function_code,
    0, FALSE, TRUE, 0);
  */
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "make-constant-symbol"),
                    make_constant_symbol_rhs_function_code,
                    -1, TRUE, FALSE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "timestamp"),
                    timestamp_rhs_function_code,
                    0, TRUE, FALSE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "accept"), accept_rhs_function_code,
                    0, TRUE, FALSE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "capitalize-symbol"),
		    capitalize_symbol_rhs_function_code,
		    1,
		    TRUE,
		    FALSE,
          0);
/* AGR 520  begin */
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "ifeq"), ifeq_rhs_function_code,
		    4, TRUE, FALSE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "strlen"), strlen_rhs_function_code,
		    1, TRUE, FALSE, 0);
/* AGR 520  end   */

  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "dont-learn"),
		    dont_learn_rhs_function_code,
                    1, FALSE, TRUE, 0);
  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "force-learn"),
		    force_learn_rhs_function_code,
                    1, FALSE, TRUE, 0);

  add_rhs_function (thisAgent, make_sym_constant (thisAgent, "deep-copy"),
                    deep_copy_rhs_function_code,
                    1,TRUE,FALSE,0);

  init_built_in_rhs_math_functions(thisAgent);
}

void remove_built_in_rhs_functions (agent* thisAgent) {

  // DJP-FREE: These used to call make_sym_constant, but the symbols must already exist and if we call make here again we leak a reference.
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "write"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "crlf"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "halt"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "make-constant-symbol"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "timestamp"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "accept"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "capitalize-symbol"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "ifeq"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "strlen"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "dont-learn"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "force-learn"));
  remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "deep-copy"));

  remove_built_in_rhs_math_functions(thisAgent);
}
