#include <portability.h>

#include "testcli.h"

/**
 * Entry point, expected arguments are 0 or more files to source before
 * starting the interactive input loop.
 */
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

        for (int i = 1; i < argc; ++i)
        {
            if (!cmd.source(argv[i]))
                exit(2);
        }

        cmd.loop();
    }

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG
    return 0;
}

/**
 * Handler for "raw" print events, the default.
 */
void PrintCallbackHandler(sml::smlPrintEventId, void* userdata, sml::Agent*, char const* message) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;	// simply display whatever comes back through the event
}

/**
 * Handler for "structured" print events.
 */
void XMLCallbackHandler(sml::smlXMLEventId, void* userdata, sml::Agent*, sml::ClientXML* pXML) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    char* message = pXML->GenerateXMLString(true, true);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;
    pXML->DeleteString(message);
}

/**
 * Handler to pump events during a run.
 */
void InterruptCallbackHandler(sml::smlSystemEventId /*id*/, void* userdata, sml::Kernel* kernel) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->update();
    kernel->CheckForIncomingCommands();
}

