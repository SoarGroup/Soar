/*************************************************************************
 *
 *  file:  symtab.c
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

/* Uncomment the following line to get symbol debugging printouts */
/* #define DEBUG_SYMBOLS */

#include <ctype.h>
#include "soarkernel.h"

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
  var = item;
  return compress (hash_string(var->name),num_bits);
}

unsigned long hash_identifier (void *item, short num_bits) {
  identifier *id;
  id = item;
  return compress (id->name_number ^ (id->name_letter << 24), num_bits);
}

unsigned long hash_sym_constant (void *item, short num_bits) {
  sym_constant *sc;
  sc = item;
  return compress (hash_string(sc->name),num_bits);
}

unsigned long hash_int_constant (void *item, short num_bits) {
  int_constant *ic;
  ic = item;
  return compress ((unsigned long)(ic->value),num_bits);
}

unsigned long hash_float_constant (void *item, short num_bits) {
  float_constant *fc;
  fc = item;
  return compress ((unsigned long)(fc->value),num_bits);
}

/* -------------------------------------------------------------------

        Basic Symbol Table Data Structures and Initialization

------------------------------------------------------------------- */

#define get_next_symbol_hash_id() (current_agent(current_symbol_hash_id) += 137)

void init_symbol_tables (void) {
  int i;

  current_agent(variable_hash_table) = make_hash_table (0, hash_variable);
  current_agent(identifier_hash_table) = make_hash_table (0, hash_identifier);
  current_agent(sym_constant_hash_table) = make_hash_table (0, hash_sym_constant);
  current_agent(int_constant_hash_table) = make_hash_table (0, hash_int_constant);
  current_agent(float_constant_hash_table) = make_hash_table (0, hash_float_constant);

  init_memory_pool (&current_agent(variable_pool), sizeof(variable), "variable");
  init_memory_pool (&current_agent(identifier_pool), sizeof(identifier), "identifier");
  init_memory_pool (&current_agent(sym_constant_pool), sizeof(sym_constant), "sym constant");
  init_memory_pool (&current_agent(int_constant_pool), sizeof(int_constant), "int constant");
  init_memory_pool (&current_agent(float_constant_pool), sizeof(float_constant),
                    "float constant");

  for (i=0; i<26; i++) current_agent(id_counter)[i]=1;
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

Symbol *find_variable (char *name) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_variable_raw_info (name,current_agent(variable_hash_table)->log2size);
  sym = (Symbol *) (*(current_agent(variable_hash_table)->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (!strcmp(sym->var.name,name)) return sym;
  }
  return NIL;
}

Symbol *find_identifier (char name_letter, unsigned long name_number) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_identifier_raw_info (name_letter,name_number,
                                         current_agent(identifier_hash_table)->log2size);
  sym = (Symbol *) (*(current_agent(identifier_hash_table)->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if ((name_letter==sym->id.name_letter) &&
        (name_number==sym->id.name_number)) return sym;
  }
  return NIL;
}

Symbol *find_sym_constant (const char *name) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_sym_constant_raw_info (name,
                                           current_agent(sym_constant_hash_table)->log2size);
  sym = (Symbol *) (*(current_agent(sym_constant_hash_table)->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (!strcmp(sym->sc.name,name)) return sym;
  }
  return NIL;
}

Symbol *find_int_constant (long value) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_int_constant_raw_info (value,
                                           current_agent(int_constant_hash_table)->log2size);
  sym = (Symbol *) (*(current_agent(int_constant_hash_table)->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (value==sym->ic.value) return sym;
  }
  return NIL;
}

Symbol *find_float_constant (float value) {
  unsigned long hash_value;
  Symbol *sym;

  hash_value = hash_float_constant_raw_info (value,
                                        current_agent(float_constant_hash_table)->log2size);
  sym = (Symbol *) (*(current_agent(float_constant_hash_table)->buckets + hash_value));
  for ( ; sym!=NIL; sym = sym->common.next_in_hash_table) {
    if (value==sym->fc.value) return sym;
  }
  return NIL;
}

Symbol *make_variable (char *name) {
  Symbol *sym;

  sym = find_variable(name);
  if (sym) {
    sym->common.reference_count++;
  } else {
    allocate_with_pool (&current_agent(variable_pool), &sym);
    sym->common.symbol_type = VARIABLE_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id();
    sym->var.name = make_memory_block_for_string (name);
    sym->var.gensym_number = 0;
    sym->var.tc_num = 0;
    sym->var.rete_binding_locations = NIL;
    add_to_hash_table (current_agent(variable_hash_table), sym);
  }
  return sym;
}

Symbol *make_new_identifier (char name_letter, goal_stack_level level) {
  Symbol *sym;

  if (isalpha(name_letter)) {
    if (islower(name_letter)) name_letter = toupper(name_letter);
  } else {
    name_letter = 'I';
  }
  allocate_with_pool (&current_agent(identifier_pool), &sym);
  sym->common.symbol_type = IDENTIFIER_SYMBOL_TYPE;
  sym->common.reference_count = 1;
  sym->common.hash_id = get_next_symbol_hash_id();
  sym->id.name_letter = name_letter;
  sym->id.name_number = current_agent(id_counter)[name_letter-'A']++;
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
  add_to_hash_table (current_agent(identifier_hash_table), sym);
  return sym;
}

Symbol *make_sym_constant (char *name) {
  Symbol *sym;

  sym = find_sym_constant(name);
  if (sym) {
    sym->common.reference_count++;
  } else {
    allocate_with_pool (&current_agent(sym_constant_pool), &sym);
    sym->common.symbol_type = SYM_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id();
    sym->sc.name = make_memory_block_for_string (name);
    sym->sc.production = NIL;
    add_to_hash_table (current_agent(sym_constant_hash_table), sym);
  }
  return sym;
}

Symbol *make_int_constant (long value) {
  Symbol *sym;

  sym = find_int_constant(value);
  if (sym) {
    sym->common.reference_count++;
  } else {
    allocate_with_pool (&current_agent(int_constant_pool), &sym);
    sym->common.symbol_type = INT_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id();
    sym->ic.value = value;
    add_to_hash_table (current_agent(int_constant_hash_table), sym);
  }
  return sym;
}

Symbol *make_float_constant (float value) {
  Symbol *sym;

  sym = find_float_constant(value);
  if (sym) {
    sym->common.reference_count++;
  } else {
    allocate_with_pool (&current_agent(float_constant_pool), &sym);
    sym->common.symbol_type = FLOAT_CONSTANT_SYMBOL_TYPE;
    sym->common.reference_count = 1;
    sym->common.hash_id = get_next_symbol_hash_id();
    sym->fc.value = value;
    add_to_hash_table (current_agent(float_constant_hash_table), sym);
  }
  return sym;
}

/* -------------------------------------------------------------------

                         Deallocate Symbol

------------------------------------------------------------------- */

void deallocate_symbol (Symbol *sym) {

#ifdef DEBUG_SYMBOLS  
  print_with_symbols ("\nDeallocating Symbol %y", sym);
#endif

  switch (sym->common.symbol_type) {
  case VARIABLE_SYMBOL_TYPE:
    remove_from_hash_table (current_agent(variable_hash_table), sym);
    free_memory_block_for_string (sym->var.name);
    free_with_pool (&current_agent(variable_pool), sym);
    break;
  case IDENTIFIER_SYMBOL_TYPE:
    remove_from_hash_table (current_agent(identifier_hash_table), sym);
    free_with_pool (&current_agent(identifier_pool), sym);
    break;
  case SYM_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (current_agent(sym_constant_hash_table), sym);
    free_memory_block_for_string (sym->sc.name);
    free_with_pool (&current_agent(sym_constant_pool), sym);
    break;
  case INT_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (current_agent(int_constant_hash_table), sym);
    free_with_pool (&current_agent(int_constant_pool), sym);
    break;
  case FLOAT_CONSTANT_SYMBOL_TYPE:
    remove_from_hash_table (current_agent(float_constant_hash_table), sym);
    free_with_pool (&current_agent(float_constant_pool), sym);
    break;
  default:
    { char msg[128];
    strcpy (msg, "Internal error: called deallocate_symbol on non-symbol.\n");
    abort_with_fatal_error(msg);
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

void reset_id_counters (void) {
  int i;

  if (current_agent(identifier_hash_table)->count != 0) {
    print ("Internal warning:  wanted to reset identifier generator numbers, but\n");
    print ("there are still some identifiers allocated.  (Probably a memory leak.)\n");
    print ("(Leaving identifier numbers alone.)\n");
    return;
  }
  for (i=0; i<26; i++) current_agent(id_counter)[i]=1;  
}

bool reset_tc_num (void *item) {
  Symbol *sym;

  sym = item;
  if (sym->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) sym->id.tc_num = 0;
  else if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE) sym->var.tc_num = 0;
  return FALSE;
}

void reset_id_and_variable_tc_numbers (void) {
  do_for_all_items_in_hash_table (current_agent(identifier_hash_table), reset_tc_num);
  do_for_all_items_in_hash_table (current_agent(variable_hash_table), reset_tc_num);
}

bool reset_gensym_number (void *item) {
  Symbol *sym;

  sym = item;
  sym->var.gensym_number = 0;
  return FALSE;
}

void reset_variable_gensym_numbers (void) {
  do_for_all_items_in_hash_table (current_agent(variable_hash_table), reset_gensym_number);
}

bool print_sym (void *item) {
  print_string (symbol_to_string (item, TRUE, NIL));
  print_string ("\n");
  return FALSE;
}

void print_internal_symbols (void) {
  print_string ("\n--- Symbolic Constants: ---\n");
  do_for_all_items_in_hash_table (current_agent(sym_constant_hash_table), print_sym);
  print_string ("\n--- Integer Constants: ---\n");
  do_for_all_items_in_hash_table (current_agent(int_constant_hash_table), print_sym);
  print_string ("\n--- Floating-Point Constants: ---\n");
  do_for_all_items_in_hash_table (current_agent(float_constant_hash_table), print_sym);
  print_string ("\n--- Identifiers: ---\n");
  do_for_all_items_in_hash_table (current_agent(identifier_hash_table), print_sym);
  print_string ("\n--- Variables: ---\n");
  do_for_all_items_in_hash_table (current_agent(variable_hash_table), print_sym);
}

Symbol *generate_new_sym_constant (char *prefix, unsigned long *counter) {
  char name[2000];  /* that ought to be long enough! */
  Symbol *new;

  while (TRUE) {
    sprintf (name, "%s%lu", prefix, (*counter)++);
    if (! find_sym_constant (name)) break;
  }
  new = make_sym_constant (name);
  return new;
}

/* --------------------------------------------------------------------
   
                         Predefined Symbols

-------------------------------------------------------------------- */

void create_predefined_symbols (void) {
  current_agent(problem_space_symbol) = make_sym_constant ("problem-space");
  current_agent(state_symbol) = make_sym_constant ("state");
  current_agent(operator_symbol) = make_sym_constant ("operator");
  current_agent(superstate_symbol) = make_sym_constant ("superstate");
  current_agent(io_symbol) = make_sym_constant ("io");
  current_agent(object_symbol) = make_sym_constant ("object");
  current_agent(attribute_symbol) = make_sym_constant ("attribute");
  current_agent(impasse_symbol) = make_sym_constant ("impasse");
  current_agent(choices_symbol) = make_sym_constant ("choices");
  current_agent(none_symbol) = make_sym_constant ("none");
  current_agent(constraint_failure_symbol) = make_sym_constant ("constraint-failure");
  current_agent(no_change_symbol) = make_sym_constant ("no-change");
  current_agent(multiple_symbol) = make_sym_constant ("multiple");
  current_agent(conflict_symbol) = make_sym_constant ("conflict");
  current_agent(tie_symbol) = make_sym_constant ("tie");
  current_agent(item_symbol) = make_sym_constant ("item");
  current_agent(quiescence_symbol) = make_sym_constant ("quiescence");
  current_agent(t_symbol) = make_sym_constant ("t");
  current_agent(nil_symbol) = make_sym_constant ("nil");
  current_agent(type_symbol) = make_sym_constant ("type");
  current_agent(goal_symbol) = make_sym_constant ("goal");
  current_agent(name_symbol) = make_sym_constant ("name");

  current_agent(ts_context_variable) = make_variable ("<ts>");
  current_agent(to_context_variable) = make_variable ("<to>");
  current_agent(sss_context_variable) = make_variable ("<sss>");
  current_agent(sso_context_variable) = make_variable ("<sso>");
  current_agent(ss_context_variable) = make_variable ("<ss>");
  current_agent(so_context_variable) = make_variable ("<so>");
  current_agent(s_context_variable) = make_variable ("<s>");
  current_agent(o_context_variable) = make_variable ("<o>");

  /* REW: begin 10.24.97 */
  current_agent(wait_symbol) = make_variable ("wait");
  /* REW: end   10.24.97 */
}

