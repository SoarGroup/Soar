#ifndef SOAR_CLI_H
#define SOAR_CLI_H

/* This is a new command line interface for soar that combines the the cleaner,
 * object-oriented implementation of test cli with the additional features of
 * multicli (which was based on mincli) */

#include "ElementXML.h"
#include "portability.h"
#include "sml_Client.h"
#include "thread_Thread.h"
#include "thread_Lock.h"
#include "thread_Event.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>

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
            bool lReadCmdResult = false;
            while (!this->m_QuitNow && std::cin.good())
            {
                lReadCmdResult = readcmd(line);
                if (!lReadCmdResult || std::cin.bad())
                {
                    break;
                }

                lock = new soar_thread::Lock(&mutex);
                lines.push(line);
                write_event.TriggerEvent();
                if (line == "quit" || line == "exit" || std::cin.eof())
                {
                    this->m_QuitNow = true;
                }
                delete lock;
            }
        }
        bool totalnesting(std::string line, int& result)
        {
            int nesting = 0;
            size_t p = line.find_first_of("{}|");

            while (p != std::string::npos)
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
                            if (p == std::string::npos)
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
        bool readcmd(std::string& result)
        {
            int nestlvl, n;
            std::string line;
            std::stringstream cmd;

            nestlvl = 0;
            while (std::getline(std::cin, line))
            {
                if (!totalnesting(line, n))
                {
                    return false;
                }
                nestlvl += n;
                cmd << line << std::endl;
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
            {
                return false;
            }
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

class SoarCLI
{
    public:

        SoarCLI()
            : m_kernel(0), m_currentAgent(0), m_quit(false), m_isMultiAgent(false),
              m_longestAgentName(0), m_seen_newline(true),
              #if defined(NO_COLORS)
                  m_color(false),
              #else
                  m_color(stdout_supports_ansi_colors()),
              #endif
              m_listen(false), m_port(sml::Kernel::kUseAnyPort) {}

        ~SoarCLI();

        bool initialize();
        bool source(const char* sourcefile);
        void process_line(const std::string& line);
        void loop();
        void update();
        void newline(bool seen)                         { m_seen_newline    = seen; }
        void set_port(int pPort)                        { m_port            = pPort; };
        void set_listen(bool pListen)                   { m_listen          = pListen; };
        void set_color_enabled(bool pColor)             { m_color           = pColor; };
        inline const char* getcol(const char* pColor)   { if (m_color) return pColor; else return ""; }
        void PrintColoredMessage(char const* message);

//      void SignalCallbackHandler(int sig);

    private:

        InputThread     m_input;
        sml::Kernel*    m_kernel;
        sml::Agent*     m_currentAgent;
        int             m_port;
        bool            m_listen;
        bool            m_color;
        bool            m_quit;
        bool            m_seen_newline;      ///< True if last character printed is a newline.
        bool            m_isMultiAgent;
        int             m_longestAgentName;

        std::vector<sml::Agent*> agents;

        void prompt();
        bool createagent(const char* agentname);
        void printagents();
        void switchagent(const char* agentname);
        void updateMultiAgent();
        void deleteagent(const char* agentname);
        void sendAllAgentsCommand(const char* cmd);

};

#endif // SOAR_CLI_H

