/* Multi-Agent Colored Soar CLI suitable for scripting
 *
 * Based on mincli.cpp */

#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include "sml_Client.h"
#include "ansi_colors.h"

using namespace std;
using namespace sml;
namespace acc = ANSI_Color_Constants;

const int stopsig = SIGINT;
Kernel* kernel = NULL;
Agent* currentAgent = NULL;
vector<Agent*> agents;

bool multiAgent = false;
int longestAgentName = 0;

void execcmd(const string &c);

inline void updateMultiAgent() {
    multiAgent = (agents.size() > 1);
    if (multiAgent)
        cout << endl << acc::Red << "Soar CLI in multiple agent mode.  Use " << acc::IYellow <<
            "list or [switch|create|delete] " << acc::Purple << "<agent-name>" << acc::Red <<
            " to manage agents." << acc::Off << endl;
    else
        cout << endl << acc::Red << "Soar CLI in single agent mode.  Use " << acc::IYellow <<
            "create " << acc::Purple << "<agent-name>" << acc::Red << " to create another agent." << acc::Off << endl;

    longestAgentName = 0;
    vector<Agent*>::iterator iter;
    for (iter = agents.begin(); iter != agents.end(); ++iter)
    {
        if (strlen((*iter)->GetAgentName()) > longestAgentName)
            longestAgentName = strlen((*iter)->GetAgentName());
    }
    ++longestAgentName;
}

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

/*
 * Determine how many levels of brace nesting this line adds. For
 * example, the line "sp {" will return +1, and the line "{(somthing)}}"
 * will return -1. Will avoid counting braces in quoted strings.
 */
bool totalnesting(string line, int &result) {
    int nesting = 0;
    size_t p = line.find_first_of("{}|");

    while (p != string::npos) {
        switch (line[p]) {
            case '{':
                ++nesting;
                break;
            case '}':
                --nesting;
                break;
            case '|':
                // skip over quoted string
                while (true) {
                    p = line.find_first_of('|', p+1);
                    if (p == string::npos) {
                        // error, no closing quote pipe on line
                        return false;
                    }
                    if (line[p-1] != '\\') {
                        break;
                    }
                }
                break;
        }
        p = line.find_first_of("{}|", p+1);
    }
    result = nesting;
    return true;
}

/* Read a command, spanning newlines if the command contains unclosed braces */
bool readcmd(string &result) {
    int nestlvl, i, n;
    string line;
    stringstream cmd;

    nestlvl = 0;
    while(getline(cin, line)) {
        if (!totalnesting(line, n)) {
            return false;
        }
        nestlvl += n;
        cmd << line << endl;
        if (nestlvl < 0) {
            return false;
        } else if (nestlvl == 0) {
            break;
        }
    }

    if (nestlvl > 0) {
        return false;
    }
    result = cmd.str();
    if (!result.empty() && result[result.size()-1] == '\n') {
        result.erase(result.size()-1);
    }
    return true;
}

void printcb(smlPrintEventId id, void *userData, Agent *a, char const *message) {

    if (multiAgent) {
        string prompt_string(a->GetAgentName());
        prompt_string.append((longestAgentName - prompt_string.size()), ' ');
        cout << acc::IYellow << prompt_string << acc::Off;
    }
    string msg(message);
    cout << trim(msg) << endl;
}

void createagent(const char *agentname) {
    currentAgent = kernel->CreateAgent(agentname);
    currentAgent->RegisterForPrintEvent(smlEVENT_PRINT, printcb, NULL);
    currentAgent->SetOutputLinkChangeTracking(false);
    agents.push_back(currentAgent);
    cout << acc::Red << "Created agent " << agents.size() << " named " << agentname <<
            ". Attempting to source " << agentname << ".soar:" << acc::Off << endl;
    execcmd("source settings/settings.soar");
    execcmd("source settings/aliases.soar");
    execcmd("source settings/" + string(agentname) + ".soar");
    updateMultiAgent();
}

void printagents() {
    if (agents.size() == 0) {
        cout << acc::Red << "No agents currently exist." << acc::Off << endl;
    }

    vector<Agent*>::iterator iter;
    int x=1;
    cout << acc::Red << "===============" << acc::Off << endl;
    cout << acc::IYellow << "Soar Agent List" << acc::Off << endl;
    cout << acc::Red << "===============" << acc::Off << endl;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        cout << acc::BBlue << "Agent " << x << ": " << acc::Purple << (*iter)->GetAgentName() << "" << acc::Off << endl;
    }
}

void switchagent(const char *agentname) {
    vector<Agent*>::iterator iter;
    int x=1;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        if (!strcmp((*iter)->GetAgentName(),agentname)) {
            currentAgent = (*iter);
            cout << acc::Red << "Switched to agent " << x << " named " << (*iter)->GetAgentName() << "." << acc::Off << endl;
            return;
        }
    }
    cout << acc::Red << "Could not find agent named " << agentname << "." << acc::Off << endl;
}

void deleteagent(const char *agentname) {
    vector<Agent*>::iterator iter;
    int x=1;
    for (iter = agents.begin(); iter != agents.end(); ++iter, ++x)
    {
        if (!strcmp((*iter)->GetAgentName(),agentname)) {
            if (currentAgent == (*iter))
               currentAgent = NULL;
            cout << acc::Red << "Destroying agent " << x << " named " << (*iter)->GetAgentName() << "." << acc::Off << endl;
            kernel->DestroyAgent(*iter);
            agents.erase(iter);
            printagents();
            if (!currentAgent && agents.size()) {
                currentAgent = agents.front();
                cout << acc::Red << "Switched to agent 1 named " << currentAgent->GetAgentName() << "." << acc::Off << endl;
            }
            updateMultiAgent();
            return;
        }
    }
    cout << acc::Red << "Could not find agent named " << agentname << "." << acc::Off << endl;
}

void sendAllAgentsCommand(const char* cmd) {
    vector<Agent*>::iterator iter;
    for (iter = agents.begin(); iter != agents.end(); ++iter)
    {
        (*iter)->ExecuteCommandLine(cmd);
    }
}

void execcmd(const string &c) {
    bool isident = false;
    string out;

    if (c == "exit") {
        sendAllAgentsCommand("halt");
        kernel->Shutdown();
        delete kernel;
        exit(0);
    } else if (c == "list") {
        printagents();
    } else if (c.substr(0, 6) == "create") {
        if (c.length() >= 8)
            createagent(c.substr(7).c_str());
    } else if (c.substr(0, 6) == "delete") {
        if (c.length() >= 8)
            deleteagent(c.substr(7).c_str());
    } else if (c.substr(0, 6) == "switch") {
        if (c.length() >= 8)
            switchagent(c.substr(7).c_str());
    } else {

        /* -- The following is a shortcut to allow the user to simply type an
         *    identifier on the command line to print it.  Not sure how
         *    useful this is, but it was in the old mincli but had a bug. -- */
        if ((c.length()>1) && isupper(c[0])) {
            isident = true;
            for (int i = 1; i < c.size(); ++i) {
                if (!isdigit(c[i])) {
                    isident = false;
                }
            }
        }
        string pc;
        if (isident) {
            pc.assign("print ");
            pc += c;
        } else {
            pc = c;
        }

        out = currentAgent->ExecuteCommandLine(pc.c_str());
        out = trim(out);
        cout << out;
    }
}

void repl() {
    string cmd, last;

    while (cin) {
        cout << endl << acc::BIYellow << currentAgent->GetAgentName() << acc::BBlue << " % " << acc::Off;
        if (!readcmd(cmd)) {
            cout << acc::BIRed << "Huh?" << acc::Off << endl;
            continue;
        }
        if (!cin.good()) {
            cin.clear(); //clear the error flags
            cin.sync(); //flush the input buffer
        }
        if (cmd.empty() && !last.empty()) {
            execcmd(last);
        } else {
            last = cmd;
            execcmd(cmd);
        }
    }
    cout << acc::Off << endl;
}

void sighandler(int sig) {
    vector<Agent*>::iterator iter;
    for (iter = agents.begin(); iter != agents.end(); ++iter)
    {
        (*iter)->StopSelf();
    }
    signal(stopsig, sighandler);
}

string exit_handler(smlRhsEventId id, void *userdata, Agent *myAgent, char const *fname, char const *args) {
    int code = atoi(args);
    kernel->Shutdown();
    delete kernel;
    exit(code);
}

string log_handler(smlRhsEventId id, void *userdata, Agent *myAgent, char const *fname, char const *args) {
    istringstream ss(args);
    ofstream f;
    bool iscmd, append;
    string fn, text;
    int c;

    iscmd = false; append = false;
    while (true) {
        ss >> fn;
        if (fn == "-c") {
            iscmd = true;
        } else if (fn == "-a") {
            append = true;
        } else {
            break;
        }
    }
    while ((c = ss.get()) == ' ' || c == '\t');  // ignore leading whitespace
    ss.putback(c);

    if (iscmd) {
        text = myAgent->ExecuteCommandLine(args + ss.tellg());
    } else {
        text = (args + ss.tellg());
    }

    f.open(fn.c_str(), ios_base::out | (append ? ios_base::app : ios_base::trunc));
    if (!f) {
        cerr << "Failed to open " << fn << " for writing" << endl;
        return "";
    }
    if (!(f << text)) {
        cerr << "Write error " << fn << endl;
        return "";
    }
    f.close();
    return "";
}

int main(int argc, char *argv[]) {
    int i, port = 0;
    bool listen = false, parse_error = false, interactive = true;
    const char *agentname = "soar";
    vector<string> sources;
    vector<string> cmds;
    vector<string>::iterator j;

    for (i = 1; i < argc; ++i) {
        string a(argv[i]);
        if (a == "-h") {
            parse_error = true;
            break;
        } else if (a == "-l") {
            listen = true;
        } else if (a == "-n") {
            if (i + 1 >= argc) {
                parse_error = true;
                break;
            }
            agentname = argv[++i];
        } else if (a == "-p") {
            if (i + 1 >= argc || !(stringstream(argv[++i]) >> port)) {
                parse_error = true;
                break;
            }
        } else if (a == "-s") {
            if (i + 1 >= argc) {
                parse_error = true;
                break;
            }
            sources.push_back(argv[++i]);
        } else if (!a.empty() && a[0] == '-') {
            parse_error = true;
            break;
        } else {
            break;
        }
    }

    if (parse_error) {
        cout << acc::Red << "Usage: " << acc::IYellow << argv[0] << " [-o] [-n <agent name>] [-p <port>] [-s <source>] <commands> ..." << acc::Off << endl;
        return 1;
    }

    for (; i < argc; ++i) {
        cmds.push_back(argv[i]);
        interactive = false;
    }

//  pthread_setname_np("minCLI_main_thread");
//  cout << "minCLI thread is " << tname2() << acc::Off << endl;

  if (listen) {
      cout << acc::Red << "Instantiating Soar.  (kernel in new thread, port " << port << ")" << acc::Off << endl;
        kernel = Kernel::CreateKernelInNewThread(port);
    } else {
    cout << acc::Red << "Instantiating Soar.  (kernel in current thread (optimized) on port " << port << ")" << acc::Off << endl;
        kernel = Kernel::CreateKernelInCurrentThread(true, port);
    }

    kernel->AddRhsFunction("exit", exit_handler, NULL);
    kernel->AddRhsFunction("log", log_handler, NULL);

    createagent(agentname);

    if (signal(stopsig, SIG_IGN) != SIG_IGN) {
        signal(stopsig, sighandler);
    }

    for (j = sources.begin(); j != sources.end(); ++j) {
        execcmd("source " + *j);
    }

    for (j = cmds.begin(); j != cmds.end(); ++j) {
        execcmd(*j);
    }

    if (interactive) {
        repl();
    }
    kernel->Shutdown();
    delete kernel;
    return 0;
}
