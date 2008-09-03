/********************************************************************
* @file gSKI_EnumRemapping.h
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_ENUMREMAPPING_H
#define GSKI_ENUMREMAPPING_H

#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "gdatastructs.h"
#include "init_soar.h"
#include "gski_event_system_data.h"
#include "callback.h"

namespace gSKI
{
   /**
    * @brief: 
    */
   class EnumRemappings
   {
   public:
      static egSKITestType ReMapTestType(unsigned short);
      static egSKISymbolType ReMapSymbolType(unsigned short);
      static egSKIPreferenceType ReMapPreferenceType(unsigned short);
      static egSKIPhaseType ReMapPhaseType(unsigned short, bool);
      static egSKIProdType  ReMapProductionType(unsigned short);
 
	  static egSKIProductionEventId Map_Kernel_to_gSKI_ProdEventId(unsigned long eventId, unsigned char occured);
      static egSKIPrintEventId      Map_Kernel_to_gSKI_PrintEventId(unsigned long eventId);
	  static egSKIXMLEventId		Map_Kernel_to_gSKI_XMLEventId(unsigned long eventId);
	  static egSKIRunEventId		Map_Kernel_to_gSKI_RunEventId(unsigned long eventId, unsigned char occured);
 


      ////////////////////////////////////////////////////////////////
      // The following don't require remapping, but we use the mapper
      //  just in case this changes in the future.
      static egSKISupportType ReMapSupportType(unsigned short value)
      {
         // They are the same, don't need to map, just cast
         return (egSKISupportType)(value);
      }
      
      ////////////////////////////////////////////////////////////////
      // OPPOSITE DIRECTION MAPPINGS:  gSKI to Kernel...
      static egSKIAgentEvents RemapProductionEventType(egSKIProductionEventId eventId);
      static egSKIAgentEvents RemapPrintEventType(egSKIPrintEventId eventId);
	  static egSKIAgentEvents RemapXMLEventType(egSKIXMLEventId eventId);
	  static egSKIAgentEvents RemapRunEventType(egSKIRunEventId eventId);
	  static SOAR_CALLBACK_TYPE KernelRunEventType(egSKIRunEventId eventId);

      static void Init();
   private:
      /**
       * @brief: 
       */
      //{
      static unsigned short TestTypeEnumMapping[NUM_TEST_TYPES+1];
      static unsigned short SymbolTypeEnumMapping[NUM_SYMBOL_TYPES];
      static unsigned short PrefTypeEnumMapping[NUM_PREFERENCE_TYPES];
      static unsigned short PhaseTypeEnumMapping[NUM_PHASE_TYPES];
      static unsigned short EventEnumMapping[gSKI_K_MAX_AGENT_EVENTS][2];
      static unsigned short RunEventEnumMapping[NUMBER_OF_MONITORABLE_CALLBACKS];
      static unsigned short PrintEventEnumMapping[gSKI_K_MAX_AGENT_EVENTS];
	  static unsigned short XMLEventEnumMapping[gSKI_K_MAX_AGENT_EVENTS];
      static unsigned short ProductionTypeEnumMapping[NUM_PRODUCTION_TYPES];
      //}

      static bool m_initialized;
   };
}
#endif
