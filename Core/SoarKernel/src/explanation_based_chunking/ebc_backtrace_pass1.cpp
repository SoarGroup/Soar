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

    bool id_has_set, attr_has_set, value_has_set;
    if (trace_cond)
    {
        id_has_set = o_ids_to_replace.id && trace_cond->data.tests.id_test->eq_test->identity_set;
        attr_has_set = o_ids_to_replace.attr && trace_cond->data.tests.attr_test->eq_test->identity_set;
        value_has_set = o_ids_to_replace.value && trace_cond->data.tests.value_test->eq_test->identity_set;
    } else {
        id_has_set = attr_has_set = value_has_set = false;
    }
    /* --- scan through conditions, collect grounds and locals --- */
    for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            // Substitute in identity sets for any v_identity
            if (!condition_is_operational(c, grounds_level))
            {
                dprint(DT_BACKTRACE1, "... adding cond %l to locals...\n", c);
                add_to_locals(c);
            } else {
                dprint(DT_BACKTRACE1, "... skipping operational cond %l...\n", c);
            }
        }
        else
        {
            dprint(DT_BACKTRACE1, "... skipping negative cond %l...\n", c);
            /* Will need to update identity sets of negated.  Maybe we still do this part in pass 1 */
//            dprint(DT_BACKTRACE1, "Adding negated condition %y (i%u): %l\n", c->inst->prod_name, c->inst->i_id, c);
//            add_to_chunk_cond_set(&negated_set, make_chunk_cond_for_negated_condition(c));
        }
    }

    m_current_bt_inst_id = last_bt_inst_id;
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

        thisAgent->outputManager->set_print_test_format(true, true);
        dprint(DT_BACKTRACE1, "Tracing through local condition of of instantiation %y (i%u): %l\n", cond->inst->prod_name, cond->inst->i_id, cond);
        thisAgent->outputManager->clear_print_test_format();

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
