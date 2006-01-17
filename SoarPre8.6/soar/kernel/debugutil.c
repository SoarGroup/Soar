#include "soarkernel.h"
#include "debugutil.h"

extern void detailed_print_wme(wme * w);
extern void detailed_print_preference(preference * p);

void examine_memory_pool(memory_pool * p)
{

    print("Examine Memory Pool:\n");
    print("Free List Head:   %p\n", p->free_list);

#ifdef MEMORY_POOL_STATS
    print("Used Count:       %ld\n", p->used_count);
#endif
#ifdef TRACE_MEMORY_USAGE
    print("Free List Length: %ld\n", p->free_list_length);
    print("Pool Size:        %ld\n", p->pool_size);
#endif

    print("Item Size:        %ld\n", p->item_size);
    print("Items / Block:    %ld\n", p->items_per_block);
    print("Num Blocks:       %ld\n", p->num_blocks);
    print("First Block:      %p\n", p->first_block);
    print("Name:             %s\n", p->name);

}

void print_item_info_header(void)
{
    print("Block Number, Block Address, Next Block, Item Address\n");
}

void *get_item_in_pool_block(memory_pool * p, long n, long b, bool p_free)
{

    char *block_head;
    long i;
    void *item;
    bool in_free_list;

    block_head = p->first_block;
    for (i = 0; i < b; i++) {

        /* I think this is right.
           What I'm going for, is as follows:
           block_head = the first few bytes of the block, cast as a pointer
           and then dereferenced.
         */
        block_head = *(char **) block_head;
    }

    block_head += sizeof(char *);

    if (n > p->items_per_block) {
        print("ERROR: The block contains less than %ld items.\n", n);
        return NULL;
    }

    for (i = 0; i < n; i++) {
        block_head += p->item_size;
    }
    item = block_head;

    in_free_list = (char) check_for_addr_in_free_list(p, block_head);

    if (!in_free_list || p_free) {
        print("%ld, %p, %p, ", b, (char *) block_head, *(char **) block_head);
        print("%p\n", block_head);
    }
    if (in_free_list && p_free) {
        print("This item is in the free list. \n");
    }
    if (in_free_list)
        return NULL;
    return block_head;

}

void print_all_productions_in_block(long b, bool full, bool p_free)
{
    long i;
    production *pro;

    print_item_info_header();
    for (i = 0; i < current_agent(production_pool).items_per_block; i++) {
        pro = (production *) get_item_in_pool_block(&current_agent(production_pool), i, b, p_free);

        if (pro) {
            if (full) {
                print_production(pro, FALSE);
            } else {
                print_with_symbols("%y\n", pro->name);
            }
        }
    }

}

void print_all_identifiers_in_block(long b, bool p_free)
{
    long i;
    Symbol *s;

    print_item_info_header();
    for (i = 0; i < current_agent(identifier_pool).items_per_block; i++) {
        s = (Symbol *) get_item_in_pool_block(&current_agent(identifier_pool), i, b, p_free);

        if (s) {
            print(symbol_to_string(s, TRUE, NIL, 0));
            print("   reference count: %d\n", s->common.reference_count);

        }

    }

}

void print_all_wmes_in_block(long b, bool p_free)
{
    long i;
    wme *the_wme;

    print_item_info_header();
    for (i = 0; i < current_agent(wme_pool).items_per_block; i++) {
        the_wme = (wme *) get_item_in_pool_block(&current_agent(wme_pool), i, b, p_free);

        if (the_wme)
            detailed_print_wme(the_wme);
    }

}

void print_all_conditions_in_block(long b, bool p_free)
{
    long i;
    condition *the_condition;

    print_item_info_header();
    for (i = 0; i < current_agent(condition_pool).items_per_block; i++) {
        the_condition = (condition *) get_item_in_pool_block(&current_agent(condition_pool), i, b, p_free);

        if (the_condition)
            print_condition(the_condition);
    }

}

void print_all_preferences_in_block(long b, bool p_free)
{
    long i;
    preference *the_pref;

    print_item_info_header();

    for (i = 0; i < current_agent(preference_pool).items_per_block; i++) {
        the_pref = (preference *) get_item_in_pool_block(&current_agent(preference_pool), i, b, p_free);

        if (the_pref)
            detailed_print_preference(the_pref);
    }

}

void print_all_instantiations_in_block(long b, wme_trace_type wtt, bool p_free)
{
    long i;
    instantiation *the_inst;

    print_item_info_header();
    for (i = 0; i < current_agent(instantiation_pool).items_per_block; i++) {
        the_inst = (instantiation *) get_item_in_pool_block(&current_agent(instantiation_pool), i, b, p_free);

        if (the_inst)
            print_instantiation_with_wmes(the_inst, wtt);
    }
}

int check_for_addr_in_free_list(memory_pool * p, void *addr)
{

    void *free_block_addr;

    free_block_addr = p->free_list;

    while (free_block_addr != NULL) {

        /*
           print( "Looking for %p in List...%p\n", addr, free_block_addr ); 
         */

        if (free_block_addr == addr)
            return 1;

        free_block_addr = *(void **) free_block_addr;

    }

    return 0;
}

#ifdef USE_DEBUG_UTILS

void allocate_with_pool_fn(memory_pool * p, void **dest_item_pointer)
{

    if (!(p)->free_list)
        add_block_to_memory_pool(p);
    *(dest_item_pointer) = (p)->free_list;
    (p)->free_list = *(void **) (*(dest_item_pointer));
    fill_with_garbage(*(dest_item_pointer), (p)->item_size);
    increment_used_count(p);
    decrement_free_list_length(p);

}

void free_with_pool_fn(memory_pool * p, void *item)
{

    fill_with_garbage((item), (p)->item_size);
    *(void **) (item) = (p)->free_list;
    (p)->free_list = (void *) (item);
    decrement_used_count(p);
    increment_free_list_length(p);

}
#endif
