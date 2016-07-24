#ifndef CORE_SOARKERNEL_SRC_SHARED_TRIPLES_H_
#define CORE_SOARKERNEL_SRC_SHARED_TRIPLES_H_

#include "kernel.h"

typedef struct symbol_triple_struct
{
        Symbol id;
        Symbol attr;
        Symbol value;

        symbol_triple_struct(Symbol new_id = NULL, Symbol new_attr = NULL, Symbol new_value = NULL): id(new_id), attr(new_attr), value(new_value) {}
} symbol_triple;

typedef struct test_triple_struct
{
        test id;
        test attr;
        test value;

        test_triple_struct(test new_id = NULL, test new_attr = NULL, test new_value = NULL): id(new_id), attr(new_attr), value(new_value) {}
} test_triple;

typedef struct identity_triple_struct
{
        uint64_t id;
        uint64_t attr;
        uint64_t value;
        uint64_t referent;

        identity_triple_struct(uint64_t new_id = 0, uint64_t new_attr = 0, uint64_t new_value = 0, uint64_t new_referent = 0): id(new_id), attr(new_attr), value(new_value), referent(new_referent) {}
} identity_triple;

typedef struct rhs_triple_struct
{
        rhs_value id;
        rhs_value attr;
        rhs_value value;

        rhs_triple_struct(rhs_value new_id = NULL, rhs_value new_attr = NULL, rhs_value new_value = NULL): id(new_id), attr(new_attr), value(new_value) {}
} rhs_triple;

#endif /* CORE_SOARKERNEL_SRC_SHARED_TRIPLES_H_ */
