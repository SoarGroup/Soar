/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_inputlink.h 
*********************************************************************
* created:	   6/13/2002   14:53
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_INPUTLINK_H
#define IGSKI_INPUTLINK_H

#include "IgSKI_Iterator.h"

namespace gSKI {

   // Forward Declarations
   class IWMObject;
   class IInputProducer;
   class IWorkingMemory;
   struct Error;

   /**
    * @brief Handles registration and invokation of InputProducers and 
    *         WmeReplacementPolicies
    *
    * The IInputLink interface is mainly responsible for handling registration
    * and invocation of the InputProducer objects. Registration is done
    * through the AddInputProducer() and AddWmeReplacementPolicy() methods,
    * which essentially associate an InputProducer with a WMObject or a 
    * WmeReplacementPolicy with a Wme. The InputProducers are responsible 
    * for adding and removing WMObjects and Wmes to the input link while the 
    * WmeReplacementPolicies essentially determine if a request for a Wme
    * replacement is performed or not.
    */
   // TODO: Still need to address automatic vs. manual update issues
   class IInputLink {
   public:
      /**
       * @brief Virtual Destructor
       *
       * The InputLink is wholly owned by the Agent object. The destructor
       * should never be called by SSI developers.
       */
     virtual ~IInputLink() {}
      
      /**
       * @brief 
       *
       */
      //virtual void UpdateInputLink() = 0;

      /**
       * @brief
       *
       */
      //virtual void SetAutomaticUpdate() = 0;

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
      virtual void GetRootObject(IWMObject** rootObject, Error* err = 0) = 0;

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
      virtual IWorkingMemory* GetInputLinkMemory(Error* err = 0) = 0;

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
      virtual void AddInputProducer(IWMObject* object,
                                    IInputProducer* producer,
                                    Error* err = 0) = 0;

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
      virtual void RemoveInputProducer(IWMObject* object,
                                       IInputProducer* producer,
                                       Error* err = 0) = 0;

   };
}


#endif
