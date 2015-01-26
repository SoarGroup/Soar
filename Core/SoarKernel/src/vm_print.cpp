/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::print_dnvl_set(TraceMode mode)
{
    dprint(mode, "------------------------------------\n");
    dprint(mode, "       Do Not Variablize List\n");
    dprint(mode, "------------------------------------\n");

    if (dnvl_set->size() == 0)
    {
        dprint(mode, "EMPTY SET\n");
    }

    std::set< Symbol*>::iterator iter;
    int d_cnt = 1;
    for (iter = dnvl_set->begin(); iter != dnvl_set->end(); ++iter, ++d_cnt)
    {
        dprint(mode, "%i %y\n", d_cnt, (*iter));
    }

    dprint(mode, "------------------------------------\n");
}
void Variablization_Manager::print_substitution_map(TraceMode mode)
{
    dprint(mode, "------------------------------------\n");
    dprint(mode, "          Substitution Map\n");
    dprint(mode, "------------------------------------\n");

    if (substitution_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    std::map< Symbol*, test>::iterator iter;

    for (iter = substitution_map->begin(); iter != substitution_map->end(); ++iter)
    {
        dprint(mode, "%y -> %t\n", iter->first, iter->second);
    }

    dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_merge_map(TraceMode mode)
{
    dprint(mode, "------------------------------------\n");
    dprint(mode, "            Merge Map\n");
    dprint(mode, "------------------------------------\n");

    if (cond_merge_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    std::map< Symbol*, std::map< Symbol*, std::map< Symbol*, condition*> > >::iterator iter_id;
    std::map< Symbol*, std::map< Symbol*, condition*> >::iterator iter_attr;
    std::map< Symbol*, condition*>::iterator iter_value;

    for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
    {
        dprint(DT_MERGE, "%y conditions: \n", iter_id->first);
        for (iter_attr = iter_id->second.begin(); iter_attr != iter_id->second.end(); ++iter_attr)
        {
            for (iter_value = iter_attr->second.begin(); iter_value != iter_attr->second.end(); ++iter_value)
            {
                dprint(DT_MERGE, "   %l\n", iter_value->second);
            }
        }
    }

    dprint(mode, "------------------------------------\n");
}
void Variablization_Manager::print_ovar_gid_propogation_table(TraceMode mode, bool printHeader)
{
    if (printHeader)
    {
        dprint(mode, "------------------------------------\n");
        dprint(mode, "OrigVariable to g_id Propagation Map\n");
        dprint(mode, "------------------------------------\n");
    }

    if (orig_var_to_g_id_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    for (std::map< Symbol*, uint64_t >::iterator it = (*orig_var_to_g_id_map).begin(); it != (*orig_var_to_g_id_map).end(); ++it)
    {
        dprint(mode, "%y -> %u\n", it->first, it->second);
    }

}

void Variablization_Manager::print_cached_constraints(TraceMode mode)
{
    dprint(mode, "------------------------------------\n");
    dprint(mode, "           STI Constraint Map\n");
    dprint(mode, "------------------------------------\n");

    if (sti_constraints->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    cons* c;

    for (std::map< Symbol*, ::list* >::iterator it = sti_constraints->begin(); it != sti_constraints->end(); ++it)
    {
        c = it->second;
        while (c)
        {
            dprint(mode, "%y: %t\n", it->first, static_cast<test>(c->first));
            c = c->rest;
        }
    }
    dprint(mode, "------------------------------------\n");
    dprint(mode, "         Non-STI Constraint Map\n");
    dprint(mode, "------------------------------------\n");
    if (constant_constraints->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }
    for (std::map< uint64_t, ::list* >::iterator it = constant_constraints->begin(); it != constant_constraints->end(); ++it)
    {
        c = it->second;
        while (c)
        {
            dprint(mode, "%u: %t\n", it->first, static_cast<test>(c->first));
            c = c->rest;
        }
    }
    dprint(mode, "------------------------------------\n");
}
/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Variablization_Manager::print_variablization_tables(TraceMode mode, int whichTable)
{
    dprint(mode, "------------------------------------\n");
//    if (whichTable == 0)
//    {
//        dprint(mode, "       Variablization Tables\n");
//        dprint(mode, "------------------------------------\n");
//    }
    if ((whichTable == 0) || (whichTable == 1))
    {
        dprint(mode, "== Symbol -> v_info table ==\n");
//        if (whichTable != 0)
//            dprint(mode, "------------------------------------\n");
        if (sym_to_var_map->size() == 0)
        {
            dprint(mode, "EMPTY MAP\n");
        }
        for (std::map< Symbol*, variablization* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
        {
            dprint(mode, "%y -> %y/%y\n", it->first, it->second->variablized_symbol, it->second->instantiated_symbol);
        }
    }
    if ((whichTable == 0) || (whichTable == 2))
    {
        dprint(mode, "== G_ID -> v_info table ==\n");
//        if (whichTable != 0)
//            dprint(mode, "------------------------------------\n");
        if (g_id_to_var_map->size() == 0)
        {
            dprint(mode, "EMPTY MAP\n");
        }
        for (std::map< uint64_t, variablization* >::iterator it = (*g_id_to_var_map).begin(); it != (*g_id_to_var_map).end(); ++it)
        {
            dprint(mode, "%u -> %y/%y\n", it->first,
                   it->second->variablized_symbol, it->second->instantiated_symbol);
        }
    }
    if ((whichTable == 0) || (whichTable == 3))
    {
        dprint(mode, "---- Original Var -> G_ID Table ----\n");
        if (whichTable != 0)
        {
            dprint(mode, "------------------------------------\n");
        }
        print_ovar_gid_propogation_table(mode);
    }
    dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_tables()
{
    print_variablization_tables(DT_VARIABLIZATION_MANAGER);
}
