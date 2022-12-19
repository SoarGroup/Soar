#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "sml_Names.h"

#include <map>

using namespace cli;
using namespace sml;

std::map<std::string, const char*> docstrings;

void initdocstrings();

bool CommandLineInterface::Print_9_4_Help_Mapping(std::string pCmd, const char* pNewCmd, const char* pNewCmdFull)
{
    std::map<std::string, const char*>::iterator i;

    if ((i = docstrings.find(pNewCmd)) != docstrings.end())
    {
        m_Result << i->second;
        m_Result << std::endl << "Note: This was the '" << pNewCmd << "' help page.  The '" << pCmd << "' command is now '" << pNewCmdFull << "'." << std::endl;
        return true;
    }
    return false;
}

bool CommandLineInterface::Find_Closest_Help_Command(const std::string pCmd)
{
    std::list<std::string> foundList;
    std::string lStr;

    for (auto it = docstrings.begin(); it != docstrings.end(); it++)
    {
        lStr = it->first;
        if (lStr.find(pCmd) == 0)
        {
            foundList.push_back(lStr);
        }
    }
    if (foundList.empty())
    {
        return false;
    }
    else if (foundList.size() == 1)
    {
        std::map<std::string, const char*>::iterator i;

        if ((i = docstrings.find(foundList.front())) != docstrings.end())
        {
            m_Result << i->second;
            return true;
        }
    } else {
        m_Result << "I'm not sure which help page you want.  Did you mean";
        std::string lastOption("");
        bool firstItem = true;
        for (auto it = foundList.begin(); it != foundList.end(); it++)
        {
            if (!lastOption.empty())
            {
                m_Result << (((firstItem || (foundList.size() == 2))) ? " " : ", ") << lastOption;
                firstItem = false;
            }
            lastOption = *it;
        }
        m_Result << " or " << lastOption << "?\n";
    }

    return true;
}

bool CommandLineInterface::DoHelp(const std::vector<std::string>& argv)
{
    std::map<std::string, const char*>::iterator i;

    if (docstrings.size() == 0)
    {
        initdocstrings();
    }

    if (argv.size() == 1)
    {
        m_Result << "Soar " << sml_Names::kSoarVersionValue << " Command List:" << std::endl << std::endl;
        for (i = docstrings.begin(); i != docstrings.end(); ++i)
        {
            m_Result << i->first << std::endl;
        }
        m_Result << "\nTo read an in-depth description of the command:               help <command>\n" <<
                    "To get a quick summary of sub-command and settings:           <command> ?\n";
        m_Result << "\nNote:  Many previous Soar commands are now sub-commands. To locate a help \n"
                      "       entry, try 'help <old command name>'.";
    }
    else
    {
        if ((i = docstrings.find(argv[1])) == docstrings.end())
        {
            /* If user does a help on a command whose name changed in 9.6.0, give them a little help */
            if (argv[1] == std::string("gds-print")) Print_9_4_Help_Mapping(argv[1], "print", "print --gds");
            else if (argv[1] == std::string("unalias")) Print_9_4_Help_Mapping(argv[1], "alias", "alias -r");
            else if (argv[1] == std::string("capture-input")) Print_9_4_Help_Mapping(argv[1], "save", "save percepts");
            else if (argv[1] == std::string("learn")) Print_9_4_Help_Mapping(argv[1], "chunk", "chunk");
            else if (argv[1] == std::string("watch")) Print_9_4_Help_Mapping(argv[1], "trace", "trace");

            else if (argv[1] == std::string("chunk-name-format")) Print_9_4_Help_Mapping(argv[1], "chunk", "chunk naming-style");
            else if (argv[1] == std::string("max-chunks")) Print_9_4_Help_Mapping(argv[1], "chunk", "chunk max-chunks");

            else if (argv[1] == std::string("allocate")) Print_9_4_Help_Mapping(argv[1], "debug", "debug allocate");
            else if (argv[1] == std::string("internal-symbols")) Print_9_4_Help_Mapping(argv[1], "debug", "debug internal-symbols");
            else if (argv[1] == std::string("port")) Print_9_4_Help_Mapping(argv[1], "debug", "debug port");
            else if (argv[1] == std::string("time")) Print_9_4_Help_Mapping(argv[1], "debug", "debug time");

            else if (argv[1] == std::string("indifferent-selection")) Print_9_4_Help_Mapping(argv[1], "decide", "decide indifferent-selection");
            else if (argv[1] == std::string("numeric-indifferent-mode")) Print_9_4_Help_Mapping(argv[1], "decide", "decide numeric-indifferent-mode");
            else if (argv[1] == std::string("predict")) Print_9_4_Help_Mapping(argv[1], "decide", "decide predict");
            else if (argv[1] == std::string("select")) Print_9_4_Help_Mapping(argv[1], "decide", "decide select");

            else if (argv[1] == std::string("load-library")) Print_9_4_Help_Mapping(argv[1], "load", "load library");
            else if (argv[1] == std::string("source")) Print_9_4_Help_Mapping(argv[1], "load", "load file");
            else if (argv[1] == std::string("replay-input")) Print_9_4_Help_Mapping(argv[1], "load", "load percepts");
            else if (argv[1] == std::string("rete-net")) Print_9_4_Help_Mapping(argv[1], "load", "load rete-net (also see 'save')");

            else if (argv[1] == std::string("clog")) Print_9_4_Help_Mapping(argv[1], "output", "output log");
            else if (argv[1] == std::string("command-to-file")) Print_9_4_Help_Mapping(argv[1], "output", "output command-to-file");
            else if (argv[1] == std::string("default-wme-depth")) Print_9_4_Help_Mapping(argv[1], "output", "output print-depth");
            else if (argv[1] == std::string("warnings")) Print_9_4_Help_Mapping(argv[1], "output", "output warnings");

            else if (argv[1] == std::string("excise")) Print_9_4_Help_Mapping(argv[1], "production", "production excise");
            else if (argv[1] == std::string("firing-counts")) Print_9_4_Help_Mapping(argv[1], "production", "production firing-counts");
            else if (argv[1] == std::string("matches")) Print_9_4_Help_Mapping(argv[1], "production", "production matches");
            else if (argv[1] == std::string("memories")) Print_9_4_Help_Mapping(argv[1], "production", "production memory-usage");
            else if (argv[1] == std::string("multi-attributes")) Print_9_4_Help_Mapping(argv[1], "production", "production optimize-attribute");
            else if (argv[1] == std::string("pbreak")) Print_9_4_Help_Mapping(argv[1], "production", "production break");
            else if (argv[1] == std::string("production-find")) Print_9_4_Help_Mapping(argv[1], "production", "production find");
            else if (argv[1] == std::string("pwatch")) Print_9_4_Help_Mapping(argv[1], "production", "production watch");

            else if (argv[1] == std::string("cli")) Print_9_4_Help_Mapping(argv[1], "soar", "soar tcl");
            else if (argv[1] == std::string("gp-max")) Print_9_4_Help_Mapping(argv[1], "soar", "soar gp-max");
            else if (argv[1] == std::string("init-soar")) Print_9_4_Help_Mapping(argv[1], "soar", "soar init");
            else if (argv[1] == std::string("max-dc-time")) Print_9_4_Help_Mapping(argv[1], "soar", "soar max-dc-time");
            else if (argv[1] == std::string("max-elaborations")) Print_9_4_Help_Mapping(argv[1], "soar", "soar max-elaborations");
            else if (argv[1] == std::string("max-goal-depth")) Print_9_4_Help_Mapping(argv[1], "soar", "soar max-goal-depth");
            else if (argv[1] == std::string("max-memory-usage")) Print_9_4_Help_Mapping(argv[1], "soar", "soar max-memory-usage");
            else if (argv[1] == std::string("max-nil-output-cycles")) Print_9_4_Help_Mapping(argv[1], "soar", "soar max-nil-output-cycles");
            else if (argv[1] == std::string("srand")) Print_9_4_Help_Mapping(argv[1], "decide", "decide srand");
            else if (argv[1] == std::string("stop-soar")) Print_9_4_Help_Mapping(argv[1], "soar", "soar stop-soar");
            else if (argv[1] == std::string("set-stop-phase")) Print_9_4_Help_Mapping(argv[1], "soar", "soar set-stop-phase");
            else if (argv[1] == std::string("timers")) Print_9_4_Help_Mapping(argv[1], "soar", "soar timers");
            else if (argv[1] == std::string("version")) Print_9_4_Help_Mapping(argv[1], "soar", "soar version");
            else if (argv[1] == std::string("waitsnc")) Print_9_4_Help_Mapping(argv[1], "soar", "soar waitsnc");

            else if (argv[1] == std::string("wma")) Print_9_4_Help_Mapping(argv[1], "wm", "wm activation");
            else if (argv[1] == std::string("add-wme")) Print_9_4_Help_Mapping(argv[1], "wm", "wm add");
            else if (argv[1] == std::string("remove-wme")) Print_9_4_Help_Mapping(argv[1], "wm", "wm remove");
            else if (argv[1] == std::string("watch-wmes")) Print_9_4_Help_Mapping(argv[1], "wm", "wm watch");
            else if (!Find_Closest_Help_Command(argv[1]))
            {
                m_Result << "No help file for '" << argv[1] << "'." << std::endl;
                return false;
            }
            return true;
        }
        m_Result << i->second;
    }
    return true;
}

void initdocstrings()
{
