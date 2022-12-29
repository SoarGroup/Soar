#include "memory_manager.h"

#include "agent.h"
#include "mem.h"
#include "print.h"
#include "run_soar.h"
#include "sml_Names.h"
#include "stats.h"

#include <iostream>
#include <stdlib.h>

/* ====================================================================

                          Memory Pool Routines

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
==================================================================== */

#define DEFAULT_INTERLEAVE_FACTOR 1
/* should be 1 for maximum speed, but to avoid a gradual slowdown due
   to a gradually decreasing CPU cache hit ratio, make this a larger
   number, must be prime */
#define DEFAULT_BLOCK_SIZE 0x7FF0   /* about 32K bytes per block */

#define POOL_NAME_ERR_MSG_LENGTH = 64 + 2 * MAX_POOL_NAME_LENGTH

Memory_Manager::Memory_Manager()
{
    memory_for_usage_overhead = memory_for_usage + STATS_OVERHEAD_MEM_USAGE;

    for (int i = 0; i < NUM_MEM_USAGE_CODES; i++)
    {
        memory_for_usage[i] = 0;
    }
}

Memory_Manager::~Memory_Manager()
{
    /* Releasing memory pools */
    memory_pool* cur_pool = memory_pools_in_use;
    memory_pool* next_pool;
    while (cur_pool != NIL)
    {
        next_pool = cur_pool->next;
        free_memory_pool_by_ptr(cur_pool);
        cur_pool = next_pool;
    }

    // dynamic memory pools (cleared in the last step)
    for (std::unordered_map< size_t, memory_pool* >::iterator it = dyn_memory_pools.begin(); it != dyn_memory_pools.end(); it++)
    {
        delete it->second;
    }
    dyn_memory_pools.clear();
}

void Memory_Manager::init_memory_pool_by_ptr(memory_pool* pThisPool, size_t item_size, const char* name)
{
    if (pThisPool->initialized) return;

    if (item_size < sizeof(char*))
    {
        item_size = sizeof(char*);
    }
    while (item_size & 3)
    {
        item_size++;    /* make sure item_size is multiple of 4 */
    }
    pThisPool->item_size = item_size;
    pThisPool->items_per_block = DEFAULT_BLOCK_SIZE / item_size;
    pThisPool->num_blocks = 0;
    pThisPool->first_block = NIL;
    pThisPool->free_list = NIL;
    pThisPool->next = memory_pools_in_use;
    pThisPool->index = num_memory_pools;
    pThisPool->initialized = true;

    memory_pools_in_use = pThisPool;
    if (strlen(name) > MAX_POOL_NAME_LENGTH)
    {
        const unsigned int msg_size = 64 + 2 * MAX_POOL_NAME_LENGTH;
        char msg[msg_size];
        SNPRINTF(msg, msg_size, "memory_manager.cpp: Internal error: memory pool name too long: %s\n", name);
        msg[msg_size - 1] = 0; /* ensure null termination */
        abort_with_fatal_error_noagent(msg);
    }
    strncpy(pThisPool->name, name, MAX_POOL_NAME_LENGTH);
    pThisPool->name[MAX_POOL_NAME_LENGTH - 1] = 0; /* ensure null termination */

    #ifdef MEMORY_POOL_STATS
        pThisPool->used_count = 0;
    #endif

}

void Memory_Manager::init_memory_pool(MemoryPoolType mempool_index, size_t item_size, const char* name)
{
//    std::cout << "Init memory pool called for" << name << std::endl;
    memory_pool* lThisPool = &(memory_pools[mempool_index]);
    init_memory_pool_by_ptr(lThisPool, item_size, name);
    lThisPool->index = mempool_index;
}

memory_pool* Memory_Manager::get_memory_pool(size_t size)
{
    memory_pool* return_val = NULL;

    std::unordered_map< size_t, memory_pool* >::iterator it = dyn_memory_pools.find(size);
    if (it == dyn_memory_pools.end())
    {
        memory_pool* newbie = new memory_pool;

        init_memory_pool_by_ptr(newbie, size, "dynamic");
        dyn_memory_pools.insert(std::make_pair(size, newbie));

        return_val = newbie;
    }
    else
    {
        return_val = it->second;
    }

    return return_val;
}

void Memory_Manager::free_memory_pool_by_ptr(memory_pool* pThisPool)
{
//    std::cout << "Free memory pool called for" << pThisPool->name << std::endl;
    char* cur_block = static_cast<char*>(pThisPool->first_block);
    char* next_block;
    for (size_t i = 0; i < pThisPool->num_blocks; i++)
    {
        //std::cout << "Free memory block for " << pThisPool->name << std::endl;
        // the first 4 bytes point to the next block
        next_block = *(char**)cur_block;
        free_memory(cur_block, POOL_MEM_USAGE);
        cur_block = next_block;
    }
    pThisPool->num_blocks = 0;
    pThisPool->first_block = NIL;
    pThisPool->free_list = NIL;
}

void Memory_Manager::free_memory_pool(MemoryPoolType mempool_index)
{
    free_memory_pool_by_ptr(&(memory_pools[mempool_index]));
}

/* This is only called by the CLI DoAllocate command */
bool Memory_Manager::add_block_to_memory_pool_by_name(const std::string& pool_name, int blocks)
{
//    char* new_block;
//    size_t size, i, item_num, interleave_factor;
//    char* item, *prev_item;

    memory_pool* lThisPool;

    for (lThisPool = memory_pools_in_use; lThisPool != NIL; lThisPool = lThisPool->next)
    {
        if (pool_name == lThisPool->name) break;
    }
    if (!lThisPool)
        return false;

    for (int i = 0; i < blocks; ++i)
    {
        add_block_to_memory_pool(lThisPool);
    }
    return true;
}

void Memory_Manager::add_block_to_memory_pool(memory_pool* pThisPool)
{
    char* new_block;
    size_t size, i, item_num, interleave_factor;
    char* item, *prev_item;


    /* --- allocate a new block for the pool --- */
    size = pThisPool->item_size * pThisPool->items_per_block + sizeof(char*);
    new_block = static_cast<char*>(allocate_memory(size, POOL_MEM_USAGE));
    *(char**)new_block = static_cast<char*>(pThisPool->first_block);
    pThisPool->first_block = new_block;
    pThisPool->num_blocks++;

    /* somewhere in here, need to check if total mem usage exceeds limit set by user
we only check when increasing pools, because the other memories are small by comparison,
we shouldn't check for every block added to any pool, since that is unduly expensive
can we keep a block counter on the agent and check it modulo some function of the limit?
     */
    /*
uint64_t total = 0;
for (i=0; i<NUM_MEM_USAGE_CODES; i++) total += memory_for_usage[i];

if (total > thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE]) {
soar_invoke_callbacks(thisAgent, thisAgent,
MAX_MEMORY_USAGE_CALLBACK,
(soar_call_data) NULL);
print (thisAgent, "%8lu bytes total memory allocated\n", total);
print (thisAgent, "exceeds total allowed for Soar: %8lu bytes \n",
thisAgent->Decider->settings[DECIDER_MAX_MEMORY_USAGE]);
}

     */

    /* --- link up the new entries onto the free list --- */
    interleave_factor = DEFAULT_INTERLEAVE_FACTOR;
    if (interleave_factor >= pThisPool->items_per_block)
    {
        interleave_factor = 1;
    }

    item_num = interleave_factor;
    prev_item = new_block + sizeof(char*);   /* prev_item is item number 0 */
    for (i = 0; i < pThisPool->items_per_block - 1; i++)
    {
        item = new_block + sizeof(char*) + item_num * pThisPool->item_size;
        *(char**)prev_item = item;
        prev_item = item;
        item_num = item_num + interleave_factor;
        if (item_num >= pThisPool->items_per_block)
        {
            item_num -= pThisPool->items_per_block;
        }
    }
    *(char**)prev_item = static_cast<char*>(pThisPool->free_list);
    pThisPool->free_list = new_block + sizeof(char*);

}

/* ====================================================================

                   Basic Memory Allocation Utilities

   All memory blocks are allocated via calls to allocate_memory().  It
   calls malloc() and aborts if we run out of memory.  Free_memory() is
   the inverse of allocate_memory().  Allocate_memory_and_zerofill()
   does the obvious thing.  These routines take a usage_code indicating
   what purpose the memory is for (hash tables, strings, etc.).  This
   is used purely for statistics keeping.

   Print_memory_statistics() prints out stats on the memory usage.
==================================================================== */

void* Memory_Manager::allocate_memory(size_t size, int usage_code)
{
    char* p;

    memory_for_usage[usage_code] += size;
    size += sizeof(size_t);
    (*memory_for_usage_overhead) += sizeof(size_t);

    p = static_cast<char*>(malloc(size));
    if (p == NULL)
    {
        char msg[BUFFER_MSG_SIZE];
        SNPRINTF(msg, BUFFER_MSG_SIZE, "\nmem.c: Error:  Tried but failed to allocate %zu bytes of memory.\n", size);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error_noagent(msg);
    }
    if (reinterpret_cast<uintptr_t>(p) & 3)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "\nmem.c: Error:  Memory allocator returned an address that's not a multiple of 4.\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error_noagent(msg);
    }

    fill_with_zeroes(p, size);

    *(reinterpret_cast<size_t*>(p)) = size;
    p += sizeof(size_t);

    return p;
}

void* Memory_Manager::allocate_memory_and_zerofill(size_t size, int usage_code)
{
    void* p;

    p = allocate_memory(size, usage_code);
    memset(p, 0, size);
    return p;
}

void Memory_Manager::free_memory(void* mem, int usage_code)
{
    size_t size;

    if (mem == 0)
    {
        return;
    }

    mem = static_cast<char*>(mem) - sizeof(size_t);
    size = *(static_cast<size_t*>(mem));
    fill_with_garbage(mem, size);

    memory_for_usage[STATS_OVERHEAD_MEM_USAGE] -= sizeof(size_t);
    memory_for_usage[usage_code] -= (size - sizeof(size_t));

    free(mem);
}

void Memory_Manager::print_memory_statistics()
{
    size_t total;
    int i;

    total = 0;
    for (i = 0; i < NUM_MEM_USAGE_CODES; i++)
    {
        total += memory_for_usage[i];
    }
}

void Memory_Manager::debug_print_memory_stats(agent* thisAgent)
{
    // Hostname
    char hostname[256];
    memset(hostname, 0, 256);
    if (gethostname(hostname, 255) == SOCKET_ERROR)
    {
        strncpy(hostname, "[host name unknown]", 255);
    }

    // Time
    time_t current_time = time(NULL);

#ifndef NO_TIMING_STUFF
    double total_kernel_time = thisAgent->timers_total_kernel_time.get_sec();
    double total_kernel_msec = total_kernel_time * 1000.0;

    double input_function_time = thisAgent->timers_input_function_cpu_time.get_sec();
    double output_function_time = thisAgent->timers_output_function_cpu_time.get_sec();

    /* Total of the time spent in callback routines. */
    double monitors_sum = thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[PREFERENCE_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[WM_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec();

    double derived_kernel_time = get_derived_kernel_time_usec(thisAgent) / 1000000.0;
    double derived_total_cpu_time = derived_kernel_time + monitors_sum + input_function_time + output_function_time;

    /* Total time spent in the input phase */
    double input_phase_total_time = thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec()
                                    + thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec()
                                    + thisAgent->timers_input_function_cpu_time.get_sec();

    /* Total time spent in the propose phase */
    double propose_phase_total_time = thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec()
                                      + thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec();

    /* Total time spent in the apply phase */
    double apply_phase_total_time = thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec()
                                    + thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec();

    /* Total time spent in the output phase */
    double output_phase_total_time = thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec()
                                     + thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec()
                                     + thisAgent->timers_output_function_cpu_time.get_sec();

    /* Total time spent in the decide phase */
    double decide_phase_total_time = thisAgent->timers_decision_cycle_phase[DECIDE_PHASE].get_sec()
                                       + thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec();
#endif // NO_TIMING_STUFF

    /* The sum of these phase timers is exactly equal to the
    * derived_total_cpu_time
    */

    std::cout << "Soar " << sml::sml_Names::kSoarVersionValue << " on " << hostname << " at " << ctime(&current_time) << "\n";

    uint64_t totalProductions = thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];

    std::cout << totalProductions << " productions ("
             << thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE] << " default, "
             << thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE] << " user, "
             << thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE] << " chunks)\n";

    std::cout << "   + " << thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE] << " justifications\n";

    /* The fields for the timers are 8.3, providing an upper limit of
    approximately 2.5 hours the printing of the run time calculations.
    Obviously, these will need to be increased if you plan on needing
    run-time data for a process that you expect to take longer than
    2 hours. :) */

#ifndef NO_TIMING_STUFF
    std::cout << "                                                        |   Computed\n";
    std::cout << "Phases:      Input   Propose   Decide   Apply    Output |     Totals\n";
    std::cout << "========================================================|===========\n";

    std::cout << "Kernel:   "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[DECIDE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << derived_kernel_time << "\n";

    std::cout << "========================================================|===========\n";

    std::cout << "Input fn: "
             << std::setw(8) << input_function_time << "                                      | "
             << std::setw(10) << input_function_time << "\n";

    std::cout << "========================================================|===========\n";
    std::cout << "Outpt fn:                                     "
             << std::setw(8) << output_function_time << "  | "
             << std::setw(10) << output_function_time << "\n";

    std::cout << "========================================================|===========\n";

    std::cout << "Callbcks: "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << monitors_sum << "\n";

    std::cout << "========================================================|===========\n";
    std::cout << "Computed------------------------------------------------+-----------\n";
    std::cout << "Totals:   "
             << std::setw(8) << input_phase_total_time << " "
             << std::setw(8) << propose_phase_total_time << " "
             << std::setw(8) << decide_phase_total_time << " "
             << std::setw(8) << apply_phase_total_time << " "
             << std::setw(8) << output_phase_total_time << "  | "
             << std::setw(10) << derived_total_cpu_time << "\n\n";

#ifdef DETAILED_TIMING_STATS
    double match_sum = thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[WM_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[DECIDE_PHASE].get_sec();
    double own_sum = thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[DECIDE_PHASE].get_sec();
    double chunk_sum = thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[DECIDE_PHASE].get_sec();
    double gds_sum = thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[DECIDE_PHASE].get_sec();

    std::cout << "Detailed Timing Stats:   " << "\n\n";
    std::cout << "=============================================================================|===========\n";
    std::cout << "Phases:        Input   Propose   Decide   Apply    Pref     WrkMem    Output |     Totals\n";
    std::cout << "=============================================================================|===========\n";
    std::cout << "Match:      "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[DECIDE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[WM_PHASE].get_sec() << "  | "
             << std::setw(10) << match_sum << "\n";
    std::cout << "Ownership:  "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[DECIDE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << own_sum << "\n";
    std::cout << "Chunking:   "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[DECIDE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << chunk_sum << "\n";
    std::cout << "GDS:        "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[DECIDE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << gds_sum << "\n";
#endif // DETAILED_TIMING_STATS

    std::cout << "Values from single timers:\n";

    std::cout << " Kernel CPU Time: "
             << std::setw(11) << total_kernel_time << " sec. \n";

    std::cout << " Total  CPU Time: "
             << std::setw(11) << thisAgent->timers_total_cpu_time.get_sec() << " sec.\n\n";

    ///* v8.6.2: print out decisions executed, not # full cycles */

    std::cout << thisAgent->decide_phases_count << " decisions ("
             << (thisAgent->decide_phases_count ? total_kernel_msec / thisAgent->decide_phases_count : 0.0)
             << " msec/decision)\n";
    std::cout << thisAgent->e_cycle_count << " elaboration cycles ("
             << (thisAgent->decide_phases_count ? static_cast<double>(thisAgent->e_cycle_count) / thisAgent->decide_phases_count : 0)
             << " ec's per dc, "
             << (thisAgent->e_cycle_count ? total_kernel_msec / thisAgent->e_cycle_count : 0)
             << " msec/ec)\n";
    std::cout << thisAgent->inner_e_cycle_count << " inner elaboration cycles\n";

    std::cout << thisAgent->pe_cycle_count << " p-elaboration cycles ("
             << (thisAgent->decide_phases_count ? static_cast<double>(thisAgent->pe_cycle_count) / thisAgent->decide_phases_count : 0)
             << " pe's per dc, "
             << (thisAgent->pe_cycle_count ? total_kernel_msec / thisAgent->pe_cycle_count : 0)
             << " msec/pe)\n";

    std::cout << thisAgent->production_firing_count << " production firings ("
             << (thisAgent->e_cycle_count ? static_cast<double>(thisAgent->production_firing_count) / thisAgent->e_cycle_count : 0.0)
             << " pf's per ec, "
             << (thisAgent->production_firing_count ? total_kernel_msec / thisAgent->production_firing_count : 0.0)
             << " msec/pf)\n";
#endif // NO_TIMING_STUFF

    uint64_t wme_changes = thisAgent->wme_addition_count + thisAgent->wme_removal_count;
    std::cout << wme_changes << " wme changes ("
             << thisAgent->wme_addition_count << " additions, "
             << thisAgent->wme_removal_count << " removals)\n";

    std::cout << "WM size: "
             << thisAgent->num_wmes_in_rete << " current, "
             << (thisAgent->num_wm_sizes_accumulated ? (thisAgent->cumulative_wm_size / thisAgent->num_wm_sizes_accumulated) : 0.0)
             << " mean, "
             << thisAgent->max_wm_size << " maximum\n\n";

    std::cout << "Single decision cycle maximums:\n";

    std::cout << "Stat             Value       Cycle\n";
    std::cout << "---------------- ----------- -----------\n";

#ifndef NO_TIMING_STUFF
    std::cout << std::setw(16) << "Time (sec)"
             << std::setw(11) << std::setprecision(6) << (thisAgent->max_dc_time_usec / 1000000.0) << " "
             << std::setw(11) << thisAgent->max_dc_time_cycle << "\n";

    std::cout << std::setw(16) << "EpMem Time (sec)"
             << std::setw(11) << std::setprecision(6) << thisAgent->max_dc_epmem_time_sec << " "
             << std::setw(11) << thisAgent->max_dc_epmem_time_cycle << "\n";

    std::cout << std::setw(16) << "SMem Time (sec)"
             << std::setw(11) << std::setprecision(6) << thisAgent->max_dc_smem_time_sec << " "
             << std::setw(11) << thisAgent->max_dc_smem_time_cycle << "\n";
#endif // NO_TIMING_STUFF

    std::cout << std::setw(16) << "WM changes"
             << std::setw(11) << thisAgent->max_dc_wm_changes_value << " "
             << std::setw(11) << thisAgent->max_dc_wm_changes_cycle << "\n";

    std::cout << std::setw(16) << "Firing count"
             << std::setw(11) << thisAgent->max_dc_production_firing_count_value << " "
             << std::setw(11) << thisAgent->max_dc_production_firing_count_cycle << "\n";
                 size_t total = 0;

    for (int i = 0; i < NUM_MEM_USAGE_CODES; i++)
    {
        total += thisAgent->memoryManager->memory_for_usage[i];
    }

    std::cout << std::setw(8) << total << " bytes total memory allocated\n";
    std::cout << std::setw(8) << thisAgent->memoryManager->memory_for_usage[STATS_OVERHEAD_MEM_USAGE] << " bytes statistics overhead\n";
    std::cout << std::setw(8) << thisAgent->memoryManager->memory_for_usage[STRING_MEM_USAGE] << " bytes for strings\n";
    std::cout << std::setw(8) << thisAgent->memoryManager->memory_for_usage[HASH_TABLE_MEM_USAGE] << " bytes for hash tables\n";
    std::cout << std::setw(8) << thisAgent->memoryManager->memory_for_usage[POOL_MEM_USAGE] << " bytes for various memory pools\n";
    std::cout << std::setw(8) << thisAgent->memoryManager->memory_for_usage[MISCELLANEOUS_MEM_USAGE] << " bytes for miscellaneous other things\n";

    std::cout << "Memory pool statistics:\n\n";
#ifdef MEMORY_POOL_STATS
    std::cout << "Pool Name        Used Items  Free Items  Item Size  Itm/Blk  Blocks  Total Bytes\n";
    std::cout << "---------------  ----------  ----------  ---------  -------  ------  -----------\n";
#else
    std::cout << "Pool Name        Item Size  Itm/Blk  Blocks  Total Bytes\n";
    std::cout << "---------------  ---------  -------  ------  -----------\n";
#endif

    for (memory_pool* p = memory_pools_in_use; p != NIL; p = p->next)
    {
        std::cout << std::setw(MAX_POOL_NAME_LENGTH) << p->name;
#ifdef MEMORY_POOL_STATS
        std::cout << "  " << std::setw(10) << p->used_count;
        size_t total_items = p->num_blocks * p->items_per_block;
        std::cout << "  " << std::setw(10) << total_items - p->used_count;
#endif
        std::cout << "  " << std::setw(9) << p->item_size;
        std::cout << "  " << std::setw(7) << p->items_per_block;
        std::cout << "  " << std::setw(6) << p->num_blocks;
        std::cout << "  " << std::setw(11) << p->num_blocks* p->items_per_block* p->item_size;
        std::cout << "\n";
    }

}
