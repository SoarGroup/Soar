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
 *  file:  symtab.cpp
 *
 * =======================================================================
 *  
 *                 Symbol Table Routines for Soar 6
 *
 * Soar 6 uses five kinds of symbols:  symbolic constants, integer
 * constants, floating-point constants, identifiers, and variables.
 * We use five resizable hash tables, one for each kind of symbol.
 *
 *   "symbol" is typedef-ed as a union of the five kinds of symbol
 *  structures.  Some fields common to all symbols are accessed via
 *  sym->common.field_name; fields particular to a certain kind of
 *  symbol are accessed via sym->var.field_name_on_variables, etc.
 *  See soarkernel.h for the Symbol structure definitions.
 *
 * =======================================================================
 */

/* Uncomment the following line to get symbol debugging printouts */
//#define DEBUG_SYMBOLS

#include "symtab.h"
#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "production.h"
#include "init_soar.h"
#include "print.h"
#include "gski_event_system_functions.h" // for XML trace output

/* -------------------------------------------------------------------
                           Hash Functions

   Compress() takes a 32-bit hash value and compresses it down to a few
   bits, xor-ing pieces of the 32-bit value to avoid throwing away bits
   that might be important for the hash function to be effective.
   
   Hash_string() produces a hash value for a string of characters.

   Hash_xxx_raw_info() are the hash functions for the five kinds of
   symbols.  These functions operate on the basic info about the symbol
   (i.e., the name, value, etc.).  Hash_xxx(), on the other hand,
   operate on the symbol table entries for the five kinds of symbols--
   these routines are the callback hashing functions used by the
   resizable hash table routines.
------------------------------------------------------------------- */

unsigned long compress (unsigned long h, short num_bits) {
  unsigned long result;

  if (num_bits < 16) h = (h & 0xFFFF) ^ (h >> 16);
  if (num_bits < 8) h = (h & 0xFF) ^ (h >> 8);
  result = 0;
  while (h) {
    result ^= (h & masks_for_n_low_order_bits[num_bits]);
    h = h >> num_bits;
  }
  return result;
}

unsigned long hash_string (const char *s) {   /* AGR 600 */
  unsigned long h;

  h = 0;
  while (*s != 0) {
    h = ((h << 8) | (h >> 24)) ^ (*s);
    s++; 
  }
  return h;
}

/* -----------------------------------------
   Hashing symbols using their basic info 
----------------------------------------- */

unsigned long hash_variable_raw_info (char *name, short num_bits) {
  return compress (hash_string(name), num_bits);
}

unsigned long hash_identifier_raw_info (char name_letter,
                                        unsigned long name_number,
                                        short num_bits) {
  return compress (name_number | (name_letter << 24), num_bits);
}

unsigned long hash_sym_constant_raw_info (const char *name, short num_bits) {
  return compress (hash_string(name), num_bits);
}

unsigned long hash_int_constant_raw_info (long value, short num_bits) {
  return compress ((unsigned long)value, num_bits);
}

unsigned long hash_float_constant_raw_info (float value, short num_bits) {
  return compress ((unsigned long)value, num_bits);
}

/* ---------------------------------------------------
   Hashing symbols using their symbol table entries
--------------------------------------------------- */

unsigned long hash_variable (void *item, short num_bits) {
  variable *var;
  var = static_cast<variable_struct *>(item);
  return compress (hash_string(var->name),num_bits);
}

unsigned long hash_identifier (void *item, short num_bits) {
  identifier *id;
  id = static_cast<identifier_struct *>(item);
  return compress (id->name_number ^ (id->name_letter << 24), num_bits);
}

unsigned long hash_sym_constant (void *item, short num_bits) {
  sym_constant *sc;
  sc = static_cast<sym_constant_struct *>(item);
  return compress (hash_string(sc->name),num_bits);
}

unsigned long hash_int_constant (void *item, short num_bits) {
  int_constant *ic;
  ic = static_cast<int_constant_struct *>(item);
  return compress ((unsigned long)(ic->value),num_bits);
}

unsigned long hash_float_constant (void *item, short num_bits) {
  float_constant *fc;
  fc = static_cast<float_constant_struct *>(item);
  return compress ((unsigned long)(fc->value),num_bits);
}

/* -------------------------------------------------------------------

        Basic Symbol Table Data Structures and Initialization

------------------------------------------------------------------- */

//#define get_next_symbol_hash_id() (thisAgent->current_symbol_hash_id += 137)
inline unsigned long get_next_symbol_hash_id(agent* thisAgent) 
{
  return (thisAgent->current_symbol_hash_id += 137);
}

void init_symbol_tables (agent* thisAgent) {
  int i;

  thisAgent->variable_hash_table = make_hash_table (thisAgent, 0, hash_variable);
  thisAgent->identifier_hash_table = make_hash_table (thisAgent, 0, hash_identifier);
  thisAgent->sym_constant_hash_table = make_hash_table (thisAgent, 0, hash_sym_constant);
  thisAgent->int_constant_hash_table = make_hash_table (thisAgent, 0, hash_int_constant);
  thisAgent->float_constant_hash_table = make_hash_table (thisAgent, 0, hash_float_constant);

  init_memory_pool (thisAgent, &thisAgent->variable_pool, sizeof(variable), "variable");
  init_memory_pool (thisAgent, &thisAgent->identifier_pool, sizeof(identifier), "identifier");
  init_memory_pool (thisAgent, &thisAgent->sym_constant_pool, sizeof(sym_constant), "sym constant");
  init_memory_pool (thisAgent, &thisAgent->int_constant_pool, sizeof(int_constant), "int constant");
  init_memory_pool (thisAgent, &thisAgent->float_constant_pool, sizeof(float_constant),
                    "float constant");

  for (i=0; i<26; i++) thisAgent->id_counter[i]=1;
}

/* -------------------------------------------------------------------
          Symbol Table Lookup and Symbol Creation Routines

   The find_xxx() routines look for an existing symbol and return it
   if found; if no such symbol exists, they return NIL.

   The make_xxx() routines look for an existing symbol; if the find one,
   they increment the reference count and return it.  If no such symbol
   exists, they create a new one, set the reference count to 1, and
   return it.

   Note that rather than a make_identifier() routine, we have a
   make_new_identifier() routine, which takes two arguments: the first
   letter for the new identifier, and its initial goal_stack_level.
   There is no way to force creation of an identifier with a particular
   name letter/number combination like J37.
------------------------------------------------------------------- */

Symbol *find_variable (agent* thisAgent, char *name) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_variable_raw_info (name,thisAgent->variable_hash_table->log2size);
  sym = (Symbol *) (*(thisAgent->variable_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (!strcmp(sym->var.name,name)) return sym;
  }
  return NIL;
}

Symbol *find_identifier (agent* thisAgent, char name_letter, unsigned long name_number) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_identifier_raw_info (name_letter,name_number,
                                         thisAgent->identifier_hash_table->log2size);
  sym = (Symbol *) (*(thisAgent->identifier_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if ((name_letter==sym->id.name_letter) &&
        (name_number==sym->id.name_number)) return sym;
  }
  return NIL;
}

Symbol *find_sym_constant (agent* thisAgent, const char *name) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_sym_constant_raw_info (name,
                                           thisAgent->sym_constant_hash_table->log2size);
  sym = (Symbol *) (*(thisAgent->sym_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (!strcmp(sym->sc.name,name)) return sym;
  }
  return NIL;
}

Symbol *find_int_constant (agent* thisAgent, long value) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_int_constant_raw_info (value,
                                           thisAgent->int_constant_hash_table->log2size);
  sym = (Symbol *) (*(thisAgent->int_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (value==sym->ic.value) return sym;
  }
  return NIL;
}

Symbol *find_float_constant (agent* thisAgent, float value) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_float_constant_raw_info (value,
                                        thisAgent->float_constant_hash_table->log2size);
  sym = (Symbol *) (*(thisAgent->float_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (value==sym->fc.value) return sym;
  }
  return NIL;
}

Symbol *make_variable (agent* thisAgent, char *name) {
  Symbol *sym;

  sym = find_variable(thisAgent, name);
  if (sym) {
    symbol_add_ref(sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->variable_pool, &sym);
    sym->common.symbol_type = VARIABLE_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id(thisAgent);
    sym->var.name = make_memory_block_for_string (thisAgent, name);
    sym->var.gensym_number = 0;
    sym->var.tc_num = 0;
    sym->var.rete_binding_locations = NIL;
    add_to_hash_table (thisAgent, thisAgent->variable_hash_table, sym);
  }
  return sym;
}

Symbol *make_new_identifier (agent* thisAgent, char name_letter, goal_stack_level level) {
  Symbol *sym;

  if (isalpha(name_letter)) {
    if (islower(name_letter)) name_letter = toupper(name_letter);
  } else {
    name_letter = 'I';
  }
  allocate_with_pool (thisAgent, &thisAgent->identifier_pool, &sym);
  sym->common.symbol_type = IDENTIFIER_SYMBOL_TYPE;
  sym->common.reference_count = 1;
  sym->common.hash_id = get_next_symbol_hash_id(thisAgent);
  sym->id.name_letter = name_letter;
  sym->id.name_number = thisAgent->id_counter[name_letter-'A']++;
  sym->id.level = level;
  sym->id.promotion_level = level;
  sym->id.slots = NIL;
  sym->id.isa_goal = FALSE;
  sym->id.isa_impasse = FALSE;
  sym->id.isa_operator = 0;
  sym->id.link_count = 0;
  sym->id.unknown_level = NIL;
  sym->id.could_be_a_link_from_below = FALSE;
  sym->id.impasse_wmes = NIL;
  sym->id.higher_goal = NIL;
/* REW: begin 09.15.96 */
  sym->id.gds = NIL;
/* REW: end   09.15.96 */
/* REW: begin 08.20.97 */
  sym->id.saved_firing_type = NO_SAVED_PRODS;
  sym->id.ms_o_assertions = NIL; 
  sym->id.ms_i_assertions = NIL; 
  sym->id.ms_retractions = NIL;  
/* REW: end   08.20.97 */
  sym->id.lower_goal = NIL;
  sym->id.operator_slot = NIL;
  sym->id.preferences_from_goal = NIL;
  sym->id.tc_num = 0;
  sym->id.associated_output_links = NIL;
  sym->id.input_wmes = NIL;
  add_to_hash_table (thisAgent, thisAgent->identifier_hash_table, sym);
  return sym;
}

Symbol *make_sym_constant (agent* thisAgent, char *name) {
  Symbol *sym;

  sym = find_sym_constant(thisAgent, name);
  if (sym) {
    symbol_add_ref(sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->sym_constant_pool, &sym);
    sym->common.symbol_type = SYM_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id(thisAgent);
    sym->sc.name = make_memory_block_for_string (thisAgent, name);
    sym->sc.production = NIL;
    add_to_hash_table (thisAgent, thisAgent->sym_constant_hash_table, sym);
  }
  return sym;
}

Symbol *make_int_constant (agent* thisAgent, long value) {
  Symbol *sym;

  sym = find_int_constant(thisAgent, value);
  if (sym) {
    symbol_add_ref(sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->int_constant_pool, &sym);
    sym->common.symbol_type = INT_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id(thisAgent);
    sym->ic.value = value;
    add_to_hash_table (thisAgent, thisAgent->int_constant_hash_table, sym);
  }
  return sym;
}

Symbol *make_float_constant (agent* thisAgent, float value) {
  Symbol *sym;

  sym = find_float_constant(thisAgent, value);
  if (sym) {
    symbol_add_ref(sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->float_constant_pool, &sym);
    sym->common.symbol_type = FLOAT_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id(thisAgent);
    sym->fc.value = value;
    add_to_hash_table (thisAgent, thisAgent->float_constant_hash_table, sym);
  }
  return sym;
}

/* -------------------------------------------------------------------

                         Deallocate Symbol

------------------------------------------------------------------- */

void deallocate_symbol (agent* thisAgent, Symbol *sym) {

#ifdef DEBUG_SYMBOLS  
  print_with_symbols (thisAgent, "\nDeallocating Symbol %y", sym);
#endif

  switch (sym->common.symbol_type) {
  case VARIABLE_SYMBOL_TYPE:
    remove_from_hash_table (thisAgent, thisAgent->variable_hash_table, sym);
    free_memory_block_for_string (thisAgent, sym->var.name);
    free_with_pool (&thisAgent->variable_pool, sym);
    break;
  case IDENTIFIER_SYMBOL_TYPE:
    remove_from_hash_table (thisAgent, thisAgent->identifier_hash_table, sym);
    free_with_pool (&thisAgent->identifier_pool, sym);
    break;
  case SYM_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (thisAgent, thisAgent->sym_constant_hash_table, sym);
    free_memory_block_for_string (thisAgent, sym->sc.name);
    free_with_pool (&thisAgent->sym_constant_pool, sym);
    break;
  case INT_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (thisAgent, thisAgent->int_constant_hash_table, sym);
    free_with_pool (&thisAgent->int_constant_pool, sym);
    break;
  case FLOAT_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (thisAgent, thisAgent->float_constant_hash_table, sym);
    free_with_pool (&thisAgent->float_constant_pool, sym);
    break;
  default:
    { char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "Internal error: called deallocate_symbol on non-symbol.\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    }
  }
}

/* -------------------------------------------------------------------
                       Other Symbol Utilities

   Reset_id_counters() is called during an init-soar to reset the id
   gensym numbers to 1.  It first makes sure there are no existing
   identifiers in the system--otherwise we might generate a second
   identifier with the same name later on.

   Reset_id_and_variable_tc_numbers() resets the tc_num field of every
   existing id and variable to 0.

   Reset_variable_gensym_numbers() resets the gensym_number field of
   every existing variable to 0.

   Print_internal_symbols() just prints a list of all existing symbols.
   (This is useful for debugging memory leaks.)

   Generate_new_sym_constant() is used to gensym new symbols that are
   guaranteed to not already exist.  It takes two arguments: "prefix"
   (the desired prefix of the new symbol's name), and "counter" (a
   pointer to a counter (unsigned long) that is incremented to produce
   new gensym names).
------------------------------------------------------------------- */

Bool print_identifier_ref_info(agent* thisAgent, void* item, FILE* f) {
   Symbol* sym;
   char msg[256];
   sym = static_cast<symbol_union *>(item);
   
   if ( sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) {
      if ( sym->common.reference_count > 0 ) {
         snprintf( msg, 256, 
                  "\t%c%lu --> %lu\n", 
                  sym->id.name_letter, 
                  sym->id.name_number, 
                  sym->common.reference_count);
		 msg[255] = 0; /* ensure null termination */
         print (thisAgent, msg);

		 if (f)
			 fprintf(f, msg) ;
      }
   } else {
      print (thisAgent, "\tERROR: HASHTABLE ITEM IS NOT AN IDENTIFIER!\n");
      return FALSE;
   }
   return TRUE;
}

bool reset_id_counters (agent* thisAgent) {
  int i;

  if (thisAgent->identifier_hash_table->count != 0) {
    print (thisAgent, "Internal warning:  wanted to reset identifier generator numbers, but\n");
    print (thisAgent, "there are still some identifiers allocated.  (Probably a memory leak.)\n");
    print (thisAgent, "(Leaving identifier numbers alone.)\n");
	GenerateWarningXML(thisAgent, "Internal warning:  wanted to reset identifier generator numbers, but\nthere are still some identifiers allocated.  (Probably a memory leak.)\n(Leaving identifier numbers alone.)");

    /* RDF 01272003: Added this to improve the output from this error message */
	//TODO: append this to previous XML string or generate separate output?
    do_for_all_items_in_hash_table( thisAgent, thisAgent->identifier_hash_table, print_identifier_ref_info, 0);

	// Also dump the ids to a txt file
	FILE *ids = fopen("leaked-ids.txt", "w") ;
	if (ids)
	{
		do_for_all_items_in_hash_table( thisAgent, thisAgent->identifier_hash_table, print_identifier_ref_info, ids);
		fclose(ids) ;
	}

    return false;
  }
  for (i=0; i<26; i++) thisAgent->id_counter[i]=1;  
  return true ;
}

Bool reset_tc_num (agent* thisAgent, void *item, FILE* f) {
  Symbol *sym;

  sym = static_cast<symbol_union *>(item);
  if (sym->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) sym->id.tc_num = 0;
  else if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE) sym->var.tc_num = 0;
  return FALSE;
}

void reset_id_and_variable_tc_numbers (agent* thisAgent) {
  do_for_all_items_in_hash_table (thisAgent, thisAgent->identifier_hash_table, reset_tc_num,0);
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, reset_tc_num,0);
}

Bool reset_gensym_number (agent* thisAgent, void *item, FILE* f) {
  Symbol *sym;

  sym = static_cast<symbol_union *>(item);
  sym->var.gensym_number = 0;
  return FALSE;
}

void reset_variable_gensym_numbers (agent* thisAgent) {
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, reset_gensym_number,0);
}

Bool print_sym (agent* thisAgent, void *item, FILE* f) {
  print_string (thisAgent, symbol_to_string (thisAgent, static_cast<symbol_union *>(item), TRUE, NIL, 0));
  print_string (thisAgent, "\n");
  return FALSE;
}

void print_internal_symbols (agent* thisAgent) {
  print_string (thisAgent, "\n--- Symbolic Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->sym_constant_hash_table, print_sym,0);
  print_string (thisAgent, "\n--- Integer Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->int_constant_hash_table, print_sym,0);
  print_string (thisAgent, "\n--- Floating-Point Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->float_constant_hash_table, print_sym,0);
  print_string (thisAgent, "\n--- Identifiers: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->identifier_hash_table, print_sym,0);
  print_string (thisAgent, "\n--- Variables: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, print_sym,0);
}

Symbol *generate_new_sym_constant (agent* thisAgent, char *prefix, unsigned long *counter) {
#define GENERATE_NEW_SYM_CONSTANT_BUFFER_SIZE 2000 /* that ought to be long enough! */ /* and what if it's not!? -voigtjr */
  char name[GENERATE_NEW_SYM_CONSTANT_BUFFER_SIZE];  
  Symbol *New;

  while (TRUE) {
    snprintf (name,GENERATE_NEW_SYM_CONSTANT_BUFFER_SIZE, "%s%lu", prefix, (*counter)++);
	name[GENERATE_NEW_SYM_CONSTANT_BUFFER_SIZE - 1] = 0;
    if (! find_sym_constant (thisAgent, name)) break;
  }
  New = make_sym_constant (thisAgent, name);
  return New;
}

/* --------------------------------------------------------------------
   
                         Predefined Symbols

-------------------------------------------------------------------- */

void create_predefined_symbols (agent* thisAgent) {
  thisAgent->problem_space_symbol = make_sym_constant (thisAgent, "problem-space");
  thisAgent->state_symbol = make_sym_constant (thisAgent, "state");
  thisAgent->operator_symbol = make_sym_constant (thisAgent, "operator");
  thisAgent->superstate_symbol = make_sym_constant (thisAgent, "superstate");
  thisAgent->io_symbol = make_sym_constant (thisAgent, "io");
  thisAgent->object_symbol = make_sym_constant (thisAgent, "object");
  thisAgent->attribute_symbol = make_sym_constant (thisAgent, "attribute");
  thisAgent->impasse_symbol = make_sym_constant (thisAgent, "impasse");
  thisAgent->choices_symbol = make_sym_constant (thisAgent, "choices");
  thisAgent->none_symbol = make_sym_constant (thisAgent, "none");
  thisAgent->constraint_failure_symbol = make_sym_constant (thisAgent, "constraint-failure");
  thisAgent->no_change_symbol = make_sym_constant (thisAgent, "no-change");
  thisAgent->multiple_symbol = make_sym_constant (thisAgent, "multiple");
  thisAgent->conflict_symbol = make_sym_constant (thisAgent, "conflict");
  thisAgent->tie_symbol = make_sym_constant (thisAgent, "tie");
  thisAgent->item_symbol = make_sym_constant (thisAgent, "item");
  thisAgent->quiescence_symbol = make_sym_constant (thisAgent, "quiescence");
  thisAgent->t_symbol = make_sym_constant (thisAgent, "t");
  thisAgent->nil_symbol = make_sym_constant (thisAgent, "nil");
  thisAgent->type_symbol = make_sym_constant (thisAgent, "type");
  thisAgent->goal_symbol = make_sym_constant (thisAgent, "goal");
  thisAgent->name_symbol = make_sym_constant (thisAgent, "name");

  thisAgent->ts_context_variable = make_variable (thisAgent, "<ts>");
  thisAgent->to_context_variable = make_variable (thisAgent, "<to>");
  thisAgent->sss_context_variable = make_variable (thisAgent, "<sss>");
  thisAgent->sso_context_variable = make_variable (thisAgent, "<sso>");
  thisAgent->ss_context_variable = make_variable (thisAgent, "<ss>");
  thisAgent->so_context_variable = make_variable (thisAgent, "<so>");
  thisAgent->s_context_variable = make_variable (thisAgent, "<s>");
  thisAgent->o_context_variable = make_variable (thisAgent, "<o>");

  /* REW: begin 10.24.97 */
  thisAgent->wait_symbol = make_variable (thisAgent, "wait");
  /* REW: end   10.24.97 */
}

void release_helper(agent* thisAgent, Symbol** sym) {
   symbol_remove_ref(thisAgent,*sym);
   *sym = 0;
}

void release_predefined_symbols(agent* thisAgent) {
  release_helper(thisAgent,&(thisAgent->problem_space_symbol));
  release_helper(thisAgent,&(thisAgent->state_symbol));
  release_helper(thisAgent,&(thisAgent->operator_symbol));
  release_helper(thisAgent,&(thisAgent->superstate_symbol));
  release_helper(thisAgent,&(thisAgent->io_symbol));
  release_helper(thisAgent,&(thisAgent->object_symbol));
  release_helper(thisAgent,&(thisAgent->attribute_symbol));
  release_helper(thisAgent,&(thisAgent->impasse_symbol));
  release_helper(thisAgent,&(thisAgent->choices_symbol));
  release_helper(thisAgent,&(thisAgent->none_symbol));
  release_helper(thisAgent,&(thisAgent->constraint_failure_symbol));
  release_helper(thisAgent,&(thisAgent->no_change_symbol));
  release_helper(thisAgent,&(thisAgent->multiple_symbol));
  release_helper(thisAgent,&(thisAgent->conflict_symbol));
  release_helper(thisAgent,&(thisAgent->tie_symbol));
  release_helper(thisAgent,&(thisAgent->item_symbol));
  release_helper(thisAgent,&(thisAgent->quiescence_symbol));
  release_helper(thisAgent,&(thisAgent->t_symbol));
  release_helper(thisAgent,&(thisAgent->nil_symbol));
  release_helper(thisAgent,&(thisAgent->type_symbol));
  release_helper(thisAgent,&(thisAgent->goal_symbol));
  release_helper(thisAgent,&(thisAgent->name_symbol));

  release_helper(thisAgent,&(thisAgent->ts_context_variable));
  release_helper(thisAgent,&(thisAgent->to_context_variable));
  release_helper(thisAgent,&(thisAgent->sss_context_variable));
  release_helper(thisAgent,&(thisAgent->sso_context_variable));
  release_helper(thisAgent,&(thisAgent->ss_context_variable));
  release_helper(thisAgent,&(thisAgent->so_context_variable));
  release_helper(thisAgent,&(thisAgent->s_context_variable));
  release_helper(thisAgent,&(thisAgent->o_context_variable));

  /* REW: begin 10.24.97 */
  release_helper(thisAgent,&(thisAgent->wait_symbol));
  /* REW: end   10.24.97 */
}
