#ifndef STL_TYPEDEFS_H_
#define STL_TYPEDEFS_H_

#include "kernel.h"

#include "mempool_allocator.h"
#include "stl_structs.h"

#include <map>
#include <string>
#include <set>
#include <unordered_set>
#include <list>
#include <vector>


struct IDSetLessThan;

#ifdef USE_MEM_POOL_ALLOCATORS

    typedef std::list< action_record*, soar_module::soar_memory_pool_allocator< action_record* > >                  action_record_list;
    typedef std::list< condition*, soar_module::soar_memory_pool_allocator< condition* > >                          condition_list;
    typedef std::list< condition_record*, soar_module::soar_memory_pool_allocator< condition_record* > >            condition_record_list;
    typedef std::list< constraint*, soar_module::soar_memory_pool_allocator< constraint* > >                        constraint_list;
    typedef std::list< Identity*, soar_module::soar_memory_pool_allocator< Identity* > >                            identity_list;
    typedef std::list< identity_mapping*, soar_module::soar_memory_pool_allocator< identity_mapping* > >            identity_mapping_list;
    typedef std::list< instantiation*, soar_module::soar_memory_pool_allocator< instantiation* > >                  inst_list;
    typedef std::list< instantiation_record*, soar_module::soar_memory_pool_allocator< instantiation_record* > >    inst_record_list;
    typedef std::list< chunk_element*, soar_module::soar_memory_pool_allocator< chunk_element* > >                  matched_symbol_list;
    typedef std::list< preference*, soar_module::soar_memory_pool_allocator< preference* > >                        preference_list;
    typedef std::list< Repair_Path*, soar_module::soar_memory_pool_allocator<Repair_Path*> >                        repair_path_list;
    typedef std::list< production*, soar_module::soar_memory_pool_allocator< production* > >                        production_list;
    typedef std::list< rhs_value, soar_module::soar_memory_pool_allocator< rhs_value > >                            rhs_value_list;
    typedef std::list< Symbol*, soar_module::soar_memory_pool_allocator< Symbol* > >                                symbol_list;
    typedef std::list< symbol_triple*, soar_module::soar_memory_pool_allocator< symbol_triple* > >                  symbol_triple_list;
    typedef std::list< deep_copy_wme*, soar_module::soar_memory_pool_allocator< deep_copy_wme* > >                  deep_copy_wme_list;
    typedef std::list< wme*, soar_module::soar_memory_pool_allocator< wme* > >                                      wme_list;

    typedef std::set< Identity*, IDSetLessThan,
                      soar_module::soar_memory_pool_allocator< Identity* > >                                        identity_set;
    typedef std::set< instantiation*, std::less< instantiation* >,
                      soar_module::soar_memory_pool_allocator< instantiation* > >                                   inst_set;
    typedef std::set< instantiation_record*, std::less< instantiation_record* >,
                      soar_module::soar_memory_pool_allocator< instantiation_record* > >                            inst_record_set;
    typedef std::set< production_record*, std::less< production_record* >,
                      soar_module::soar_memory_pool_allocator< production_record* > >                               production_record_set;
    typedef std::set< Symbol*, std::less< Symbol* >, soar_module::soar_memory_pool_allocator< Symbol* > >           symbol_set;
    typedef std::set< wma_decay_element*, std::less< wma_decay_element* >,
                      soar_module::soar_memory_pool_allocator< wma_decay_element* > >                               wma_decay_set;
    typedef std::set< wma_d_cycle, std::less< wma_d_cycle >,
                      soar_module::soar_memory_pool_allocator< wma_d_cycle > >                                      wma_decay_cycle_set;
    typedef std::set< wme*, std::less< wme* >, soar_module::soar_memory_pool_allocator< wme* > >                    wme_set;

    typedef std::map< Symbol*, Symbol*, std::less< Symbol* >,
                      soar_module::soar_memory_pool_allocator< std::pair< Symbol*, Symbol* > > >                    rl_symbol_map;
    typedef std::set< rl_symbol_map, std::less< rl_symbol_map >,
                      soar_module::soar_memory_pool_allocator< rl_symbol_map > >                                    rl_symbol_map_set;
    typedef std::map< production*, double, std::less< production* >,
                      soar_module::soar_memory_pool_allocator< std::pair< production*, double > > >                 rl_et_map;

    typedef std::map< wma_d_cycle, wma_decay_set*, std::less< wma_d_cycle >,
                      soar_module::soar_memory_pool_allocator< std::pair< wma_d_cycle, wma_decay_set* > > >         wma_forget_p_queue;
    typedef std::map< Symbol*, uint64_t, std::less< Symbol* >,
                      soar_module::soar_memory_pool_allocator< std::pair< Symbol*, uint64_t > > >                   wma_sym_reference_map;



#else
    typedef std::list< action_record* >                         action_record_list;
    typedef std::list< condition_record* >                      condition_record_list;
    typedef std::list< condition* >                             condition_list;
    typedef std::list< constraint* >                            constraint_list;
    typedef std::list< deep_copy_wme* >                         deep_copy_wme_list;
    typedef std::list< Identity* >                              identity_list;
    typedef std::list< identity_mapping* >                      identity_mapping_list;
    typedef std::list< instantiation* >                         inst_list;
    typedef std::list< instantiation_record* >                  inst_record_list;
    typedef std::list< chunk_element* >                         matched_symbol_list;
    typedef std::list< Repair_Path* >                           repair_path_list;
    typedef std::list< preference* >                            preference_list;
    typedef std::list< production* >                            production_list;
    typedef std::list< rhs_value >                              rhs_value_list;
    typedef std::list< symbol_triple* >                         symbol_triple_list;
    typedef std::list< Symbol* >                                symbol_list;
    typedef std::list< wme* >                                   wme_list;

    typedef std::set< Identity*, IDSetLessThan >                identity_set;
    typedef std::set< instantiation_record* >                   inst_record_set;
    typedef std::set< instantiation* >                          inst_set;
    typedef std::set< production_record* >                      production_record_set;
    typedef std::set< Symbol* >                                 symbol_set;
    typedef std::set< wma_decay_element* >                      wma_decay_set;
    typedef std::set< wma_d_cycle >                             wma_decay_cycle_set;
    typedef std::set< wme* >                                    wme_set;

    typedef std::map< production*, double >                     rl_et_map;
    typedef std::map< Symbol*, Symbol* >                        rl_symbol_map;
    typedef std::set< rl_symbol_map >                           rl_symbol_map_set;
    typedef std::map< wma_d_cycle, wma_decay_set* >             wma_forget_p_queue;
    typedef std::map< Symbol*, uint64_t >                       wma_sym_reference_map;

#endif

#if (!defined USE_MEM_POOL_ALLOCATORS) || (defined USE_UNORDERED_STL)
    typedef std::unordered_set< augmentation* >                     augmentation_set;
    typedef std::unordered_set< uint64_t >                          id_set;
    typedef std::unordered_map< uint64_t, uint64_t >                id_to_id_map;
    typedef std::unordered_map< uint64_t, Symbol* >                 id_to_sym_map;
    typedef std::unordered_map< uint64_t, Identity*>                id_to_join_map;
    typedef std::unordered_map< uint64_t, std::string >             id_to_string_map;
    typedef std::unordered_map< uint64_t, identity_mapping_list* >  inst_identities_map;
    typedef std::unordered_map< rhs_value, std::string >            rhs_val_to_string_map;
    typedef std::unordered_map< Symbol*, augmentation_set* >        sym_to_aug_map;
    typedef std::unordered_map< Symbol*, condition* >               sym_to_cond_map;
    typedef std::unordered_map< Symbol*, uint64_t >                 sym_to_id_map;
    typedef std::unordered_map< Symbol*, identity_set* >            sym_to_identity_set_map;
    typedef std::unordered_map< Symbol*, chunk_element* >           sym_to_sym_id_map;
    typedef std::unordered_map< Symbol*, sym_to_cond_map >          sym_to_sym_to_cond_map;
    typedef std::unordered_map< Symbol*, sym_to_sym_to_cond_map >   triple_merge_map;
    typedef std::unordered_map< Symbol*, chunk_record* >            chunk_record_symbol_map;
    typedef std::unordered_map< uint64_t, chunk_record* >           chunk_record_id_map;
    typedef std::unordered_map< uint64_t, instantiation_record* >   instantiation_record_map;
    typedef std::unordered_map< uint64_t, condition_record* >       condition_record_map;
    typedef std::unordered_map< uint64_t, action_record* >          action_record_map;
    typedef std::unordered_map< uint64_t, production* >             production_map;
    /*------ SMem stl typedefs ------*/
    // - Could create allocator versions of a lot of these
    // - Many of these could be replaced by more general versions above.  Same with epmem

    typedef std::unordered_set<ltm_object*>                 ltm_set;
    typedef std::vector<ltm_value*>                         ltm_slot;
    typedef std::unordered_map<Symbol*, ltm_slot*>          ltm_slot_map;
    typedef std::vector<smem_weighted_cue_element*>         smem_weighted_cue_list;
    typedef std::unordered_map<std::string, ltm_object*>    str_to_ltm_map;
    typedef std::unordered_map<Symbol*, ltm_object*>        sym_to_ltm_map;
#else
    /* The following were unordered STL containers, but in certain cases it seems allocation costs more than we gain.  Still
     * not sure whether to switch or whether we should just switch some.  Might try new allocator that can be used with STL
     * containers that request variable memory allocations like unordered_set, vector and unordered_map  */

    typedef std::set< augmentation*, std::less< augmentation* >, soar_module::soar_memory_pool_allocator< augmentation* > >     augmentation_set;
    typedef std::set< uint64_t, std::less< uint64_t >, soar_module::soar_memory_pool_allocator< uint64_t > >                    id_set;
    typedef std::set< ltm_object*, std::less< ltm_object* >, soar_module::soar_memory_pool_allocator< ltm_object* > >           ltm_set;

    typedef std::list< ltm_value*, soar_module::soar_memory_pool_allocator< ltm_value* > >                                      ltm_slot;
    typedef std::list< smem_weighted_cue_element*, soar_module::soar_memory_pool_allocator< smem_weighted_cue_element* > >      smem_weighted_cue_list;

    typedef std::map< uint64_t, uint64_t, std::less< uint64_t >,
                          soar_module::soar_memory_pool_allocator< std::pair< uint64_t, uint64_t > > >                id_to_id_map;
    typedef std::map< uint64_t, Symbol*, std::less< uint64_t >,
                          soar_module::soar_memory_pool_allocator< std::pair< uint64_t, Symbol* > > >                 id_to_sym_map;
    typedef std::map< uint64_t, Identity*, std::less< uint64_t >,
                          soar_module::soar_memory_pool_allocator< std::pair< uint64_t, Identity* > >>                id_to_join_map;
    typedef std::map< uint64_t, std::string, std::less< uint64_t >,
                          soar_module::soar_memory_pool_allocator< std::pair< uint64_t, std::string > > >             id_to_string_map;
    typedef std::map< uint64_t, identity_mapping_list*, std::less< uint64_t >,
                          soar_module::soar_memory_pool_allocator< std::pair< uint64_t, identity_mapping_list* > > >  inst_identities_map;
    typedef std::map< rhs_value, std::string, std::less< rhs_value >,
                          soar_module::soar_memory_pool_allocator< std::pair< rhs_value, std::string > > >            rhs_val_to_string_map;
    typedef std::map< Symbol*, augmentation_set*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, augmentation_set* > > >        sym_to_aug_map;
    typedef std::map< Symbol*, condition*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, condition* > > >               sym_to_cond_map;
    typedef std::map< Symbol*, uint64_t, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, uint64_t > > >                 sym_to_id_map;
    typedef std::map< Symbol*, identity_set*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, identity_set* > > >            sym_to_identity_set_map;
    typedef std::map< Symbol*, chunk_element*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, chunk_element* > > >           sym_to_sym_id_map;
    typedef std::map< Symbol*, sym_to_cond_map, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, sym_to_cond_map > > >          sym_to_sym_to_cond_map;
    typedef std::map< Symbol*, sym_to_sym_to_cond_map, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, sym_to_sym_to_cond_map > > >   triple_merge_map;
    typedef std::map< Symbol*, ltm_slot*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, Symbol* > > >                  ltm_slot_map;
    typedef std::map< std::string, ltm_object*, std::less< std::string >,
                          soar_module::soar_memory_pool_allocator< std::pair< std::string, ltm_object* > > >          str_to_ltm_map;
    typedef std::map< Symbol*, ltm_object*, std::less< Symbol* >,
                          soar_module::soar_memory_pool_allocator< std::pair< Symbol*, ltm_object* > > >              sym_to_ltm_map;
    typedef std::map< Symbol*, chunk_record*, std::less< Symbol* >,
        soar_module::soar_memory_pool_allocator< std::pair< Symbol*, chunk_record* > > >                              chunk_record_symbol_map;
    typedef std::map< uint64_t, chunk_record*, std::less< uint64_t >,
        soar_module::soar_memory_pool_allocator< std::pair< uint64_t, chunk_record* > > >                             chunk_record_id_map;
    typedef std::map< uint64_t, instantiation_record*, std::less< uint64_t >,
        soar_module::soar_memory_pool_allocator< std::pair< uint64_t, instantiation_record* > > >                     instantiation_record_map;
    typedef std::map< uint64_t, condition_record*, std::less< uint64_t >,
        soar_module::soar_memory_pool_allocator< std::pair< uint64_t, condition_record* > > >                         condition_record_map;
    typedef std::map< uint64_t, action_record*, std::less< uint64_t >,
        soar_module::soar_memory_pool_allocator< std::pair< uint64_t, action_record* > > >                            action_record_map;
    typedef std::map< uint64_t, production*, std::less< uint64_t >,
        soar_module::soar_memory_pool_allocator< std::pair< uint64_t, production* > > >                               production_map;

    /* A version with no pools just for performance testing */
    //    typedef std::set< augmentation* >                     augmentation_set;
    //    typedef std::set< uint64_t >                          id_set;
    //    typedef std::set< ltm_object* >                       ltm_set;
    //    typedef std::list< ltm_value* >                       ltm_slot;
    //    typedef std::list< smem_weighted_cue_element* >       smem_weighted_cue_list;
    //    typedef std::map< uint64_t, uint64_t >                id_to_id_map;
    //    typedef std::map< uint64_t, Symbol* >                 id_to_sym_map;
    //    typedef std::map< uint64_t, IdentitySet* >            id_to_join_map;
    //    typedef std::map< uint64_t, std::string >             id_to_string_map;
    //    typedef std::map< uint64_t, identity_mapping_list* >  inst_identities_map;
    //    typedef std::map< rhs_value, std::string >            rhs_val_to_string_map;
    //    typedef std::map< Symbol*, augmentation_set* >        sym_to_aug_map;
    //    typedef std::map< Symbol*, condition* >               sym_to_cond_map;
    //    typedef std::map< Symbol*, uint64_t >                 sym_to_id_map;
    //    typedef std::map< Symbol*, chunk_element* >           sym_to_sym_id_map;
    //    typedef std::map< Symbol*, sym_to_cond_map >          sym_to_sym_to_cond_map;
    //    typedef std::map< Symbol*, sym_to_sym_to_cond_map >   triple_merge_map;
    //    typedef std::map< Symbol*, ltm_slot* >                ltm_slot_map;
    //    typedef std::map< std::string, ltm_object* >          str_to_ltm_map;
    //    typedef std::map< Symbol*, ltm_object* >              sym_to_ltm_map;

    #endif

typedef std::pair< double, uint64_t >                   smem_activated_lti;

#endif /* STL_TYPEDEFS_H_ */
