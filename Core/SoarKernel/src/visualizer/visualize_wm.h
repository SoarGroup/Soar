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

typedef struct aug_struct
{
        Symbol* attr;
        Symbol* value;
} augmentation;

class WM_Visualization_Map
{
    public:

        WM_Visualization_Map(agent* myAgent);
        ~WM_Visualization_Map();

        void reset();
        void add_triple(Symbol* id, Symbol* attr, Symbol* value);
        void add_current_wm();
        void visualize_wm_as_linked_records();
        void visualize_wm_as_graph();

    private:

        agent*             thisAgent;
        sym_to_aug_map*    id_augmentations;
};

#endif /* CORE_SOARKERNEL_SRC_VISUALIZER_VISUALIZE_WM_H_ */
