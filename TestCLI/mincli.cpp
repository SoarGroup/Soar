/* Minimal Soar CLI suitable for scripting */
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

using namespace std;
using namespace sml;

const int stopsig = SIGINT;
Kernel* kernel = NULL;
Agent* agent = NULL;

// trim from start
static inline std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s)
{
    return ltrim(rtrim(s));
}

/*
 * Determine how many levels of brace nesting this line adds. For
 * example, the line "sp {" will return +1, and the line "{(somthing)}}"
 * will return -1. Will avoid counting braces in quoted strings.
 */
bool totalnesting(string line, int& result)
{
    int nesting = 0;
    size_t p = line.find_first_of("{}|");
    
    while (p != string::npos)
    {
        switch (line[p])
        {
            case '{':
                ++nesting;
                break;
            case '}':
                --nesting;
                break;
            case '|':
                // skip over quoted string
                while (true)
                {
                    p = line.find_first_of('|', p + 1);
                    if (p == string::npos)
                    {
                        // error, no closing quote pipe on line
                        return false;
                    }
                    if (line[p - 1] != '\\')
                    {
                        break;
                    }
                }
                break;
        }
        p = line.find_first_of("{}|", p + 1);
    }
    result = nesting;
    return true;
}

/* Read a command, spanning newlines if the command contains unclosed braces */
bool readcmd(string& result)
{
    int nestlvl, i, n;
    string line;
    stringstream cmd;
    
    nestlvl = 0;
    while (getline(cin, line))
    {
        if (!totalnesting(line, n))
        {
            return false;
        }
        nestlvl += n;
        cmd << line << endl;
        if (nestlvl < 0)
        {
            return false;
        }
        else if (nestlvl == 0)
        {
            break;
        }
    }
    
    if (nestlvl > 0)
    {
        return false;
    }
    result = cmd.str();
    if (!result.empty() && result[result.size() - 1] == '\n')
    {
        result.erase(result.size() - 1);
    }
    return true;
}

void printcb(smlPrintEventId id, void* d, Agent* a, char const* m)
{
    string msg(m);
    cout << trim(msg) << endl;
}

void execcmd(const string& c)
{
    bool isident = false;
    string out;
    
    if (c == "exit")
    {
        agent->ExecuteCommandLine("halt");
        kernel->Shutdown();
        delete kernel;
        exit(0);
    }
    else
    {
    
        /* -- The following is a shortcut to allow the user to simply type an
         *    identifier on the command line to print it.  Not sure how
         *    useful this is, but it was in the old mincli but had a bug. -- */
        if ((c.length() > 1) && isupper(c[0]))
    {
        isident = true;
        for (int i = 1; i < c.size(); ++i)
        {
            if (!isdigit(c[i]))
            {
                isident = false;
            }
        }
    }
        string pc;
    if (isident)
    {
            pc.assign("print ");
        pc += c;
    }
    else
    {
            pc = c;
    }
        
        out = agent->ExecuteCommandLine(pc.c_str());
        out = trim(out);
        cout << out;
    }
}

void repl()
{
    string cmd, last;
    
    while (cin)
    {
        cout << endl << "% ";
        if (!readcmd(cmd))
        {
            cout << "?" << endl;
            continue;
        }
        if (!cin)
        {
            return;
        }
        if (cmd.empty() && !last.empty())
        {
            execcmd(last);
        }
        else
        {
            last = cmd;
            execcmd(cmd);
        }
    }
    cout << endl;
}

void sighandler(int sig)
{
    if (agent)
    {
        agent->StopSelf();
    }
    signal(stopsig, sighandler);
}

string exit_handler(smlRhsEventId id, void* userdata, Agent* agent, char const* fname, char const* args)
{
    int code = atoi(args);
    kernel->Shutdown();
    delete kernel;
    exit(code);
}

string log_handler(smlRhsEventId id, void* userdata, Agent* agent, char const* fname, char const* args)
{
    istringstream ss(args);
    ofstream f;
    bool iscmd, append;
    string fn, text;
    int c;
    
    iscmd = false;
    append = false;
    while (true)
    {
        ss >> fn;
        if (fn == "-c")
        {
            iscmd = true;
        }
        else if (fn == "-a")
        {
            append = true;
        }
        else
        {
            break;
        }
    }
    while ((c = ss.get()) == ' ' || c == '	');  // ignore leading whitespace
    ss.putback(c);
    
    if (iscmd)
    {
        text = agent->ExecuteCommandLine(args + ss.tellg());
    }
    else
    {
        text = (args + ss.tellg());
    }
    
    f.open(fn.c_str(), ios_base::out | (append ? ios_base::app : ios_base::trunc));
    if (!f)
    {
        cerr << "Failed to open " << fn << " for writing" << endl;
        return "";
    }
    if (!(f << text))
    {
        cerr << "Write error " << fn << endl;
        return "";
    }
    f.close();
    return "";
}

int main(int argc, char* argv[])
{
    int i, port = 0;
    bool listen = false, parse_error = false, interactive = true;
    const char* agentname = "soar";
    vector<string> sources;
    vector<string> cmds;
    vector<string>::iterator j;
    
    for (i = 1; i < argc; ++i)
    {
        string a(argv[i]);
        if (a == "-h")
        {
            parse_error = true;
            break;
        }
        else if (a == "-l")
        {
            listen = true;
        }
        else if (a == "-n")
        {
            if (i + 1 >= argc)
            {
                parse_error = true;
                break;
            }
            agentname = argv[++i];
        }
        else if (a == "-p")
        {
            if (i + 1 >= argc || !(stringstream(argv[++i]) >> port))
            {
                parse_error = true;
                break;
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
        cout << "usage: " << argv[0] << " [-o] [-n <agent name>] [-p <port>] [-s <source>] <commands> ..." << endl;
        return 1;
    }
    
    for (; i < argc; ++i)
    {
        cmds.push_back(argv[i]);
        interactive = false;
    }
    
//  pthread_setname_np("minCLI_main_thread");
//  cout << "minCLI thread is " << tname2() << endl;

    if (listen)
    {
        cout << "Instantiating Soar.  (kernel in new thread, port " << port << ")" << endl;
        kernel = Kernel::CreateKernelInNewThread(port);
    }
    else
    {
        cout << "Instantiating Soar.  (kernel in current thread (optimized) on port " << port << ")" << endl;
        kernel = Kernel::CreateKernelInCurrentThread(true, port);
    }
    
    kernel->AddRhsFunction("exit", exit_handler, NULL);
    kernel->AddRhsFunction("log", log_handler, NULL);
    
    agent = kernel->CreateAgent(agentname);
    agent->RegisterForPrintEvent(smlEVENT_PRINT, printcb, NULL);
    agent->SetOutputLinkChangeTracking(false);
    
    if (signal(stopsig, SIG_IGN) != SIG_IGN)
    {
        signal(stopsig, sighandler);
    }
    
    for (j = sources.begin(); j != sources.end(); ++j)
    {
        execcmd("source " + *j);
    }
    
    for (j = cmds.begin(); j != cmds.end(); ++j)
    {
        execcmd(*j);
    }
    
    if (interactive)
    {
        repl();
    }
    kernel->Shutdown();
    delete kernel;
    return 0;
}
