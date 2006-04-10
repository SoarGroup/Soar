/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                wmem.h 

                Working Memory Management and Utility Routines

   Reset_wme_timetags() resets the wme timetag generator back to 1.
   This should be called during an init-soar.

   Make_wme() creates and returns a new wme.  The caller should add the
   wme onto the appropriate dll (e.g., my_slot->wmes) and should call
   add_wme_to_wm() on it.

   Add_wme_to_wm() and remove_wme_from_wm() make changes to WM.  Again,
   the caller is responsible for manipulating the appropriate dll.  WM
   changes don't actually get stuffed down the rete until the end of the
   phase, when do_buffered_wm_and_ownership_changes() gets be called.

   Remove_wme_list_from_wm() is a utility routine that scans through a
   list of wmes, linked by their "next" fields, and calls remove_wme_from_wm()
   on each one.

   Wme_add_ref() and wme_remove_ref() are macros for incrementing and
   decrementing the reference count on a wme.  Deallocate_wme() deallocates
   a wme; this should only be invoked via the wme_remove_ref() macro.

   Find_name_of_object() is a utility function for finding the value of
   the ^name attribute on a given object (symbol).  It returns the name,
   or NIL if the object has no name.
======================================================================= */

#ifndef WMEM_H
#define WMEM_H

#include "kernel.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned long tc_number;
typedef struct wme_struct wme;
typedef struct agent_struct agent;
typedef union symbol_union Symbol;

extern void reset_wme_timetags (agent* thisAgent);
extern wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value,Bool acceptable);
extern void add_wme_to_wm (agent* thisAgent, wme *w);
extern void remove_wme_from_wm (agent* thisAgent, wme *w);
extern void remove_wme_list_from_wm (agent* thisAgent, wme *w);
extern void do_buffered_wm_changes (agent* thisAgent);

extern void deallocate_wme (agent* thisAgent, wme *w);
extern Symbol *find_name_of_object (agent* thisAgent, Symbol *id);

/* ------------------------------------------------------------------------
                      Working Memory Elements (WMEs)

   Fields in a WME:

      id, attr, value:  points to symbols for the wme fields

      acceptable:  TRUE iff this is an acceptable pref. wme

      timetag:  timetag of the wme

      reference count:  (see below)

      rete_next, rete_prev:  pointers in the doubly-linked list of all
         wmes currently known to the rete (header is all_wmes_in_rete)
         (this equals WM except while WM is being changed)

      right_mems:  header of a doubly-linked list of right memory entries
         (in one or more alpha memories containing the wme).  This is used
         only by the Rete, as part of list-based remove.

      tokens:  header of a doubly-linked list of tokens in the Rete.
         This is used only by the Rete, as part of list-based remove.

      next, prev:  pointers in a doubly-linked list of wmes.
         Depending on the wme type, the header of this DLL is:
           - slot.wmes (for ordinary wmes)
           - slot.acceptable_preference_wmes (for acceptable pref. wmes)
           - id.impasse_wmes (for architecture-created goal/impasse wmes)
           - id.input_wmes (for Soar I/O wmes)

      preference:  points to the preference supporting the wme.  For I/O
         wmes and (most) architecture-created wmes, this is NIL.

      output_link:  this is used only for top-state output links.
         It points to an output_link structure used by the I/O routines.

      grounds_tc, potentials_tc, locals_tc:  used by the chunker to indicate
         whether this wme is in the grounds, potentials, and/or locals sets

      chunker_bt_pref: used by the chunker; set to cond->bt.trace when
         a wme is added to either the potentials or locals set

      These are the additions to the WME structure that will be used
         to track dependencies for goals.  Each working memory element
	 now includes a pointer  to a gds_struct (defined below) and
	 pointers to other WMEs on the same GDS.

      gds: the goal dependency set the wme is in
      gds_next, gds_prev:  used for dll of all wmes in gds

      If a particular working memory element is not dependent for any goal,
	 then the values for these pointers will all be NIL. If a WME is
	 dependent for more than one goal, then it will point to the GDS
	 of the highest goal.
	

      

   Reference counts on wmes:
      +1 if the wme is currently in WM
      +1 for each instantiation condition that points to it (bt.wme)
   We deallocate a wme when its reference count goes to 0.
------------------------------------------------------------------------ */

typedef struct wme_struct {
  /* WARNING:  The next three fields (id,attr,value) MUST be consecutive--
     the rete code relies on this! */
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  Bool acceptable;
  unsigned long timetag;
  unsigned long reference_count;
  struct wme_struct *rete_next, *rete_prev; /* used for dll of wmes in rete */
  struct right_mem_struct *right_mems;      /* used for dll of rm's it's in */
  struct token_struct *tokens;              /* dll of tokens in rete */
  struct wme_struct *next, *prev;           /* (see above) */
  struct preference_struct *preference;     /* pref. supporting it, or NIL */
  struct output_link_struct *output_link;   /* for top-state output commands */
  tc_number grounds_tc;                     /* for chunker use only */
  tc_number potentials_tc, locals_tc;
  struct preference_struct *chunker_bt_pref;

  /* REW: begin 09.15.96 */
  struct gds_struct *gds;
  struct wme_struct *gds_next, *gds_prev; /* used for dll of wmes in gds */
  /* REW: end   09.15.96 */

#ifdef SOAR_WMEM_ACTIVATION
    /* MRJ 5/23/01 */
    struct wme_decay_element_struct *decay_element;
    bool has_decay_element;
    /* MRJ end */
#endif
    
    
} wme;

#ifdef USE_MACROS

#define wme_add_ref(w) { (w)->reference_count++; }
#define wme_remove_ref(thisAgent, w) { \
  if ((w)->reference_count != 0) (w)->reference_count--;   \
  if ((w)->reference_count == 0) deallocate_wme(thisAgent, w); }

#else

inline void wme_add_ref(wme * w) { 
   (w)->reference_count++; 
}
inline void wme_remove_ref(agent* thisAgent, wme * w)
{
  /* There are occaisionally wme's with zero reference counts 
     created in the system. Make sure this function handles them 
     correctly. */
  if ((w)->reference_count != 0) (w)->reference_count--;
  if ((w)->reference_count == 0) deallocate_wme(thisAgent, w);
}

#endif /* USE_MACROS */


#ifdef __cplusplus
}
#endif

#endif
