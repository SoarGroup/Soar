#ifndef CLI_TEST_H
#define CLI_TEST_H

#include <string>
#include <iostream>
#include <queue>

#include "sml_Client.h"
#include "thread_Thread.h"
#include "thread_Lock.h"
#include "thread_Event.h"
#include "ElementXML.h"

void PrintCallbackHandler(sml::smlPrintEventId, void*, sml::Agent*, char const*);
void XMLCallbackHandler(sml::smlXMLEventId, void*, sml::Agent*, sml::ClientXML*);
void InterruptCallbackHandler(sml::smlSystemEventId, void*, sml::Kernel*);

class InputThread : public soar_thread::Thread
{
public:
    void Run()
    {
        std::string line;
        soar_thread::Lock* lock;
        while (std::cin.good())
        {
            std::getline(std::cin, line);
            if (std::cin.bad())
                break;

            lock = new soar_thread::Lock(&mutex);
            lines.push(line);
            write_event.TriggerEvent();
            delete lock;
        }
    }

    void get_line(std::string& line)
    {
        soar_thread::Lock* lock = new soar_thread::Lock(&mutex);
        while (lines.empty())
        {
            delete lock;
            write_event.WaitForEventForever();
            lock = new soar_thread::Lock(&mutex);
        }
        line.assign(lines.front());
        lines.pop();
        delete lock;
    }

    bool try_get_line(std::string& line)
    {
        soar_thread::Lock lock(&mutex);
        if (lines.empty())
            return false;
        line.assign(lines.front());
        lines.pop();
        return true;
    }

private:
    soar_thread::Mutex mutex;
    soar_thread::Event write_event;
    std::queue<std::string> lines;
};

class CommandProcessor {
public:
    CommandProcessor()
        : kernel(0), agent(0), raw(true), quit(false), seen_newline(true)
    {
    }

    ~CommandProcessor()
    {
        if (kernel)
        {
            agent = 0;
            delete kernel;
            kernel = 0;
        }
    }

    bool initialize()
    {
        kernel = sml::Kernel::CreateKernelInNewThread(sml::Kernel::kDefaultLibraryName, sml::Kernel::kUseAnyPort);
        if(!kernel || kernel->HadError()) 
        {
            std::cerr << "Error creating kernel";
            if (kernel)
                std::cerr << ": " << kernel->GetLastErrorDescription();
            std::cerr << std::endl;
            return false;
        }

        agent = kernel->CreateAgent("test");
        if (!agent)
        {
            std::cerr << "Error creating agent: " << kernel->GetLastErrorDescription() << std::endl;
            return false;
        }

        kernel->RegisterForSystemEvent(sml::smlEVENT_INTERRUPT_CHECK, InterruptCallbackHandler, this);
        kernel->SetInterruptCheckRate(10);

        trace = agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, this);

        // No change tracking
        agent->SetOutputLinkChangeTracking(false);

        std::cout << "Use the meta-commands 'raw' and 'structured' to switch output style" << std::endl;
        input.Start();
        return true;
    }

    bool source(const char* sourcefile)
    {
        if (!agent->LoadProductions(sourcefile))
        {
            std::cerr << "Error loading productions: " << agent->GetLastErrorDescription() << std::endl;
            return false;
        }

        return true;
    }

    void loop()
    {
        while (!quit)
        {
            prompt();
            std::string line;
            input.get_line(line);
            seen_newline = true;
            process_line(line);
        }
    }

    void update()
    {
        std::string line;
        if (input.try_get_line(line))
            process_line(line);
    }

    void newline(bool seen)
    {
        seen_newline = seen;
    }

private:
    sml::Kernel* kernel;
    sml::Agent* agent;
    bool raw;
    bool quit;
    InputThread input;
    int trace;
    bool seen_newline;

    void prompt() const
    {
        if (!seen_newline)
            std::cout << "\n";
        std::cout << "test:" << (raw ? "raw" : "structured") << "> ";
        std::cout.flush();
    }

    void process_line(const std::string& line)
    {
        if (line.empty())
            return;

        if (line.substr(0, 4) == "quit" || line.substr(0, 4) == "exit")
        {
            quit = true;
            kernel->StopAllAgents();
        }
        else if (line.substr(0, 3) == "raw")
        {
            if (raw)
                return;

            agent->UnregisterForXMLEvent(trace);
            trace = agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, this);
            raw = true;
        }
        else if (line.substr(0, 10) == "structured")
        {
            if (!raw)
                return;

            agent->UnregisterForPrintEvent(trace);
            trace = agent->RegisterForXMLEvent(sml::smlEVENT_XML_TRACE_OUTPUT, XMLCallbackHandler, this);
            raw = false;
        }
        else if (raw)
        {
            const char* out = agent->ExecuteCommandLine(line.c_str());
            seen_newline = out[strlen(out) - 1] == '\n';
            std::cout << out;
        }
        else 
        {
            sml::ClientAnalyzedXML* response = new sml::ClientAnalyzedXML();
            agent->ExecuteCommandLineXML(line.c_str(), response);
            const soarxml::ElementXML* result = response->GetResultTag();

            if (result) 
            {
                char* out = result->GenerateXMLString(true, true);
                seen_newline = out[strlen(out) - 1] == '\n';
                if (out)
                    std::cout << out;
                result->DeleteString(out);
            }

            const soarxml::ElementXML* error = response->GetErrorTag();
            if (error) {
                char* out = error->GenerateXMLString(true, true);
                seen_newline = out[strlen(out) - 1] == '\n';
                if (out)
                    std::cout << out;
                error->DeleteString(out);
            }

            delete response;
        }
    }
};

#endif // CLI_TEST_H
