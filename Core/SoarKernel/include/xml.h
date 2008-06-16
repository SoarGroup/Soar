/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                 xml.h
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

======================================================================= */

#ifndef SOAR_XML_H
#define SOAR_XML_H

typedef struct agent_struct agent;
typedef struct wme_struct wme;
typedef union symbol_union Symbol;

void xml_create( agent* pAgent );
void xml_reset( agent* pAgent );
void xml_destroy( agent* pAgent );

void xml_begin_tag( agent* pAgent, char const* pTag ) ;
void xml_end_tag( agent* pAgent, char const* pTag ) ;

void xml_move_current_to_parent( agent* pAgent ) ;
void xml_move_current_to_child( agent* pAgent, int index ) ;
void xml_move_current_to_last_child( agent* pAgent ) ;

void xml_att_val( agent* pAgent, char const* pAttribute, unsigned long value ) ;
void xml_att_val( agent* pAgent, char const* pAttribute, int value ) ;
void xml_att_val( agent* pAgent, char const* pAttribute, long value ) ;
void xml_att_val( agent* pAgent, char const* pAttribute, double value ) ;
void xml_att_val( agent* pAgent, char const* pAttribute, char const* pValue ) ;
void xml_att_val( agent* pAgent, char const* pAttribute, Symbol* pSymbol ) ;

void xml_object( agent* pAgent, char const* pTag ) ;
void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, char const* pValue ) ;
void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, unsigned long value ) ;
void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, long value ) ;
void xml_object( agent* pAgent, char const* pTag, char const* pAttribute, double value ) ;

#define XML_WME_NO_TIMETAG false
void xml_object( agent* pAgent, wme* pWME, bool printTimetag = true ) ;

void xml_generate_warning( agent* pAgent, const char* pMessage);
void xml_generate_error( agent* pAgent, const char* pMessage);
void xml_generate_message( agent* pAgent, const char* pMessage);
void xml_generate_verbose( agent* pAgent, const char* pMessage);

void xml_invoke_callback( agent* pAgent );

// BADBAD: The kernel should not use these methods. This method should probably be in a different header.
namespace soarxml
{
	class ElementXML;
}

soarxml::ElementXML* xml_get_xml( agent* pAgent );
void xml_begin_command_mode( agent* pAgent );
soarxml::ElementXML* xml_end_command_mode( agent* pAgent );

#endif
