/* =======================================================================
                                 json.h
 * =======================================================================
 *
 * Contains methods for generating JSON objects in response to kernel commands.
 *
 * The commands are modelled after the existing kernel functions which are tied to generating
 * string output.  In the past we added code to some of those functions so they'd
 * generate string output and also JSON output (as a string: <aaa>...</aaa>).  To capture
 * the JSON output a caller would register for the JSON callback, generate the JSON as a string,
 * parse the JSON back to an object and return it.
 *
 * These methods generate JSON as an object, so there are no strings being created
 * and subsequently parsed and no need to "intercept" the JSON callback channel (which is
 * really for JSON trace output to the debugger, not for results from commands).
 * This new approach is more efficient to both create and to subsequently use.

======================================================================= */

#ifndef SOAR_JSON_H
#define SOAR_JSON_H

#include "kernel.h"

void json_create(agent* pAgent);
void json_reset(agent* pAgent);
void json_destroy(agent* pAgent);

void json_begin_tag(agent* pAgent, char const* pTag) ;
void json_end_tag(agent* pAgent, char const* pTag) ;

void json_move_current_to_parent(agent* pAgent) ;
void json_move_current_to_child(agent* pAgent, int index) ;
void json_move_current_to_last_child(agent* pAgent) ;

void json_att_val(agent* pAgent, char const* pAttribute, uint64_t value) ;
void json_att_val(agent* pAgent, char const* pAttribute, int value) ;
void json_att_val(agent* pAgent, char const* pAttribute, int64_t value) ;
void json_att_val(agent* pAgent, char const* pAttribute, double value) ;
void json_att_val(agent* pAgent, char const* pAttribute, char const* pValue) ;
void json_att_val(agent* pAgent, char const* pAttribute, Symbol* pSymbol) ;

void json_object(agent* pAgent, char const* pTag) ;
void json_object(agent* pAgent, char const* pTag, char const* pAttribute, char const* pValue) ;
void json_object(agent* pAgent, char const* pTag, char const* pAttribute, uint64_t value) ;
void json_object(agent* pAgent, char const* pTag, char const* pAttribute, int64_t value) ;
void json_object(agent* pAgent, char const* pTag, char const* pAttribute, double value) ;

#define JSON_WME_NO_TIMETAG false
void json_object(agent* pAgent, wme* pWME, bool printTimetag = true) ;

void json_generate_warning(agent* pAgent, const char* pMessage);
void json_generate_error(agent* pAgent, const char* pMessage);
void json_generate_message(agent* pAgent, const char* pMessage);
void json_generate_verbose(agent* pAgent, const char* pMessage);

void json_invoke_callback(agent* pAgent);

// BADBAD: The kernel should not use these methods. This method should probably be in a different header.
namespace soarjson
{
    class ElementJSON;
}

soarjson::ElementJSON* json_get_json(agent* pAgent);
void json_begin_command_mode(agent* pAgent);
soarjson::ElementJSON* json_end_command_mode(agent* pAgent);

#endif
