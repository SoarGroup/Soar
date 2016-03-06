#include "soar_cli.h"

#include "ansi_colors.h"

//#include <signal.h>

using namespace std;
using namespace sml;
namespace acc = ANSI_Color_Constants;

int main(int argc, char** argv)
{
#ifdef _DEBUG
    //_crtBreakAlloc = 2168;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    // create local scope to help with leak detection
    {
        int i, port = sml::Kernel::kUseAnyPort;
        bool parse_error = false, interactive = true;

        std::vector<std::string> sources;
        std::vector<std::string> cmds;
        std::vector<std::string>::iterator j;
        std::string a;

        SoarCLI cmd;

        for (i = 1; i < argc; ++i)
        {
            a = argv[i];
            if (a == "-l")
            {
                cmd.set_listen(true);
            }
            else if (a == "-p")
            {
                if (i + 1 >= argc || !(stringstream(argv[++i]) >> port))
                {
                    parse_error = true;
                    break;
                } else {
                    cmd.set_port(port);
                }
            }
            else if (a == "-s")
            {
                if (i + 1 >= argc)
                {
                    parse_error = true;
                    break;
                }
                sources.push_back(argv[++i]);
            }
            else if (!a.empty() && a[0] == '-')
            {
                parse_error = true;
                break;
            }
            else
            {
                break;
            }
        }
        if (parse_error)
        {
            cout << acc::Red << "Usage: " << acc::IYellow << argv[0] << " [-p <port>] [-s <source>] <commands> ..." << acc::Off << endl;
            return 1;
        }

        for (; i < argc; ++i)
        {
            cmds.push_back(argv[i]);
            interactive = false;
        }

        if (!cmd.initialize())
        {
            exit(1);
        }

        for (j = sources.begin(); j != sources.end(); ++j)
        {
            cmd.source((*j).c_str());
        }

        for (j = cmds.begin(); j != cmds.end(); ++j)
        {
            cmd.process_line((*j));
        }

        if (interactive)
        {
            cmd.loop();
        }
    }

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif // _DEBUG
    return 0;
}

/**
 * Handler for "raw" print events, the default.
 */
void PrintCallbackHandler(sml::smlPrintEventId, void* userdata, sml::Agent*, char const* message)
{
    SoarCLI* cmd = reinterpret_cast<SoarCLI*>(userdata);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;   // simply display whatever comes back through the event
}

/**
 * Handler for "structured" print events.
 */
void XMLCallbackHandler(sml::smlXMLEventId, void* userdata, sml::Agent*, sml::ClientXML* pXML)
{
    SoarCLI* cmd = reinterpret_cast<SoarCLI*>(userdata);
    char* message = pXML->GenerateXMLString(true, true);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;
    pXML->DeleteString(message);
}

/**
 * Handler to pump events during a run.
 */
void InterruptCallbackHandler(sml::smlSystemEventId /*id*/, void* userdata, sml::Kernel* kernel)
{
    SoarCLI* cmd = reinterpret_cast<SoarCLI*>(userdata);
    cmd->update();
    kernel->CheckForIncomingCommands();
}

//
//void SoarCLI::SignalCallbackHandler(int sig)
//{
//    vector<Agent*>::iterator iter;
//    for (iter = agents.begin(); iter != agents.end(); ++iter)
//    {
//        (*iter)->StopSelf();
//    }
//    signal(SIGINT, SoarCLI::SignalCallbackHandler);
//}

SoarCLI::~SoarCLI()
{
    if (m_kernel)
    {
        m_kernel->StopAllAgents();
        m_currentAgent = NULL;
        delete m_kernel;
        m_kernel = NULL;
    }
}
bool SoarCLI::initialize()
{
    if (m_listen)
    {
        cout << acc::Red << "Launching Soar 9.5.0..." << endl << acc::Off << "...created Soar kernel in new thread ";
        if (m_port > 0)
        {
            cout << "on a random port" << endl;
        } else {
            cout << "on port " << m_port << endl;

        }
        m_kernel = Kernel::CreateKernelInNewThread(m_port);
    }
    else
    {
        cout << acc::Red << "Launching Soar 9.5.0..." << endl << acc::Off << "...created Soar kernel in current thread ";
        if (m_port > 0)
        {
            cout << "using a random port" << endl;
        } else {
            cout << "using port " << m_port << endl;

        }
        m_kernel = Kernel::CreateKernelInCurrentThread(true, m_port);
    }

    if (!m_kernel || m_kernel->HadError())
    {
        std::cerr << "Error creating kernel";
        if (m_kernel)
        {
            std::cerr << ": " << m_kernel->GetLastErrorDescription();
        }
        std::cerr << std::endl;
        return false;
    }

    if (!createagent("soar"))
    {
        std::cerr << "Error creating agent: " << m_kernel->GetLastErrorDescription() << std::endl;
        return false;
    }

//    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
//    {
//        signal(SIGINT, SoarCLI::SignalCallbackHandler);
//    }

//    std::cout << "Use the meta-commands 'raw' and 'structured' to switch output style" << std::endl;

    m_input.Start();
    return true;
}

void SoarCLI::agent_init_source(const char* agentname)
{
    std::stringstream ss;
    ss << "source " << agentname << ".soar";
    std::cout << m_currentAgent->ExecuteCommandLine(ss.str().c_str(), false) << std::endl;
    if (m_currentAgent->GetLastCommandLineResult())
    {
        cout << "loaded agent-specific initialization file " << agentname << ".soar." << endl;
    }
}

bool SoarCLI::source(const char* sourcefile)
{
    std::stringstream ss;
    ss << "source " << sourcefile;
    std::cout << m_currentAgent->ExecuteCommandLine(ss.str().c_str()) << std::endl;
    return m_currentAgent->GetLastCommandLineResult();
}

void SoarCLI::loop()
{
    while (!m_quit)
    {
        prompt();
        std::string line;
        m_input.get_line(line);
        m_seen_newline = true;
        process_line(line);
    }
}

void SoarCLI::update()
{
    if (m_quit)
    {
        m_kernel->StopAllAgents();
    }

    std::string line;
    if (m_input.try_get_line(line))
    {
        m_seen_newline = true;
        process_line(line);
    }
}

void SoarCLI::prompt() const
{
    if (!m_seen_newline)
    {
        std::cout << "\n";
    }
    cout << endl << acc::BIYellow << m_currentAgent->GetAgentName() << acc::BBlue << " % " << acc::Off;
    std::cout.flush();
}

void SoarCLI::process_line(const std::string& line)
{
    if (line.empty())
    {
        return;
    }

    if (line == "quit" || line == "exit")
    {
        m_quit = true;
    }
    else if (line == "list")
    {
        printagents();
    }
    else if (line.substr(0, 6) == "create")
    {
        if (line.length() >= 8)
        {
            createagent(line.substr(7).c_str());
        }
    }
    else if (line.substr(0, 6) == "delete")
    {
        if (line.length() >= 8)
        {
            deleteagent(line.substr(7).c_str());
        }
    }
    else if (line.substr(0, 6) == "switch")
    {
        if (line.length() >= 8)
        {
            switchagent(line.substr(7).c_str());
        }
    }
    else
    {
        const char* out = m_currentAgent->ExecuteCommandLine(line.c_str());
        if (out && strlen(out))
        {
            m_seen_newline = out[strlen(out) - 1] == '\n';
        }
        std::cout << out;
    }
}

void SoarCLI::updateMultiAgent()
{
    m_isMultiAgent = (agents.size() > 1);
    if (m_isMultiAgent)
        cout << endl << acc::Red << "Soar CLI in multiple agent mode.  Use " << acc::IYellow <<
             "list or [switch|create|delete] " << acc::Purple << "<agent-name>" << acc::Red <<
             " to manage agents." << acc::Off << endl;
    else
        cout << endl << acc::Red << "Soar CLI in single agent mode.  Use " << acc::IYellow <<
             "create " << acc::Purple << "<agent-name>" << acc::Red << " to create another agent." << acc::Off << endl;

    m_longestAgentName = 0;
    vector<Agent*>::iterator iter;
    for (iter = agents.begin(); iter != agents.end(); ++iter)
    {
        if (strlen((*iter)->GetAgentName()) > m_longestAgentName)
        {
            m_longestAgentName = strlen((*iter)->GetAgentName());
        }
    }
    ++m_longestAgentName;
}

bool SoarCLI::createagent(const char* agentname)
{
    m_currentAgent = m_kernel->CreateAgent(agentname);
    if (!m_currentAgent) return false;

    m_kernel->RegisterForSystemEvent(sml::smlEVENT_INTERRUPT_CHECK, InterruptCallbackHandler, this);
    m_kernel->SetInterruptCheckRate(10);

    m_currentAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, this);

    // No change tracking
    m_currentAgent->SetOutputLinkChangeTracking(false);

    agents.push_back(m_currentAgent);
    cout << "...created agent #" << agents.size() << " named '" << agentname << "'" << endl;
//    agent_init_source(agentname);
    updateMultiAgent();
    return true;
}

void SoarCLI::printagents()
{
    if (agents.size() == 0)
    {
        cout << acc::Red << "No agents currently exist." << acc::Off << endl;
    }

    vector<Agent*>::iterator iter;
    int x = 1;
    cout << acc::Red << "===============" << acc::Off << endl;
    cout << acc::IYellow << "Soar Agent List" << acc::Off << endl;
    cout << acc::Red << "===============" << acc::Off << endl;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        cout << acc::BBlue << "Agent " << x << ": " << acc::Purple << (*iter)->GetAgentName() << "" << acc::Off << endl;
    }
}

void SoarCLI::switchagent(const char* agentname)
{
    vector<Agent*>::iterator iter;
    int x = 1;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        if (!strcmp((*iter)->GetAgentName(), agentname))
        {
            m_currentAgent = (*iter);
            cout << acc::Red << "Switched to agent " << x << " named " << (*iter)->GetAgentName() << "." << acc::Off << endl;
            return;
        }
    }
    cout << acc::Red << "Could not find agent named " << agentname << "." << acc::Off << endl;
}

void SoarCLI::deleteagent(const char* agentname)
{
    vector<Agent*>::iterator iter;
    int x = 1;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        if (!strcmp((*iter)->GetAgentName(), agentname))
        {
            if (m_currentAgent == (*iter))
            {
                m_currentAgent = NULL;
            }
            cout << acc::Red << "Destroying agent " << x << " named " << (*iter)->GetAgentName() << "." << acc::Off << endl;
            m_kernel->DestroyAgent(*iter);
            agents.erase(iter);
            printagents();
            if (!m_currentAgent && agents.size())
            {
                m_currentAgent = agents.front();
                cout << acc::Red << "Switched to agent 1 named " << m_currentAgent->GetAgentName() << "." << acc::Off << endl;
            }
            updateMultiAgent();
            return;
        }
    }
    cout << acc::Red << "Could not find agent named " << agentname << "." << acc::Off << endl;
}

void SoarCLI::sendAllAgentsCommand(const char* cmd)
{
    vector<Agent*>::iterator iter;
    for (iter = agents.begin(); iter != agents.end(); ++iter)
    {
        (*iter)->ExecuteCommandLine(cmd);
    }
}
