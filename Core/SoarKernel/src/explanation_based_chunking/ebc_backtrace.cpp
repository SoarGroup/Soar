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

/* ====================================================================

                            Backtracing

   Three sets of conditions are maintained during backtracing:  locals,
   grounds and negateds.  Negateds are kept separately throughout backtracing,
   and ground at the very end.  Note that this means during backtracing,
   the grounds and locals are all instantiated top-level positive conditions,
   so they all have a bt.wme_ on them.

   In order to avoid backtracing through the same instantiation twice,
   we mark each instantiation as we BT it, by setting
   inst->backtrace_number = backtrace_number (this is a global variable
   which gets incremented each time we build a chunk).

   The add_to_grounds() and add_to_locals()
   macros below are used to add conditions to these sets.  The negated
   conditions are maintained in the chunk_cond_set "negated_set."

==================================================================== */

void Explanation_Based_Chunker::add_to_grounds(condition* cond)
{
    dprint(DT_BACKTRACE, "--> Ground condition added: %l.\n", cond);
    if ((cond)->bt.wme_->tc != grounds_tc)
    {
        (cond)->bt.wme_->tc = grounds_tc;
        cond->bt.wme_->chunker_bt_last_ground_cond = cond;
    }
    if (cond->bt.wme_->chunker_bt_last_ground_cond != cond)
    {
        add_singleton_unification_if_needed(cond);
    }
    push(thisAgent, (cond), grounds);
}

void Explanation_Based_Chunker::add_to_locals(condition* cond)
{
    dprint(DT_BACKTRACE, "--> Local condition added: %l.\n", cond);
    push(thisAgent, (cond), locals);
}

/* -------------------------------------------------------------------
                     Backtrace Through Instantiation

   This routine BT's through a given instantiation.  The general method
   is as follows:

     1. If we've already BT'd this instantiation, then skip it.
     2. Mark the TC (in the instantiated conditions) of all higher goal
        ids tested in top-level positive conditions
     3. Scan through the instantiated conditions; add each one to the
        appropriate set (locals, grounds, negated_set).
------------------------------------------------------------------- */

/* mvp 5-17-94 */
void print_consed_list_of_conditions(agent* thisAgent, cons* c, int indent)
{
    for (; c != NIL; c = c->rest)
    {
        if (thisAgent->outputManager->get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE - 20)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "\n      ");
        }

        /* mvp 5-17-94 */
        thisAgent->outputManager->print_spaces(thisAgent, indent);
        print_condition(thisAgent, static_cast<condition_struct*>(c->first));
    }
}

/* mvp 5-17-94 */
void print_consed_list_of_condition_wmes(agent* thisAgent, cons* c, int indent)
{
    for (; c != NIL; c = c->rest)
    {
        if (thisAgent->outputManager->get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE - 20)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "\n      ");
        }

        /* mvp 5-17-94 */
        thisAgent->outputManager->print_spaces(thisAgent, indent);
        thisAgent->outputManager->printa_sf(thisAgent,  "     ");
        print_wme(thisAgent, (static_cast<condition*>(c->first))->bt.wme_);
    }
}

/* This is the wme which is causing this production to be backtraced through.
   It is NULL when backtracing for a result preference.                   */

inline bool condition_is_operational(condition* cond, goal_stack_level grounds_level)
{
    Symbol* thisID = cond->data.tests.id_test->eq_test->data.referent;
     
//    assert(thisID->id->is_sti());
//    assert(thisID->id->level <= cond->bt.level);
//
//    uint64_t idLevel = thisID->id->level;
//    uint64_t btLevel = cond->bt.level;
//    uint64_t prefLevel = cond->bt.trace ? cond->bt.trace->id->id->level : 0;

    return  (thisID->id->level <= grounds_level);
//    return (btLevel <= grounds_level);
}

void Explanation_Based_Chunker::backtrace_through_instantiation(instantiation* inst,
                                     goal_stack_level grounds_level,
                                     condition* trace_cond,
                                     const identity_quadruple o_ids_to_replace,
                                     const rhs_quadruple rhs_funcs,
                                     uint64_t bt_depth,
                                     BTSourceType bt_type)
{

    condition* c;
    cons* grounds_to_print, *locals_to_print, *negateds_to_print;
    uint64_t last_bt_inst_id = m_current_bt_inst_id;
    m_current_bt_inst_id = inst->i_id;

    dprint(DT_BACKTRACE, "Backtracing %y :i%u (matched level %d):\n", inst->prod_name, inst->i_id, static_cast<int64_t>(grounds_level));
//    dprint(DT_BACKTRACE, "           RHS identities: (%y [o%u] ^%y [o%u] %y [o%u]),\n           Matched cond: %l\n",
//        get_ovar_for_o_id(o_ids_to_replace.id),o_ids_to_replace.id,
//        get_ovar_for_o_id(o_ids_to_replace.attr),o_ids_to_replace.attr,
//        get_ovar_for_o_id(o_ids_to_replace.value), o_ids_to_replace.value, trace_cond);

//    break_if_id_matches(inst->i_id, 41);
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "... BT through instantiation of ");
        if (inst->prod)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%y\n", inst->prod_name);
        }
        else
        {
            thisAgent->outputManager->printa(thisAgent, "[Architectural Fake Instantiation]\n");
        }

        xml_begin_tag(thisAgent, kTagBacktrace);
        if (inst->prod)
        {
            xml_att_val(thisAgent, kProduction_Name, inst->prod_name);
        }
        else
        {
            xml_att_val(thisAgent, kProduction_Name, "[Architectural Fake Instantiation]");
        }

    }

    if (trace_cond)
    {
        unify_backtraced_conditions(trace_cond, o_ids_to_replace, rhs_funcs);
    }

    ++bt_depth;
    if (inst->explain_depth > bt_depth)
    {
        inst->explain_depth = bt_depth;
    }
    /* --- if the instantiation has already been BT'd, don't repeat it --- */
    if (inst->backtrace_number == backtrace_number)
    {
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {

            /* mvp 5-17-94 */
            thisAgent->outputManager->printa(thisAgent, "(We already backtraced through this instantiation.)\n");
            xml_att_val(thisAgent, kBacktracedAlready, "true");
            xml_end_tag(thisAgent, kTagBacktrace);
        }
        thisAgent->explanationMemory->increment_stat_seen_instantations_backtraced();
        dprint(DT_BACKTRACE, "... already backtraced through.\n");
        m_current_bt_inst_id = last_bt_inst_id;
        return;
    }

    inst->backtrace_number = backtrace_number;
    thisAgent->explanationMemory->add_bt_instantiation(inst, bt_type);
    thisAgent->explanationMemory->increment_stat_instantations_backtraced();

    if (inst->tested_quiescence) m_tested_quiescence = true;
    if (inst->tested_local_negation) m_tested_local_negation = true;
    if (inst->tested_LTM) m_tested_ltm_recall = true;
    if (inst->creates_deep_copy) m_tested_deep_copy = true;

    /* We must backtrace through OSK even if we're not adding OSK because prohibits also use this mechanism */
    if (inst->OSK_prefs)
    {
        backtrace_through_OSK(inst->OSK_prefs, grounds_level, inst->explain_depth);
    }
    if (inst->OSK_proposal_prefs)
    {
        backtrace_through_OSK(inst->OSK_proposal_prefs, grounds_level, inst->explain_depth);
    }
    Symbol* thisID, *value;

    for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            cache_constraints_in_cond(c);
            if (condition_is_operational(c, grounds_level))
            {
                if (c->bt.wme_->tc != grounds_tc)   /* First time we've seen something matching this wme*/
                {
                    add_to_grounds(c);
                }
                else                                        /* Another condition that matches the same wme */
                {
                    add_to_grounds(c);
                }
            } else {
                add_to_locals(c);
                add_local_singleton_unification_if_needed(c);
            }
        }
        else
        {
            dprint(DT_BACKTRACE, "Adding negated condition %y (i%u): %l\n", c->inst->prod_name, c->inst->i_id, c);
            add_to_chunk_cond_set(&negated_set, make_chunk_cond_for_negated_condition(c));
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                push(thisAgent, c, negateds_to_print);
            }
        }
    }

    /* --- scan through conditions, collect grounds and locals --- */
    grounds_to_print = NIL;
    locals_to_print = NIL;
    negateds_to_print = NIL;

    /* --- if tracing BT, print the resulting conditions, etc. --- */
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        /* mvp 5-17-94 */
        thisAgent->outputManager->printa(thisAgent, "  -->Grounds:\n");
        xml_begin_tag(thisAgent, kTagGrounds);
        print_consed_list_of_condition_wmes(thisAgent, grounds, 0);
        xml_end_tag(thisAgent, kTagGrounds);
        thisAgent->outputManager->printa(thisAgent,  "\n");
        thisAgent->outputManager->printa(thisAgent, "  -->Locals:\n");
        xml_begin_tag(thisAgent, kTagLocals);
        print_consed_list_of_condition_wmes(thisAgent, locals, 0);
        xml_end_tag(thisAgent, kTagLocals);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");
//        thisAgent->outputManager->printa(thisAgent, "  -->Negated:\n");
//        xml_begin_tag(thisAgent, kTagNegated);
//        print_consed_list_of_conditions(thisAgent, negateds_to_print, 0);
//        xml_end_tag(thisAgent, kTagNegated);
//        thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        /* mvp done */

        xml_end_tag(thisAgent, kTagBacktrace);
    }

    /* Moved these free's down to here, to ensure they are cleared even if we're not printing these lists     */

    free_list(thisAgent, grounds_to_print);
    free_list(thisAgent, locals_to_print);
    free_list(thisAgent, negateds_to_print);

    m_current_bt_inst_id = last_bt_inst_id;
}

/* ---------------------------------------------------------------
                             Trace Locals

   This routine backtraces through locals, and keeps doing so until
   there are no more locals to BT.
--------------------------------------------------------------- */
void Explanation_Based_Chunker::backtrace_through_OSK(cons* pOSKPrefList, goal_stack_level grounds_level, uint64_t lExplainDepth)
{
    cons* l_OSK_prefs;
    preference* p;
    for (l_OSK_prefs = pOSKPrefList; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
    {
        p = static_cast<preference_struct*>(l_OSK_prefs->first);
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "     Tracing through OSK preference: ");
            xml_begin_tag(thisAgent, kTagOSKPreference);
            print_preference(thisAgent, p);
        }

        dprint(DT_BACKTRACE, "Tracing through OSK pref %p for instantiation \n", p);
        backtrace_through_instantiation(p->inst, grounds_level, NULL, p->identities, p->rhs_funcs, lExplainDepth, BT_OSK);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagOSKPreference);
        }
    }
}
void Explanation_Based_Chunker::trace_locals(goal_stack_level grounds_level)
{

    /* mvp 5-17-94 */
    cons* c, *l_OSK_prefs;
    condition* cond;
    preference* bt_pref, *p;

    dprint(DT_BACKTRACE, "Tracing locals...\n");
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa(thisAgent, "\n\n*** Tracing Locals ***\n");
        xml_begin_tag(thisAgent, kTagLocals);
    }

    while (locals)
    {
        c = locals;
        locals = locals->rest;
        cond = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "\nFor local ");
            xml_begin_tag(thisAgent, kTagLocal);
            print_wme(thisAgent, cond->bt.wme_);
            thisAgent->outputManager->printa(thisAgent, " ");
        }
        thisAgent->outputManager->set_print_test_format(true, true);
        dprint(DT_BACKTRACE, "Tracing through local condition of of instantiation %y (i%u): %l\n", cond->inst->prod_name, cond->inst->i_id, cond);
        thisAgent->outputManager->clear_print_test_format();
        bt_pref = find_clone_for_level(cond->bt.trace, static_cast<goal_stack_level>(grounds_level + 1));

        if (bt_pref)
        {
            backtrace_through_instantiation(bt_pref->inst, grounds_level, cond, bt_pref->identities, bt_pref->rhs_funcs, cond->inst->explain_depth, BT_Normal);

//            if (cond->bt.OSK_prefs)
//            {
//                backtrace_through_OSK(cond->bt.OSK_prefs, grounds_level, cond->inst->explain_depth);
////                for (l_OSK_prefs = cond->bt.OSK_prefs; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
////                {
////                    p = static_cast<preference_struct*>(l_OSK_prefs->first);
////                    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
////                    {
////                        thisAgent->outputManager->printa(thisAgent, "     Tracing through OSK preference: ");
////                        xml_begin_tag(thisAgent, kTagOSKPreference);
////                        print_preference(thisAgent, p);
////                    }
////
////                    dprint(DT_BACKTRACE, "Tracing through OSK pref %p for instantiation \n", p);
////                    backtrace_through_instantiation(p->inst, grounds_level, NULL, p->identities, p->rhs_funcs, cond->inst->explain_depth, BT_OSK);
////
////                    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
////                    {
////                        xml_end_tag(thisAgent, kTagOSKPreference);
////                    }
////                }
//            }

            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                xml_end_tag(thisAgent, kTagLocal);
            }
            continue;
        }

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "...no trace, can't BT");
            // add an empty <backtrace> tag to make parsing XML easier
            xml_begin_tag(thisAgent, kTagBacktrace);
            xml_end_tag(thisAgent, kTagBacktrace);
        }
        /* --- for augmentations of the local goal id, either handle the "^quiescence t" test or discard it --- */
        Symbol* thisID = cond->data.tests.id_test->eq_test->data.referent;
        Symbol* thisAttr = cond->data.tests.attr_test->eq_test->data.referent;
        Symbol* thisValue = cond->data.tests.value_test->eq_test->data.referent;
        if (thisID->id->isa_goal)
        {
            if (cond->inst->tested_quiescence ||
               ((thisAttr == thisAgent->symbolManager->soarSymbols.quiescence_symbol) &&
                (thisValue == thisAgent->symbolManager->soarSymbols.t_symbol) &&
                (! cond->test_for_acceptable_preference)))
            {
                m_tested_quiescence = true;
                cond->inst->tested_quiescence = true;
            }
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                xml_end_tag(thisAgent, kTagLocal);
            }
            continue;
        }

        dprint(DT_BACKTRACE, "--! Local condition removed (no trace): %l.\n", cond);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagLocal);
        }

    } /* end of while locals loop */

    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        xml_end_tag(thisAgent, kTagLocals);
    }
}

void Explanation_Based_Chunker::report_local_negation(condition* c)
{
    cons* negated_to_print = NIL;
    push(thisAgent, c, negated_to_print);

    thisAgent->outputManager->printa(thisAgent, "\n*** Rule learned that used negative reasoning about local sub-state.***\n");
    xml_begin_tag(thisAgent, kTagLocalNegation);
    print_consed_list_of_conditions(thisAgent, negated_to_print, 2);
    xml_end_tag(thisAgent, kTagLocalNegation);

    free_list(thisAgent, negated_to_print);
}
