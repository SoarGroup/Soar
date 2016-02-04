
#include "ebc.h"

#include "agent.h"
#include "connect_conditions.h"
#include "debug.h"
#include "slot.h"
#include "working_memory.h"

void Explanation_Based_Chunker::walk_and_find_lti(Symbol* root, Symbol* targetLTI)
{
    slot* s;
    preference* pref;
    wme* w;
    dl_cons* dc;
    Symbol* idSym;

//#ifdef USE_MEM_POOL_ALLOCATORS
//    symbol_list ids_to_walk = symbol_list(soar_module::soar_memory_pool_allocator< Symbol* >());
//#else
//    symbol_list ids_to_walk;
//#endif
    sym_grounding_path_list ids_to_walk;
    sym_grounding_path* lCurrentPath = NULL, *lNewPath = NULL;

//    dprint(DT_GROUND_LTI, "walk_and_find_lti called to find %y in %y.\n", targetLTI, root);

    lNewPath = new sym_grounding_path(root);
    ids_to_walk.push_back(lNewPath);
    root->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath) delete lCurrentPath;
        lCurrentPath = ids_to_walk.back();
        ids_to_walk.pop_back();
        idSym = lCurrentPath->get_root();

//        dprint(DT_GROUND_LTI, "- Processing %y.  symbol level = %d, walk_level = %d, walk tc = %u\n",
//            idSym, idSym->id->level, root->id->level, ground_lti_tc);

        /* --- if we already know its level, and it's higher up, then exit --- */
//        if (idSym->id->isa_goal)
//        {
//            dprint(DT_GROUND_LTI, "...This id is a goal: %y!\n", id);
//            continue;
//        }

        /* -- scan through all preferences and wmes for all slots for this id -- */
//        dprint(DT_GROUND_LTI, "   Adding IDs from input wme's of %y to walk list:\n", id);
//        for (w = idSym->id->input_wmes; w != NIL; w = w->next)
//        {
//            if (w->value->is_identifier() && (w->value->tc_num != ground_lti_tc))
//            {
//                if (w->value == targetLTI)
//                {
//                    dprint(DT_GROUND_LTI, "      - Found target LTI!!!\n");
//                    /* Print currID->wme_path + w */
//                }
//                dprint(DT_GROUND_LTI, "      - from input wme: %y (%y ^%y %y)\n", w->value, w->id, w->attr, w->value);
//                w->value->tc_num = ground_lti_tc;
//                lNewPath = new sym_grounding_path(w->value, currID->wme_path, w);
//                ids_to_walk.push_back(lNewPath);
//            } else {
//
//            }
//        }

//        dprint(DT_GROUND_LTI, "   Adding IDs from slots of %y to walk list:\n", idSym);
        for (s = idSym->id->slots; s != NIL; s = s->next)
        {
//            dprint(DT_GROUND_LTI, "   - from slot preferences for %y ^%y:\n", s->id, s->attr);
//            for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
//            {
//                if (pref->value->is_identifier() && (pref->value->tc_num != ground_lti_tc))
//                {
//                    if (pref->value == targetLTI)
//                    {
//                        dprint(DT_GROUND_LTI, "      - Found target LTI!!!\n");
//                    }
//                    dprint(DT_GROUND_LTI, "      - from pref: %p\n", pref);
//                    pref->value->tc_num = ground_lti_tc;
//                    lNewPath = new sym_grounding_path(pref->value, lCurrentPath->get_path());
//                    ids_to_walk.push_back(lNewPath);
//                }
//            }
//            if (s->impasse_id)
//            {
//                dprint(DT_GROUND_LTI, "   - from slot impasse id for %y ^%y:\n", s->id, s->attr);
//                if (s->impasse_id->is_identifier() && (s->impasse_id->tc_num != ground_lti_tc))
//                {
//                    if (s->impasse_id == targetLTI)
//                    {
//                        dprint(DT_GROUND_LTI, "      - Found target LTI!!!\n");
//                    }
//                    dprint(DT_GROUND_LTI, "      - from impasse id: %y\n", s->impasse_id);
//                    s->impasse_id->tc_num = ground_lti_tc;
//                    lNewPath = new sym_grounding_path(pref->value, currID->wme_path);
//                    ids_to_walk.push_back(lNewPath);
//                    ids_to_walk.push_back(s->impasse_id);
//                }
//            }
//            dprint(DT_GROUND_LTI, "   - from slot wme's for %y ^%y:\n", s->id, s->attr);
            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (w->value->is_identifier() && (w->value->tc_num != ground_lti_tc))
                {
//                    dprint(DT_GROUND_LTI, "      - from slot wme: (%y ^%y %y)\n", w->id, w->attr, w->value);
                    w->value->tc_num = ground_lti_tc;
                    lNewPath = new sym_grounding_path(w->value, lCurrentPath->get_path(), w);
                    ids_to_walk.push_back(lNewPath);
                    if (w->value == targetLTI)
                    {
                        dprint(DT_GROUND_LTI, "      - Found target LTI!!!\nGrounding Path:\n");
                        wme_list* final_path = lNewPath->get_path();
                        for (auto it = final_path->rbegin(); it != final_path->rend(); it++) {
                            dprint(DT_GROUND_LTI, "      (%y ^%y %y)\n", (*it)->id, (*it)->attr, (*it)->value);
                        }
                    }
                } else {
//                    dprint(DT_GROUND_LTI, "      - skipping slot wme: (%y ^%y %y)\n", w->id, w->attr, w->value);
                }
            }
        }
    }
    dprint(DT_GROUND_LTI, "walk_and_find_lti DONE for %y.\n", root);
}

void Explanation_Based_Chunker::generate_conditions_to_ground_lti(Symbol* pUnconnected_LTI)
{
    sym_grounding_path_list ids_to_walk;

    dprint(DT_GROUND_LTI, "generate_conditions_to_ground_lti called for %y.\n", pUnconnected_LTI);
    Symbol* lIdSym;
    sym_grounding_path* lPath;
    wme* w;
    bool lPathFound = false;

    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < pUnconnected_LTI->id->level)
    {
        g = g->id->lower_goal;
    }

    walk_and_find_lti(g, pUnconnected_LTI);

//    while (true)
//    {
//        if (!g)
//        {
//            break;
//        }
//        walk_and_update_levels(thisAgent, g);
//        g = g->id->lower_goal;
//    }
//
//    lPath = new sym_grounding_path(pUnconnected_LTI);
//
//    ids_to_walk.push_back(lPath);
//
//    while (!ids_to_walk.empty() && !lPathFound)
//    {
//        lPath = ids_to_walk.front();
//        ids_to_walk.pop_front();
//        lIdSym = lPath->topSym;
//
//        /* --- mark id so we don't walk it twice --- */
//        lIdSym->tc_num = ground_lti_tc;
//
//        /* --- do the walk --- */
//
//
//        dprint(DT_GROUND_LTI, "Adding paths for input wmes of %y.\n", lIdSym);
//        for (w = lIdSym->id->input_wmes; w != NIL; w = w->next)
//        {
//            if (w->id->tc_num == ground_lti_tc)
//            {
//                dprint(DT_GROUND_LTI, "   Already marked.  Not adding path for %y through %w.\n", lIdSym, w);
//                continue;
//            }
//            dprint(DT_GROUND_LTI, "   Adding path for %y through %w.\n", lIdSym, w);
//            lPath = new sym_grounding_path(w->id, lPath->wme_path);
//            ids_to_walk.push_back(lPath);
//        }
//    }
}

