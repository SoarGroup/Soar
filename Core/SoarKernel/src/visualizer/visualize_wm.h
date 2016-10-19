/*
 * visualize_wm.h
 *
 *  Created on: Sep 25, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_WM_H_
#define CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_WM_H_

#include "kernel.h"
#include "stl_typedefs.h"

class WM_Visualization_Map
{
    public:

        WM_Visualization_Map(agent* myAgent);
        ~WM_Visualization_Map();

        void reset();
        void add_triple(Symbol* id, Symbol* attr, Symbol* value);
        void add_current_wm();
        void visualize_wm_as_linked_records(Symbol* pSym, int depth);
        void visualize_wm_as_graph(Symbol* pSym, int depth);

    private:

        agent*             thisAgent;
        sym_to_aug_map*    id_augmentations;

        void add_wmes_of_id(Symbol* id, int depth, int maxdepth, tc_number tc);
        void mark_depths_augs_of_id(Symbol* id, int depth, tc_number tc);
        void get_wmes_for_symbol(Symbol* pSym, int pDepth);
};

#endif /* CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_WM_H_ */
