#include <portability.h>

#include "cli_Test.h"

// Main program
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

// callback functions
void PrintCallbackHandler(sml::smlPrintEventId, void* userdata, sml::Agent*, char const* message) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;	// simply display whatever comes back through the event
}

void XMLCallbackHandler(sml::smlXMLEventId, void* userdata, sml::Agent*, sml::ClientXML* pXML) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    char* message = pXML->GenerateXMLString(true, true);
    cmd->newline(message[strlen(message) - 1] == '\n');
    std::cout << message;
    pXML->DeleteString(message);
}

void InterruptCallbackHandler(sml::smlSystemEventId /*id*/, void* userdata, sml::Kernel* /*pKernel*/) {
    CommandProcessor* cmd = reinterpret_cast<CommandProcessor*>(userdata);
    cmd->update();
}
