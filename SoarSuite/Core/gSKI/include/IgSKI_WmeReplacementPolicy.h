/********************************************************************
* @file igski_wmereplacementpolicy.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/14/2002   14:54
*
* purpose: 
********************************************************************/

#ifndef IGSKI_WME_REPLACEMENT_POLICY_H
#define IGSKI_WME_REPLACEMENT_POLICY_H

#include "IgSKI_Symbol.h"

namespace gSKI {

   /**
    * @brief Interface for determining if a Wme should be replaced with
    *         a Wme that has a new value
    *
    * This interface is used to control the replacemnt of Wme's on the
    * Input Link. Instances of this interface are not produced by gSKI, 
    * but are instead implemented by integrators to set policies for 
    * WME value changes.
    *
    * These policies can be useful for cases where it is undesirable for 
    * Wme's to be updated for small changes in world values. An example from
    * TAS might be when the altitude varies slightly in level flight. The
    * Agent may be interested only in changes that exceed a certain magnitude.
    * Constantly replacing Wme's that vary only slightly may adversly 
    * effect system performance.
    */
   class IWmeReplacementPolicy {
   public:
      /**
       * @brief Virtual Destructor
       *
       * Including a virtual destructor for the usual C++ safety reasons.
       */
      virtual ~IWmeReplacementPolicy() {}

      /**
       * @brief Method called to determine if the Wme should be replaced
       *
       * This method is called by the InputLink when the value of a WME is 
       * changed using the various ReplaceWme() methods of the IWorkingMemory
       * interface.
       * The parameters to this function include the proposed and current WME
       * values (both encapsulated using the ISymbol interface). If the current
       * WME value should be replaced with the proposed value this function
       * should return "true", if not, it should return false.
       *
       * @param pProposedValue The proposed value for the IInputWME
       * @param pCurrentValue  The current value of the IInputWME
       *
       * @return true if the current value of the WME should be replaced with 
       *         the proposed value, false otherwise
       */
      // TODO: Reevaluate whether an error here is appropriate since it
      // will be user supplied. I'm also tempted to add the ability to
      // transform the value to this interface (please stop me!).
      virtual bool ShouldReplace(const ISymbol* pProposedValue,
                                 const ISymbol* pCurrentValue ) = 0;

   };

}

#endif
