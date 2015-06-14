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
#include "wmem.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::print_o_id_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    print_ovar_to_o_id_map(mode);
    print_o_id_substitution_map(mode);
    print_o_id_to_ovar_debug_map(mode);

}

void Variablization_Manager::print_merge_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
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
        dprint(mode, "%y conditions: \n", iter_id->first);
        for (iter_attr = iter_id->second.begin(); iter_attr != iter_id->second.end(); ++iter_attr)
        {
            for (iter_value = iter_attr->second.begin(); iter_value != iter_attr->second.end(); ++iter_value)
            {
                dprint(mode, "   %l\n", iter_value->second);
            }
        }
    }

    dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_ovar_to_o_id_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    dprint(mode, "        ovar_to_o_id_map Map\n");
    dprint(mode, "------------------------------------\n");

    if (rulesym_to_identity_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    std::map< uint64_t, std::map< Symbol*, uint64_t > >::iterator iter_inst;
    std::map< Symbol*, uint64_t > ::iterator iter_sym;

    for (iter_inst = rulesym_to_identity_map->begin(); iter_inst != rulesym_to_identity_map->end(); ++iter_inst)
    {
        dprint(mode, "o_id's for i%u: \n", iter_inst->first);
        for (iter_sym = iter_inst->second.begin(); iter_sym != iter_inst->second.end(); ++iter_sym)
        {
            dprint(mode, "   %y = o%u\n", iter_sym->first, iter_sym->second);
        }
    }

    dprint(mode, "------------------------------------\n");
}


void Variablization_Manager::print_o_id_substitution_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    dprint(mode, "     o_id_substitution_map Map\n");
    dprint(mode, "------------------------------------\n");

    if (unification_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    std::map< uint64_t, uint64_t >::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {
        dprint(mode, "   o%u(%y) = o%u(%y)\n",
            iter->first, thisAgent->variablizationManager->get_ovar_for_o_id(iter->first),
            iter->second, thisAgent->variablizationManager->get_ovar_for_o_id(iter->second));
    }

    dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_o_id_to_ovar_debug_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    dprint(mode, "     o_id_to_ovar_debug_map Map\n");
    dprint(mode, "------------------------------------\n");

    if (o_id_to_ovar_debug_map->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    std::map< uint64_t, Symbol* >::iterator iter;

    for (iter = o_id_to_ovar_debug_map->begin(); iter != o_id_to_ovar_debug_map->end(); ++iter)
    {
        dprint(mode, "   o%u = %y\n",  iter->first, iter->second);
    }

    dprint(mode, "------------------------------------\n");
}
void Variablization_Manager::print_attachment_points(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    dprint(mode, "   Attachment Points in conditions\n");
    dprint(mode, "------------------------------------\n");

    if (attachment_points->size() == 0)
    {
        dprint(mode, "EMPTY MAP\n");
    }

    for (std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        dprint(mode, "%y(o%u) -> %s of %l\n", get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
    }

}
void Variablization_Manager::print_constraints(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    dprint(mode, "    Relational Constraints List\n");
    dprint(mode, "------------------------------------\n");
    if (constraints->empty())
    {
        dprint(mode, "NO CONSTRAINTS RECORDED\n");
    }
    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        dprint(mode, "%t[%g]:   %t[%g]\n", (*it)->eq_test, (*it)->eq_test, (*it)->constraint_test, (*it)->constraint_test);
    }

    dprint(mode, "------------------------------------\n");
}
/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Variablization_Manager::print_variablization_tables(TraceMode mode, int whichTable)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    dprint(mode, "------------------------------------\n");
    if ((whichTable == 0) || (whichTable == 1) || (whichTable == 3))
    {
        dprint(mode, "== Symbol -> Variablization ==\n");
        if (sym_to_var_map->size() == 0)
        {
            dprint(mode, "EMPTY MAP\n");
        }
        for (std::map< Symbol*, Symbol* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
        {
            dprint(mode, "%y -> %y\n", it->first, it->second);
        }
    }
    if ((whichTable == 0) || (whichTable == 2) || (whichTable == 3))
    {
        dprint(mode, "== O_ID -> Variablization ==\n");
        if (o_id_to_var_map->size() == 0)
        {
            dprint(mode, "EMPTY MAP\n");
        }
        for (std::map< uint64_t, Symbol* >::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
        {
            dprint(mode, "o%u -> %y\n", it->first, , it->second);
        }
    }
    dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    print_variablization_tables(mode);
    print_o_id_tables(mode);
}
