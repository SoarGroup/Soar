/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  ebc_build.cpp
 *
 * =======================================================================
 *  These are the routines that support printing Soar data structures.
 *
 * =======================================================================
 */

#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_repair.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "debug.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "run_soar.h"
#include "soar_TraceNames.h"
#include "slot.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <string>

// Just for sort_rule_str():
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace soar_TraceNames;

/* =====================================================================

                           Results Calculation

   Get_results_for_instantiation() finds and returns the result preferences
   for a given instantiation.  This is the main routine here.

   The results are accumulated in the list "results," linked via the
   "next_result" field of the preference structures.  (NOTE: to save
   space, just use conses for this.)

   Add_pref_to_results() adds a preference to the results.
   Add_results_for_id() adds any preferences for the given identifier.
   Identifiers are marked with results_tc_number as they are added.
===================================================================== */
inline bool pref_has_identity_set_in_field(preference* pPref, WME_Field pField)
{
    if (pField == ID_ELEMENT) { if (pPref->identities.id) return true; else return false; }
    else if (pField == ATTR_ELEMENT) { if (pPref->identities.attr) return true; else return false; }
    else if (pField == VALUE_ELEMENT) { if (pPref->identities.value) return true; else return false; }
    else if (pField == REFERENT_ELEMENT) { if (pPref->identities.referent) return true; else return false; }

    return false;
}

inline bool pref_has_same_identity_sets_in_2_fields(preference* pPref1, WME_Field pField1, preference* pPref2, WME_Field pField2)
{
    if (pField1 == ID_ELEMENT)
    {
        if (pField2 == ID_ELEMENT) { if (pPref1->identities.id == pPref2->identities.id) return true; else return false; }
        if (pField2 == ATTR_ELEMENT) { if (pPref1->identities.id == pPref2->identities.attr) return true; else return false; }
        if (pField2 == VALUE_ELEMENT) { if (pPref1->identities.id == pPref2->identities.value) return true; else return false; }
        if (pField2 == REFERENT_ELEMENT) { if (pPref1->identities.id == pPref2->identities.referent) return true; else return false; }
    }
    else if (pField1 == ATTR_ELEMENT)
    {
        if (pField2 == ID_ELEMENT) { if (pPref1->identities.attr == pPref2->identities.id) return true; else return false; }
        if (pField2 == ATTR_ELEMENT) { if (pPref1->identities.attr == pPref2->identities.attr) return true; else return false; }
        if (pField2 == VALUE_ELEMENT) { if (pPref1->identities.attr == pPref2->identities.value) return true; else return false; }
        if (pField2 == REFERENT_ELEMENT) { if (pPref1->identities.attr == pPref2->identities.referent) return true; else return false; }
    }
    else if (pField1 == VALUE_ELEMENT)
    {
        if (pField2 == ID_ELEMENT) { if (pPref1->identities.value == pPref2->identities.id) return true; else return false; }
        if (pField2 == ATTR_ELEMENT) { if (pPref1->identities.value == pPref2->identities.attr) return true; else return false; }
        if (pField2 == VALUE_ELEMENT) { if (pPref1->identities.value == pPref2->identities.value) return true; else return false; }
        if (pField2 == REFERENT_ELEMENT) { if (pPref1->identities.value == pPref2->identities.referent) return true; else return false; }
    }
    else if (pField1 == VALUE_ELEMENT)
    {
        if (pField2 == ID_ELEMENT) { if (pPref1->identities.value == pPref2->identities.id) return true; else return false; }
        if (pField2 == ATTR_ELEMENT) { if (pPref1->identities.value == pPref2->identities.attr) return true; else return false; }
        if (pField2 == VALUE_ELEMENT) { if (pPref1->identities.value == pPref2->identities.value) return true; else return false; }
        if (pField2 == REFERENT_ELEMENT) { if (pPref1->identities.value == pPref2->identities.referent) return true; else return false; }
    }
    return false;
}

// Based on version from boost
inline uint32_t hash_combine(const uint32_t& hash1, const uint32_t& hash2)
{
	uint32_t result = hash1;
    result ^= hash2 + 0x9e3779b9 + (hash1<<6) + (hash1>>2);
    return result;
}

/* An attempt to hash a rhs_value (char*) without entirely knowing how all these data structures are used.
 * (In particular, cons->first is a generic void*.)
 * Based on the rhs_values_equal definition in rhs.h. */
inline uint32_t hash_rhs_value(rhs_value val) {
	if (rhs_value_is_symbol(val)) {
		if (!val)
			return 1;	// When the rhs_value has no referent
		return reinterpret_cast<rhs_symbol>(val)->referent->hash_id;
	}
	std::hash<char> c_hasher;
	if (rhs_value_is_funcall(val)) {
		cons* fl = rhs_value_to_funcall_list(val);
		uint32_t result = c_hasher(*static_cast<char*>(fl->first));
		for (cons* c = fl->rest; c != NIL; c = c->rest)
			result = hash_combine(result, hash_rhs_value(static_cast<char*>(c->first)));
		return result;
	}
	return c_hasher(*val);	// TODO: Is this valid? Doesn't seem to be reached normally.
}

inline uint32_t hash_action(action* pAct) {
	
	std::hash<uint64_t> int64_hasher;
	uint32_t result = pAct->type + 129064587;	// random number
	result = hash_combine(result, hash_rhs_value(pAct->id));
	result = hash_combine(result, hash_rhs_value(pAct->attr));
	result = hash_combine(result, hash_rhs_value(pAct->value));
	result = hash_combine(result, hash_rhs_value(pAct->referent));
	return result;
}

inline uint32_t get_lhs_hash(agent* thisAgent, condition* pCond) {
	uint32_t hsh = 0,
			minihsh = 0;
	while (pCond != NULL)
	{
		minihsh = hash_condition(thisAgent, pCond);
		if (!hsh)
			hsh = minihsh;
		else
			hsh = hash_combine(hsh, minihsh);
		pCond = pCond->next;
	}
	return hsh;
}

inline uint32_t get_rhs_hash(action* pAct) {
	uint32_t hsh = 0,
			minihsh = 0;
	while (pAct != NULL)
	{
		minihsh = hash_action(pAct);
		if (!hsh)
			hsh = minihsh;
		else
			hsh = hash_combine(hsh, minihsh);
		pAct = pAct->next;
	}
	return hsh;
}

inline uint32_t get_rule_hash(agent* thisAgent, condition* lhs, action* rhs) {
	uint32_t result = get_lhs_hash(thisAgent, lhs);
	return hash_combine(result, get_rhs_hash(rhs));
}

std::string sorted_conds_str(agent* thisAgent, condition* conds) {
    std::string line;
    std::string ret_str = "";
	std::vector<std::string> srt_list = std::vector<std::string>();

	// Split the structure into condition lines
    condition* c;
	for (c = conds; c != NIL; c = c->next) {
		line = "";
		if (c->type == CONJUNCTIVE_NEGATION_CONDITION) {
			line = sorted_conds_str(thisAgent, c->data.ncc.top);
			srt_list.push_back("-{" + line + "}");
			continue;
		}
		if (c->type == NEGATIVE_CONDITION)
		    line = "-";
		line += "(^";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.attr_test);
		line += " ";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.value_test);
		line += " ";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.id_test);
		line += ")";

		srt_list.push_back(line);
	}
	std::sort(srt_list.begin(), srt_list.end(), std::greater<std::string>());
	for (auto const& vs : srt_list) { ret_str += vs + "\n"; }
	return ret_str;
}

std::string sorted_acts_str(agent* thisAgent, action* acts) {
	std::string line;
	std::string ret_str = "";
	std::vector<std::string> srt_list = std::vector<std::string>();

	// Split the structure into action lines
	action* a;
	for (a = acts; a != NIL; a = a->next) {
		line = "";
		if (a->type == FUNCALL_ACTION) {
			thisAgent->outputManager->rhs_value_to_string(a->value, line);
		}
		else {
			line += preference_to_char(a->preference_type);
			if (a->referent) {
				line += " ";
				thisAgent->outputManager->rhs_value_to_string(a->referent, line);
			}
			line += "(^";
			thisAgent->outputManager->rhs_value_to_string(a->attr, line);
			line += " ";
			thisAgent->outputManager->rhs_value_to_string(a->value, line);
			line += " ";
			thisAgent->outputManager->rhs_value_to_string(a->id, line);
			line += ')';
		}

		srt_list.push_back(line);
	}
	std::sort(srt_list.begin(), srt_list.end(), std::greater<std::string>());
	for (auto const& vs : srt_list) { ret_str += vs + "\n"; }
	return ret_str;
}

std::string sorted_conds_str2(agent* thisAgent, condition* conds) {
    std::string line;
    std::string ret_str = "";
	std::vector<std::string> srt_list = std::vector<std::string>();

	// Split the structure into condition lines
    condition* c;
	for (c = conds; c != NIL; c = c->next) {
		line = "";
		if (c->type == CONJUNCTIVE_NEGATION_CONDITION) {
			line = sorted_conds_str(thisAgent, c->data.ncc.top);
			srt_list.push_back("-{" + line + "}");
			continue;
		}
		if (c->type == NEGATIVE_CONDITION)
		    line = "-";
		line += "(^";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.attr_test);
		line += " ";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.value_test);
		line += " ";
		thisAgent->outputManager->sprinta_sf(thisAgent, line, "%t", c->data.tests.id_test);
		line += ")";

		if (line.find("name") != std::string::npos)
			srt_list.push_back(line);
	}
	if (!srt_list.size())
		ret_str = "";
	else {
		std::sort(srt_list.begin(), srt_list.end(), std::greater<std::string>());
		for (auto const& vs : srt_list) { ret_str += vs + "\n"; }
	}
	return ret_str;
}


std::string rename_str_vars(agent* thisAgent, const std::string& rule_str) {
	std::unordered_map<std::string, std::string> newvars;
	std::string ret_str = rule_str;
	std::string temp_str;
	int var_num = 1;
	std::string gen_str = "1";
	int curr_ind = ret_str.find("<");
	int end_ind = curr_ind;

	while (curr_ind != std::string::npos) {
		end_ind = ret_str.find(">",curr_ind+1);	// Find closing '>'
		if (!isspace(ret_str.at(curr_ind+1)) && end_ind != std::string::npos) {
		 	temp_str = ret_str.substr(curr_ind+1, end_ind-curr_ind-1);
		 	if (newvars.find(temp_str) == newvars.end()) {
		 		newvars[temp_str] = gen_str;
		 		temp_str = gen_str;
		 		gen_str = std::to_string(++var_num);
		 	}
		 	else {
		 		temp_str = newvars[temp_str];
		 	}
		 	ret_str.replace(curr_ind+1, end_ind-curr_ind-1, temp_str);
		}
		curr_ind = ret_str.find("<", curr_ind+2);
	}

	// Also remove identity number []'s from rhs IDs
	curr_ind = ret_str.find("[");
	while (curr_ind != std::string::npos) {
		end_ind = ret_str.find("]",curr_ind+1);	// Find closing ']'
		if (end_ind != std::string::npos) {
			ret_str.erase(curr_ind, end_ind-curr_ind+1);
		}
		curr_ind = ret_str.find("[", curr_ind);
	}

	return ret_str;
}

std::string get_rule_str(agent* thisAgent, condition* conds, action* acts) {
	std::string rule_str = sorted_conds_str2(thisAgent, conds) + "-->\n" + sorted_acts_str(thisAgent, acts);
	rule_str = rename_str_vars(thisAgent, rule_str);
	return rule_str;
}


void Explanation_Based_Chunker::add_pref_to_results(preference* pref, preference* pLinkPref, WME_Field pField)
{
    preference* p;

    dprint(DT_EXTRA_RESULTS, "Attempting to add pref to results: %p \n", pref);
    /* --- if an equivalent pref is already a result, don't add this one --- */
    for (p = m_results; p != NIL; p = p->next_result)
    {
        if (p->id != pref->id) continue;
        if (p->attr != pref->attr) continue;
        if (p->value != pref->value) continue;
        if (p->type != pref->type) continue;
        if (preference_is_unary(pref->type)) return;
        if (p->referent != pref->referent) continue;
        return;
    }

    /* --- if pref isn't at the right level, find a clone that is --- */
    if (pref->inst->match_goal_level != m_results_match_goal_level)
    {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->inst->match_goal_level == m_results_match_goal_level)
            {
                break;
            }
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->inst->match_goal_level == m_results_match_goal_level)
                {
                    break;
                }
        if (!p)
        {
            return;    /* if can't find one, it isn't a result */
        }
        pref = p;
    }

    /* --- add this preference to the result list --- */
    pref->next_result = m_results;
    m_results = pref;
    if (pref->identities.id && pref_has_identity_set_in_field(pLinkPref, pField))
    {
        if (!pref_has_same_identity_sets_in_2_fields(pref, ID_ELEMENT, pLinkPref, pField))
        {
            if (pField == ID_ELEMENT)
            {
                join_identities(pref->identities.id, pLinkPref->identities.id);
                thisAgent->explanationMemory->add_identity_set_mapping(pref->inst->i_id, IDS_unified_child_result, pref->identities.id, pLinkPref->identities.id);
            }
            if (pField == ATTR_ELEMENT)
            {
                join_identities(pref->identities.id, pLinkPref->identities.attr);
                thisAgent->explanationMemory->add_identity_set_mapping(pref->inst->i_id, IDS_unified_child_result, pref->identities.id, pLinkPref->identities.attr);
            }
            if (pField == VALUE_ELEMENT)
            {
                join_identities(pref->identities.id, pLinkPref->identities.value);
                thisAgent->explanationMemory->add_identity_set_mapping(pref->inst->i_id, IDS_unified_child_result, pref->identities.id, pLinkPref->identities.value);
            }
        }
    }

    /* --- follow transitive closure through value, referent links --- */
    add_results_if_needed(pref->value, pref, VALUE_ELEMENT);
    if (preference_is_binary(pref->type))
    {
        add_results_if_needed(pref->referent, pref, REFERENT_ELEMENT);
    }
}

void Explanation_Based_Chunker::add_results_if_needed(Symbol* pSym, preference* pPref, WME_Field pField)
{
    slot* s;
    preference* pref;
    wme* w;

    if ((pSym)->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        if (((pSym)->id->level >= m_results_match_goal_level) && ((pSym)->tc_num != m_results_tc))
        {
            pSym->tc_num = m_results_tc;

            /* --- scan through all preferences and wmes for all slots for this id --- */
            for (w = pSym->id->input_wmes; w != NIL; w = w->next)
            {
                add_results_if_needed(w->value, w->preference, w->preference ? VALUE_ELEMENT : NO_ELEMENT);
            }
            for (s = pSym->id->slots; s != NIL; s = s->next)
            {
                for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
                {
                    add_pref_to_results(pref, pPref, pField);
                }
                for (w = s->wmes; w != NIL; w = w->next)
                {
                    add_results_if_needed(w->value, w->preference, w->preference ? VALUE_ELEMENT : NO_ELEMENT);
                }
            } /* end of for slots loop */
            /* --- now scan through extra prefs and look for any with this id --- */
            for (pref = m_extra_results; pref != NIL;
                pref = pref->inst_next)
            {
                if (pref->id == pSym)
                {
                    add_pref_to_results(pref, pPref, pField);
                }
            }
            return;
        }
    }
}

void Explanation_Based_Chunker::get_results_for_instantiation()
{
    preference* pref;

    m_results = NIL;
    m_results_match_goal_level = m_inst->match_goal_level;
    m_results_tc = get_new_tc_number(thisAgent);
    m_extra_results = m_inst->preferences_generated;
    Identity* lNULL;

    for (pref = m_inst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        if ((pref->id->id->level < m_results_match_goal_level) &&
                (pref->id->tc_num != m_results_tc))
        {
            add_pref_to_results(pref, NULL, NO_ELEMENT);
        }
    }
}

/* ====================================================================

     Chunk Conditions, and Chunk Conditions Set Manipulation Routines

   These structures have two uses.  First, for every ground condition,
   one of these structures maintains certain information about it--
   pointers to the original (instantiation's) condition, the chunks's
   instantiation's condition, and the variablized condition, etc.

   Second, for negated conditions, these structures are entered into
   a hash table with keys hash_condition(thisAgent, this_cond).  This hash
   table is used so we can add a new negated condition to the set of
   negated potentials quickly--we don't want to add a duplicate of a
   negated condition that's already there, and the hash table lets us
   quickly determine whether a duplicate is already there.

   I used one type of structure for both of these uses, (1) for simplicity
   and (2) to avoid having to do a second allocation when we move
   negated conditions over to the ground set.
==================================================================== */

/* --------------------------------------------------------------------
                      Chunk Cond Set Routines

   Init_chunk_cond_set() initializes a given chunk_cond_set to be empty.

   Make_chunk_cond_for_condition() takes a condition and returns a
   chunk_cond for it, for use in a chunk_cond_set.  This is used only
   for the negated conditions, not grounds.

   Add_to_chunk_cond_set() adds a given chunk_cond to a given chunk_cond_set
   and returns true if the condition isn't already in the set.  If the
   condition is already in the set, the routine deallocates the given
   chunk_cond and returns false.

   Remove_from_chunk_cond_set() removes a given chunk_cond from a given
   chunk_cond_set, but doesn't deallocate it.
-------------------------------------------------------------------- */

/* set of all negated conditions we encounter during backtracing--these are
 * all potentials and (some of them) are added to the grounds in one pass at
 * the end of the backtracing */

void Explanation_Based_Chunker::init_chunk_cond_set(chunk_cond_set* set)
{
    int i;

    set->all = NIL;
    for (i = 0; i < CHUNK_COND_HASH_TABLE_SIZE; i++)
    {
        set->table[i] = NIL;
    }
}

/* -- Note:  add_to_chunk_cond_set and make_chunk_cond_for_negated_condition are both
 *           only used for negative conditions and NCCS.  Used in a single line in
 *           backtrace_through_instantiation() -- */

chunk_cond* Explanation_Based_Chunker::make_chunk_cond_for_negated_condition(condition* cond)
{
    chunk_cond* cc;
    uint32_t remainder, hv;

    thisAgent->memoryManager->allocate_with_pool(MP_chunk_cond, &cc);
    cc->cond = cond;
    cc->hash_value = hash_condition(thisAgent, cond);
    remainder = cc->hash_value;
    hv = 0;
    while (remainder)
    {
        hv ^= (remainder &
               masks_for_n_low_order_bits[LOG_2_CHUNK_COND_HASH_TABLE_SIZE]);
        remainder = remainder >> LOG_2_CHUNK_COND_HASH_TABLE_SIZE;
    }
    cc->compressed_hash_value = hv;
    return cc;
}

bool Explanation_Based_Chunker::add_to_chunk_cond_set(chunk_cond_set* set, chunk_cond* new_cc)
{
    chunk_cond* old;

    for (old = set->table[new_cc->compressed_hash_value]; old != NIL;
            old = old->next_in_bucket)
        if (old->hash_value == new_cc->hash_value)
            if (conditions_are_equal(old->cond, new_cc->cond))
            {
                break;
            }
    if (old)
    {
        /* --- the new condition was already in the set; so don't add it --- */
        thisAgent->memoryManager->free_with_pool(MP_chunk_cond, new_cc);
        return false;
    }
    /* --- add new_cc to the table --- */
    insert_at_head_of_dll(set->all, new_cc, next, prev);
    insert_at_head_of_dll(set->table[new_cc->compressed_hash_value], new_cc,
                          next_in_bucket, prev_in_bucket);
    return true;
}

void Explanation_Based_Chunker::remove_from_chunk_cond_set(chunk_cond_set* set, chunk_cond* cc)
{
    remove_from_dll(set->all, cc, next, prev);
    remove_from_dll(set->table[cc->compressed_hash_value],
                    cc, next_in_bucket, prev_in_bucket);
}

/* ====================================================================

                 Other Miscellaneous Chunking Routines

==================================================================== */

/* --------------------------------------------------------------------
            Build Chunk Conds For Grounds And Add Negateds

   This routine is called once backtracing is finished.  It goes through
   the ground conditions and builds a chunk_cond (see above) for each
   one.  The chunk_cond includes two new copies of the condition:  one
   to be used for the initial instantiation of the chunk, and one to
   be (variablized and) used for the chunk itself.

   This routine also goes through the negated conditions and adds to
   the ground set (again building chunk_cond's) any negated conditions
   that are connected to the grounds.

   At exit, the "dest_top" and "dest_bottom" arguments are set to point
   to the first and last chunk_cond in the ground set.  The "tc_to_use"
   argument is the tc number that this routine will use to mark the
   TC of the ground set.  At exit, this TC indicates the set of identifiers
   in the grounds.  (This is used immediately afterwards to figure out
   which Nots must be added to the chunk.)
-------------------------------------------------------------------- */

inline void add_cond(condition** c, condition** prev, condition** first)
{
    if (*prev)
    {
        (*c)->prev = *prev;
        (*prev)->next = *c;
    }
    else
    {
        *first = *c;
        *prev = NIL;
        (*c)->prev = NIL;
    }
    *prev = *c;

}

void Explanation_Based_Chunker::create_initial_chunk_condition_lists()
{
    cons* c;
    condition* ground, *c_vrblz, *first_vrblz = nullptr, *prev_vrblz;
    bool should_unify_and_simplify = m_learning_on_for_instantiation;

    tc_number tc_to_use = get_new_tc_number(thisAgent);

    c_vrblz  = NIL; /* unnecessary, but gcc -Wall warns without it */

    dprint(DT_BUILD_CHUNK_CONDS, "Building conditions for new chunk...\n");
    dprint(DT_BUILD_CHUNK_CONDS, "Grounds from backtrace: \n");
    dprint_noprefix(DT_BUILD_CHUNK_CONDS, "%3", grounds);

    /* --- build instantiated conds for grounds --- */
    prev_vrblz = NIL;
    while (grounds)
    {
        c = grounds;
        grounds = grounds->rest;
        ground = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);
        /* --- make the instantiated condition --- */
        dprint(DT_BACKTRACE, "   processing ground condition: %l\n", ground);

        c_vrblz = copy_condition(thisAgent, ground, true, should_unify_and_simplify, true, true);

        /* Find tests in conditions that we can attach transitive constraints to */
        if (ebc_settings[SETTING_EBC_LEARNING_ON])
        {
            if (c_vrblz->data.tests.value_test->eq_test->identity && !c_vrblz->data.tests.value_test->eq_test->identity->get_operational_cond())
            {
                c_vrblz->data.tests.value_test->eq_test->identity->set_operational_cond(c_vrblz, VALUE_ELEMENT);
            } else {
                if (c_vrblz->data.tests.value_test->eq_test->identity)
                    dprint(DT_CONSTRAINTS, "Not setting value element of %l as operational cond because operational cond %l already exists.\n", c_vrblz, c_vrblz->data.tests.value_test->eq_test->identity->get_operational_cond());
//                else
//                    dprint(DT_CONSTRAINTS, "Not setting value element of %l as operational cond because no identity set exists.\n", c_vrblz);
            }
            if (c_vrblz->data.tests.attr_test->eq_test->identity && !c_vrblz->data.tests.attr_test->eq_test->identity->get_operational_cond())
            {
                c_vrblz->data.tests.attr_test->eq_test->identity->set_operational_cond(c_vrblz, ATTR_ELEMENT);
            } else {
                if (c_vrblz->data.tests.attr_test->eq_test->identity)
                    dprint(DT_CONSTRAINTS, "Not setting attr element of %l as operational cond because operational cond %l already exists.\n", c_vrblz, c_vrblz->data.tests.attr_test->eq_test->identity->get_operational_cond());
//                else
//                    dprint(DT_CONSTRAINTS, "Not setting attr element of %l as operational cond because no identity set exists.\n", c_vrblz);
            }
            if (c_vrblz->data.tests.id_test->eq_test->identity && !c_vrblz->data.tests.id_test->eq_test->identity->get_operational_cond())
            {
                c_vrblz->data.tests.id_test->eq_test->identity->set_operational_cond(c_vrblz, ID_ELEMENT);
            } else {
                if (c_vrblz->data.tests.id_test->eq_test->identity)
                    dprint(DT_CONSTRAINTS, "Not setting id element of %l as operational cond because operational cond %l already exists.\n", c_vrblz, c_vrblz->data.tests.id_test->eq_test->identity->get_operational_cond());
//                else
//                    dprint(DT_CONSTRAINTS, "Not setting id element of %l as operational cond because no identity set exists.\n", c_vrblz);
            }
        }

        /* --- Add condition and add to the TC so we can see if NCCs are grounded. --- */
        add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);
        add_cond_to_tc(thisAgent, ground, tc_to_use, NIL, NIL);
    }

    dprint(DT_BUILD_CHUNK_CONDS, "...adding negated conditions from backtraced negated set.\n");
    /* --- scan through negated conditions and check which ones are connected to the grounds --- */
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa(thisAgent, "\n\n*** Adding Grounded Negated Conditions ***\n");
    }

    chunk_cond *cc;
    bool has_local_negation = false;
    while (negated_set.all)
    {
        cc = negated_set.all;
        remove_from_chunk_cond_set(&negated_set, cc);
        if (cond_is_in_tc(thisAgent, cc->cond, tc_to_use))
        {
            /* --- negated cond is in the TC, so add it to the grounds --- */
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                thisAgent->outputManager->printa(thisAgent, "\n-->Moving to grounds: ");
                print_condition(thisAgent, cc->cond);
            }
            dprint(DT_BUILD_CHUNK_CONDS, "...adding negated condition %l\n", cc->cond);
            c_vrblz = copy_condition(thisAgent, cc->cond, true, false, true, true);

            add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);
        }
        else
        {
            /* --- not in TC, so discard the condition --- */

            if (ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] == false)
            {
                if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                {
                    report_local_negation(cc->cond);
                }
            }
            has_local_negation = true;
        }
        thisAgent->memoryManager->free_with_pool(MP_chunk_cond, cc);
    }

    if (has_local_negation)
    {
        m_tested_local_negation = true;
        if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING] && !ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS])
        {
            thisAgent->stop_soar = true;
            thisAgent->reason_for_stopping = "Chunking issue detected:  Problem-solving contained negated reasoning about sub-state structures.";
        }
    }

    if (prev_vrblz)
    {
        prev_vrblz->next = NIL;
    }
    else if (first_vrblz)
    {
        first_vrblz->next = NIL;
    }

    m_lhs = first_vrblz;

    if (first_vrblz && ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        add_additional_constraints();
    }
    dprint(DT_BUILD_CHUNK_CONDS, "build_chunk_conds_for_grounds_and_add_negateds done.\n");
}

/* --------------------------------------------------------------------
                     Add Goal or Impasse Tests

   This routine adds goal id or impasse id tests to the variablized
   conditions.  For each id in the grounds that happens to be the
   identifier of a goal or impasse, we add a goal/impasse id test
   to the variablized conditions, to make sure that in the resulting
   chunk, the variablization of that id is constrained to match against
   a goal/impasse.  (Note:  actually, in the current implementation of
   chunking, it's impossible for an impasse id to end up in the ground
   set.  So part of this code is unnecessary.)
-------------------------------------------------------------------- */

void Explanation_Based_Chunker::add_goal_or_impasse_tests()
{
    condition* cc;
    tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
    Symbol* idSym, *id_vrblz;
    test t;
    bool isa_goal, isa_impasse;

    tc = get_new_tc_number(thisAgent);
    for (cc = m_lhs; cc != NIL; cc = cc->next)
    {
        if (cc->type != POSITIVE_CONDITION)
        {
            continue;
        }
        idSym = cc->data.tests.id_test->eq_test->data.referent;
        isa_goal = idSym->is_variable() ? idSym->var->instantiated_sym->id->isa_goal : idSym->id->isa_goal;
        isa_impasse = idSym->is_variable() ? idSym->var->instantiated_sym->id->isa_impasse : idSym->id->isa_impasse;
        if ((isa_goal || isa_impasse) && (idSym->tc_num != tc))
        {
            t = make_test(thisAgent, NULL, (isa_goal ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            add_test(thisAgent, &(cc->data.tests.id_test), t);
            idSym->tc_num = tc;
        }
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Conditions after add goal tests: \n%1", m_lhs);

}

/* --------------------------------------------------------------------
                       Make Clones of Results

   When we build the initial instantiation of the new chunk, we have
   to fill in preferences_generated with *copies* of all the result
   preferences.  These copies are clones of the results.  This routine
   makes these clones and fills in chunk_inst->preferences_generated.
-------------------------------------------------------------------- */

void Explanation_Based_Chunker::make_clones_of_results()
{
    preference* lClonedPref, *lResultPref;
    m_chunk_inst->preferences_generated = NIL;
    for (lResultPref = m_results; lResultPref != NIL; lResultPref = lResultPref->next_result)
    {
        /* --- copy the preference --- */
        dprint(DT_CLONES, "Creating clone for result preference %p (instantiation i%u %y)\n", lResultPref, lResultPref->inst->i_id, lResultPref->inst->prod_name);
        lClonedPref = make_preference(thisAgent, lResultPref->type, lResultPref->id, lResultPref->attr, lResultPref->value, lResultPref->referent, lResultPref->chunk_inst_identities);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->id);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->attr);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->value);
        if (preference_is_binary(lClonedPref->type))
        {
            thisAgent->symbolManager->symbol_add_ref(lClonedPref->referent);
        }
        lClonedPref->inst = m_chunk_inst;
        lClonedPref->level = m_chunk_inst->match_goal_level;

        /* Move cloned_rhs_funcs into rhs_funs of cloned pref */
        if (lResultPref->rhs_func_chunk_inst_identities.id)
        {
            lClonedPref->rhs_func_inst_identities.id = lResultPref->rhs_func_chunk_inst_identities.id;
            lResultPref->rhs_func_chunk_inst_identities.id = NULL;
        }
        if (lResultPref->rhs_func_chunk_inst_identities.attr)
        {
            lClonedPref->rhs_func_inst_identities.attr = lResultPref->rhs_func_chunk_inst_identities.attr;
            lResultPref->rhs_func_chunk_inst_identities.attr = NULL;
        }
        if (lResultPref->rhs_func_chunk_inst_identities.value)
        {
            lClonedPref->rhs_func_inst_identities.value = lResultPref->rhs_func_chunk_inst_identities.value;
            lResultPref->rhs_func_chunk_inst_identities.value = NULL;
        }
        if (lResultPref->rhs_func_chunk_inst_identities.referent)
        {
            lClonedPref->rhs_func_inst_identities.referent = lResultPref->rhs_func_chunk_inst_identities.referent;
            lResultPref->rhs_func_chunk_inst_identities.referent = NULL;
        }

        /* We record the identity set ids for the explainer so they match up with identity sets on this level */
        if (thisAgent->explanationMemory->is_any_enabled())
        {
            if (lResultPref->identities.id) lClonedPref->chunk_inst_identities.id = lResultPref->identities.id->get_identity();
            if (lResultPref->identities.attr) lClonedPref->chunk_inst_identities.attr = lResultPref->identities.attr->get_identity();
            if (lResultPref->identities.value) lClonedPref->chunk_inst_identities.value = lResultPref->identities.value->get_identity();
            if (lResultPref->identities.referent) lClonedPref->chunk_inst_identities.referent = lResultPref->identities.referent->get_identity();
        }
        dprint(DT_CLONES, "Created clone for result preference %p (instantiation i%u %y)\n", lClonedPref, lClonedPref->inst->i_id, lClonedPref->inst->prod_name);

        /* --- put it onto the list for chunk_inst --- */
        insert_at_head_of_dll(m_chunk_inst->preferences_generated, lClonedPref, inst_next, inst_prev);

        /* --- insert it into the list of clones for this preference --- */
        lClonedPref->next_clone = lResultPref;
        lClonedPref->prev_clone = lResultPref->prev_clone;
        lResultPref->prev_clone = lClonedPref;
        if (lClonedPref->prev_clone)
        {
            lClonedPref->prev_clone->next_clone = lClonedPref;
        }

    }
}

void Explanation_Based_Chunker::remove_chunk_instantiation()
{
    preference* lNext, *lResultPref;

    excise_production(thisAgent, m_chunk_inst->prod, false, true);
    if (m_rule_type == ebc_chunk)
        production_remove_ref(thisAgent, m_chunk_inst->prod);
    m_chunk_inst->prod = NULL;
    m_chunk_inst->in_ms = false;
    for (lResultPref = m_chunk_inst->preferences_generated; lResultPref != NIL; lResultPref = lNext)
    {
        lNext = lResultPref->inst_next;
        dprint(DT_EBC_CLEANUP, "Removing cloned preference %p (%d)\n", lResultPref, lResultPref->reference_count);
        remove_preference_from_clones_and_deallocate(thisAgent, lResultPref);
    }
    m_chunk_inst = NULL;
}

bool Explanation_Based_Chunker::can_learn_from_instantiation()
{
    preference* pref;

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (! m_inst->match_goal) return false;

    /* --- if no preference is above the match goal level, exit --- */
    for (pref = m_inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        if (pref->id->id->level < m_inst->match_goal_level) break;

    /* Note that in Soar 9.3.4, chunking would not create a chunk for a
     * a result if another result was also being returned to an even higher
     * superstate.  In this part of the code, it would set learning off
     * so that an intermediate justification is created instead.  We disabled that
     * aspect in 2/2013 to fix issues that Shiwali was having with her research
     * agents. Documenting here in case this ever needs to changed back or made
     * an option. */

    return (pref != NULL);
}

void Explanation_Based_Chunker::deallocate_failed_chunk()
{
    deallocate_condition_list(thisAgent, m_lhs);
    m_lhs = NULL;
    deallocate_action_list(thisAgent, m_rhs);
    m_rhs = NULL;
}

bool Explanation_Based_Chunker::add_chunk_to_rete()
{
    byte rete_addition_result;
    production* duplicate_rule = NULL;

    rete_addition_result = add_production_to_rete(thisAgent, m_prod, m_lhs, m_chunk_inst, m_should_print_name, duplicate_rule);

    if (m_should_print_prod && (rete_addition_result != DUPLICATE_PRODUCTION))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "\n");
        xml_begin_tag(thisAgent, kTagLearning);
        print_production(thisAgent, m_prod, false);
        xml_end_tag(thisAgent, kTagLearning);
    }
    if (rete_addition_result == REFRACTED_INST_MATCHED)
    {
        thisAgent->explanationMemory->record_chunk_contents(m_prod, m_lhs, m_rhs, m_results, inst_id_to_identity_map, m_inst, m_chunk_inst, m_prod_type);
        if (m_prod_type == JUSTIFICATION_PRODUCTION_TYPE) {
            thisAgent->explanationMemory->increment_stat_justifications_succeeded();
            /* We'll interrupt on justification learning only if explainer is recording justifications.  In
             * most cases I think we wouldn't want to interrupt on every justification learned */
            if (ebc_settings[SETTING_EBC_INTERRUPT] && thisAgent->explanationMemory->isRecordingJustifications())
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Soar learned a new justification.";

            }
        } else {
            thisAgent->explanationMemory->increment_stat_chunks_succeeded();
            if (ebc_settings[SETTING_EBC_INTERRUPT])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Soar learned a new rule.";

            }
            if (ebc_settings[SETTING_EBC_INTERRUPT_WATCHED] && thisAgent->explanationMemory->isCurrentlyRecording())
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Soar learned a new rule from a watched production.";
            }
        }
        dprint(DT_VARIABLIZATION_MANAGER, "Add production %y to rete result: Refracted instantiation matched.\n", m_chunk_inst->prod_name);
        return true;
    } else if (rete_addition_result == DUPLICATE_PRODUCTION) {
        if (m_inst->prod)
        {
            if (thisAgent->d_cycle_count == m_inst->prod->last_duplicate_dc)
            {
                m_inst->prod->duplicate_chunks_this_cycle++;
            } else {
                m_inst->prod->duplicate_chunks_this_cycle = 1;
                m_inst->prod->last_duplicate_dc = thisAgent->d_cycle_count;
            }
        }
        thisAgent->explanationMemory->increment_stat_duplicates(duplicate_rule);
        thisAgent->explanationMemory->cancel_chunk_record();
        dprint(DT_VARIABLIZATION_MANAGER, "Add production %y to rete result: Duplicate production.\n", m_chunk_inst->prod_name);
        return false;
    } else if (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH) {
        if (m_prod_type == JUSTIFICATION_PRODUCTION_TYPE)
        {
            thisAgent->explanationMemory->increment_stat_justifications_succeeded();
            if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Warning:  Justification did not match working memory.  Potential issue.";
                print_current_built_rule("Justification that did not match WM: ");
            }
        } else {
            thisAgent->explanationMemory->increment_stat_chunks_succeeded();
            if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Warning:  Chunk did not match working memory.  Potential issue.";
                print_current_built_rule("Chunk that did not match WM: ");
            }
        }
        dprint(DT_VARIABLIZATION_MANAGER, "Add production %y to rete result: Refracted instantiation did not match.\n", m_chunk_inst->prod_name);

        thisAgent->explanationMemory->record_chunk_contents(m_prod, m_lhs, m_rhs, m_results, inst_id_to_identity_map, m_inst, m_chunk_inst, m_prod_type);

        m_chunk_inst->in_ms = false;
        return true;
    }

    return false;
}

void Explanation_Based_Chunker::learn_rule_from_instance(instantiation* inst, instantiation** new_inst_list)
{
    preference*         pref;
    bool                lChunkValidated = true;
    bool                lRevertedChunk = false;
    condition*          l_inst_top = NULL;
    condition*          l_inst_bottom = NULL;
    uint64_t            l_clean_up_id;
    soar_timer*          lLocalTimerPtr = NULL;

    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    soar_timer local_timer;
    lLocalTimerPtr = &local_timer;
    local_timer.set_enabled(&(thisAgent->timers_enabled));
    #endif

    /* --- If we're over MAX_CHUNKS, abort chunk --- */
    if (chunks_this_d_cycle >= max_chunks)
    {
        thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_max_chunks, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
        thisAgent->explanationMemory->increment_stat_max_chunks();
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    m_inst = inst;
    m_failure_type = ebc_success;

    if (!can_learn_from_instantiation()) { m_inst = NULL; return; }

    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    local_timer.start();
    #endif

    get_results_for_instantiation();
    if (!m_results) {
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    if (m_inst->prod && (thisAgent->d_cycle_count == m_inst->prod->last_duplicate_dc) && (m_inst->prod->duplicate_chunks_this_cycle >= max_dupes))
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_max_dupes, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            thisAgent->outputManager->printa_sf(thisAgent, "         Rule that has reached the max-dupes limit: %y\n", m_inst->prod_name);
        }
        thisAgent->explanationMemory->increment_stat_max_dupes();
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    /* Set up a new instantiation and ID for this chunk's refracted instantiation */
    init_instantiation(thisAgent, m_chunk_inst, NULL);
    l_clean_up_id = m_chunk_inst->i_id;

    m_chunk_inst->tested_local_negation     = m_inst->tested_local_negation;
    m_chunk_inst->creates_deep_copy         = m_inst->creates_deep_copy;
    m_chunk_inst->tested_LTM                = m_inst->tested_LTM;
    m_chunk_inst->tested_quiescence         = m_inst->tested_quiescence;

    dprint_header(DT_MILESTONES, PrintBoth, "EBC learning new rule with id %u for match of %y (i%u)\n", m_chunk_inst->i_id, m_inst->prod_name, inst->i_id);
    dprint(DT_VARIABLIZATION_MANAGER, "EBC learning new rule with id %u for match of %y (i%u):\n%5\n...which produced results...\n%6\n", m_chunk_inst->i_id, inst->prod_name, inst->i_id, inst->top_of_instantiated_conditions, inst->preferences_generated, NULL, m_results);
    thisAgent->explanationMemory->add_chunk_record(m_inst);
    thisAgent->explanationMemory->increment_stat_chunks_attempted();

    /* Set allow_bottom_up_chunks to false for all higher goals to prevent chunking */
    {
        Symbol* g;
        for (g = m_inst->match_goal->id->higher_goal; g && g->id->allow_bottom_up_chunks; g = g->id->higher_goal)
        {
            g->id->allow_bottom_up_chunks = false;
        }
    }

    /* Determine which WMEs in the topstate were relevent to problem-solving */
    m_correctness_issue_possible = false;
    m_tested_deep_copy = false;
    m_tested_local_negation = false;
    m_tested_ltm_recall = false;
    m_tested_quiescence = false;

    perform_dependency_analysis();

    /* Collect the grounds into the chunk condition lists */
    create_initial_chunk_condition_lists();

    /* If there aren't any conditions, abort chunk */
    if (!m_lhs)
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_no_conditions, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            thisAgent->outputManager->printa_sf(thisAgent, "\nRule firing that led to invalid chunk: %y\n", m_inst->prod_name);
        }
        thisAgent->explanationMemory->increment_stat_no_grounds();
        thisAgent->explanationMemory->cancel_chunk_record();
        if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
        {
            thisAgent->stop_soar = true;
            thisAgent->reason_for_stopping = "Chunking issue detected:  Rule learned had no conditions.";
        }
        clean_up(l_clean_up_id, lLocalTimerPtr);
        return;
    }
    dprint(DT_MILESTONES, "Dependency analysis complete.  Unified chunk conditions built for chunk id %u based on firing of %y (i %u)\n", m_chunk_inst->i_id, inst->prod_name, inst->i_id);
    dprint(DT_VARIABLIZATION_MANAGER, "Starting conditions from dependency analysis: \n%1", m_lhs);

    /* Determine if we create a justification or chunk */
    m_rule_type = m_learning_on_for_instantiation ? ebc_chunk : ebc_justification;

    /* Apply EBC correctness and (<s> ^quiescence t) filters that prevents rule learning */
    if ((m_tested_local_negation && !ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS]) ||
        (m_tested_ltm_recall && !ebc_settings[SETTING_EBC_ALLOW_OPAQUE])
        || m_tested_quiescence)
    {
        m_correctness_issue_possible = true;
    }

    /* Apply EBC correctness filters */
    if ((m_rule_type == ebc_chunk) && m_correctness_issue_possible)
    {
        m_rule_type = ebc_justification;
    }

    if ((m_rule_type == ebc_justification) && !thisAgent->explanationMemory->isRecordingJustifications())
    {
        thisAgent->explanationMemory->cancel_chunk_record();
    }
    if (m_tested_local_negation) thisAgent->explanationMemory->increment_stat_tested_local_negation(m_rule_type);
    if (m_tested_ltm_recall) thisAgent->explanationMemory->increment_stat_tested_ltm_recall(m_rule_type);
    if (m_tested_quiescence) thisAgent->explanationMemory->increment_stat_tested_quiescence();

    /* Create the name of the rule based on the type and circumstances of the problem-solving */
    set_up_rule_name();

    thisAgent->explanationMemory->add_result_instantiations(m_inst, m_results);

    if (ebc_settings[SETTING_EBC_LEARNING_ON] && (m_rule_type == ebc_chunk))
    {
        thisAgent->symbolManager->reset_variable_generator(m_lhs, NIL);
        variablize_condition_list(m_lhs);
        dprint(DT_VARIABLIZATION_MANAGER, "Conditions after variablizing: \n%1", m_lhs);
        merge_conditions();
        m_rhs = variablize_results_into_actions();
    } else {
        update_identities_in_condition_list(m_lhs);
        m_rhs = convert_results_into_actions();
    }

    /* Add isa_goal tests for first conditions seen with a goal identifier */
    add_goal_or_impasse_tests();

    /* Validate connectedness of chunk, repair if necessary and then re-order conditions to reduce match costs */
    thisAgent->name_of_production_being_reordered = m_prod_name->sc->name;
    if (m_rule_type == ebc_chunk)
    {
        lChunkValidated = reorder_and_validate_chunk();
        dprint(DT_VARIABLIZATION_MANAGER, "Variablized rule after re-ordering and repair:\n%1\n-->\n%2", m_lhs, m_rhs);
    }

    /* CBC: Sample chunk generation for confidence update. Make it a justification if not confident. (bws, 07/17) */
    /*if (m_rule_type == ebc_chunk) {
    	// Sample the rule
    	try {
    	std::string str = get_rule_str(thisAgent,m_lhs,m_rhs);
    	thisAgent->outputManager->printa(thisAgent, str.c_str());
    	//
    	uint32_t rule_hash = std::hash<std::string>{}(str);//get_rule_hash(thisAgent, m_lhs, m_rhs);
    	if (m_cbc_chunk_counts.find(rule_hash) == m_cbc_chunk_counts.end())
    		m_cbc_chunk_counts[rule_hash] = 1;
    	else
    		m_cbc_chunk_counts[rule_hash]++;

    	// Rebuild the rule as a justification if not confident in the chunk
    	if (m_cbc_chunk_counts[rule_hash] < confidence_threshold) {
    		std::ostringstream message;
    		message << "\nChunk confidence at " << std::to_string(m_cbc_chunk_counts[rule_hash]) << " < " << std::to_string(confidence_threshold) << ". Rebuilding as justification:";
    		thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
    		xml_generate_verbose(thisAgent, message.str().c_str());

    		lChunkValidated = false;
    	}
    	}
    	catch(const std::runtime_error& re) {
    		thisAgent->outputManager->printa(thisAgent, "Runtime error: ");
    		thisAgent->outputManager->printa(thisAgent, re.what());
    	}
    	catch(const std::exception& ex)
    	{
    		thisAgent->outputManager->printa(thisAgent, "Error occurred: ");
    		thisAgent->outputManager->printa(thisAgent, ex.what());
    	}
    	catch(...)
    	{
    	    // catch any other errors (that we have no information about)
    		thisAgent->outputManager->printa(thisAgent, "Unknown error occurred.\n");
    	    std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    	}
    }*/

    /* Handle rule learning failure.  With the addition of rule repair, this should only happen when there
     * is a repair failure.  Unless there's a bug in the repair code, all rules should be reparable. */
    if (!lChunkValidated)
    {
        if (m_rule_type == ebc_chunk)
        {
            /* Could not re-order chunk, so we create a justification for the results instead */
            m_rule_type = ebc_justification;
            lRevertedChunk = true;
            thisAgent->symbolManager->symbol_remove_ref(&m_prod_name);
            m_prod_name = generate_name_for_new_rule();
            m_prod_type = JUSTIFICATION_PRODUCTION_TYPE;
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Soar will learn a justification instead of a variablized rule.");
            }
        }
    }

    /* Perform re-instantiation
     *
     * The next step in EBC will create an instantiated list of conditions based on the
     * variablized conditions.  This is used as the instantiation for the chunk at the goal
     * level (m_chunk_inst).  After being submitted to the RETE, it is then chunked over
     * if necessary for bottom up chunking. */

    if (ebc_settings[SETTING_EBC_LEARNING_ON] && ((m_rule_type == ebc_chunk) || lRevertedChunk))
    {
        l_inst_top = reinstantiate_current_rule();
        l_inst_bottom = l_inst_top;
        while (l_inst_bottom->next) l_inst_bottom = l_inst_bottom->next;
    } else {
        copy_condition_list(thisAgent, m_lhs, &l_inst_top, &l_inst_bottom, false, false, false, false);
    }

    /* Create the production that will be added to the RETE */
    m_prod = make_production(thisAgent, m_prod_type, m_prod_name, m_inst->prod ? m_inst->prod->original_rule_name : m_inst->prod_name->sc->name, &m_lhs, &m_rhs, false, NULL);
    m_prod->naming_depth = m_chunk_inst->prod_naming_depth;

    if (m_inst->prod && m_inst->prod->explain_its_chunks)
    {
        m_prod->explain_its_chunks = true;
    }

    m_prod_name = NULL;     /* Production struct is now responsible for the production name */

    /* Fill out the instantiation for the chunk */
    m_chunk_inst->top_of_instantiated_conditions    = l_inst_top;
    m_chunk_inst->bottom_of_instantiated_conditions = l_inst_bottom;
    m_chunk_inst->prod                              = m_prod;
    m_chunk_inst->prod_name                         = m_prod->name;
    thisAgent->symbolManager->symbol_add_ref(m_chunk_inst->prod_name);
    m_chunk_inst->in_ms                             = true;                     /* set true for now, we'll find out later... */
    m_chunk_inst->in_newly_created                  = true;
    m_chunk_inst->tested_local_negation             = m_tested_local_negation;
    m_chunk_inst->creates_deep_copy                 = m_tested_deep_copy;
    m_chunk_inst->tested_LTM                        = m_tested_ltm_recall;
    m_chunk_inst->tested_quiescence                 = m_tested_quiescence;

    find_match_goal(thisAgent, m_chunk_inst);
    make_clones_of_results();
    finalize_instantiation(thisAgent, m_chunk_inst, true, m_inst, true, true);

    dprint(DT_VARIABLIZATION_MANAGER, "Production adding to RETE: \n%4", m_lhs, m_rhs);
    dprint(DT_VARIABLIZATION_MANAGER, "Instantiation adding to RETE: \n%5", m_chunk_inst->top_of_instantiated_conditions, m_chunk_inst->preferences_generated);

    /* Add to RETE */
    bool lAddedSuccessfully = add_chunk_to_rete();

    if (lAddedSuccessfully)
    {
        /* --- Add chunk instantiation to list of newly generated instantiations --- */
        m_chunk_inst->next = (*new_inst_list);
        (*new_inst_list) = m_chunk_inst;

        /* Clean up.  (Now that m_chunk_inst s on the list of insts to be asserted, we
         *             set it to to null because so that clean_up() won't delete it.) */
        m_chunk_inst = NULL;
        clean_up(l_clean_up_id, lLocalTimerPtr);

        if ((*new_inst_list)->match_goal_level > TOP_GOAL_LEVEL)
        {
            /* Initiate bottom-up chunking, i.e. tell EBC to learn a rule for the chunk instantiation,
             * which is what would have been created if the chunk had fired on its goal level */
            dprint(DT_MILESTONES, "Starting bottom-up call to learn_ebc_rule() from %y\n", (*new_inst_list)->prod_name);
            set_learning_for_instantiation(*new_inst_list);
            learn_rule_from_instance(*new_inst_list, new_inst_list);
            dprint(DT_MILESTONES, "Finished bottom-up call to learn_ebc_rule()\n");
        }
    } else {
        /* Clean up failed chunk completely*/
        dprint(DT_DEALLOCATE_INST, "Rule addition failed.  Deallocating chunk instantiation.\n");
        m_chunk_inst->in_newly_created = false;
        if (ebc_settings[SETTING_EBC_LEARNING_ON])
        {
            clean_up_identities();
        }
        remove_chunk_instantiation();
        clean_up(l_clean_up_id, lLocalTimerPtr);
    }
}

void Explanation_Based_Chunker::clean_up (uint64_t pClean_up_id, soar_timer* pTimer)
{
    thisAgent->explanationMemory->end_chunk_record();
    if (m_chunk_inst)
    {
        thisAgent->memoryManager->free_with_pool(MP_instantiation, m_chunk_inst);
        m_chunk_inst = NULL;
    }
    if (m_lhs)
    {
        deallocate_condition_list(thisAgent, m_lhs);
    }
    if (m_prod_name)
    {
        dprint_header(DT_MILESTONES, PrintAfter, "chunk_instantiation() done building and cleaning up for chunk %y.\n", m_prod_name);
        thisAgent->symbolManager->symbol_remove_ref(&m_prod_name);
    }
    m_inst                              = NULL;
    m_results                           = NULL;
    m_extra_results                     = NULL;
    m_lhs                         = NULL;
    m_rhs                               = NULL;
    m_prod                              = NULL;
    m_chunk_inst                        = NULL;
    m_prod_name                         = NULL;
    m_rule_type                         = ebc_no_rule;
    m_failure_type                      = ebc_success;

    clear_symbol_identity_map();
    if (ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        clear_id_to_identity_map();
        clean_up_identities();
        clear_cached_constraints();
    }
    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    pTimer->stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(*pTimer);
    #endif

}

