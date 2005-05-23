/////////////////////////////////////////////////////////////////
//xmlTraceNames
//
// Author: Bob Marinier, University of Michigan
// Date  : May 2005
//
// The names (identifiers) we use in the XML trace.
// These were originally created by Karen and Doug in sml_Names.h
//  but we decided to copy them here to prevent the kernel from
//  needing ConnectionSML stuff to build (or vice versa).
//
// NOTE: It is very important that these constants match their
//  partners in sml_Names.h/cpp. Those files may actually
//  define more constants (that is ok), but the ones here must
//  also be there and have the same values.
//
/////////////////////////////////////////////////////////////////

#ifndef XML_TRACE_NAMESH
#define XML_TRACE_NAMESH

class xmlTraceNames
{
public:
	//for RHS output
	static char const* const kTagRHS_write ;
	static char const* const kRHS_String ;

	// Tags defined for Trace output at each watch level:
	// <trace> contains the rest.
	static char const* const kTagTrace ;

	// <context> tag identifiers for Watch level 1
	static char const* const kTagState ;
	static char const* const kTagOperator ;
	static char const* const kState_ID;
	static char const* const kState_Name;
	static char const* const kState_DecisionCycleCt;
	static char const* const kState_ImpasseObject;
	static char const* const kState_ImpasseType;
	static char const* const kState_StackLevel ;
	static char const* const kOperator_ID;
	static char const* const kOperator_Name;
	static char const* const kOperator_DecisionCycleCt;
	static char const* const kOperator_StackLevel ;
 
	// <phase> tag identifiers for Watch level 2
	static char const* const kTagPhase ;
	static char const* const kPhase_Name ;
	static char const* const kPhase_Status ;
	static char const* const kPhase_FiringType ;
	static char const* const kPhaseName_Input ;
	static char const* const kPhaseName_Pref ;
	static char const* const kPhaseName_WM ;
	static char const* const kPhaseName_Decision ;
	static char const* const kPhaseName_Output ;
	static char const* const kPhaseName_Propose ;
	static char const* const kPhaseName_Apply ;
	static char const* const kPhaseName_Unknown;
	static char const* const kPhaseStatus_Begin ;
	static char const* const kPhaseStatus_End ;
 	static char const* const kPhaseFiringType_IE ;
	static char const* const kPhaseFiringType_PE ;

	// <prod-firing> tag identifiers for Watch level 3
	static char const* const kTagProduction ;
	static char const* const kProduction_Name ;
 	static char const* const kTagProduction_Firing ;
	static char const* const kTagProduction_Retracting ;
	
	// <wme> tag identifiers, also Watch level 4
	static char const* const kTagWME ;
	static char const* const kWME_TimeTag ;
	static char const* const kWME_Id ;
	static char const* const kWME_Attribute ;
	static char const* const kWME_Value ;
	static char const* const kWME_ValueType ;
	static char const* const kWMEPreference;
	static char const* const kWME_Action ;
	// kjc question:  should the next entry be kWMEAction_Add?
	static char const* const kValueAdd	;
	static char const* const kValueRemove ;
	static char const* const kTagWMERemove ;
	static char const* const kTagWMEAdd ;

	// <preference> tag identifiers, also Watch level 5
	static char const* const kTagPreference ;
	static char const* const kPreference_Type ;
	static char const* const kOSupported ;
	static char const* const kReferent ;

	// for warnings controlled by WARNINGS_SYSPARAM
	static char const* const kTagWarning ;

	// XML function types for XML output event
	static char const* const kFunctionBeginTag;
	static char const* const kFunctionEndTag;
	static char const* const kFunctionAddAttribute;

	// learning stuff
	static char const* const kTagLearning;
	// Tag learning has attribute kTypeString

	// end of tags for Trace output

	static char const* const kTypeString;
};

#endif // XML_TRACE_NAMESH