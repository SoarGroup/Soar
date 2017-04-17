#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "instantiation.h"
#include "mem.h"
#include "preference.h"
#include "slot.h"
#include "symbol.h"
#include "test.h"

/* ------------------------------------------------------------------
                  Add a preference to a slot's OSK prefs
   This function adds a preference to a slots's context dependent
   preference set, checking to first see whether the pref is already
   there. If an operator The slot's OSK prefs is copied to conditions' bt structs in
   create_instatiation.  Those copies of the OSK prefs are used to
   backtrace through all relevant local evaluation rules that led to the
   selection of the operator that produced a result.
------------------------------------------------------------------ */
void Explanation_Based_Chunker::add_to_OSK(slot* pSlot, preference* pPref, bool unique_value)
{

    bool already_exists = false;
    cons* lOSK_pref;
    preference* lPref;

    for (lOSK_pref = pSlot->OSK_prefs; lOSK_pref != NIL; lOSK_pref = lOSK_pref->rest)
    {
        lPref = static_cast<preference*>(lOSK_pref->first);
        if (lPref == pPref)
        {
            already_exists = true;
            break;
        }

        if (unique_value)
        {
            /* Checking if a preference is unique differs depending on the preference type */

            /* Binary preferences can be considered equivalent if they point to the same
             * operators in the correct relative spots */
            if (((pPref->type == BETTER_PREFERENCE_TYPE) || (pPref->type == WORSE_PREFERENCE_TYPE)) &&
                    ((lPref->type == BETTER_PREFERENCE_TYPE) || (lPref->type == WORSE_PREFERENCE_TYPE)))
            {
                if (pPref->type == lPref->type)
                {
                    already_exists = ((pPref->value == lPref->value) && (pPref->referent == lPref->referent));
                }
                else
                {
                    already_exists = ((pPref->value == lPref->referent) && (pPref->referent == lPref->value));
                }
            }
            else if ((pPref->type == BINARY_INDIFFERENT_PREFERENCE_TYPE) &&
                     (lPref->type == BINARY_INDIFFERENT_PREFERENCE_TYPE))
            {
                already_exists = (((pPref->value == lPref->value) && (pPref->referent == lPref->referent)) ||
                                  ((pPref->value == lPref->referent) && (pPref->referent == lPref->value)));
            }
            else
            {
                /* Otherwise they are equivalent if they have the same value and type */
                already_exists = (pPref->value == lPref->value) && (pPref->type == lPref->type);
            }
            if (already_exists)
            {
                break;
            }
        }
    }
    if (!already_exists)
    {
        push(thisAgent, pPref, pSlot->OSK_prefs);
        preference_add_ref(pPref);
    }
}


/* --------------------------------------------------------------------------
                 Build context-dependent preference set

  This function will copy the OSK prefs from a slot to the backtrace info for the
  corresponding condition.  The copied OSK prefs will later be backtraced through.

  Note: Until prohibits are included explicitly as part of the OSK prefs, we will
  just copy them directly from the prohibits list so that there is no
  additional overhead.

 --------------------------------------------------------------------------*/

void Explanation_Based_Chunker::copy_proposal_OSK(instantiation* inst, cons* newOSK)
{
    preference* pref;
    cons* l_OSK_prefs;

    assert (!inst->OSK_proposal_prefs);
    if (ebc_settings[SETTING_EBC_ADD_OSK])
    {
        for (l_OSK_prefs = newOSK; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
        {
            pref = static_cast<preference*>(l_OSK_prefs->first);
            push(thisAgent, pref, inst->OSK_proposal_prefs);
            dprint(DT_OSK, "Adding OSK proposal preference %p to instantiation.\n",  pref);
            /* Note that we don't add refcount to the preference here. If we did, this 
             * instantiation would never be deallocated since it will hold a refcount
             * on one of its own preferences.  */
        }
    }
}

void Explanation_Based_Chunker::copy_OSK(instantiation* inst)
{
    condition* cond;
    preference* pref;
    cons* l_OSK_prefs;

    inst->OSK_prefs = NIL;
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION && cond->bt.trace && cond->bt.trace->slot)
        {
            if (ebc_settings[SETTING_EBC_ADD_OSK])
            {
                if (cond->bt.trace->slot->OSK_prefs && (cond->data.tests.id_test->eq_test->data.referent->id->level == inst->match_goal_level) && !cond->test_for_acceptable_preference)
                {
                    for (l_OSK_prefs = cond->bt.trace->slot->OSK_prefs; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
                    {
                        pref = static_cast<preference*>(l_OSK_prefs->first);
                        push(thisAgent, pref, inst->OSK_prefs);
                        dprint(DT_OSK, "Adding OSK preference %p to instantiation.\n",  pref);
                        preference_add_ref(pref);
                    }
                }
            }
            pref = cond->bt.trace->slot->preferences[PROHIBIT_PREFERENCE_TYPE];
            while (pref)
            {
                push(thisAgent, pref, inst->OSK_prefs);
                preference_add_ref(pref);
                dprint(DT_OSK, "Adding OSK prohibit preference %p to instantiation.\n",  pref);
                pref = pref->next;
            }
        }
    }
}

void Explanation_Based_Chunker::update_proposal_OSK(slot* s, preference* winner)
{
    if (s->instantiation_with_temp_OSK)
    {
        dprint(DT_OSK, "Cleaning up OSK proposal preferences contained in inst %u (%y) %s\n",  s->instantiation_with_temp_OSK->i_id,  s->instantiation_with_temp_OSK->prod_name, s->instantiation_with_temp_OSK->OSK_proposal_prefs ? "exists" : "NULL");
        /* These prefs did not have their refcounts increased, so we don't want to call clear_preference_list */
        free_list(thisAgent, s->instantiation_with_temp_OSK->OSK_proposal_prefs);
        s->instantiation_with_temp_OSK->OSK_proposal_prefs = NULL;
        s->instantiation_with_temp_OSK->OSK_proposal_slot = NULL;
        s->instantiation_with_temp_OSK = NULL;
    }
    if (winner)
    {
        s->instantiation_with_temp_OSK = winner->inst;
        s->instantiation_with_temp_OSK->OSK_proposal_slot = s;
        dprint(DT_OSK, "Adding OSK proposal preferences contained to inst %u (%y) from slot (%y ^%y)\n",  s->instantiation_with_temp_OSK->i_id,  s->instantiation_with_temp_OSK->prod_name, s->id, s->attr);
        copy_proposal_OSK(winner->inst, s->OSK_prefs);
    }
}
