/*
 * debug_inventories.cpp
 *
 *  Created on: Feb 1, 2017
 *      Author: mazzin
 */
#include "debug_inventories.h"

#include "agent.h"
#include "dprint.h"
#include "decide.h"
#include "ebc.h"
#include "instantiation.h"
#include "misc.h"
#include "preference.h"
#include "rhs.h"
#include "soar_instance.h"
#include "symbol_manager.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"

#include <assert.h>
#include <string>

void clean_up_debug_inventories(agent* thisAgent)
{
    IDI_print_and_cleanup(thisAgent);
    PDI_print_and_cleanup(thisAgent);
    WDI_print_and_cleanup(thisAgent);
    GDI_print_and_cleanup(thisAgent);
    ISI_print_and_cleanup(thisAgent);
}

#if defined(DEBUG_TRACE_REFCOUNT_FOR) && defined(DEBUG_REFCOUNT_CHANGE_REGIONS)

    static int64_t debug_last_refcount = 0;
    static int64_t debug_last_refcount2 = 0;

    void debug_refcount_change_start(agent* thisAgent, bool twoPart)
    {
        int lSymNum;
        std::string numString;
        numString.push_back(DEBUG_TRACE_REFCOUNT_FOR[1]);
        if (!from_string(lSymNum, std::string(numString)) || (lSymNum < 1) ) assert(false);
        Symbol *sym = thisAgent->symbolManager->find_identifier(DEBUG_TRACE_REFCOUNT_FOR[0], 1);
        if (sym)
        {
            int64_t* last_count = twoPart ? &(debug_last_refcount2) : &(debug_last_refcount);
            (*last_count) = sym->reference_count;
        };
    }
    void debug_refcount_change_end(agent* thisAgent, const char* callerName, const char* callerString, bool twoPart)
    {
        int lSymNum;
        std::string numString;
        numString.push_back(DEBUG_TRACE_REFCOUNT_FOR[1]);
        if (!from_string(lSymNum, std::string(numString)) || (lSymNum < 1) ) assert(false);
        Symbol *sym = thisAgent->symbolManager->find_identifier(DEBUG_TRACE_REFCOUNT_FOR[0], 1);
        if (sym)
        {
            int64_t new_count = static_cast<int64_t>(sym->reference_count);
            int64_t* last_count = twoPart ? &(debug_last_refcount2) : &(debug_last_refcount);
            if (new_count != (*last_count))
            {
                dprint_noprefix(DT_ID_LEAKING, "%s Reference count of %s %s changed (%d -> %d) by %d\n", callerName, callerString, DEBUG_TRACE_REFCOUNT_FOR,
                    (*last_count), new_count, (new_count - (*last_count)));
            }
            (*last_count) = 0;
            if (twoPart) debug_last_refcount2 = debug_last_refcount2 + (new_count - debug_last_refcount);
        };
    }
    void debug_refcount_reset()
    {
        debug_last_refcount = 0;
        debug_last_refcount2 = 0;
    }

#else
    void debug_refcount_change_start(agent* thisAgent, bool twoPart) {}
    void debug_refcount_change_end(agent* thisAgent, const char* callerName, const char* callerString, bool twoPart) {}
    void debug_refcount_reset() {}
#endif

#ifdef DEBUG_GDS_INVENTORY
    id_to_string_map gds_deallocation_map;

    uint64_t GDI_id_counter = 0;
    bool     GDI_double_deallocation_seen = false;

    void GDI_add(agent* thisAgent, goal_dependency_set* pGDS)
    {
        std::string lPrefString;
        pGDS->g_id = ++GDI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lPrefString, "GDS %u (%y)", pGDS->g_id, pGDS->goal);
        gds_deallocation_map[pGDS->g_id] = lPrefString;
    }
    void GDI_remove(agent* thisAgent, goal_dependency_set* pGDS)
    {
        auto it = gds_deallocation_map.find(pGDS->g_id);
        assert (it != gds_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            gds_deallocation_map[pGDS->g_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "GDS %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            GDI_double_deallocation_seen = true;
        }
    }
    void GDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lPrefString;
        uint64_t bugCount = 0;
        for (auto it = gds_deallocation_map.begin(); it != gds_deallocation_map.end(); ++it)
        {
            lPrefString = it->second;
            if (!lPrefString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "GDS inventory:            ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, GDI_id_counter);
                if (GDI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some GDSs were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (GDI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u GDSs were deallocated properly.\n", GDI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No GDSs were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = gds_deallocation_map.begin(); it != gds_deallocation_map.end(); ++it)
                {
                    lPrefString = it->second;
                    if (!lPrefString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lPrefString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || GDI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " GDSets leaked";
            else
                std::cout << "GDS inventory failure.  Leaked goal dependency sets detected.\n";
            //assert(false);
        }
        GDI_double_deallocation_seen = false;
        gds_deallocation_map.clear();
        GDI_id_counter = 0;
    }
#else
    void GDI_add(agent* thisAgent, goal_dependency_set* pGDS) {}
    void GDI_remove(agent* thisAgent, goal_dependency_set* pGDS) {}
    void GDI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_INSTANTIATION_INVENTORY
    id_to_string_map IDI_deallocation_map;
    bool             IDI_double_deallocation_seen = false;

    void IDI_add(agent* thisAgent, instantiation* pInst)
    {
        std::string lInstString;
        thisAgent->outputManager->sprinta_sf(thisAgent, lInstString, "i %u in %y (%d)", pInst->i_id, pInst->match_goal, static_cast<int64_t>(pInst->match_goal_level));
        IDI_deallocation_map[pInst->i_id] = lInstString;
    }
    void IDI_remove(agent* thisAgent, uint64_t pID)
    {
        auto it = IDI_deallocation_map.find(pID);
        assert (it != IDI_deallocation_map.end());

        std::string lInstString = it->second;
        if (!lInstString.empty())
        {
            IDI_deallocation_map[pID].clear();
        } else {
            std::string lInstString;
            thisAgent->outputManager->sprinta_sf(thisAgent, lInstString, "Instantiation %u was deallocated twice!\n", it->first);
            IDI_deallocation_map[pID] = lInstString;
            break_if_bool(true);
            IDI_double_deallocation_seen = true;
        }
    }
    void IDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lInstString;
        uint64_t bugCount = 0;

        for (auto it = IDI_deallocation_map.begin(); it != IDI_deallocation_map.end(); ++it)
        {
            lInstString = it->second;
            if (!lInstString.empty())
            {
                bugCount++;
            }
        }

        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Instantiation inventory:  ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, IDI_deallocation_map.size());
                if (IDI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some instantiations sets were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (IDI_deallocation_map.size())
                thisAgent->outputManager->printa_sf(thisAgent, "All %u instantiations were deallocated properly.\n", IDI_deallocation_map.size());
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No instantiations were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = IDI_deallocation_map.begin(); it != IDI_deallocation_map.end(); ++it)
                {
                    lInstString = it->second;
                    if (!lInstString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lInstString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || IDI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " instantiations leaked";
            else
                std::cout << "Instantiation inventory failure.  Leaked instantiations detected.\n";
            //assert(false);
        }
        IDI_double_deallocation_seen = false;
        IDI_deallocation_map.clear();
    }
#else
    void IDI_add(agent* thisAgent, instantiation* pInst) {}
    void IDI_remove(agent* thisAgent, uint64_t pID) {}
    void IDI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_PREFERENCE_INVENTORY
    id_to_string_map pref_deallocation_map;

    uint64_t PDI_id_counter = 0;
    bool     PDI_double_deallocation_seen = false;

    void PDI_add(agent* thisAgent, preference* pPref, bool isShallow)
    {
        std::string lPrefString;
        pPref->p_id = ++PDI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lPrefString, "%u: %p", pPref->p_id, pPref);
        pref_deallocation_map[pPref->p_id] = lPrefString;
    }
    void PDI_remove(agent* thisAgent, preference* pPref)
    {
        auto it = pref_deallocation_map.find(pPref->p_id);
        assert (it != pref_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            pref_deallocation_map[pPref->p_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "Preferences %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            PDI_double_deallocation_seen = true;
        }
    }

    void PDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lPrefString;
        uint64_t bugCount = 0;
        for (auto it = pref_deallocation_map.begin(); it != pref_deallocation_map.end(); ++it)
        {
            lPrefString = it->second;
            if (!lPrefString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Preference inventory:     ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, PDI_id_counter);
                if (PDI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some preferences were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (PDI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u preferences were deallocated properly.\n", PDI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No preferences were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = pref_deallocation_map.begin(); it != pref_deallocation_map.end(); ++it)
                {
                    lPrefString = it->second;
                    if (!lPrefString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lPrefString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || PDI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " preferences leaked";
            else
                std::cout << "Preference inventory failure.  Leaked preferences detected.\n";
            //assert(false);
        }
        PDI_double_deallocation_seen = false;
        pref_deallocation_map.clear();
        PDI_id_counter = 0;
    }
#else
    void PDI_add(agent* thisAgent, preference* pPref, bool isShallow) {}
    void PDI_remove(agent* thisAgent, preference* pPref) {}
    void PDI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_WME_INVENTORY
    id_to_string_map wme_deallocation_map;

    uint64_t WDI_id_counter = 0;
    bool     WDI_double_deallocation_seen = false;

    void WDI_add(agent* thisAgent, wme* pWME)
    {
        std::string lAddString;
        pWME->w_id = ++WDI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lAddString, "%u: %w", pWME->w_id, pWME);
        wme_deallocation_map[pWME->w_id] = lAddString;
    }
    void WDI_remove(agent* thisAgent, wme* pWME)
    {
        auto it = wme_deallocation_map.find(pWME->w_id);
        assert (it != wme_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            wme_deallocation_map[pWME->w_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "WME %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            WDI_double_deallocation_seen = true;
        }
    }

    void WDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lAddString;
        uint64_t bugCount = 0;
        for (auto it = wme_deallocation_map.begin(); it != wme_deallocation_map.end(); ++it)
        {
            lAddString = it->second;
            if (!lAddString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "WME inventory:            ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, WDI_id_counter);
                if (WDI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some WMEs were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (WDI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u WMEs were deallocated properly.\n", WDI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No WMEs were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = wme_deallocation_map.begin(); it != wme_deallocation_map.end(); ++it)
                {
                    lAddString = it->second;
                    if (!lAddString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lAddString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || WDI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " WMEs leaked";
            else
                std::cout << "WME inventory failure.  Leaked WMEs detected.\n";
            //assert(false);
        }
        WDI_double_deallocation_seen = false;
        wme_deallocation_map.clear();
        WDI_id_counter = 0;
    }
#else
    void WDI_add(agent* thisAgent, wme* pWME) {}
    void WDI_remove(agent* thisAgent, wme* pWME) {}
    void WDI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_IDSET_INVENTORY
    id_to_string_map idset_deallocation_map;

    bool     ISI_double_deallocation_seen = false;

    void ISI_add(agent* thisAgent, uint64_t pIdentityID)
    {
        std::string lPrefString;
        thisAgent->outputManager->sprinta_sf(thisAgent, lPrefString, "%u", pIdentityID);
        idset_deallocation_map[pIdentityID].assign(lPrefString);
//        break_if_id_matches(pIdentityID, 3);
    }
    void ISI_remove(agent* thisAgent, uint64_t pIdentityID)
    {
        auto it = idset_deallocation_map.find(pIdentityID);
        assert (it != idset_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            idset_deallocation_map[pIdentityID].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "Identity set %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            ISI_double_deallocation_seen = true;
        }
    }
    void ISI_print_and_cleanup(agent* thisAgent)
    {
        std::string lPrefString;
        uint64_t bugCount = 0;
        for (auto it = idset_deallocation_map.begin(); it != idset_deallocation_map.end(); ++it)
        {
            lPrefString = it->second;
            if (!lPrefString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Identity inventory:       ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%d were not deallocated", bugCount, static_cast<int64_t>(idset_deallocation_map.size()));
                if (ISI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some identities were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (idset_deallocation_map.size())
                thisAgent->outputManager->printa_sf(thisAgent, "All %d identities were deallocated properly.\n", static_cast<int64_t>(idset_deallocation_map.size()));
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No identities were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = idset_deallocation_map.begin(); it != idset_deallocation_map.end(); ++it)
                {
                    lPrefString = it->second;
                    if (!lPrefString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lPrefString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || ISI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " identities leaked";
            else
                std::cout << "Identity inventory failure.  Leaked identities detected.\n";
            //assert(false);
        }
        ISI_double_deallocation_seen = false;
        idset_deallocation_map.clear();
    }
#else
    void ISI_add(agent* thisAgent, uint64_t pIdentityID) {}
    void ISI_remove(agent* thisAgent, uint64_t pIdentityID) {}
    void ISI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_RHS_SYMBOL_INVENTORY
    id_to_string_map rhs_deallocation_map;

    uint64_t RSI_id_counter = 0;
    bool    RSI_double_deallocation_seen = false;

    void RSI_add(agent* thisAgent, rhs_symbol pRHS)
    {
        /* This assert may help detect cases when we get those pseudo-null rhs_values that give us problems (null with type bit set)*/
        assert(pRHS);
        std::string lAddString;
        pRHS->r_id = ++RSI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lAddString, "%u: %r", pRHS->r_id, pRHS);
        rhs_deallocation_map[pRHS->r_id] = lAddString;
//        std::cout << "Allocating rhs symbol " << pRHS->referent->to_string() << " " << pRHS->r_id << std::endl;

//        break_if_id_matches(pRHS->r_id, 2038);
    }
    void RSI_remove(agent* thisAgent, rhs_symbol pRHS)
    {
//        std::cout << "Deallocating rhs symbol " << pRHS->referent->to_string() << " " << pRHS->r_id << std::endl;
        auto it = rhs_deallocation_map.find(pRHS->r_id);
        assert (it != rhs_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            rhs_deallocation_map[pRHS->r_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "RHS symbol %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            RSI_double_deallocation_seen = true;
        }
    }

    void RSI_print_and_cleanup(agent* thisAgent)
    {
        std::string lAddString;
        uint64_t bugCount = 0;
        for (auto it = rhs_deallocation_map.begin(); it != rhs_deallocation_map.end(); ++it)
        {
            lAddString = it->second;
            if (!lAddString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "RHS Symbol inventory:     ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, RSI_id_counter);
                if (RSI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some RHS symbols were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (RSI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u RHS symbols were deallocated properly.\n", RSI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No RHS symbols were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = rhs_deallocation_map.begin(); it != rhs_deallocation_map.end(); ++it)
                {
                    lAddString = it->second;
                    if (!lAddString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lAddString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || RSI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " RHS symbols leaked";
            else
                std::cout << "RHS symbols inventory failure.  Leaked RHS symbols detected.\n";
            //assert(false);
        }
        RSI_double_deallocation_seen = false;
        rhs_deallocation_map.clear();
        RSI_id_counter = 0;
    }
#else
    void RSI_add(agent* thisAgent, rhs_symbol pRHS) {}
    void RSI_remove(agent* thisAgent, rhs_symbol pRHS) {}
    void RSI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_ACTION_INVENTORY
    id_to_string_map action_deallocation_map;

    uint64_t ADI_id_counter = 0;
    bool    ADI_double_deallocation_seen = false;

    void ADI_add(agent* thisAgent, action* pAction)
    {
        std::string lAddString;
        pAction->a_id = ++ADI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lAddString, "a%u", pAction->a_id);
        action_deallocation_map[pAction->a_id] = lAddString;
//        thisAgent->outputManager->printa_sf(thisAgent, "Action inventory adding %u\n", pAction->a_id);

//        break_if_id_matches(pAction->a_id, 20);
    }
    void ADI_remove(agent* thisAgent, action* pAction)
    {
//        thisAgent->outputManager->printa_sf(thisAgent, "Action inventory removing %u\n", pAction->a_id);

        auto it = action_deallocation_map.find(pAction->a_id);
        assert (it != action_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            action_deallocation_map[pAction->a_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "Action %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            ADI_double_deallocation_seen = true;
        }
//        break_if_id_matches(pAction->a_id, 581);
    }

    void ADI_print_and_cleanup(agent* thisAgent)
    {
        std::string lAddString;
        uint64_t bugCount = 0;
        for (auto it = action_deallocation_map.begin(); it != action_deallocation_map.end(); ++it)
        {
            lAddString = it->second;
            if (!lAddString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Action inventory:         ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, ADI_id_counter);
                if (ADI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some actions were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (ADI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u actions were deallocated properly.\n", ADI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No actions were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = action_deallocation_map.begin(); it != action_deallocation_map.end(); ++it)
                {
                    lAddString = it->second;
                    if (!lAddString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lAddString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || ADI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " actions leaked";
            else
                std::cout << "Action inventory failure.  Leaked actions detected.\n";
            //assert(false);
        }
        ADI_double_deallocation_seen = false;
        action_deallocation_map.clear();
        ADI_id_counter = 0;
    }
#else
    void ADI_add(agent* thisAgent, action* pAction) {}
    void ADI_remove(agent* thisAgent, action* pAction) {}
    void ADI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_RHS_FUNCTION_INVENTORY
    rhs_val_to_string_map rhs_func_deallocation_map;

    uint64_t RFI_id_counter = 0;
    bool    RFI_double_deallocation_seen = false;

    void RFI_add(agent* thisAgent, rhs_value pRHS)
    {
        std::string lAddString;
        /* This assert may help detect cases when we get those pseudo-null rhs_values that give us problems (null with type bit set)*/
        assert(pRHS);
        thisAgent->outputManager->sprinta_sf(thisAgent, lAddString, "%u: &%u", (++RFI_id_counter), (uint64_t) pRHS);
        rhs_func_deallocation_map[pRHS] = lAddString;
//        break_if_id_matches(RFI_id_counter, 890);
    }
    void RFI_remove(agent* thisAgent, rhs_value pRHS)
    {
        auto it = rhs_func_deallocation_map.find(pRHS);
        assert (it != rhs_func_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            rhs_func_deallocation_map[pRHS].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "RHS function was deallocated twice!: %s\n", lPrefString.c_str());
            break_if_bool(true);
            RFI_double_deallocation_seen = true;
        }
    }

    void RFI_print_and_cleanup(agent* thisAgent)
    {
        std::string lAddString;
        uint64_t bugCount = 0;
        for (auto it = rhs_func_deallocation_map.begin(); it != rhs_func_deallocation_map.end(); ++it)
        {
            lAddString = it->second;
            if (!lAddString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "RHS Function inventory:   ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, RFI_id_counter);
                if (RFI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some RHS functions were deallocated twice");
                if (bugCount <= 50)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (RFI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u RHS functions were deallocated properly.\n", RFI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No RHS functions were created.\n");

            if (bugCount && (bugCount <= 50))
            {
                for (auto it = rhs_func_deallocation_map.begin(); it != rhs_func_deallocation_map.end(); ++it)
                {
                    lAddString = it->second;
                    if (!lAddString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lAddString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || RFI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " RHS functions leaked";
            else
                std::cout << "RHS functions inventory failure.  Leaked RHS functions detected.\n";

            //assert(false);
        }
        RFI_double_deallocation_seen = false;
        rhs_func_deallocation_map.clear();
        RFI_id_counter = 0;
    }
#else
    void RFI_add(agent* thisAgent, rhs_value pRHS) {}
    void RFI_remove(agent* thisAgent, rhs_value pRHS) {}
    void RFI_print_and_cleanup(agent* thisAgent) {}
#endif

#ifdef DEBUG_TEST_INVENTORY
    id_to_string_map test_deallocation_map;

    uint64_t TDI_id_counter = 0;
    bool    TDI_double_deallocation_seen = false;

    void TDI_add(agent* thisAgent, test pTest)
    {
        std::string lAddString;
        pTest->t_id = ++TDI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lAddString, "%u: %t", pTest->t_id, pTest);
        test_deallocation_map[pTest->t_id] = lAddString;
//        std::string lTestString;
//        thisAgent->outputManager->test_to_string(pTest, lTestString, true);
//        std::cout << "Allocating test " << pTest->t_id << ": " << lTestString << "\n";

//        break_if_id_matches(pTest->t_id, 2526);
    }
    void TDI_remove(agent* thisAgent, test pTest)
    {
//        std::string lTestString;
//        thisAgent->outputManager->test_to_string(pTest, lTestString, true);
//        std::cout << "DEALLOCATE test " << pTest->t_id <<  ": " << lTestString<< "\n";

        auto it = test_deallocation_map.find(pTest->t_id);
        assert (it != test_deallocation_map.end());

        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
            test_deallocation_map[pTest->t_id].clear();
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "test %u was deallocated twice!\n", it->first);
            break_if_bool(true);
            TDI_double_deallocation_seen = true;
        }
    }

    void TDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lAddString;
        uint64_t bugCount = 0;
        for (auto it = test_deallocation_map.begin(); it != test_deallocation_map.end(); ++it)
        {
            lAddString = it->second;
            if (!lAddString.empty())
            {
                bugCount++;
            }
        }
        if (!Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Test inventory:           ");
            if (bugCount)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%u/%u were not deallocated", bugCount, TDI_id_counter);
                if (TDI_double_deallocation_seen)
                    thisAgent->outputManager->printa_sf(thisAgent, " and some tests were deallocated twice");
                if (bugCount <= 23)
                    thisAgent->outputManager->printa_sf(thisAgent, ":");
                else
                    thisAgent->outputManager->printa_sf(thisAgent, "!\n");
            }
            else if (TDI_id_counter)
                thisAgent->outputManager->printa_sf(thisAgent, "All %u tests were deallocated properly.\n", TDI_id_counter);
            else
                thisAgent->outputManager->printa_sf(thisAgent, "No tests were created.\n");

            if (bugCount && (bugCount <= 23))
            {
                for (auto it = test_deallocation_map.begin(); it != test_deallocation_map.end(); ++it)
                {
                    lAddString = it->second;
                    if (!lAddString.empty()) thisAgent->outputManager->printa_sf(thisAgent, " %s", lAddString.c_str());
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        }
        if (((bugCount > 0) || TDI_double_deallocation_seen) && Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
        {
            if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
                std::cout << ' ' << bugCount << " tests leaked";
            else
                std::cout << "Test inventory failure! " << bugCount << "/" << TDI_id_counter << " were not deallocated.\n";
            //assert(false);
        }
        TDI_double_deallocation_seen = false;
        test_deallocation_map.clear();
        TDI_id_counter = 0;
    }
#else
    void TDI_add(agent* thisAgent, test pTest) {}
    void TDI_remove(agent* thisAgent, test pTest) {}
    void TDI_print_and_cleanup(agent* thisAgent) {}
#endif
