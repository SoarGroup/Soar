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
    double                  edge_weight;
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

typedef std::pair<double, std::list<std::pair<uint64_t,double>>*> smem_activated_lti_list;

struct smem_compare_activated_lti_list
{
    bool operator()(const smem_activated_lti_list a, const smem_activated_lti_list b) const
    {
        return (b.first > a.first);
    }
};

typedef std::priority_queue<smem_activated_lti_list, std::vector<smem_activated_lti_list>, smem_compare_activated_lti_list> smem_prioritized_lti_traversal_queue;

typedef struct smem_edge_update_struct
{
    uint64_t lti_edge_id;
    double num_touches;
    uint64_t update_time;
} smem_edge_update;

typedef std::unordered_map<uint64_t, std::list<smem_edge_update*>> smem_update_map;

typedef union ltm_value_union
{
    struct ltm_value_const       val_const;
    struct ltm_value_lti         val_lti;
} ltm_value;

struct activation_decomposition
{
    double base_level;
    double base_inhibition; //In cases where this is difficult to isolate (especially using incremental base-level updating), may only report base_level_total.
                            //May effectively force smem -q to be "update-policy naive", except if done wrong, could change database state unexpectedly. not implementing for now.
    double base_level_total;
    double spread_total;
    double activation_total;
    uint64_t recipient;
    std::set<uint64_t> contributing_sources;
    std::map<uint64_t,double> source_to_network_factor;

    activation_decomposition(uint64_t recipient)//double base_level, double base_inhibition, double base_level_total, double spread_total, double activation_total, uint64_t recipient, std::set<uint64_t> contributing_sources)
        : base_level(0), base_inhibition(0), base_level_total(0), spread_total(0), activation_total(0), recipient(recipient), contributing_sources(), source_to_network_factor()
    {
    }
};

struct command_line_activation_metadata
{//This needs to contain on a recipient basis the spread, base-level act, base-level inhibition, and then decomposition of spread.
            //Decomposition of spread needs to be on a source by source basis. For each source,
            //WMA_factor, network_factor. For each WMA factor, want to also have separation of history by instance. can then post-calculate here the instances' effective WMA_factors.
            //Can separately contain map from each source instance to its WMA elements within this struct (not stored on a recipient basis) for use in calculating here the WMA_factors for individual instances.
            //Because the WMA factors are shared across all recipients from a source, will not issue them alongside individual recipient results.
            //Network factors as pairs, source, recipient, network factor.

    std::map<uint64_t,activation_decomposition> recipient_decomposition_list;
    std::map<uint64_t,double> contributing_sources_to_WMA_factors;
    std::multimap<uint64_t,Symbol*> contributing_sources_to_instances;
    std::multimap<Symbol*,wma_decay_element*> instances_to_WMA_decay_elements;
    //std::map<std::pair<uint64_t,uint64_t>,double> source_and_recipient_to_network_factors;
};

#endif /* CORE_SOARKERNEL_SRC_SEMANTIC_MEMORY_SMEM_STRUCTS_H_ */
