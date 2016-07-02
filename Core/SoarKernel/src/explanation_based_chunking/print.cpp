/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "agent.h"
#include "instantiation.h"
#include "assert.h"
#include "test.h"
#include "working_memory.h"
#include "output_manager.h"
#include "print.h"

#include "../debug_code/dprint.h"

void Explanation_Based_Chunker::print_current_built_rule()
{
    if (m_prod_name)
    {
        print_with_symbols(thisAgent, "\nsp {%y\n", m_prod_name);
    }
    if (m_vrblz_top)
    {
        print_condition_list(thisAgent, m_vrblz_top, 2, false);
    }
    if (m_rhs)
    {
        print(thisAgent, "\n  -->\n   ");
        print_action_list(thisAgent, m_rhs, 3, false);
    }
    print(thisAgent, "}\n\n");
}

void Explanation_Based_Chunker::print_o_id_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    print_ovar_to_o_id_map(mode);
    print_o_id_substitution_map(mode);
    print_o_id_to_ovar_debug_map(mode);

}

void Explanation_Based_Chunker::print_merge_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
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

void Explanation_Based_Chunker::print_ovar_to_o_id_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "        ovar_to_o_id_map Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (instantiation_identities->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    inst_to_id_map_type::iterator iter_inst;
    sym_to_id_map_type::iterator iter_sym;

    for (iter_inst = instantiation_identities->begin(); iter_inst != instantiation_identities->end(); ++iter_inst)
    {
        outputManager->printa_sf(thisAgent, "o_id's for i%u: \n", iter_inst->first);
        for (iter_sym = iter_inst->second.begin(); iter_sym != iter_inst->second.end(); ++iter_sym)
        {
            outputManager->printa_sf(thisAgent, "   %y = o%u\n", iter_sym->first, iter_sym->second);
        }
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}


void Explanation_Based_Chunker::print_o_id_substitution_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     o_id_substitution_map Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (unification_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    id_to_id_map_type::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "   o%u(%y) = o%u(%y)\n",
            iter->first, get_ovar_for_o_id(iter->first),
            iter->second, get_ovar_for_o_id(iter->second));
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_o_id_to_ovar_debug_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     o_id_to_ovar_debug_map Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (id_to_rule_sym_debug_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    id_to_sym_map_type::iterator iter;

    for (iter = id_to_rule_sym_debug_map->begin(); iter != id_to_rule_sym_debug_map->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "   o%u = %y\n",  iter->first, iter->second);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}
void Explanation_Based_Chunker::print_attachment_points(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "   Attachment Points in conditions\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (attachment_points->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    for (std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%y(o%u) -> %s of %l\n", get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
    }

}
void Explanation_Based_Chunker::print_constraints(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "    Relational Constraints List\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    if (constraints->empty())
    {
        outputManager->printa_sf(thisAgent, "NO CONSTRAINTS RECORDED\n");
    }
    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%t[%g]:   %t[%g]\n", (*it)->eq_test, (*it)->eq_test, (*it)->constraint_test, (*it)->constraint_test);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}
/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Explanation_Based_Chunker::print_variablization_table(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "== Identity Set -> Variablization ==\n");
    if (o_id_to_var_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }
    for (id_to_sym_map_type::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "o%u -> %y\n", it->first, it->second);
    }
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    print_variablization_table(mode);
    print_o_id_tables(mode);
}
