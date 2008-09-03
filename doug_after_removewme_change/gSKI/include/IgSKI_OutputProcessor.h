/********************************************************************
* @file igski_outputprocessor.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/14/2002   14:54
*
* purpose: 
********************************************************************/

#ifndef IGSKI_OUTPUT_PROCESSOR_H
#define IGSKI_OUTPUT_PROCESSOR_H

namespace gSKI {

   // Forward Declarations
   class IWMObject;
   class IWorkingMemory;

   /**
    * @brief IOutputConsumer interface for converting WMObjects on the 
    * output link to function calls etc. in the simulation environment.
    *
    * This interface is used to automate the processing of the WMObjects on
    * the output link. Instances of this interface are not produced by gSKI,
    * instead this interface should be implemented by integrators to process
    * wme's on the output link and transform them into the proper function
    * call or calls in the simulation environment.
    *
    * IOutputConsumers are registered with the IOutputLink using a search
    * pattern much like the WMObject Find() method (ID, Atribute (dot 
    * notation), value) in the output link's IWorkingMemory object. The 
    * output consumers are then invoked automatically during the Soar output 
    * phase and called once for each WMObject that matches the pattern
    * they specified during registration.
    */
   class IOutputProcessor {
   public:
      /**
       * @brief Virtual Destructor
       *
       * Including a virtual destructor for the usual C++ safety reasons.
       */
      virtual ~IOutputProcessor() {}

      /**
       * @brief Update function processes WMObject's matching the registration 
       * pattern
       *
       * This function is called once for every WMObject that matches the 
       * pattern specified when this interface was registered with the 
       * OutputLink.
       *
       * @param wmemory  The IWorkingMemory object associated with the output
       *                  link
       * @param object The output wme that matches the registration pattern
       */
      virtual void ProcessOutput(IWorkingMemory* wmemory,
                                 IWMObject* object) = 0;

   };

}

#endif
