#include "ebc.h"

#include "agent.h"
#include "symbol_manager.h"
#include "rhs.h"
#include "test.h"

void Explanation_Based_Chunker::add_sti_variablization(Symbol* pSym, Symbol* pVar, uint64_t pInstIdentity, uint64_t pInstCIdentity)
{
    chunk_element* lVarInfo;
    thisAgent->memoryManager->allocate_with_pool(MP_chunk_element, &lVarInfo);
    lVarInfo->variable_sym = pVar;
    pVar->var->instantiated_sym = pSym;
    lVarInfo->inst_identity = pInstIdentity;
    lVarInfo->cv_id = pInstCIdentity;
    (*m_sym_to_var_map)[pSym] = lVarInfo;
}

void Explanation_Based_Chunker::sti_variablize_test(test pTest, bool generate_identity)
{
    char prefix[2];
    Symbol* lNewVar = NULL, *lMatchedSym = pTest->data.referent;
    uint64_t lMatchedIdentity = LITERAL_VALUE, lMatchedCIdentity = LITERAL_VALUE;

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
        add_sti_variablization(lMatchedSym, lNewVar, lMatchedIdentity, lMatchedCIdentity);
    }
    else
    {
        lNewVar = iter_sym->second->variable_sym;
        thisAgent->symbolManager->symbol_add_ref(lNewVar);
        if (generate_identity)
        {
        	lMatchedIdentity = iter_sym->second->inst_identity;
        	lMatchedCIdentity = iter_sym->second->cv_id;
        }
    }

    pTest->data.referent = lNewVar;
    pTest->inst_identity = lMatchedIdentity;
    pTest->chunk_inst_identity = lMatchedCIdentity;
    thisAgent->symbolManager->symbol_remove_ref(&lMatchedSym);
}

void Explanation_Based_Chunker::sti_variablize_rhs_symbol(rhs_value &pRhs_val, bool generate_identity)
{
    char prefix[2];
    Symbol* var;
    bool has_variablization = false, was_unbound = false;
    uint64_t lMatchedIdentity = LITERAL_VALUE, lMatchedCIdentity = LITERAL_VALUE;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;
        rhs_value lRhsValue;

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            lRhsValue = static_cast<rhs_value>(c->first);
            sti_variablize_rhs_symbol(lRhsValue, false);
        }
        return;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    auto iter_sym = m_sym_to_var_map->find(rs->referent);
    has_variablization = (iter_sym != m_sym_to_var_map->end());

    if (!has_variablization && rs->referent->is_sti())
    {
        /* -- First time we've encountered an unbound rhs var. -- */
        prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
        prefix[1] = 0;
        var = thisAgent->symbolManager->generate_new_variable(prefix);
        if (generate_identity) lMatchedIdentity = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(var);
        add_sti_variablization(rs->referent, var, lMatchedIdentity, lMatchedCIdentity);
        has_variablization = true;
        was_unbound = true;
    } else if (rs->referent->is_sti()) {
        var = iter_sym->second->variable_sym;
        if (generate_identity)
        {
        	lMatchedIdentity = iter_sym->second->inst_identity;
            lMatchedCIdentity = iter_sym->second->cv_id;
        }
        has_variablization = true;
    }
    if (has_variablization)
    {
        thisAgent->symbolManager->symbol_remove_ref(&rs->referent);
        thisAgent->symbolManager->symbol_add_ref(var);
        rs->referent = var;
        rs->inst_identity = lMatchedIdentity;
        rs->cv_id = lMatchedCIdentity;
        rs->identity = NULL;
        rs->was_unbound_var = was_unbound;
    }
    else
    {
        rs->inst_identity = LITERAL_VALUE;
        rs->identity = NULL;
    }
}
