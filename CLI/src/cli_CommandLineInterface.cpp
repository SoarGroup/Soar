#include <portability.h>

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
    m_Parser.AddCommand(new cli::CLogCommand(*this));
    m_Parser.AddCommand(new cli::CommandToFileCommand(*this));
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
    m_Parser.AddCommand(new cli::MaxElaborationsCommand(*this));
    m_Parser.AddCommand(new cli::MaxGoalDepthCommand(*this));
    m_Parser.AddCommand(new cli::MaxMemoryUsageCommand(*this));
    m_Parser.AddCommand(new cli::MaxNilOutputCyclesCommand(*this));
    m_Parser.AddCommand(new cli::MemoriesCommand(*this));
    m_Parser.AddCommand(new cli::MultiAttributesCommand(*this));
    m_Parser.AddCommand(new cli::NumericIndifferentModeCommand(*this));
    m_Parser.AddCommand(new cli::OSupportModeCommand(*this));
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
    m_Parser.AddCommand(new cli::SetLibraryLocationCommand(*this));
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
        return false ;

    // echo everything but edit-production
    return strncmp(pCommandLine, "edit-production", strlen("edit_production")) != 0;
}

EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, sml::AgentSML* pAgent, const char* pCommandLine, bool echoResults, bool rawOutput, ElementXML* pResponse) 
{
    if (!m_pKernelSML) return false;

    PushCall( CallData(pAgent, rawOutput) );

    // Log input
    if (m_pLogFile) 
    {
        if (pAgent) (*m_pLogFile) << pAgent->GetName() << "> ";
        (*m_pLogFile) << pCommandLine << std::endl;
    }

    SetTrapPrintCallbacks( true );

    m_LastError.clear();

    soar::tokenizer tokenizer;
    tokenizer.set_handler(&m_Parser);
    if (!tokenizer.evaluate(pCommandLine))
    {
        if (!m_Parser.GetError().empty())
            m_LastError = m_Parser.GetError();
        else if (!tokenizer.get_error_string())
            m_LastError = tokenizer.get_error_string();
    }

    SetTrapPrintCallbacks( false );

    if (pConnection && pResponse)
        GetLastResultSML(pConnection, pResponse, echoResults);

    PopCall();

    // Always returns true to indicate that we've generated any needed error message already
    return true;
}

void CommandLineInterface::PushCall( CallData callData )
{
    m_CallDataStack.push( callData );

    if (callData.pAgent) 
        m_pAgentSML = callData.pAgent;
    else 
        m_pAgentSML = 0;

    m_RawOutput = callData.rawOutput;

    // For kernel callback class we inherit
    SetAgentSML(m_pAgentSML) ;
}

void CommandLineInterface::PopCall()
{
    m_CallDataStack.pop();
    sml::AgentSML* pAgent = 0;
    
    if ( m_CallDataStack.size() )
    {
        const CallData& callData = m_CallDataStack.top();
        pAgent = callData.pAgent;
        m_RawOutput = callData.rawOutput;

        // reset these for the next command
        SetAgentSML( pAgent ) ;
        m_pAgentSML = pAgent;
    }
}

void CommandLineInterface::SetTrapPrintCallbacks(bool setting)
{
    if (!m_pAgentSML)
        return;

    // If we've already set it, don't re-set it
    if ( m_TrapPrintEvents == setting )
        return;

    if (setting)
    {
        // Trap print callbacks
        m_pAgentSML->DisablePrintCallback();
        m_TrapPrintEvents = true;
        if (!m_pLogFile) 
            // If we're logging, we're already registered for this.
            RegisterWithKernel(smlEVENT_PRINT);

        // Tell kernel to collect result in command buffer as opposed to trace buffer
        xml_begin_command_mode( m_pAgentSML->GetSoarAgent() );
    }
    else
    {
        // Retrieve command buffer, tell kernel to use trace buffer again
        ElementXML* pXMLCommandResult = xml_end_command_mode( m_pAgentSML->GetSoarAgent() );

        // The root object is just a <trace> tag.  The substance is in the children
        // Add childrend of the command buffer to response tags
        for ( int i = 0; i < pXMLCommandResult->GetNumberChildren(); ++i )
        {
            ElementXML* pChildXML = new ElementXML();
            pXMLCommandResult->GetChild( pChildXML, i );

            m_ResponseTags.push_back( pChildXML );
        }

        delete pXMLCommandResult;

        if ( !m_RawOutput )
        {
            // Add text result to response tags
            if ( m_Result.str().length() )
            {
                AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.str() );
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

    if (m_LastError.empty()) 
    {
        // Log output
        if (m_pLogFile) (*m_pLogFile) << m_Result.str() << std::endl;

        // The command succeeded, so return the result if raw output
        if (m_RawOutput) 
        {
            pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.str().c_str());
            if (echoResults && m_pAgentSML)
                m_pAgentSML->FireEchoEvent(pConnection, m_Result.str().c_str()) ;
        } 
        else 
        {
            // If there are tags in the response list, add them and return
            if (m_ResponseTags.size()) 
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
                // Or, simply return true
                pConnection->AddSimpleResultToSMLResponse(pResponse, sml_Names::kTrue);
            }
        }
    }
    else 
    {
        // The command failed, add the error message
        if (!m_Result.str().empty())
            m_Result << std::endl;
        m_Result << m_LastError;

        pConnection->AddErrorToSMLResponse(pResponse, m_Result.str().c_str(), 1);
        if (echoResults && m_pAgentSML)
            m_pAgentSML->FireEchoEvent(pConnection, m_Result.str().c_str()) ;

        // Log error
        if (m_pLogFile) (*m_pLogFile) << m_Result.str() << std::endl;
    }

    // reset state
    m_Result.str("");

    // Delete all remaining xml objects
    for ( ElementXMLListIter cleanupIter = m_ResponseTags.begin(); cleanupIter != m_ResponseTags.end(); ++cleanupIter )
        delete *cleanupIter;

    m_ResponseTags.clear();    
}

bool CommandLineInterface::CheckForHelp(std::vector<std::string>& argv) 
{
    // Standard help check if there is more than one argument
    if (argv.size() > 1) 
    {
        // Is one of the two help strings present?
        if (argv[1] == "-h" || argv[1] == "--help")
            return true;
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
    if (!ret) return SetError("Error getting current working directory.");

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
    m_LastError = error;
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
                    (*m_pLogFile) << msg;
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
                    (*m_pLogFile) << msg;
            }
        }
    }
    else if (eventID == smlEVENT_BEFORE_PRODUCTION_REMOVED)
    {
        // Only called when source command is active
        production* p = static_cast<production*>(pCallData);
        assert(p);
        assert(p->name->sc.name);
        m_ExcisedDuringSource.push_back(std::string(p->name->sc.name));
    }
    else
    {
        assert(false);
        // unknown event
        // TODO: gracefully (?) deal with this error
    }
} // function

bool CommandLineInterface::IsLogOpen() {
	return m_pLogFile ? true : false;
}
