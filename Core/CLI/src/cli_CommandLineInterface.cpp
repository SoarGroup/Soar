#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "sml_Utils.h"

#include <assert.h>

#include <iostream>
#include <fstream>

#include "cli_Commands.h"

// SML includes
#include "sml_Connection.h"
#include "sml_TagResult.h"
#include "sml_TagArg.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "XMLTrace.h"
#include "KernelHeaders.h"

#include "agent.h"
#include "xml.h"
#include "lexer.h"

using namespace cli;
using namespace sml;
using namespace soarxml;

EXPORT CommandLineInterface::CommandLineInterface()
{
    m_pLogFile        = 0;
    m_TrapPrintEvents = false;
    m_pAgentSML       = 0 ;
    m_VarPrint        = false;
    m_GPMax           = 20000;
    m_XMLResult       = new XMLTrace() ;

    // parser takes ownership and deletes commands in its destructor
    m_Parser.AddCommand(new cli::AddWMECommand(*this));
    m_Parser.AddCommand(new cli::AliasCommand(*this));
    m_Parser.AddCommand(new cli::AllocateCommand(*this));
    m_Parser.AddCommand(new cli::CaptureInputCommand(*this));
    m_Parser.AddCommand(new cli::CDCommand(*this));
    m_Parser.AddCommand(new cli::ChunkNameFormatCommand(*this));
    m_Parser.AddCommand(new cli::CliExtensionMessageCommand(*this));
    m_Parser.AddCommand(new cli::CLogCommand(*this));
    m_Parser.AddCommand(new cli::CommandToFileCommand(*this));
    m_Parser.AddCommand(new cli::DebugCommand(*this));
    m_Parser.AddCommand(new cli::DefaultWMEDepthCommand(*this));
    m_Parser.AddCommand(new cli::DirsCommand(*this));
    m_Parser.AddCommand(new cli::EchoCommand(*this));
    m_Parser.AddCommand(new cli::EchoCommandsCommand(*this));
    m_Parser.AddCommand(new cli::EditProductionCommand(*this));
    m_Parser.AddCommand(new cli::EpMemCommand(*this));
    m_Parser.AddCommand(new cli::ExciseCommand(*this));
    m_Parser.AddCommand(new cli::ExplainBacktracesCommand(*this));
    m_Parser.AddCommand(new cli::FiringCountsCommand(*this));
    m_Parser.AddCommand(new cli::GDSPrintCommand(*this));
    m_Parser.AddCommand(new cli::GPCommand(*this));
    m_Parser.AddCommand(new cli::GPMaxCommand(*this));
    m_Parser.AddCommand(new cli::HelpCommand(*this));
    m_Parser.AddCommand(new cli::IndifferentSelectionCommand(*this));
    m_Parser.AddCommand(new cli::InitSoarCommand(*this));
    m_Parser.AddCommand(new cli::InternalSymbolsCommand(*this));
    m_Parser.AddCommand(new cli::LearnCommand(*this));
    m_Parser.AddCommand(new cli::LoadLibraryCommand(*this));
    m_Parser.AddCommand(new cli::LSCommand(*this));
    m_Parser.AddCommand(new cli::MatchesCommand(*this));
    m_Parser.AddCommand(new cli::MaxChunksCommand(*this));
    m_Parser.AddCommand(new cli::MaxDCTimeCommand(*this));
    m_Parser.AddCommand(new cli::MaxElaborationsCommand(*this));
    m_Parser.AddCommand(new cli::MaxGoalDepthCommand(*this));
    m_Parser.AddCommand(new cli::MaxMemoryUsageCommand(*this));
    m_Parser.AddCommand(new cli::MaxNilOutputCyclesCommand(*this));
    m_Parser.AddCommand(new cli::MemoriesCommand(*this));
    m_Parser.AddCommand(new cli::MultiAttributesCommand(*this));
    m_Parser.AddCommand(new cli::NumericIndifferentModeCommand(*this));
    m_Parser.AddCommand(new cli::OSupportModeCommand(*this));
    m_Parser.AddCommand(new cli::PbreakCommand(*this));
    m_Parser.AddCommand(new cli::PopDCommand(*this));
    m_Parser.AddCommand(new cli::PortCommand(*this));
    m_Parser.AddCommand(new cli::PredictCommand(*this));
    m_Parser.AddCommand(new cli::PreferencesCommand(*this));
    m_Parser.AddCommand(new cli::PrintCommand(*this));
    m_Parser.AddCommand(new cli::ProductionFindCommand(*this));
    m_Parser.AddCommand(new cli::PushDCommand(*this));
    m_Parser.AddCommand(new cli::PWatchCommand(*this));
    m_Parser.AddCommand(new cli::PWDCommand(*this));
    m_Parser.AddCommand(new cli::RandCommand(*this));
    m_Parser.AddCommand(new cli::RemoveWMECommand(*this));
    m_Parser.AddCommand(new cli::ReplayInputCommand(*this));
    m_Parser.AddCommand(new cli::ReteNetCommand(*this));
    m_Parser.AddCommand(new cli::RLCommand(*this));
    m_Parser.AddCommand(new cli::RunCommand(*this));
    m_Parser.AddCommand(new cli::SaveBacktracesCommand(*this));
    m_Parser.AddCommand(new cli::SelectCommand(*this));
    m_Parser.AddCommand(new cli::SetStopPhaseCommand(*this));
    m_Parser.AddCommand(new cli::SMemCommand(*this));
    m_Parser.AddCommand(new cli::SoarNewsCommand(*this));
    m_Parser.AddCommand(new cli::SourceCommand(*this));
    m_Parser.AddCommand(new cli::SPCommand(*this));
    m_Parser.AddCommand(new cli::SRandCommand(*this));
    m_Parser.AddCommand(new cli::StatsCommand(*this));
    m_Parser.AddCommand(new cli::StopSoarCommand(*this));
    m_Parser.AddCommand(new cli::TimeCommand(*this));
    m_Parser.AddCommand(new cli::TimersCommand(*this));
    m_Parser.AddCommand(new cli::UnaliasCommand(*this));
    m_Parser.AddCommand(new cli::VerboseCommand(*this));
    m_Parser.AddCommand(new cli::VersionCommand(*this));
    m_Parser.AddCommand(new cli::WaitSNCCommand(*this));
    m_Parser.AddCommand(new cli::WarningsCommand(*this));
    m_Parser.AddCommand(new cli::WatchCommand(*this));
    m_Parser.AddCommand(new cli::WatchWMEsCommand(*this));
    m_Parser.AddCommand(new cli::WMACommand(*this));
#ifndef NO_SVS
    m_Parser.AddCommand(new cli::SVSCommand(*this));
#endif
}

EXPORT CommandLineInterface::~CommandLineInterface()
{
    if (m_pLogFile)
    {
        (*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
        delete m_pLogFile;
    }

    delete m_XMLResult ;
    m_XMLResult = NULL ;
}

EXPORT bool CommandLineInterface::ShouldEchoCommand(char const* pCommandLine)
{
    if (!pCommandLine)
    {
        return false ;
    }

    // echo everything but edit-production
    return strncmp(pCommandLine, "edit-production", strlen("edit_production")) != 0;
}

EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, sml::AgentSML* pAgent, const char* pCommandLine, bool echoResults, bool rawOutput, ElementXML* pResponse)
{
    if (!m_pKernelSML)
    {
        return false;
    }

    PushCall(CallData(pAgent, rawOutput));

    // Log input
    if (m_pLogFile)
    {
        if (pAgent)
        {
            (*m_pLogFile) << pAgent->GetName() << "> ";
        }
        (*m_pLogFile) << pCommandLine << std::endl;
    }

    SetTrapPrintCallbacks(true);

    m_LastError.clear();

    Source(pCommandLine);

    SetTrapPrintCallbacks(false);

    if (pConnection && pResponse)
    {
        GetLastResultSML(pConnection, pResponse, echoResults);
    }

    PopCall();

    // Always returns true to indicate that we've generated any needed error message already
    return true;
}

void CommandLineInterface::PushCall(CallData callData)
{
    m_CallDataStack.push(callData);

    if (callData.pAgent)
    {
        m_pAgentSML = callData.pAgent;
    }
    else
    {
        m_pAgentSML = 0;
    }

    m_RawOutput = callData.rawOutput;

    // For kernel callback class we inherit
    SetAgentSML(m_pAgentSML) ;
}

void CommandLineInterface::PopCall()
{
    m_CallDataStack.pop();
    sml::AgentSML* pAgent = 0;

    if (m_CallDataStack.size())
    {
        const CallData& callData = m_CallDataStack.top();
        pAgent = callData.pAgent;
        m_RawOutput = callData.rawOutput;

        // reset these for the next command
        SetAgentSML(pAgent) ;
        m_pAgentSML = pAgent;
    }
}

void CommandLineInterface::SetTrapPrintCallbacks(bool setting)
{
    if (!m_pAgentSML)
    {
        return;
    }

    // If we've already set it, don't re-set it
    if (m_TrapPrintEvents == setting)
    {
        return;
    }

    if (setting)
    {
        // Trap print callbacks
        m_pAgentSML->DisablePrintCallback();
        m_TrapPrintEvents = true;
        if (!m_pLogFile)
            // If we're logging, we're already registered for this.
        {
            RegisterWithKernel(smlEVENT_PRINT);
        }

        // Tell kernel to collect result in command buffer as opposed to trace buffer
        xml_begin_command_mode(m_pAgentSML->GetSoarAgent());
    }
    else
    {
        // Retrieve command buffer, tell kernel to use trace buffer again
        ElementXML* pXMLCommandResult = xml_end_command_mode(m_pAgentSML->GetSoarAgent());

        // The root object is just a <trace> tag.  The substance is in the children
        // Add children of the command buffer to response tags
        for (int i = 0; i < pXMLCommandResult->GetNumberChildren(); ++i)
        {
            ElementXML* pChildXML = new ElementXML();
            pXMLCommandResult->GetChild(pChildXML, i);

            m_ResponseTags.push_back(pChildXML);
        }

        delete pXMLCommandResult;

        if (!m_RawOutput)
        {
            // Add text result to response tags
            if (m_Result.str().length())
            {
                AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.str());
                m_Result.str("");
            }
        }

        // Re-enable print callbacks
        if (!m_pLogFile)
        {
            // If we're logging, we want to stay registered for this
            UnregisterWithKernel(smlEVENT_PRINT);
        }
        m_TrapPrintEvents = false;
        m_pAgentSML->EnablePrintCallback();
    }
}

void CommandLineInterface::GetLastResultSML(sml::Connection* pConnection, soarxml::ElementXML* pResponse, bool echoResults)
{
    assert(pConnection);
    assert(pResponse);

    // Log output
    if (m_pLogFile)
    {
        (*m_pLogFile) << m_Result.str() << std::endl;
    }

    if (m_LastError.empty())
    {
        if (m_RawOutput)
        {
            pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.str().c_str());
        }
        else
        {
            // If there are tags in the response list, add them
            if (!m_ResponseTags.empty())
            {
                TagResult* pTag = new TagResult();

                ElementXMLListIter iter = m_ResponseTags.begin();
                while (iter != m_ResponseTags.end())
                {
                    pTag->AddChild(*iter);
                    m_ResponseTags.erase(iter);
                    iter = m_ResponseTags.begin();
                }

                pResponse->AddChild(pTag);
            }
            else
            {
                pConnection->AddSimpleResultToSMLResponse(pResponse, sml_Names::kTrue);
            }
        }
    }
    else
    {
        pConnection->AddErrorToSMLResponse(pResponse, m_Result.str().c_str(), 1);
    }

    if (echoResults && m_pAgentSML)
    {
        m_pAgentSML->FireEchoEvent(pConnection, m_Result.str().c_str()) ;
    }

    // reset state
    m_Result.str("");

    // Delete all remaining xml objects
    for (ElementXMLListIter cleanupIter = m_ResponseTags.begin(); cleanupIter != m_ResponseTags.end(); ++cleanupIter)
    {
        delete *cleanupIter;
    }

    m_ResponseTags.clear();
}

bool CommandLineInterface::CheckForHelp(std::vector<std::string>& argv)
{
    // Standard help check if there is more than one argument
    if (argv.size() > 1)
    {
        // Is one of the two help strings present?
        if (argv[1] == "-h" || argv[1] == "--help")
        {
            return true;
        }
    }
    return false;
}

EXPORT void CommandLineInterface::SetKernel(sml::KernelSML* pKernelSML)
{
    m_pKernelSML = pKernelSML;
}

bool CommandLineInterface::GetCurrentWorkingDirectory(std::string& directory)
{
    // Pull an arbitrary buffer size of 1024 out of a hat and use it
    char buf[1024];
    char* ret = getcwd(buf, 1024);

    // If getcwd returns 0, that is bad
    if (!ret)
    {
        return SetError("Error getting current working directory.");
    }

    // Store directory in output parameter and return success
    directory = buf;
    normalize_separators(directory);
    return true;
}

void CommandLineInterface::AppendArgTag(const char* pParam, const char* pType, const std::string& value)
{
    AppendArgTag(pParam, pType, value.c_str());
}

void CommandLineInterface::AppendArgTagFast(const char* pParam, const char* pType, const std::string& value)
{
    AppendArgTagFast(pParam, pType, value.c_str());
}

void CommandLineInterface::PrependArgTag(const char* pParam, const char* pType, const std::string& value)
{
    PrependArgTag(pParam, pType, value.c_str());
}

void CommandLineInterface::PrependArgTagFast(const char* pParam, const char* pType, const std::string& value)
{
    PrependArgTagFast(pParam, pType, value.c_str());
}

void CommandLineInterface::AppendArgTag(const char* pParam, const char* pType, const char* pValue)
{
    TagArg* pTag = new TagArg();
    pTag->SetParam(pParam);
    pTag->SetType(pType);
    pTag->SetValue(pValue);
    m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::AppendArgTagFast(const char* pParam, const char* pType, const char* pValue)
{
    TagArg* pTag = new TagArg();
    pTag->SetParamFast(pParam);
    pTag->SetTypeFast(pType);
    pTag->SetValue(pValue);
    m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::PrependArgTag(const char* pParam, const char* pType, const char* pValue)
{
    TagArg* pTag = new TagArg();
    pTag->SetParam(pParam);
    pTag->SetType(pType);
    pTag->SetValue(pValue);
    m_ResponseTags.push_front(pTag);
}

void CommandLineInterface::PrependArgTagFast(const char* pParam, const char* pType, const char* pValue)
{
    TagArg* pTag = new TagArg();
    pTag->SetParamFast(pParam);
    pTag->SetTypeFast(pType);
    pTag->SetValue(pValue);
    m_ResponseTags.push_front(pTag);
}

bool CommandLineInterface::SetError(const std::string& error)
{
    if (!m_Result.str().empty())
        if (m_Result.str().at(m_Result.str().length() - 1) != '\n')
        {
            m_Result << std::endl;
        }
    m_Result << error;
    m_LastError = error;
    return false;
}

bool CommandLineInterface::AppendError(const std::string& error)
{
    m_Result << error;
    m_LastError.append(error);
    return false;
}

void CommandLineInterface::XMLBeginTag(char const* pTagName)
{
    m_XMLResult->BeginTag(pTagName) ;
}

void CommandLineInterface::XMLAddAttribute(char const* pAttribute, char const* pValue)
{
    m_XMLResult->AddAttribute(pAttribute, pValue) ;
}

void CommandLineInterface::XMLEndTag(char const* pTagName)
{
    m_XMLResult->EndTag(pTagName) ;
}

bool CommandLineInterface::XMLMoveCurrentToParent()
{
    return m_XMLResult->MoveCurrentToParent() ;
}

bool CommandLineInterface::XMLMoveCurrentToChild(int index)
{
    return m_XMLResult->MoveCurrentToChild(index) ;
}

bool CommandLineInterface::XMLMoveCurrentToLastChild()
{
    return m_XMLResult->MoveCurrentToLastChild() ;
}

void CommandLineInterface::XMLResultToResponse(char const* pCommandName)
{
    // The copies over the m_XMLResult object to the response XML object and sets the
    // tag name to the command that was just executed.
    // The result is XML in this format (e.g. for matches):
    // <result><matches>...</matches></result>
    // where ... contains the XML specific to that command.

    // Extract the XML object from the xmlTrace object and
    // add it as a child of this message.  This is just moving a few pointers around, nothing is getting copied.
    ElementXML_Handle xmlHandle = m_XMLResult->Detach() ;
    ElementXML* pXMLResult = new ElementXML(xmlHandle) ;
    pXMLResult->SetTagName(pCommandName) ;

    m_ResponseTags.push_back(pXMLResult) ;

    // Clear the XML result, so it's ready for use again.
    m_XMLResult->Reset() ;
}

void CommandLineInterface::OnKernelEvent(int eventID, AgentSML*, void* pCallData)
{
    if (eventID == smlEVENT_PRINT)
    {
        char const* msg = static_cast<char const*>(pCallData);

        if (m_TrapPrintEvents || m_pLogFile)
        {
            if (m_VarPrint)
            {
                // Transform if varprint, see print command
                std::string message(msg);

                regex_t comp;
                regcomp(&comp, "[A-Z][0-9]+", REG_EXTENDED);

                regmatch_t match;
                memset(&match, 0, sizeof(regmatch_t));

                while (regexec(&comp, message.substr(match.rm_eo, message.size() - match.rm_eo).c_str(), 1, &match, 0) == 0)
                {
                    message.insert(match.rm_so, "<");
                    message.insert(match.rm_eo + 1, ">");
                    match.rm_eo += 2;
                }

                regfree(&comp);

                // Simply append to message result
                if (m_TrapPrintEvents)
                {
                    CommandLineInterface::m_Result << message;
                    //std::cout << msg;
                    //std::cout.flush();
                }
                else if (m_pLogFile)
                {
                    (*m_pLogFile) << msg;
                }
            }
            else
            {
                if (m_TrapPrintEvents)
                {
                    CommandLineInterface::m_Result << msg;
                    //std::cout << msg;
                    //std::cout.flush();
                }
                else if (m_pLogFile)
                {
                    (*m_pLogFile) << msg;
                }
            }
        }
    }
    else if (eventID == smlEVENT_BEFORE_PRODUCTION_REMOVED)
    {
        // Only called when source command is active
        production* p = static_cast<production*>(pCallData);
        assert(p);
        assert(p->name->sc->name);
        m_ExcisedDuringSource.push_back(std::string(p->name->sc->name));
    }
    else
    {
        assert(false);
        // unknown event
        // TODO: gracefully (?) deal with this error
    }
} // function

bool CommandLineInterface::IsLogOpen()
{
    return m_pLogFile ? true : false;
}

void CommandLineInterface::PrintCLIMessage(const char* printString, bool add_raw_lf)
{
    if (m_RawOutput)
    {
        m_Result << printString << (add_raw_lf ? "\n" : "");
    }
    else
    {
        AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, printString);
    }

}
void CommandLineInterface::PrintCLIMessage(std::string* printString, bool add_raw_lf)
{
    PrintCLIMessage(printString->c_str(), add_raw_lf);
}

void CommandLineInterface::PrintCLIMessage(std::ostringstream* printString, bool add_raw_lf)
{
    PrintCLIMessage(printString->str().c_str(), add_raw_lf);
}

void CommandLineInterface::PrintCLIMessage_Justify(const char* prefixString, const char* printString, int column_width, bool add_raw_lf)
{
    std::ostringstream tempString;
    int left_width, right_width, middle_width;
    std::string sep_string("");

    left_width = strlen(prefixString);
    right_width = strlen(printString);
    middle_width = column_width - left_width - right_width;
    if (middle_width < 0)
    {
        middle_width = 1;
    }
    sep_string.insert(0, middle_width, ' ');

    tempString << prefixString << sep_string << printString;
    PrintCLIMessage(&tempString);
}

void CommandLineInterface::PrintCLIMessage_Item(const char* prefixString, soar_module::named_object* printObject, int column_width, bool add_raw_lf)
{
    std::ostringstream tempString;
    char* temp = printObject->get_string();
    PrintCLIMessage_Justify(prefixString, temp, column_width);
    delete temp;

}

void CommandLineInterface::PrintCLIMessage_Header(const char* headerString, int column_width, bool add_raw_lf)
{
    std::ostringstream tempString;
    int left_width, right_width, header_width;
    std::string left_string(""), right_string(""), sep_string("");

    header_width = strlen(headerString) + 2;
    left_width = (column_width - header_width) / 2;
    right_width = column_width - left_width - header_width;
    left_string.insert(0, left_width, ' ');
    right_string.insert(0, right_width, ' ');
    sep_string.insert(0, column_width, '=');

    tempString << "" << left_string << ' ' << headerString << ' ' << right_string << "";

    PrintCLIMessage(&sep_string);
    PrintCLIMessage(&tempString);
    PrintCLIMessage(&sep_string);
}

void CommandLineInterface::PrintCLIMessage_Section(const char* headerString, int column_width, bool add_raw_lf)
{
    std::ostringstream tempString;
    int left_width, right_width, header_width;
    std::string left_string(""), right_string("");

    header_width = strlen(headerString) + 2;
    left_width = (column_width - header_width) / 2;
    right_width = column_width - left_width - header_width;

    left_string.insert(0, left_width, '-');
    right_string.insert(0, right_width, '-');
    tempString << left_string << ' ' << headerString << ' ' << right_string;

    PrintCLIMessage(&tempString);
}

void get_context_var_info(agent* thisAgent, const char* var_name,
                          Symbol** dest_goal,
                          Symbol** dest_attr_of_slot,
                          Symbol** dest_current_value)
{
    Symbol* v, *g;
    int levels_up;
    wme* w;

    v = find_variable(thisAgent, var_name);
    if (v == thisAgent->s_context_variable)
    {
        levels_up = 0;
        *dest_attr_of_slot = thisAgent->state_symbol;
    }
    else if (v == thisAgent->o_context_variable)
    {
        levels_up = 0;
        *dest_attr_of_slot = thisAgent->operator_symbol;
    }
    else if (v == thisAgent->ss_context_variable)
    {
        levels_up = 1;
        *dest_attr_of_slot = thisAgent->state_symbol;
    }
    else if (v == thisAgent->so_context_variable)
    {
        levels_up = 1;
        *dest_attr_of_slot = thisAgent->operator_symbol;
    }
    else if (v == thisAgent->sss_context_variable)
    {
        levels_up = 2;
        *dest_attr_of_slot = thisAgent->state_symbol;
    }
    else if (v == thisAgent->sso_context_variable)
    {
        levels_up = 2;
        *dest_attr_of_slot = thisAgent->operator_symbol;
    }
    else if (v == thisAgent->ts_context_variable)
    {
        levels_up = thisAgent->top_goal ? thisAgent->bottom_goal->id->level - thisAgent->top_goal->id->level : 0;
        *dest_attr_of_slot = thisAgent->state_symbol;
    }
    else if (v == thisAgent->to_context_variable)
    {
        levels_up = thisAgent->top_goal ? thisAgent->bottom_goal->id->level - thisAgent->top_goal->id->level : 0;
        *dest_attr_of_slot = thisAgent->operator_symbol;
    }
    else
    {
        *dest_goal = NIL;
        *dest_attr_of_slot = NIL;
        *dest_current_value = NIL;
        return;
    }

    g = thisAgent->bottom_goal;
    while (g && levels_up)
    {
        g = g->id->higher_goal;
        levels_up--;
    }
    *dest_goal = g;

    if (!g)
    {
        *dest_current_value = NIL;
        return;
    }

    if (*dest_attr_of_slot == thisAgent->state_symbol)
    {
        *dest_current_value = g;
    }
    else
    {
        w = g->id->operator_slot->wmes;
        *dest_current_value = w ? w->value : NIL;
    }
}

bool read_id_or_context_var_from_string(agent* thisAgent, const char* lex_string,
                                        Symbol** result_id)
{
    Symbol* id;
    Symbol* g, *attr, *value;

    soar::Lexeme lexeme = get_lexeme_from_string(thisAgent, lex_string);

    if (lexeme.type == IDENTIFIER_LEXEME)
    {
        id = find_identifier(thisAgent, lexeme.id_letter, lexeme.id_number);
        if (!id)
        {
            return false;
        }
        else
        {
            *result_id = id;
            return true;
        }
    }

    if (lexeme.type == VARIABLE_LEXEME)
    {
        get_context_var_info(thisAgent, lexeme.string(), &g, &attr, &value);

        if ((!attr) || (!value))
        {
            return false;
        }

        if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE)
        {
            return false;
        }

        *result_id = value;
        return true;
    }

    return false;
}

soar::Lexeme get_lexeme_from_string(agent* thisAgent, const char* lex_string)
{
    soar::Lexer lexer(thisAgent, lex_string);
    lexer.get_lexeme();
    return lexer.current_lexeme;
}

Symbol* read_identifier_or_context_variable(agent* thisAgent, soar::Lexeme* lexeme)
{
    Symbol* id;
    Symbol* g, *attr, *value;

    if (lexeme->type == IDENTIFIER_LEXEME)
    {
        id = find_identifier(thisAgent, lexeme->id_letter, lexeme->id_number);
        if (!id)
        {
            print(thisAgent,  "There is no identifier %c%lu.\n", lexeme->id_letter,
                  lexeme->id_number);
            // TODO: store location in lexeme and then rewrite comment print statements
            // lexer->print_location_of_most_recent_lexeme();
            return NIL;
        }
        return id;
    }
    if (lexeme->type == VARIABLE_LEXEME)
    {
        get_context_var_info(thisAgent, lexeme->string(), &g, &attr, &value);
        if (!attr)
        {
            print(thisAgent,  "Expected identifier (or context variable)\n");
            // print_location_of_most_recent_lexeme();
            return NIL;
        }
        if (!value)
        {
            print(thisAgent,  "There is no current %s.\n", lexeme->string());
            // lexer->print_location_of_most_recent_lexeme();
            return NIL;
        }
        if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE)
        {
            print(thisAgent,  "The current %s ", lexeme->string());
            print_with_symbols(thisAgent, "(%y) is not an identifier.\n", value);
            // lexer->print_location_of_most_recent_lexeme();
            return NIL;
        }
        return value;
    }
    print(thisAgent,  "Expected identifier (or context variable)\n");
    // lexer->print_location_of_most_recent_lexeme();
    return NIL;
}
