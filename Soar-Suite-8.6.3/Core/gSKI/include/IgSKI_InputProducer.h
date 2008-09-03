/********************************************************************
* @file igski_inputproducer.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/21/2002   08:38
*
* purpose: 
********************************************************************/

#ifndef IGSKI_INPUT_PRODUCER_H
#define IGSKI_INPUT_PRODUCER_H

namespace gSKI {

   // Forward Declarations
   class IWorkingMemory;
   class IWMObject;
   
   /**
    * @brief Interface for automatically performing updates on WMObjects on
    *         the input link.
    *
    * This interface is used to automatically update the values of WMObjects 
    * on the input link. Instances of this interface are not produced by gSKI
    * but are instead implemented by integrators to handle the conversion of
    * simulation/external data to Soar input wme structures. InputProducers 
    * are registered with the input link and associated with a WMObject 
    * through the InputLink's AddInputProducer() method
    *
    * This interface is intended only to encapsulate the transformation of 
    * simulation data to Soar input wme values. The actual adding, removing
    * and replacing of Wmes is done through the IWorkingMemory interface.
    */
   class IInputProducer {
   public:
      /**
       * @brief Virtual Destructor
       *
       * Including a virtual destructor for the usual C++ safety reasons.
       */
      virtual ~IInputProducer() {}

      /**
       * @brief Method called to perform an update of a  WMObject
       *
       * This method is called by the InputLink during the Input update cycle
       * to update the values of the associated WMObject (which is passed in
       * as a parameter). An InputProducer may change the value associated with
       * a single wme or it may manage the values of many wmes. This is a 
       * design decision left up to the SSI implementer.
       *
       * @param wmemory The working memory interface associated with
       *                         the agent's input link.
       * @param object The input wme associated with this InputProducer.
       */
      // TODO: Verify that the ShouldUpdate() method is no longer necessary.
      virtual void Update(IWorkingMemory* wmemory,
                          IWMObject* object) = 0;

   };
}
#endif

       
