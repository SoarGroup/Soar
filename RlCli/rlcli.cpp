#include <portability.h>

#include "rlcli.h"

const int CommandProcessor::MAX_DECISIONS = 10000;

void usage(char command[])
{
    std::cout << "Usage: " << command << "[productions] [runs]" 
        << std::endl;
}

int main(int argc, char** argv)
{
#ifdef _DEBUG
    //_crtBreakAlloc = 2168;
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
#endif // _DEBUG

    // create local scope to help with leak detection
    {
        CommandProcessor cmd;
        if (!cmd.initialize())
            exit(1);

        if (argc >= 2)
        {
            if (!cmd.source(argv[1]))
                exit(2);
        }

        if (argc >= 3)
        {
            std::cout << "Command line runs not implemented yet. Ignored."
                << std::endl;
        }

        cmd.loop();
    }

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG
    return 0;
}

/**
 * Print handler.
 */
void PrintCallbackHandler(sml::smlPrintEventId, void* userdata, sml::Agent*, char const* message) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;	// simply display whatever comes back through the event
}

/**
 * Handler to pump events during a run.
 */
void InterruptCallbackHandler(sml::smlSystemEventId /*id*/, void* userdata, sml::Kernel* kernel) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->update();
    kernel->CheckForIncomingCommands();
}
