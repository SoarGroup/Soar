/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

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
     return TRUE for the elements to be extracted.  The two extraction
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
     normally return FALSE.  If the callback function ever returns TRUE,
     iteration over the hash table items stops and the do_for_xxx()
     routine returns immediately.  
====================================================================== */

#ifndef MEM_H
#define MEM_H

#include "chunk.h"
#include "kernel.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void init_memory_utilities (agent* thisAgent);

/* ----------------------- */
/* basic memory allocation */
/* ----------------------- */

#ifdef DEBUG_MEMORY

#ifdef USE_MACROS
#define fill_with_garbage(block,size) memset((void *)(block), 0xBB, (size))
#else
template <typename T>
inline void fill_with_garbage(T * block, size_t size)
{
  memset(static_cast<void *>(block), 0xBB, (size));
}
#endif /* USE_MACROS */

#else

#define fill_with_garbage(block,size) { }

#endif

#define MISCELLANEOUS_MEM_USAGE  0
#define HASH_TABLE_MEM_USAGE     1
#define STRING_MEM_USAGE         2
#define POOL_MEM_USAGE           3
#define STATS_OVERHEAD_MEM_USAGE 4

#define NUM_MEM_USAGE_CODES 5

extern void *allocate_memory (agent* thisAgent, unsigned long size, int usage_code);
extern void *allocate_memory_and_zerofill (agent* thisAgent, unsigned long size, int usage_code);
extern void free_memory (agent* thisAgent, void *mem, int usage_code);
extern void print_memory_statistics (agent* thisAgent);

/* ---------------- */
/* string utilities */
/* ---------------- */

extern char *make_memory_block_for_string (agent* thisAgent, char *s);
extern void free_memory_block_for_string (agent* thisAgent, char *p);

typedef char Bool;
typedef void * growable_string;

#ifdef USE_MACROS

#define savestring(x) (char *) strcpy ((char *)(malloc (strlen (x) + 1)), (x))
#define memsize_of_growable_string(gs) (*((int *)(gs)))
#define length_of_growable_string(gs) (*(((int *)(gs))+1))
#define text_of_growable_string(gs) (((char *)(gs)) + 2*sizeof(int *))

#else

// voigtjr 11/2005: platform specific code (strlen/malloc/etc) should be in .cpp files!
inline char * savestring(char * x)
{
  return strcpy(static_cast<char *>(malloc (strlen (x) + 1)), (x));
}

inline int & memsize_of_growable_string(growable_string gs)
{
  return (*((int *)(gs)));
}

inline int & length_of_growable_string(growable_string gs)
{
  return (*(((int *)(gs))+1));
}

inline char * text_of_growable_string(growable_string gs)
{
  return (((char *)(gs)) + 2*sizeof(int *));
}

#endif /* USE_MACROS */

extern growable_string make_blank_growable_string (agent* thisAgent);
extern void add_to_growable_string (agent* thisAgent, growable_string *gs, char *string_to_add);
extern void free_growable_string (agent* thisAgent, growable_string gs);

/* ------------ */
/* memory pools */
/* ------------ */

#define MAX_POOL_NAME_LENGTH 15

typedef struct memory_pool_struct {
  void *free_list;             /* header of chain of free items */
  /// #ifdef MEMORY_POOL_STATS /* only one long per pool, so always include */
  long used_count;             /* used for statistics only when #def'd MEMORY_POOL_STATS */
  /// #endif
  long item_size;               /* bytes per item */
  long items_per_block;        /* number of items in each big block */
  long num_blocks;             /* number of big blocks in use by this pool */
  void *first_block;           /* header of chain of blocks */
  char name[MAX_POOL_NAME_LENGTH];  /* name of the pool (for memory-stats) */
  struct memory_pool_struct *next;  /* next in list of all memory pools */
} memory_pool;

extern void add_block_to_memory_pool (agent* thisAgent, memory_pool *p);
extern void init_memory_pool (agent* thisAgent, memory_pool *p, long item_size, char *name);
extern void print_memory_pool_statistics (agent* thisAgent);
extern void free_memory_pool (memory_pool *p);

#ifdef MEMORY_POOL_STATS

#ifdef USE_MACROS

#define increment_used_count(p) {(p)->used_count++;}
#define decrement_used_count(p) {(p)->used_count--;}

#else

#ifdef __cplusplus
}
#endif

template <typename P>
inline void increment_used_count(P p)
{
  (p)->used_count++;
}

template <typename P>
inline void decrement_used_count(P p)
{
  (p)->used_count--;
}

#ifdef __cplusplus
extern "C"
{
#endif

#endif /* USE_MACROS */

#else

#define increment_used_count(p) { }
#define decrement_used_count(p) { }

#endif /* MEMORY_POOL_STATS */

#ifndef __cplusplus

/* old C macro, replaced with template function in port to C++ due to casting problems */
#define allocate_with_pool(thisAgent, p,dest_item_pointer) { \
  if (! (p)->free_list) add_block_to_memory_pool(thisAgent, p); \
  *(dest_item_pointer) = (p)->free_list; \
  (p)->free_list = *(void * *)(*(dest_item_pointer)); \
  fill_with_garbage (*(dest_item_pointer), (p)->item_size); \
  increment_used_count(p); }

#define free_with_pool(p,item) { \
  fill_with_garbage ((item), (p)->item_size); \
  *(void * *)(item) = (p)->free_list; \
  (p)->free_list = (void *)(item); \
  decrement_used_count(p); }

#else

}

//
// This function was overtemplated.  The P type is unnecessary -- it's always memory_pool*
//
template <typename P, typename T>
inline void allocate_with_pool(agent* thisAgent, P p, T** dest_item_pointer)
{
  
  // if there's no memory blocks left in the pool, then allocate a new one
  if (! (p)->free_list) add_block_to_memory_pool(thisAgent, p);
  // take the beginning of the next free block and give it to the T pointer
  *(dest_item_pointer) = static_cast< T* > ((p)->free_list);
  // we think this line increments free_list to the next available memory block
  // we thought it took advantage of the fact that free_list is the first
  //  member of memory_pool, but I tried changing that and it still works, so now I'm at a loss
  // if it helps, we think this line is equivalent to the following
  //  (at least, everything appears to work properly if you swap these lines):
  // (p)->free_list = (*reinterpret_cast<P*>(dest_item_pointer))->free_list;
  (p)->free_list =  *(void * *)(*(dest_item_pointer));

  fill_with_garbage (*(dest_item_pointer), (p)->item_size);
  increment_used_count(p);
 
   // this is for debugging -- it disables the memory pool usage and just allocates
   //  new memory every time.  If you want to use it, be sure to make the corresponding
   //  change to free_with_pool below
   //*dest_item_pointer = static_cast< T * > (malloc(sizeof(T)));
}

template <typename P, typename T>
inline void free_with_pool(P p, T * item)
{

  fill_with_garbage ((item), (p)->item_size);
  *(void * *)(item) = (p)->free_list;
  (p)->free_list = (void *)(item);
  decrement_used_count(p); 
 
   // this is for debugging -- it disables the memory pool usage and just deallocates
   //  the memory every time.  If you want to use it, be sure to make the corresponding
   //  change to allocate_with_pool above
   //free(item);
}

extern "C"
{

#endif

/* ---------------------------------------------------------------------
     Macros for Inserting and Removing Stuff from Doubly-Linked Lists 

   Note: fast_remove_from_dll() is the same as remove_from_dll() except
   slightly faster.  I (RBD) only realized this after writing all the 
   original code.  With fast_remove_from_dll(), you have to tell it
   the type (i.e., structure name) of the item being spliced out of
   the list.  At some point we might want to go through all the code
   and have it use fast_remove_from_dll(), but it's probably not worth
   the effort right now.
-------------------------------------------------------------------- */

/* This macro cannot be easily converted to an inline function. 
   Some additional changes are required.
*/
#define insert_at_head_of_dll(header,item,next_field_name,prev_field_name) { \
  ((item)->next_field_name) = (header) ; \
  ((item)->prev_field_name) = NIL ; \
  if (header) ((header)->prev_field_name) = (item) ; \
  (header) = (item) ; }
/*template <typename T>
inline void insert_at_head_of_dll(T header, T item, T next_field_name, 
                                  T prev_field_name) 
{
  ((item)->next_field_name) = (header);
  ((item)->prev_field_name) = NIL;
  if (header) ((header)->prev_field_name) = (item);
  (header) = (item);
}*/

/* This macro cannot be easily converted to an inline function. 
   Some additional changes are required.
*/
#define remove_from_dll(header,item,next_field_name,prev_field_name) { \
  if ((item)->next_field_name) \
    ((item)->next_field_name->prev_field_name) = ((item)->prev_field_name); \
  if ((item)->prev_field_name) { \
    ((item)->prev_field_name->next_field_name) = ((item)->next_field_name); \
  } else { \
    (header) = ((item)->next_field_name); \
  } }
/*template <typename T>
inline void remove_from_dll(T header, T item, T next_field_name, 
                            T prev_field_name)
{  
  if ((item)->next_field_name)
    ((item)->next_field_name->prev_field_name) = ((item)->prev_field_name);
  if ((item)->prev_field_name) {
    ((item)->prev_field_name->next_field_name) = ((item)->next_field_name);
  } else {
    (header) = ((item)->next_field_name);
  }
}*/

/* This macro cannot be easily converted to an inline function. 
   Some additional changes are required.
*/
#define fast_remove_from_dll(header,item,typename,next_field_name,prev_field_name) { \
  typename *tempnext, *tempprev; \
  tempnext = (item)->next_field_name; \
  tempprev = (item)->prev_field_name; \
  if (tempnext) tempnext->prev_field_name = tempprev; \
  if (tempprev) { \
    tempprev->next_field_name = tempnext; \
  } else { \
    (header) = tempnext; } }

/* ------------------------- */
/* Cons cell, list utilities */
/* ------------------------- */

typedef struct cons_struct {
  void *first;
  struct cons_struct *rest;
} cons;

typedef cons list;

typedef struct dl_cons_struct {
  void *item;
  struct dl_cons_struct *next;
  struct dl_cons_struct *prev;
} dl_cons;

typedef dl_cons dl_list;

extern list *destructively_reverse_list (list *c);
extern Bool member_of_list (void *item, list *the_list);
extern list *add_if_not_member (agent* thisAgent, void *item, list *old_list);
extern void free_list (agent* thisAgent, list *the_list);

/* Added a void* parameter to cons_test_fn, because remove_pwatch_test_fn(), 
   one of the callback functions, requires a third parameter that points to a
   production. In the future, other callback functions of type cons_test_fn may
   need parameters of different types, so a void pointer is best. -AJC (8/7/02) */
/* Added thisAgent to cons_test_fn type, because we are eliminating the 
   global soar_agent. -AJC (8/7/02) */
//typedef Bool (*cons_test_fn)(cons *c);
typedef Bool (*cons_test_fn)(agent* thisAgent, cons *c, void* data);

typedef Bool (*dl_cons_test_fn)(dl_cons *dc, agent* thisAgent);

/* Added a void* parameter to extract_list_elements, because remove_pwatch_test_fn(), 
   one of the callback functions, requires a third parameter that points to a
   production. In the future, other callback functions of type cons_test_fn may
   need parameters of different types, so a void pointer is best. -AJC (8/7/02) */
extern list *extract_list_elements (agent* thisAgent, list **header, cons_test_fn f, void* data = 0);

extern dl_list *extract_dl_list_elements (agent* thisAgent, dl_list **header, dl_cons_test_fn f);

/* ----------------------------- */
/* Resizable hash table routines */
/* ----------------------------- */

extern unsigned long masks_for_n_low_order_bits[33];
typedef unsigned long ((*hash_function)(void *item, short num_bits));

typedef struct item_in_hash_table_struct {
  struct item_in_hash_table_struct *next;
  char data;
} item_in_hash_table;

typedef item_in_hash_table *bucket_array;

typedef struct hash_table_struct {
  unsigned long count;      /* number of items in the table */
  unsigned long size;       /* number of buckets */
  short log2size;           /* log (base 2) of size */
  short minimum_log2size;   /* table never shrinks below this size */
  bucket_array *buckets;
  hash_function h;          /* call this to hash or rehash an item */
} hash_table;  

extern struct hash_table_struct *make_hash_table (agent* thisAgent, short minimum_log2size,
                                                  hash_function h);
extern void remove_from_hash_table (agent* thisAgent, struct hash_table_struct *ht, void *item);
extern void add_to_hash_table (agent* thisAgent, struct hash_table_struct *ht, void *item);

typedef Bool (*hash_table_callback_fn)(void *item);
typedef Bool (*hash_table_callback_fn2)(agent* thisAgent, void *item, FILE* f);

extern void do_for_all_items_in_hash_table (agent* thisAgent, struct hash_table_struct *ht,
                                            hash_table_callback_fn2 f, FILE* fn);
extern void do_for_all_items_in_hash_bucket (struct hash_table_struct *ht,
                                             hash_table_callback_fn f,
                                             unsigned long hash_value);

#ifdef __cplusplus
}
#endif

#endif


