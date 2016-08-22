/*
 * smem_lti.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_stats.h"

#include "episodic_memory.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"

void SMem_Manager::_smem_lti_from_test(test t, std::set<Symbol*>* valid_ltis)
{
    if (!t)
    {
        return;
    }

    if (t->type == EQUALITY_TEST)
    {
        if ((t->data.referent->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (t->data.referent->id->smem_lti != NIL))
        {
            valid_ltis->insert(t->data.referent);
        }

        return;
    }

    {
        if (t->type == CONJUNCTIVE_TEST)
        {
            for (cons* c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                _smem_lti_from_test(static_cast<test>(c->first), valid_ltis);
            }
        }
    }
}

void SMem_Manager::_smem_lti_from_rhs_value(rhs_value rv, std::set<Symbol*>* valid_ltis)
{
    if (rhs_value_is_symbol(rv))
    {
        Symbol* sym = rhs_value_to_symbol(rv);
        if ((sym->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (sym->id->smem_lti != NIL))
        {
            valid_ltis->insert(sym);
        }
    }
    else
    {
        list* fl = rhs_value_to_funcall_list(rv);
        for (cons* c = fl->rest; c != NIL; c = c->rest)
        {
            _smem_lti_from_rhs_value(static_cast<rhs_value>(c->first), valid_ltis);
        }
    }
}

// gets the lti id for an existing lti soar_letter/number pair (or NIL if failure)
smem_lti_id SMem_Manager::smem_lti_get_id(char name_letter, uint64_t name_number)
{
    smem_lti_id return_val = NIL;

    if (smem_db->get_status() != soar_module::disconnected)
    {   // getting lti ids requires an open semantic database
        // soar_letter=? AND number=?
        smem_stmts->lti_get->bind_int(1, static_cast<uint64_t>(name_letter));
        smem_stmts->lti_get->bind_int(2, static_cast<uint64_t>(name_number));

        if (smem_stmts->lti_get->execute() == soar_module::row)
        {
            return_val = smem_stmts->lti_get->column_int(0);
        }

        smem_stmts->lti_get->reinitialize();
    }
    return return_val;
}

// adds a new lti id for a soar_letter/number pair
smem_lti_id SMem_Manager::smem_lti_add_id(char name_letter, uint64_t name_number)
{
    smem_lti_id return_val;

    // create lti: soar_letter, number, total_augmentations, activation_value, activations_total, activations_last, activations_first
    smem_stmts->lti_add->bind_int(1, static_cast<uint64_t>(name_letter));
    smem_stmts->lti_add->bind_int(2, static_cast<uint64_t>(name_number));
    smem_stmts->lti_add->bind_int(3, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_double(4, static_cast<double>(0));
    smem_stmts->lti_add->bind_int(5, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_int(6, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_int(7, static_cast<uint64_t>(0));
    smem_stmts->lti_add->execute(soar_module::op_reinit);

    return_val = static_cast<smem_lti_id>(smem_db->last_insert_rowid());

    // increment stat
    smem_stats->chunks->set_value(smem_stats->chunks->get_value() + 1);

    return return_val;
}

/* I don't know how important the inline was for Nate to include it, so I just made a copy
   of that function for use in other files. */
void SMem_Manager::smem_lti_soar_promote_STI(Symbol* id)
{
    assert(id->is_identifier());
    if (id->id->smem_lti == NIL)
    {
        // try to find existing lti
        id->id->smem_lti = smem_lti_get_id(id->id->name_letter, id->id->name_number);

        // if doesn't exist, add
        if (id->id->smem_lti == NIL)
        {
            id->id->smem_lti = smem_lti_add_id(id->id->name_letter, id->id->name_number);
            id->id->smem_time_id = thisAgent->EpMem->epmem_stats->time->get_value();
            id->id->smem_valid = thisAgent->EpMem->epmem_validation;
            epmem_schedule_promotion(thisAgent, id);
        }
    }
}

// makes a non-long-term identifier into a long-term identifier
void SMem_Manager::smem_lti_soar_add(Symbol* id)
{
    if ((id->is_identifier()) &&
            (id->id->smem_lti == NIL))
    {
        // try to find existing lti
        id->id->smem_lti = smem_lti_get_id(id->id->name_letter, id->id->name_number);

        // if doesn't exist, add
        if (id->id->smem_lti == NIL)
        {
            id->id->smem_lti = smem_lti_add_id(id->id->name_letter, id->id->name_number);

            id->id->smem_time_id = thisAgent->EpMem->epmem_stats->time->get_value();
            id->id->smem_valid = thisAgent->EpMem->epmem_validation;
            epmem_schedule_promotion(thisAgent, id);
        }
    }
}

// returns a reference to an lti
Symbol* SMem_Manager::smem_lti_soar_make(smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level)
{
    Symbol* return_val;

    // try to find existing
    return_val = thisAgent->symbolManager->find_identifier(name_letter, name_number);

    // otherwise create
    if (return_val == NIL)
    {
        return_val = thisAgent->symbolManager->make_new_identifier(name_letter, level, name_number);
    }
    else
    {
        thisAgent->symbolManager->symbol_add_ref(return_val);

        if ((return_val->id->level == SMEM_LTI_UNKNOWN_LEVEL) && (level != SMEM_LTI_UNKNOWN_LEVEL))
        {
            return_val->id->level = level;
            return_val->id->promotion_level = level;
        }
    }

    // set lti field irrespective
    return_val->id->smem_lti = lti;

    return return_val;
}

void SMem_Manager::smem_reset_id_counters()
{
    if (smem_db->get_status() == soar_module::connected)
    {
        // soar_letter, max
        while (smem_stmts->lti_max->execute() == soar_module::row)
        {
            uint64_t name_letter = static_cast<uint64_t>(smem_stmts->lti_max->column_int(0));
            uint64_t letter_max = static_cast<uint64_t>(smem_stmts->lti_max->column_int(1));

            // shift to alphabet
            name_letter -= static_cast<uint64_t>('A');

            // get count
            uint64_t* letter_ct = thisAgent->symbolManager->get_id_counter(name_letter);

            // adjust if necessary
            if ((*letter_ct) <= letter_max)
            {
                (*letter_ct) = (letter_max + 1);
            }
        }

        smem_stmts->lti_max->reinitialize();
    }
}

