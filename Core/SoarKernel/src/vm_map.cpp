/*
 * variablization_manager_map.cpp
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


inline variablization* copy_variablization(agent* thisAgent, variablization* v)
{
    variablization* new_variablization = new variablization;
    new_variablization->instantiated_symbol = v->instantiated_symbol;
    new_variablization->variablized_symbol = v->variablized_symbol;
    symbol_add_ref(thisAgent, new_variablization->instantiated_symbol);
    symbol_add_ref(thisAgent, new_variablization->variablized_symbol);
    new_variablization->grounding_id = v->grounding_id;
    return new_variablization;
}

void Variablization_Manager::clear_data()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing variablization maps.\n");
    clear_cached_constraints();
    clear_ovar_gid_table();
    clear_variablization_tables();
    clear_merge_map();
    clear_substitution_map();
    clear_dnvl();
}

void Variablization_Manager::clear_ovar_gid_table()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar g_id table...\n");
    /* -- Clear original variable map -- */
    for (std::map< Symbol*, uint64_t >::iterator it = (*orig_var_to_g_id_map).begin(); it != (*orig_var_to_g_id_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %y -> %u\n", it->first, it->second);
        symbol_remove_ref(thisAgent, it->first);
    }
    orig_var_to_g_id_map->clear();
}

void Variablization_Manager::clear_variablization_tables()
{

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol->variablization map...\n");
    /* -- Clear symbol->variablization map -- */
    for (std::map< Symbol*, variablization* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %y -> %y(%u)/%y(%u)\n",
               it->first,
               it->second->instantiated_symbol, it->second->instantiated_symbol->reference_count,
               it->second->variablized_symbol,  it->second->variablized_symbol->reference_count);
        symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
        symbol_remove_ref(thisAgent, it->second->variablized_symbol);
        delete it->second;
    }
    sym_to_var_map->clear();

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing grounding_id->variablization map...\n");
    /* -- Clear grounding_id->variablization map -- */
    for (std::map< uint64_t, variablization* >::iterator it = (*g_id_to_var_map).begin(); it != (*g_id_to_var_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %u -> %y(%u)/%y(%u)\n",
               it->first,
               it->second->instantiated_symbol, it->second->instantiated_symbol->reference_count,
               it->second->variablized_symbol,  it->second->variablized_symbol->reference_count);
        symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
        symbol_remove_ref(thisAgent, it->second->variablized_symbol);
        delete it->second;
    }
    g_id_to_var_map->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing variablization data.\n");
}

variablization* Variablization_Manager::get_variablization(uint64_t index_id)
{
    if (index_id == 0)
    {
        return NULL;
    }

    std::map< uint64_t, variablization* >::iterator iter = (*g_id_to_var_map).find(index_id);
    if (iter != (*g_id_to_var_map).end())
    {
        dprint(DT_LHS_VARIABLIZATION, "...found %u in g_id variablization table: %y/%y\n", index_id,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_LHS_VARIABLIZATION, "...did not find %u in g_id variablization table.\n", index_id);
        print_variablization_tables(DT_LHS_VARIABLIZATION, 2);
        return NULL;
    }
}

variablization* Variablization_Manager::get_variablization_for_symbol(std::map< Symbol*, variablization* >* pMap, Symbol* index_sym)
{
    std::map< Symbol*, variablization* >::iterator iter = (*pMap).find(index_sym);
    if (iter != (*pMap).end())
    {
        dprint(DT_LHS_VARIABLIZATION, "...found %y in variablization table: %y/%y\n", index_sym,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_LHS_VARIABLIZATION, "...did not find %y in variablization table.\n", index_sym);
        print_variablization_tables(DT_LHS_VARIABLIZATION, 1);
        return NULL;
    }
}
variablization* Variablization_Manager::get_variablization(Symbol* index_sym)
{
    return get_variablization_for_symbol(sym_to_var_map, index_sym);
}

variablization* Variablization_Manager::get_variablization(test t)
{
    assert(t->data.referent);
    if (t->data.referent->is_sti())
    {
        return get_variablization(t->data.referent);
    }
    else
    {
        return get_variablization(t->identity->grounding_id);
    }
}

uint64_t Variablization_Manager::get_gid_for_orig_var(Symbol* index_sym)
{
    std::map< Symbol*, uint64_t >::iterator iter = (*orig_var_to_g_id_map).find(index_sym);
    if (iter != (*orig_var_to_g_id_map).end())
    {
        dprint(DT_LHS_VARIABLIZATION, "...found %u in orig_var variablization table for %y\n",
               iter->second, index_sym);

        return iter->second;
    }
    else
    {
        dprint(DT_LHS_VARIABLIZATION, "...did not find %y in orig_var variablization table.\n", index_sym);
        print_ovar_gid_propogation_table(DT_LHS_VARIABLIZATION);
    }

    return 0;
}

uint64_t Variablization_Manager::add_orig_var_to_gid_mapping(Symbol* index_sym, uint64_t index_g_id)
{
    std::map< Symbol*, uint64_t >::iterator iter = (*orig_var_to_g_id_map).find(index_sym);
    if (iter == (*orig_var_to_g_id_map).end())
    {
        dprint(DT_OVAR_MAPPINGS, "Adding original variable mappings entry: %y to %u\n", index_sym, index_g_id);
        (*orig_var_to_g_id_map)[index_sym] = index_g_id;
        symbol_add_ref(thisAgent, index_sym);
        return 0;
    }
    else
    {
        dprint(DT_OVAR_MAPPINGS,
               "...%u already exists in orig_var variablization table for %y.  add_orig_var_to_gid_mapping returning false.\n",
               iter->second, index_sym);
    }
    return iter->second;
}

void Variablization_Manager::store_variablization(Symbol* instantiated_sym,
        Symbol* variable,
        identity_info* identity)
{
    variablization* new_variablization;
    assert(instantiated_sym && variable);
    dprint(DT_LHS_VARIABLIZATION, "Storing variablization for %y(%u) to %y.\n",
           instantiated_sym,
           identity ? identity->grounding_id : 0,
           variable);

    new_variablization = new variablization;
    new_variablization->instantiated_symbol = instantiated_sym;
    new_variablization->variablized_symbol = variable;
    symbol_add_ref(thisAgent, instantiated_sym);
    symbol_add_ref(thisAgent, variable);
    new_variablization->grounding_id = identity ? identity->grounding_id : 0;

    if (instantiated_sym->is_sti())
    {
        /* -- STI may have more than one original symbol (mostly due to the fact
         *    that placeholder variables still exist to handle dot notation).  So, we
         *    look them up using the identifier symbol instead of the original variable.
         *
         *    Note that we also store an entry using the new variable as an index. Later,
         *    when looking for ungrounded variables in relational tests, the
         *    identifier symbol will have already been replaced with a variable,
         *    so we must use the variable instead to look up variablization info.
         *    This may not be necessary after we resurrect the old NOT code. -- */

        (*sym_to_var_map)[instantiated_sym] = new_variablization;
        (*sym_to_var_map)[variable] = copy_variablization(thisAgent, new_variablization);
        dprint_noprefix(DT_LHS_VARIABLIZATION, "Created symbol_to_var_map ([%y] and [%y] to new variablization.\n",
                        instantiated_sym, variable);
    }
    else if (identity)
    {

        /* -- A constant symbol is being variablized, so store variablization info
         *    indexed by the constant's grounding id. -- */
        (*g_id_to_var_map)[identity->grounding_id] = new_variablization;

        dprint_noprefix(DT_LHS_VARIABLIZATION, "Created g_id_to_var_map[%u] to new variablization.\n",
                        identity->grounding_id);
    }
    else
    {
        assert(false);
    }
    //  print_variablization_table();
}

void Variablization_Manager::clear_dnvl()
{
    dnvl_set->clear();
}
void Variablization_Manager::add_dnvl(Symbol* sym)
{
    dprint(DT_IDENTITY_PROP, "...adding symbol %y.\n", sym);
    dnvl_set->insert(sym);
}

bool Variablization_Manager::is_in_dnvl(Symbol* sym)
{
    std::set< Symbol* >::iterator iter;
    iter = dnvl_set->find(sym);
    return (iter != dnvl_set->end());
}
