#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  xml.cpp
 *
 * =======================================================================
 *  
 * Contains methods for generating XML objects in response to kernel commands.
 *
 * The commands are modelled after the existing kernel functions which are tied to generating
 * string output.  In the past we added code to some of those functions so they'd
 * generate string output and also XML output (as a string: <aaa>...</aaa>).  To capture
 * the XML output a caller would register for the XML callback, generate the XML as a string,
 * parse the XML back to an object and return it.
 *
 * These methods generate XML as an object, so there are no strings being created
 * and subsequently parsed and no need to "intercept" the XML callback channel (which is
 * really for XML trace output to the debugger, not for results from commands).
 * This new approach is more efficient to both create and to subsequently use.
 *
 * =======================================================================
 */

#include "xml.h"

#include "agent.h"
#include "soar_TraceNames.h"
#include "XMLTrace.h"
#include "ElementXML.h"
#include "print.h"
#include "wmem.h"

#include "assert.h" 

using namespace soar_TraceNames;
namespace stn = soar_TraceNames;

void xml_create( agent* pAgent )
{
	if ( !pAgent )
	{
		assert( pAgent );
		return;
	}

	soarxml::XMLTrace* pTrace = new soarxml::XMLTrace();
	soarxml::XMLTrace* pCommands = new soarxml::XMLTrace();
	
	pAgent->xml_trace = static_cast< xml_handle >( pTrace );
	pAgent->xml_commands = static_cast< xml_handle >( pCommands );
	
	pAgent->xml_destination = pAgent->xml_trace;
}

void xml_reset( agent* pAgent )
{
	if ( !pAgent || !pAgent->xml_trace || !pAgent->xml_commands )
	{
		assert( pAgent );
		assert( pAgent->xml_trace );
		assert( pAgent->xml_commands );
		return;
	}

	soarxml::XMLTrace* pTrace = static_cast< soarxml::XMLTrace* >( pAgent->xml_trace );
	soarxml::XMLTrace* pCommands = static_cast< soarxml::XMLTrace* >( pAgent->xml_commands );

	pTrace->Reset();
	pCommands->Reset();
}

void xml_destroy( agent* pAgent )
{
	if ( !pAgent || !pAgent->xml_trace || !pAgent->xml_commands )
	{
		assert( pAgent );
		assert( pAgent->xml_trace );
		assert( pAgent->xml_commands );
		return;
	}

	soarxml::XMLTrace* pTrace = static_cast< soarxml::XMLTrace* >( pAgent->xml_trace );
	soarxml::XMLTrace* pCommands = static_cast< soarxml::XMLTrace* >( pAgent->xml_commands );

	delete pTrace;
	delete pCommands;

	pAgent->xml_trace = 0;
	pAgent->xml_commands = 0;

	pAgent->xml_destination = 0;
}

void xml_begin_tag( agent* pAgent, char const* pTag )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->BeginTag( pTag ) ;
}

void xml_end_tag( agent* pAgent, char const* pTag )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->EndTag( pTag ) ;
}

// These "moveCurrent" methods allow us to move the entry point for new XML
// around in the existing structure.  That's not often required but occassionally is helpful.
void xml_move_current_to_parent( agent* pAgent )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->MoveCurrentToParent();
}

void xml_move_current_to_child( agent* pAgent, int index )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->MoveCurrentToChild( index );
}

void xml_move_current_to_last_child( agent* pAgent )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->MoveCurrentToLastChild();
}

void xml_att_val( agent* pAgent, char const* pAttribute, uint64_t value )
{
	char buf[51];
	SNPRINTF(buf, 50, "%llu", value);

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, buf ) ;
}

void xml_att_val( agent* pAgent, char const* pAttribute, int value )
{
	char buf[51];
	SNPRINTF(buf, 50, "%d", value);

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, buf ) ;
}

void xml_att_val( agent* pAgent, char const* pAttribute, int64_t value )
{
	char buf[51];
	SNPRINTF(buf, 50, "%lld", value);

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, buf ) ;
}

void xml_att_val( agent* pAgent, char const* pAttribute, double value )
{
	char buf[51];
	SNPRINTF(buf, 50, "%f", value);

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, buf ) ;
}

void xml_att_val( agent* pAgent, char const* pAttribute, char const* pValue )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, pValue ) ;
}

void xml_att_val( agent* pAgent, char const* pAttribute, Symbol* pSymbol )
{
	// Passing 0, 0 as buffer to symbol to string causes it to use internal, temporary buffers
	// which is fine because we immediately copy that string in XMLAddAttribute.
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	pXML->AddAttribute( pAttribute, symbol_to_string( pAgent, pSymbol, true, 0, 0 ) ) ;
}

void xml_object( agent* pAgent, char const* pTag )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( pTag ) ;
	pXML->EndTag( pTag ) ;
}

void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, char const* pValue )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( pTag ) ;
	pXML->AddAttribute( pAttribute, pValue ) ;
	pXML->EndTag( pTag ) ;
}

void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, uint64_t value )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( pTag ) ;
	xml_att_val( pAgent, pAttribute, value );
	pXML->EndTag( pTag ) ;
}

void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, int64_t value )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( pTag ) ;
	xml_att_val( pAgent, pAttribute, value );
	pXML->EndTag( pTag ) ;
}

void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, double value )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( pTag ) ;
	xml_att_val( pAgent, pAttribute, value );
	pXML->EndTag( pTag ) ;
}

void xml_object( agent* pAgent, wme* pWME, bool printTimetag ) {
	// <wme tag="123" id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	pXML->BeginTag( stn::kTagWME ) ;

	// BADBAD: These calls are redundantly converting pXML again. Possibly optimize.
	if ( printTimetag )
	{
		xml_att_val( pAgent, kWME_TimeTag, pWME->timetag );
	}
	xml_att_val( pAgent, kWME_Id, pWME->id );
	xml_att_val( pAgent, kWME_Attribute, pWME->attr );
	xml_att_val( pAgent, kWME_Value, pWME->value );
	xml_att_val( pAgent, kWME_ValueType, symbol_to_typeString( pAgent, pWME->value ) );

	if ( pWME->acceptable ) 
	{
		xml_att_val( pAgent, kWMEPreference, "+" );
	}

	pXML->EndTag( stn::kTagWME ) ;
}

void xml_generate_warning( agent* pAgent, const char* pMessage )
{
	xml_object( pAgent, stn::kTagWarning, stn::kTypeString, pMessage );
}

void xml_generate_error( agent* pAgent, const char* pMessage )
{
	xml_object( pAgent, stn::kTagError, stn::kTypeString, pMessage );
}

void xml_generate_message( agent* pAgent, const char* pMessage )
{
	xml_object( pAgent, stn::kTagMessage, stn::kTypeString, pMessage );
}

void xml_generate_verbose( agent* pAgent, const char* pMessage )
{
	xml_object( pAgent, stn::kTagVerbose, stn::kTypeString, pMessage );
}

void xml_invoke_callback( agent* pAgent )
{
	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );
	if ( pXML->IsEmpty() )
	{
		return;
	}

	soarxml::ElementXML* pResult = pXML->DetatchObject();
	pXML->Reset();

#ifdef _DEBUG
	char* pStr = pResult->GenerateXMLString( true ) ;
	pResult->DeleteString( pStr ) ;
#endif // _DEBUG

	// We need to call the handler explicitly here instead of using soar_invoke_callbacks
	// because we need to create a new ElementXML object for each handler that gets called.
	for ( cons* c = pAgent->soar_callbacks[XML_GENERATION_CALLBACK]; c != NIL; c = c->rest )
	{
		soarxml::ElementXML* pCallbackData = new soarxml::ElementXML( pResult->GetXMLHandle() );
		pCallbackData->AddRefOnHandle();

		soar_callback* cb = static_cast< soar_callback* >( c->first );

		// handler responsible for deleting pCallbackData
		cb->function( pAgent, cb->eventid, cb->data, static_cast< soar_call_data >( pCallbackData ) );
	}

	delete pResult;
}

soarxml::ElementXML* xml_get_xml( agent* pAgent )
{
	if ( !pAgent || !pAgent->xml_destination )
	{
		assert( pAgent );
		assert( pAgent->xml_destination );
		return 0;
	}

	soarxml::XMLTrace* pXML = static_cast< soarxml::XMLTrace* >( pAgent->xml_destination );

	soarxml::ElementXML* pReturn = pXML->DetatchObject();
	pXML->Reset();

	return pReturn;
}

void xml_begin_command_mode( agent* pAgent )
{
	if ( !pAgent || !pAgent->xml_trace || !pAgent->xml_commands )
	{
		assert( pAgent );
		assert( pAgent->xml_trace );
		assert( pAgent->xml_commands );
		return;
	}

	pAgent->xml_destination = pAgent->xml_commands;
}

soarxml::ElementXML* xml_end_command_mode( agent* pAgent )
{
	if ( !pAgent )
	{
		assert( pAgent );
		return 0;
	}

	soarxml::ElementXML* pReturn = xml_get_xml( pAgent );

	pAgent->xml_destination = pAgent->xml_trace;

	return pReturn;
}

