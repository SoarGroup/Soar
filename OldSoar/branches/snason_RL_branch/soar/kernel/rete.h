
extern void print_node_count_statistics(void);
extern void print_null_activation_statistics(void);

#ifdef NULL_ACTIVATION_STATS
extern void print_null_activation_stats(void);
#else
#define print_null_activation_stats() {}
#endif

typedef struct token_struct {
    /* --- Note: "parent" is NIL on negative node negrm (local join result) 
       tokens, non-NIL on all other tokens including CN and CN_P stuff.
       I put "parent" at offset 0 in the structure, so that upward scans
       are fast (saves doing an extra integer addition in the inner loop) --- */
    struct token_struct *parent;
    union token_a_union {
        struct token_in_hash_table_data_struct {
            struct token_struct *next_in_bucket, *prev_in_bucket;       /*hash bucket dll */
            Symbol *referent;   /* referent of the hash test (thing we hashed on) */
        } ht;
        struct token_from_right_memory_of_negative_or_cn_node_struct {
            struct token_struct *next_negrm, *prev_negrm;       /*other local join results */
            struct token_struct *left_token;    /* token this is local join result for */
        } neg;
    } a;
    struct rete_node_struct *node;
    wme *w;
    struct token_struct *first_child;   /* first of dll of children */
    struct token_struct *next_sibling, *prev_sibling;   /* for dll of children */
    struct token_struct *next_of_node, *prev_of_node;   /* dll of tokens at node */
    struct token_struct *next_from_wme, *prev_from_wme; /* tree-based remove */
    struct token_struct *negrm_tokens;  /* join results: for Neg, CN nodes only */
} token;

typedef struct match_set_trace {
    Symbol *sym;
    int count;
    struct match_set_trace *next;
    /* REW: begin 08.20.97 */
    /* Add match goal to the print of the matching production */
    Symbol *goal;
    /* REW: end   08.20.97 */
} MS_trace;

void print_whole_token(token * t, wme_trace_type wtt);
