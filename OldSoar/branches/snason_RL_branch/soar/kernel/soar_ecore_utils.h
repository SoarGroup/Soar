#ifndef _SOAR_ECORE_UTILS_H_    /* ExcludeFromBuildInfo */
#define _SOAR_ECORE_UTILS_H_

#include "soarkernel.h"
#include "soar_core_utils.h"
#include "soarapi_datatypes.h"

typedef struct production_memory_use_struct {
    Symbol *name;
    int mem;
    struct production_memory_use_struct *next;
} production_memory_use;

typedef struct wme_filter_struct {
    Symbol *id;
    Symbol *attr;
    Symbol *value;
    bool adds;
    bool removes;
} wme_filter;

typedef struct replayed_id_map_struct {
    /* Warning this MUST be the first field for the ht routines */
    struct replayed_id_map_struct *next;

    char *from;
    char *to;
} replayed_id_map;

void cb_soar_PrintToFile(soar_callback_agent a, soar_callback_data d, soar_call_data c);

#ifdef USE_CAPTURE_REPLAY
extern void capture_input_wme(enum captured_action_type action, soarapi_wme * sw, wme * w);
extern void replay_input_wme(soar_callback_agent agent, soar_callback_data dummy, soar_call_data mode);
#endif

extern int compare_firing_counts(const void *e1, const void *e2);

extern production_memory_use *print_memories_insert_in_list(production_memory_use * n, production_memory_use * l);
extern int read_wme_filter_component(const char *s, Symbol ** sym);
extern void soar_alternate_input(agent * ai_agent, const char *ai_string, const char *ai_suffix, bool ai_exit);

extern int read_attribute_from_string(Symbol * id, const char *the_lexeme, Symbol * *attr);
extern void print_preference_and_source(preference * pref, bool print_source, wme_trace_type wtt);

unsigned long int hash_soar_id_string(void *id, short nbits);
char *replayed_hash_contains_id(char *fromid);
#endif
