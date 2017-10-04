#include "ebc.h"

#include "agent.h"
#include "dprint.h"
#include "symbol_manager.h"
#include "rhs.h"
#include "test.h"

chunk_element* Explanation_Based_Chunker::add_sti_variablization(Symbol* pSym, Symbol* pVar, uint64_t pInstIdentity)
{
    dprint(DT_LHS_VARIABLIZATION, "Adding variablization found for %y -> %y [%u]\n", pSym, pVar, pInstIdentity);
    chunk_element* lVarInfo;
    //if ((*m_sym_to_var_map).find(pSym) == (*m_sym_to_var_map).end()) {
    	thisAgent->memoryManager->allocate_with_pool(MP_chunk_element, &lVarInfo);
    	lVarInfo->variable_sym = pVar;
    	pVar->var->instantiated_sym = pSym;
    	lVarInfo->inst_identity = pInstIdentity;
    	(*m_sym_to_var_map)[pSym] = lVarInfo;
    	return NIL;
    /*}
    else {	// CBC Patch:
    	dprint(DT_REPAIR, "*** Already in sym_to_var_map! Should replace local %y [%y/%u] with %y.\n", pSym, pVar, pInstIdentity, pVar, ((*m_sym_to_var_map)[pSym])->variable_sym);
        //thisAgent->memoryManager->free_with_pool(MP_chunk_element, lVarInfo);
    	//pVar = ((*m_sym_to_var_map)[pSym])->variable_sym;
    	return ((*m_sym_to_var_map)[pSym]);
    }*/
}

void Explanation_Based_Chunker::sti_variablize_test(test pTest, bool generate_identity)
{
    char prefix[2];
    Symbol* lNewVar = NULL, *lMatchedSym = pTest->data.referent;
    uint64_t lMatchedIdentity = LITERAL_VALUE;

    /* Copy in any identities for the unconnected identifier that was used in the unconnected conditions */
    auto iter_sym = m_sym_to_var_map->find(lMatchedSym);
    if (iter_sym == m_sym_to_var_map->end())
    {
        /* Create a new variable.  If constant is being variablized just used
         * 'c' instead of first letter of id name.  We now don't use 'o' for
         * non-operators and don't use 's' for non-states.  That makes things
         * clearer in chunks because of standard naming conventions. --- */
        char prefix_char = static_cast<char>(tolower(lMatchedSym->id->name_letter));
        if ((((prefix_char == 's') || (prefix_char == 'S')) && !lMatchedSym->id->isa_goal) ||
            (((prefix_char == 'o') || (prefix_char == 'O')) && !lMatchedSym->id->isa_operator))
        {
            prefix[0] = 'c';
        } else {
            prefix[0] = prefix_char;
        }
        prefix[1] = 0;
        lNewVar = thisAgent->symbolManager->generate_new_variable(prefix);
        lNewVar->var->instantiated_sym = lMatchedSym;
        if (generate_identity) lMatchedIdentity = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(lNewVar);
    }
    else
    {
        lNewVar = iter_sym->second->variable_sym;
        thisAgent->symbolManager->symbol_add_ref(lNewVar);
        if (generate_identity) lMatchedIdentity = iter_sym->second->inst_identity;
    }

    //if ((*m_sym_to_var_map).find(lMatchedSym) == (*m_sym_to_var_map).end()) {
    	add_sti_variablization(lMatchedSym, lNewVar, lMatchedIdentity);
    //}
    //else {
    //	dprint(DT_REPAIR, "*** Did not replace %y with [%y/%u] in sym_to_var_map. Replaced %y with %y.\n", lMatchedSym, lNewVar, lMatchedIdentity, lNewVar, ((*m_sym_to_var_map)[lMatchedSym])->variable_sym);
    //	lNewVar = ((*m_sym_to_var_map)[lMatchedSym])->variable_sym;
    //}
    pTest->data.referent = lNewVar;
    pTest->inst_identity = lMatchedIdentity;
    thisAgent->symbolManager->symbol_remove_ref(&lMatchedSym);
}

void Explanation_Based_Chunker::sti_variablize_rhs_symbol(rhs_value &pRhs_val, bool generate_identity)
{
    char prefix[2];
    Symbol* var;
    bool has_variablization = false, was_unbound = false;
    uint64_t lMatchedIdentity = LITERAL_VALUE;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;
        rhs_value lRhsValue, *lc;

        dprint(DT_RHS_FUN_VARIABLIZATION, "STI-variablizing RHS funcall %r\n", pRhs_val);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            lRhsValue = static_cast<rhs_value>(c->first);
            dprint(DT_RHS_FUN_VARIABLIZATION, "STI-variablizing RHS funcall argument %r\n", lRhsValue);
            sti_variablize_rhs_symbol(lRhsValue, false);
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS funcall argument is now   %r\n", static_cast<char*>(c->first));
        }
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    dprint(DT_RHS_VARIABLIZATION, "STI-variablizing %y [%u].\n", rs->referent, rs->inst_identity);

    auto iter_sym = m_sym_to_var_map->find(rs->referent);
    has_variablization = (iter_sym != m_sym_to_var_map->end());

    if (!has_variablization && rs->referent->is_sti())
    {
        /* -- First time we've encountered an unbound rhs var. -- */
        dprint(DT_RHS_VARIABLIZATION, "...is new unbound variable.\n");
        prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
        prefix[1] = 0;
        var = thisAgent->symbolManager->generate_new_variable(prefix);
        dprint(DT_RHS_VARIABLIZATION, "...created new variable for unbound var %y: %y\n", rs->referent, var);
        if (generate_identity) lMatchedIdentity = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(var);
        
        //if ((*m_sym_to_var_map).find(rs->referent) == (*m_sym_to_var_map).end()) {
        	//add_sti_variablization(lMatchedSym, lNewVar, lMatchedIdentity);
        	add_sti_variablization(rs->referent, var, lMatchedIdentity);
        //}
        //else {
        //	dprint(DT_REPAIR, "*** Did not replace %y with [%y/%u] in sym_to_var_map. Replaced %y with %y.\n", rs->referent, var, lMatchedIdentity, var, ((*m_sym_to_var_map)[rs->referent])->variable_sym);
        //	var = ((*m_sym_to_var_map)[rs->referent])->variable_sym;
        //}
        
        has_variablization = true;
        was_unbound = true;
    } else if (rs->referent->is_sti()) {
        var = iter_sym->second->variable_sym;
        if (generate_identity) lMatchedIdentity = iter_sym->second->inst_identity;
        has_variablization = true;
    }
    if (has_variablization)
    {
        dprint(DT_RHS_VARIABLIZATION, "... using variablization %y\n", var);
        thisAgent->symbolManager->symbol_remove_ref(&rs->referent);
        thisAgent->symbolManager->symbol_add_ref(var);
        rs->referent = var;
        rs->inst_identity = lMatchedIdentity;
//        rs->identity_set_wp.reset();
        rs->identity = NULL;
        rs->was_unbound_var = was_unbound;
    }
    else
    {
        dprint(DT_RHS_VARIABLIZATION, "...literal RHS symbol, maps to null identity set or has an identity not found on LHS.  Not variablizing.\n");
        rs->inst_identity = LITERAL_VALUE;
        rs->identity = NULL;
//        rs->identity_set_wp.reset();
    }
}
