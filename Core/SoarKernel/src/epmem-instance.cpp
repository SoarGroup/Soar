/*************************************************************************
 *
 * file:  epmem.c
 *
 * Routines for Soar's episodic memory module (added in 2002 by :AMN:)
 *
 *
 * Copyright (c) 1995-2002 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 * =======================================================================
 */

/*
 *  %%%TODO:
 *  1.  Add my own MEM_USAGE constant?
 *  2.  Put global variables into soar_agent (e.g., current_agent())
 *  3.  Allow user to configure the global variables on the command line
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

#include <limits.h>
#include "soarkernel.h"
#include "epmem.h"
#include "agent.h"
#include "prefmem.h"
#include "print.h"
#include "io.h"
#include "activate.h"
#include "utilities.h"
#include "gski_event_system_functions.h"

#ifdef EPISODIC_MEMORY

//defined in symtab.c
extern unsigned long compress(unsigned long h, short num_bits);
extern unsigned long hash_string(const char *s);


/* EpMem constants
   
   num_active_wmes - epmem uses the n most active wmes to decide whether to
                     record a new memory.  This is n.
   num_wmes_changed - number of wmes in the current list that must be
                      different from the previous list to trigger
                      a new memory.
   memories_init_size  - starting size for the g_memories array
   memory_match_wait   - How long to wait before memories can be recalled.  This
                         value is expressed in the number of newer memories that
                         must exist before this one is retrievable.
   ubiquitous_threshold - When a wmetree node's assoc_memories list has
                          a number of entries that exceeds this fraction
                          of the total number of episodic memories then it
                          has become too ubiquitous and is no longer used
                          in matching.
   ubiquitous_max       - There must be at least this many episodic memories
                          before any node can be considered ubiquitous
   fraction_to_trim     - fraction of a new memory to trim before recording it
                          
   %%%TODO:  Made these values command line configurable

*/

#define num_active_wmes 1
#define num_wmes_changed 1
#define memories_init_size 512
#define memory_match_wait 1     // %%%TODO: no longer needed?
#define ubiquitous_threshold 1.0
#define ubiquitous_max 25
#define fraction_to_trim 0.0

/*======================================================================
 * Data structures
 *----------------------------------------------------------------------
 */

/*
 * arraylist - A growable array.  The soar kernel in general could
 *             benefit from this struct and its related routines.
 *
 *             I use an arraylist to store each episodic memory (as a list of
 *             pointers into g_wmetree).  I also use it in the wmetree stuct to
 *             store a list of pointers to memories that use that node.
 *
 *             array - an array of void pointers
 *             capactiy - the current size of the array
 *             size - the number of cells in the array currently being used
 *             next - used to create a linked list of these (also allows
 *                    hashing)
 *
 */
typedef struct arraylist_struct
{
    struct arraylist_struct *next;
    void **array;
    long capacity;
    long size;
} arraylist;

/*

   wmetree - Used to build a tree representation of the structure and content of
             all states that the agent has encountered.  This in turn is used to
             construct an episodic memory

             id             - every WME in the tree has a unique id.  I can't
                              use the timetag because multiple WMEs may have the same
                              value.
             attr           - the string representing the WME attribute associated
                              with this tree node
             val            - the string representing the WME value associated
                              with this tree node (*only* if this is a leaf node)
             children       - children of the current node (hash table)
             parent         - parent of the current node.
             depth          - depth in the tree (root node is depth=0)
             next/prev      - dll of all nodes in the tree to allow iterative
                              implementation of several algorithms
             assoc_wmes     - a list of wmes that are currently in WM that
                              were created using this node as their template
                              (i.e., as part of a retrieval).  The index of
                              a particular WME in the list always equals
                              the index of the epmem_header for the memory.
             assoc_memories - this is a list of pointers to all the episodic
                              memories that use this WME.  As such it is
                              effectively an arraylist of arraylists.
             query_count    - How many times this WME has been in a cue and
                              therfore triggered its associated memories to be
                              examined for a match. (performance diagnostic use
                              only)
                      
*/


typedef struct wmetree_struct
{
    struct wmetree_struct *next; // used by the hash table
    char *attr;
    union
    {
        char *strval;
        long intval;
        float floatval;
    } val;
    int val_type;

    hash_table *children;
    struct wmetree_struct *parent; // %%%TODO: make this an array later?
    int depth;
    arraylist *assoc_wmes;
    arraylist *assoc_memories;
    int query_count;
    int ubiquitous;
} wmetree;


/*
 * actwme - a wme paired with an activation value.  Activation values
 *          are stored when a memory is recorded.  They need to share
 *          a data structure together so an array of them can be sorted.
 *
 */
typedef struct actwme_struct
{
    wmetree *node;
    int activation;
} actwme;


/*
 * episodic_memory - This data structure contains a single episodic memory.
 *
 *         content     - a list of actwme structs.
 *         index       - this memory's index in the g_memories arraylist
 *         last_usage  - the number of the last retrieval that partially
 *                       matched this memory
 *         match_score - the total match score from the last partial match
 *
 */
typedef struct episodic_memory_struct
{
    arraylist *content;
    int index;
    int last_usage;
    int match_score;
} episodic_memory;


/*
 * epmem_header - An arraylist of these is used to keep track of the
 *                ^epmem link attached to each state in working memory
 *
 *  index        - the index of this header in g_header_stack.  This
 *                 index is used to reference the assoc_wmes arraylist
 *                 in a wmetree node
 *  state        - the state that ^epmem is attached to
 *  ss_wme       - a pointer to the state's ^superstate WME (used for
 *                 creating fake prefs)
 *  epmem        - The symbol that ^epmem has as an attribute
 *  query        - The symbol that ^query has as an attribute
 *  retrieved    - The symbol that ^retrieved has as an attribute
 *  curr_memory  - Pointer to the memory currently in the ^retrieved link
 *
 */
typedef struct epmem_header_struct
{
    int index;
    Symbol *state;
    wme *ss_wme;
    Symbol *epmem;
    wme *epmem_wme;
    Symbol *query;
    wme *query_wme;
    Symbol *retrieved;
    wme *retrieved_wme;
    episodic_memory *curr_memory;
} epmem_header;


/*======================================================================
 * EpMem globals
 *----------------------------------------------------------------------
 */

/*

   g_current_active_wmes  - the n most active wmes in working memory
   g_previous_active_wmes - the n most active wmes in working memory
                            when the last memory was recorded
   g_wmetree              - The head of a giant wmetree used to represent all
                            the states that that agent has seen so far.  This is,
                            in effect, the episodic store of the agent.
   g_memories             - This is an array of all the memories that the
                            agent has.  Each memory is an index into g_wmetree
   g_last_tag             - The timetag of the last command on the output-link
   g_last_ret_id          - This value gets incremeted at each retrieval. It's
                            currently used by the matcher to keep track of the
                            best match for this retrieval in wmetree->last_usage
   g_num_queries          - Total number of queries that have been performed so far
   g_header_stack         - A stack of epmem_header structs used to mirror the
                            current state stack in WM and keep track of the
                            ^epmem link attached to each one.
   
*/

//%%%FIXME:  move these globals to current_agent()
wme **g_current_active_wmes;
wme **g_previous_active_wmes;
wmetree g_wmetree;
arraylist *g_memories;
unsigned long g_last_tag = 0;
long g_last_ret_id = 0;
long g_num_queries = 0;
arraylist *g_header_stack;


/* EpMem macros

   IS_LEAF_WME(w) - is w a leaf WME? (i.e., value is not an identifier)
   
*/
#define IS_LEAF_WME(w) ((w)->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)


/* ===================================================================
   compare_ptr

   Compares two void * pointers.  (Used for qsort() calls)
   
   Created: 06 Nov 2002
   =================================================================== */
int compare_ptr( const void *arg1, const void *arg2 )
{
    return *((long *)arg1) - *((long *)arg2);
}//compare_ptr

/* ===================================================================
   compare_actwme

   Compares two actwmes.   (Used for qsort() calls)
   
   Created:  22 April 2004
   Modified: 26 Aug 2004 to use wmetree depth  (Happy Birthday to me!)
   =================================================================== */
int compare_actwme( const void *arg1, const void *arg2 )
{
    wmetree *w1 = (*((actwme **)arg1))->node;
    wmetree *w2 = (*((actwme **)arg2))->node;

    if (w1->depth != w2->depth)
    {
        return w1->depth - w2->depth;
    }

    return ((int)w1) - ((int)w2);
}//compare_actwme

/* ===================================================================
   compare_actwme_by_act

   Compares two actwmes based on activation.   (Used for qsort() calls)
   This comparison results in a descending sort:  higher activation
   is "smaller".
   
   Created:  13 Dec 2004
   =================================================================== */
int compare_actwme_by_act( const void *arg1, const void *arg2 )
{
    actwme *aw1 = (*(actwme **)arg1);
    actwme *aw2 = (*(actwme **)arg2);

    return aw2->activation - aw1->activation;
}//compare_actwme_by_act


/* ===================================================================
   hash_wmetree

   Creates a hash value for a wmetree node

   Created: 22 Apr 2004
   =================================================================== */
unsigned long hash_wmetree(void *item, short num_bits)
{
    wmetree *node = (wmetree *)item;
    unsigned long hash_value;

    //Generate a hash value for the node's attr and value
    hash_value = hash_string(node->attr);
    switch(node->val_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            hash_value += hash_string(node->val.strval);
            break;
            
        case INT_CONSTANT_SYMBOL_TYPE:
            hash_value += node->val.intval;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            hash_value += (unsigned long)node->val.floatval;
            break;
    }//switch

    return compress(hash_value, num_bits);
}//hash_wmetree

/* ===================================================================
   hash_wme

   Creates a hash value for a WME.  This is used to find the
   corresponding wmetree node in a hash table.

   Created: 22 Apr 2004
   =================================================================== */
unsigned long hash_wme(wme *w, short num_bits)
{
    unsigned long hash_value;

    //Generate a hash value for the WME's attr and value
    hash_value = hash_string(w->attr->sc.name);
    switch(w->value->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            hash_value += hash_string(w->value->sc.name);
            break;
            
        case INT_CONSTANT_SYMBOL_TYPE:
            hash_value += w->value->ic.value;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            hash_value += (unsigned long)w->value->fc.value;
            break;
    }//switch

    return compress(hash_value, num_bits);
    
}//hash_wme

/* ===================================================================
   make_arraylist

   Allocates and initializes a new, empty arraylist and returns a pointer
   to the caller.

   init_cap = initial capacity
   
   Created: 13 Jan 2004
   =================================================================== */
arraylist *make_arraylist(agent *thisAgent, int init_cap)
{
    arraylist *al;
    int i;

    if (init_cap <= 0) init_cap = 32; // default value
    
    al = (arraylist *)allocate_memory(thisAgent, sizeof(arraylist),
                                      MISCELLANEOUS_MEM_USAGE);
    al->array = (void **)allocate_memory(thisAgent, sizeof(void*) * init_cap,
                                         MISCELLANEOUS_MEM_USAGE);
    al->capacity = init_cap;
    al->size = 0;
    al->next = NULL;

    for(i = 0; i < init_cap; i++)
    {
        al->array[i] = NULL;
    }

    return al;
}//make_arraylist

/* ===================================================================
   destroy_arraylist

   Deallocates a given arraylist.
   
   Created: 19 Jan 2004
   =================================================================== */
void destroy_arraylist(agent *thisAgent, arraylist *al)
{
    if (al == NULL) return;
    
    if ( (al->capacity > 0) && (al->array != NULL) )
    {
        free_memory(thisAgent, al->array, MISCELLANEOUS_MEM_USAGE);
    }
    free_memory(thisAgent, al, MISCELLANEOUS_MEM_USAGE);
}//destroy_arraylist

/* ===================================================================
   grow_arraylist

   This funciton increases the length of an arraylist to a minimum
   of a given capacity.
   
   Created: 06 Oct 2004
   =================================================================== */
void grow_arraylist(agent *thisAgent, arraylist *al, int desired_capacity)
{
    int i;
    void **new_array;
    int new_capacity;

    //Check to see if capacity is already correct
    if (desired_capacity <= al->capacity) return;

    //Determine the new capacity
    new_capacity = al->capacity;
    if (new_capacity == 0) new_capacity = 32;
    while (new_capacity < desired_capacity)
    {
        new_capacity *= 2;
    }
    
    //Grow the array (can't use realloc b/c of Soar's memory routines)
    new_array = (void **)allocate_memory_and_zerofill(thisAgent,
                                                      new_capacity * sizeof(void*),
                                                      MISCELLANEOUS_MEM_USAGE);
    for(i = 0; i < al->size; i++)
    {
        new_array[i] = al->array[i];
    }

    if (al->array != NULL)
    {
        free_memory(thisAgent, al->array, MISCELLANEOUS_MEM_USAGE);
    }

    al->array = new_array;
    al->capacity = new_capacity;

}//grow_arraylist


/* ===================================================================
   append_entry_to_arraylist

   This function adds a new entry to the end of an arraylist
   
   Created: 22 Apr 2004
   =================================================================== */
void append_entry_to_arraylist(agent *thisAgent, arraylist *al, void *new_entry)
{
    if (al->size == al->capacity)
    {
        grow_arraylist(thisAgent, al, al->size+1);
    }//if


    //Add the node to the array
    al->array[al->size] = new_entry;
    al->size++;
    
}//append_entry_to_arraylist

/* ===================================================================
   remove_entry_from_arraylist

   Given an index, this function removes the entry at that index from
   an arraylist and moves down subsequent entries to fill in the gap.
   The caller is responsible for cleaning up the entry itself.
   
   Created: 04 Oct 2004
   =================================================================== */
void *remove_entry_from_arraylist(arraylist *al, int index)
{
    int i;
    void *retval = NULL;
    
    //Catch erroneous input values
    if (al == NULL) return NULL;
    if (index >= al->size) return NULL;
    if (index < 0) return NULL;

    retval = al->array[index];
    al->array[index] = NULL;
    for(i = index + 1; i < al->size; i++)
    {
        al->array[i - 1] = al->array[i];
    }
    
    (al->size)--;

    return retval;
    
}//remove_entry_from_arraylist

/* ===================================================================
   get_arraylist_entry

   Given an index, this function returns the entry in the arraylist
   at that point.
   
   Created: 06 Oct 2004
   =================================================================== */
void *get_arraylist_entry(agent *thisAgent, arraylist *al, int index)
{
    //Catch erroneous input values
    if (al == NULL) return NULL;
    if (index < 0) return NULL;
    if (index >= al->size)
    {
        grow_arraylist(thisAgent, al, index + 1);
    }

    return al->array[index];
}//get_arraylist_entry

/* ===================================================================
   set_arraylist_entry

   Given an index, this function sets the entry in the arraylist
   at that index to a given value.
   
   Created: 06 Oct 2004
   =================================================================== */
void set_arraylist_entry(agent *thisAgent, arraylist *al, int index, void *newval)
{
    //Catch erroneous input values
    if (al == NULL) return;
    if (index < 0) return;
    if (index >= al->size)
    {
        grow_arraylist(thisAgent, al, index + 1);
    }

    al->array[index] = newval;
}//set_arraylist_entry


/* ===================================================================
   make_wmetree_node

   Creates a new wmetree node based upon a given wme.  If no WME is
   given (w == NULL) then an empty node is allocated.  The caller is
   responsible for setting the parent pointer.
   
   Created: 09 Jan 2004
   =================================================================== */
wmetree *make_wmetree_node(agent *thisAgent, wme *w)
{
    wmetree *node;

    node = (wmetree *)allocate_memory(thisAgent, sizeof(wmetree), MISCELLANEOUS_MEM_USAGE);
    node->next = NULL;
    node->attr = NULL;
    node->val.intval = 0;
    node->val_type = IDENTIFIER_SYMBOL_TYPE;
    node->children = make_hash_table(thisAgent, 0, hash_wmetree);
    node->parent = NULL;
    node->depth = -1;
    node->assoc_wmes = make_arraylist(thisAgent, 20);
    node->assoc_memories = make_arraylist(thisAgent, 16);
    node->query_count = 0;
    node->ubiquitous = FALSE;

    if (w == NULL) return node;
    
    node->attr = (char *)allocate_memory(thisAgent, sizeof(char)*strlen(w->attr->sc.name) + 1,
                                MISCELLANEOUS_MEM_USAGE);
    strcpy(node->attr, w->attr->sc.name);
    
    switch(w->value->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            node->val_type = SYM_CONSTANT_SYMBOL_TYPE;
            node->val.strval =
                (char *)allocate_memory(thisAgent, sizeof(char)*strlen(w->value->sc.name) + 1,
                                MISCELLANEOUS_MEM_USAGE);
            strcpy(node->val.strval, w->value->sc.name);
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            node->val_type = INT_CONSTANT_SYMBOL_TYPE;
            node->val.intval = w->value->ic.value;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            node->val_type = FLOAT_CONSTANT_SYMBOL_TYPE;
            node->val.floatval = w->value->fc.value;
            break;

        default:
            node->val_type = IDENTIFIER_SYMBOL_TYPE;
            node->val.intval = 0;
            break;
    }

    return node;
}//make_wmetree_node

/* ===================================================================
   destroy_wmetree (+ dw_helper)       *RECURSIVE*

   Deallocate an entire wmetree.

   Created 10 Nov 2002
   =================================================================== */
void dw_helper(agent *thisAgent, wmetree *tree)
{
    unsigned long hash_value;
    wmetree *child;

    if (tree->attr != NULL)
    {
        free_memory(thisAgent, tree->attr, MISCELLANEOUS_MEM_USAGE);
    }
    
    if (tree->val_type == SYM_CONSTANT_SYMBOL_TYPE)
    {
        free_memory(thisAgent, tree->val.strval, MISCELLANEOUS_MEM_USAGE);
    }
    
    if (tree->children->count == 0)
    {
        return;
    }


    //Recursively destroy all the children before the parent
    for (hash_value = 0; hash_value < tree->children->size; hash_value++)
    {
        child = (wmetree *) (*(tree->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            dw_helper(thisAgent, child);
            free_memory(thisAgent, child, MISCELLANEOUS_MEM_USAGE);
        }
    }

    free_memory(thisAgent, tree->children, HASH_TABLE_MEM_USAGE);
}//dw_helper

void destroy_wmetree(agent *thisAgent, wmetree *tree)
{
    dw_helper(thisAgent, tree);

    free_memory(thisAgent, tree, MISCELLANEOUS_MEM_USAGE);
}//destroy_wmetree

/* ===================================================================
   wme_has_value

   This routine returns TRUE is the given WMEs attribute and value are
   both symbols and have then names given.  If either of the given
   names are NULL then they are assumed to be a match (i.e., a
   wildcard).

   Created: 14 Dec 2002
   =================================================================== */
int wme_has_value(wme *w, char *attr_name, char *value_name)
{
    if (w == NULL) return FALSE;
    
    if (attr_name != NULL)
    {
        if (w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
        {
            return FALSE;
        }

        if (strcmp(w->attr->sc.name, attr_name) != 0)
        {
            return FALSE;
        }
    }

    if (value_name != NULL)
    {
        if (w->value->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
        {
            return FALSE;
        }

        if (strcmp(w->value->sc.name, value_name) != 0)
        {
            return FALSE;
        }
    }

    return TRUE;

}//wme_has_value

/* ===================================================================
   get_aug_of_id()

   This routine examines a symbol for an augmentation that as the
   given attribute and value and returns it.  See wme_has_value()
   for info on how the correct wme is matched to the given strings.
   
   Created: 19 Oct 2004
   =================================================================== */
wme *get_aug_of_id(agent *thisAgent, Symbol *sym, char *attr_name, char *value_name)
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    wme *ret_wme = NULL;
    
    tc = sym->id.tc_num + 1;
    wmes = get_augs_of_id(thisAgent, sym, tc, &len);
    sym->id.tc_num = tc - 1;
    if (wmes == NULL) return NULL;

    for(i = 0; i < len; i++)
    {
        if (wme_has_value(wmes[i], attr_name, value_name))
        {
            ret_wme = wmes[i];
            break;
        }
    }

    free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
    return ret_wme;
}//get_aug_of_id

/* ===================================================================
   retrieve_all_child_wmes

   Generates an arraylist of wme* that points to all the WMEs that
   descend from a given symbol

   sym   - the parent symbol
   syms  - an arraylist to fill with wme* pointers that descend from the
           parent.
   tc    - a transitive closure number for avoiding loops in working mem

   CAVEAT:  This function is computationally expensive.
   
   Created: 06 Dec 2005
   =================================================================== */
void retrieve_all_child_wmes(agent *thisAgent,
                             Symbol *sym, 
                             arraylist *syms,
                             tc_number tc)
{
    wme **wmes = NULL;
    int len = 0;
    int i;
    Symbol *ss = NULL;
    int pos = -1;
    wme *w;

    do
    {
        pos++;
        
        start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
        wmes = get_augs_of_id(thisAgent,  sym, tc, &len );
        stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

        if (wmes != NULL)
        {
            for(i = 0; i < len; i++)
            {
                //insert childnode into the arraylist
                append_entry_to_arraylist(thisAgent, syms, (void *)wmes[i]);
            }//for
            
            free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
        }//if

        //Find the next WME in the list that has an identifier as
        //a value (and thus may have child WMEs)
        while (pos < syms->size)
        {
            w = (wme *)get_arraylist_entry(thisAgent, syms, pos);
            sym = w->value;
            if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
            {
                break;
            }
            
            pos++;
        }//while
          
    }while(pos < syms->size);

    
}//retrieve_all_child_wmes


/* ===================================================================
   make_fake_preference_for_epmem_wme

   This function adds a fake preference to a WME so that it will not
   be added to the goal dependency set of the state it is attached
   to.  Currently I only need to do this for the three epmem header
   WMEs.

   (The bulk of the content of this function is taken from
    make_fake_preference_for_goal_item() in decide.c)
   
   
   Created: 18 Oct 2004
   Changed: 24 Jan 2005 (to improve performance the ^superstate wme is now
                         cached in the epmem_header)
   Changed 12 Jan 2006 (added support for chunking by using the cue WMEs
                        as the conditions of the instantiation.  In
                        a sense, this "fake preference" isn't realy fake
                        anymore.
   =================================================================== */
preference *make_fake_preference_for_epmem_wme(agent *thisAgent, epmem_header *h, Symbol *goal, wme *w)
{
    instantiation *inst;
    preference *pref;
    condition *cond;
    condition *prev_cond = NULL;
    int i;

    //%%%DEBUGGING
    print_with_symbols(thisAgent, "\nBuilding fake preference for WME: (%y ^%y %y)",
                       w->id, w->attr, w->value);
    
    /*
     * make the fake preference
     */
    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, w->id, w->attr, w->value, NIL);
    pref->o_supported = TRUE;
    symbol_add_ref(pref->id);
    symbol_add_ref(pref->attr);
    symbol_add_ref(pref->value);

    //This may not be necessary??
    insert_at_head_of_dll(goal->id.preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
    pref->on_goal_list = TRUE;

    preference_add_ref(pref);

        
    /*
     * make the fake instantiation
     */
    allocate_with_pool(thisAgent, &(thisAgent->instantiation_pool), &inst);
    pref->inst = inst;
    pref->inst_next = pref->inst_prev = NIL;
    inst->preferences_generated = pref;
    inst->prod = NIL;
    inst->next = inst->prev = NIL;
    inst->rete_token = NIL;
    inst->rete_wme = NIL;
    inst->match_goal = goal;
    inst->match_goal_level = goal->id.level;
    inst->okay_to_variablize = TRUE;
    inst->backtrace_number = 0;
    inst->in_ms = FALSE;

    /*
     * make the fake condition for all cue elements
     *
     * NOTE: This is effectively an instantiation whose condition is
     *       the cue WMEs and whose action is the retrieved epmem WME
     */
    //Retrieve all the cue WMEs
    arraylist *query = make_arraylist(thisAgent, 32);
    retrieve_all_child_wmes(thisAgent, h->query, query, h->query->id.tc_num + 1);

    //If there is no cue than just use ^superstate instead.  
    //(This should only happen with header WMEs).
    if (query->size ==0)
    {
        append_entry_to_arraylist(thisAgent, query, (void *)h->ss_wme);
    }
    
    //Create a condition for each cue WME
    for(i = 0; i < query->size; i++)
    {
        wme *cue_wme = (wme *)get_arraylist_entry(thisAgent, query, i);

        //%%%DEBUGGING
        print(thisAgent, "\n Condition #%d: ", i);
        print_with_symbols(thisAgent, "(%y %y %y)",
                           cue_wme->id, cue_wme->attr, cue_wme->value);
        
        //Construct the condition
        allocate_with_pool(thisAgent, &(thisAgent->condition_pool), &cond);
        cond->type = POSITIVE_CONDITION;
        cond->prev = prev_cond;
        cond->next = NULL;
        if (prev_cond != NULL)
        {
            prev_cond->next = cond;
        }
        else
        {
            inst->top_of_instantiated_conditions = cond;
            inst->bottom_of_instantiated_conditions = cond;
            inst->nots = NIL;
        }
        cond->data.tests.id_test = make_equality_test(cue_wme->id);
        cond->data.tests.attr_test = make_equality_test(cue_wme->attr);
        cond->data.tests.value_test = make_equality_test(cue_wme->value);
        cond->test_for_acceptable_preference = TRUE;
        cond->bt.wme_= cue_wme;
        wme_add_ref(cue_wme);  //%%%Causes mem leak?
        cond->bt.level = cue_wme->id->id.level;
        cond->bt.trace = NIL;
        cond->bt.prohibits = NIL;

        prev_cond = cond;

    }//for

    destroy_arraylist(thisAgent, query);
    
    /* --- return the fake preference --- */
    return pref;
}//make_fake_preference_for_epmem_wme

/* ===================================================================
   remove_fake_preference_for_epmem_wme

   This function removes a fake preference on a WME created by
   make_fake_preference_for_epmem_wme()
   
   Created: 21 Oct 2004
   =================================================================== */
void remove_fake_preference_for_epmem_wme(agent *thisAgent, wme *w)
{

//%%%    preference_remove_ref(thisAgent, w->preference);
    

}//remove_fake_preference_for_epmem_wme


/* ===================================================================
   make_epmem_header

   Allocates a new epmem_header struct for a given state and fills in
   the proper values.  The caller is responsible for setting the
   index field.

   s - the state that the header is to be attached to
   
   Created: 04 Oct 2004
   =================================================================== */
epmem_header *make_epmem_header(agent *thisAgent, Symbol *s)
{
    epmem_header *h;

    h = (epmem_header *)allocate_memory(thisAgent, sizeof(epmem_header), MISCELLANEOUS_MEM_USAGE);
    h->index = -42; //A bogus value to help with debugging
    h->state = s;
    h->curr_memory = NULL;

    //Find the superstate wme
    h->ss_wme = get_aug_of_id(thisAgent, s, "superstate", NULL);
    if (!h->ss_wme)
    {
        //This should never happen.
        print_with_symbols(thisAgent, "\nepmem.c: Internal error: couldn't find ^superstate WME on state %y\n", s);
        return NULL;
    }
    
    //Create the ^epmem header symbols
    h->epmem = make_new_identifier(thisAgent, 'E', s->id.level);
    h->query = make_new_identifier(thisAgent, 'E', s->id.level);
    h->retrieved = make_new_identifier(thisAgent, 'E', s->id.level);

    //Add the ^epmem header WMEs
    h->epmem_wme = add_input_wme(thisAgent, s, make_sym_constant(thisAgent, "epmem"), h->epmem);
    h->epmem_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->epmem_wme);
    wme_add_ref(h->epmem_wme);

    h->query_wme = add_input_wme(thisAgent, h->epmem, make_sym_constant(thisAgent, "query"), h->query);
    h->query_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->query_wme);
    wme_add_ref(h->query_wme);

    h->retrieved_wme = add_input_wme(thisAgent, h->epmem, make_sym_constant(thisAgent, "retrieved"), h->retrieved);
    h->retrieved_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->retrieved_wme);
    wme_add_ref(h->retrieved_wme);

    return h;
}//make_epmem_header


//Declare this in advance so that destroy_epmem_header() can call it.
void epmem_clear_curr_mem(agent *thisAgent, epmem_header *h);

/* ===================================================================
   destroy_epmem_header

   Frees the resources used by an epmem_header struct.

   Created: 04 Oct 2004
   =================================================================== */
void destroy_epmem_header(agent *thisAgent, epmem_header *h)
{
    //Remove any active memory (or a "no-retrieval" WME) from WM
    epmem_clear_curr_mem(thisAgent, h);

    //Remove the ^epmem header WMEs
    remove_input_wme(thisAgent, h->epmem_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->epmem_wme);
    wme_remove_ref(thisAgent, h->epmem_wme);

    remove_input_wme(thisAgent, h->query_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->query_wme);
    wme_remove_ref(thisAgent, h->query_wme);

    remove_input_wme(thisAgent, h->retrieved_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->retrieved_wme);
    wme_remove_ref(thisAgent, h->retrieved_wme);
    

    //Dereference the ^epmem header symbols
    symbol_remove_ref(thisAgent, h->epmem);
    symbol_remove_ref(thisAgent, h->query);
    symbol_remove_ref(thisAgent, h->retrieved);

    //Free the struct
    free_memory(thisAgent, h, MISCELLANEOUS_MEM_USAGE);
    
}//destroy_epmem_header

/* ===================================================================
   symbols_are_equal_value

   This compares two symbols.  If they have the same type and value
   it returns TRUE, otherwise FALSE.  Identifiers and variables
   yield a response of TRUE.

   Created: 12 Dec 2002
   =================================================================== */
int symbols_are_equal_value(Symbol *a, Symbol *b)
{
    if (a->common.symbol_type != b->common.symbol_type)
    {
        return FALSE;
    }

    switch(a->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            return ! (strcmp(a->sc.name, b->sc.name));
        case INT_CONSTANT_SYMBOL_TYPE:
            return a->ic.value == b->ic.value;
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return a->fc.value == b->fc.value;
        case VARIABLE_SYMBOL_TYPE:
        case IDENTIFIER_SYMBOL_TYPE:
            return TRUE;
        default:
            return FALSE;
    }

}//symbols_are_equal_value

/* ===================================================================
   wmes_are_equal_value

   This compares two wmes.  If they have the same type and value
   it returns true.

   Created: 12 Dec 2002
   =================================================================== */
int wmes_are_equal_value(wme *a, wme *b)
{
    return (symbols_are_equal_value(a->attr, b->attr))
        && (symbols_are_equal_value(a->value, b->value));

}//wmes_are_equal_value

/* ===================================================================
   wme_equals_node

   This routine returns TRUE is the given WME's attribute and value are
   both symbols and match the values in the given wmetree node.  If
   the value in the wmetree node is of type identifier/variable
   then it always matches.

   Created: 12 Jan 2004
   =================================================================== */
int wme_equals_node(wme *w, wmetree *node)
{
    if (w == NULL) return FALSE;

    if (w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
    {
        return FALSE;
    }

    //Compare attribute
    if (strcmp(w->attr->sc.name, node->attr) != 0)
    {
        return FALSE;
    }

    //Compare value
    switch(node->val_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            return strcmp(w->value->sc.name, node->val.strval) == 0;

        case INT_CONSTANT_SYMBOL_TYPE:
            return w->value->ic.value == node->val.intval;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return w->value->fc.value == node->val.floatval;
            break;

        default:
            return TRUE;
    }

}//wme_equals_node

/* ===================================================================
   print_wmetree

   Prints an ASCII graphic representation of the wmetree rooted by the
   given node.

   node - wmetree to print
   indent - how many spaces to indent
   depth - how many levels to descend into the tree.  A depth of zero
           will just print the node.  
   
   Created: 09 Nov 2002
   =================================================================== */
void print_wmetree(agent *thisAgent, wmetree *node, int indent, int depth)
{
    unsigned long hash_value;
    wmetree *child;
    
    if (node == NULL) return;

    if (node->parent == NULL) // check for root
    {
        print(thisAgent, "\n\nROOT\n");
    }
    else
    {
        if (indent)
        {
            print(thisAgent, "%*s+--", indent, "");
        }
        print(thisAgent, "%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %f", node->val.floatval);
                break;
            default:
                break;
        }//switch
    }//else
    print(thisAgent, "\n");

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_wmetree(thisAgent, child, indent + 3, depth - 1);
            }
        }
    }
}//print_wmetree

/* ===================================================================
   epmem_find_wmetree_entry

   Finds a descendent entry that has a particular id and attribute in a
   given memory.  If the given parent is &g_wmetree then it is assumed
   to be a wildcard match.

   Returns NULL if not found.

   Created: 20 Feb 2004
   =================================================================== */
wmetree *epmem_find_wmetree_entry(agent *thisAgent, arraylist *epmem, wmetree *id, char *s)
{
    int i;

    if (epmem == NULL) return NULL;
    
    for(i = 0; i < epmem->size; i++)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, epmem, i))->node;
        if ( (id == &g_wmetree) || (id == node->parent) )
        { 
            if (strcmp(node->attr, s) == 0)
            {
                return node;
            }
        }
    }//for

    return NULL;
    
}//epmem_find_wmetree_entry

/* ===================================================================
   epmem_find_actwme_entry

   Finds an actwme entry in a given episodic memory that points to a
   given wmetree.

   Returns NULL if not found.

   Created: 29 Nov 2004
   =================================================================== */
actwme *epmem_find_actwme_entry(agent *thisAgent, arraylist *epmem, wmetree *target)
{
    int i;

    if (epmem == NULL) return NULL;
    
    for(i = 0; i < epmem->size; i++)
    {
        actwme *entry = (actwme *)get_arraylist_entry(thisAgent, epmem, i);
        if (entry->node == target) return entry;
    }//for

    return NULL;
    
}//epmem_find_actwme_entry

/* ===================================================================
   print_memory

   thisAgent - duh
   epmem - an arraylist containing the epmem
   node - the wmetree this memory is drawn from (probably &g_wmetree)
   indent - number of space to indent
   depth - how deep to traverse the tree
   attrs - an optional string containing a list of attributes that
           should be printed
   
   Created: 01 Mar 2004
   =================================================================== */
void print_memory(agent *thisAgent, arraylist *epmem,
                  wmetree *node, int indent, int depth, char *attrs = NULL)
{
    int i;
    unsigned long hash_value;
    wmetree *child;
    actwme *aw;

    if (epmem == NULL) return;
    if (node == NULL) return;
    
    if (node->parent == NULL) // check for root
    {
        print(thisAgent, "\n\nROOT\n");
    }
    else
    {
        int bFound = FALSE;
        
        //Find out if this node is in the arraylist
        for(i = 0; i < epmem->size; i++)
        {
            aw = (actwme *)get_arraylist_entry(thisAgent, epmem, i);
            if (aw->node == node)
            {
                bFound = TRUE;
                break;
            }
        }
        
        if (!bFound) return;

        //Check to make sure I have an attr I'm allowed to print
        if ( (attrs != NULL) && (!strstr(attrs, node->attr)) ) return;
        
        if (indent)
        {
            print(thisAgent, "%*s+--", indent, "");
        }
        print(thisAgent, "%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %f", node->val.floatval);
                break;
            default:
                break;
        }//switch

        //print the activation level
        print(thisAgent, "(%d)", aw->activation);
    }//else
    print(thisAgent, "\n");

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_memory(thisAgent, epmem, child, indent + 3, depth - 1, attrs);
            }
        }
    }
    
}//print_memory

/* ===================================================================
   print_memory_graphically         *EATERS DOMAIN DEPENDENT*

   
   Created: 01 Mar 2004
   =================================================================== */
void print_memory_graphically(agent *thisAgent, arraylist *epmem)
{
    wmetree *ol            = NULL;
    wmetree *il            = NULL;
    wmetree *move          = NULL;
    wmetree *direction     = NULL;
    wmetree *eater         = NULL;
    wmetree *score         = NULL;
    wmetree *my_location   = NULL;
    wmetree *north         = NULL;
    wmetree *south         = NULL;
    wmetree *east          = NULL;
    wmetree *west          = NULL;
    wmetree *n_content     = NULL;
    wmetree *s_content     = NULL;
    wmetree *e_content     = NULL;
    wmetree *w_content     = NULL;
    char n_char = '?';
    char s_char = '?';
    char e_char = '?';
    char w_char = '?';
    char dir_char = '?';

    //Find the direction of movement
    ol = epmem_find_wmetree_entry(thisAgent, epmem, &g_wmetree, "output-link");
    move = epmem_find_wmetree_entry(thisAgent, epmem, ol, "move");
    direction = epmem_find_wmetree_entry(thisAgent, epmem, move, "direction");

    //Find the current score
    il = epmem_find_wmetree_entry(thisAgent, epmem, &g_wmetree, "input-link");
    eater = epmem_find_wmetree_entry(thisAgent, epmem, il, "eater");
    score = epmem_find_wmetree_entry(thisAgent, epmem, eater, "score");
    
    //Find the contents of each surrounding cell
    my_location = epmem_find_wmetree_entry(thisAgent, epmem, il, "my-location");
    north = epmem_find_wmetree_entry(thisAgent, epmem, my_location, "north");
    south = epmem_find_wmetree_entry(thisAgent, epmem, my_location, "south");
    east = epmem_find_wmetree_entry(thisAgent, epmem, my_location, "east");
    west = epmem_find_wmetree_entry(thisAgent, epmem, my_location, "west");
    n_content = epmem_find_wmetree_entry(thisAgent, epmem, north, "content");
    s_content = epmem_find_wmetree_entry(thisAgent, epmem, south, "content");
    e_content = epmem_find_wmetree_entry(thisAgent, epmem, east, "content");
    w_content = epmem_find_wmetree_entry(thisAgent, epmem, west, "content");

    if (n_content != NULL) n_char   = n_content->val.strval[0];
    if (s_content != NULL) s_char   = s_content->val.strval[0];
    if (e_content != NULL) e_char   = e_content->val.strval[0];
    if (w_content != NULL) w_char   = w_content->val.strval[0];
    if (direction != NULL) dir_char = direction->val.strval[0];

    print(thisAgent, "\n         %c", n_char);
    print(thisAgent, "\n        %c%c%c",w_char, dir_char, e_char);
    print(thisAgent, "\n         %c", s_char);

    if (score != NULL)
    {
        print(thisAgent, "\n  score=%d.", score->val.intval);
    }
    else
    {
        print(thisAgent, "\n  score=NOT AVAILABLE");
    }
    
}//print_memory_graphically


/* ===================================================================
   find_child_node

   Given a wmetree node and a wme, this function returns the child
   node that represents that WME (or NULL).
   
   Created: 12 Jan 2004
   =================================================================== */
wmetree *find_child_node(wmetree *node, wme *w)
{
    unsigned long hash_value;
    wmetree *child;

    hash_value = hash_wme(w, node->children->log2size);
    child = (wmetree *) (*(node->children->buckets + hash_value));
    for (; child != NIL; child = child->next)
    {
        if (wme_equals_node(w, child))
        {
            return child;
        }
    }
    
    return NULL;
    
}//find_child_node



/* ===================================================================
   add_node_to_memory

   This function adds a wmetree node to a given episodic memory.
   
   Created: 12 Jan 2004
   =================================================================== */
void add_node_to_memory(agent *thisAgent, arraylist *epmem, wmetree *node, int activation)
{
    //Allocate and init a new actwme
    actwme *aw = (actwme *)allocate_memory(thisAgent, sizeof(actwme),
                                           MISCELLANEOUS_MEM_USAGE);
    aw->node = node;
    aw->activation = activation;
    
    append_entry_to_arraylist(thisAgent, epmem, (void *)aw);
    
}//add_node_to_memory


/* ===================================================================
   update_wmetree

   Updates the wmetree given a pointer to a corresponding wme in working
   memory.  The wmetree node is assumed to be initialized and empty.
   Each wme that is discovered by this algorithm is also added to a given
   episodic memory.

   If this function finds a ^superstate WME it does not traverse that link.
   Instead, it records the find and returns it to the caller.  The caller
   can then call update_wmetree again if desired.

   node - the root of the WME tree to be updated
   sym - the root of the tree in working memory to update it with
   epmem - this function generates an arraylist of actwme structs
           representing all the nodes in the wmetree that are
           referenced by the working memory tree rooted by sym.
   tc - a transitive closure number for avoiding loops in working mem

   Created: 09 Jan 2004
   Updated: 23 Feb 2004 - made breadth first and non-recursive
   =================================================================== */
Symbol *update_wmetree(agent *thisAgent,
                       wmetree *node,
                       Symbol *sym,
                       arraylist *epmem,
                       tc_number tc)
{
    wme **wmes = NULL;
    wmetree *childnode;
    int len = 0;
    int i;
    Symbol *ss = NULL;
    arraylist *syms = make_arraylist(thisAgent, 32);
    int pos = 0;

    start_timer(thisAgent, &(thisAgent->epmem_updatewmetree_start_time));

    
    while(pos <= epmem->size)
    {
        start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
        wmes = get_augs_of_id(thisAgent,  sym, tc, &len );
        stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

        if (wmes != NULL)
        {
            for(i = 0; i < len; i++)
            {
                start_timer(thisAgent, &(thisAgent->epmem_findchild_start_time));
                childnode = find_child_node(node, wmes[i]);
                stop_timer(thisAgent, &(thisAgent->epmem_findchild_start_time), &(thisAgent->epmem_findchild_total_time));

                if (childnode == NULL)
                {
                    childnode = make_wmetree_node(thisAgent, wmes[i]);
                    childnode->parent = node;
                    childnode->depth = node->depth + 1;
                    add_to_hash_table(thisAgent, node->children, childnode);
                }

                //Check for special case: "superstate" 
                if (wme_has_value(wmes[i], "superstate", NULL))
                {
                    if ( (ss == NULL)
                         && (wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) )
                    {
                        ss = wmes[i]->value;
                    }
                   continue;
                }

                //insert childnode into the arraylist
                start_timer(thisAgent, &(thisAgent->epmem_addnode_start_time));
                add_node_to_memory(thisAgent, epmem, childnode, decay_activation_level(thisAgent, wmes[i]));
                stop_timer(thisAgent, &(thisAgent->epmem_addnode_start_time), &(thisAgent->epmem_addnode_total_time));
                append_entry_to_arraylist(thisAgent, syms, (void *)wmes[i]->value);

            }//for
        }//if

        //Special Case:  no wmes found attached to the given symbol
        if (epmem->size == 0) break;

        //We've retrieved every WME in the query
        if (epmem->size == pos) break;
        
        node = ((actwme *)get_arraylist_entry(thisAgent, epmem,pos))->node;
        sym = (Symbol *)get_arraylist_entry(thisAgent, syms,pos);
        pos++;

        //Deallocate the last wmes list
        if (wmes != NULL)
        {
            free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
        }
        
    }//while
    
    //Sort the memory's arraylist using the node pointers
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme );

    //Deallocate the symbol list
    destroy_arraylist(thisAgent, syms);

    stop_timer(thisAgent, &(thisAgent->epmem_updatewmetree_start_time), &(thisAgent->epmem_updatewmetree_total_time));
    
    return ss;
    
}//update_wmetree

/* ===================================================================
   trim_epmem

   Removes entries from an episodic memory that have activation
   lower than the given value.

   NOTE:  This routine could be a lot more efficient.

   Created: 13 Dec 2004
   =================================================================== */
arraylist *trim_epmem(arraylist *epmem)
{
    if (epmem == NULL) return NULL;

    //Sort the memory's arraylist using activation
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme_by_act );

    epmem->size = (int)((double)(epmem->size) * (1.0 - fraction_to_trim));

    //Sort the memory's arraylist using the node pointers
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme );

    return epmem;
    
}//trim_epmem


/* ===================================================================
   record_epmem

   Once it has been determined that an epmem needs to be recorded,
   this routine manages all the steps for recording it.

   Created: 12 Jan 2004
   =================================================================== */
void record_epmem(agent *thisAgent)
{
    tc_number tc;
    Symbol *sym;
    arraylist *curr_state;
    arraylist *next_state;
    int i;
    episodic_memory *new_epmem;

    //Allocate and initialize the new memory
    new_epmem = (episodic_memory *)allocate_memory(thisAgent, sizeof(episodic_memory),
                                                   MISCELLANEOUS_MEM_USAGE);
    new_epmem->last_usage = -1;
    new_epmem->match_score = 0;

    //Starting with bottom_goal and moving toward top_goal, add all
    //the current states to the wmetree and record the full WM
    //state as an arraylist of actwmes
    sym = (thisAgent->bottom_goal);
    
    //Do only top-state for now
    sym = (thisAgent->top_goal);  //%%%TODO: remove this later
    
    curr_state = NULL;
    next_state = NULL;
    while(sym != NULL)
    {
        next_state = make_arraylist(thisAgent, 128);
        next_state->next = curr_state;
        curr_state = next_state;

        tc = sym->id.tc_num + 1;
        sym = update_wmetree(thisAgent, &g_wmetree, sym, curr_state, tc);

        //Update the assoc_memories link on each wmetree node in curr_state
        for(i = 0; i < curr_state->size; i++)
        {
            actwme *curr_actwme = (actwme *)get_arraylist_entry(thisAgent, curr_state,i);
            wmetree *node = curr_actwme->node;
            int activation = curr_actwme->activation;

            //In order to be recorded, a WME must meet the following criteria:
            //1.  It must be a leaf WME (i.e., it has no children)
            //2.  It must be activated (i.e., it has a decay element)
            //3.  It must not be marked as ubiquitous
            if ( (node->children->count == 0)
                 && (activation != -1)
                 && (! node->ubiquitous) )
            {
                append_entry_to_arraylist(thisAgent, node->assoc_memories, (void *)new_epmem);
                
                //Test to see if the new arraylist has too many entries.
                //If so, this node has become too ubiquitous and will no
                //longer be used in mat ching
                if (g_memories->size > ubiquitous_max)
                {
                    float ubiquity =
                        ((float)node->assoc_memories->size) / ((float)g_memories->size);
                    if (ubiquity > ubiquitous_threshold)
                    {
                        node->ubiquitous = TRUE;
                        destroy_arraylist(thisAgent, node->assoc_memories);
                        node->assoc_memories = make_arraylist(thisAgent, 1);
                    }
                    
                }                
            }
        }//for
    }//while

//      //Reduce the size of the episodic memory
//      new_epmem->content = trim_epmem(curr_state);
    new_epmem->content = curr_state;

    //Store the recorded memory
    new_epmem->index = g_memories->size;
    append_entry_to_arraylist(thisAgent, g_memories, (void *)new_epmem);

//      //%%%DEBUGGING
//      print(thisAgent, "\nRECORDED MEMORY %d:\n", g_memories->size - 1);
//      print_memory_graphically(((episodic_memory *)get_arraylist_entry(thisAgent, g_memories,g_memories->size - 1))->content);
    
}//record_epmem


/* ===================================================================
   wme_array_diff()

   Given two arrays of wme* this routine determines how many are different.
   Both arrays must be sorted order.
   
   Created: 08 Nov 2002
   =================================================================== */
int wme_array_diff(wme **arr1, wme** arr2, int len)
{
    int count = len;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < len) && (pos2 < len))
    {
        if (arr1[pos1] == arr2[pos2])
        {
            count--;
            pos1++;
            pos2++;
        }
        else if (arr1[pos1] < arr2[pos2])
        {
            pos1++;
        }
        else
        {
            pos2++;
        }
    }//while

    return count;

}//wme_array_diff

/* ===================================================================
   wme_to_skip

   This is a helper function for get_most_activated_wmes.  Certain
   WMEs from the environment update every cycle (e.g., ^random) and
   thus gain high activation every cycle.  However, since the value is
   different and virtually unique every time they cause recall
   proposal rules to be created that will never fire.  To prevent
   this, WMEs with certain hard-coded names are ignored when
   retrieving the top n most activated wmes.  I realize this isn't
   a very psychologically plausible fix for this problem but it
   will do in the near term.

   Currently Ignoring these Tanksoar WMEs:
     cycle
     random
     clock
     x
     y
   And these Eaters WMEs:
     ^name eaters
     cycle
     score
     eater-score
     x
     y
   And these WMEs which are part of the performance data recorded by
   the eaters agent and not part of the agent proper.
     prev-score
     reward
     score
     x
     y
     move-count
     could-reflect
    

   %%%TODO:  Remove the need for this function.
     
   Created: 12 Dec 2002
   =================================================================== */
int wme_to_skip(wme *w)
{
    char *s;
    
    if (w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
    {
        return FALSE;
    }

    s = w->attr->sc.name;
    switch(s[0])
    {
        case 'c':
            switch(s[1])
            {
                case 'y': return strcmp(s, "cycle") == 0;
                case 'l': return strcmp(s, "clock") == 0;
                case 'o': return strcmp(s, "could-reflect") == 0;
                default:  return FALSE;
            }
        case 'e':
            return strcmp(s, "eater-score") == 0;
        case 'm':
            return strcmp(s, "move-count") == 0;
        case 'n':
            if (    (strcmp(s, "name") == 0)
                 && (strcmp(w->value->sc.name, "eaters") == 0) )
            {
                return TRUE;
            }
            return FALSE;
        case 'p':
            return strcmp(s, "prev-score") == 0;
        case 's':
            return strcmp(s, "score") == 0;
        case 'r':
            if (s[1] == 'a') return strcmp(s, "random") == 0;
            if (s[1] == 'e') return strcmp(s, "reward") == 0;
            return FALSE;
        case 'x':
            return s[1] == '\0';
        case 'y':
            return s[1] == '\0';
        default:
            return FALSE;
    }//switch
}//wme_to_skip

/* ===================================================================
   get_most_activated_wmes

   Use the decay wmes array to retrive the n most activated wmes.  The
   given array should already be allocated and of length n. This
   routine can can be directed to favor leaf wmes (i.e., a wme whose
   value is not an identifier) via the threshold parameter.  The
   threshold specifies how much more activated a non-leaf wme must be
   than a leaf wme for it to be acceptable.  This threshold is
   specified as an nonnegative integer indicating how many positions
   the leaf wme must be behind the non-leaf wme in the decay queue
   (i.e., (thisAgent->decay_timelist)).  A value of 0 for this
   threshold indicates no preference between non-leaf and leaf wmes.
   The  value MAX_DECAY assures that leaf WMEs will always
   be selected.

   This function returns the actual number of wmes that were placed in
   given array.  
   
   Created: 06 Nov 2002
   Changed: 25 April 2003 - added leaf preference threshold
   =================================================================== */

int get_most_activated_wmes(agent *thisAgent, wme **active_wmes, int n, int nonleaf_threshold)
{
    decay_timelist_element *decay_list;
    int pos = 0;
    int decay_pos;              // current position in the decay array
    int nl_decay_pos = -1;      // non-leaf decay pos
    wme_decay_element *decay_element;
    int i;

    decay_list = (thisAgent->decay_timelist);
    decay_pos = (thisAgent->current_decay_timelist_element)->position;

    for(i = 0; i < MAX_DECAY; i++)
    {
        //Traverse the decay array backwards in order to get the most
        //activated wmes first
        decay_pos = decay_pos > 0 ? decay_pos - 1 : MAX_DECAY - 1;
        
        /*
         * Search for leaf wmes first
         */
        if (decay_list[decay_pos].first_decay_element != NULL)
        {
            decay_element = decay_list[decay_pos].first_decay_element;
            while (decay_element != NULL)
            {
                if ( (IS_LEAF_WME(decay_element->this_wme))
                     && (! wme_to_skip(decay_element->this_wme)) )
                {
                    active_wmes[pos] = decay_element->this_wme;
                    pos++;
                }
                decay_element = decay_element->next;
                if (pos == n) return pos;
            }
        }//if
        
        /*
         * Search for non-leaf wmes that are above the threshold
         */
        if (i >= nonleaf_threshold)
        {
            nl_decay_pos = decay_pos - nonleaf_threshold;
            if (nl_decay_pos < 0)
            {
                nl_decay_pos = nl_decay_pos + MAX_DECAY;
            }
        
            if (decay_list[nl_decay_pos].first_decay_element != NULL)
            {
                decay_element = decay_list[nl_decay_pos].first_decay_element;
                while (decay_element != NULL)
                {
                    if (! IS_LEAF_WME(decay_element->this_wme) )
                    {
                        active_wmes[pos] = decay_element->this_wme;
                        pos++;
                    }
                    
                    decay_element = decay_element->next;
                    if (pos == n) return pos;
                }
            }//if
        }//if
        
    }//for
    
    
    /*
     * In the rare case there aren't enough WMEs to fill the array
     * set the remaining entries to NULL.
     */
    if (pos < n)
    {
        for(i = pos; i < n; i++)
        {
            active_wmes[i] = NULL;
        }
    }//if

    return pos;

}//get_most_activated_wmes


/* ===================================================================
   consider_new_epmem_via_output() 

   This routine determines whether a new command has been placed on
   the output link.  If so, record_epmem() is called.

   Created: 01 Mar 2004
   =================================================================== */

void consider_new_epmem_via_output(agent *thisAgent)
{
    slot *s;
    wme *w;
    Symbol *ol = (thisAgent->io_header_output);
    int bNewMemory = FALSE;
    
    //Examine all the commands on the output link for any that have
    //appeared since the last memory was recorded
    for (s = ol->id.slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            if (w->timetag > g_last_tag)
            {
                bNewMemory = TRUE;
                g_last_tag = w->timetag;
            }
        }
    }

    //If there's a new command record a new memory
    if (bNewMemory)
    {
        record_epmem(thisAgent);         // The big one
    }

}//consider_new_epmem_via_output

/* ===================================================================
   consider_new_epmem_via_activation() 

   This routine decides whether the current state is worthy of being
   remembered as an episodic memory based upon recent changes in the
   top N most activated WMEs.  If so, record_epmem() is called.

   Created: 06 Nov 2002
   =================================================================== */

void consider_new_epmem_via_activation(agent *thisAgent)
{
    int i;

    i = get_most_activated_wmes(thisAgent, g_current_active_wmes, num_active_wmes, MAX_DECAY);
    if (i < num_active_wmes)
    {
        // no WMEs are activated right now
        return; 
    }

    //See if enough of these WMEs have changed
    qsort( (void *)g_current_active_wmes, (size_t)num_active_wmes, sizeof( wme * ),compare_ptr );
    i = wme_array_diff(g_current_active_wmes, g_previous_active_wmes, num_active_wmes);
    
    if ( i >= num_wmes_changed )
    {
        //Save the WMEs used for this memory in order to compare for
        //the next memory
        for(i = 0; i < num_active_wmes; i++)
        {
            g_previous_active_wmes[i] = g_current_active_wmes[i];
        }
        
        record_epmem(thisAgent);         // The big one

    }

    
}//consider_new_epmem_via_activation

/* ===================================================================
   epmem_clear_curr_mem

   This routine removes all the epmem WMEs from working memory that
   are associated with the current memory (h->retrieved).  

   Created: 16 Feb 2004
   Overhauled: 26 Aug 2004
   =================================================================== */
void epmem_clear_curr_mem(agent *thisAgent, epmem_header *h)
{
    int i;
    wme *w;

    //Check for "no-retrieval" wme
    if (get_arraylist_entry(thisAgent, g_wmetree.assoc_wmes,h->index) != NULL)
    {
        w = (wme *)get_arraylist_entry(thisAgent, g_wmetree.assoc_wmes,h->index);

        remove_fake_preference_for_epmem_wme(thisAgent, w);
        remove_input_wme(thisAgent, w);
        wme_remove_ref(thisAgent, w);
        set_arraylist_entry(thisAgent, g_wmetree.assoc_wmes, h->index, NULL);

        return;
    }

    //Check for trivial case
    if (h->curr_memory == NULL)
    {
        return;
    }

    //Remove the WMEs (Traverse the array in reverse order so that
    //                 children are removed before their parents.)
    for(i = h->curr_memory->content->size - 1; i >=0 ; i--)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, h->curr_memory->content,i))->node;

        if (get_arraylist_entry(thisAgent, node->assoc_wmes,h->index) == NULL)
        {
            //This is a memory leak caused by multi-valued attributes.
            //I'm going to punt on it for now.
//%%%              print(thisAgent, "\nERROR: WME should be in memory for: ");
//%%%              print_wmetree(thisAgent, node, 0, 0);
            continue;
        }

        //Remove from WM
        w = (wme *)get_arraylist_entry(thisAgent, node->assoc_wmes,h->index);
        remove_fake_preference_for_epmem_wme(thisAgent, w);
        remove_input_wme(thisAgent, w);
        wme_remove_ref(thisAgent, w);

        //Bookkeeping
        set_arraylist_entry(thisAgent, node->assoc_wmes,h->index, NULL);
    
    }//for
    
}//epmem_clear_curr_mem

/* ===================================================================
   compare_memories

   Compares two episodic memories and returns the number of *leaf
   WMEs* they have in common.  Obviously both lists should reference
   the same wmetree for this comparison to be useful.

   This function ignores WMEs that are not leaf WMEs.
   
   Created: 23 Feb 2004
   =================================================================== */
int compare_memories(agent *thisAgent, arraylist *epmem1, arraylist *epmem2)
{
    int count = 0;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < epmem1->size) && (pos2 < epmem2->size))
    {
        wmetree *node1 = ((actwme *)get_arraylist_entry(thisAgent, epmem1,pos1))->node;
        wmetree *node2 = ((actwme *)get_arraylist_entry(thisAgent, epmem2,pos2))->node;
        
        if (node1->val_type == IDENTIFIER_SYMBOL_TYPE)
        {
            pos1++;
            continue;
        }
        
        if (node2->val_type == IDENTIFIER_SYMBOL_TYPE)
        {
            pos2++;
            continue;
        }
        
        if (node1 == node2)
        {
            count++;
            pos1++;
            pos2++;
        }
        else if (node1 < node2)
        {
            pos1++;
        }
        else
        {
            pos2++;
        }
    }//while

    return count;
}//compare_memories

/* ===================================================================
   compare_memories_ideal               *EATERS ONLY*

   Compares two episoidic memories and returns the number of ideal
   wmes that match.  I've handcoded ideal wmes to be the contents of
   cells immediately adjacent to the eater and the direction the eater
   is headed.
  
   Created: 23 Feb 2004
   =================================================================== */
int compare_memories_ideal(agent *thisAgent, arraylist *epmem1, arraylist *epmem2)
{
    wmetree *node1;
    wmetree *node2;
    wmetree *tmp1;
    wmetree *tmp2;
    char *direction = NULL;

    //Compare the direction eater is travelling
    node1 = epmem_find_wmetree_entry(thisAgent, epmem1, &g_wmetree, "output-link");
    node1 = epmem_find_wmetree_entry(thisAgent, epmem1, node1, "move");
    node1 = epmem_find_wmetree_entry(thisAgent, epmem1, node1, "direction");
    node2 = epmem_find_wmetree_entry(thisAgent, epmem2, &g_wmetree, "output-link");
    node2 = epmem_find_wmetree_entry(thisAgent, epmem2, node2, "move");
    node2 = epmem_find_wmetree_entry(thisAgent, epmem2, node2, "direction");
    if ( (node1 != NULL) && (node1 == node2) )
    {
        direction = node1->val.strval;
    }
    else
    {
        //Wrong direction
        return 0;
    }

    //Find the eater's cell
    node1 = epmem_find_wmetree_entry(thisAgent, epmem1, &g_wmetree, "input-link");
    node1 = epmem_find_wmetree_entry(thisAgent, epmem1, node1, "my-location");
    node2 = epmem_find_wmetree_entry(thisAgent, epmem2, &g_wmetree, "input-link");
    node2 = epmem_find_wmetree_entry(thisAgent, epmem2, node2, "my-location");

    //Compare destination cell content
    tmp1 = epmem_find_wmetree_entry(thisAgent, epmem1, node1, direction);
    tmp1 = epmem_find_wmetree_entry(thisAgent, epmem1, tmp1, "content");
    tmp2 = epmem_find_wmetree_entry(thisAgent, epmem2, node2, direction);
    tmp2 = epmem_find_wmetree_entry(thisAgent, epmem2, tmp2, "content");
    if ( (tmp1 != NULL) && (tmp1 == tmp2) ) return 2;


    return 0;

}//compare_memories_ideal

/* ===================================================================
   compare_memories_act_indiv_mem

   Compares two episodic memories and returns the number of *leaf*
   WMEs they have in common.  Obviously both lists should reference
   the same wmetree for this comparison to be useful.

   This function ignores WMEs that are not leaf WMEs.
   This function weights matches based upon the current activation
   level of matching WMEs.
   
   
   Created: 09 March 2004
   =================================================================== */
int compare_memories_act_indiv_mem(agent *thisAgent, arraylist *epmem1, arraylist *epmem2)
{
    int count = 0;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < epmem1->size) && (pos2 < epmem2->size))
    {
        wmetree *node1 = ((actwme *)get_arraylist_entry(thisAgent, epmem1,pos1))->node;
        wmetree *node2 = ((actwme *)get_arraylist_entry(thisAgent, epmem2,pos2))->node;
        
        if (node1->val_type == IDENTIFIER_SYMBOL_TYPE)
        {
            pos1++;
            continue;
        }
        
        if (node2->val_type == IDENTIFIER_SYMBOL_TYPE)
        {
            pos2++;
            continue;
        }
        
        if (node1 == node2)
        {
            count += ((actwme *)get_arraylist_entry(thisAgent, epmem1,pos1))->activation;
            pos1++;
            pos2++;
        }
        else if (node1 < node2)
        {
            pos1++;
        }
        else
        {
            pos2++;
        }
    }//while

    return count;
}//compare_memories_act_indiv_mem


/* ===================================================================
   find_best_match

   Finds the index of the episodic memory in g_memories that most closely
   matches the cue given to the function.

   Created:  19 Jan 2004
   =================================================================== */
episodic_memory *find_best_match(agent *thisAgent, arraylist *cue)
{
    int best_score = 0;
    episodic_memory *best_mem = NULL;
    int i;
    int j;
    int comp_count = 0;         // number of epmems that were examined

    start_timer(thisAgent, &(thisAgent->epmem_match_start_time));

    //If there aren't enough memories to examine just return
    //the first one
    if (g_memories->size <= memory_match_wait)
    {
        return 0;
    }

    //Give this match a unique id
    g_last_ret_id++;

    //Every memory gets a match score boost if it contains a WME
    //that's in the cue.  So we need to loop over the assoc_memories
    //list for each wmetree node in the cue
    for(i = 0; i < cue->size; i++)
    {
        actwme *aw_cue = (actwme *)get_arraylist_entry(thisAgent, cue,i);

        if (aw_cue->node->assoc_memories->size > 0)
        {
            // note that this node was used in the match
            aw_cue->node->query_count++;
        }
            
        for(j = 0; j < aw_cue->node->assoc_memories->size; j++)
        {
            episodic_memory *epmem =
                (episodic_memory *)get_arraylist_entry(thisAgent, aw_cue->node->assoc_memories,j);
            actwme *aw_mem = epmem_find_actwme_entry(thisAgent, epmem->content, aw_cue->node);

            if (aw_mem != NULL)
            {
                if (epmem->last_usage != g_last_ret_id)
                {
                    epmem->last_usage = g_last_ret_id;
                    epmem->match_score = aw_mem->activation;
                    comp_count++;
                }
                else
                {
                    epmem->match_score += aw_mem->activation;
                }
            }

            if (epmem->match_score > best_score)
            {
                best_score = epmem->match_score;
                best_mem = epmem;
            }
        }//for
    }//for

    stop_timer(thisAgent, &(thisAgent->epmem_match_start_time), &(thisAgent->epmem_match_total_time));

    //%%%DEBUGGING
//      stop_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time), &(thisAgent->epmem_retrieve_total_time));
//      stop_timer(thisAgent, &(thisAgent->epmem_start_time), &(thisAgent->epmem_total_time));
//      print(thisAgent, "\nmemories searched:\t%d\t%d\n",
//            g_memories->size, comp_count);
//      start_timer(thisAgent, &(thisAgent->epmem_start_time));
//      start_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time));

    return best_mem;
}//find_best_match

/* ===================================================================
   install_epmem_in_wm

   Given an episodic memory this function recreates the working memory
   fragment represented by the memory in working memory.  The
   retrieved memory is placed in the given ^epmem header.

   Created:    19 Jan 2004
   Overhauled: 26 Aug 2004 
   =================================================================== */
void install_epmem_in_wm(agent *thisAgent, epmem_header *h, arraylist *epmem)
{
    int i;
    Symbol *id;
    Symbol *attr;
    Symbol *val;
    wme *new_wme;

    //Install the WMEs
    for(i = 0; i < epmem->size; i++)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, epmem,i))->node;

        //For now, avoid recursing into previous memories.
        if (strcmp(node->attr, "epmem") == 0)
        {
            continue;
        }

        if (get_arraylist_entry(thisAgent, node->assoc_wmes,h->index) != NULL)
        {
            //%%%TODO: This happens when a memory contains the same
            //         node multiple times (e.g., a multi-valued attribute)
            //         Currently, I'm just ignoring it.  That is bad.
            continue;
        }

        //Determine the WME's ID
        if (node->parent->depth == 0)
        {
            id = h->retrieved;
        }
        else
        {
            //If the parent is not in memory then the child can not be either
            wme *parent_wme = (wme *)get_arraylist_entry(thisAgent, node->parent->assoc_wmes,h->index);
            if (parent_wme == NULL)
            {
                continue;
            }

            //The value of the parent WME is the id for this WME
            id = parent_wme->value;
        }

        //Determine the WME's attribute
        attr = make_sym_constant(thisAgent, node->attr);

        //Determine the WME's value
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                val = make_sym_constant(thisAgent, node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                val = make_int_constant(thisAgent, node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                val = make_float_constant(thisAgent, node->val.floatval);
                break;
            default:
                val = make_new_identifier(thisAgent, node->attr[0],
                                          id->id.level);
                break;
        }//switch

        new_wme = add_input_wme(thisAgent, id, attr, val);
        set_arraylist_entry(thisAgent, node->assoc_wmes,h->index, new_wme);
        wme_add_ref(new_wme);
        new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);
        
    }//for

}//install_epmem_in_wm

/* ===================================================================
   respond_to_query()

   This routine examines a query attached to a given symbol.  The result
   is attached to another given symbol.  Any existing WMEs in the retrieval
   buffer are removed.

   The routine returns a pointer to the arraylist representing the
   memory which was placed on the the retrieved link (or NULL).

   query - the query
   retrieved - where to install the retrieved result

   Created: 19 Jan 2004
   =================================================================== */
arraylist *respond_to_query(agent *thisAgent, epmem_header *h)
{
    arraylist *al_query;
    arraylist *al_retrieved;
    tc_number tc;
    wme *new_wme;

    //Remove the old retrieved memory
    start_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time));
    epmem_clear_curr_mem(thisAgent, h);
    stop_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time), &(thisAgent->epmem_clearmem_total_time));
    h->curr_memory = NULL;

    //Create an arraylist representing the current query
    al_query = make_arraylist(thisAgent, 32);
    tc = h->query->id.tc_num + 1;
    update_wmetree(thisAgent, &g_wmetree, h->query, al_query, tc);

    //If the query is empty then we're done
    if (al_query->size == 0)
    {
        destroy_arraylist(thisAgent, al_query);
        return NULL;
    }

    //Diagnostic Counter
    g_num_queries++;

    //Match query to current memories list
    h->curr_memory = find_best_match(thisAgent, al_query);

    //Cleanup
    destroy_arraylist(thisAgent, al_query);
    
    //Place the best fit on the retrieved link
    if (h->curr_memory != NULL)
    {
        //Install the memory
        al_retrieved = h->curr_memory->content;
        start_timer(thisAgent, &(thisAgent->epmem_installmem_start_time));
        install_epmem_in_wm(thisAgent, h, al_retrieved);
        stop_timer(thisAgent, &(thisAgent->epmem_installmem_start_time), &(thisAgent->epmem_installmem_total_time));
    }
    else
    {
        al_retrieved = NULL;

        //Notify the user of failed retrieval
        new_wme = add_input_wme(thisAgent, h->retrieved,
                                make_sym_constant(thisAgent, "no-retrieval"),
                                make_sym_constant(thisAgent, "true"));
        set_arraylist_entry(thisAgent, g_wmetree.assoc_wmes, h->index, new_wme);
        wme_add_ref(new_wme);
        new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    }

    return al_retrieved;

}//respond_to_query

/* ===================================================================
   epmem_retrieve_command() 

   This routine examines the ^epmem link for a command.  Commands are
   always of the form (<s> ^epmem.command cmd) where "cmd" is a
   symbolic constant.  If a command is found its value is returned to
   the caller.  Otherwise NULL is returned.  The pointer returned is a
   direct reference to the WME so the pointer may *not* be valid on
   subsequent cycles.
   
   Created: 27 Jan 2004
   Changed: 19 Oct 2004 - moved the loop to a general purpose function
   =================================================================== */
char *epmem_retrieve_command(agent *thisAgent, Symbol *sym)
{
    wme *w = get_aug_of_id(thisAgent, sym, "command", NULL);
    if ( (w != NULL) && (w->value->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) )
    {
        return w->value->sc.name;
    }

    return NULL;
}//epmem_retrieve_command

/* ===================================================================
   increment_retrieval_count

   Increments the value stored in ^epmem.retieval-count by a positive
   integer. If the user passes a zero value then the present value is
   reset to 1.  If the user passes a negative value then the present
   value is removed from working memory.

   27 Jan 2004
   =================================================================== */
void increment_retrieval_count(agent *thisAgent, epmem_header *h, long inc_amt)
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    long current_count = 0;
    wme *w;

    //Find the (epmem ^retreival-count n) WME, save the value,
    //and remove the WME from WM
    //%%%If I use get_aug_of_id() to do this part the agents slows 
    //%%%way down.  WHY??
    tc = h->epmem->id.tc_num + 1;
    wmes = get_augs_of_id(thisAgent,  h->epmem, tc, &len );
    h->epmem->id.tc_num = tc - 1; // %%%Why is this necessary?
    
    if (wmes == NULL) return;
    for(i = 0; i < len; i++)
    {
        if ( (wme_has_value(wmes[i], "retrieval-count", NULL))
             && (wmes[i]->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) )
        {
            current_count = wmes[i]->value->ic.value;
            remove_input_wme(thisAgent, wmes[i]);
            break;
        }
    }
    free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);

    //Check for remove only
    if (inc_amt < 0)
    {
        return;
    }

    //Calculate the new retrieval count
    current_count += inc_amt;
    if (inc_amt == 0)
    {
        current_count = 1;
    }

    //Install a new WME
    w = add_input_wme(thisAgent, h->epmem,
                      make_sym_constant(thisAgent, "retrieval-count"),
                      make_int_constant(thisAgent, current_count));
    wme_add_ref(w);
    w->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, w);

}//increment_retrieval_count


/* ===================================================================
   respond_to_command() 

   This routine responds to agent commands given on the ^epmem link.
   The following commands are supported:
       "next" - populate ^epmem.retrieved with the next memory
                in the sequence

   cmd - the command to execute
   h - the epmem header where the command was found

   Created: 27 Jan 2004
   =================================================================== */
void respond_to_command(agent *thisAgent, char *cmd, epmem_header *h)
{
    if (strcmp(cmd, "next") == 0)
    {
        //Remove the old retrieved memory
        start_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time));
        epmem_clear_curr_mem(thisAgent, h);
        stop_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time), &(thisAgent->epmem_clearmem_total_time));

        //Check that there is a next memory available
        if ( (h->curr_memory != NULL)
             && (h->curr_memory->index < g_memories->size - memory_match_wait) )
        {
            //Update the current memory pointer to point to the next epmem
            h->curr_memory =
                (episodic_memory *)get_arraylist_entry(thisAgent, g_memories,
                                                       h->curr_memory->index + 1);
            
            //Install the new memory
            start_timer(thisAgent, &(thisAgent->epmem_installmem_start_time));
            install_epmem_in_wm(thisAgent, h, h->curr_memory->content);
            stop_timer(thisAgent, &(thisAgent->epmem_installmem_start_time), &(thisAgent->epmem_installmem_total_time));
        }
        else
        {
            //Notify the user of failed retrieval
            wme *w = add_input_wme(thisAgent, h->retrieved,
                                   make_sym_constant(thisAgent, "no-retrieval"),
                                   make_sym_constant(thisAgent, "true"));
            set_arraylist_entry(thisAgent, g_wmetree.assoc_wmes, h->index, w);
            wme_add_ref(w);
            w->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, w);

        }
        
        increment_retrieval_count(thisAgent, h, 1);
    }
    
}//respond_to_command

/* ===================================================================
   find_superstate()

   Given a symbol for a state, return the symbol for its superstate.

   Created: 05 Oct 2004
   =================================================================== */
Symbol *find_superstate(agent *thisAgent, Symbol *sym)
{
    wme **wmes = NULL;
    tc_number tc = sym->id.tc_num + 1; // Is this going to cause problems?
    int len = 0;
    int i;
    Symbol *ss = NULL;

    start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
    wmes = get_augs_of_id(thisAgent,  sym, tc, &len );
    stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

    if (wmes != NULL)
    {
        for(i = 0; i < len; i++)
        {
            //Check for special case: "superstate" 
            if ( (wme_has_value(wmes[i], "superstate", NULL))
                 && (ss == NULL)
                 && (wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) )
            {
                ss = wmes[i]->value;
                break;
            }
        }

        free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
    }//if

    return ss;
        
}//find_superstate


/* ===================================================================
   epmem_update_header_stack()

   Update the list of epmem_header structs which maintain information
   about what memories have been retrieved for each state in the
   state stack.

   Created: 03 Oct 2004
   =================================================================== */
void epmem_update_header_stack(agent *thisAgent)
{
    Symbol *sym;
    int i;
    epmem_header *h;
    arraylist *new_states = make_arraylist(thisAgent, 20);
    int bFound = FALSE;


    /*
     * Find the lowest state in g_header_stack that has an analog
     * in working memory.  As we do the search, record any states
     * that are lower than this in the new_states list.
     */

    //Start at the bottom state and work our way up
    sym = (thisAgent->bottom_goal);
    while(sym != (thisAgent->top_goal))
    {
        //Search for an analog to the state in g_header_stack
        for(i = g_header_stack->size - 1; i >= 0; i--)
        {
            h = (epmem_header *)get_arraylist_entry(thisAgent, g_header_stack,i);
            if (h->state == sym)
            {
                //An analog was found, remove any of its children and break
                int j;
                for(j = g_header_stack->size - 1; j > i; j--)
                {
                    destroy_epmem_header(
                        thisAgent, 
                        (epmem_header *)remove_entry_from_arraylist(g_header_stack, j));
                }

                bFound = TRUE;
                break;
            }//if
        }//for

        //If an analog was found, exit the loop
        if (bFound) break;
        
        //The current sym is a new state that needs an ^epmem link
        //so save it away for later
        append_entry_to_arraylist(thisAgent, new_states, (void *)sym);

        //Move to the parent state and iterate
        sym = find_superstate(thisAgent, sym);
        
    }//while

    /*
     * The states in new_states need an ^epmem link and a corresponding
     * entry in the g_header_stack list.
     */
    for(i = new_states->size - 1; i >= 0; i--)
    {
        h = make_epmem_header(thisAgent, (Symbol *)get_arraylist_entry(thisAgent, new_states,i));
        h->index = g_header_stack->size;
        append_entry_to_arraylist(thisAgent, g_header_stack, (void *)h);
    }

//      //%%%DEBUGGING:  Print the current header stack
//      print(thisAgent, "\nEpMem Header Stack: ");
//      for(i = 0; i < g_header_stack->size; i++)
//      {
//          print_with_symbols("%y ",
//                             ((epmem_header *)get_arraylist_entry(thisAgent, g_header_stack,i))->state);
//      }
//      print(thisAgent, "\n");
    
    
}//epmem_update_header_stack


/* ===================================================================
   epmem_print_curr_memory()           *DEBUGGING*

   Prints information about what's currently on the ^epmem link.

   Created: 20 Feb 2004
   =================================================================== */
void epmem_print_curr_memory(agent *thisAgent, epmem_header *h)
{
    if (h->curr_memory == NULL)
    {
        print(thisAgent, "\nCURRENT MEMORY:  None.\n");
        return;
    }
    
    //Print the current memory
    print(thisAgent, "\nCURRENT MEMORY:  %d of %d\t\t", h->curr_memory->index, g_memories->size);
    print_memory_graphically(thisAgent, h->curr_memory->content);

}//epmem_print_curr_memory

/* ===================================================================
   epmem_calc_wmetree_size      *RECURSIVE*

   Calculates the size of a wmetree in terms of numbers of nodes

   Created: 15 June 2004
   =================================================================== */
int epmem_calc_wmetree_size(wmetree *node)
{
    int size = 0;
    unsigned long hash_value;
    wmetree *child;
    
    if (node == NULL) return 0;

    size += node->children->size;
    
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            size += epmem_calc_wmetree_size(child);
        }
    }

    return size;
}//epmem_calc_wmetree_size

/* ===================================================================
   epmem_print_detailed_usage_memories

   Prints a memory usage trace for g_memories to a given file handle.

   Created: 15 June 2004
   =================================================================== */
//NOTE:  NUM_BINS must be less than 256 and HISTOGRAM_SIZE must be greater
//       than the maximum size of any memory.
#define BIN_SIZE 3
#define NUM_BINS 200
#define HISTOGRAM_SIZE (BIN_SIZE * NUM_BINS + 1)
void epmem_print_detailed_usage_memories(agent *thisAgent, FILE *f)
{
    int histogram[HISTOGRAM_SIZE];
    int i,j;
    int largest = 0;
    

    for(i = 0; i < HISTOGRAM_SIZE; i++)
    {
        histogram[i] = 0;
    }

    for(i = 0; i < g_memories->size; i++)
    {
        int memsize =  ((arraylist *)get_arraylist_entry(thisAgent, g_memories,i))->size;
        if (memsize > HISTOGRAM_SIZE)
        {
            fprintf(f, "ERROR!  Memories are too large.\n");
        }
        else
        {
            (histogram[memsize])++;
        }

        if (memsize > largest) largest = memsize;
    }

    //To accomodate maximum excel spreadsheet dimensions , bin the results
    for(i = 0; i < NUM_BINS; i++)
    {
        int sum = 0;
        for(j = 1; j <= BIN_SIZE; j++)
        {
            sum += histogram[i*BIN_SIZE + j];
        }
        fprintf(f, "%d\t", sum);
    }
    fprintf(f, "\n");
    
}//epmem_print_detailed_usage_memories

/* ===================================================================
   epmem_print_detailed_usage_wmetree      *RECURSIVE*

   Prints a memory usage trace for a wmetree to a given file handle.

   f - file handle to print to
   node - the wmetree to print the usage stats about
   largest - initial caller should set an int to zero and pass in a
             ptr to it here

   Created: 15 June 2004
   =================================================================== */
void epduw_helper(wmetree *node, int *histogram, int *largest)
{
    int nodesize;
    unsigned long hash_value;
    wmetree *child;

    if (node->assoc_memories != NULL)
    {
        nodesize = node->assoc_memories->size;
        (histogram[nodesize])++;
        if (nodesize > *largest) *largest = nodesize;
    }
    
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            epduw_helper(child, histogram, largest);
        }
    }
    
}//epduw_helper

void epmem_print_detailed_usage_wmetree(FILE *f, wmetree *node)
{
    static int histogram[HISTOGRAM_SIZE];
    int i;
    int largest = 0;
    
    if (node == NULL)
    {
        fprintf(f, "\n");
        return;
    }

    for(i = 0; i < HISTOGRAM_SIZE; i++)
    {
        histogram[i] = 0;
    }

    epduw_helper(node, histogram, &largest);
    
    fprintf(f, "%d", histogram[2]);
    for(i = 2; i <= largest; i++)
    {
        fprintf(f, "\t%d", histogram[i]);
    }
    fprintf(f, "\n");
    
}//epmem_print_detailed_usage_wmetree



/* ===================================================================
   epmem_print_mem_usage

   Prints information about how much RAM the episodic memories are
   using to a file.

   Created: 14 June 2004
   =================================================================== */
void epmem_print_mem_usage(agent *thisAgent)
{
    wmetree *parent = &g_wmetree;
    wmetree *child;
    arraylist *queue = make_arraylist(thisAgent, 32);
    int qpos = 0;                // current position in the queue
    unsigned long hash_value;
    int num_wmetrees = 0;
    int num_am_ptrs = 0;
    int num_strings = 0;
    int total_strlen = 0;
    int num_epmems = g_memories->size;
    int num_actwmes = 0;
    int i;
    episodic_memory *epmem;

    while(parent != NULL)
    {
        for (hash_value = 0; hash_value < parent->children->size; hash_value++)
        {
            child = (wmetree *) (*(parent->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                if (child->val_type == SYM_CONSTANT_SYMBOL_TYPE)
                {
                    num_strings++;
                    total_strlen += strlen(child->val.strval);
                }

                num_am_ptrs += child->assoc_memories->size;
                
                num_wmetrees++;

                //If this WME has children add it to the queue for future
                //processing
                if (child->children->count > 0)
                {
                    append_entry_to_arraylist(thisAgent, queue, child);
                }

            }//for
        }//for

        //Retrieve the next node from the queue
        parent = (wmetree *)get_arraylist_entry(thisAgent, queue, qpos);
        qpos++;
    }//while

    destroy_arraylist(thisAgent, queue);

    for(i = 1; i < g_memories->size; i++)
    {
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent, g_memories, i);
        num_actwmes += epmem->content->size;
    }
    
    print(thisAgent, "\n");
    print(thisAgent, "%.6d wmetree structs = %.10d bytes\n", num_wmetrees, num_wmetrees*65);
    print(thisAgent, "%.6d epmem ptrs      = %.10d bytes\n", num_am_ptrs, num_am_ptrs*4);
    print(thisAgent, "%.6d symbol strings  = %.10d bytes\n", num_strings, total_strlen);
    print(thisAgent, "%.6d epmem structs   = %.10d bytes\n", num_epmems, num_epmems*28);
    print(thisAgent, "%.6d actwme structs  = %.10d bytes\n", num_actwmes, num_actwmes*8);
    print(thisAgent, "     TOTAL EPMEM ALLOC = %.10d bytes\n",
          num_wmetrees*65 + num_am_ptrs*4 + total_strlen + num_epmems*28 + num_actwmes*8);
    
}//epmem_print_mem_usage


/* ===================================================================
   epmem_reset_cpu_usage_timers() 

   Created: 07 Jul 2004
   =================================================================== */
void epmem_reset_cpu_usage_timers(agent *thisAgent)
{
    reset_timer(&(thisAgent->epmem_total_time));
    reset_timer(&(thisAgent->epmem_record_total_time));
    reset_timer(&(thisAgent->epmem_retrieve_total_time));
    reset_timer(&(thisAgent->epmem_clearmem_start_time));
    reset_timer(&(thisAgent->epmem_clearmem_total_time));
    reset_timer(&(thisAgent->epmem_updatewmetree_total_time));
    reset_timer(&(thisAgent->epmem_getaugs_total_time));
    reset_timer(&(thisAgent->epmem_match_total_time));
    reset_timer(&(thisAgent->epmem_findchild_total_time));
    reset_timer(&(thisAgent->epmem_addnode_total_time));
    reset_timer(&(thisAgent->epmem_installmem_total_time));
    reset_timer(&(thisAgent->epmem_misc1_total_time));
    reset_timer(&(thisAgent->epmem_misc2_total_time));
}

/* ===================================================================
   epmem_print_cpu_usage() 

   Created: 07 Jul 2004
   =================================================================== */
void epmem_print_cpu_usage(agent *thisAgent)
{
    double total = timer_value(&(thisAgent->epmem_total_time));
    double f;
    
    print(thisAgent, "\n**** Epmem CPU Usage Results ****\n\n");
    print(thisAgent, "Routine                             Time      Fraction of Total Time\n");
    print(thisAgent, "----------------------------------  --------  ----------------------\n");

    f = timer_value(&(thisAgent->epmem_record_total_time));
    print(thisAgent, "record_epmem()                       %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_updatewmetree_total_time));
    print(thisAgent, "    update_wmetree( )                 %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_getaugs_total_time));
    print(thisAgent, "        get_augs_of_id()             %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_findchild_total_time));
    print(thisAgent, "        find_child_node()            %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_addnode_total_time));
    print(thisAgent, "        add_node_to_memory()         %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_retrieve_total_time));
    print(thisAgent, "episodic retrieval                   %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_clearmem_total_time));
    print(thisAgent, "    epmem_clear_curr_mem()           %.3lf     %.3lf\n", f, f / total);
    print(thisAgent, "    update_wmetree() is also called here (see above)\n");
    f = timer_value(&(thisAgent->epmem_match_total_time));
    print(thisAgent, "    find_best_match()                %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_installmem_total_time));
    print(thisAgent, "    install_epmem_in_wm()            %.3lf     %.3lf\n", f, f / total);

    print(thisAgent, "--------------------------------------------------------------------\n");
    print(thisAgent, "TOTAL                                %.3lf     1.0\n", total);

    epmem_reset_cpu_usage_timers(thisAgent);

}//epmem_print_cpu_usage

/* ===================================================================
   epmem_print_query_usage_wmetree      *RECURSIVE*

   Prints a info showing how often nodes of a particular size
   (in terms of number of children) were used in a query cue.

   Created: 13 July 2004
   =================================================================== */
void epquw_helper(agent *thisAgent,
                  wmetree *node,
                  int *total_queries,
                  int *num_nodes,
                  int *largest)
{
    int nodesize;
    unsigned long hash_value;
    wmetree *child;

    if (node->assoc_memories != NULL)
    {
        nodesize = node->assoc_memories->size;
        if (nodesize < HISTOGRAM_SIZE)
        {
            (total_queries[nodesize]) += node->query_count;
            (num_nodes[nodesize]) ++;
            if (nodesize > *largest) *largest = nodesize;
        }

//          //%%%DEBUGGING:  REMOVE THIS IF-CLAUSE LATER
//          if ( (num_nodes[nodesize] > 0)
//               && ( total_queries[nodesize] / num_nodes[nodesize] > 250)
//               && ( total_queries[nodesize] / num_nodes[nodesize] < 300) )
//          {
//              print_wmetree(thisAgent, node, 0, 3);
//              print(thisAgent, "...has a high contribution to query time: %d.\n",
//                    total_queries[nodesize] / num_nodes[nodesize]);
//          }

    }

    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            epquw_helper(thisAgent, child, total_queries, num_nodes, largest);
        }
    }
    
}//epquw_helper

void epmem_print_query_usage_wmetree(agent *thisAgent)
{
    FILE *f;
    static int total_queries[HISTOGRAM_SIZE];
    static int num_nodes[HISTOGRAM_SIZE];
    int i;
    int largest = 0;

    //Print the episodic memories' usage
    f = fopen("\\temp\\epmem_query_usage_wmetree.txt", "aw");
    if (f == NULL) return;

    for(i = 0; i < HISTOGRAM_SIZE; i++)
    {
        total_queries[i] = 0;
        num_nodes[i] = 0;
    }

    epquw_helper(thisAgent, &g_wmetree, total_queries, num_nodes, &largest);
    
    if (num_nodes[2] == 0)
    {
        fprintf(f, "0");
    }
    else
    {
        fprintf(f, "%d", total_queries[2] / num_nodes[2]);
    }
        
    for(i = 2; i <= largest; i++)
    {
        if (num_nodes[i] == 0)
        {
            fprintf(f, "\t0");
        }
        else
        {
            fprintf(f, "\t%d", total_queries[i] / num_nodes[i]);
        }

    }
    fprintf(f, "\n");
    fclose(f);
    
}//epmem_print_query_usage_wmetree



/* ===================================================================
   epmem_update() 

   This routine is called at every output phase to allow the episodic
   memory system to update its current memory store and respond to any
   queries. 

   Created: 19 Jan 2004
   =================================================================== */
void epmem_update(agent *thisAgent)
{
    char *cmd;
    arraylist *epmem = NULL;
    static int count = 0;
    int i;

    count++;

    start_timer(thisAgent, &(thisAgent->epmem_start_time));

    //Update the stack of epmem_header structs
    epmem_update_header_stack(thisAgent);
    
    start_timer(thisAgent, &(thisAgent->epmem_record_start_time));

    //Consider recording a new epmem
    consider_new_epmem_via_output(thisAgent);

    stop_timer(thisAgent, &(thisAgent->epmem_record_start_time), &(thisAgent->epmem_record_total_time));
    start_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time));
    
    /*
     * Update the ^retrieved link on each epmem header as necessary
     */

    for(i = 0; i < g_header_stack->size; i++)
    {
        epmem_header *h = (epmem_header *)get_arraylist_entry(thisAgent, g_header_stack, i);
        //Look for a command
        cmd = epmem_retrieve_command(thisAgent, h->query);
        if (cmd != NULL)
        {
            respond_to_command(thisAgent, cmd, h);
        }
        else
        {
            //Look for a new cue on the query link
        
            //%%%DEBUGGING
            //decay_print_most_activated_wmes(50);
        
            epmem = respond_to_query(thisAgent, h);

            if (epmem != NULL)
            {
                //New retrieval:  reset count to zero
                increment_retrieval_count(thisAgent, h, 0);
            }
            else
            {
                //No retrieval:  remove count from working memory
                increment_retrieval_count(thisAgent, h, -1);
            }
        }//else

//          //%%%DEBUGGING
//          if (h->curr_memory != NULL)
//          {
//              epmem_print_curr_memory(h);
//          }
        
    }//for
    
    stop_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time), &(thisAgent->epmem_retrieve_total_time));
    stop_timer(thisAgent, &(thisAgent->epmem_start_time), &(thisAgent->epmem_total_time));

    
    
    //%%%DEBUGGING
    if (count % 500 == 0)
    {
        epmem_print_mem_usage(thisAgent);
//        epmem_print_cpu_usage();
        print(thisAgent, "\nend of run %d\n", count / 1500);
    }
    
//      //%%%DEBUGGING
//      if (count % 100 == 0)
//      {
//          epmem_print_query_usage_wmetree();
//          print(thisAgent, "\n%d queries and %d memories\n", g_num_queries, g_memories->size);
//      }

    
}//epmem_update

/* ===================================================================
   epmem_create_buffer()

   This routine creates the ^epmem link on the top state.  This is
   used to query the episodic memory and provided the retrieved
   result.  The Soar kernel is expected to call this function once
   the top state has been created.

   Created: 08 Jan 2004
   Updated: 04 Oct 2004 - to handle new epmem_header stack
   =================================================================== */

void epmem_create_buffer(agent *thisAgent, Symbol *s)
{
    epmem_header *top_state_header;
    
    top_state_header = make_epmem_header(thisAgent, s);
    top_state_header->index = 0;
    append_entry_to_arraylist(thisAgent, g_header_stack, (void *)top_state_header);
    
}//epmem_create_buffer


/* ===================================================================
   init_epemem()

   This routine is called once to initialize the episodic memory system.

   Created: 03 Nov 2002
   =================================================================== */

void init_epmem(agent *thisAgent) 
{
    //Allocate the active wmes arrays
    g_current_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));
    g_previous_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));

    //Initialize the wmetree
    g_wmetree.next = NULL;
    g_wmetree.attr = NULL;
    g_wmetree.val.intval = 0;
    g_wmetree.val_type = IDENTIFIER_SYMBOL_TYPE;
    g_wmetree.children = make_hash_table(thisAgent, 0, hash_wmetree);;
    g_wmetree.parent = NULL;
    g_wmetree.depth = 0;
    g_wmetree.assoc_wmes = make_arraylist(thisAgent, 20);

    //Initialize the memories array
    g_memories = make_arraylist(thisAgent, 512);

    //Initialize the g_header_stack array
    g_header_stack = make_arraylist(thisAgent, 20);
    
    
    //Reset the timers
    epmem_reset_cpu_usage_timers(thisAgent);
    
}/*init_epmem*/




#endif /* AMN end */

/* ===================================================================
   =================================================================== */
