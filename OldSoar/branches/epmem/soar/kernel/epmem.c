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

#include "soarkernel.h"

#ifdef AMN_EP_MEM

//defined in soar_core_utils.c
extern wme ** get_augs_of_id( Symbol *id, tc_number tc, int *num_attr );

//defined in activate.c
extern int decay_activation_level(wme *w);
extern void decay_print_most_activated_wmes(int n);

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
                      

   %%%TODO:  Made these values command line configurable

*/

#define num_active_wmes 1
#define num_wmes_changed 1
#define memories_init_size 512
#define memory_match_wait 1     // %%%TODO: no longer needed?

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

             id   - every WME in the tree has a unique id.  I can't
                    use the timetag because multiple WMEs may have the same
                    value.
             attr - the string representing the WME attribute associated
                    with this tree node
             attr - the string representing the WME value associated
                    with this tree node (*only* if this is a leaf node)
             is_leaf - TRUE means that this node has no children
             children - children of the current node (hash table)
             parent - parent of the current node.
             next/prev - dll of all nodes in the tree to allow iterative
                         implementation of several algorithms
             in_mem - whether this "wme" is in the current memory
                      
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
    bool in_mem;
    struct wme_struct *assoc_wme;
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
   g_curr_memory          - Index of the memory currently in the retrieved link
   g_epmem_header         - The symbol that ^epmem is attached to
   g_epmem_query          - The symbold that ^query is attached to
   g_epmem_retrieved      - The symbold that ^retrieved is attached to
   g_last_tag             - The timetag of the last command on the output-link
                            
*/

//%%%FIXME:  move these globals to current_agent()
wme **g_current_active_wmes;
wme **g_previous_active_wmes;
wmetree g_wmetree;
arraylist *g_memories;
int g_curr_memory;
Symbol *g_epmem_header;
Symbol * g_epmem_query;
Symbol * g_epmem_retrieved;
unsigned long g_last_tag = 0; 



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

   Compares two actwmes passed in as void *.  (Used for qsort() calls)
   
   Created: 22 April 2004
   =================================================================== */
int compare_actwme( const void *arg1, const void *arg2 )
{
    actwme *aw1 = (actwme *)arg1;
    actwme *aw2 = (actwme *)arg2;

    return aw1->node == aw2->node;
}//compare_actwme


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
   make_wmetree_node

   Creates a new wmetree node based upon a given wme.  If no WME is
   given (w == NULL) then an empty node is allocated.  The caller is
   responsible for setting the parent pointer.
   
   Created: 09 Jan 2004
   =================================================================== */
wmetree *make_wmetree_node(wme *w)
{
    wmetree *node;

    node = allocate_memory(sizeof(wmetree), MISCELLANEOUS_MEM_USAGE);
    node->next = NULL;
    node->attr = NULL;
    node->val.intval = 0;
    node->val_type = IDENTIFIER_SYMBOL_TYPE;
    node->children = make_hash_table(0, hash_wmetree);
    node->parent = NULL;
    node->in_mem = FALSE;
    node->assoc_wme = NULL;

    if (w == NULL) return node;
    
    node->attr = allocate_memory(sizeof(char)*strlen(w->attr->sc.name) + 1,
                                MISCELLANEOUS_MEM_USAGE);
    strcpy(node->attr, w->attr->sc.name);
    
    switch(w->value->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            node->val_type = SYM_CONSTANT_SYMBOL_TYPE;
            node->val.strval =
                allocate_memory(sizeof(char)*strlen(w->value->sc.name) + 1,
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
void dw_helper(wmetree *tree)
{
    unsigned long hash_value;
    wmetree *child;

    if (tree->attr != NULL)
    {
        free_memory(tree->attr, MISCELLANEOUS_MEM_USAGE);
    }
    
    if (tree->val_type == SYM_CONSTANT_SYMBOL_TYPE)
    {
        free_memory(tree->val.strval, MISCELLANEOUS_MEM_USAGE);
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
            dw_helper(child);
            free_memory(child, MISCELLANEOUS_MEM_USAGE);
        }
    }

    free_memory(tree->children, HASH_TABLE_MEM_USAGE);
}//dw_helper

void destroy_wmetree(wmetree *tree)
{
    dw_helper(tree);

    free_memory(tree, MISCELLANEOUS_MEM_USAGE);
}//destroy_wmetree

/* ===================================================================
   make_arraylist

   Allocates and initializes a new, empty arraylist and returns a pointer
   to the caller.

   init_cap = initial capacity
   
   Created: 13 Jan 2004
   =================================================================== */
arraylist *make_arraylist(int init_cap)
{
    arraylist *al;

    if (init_cap <= 0) init_cap = 32; // default value
    
    al = (arraylist *)allocate_memory(sizeof(arraylist),
                                      MISCELLANEOUS_MEM_USAGE);
    al->array = (void **)allocate_memory(sizeof(void*) * init_cap,
                                         MISCELLANEOUS_MEM_USAGE);
    al->capacity = init_cap;
    al->size = 0;
    al->next = NULL;

    return al;
}//make_arraylist

/* ===================================================================
   destroy_arraylist

   Deallocates a given arraylist.
   
   Created: 19 Jan 2004
   =================================================================== */
void destroy_arraylist(arraylist *al)
{
    if (al == NULL) return;
    
    if ( (al->capacity > 0) && (al->array != NULL) )
    {
        free_memory(al->array, MISCELLANEOUS_MEM_USAGE);
    }
    free_memory(al, MISCELLANEOUS_MEM_USAGE);
}//destroy_arraylist

/* ===================================================================
   append_entry_to_arraylist

   This function adds a new entry to the end of an arraylist
   
   Created: 22 Apr 2004
   =================================================================== */
void append_entry_to_arraylist(arraylist *al, void *new_entry)
{
    if (al->size == al->capacity)
    {
        int i;
        void **new_array;
        int new_capacity;

        //Select the new array size
        new_capacity = al->capacity * 2;
        if (new_capacity == 0) new_capacity = 32;
        
        //Grow the array (can't use realloc b/c of Soar's memory routines)
        new_array = (void **)allocate_memory_and_zerofill(new_capacity * sizeof(void*),
                                                          MISCELLANEOUS_MEM_USAGE);
        for(i = 0; i < al->size; i++)
        {
            new_array[i] = al->array[i];
        }

        if (al->array != NULL)
        {
            free_memory(al->array, MISCELLANEOUS_MEM_USAGE);
        }

        al->array = new_array;
        al->capacity = new_capacity;
    }//if


    //Add the node to the array
    al->array[al->size] = new_entry;
    al->size++;
    
}//append_entry_to_arraylist


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
void print_wmetree(wmetree *node, int indent, int depth)
{
    unsigned long hash_value;
    wmetree *child;
    
    if (node == NULL) return;

    if (node->parent == NULL) // check for root
    {
        print("\n\nROOT\n");
    }
    else
    {
        if (indent)
        {
            print("%*s+--", indent, "");
        }
        print("%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(" %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(" %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(" %f", node->val.floatval);
                break;
            default:
                break;
        }//switch
    }//else
    print("\n");

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_wmetree(child, indent + 3, depth - 1);
            }
        }
    }
}//print_wmetree

/* ===================================================================
   epmem_find_memory_entry

   Finds a descendent entry that has a particular id and attributein a
   given memory.  If the given parent is &g_wmetree then it is assumed
   to be a wildcard match.

   Returns NULL if not found.

   Created: 20 Feb 2004
   =================================================================== */
wmetree *epmem_find_memory_entry(arraylist *epmem, wmetree *id, char *s)
{
    int i;

    if (epmem == NULL) return NULL;
    
    for(i = 0; i < epmem->size; i++)
    {
        wmetree *node = ((actwme *)(epmem->array[i]))->node;
        if ( (id == &g_wmetree) || (id == node->parent) )
        { 
            if (strcmp(node->attr, s) == 0)
            {
                return node;
            }
        }
    }//for

    return NULL;
    
}//epmem_find_memory_entry

/* ===================================================================
   print_memory

   
   Created: 01 Mar 2004
   =================================================================== */
void print_memory(arraylist *epmem, wmetree *node, int indent, int depth)
{
    int i;
    unsigned long hash_value;
    wmetree *child;

    if (epmem == NULL) return;
    if (node == NULL) return;

    if (node->parent == NULL) // check for root
    {
        print("\n\nROOT\n");
    }
    else
    {
        int bFound = FALSE;
        
        //Find out if this node is in the arraylist
        for(i = 0; i < epmem->size; i++)
        {
            if (((actwme *)epmem->array[i])->node == node)
            {
                bFound = TRUE;
            }
        }
        
        if (!bFound) return;
        
        if (indent)
        {
            print("%*s+--", indent, "");
        }
        print("%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(" %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(" %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(" %f", node->val.floatval);
                break;
            default:
                break;
        }//switch
    }//else
    print("\n");

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_memory(epmem, child, indent + 3, depth - 1);
            }
        }
    }
    
}//print_memory

/* ===================================================================
   print_memory_graphically         *EATERS DOMAIN DEPENDENT*

   
   Created: 01 Mar 2004
   =================================================================== */
void print_memory_graphically(arraylist *epmem)
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
    ol = epmem_find_memory_entry(epmem, &g_wmetree, "output-link");
    move = epmem_find_memory_entry(epmem, ol, "move");
    direction = epmem_find_memory_entry(epmem, move, "direction");

    //Find the current score
    il = epmem_find_memory_entry(epmem, &g_wmetree, "input-link");
    eater = epmem_find_memory_entry(epmem, il, "eater");
    score = epmem_find_memory_entry(epmem, eater, "score");
    
    //Find the contents of each surrounding cell
    my_location = epmem_find_memory_entry(epmem, il, "my-location");
    north = epmem_find_memory_entry(epmem, my_location, "north");
    south = epmem_find_memory_entry(epmem, my_location, "south");
    east = epmem_find_memory_entry(epmem, my_location, "east");
    west = epmem_find_memory_entry(epmem, my_location, "west");
    n_content = epmem_find_memory_entry(epmem, north, "content");
    s_content = epmem_find_memory_entry(epmem, south, "content");
    e_content = epmem_find_memory_entry(epmem, east, "content");
    w_content = epmem_find_memory_entry(epmem, west, "content");

    if (n_content != NULL) n_char   = n_content->val.strval[0];
    if (s_content != NULL) s_char   = s_content->val.strval[0];
    if (e_content != NULL) e_char   = e_content->val.strval[0];
    if (w_content != NULL) w_char   = w_content->val.strval[0];
    if (direction != NULL) dir_char = direction->val.strval[0];

    print("\n         %c", n_char);
    print("\n        %c%c%c",w_char, dir_char, e_char);
    print("\n         %c", s_char);

    if (score != NULL)
    {
        print("\n  score=%d.", score->val.intval);
    }
    else
    {
        print("\n  score=NOT AVAILABLE");
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
void add_node_to_memory(arraylist *epmem, wmetree *node, int activation)
{
    //Allocate and init a new actwme
    actwme *aw = (actwme *)allocate_memory(sizeof(actwme),
                                           MISCELLANEOUS_MEM_USAGE);
    aw->node = node;
    aw->activation = activation;
    
    append_entry_to_arraylist(epmem, (void *)aw);
    
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
   
   This function requires a tc number.

   Created: 09 Jan 2004
   Updated: 23 Feb 2004 - made breadth first and non-recursive
   =================================================================== */
Symbol *update_wmetree(wmetree *node,
                       Symbol *sym,
                       arraylist *epmem,
                       tc_number tc)
{
    wme **wmes = NULL;
    wmetree *childnode;
    int len = 0;
    int i;
    Symbol *ss = NULL;
    arraylist *syms = make_arraylist(32);
    int pos = 0;

    do
    {
        wmes = get_augs_of_id( sym, tc, &len );
        if (wmes != NULL)
        {
            for(i = 0; i < len; i++)
            {
                childnode = find_child_node(node, wmes[i]);
                if (childnode == NULL)
                {
                    childnode = make_wmetree_node(wmes[i]);
                    add_to_hash_table(node->children, childnode);
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
                add_node_to_memory(epmem, childnode, decay_activation_level(wmes[i]));
                append_entry_to_arraylist(syms, (void *)wmes[i]->value);
            }//for
        }//if

        //Special Case:  no wmes found attached to the given symbol
        if (epmem->size == 0) break;
        
        node = ((actwme *)epmem->array[pos])->node;
        sym = (Symbol *)syms->array[pos];
        pos++;

    } while(pos < epmem->size);
    
    //Sort the memory's arraylist using the node pointers
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme );

    //Deallocate the symbol list
    destroy_arraylist(syms);

    return ss;
    
}//update_wmetree

/* ===================================================================
   record_epmem

   Once it has been determined that an epmem needs to be recorded,
   this routine manages all the steps for recording it.

   Created: 12 Jan 2004
   =================================================================== */
void record_epmem( )
{
    tc_number tc;
    Symbol *sym;
    arraylist *curr_state;
    arraylist *next_state;

    //Starting with bottom_goal and moving toward top_goal, add all
    //the current states to the wmetree and record the full WM
    //state as an arraylist of actwmes
    sym = current_agent(bottom_goal);
    
    //Do only top-state for now
    sym = current_agent(top_goal);  //%%%TODO: remove this later
    
    curr_state = NULL;
    next_state = NULL;
    while(sym != NULL)
    {
        next_state = make_arraylist(128);
        next_state->next = curr_state;
        curr_state = next_state;

        tc = sym->id.tc_num + 1;
        sym = update_wmetree(&g_wmetree, sym, curr_state, tc);
    }

    //Store the recorded memory
    append_entry_to_arraylist(g_memories, (void *)curr_state);

//      //%%%DEBUGGING
//      print("\nRECORDED MEMORY %d:\n", g_memories->size - 1);
//      print_memory_graphically(g_memories->array[g_memories->size - 1]);
    
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
   (i.e., current_agent(decay_timelist)).  A value of 0 for this
   threshold indicates no preference between non-leaf and leaf wmes.
   The  value MAX_DECAY assures that leaf WMEs will always
   be selected.

   This function returns the actual number of wmes that were placed in
   given array.  
   
   Created: 06 Nov 2002
   Changed: 25 April 2003 - added leaf preference threshold
   =================================================================== */

int get_most_activated_wmes(wme **active_wmes, int n, int nonleaf_threshold)
{
    decay_timelist_element *decay_list;
    int pos = 0;
    int decay_pos;              // current position in the decay array
    int nl_decay_pos = -1;      // non-leaf decay pos
    wme_decay_element *decay_element;
    int i;

    decay_list = current_agent(decay_timelist);
    decay_pos = current_agent(current_decay_timelist_element)->position;

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

void consider_new_epmem_via_output()
{
    slot *s;
    wme *w;
    Symbol *ol = current_agent(io_header_output);
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
        record_epmem();         // The big one
    }

}//consider_new_epmem_via_output

/* ===================================================================
   consider_new_epmem_via_activation() 

   This routine decides whether the current state is worthy of being
   remembered as an episodic memory based upon recent changes in the
   top N most activated WMEs.  If so, record_epmem() is called.

   Created: 06 Nov 2002
   =================================================================== */

void consider_new_epmem_via_activation()
{
    int i;

    i = get_most_activated_wmes(g_current_active_wmes, num_active_wmes, MAX_DECAY);
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
        
        record_epmem();         // The big one

    }

    
}//consider_new_epmem_via_activation

/* ===================================================================
   epmem_clear_mem               *RECURSIVE*

   This routine removes all the epmem WMEs from working memory that
   are associated with a given wmetree (via ->assoc_wme).  This
   function assumes that it will initially be called with an argument
   of &g_wmetree.

   Created: 16 Feb 2004
   =================================================================== */
void epmem_clear_mem(wmetree *node)
{
    wmetree *child;
    unsigned long hash_value;

    //Check for "no-retrieval" wme
    if (g_wmetree.assoc_wme != NULL)
    {
        remove_input_wme(g_wmetree.assoc_wme);
        wme_remove_ref(g_wmetree.assoc_wme);
        g_wmetree.assoc_wme = NULL;

        return;
    }

    //Check for trivial case
    if (g_curr_memory == -1)
    {
        return;
    }

    //Find all the wmes attached to the given node
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            if (! child->in_mem) continue;
            if (strcmp(child->attr, "epmem") == 0) continue;
            if (child->assoc_wme == NULL)
            {
                print("\nERROR: WME should be in memory for: ");
                print_wmetree(child, 0, 0);
                continue;
            }

            //Recursive call.  Do this first so leaf WMEs are
            //deallocated first.
            epmem_clear_mem(child);

            //Remove from WM
            remove_input_wme(child->assoc_wme);
            wme_remove_ref(child->assoc_wme);

            //Bookkeeping
            child->assoc_wme = NULL;
            child->in_mem = FALSE;
        }//for
    }//for
    
}//epmem_clear_mem

/* ===================================================================
   compare_memories

   Compares two episodic memories and returns the number of *leaf
   WMEs* they have in common.  Obviously both lists should reference
   the same wmetree for this comparison to be useful.

   This function ignores WMEs that are not leaf WMEs.
   
   Created: 23 Feb 2004
   =================================================================== */
int compare_memories(arraylist *epmem1, arraylist *epmem2)
{
    int count = 0;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < epmem1->size) && (pos2 < epmem2->size))
    {
        wmetree *node1 = ((actwme *)epmem1->array[pos1])->node;
        wmetree *node2 = ((actwme *)epmem2->array[pos2])->node;
        
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
int compare_memories_ideal(arraylist *epmem1, arraylist *epmem2)
{
    wmetree *node1;
    wmetree *node2;
    wmetree *tmp1;
    wmetree *tmp2;
    char *direction = NULL;

    //Compare the direction eater is travelling
    node1 = epmem_find_memory_entry(epmem1, &g_wmetree, "output-link");
    node1 = epmem_find_memory_entry(epmem1, node1, "move");
    node1 = epmem_find_memory_entry(epmem1, node1, "direction");
    node2 = epmem_find_memory_entry(epmem2, &g_wmetree, "output-link");
    node2 = epmem_find_memory_entry(epmem2, node2, "move");
    node2 = epmem_find_memory_entry(epmem2, node2, "direction");
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
    node1 = epmem_find_memory_entry(epmem1, &g_wmetree, "input-link");
    node1 = epmem_find_memory_entry(epmem1, node1, "my-location");
    node2 = epmem_find_memory_entry(epmem2, &g_wmetree, "input-link");
    node2 = epmem_find_memory_entry(epmem2, node2, "my-location");

    //Compare destination cell content
    tmp1 = epmem_find_memory_entry(epmem1, node1, direction);
    tmp1 = epmem_find_memory_entry(epmem1, tmp1, "content");
    tmp2 = epmem_find_memory_entry(epmem2, node2, direction);
    tmp2 = epmem_find_memory_entry(epmem2, tmp2, "content");
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
int compare_memories_act_indiv_mem(arraylist *epmem1, arraylist *epmem2)
{
    int count = 0;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < epmem1->size) && (pos2 < epmem2->size))
    {
        wmetree *node1 = ((actwme *)epmem1->array[pos1])->node;
        wmetree *node2 = ((actwme *)epmem2->array[pos2])->node;
        
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
            count += ((actwme *)epmem1->array[pos1])->activation;
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
int find_best_match(arraylist *cue)
{
    int best_match = 0;
    int best_index = 0;
    int n;
    int i;

    //If there aren't enough memories to examine just return
    //the first one
    if (g_memories->size <= memory_match_wait)
    {
        return 0;
    }
    
    for(i = 0; i < g_memories->size - memory_match_wait; i++)
    {
        n = compare_memories_act_indiv_mem((arraylist *)g_memories->array[i],
                                           cue);
        if (n >= best_match)
        {
            best_index = i;
            best_match = n;
        }
    }

    //Check for no match found
    if (best_match == 0) return -1;

    return best_index;
}//find_best_match

/* ===================================================================
   iwiwm_helper                *RECURSIVE*

   helper function for install_epmem_in_wm.

   Created: 20 Jan 2004
   =================================================================== */
void iwiwm_helper(Symbol *id, wmetree *node)
{
    wmetree *child;
    unsigned long hash_value;
    Symbol *attr;
    Symbol *val;

    //Create the wmes attached to sym
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            if (! child->in_mem) continue;
            //For now, avoid recursing into previous memories.
            if (strcmp(child->attr, "epmem") == 0) continue;
            if (child->assoc_wme != NULL)
            {
                //%%%TODO: I think this is an error.  What do I do about it?
            }
        
            attr = make_sym_constant(child->attr);

            switch(child->val_type)
            {
                case SYM_CONSTANT_SYMBOL_TYPE:
                    val = make_sym_constant(child->val.strval);
                    break;
                case INT_CONSTANT_SYMBOL_TYPE:
                    val = make_int_constant(child->val.intval);
                    break;
                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    val = make_float_constant(child->val.floatval);
                    break;

                default:
                    val = make_new_identifier(child->attr[0],
                                              id->id.level);
                    break;
            }//switch

            child->assoc_wme = add_input_wme(id, attr, val);
            wme_add_ref(child->assoc_wme);

            //Recursively create wmes for children's children
            iwiwm_helper(val, child);

        }//for
    }//for
    
}//iwiwm_helper

/* ===================================================================
   install_epmem_in_wm

   Given an episodic memory this function recreates the working memory
   fragment represented by the memory in working memory.  The
   retrieved memory is rooted at the the given symbol.

   Created:  19 Jan 2004
   =================================================================== */
void install_epmem_in_wm(Symbol *sym, arraylist *epmem)
{
    int i;

    //set the in_mem flag on all wmes in the wmelist
    for(i = 0; i < epmem->size; i++)
    {
        ((actwme *)epmem->array[i])->node->in_mem = TRUE;
    }

    //Use the recursive helper function to create the wmes
    iwiwm_helper(sym, &(g_wmetree));
    
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
arraylist *respond_to_query(Symbol *query, Symbol *retrieved)
{
    arraylist *al_query;
    arraylist *al_retrieved;
    tc_number tc;

    //Verify that there is at least one memory to retrieve
    if (g_memories->size < 1) return NULL;
    
    //Remove the old retrieved memory
    epmem_clear_mem(&g_wmetree);
    g_curr_memory = -1;

    //Create an arraylist representing the current query
    al_query = make_arraylist(32);
    tc = query->id.tc_num + 1;
    update_wmetree(&g_wmetree, query, al_query, tc);

    //If the query is empty then we're done
    if (al_query->size == 0)
    {
        destroy_arraylist(al_query);
        return NULL;
    }

    //Match query to current memories list
    g_curr_memory = find_best_match(al_query);
    destroy_arraylist(al_query);

    //Place the best fit on the retrieved link
    if (g_curr_memory > -1)
    {
        al_retrieved = (arraylist *)g_memories->array[g_curr_memory];
        install_epmem_in_wm(retrieved, al_retrieved);
    }
    else
    {
        al_retrieved = NULL;

        //Notify the user of failed retrieval
        g_wmetree.assoc_wme = add_input_wme(retrieved,
                                             make_sym_constant("no-retrieval"),
                                             make_sym_constant("true"));
        wme_add_ref(g_wmetree.assoc_wme);
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
   =================================================================== */
char *epmem_retrieve_command()
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    char *ret = NULL;
    
    tc = g_epmem_query->id.tc_num + 1;
    wmes = get_augs_of_id(g_epmem_query, tc, &len);
    g_epmem_query->id.tc_num = tc - 1;
    if (wmes == NULL) return NULL;

    for(i = 0; i < len; i++)
    {
        if ( (wme_has_value(wmes[i], "command", NULL))
             && (wmes[i]->value->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) )
        {
            ret = wmes[i]->value->sc.name;
        }
    }

    return ret;
}//epmem_retrieve_command

/* ===================================================================
   increment_retrieval_count

   Increments the value stored in ^epmem.retieval-count by a positive
   integer. If the user passes a zero value then the present value is
   reset to 1.  If the user passes a negative value then the present
   value is removed from working memory.

   27 Jan 2004
   =================================================================== */
void increment_retrieval_count(long inc_amt)
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    long current_count = 0;
    wme *w;

    //Find the (epmem ^retreival-count n) WME, save the value,
    //and remove the WME from WM
    tc = g_epmem_header->id.tc_num + 1;
    wmes = get_augs_of_id( g_epmem_header, tc, &len );
    g_epmem_header->id.tc_num = tc - 1;
    
    if (wmes == NULL) return;
    for(i = 0; i < len; i++)
    {
        if ( (wme_has_value(wmes[i], "retrieval-count", NULL))
             && (wmes[i]->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) )
        {
            current_count = wmes[i]->value->ic.value;
            remove_input_wme(wmes[i]);
            break;
        }
    }

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
    w = add_input_wme(g_epmem_header,
                      make_sym_constant("retrieval-count"),
                      make_int_constant(current_count));
    wme_add_ref(w);


}//increment_retrieval_count

/* ===================================================================
   respond_to_command() 

   This routine responds to agent commands given on the ^epmem link.
   The following commands are supported:
       "next" - populate ^epmem.retrieved with the next memory
                in the sequence

   Created: 27 Jan 2004
   =================================================================== */
void respond_to_command(char *cmd)
{
    if (strcmp(cmd, "next") == 0)
    {
        //Remove the old retrieved memory
        epmem_clear_mem(&g_wmetree);

        //Check that there is a next memory available
        if ( (g_curr_memory > -1)
             && (g_curr_memory < g_memories->size - memory_match_wait) )
        {
            //Install the new memory
            g_curr_memory++;
            install_epmem_in_wm(g_epmem_retrieved,
                                (arraylist *)g_memories->array[g_curr_memory]);
        }
        else
        {
            //Notify the user of failed retrieval
            g_wmetree.assoc_wme = add_input_wme(g_epmem_retrieved,
                                                make_sym_constant("no-retrieval"),
                                                make_sym_constant("true"));
            wme_add_ref(g_wmetree.assoc_wme);
        }
        
        increment_retrieval_count(1);
    }
    
}//respond_to_command


/* ===================================================================
   epmem_print_curr_memory()           *DEBUGGING*

   Prints information about what's currently on the ^epmem link.

   Created: 20 Feb 2004
   =================================================================== */
void epmem_print_curr_memory()
{
    if (g_curr_memory < 0)
    {
        print("\nCURRENT MEMORY:  None.\n");
        return;
    }
    
    //Print the current memory
    print("\nCURRENT MEMORY:  %d of %d\t\t", g_curr_memory, g_memories->size);
    print_memory_graphically((arraylist *)g_memories->array[g_curr_memory]);

}//epmem_print_curr_memory


/* ===================================================================
   epmem_update() 

   This routine is called at every output phase to allow the episodic
   memory system to update its current memory store and respond to any
   queries. 

   Created: 19 Jan 2004
   =================================================================== */
void epmem_update()
{
    char *cmd;
    arraylist *epmem = NULL;

    //Consider recording a new epmem
    consider_new_epmem_via_output();

    //Update the ^retrieved link as necessary
    cmd = epmem_retrieve_command();
    if (cmd != NULL)
    {
        respond_to_command(cmd);
    }
    else
    {
        //%%%DEBUGGING
        //decay_print_most_activated_wmes(50);
        
        epmem = respond_to_query(g_epmem_query, g_epmem_retrieved);
        if (epmem != NULL)
        {
            //New retrieval:  reset count to zero
            increment_retrieval_count(0);
        }
        else
        {
            //No retrieval:  remove count from working memory
            increment_retrieval_count(-1);
        }
    }

    //%%%DEBUGGING
    epmem_print_curr_memory();

}//epmem_update

/* ===================================================================
   epmem_create_buffer()

   This routine creates the ^epmem link on a given state.  This is
   used to query the episodic memory and provided the retrieved
   result.

   Created: 08 Jan 2004
   =================================================================== */

void epmem_create_buffer(Symbol *s)
{
    wme *w;
    
    //Add the ^epmem header WMEs
    g_epmem_header = make_new_identifier('E', s->id.level);
    g_epmem_query = make_new_identifier('E', s->id.level);
    g_epmem_retrieved = make_new_identifier('E', s->id.level);

    w = add_input_wme(s, make_sym_constant("epmem"), g_epmem_header);
    wme_add_ref(w);
    w = add_input_wme(g_epmem_header, make_sym_constant("query"), g_epmem_query);
    wme_add_ref(w);
    w = add_input_wme(g_epmem_header, make_sym_constant("retrieved"), g_epmem_retrieved);
    wme_add_ref(w);

    
}//epmem_create_buffer


/* ===================================================================
   init_epemem()

   This routine is called once to initialize the episodic memory system.

   Created: 03 Nov 2002
   =================================================================== */

void init_epmem(void) 
{
    //Allocate the active wmes arrays
    g_current_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));
    g_previous_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));

    //Initialize the wmetree
    g_wmetree.next = NULL;
    g_wmetree.attr = NULL;
    g_wmetree.val.intval = 0;
    g_wmetree.val_type = IDENTIFIER_SYMBOL_TYPE;
    g_wmetree.children = make_hash_table(0, hash_wmetree);;
    g_wmetree.parent = NULL;
    g_wmetree.assoc_wme = NULL;

    //Initialize the memories array
    g_memories = make_arraylist(512);
    g_curr_memory = -1;

    
}/*init_epmem*/




#endif /* AMN end */

/* ===================================================================
   =================================================================== */
