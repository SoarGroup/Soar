#ifndef STL_TYPEDEFS_H_
#define STL_TYPEDEFS_H_

#include "kernel.h"
#include "triples.h"
#include "mempool_allocator.h"

#include <map>
#include <string>
#include <set>
#include <unordered_set>
#include <list>
#include <functional>
#include <assert.h>
#include <cmath>


class sym_grounding_path;
class condition_record;
class action_record;
class instantiation_record;
typedef struct constraint_struct constraint;
typedef struct attachment_struct attachment_point;
typedef struct identity_set_struct identity_set_info;
typedef struct aug_struct augmentation;
typedef struct ungrounded_sym_struct ungrounded_sym;

#ifdef USE_MEM_POOL_ALLOCATORS

    typedef std::list< condition*, soar_module::soar_memory_pool_allocator< condition* > >                          condition_list;
    typedef std::list< Symbol*, soar_module::soar_memory_pool_allocator< Symbol* > >                                symbol_list;
    typedef std::list< ungrounded_sym*, soar_module::soar_memory_pool_allocator< ungrounded_sym* > >                ungrounded_symbol_list;
    typedef std::list< instantiation_record*, soar_module::soar_memory_pool_allocator< instantiation_record* > >    inst_record_list;
    typedef std::list< inst_record_list*, soar_module::soar_memory_pool_allocator< inst_record_list* > >            inst_path_list;
    typedef std::list< condition_record*, soar_module::soar_memory_pool_allocator< condition_record* > >            condition_record_list;
    typedef std::list< action_record*, soar_module::soar_memory_pool_allocator< action_record* > >                  action_record_list;
    typedef std::list< uint64_t, soar_module::soar_memory_pool_allocator< uint64_t > >                              id_list;
    typedef std::list< sym_grounding_path*, soar_module::soar_memory_pool_allocator<sym_grounding_path*> >          sym_grounding_path_list;
    typedef std::list< production*, soar_module::soar_memory_pool_allocator< production* > >                        rl_rule_list;
    typedef std::list< constraint*, soar_module::soar_memory_pool_allocator< constraint* > >                        constraint_list;
    typedef std::list< symbol_triple*, soar_module::soar_memory_pool_allocator< symbol_triple* > >                  symbol_triple_list;
    typedef std::list< test_triple*, soar_module::soar_memory_pool_allocator< test_triple* > >                      test_triple_list;
    typedef std::list< identity_triple*, soar_module::soar_memory_pool_allocator< identity_triple* > >              identity_triple_list;
    typedef std::list< wme*, soar_module::soar_memory_pool_allocator< wme* > >                                      wme_list;

    typedef std::set< Symbol*, std::less< Symbol* >, soar_module::soar_memory_pool_allocator< Symbol* > >           symbol_set;
    typedef std::set< wme*, std::less< wme* >, soar_module::soar_memory_pool_allocator< wme* > >                    wme_set;
    typedef std::set< instantiation*, std::less< instantiation* >,
                      soar_module::soar_memory_pool_allocator< instantiation* > >                                   inst_set;
    typedef std::set< instantiation_record*, std::less< instantiation_record* >,
                      soar_module::soar_memory_pool_allocator< instantiation_record* > >                            inst_record_set;
    typedef std::set< wma_decay_element*, std::less< wma_decay_element* >,
                      soar_module::soar_memory_pool_allocator< wma_decay_element* > >                               wma_decay_set;
    typedef std::set< wma_d_cycle, std::less< wma_d_cycle >,
                      soar_module::soar_memory_pool_allocator< wma_d_cycle > >                                      wma_decay_cycle_set;
    typedef std::set< wme*, std::less< wme* >, soar_module::soar_memory_pool_allocator< wme* > >                    wma_pooled_wme_set;

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

//    typedef std::unordered_set< uint64_t, std::hash< uint64_t >,  std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< uint64_t > >                                        id_set;
//    typedef std::unordered_map< uint64_t, uint64_t, std::hash< uint64_t >, std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair< uint64_t const, uint64_t> > >             id_to_id_map_type;
//    typedef std::unordered_map< const Symbol*, uint64_t, std::hash< const Symbol* >, std::equal_to< const Symbol* >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair<const Symbol* const, uint64_t> > >        sym_to_id_map_type;
//    typedef std::unordered_map< uint64_t, Symbol*, std::hash< uint64_t >,  std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair<uint64_t const, Symbol* > > >             id_to_sym_map_type;
//    typedef std::unordered_map< uint64_t, identity_set_info*, std::hash< uint64_t >, std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair<uint64_t const, identity_set_info* > > >  id_to_idset_map_type;
//    typedef std::unordered_map< uint64_t, attachment_point*, std::hash< uint64_t >, std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair<uint64_t const, attachment_point* > > >   attachment_points_map_type;
//    typedef std::unordered_map< uint64_t, sym_to_id_map_type, std::hash< uint64_t >, std::equal_to< uint64_t >,
//                       soar_module::soar_memory_pool_allocator_n< std::pair<const uint64_t, sym_to_id_map_type > > >  inst_to_id_map_type;
//
//    typedef std::unordered_map< const Symbol*, condition*,
//            std::hash< const Symbol* >, std::equal_to< const Symbol* >,
//            soar_module::soar_memory_pool_allocator_n< std::pair<const Symbol* const, condition* > > >                sym_to_cond_map;
//    typedef std::unordered_map< const Symbol*, sym_to_cond_map,
//            std::hash< const Symbol* >, std::equal_to< const Symbol* >,
//            soar_module::soar_memory_pool_allocator_n< std::pair<const Symbol* const, sym_to_cond_map > > >           sym_to_sym_to_cond_map;
//    typedef std::unordered_map< const Symbol*, sym_to_sym_to_cond_map,
//            std::hash< const Symbol* >, std::equal_to< const Symbol* >,
//            soar_module::soar_memory_pool_allocator_n< std::pair<const Symbol* const, sym_to_sym_to_cond_map > > >    triple_merge_map;

#else
    typedef std::list< condition* >                             condition_list;
    typedef std::list< Symbol* >                                symbol_list;
    typedef std::list< ungrounded_sym* >                        ungrounded_symbol_list;
    typedef std::list< sym_grounding_path* >                    sym_grounding_path_list;
    typedef std::list< production* >                            rl_rule_list;
    typedef std::list< instantiation_record* >                  inst_record_list;
    typedef std::list< inst_record_list* >                      inst_path_list;
    typedef std::list< condition_record* >                      condition_record_list;
    typedef std::list< action_record* >                         action_record_list;
    typedef std::list< uint64_t >                               id_list;
    typedef std::list< constraint* >                            constraint_list;
    typedef std::list< symbol_triple* >                         symbol_triple_list;
    typedef std::list< test_triple* >                           test_triple_list;
    typedef std::list< identity_triple* >                       identity_triple_list;
    typedef std::list< wme* >                                   wme_list;

    typedef std::set< Symbol* >                                 symbol_set;
    typedef std::set< wme* >                                    wme_set;
    typedef std::set< instantiation* >                          inst_set;
    typedef std::set< instantiation_record* >                   inst_record_set;
    typedef std::set< wma_decay_element* >                      wma_decay_set;
    typedef std::set< wma_d_cycle >                             wma_decay_cycle_set;
    typedef std::set< wme* >                                    wma_pooled_wme_set;

    typedef std::map< production*, double >                     rl_et_map;
    typedef std::map< Symbol*, Symbol* >                        rl_symbol_map;
    typedef std::map< wma_d_cycle, wma_decay_set* >             wma_forget_p_queue;
    typedef std::map< Symbol*, uint64_t >                       wma_sym_reference_map;

    typedef std::set< rl_symbol_map >                           rl_symbol_map_set;

#endif

/* We need a more flexible custom memory pool allocator for unordered STL data structures.  They
 * can request multiple objects at once, which prevents our normal custom allocator to be used. It
 * looks like there may be some subtle issues that our allocator might have problems with, so we
 * may want to use some of the more full-featured memory pool custom allocators that are publicly
 * available.
 *
 * It's also not clear whether allocations that requests variable memory size would benefit from
 * using memory pools.  It looks like it requests only one object the majority of the time, so
 * it may still be of some benefit.
 *
 * For now, we just use standard heap allocation for unordered STL structures */

typedef std::unordered_set< uint64_t >                      id_set;

typedef std::unordered_map< uint64_t, uint64_t >            id_to_id_map_type;
typedef std::unordered_map< uint64_t, Symbol* >             id_to_sym_map_type;
typedef std::unordered_map< uint64_t, identity_set_info* >  id_to_idset_map_type;
typedef std::unordered_map< uint64_t, attachment_point* >   attachment_points_map_type;
typedef std::unordered_map< Symbol*, uint64_t >             sym_to_id_map_type;
typedef std::unordered_map< uint64_t, sym_to_id_map_type >  inst_to_id_map_type;

typedef std::unordered_map< Symbol*, condition* >               sym_to_cond_map;
typedef std::unordered_map< Symbol*, sym_to_cond_map >          sym_to_sym_to_cond_map;
typedef std::unordered_map< Symbol*, sym_to_sym_to_cond_map >   triple_merge_map;

typedef std::unordered_set< augmentation* >                     augmentation_set;
typedef std::unordered_map< Symbol*, augmentation_set* >        sym_to_aug_map;

#endif /* STL_TYPEDEFS_H_ */
