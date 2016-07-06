#ifndef CONNECT_CONDITIONS_H_
#define CONNECT_CONDITIONS_H_

#include "kernel.h"
#include "stl_typedefs.h"

typedef struct matched_identity_struct {
        Symbol*     sym;
        Symbol*     matched_sym;
        uint64_t    identity;
        matched_identity_struct() {
            sym = NULL;
            matched_sym = NULL;
            identity = 0;
        }
} matched_identity;

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

void delete_ungrounded_symbol_list(ungrounded_symbol_list** unconnected_syms);

class Repair_Manager
{
    public:

        Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level);
        ~Repair_Manager();

        void        repair_rule(condition*& m_vrblz_top, condition*& m_inst_top, condition*& m_inst_bottom, ungrounded_symbol_list* pUnconnected_LTIs, uint64_t pInstID);

    private:

        agent*              thisAgent;
        sym_to_id_map_type  m_sym_to_identity_map;
        wme_set             m_repair_WMEs;
        goal_stack_level    m_match_goal_level;

        wme_list*   find_path_to_goal_for_symbol(Symbol* targetLTI);
        condition*  find_cond_for_unconnected_var(condition* pCondList, Symbol* pUnconnected_LTI);
        condition*  make_condition_from_wme(wme* lWME);
        void        repair_add_path_to_goal_WMEs(matched_identity* lUngroundedSym);
        void        repair_add_state_link_WMEs(matched_identity* lUngroundedSym);


};
#endif /* CONNECT_CONDITIONS_H_ */
