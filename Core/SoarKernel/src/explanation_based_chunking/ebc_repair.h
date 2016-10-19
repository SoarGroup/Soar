#ifndef CONNECT_CONDITIONS_H_
#define CONNECT_CONDITIONS_H_

#include "kernel.h"
#include "stl_typedefs.h"

class Path_to_Goal_State
{
    public:
        Path_to_Goal_State(Symbol* new_root, wme_list* new_path = NULL, wme* new_wme = NULL) {
            topSym = new_root;
            wme_path = new wme_list();
            if (new_path) {
                (*wme_path) = (*new_path);
                if (new_wme) wme_path->push_front(new_wme);
            }
        }
        ~Path_to_Goal_State() { delete wme_path; }

        Symbol* get_root() {return topSym;}
        wme_list* get_path() {return wme_path;}

    private:

        Symbol*     topSym;
        wme_list*   wme_path;
};

void delete_ungrounded_symbol_list(agent* thisAgent, symbol_with_match_list** unconnected_syms);

class Repair_Manager
{
    public:

        Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level, uint64_t p_chunk_ID);
        ~Repair_Manager();

        void        repair_rule(condition*& m_vrblz_top, condition*& m_inst_top, condition*& m_inst_bottom, symbol_with_match_list* pUnconnected_LTIs);

    private:

        agent*                  thisAgent;
        sym_to_sym_map     m_sym_to_var_map;
        sym_to_id_map      m_sym_to_id_map;
        wme_set                 m_repair_WMEs;
        goal_stack_level        m_match_goal_level;
        uint64_t                m_chunk_ID;


        wme_list*   find_path_to_goal_for_symbol(Symbol* targetLTI);
        void        mark_states_in_cond_list(condition* pCondList, tc_number pTC);
        condition*  make_condition_from_wme(wme* lWME);
        void        add_variablization(Symbol* pSym, Symbol* pVar, uint64_t pIdentity, const char* pTypeStr = "existing state");
        void        variablize_connecting_sti(test pTest);
        void        add_path_to_goal_WMEs(symbol_with_match* pTargetSym);
        void        add_state_link_WMEs(goal_stack_level pTargetGoal, tc_number pSeenTC);


};
#endif /* CONNECT_CONDITIONS_H_ */
