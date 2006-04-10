/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_event_system_kernel.h 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#ifndef GSKI_EVENT_SYSTEM_KERNEL_H
#define GSKI_EVENT_SYSTEM_KERNEL_H

/**
 * gski_event_system_kernel.h/c defines the event propagation routines that 
 *  tie the kernel to the gSKI project.
 *
 * These events and the data in them do not necessarily correspond to
 *  the events, callbacks, and data in the gSKI event system.  These
 *  events callback into gSKI, where gSKI translates the event into
 *  gSKI style listener events and notifies listeners.
 *
 * !!!  In fact, the only events propagated to gSKI as of Soar 8.6, are:
 *			 gSKI_K_EVENT_XML_OUTPUT,
 *           gSKI_K_EVENT_PRINT_CALLBACK,
 *           gSKI_K_EVENT_PRODUCTION_* events
 *      gSKI doesn't register for any other event.  Instead, it mirrors
 *      the generation of these events within its own scheduler and makes
 *      assumptions about Soar's execution cycle.  (KJC June 2005)
 *  
 * Only one object at a time can listen to any given event in the kernel.
 *  Propagation to listeners is handled by the gSKI objects.
 */

/*
 * Kernel events are essentially global system events.  The callback
 *  functions for these cannot be stored in the agent structure because
 *  they are not associated with an existing agent.
 *
 * These shoudl be stored in a kernel object (as opposed to global data).
 */
// THESE CAN ALL BE PROPAGATED FROM GSKI!!
//enum egSKIKernelEvents
//{
//   gSKI_K_EVENT_KERNEL_SHUTDOWN = 0,      /* Data: Kernel* (when available) */
//   gSKI_K_EVENT_KERNEL_RESTART,           /* Data: Kernel* (when available) */
//   gSKI_K_EVENT_AGENT_ADDED,              /* Data: agent*                   */
//   gSKI_K_EVENT_AGENT_REMOVED,            /* Data: agent*                   */ 
//   gSKI_K_EVENT_THREAD_GROUP_RUN,         /* Data: Kernel* ??               */ 
//   gSKI_K_EVENT_THREAD_GROUP_STOP,        /* Data: Kernel* ??               */
//   gSKI_K_EVENT_RHS_FUNC_ADDED,           /* Data: Kernel* & rhs function   */ 
//   gSKI_K_EVENT_RHS_FUNC_REMOVED,         /* Data: Kernel* & rhs function   */ 
//   gSKI_K_EVENT_RHS_FUNC_EXECUTED,        /* Data: Kernel* & rhs function & sym params? & ret val? */
//
//   /* Must be the last value in this enumeration */
//   gSKI_K_MAX_KERNEL_EVENTS       
//};

/*
 * All of the events that are fired for the agent.
 */
enum egSKIAgentEvents
{
   /* Do we need WME preference added? */
   gSKI_K_EVENT_WME_ADDED = 0,            /* Data: wme*              */          // DONE!
   gSKI_K_EVENT_WME_REMOVED,              /* Data: wme*              */          // DONE!
   gSKI_K_EVENT_WMOBJECT_ADDED,           /* Data: symbol* & wme*    */          // DONE!
   gSKI_K_EVENT_WMOBJECT_REMOVED,         /* Data: symbol* & 0       */          // DONE!
   gSKI_K_EVENT_PRODUCTION_ADDED,         /* Data: production*       */          // DONE!
   gSKI_K_EVENT_PRODUCTION_REMOVED,       /* Data: production*       */          // DONE!
   gSKI_K_EVENT_PRODUCTION_FIRED,         /* Data: instantiation* */             // DONE!
   gSKI_K_EVENT_PRODUCTION_RETRACTED,     /* Data: instantiation* */             // DONE!
   gSKI_K_EVENT_OPERATOR_PROPOSED,        /* Data: Symbol* (state) & Symbol* (operator) */                     // DONE!
   gSKI_K_EVENT_OPERATOR_SELECTED,        /* Data: Symbol* (state) & Symbol* (operator) */                     // DONE!
   gSKI_K_EVENT_OPERATOR_RETRACTED,       /* Data: Symbol* (state) & Symbol* (operator) */                     // DONE!
   gSKI_K_EVENT_OPERATOR_PREF_ADDED,      /* Data: Symbol* (state) & Symbol* (operator) & Pref Type (char) */  // DONE!
   gSKI_K_EVENT_GDS_WME_ADDED,            /* Data: Symbol* (state) & wme* */     // DONE!
   gSKI_K_EVENT_GDS_VIOLATED,             /* Data: wme* (contains gds!) */       // DONE!
   gSKI_K_EVENT_SUBSTATE_CREATED,         /* Data: Symbol* (state) */            // DONE!
   gSKI_K_EVENT_SUBSTATE_DESTROYED,       /* Data: Symbol* (state) */            // DONE!
   gSKI_K_EVENT_PHASE,                    /* Data: Phase type (char) */          // DONE!
   gSKI_K_EVENT_ELABORATION_CYCLE,        /* Data: Phase type (char) & elab cycle count (int) ? */ // DONE!
   gSKI_K_EVENT_DECISION_CYCLE,           /* Data: cycle count (int) */          // DONE!
   
   // DON'T NEED, CAN BE PROPAGATED BY gSKI
   // gSKI_K_EVENT_RETE_LOADED,              
   // gSKI_K_EVENT_AGENT_INITIALIZED,  */    
   // gSKI_K_EVENT_AGENT_RUN_STARTED,        
   // gSKI_K_EVENT_AGENT_RUN_STOPPED,        

   /* NOT Deprecated.  gSKI not close to removing this yet.  */
   gSKI_K_EVENT_PRINT_CALLBACK,           /* Data: agent* & const char*     */   // DONE!

   gSKI_K_EVENT_XML_OUTPUT,        /* Data: agent* & const char* & const char* & const char*    */ 

   /* Must be the last value in this enumeration */
   gSKI_K_MAX_AGENT_EVENTS                
};

/**
 * List of the phases for the soar 8 kernel. 
 *  These are used for the phase callbacks.
 */
enum egSKIPhases
{
   gSKI_K_INPUT_PHASE = 0,
   gSKI_K_PROPOSAL_PHASE,
   gSKI_K_DECISION_PHASE,
   gSKI_K_APPLY_PHASE,
   gSKI_K_OUTPUT_PHASE,
   gSKI_K_PREFERENCE_PHASE,	// Soar 7 mode only
   gSKI_K_WM_PHASE 			// Soar 7 mode only
};

/* We have to forward declare these to keep from having a circular reference */
struct agent_struct;
union  symbol_union;

/**
 * @brief Very general callback function
 *
 * All callbacks are sent through this generic interface so the kernel can be completely
 *  unlinked from the gSKI implementation.
 *
 * @li The first parameter is the event id (one of those listed above)
 * @li The second parameter is an unsigned char that behaves like a boolean.  Its value is 1 (true)
 *         if the event already occured and 0 (false) if it is about to occur 
 *         (before event and after event flag)
 * @li The third parameter is the gSKI object that will recieve the callback.
 * @li The fourth parameter is a pointer to the internal "kernel" object.  This is 0 until the 
 *           internal kernel object is created.
 * @li The fifth parameter is the good old agent structure (this will be 0 for callbacks where
 *           an agent structure doesn't make sense)
 * @li The sixth parameter is the event specific data.
 *
 */
typedef void (*gSKI_K_CallbackFunctionPtr)(unsigned long         eventId, 
                                           unsigned char         eventOccured,
                                           void*                 object, 
                                           struct agent_struct*  soarAgent, 
                                           void*                 data);

/**
 * @brief Structure used to store callback functions and the object to recieve the callback.
 */
typedef struct gSKI_K_CallbackData_struct {
   
   /** Pointer to callback function.  If this is 0, a callback is not made */
   gSKI_K_CallbackFunctionPtr    function;

   /** Pointer to the gski object that will recieve teh callback */
   void*                         gski_object;

} gSKI_K_CallbackData;

/** ----------------------------- COMPLEXT EVENT DATA --------------------------------*/

/** 
 * @brief Structure for WMObject creation callback
 */
typedef struct gSKI_K_WMObjectCallbackData_struct {

   /** Pointer to the object being created */
   union symbol_union*           wm_new_object;

   /** Attribute used to reference new object. NIL for new state object. */
   union symbol_union*           wm_referencing_attr;

   /** Pointer to the object that is referencing the new object. NIL for new state object. */
   union symbol_union*           wm_referencing_object;

} gSKI_K_WMObjectCallbackData;

/** 
 * @brief Structure for decision cycle callbacks
 */
typedef struct gSKI_K_PhaseCallbackData_struct {

   /** Current phase of the decision cycle */
   egSKIPhases                   phase_type;

   /** Current decision cycle number */
   unsigned int                  decision_cycles;

   /** Current elaboration cycle number */
   unsigned int                  elaboration_cycles;

   /** Number of elaborations this cycle */
   unsigned int                  elaborations_this_phase;

} gSKI_K_PhaseCallbackData;

typedef struct gSKI_K_XMLCallbackData_struct {

   /** Current XML function type (i.e. addTag, endTag, addAttributeValuePair */
   const char*                  funcType;

   /** Current elaboration cycle number */
   const char*                  attOrTag;

   /** Number of elaborations this cycle */
   const char*                  value;

} gSKI_K_XMLCallbackData;

#endif
