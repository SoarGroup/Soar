/*************************************************************************
 *
 *  file:  backtrace.cpp
 *
 * =======================================================================
 *  Backtracing structures and routines.  See also explain.c
 * =======================================================================
 */

/* ====================================================================
                        Backtracing routines
   ==================================================================== */

#include "ebc.h"
#include "ebc_timers.h"
#include "ebc_identity_sets.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "mem.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "instantiation.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"
#include "xml.h"

#include <stdlib.h>

using namespace soar_TraceNames;

void Explanation_Based_Chunker::btpass1_backtrace_through_instantiation(instantiation* inst,
                                     goal_stack_level grounds_level,
                                     condition* trace_cond,
                                     const identity_quadruple o_ids_to_replace,
                                     const rhs_quadruple rhs_funcs,
                                     uint64_t bt_depth,
                                     BTSourceType bt_type)
{

    condition* c;
    uint64_t last_bt_inst_id = m_current_bt_inst_id;
    m_current_bt_inst_id = inst->i_id;

    dprint(DT_BACKTRACE1, "Backtracing %y :i%u (matched level %d):\n", inst->prod_name, inst->i_id, static_cast<int64_t>(grounds_level));

    ++bt_depth;
    if (inst->explain_depth > bt_depth)
    {
        inst->explain_depth = bt_depth;
    }

    /* --- if the instantiation has already been BT'd, don't repeat it --- */
    if (inst->backtrace_number == backtrace_number)
    {
        dprint(DT_BACKTRACE1, "... already backtraced through.\n");
        m_current_bt_inst_id = last_bt_inst_id;
        return;
    }

    inst->backtrace_number = backtrace_number;

    /* We must backtrace through OSK even if we're not adding OSK because prohibits also use this mechanism */
    if (inst->OSK_prefs)
    {
        btpass1_backtrace_through_OSK(inst->OSK_prefs, grounds_level, inst->explain_depth);
    }
    if (inst->OSK_proposal_prefs)
    {
        btpass1_backtrace_through_OSK(inst->OSK_proposal_prefs, grounds_level, inst->explain_depth);
    }
    Symbol* thisID, *value;

    if (trace_cond)
    {
        if (o_ids_to_replace.id && (trace_cond->data.tests.id_test->eq_test->id_set_tc_num == id_set_pass1_tc) && trace_cond->data.tests.id_test->eq_test->identity_set)
        {
            identitySets->add_id_set_propagation(o_ids_to_replace.id, trace_cond->data.tests.id_test->eq_test->identity_set);
        }
        if (o_ids_to_replace.attr && (trace_cond->data.tests.attr_test->eq_test->id_set_tc_num == id_set_pass1_tc) && trace_cond->data.tests.attr_test->eq_test->identity_set)
        {
            identitySets->add_id_set_propagation(o_ids_to_replace.attr, trace_cond->data.tests.attr_test->eq_test->identity_set);
        }
        if (o_ids_to_replace.value && (trace_cond->data.tests.value_test->eq_test->id_set_tc_num == id_set_pass1_tc) && trace_cond->data.tests.value_test->eq_test->identity_set)
        {
            identitySets->add_id_set_propagation(o_ids_to_replace.value, trace_cond->data.tests.value_test->eq_test->identity_set);
        }
    }
    /* --- scan through conditions, collect grounds and locals --- */
    bool isOperational;
    for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            // Substitute in identity sets for any v_identity
            isOperational = condition_is_operational(c, grounds_level);
            if (!isOperational)
            {
                dprint(DT_BACKTRACE1, "... adding cond %l to locals...\n", c);
                add_to_locals(c);
            } else {
                dprint(DT_BACKTRACE1, "... skipping operational cond %l...\n", c);
            }
            dprint(DT_BACKTRACE1, "... adding identity sets to local cond %l...\n", c);
            test id_test = c->data.tests.id_test->eq_test;
            test attr_test = c->data.tests.attr_test->eq_test;
            test value_test = c->data.tests.value_test->eq_test;
            if (id_test->identity && (id_test->id_set_tc_num != id_set_pass1_tc))
            {
                id_test->identity_set = identitySets->get_or_create_id_set(id_test->identity);
                id_test->id_set_tc_num = id_set_pass1_tc;
            }
            if (attr_test->identity && (attr_test->id_set_tc_num != id_set_pass1_tc))
            {
                attr_test->identity_set = identitySets->get_or_create_id_set(attr_test->identity);
                attr_test->id_set_tc_num = id_set_pass1_tc;
            }
            if (value_test->identity && (value_test->id_set_tc_num != id_set_pass1_tc))
            {
                value_test->identity_set = identitySets->get_or_create_id_set(value_test->identity);
                value_test->id_set_tc_num = id_set_pass1_tc;
            }
        }
    }
    update_remaining_identity_sets_in_condlist(inst->top_of_instantiated_conditions, inst);
    identitySets->clear_inst_id_sets();
    m_current_bt_inst_id = last_bt_inst_id;
}

void Explanation_Based_Chunker::update_remaining_identity_sets_in_test(test t, instantiation* pInst)
{
    cons* c;
    switch (t->type)
        {
            case GOAL_ID_TEST:
            case IMPASSE_ID_TEST:
            case SMEM_LINK_UNARY_TEST:
            case SMEM_LINK_UNARY_NOT_TEST:
            case DISJUNCTION_TEST:
            case CONJUNCTIVE_TEST:
                for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                {
                    update_remaining_identity_sets_in_test(static_cast<test>(c->first), pInst);
                }
                break;
            default:
                if (t->identity && (t->id_set_tc_num != id_set_pass1_tc))
                {
                    if (pInst->bt_identity_set_mappings->find(t->identity) == pInst->bt_identity_set_mappings->end())
                    {
                        for (auto iter = pInst->bt_identity_set_mappings->begin(); iter != pInst->bt_identity_set_mappings->end(); ++iter)
                        {
                            dprint_noprefix(DT_BACKTRACE1, "%u -> %u\n", iter->first, iter->second);
                        }
                    }
                    t->identity_set = pInst->bt_identity_set_mappings->at(t->identity);
                    t->id_set_tc_num = id_set_pass1_tc;
                }
                break;
        }
}

void Explanation_Based_Chunker::update_remaining_identity_sets_in_condlist(condition* pCondTop, instantiation* pInst)
{
    condition* pCond;

    for (pCond = pCondTop; pCond != NIL; pCond = pCond->next)
    {
        if (pCond->type == POSITIVE_CONDITION)
        {
            update_remaining_identity_sets_in_test(pCond->data.tests.id_test, pInst);
            update_remaining_identity_sets_in_test(pCond->data.tests.attr_test, pInst);
            update_remaining_identity_sets_in_test(pCond->data.tests.value_test, pInst);
        } else {
            update_remaining_identity_sets_in_condlist(pCond->data.ncc.top, pInst);
        }
    }
}

/* ---------------------------------------------------------------
                             Trace Locals

   This routine backtraces through locals, and keeps doing so until
   there are no more locals to BT.
--------------------------------------------------------------- */
void Explanation_Based_Chunker::btpass1_backtrace_through_OSK(cons* pOSKPrefList, goal_stack_level grounds_level, uint64_t lExplainDepth)
{
    cons* l_OSK_prefs;
    preference* p;
    for (l_OSK_prefs = pOSKPrefList; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
    {
        p = static_cast<preference_struct*>(l_OSK_prefs->first);
        dprint(DT_BACKTRACE1, "Tracing through OSK pref %p for instantiation \n", p);
        btpass1_backtrace_through_instantiation(p->inst, grounds_level, NULL, p->identities, p->rhs_funcs, lExplainDepth, BT_OSK);
    }
}
void Explanation_Based_Chunker::btpass1_trace_locals(goal_stack_level grounds_level)
{

    /* mvp 5-17-94 */
    cons* c, *l_OSK_prefs;
    condition* cond;
    preference* bt_pref, *p;

    dprint(DT_BACKTRACE1, "Tracing locals...\n");

    while (locals)
    {
        c = locals;
        locals = locals->rest;
        cond = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);

        dprint(DT_BACKTRACE1, "Tracing through local condition of of instantiation %y (i%u): %l\n", cond->inst->prod_name, cond->inst->i_id, cond);

        bt_pref = NULL;
        if (cond->bt.trace)
        {
            bt_pref = (cond->bt.trace->level != grounds_level + 1) ? find_clone_for_level(cond->bt.trace, static_cast<goal_stack_level>(grounds_level + 1)) : cond->bt.trace;
        }
        if (bt_pref)
        {
            btpass1_backtrace_through_instantiation(bt_pref->inst, grounds_level, cond, bt_pref->identities, bt_pref->rhs_funcs, cond->inst->explain_depth, BT_Normal);

        }
        /* --- for augmentations of the local goal id, either handle the "^quiescence t" test or discard it --- */
        Symbol* thisID = cond->data.tests.id_test->eq_test->data.referent;
        Symbol* thisAttr = cond->data.tests.attr_test->eq_test->data.referent;
        Symbol* thisValue = cond->data.tests.value_test->eq_test->data.referent;
        if (thisID->id->isa_goal)
        {
            /* Can skip this test in second pass and just use flags set here */
            if (cond->inst->tested_quiescence ||
               ((thisAttr == thisAgent->symbolManager->soarSymbols.quiescence_symbol) &&
                (thisValue == thisAgent->symbolManager->soarSymbols.t_symbol) &&
                (! cond->test_for_acceptable_preference)))
            {
                m_tested_quiescence = true;
                cond->inst->tested_quiescence = true;
            }
            continue;
        }

        dprint(DT_BACKTRACE1, "--! Local condition removed (no trace): %l.\n", cond);

    } /* end of while locals loop */
}
