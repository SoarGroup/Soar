/*
 * ebc_structs.h
 *
 *  Created on: Sep 12, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_STRUCTS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_STRUCTS_H_

#define BUFFER_PROD_NAME_SIZE 256
#define CHUNK_COND_HASH_TABLE_SIZE 1024
#define LOG_2_CHUNK_COND_HASH_TABLE_SIZE 10

typedef struct chunk_cond_struct
{
    condition* cond;                /* points to the original condition */

    condition* instantiated_cond;   /* points to cond in chunk instantiation */
    condition* variablized_cond;    /* points to cond in the actual chunk */

    /* dll of all cond's in a set (i.e., a chunk_cond_set, or the grounds) */
    struct chunk_cond_struct* next, *prev;

    /* dll of cond's in this particular hash bucket for this set */
    struct chunk_cond_struct* next_in_bucket, *prev_in_bucket;

    uint32_t hash_value;             /* equals hash_condition(cond) */
    uint32_t compressed_hash_value;  /* above, compressed to a few bits */
} chunk_cond;

typedef struct chunk_cond_set_struct
{
    chunk_cond* all;       /* header for dll of all chunk_cond's in the set */
    chunk_cond* table[CHUNK_COND_HASH_TABLE_SIZE];  /* hash table buckets */
} chunk_cond_set;

typedef struct backtrace_struct
{
    int result;                    /* 1 when this is a result of the chunk */
    condition* trace_cond;         /* The (local) condition being traced */
    char       prod_name[BUFFER_PROD_NAME_SIZE];         /* The production's name */
    condition* grounds;            /* The list of conds for the LHS of chunk */
    condition* potentials;         /* The list of conds which aren't linked */
    condition* locals;             /* Conds in the subgoal -- need to BT */
    condition* negated;            /* Negated conditions (sub/super) */
    struct backtrace_struct* next_backtrace; /* Pointer to next in this list */
} backtrace_str;

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_STRUCTS_H_ */
