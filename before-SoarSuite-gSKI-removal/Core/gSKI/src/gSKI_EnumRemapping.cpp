#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_enumremapping.cpp
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
   /* unsigned short EnumRemappings::EventEnumMapping[gSKI_K_MAX_AGENT_EVENTS]; */
   unsigned short EnumRemappings::RunEventEnumMapping[NUMBER_OF_MONITORABLE_CALLBACKS];
   unsigned short EnumRemappings::PrintEventEnumMapping[gSKI_K_MAX_AGENT_EVENTS];
   unsigned short EnumRemappings::XMLEventEnumMapping[gSKI_K_MAX_AGENT_EVENTS];
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
      PhaseTypeEnumMapping[PROPOSE_PHASE]       = gSKI_PROPOSAL_PHASE;
      PhaseTypeEnumMapping[DECISION_PHASE]      = gSKI_DECISION_PHASE;
      PhaseTypeEnumMapping[APPLY_PHASE]         = gSKI_APPLY_PHASE;
      PhaseTypeEnumMapping[OUTPUT_PHASE]        = gSKI_OUTPUT_PHASE;
      PhaseTypeEnumMapping[PREFERENCE_PHASE]    = gSKI_PREFERENCE_PHASE;
      PhaseTypeEnumMapping[WM_PHASE]            = gSKI_WM_PHASE;

      // Event mappings
      EventEnumMapping[gSKI_K_EVENT_WME_ADDED][0]   = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_ADDED][1]   = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_REMOVED][0] = 0;
      EventEnumMapping[gSKI_K_EVENT_WME_REMOVED][1] = 0;

      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_ADDED][1] = gSKIEVENT_AFTER_PRODUCTION_ADDED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_REMOVED][0] = gSKIEVENT_BEFORE_PRODUCTION_REMOVED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_FIRED][1] = gSKIEVENT_AFTER_PRODUCTION_FIRED;
      EventEnumMapping[gSKI_K_EVENT_PRODUCTION_RETRACTED][0] = gSKIEVENT_BEFORE_PRODUCTION_RETRACTED;

	  EventEnumMapping[gSKI_K_EVENT_PHASE][0]             = gSKIEVENT_BEFORE_PHASE_EXECUTED;
	  EventEnumMapping[gSKI_K_EVENT_PHASE][1]             = gSKIEVENT_AFTER_PHASE_EXECUTED;
	  EventEnumMapping[gSKI_K_EVENT_ELABORATION_CYCLE][0] = gSKIEVENT_BEFORE_ELABORATION_CYCLE;
	  EventEnumMapping[gSKI_K_EVENT_ELABORATION_CYCLE][1] = gSKIEVENT_AFTER_ELABORATION_CYCLE;
	  EventEnumMapping[gSKI_K_EVENT_DECISION_CYCLE][0]    = gSKIEVENT_BEFORE_DECISION_CYCLE;
	  EventEnumMapping[gSKI_K_EVENT_DECISION_CYCLE][1]    = gSKIEVENT_AFTER_DECISION_CYCLE;

      RunEventEnumMapping[BEFORE_INPUT_PHASE_CALLBACK]	  = gSKIEVENT_BEFORE_INPUT_PHASE;
      RunEventEnumMapping[BEFORE_PROPOSE_PHASE_CALLBACK]  = gSKIEVENT_BEFORE_PROPOSE_PHASE;
      RunEventEnumMapping[BEFORE_DECISION_PHASE_CALLBACK] = gSKIEVENT_BEFORE_DECISION_PHASE;
      RunEventEnumMapping[BEFORE_APPLY_PHASE_CALLBACK]	  = gSKIEVENT_BEFORE_APPLY_PHASE;
      RunEventEnumMapping[BEFORE_OUTPUT_PHASE_CALLBACK]   = gSKIEVENT_BEFORE_OUTPUT_PHASE;
      RunEventEnumMapping[BEFORE_PREFERENCE_PHASE_CALLBACK]  = gSKIEVENT_BEFORE_PREFERENCE_PHASE;	// Soar-7 mode only
      RunEventEnumMapping[BEFORE_WM_PHASE_CALLBACK]       = gSKIEVENT_BEFORE_WM_PHASE;	// Soar-7 mode only
      RunEventEnumMapping[AFTER_INPUT_PHASE_CALLBACK]	  = gSKIEVENT_AFTER_INPUT_PHASE;
      RunEventEnumMapping[AFTER_PROPOSE_PHASE_CALLBACK]   = gSKIEVENT_AFTER_PROPOSE_PHASE;
      RunEventEnumMapping[AFTER_DECISION_PHASE_CALLBACK]  = gSKIEVENT_AFTER_DECISION_PHASE;
      RunEventEnumMapping[AFTER_APPLY_PHASE_CALLBACK]	  = gSKIEVENT_AFTER_APPLY_PHASE;
      RunEventEnumMapping[AFTER_OUTPUT_PHASE_CALLBACK]	  = gSKIEVENT_AFTER_OUTPUT_PHASE;
      RunEventEnumMapping[AFTER_PREFERENCE_PHASE_CALLBACK]	  = gSKIEVENT_AFTER_PREFERENCE_PHASE;	// Soar-7 mode only
      RunEventEnumMapping[AFTER_WM_PHASE_CALLBACK]		  = gSKIEVENT_AFTER_WM_PHASE;	// Soar-7 mode only
	  /* RunEventEnumMapping[BEFORE_ELABORATION_CYCLE_CALLBACK] = gSKIEVENT_BEFORE_ELABORATION_CYCLE;
	     RunEventEnumMapping[AFTER_ELABORATION_CYCLE_CALLBACK] = gSKIEVENT_AFTER_ELABORATION_CYCLE; */
	  RunEventEnumMapping[BEFORE_DECISION_CYCLE_CALLBACK] = gSKIEVENT_BEFORE_DECISION_CYCLE;
	  RunEventEnumMapping[AFTER_DECISION_CYCLE_CALLBACK]  = gSKIEVENT_AFTER_DECISION_CYCLE;
	  RunEventEnumMapping[MAX_MEMORY_USAGE_CALLBACK] = gSKIEVENT_MAX_MEMORY_USAGE_EXCEEDED;

	  PrintEventEnumMapping[gSKI_K_EVENT_PRINT_CALLBACK] = gSKIEVENT_PRINT;

	  XMLEventEnumMapping[gSKI_K_EVENT_XML_OUTPUT] = gSKIEVENT_XML_TRACE_OUTPUT;

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
 
	  return static_cast<egSKIPhaseType>(PhaseTypeEnumMapping[phase]);
   }

   /*
   ==================================
   ==================================
   */
   egSKIAgentEvents EnumRemappings::RemapProductionEventType(egSKIProductionEventId eventId)
   /** this goes from gSKI to Kernel Events **/
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

  egSKIProductionEventId EnumRemappings::Map_Kernel_to_gSKI_ProdEventId(unsigned long eventId, unsigned char occured)
  { 
	unsigned long gSKIeventId;

    if(!m_initialized) 
       Init();

	gSKIeventId = (EventEnumMapping[eventId][occured]);
    
	if (IsProductionEventID(gSKIeventId)) {
		return static_cast<egSKIProductionEventId>(gSKIeventId);
      } else {
	// Error condition
	MegaAssert(false, "Could not map a production event id");
	return static_cast<egSKIProductionEventId>(0);
      }
  }
  /**  could be changed to lines below for more explicit coding 
      switch (eventId)
      {
      case gSKI_K_EVENT_PRODUCTION_ADDED:     return gSKIEVENT_AFTER_PRODUCTION_ADDED;
      case gSKI_K_EVENT_PRODUCTION_REMOVED:   return gSKIEVENT_BEFORE_PRODUCTION_REMOVED;
      case gSKI_K_EVENT_PRODUCTION_FIRED:     return gSKIEVENT_AFTER_PRODUCTION_FIRED;
      case gSKI_K_EVENT_PRODUCTION_RETRACTED: return gSKIEVENT_BEFORE_PRODUCTION_RETRACTED;
      }
  **/

   /*
   ==================================
   ==================================
   */
   egSKIAgentEvents EnumRemappings::RemapRunEventType(egSKIRunEventId eventId)
   /** this goes from gSKI to gSKI-added Kernel Events **/
   {
      if(!m_initialized) 
         Init();
      switch (eventId)
      {
      case gSKIEVENT_BEFORE_PHASE_EXECUTED:            return gSKI_K_EVENT_PHASE;
      case gSKIEVENT_AFTER_PHASE_EXECUTED:             return gSKI_K_EVENT_PHASE;
      case gSKIEVENT_BEFORE_ELABORATION_CYCLE:         return gSKI_K_EVENT_ELABORATION_CYCLE;
      case gSKIEVENT_AFTER_ELABORATION_CYCLE:          return gSKI_K_EVENT_ELABORATION_CYCLE;
      case gSKIEVENT_BEFORE_DECISION_CYCLE:			   return gSKI_K_EVENT_DECISION_CYCLE;      
      case gSKIEVENT_AFTER_DECISION_CYCLE: 	           return gSKI_K_EVENT_DECISION_CYCLE;
 
       default:
         // Error condition
         MegaAssert(false, "Could not map a run event id");
         return static_cast<egSKIAgentEvents>(0);
      }
   }

   SOAR_CALLBACK_TYPE EnumRemappings::KernelRunEventType(egSKIRunEventId eventId)
   /** this goes from gSKI to Kernel native callbacks **/
   {
      if(!m_initialized) 
         Init();
      switch (eventId)
      {
      case gSKIEVENT_BEFORE_DECISION_CYCLE:      	  return BEFORE_DECISION_CYCLE_CALLBACK;
      case gSKIEVENT_AFTER_DECISION_CYCLE:            return AFTER_DECISION_CYCLE_CALLBACK;
	  case gSKIEVENT_BEFORE_INPUT_PHASE:			  return BEFORE_INPUT_PHASE_CALLBACK;
	  case gSKIEVENT_BEFORE_PROPOSE_PHASE:			  return BEFORE_PROPOSE_PHASE_CALLBACK;
	  case gSKIEVENT_BEFORE_DECISION_PHASE:			  return BEFORE_DECISION_PHASE_CALLBACK;
	  case gSKIEVENT_BEFORE_APPLY_PHASE:			  return BEFORE_APPLY_PHASE_CALLBACK; 
	  case gSKIEVENT_BEFORE_OUTPUT_PHASE:			  return BEFORE_OUTPUT_PHASE_CALLBACK;
	  case gSKIEVENT_BEFORE_PREFERENCE_PHASE:		  return BEFORE_PREFERENCE_PHASE_CALLBACK; // Soar-7 mode only
	  case gSKIEVENT_BEFORE_WM_PHASE:				  return BEFORE_WM_PHASE_CALLBACK;  // Soar-7 mode only
	  case gSKIEVENT_AFTER_INPUT_PHASE:				  return AFTER_INPUT_PHASE_CALLBACK;
	  case gSKIEVENT_AFTER_PROPOSE_PHASE:			  return AFTER_PROPOSE_PHASE_CALLBACK;
	  case gSKIEVENT_AFTER_DECISION_PHASE:			  return AFTER_DECISION_PHASE_CALLBACK;
	  case gSKIEVENT_AFTER_APPLY_PHASE:				  return AFTER_APPLY_PHASE_CALLBACK;
	  case gSKIEVENT_AFTER_OUTPUT_PHASE:			  return AFTER_OUTPUT_PHASE_CALLBACK;
	  case gSKIEVENT_AFTER_PREFERENCE_PHASE:		  return AFTER_PREFERENCE_PHASE_CALLBACK; // Soar-7 mode only
	  case gSKIEVENT_AFTER_WM_PHASE:				  return AFTER_WM_PHASE_CALLBACK; // Soar-7 mode only
	  case gSKIEVENT_MAX_MEMORY_USAGE_EXCEEDED:       return MAX_MEMORY_USAGE_CALLBACK;
 
	  default:
         // Error condition
         MegaAssert(false, "Could not map to a kernel native callback");
         return static_cast<SOAR_CALLBACK_TYPE>(0);
      }
   }
   /*
   ==================================
   ==================================
   */

  egSKIRunEventId EnumRemappings::Map_Kernel_to_gSKI_RunEventId(unsigned long eventId, unsigned char occured)
  { 
	unsigned long gSKIeventId;

    if(!m_initialized) 
       Init();

	gSKIeventId = (EventEnumMapping[eventId][occured]);
    
	if (IsRunEventID(gSKIeventId)) {
		return static_cast<egSKIRunEventId>(gSKIeventId);
      } else {
	// Error condition
	MegaAssert(false, "Could not map a Run event id");
	return static_cast<egSKIRunEventId>(0);
      }
  }
   /*
   ==================================
   ==================================
   */
   egSKIAgentEvents EnumRemappings::RemapPrintEventType(egSKIPrintEventId eventId)
   /** this goes from gSKI to Kernel Events **/
   {
      if(!m_initialized) 
         Init();
      switch (eventId)	 
      {
      case gSKIEVENT_PRINT:
         return gSKI_K_EVENT_PRINT_CALLBACK;
      default:
         // Error condition
         MegaAssert(false, "Could not map a print event id");
         return static_cast<egSKIAgentEvents>(0);
      }
   }

  egSKIPrintEventId EnumRemappings::Map_Kernel_to_gSKI_PrintEventId(unsigned long eventId)
  { 
	unsigned long gSKIeventId;

    if(!m_initialized) 
       Init();

	gSKIeventId = (PrintEventEnumMapping[eventId]);
    
	if (IsPrintEventID(gSKIeventId)) {
		return static_cast<egSKIPrintEventId>(gSKIeventId);
      } else {
	// Error condition
	MegaAssert(false, "Could not map a print event id");
	return static_cast<egSKIPrintEventId>(0);
      }
  }

  egSKIAgentEvents EnumRemappings::RemapXMLEventType(egSKIXMLEventId eventId)
   /** this goes from gSKI to Kernel Events **/
   {
      if(!m_initialized) 
         Init();
      switch (eventId)	 
      {
      case gSKIEVENT_XML_TRACE_OUTPUT:
         return gSKI_K_EVENT_XML_OUTPUT;
      default:
         // Error condition
         MegaAssert(false, "Could not map an XML event id");
         return static_cast<egSKIAgentEvents>(0);
      }
   }

   egSKIXMLEventId EnumRemappings::Map_Kernel_to_gSKI_XMLEventId(unsigned long eventId)
  { 
	unsigned long gSKIeventId;

    if(!m_initialized) 
       Init();

	gSKIeventId = (XMLEventEnumMapping[eventId]);
    
	if (IsXMLEventID(gSKIeventId)) {
		return static_cast<egSKIXMLEventId>(gSKIeventId);
      } else {
	// Error condition
	MegaAssert(false, "Could not map an XNL event id");
	return static_cast<egSKIXMLEventId>(0);
      }
  }

   /*
   ==================================
   ==================================
   */
   /** KJC:  egSKIEventId no longer exists.  If needed, must be EventType-specific.

   egSKIEventId EnumRemappings::RemapEventType(unsigned long eventId, unsigned char occured)
   // this goes from Kernel to gSKI events.  
   {
      if(!m_initialized) 
         Init();
      switch (eventId)
      {
	  MUST CONVERT THE eventID to a gSKI ENUM for these tests to work
	  case IsSystemEventId(eventId):
		  return static_cast<egSKISystemEventId>(EventEnumMapping[eventId][occured]);
	  case IsRunEventId(eventId):
		  return static_cast<egSKIRunEventId>(EventEnumMapping[eventId][occured]);
	  case IsAgentEventId(eventId):
		  return static_cast<egSKIAgentEventId>(EventEnumMapping[eventId][occured]);
	  case IsWorkingMemoryEventId(eventId):
		  return static_cast<egSKIWorkingMemoryEventId>(EventEnumMapping[eventId][occured]);
	  case IsPrintEventId(eventId):
		  return static_cast<egSKIPrintEventId>(EventEnumMapping[eventId][occured]);
	  case IsRhsEventId(eventId):
		  return static_cast<egSKIRhsEventId>(EventEnumMapping[eventId][occured]);
	  case IsGenericEventId(eventId):
		  return static_cast<egSKIGenericEventId>(EventEnumMapping[eventId][occured]);
      default:
         // Error condition
         MegaAssert(false, "Could not map a production event id");
         return static_cast<egSKIProductionEventId>(0);   
	  }
   }
   */

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
