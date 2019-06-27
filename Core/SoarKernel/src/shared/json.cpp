/*************************************************************************
 *
 *  file:  json.cpp
 *
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
 *
 * =======================================================================
 */

#include "json.h"

#include "agent.h"
#include "callback.h"
#include "ElementJSON.h"
#include "print.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "working_memory.h"
#include "JSONTrace.h"

#include <assert.h>

using namespace soar_TraceNames;
namespace stn = soar_TraceNames;

void json_create(agent* pAgent)
{
    if (!pAgent)
    {
        assert(pAgent);
        return;
    }

    soarjson::JSONTrace* pTrace = new soarjson::JSONTrace();
    soarjson::JSONTrace* pCommands = new soarjson::JSONTrace();

    pAgent->json_trace = static_cast< json_handle >(pTrace);
    pAgent->json_commands = static_cast< json_handle >(pCommands);

    pAgent->json_destination = pAgent->json_trace;
}

void json_reset(agent* pAgent)
{
    if (!pAgent || !pAgent->json_trace || !pAgent->json_commands)
    {
        assert(pAgent);
        assert(pAgent->json_trace);
        assert(pAgent->json_commands);
        return;
    }

    soarjson::JSONTrace* pTrace = static_cast< soarjson::JSONTrace* >(pAgent->json_trace);
    soarjson::JSONTrace* pCommands = static_cast< soarjson::JSONTrace* >(pAgent->json_commands);

    pTrace->Reset();
    pCommands->Reset();
}

void json_destroy(agent* pAgent)
{
    if (!pAgent || !pAgent->json_trace || !pAgent->json_commands)
    {
        assert(pAgent);
        assert(pAgent->json_trace);
        assert(pAgent->json_commands);
        return;
    }

    soarjson::JSONTrace* pTrace = static_cast< soarjson::JSONTrace* >(pAgent->json_trace);
    soarjson::JSONTrace* pCommands = static_cast< soarjson::JSONTrace* >(pAgent->json_commands);

    delete pTrace;
    delete pCommands;

    pAgent->json_trace = 0;
    pAgent->json_commands = 0;

    pAgent->json_destination = 0;
}

void json_begin_tag(agent* pAgent, char const* pTag)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->BeginTag(pTag) ;
}

void json_end_tag(agent* pAgent, char const* pTag)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->EndTag(pTag) ;
}

// These "moveCurrent" methods allow us to move the entry point for new JSON
// around in the existing structure.  That's not often required but occassionally is helpful.
void json_move_current_to_parent(agent* pAgent)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->MoveCurrentToParent();
}

void json_move_current_to_child(agent* pAgent, int index)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->MoveCurrentToChild(index);
}

void json_move_current_to_last_child(agent* pAgent)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->MoveCurrentToLastChild();
}

void json_att_val(agent* pAgent, char const* pAttribute, uint64_t value)
{
    char buf[51];
    SNPRINTF(buf, 50, "%llu", static_cast<long long unsigned>(value));

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, buf) ;
}

void json_att_val(agent* pAgent, char const* pAttribute, int value)
{
    char buf[51];
    SNPRINTF(buf, 50, "%d", value);

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, buf) ;
}

void json_att_val(agent* pAgent, char const* pAttribute, int64_t value)
{
    char buf[51];
    SNPRINTF(buf, 50, "%lld", static_cast<long long>(value));

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, buf) ;
}

void json_att_val(agent* pAgent, char const* pAttribute, double value)
{
    char buf[51];
    SNPRINTF(buf, 50, "%f", value);

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, buf) ;
}

void json_att_val(agent* pAgent, char const* pAttribute, char const* pValue)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, pValue) ;
}

void json_att_val(agent* pAgent, char const* pAttribute, Symbol* pSymbol)
{
    // Passing 0, 0 as buffer to symbol to string causes it to use internal, temporary buffers
    // which is fine because we immediately copy that string in JSONAddAttribute.
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    pJSON->AddAttribute(pAttribute, pSymbol->to_string(true)) ;
}

void json_object(agent* pAgent, char const* pTag)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(pTag) ;
    pJSON->EndTag(pTag) ;
}

void json_object(agent* pAgent, char const* pTag, char const* pAttribute, char const* pValue)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(pTag) ;
    pJSON->AddAttribute(pAttribute, pValue) ;
    pJSON->EndTag(pTag) ;
}

void json_object(agent* pAgent, char const* pTag, char const* pAttribute, uint64_t value)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(pTag) ;
    json_att_val(pAgent, pAttribute, value);
    pJSON->EndTag(pTag) ;
}

void json_object(agent* pAgent, char const* pTag, char const* pAttribute, int64_t value)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(pTag) ;
    json_att_val(pAgent, pAttribute, value);
    pJSON->EndTag(pTag) ;
}

void json_object(agent* pAgent, char const* pTag, char const* pAttribute, double value)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(pTag) ;
    json_att_val(pAgent, pAttribute, value);
    pJSON->EndTag(pTag) ;
}

inline char const* symbol_type_to_string(byte pType)
{
    switch (pType)
    {
        case VARIABLE_SYMBOL_TYPE:
            return soar_TraceNames::kTypeVariable ;
        case IDENTIFIER_SYMBOL_TYPE:
            return soar_TraceNames::kTypeID ;
        case INT_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeInt ;
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeDouble ;
        case STR_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeString ;
        default:
            return "UNDEFINED!";
    }
}

void json_object(agent* pAgent, wme* pWME, bool printTimetag)
{
    // <wme tag="123" id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    pJSON->BeginTag(stn::kTagWME) ;

    // BADBAD: These calls are redundantly converting pJSON again. Possibly optimize.
    if (printTimetag)
    {
        json_att_val(pAgent, kWME_TimeTag, pWME->timetag);
    }
    json_att_val(pAgent, kWME_Id, pWME->id);
    json_att_val(pAgent, kWME_Attribute, pWME->attr);
    json_att_val(pAgent, kWME_Value, pWME->value);
    json_att_val(pAgent, kWME_ValueType, symbol_type_to_string(pWME->value->symbol_type));

    if (pWME->acceptable)
    {
        json_att_val(pAgent, kWMEPreference, "+");
    }

    pJSON->EndTag(stn::kTagWME) ;
}

void json_generate_warning(agent* pAgent, const char* pMessage)
{
    json_object(pAgent, stn::kTagWarning, stn::kTypeString, pMessage);
}

void json_generate_error(agent* pAgent, const char* pMessage)
{
    json_object(pAgent, stn::kTagError, stn::kTypeString, pMessage);
}

void json_generate_message(agent* pAgent, const char* pMessage)
{
    json_object(pAgent, stn::kTagMessage, stn::kTypeString, pMessage);
}

void json_generate_verbose(agent* pAgent, const char* pMessage)
{
    json_object(pAgent, stn::kTagVerbose, stn::kTypeString, pMessage);
}

void json_invoke_callback(agent* pAgent)
{
    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);
    if (pJSON->IsEmpty())
    {
        return;
    }

    soarjson::ElementJSON* pResult = pJSON->DetatchObject();
    pJSON->Reset();

#ifdef _DEBUG
    char* pStr = pResult->GenerateJSONString(true) ;
    pResult->DeleteString(pStr) ;
#endif // _DEBUG

    // We need to call the handler explicitly here instead of using soar_invoke_callbacks
    // because we need to create a new ElementJSON object for each handler that gets called.
    for (cons* c = pAgent->soar_callbacks[JSON_GENERATION_CALLBACK]; c != NIL; c = c->rest)
    {
        soarjson::ElementJSON* pCallbackData = new soarjson::ElementJSON(pResult->GetJSONHandle());
        pCallbackData->AddRefOnHandle();

        soar_callback* cb = static_cast< soar_callback* >(c->first);

        // handler responsible for deleting pCallbackData
        cb->function(pAgent, cb->eventid, cb->data, static_cast< soar_call_data >(pCallbackData));
    }

    delete pResult;
}

soarjson::ElementJSON* json_get_json(agent* pAgent)
{
    if (!pAgent || !pAgent->json_destination)
    {
        assert(pAgent);
        assert(pAgent->json_destination);
        return 0;
    }

    soarjson::JSONTrace* pJSON = static_cast< soarjson::JSONTrace* >(pAgent->json_destination);

    soarjson::ElementJSON* pReturn = pJSON->DetatchObject();
    pJSON->Reset();

    return pReturn;
}

void json_begin_command_mode(agent* pAgent)
{
    if (!pAgent || !pAgent->json_trace || !pAgent->json_commands)
    {
        assert(pAgent);
        assert(pAgent->json_trace);
        assert(pAgent->json_commands);
        return;
    }

    pAgent->json_destination = pAgent->json_commands;
}

soarjson::ElementJSON* json_end_command_mode(agent* pAgent)
{
    if (!pAgent)
    {
        assert(pAgent);
        return 0;
    }

    soarjson::ElementJSON* pReturn = json_get_json(pAgent);

    pAgent->json_destination = pAgent->json_trace;

    return pReturn;
}

