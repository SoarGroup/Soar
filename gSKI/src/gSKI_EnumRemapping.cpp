/********************************************************************
* @file gski_enumremapping.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_EnumRemapping.h"


#include "MegaAssert.h"
#include "gsysparam.h"

namespace gSKI 
{

   unsigned short EnumRemappings::TestTypeEnumMapping[NUM_TEST_TYPES+1];
   unsigned short EnumRemappings::SymbolTypeEnumMapping[NUM_SYMBOL_TYPES];
   unsigned short EnumRemappings::PrefTypeEnumMapping[NUM_PREFERENCE_TYPES];
   unsigned short EnumRemappings::PhaseTypeEnumMapping[NUM_PHASE_TYPES];
   unsigned short EnumRemappings::EventEnumMapping[gSKI_K_MAX_AGENT_EVENTS][2];
   unsigned short EnumRemappings::ProductionTypeEnumMapping[NUM_PRODUCTION_TYPES];
   bool EnumRemappings::m_initialized = false;

   /*
   ==================================
 _____                       ____                                  _
| ____|_ __  _   _ _ __ ___ |  _ \ ___ _ __ ___   __ _ _ __  _ __ (_)_ __   __ _ ___
|  _| | '_ \| | | | '_ ` _ \| |_) / _ \ '_ ` _ \ / _` | '_ \| '_ \| | '_ \ / _` / __|
| |___| | | | |_| | | | | | |  _ <  __/ | | | | | (_| | |_) | |_) | | | | | (_| \__ \
|_____|_| |_|\__,_|_| |_| |_|_| \_\___|_| |_| |_|\__,_| .__/| .__/|_|_| |_|\__, |___/
                                                      |_|   |_|            |___/
   ==================================
   */
   void EnumRemappings::Init(void)
   {
      TestTypeEnumMapping[NOT_EQUAL_TEST] =          gSKI_NOT_EQUAL;
      TestTypeEnumMapping[LESS_TEST] =               gSKI_LESS_THAN;
      TestTypeEnumMapping[GREATER_TEST] =            gSKI_GREATER_THAN;
      TestTypeEnumMapping[LESS_OR_EQUAL_TEST] =      gSKI_LESS_THAN_OR_EQUAL;
      TestTypeEnumMapping[GREATER_OR_EQUAL_TEST] =   gSKI_GREATER_OR_EQUAL;
      TestTypeEnumMapping[SAME_TYPE_TEST] =          gSKI_EQUAL;
      TestTypeEnumMapping[DISJUNCTION_TEST] =        gSKI_DISJUNCTION;
      TestTypeEnumMapping[CONJUNCTIVE_TEST] =        gSKI_CONJUNCTION;
      TestTypeEnumMapping[GOAL_ID_TEST] =            gSKI_OTHER;
      TestTypeEnumMapping[IMPASSE_ID_TEST] =         gSKI_OTHER;


      //SymbolTypeEnumMapping[]                          = gSKI_ANY_SYMBOL;
      SymbolTypeEnumMapping[FLOAT_CONSTANT_SYMBOL_TYPE]  = gSKI_DOUBLE;
      SymbolTypeEnumMapping[INT_CONSTANT_SYMBOL_TYPE]    = gSKI_INT;
      SymbolTypeEnumMapping[SYM_CONSTANT_SYMBOL_TYPE]    = gSKI_STRING;
      SymbolTypeEnumMapping[IDENTIFIER_SYMBOL_TYPE]      = gSKI_OBJECT;
      SymbolTypeEnumMapping[VARIABLE_SYMBOL_TYPE]        = gSKI_VARIABLE;

      // Preference remappings
      PrefTypeEnumMapping[ACCEPTABLE_PREFERENCE_TYPE]          = gSKI_ACCEPTABLE_PREF;
      PrefTypeEnumMapping[REQUIRE_PREFERENCE_TYPE]             = gSKI_REQUIRE_PREF;
      PrefTypeEnumMapping[REJECT_PREFERENCE_TYPE]              = gSKI_REJECT_PREF;
      PrefTypeEnumMapping[PROHIBIT_PREFERENCE_TYPE]            = gSKI_PROHIBIT_PREF;
      PrefTypeEnumMapping[UNARY_INDIFFERENT_PREFERENCE_TYPE]   = gSKI_INDIFFERENT_PREF;
      PrefTypeEnumMapping[BEST_PREFERENCE_TYPE]                = gSKI_BEST_PREF;
      PrefTypeEnumMapping[WORST_PREFERENCE_TYPE]               = gSKI_WORST_PREF;
      PrefTypeEnumMapping[BINARY_INDIFFERENT_PREFERENCE_TYPE]  = gSKI_BIN_INDIFFERENT_PREF;
      PrefTypeEnumMapping[BETTER_PREFERENCE_TYPE]              = gSKI_BIN_BETTER_PREF;
      PrefTypeEnumMapping[WORSE_PREFERENCE_TYPE]               = gSKI_BIN_WORSE_PREF;

      // Phase remappings (some phases cannot use simple remappings)
      PhaseTypeEnumMapping[INPUT_PHASE]         = gSKI_INPUT_PHASE;
      PhaseTypeEnumMapping[DECISION_PHASE]      = gSKI_DECISION_PHASE;
      PhaseTypeEnumMapping[OUTPUT_PHASE]        = gSKI_OUTPUT_PHASE;

      // Event mappings
      EventEnumMapping[gSKI_K_EVENT_WME_ADDED][0]   = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_ADDED][1]   = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_REMOVED][0] = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_REMOVED][1] = 0;

      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_ADDED][1] = gSKIEVENT_AFTER_PRODUCTION_ADDED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_REMOVED][0] = gSKIEVENT_BEFORE_PRODUCTION_REMOVED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_FIRED][1] = gSKIEVENT_AFTER_PRODUCTION_FIRED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_RETRACTED][0] = gSKIEVENT_BEFORE_PRODUCTION_RETRACTED;

      ProductionTypeEnumMapping[JUSTIFICATION_PRODUCTION_TYPE] = gSKI_JUSTIFICATION;
      ProductionTypeEnumMapping[USER_PRODUCTION_TYPE] = gSKI_USER;
      ProductionTypeEnumMapping[CHUNK_PRODUCTION_TYPE] = gSKI_CHUNK;
      ProductionTypeEnumMapping[DEFAULT_PRODUCTION_TYPE] = gSKI_DEFAULT;
      // Setting the initialized flags
      m_initialized = true;
   }

   /*
   ==================================
 ____      __  __           _____        _  _____
|  _ \ ___|  \/  | __ _ _ _|_   _|__ ___| ||_   _|   _ _ __   ___
| |_) / _ \ |\/| |/ _` | '_ \| |/ _ Y __| __|| || | | | '_ \ / _ \
|  _ <  __/ |  | | (_| | |_) | |  __|__ \ |_ | || |_| | |_) |  __/
|_| \_\___|_|  |_|\__,_| .__/|_|\___|___/\__||_| \__, | .__/ \___|
                       |_|                       |___/|_|
   ==================================
   */
   egSKITestType EnumRemappings::ReMapTestType(unsigned short type)
   {
      if(!m_initialized) 
         Init();
      
      return static_cast<egSKITestType>(TestTypeEnumMapping[type]);
   }

   /*
   ==================================
 ____      __  __            ____                  _           _ _____
|  _ \ ___|  \/  | __ _ _ __/ ___| _   _ _ __ ___ | |__   ___ | |_   _|   _ _ __   ___
| |_) / _ \ |\/| |/ _` | '_ \___ \| | | | '_ ` _ \| '_ \ / _ \| | | || | | | '_ \ / _ \
|  _ <  __/ |  | | (_| | |_) |__) | |_| | | | | | | |_) | (_) | | | || |_| | |_) |  __/
|_| \_\___|_|  |_|\__,_| .__/____/ \__, |_| |_| |_|_.__/ \___/|_| |_| \__, | .__/ \___|
                       |_|         |___/                              |___/|_|
   ==================================
   */
   egSKISymbolType EnumRemappings::ReMapSymbolType(unsigned short type)
   {
      if(!m_initialized) 
         Init();
      
      return static_cast<egSKISymbolType>(SymbolTypeEnumMapping[type]);
   }

   /*
   ==================================
   ==================================
   */
   egSKIPreferenceType EnumRemappings::ReMapPreferenceType(unsigned short type)
   {
      if(!m_initialized) 
         Init();
      
      return static_cast<egSKIPreferenceType>(PrefTypeEnumMapping[type]);
   }
   
   /*
   ==================================
   ==================================
   */
   egSKIPhaseType EnumRemappings::ReMapPhaseType(unsigned short phase, bool application)
   {
      if(!m_initialized) 
         Init();
      // Special case where we need more info for phase remappings
      // Soar goals the apply and proposal phases both preference phases
      if(phase == PREFERENCE_PHASE)
         return (application)? gSKI_APPLY_PHASE: gSKI_PROPOSAL_PHASE;
      else
         return static_cast<egSKIPhaseType>(PhaseTypeEnumMapping[phase]);
   }

   /*
   ==================================
   ==================================
   */
   egSKIAgentEvents EnumRemappings::RemapProductionEventType(egSKIEventId eventId)
   {
      if(!m_initialized) 
         Init();
      switch (eventId)
      {
      case gSKIEVENT_AFTER_PRODUCTION_ADDED:
         return gSKI_K_EVENT_PRODUCTION_ADDED;
      case gSKIEVENT_BEFORE_PRODUCTION_REMOVED:
         return gSKI_K_EVENT_PRODUCTION_REMOVED;
      case gSKIEVENT_AFTER_PRODUCTION_FIRED:
         return gSKI_K_EVENT_PRODUCTION_FIRED;
      case gSKIEVENT_BEFORE_PRODUCTION_RETRACTED:
         return gSKI_K_EVENT_PRODUCTION_RETRACTED;
      default:
         // Error condition
         MegaAssert(false, "Could not map a production event id");
         return static_cast<egSKIAgentEvents>(0);
      }
   }


   /*
   ==================================
   ==================================
   */
   egSKIEventId EnumRemappings::RemapEventType(unsigned long eventId, unsigned char occured)
   {
      if(!m_initialized) 
         Init();
      return static_cast<egSKIEventId>(EventEnumMapping[eventId][occured]);
   }

   /*
   ==================================
   ==================================
   */
   egSKIProdType EnumRemappings::ReMapProductionType(unsigned short type)
   {
      if(!m_initialized) 
         Init();
      return static_cast<egSKIProdType>(ProductionTypeEnumMapping[type]);
   }
}
