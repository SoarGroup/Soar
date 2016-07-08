#ifndef CONNECT_CONDITIONS_H_
#define CONNECT_CONDITIONS_H_

#include "kernel.h"
#include "stl_typedefs.h"

typedef struct symbol_with_match_struct {
        Symbol*     sym;
        Symbol*     matched_sym;
        uint64_t    identity;
        symbol_with_match_struct() {
            sym = NULL;
            matched_sym = NULL;
            identity = 0;
        }
} symbol_with_match;

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

void delete_ungrounded_symbol_list(symbol_with_match_list** unconnected_syms);

class Repair_Manager
{
    public:

        Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level);
        ~Repair_Manager();

        void        repair_rule(condition*& m_vrblz_top, condition*& m_inst_top, condition*& m_inst_bottom, symbol_with_match_list* pUnconnected_LTIs, uint64_t pInstID);

    private:

        agent*                  thisAgent;
        sym_to_sym_map_type     m_sym_to_var_map;
        wme_set                 m_repair_WMEs;
        goal_stack_level        m_match_goal_level;

        void        add_var_for_sym(Symbol* pSym, Symbol* pVar);
        void        variablize_connecting_sti(Symbol*& pSym);

        wme_list*   find_path_to_goal_for_symbol(Symbol* targetLTI);
        condition*  find_cond_for_unconnected_var(condition* pCondList, Symbol* pUnconnected_LTI);
        void        mark_states_in_cond_list(condition* pCondList, tc_number pTC);
        condition*  make_condition_from_wme(wme* lWME);
        void        add_path_to_goal_WMEs(symbol_with_match* lUngroundedSym);
        void        add_state_link_WMEs(goal_stack_level pTargetGoal, tc_number pSeenTC);


};
#endif /* CONNECT_CONDITIONS_H_ */
