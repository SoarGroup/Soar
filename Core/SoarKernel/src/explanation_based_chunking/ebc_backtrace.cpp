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

void Explanation_Based_Chunker::add_starting_identity_sets_to_cond(condition* trace_cond)
{
    if (trace_cond->data.tests.id_test->eq_test->identity)
    {
        trace_cond->data.tests.id_test->eq_test->identity_set = trace_cond->inst->bt_identity_set_mappings->at(trace_cond->data.tests.id_test->eq_test->identity);
        if (!trace_cond->data.tests.id_test->eq_test->identity_set)
        {
            trace_cond->data.tests.id_test->eq_test->identity_set = trace_cond->data.tests.id_test->eq_test->identity;
        }
    } else {
        trace_cond->data.tests.id_test->eq_test->identity_set = NULL_IDENTITY_SET;
    }
    if (trace_cond->data.tests.attr_test->eq_test->identity)
    {
        trace_cond->data.tests.attr_test->eq_test->identity_set = trace_cond->inst->bt_identity_set_mappings->at(trace_cond->data.tests.attr_test->eq_test->identity);
        if (!trace_cond->data.tests.attr_test->eq_test->identity_set)
        {
            trace_cond->data.tests.attr_test->eq_test->identity_set = trace_cond->data.tests.attr_test->eq_test->identity;
        }
    } else {
        trace_cond->data.tests.attr_test->eq_test->identity_set = NULL_IDENTITY_SET;
    }
    if (trace_cond->data.tests.value_test->eq_test->identity)
    {
        trace_cond->data.tests.value_test->eq_test->identity_set = trace_cond->inst->bt_identity_set_mappings->at(trace_cond->data.tests.value_test->eq_test->identity);
        if (!trace_cond->data.tests.value_test->eq_test->identity_set)
        {
            trace_cond->data.tests.value_test->eq_test->identity_set = trace_cond->data.tests.value_test->eq_test->identity;
        }
    } else {
        trace_cond->data.tests.value_test->eq_test->identity_set = NULL_IDENTITY_SET;
    }

}

void Explanation_Based_Chunker::propagate_identity_sets(id_to_id_map* identity_set_mappings, condition* trace_cond, const identity_quadruple o_ids_to_replace)
{
    if (trace_cond)
    {
//        if (o_ids_to_replace.id && (trace_cond->data.tests.id_test->eq_test->id_set_tc_num == id_set_pass1_tc) && trace_cond->data.tests.id_test->eq_test->identity_set;
        if (o_ids_to_replace.id && trace_cond->data.tests.id_test->eq_test->identity_set)
        {
            assert(identity_set_mappings->find(o_ids_to_replace.id) != identity_set_mappings->end());
            auto iter = identity_set_mappings->find(o_ids_to_replace.id);
            if (iter->second == NULL_IDENTITY_SET)
            {
                (*identity_set_mappings)[o_ids_to_replace.id] = trace_cond->data.tests.id_test->eq_test->identity_set;
            }
        }
        if (o_ids_to_replace.attr && trace_cond->data.tests.attr_test->eq_test->identity_set)
        {
            assert(identity_set_mappings->find(o_ids_to_replace.attr) != identity_set_mappings->end());
            auto iter = identity_set_mappings->find(o_ids_to_replace.attr);
            if (iter->second == NULL_IDENTITY_SET)
            {
                (*identity_set_mappings)[o_ids_to_replace.attr] = trace_cond->data.tests.attr_test->eq_test->identity_set;
            }
        }
        if (o_ids_to_replace.value && trace_cond->data.tests.value_test->eq_test->identity_set)
        {
            assert(identity_set_mappings->find(o_ids_to_replace.value) != identity_set_mappings->end());
            auto iter = identity_set_mappings->find(o_ids_to_replace.value);
            if (iter->second == NULL_IDENTITY_SET)
            {
                (*identity_set_mappings)[o_ids_to_replace.value] = trace_cond->data.tests.value_test->eq_test->identity_set;
            }
        }
    }
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

    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "... BT through instantiation of ");
        thisAgent->outputManager->printa_sf(thisAgent, "%y\n", inst->prod ? inst->prod_name : thisAgent->symbolManager->soarSymbols.architecture_inst_symbol);
        xml_begin_tag(thisAgent, kTagBacktrace);
        xml_att_val(thisAgent, kProduction_Name, inst->prod ? inst->prod_name: thisAgent->symbolManager->soarSymbols.architecture_inst_symbol);
    }

    if (inst->backtrace_number != backtrace_number)
    {
        if (inst->bt_identity_set_mappings->size() > 0)
        {
            dprint_noprefix(DT_BACKTRACE1, "\nClearing identity set mapping entries.\n");
            for (auto iter = inst->bt_identity_set_mappings->begin(); iter != inst->bt_identity_set_mappings->end(); ++iter)
            {
                iter->second = NULL_IDENTITY_SET;
            }
            dprint_noprefix(DT_BACKTRACE1, "Identity set mapping entries:\n");
            for (auto iter = inst->bt_identity_set_mappings->begin(); iter != inst->bt_identity_set_mappings->end(); ++iter)
            {
                dprint_noprefix(DT_BACKTRACE1, "%u -> %u\n", iter->first, iter->second);
            }
        }
    }

    if (trace_cond && ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        ebc_timers->dependency_analysis->stop();
        propagate_identity_sets(inst->bt_identity_set_mappings, trace_cond, o_ids_to_replace);
        unify_backtraced_conditions(trace_cond, o_ids_to_replace, rhs_funcs);
        ebc_timers->dependency_analysis->start();
        dprint(DT_BACKTRACE1,  "Backtraced instantiation for match of %y (%u) in %y (%d) : \n%5", inst->prod_name, inst->i_id, inst->match_goal, static_cast<long long>(inst->match_goal_level), inst->top_of_instantiated_conditions, inst->preferences_generated);
        if (inst->bt_identity_set_mappings->size() > 0)
        {
            dprint_noprefix(DT_BACKTRACE1, "\nIdentity set mapping entries: \n");
            for (auto iter = inst->bt_identity_set_mappings->begin(); iter != inst->bt_identity_set_mappings->end(); ++iter)
            {
                dprint_noprefix(DT_BACKTRACE1, "%u -> %u\n", iter->first, iter->second);
            }
            dprint_noprefix(DT_BACKTRACE1, "\n\n");
        }
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

    /* --- scan through conditions, collect grounds and locals --- */
    negateds_to_print = NIL;


    for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            update_remaining_identity_sets_in_test(c->data.tests.id_test, inst);
            update_remaining_identity_sets_in_test(c->data.tests.attr_test, inst);
            update_remaining_identity_sets_in_test(c->data.tests.value_test, inst);

            /* Might be able to only cache constraints for non-operational conds.  The others should show
             * up.  In fact, we may not need the whole tc_num mechanism if that would limit it to only the
             * ones needed. */
            cache_constraints_in_cond(c);
            if (condition_is_operational(c, grounds_level))
            {
                if (c->bt.wme_->tc != grounds_tc)                   /* First time we've seen something matching this wme*/
                {
                    add_to_grounds(c);
                }
                else if (ebc_settings[SETTING_EBC_LEARNING_ON])     /* Another condition that matches the same wme */
                {
                    add_to_grounds(c);
                }
            } else {                                                /* A local sub-state WME */
                add_to_locals(c);
                add_local_singleton_unification_if_needed(c);
            }
        }
        else
        {
            dprint(DT_BACKTRACE, "Adding NC or NCC condition %y (i%u): %l\n", c->inst->prod_name, c->inst->i_id, c);
            if (c->type == NEGATIVE_CONDITION)
            {
                update_remaining_identity_sets_in_test(c->data.tests.id_test, inst);
                update_remaining_identity_sets_in_test(c->data.tests.attr_test, inst);
                update_remaining_identity_sets_in_test(c->data.tests.value_test, inst);
            } else {
                update_remaining_identity_sets_in_condlist(c->data.ncc.top, inst);
            }
            add_to_chunk_cond_set(&negated_set, make_chunk_cond_for_negated_condition(c));
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM]) push(thisAgent, c, negateds_to_print);
        }
    }

    /* --- if tracing BT, print the resulting conditions, etc. --- */
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
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
        thisAgent->outputManager->printa(thisAgent, "  -->Negated:\n");
        xml_begin_tag(thisAgent, kTagNegated);
        print_consed_list_of_conditions(thisAgent, negateds_to_print, 0);
        xml_end_tag(thisAgent, kTagNegated);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");

        xml_end_tag(thisAgent, kTagBacktrace);
    }

    /* Moved these free's down to here, to ensure they are cleared even if we're not printing these lists     */
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

        dprint(DT_BACKTRACE, "Tracing through local condition of of instantiation %y (i%u): %l\n", cond->inst->prod_name, cond->inst->i_id, cond);

        bt_pref = NULL;
        if (cond->bt.trace)
        {
            bt_pref = (cond->bt.trace->level != grounds_level + 1) ? find_clone_for_level(cond->bt.trace, static_cast<goal_stack_level>(grounds_level + 1)) : cond->bt.trace;
        }
        if (bt_pref)
        {
            add_starting_identity_sets_to_cond(cond);
            backtrace_through_instantiation(bt_pref->inst, grounds_level, cond, bt_pref->identities, bt_pref->rhs_funcs, cond->inst->explain_depth, BT_Normal);

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

void Explanation_Based_Chunker::perform_dependency_analysis()
{
    preference* pref;
    goal_stack_level grounds_level = m_inst->match_goal_level - 1;

//    outputManager->set_print_test_format(true, false);
    outputManager->set_print_test_format(true, true);
    dprint(DT_BACKTRACE,  "\nBacktracing through base instantiation %y: \n", m_inst->prod_name);
    dprint_header(DT_BACKTRACE, PrintBefore, "Starting dependency analysis...\n");

    ebc_timers->dependency_analysis->start();

//    increment_counter(backtrace_number);
//    increment_counter(grounds_tc);
    increment_counter(id_set_pass1_tc);
//    grounds = NIL;
//    locals = NIL;
//
//    /* MToDo | When do we want to do this? */
//    //thisAgent->explanationMemory->set_backtrace_number(backtrace_number);
//
//    /* Backtrace through the instantiation that produced each result --- */
//    for (pref = m_results; pref != NIL; pref = pref->next_result)
//    {
//        dprint(DT_BACKTRACE1, "Starting dependency analysis of result preference %p...\n", pref);
//        btpass1_backtrace_through_instantiation(pref->inst, grounds_level, NULL, pref->identities, pref->rhs_funcs, 0, (pref->inst == m_inst) ? BT_BaseInstantiation : BT_ExtraResults);
//    }
//
//    btpass1_trace_locals(grounds_level);
//
//    if (locals) free_list(thisAgent, grounds);
//    if (locals) free_list(thisAgent, locals);

    increment_counter(backtrace_number);
    increment_counter(grounds_tc);
    grounds = NIL;
    locals = NIL;

    thisAgent->explanationMemory->set_backtrace_number(backtrace_number);

    /* Backtrace through the instantiation that produced each result --- */
    for (pref = m_results; pref != NIL; pref = pref->next_result)
    {
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "\nFor result preference ");
            xml_begin_tag(thisAgent, kTagBacktraceResult);
            print_preference(thisAgent, pref);
            thisAgent->outputManager->printa(thisAgent, " ");
        }
        backtrace_through_instantiation(pref->inst, grounds_level, NULL, pref->identities, pref->rhs_funcs, 0, (pref->inst == m_inst) ? BT_BaseInstantiation : BT_ExtraResults);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagBacktraceResult);
        }
    }

    trace_locals(grounds_level);

    outputManager->clear_print_test_format();

    ebc_timers->dependency_analysis->stop();

    dprint_header(DT_BACKTRACE, PrintAfter, "Dependency analysis complete.\n");
    dprint_unification_map(DT_BACKTRACE);
    dprint(DT_BACKTRACE, "Grounds:\n%3", grounds);
    dprint(DT_BACKTRACE, "Locals:\n%3", locals);

}
