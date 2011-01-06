/** 
 * @file rlcli.h
 * @author Jonathan Voigt <voigtjr@gmail.com>
 * @date 2011
 *
 * Simple command line app for running RL experiments. 
 * Based on TestCLI.
 */

#ifndef RLCLI_H
#define RLCLI_H

#include <string>
#include <iostream>
#include <queue>
#include <vector>

#include "misc.h"
#include "sml_Client.h"
#include "thread_Thread.h"
#include "thread_Lock.h"
#include "thread_Event.h"
#include "ElementXML.h"

void PrintCallbackHandler(sml::smlPrintEventId, void*, sml::Agent*, char const*);
void XMLCallbackHandler(sml::smlXMLEventId, void*, sml::Agent*, sml::ClientXML*);
void InterruptCallbackHandler(sml::smlSystemEventId, void*, sml::Kernel*);

/** 
 * Reads lines from input and stores them in a queue for processing.
 *
 * InputThread blocks on stdin using std::getline reading strings delimited by
 * newline characters. This operation must be in its own thread so that its
 * results can be polled in a callback during a run.
 */
class InputThread : public soar_thread::Thread
{
public:
    virtual ~InputThread() {}

    /**
     * Thread entry point. 
     *
     * This thread spends most of its time blocking on getline. It loops until
     * stdin goes bad or the thread is stopped before it blocks on getline
     * again. Most of the time this thread can't be stopped because quit isn't
     * called until after it is blocking again.
     */
    virtual void Run()
    {
        soar_thread::Lock* lock;
        while (!this->m_QuitNow && std::cin.good())
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

    /**
     * Get a line of input, block until it is received.
     * @param[out] Buffer to copy the line of input to.
     */
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

    /**
     * Try to get a line of input, do not block if one is not available.
     * @param[out] Buffer to copy the line of input to, only valid if function returns true.
     * @return true if a line of input was received.
     */
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
    soar_thread::Mutex mutex;           ///< Protects lines
    soar_thread::Event write_event;     ///< Signals new input
    std::string line;                   ///< Buffer for getline
    std::queue<std::string> lines;      ///< Lines read from input, protected by mutex
};

struct run_data
{
    int decisions;

};

/**
 * Container for Soar objects and the main program loop with some extra logic
 * to make the output look nice.
 */
class CommandProcessor {
public:
    /**
     * Creates the object, must call initialize next.
     */
    CommandProcessor()
        : kernel(0), agent(0), quit(false), seen_newline(true), runs(-1)
    {
    }

    ~CommandProcessor()
    {
        if (kernel)
        {
            kernel->StopAllAgents();
            agent = 0;
            delete kernel;
            kernel = 0;
        }
    }

    /**
     * Create and prepare Soar interface, start input thread.
     * @return false on failure, error message to stderr
     */
    bool initialize()
    {
        kernel = sml::Kernel::CreateKernelInCurrentThread(sml::Kernel::kDefaultLibraryName, true, sml::Kernel::kUseAnyPort);
        if(!kernel || kernel->HadError()) 
        {
            std::cerr << "Error creating kernel";
            if (kernel)
                std::cerr << ": " << kernel->GetLastErrorDescription();
            std::cerr << std::endl;
            return false;
        }

        agent = kernel->CreateAgent("soar");
        if (!agent)
        {
            std::cerr << "Error creating agent: " << kernel->GetLastErrorDescription() << std::endl;
            return false;
        }

        kernel->RegisterForSystemEvent(sml::smlEVENT_INTERRUPT_CHECK, InterruptCallbackHandler, this);
        kernel->SetInterruptCheckRate(10);

        agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, this);

        // No change tracking
        agent->SetOutputLinkChangeTracking(false);

        std::cout << "Help text goes here." << std::endl;
        input.Start();
        return true;
    }

    /**
     * Source a file, report errors to stderr.
     * @param[in] The file to source, passed to source command so relative to
     *            working directory.
     * @return false on error
     */
    bool source(const char* sourcefile)
    {
        if (!agent->LoadProductions(sourcefile))
        {
            std::cerr << "Error loading productions: " << agent->GetLastErrorDescription() << std::endl;
            return false;
        }

        return true;
    }

    /**
     * Run the command loop, returning when major error (not an error with a
     * command) or when "quit" is entered.
     */
    void loop()
    {
        while (!quit)
        {
            prompt();
            std::string line;
            input.get_line(line);
            process_line(line);
        }
    }

    /**
     * Called while Soar is running to pump the command line queue, allowing
     * "stop-soar" to work.
     */
    void update()
    {
        if (quit)
            kernel->StopAllAgents();

        std::string line;
        if (input.try_get_line(line))
        {
            // stop rlcli on any user input
            runs = -1;

            process_line(line);
        }
    }

    /**
     * Helper function called by print callbacks to assist in formatting output to the screen.
     *
     * Call with a true value when the last character written is a newline.
     * @param[in] New setting.
     */
    void newline(bool seen)
    {
        seen_newline = seen;
    }

private:
    sml::Kernel* kernel;
    sml::Agent* agent;
    bool quit;
    InputThread input;
    bool seen_newline;      ///< True if last character printed is a newline.
    int runs;
    std::vector<run_data> data;

    void println()
    {
        if (!seen_newline)
            std::cout << "\n";
        seen_newline = true;
    }

    void prompt()
    {
        println();
        std::cout << "soar> ";
        std::cout.flush();
    }

    void process_line(const std::string& line)
    {
        if (line.empty())
            return;

        if (line.substr(0, 4) == "quit" || line.substr(0, 4) == "exit")
        {
            quit = true;
        }
        else if (line.substr(0,5) == "rlcli")
        {
            if (runs != -1)
            {
                std::cout << "Already running." << std::endl;
                return;
            }

            if (line.length() < 7)
            {
                std::cout << "Number of runs required." << std::endl;
                return;
            }

            if (!from_string(runs, line.substr(6)))
            {
                std::cout << "Error parsing number of runs." << std::endl;
                return;
            }

            if (runs <= 0)
            {  
                std::cout << "Runs must be positive." << std::endl;
                runs = -1;
                return;
            }

            data.clear();

            for (int i = 0; i < runs; ++i)
            {
                print_and_check_newline(kernel->RunAllAgentsForever());
                println();

                const char* dcs = agent->ExecuteCommandLine("stats --decision");
                run_data d;
                from_string(d.decisions, dcs);
                data.push_back(d);

                print_and_check_newline(agent->InitSoar());
                println();
            }

            std::vector<run_data>::iterator iter = data.begin();
            std::cout << "\nSummary: ";
            while (iter != data.end())
            {
                std::cout << iter->decisions << ", ";
                ++iter;
            }
            std::cout << std::endl;
            runs = -1;
        }
        else
        {
            print_and_check_newline(agent->ExecuteCommandLine(line.c_str()));
        }
    }

    void print_and_check_newline(const char* out)
    {
        if (out && strlen(out))
            seen_newline = out[strlen(out) - 1] == '\n';
        std::cout << out;
    }
};

#endif // RLCLI_H
