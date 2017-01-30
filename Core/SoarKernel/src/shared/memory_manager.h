/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  memory_manager.h
 *
 * A memory manager class that decouples memory pools from the individual
 * agent.
 *
 * - MPM is a singleton like the OutputManager and SoarInstance.  It
 *   is created on Kernel creation.
 *
 * - MPM uses an enum list for all the core memory pool types.  Kernel
 *   calls that deal with memory pools now pass in a parameter to
 *   specify which pool instead of the actual pool itself (which was
 *   in the agent, but is now in the MPM)
 *
 * - Memory pools now have an initialized flag, since more than one
 *   agent may try to initialize a pool.
 *
 * - Agent caches a pointer to MPM to ease access.  Also made
 *   refactoring slightly less painful.
 *
 * =======================================================================
 */

#ifndef MEMPOOL_MANAGER_H_
#define MEMPOOL_MANAGER_H_

#include "kernel.h"

#include <unordered_map>

#ifndef _WIN32
#include <stdlib.h> // malloc
#endif // !_WIN32

#define MAX_POOL_NAME_LENGTH 15
#define MISCELLANEOUS_MEM_USAGE  0
#define HASH_TABLE_MEM_USAGE     1
#define STRING_MEM_USAGE         2
#define POOL_MEM_USAGE           3
#define STATS_OVERHEAD_MEM_USAGE 4

#define NUM_MEM_USAGE_CODES 5

typedef struct memory_pool_struct
{
    void* free_list;             /* header of chain of free items */
    MemoryPoolType index;
    size_t used_count;             /* used for statistics only when #def'd MEMORY_POOL_STATS */
    size_t item_size;               /* bytes per item */
    size_t items_per_block;        /* number of items in each big block */
    size_t num_blocks;             /* number of big blocks in use by this pool */
    void* first_block;           /* header of chain of blocks */
    char name[MAX_POOL_NAME_LENGTH];  /* name of the pool (for memory-stats) */
    bool initialized;
    struct memory_pool_struct* next;  /* next in list of all memory pools */
    memory_pool_struct() : initialized(false) {}
} memory_pool;

/* ----------------------- */
/* basic memory allocation */
/* ----------------------- */

/* Note:  DEBUG_MEMORY currently does nothing on allocation. It previously filled with zero's on
 *        allocation and 0xbbb on deallocation.  The reasoning is that filling with zeroes on
 *        allocation seems to hide uninitialization variable bugs, though I suppose if you suspect
 *        that an uninitialized variable is causing a problem, you could use that option to test it.
 */
#ifdef DEBUG_MEMORY
    template <typename T>
    inline void fill_with_garbage(T* block, size_t size)
    {
        memset(static_cast<void*>(block), 0xBB, (size));
    }
    /* Disabling the fill with zeros part since that may actually hide bugs */
    #define fill_with_zeroes(block,size) { }

    //template <typename T>
    //inline void fill_with_zeroes(T* block, size_t size)
    //{
    //    memset(static_cast<void*>(block), 0, (size));
    //}
#else
    #define fill_with_garbage(block,size) { }
    #define fill_with_zeroes(block,size) { }
#endif

#ifdef MEMORY_POOL_STATS

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

#else

#define increment_used_count(p) { }
#define decrement_used_count(p) { }

#endif /* MEMORY_POOL_STATS */

class Memory_Manager
{
        /* CLI is a friend because it prints out mempool stats */
        friend class cli::CommandLineInterface;

    public:
        static Memory_Manager& Get_MPM()
        {
            static Memory_Manager instance;
            return instance;
        }
        virtual ~Memory_Manager();

        void init_MemPool_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance);
        void add_block_to_memory_pool(memory_pool* pThisPool);
        bool add_block_to_memory_pool_by_name(const std::string& pool_name, int blocks);
        void reinit_memory_pool(MemoryPoolType mempool_index);
        void init_memory_pool(MemoryPoolType mempool_index, size_t item_size, const char* name);
        void init_memory_pool_by_ptr(memory_pool* pThisPool, size_t item_size, const char* name);
        void free_memory_pool(MemoryPoolType mempool_index);
        memory_pool* get_memory_pool(size_t size);
        void* allocate_memory(size_t size, int usage_code);
        void* allocate_memory_and_zerofill(size_t size, int usage_code);
        void free_memory(void* mem, int usage_code);
        void print_memory_statistics();
        void debug_print_memory_stats(agent* thisAgent);

        std::unordered_map< size_t, memory_pool* >*   dyn_memory_pools;

    private:

        Memory_Manager();

        /* The following two functions are declared but not implemented to avoid copies of singletons */
        Memory_Manager(Memory_Manager const&) {};
        void operator=(Memory_Manager const&) {};

        Soar_Instance*                            m_Soar_Instance;
        sml::Kernel*                              m_Kernel;

        memory_pool                         memory_pools[num_memory_pools];

        /* Counters for memory usage of various types */
        size_t              memory_for_usage[NUM_MEM_USAGE_CODES];

        /* List of all memory pools being used */
        memory_pool*        memory_pools_in_use;
        size_t* memory_for_usage_overhead;
        void free_memory_pool_by_ptr(memory_pool* pThisPool);

    public:
        template <typename T>
        inline void allocate_with_pool(MemoryPoolType mempool_index, T** dest_item_pointer)
        {
            memory_pool* lThisPool = &(memory_pools[mempool_index]);

        #if MEM_POOLS_ENABLED
            // if there's no memory blocks left in the pool, then allocate a new one
            if (!lThisPool->free_list)
            {
                add_block_to_memory_pool(lThisPool);
            }
            // take the beginning of the next free block and give it to the T pointer
            *(dest_item_pointer) = static_cast< T* >(lThisPool->free_list);
            // we think this line increments free_list to the next available memory block
            // we thought it took advantage of the fact that free_list is the first
            //  member of memory_pool, but I tried changing that and it still works, so now I'm at a loss
            // if it helps, we think this line is equivalent to the following
            //  (at least, everything appears to work properly if you swap these lines):
            // lThisPool->free_list = (*static_cast<P*>(dest_item_pointer))->free_list;
            lThisPool->free_list =  *(void**)(*(dest_item_pointer));

            increment_used_count(lThisPool);

        #else // !MEM_POOLS_ENABLED
            // this is for debugging -- it disables the memory pool usage and just allocates
            //  new memory every time.  If you want to use it, be sure to make the corresponding
            //  change to free_with_pool below
            *dest_item_pointer = static_cast< T* >(malloc(sizeof(T)));

            // simply prevents compiler warnings when memory pools disabled
        //   thisAgent=thisAgent;
        //   p=p;

        #endif // !MEM_POOLS_ENABLED
            fill_with_zeroes(*(dest_item_pointer), lThisPool->item_size);
        }

        template <typename T>
        inline void allocate_with_pool_ptr(memory_pool* pThisPool, T** dest_item_pointer)
        {

        #if MEM_POOLS_ENABLED
            // if there's no memory blocks left in the pool, then allocate a new one
            if (!pThisPool->free_list)
            {
                add_block_to_memory_pool(pThisPool);
            }
            // take the beginning of the next free block and give it to the T pointer
            *(dest_item_pointer) = static_cast< T* >(pThisPool->free_list);
            // we think this line increments free_list to the next available memory block
            // we thought it took advantage of the fact that free_list is the first
            //  member of memory_pool, but I tried changing that and it still works, so now I'm at a loss
            // if it helps, we think this line is equivalent to the following
            //  (at least, everything appears to work properly if you swap these lines):
            // pThisPool->free_list = (*static_cast<P*>(dest_item_pointer))->free_list;
            pThisPool->free_list =  *(void**)(*(dest_item_pointer));

            increment_used_count(pThisPool);

        #else // !MEM_POOLS_ENABLED
            // this is for debugging -- it disables the memory pool usage and just allocates
            //  new memory every time.  If you want to use it, be sure to make the corresponding
            //  change to free_with_pool below
            *dest_item_pointer = static_cast< T* >(malloc(sizeof(T)));

            // simply prevents compiler warnings when memory pools disabled
        //   thisAgent=thisAgent;
        //   p=p;

        #endif // !MEM_POOLS_ENABLED
            fill_with_zeroes(*(dest_item_pointer), pThisPool->item_size);
        }

        template <typename T>
        inline void free_with_pool(MemoryPoolType mempool_index, T* item)
        {
            memory_pool* lThisPool = &(memory_pools[mempool_index]);
            fill_with_garbage((item), lThisPool->item_size);
//            fill_with_zeroes((item), lThisPool->item_size);
        #if MEM_POOLS_ENABLED
            *(void**)(item) = lThisPool->free_list;
            lThisPool->free_list = (void*)(item);
            decrement_used_count(lThisPool);

        #else // !MEM_POOLS_ENABLED
            // this is for debugging -- it disables the memory pool usage and just deallocates
            //  the memory every time.  If you want to use it, be sure to make the corresponding
            //  change to allocate_with_pool above
            free(item);

            // simply prevents compiler warnings when memory pools disabled
        //   p=p;
        #endif // !MEM_POOLS_ENABLED
        }

        template <typename T>
        inline void free_with_pool_ptr(memory_pool* pThisPool, T* item)
        {
            fill_with_garbage((item), pThisPool->item_size);
//            fill_with_zeroes((item), pThisPool->item_size);
        #if MEM_POOLS_ENABLED
            *(void**)(item) = pThisPool->free_list;
            pThisPool->free_list = (void*)(item);
            decrement_used_count(pThisPool);

        #else // !MEM_POOLS_ENABLED
            // this is for debugging -- it disables the memory pool usage and just deallocates
            //  the memory every time.  If you want to use it, be sure to make the corresponding
            //  change to allocate_with_pool above
            free(item);

            // simply prevents compiler warnings when memory pools disabled
        //   p=p;
        #endif // !MEM_POOLS_ENABLED
        }
//
//        /* Memory pools */
//        memory_pool         float_constant_pool;
//        memory_pool         identifier_pool;
//        memory_pool         int_constant_pool;
//        memory_pool         str_constant_pool;
//        memory_pool         variable_pool;
//
//        memory_pool         instantiation_pool;
//        memory_pool         chunk_cond_pool;
//
//        memory_pool         preference_pool;
//        memory_pool         wme_pool;
//        memory_pool         output_link_pool;
//        memory_pool         io_wme_pool;
//        memory_pool         slot_pool;
//        memory_pool         gds_pool;
//
//        /* ----------------------- Misc. top-level stuff -------------------------- */
//
//        memory_pool         action_pool;
//        memory_pool         test_pool;
//        memory_pool         condition_pool;
//        memory_pool         not_pool;
//        memory_pool         production_pool;
//        memory_pool         rhs_symbol_pool;
//
//        /* ----------------------- Reorderer stuff -------------------------- */
//
//        memory_pool         saved_test_pool;
//
//        memory_pool         cons_cell_pool; /* pool for cons cells */
//        memory_pool         dl_cons_pool;   /* doubly-linked list cells */
//
//        memory_pool         rete_node_pool;
//        memory_pool         rete_test_pool;
//        memory_pool         right_mem_pool;
//        memory_pool         token_pool;
//        memory_pool         alpha_mem_pool;
//        memory_pool         ms_change_pool;
//        memory_pool         node_varnames_pool;
//
//        memory_pool         rl_info_pool;
//        memory_pool         rl_et_pool;
//        memory_pool         rl_rule_pool;
//
//        memory_pool         wma_decay_element_pool;
//        memory_pool         wma_decay_set_pool;
//        memory_pool         wma_wme_oset_pool;
//        memory_pool         wma_slot_refs_pool;
//
//        memory_pool         epmem_wmes_pool;
//        memory_pool         epmem_info_pool;
//        memory_pool         smem_wmes_pool;
//        memory_pool         smem_info_pool;
//
//        memory_pool         epmem_literal_pool;
//        memory_pool         epmem_pedge_pool;
//        memory_pool         epmem_uedge_pool;
//        memory_pool         epmem_interval_pool;

};

#endif
