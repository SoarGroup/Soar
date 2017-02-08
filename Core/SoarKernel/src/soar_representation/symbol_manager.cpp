/*
 * symbol_factory.cpp
 *
 *  Created on: Aug 19, 2016
 *      Author: mazzin
 */

#include "symbol_manager.h"

#include "agent.h"
#include "debug_inventories.h"
#include "dprint.h"
#include "mem.h"
#include "output_manager.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "soar_instance.h"
#include "smem_db.h"
#include "symbol.h"

Symbol_Manager::Symbol_Manager(agent* pAgent)
{
    thisAgent = pAgent;
    thisAgent->symbolManager = this;
    current_symbol_hash_id             = 0;
    current_variable_gensym_number     = 0;
    init_symbol_tables();
    create_predefined_symbols();
}

Symbol_Manager::~Symbol_Manager()
{
    free_hash_table(thisAgent, variable_hash_table);
    free_hash_table(thisAgent, identifier_hash_table);
    free_hash_table(thisAgent, str_constant_hash_table);
    free_hash_table(thisAgent, int_constant_hash_table);
    free_hash_table(thisAgent, float_constant_hash_table);
}

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

uint32_t compress(uint32_t h, short num_bits)
{
    uint32_t result;

    if (num_bits < 16)
    {
        h = (h & 0xFFFF) ^ (h >> 16);
    }
    if (num_bits < 8)
    {
        h = (h & 0xFF) ^ (h >> 8);
    }
    result = 0;
    while (h)
    {
        result ^= (h & masks_for_n_low_order_bits[num_bits]);
        h = h >> num_bits;
    }
    return result;
}

uint32_t hash_string(const char* s)      /* AGR 600 */
{
    uint32_t h;

    h = 0;
    while (*s != 0)
    {
        h = ((h << 8) | (h >> 24)) ^ (*s);
        s++;
    }
    return h;
}

/* -----------------------------------------
   Hashing symbols using their basic info
----------------------------------------- */

uint32_t hash_variable_raw_info(const char* name, short num_bits)
{
    return compress(hash_string(name), num_bits);
}

uint32_t hash_identifier_raw_info(char name_letter,
                                  uint64_t name_number,
                                  short num_bits)
{
    return compress(static_cast<uint32_t>(name_number) ^ (static_cast<uint32_t>(name_letter) << 24), num_bits);  // FIXME: cast from 64 to 32 bits
}

uint32_t hash_str_constant_raw_info(const char* name, short num_bits)
{
    return compress(hash_string(name), num_bits);
}

uint32_t hash_int_constant_raw_info(int64_t value, short num_bits)
{
    return compress(static_cast<uint32_t>(value), num_bits);
}

uint32_t hash_float_constant_raw_info(double value, short num_bits)
{
    return compress(static_cast<uint32_t>(value), num_bits);
}

/* ---------------------------------------------------
   Hashing symbols using their symbol table entries
--------------------------------------------------- */

uint32_t hash_variable(void* item, short num_bits)
{
    varSymbol* var;
    var = static_cast<varSymbol*>(item);
    return compress(hash_string(var->name), num_bits);
}

uint32_t hash_identifier(void* item, short num_bits)
{
    idSymbol* id;
    id = static_cast<idSymbol*>(item);
    return compress(static_cast<uint32_t>(id->name_number) ^ (static_cast<uint32_t>(id->name_letter) << 24), num_bits);  // FIXME: cast from 64 to 32 bits
}

uint32_t hash_str_constant(void* item, short num_bits)
{
    strSymbol* sc;
    sc = static_cast<strSymbol*>(item);
    return compress(hash_string(sc->name), num_bits);
}

uint32_t hash_int_constant(void* item, short num_bits)
{
    intSymbol* ic;
    ic = static_cast<intSymbol*>(item);
    return compress(static_cast<uint32_t>(ic->value), num_bits);
}

uint32_t hash_float_constant(void* item, short num_bits)
{
    floatSymbol* fc;
    fc = static_cast<floatSymbol*>(item);
    return compress(static_cast<uint32_t>(fc->value), num_bits);
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

void Symbol_Manager::init_symbol_tables()
{
    variable_hash_table = make_hash_table(thisAgent, 0, hash_variable);
    identifier_hash_table = make_hash_table(thisAgent, 0, hash_identifier);
    str_constant_hash_table = make_hash_table(thisAgent, 0, hash_str_constant);
    int_constant_hash_table = make_hash_table(thisAgent, 0, hash_int_constant);
    float_constant_hash_table = make_hash_table(thisAgent, 0, hash_float_constant);

    thisAgent->memoryManager->init_memory_pool(MP_variable, sizeof(varSymbol), "variable");
    thisAgent->memoryManager->init_memory_pool(MP_identifier, sizeof(idSymbol), "identifier");
    thisAgent->memoryManager->init_memory_pool(MP_str_constant, sizeof(strSymbol), "str constant");
    thisAgent->memoryManager->init_memory_pool(MP_int_constant, sizeof(intSymbol), "int constant");
    thisAgent->memoryManager->init_memory_pool(MP_float_constant, sizeof(floatSymbol), "float constant");

    reset_id_counters();
}

bool retesave_symbol_and_assign_index(agent* thisAgent, void* item, void* userdata)
{
    Symbol* sym;
    FILE* f = reinterpret_cast<FILE*>(userdata);

    sym = static_cast<symbol_struct*>(item);
    thisAgent->current_retesave_symindex++;
    sym->retesave_symindex = thisAgent->current_retesave_symindex;
    retesave_string(sym->to_string(), f);
    return false;
}


void Symbol_Manager::retesave(FILE* f)
{
    thisAgent->current_retesave_symindex = 0;

    retesave_eight_bytes(str_constant_hash_table->count, f);
    retesave_eight_bytes(variable_hash_table->count, f);
    retesave_eight_bytes(int_constant_hash_table->count, f);
    retesave_eight_bytes(float_constant_hash_table->count, f);

    do_for_all_items_in_hash_table(thisAgent, str_constant_hash_table, retesave_symbol_and_assign_index, f);
    do_for_all_items_in_hash_table(thisAgent, variable_hash_table, retesave_symbol_and_assign_index, f);
    do_for_all_items_in_hash_table(thisAgent, int_constant_hash_table, retesave_symbol_and_assign_index, f);
    do_for_all_items_in_hash_table(thisAgent, float_constant_hash_table, retesave_symbol_and_assign_index, f);
}
Symbol* Symbol_Manager::find_variable(const char* name)
{
    uint32_t hash_value;
    varSymbol* sym;

    hash_value = hash_variable_raw_info(name, variable_hash_table->log2size);
    sym = reinterpret_cast<varSymbol*>(*(variable_hash_table->buckets + hash_value));
    for (; sym != NIL; sym = static_cast<varSymbol*>(sym->next_in_hash_table))
    {
        if (!strcmp(sym->name, name))
        {
            return sym;
        }
    }
    return NIL;
}

Symbol* Symbol_Manager::find_identifier(char name_letter, uint64_t name_number)
{
    uint32_t hash_value;
    idSymbol* sym;

    hash_value = hash_identifier_raw_info(name_letter, name_number,
                                          identifier_hash_table->log2size);
    sym = reinterpret_cast<idSymbol*>(*(identifier_hash_table->buckets + hash_value));
    for (; sym != NIL; sym = static_cast<idSymbol*>(sym->next_in_hash_table))
    {
        if ((name_letter == sym->name_letter) &&
                (name_number == sym->name_number))
        {
            return sym;
        }
    }
    return NIL;
}

Symbol* Symbol_Manager::find_str_constant(const char* name)
{
    uint32_t hash_value;
    strSymbol* sym;

    hash_value = hash_str_constant_raw_info(name,
                                            str_constant_hash_table->log2size);
    sym = reinterpret_cast<strSymbol*>(*(str_constant_hash_table->buckets + hash_value));
    for (; sym != NIL; sym = static_cast<strSymbol*>(sym->next_in_hash_table))
    {
        if (!strcmp(sym->name, name))
        {
            return sym;
        }
    }
    return NIL;
}

Symbol* Symbol_Manager::find_int_constant(int64_t value)
{
    uint32_t hash_value;
    intSymbol* sym;

    hash_value = hash_int_constant_raw_info(value,
                                            int_constant_hash_table->log2size);
    sym = reinterpret_cast<intSymbol*>(*(int_constant_hash_table->buckets + hash_value));
    for (; sym != NIL; sym = static_cast<intSymbol*>(sym->next_in_hash_table))
    {
        if (value == sym->value)
        {
            return sym;
        }
    }
    return NIL;
}

Symbol* Symbol_Manager::find_float_constant(double value)
{
    uint32_t hash_value;
    floatSymbol* sym;

    hash_value = hash_float_constant_raw_info(value,
                 float_constant_hash_table->log2size);
    sym = reinterpret_cast<floatSymbol*>(*(float_constant_hash_table->buckets + hash_value));
    for (; sym != NIL; sym = static_cast<floatSymbol*>(sym->next_in_hash_table))
    {
        if (value == sym->value)
        {
            return sym;
        }
    }
    return NIL;
}

Symbol* Symbol_Manager::make_variable(const char* name)
{

    varSymbol* sym;

    sym = static_cast<varSymbol*>(find_variable(name));
    if (sym)
    {
        symbol_add_ref(sym);
        return sym;
    }

    thisAgent->memoryManager->allocate_with_pool(MP_variable, &sym);
    sym->symbol_type = VARIABLE_SYMBOL_TYPE;
    sym->reference_count = 0;
    sym->hash_id = get_next_symbol_hash_id(thisAgent);
    sym->tc_num = 0;
    sym->name = make_memory_block_for_string(thisAgent, name);
    sym->gensym_number = 0;
    sym->current_binding_value = NULL;
    sym->instantiated_sym = NULL;
    sym->rete_binding_locations = NULL;

    sym->fc = NULL;
    sym->ic = NULL;
    sym->sc = NULL;
    sym->id = NULL;
    sym->var = sym;
    symbol_add_ref(sym);
    add_to_hash_table(thisAgent, variable_hash_table, sym);

    return sym;
}

Symbol* Symbol_Manager::make_new_identifier(char name_letter, goal_stack_level level, uint64_t name_number, bool prohibit_S)
{

    idSymbol* sym;
    if (isalpha(name_letter))
    {
        if (islower(name_letter))
        {
            name_letter = static_cast<char>(toupper(name_letter));
        }
        if (prohibit_S && (name_letter == 'S'))
        {
            name_letter = 'I';
        }
    }
    else
    {
        name_letter = 'I';
    }
    thisAgent->memoryManager->allocate_with_pool(MP_identifier, &sym);
    sym->symbol_type = IDENTIFIER_SYMBOL_TYPE;
    sym->reference_count = 0;
    sym->hash_id = get_next_symbol_hash_id(thisAgent);
    sym->tc_num = 0;
    sym->thisAgent = thisAgent;
    sym->cached_print_str = NULL;
    sym->name_letter = name_letter;

    if (name_number == NIL)
    {
        name_number = id_counter[name_letter - 'A']++;
    }
    else
    {
        uint64_t* current_number = &(id_counter[ name_letter - 'A' ]);
        if (name_number >= (*current_number))
        {
            (*current_number) = (name_number + 1);
        }
    }
    sym->name_number = name_number;
    sym->level = level;
    sym->promotion_level = level;
    sym->slots = NULL;
    sym->isa_goal = false;
    sym->isa_impasse = false;
    sym->isa_operator = 0;
    sym->link_count = 0;
    sym->unknown_level = NIL;
    sym->could_be_a_link_from_below = false;
    sym->impasse_wmes = NULL;
    sym->higher_goal = NULL;
    sym->gds = NULL;
    sym->saved_firing_type = NO_SAVED_PRODS;
    sym->ms_o_assertions = NULL;
    sym->ms_i_assertions = NULL;
    sym->ms_retractions = NULL;
    sym->lower_goal = NULL;
    sym->operator_slot = NULL;
    sym->preferences_from_goal = NULL;
    sym->associated_output_links = NULL;
    sym->input_wmes = NULL;

    sym->rl_info = NULL;

    sym->epmem_info = NULL;
    sym->epmem_id = EPMEM_NODEID_BAD;
    sym->epmem_valid = NIL;

    sym->smem_info = NULL;
    sym->LTI_ID = NIL;
    sym->LTI_epmem_valid = NIL;
    sym->smem_valid = NIL;

    sym->rl_trace = NULL;

    sym->fc = NULL;
    sym->ic = NULL;
    sym->sc = NULL;
    sym->var = NULL;
    sym->id = sym;
    symbol_add_ref(sym);
    add_to_hash_table(thisAgent, identifier_hash_table, sym);

    return sym;
}

Symbol* Symbol_Manager::make_str_constant(char const* name)
{
    strSymbol* sym;
    sym = static_cast<strSymbol*>(find_str_constant(name));
    if (sym)
    {
        symbol_add_ref(sym);
    }
    else
    {
        thisAgent->memoryManager->allocate_with_pool(MP_str_constant, &sym);
        sym->symbol_type = STR_CONSTANT_SYMBOL_TYPE;
        sym->reference_count = 0;
        sym->hash_id = get_next_symbol_hash_id(thisAgent);
        sym->tc_num = 0;
        sym->singleton.possible = false;
        sym->epmem_hash = 0;
        sym->epmem_valid = 0;
        sym->smem_hash = 0;
        sym->smem_valid = 0;
        sym->name = make_memory_block_for_string(thisAgent, name);
        sym->thisAgent = thisAgent;
        sym->cached_print_str = NULL;
        sym->production = NULL;
        sym->fc = NULL;
        sym->ic = NULL;
        sym->id = NULL;
        sym->var = NULL;
        sym->sc = sym;
        symbol_add_ref(sym);
        add_to_hash_table(thisAgent, str_constant_hash_table, sym);
    }
    return sym;
}

Symbol* Symbol_Manager::make_int_constant(int64_t value)
{
    intSymbol* sym;

    sym = static_cast<intSymbol*>(find_int_constant(value));
    if (sym)
    {
        symbol_add_ref(sym);
    }
    else
    {
        thisAgent->memoryManager->allocate_with_pool(MP_int_constant, &sym);
        sym->symbol_type = INT_CONSTANT_SYMBOL_TYPE;
        sym->reference_count = 0;
        sym->hash_id = get_next_symbol_hash_id(thisAgent);
        sym->tc_num = 0;
        sym->epmem_hash = 0;
        sym->epmem_valid = 0;
        sym->smem_hash = 0;
        sym->smem_valid = 0;
        sym->value = value;
        sym->thisAgent = thisAgent;
        sym->cached_print_str = NULL;
        sym->fc = NULL;
        sym->sc = NULL;
        sym->id = NULL;
        sym->var = NULL;
        sym->ic = sym;
        symbol_add_ref(sym);
        add_to_hash_table(thisAgent, int_constant_hash_table, sym);
    }
    return sym;
}

Symbol* Symbol_Manager::make_float_constant(double value)
{
    floatSymbol* sym;

    sym = static_cast<floatSymbol*>(find_float_constant(value));
    if (sym)
    {
        symbol_add_ref(sym);
    }
    else
    {
        thisAgent->memoryManager->allocate_with_pool(MP_float_constant, &sym);
        sym->symbol_type = FLOAT_CONSTANT_SYMBOL_TYPE;
        sym->reference_count = 0;
        sym->hash_id = get_next_symbol_hash_id(thisAgent);
        sym->tc_num = 0;
        sym->epmem_hash = 0;
        sym->epmem_valid = 0;
        sym->smem_hash = 0;
        sym->smem_valid = 0;
        sym->value = value;
        sym->thisAgent = thisAgent;
        sym->cached_print_str = NULL;
        sym->ic = NULL;
        sym->sc = NULL;
        sym->id = NULL;
        sym->var = NULL;
        sym->fc = sym;
        symbol_add_ref(sym);
        add_to_hash_table(thisAgent, float_constant_hash_table, sym);
    }
    return sym;
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

void Symbol_Manager::create_predefined_symbols()
{
    soarSymbols.crlf_symbol = make_str_constant("\n");
    soarSymbols.at_symbol = make_str_constant("@");
    soarSymbols.problem_space_symbol = make_str_constant("problem-space");
    soarSymbols.state_symbol = make_str_constant("state");
    soarSymbols.operator_symbol = make_str_constant("operator");
    soarSymbols.superstate_symbol = make_str_constant("superstate");
    soarSymbols.io_symbol = make_str_constant("io");
    soarSymbols.object_symbol = make_str_constant("object");
    soarSymbols.attribute_symbol = make_str_constant("attribute");
    soarSymbols.impasse_symbol = make_str_constant("impasse");
    soarSymbols.choices_symbol = make_str_constant("choices");
    soarSymbols.none_symbol = make_str_constant("none");
    soarSymbols.constraint_failure_symbol = make_str_constant("constraint-failure");
    soarSymbols.no_change_symbol = make_str_constant("no-change");
    soarSymbols.multiple_symbol = make_str_constant("multiple");

    // SBW 5/07
    soarSymbols.item_count_symbol = make_str_constant("item-count");

    // NLD 11/11
    soarSymbols.non_numeric_count_symbol = make_str_constant("non-numeric-count");

    soarSymbols.conflict_symbol = make_str_constant("conflict");
    soarSymbols.tie_symbol = make_str_constant("tie");
    soarSymbols.item_symbol = make_str_constant("item");
    soarSymbols.non_numeric_symbol = make_str_constant("non-numeric");
    soarSymbols.quiescence_symbol = make_str_constant("quiescence");
    soarSymbols.t_symbol = make_str_constant("t");
    soarSymbols.nil_symbol = make_str_constant("nil");
    soarSymbols.type_symbol = make_str_constant("type");
    soarSymbols.goal_symbol = make_str_constant("goal");
    soarSymbols.name_symbol = make_str_constant("name");

    soarSymbols.ts_context_variable = make_variable("<ts>");
    soarSymbols.to_context_variable = make_variable("<to>");
    soarSymbols.sss_context_variable = make_variable("<sss>");
    soarSymbols.sso_context_variable = make_variable("<sso>");
    soarSymbols.ss_context_variable = make_variable("<ss>");
    soarSymbols.so_context_variable = make_variable("<so>");
    soarSymbols.s_context_variable = make_variable("<s>");
    soarSymbols.o_context_variable = make_variable("<o>");

    soarSymbols.fake_instantiation_symbol = make_str_constant("Memory System Recall");
    soarSymbols.architecture_inst_symbol = make_str_constant("Architecture");
    soarSymbols.sti_symbol = make_str_constant("[STI]");

    soarSymbols.input_link_symbol = make_str_constant("input-link");
    soarSymbols.output_link_symbol = make_str_constant("output-link");

    soarSymbols.rl_sym_reward_link = make_str_constant("reward-link");
    soarSymbols.rl_sym_reward = make_str_constant("reward");
    soarSymbols.rl_sym_value = make_str_constant("value");

    soarSymbols.epmem_sym = make_str_constant("epmem");
    soarSymbols.epmem_sym_cmd = make_str_constant("command");
    soarSymbols.epmem_sym_result = make_str_constant("result");

    soarSymbols.epmem_sym_retrieved = make_str_constant("retrieved");
    soarSymbols.epmem_sym_status = make_str_constant("status");
    soarSymbols.epmem_sym_match_score = make_str_constant("match-score");
    soarSymbols.epmem_sym_cue_size = make_str_constant("cue-size");
    soarSymbols.epmem_sym_normalized_match_score = make_str_constant("normalized-match-score");
    soarSymbols.epmem_sym_match_cardinality = make_str_constant("match-cardinality");
    soarSymbols.epmem_sym_memory_id = make_str_constant("memory-id");
    soarSymbols.epmem_sym_present_id = make_str_constant("present-id");
    soarSymbols.epmem_sym_no_memory = make_str_constant("no-memory");
    soarSymbols.epmem_sym_graph_match = make_str_constant("graph-match");
    soarSymbols.epmem_sym_graph_match_mapping = make_str_constant("mapping");
    soarSymbols.epmem_sym_graph_match_mapping_node = make_str_constant("node");
    soarSymbols.epmem_sym_graph_match_mapping_cue = make_str_constant("cue");
    soarSymbols.epmem_sym_success = make_str_constant("success");
    soarSymbols.epmem_sym_failure = make_str_constant("failure");
    soarSymbols.epmem_sym_bad_cmd = make_str_constant("bad-cmd");

    soarSymbols.epmem_sym_retrieve = make_str_constant("retrieve");
    soarSymbols.epmem_sym_next = make_str_constant("next");
    soarSymbols.epmem_sym_prev = make_str_constant("previous");
    soarSymbols.epmem_sym_query = make_str_constant("query");
    soarSymbols.epmem_sym_negquery = make_str_constant("neg-query");
    soarSymbols.epmem_sym_before = make_str_constant("before");
    soarSymbols.epmem_sym_after = make_str_constant("after");
    soarSymbols.epmem_sym_prohibit = make_str_constant("prohibit");
    soarSymbols.yes = make_str_constant("yes");
    soarSymbols.no = make_str_constant("no");


    soarSymbols.smem_sym = make_str_constant("smem");
    soarSymbols.smem_sym_cmd = make_str_constant("command");
    soarSymbols.smem_sym_result = make_str_constant("result");

    soarSymbols.smem_sym_retrieved = make_str_constant("retrieved");
    soarSymbols.smem_sym_depth_retrieved = make_str_constant("depth-retrieved");
    soarSymbols.smem_sym_status = make_str_constant("status");
    soarSymbols.smem_sym_success = make_str_constant("success");
    soarSymbols.smem_sym_failure = make_str_constant("failure");
    soarSymbols.smem_sym_bad_cmd = make_str_constant("bad-cmd");
    soarSymbols.smem_sym_depth = make_str_constant("depth");
    soarSymbols.smem_sym_store_new = make_str_constant("store-new");
    soarSymbols.smem_sym_overwrite = make_str_constant("link-to-new-ltm");
    soarSymbols.smem_sym_link_to_ltm = make_str_constant("link-to-ltm");
    soarSymbols.smem_sym_retrieve = make_str_constant("retrieve");
    soarSymbols.smem_sym_query = make_str_constant("query");
    soarSymbols.smem_sym_negquery = make_str_constant("neg-query");
    soarSymbols.smem_sym_prohibit = make_str_constant("prohibit");
    soarSymbols.smem_sym_store = make_str_constant("store");
    soarSymbols.smem_sym_math_query = make_str_constant("math-query");
    soarSymbols.smem_sym_math_query_less = make_str_constant("less");
    soarSymbols.smem_sym_math_query_greater = make_str_constant("greater");
    soarSymbols.smem_sym_math_query_less_or_equal = make_str_constant("less-or-equal");
    soarSymbols.smem_sym_math_query_greater_or_equal = make_str_constant("greater-or-equal");
    soarSymbols.smem_sym_math_query_max = make_str_constant("max");
    soarSymbols.smem_sym_math_query_min = make_str_constant("min");
}

void Symbol_Manager::release_predefined_symbols()
{
    symbol_remove_ref(&(soarSymbols.crlf_symbol));
    symbol_remove_ref(&(soarSymbols.at_symbol));
    symbol_remove_ref(&(soarSymbols.problem_space_symbol));
    symbol_remove_ref(&(soarSymbols.state_symbol));
    symbol_remove_ref(&(soarSymbols.operator_symbol));
    symbol_remove_ref(&(soarSymbols.superstate_symbol));
    symbol_remove_ref(&(soarSymbols.io_symbol));
    symbol_remove_ref(&(soarSymbols.object_symbol));
    symbol_remove_ref(&(soarSymbols.attribute_symbol));
    symbol_remove_ref(&(soarSymbols.impasse_symbol));
    symbol_remove_ref(&(soarSymbols.choices_symbol));
    symbol_remove_ref(&(soarSymbols.none_symbol));
    symbol_remove_ref(&(soarSymbols.constraint_failure_symbol));
    symbol_remove_ref(&(soarSymbols.no_change_symbol));
    symbol_remove_ref(&(soarSymbols.multiple_symbol));
    symbol_remove_ref(&(soarSymbols.conflict_symbol));
    symbol_remove_ref(&(soarSymbols.tie_symbol));
    symbol_remove_ref(&(soarSymbols.item_symbol));
    symbol_remove_ref(&(soarSymbols.non_numeric_symbol));
    symbol_remove_ref(&(soarSymbols.quiescence_symbol));
    symbol_remove_ref(&(soarSymbols.t_symbol));
    symbol_remove_ref(&(soarSymbols.nil_symbol));
    symbol_remove_ref(&(soarSymbols.type_symbol));
    symbol_remove_ref(&(soarSymbols.goal_symbol));
    symbol_remove_ref(&(soarSymbols.name_symbol));

    symbol_remove_ref(&(soarSymbols.ts_context_variable));
    symbol_remove_ref(&(soarSymbols.to_context_variable));
    symbol_remove_ref(&(soarSymbols.sss_context_variable));
    symbol_remove_ref(&(soarSymbols.sso_context_variable));
    symbol_remove_ref(&(soarSymbols.ss_context_variable));
    symbol_remove_ref(&(soarSymbols.so_context_variable));
    symbol_remove_ref(&(soarSymbols.s_context_variable));
    symbol_remove_ref(&(soarSymbols.o_context_variable));

    symbol_remove_ref(&(soarSymbols.item_count_symbol));
    symbol_remove_ref(&(soarSymbols.non_numeric_count_symbol));

    symbol_remove_ref(&(soarSymbols.fake_instantiation_symbol));
    symbol_remove_ref(&(soarSymbols.architecture_inst_symbol));
    symbol_remove_ref(&(soarSymbols.sti_symbol));

    symbol_remove_ref(&(soarSymbols.input_link_symbol));
    symbol_remove_ref(&(soarSymbols.output_link_symbol));

    symbol_remove_ref(&(soarSymbols.rl_sym_reward_link));
    symbol_remove_ref(&(soarSymbols.rl_sym_reward));
    symbol_remove_ref(&(soarSymbols.rl_sym_value));

    symbol_remove_ref(&(soarSymbols.epmem_sym));
    symbol_remove_ref(&(soarSymbols.epmem_sym_cmd));
    symbol_remove_ref(&(soarSymbols.epmem_sym_result));

    symbol_remove_ref(&(soarSymbols.epmem_sym_retrieved));
    symbol_remove_ref(&(soarSymbols.epmem_sym_status));
    symbol_remove_ref(&(soarSymbols.epmem_sym_match_score));
    symbol_remove_ref(&(soarSymbols.epmem_sym_cue_size));
    symbol_remove_ref(&(soarSymbols.epmem_sym_normalized_match_score));
    symbol_remove_ref(&(soarSymbols.epmem_sym_match_cardinality));
    symbol_remove_ref(&(soarSymbols.epmem_sym_memory_id));
    symbol_remove_ref(&(soarSymbols.epmem_sym_present_id));
    symbol_remove_ref(&(soarSymbols.epmem_sym_no_memory));
    symbol_remove_ref(&(soarSymbols.epmem_sym_graph_match));
    symbol_remove_ref(&(soarSymbols.epmem_sym_graph_match_mapping));
    symbol_remove_ref(&(soarSymbols.epmem_sym_graph_match_mapping_node));
    symbol_remove_ref(&(soarSymbols.epmem_sym_graph_match_mapping_cue));
    symbol_remove_ref(&(soarSymbols.epmem_sym_success));
    symbol_remove_ref(&(soarSymbols.epmem_sym_failure));
    symbol_remove_ref(&(soarSymbols.epmem_sym_bad_cmd));

    symbol_remove_ref(&(soarSymbols.epmem_sym_retrieve));
    symbol_remove_ref(&(soarSymbols.epmem_sym_next));
    symbol_remove_ref(&(soarSymbols.epmem_sym_prev));
    symbol_remove_ref(&(soarSymbols.epmem_sym_query));
    symbol_remove_ref(&(soarSymbols.epmem_sym_negquery));
    symbol_remove_ref(&(soarSymbols.epmem_sym_before));
    symbol_remove_ref(&(soarSymbols.epmem_sym_after));
    symbol_remove_ref(&(soarSymbols.epmem_sym_prohibit));
    symbol_remove_ref(&(soarSymbols.yes));
    symbol_remove_ref(&(soarSymbols.no));

    symbol_remove_ref(&(soarSymbols.smem_sym));
    symbol_remove_ref(&(soarSymbols.smem_sym_cmd));
    symbol_remove_ref(&(soarSymbols.smem_sym_result));

    symbol_remove_ref(&(soarSymbols.smem_sym_retrieved));
    symbol_remove_ref(&(soarSymbols.smem_sym_depth_retrieved));
    symbol_remove_ref(&(soarSymbols.smem_sym_status));
    symbol_remove_ref(&(soarSymbols.smem_sym_success));
    symbol_remove_ref(&(soarSymbols.smem_sym_failure));
    symbol_remove_ref(&(soarSymbols.smem_sym_bad_cmd));
    symbol_remove_ref(&(soarSymbols.smem_sym_depth));
    symbol_remove_ref(&(soarSymbols.smem_sym_store_new));
    symbol_remove_ref(&(soarSymbols.smem_sym_overwrite));
    symbol_remove_ref(&(soarSymbols.smem_sym_link_to_ltm));

    symbol_remove_ref(&(soarSymbols.smem_sym_retrieve));
    symbol_remove_ref(&(soarSymbols.smem_sym_query));
    symbol_remove_ref(&(soarSymbols.smem_sym_negquery));
    symbol_remove_ref(&(soarSymbols.smem_sym_prohibit));
    symbol_remove_ref(&(soarSymbols.smem_sym_store));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_less));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_greater));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_less_or_equal));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_greater_or_equal));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_max));
    symbol_remove_ref(&(soarSymbols.smem_sym_math_query_min));
}

/* -------------------------------------------------------------------

                         Deallocate Symbol

------------------------------------------------------------------- */

void Symbol_Manager::deallocate_symbol(Symbol*& sym)
{

    #ifdef DEBUG_TRACE_REFCOUNT_FOR
        std::string strName(sym->to_string());
        if (strName == DEBUG_TRACE_REFCOUNT_FOR)
        {
            std::string caller_string = get_stacktrace("dea_sym");
//            dprint(DT_ID_LEAKING, "-- | %s(%u) | %s++\n", strName.c_str(), sym->reference_count, caller_string.c_str());
            if (is_DT_mode_enabled(DT_ID_LEAKING))
            {
                std::cout << "DA | " << strName.c_str() << " |" << sym->reference_count << " | " << caller_string.c_str() << "\n";
            }
        }
    #else
        dprint(DT_DEALLOCATE_SYMBOL, "DEALLOCATE symbol %y\n", sym);
//        std::string caller_string = get_stacktrace("dea_sym");
//            dprint(DT_ID_LEAKING, "-- | %s(%u) | %s++\n", strName.c_str(), sym->reference_count, caller_string.c_str());
    #endif
    switch (sym->symbol_type)
    {
        case VARIABLE_SYMBOL_TYPE:
            remove_from_hash_table(thisAgent, variable_hash_table, sym);
            free_memory_block_for_string(thisAgent, sym->var->name);
            thisAgent->memoryManager->free_with_pool(MP_variable, sym);
            break;
        case IDENTIFIER_SYMBOL_TYPE:
            if (sym->id->cached_print_str) free_memory_block_for_string(thisAgent, sym->id->cached_print_str);
            remove_from_hash_table(thisAgent, identifier_hash_table, sym);
            thisAgent->memoryManager->free_with_pool(MP_identifier, sym);
            break;
        case STR_CONSTANT_SYMBOL_TYPE:
            if (sym->sc->cached_print_str) free_memory_block_for_string(thisAgent, sym->sc->cached_print_str);
            remove_from_hash_table(thisAgent, str_constant_hash_table, sym);
            free_memory_block_for_string(thisAgent, sym->sc->name);
            thisAgent->memoryManager->free_with_pool(MP_str_constant, sym);
            break;
        case INT_CONSTANT_SYMBOL_TYPE:
            if (sym->ic->cached_print_str) free_memory_block_for_string(thisAgent, sym->ic->cached_print_str);
            remove_from_hash_table(thisAgent, int_constant_hash_table, sym);
            thisAgent->memoryManager->free_with_pool(MP_int_constant, sym);
            break;
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            if (sym->fc->cached_print_str) free_memory_block_for_string(thisAgent, sym->fc->cached_print_str);
            remove_from_hash_table(thisAgent, float_constant_hash_table, sym);
            thisAgent->memoryManager->free_with_pool(MP_float_constant, sym);
            break;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "Internal error: called deallocate_symbol on non-symbol.\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
    }
    sym = NULL;
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

bool print_identifier_ref_info(agent* thisAgent, void* item, void* userdata)
{
    Symbol* sym;
    char msg[256];
    /* ensure null termination */
    msg[0] = 0;
    msg[255] = 0;
    sym = static_cast<symbol_struct*>(item);
    FILE* f = reinterpret_cast<FILE*>(userdata);

    if (sym->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        if (sym->reference_count > 0)
        {
                SNPRINTF(msg, 256,
                    "\t%c%llu --> %llu\n",
                    sym->id->name_letter,
                    static_cast<long long unsigned>(sym->id->name_number),
                    static_cast<long long unsigned>(sym->reference_count));
                thisAgent->outputManager->printa_sf(thisAgent,  msg);
            //                xml_generate_warning(thisAgent, msg);

                if (f)
                {
                    fprintf(f, "%s", msg) ;
                }
            }
        }
    else
    {
            thisAgent->outputManager->printa_sf(thisAgent,  "\tERROR: HASHTABLE ITEM IS NOT AN IDENTIFIER!\n");
            return true;
        }
        return false;
}

bool clear_gensym_number(agent* /*thisAgent*/, void* item, void*)
{
    Symbol* sym;

    sym = static_cast<symbol_struct*>(item);
    sym->var->gensym_number = 0;
    return false;
}

bool print_sym(agent* thisAgent, void* item, void*)
{
    thisAgent->outputManager->printa_sf(thisAgent,  "%s (%u)\n", static_cast<symbol_struct*>(item)->to_string(), static_cast<symbol_struct*>(item)->reference_count);
    return false;
}

void Symbol_Manager::clear_variable_gensym_numbers()
{
    do_for_all_items_in_hash_table(thisAgent, variable_hash_table, clear_gensym_number, 0);
}

void Symbol_Manager::print_internal_symbols()
{
    thisAgent->outputManager->printa_sf(thisAgent,  "\n--- Symbolic Constants: ---\n");
    do_for_all_items_in_hash_table(thisAgent, str_constant_hash_table, print_sym, 0);
    thisAgent->outputManager->printa_sf(thisAgent,  "\n--- Integer Constants: ---\n");
    do_for_all_items_in_hash_table(thisAgent, int_constant_hash_table, print_sym, 0);
    thisAgent->outputManager->printa_sf(thisAgent,  "\n--- Floating-Point Constants: ---\n");
    do_for_all_items_in_hash_table(thisAgent, float_constant_hash_table, print_sym, 0);
    thisAgent->outputManager->printa_sf(thisAgent,  "\n--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(thisAgent, identifier_hash_table, print_sym, 0);
    thisAgent->outputManager->printa_sf(thisAgent,  "\n--- Variables: ---\n");
    do_for_all_items_in_hash_table(thisAgent, variable_hash_table, print_sym, 0);
}

void Symbol_Manager::reset_hash_table(MemoryPoolType lHashTable)
{
    if (lHashTable == MP_identifier)
    {
        if (identifier_hash_table->count != 0)
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
            {
                /* If you #define CONFIGURE_SOAR_FOR_UNIT_TESTS and INIT_AFTER_RUN unit_tests.h, the following
                 * detect refcount leaks in unit tests and print out a message accordingly */
                /* Note:  The do_for_all_items_in_hash_table printing could cause a crash if there's
                 *        memory corruption, but usually prints out and is good for debugging. */
                #ifndef SOAR_RELEASE_VERSION
                    if (identifier_hash_table->count < 23)
                        do_for_all_items_in_hash_table(thisAgent, identifier_hash_table, print_sym, 0);
                    else
                        std::cout << "Refcount leak of " << identifier_hash_table->count << " identifiers detected. ";
                #else
                    std::cout << "Refcount leak of " << identifier_hash_table->count << " identifiers detected. ";
                #endif
            }
            else if (thisAgent->outputManager->settings[OM_WARNINGS])
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%d identifiers still exist.  Forcing deletion.\n", identifier_hash_table->count);
                do_for_all_items_in_hash_table(thisAgent, identifier_hash_table, print_sym, 0);
            }
            free_hash_table(thisAgent, identifier_hash_table);
            thisAgent->memoryManager->free_memory_pool(MP_identifier);
            identifier_hash_table = make_hash_table(thisAgent, 0, hash_identifier);
        }
    }
}

void Symbol_Manager::reset_id_counters()
{
    for (int i = 0; i < 26; i++) id_counter[i] = 1;
}

bool reset_tc_num(agent* /*thisAgent*/, void* item, void*)
{
    Symbol* sym;

    sym = static_cast<symbol_struct*>(item);
    sym->tc_num = 0;
    return false;
}

void Symbol_Manager::reset_id_and_variable_tc_numbers()
{
    do_for_all_items_in_hash_table(thisAgent, identifier_hash_table, reset_tc_num, 0);
    do_for_all_items_in_hash_table(thisAgent, variable_hash_table, reset_tc_num, 0);
}

Symbol* Symbol_Manager::generate_new_str_constant(const char* prefix, uint64_t* counter)
{

#define GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE 2000 /* that ought to be long enough! */
    char name[GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE];
    Symbol* New;

    while (true)
    {
        SNPRINTF(name, GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE, "%s%lu", prefix, static_cast<long unsigned int>((*counter)++));
        name[GENERATE_NEW_STR_CONSTANT_BUFFER_SIZE - 1] = 0;
        if (! find_str_constant(name))
        {
            break;
        }
    }
    New = make_str_constant(name);
    return New;
}

/* *********************************************************************

                         Variable Generator

   These routines are used for generating new variables.  The variables
   aren't necessarily "completely" new--they might occur in some existing
   production.  But we usually need to make sure the new variables don't
   overlap with those already used in a *certain* production--for instance,
   when variablizing a chunk, we don't want to introduce a new variable that
   conincides with the name of a variable already in an NCC in the chunk.

   To use these routines, first call reset_variable_generator(), giving
   it lists of conditions and actions whose variables should not be
   used.  Then call generate_new_variable() any number of times; each
   time, you give it a string to use as the prefix for the new variable's
   name.  The prefix string should not include the opening "<".
********************************************************************* */


void Symbol_Manager::reset_variable_generator(condition* conds_with_vars_to_avoid, action* actions_with_vars_to_avoid)
{
    tc_number tc;
    cons* var_list;
    cons* c;
    int i;

    /* --- reset counts, and increment the gensym number --- */
    for (i = 0; i < 26; i++)
    {
        gensymed_variable_count[i] = 1;
    }
    current_variable_gensym_number++;
    if (current_variable_gensym_number == 0)
    {
        clear_variable_gensym_numbers();
        current_variable_gensym_number = 1;
    }

    /* --- mark all variables in the given conds and actions --- */
    tc = get_new_tc_number(thisAgent);
    var_list = NIL;
    add_all_variables_in_condition_list(thisAgent, conds_with_vars_to_avoid, tc, &var_list);
    add_all_variables_in_action_list(thisAgent, actions_with_vars_to_avoid, tc, &var_list);
    for (c = var_list; c != NIL; c = c->rest)
    {
        static_cast<Symbol*>(c->first)->var->gensym_number = current_variable_gensym_number;
    }
    free_list(thisAgent, var_list);
}

Symbol* Symbol_Manager::generate_new_variable(const char* prefix)
{
#define GENERATE_NEW_VARIABLE_BUFFER_SIZE 200 /* that ought to be long enough! */
    char name[GENERATE_NEW_VARIABLE_BUFFER_SIZE];
    Symbol* New;
    char first_letter;

    first_letter = *prefix;
    if (isalpha(first_letter))
    {
        if (isupper(first_letter))
        {
            first_letter = static_cast<char>(tolower(first_letter));
        }
    }
    else
    {
        first_letter = 'v';
    }

    while (true)
    {
        SNPRINTF(name, GENERATE_NEW_VARIABLE_BUFFER_SIZE, "<%s%lu>", prefix,
                 static_cast<long unsigned int>(gensymed_variable_count[first_letter - 'a']++));
        name[GENERATE_NEW_VARIABLE_BUFFER_SIZE - 1] = 0; /* ensure null termination */

        New = make_variable(name);
        if (New->var->gensym_number != current_variable_gensym_number)
        {
            break;
        }
        /* -- A variable with that name already existed.  make_variable just returned it and
         *    incremented its refcount, so reverse that refcount addition and try again. -- */
        symbol_remove_ref(&New);
    }

    New->var->current_binding_value = NIL;
    New->var->gensym_number = current_variable_gensym_number;
    return New;
}

/* ----------------------------------------------------------------
   Takes a list of symbols and returns a copy of the same list,
   incrementing the reference count on each symbol in the list.
---------------------------------------------------------------- */

cons* Symbol_Manager::copy_symbol_list_adding_references(cons* sym_list)
{
    cons* c, *first, *prev;

    if (! sym_list)
    {
        return NIL;
    }
    allocate_cons(thisAgent, &first);
    first->first = sym_list->first;
    symbol_add_ref(static_cast<Symbol*>(first->first));
    sym_list = sym_list->rest;
    prev = first;
    while (sym_list)
    {
        allocate_cons(thisAgent, &c);
        prev->rest = c;
        c->first = sym_list->first;
        symbol_add_ref(static_cast<Symbol*>(c->first));
        sym_list = sym_list->rest;
        prev = c;
    }
    prev->rest = NIL;
    return first;
}

/* ----------------------------------------------------------------
   Frees a list of symbols, decrementing their reference counts.
---------------------------------------------------------------- */

void Symbol_Manager::deallocate_symbol_list_removing_references(cons*& sym_list)
{
    cons* c;
    Symbol* lSym;
    while (sym_list)
    {
        c = sym_list;
        sym_list = sym_list->rest;
        lSym = static_cast<Symbol*>(c->first);
        symbol_remove_ref(&lSym);
        free_cons(thisAgent, c);
    }
    sym_list = NULL;
}
