#ifndef CORE_SOARKERNEL_SRC_SHARED_STL_STRUCTS_H_
#define CORE_SOARKERNEL_SRC_SHARED_STL_STRUCTS_H_

#include "kernel.h"
#include "mempool_allocator.h"

#include <unordered_set>

#ifdef USE_MEM_POOL_ALLOCATORS
    typedef std::list< constraint*, soar_module::soar_memory_pool_allocator< constraint* > >            constraint_list;
    typedef std::list< identity_set*, soar_module::soar_memory_pool_allocator< identity_set* > >        identity_set_list;
#else
    typedef std::list< identity_set* >                                                                  identity_set_list;
    typedef std::list< constraint* >                                                                    constraint_list;
#endif

typedef struct symbol_triple_struct
{
        Symbol* id;
        Symbol* attr;
        Symbol* value;

        symbol_triple_struct(Symbol* new_id = NULL, Symbol* new_attr = NULL, Symbol* new_value = NULL): id(new_id), attr(new_attr), value(new_value) {}
} symbol_triple;

typedef struct test_triple_struct
{
        test id;
        test attr;
        test value;

        test_triple_struct(test new_id = NULL, test new_attr = NULL, test new_value = NULL): id(new_id), attr(new_attr), value(new_value) {}
} test_triple;

typedef struct identity_quadruple_struct
{
        uint64_t id;
        uint64_t attr;
        uint64_t value;
        uint64_t referent;

        identity_quadruple_struct(uint64_t new_id = 0, uint64_t new_attr = 0, uint64_t new_value = 0, uint64_t new_referent = 0): id(new_id), attr(new_attr), value(new_value), referent(new_referent) {}
} identity_quadruple;

typedef struct identity_set_struct
{
        /* Identity ID is static and never changes after instantiation creation.
         * The remaining fields are transient based on the explanation trace being analyzed
         * and are cleaned up after a learning episode. */

        /* An ID used for printing and to make debugging easier. Not used by identity analysis logic. */
        uint64_t                    identity;

        /* Flag to avoid adding to identity set clean up list if it is already on it */
        bool                        dirty;

        /* Fields that link this identity_set node to others in the identity graph */
        identity_set_struct*        super_join;
        identity_set_list*          identity_sets;

        /* Fields for variablization and chunk instantiation identity creation */
        Symbol*                     new_var;
        uint64_t                    clone_identity;
        bool                        literalized;
        bool                        has_joined_constraints;
        cons*                       constraints;
        cons*                       inverted_constraints;  // These need to be deleted, so they have their own list

        /* Fields for transitive constraint attachment  */
        condition*                  operational_cond;
        WME_Field                   operational_field;

        /* This is an ID used by DEBUG_IDSET_INVENTORY.  Not used otherwise. */
        uint64_t                    is_id;
} identity_set;


typedef struct identity_set_quadruple_struct
{
        identity_set* id;
        identity_set* attr;
        identity_set* value;
        identity_set* referent;

        identity_set_quadruple_struct(identity_set* new_id = NULL, identity_set* new_attr = NULL, identity_set* new_value = NULL, identity_set* new_referent = NULL): id(new_id), attr(new_attr), value(new_value), referent(new_referent) {}
} identity_set_quadruple;

typedef struct bool_quadruple_struct
{
        bool id;
        bool attr;
        bool value;
        bool referent;

        bool_quadruple_struct(uint64_t new_id = false, uint64_t new_attr = false, uint64_t new_value = false, uint64_t new_referent = false): id(new_id), attr(new_attr), value(new_value), referent(new_referent) {}
} bool_quadruple;

typedef struct rhs_quadruple_struct
{
        rhs_value id;
        rhs_value attr;
        rhs_value value;
        rhs_value referent;

        rhs_quadruple_struct(rhs_value new_id = NULL, rhs_value new_attr = NULL, rhs_value new_value = NULL, rhs_value new_referent = NULL): id(new_id), attr(new_attr), value(new_value), referent(new_referent) {}
} rhs_quadruple;

typedef struct deep_copy_struct
{
        Symbol* id;
        Symbol* attr;
        Symbol* value;
        wme*    deep_copied_wme;
        deep_copy_struct(Symbol* new_id = NULL, Symbol* new_attr = NULL, Symbol* new_value = NULL, wme* new_wme = NULL): id(new_id), attr(new_attr), value(new_value), deep_copied_wme(new_wme) {}
} deep_copy_wme;

typedef struct identity_mapping_struct {
        uint64_t            from_identity;
        uint64_t            to_identity;
        IDSet_Mapping_Type  mappingType;
} identity_mapping;

typedef struct chunk_element_struct {
        Symbol*     variable_sym;
        Symbol*     instantiated_sym;
        uint64_t    identity;
        chunk_element_struct() {
            variable_sym = NULL;
            instantiated_sym = NULL;
            identity = 0;
        }
} chunk_element;

typedef struct aug_struct
{
        Symbol* attr;
        Symbol* value;
} augmentation;

typedef struct constraint_struct
{
    test eq_test;
    test constraint_test;
    constraint_struct(test new_eq, test new_constraint) : eq_test(new_eq), constraint_test(new_constraint) {}
} constraint;

typedef struct attachment_struct
{
        condition* cond;
        WME_Field field;
        attachment_struct(condition* new_cond, WME_Field new_field) : cond(new_cond), field(new_field) {}

} attachment_point;

#endif /* CORE_SOARKERNEL_SRC_SHARED_STL_STRUCTS_H_ */
