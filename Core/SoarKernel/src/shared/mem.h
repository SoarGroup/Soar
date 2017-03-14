/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/


#ifndef MEM_H
#define MEM_H

#include "kernel.h"

#include <stdio.h>  // Needed for FILE token below
#include <string.h>     // Needed for strlen, etc. below

#ifndef _WIN32
#include <strings.h>
#include <stdlib.h> // malloc
#endif // !_WIN32

extern void init_memory_utilities(agent* thisAgent);


/* ---------------- */
/* string utilities */
/* ---------------- */

extern char* make_memory_block_for_string(agent* thisAgent, char const* s);
extern void free_memory_block_for_string(agent* thisAgent, char* p);


typedef void* growable_string;

// voigtjr 11/2005: platform specific code (strlen/malloc/etc) should be in .cpp files!
// except it can't be (?) because of the inline restriction
inline char* savestring(char* x)
{
    return strcpy(static_cast<char*>(malloc(strlen(x) + 1)), (x));
}

inline int& memsize_of_growable_string(growable_string gs)
{
    return (*((int*)(gs)));
}

inline int& length_of_growable_string(growable_string gs)
{
    return (*(((int*)(gs)) + 1));
}

inline char* text_of_growable_string(growable_string gs)
{
    return (((char*)(gs)) + 2 * sizeof(int*));
}

extern growable_string make_blank_growable_string(agent* thisAgent);
extern void add_to_growable_string(agent* thisAgent, growable_string* gs, const char* string_to_add);
extern void free_growable_string(agent* thisAgent, growable_string gs);





/* ------------------------- */
/* Cons cell, list utilities */
/* ------------------------- */

typedef struct cons_struct
{
    void* first;
    struct cons_struct* rest;
} cons;

typedef struct dl_cons_struct
{
    void* item;
    struct dl_cons_struct* next;
    struct dl_cons_struct* prev;
} dl_cons;

typedef dl_cons dl_list;

extern cons* destructively_reverse_list(cons* c);
extern bool member_of_list(void* item, cons* the_list);
extern cons* add_if_not_member(agent* thisAgent, void* item, cons* old_list);
extern void free_list(agent* thisAgent, cons* the_list);

/* Added a void* parameter to cons_test_fn, because remove_pwatch_test_fn(),
   one of the callback functions, requires a third parameter that points to a
   production. In the future, other callback functions of type cons_test_fn may
   need parameters of different types, so a void pointer is best. -AJC (8/7/02) */
/* Added thisAgent to cons_test_fn type, because we are eliminating the
   global soar_agent. -AJC (8/7/02) */
//typedef bool (*cons_test_fn)(cons *c);
typedef bool (*cons_test_fn)(agent* thisAgent, cons* c, void* data);

typedef bool (*dl_cons_test_fn)(dl_cons* dc, agent* thisAgent);

/* Added a void* parameter to extract_list_elements, because remove_pwatch_test_fn(),
   one of the callback functions, requires a third parameter that points to a
   production. In the future, other callback functions of type cons_test_fn may
   need parameters of different types, so a void pointer is best. -AJC (8/7/02) */
extern cons* extract_list_elements(agent* thisAgent, cons** header, cons_test_fn f, void* data = 0);

extern dl_list* extract_dl_list_elements(agent* thisAgent, dl_list** header, dl_cons_test_fn f);

extern bool cons_equality_fn(agent*, cons* c, void* data);

/* ----------------------------- */
/* Resizable hash table routines */
/* ----------------------------- */

extern uint32_t masks_for_n_low_order_bits[33];

typedef uint32_t ((*hash_function)(void* item, short num_bits));

typedef struct item_in_hash_table_struct
{
    struct item_in_hash_table_struct* next;
    char data;
} item_in_hash_table;

typedef item_in_hash_table* bucket_array;

typedef struct hash_table_struct
{
    int64_t count;            /* number of items in the table */
    uint32_t size;            /* number of buckets */
    short log2size;           /* log (base 2) of size */
    short minimum_log2size;   /* table never shrinks below this size */
    bucket_array* buckets;
    hash_function h;          /* call this to hash or rehash an item */
} hash_table;

extern struct hash_table_struct* make_hash_table(agent* thisAgent, short minimum_log2size,
        hash_function h);
extern void free_hash_table(agent* thisAgent, struct hash_table_struct* ht); /* RPM 6/09 */
extern void remove_from_hash_table(agent* thisAgent, struct hash_table_struct* ht, void* item);
extern void add_to_hash_table(agent* thisAgent, struct hash_table_struct* ht, void* item);

typedef bool (*hash_table_callback_fn)(void* item);
typedef bool (*hash_table_callback_fn2)(agent* thisAgent, void* item, void* f);

extern void do_for_all_items_in_hash_table(agent* thisAgent, struct hash_table_struct* ht,
        hash_table_callback_fn2 f, void* userdata);
extern void do_for_all_items_in_hash_bucket(struct hash_table_struct* ht,
        hash_table_callback_fn f,
        uint32_t hash_value);

#endif

/* ======================================================================
                                 mem.h

   Init_memory_utilities() should be called before any of these routines
   are used.

   Basic memory allocation utilities:

     All memory blocks are allocated via calls to allocate_memory().  It
     calls malloc() and aborts if we run out of memory.  Free_memory() is
     the inverse of allocate_memory().  Allocate_memory_and_zerofill()
     does the obvious thing.  These routines take a usage_code indicating
     what purpose the memory is for (hash tables, strings, etc.).  This
     is used purely for statistics keeping.

     Print_memory_statistics() prints out stats on the memory usage.

   String utilities:

     Make_memory_block_for_string() takes a pointer to a string, allocates
     a memory block just large enough to hold the string, and copies the
     string into the block.   Free_memory_block_for_string() frees the
     block.

     "Growable strings" provide a convenient way of building up a string
     piece by piece without having to pre-allocate the right amount of
     memory.  To initialize one, say "gs = make_blank_growable_string()"
     where "gs" is of type "growable_string".  To concatenate a new string
     onto the end of an existing growable string, call
     add_to_growable_string(&gs,new_string) [note the "&"].  When you're
     done using it, call free_growable_string(gs).  Growable strings are
     implemented by allocating a block of memory large enough to hold
     (1) the memory block's size, (2) the current string length, and (3)
     the current text of the string.  Add_to_growable_string() may result
     in a new (larger) memory block being allocated and the text of the
     string being copied.  Three macros provide access to a growable string's
     parts:  memsize_of_growable_string(), length_of_growable_string(),
     and (most importantly) text_of_growable_string(), which is of type
     (char *).

   Memory pools:

     To allocate and free memory items efficiently at run time, we use
     pools of small fixed-size items and do allocation and freeing using
     inline macros.  Different memory pools are used for different things
     and contain different size items.  Each pool consists of a memory_pool
     structure (used for maintaining the pool) and a chain of big blocks
     of memory (currently about 32K per block) obtained from allocate_memory().
     We maintain a free_list of small items not being used, and allocate by
     grabbing the first item on the free list.  If the free list is empty,
     we add another big block to the pool.

     Init_memory_pool() should be called to initialize a memory_pool
     structure before it is used.  After that, the macro forms
     allocate_with_pool (&mem_pool, &pointer_to_be_set_to_new_item) and
     free_with_pool (&mem_pool, pointer_to_item)
     are used to allocate and free items.  Print_memory_pool_statistics()
     prints stats about the various pools in use and how much memory each
     is using.

   Cons cell and list utilities:

     This provides a simple facility for manipulating generic lists, just
     like in Lisp.  A "cons" is a pair of pointers; a "list" is just a cons
     (or NIL).  We maintain a memory pool (see above) of conses.
     Allocate_cons() is like allocate_with_pool() for conses; free_cons()
     is like free_with_pool.  Push(new_item,my_list) is a macro for adding
     a new item onto the front of an existing list.

     In addition to the regular conses, we also have a pool of "dl_cons"es--
     these are like conses, only doubly-linked.  A "dl_list" is a just a
     dl_cons (or NIL).

     Some (consed) list utility functions:  destructively_reverse_list()
     does just what it says, and returns a pointer to the new head of the
     list (formerly the tail).  Member_of_list() tests membership, using
     "==" as the equality predicates.  Add_if_not_member() is like Lisp's
     "pushnew"; it returns the new list (possibly unchanged) list.
     Free_list() frees all the cons cells in the list.

     Sometimes we need to surgically extract particular elements from a
     list.  Extract_list_elements() and extract_dl_list_elements() do this.
     They use a callback function that indicates which elements to extract:
     the callback function is called on each element of the list, and should
     return true for the elements to be extracted.  The two extraction
     functions return a list (or dl_list) of the extracted elements.

   Hash table routines:

     We use hash tables in various places, and don't want to have to fix
     their size ahead of time.  These routines provide hash tables that
     are dynamically resized as items are added and removed.  We use
     "open hashing" with a hash table whose size is a power of two.  We
     keep track of how many items are in the table.  The table grows
     when # of items >= 2*size, and shrinks when # of items < size/2.
     To resize a hash table, we rehash every item in it.

     Each item must be a structure whose first field is reserved for use
     by these hash table routines--it points to the next item in the hash
     bucket.  Hash tables are created and initialized via make_hash_table();
     you give it a hash function (i.e., a C function) that finds the hash
     value for a given item, hashing it to a value num_bits wide.  For aid
     in this, we provide masks_for_n_low_order_bits[] that select out the
     low-order bits of a value:  (x & masks_for_n_low_order_bits[23]) picks
     out the 23 low-order bits of x.

     Items are added/removed from a hash table via add_to_hash_table() and
     remove_from_hash_table().  These calls resize the hash table if
     necessary.

     The contents of a hash table (or one bucket in the table) can be
     retrieved via do_for_all_items_in_hash_table() and
     do_for_all_items_in_hash_bucket().  Each uses a callback function,
     invoking it with each successive item.  The callback function should
     normally return false.  If the callback function ever returns true,
     iteration over the hash table items stops and the do_for_xxx()
     routine returns immediately.
====================================================================== */

