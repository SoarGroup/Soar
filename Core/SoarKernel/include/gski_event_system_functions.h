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

#ifndef GSKI_EVENT_SYSTEM_FUNCTIONS_H
#define GSKI_EVENT_SYSTEM_FUNCTIONS_H

/*********************************** INLINE FUNCTIONS *****************************************/
/* Gotta include these after the other stuff to keep circular includes from happening. */
#include "agent.h"
#include "xmlTraceNames.h"

using namespace xmlTraceNames;

/**
 * @brief Initialize the agent callbacks
 *
 * This method is called when an agent is initialized.  It clears out the list
 *  of agent callback listeners.
 *
 * @param soarAgent The agent for which to clear the callbacks.
 */
inline void gSKI_InitializeAgentCallbacks(agent* soarAgent)
{
   assert(soarAgent != 0 && "Cannot initialize callbacks on a NULL agent pointer.");
   if(soarAgent)
      memset(soarAgent->gskiCallbacks, 0, sizeof(gSKI_K_CallbackData) * gSKI_K_MAX_AGENT_EVENTS);
}


/**
 * @brief Sets an agent callback
 *
 * Sets the callback for a particular agent event
 * (Only used for gSKI)
 *
 * @param soarAgent   Pointer to the agent for which to set the callback.
 * @param eventId     Id of the event for which to set the callback.
 * @param gski_object GSKI object that will recieve the callback.
 * @param functionPtr Pointer to the function that will recieve the callback.
 *
 */
inline void gSKI_SetAgentCallback(agent*                     soarAgent, 
                                  unsigned long              eventId, 
                                  void*                      gski_object, 
                                  gSKI_K_CallbackFunctionPtr functionPtr)
{
   soarAgent->gskiCallbacks[eventId].gski_object = gski_object;
   soarAgent->gskiCallbacks[eventId].function = functionPtr;
}

/**
 * @brief Makes an agent callback
 *
 * This function checks the callback for the given data and calls back the listening
 *  gSKI object with the event data.
 *
 * @param eventId       One of the egSKIAgentEvents enumerated values
 * @param eventOccured  1 (true) if the event given by eventId has already
 *                       occured, 0 (false) if it has not.
 * @param soarKernel    Soar kernel object that contains the given agent
 * @param soarAgent     The soar agent that is related to the event
 * @param data          Event specific data
 */
 
inline void gSKI_MakeAgentCallback(unsigned long eventId, 
                                   unsigned char eventOccured, 
                                   agent*        soarAgent,
                                   void*         data)
{
   /* Only make a callback if someone is looking */
   if(soarAgent->gskiCallbacks[eventId].gski_object != 0)
   {
       assert(soarAgent->gskiCallbacks[eventId].function != 0 && "Callback object valid but with invalid function.");
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   
	   soarAgent->gskiCallbacks[eventId].function(eventId, eventOccured, soarAgent->gskiCallbacks[eventId].gski_object, soarAgent, data);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
  }
}

/**
 * @brief Special function for more complex WMObject added method.
 */
// !!!KJC :  can this go away?  Can Soar callback create struct?  Is it used?
inline void gSKI_MakeAgentCallbackWMObjectAdded(struct agent_struct* soarAgent,
                                                Symbol*              new_object,
                                                Symbol*              ref_attr,
                                                Symbol*              ref_object)
{
   gSKI_K_WMObjectCallbackData wmobject_data;

   wmobject_data.wm_new_object         = new_object;
   wmobject_data.wm_referencing_attr   = ref_attr;
   wmobject_data.wm_referencing_object = ref_object; 

   /* JC ADDED: Tell gSKI we have a new object in general (there are three places this can occur). */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_WMOBJECT_ADDED, 1, soarAgent, static_cast<void*>(&wmobject_data));
}

/**
 * @brief Special function to handle the phase and decision cycle callbacks
 */
// !!! KJC: this should go away.  Use Soar callbacks and create struct in handler.
inline void gSKI_MakeAgentCallbackPhase(struct agent_struct* soarAgent,
                                        egSKIAgentEvents     event_type,
                                        egSKIPhases          phase_type,
                                        unsigned char        eventOccured)
{
   gSKI_K_PhaseCallbackData phase_data;

   phase_data.phase_type              = phase_type;
   phase_data.decision_cycles         = soarAgent->d_cycle_count;
   phase_data.elaboration_cycles      = soarAgent->e_cycle_count;
   phase_data.elaborations_this_phase = soarAgent->e_cycles_this_d_cycle;

   gSKI_MakeAgentCallback(event_type, eventOccured, soarAgent, static_cast<void*>(&phase_data));
}

/**
 * @brief Special functions to handle the xml callback
 */

inline void gSKI_MakeAgentCallbackXML(	agent*		soarAgent,
                                        const char*	funcType,
                                        const char*	attOrTag,
										const char*	value=0)
{
   gSKI_K_XMLCallbackData xml_data;
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   
   xml_data.funcType = funcType;
   xml_data.attOrTag = attOrTag;
   xml_data.value = value;

   gSKI_MakeAgentCallback(gSKI_K_EVENT_XML_OUTPUT, 0, soarAgent, static_cast<void*>(&xml_data));
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void gSKI_MakeAgentCallbackXML(	agent*			soarAgent,
                                        const char*		funcType,
                                        const char*		attOrTag,
										unsigned long	value)
{
	char buf[25];
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   

	snprintf(buf, 24, "%lu", value);
	gSKI_MakeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void gSKI_MakeAgentCallbackXML(	agent*		soarAgent,
                                        const char*	funcType,
                                        const char*	attOrTag,
										double      value)
{
	char buf[25];
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   
	snprintf(buf, 24, "%f", value);
	gSKI_MakeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void GenerateWarningXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionBeginTag, kTagWarning);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionAddAttribute, kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionEndTag, kTagWarning);
}

inline void GenerateErrorXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionBeginTag, kTagError);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionAddAttribute, kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionEndTag, kTagError);
}

inline void GenerateMessageXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionBeginTag, kTagMessage);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionAddAttribute, kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionEndTag, kTagMessage);
}

inline void GenerateVerboseXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionBeginTag, kTagVerbose);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionAddAttribute, kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, kFunctionEndTag, kTagVerbose);
}

///**
// * @brief Initialize the kernel callbacks
// *
// * This method is called when the kernel object is created.  It initializes
// *  the kernel callback list to empty.
// *
// * @param soarKernel Pointer to the kernel object containing the callback list
// */
//inline void gSKI_InitializeKernelCallbacks(Kernel* soarKernel)
//{
//   assert(soarKernel != 0 && "Cannot initialize callbacks on a NULL agent pointer.");
//   if(soarKernel)
//      memset(soarKernel->gskiCallbacks, 0, sizeof(gSKI_K_CallbackData) * gSKI_K_MAX_AGENT_EVENTS);
//}
//
///**
// * @brief Sets a kernel callback
// *
// * Sets the callback for a particular kernel event.
// *  (Only used for gSKI)
// *
// * @param soarKernel  Pointer to the kernel instance for which to set the callback.
// * @param eventId     Id of the event for which to set the callback.
// * @param gski_object GSKI object that will recieve the callback.
// * @param functionPtr Pointer to the function that will recieve the callback.
// *
// */
//inline void gSKI_SetKernelCallback(Kernel*                     soarKernel, 
//                                   unsigned char               eventId, 
//                                   void*                       gski_object, 
//                                   gSKI_K_CallbackFunctionPtr  functionPtr)
//{
//   soarKernel->gskiCallbacks[eventId].gski_object = gski_object;
//   soarKernel->gskiCallbacks[eventId].function = functionPtr;
//}
//
///**
// * @brief Makes a kernel callback
// *
// * This function checks the callback for the given data and calls back the listening
// *  gSKI object with the event data.
// *
// * @param eventId       One of the egSKIAgentEvents enumerated values
// * @param eventOccured  1 (true) if the event given by eventId has already
// *                       occured, 0 (false) if it has not.
// * @param object        gSKI object that is managing this callback.
// * @param soarKernel    Soar kernel object that triggered or relates to the event
// * @param soarAgent     The soar agent that is related to the event (sometime 0)
// * @param data          Event specific data
// */
//inline void gSKI_MakeKernelCallback(unsigned char  eventId, 
//                                    unsigned char  eventOccured, 
//                                    void*          object,
//                                    Kernel*        soarKernel,
//                                    agent*         soarAgent,
//                                    void*          data)
//{
//   /* Only make a callback if someone is looking */
//   if(soarKernel->gskiCallbacks[eventId].gski_object != 0)
//   {
//      assert(soarKernel->gskiCallbacks[eventId].function != 0 && "Callback object valid but with invalid function.");
//      soarKernel->gskiCallbacks[eventId].function(eventId, eventOccured, object, soarKernel, soarAgent, data);
//   }

#endif

