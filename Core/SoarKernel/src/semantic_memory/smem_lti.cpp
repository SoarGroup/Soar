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
        if ((t->data.referent->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (t->data.referent->id->LTI_ID != NIL))
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
        if ((sym->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (sym->id->LTI_ID != NIL))
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
smem_lti_id SMem_Manager::lti_exists(uint64_t pLTI_ID)
{
    smem_lti_id return_val = NIL;

    if (smem_db->get_status() != soar_module::disconnected)
    {   // getting lti ids requires an open semantic database
        // soar_letter=? AND number=?
        smem_stmts->lti_id_exists->bind_int(1, static_cast<uint64_t>(pLTI_ID));

        if (smem_stmts->lti_id_exists->execute() == soar_module::row)
        {
            return_val = smem_stmts->lti_id_exists->column_int(0);
        }

        smem_stmts->lti_id_exists->reinitialize();
    }
    return return_val;
}

// Searches smem db for the lti id for a soar_letter/number pair (or NIL if failure)
smem_lti_id SMem_Manager::get_max_lti_id()
{
    smem_lti_id return_val = 0;

    if (smem_db->get_status() != soar_module::disconnected)
    {
        if (smem_stmts->lti_id_max->execute() == soar_module::row)
        {
            return_val = smem_stmts->lti_id_max->column_int(0);
        }

        smem_stmts->lti_id_max->reinitialize();
    }
    return return_val;
}

// adds a new lti id for a soar_letter/number pair
smem_lti_id SMem_Manager::add_new_lti_id()
{
    // add lti_id, total_augmentations, activation_value, activations_total, activations_last, activations_first
    smem_stmts->lti_add->bind_int(1, static_cast<uint64_t>(++lti_id_counter));
    smem_stmts->lti_add->bind_int(2, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_double(3, static_cast<double>(0));
    smem_stmts->lti_add->bind_int(4, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_int(5, static_cast<uint64_t>(0));
    smem_stmts->lti_add->bind_int(6, static_cast<uint64_t>(0));
    smem_stmts->lti_add->execute(soar_module::op_reinit);

//    assert(lti_id_counter == smem_db->last_insert_rowid());

    smem_stats->chunks->set_value(smem_stats->chunks->get_value() + 1);

    return lti_id_counter;
}

// Creates an LTI for a STI that does not have one
void SMem_Manager::link_sti_to_lti(Symbol* id)
{
    if (id->is_identifier())
    {
        if (id->id->LTI_ID == NIL)
        {
            id->id->LTI_ID = add_new_lti_id();
            id->id->smem_valid = thisAgent->EpMem->epmem_validation;
        } else {
            /* Already linked?  Should not be possible */
            assert(lti_exists(id->id->LTI_ID));
            assert(false);
        }
    } else {

        id->id->LTI_ID = 0;
        id->id->smem_valid = 0;
    }
}

void SMem_Manager::reset_id_counters()
{
    lti_id_counter = get_max_lti_id();
}

