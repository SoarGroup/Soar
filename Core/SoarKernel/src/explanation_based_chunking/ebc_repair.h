#ifndef CONNECT_CONDITIONS_H_
#define CONNECT_CONDITIONS_H_

#include "kernel.h"
#include "stl_typedefs.h"

class Repair_Path
{
    public:
        Repair_Path() {};
        ~Repair_Path() {};

        void init(Symbol* new_root, wme_list* new_path = NULL, wme* new_wme = NULL)
        {
            topSym = new_root;
            wme_path = new wme_list();
            if (new_path) {
                (*wme_path) = (*new_path);
                if (new_wme) wme_path->push_front(new_wme);
            }
        }
        void clean_up() { delete wme_path; }

        Symbol* get_root() {return topSym;}
        wme_list* get_path() {return wme_path;}

    private:

        Symbol*     topSym;
        wme_list*   wme_path;
};

void delete_ungrounded_symbol_list(agent* thisAgent, matched_symbol_list** unconnected_syms);

class Repair_Manager
{
    public:

        Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level, uint64_t p_chunk_ID);
        ~Repair_Manager();

        void        repair_rule(condition*& p_lhs_top, matched_symbol_list* pUnconnected_LTIs);

    private:

        agent*                  thisAgent;
        sym_to_sym_map          m_sym_to_var_map;
        sym_to_id_map           m_sym_to_id_map;
        wme_set                 m_repair_WMEs;
        goal_stack_level        m_match_goal_level;
        uint64_t                m_chunk_ID;


        wme_list*   find_path_to_goal_for_symbol(Symbol* targetLTI);
        void        mark_states_WMEs_and_store_variablizations(condition* pCondList, tc_number pTC);
        condition*  make_condition_from_wme(wme* lWME);
        void        add_variablization(Symbol* pSym, Symbol* pVar, uint64_t pIdentity, const char* pTypeStr = "existing state");
        void        variablize_connecting_sti(test pTest);
        void        add_path_to_goal_WMEs(chunk_element* pTargetSym, tc_number cond_tc);
        void        add_state_link_WMEs(goal_stack_level pTargetGoal, tc_number pSeenTC);


};
#endif /* CONNECT_CONDITIONS_H_ */
