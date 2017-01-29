#include <map>
#include "portability.h"
#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

std::map<std::string, const char*> docstrings;

void initdocstrings();

bool CommandLineInterface::backwards_compatibility_help(std::string pCmd, const char* pNewCmd, const char* pNewCmdFull)
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

bool CommandLineInterface::DoHelp(const std::vector<std::string>& argv)
{
    std::map<std::string, const char*>::iterator i;
    
    if (docstrings.size() == 0)
    {
        initdocstrings();
    }
    
    if (argv.size() == 1)
    {
        m_Result << "Soar 9.6.0 Command List:" << std::endl << std::endl;
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
            if (argv[1] == std::string("gds-print")) backwards_compatibility_help(argv[1], "print", "print --gds");
            else if (argv[1] == std::string("unalias")) backwards_compatibility_help(argv[1], "alias", "alias -r");
            else if (argv[1] == std::string("capture-input")) backwards_compatibility_help(argv[1], "save", "save percepts");
            else if (argv[1] == std::string("learn")) backwards_compatibility_help(argv[1], "chunk", "chunk");
            else if (argv[1] == std::string("watch")) backwards_compatibility_help(argv[1], "trace", "trace");

            else if (argv[1] == std::string("chunk-name-format")) backwards_compatibility_help(argv[1], "chunk", "chunk naming-style");
            else if (argv[1] == std::string("max-chunks")) backwards_compatibility_help(argv[1], "chunk", "chunk max-chunks");

            else if (argv[1] == std::string("allocate")) backwards_compatibility_help(argv[1], "debug", "debug allocate");
            else if (argv[1] == std::string("internal-symbols")) backwards_compatibility_help(argv[1], "debug", "debug internal-symbols");
            else if (argv[1] == std::string("port")) backwards_compatibility_help(argv[1], "debug", "debug port");
            else if (argv[1] == std::string("time")) backwards_compatibility_help(argv[1], "debug", "debug time");

            else if (argv[1] == std::string("indifferent-selection")) backwards_compatibility_help(argv[1], "decide", "decide indifferent-selection");
            else if (argv[1] == std::string("numeric-indifferent-mode")) backwards_compatibility_help(argv[1], "decide", "decide numeric-indifferent-mode");
            else if (argv[1] == std::string("predict")) backwards_compatibility_help(argv[1], "decide", "decide predict");
            else if (argv[1] == std::string("select")) backwards_compatibility_help(argv[1], "decide", "decide select");

            else if (argv[1] == std::string("load-library")) backwards_compatibility_help(argv[1], "load", "load library");
            else if (argv[1] == std::string("source")) backwards_compatibility_help(argv[1], "load", "load file");
            else if (argv[1] == std::string("replay-input")) backwards_compatibility_help(argv[1], "load", "load percepts");
            else if (argv[1] == std::string("rete-net")) backwards_compatibility_help(argv[1], "load", "load rete-net (also see 'save')");

            else if (argv[1] == std::string("clog")) backwards_compatibility_help(argv[1], "output", "output log");
            else if (argv[1] == std::string("command-to-file")) backwards_compatibility_help(argv[1], "output", "output command-to-file");
            else if (argv[1] == std::string("default-wme-depth")) backwards_compatibility_help(argv[1], "output", "output print-depth");
            else if (argv[1] == std::string("warnings")) backwards_compatibility_help(argv[1], "output", "output warnings");

            else if (argv[1] == std::string("excise")) backwards_compatibility_help(argv[1], "production", "production excise");
            else if (argv[1] == std::string("firing-counts")) backwards_compatibility_help(argv[1], "production", "production firing-counts");
            else if (argv[1] == std::string("matches")) backwards_compatibility_help(argv[1], "production", "production matches");
            else if (argv[1] == std::string("memories")) backwards_compatibility_help(argv[1], "production", "production memory-usage");
            else if (argv[1] == std::string("multi-attributes")) backwards_compatibility_help(argv[1], "production", "production optimize-attribute");
            else if (argv[1] == std::string("pbreak")) backwards_compatibility_help(argv[1], "production", "production break");
            else if (argv[1] == std::string("production-find")) backwards_compatibility_help(argv[1], "production", "production find");
            else if (argv[1] == std::string("pwatch")) backwards_compatibility_help(argv[1], "production", "production watch");

            else if (argv[1] == std::string("cli")) backwards_compatibility_help(argv[1], "soar", "soar tcl");
            else if (argv[1] == std::string("gp-max")) backwards_compatibility_help(argv[1], "soar", "soar gp-max");
            else if (argv[1] == std::string("init-soar")) backwards_compatibility_help(argv[1], "soar", "soar init");
            else if (argv[1] == std::string("max-dc-time")) backwards_compatibility_help(argv[1], "soar", "soar max-dc-time");
            else if (argv[1] == std::string("max-elaborations")) backwards_compatibility_help(argv[1], "soar", "soar max-elaborations");
            else if (argv[1] == std::string("max-goal-depth")) backwards_compatibility_help(argv[1], "soar", "soar max-goal-depth");
            else if (argv[1] == std::string("max-memory-usage")) backwards_compatibility_help(argv[1], "soar", "soar max-memory-usage");
            else if (argv[1] == std::string("max-nil-output-cycles")) backwards_compatibility_help(argv[1], "soar", "soar max-nil-output-cycles");
            else if (argv[1] == std::string("srand")) backwards_compatibility_help(argv[1], "soar", "soar srand");
            else if (argv[1] == std::string("stop-soar")) backwards_compatibility_help(argv[1], "soar", "soar stop-soar");
            else if (argv[1] == std::string("set-stop-phase")) backwards_compatibility_help(argv[1], "soar", "soar set-stop-phase");
            else if (argv[1] == std::string("timers")) backwards_compatibility_help(argv[1], "soar", "soar timers");
            else if (argv[1] == std::string("version")) backwards_compatibility_help(argv[1], "soar", "soar version");
            else if (argv[1] == std::string("waitsnc")) backwards_compatibility_help(argv[1], "soar", "soar waitsnc");

            else if (argv[1] == std::string("wma")) backwards_compatibility_help(argv[1], "wm", "wm activation");
            else if (argv[1] == std::string("add-wme")) backwards_compatibility_help(argv[1], "wm", "wm add");
            else if (argv[1] == std::string("remove-wme")) backwards_compatibility_help(argv[1], "wm", "wm remove");
            else if (argv[1] == std::string("watch-wmes")) backwards_compatibility_help(argv[1], "wm", "wm watch");
            else
            {
                m_Result << "No such command" << std::endl;
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
