/*
 * smem_typedefs.h
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STRUCTS_H_
#define CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STRUCTS_H_

#include "kernel.h"

#include "stl_typedefs.h"

#include <queue>

typedef struct smem_data_struct
{   uint64_t                last_cmd_time[2];          // last update to smem.command
    uint64_t                last_cmd_count[2];         // last update to smem.command
    preference_list*        smem_wmes;          // wmes in last smem
    wme*                    smem_link_wme;
    wme*                    cmd_wme;
    wme*                    result_wme;
} smem_data;

typedef struct ltm_object_struct
{
    uint64_t                lti_id;
    ltm_slot_map*           slots;
} ltm_object;

struct ltm_value_const
{   smem_cue_element_type   val_type;
    Symbol*                 val_value;
};

struct ltm_value_lti
{   smem_cue_element_type   val_type;
    ltm_object*             val_value;
};

typedef struct smem_vis_lti_struct
{   uint64_t                lti_id;
    std::string             lti_name;
    unsigned int            level;
} smem_vis_lti;

typedef struct smem_weighted_cue_element_struct
{   uint64_t                weight;

    struct wme_struct*      cue_element;
    smem_hash_id            attr_hash;
    smem_hash_id            value_hash;
    uint64_t                value_lti;

    smem_cue_element_type   element_type;
    bool                    pos_element;
    MathQuery*              mathElement;

} smem_weighted_cue_element;

struct smem_compare_weighted_cue_elements
{
    bool operator()(const smem_weighted_cue_element* a, const smem_weighted_cue_element* b) const { return (a->weight > b->weight); }
};

struct smem_compare_activated_lti
{
    bool operator()(const smem_activated_lti a, const smem_activated_lti b) const { return (b.first > a.first); }
};

typedef std::priority_queue<smem_weighted_cue_element*, std::vector<smem_weighted_cue_element*>, smem_compare_weighted_cue_elements>    smem_prioritized_weighted_cue;
typedef std::priority_queue< smem_activated_lti, std::vector<smem_activated_lti>, smem_compare_activated_lti>                           smem_prioritized_activated_lti_queue;

typedef union ltm_value_union
{
    struct ltm_value_const       val_const;
    struct ltm_value_lti         val_lti;
} ltm_value;

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STRUCTS_H_ */
