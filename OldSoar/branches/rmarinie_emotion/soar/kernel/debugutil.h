
extern void examine_memory_pool(memory_pool * p);

extern void *get_item_in_pool_block(memory_pool * p, long item, long block, bool pfr);
extern void print_wme_in_pool(long w, long b);
extern void print_all_wmes_in_block(long b, bool pfree);
extern void print_conditions_in_pool(long w, long b, bool pfree);
extern void print_all_conditions_in_block(long b, bool pfree);
extern void print_all_identifiers_in_block(long b, bool pfree);
extern void print_all_productions_in_block(long b, bool full, bool pfree);
extern int check_for_addr_in_free_list(memory_pool * p, void *addr);
extern void print_all_preferences_in_block(long b, bool p_free);

extern void print_all_instantiations_in_block(long b, wme_trace_type wtt, bool p_free);
