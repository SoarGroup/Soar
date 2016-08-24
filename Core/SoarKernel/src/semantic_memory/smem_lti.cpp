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

void SMem_Manager::lti_from_test(test t, std::set<Symbol*>* valid_ltis)
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
                lti_from_test(static_cast<test>(c->first), valid_ltis);
            }
        }
    }
}

void SMem_Manager::lti_from_rhs_value(rhs_value rv, std::set<Symbol*>* valid_ltis)
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
            lti_from_rhs_value(static_cast<rhs_value>(c->first), valid_ltis);
        }
    }
}

// Searches smem db for the lti id for a soar_letter/number pair (or NIL if failure)
smem_lti_id SMem_Manager::lti_get_id(char name_letter, uint64_t name_number)
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
smem_lti_id SMem_Manager::lti_add_id(char name_letter, uint64_t name_number)
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

// Creates an LTI for a STI that does not have one
void SMem_Manager::link_sti_to_lti(Symbol* id)
{
    if ((id->is_identifier()) && (id->id->smem_lti != NIL))
    {
        // if no smem_lti, then we should not have a corresponding lti!
        assert(!lti_get_id(id->id->name_letter, id->id->name_number));

        id->id->smem_lti = lti_add_id(id->id->name_letter, id->id->name_number);
        id->id->smem_time_id = thisAgent->EpMem->epmem_stats->time->get_value();
        id->id->smem_valid = thisAgent->EpMem->epmem_validation;
        epmem_schedule_promotion(thisAgent, id);
    }
}

void SMem_Manager::reset_id_counters()
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

