/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_identity.h"

#include "agent.h"
#include "dprint.h"
#include "ebc_settings.h"
#include "instantiation.h"
#include "output_manager.h"
#include "print.h"
#include "soar_TraceNames.h"
#include "test.h"
#include "working_memory.h"
#include "xml.h"

void Explanation_Based_Chunker::print_current_built_rule(const char* pHeader)
{
	outputManager->begin_output_record();
    if (pHeader)
    {
        outputManager->printa_sf(thisAgent, "\n%s\n   ", pHeader);
    }
    if (m_prod_name)
    {
        outputManager->printa_sf(thisAgent, "\nsp {%y\n   ", m_prod_name);
    }
    if (m_lhs)
    {
        print_condition_list(thisAgent, m_lhs, 2, false);
    }
    if (m_rhs)
    {
        outputManager->printa(thisAgent, "\n  -->\n   ");
        print_action_list(thisAgent, m_rhs, 3, false);
        outputManager->printa_sf(thisAgent, "}\n\n");
    }
}

void Explanation_Based_Chunker::report_local_negation(condition* c)
{
    cons* negated_to_print = NIL;
    push(thisAgent, c, negated_to_print);

    thisAgent->outputManager->printa(thisAgent, "\n*** Rule learned that used negative reasoning about local sub-state.***\n");
    xml_begin_tag(thisAgent, soar_TraceNames::kTagLocalNegation);
    print_consed_list_of_conditions(thisAgent, negated_to_print, 2);
    xml_end_tag(thisAgent, soar_TraceNames::kTagLocalNegation);

    free_list(thisAgent, negated_to_print);
}

void Explanation_Based_Chunker::print_merge_map(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "            Merge Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (cond_merge_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    triple_merge_map::iterator          iter_id;
    sym_to_sym_to_cond_map::iterator    iter_attr;
    sym_to_cond_map::iterator           iter_value;

    for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
    {
        outputManager->printa_sf(thisAgent, "%y conditions: \n", iter_id->first);
        for (iter_attr = iter_id->second.begin(); iter_attr != iter_id->second.end(); ++iter_attr)
        {
            for (iter_value = iter_attr->second.begin(); iter_value != iter_attr->second.end(); ++iter_value)
            {
                outputManager->printa_sf(thisAgent, "   %l\n", iter_value->second);
            }
        }
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_instantiation_identities_map(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     Instantiation Identity Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (instantiation_identities->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    sym_to_id_map::iterator iter_sym;

    for (auto iter_sym = instantiation_identities->begin(); iter_sym != instantiation_identities->end(); ++iter_sym)
    {
        outputManager->printa_sf(thisAgent, "   %y = o%u\n", iter_sym->first, iter_sym->second);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}


void Explanation_Based_Chunker::print_id_to_identity_map(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     Identity to Identity Set Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (inst_id_to_identity_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    id_to_join_map::iterator iter;

    for (iter = inst_id_to_identity_map->begin(); iter != inst_id_to_identity_map->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "   %u = %u\n", iter->first, iter->second->get_identity());
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_constraints(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "    Relational Constraints List\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    if (constraints->empty())
    {
        outputManager->printa_sf(thisAgent, "NO CONSTRAINTS RECORDED\n");
    }
    for (constraint_list::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%t[%g]:   %t[%g]\n", (*it)->eq_test, (*it)->eq_test, (*it)->constraint_test, (*it)->constraint_test);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}
/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Explanation_Based_Chunker::print_variablization_table(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "== Identity Set -> Variablization ==\n");
//        outputManager->printa_sf(thisAgent, "%u -> %y (%u)\n", it->first, it->second->variable_sym, it->second->identity);
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_tables(TraceMode mode)
{
    if (!thisAgent->outputManager->is_trace_enabled(mode)) return;
    print_variablization_table(mode);
    print_instantiation_identities_map(mode);
    print_id_to_identity_map(mode);
}

const char* Explanation_Based_Chunker::singletonTypeToString(singleton_element_type pType)
{
    switch (pType) {
        case ebc_any:
            return "<any>";
        case ebc_state:
            return "<state>";
        case ebc_operator:
            return "<operator>";
        case ebc_identifier:
            return "<identifier>";
        case ebc_constant:
            return "<constant>";
        default:
            return "INVALID";
    }
}

void Explanation_Based_Chunker::print_singleton_summary()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa(thisAgent,    "==== Singleton WME Unification Patterns ====\n");
    outputManager->printa(thisAgent,    "----------------- Local --------------------\n");
    outputManager->printa(thisAgent,    "   (<state> ^superstate <state>)\n");
    outputManager->printa(thisAgent,    "\n-------------- Super-state -----------------\n");
    outputManager->printa(thisAgent,    "   (<state> ^superstate <any>)\n");
    outputManager->printa_sf(thisAgent, "   (<state> ^operator   <operator>)             %-(unless condition only tests operator proposal)\n");
    outputManager->printa(thisAgent,    "   (<state> ^type       <constant>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^impasse    <constant>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^smem       <identifier>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^epmem      <identifier>)\n");
    outputManager->printa(thisAgent,    "   ---------- user-defined ----------\n");
    if (singletons->empty())
    {
        thisAgent->outputManager->printa(thisAgent, "   None.\n");
    }
    else
    {
        Symbol* lSym;
        for (auto it = singletons->begin(); it != singletons->end(); it++)
        {
            lSym = static_cast<Symbol*>(*it);
            thisAgent->outputManager->printa_sf(thisAgent, "   (%s ^%y %s)\n", singletonTypeToString(lSym->sc->singleton.id_type), lSym, singletonTypeToString(lSym->sc->singleton.value_type));
        }
    }
    outputManager->printa(thisAgent, "\n\n"
        "To add a new pattern:    chunk singleton    <type> attribute <type>\n"
        "To remove a pattern:     chunk singleton -r <type> attribute <type>\n\n"
        "   Valid types:          [ any | constant | identifier | operator | state ]  \n");
}
