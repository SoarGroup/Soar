#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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
 *  sym->field_name; fields particular to a certain kind of
 *  symbol are accessed via sym->var->field_name_on_variables, etc.
 *  See soarkernel.h for the Symbol structure definitions.
 *
 * =======================================================================
 */

#include <stdlib.h>

#include "symtab.h"
#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "production.h"
#include "init_soar.h"
#include "print.h"
#include "xml.h"
#include "output_manager.h"
#include <ctype.h>

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

uint32_t compress (uint32_t h, short num_bits) {
  uint32_t result;

  if (num_bits < 16) h = (h & 0xFFFF) ^ (h >> 16);
  if (num_bits < 8) h = (h & 0xFF) ^ (h >> 8);
  result = 0;
  while (h) {
    result ^= (h & masks_for_n_low_order_bits[num_bits]);
    h = h >> num_bits;
  }
  return result;
}

uint32_t hash_string (const char *s) {   /* AGR 600 */
  uint32_t h;

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

uint32_t hash_variable_raw_info (const char *name, short num_bits) {
  return compress (hash_string(name), num_bits);
}

uint32_t hash_identifier_raw_info (char name_letter,
    uint64_t name_number,
    short num_bits) {
  return compress (static_cast<uint32_t>(name_number) ^ (static_cast<uint32_t>(name_letter) << 24), num_bits); // FIXME: cast from 64 to 32 bits
}

uint32_t hash_str_constant_raw_info (const char *name, short num_bits) {
  return compress (hash_string(name), num_bits);
}

uint32_t hash_int_constant_raw_info (int64_t value, short num_bits) {
  return compress (static_cast<uint32_t>(value), num_bits);
}

uint32_t hash_float_constant_raw_info (double value, short num_bits) {
  return compress (static_cast<uint32_t>(value), num_bits);
}

/* ---------------------------------------------------
   Hashing symbols using their symbol table entries
--------------------------------------------------- */

uint32_t hash_variable (void *item, short num_bits) {
  varSymbol *var;
  var = static_cast<varSymbol *>(item);
  return compress (hash_string(var->name),num_bits);
}

uint32_t hash_identifier (void *item, short num_bits) {
  idSymbol *id;
  id = static_cast<idSymbol *>(item);
  return compress (static_cast<uint32_t>(id->name_number) ^ (static_cast<uint32_t>(id->name_letter) << 24), num_bits); // FIXME: cast from 64 to 32 bits
}

uint32_t hash_str_constant (void *item, short num_bits) {
  strSymbol *sc;
  sc = static_cast<strSymbol *>(item);
  return compress (hash_string(sc->name),num_bits);
}

uint32_t hash_int_constant (void *item, short num_bits) {
  intSymbol *ic;
  ic = static_cast<intSymbol *>(item);
  return compress (static_cast<uint32_t>(ic->value),num_bits);
}

uint32_t hash_float_constant (void *item, short num_bits) {
  floatSymbol *fc;
  fc = static_cast<floatSymbol *>(item);
  return compress (static_cast<uint32_t>(fc->value),num_bits);
}

/* -----------------------------------------------------------------
                       Symbol Table Routines

   Initialization:

     Init_symbol_tables() should be called first, to initialize the
     module.

   Lookup and Creation:

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

   Reference Counting:

     Symbol_add_ref() and symbol_remove_ref() are macros for incrementing
     and decrementing the reference count on a symbol.  When the count
     goes to zero, symbol_remove_ref() calls deallocate_symbol().

   Other Utilities:

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

     Generate_new_str_constant() is used to gensym new symbols that are
     guaranteed to not already exist.  It takes two arguments: "prefix"
     (the desired prefix of the new symbol's name), and "counter" (a
     pointer to a counter (uint64_t) that is incremented to produce
     new gensym names).
----------------------------------------------------------------- */

inline uint32_t get_next_symbol_hash_id(agent* thisAgent)
{
  return (thisAgent->current_symbol_hash_id += 137);
}

void init_symbol_tables (agent* thisAgent) {
  thisAgent->variable_hash_table = make_hash_table (thisAgent, 0, hash_variable);
  thisAgent->identifier_hash_table = make_hash_table (thisAgent, 0, hash_identifier);
  thisAgent->str_constant_hash_table = make_hash_table (thisAgent, 0, hash_str_constant);
  thisAgent->int_constant_hash_table = make_hash_table (thisAgent, 0, hash_int_constant);
  thisAgent->float_constant_hash_table = make_hash_table (thisAgent, 0, hash_float_constant);

  init_memory_pool (thisAgent, &thisAgent->variable_pool, sizeof(varSymbol), "variable");
  init_memory_pool (thisAgent, &thisAgent->identifier_pool, sizeof(idSymbol), "identifier");
  init_memory_pool (thisAgent, &thisAgent->str_constant_pool, sizeof(strSymbol), "str constant");
  init_memory_pool (thisAgent, &thisAgent->int_constant_pool, sizeof(intSymbol), "int constant");
  init_memory_pool (thisAgent, &thisAgent->float_constant_pool, sizeof(floatSymbol), "float constant");

  reset_id_counters( thisAgent );
}

Symbol *find_variable (agent* thisAgent, const char *name) {
  uint32_t hash_value;
  varSymbol *sym;

  hash_value = hash_variable_raw_info (name,thisAgent->variable_hash_table->log2size);
  sym = reinterpret_cast<varSymbol *>(*(thisAgent->variable_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = varSym(sym->next_in_hash_table)) {
    if (!strcmp(sym->name,name)) return sym;
  }
  return NIL;
}

Symbol *find_identifier (agent* thisAgent, char name_letter, uint64_t name_number) {
  uint32_t hash_value;
  idSymbol *sym;

  hash_value = hash_identifier_raw_info (name_letter,name_number,
      thisAgent->identifier_hash_table->log2size);
  sym = reinterpret_cast<idSymbol *>(*(thisAgent->identifier_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = idSym(sym->next_in_hash_table)) {
    if ((name_letter==sym->name_letter) &&
        (name_number==sym->name_number)) return sym;
  }
  return NIL;
}

Symbol *find_str_constant (agent* thisAgent, const char *name) {
  uint32_t hash_value;
  strSymbol *sym;

  hash_value = hash_str_constant_raw_info (name,
      thisAgent->str_constant_hash_table->log2size);
  sym = reinterpret_cast<strSymbol *>(*(thisAgent->str_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = strSym(sym->next_in_hash_table)) {
    if (!strcmp(sym->name,name)) return sym;
  }
  return NIL;
}

Symbol *find_int_constant (agent* thisAgent, int64_t value) {
  uint32_t hash_value;
  intSymbol *sym;

  hash_value = hash_int_constant_raw_info (value,
      thisAgent->int_constant_hash_table->log2size);
  sym = reinterpret_cast<intSymbol *>(*(thisAgent->int_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = intSym(sym->next_in_hash_table)) {
    if (value==sym->value) return sym;
  }
  return NIL;
}

Symbol *find_float_constant (agent* thisAgent, double value) {
  uint32_t hash_value;
  floatSymbol *sym;

  hash_value = hash_float_constant_raw_info (value,
      thisAgent->float_constant_hash_table->log2size);
  sym = reinterpret_cast<floatSymbol *>(*(thisAgent->float_constant_hash_table->buckets + hash_value));
  for ( ; sym!=NIL; sym = floatSym(sym->next_in_hash_table)) {
    if (value==sym->value) return sym;
  }
  return NIL;
}

Symbol *make_variable (agent* thisAgent, const char *name) {

  varSymbol *sym;

  sym = varSym(find_variable(thisAgent, name));
  if (sym) {
    symbol_add_ref(thisAgent, sym);
    return sym;
  }

  allocate_with_pool (thisAgent, &thisAgent->variable_pool, &sym);
  sym->symbol_type = VARIABLE_SYMBOL_TYPE;
  sym->reference_count = 0;
  sym->hash_id = get_next_symbol_hash_id(thisAgent);
  sym->tc_num = 0;
  sym->name = make_memory_block_for_string (thisAgent, name);
  sym->gensym_number = 0;
  sym->rete_binding_locations = NIL;
  sym->fc = NIL;
  sym->ic = NIL;
  sym->sc = NIL;
  sym->id = NIL;
  sym->var = sym;
  symbol_add_ref(thisAgent, sym);
  add_to_hash_table (thisAgent, thisAgent->variable_hash_table, sym);

  return sym;
}

Symbol *make_new_identifier (agent* thisAgent, char name_letter, goal_stack_level level, uint64_t name_number) {

  idSymbol *sym;

  if (isalpha(name_letter)) {
    if (islower(name_letter)) name_letter = static_cast<char>(toupper(name_letter));
  } else {
    name_letter = 'I';
  }
  allocate_with_pool (thisAgent, &thisAgent->identifier_pool, &sym);
  sym->symbol_type = IDENTIFIER_SYMBOL_TYPE;
  sym->reference_count = 0;
  sym->hash_id = get_next_symbol_hash_id(thisAgent);
  sym->tc_num = 0;
  sym->name_letter = name_letter;

  // For long-term identifiers
  if ( name_number == NIL )
  {
    name_number = thisAgent->id_counter[name_letter-'A']++;
  }
  else
  {
    uint64_t *current_number = &( thisAgent->id_counter[ name_letter - 'A' ] );
    if ( name_number >= (*current_number) )
    {
      (*current_number) = ( name_number + 1 );
    }
  }
  sym->name_number = name_number;

  sym->level = level;
  sym->promotion_level = level;
  sym->slots = NIL;
  sym->isa_goal = false;
  sym->isa_impasse = false;
  sym->isa_operator = 0;
  sym->link_count = 0;
  sym->unknown_level = NIL;
  sym->could_be_a_link_from_below = false;
  sym->impasse_wmes = NIL;
  sym->higher_goal = NIL;
  sym->gds = NIL;
  sym->saved_firing_type = NO_SAVED_PRODS;
  sym->ms_o_assertions = NIL;
  sym->ms_i_assertions = NIL;
  sym->ms_retractions = NIL;
  sym->lower_goal = NIL;
  sym->operator_slot = NIL;
  sym->preferences_from_goal = NIL;
  sym->associated_output_links = NIL;
  sym->input_wmes = NIL;

  sym->rl_info = NIL;
  sym->reward_header = NIL;

  sym->epmem_header = NIL;
  sym->epmem_cmd_header = NIL;
  sym->epmem_result_header = NIL;
  sym->epmem_id = EPMEM_NODEID_BAD;
  sym->epmem_valid = NIL;
  sym->epmem_time_wme = NIL;

  sym->smem_header = NIL;
  sym->smem_cmd_header = NIL;
  sym->smem_result_header = NIL;
  sym->smem_lti = NIL;
  sym->smem_time_id = EPMEM_MEMID_NONE;
  sym->smem_valid = NIL;

  sym->variablization = NIL;

  sym->rl_trace = NIL;

  sym->fc = NIL;
  sym->ic = NIL;
  sym->sc = NIL;
  sym->var = NIL;
  sym->id = sym;
  symbol_add_ref(thisAgent, sym);
  add_to_hash_table (thisAgent, thisAgent->identifier_hash_table, sym);

  return sym;
}

Symbol *make_str_constant (agent* thisAgent, char const* name) {
  strSymbol *sym;

  sym = strSym(find_str_constant(thisAgent, name));
  if (sym) {
    symbol_add_ref(thisAgent, sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->str_constant_pool, &sym);
    sym->symbol_type = STR_CONSTANT_SYMBOL_TYPE;
    sym->reference_count = 0;
    sym->hash_id = get_next_symbol_hash_id(thisAgent);
    sym->tc_num = 0;
    sym->epmem_hash = 0;
    sym->epmem_valid = 0;
    sym->smem_hash = 0;
    sym->smem_valid = 0;
    sym->name = make_memory_block_for_string (thisAgent, name);
    sym->production = NIL;
    sym->fc = NIL;
    sym->ic = NIL;
    sym->id = NIL;
    sym->var = NIL;
    sym->sc = sym;
    symbol_add_ref(thisAgent, sym);
    add_to_hash_table (thisAgent, thisAgent->str_constant_hash_table, sym);
  }
  return sym;
}

Symbol *make_int_constant (agent* thisAgent, int64_t value) {
  intSymbol *sym;

  sym = intSym(find_int_constant(thisAgent, value));
  if (sym) {
    symbol_add_ref(thisAgent, sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->int_constant_pool, &sym);
    sym->symbol_type = INT_CONSTANT_SYMBOL_TYPE;
    sym->reference_count = 0;
    sym->hash_id = get_next_symbol_hash_id(thisAgent);
    sym->tc_num = 0;
    sym->epmem_hash = 0;
    sym->epmem_valid = 0;
    sym->smem_hash = 0;
    sym->smem_valid = 0;
    sym->value = value;
    sym->fc = NIL;
    sym->sc = NIL;
    sym->id = NIL;
    sym->var = NIL;
    sym->ic = sym;
    symbol_add_ref(thisAgent, sym);
    add_to_hash_table (thisAgent, thisAgent->int_constant_hash_table, sym);
  }
  return sym;
}

Symbol *make_float_constant (agent* thisAgent, double value) {
  floatSymbol *sym;

  sym = floatSym(find_float_constant(thisAgent, value));
  if (sym) {
    symbol_add_ref(thisAgent, sym);
  } else {
    allocate_with_pool (thisAgent, &thisAgent->float_constant_pool, &sym);
    sym->symbol_type = FLOAT_CONSTANT_SYMBOL_TYPE;
    sym->reference_count = 0;
    sym->hash_id = get_next_symbol_hash_id(thisAgent);
    sym->tc_num = 0;
    sym->epmem_hash = 0;
    sym->epmem_valid = 0;
    sym->smem_hash = 0;
    sym->smem_valid = 0;
    sym->value = value;
    sym->ic = NIL;
    sym->sc = NIL;
    sym->id = NIL;
    sym->var = NIL;
    sym->fc = sym;
    symbol_add_ref(thisAgent, sym);
    add_to_hash_table (thisAgent, thisAgent->float_constant_hash_table, sym);
  }
  return sym;
}

/* -------------------------------------------------------------------

                         Deallocate Symbol

------------------------------------------------------------------- */

void deallocate_symbol (agent* thisAgent, Symbol *sym, long indent) {

  dprint(DT_DEALLOCATE_SYMBOLS, "%*sDEALLOCATE symbol %s\n", indent, "", sym->to_string());

  switch (sym->symbol_type) {
    case VARIABLE_SYMBOL_TYPE:
      remove_from_hash_table (thisAgent, thisAgent->variable_hash_table, sym);
      free_memory_block_for_string (thisAgent, sym->var->name);
      free_with_pool (&thisAgent->variable_pool, sym);
      break;
    case IDENTIFIER_SYMBOL_TYPE:
      remove_from_hash_table (thisAgent, thisAgent->identifier_hash_table, sym);
      free_with_pool (&thisAgent->identifier_pool, sym);
      break;
    case STR_CONSTANT_SYMBOL_TYPE:
      remove_from_hash_table (thisAgent, thisAgent->str_constant_hash_table, sym);
      free_memory_block_for_string (thisAgent, sym->sc->name);
      free_with_pool (&thisAgent->str_constant_pool, sym);
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

   Generate_new_str_constant() is used to gensym new symbols that are
   guaranteed to not already exist.  It takes two arguments: "prefix"
   (the desired prefix of the new symbol's name), and "counter" (a
   pointer to a counter (uint64_t) that is incremented to produce
   new gensym names).
------------------------------------------------------------------- */

bool print_identifier_ref_info(agent* thisAgent, void* item, void* userdata) {
  Symbol* sym;
  char msg[256];
  sym = static_cast<symbol_struct *>(item);
  FILE* f = reinterpret_cast<FILE*>(userdata);

  if ( sym->symbol_type == IDENTIFIER_SYMBOL_TYPE ) {
    if ( sym->reference_count > 0 ) {

      if ( sym->id->smem_lti != NIL )
      {
        SNPRINTF( msg, 256,
            "\t@%c%llu --> %llu\n",
            sym->id->name_letter,
            static_cast<long long unsigned>(sym->id->name_number),
            static_cast<long long unsigned>(sym->reference_count));
      }
      else
      {
        SNPRINTF( msg, 256,
            "\t%c%llu --> %llu\n",
            sym->id->name_letter,
            static_cast<long long unsigned>(sym->id->name_number),
            static_cast<long long unsigned>(sym->reference_count));
      }

      msg[255] = 0; /* ensure null termination */
      print(thisAgent,  msg);
      xml_generate_warning(thisAgent, msg);

      if (f) {
        fprintf(f, "%s", msg) ;
      }
    }
  } else {
    print(thisAgent,  "\tERROR: HASHTABLE ITEM IS NOT AN IDENTIFIER!\n");
    return true;
  }
  return false;
}

bool reset_id_counters (agent* thisAgent) {
  int i;

  if (thisAgent->identifier_hash_table->count != 0) {
    // As long as all of the existing identifiers are long term identifiers (lti), there's no problem
    uint64_t ltis = 0;
    do_for_all_items_in_hash_table( thisAgent, thisAgent->identifier_hash_table, smem_count_ltis, &ltis );
    if (static_cast<uint64_t>(thisAgent->identifier_hash_table->count) != ltis) {
      print(thisAgent,  "Internal warning:  wanted to reset identifier generator numbers, but\n");
      print(thisAgent,  "there are still some identifiers allocated.  (Probably a memory leak.)\n");
      print(thisAgent,  "(Leaving identifier numbers alone.)\n");
      xml_generate_warning(thisAgent, "Internal warning:  wanted to reset identifier generator numbers, but\nthere are still some identifiers allocated.  (Probably a memory leak.)\n(Leaving identifier numbers alone.)");

      print_internal_symbols(thisAgent);
      /* RDF 01272003: Added this to improve the output from this error message */
      //TODO: append this to previous XML string or generate separate output?
      //do_for_all_items_in_hash_table( thisAgent, thisAgent->identifier_hash_table, print_identifier_ref_info, 0);

      // Also dump the ids to a txt file
      FILE *ids = fopen("leaked-ids.txt", "w") ;
      if (ids)
      {
        do_for_all_items_in_hash_table( thisAgent, thisAgent->identifier_hash_table, print_identifier_ref_info, reinterpret_cast<void*>(ids));
        fclose(ids) ;
      }

      return false;
    }

    // Getting here means that there are still identifiers but that
    // they are all long-term and (hopefully) exist only in production memory.
  }
  for (i=0; i<26; i++) thisAgent->id_counter[i]=1;

  if ( thisAgent->smem_db->get_status() == soar_module::connected )
  {
    smem_reset_id_counters( thisAgent );
  }

  return true ;
}

bool reset_tc_num (agent* /*thisAgent*/, void *item, void*) {
  Symbol *sym;

  sym = static_cast<symbol_struct *>(item);
  sym->tc_num = 0;
  return false;
}

void reset_id_and_variable_tc_numbers (agent* thisAgent) {
  do_for_all_items_in_hash_table (thisAgent, thisAgent->identifier_hash_table, reset_tc_num,0);
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, reset_tc_num,0);
}

bool reset_gensym_number (agent* /*thisAgent*/, void *item, void*) {
  Symbol *sym;

  sym = static_cast<symbol_struct *>(item);
  sym->var->gensym_number = 0;
  return false;
}

void reset_variable_gensym_numbers (agent* thisAgent) {
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, reset_gensym_number,0);
}

bool print_sym (agent* thisAgent, void *item, void*) {
  print(thisAgent,  "%s (%lld)\n", static_cast<symbol_struct *>(item)->to_string(), static_cast<symbol_struct *>(item)->reference_count);
  return false;
}

void print_internal_symbols (agent* thisAgent) {
  print(thisAgent,  "\n--- Symbolic Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->str_constant_hash_table, print_sym,0);
  print(thisAgent,  "\n--- Integer Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->int_constant_hash_table, print_sym,0);
  print(thisAgent,  "\n--- Floating-Point Constants: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->float_constant_hash_table, print_sym,0);
  print(thisAgent,  "\n--- Identifiers: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->identifier_hash_table, print_sym,0);
  print(thisAgent,  "\n--- Variables: ---\n");
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table, print_sym,0);
}

Symbol *generate_new_str_constant (agent* thisAgent, const char *prefix, uint64_t* counter) {

#define GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE 2000 /* that ought to be long enough! */
  char name[GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE];
  Symbol *New;

  while (true) {
    SNPRINTF (name,GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE, "%s%lu", prefix, static_cast<long unsigned int>((*counter)++));
    name[GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE - 1] = 0;
    if (! find_str_constant (thisAgent, name)) break;
  }
  New = make_str_constant (thisAgent, name);
  return New;
}

/* -----------------------------------------------------------------
                       First Letter From Symbol

   When creating dummy variables or identifiers, we try to give them
   names that start with a "reasonable" letter.  For example, ^foo <dummy>
   becomes ^foo <f*37>, where the variable starts with "f" because
   the attribute test starts with "f" also.  This routine looks at
   a symbol and tries to figure out a reasonable choice of starting
   letter for a variable or identifier to follow it.  If it can't
   find a reasonable choice, it returns '*'.
----------------------------------------------------------------- */

char first_letter_from_symbol (Symbol *sym) {
  switch (sym->symbol_type) {
  case VARIABLE_SYMBOL_TYPE: return *(sym->var->name + 1);
  case IDENTIFIER_SYMBOL_TYPE: return sym->id->name_letter;
  case STR_CONSTANT_SYMBOL_TYPE: return *(sym->sc->name);
  default: return '*';
  }
}

/***************************************************************************
 * Function     : get_number_from_symbol
 **************************************************************************/
double get_number_from_symbol( Symbol *sym )
{
  if ( sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
    return sym->fc->value;
  else if ( sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE )
    return static_cast<double>(sym->ic->value);

  return 0.0;
}

/* ----------------------------------------------------------------
   Takes a list of symbols and returns a copy of the same list,
   incrementing the reference count on each symbol in the list.
---------------------------------------------------------------- */

list *copy_symbol_list_adding_references (agent* thisAgent,
                      list *sym_list) {
  cons *c, *first, *prev;

  if (! sym_list) return NIL;
  allocate_cons (thisAgent, &first);
  first->first = sym_list->first;
  symbol_add_ref(thisAgent, static_cast<Symbol *>(first->first));
  sym_list = sym_list->rest;
  prev = first;
  while (sym_list) {
    allocate_cons (thisAgent, &c);
    prev->rest = c;
    c->first = sym_list->first;
    symbol_add_ref(thisAgent, static_cast<Symbol *>(c->first));
    sym_list = sym_list->rest;
    prev = c;
  }
  prev->rest = NIL;
  return first;
}

/* ----------------------------------------------------------------
   Frees a list of symbols, decrementing their reference counts.
---------------------------------------------------------------- */

void deallocate_symbol_list_removing_references (agent* thisAgent,
                         list *sym_list, long indent) {
  cons *c;

  while (sym_list) {
    c = sym_list;
    sym_list = sym_list->rest;
#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
    symbol_remove_ref (thisAgent, static_cast<Symbol *>(c->first));
#else
    symbol_remove_ref (thisAgent, static_cast<Symbol *>(c->first), indent);
#endif
    free_cons (thisAgent, c);
  }
}

/* -----------------------------------------------------------------
                       Predefined Symbols

   Certain symbols are used so frequently that we create them at
   system startup time and never deallocate them.  These symbols are
   global variables (per-agent) and are named xxx_symbol (see glob_vars.h).

   Create_predefined_symbols() should be called to do the creation.
   After that, the global variables can be accessed freely.  Note that
   the reference counts on these symbols should still be updated--
   symbol_add_ref() should be called, etc.--it's just that when the
   symbol isn't really being used, it stays around because the count
   is still 1.
----------------------------------------------------------------- */

void create_predefined_symbols (agent* thisAgent) {
  thisAgent->problem_space_symbol = make_str_constant (thisAgent, "problem-space");
  thisAgent->state_symbol = make_str_constant (thisAgent, "state");
  thisAgent->operator_symbol = make_str_constant (thisAgent, "operator");
  thisAgent->superstate_symbol = make_str_constant (thisAgent, "superstate");
  thisAgent->io_symbol = make_str_constant (thisAgent, "io");
  thisAgent->object_symbol = make_str_constant (thisAgent, "object");
  thisAgent->attribute_symbol = make_str_constant (thisAgent, "attribute");
  thisAgent->impasse_symbol = make_str_constant (thisAgent, "impasse");
  thisAgent->choices_symbol = make_str_constant (thisAgent, "choices");
  thisAgent->none_symbol = make_str_constant (thisAgent, "none");
  thisAgent->constraint_failure_symbol = make_str_constant (thisAgent, "constraint-failure");
  thisAgent->no_change_symbol = make_str_constant (thisAgent, "no-change");
  thisAgent->multiple_symbol = make_str_constant (thisAgent, "multiple");

  // SBW 5/07
  thisAgent->item_count_symbol = make_str_constant (thisAgent, "item-count");

  // NLD 11/11
  thisAgent->non_numeric_count_symbol = make_str_constant( thisAgent, "non-numeric-count" );

  thisAgent->conflict_symbol = make_str_constant (thisAgent, "conflict");
  thisAgent->tie_symbol = make_str_constant (thisAgent, "tie");
  thisAgent->item_symbol = make_str_constant (thisAgent, "item");
  thisAgent->non_numeric_symbol = make_str_constant (thisAgent, "non-numeric");
  thisAgent->quiescence_symbol = make_str_constant (thisAgent, "quiescence");
  thisAgent->t_symbol = make_str_constant (thisAgent, "t");
  thisAgent->nil_symbol = make_str_constant (thisAgent, "nil");
  thisAgent->type_symbol = make_str_constant (thisAgent, "type");
  thisAgent->goal_symbol = make_str_constant (thisAgent, "goal");
  thisAgent->name_symbol = make_str_constant (thisAgent, "name");

  thisAgent->ts_context_variable = make_variable (thisAgent, "<ts>");
  thisAgent->to_context_variable = make_variable (thisAgent, "<to>");
  thisAgent->sss_context_variable = make_variable (thisAgent, "<sss>");
  thisAgent->sso_context_variable = make_variable (thisAgent, "<sso>");
  thisAgent->ss_context_variable = make_variable (thisAgent, "<ss>");
  thisAgent->so_context_variable = make_variable (thisAgent, "<so>");
  thisAgent->s_context_variable = make_variable (thisAgent, "<s>");
  thisAgent->o_context_variable = make_variable (thisAgent, "<o>");

  thisAgent->input_link_symbol = make_str_constant(thisAgent, "input-link");
  thisAgent->output_link_symbol = make_str_constant(thisAgent, "output-link");

  thisAgent->rl_sym_reward_link = make_str_constant( thisAgent, "reward-link" );
  thisAgent->rl_sym_reward = make_str_constant( thisAgent, "reward" );
  thisAgent->rl_sym_value = make_str_constant( thisAgent, "value" );

  thisAgent->epmem_sym = make_str_constant( thisAgent, "epmem" );
  thisAgent->epmem_sym_cmd = make_str_constant( thisAgent, "command" );
  thisAgent->epmem_sym_result = make_str_constant( thisAgent, "result" );

  thisAgent->epmem_sym_retrieved = make_str_constant( thisAgent, "retrieved" );
  thisAgent->epmem_sym_status = make_str_constant( thisAgent, "status" );
  thisAgent->epmem_sym_match_score = make_str_constant( thisAgent, "match-score" );
  thisAgent->epmem_sym_cue_size = make_str_constant( thisAgent, "cue-size" );
  thisAgent->epmem_sym_normalized_match_score = make_str_constant( thisAgent, "normalized-match-score" );
  thisAgent->epmem_sym_match_cardinality = make_str_constant( thisAgent, "match-cardinality" );
  thisAgent->epmem_sym_memory_id = make_str_constant( thisAgent, "memory-id" );
  thisAgent->epmem_sym_present_id = make_str_constant( thisAgent, "present-id" );
  thisAgent->epmem_sym_no_memory = make_str_constant( thisAgent, "no-memory" );
  thisAgent->epmem_sym_graph_match = make_str_constant( thisAgent, "graph-match" );
  thisAgent->epmem_sym_graph_match_mapping = make_str_constant( thisAgent, "mapping" );
  thisAgent->epmem_sym_graph_match_mapping_node = make_str_constant( thisAgent, "node" );
  thisAgent->epmem_sym_graph_match_mapping_cue = make_str_constant( thisAgent, "cue" );
  thisAgent->epmem_sym_success = make_str_constant( thisAgent, "success" );
  thisAgent->epmem_sym_failure = make_str_constant( thisAgent, "failure" );
  thisAgent->epmem_sym_bad_cmd = make_str_constant( thisAgent, "bad-cmd" );

  thisAgent->epmem_sym_retrieve = make_str_constant( thisAgent, "retrieve" );
  thisAgent->epmem_sym_next = make_str_constant( thisAgent, "next" );
  thisAgent->epmem_sym_prev = make_str_constant( thisAgent, "previous" );
  thisAgent->epmem_sym_query = make_str_constant( thisAgent, "query" );
  thisAgent->epmem_sym_negquery = make_str_constant( thisAgent, "neg-query" );
  thisAgent->epmem_sym_before = make_str_constant( thisAgent, "before" );
  thisAgent->epmem_sym_after = make_str_constant( thisAgent, "after" );
  thisAgent->epmem_sym_prohibit = make_str_constant( thisAgent, "prohibit" );
  thisAgent->epmem_sym_yes = make_str_constant( thisAgent, "yes" );
  thisAgent->epmem_sym_no = make_str_constant( thisAgent, "no" );


  thisAgent->smem_sym = make_str_constant( thisAgent, "smem" );
  thisAgent->smem_sym_cmd = make_str_constant( thisAgent, "command" );
  thisAgent->smem_sym_result = make_str_constant( thisAgent, "result" );

  thisAgent->smem_sym_retrieved = make_str_constant( thisAgent, "retrieved" );
  thisAgent->smem_sym_status = make_str_constant( thisAgent, "status" );
  thisAgent->smem_sym_success = make_str_constant( thisAgent, "success" );
  thisAgent->smem_sym_failure = make_str_constant( thisAgent, "failure" );
  thisAgent->smem_sym_bad_cmd = make_str_constant( thisAgent, "bad-cmd" );

  thisAgent->smem_sym_retrieve = make_str_constant( thisAgent, "retrieve" );
  thisAgent->smem_sym_query = make_str_constant( thisAgent, "query" );
  thisAgent->smem_sym_negquery = make_str_constant( thisAgent, "neg-query" );
  thisAgent->smem_sym_prohibit = make_str_constant( thisAgent, "prohibit" );
  thisAgent->smem_sym_store = make_str_constant( thisAgent, "store" );
  thisAgent->smem_sym_math_query = make_str_constant( thisAgent, "math-query" );
  thisAgent->smem_sym_math_query_less = make_str_constant( thisAgent, "less" );
  thisAgent->smem_sym_math_query_greater = make_str_constant( thisAgent, "greater" );
  thisAgent->smem_sym_math_query_less_or_equal = make_str_constant( thisAgent, "less-or-equal" );
  thisAgent->smem_sym_math_query_greater_or_equal = make_str_constant( thisAgent, "greater-or-equal" );
  thisAgent->smem_sym_math_query_max = make_str_constant( thisAgent, "max" );
  thisAgent->smem_sym_math_query_min = make_str_constant( thisAgent, "min" );}

inline void release_helper(agent* thisAgent, Symbol** sym) {
  symbol_remove_ref(thisAgent,(*sym));
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
  release_helper(thisAgent,&(thisAgent->non_numeric_symbol));
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

  release_helper(thisAgent,&(thisAgent->item_count_symbol));
  release_helper(thisAgent,&(thisAgent->non_numeric_count_symbol));

  release_helper(thisAgent,&(thisAgent->input_link_symbol));
  release_helper(thisAgent,&(thisAgent->output_link_symbol));

  release_helper( thisAgent, &( thisAgent->rl_sym_reward_link ) );
  release_helper( thisAgent, &( thisAgent->rl_sym_reward ) );
  release_helper( thisAgent, &( thisAgent->rl_sym_value ) );

  release_helper( thisAgent, &( thisAgent->epmem_sym ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_cmd ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_result ) );

  release_helper( thisAgent, &( thisAgent->epmem_sym_retrieved ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_status ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_match_score ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_cue_size ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_normalized_match_score ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_match_cardinality ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_memory_id ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_present_id ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_no_memory ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_graph_match ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_graph_match_mapping ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_graph_match_mapping_node ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_graph_match_mapping_cue ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_success ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_failure ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_bad_cmd ) );

  release_helper( thisAgent, &( thisAgent->epmem_sym_retrieve ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_next ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_prev ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_query ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_negquery ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_before ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_after ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_prohibit ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_yes ) );
  release_helper( thisAgent, &( thisAgent->epmem_sym_no ) );

  release_helper( thisAgent, &( thisAgent->smem_sym ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_cmd ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_result ) );

  release_helper( thisAgent, &( thisAgent->smem_sym_retrieved ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_status ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_success ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_failure ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_bad_cmd ) );

  release_helper( thisAgent, &( thisAgent->smem_sym_retrieve ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_query ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_negquery ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_prohibit ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_store ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_less ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_greater ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_less_or_equal ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_greater_or_equal ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_max ) );
  release_helper( thisAgent, &( thisAgent->smem_sym_math_query_min ) );
}

char *Symbol::to_string (bool rereadable, char *dest, size_t dest_size) {

  bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
  bool is_rereadable;
  bool has_angle_bracket;

  /* -- Not sure if this is legit, but works and smooths debugging -- */
  if (!this)
  {
    //assert(false);
    return Output_Manager::Get_OM().NULL_SYM_STR;
  }

  switch(symbol_type) {
    case VARIABLE_SYMBOL_TYPE:
      if (!dest) return var->name;
      strncpy (dest, var->name, dest_size);
      dest[dest_size - 1] = 0; /* ensure null termination */
      return dest;

    case IDENTIFIER_SYMBOL_TYPE:
      if (!dest)
      {
        dest_size= MAX_LEXEME_LENGTH*2+10; /* from agent.h */;
        dest=Output_Manager::Get_OM().get_printed_output_string();
      }
      if (is_lti())
        SNPRINTF (dest, dest_size, "@%c%llu", id->name_letter, static_cast<long long unsigned>(id->name_number));
      else
        SNPRINTF (dest, dest_size, "%c%llu", id->name_letter, static_cast<long long unsigned>(id->name_number));
      dest[dest_size - 1] = 0; /* ensure null termination */
      return dest;

    case INT_CONSTANT_SYMBOL_TYPE:
      if (!dest)
      {
        dest_size= MAX_LEXEME_LENGTH*2+10; /* from agent.h */;
        dest=Output_Manager::Get_OM().get_printed_output_string();
      }
      SNPRINTF (dest, dest_size, "%ld", static_cast<long int>(ic->value));
      dest[dest_size - 1] = 0; /* ensure null termination */
      return dest;

    case FLOAT_CONSTANT_SYMBOL_TYPE:
      if (!dest)
      {
        dest_size= MAX_LEXEME_LENGTH*2+10; /* from agent.h */;
        dest=Output_Manager::Get_OM().get_printed_output_string();
      }
      SNPRINTF (dest, dest_size, "%#.16g", fc->value);
      dest[dest_size - 1] = 0; /* ensure null termination */
      /* MToDo | Is stripping off trailing zero's still necessary? -- */
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

    case STR_CONSTANT_SYMBOL_TYPE:
      if (!rereadable) {
        if (!dest) return sc->name;
        strncpy (dest, sc->name, dest_size);
        return dest;
      }

      determine_possible_symbol_types_for_string (sc->name, strlen (sc->name),
          &possible_id, &possible_var, &possible_sc, &possible_ic, &possible_fc, &is_rereadable);

      has_angle_bracket = sc->name[0] == '<' || sc->name[strlen(sc->name)-1] == '>';

      if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
          (!is_rereadable) || has_angle_bracket) {
        /* BUGBUG - if in context where id's could occur, should check possible_id flag here also
         *        - Shouldn't it also check whether dest char * was passed in and get a printed
         *          output string instead?  */
        return string_to_escaped_string (sc->name, '|', dest);
      }
      if (!dest) return sc->name;
      strncpy (dest, sc->name, dest_size);
      return dest;

    default:
    {
      char msg[BUFFER_MSG_SIZE];
      strncpy(msg, "Internal Soar Error:  symbol->to_string() called on bad symbol!\n", BUFFER_MSG_SIZE);
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error_noagent(msg);
      break;
    }
  }
  return NIL; /* unreachable, but without it, gcc -Wall warns here */
}

