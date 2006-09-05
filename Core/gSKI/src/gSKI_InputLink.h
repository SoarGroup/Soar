/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_inputlink.h 
*********************************************************************
* created:	   6/13/2002   14:53
*
* purpose: 
*********************************************************************/
#ifndef GSKI_INPUTLINK_H
#define GSKI_INPUTLINK_H

#include "IgSKI_InputLink.h"

#include "gSKI_InputWorkingMemory.h"
#include "gSKI_InputWMObject.h"

#include "callback.h"

namespace gSKI {

  class Agent;

  /**
   * @brief Handles registration and invokation of InputProducers and 
   *         WmeReplacementPolicies
   *
   * The IInputLink interface is mainly responsible for handling registration
   * and invocation of the InputProducer objects. Registration is done
   * through the AddInputProducer() and AddReplacementPolicy() methods,
   * which essentially associate an InputProducer with a WMObject or a 
   * WmeReplacementPolicy with a Wme. The InputProducers are responsible 
   * for adding and removing WMObjects and Wmes to the input link while the 
   * WmeReplacementPolicies essentially determine if a request for a Wme
   * replacement is performed or not.
   */
  // TODO: Still need to address automatic vs. manual update issues
  class InputLink: public IInputLink {
  public:
    /**
     * @brief Constructor
     *
     * This constructor is responsible for creating the input link object
     * and registering it to receive the appropriate callbacks from soar
     * kernel. A pointer to the owning agent is passed in to provide information
     * for the callback registration process.
     *
     * @param agent The agent that owns the input link.
     */
    InputLink(Agent* agent);

    /**
     * @brief Virtual Destructor
     *
     * The InputLink is wholly owned by the Agent object. The destructor
     * should never be called by SSI developers.
     */
    ~InputLink();
      
    /**
     * @brief 
     *
     */
    //virtual void UpdateInputLink();

    /**
     * @brief
     *
     */
    //virtual void SetAutomaticUpdate();

    /**
     * @brief Returns aa pointer to the root WMObject on the input link
     *
     * This method returns a pointer to the root WMObject on the input link.
     * This is a convenience method that may help with navigating the agent's
     * input link.
     *
     * @param rootObject A pointer to the root WMObject pointer.
     *          Used to hold the return value. Returning the actual
     *          pointer from this method would make it too easy to
     *          create memory leaks since the pointer has to be
     *          released.
     * @param err Pointer to client-owned error structure.  If the pointer
     *        is not 0 this structure is filled with extended error
     *        information.  If it is 0 (the default) extended error
     *        information is not returned.
     *
     * @returns the root WMObject of the input link
     */
    void GetRootObject(IWMObject** rootObject, Error* err = 0);

    /**
     * @brief Returns the IWorkingMemory object associated with the 
     *         input link.
     *
     * This method returns a pointer to the IWorkingMemory object 
     * associated with the InputLink. This IWorkingMemory object allows Wmes
     * and WMObjects to be easily added, removed and replaced. It also
     * provides convenience methods for searching for Wmes and obtaining
     * static snap shots of the agent's working memory data (on the input
     * link).
     *
     * @param err Pointer to client-owned error structure.  If the pointer
     *        is not 0 this structure is filled with extended error
     *        information.  If it is 0 (the default) extended error
     *        information is not returned.
     *
     * @returns The IWorkingMemory object associated with the data on 
     *           the agent's input link
     */
    IWorkingMemory* GetInputLinkMemory(Error* err = 0);

    /**
     * @brief Registers an InputProducer with a WMObject on the agent's
     *         input link
     *
     * This method registers an InputProducer with a WMObject on the agent's
     * input link. This InputProducer is responsible for managing the
     * WMObject's Wmes (and possibly any sub WMObjects, this is an SSI 
     * design decision) that are used to represent the simulation state
     * to the Soar agent. If the specified WMObject is invalid then an 
     * error will be returned.
     *
     * @param object The WMObject with which to associate the InputProducer
     * @param producer The InputProduer to associate with the WMObject.
     * @param err Pointer to client-owned error structure.  If the pointer
     *        is not 0 this structure is filled with extended error
     *        information.  If it is 0 (the default) extended error
     *        information is not returned.
     */
    void AddInputProducer(IWMObject* object,
                          IInputProducer* producer,
                          Error* err = 0);

    /**
     * @brief Unregisters an InputProducer from a WMObject
     *
     * This method unregisters an InputProducer from a WMObject on the 
     * agent's input link. Once the InputProducer is unregistered it is
     * wholly owned by the SSI developer and should be cleaned up 
     * appropriately. If the specified WMObject is invalid or the 
     * specified InputProducer cannot be found the an error will be 
     * returned.
     *
     * @param object The WMObject from which to unregister the InputProducer
     * @param producer The InputProducer to unregister from the WMObject
     * @param err Pointer to client-owned error structure.  If the pointer
     *        is not 0 this structure is filled with extended error
     *        information.  If it is 0 (the default) extended error
     *        information is not returned.       
     */
    void RemoveInputProducer(IWMObject* object,
                             IInputProducer* producer,
                             Error* err = 0);

      /**
       * @brief Reinitializes the input link
       *
       * This method reinitializes the input link by releasing all the wmes
       * and wmobjects held by the input link. Since the input producers are
       * associated with the input wm objects they need to be reconstructed
       * after reinitialization.
       *
       */
    // TODO: InputWMObjects could probably be modified so that the underlying 
    // symbols can be changed without destroying the WMObject. This should make
    // it so reinitializing the input link doesn't require redefining the node
    // structure (although that shouldn't be too hard either).
      void Reinitialize() { 
         //  if ( m_rootObject != 0 ) m_rootObject->Release();
         //  m_rootObject = 0;
        m_memory.Reinitialize(); 
      }

   private:
    /**
     * @brief Static callback function that handles dispatching raw kernel
     *  callbacks to the various input link objects.
     *
     * This method serves as the central dispatching function for the raw
     * kernel input link callbacks. The soar_callback_data recieved by this 
     * function is actually a pointer to the appropriate InputLink object.
     */
    static void InputPhaseCallback( soar_callback_agent agent,
                                    soar_callback_data callbackdata,
                                    soar_call_data calldata );
    
    
    /**
     * @brief Method called when the input link is created by the
     *  Soar kernel
     *
     * This method is called by the InputPhaseCallback when the callback
     * is type TOP_STATE_JUST_CREATED.
     */
    void InitialUpdate();

    /**
     * @brief Method called when the during a normal input cycle
     *
     * This method is called by the InputPhaseCallback when the callback
     * is type NORMAL_INPUT_CYCLE.
     */
    void Update();

    /**
     * @brief Method called when the input link is created by the
     *  Soar kernel
     *
     * This method is called by the InputPhaseCallback when the callback
     * is type TOP_STATE_JUST_REMOVED.
     */
    void FinalUpdate();

    Agent* m_agent;  /**< The agent that owns this input link. */
    InputWorkingMemory m_memory; /**< The working memory object that handles input working memory */
/*     InputWMObject* m_rootObject; */
    
  };
}


#endif
